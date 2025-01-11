#include "api.h"

#include <iostream>
#include <ostream>
#include <windows.h>
#include "lru_policy.h"


struct FileDescriptor {
    HANDLE handle;      // Дескриптор файла
    std::string path;   // Путь к файлу (опционально, для отладки или логирования)
    size_t position;    // Текущая позиция в файле
};

static std::unordered_map<lab2_fd, FileDescriptor> file_table;
static lab2_fd next_fd = 0;

// Глобальный флаг для управления кэшем
static bool cache_enabled = false;


static HANDLE get_file_handle(int fd) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        return INVALID_HANDLE_VALUE;
    }
    return it->second.handle;
}


static LRUCache cache(2048 * 1024, get_file_handle); // 2 MB cache for example

// Вспомогательная функция для выделения выровненной памяти
void* aligned_alloc(size_t alignment, size_t size) {
    void* ptr = nullptr;
    size_t offset = alignment - 1 + sizeof(void*);
    if (void* p = malloc(size + offset)) {
        ptr = reinterpret_cast<void*>((reinterpret_cast<size_t>(p) + offset) & ~(alignment - 1));
        *(reinterpret_cast<void**>(ptr) - 1) = p;
    }
    return ptr;
}

void aligned_free(void* ptr) {
    if (ptr) {
        free(*(reinterpret_cast<void**>(ptr) - 1));
    }
}

void lab2_set_cache_enabled(bool enabled) {
    cache_enabled = enabled;
}


lab2_fd lab2_open(const char *path) {

    std::cout << "Открытие файла: " << path << std::endl;

    HANDLE file = CreateFileA(
        path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,  // Открыть файл, если он не существует, то создать
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка открытия файла: " << GetLastError() << std::endl;
        return -1;
    }

    lab2_fd fd = next_fd++;
    file_table[fd] = {file, path, 0};
    std::cout << "Файл открыт успешно." << std::endl;
    return fd;
}

int lab2_close(lab2_fd fd) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        return -1;
    }

    if (cache_enabled) {
        cache.flush(fd); // Сбросить все dirty блоки на диск
    }

    CloseHandle(it->second.handle);
    file_table.erase(it);
    return 0;
}


ssize_t lab2_read(lab2_fd fd, void* buf, size_t count) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        return -1;
    }

    FileDescriptor& desc = it->second;
    size_t total_read = 0;

    if (cache_enabled) {
        // Читаем данные из кэша
        total_read = cache.read(fd, desc.position, static_cast<char*>(buf), count);
        desc.position += total_read;
    }

    if (total_read < count) {
        // Читаем оставшиеся данные с диска
        DWORD bytesRead = 0;
        void* aligned_buf = aligned_alloc(SECTOR_SIZE, SECTOR_SIZE);

        while (total_read < count) {
            size_t block_offset = desc.position / SECTOR_SIZE * SECTOR_SIZE;
            SetFilePointer(desc.handle, static_cast<LONG>(block_offset), NULL, FILE_BEGIN);
            BOOL result = ReadFile(desc.handle, aligned_buf, SECTOR_SIZE, &bytesRead, NULL);

            if (!result || bytesRead == 0) {
                aligned_free(aligned_buf);
                break;
            }

            size_t to_copy = min(count - total_read, static_cast<size_t>(bytesRead));
            std::memcpy(static_cast<char*>(buf) + total_read, aligned_buf, to_copy);

            if (cache_enabled) {
                cache.write(fd, block_offset, static_cast<char*>(aligned_buf), bytesRead, false);
            }

            total_read += to_copy;
            desc.position += to_copy;
        }

        aligned_free(aligned_buf);
    }

    return total_read;
}

ssize_t lab2_write(lab2_fd fd, const void* buf, size_t count) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        return -1;
    }

    // std::cout << "lab2_write buffer (first 16 bytes): ";
    // for (size_t i = 0; i < std::min<size_t>(16, count); ++i) {
    //     std::cout << static_cast<const char*>(buf)[i];
    // }
    // std::cout << std::endl;


    FileDescriptor& desc = it->second;
    size_t total_written = 0;
    if (cache_enabled) {
        total_written = cache.write(fd, desc.position, static_cast<const char*>(buf), count, true);
        // Обновляем позицию в таблице файлов
        desc.position += total_written;
    } else {
        DWORD bytesWritten = 0;
        void* aligned_buf = aligned_alloc(SECTOR_SIZE, SECTOR_SIZE);

        while (total_written < count) {
            size_t block_offset = desc.position / SECTOR_SIZE * SECTOR_SIZE;
            size_t block_pos = desc.position % SECTOR_SIZE;
            size_t to_write = min(count - total_written, SECTOR_SIZE - block_pos);

            std::memcpy(aligned_buf, static_cast<const char*>(buf) + total_written, to_write);
            SetFilePointer(desc.handle, static_cast<LONG>(block_offset), NULL, FILE_BEGIN);
            BOOL result = WriteFile(desc.handle, aligned_buf, SECTOR_SIZE, &bytesWritten, NULL);

            if (!result || bytesWritten != SECTOR_SIZE) {
                aligned_free(aligned_buf);
                return -1;
            }

            total_written += to_write;
            desc.position += to_write;
        }
        aligned_free(aligned_buf);
    }

    return count;
}



off_t lab2_lseek(lab2_fd fd, off_t offset, int whence) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        return -1;
    }

    FileDescriptor& desc = it->second;

    switch (whence) {
        case SEEK_SET:
            desc.position = offset;
        break;
        case SEEK_CUR:
            desc.position += offset;
        break;
        case SEEK_END:
            LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(desc.handle, &fileSize)) {
            return -1;
        }
        desc.position = fileSize.QuadPart + offset;
        break;
        default:
            return -1;
    }

    return desc.position;
}


int lab2_fsync(lab2_fd fd) {
    if (!cache_enabled) {
        return 0;
    }

    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        std::cerr << "Неверный дескриптор файла" << std::endl;
        return -1;
    }

    cache.flush(fd);
    return 0;
}
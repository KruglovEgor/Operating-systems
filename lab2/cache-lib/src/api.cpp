#include "api.h"

#include <iostream>
#include <ostream>
#include <windows.h>
#include "lru_policy.h"

struct FileDescriptor {
    HANDLE handle;
    std::string path;
    size_t position;
};

static std::unordered_map<lab2_fd, FileDescriptor> file_table;
static lab2_fd next_fd = 0;

// Размер сектора для работы с `FILE_FLAG_NO_BUFFERING`
constexpr size_t SECTOR_SIZE = 512;

static LRUCache cache(1024 * 1024); // 1 MB cache for example

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
    CloseHandle(it->second.handle);
    file_table.erase(it);
    return 0;
}


ssize_t lab2_read(lab2_fd fd, void* buf, size_t count) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        std::cerr << "Неверный дескриптор файла" << std::endl;
        return -1;
    }

    size_t total_read = 0;

    while (total_read < count) {
        size_t to_read = min(count - total_read, SECTOR_SIZE);
        void* aligned_buf = aligned_alloc(SECTOR_SIZE, SECTOR_SIZE);

        DWORD bytesRead = 0;
        BOOL result = ReadFile(it->second.handle, aligned_buf, static_cast<DWORD>(SECTOR_SIZE), &bytesRead, NULL);

        if (!result || bytesRead == 0) {
            aligned_free(aligned_buf);
            break; // Конец файла или ошибка
        }

        memcpy(static_cast<char*>(buf) + total_read, aligned_buf, min(to_read, static_cast<size_t>(bytesRead)));
        aligned_free(aligned_buf);

        total_read += bytesRead;
        it->second.position += bytesRead;
    }

    return total_read;
}

ssize_t lab2_write(lab2_fd fd, const void* buf, size_t count) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        std::cerr << "Неверный дескриптор файла" << std::endl;
        return -1;
    }

    size_t total_written = 0;

    while (total_written < count) {
        size_t to_write = min(count - total_written, SECTOR_SIZE);
        void* aligned_buf = aligned_alloc(SECTOR_SIZE, SECTOR_SIZE);

        // Заполнить выровненный буфер данными
        memset(aligned_buf, 0, SECTOR_SIZE); // Заполняем нулями
        memcpy(aligned_buf, static_cast<const char*>(buf) + total_written, to_write);

        DWORD bytesWritten = 0;
        BOOL writeResult = WriteFile(it->second.handle, aligned_buf, static_cast<DWORD>(SECTOR_SIZE), &bytesWritten, NULL);
        aligned_free(aligned_buf);

        if (!writeResult) {
            std::cerr << "Ошибка записи в файл: " << GetLastError() << std::endl;
            return -1;
        }

        total_written += to_write;
        it->second.position += to_write;
    }

    return total_written;
}


off_t lab2_lseek(lab2_fd fd, off_t offset, int whence) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        return -1;
    }

    LARGE_INTEGER li;
    li.QuadPart = offset;
    DWORD moveMethod;

    switch (whence) {
        case SEEK_SET:
            moveMethod = FILE_BEGIN;
            break;
        case SEEK_CUR:
            moveMethod = FILE_CURRENT;
            break;
        case SEEK_END:
            moveMethod = FILE_END;
            break;
        default:
            return -1;
    }

    li.LowPart = SetFilePointer(it->second.handle, li.LowPart, &li.HighPart, moveMethod);
    if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        return -1;
    }

    it->second.position = li.QuadPart;

    return li.QuadPart;
}

int lab2_fsync(lab2_fd fd) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        return -1;
    }

    if (!FlushFileBuffers(it->second.handle)) {
        return -1;
    }
    return 0;
}
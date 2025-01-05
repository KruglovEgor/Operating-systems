#include "api.h"

#include <iostream>
#include <ostream>
#include <windows.h>
#include <stdexcept>
#include "lru_policy.h"

struct FileDescriptor {
    HANDLE handle;
    std::string path;
    size_t position;
};

static std::unordered_map<lab2_fd, FileDescriptor> file_table;
static lab2_fd next_fd = 0;

static LRUCache cache(1024 * 1024); // 1 MB cache for example

lab2_fd lab2_open(const char *path) {

    std::cout << "Открытие файла: " << path << std::endl;

    HANDLE file = CreateFileA(
        path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,  // Открыть файл, если он не существует, то создать
        0,
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

ssize_t lab2_read(lab2_fd fd, void *buf, size_t count) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        return -1;
    }

    size_t remaining = count;
    size_t offset = it->second.position;
    char *dst = static_cast<char *>(buf);

    while (remaining > 0) {
        size_t to_read = cache.read(fd, offset, dst, remaining);
        if (to_read > 0) {
            remaining -= to_read;
            offset += to_read;
            dst += to_read;
        } else {
            DWORD bytesRead = 0;
            if (!ReadFile(it->second.handle, dst, static_cast<DWORD>(remaining), &bytesRead, NULL)) {
                return -1;
            }
            cache.write(fd, offset, dst, bytesRead);
            remaining -= bytesRead;
            offset += bytesRead;
            dst += bytesRead;
        }
    }

    it->second.position += count;
    return count;
}

ssize_t lab2_write(lab2_fd fd, const void *buf, size_t count) {
    auto it = file_table.find(fd);
    if (it == file_table.end()) {
        std::cerr << "Неверный дескриптор файла" << std::endl;
        return -1;
    }

    const char *src = static_cast<const char *>(buf);
    size_t remaining = count;
    size_t offset = it->second.position;

    std::cout << "Запись в файл. Количество байт: " << count << std::endl;
    cache.write(fd, offset, src, count);

    DWORD bytesWritten = 0;
    BOOL writeResult = WriteFile(it->second.handle, src, static_cast<DWORD>(count), &bytesWritten, NULL);
    if (!writeResult) {
        DWORD error = GetLastError();
        std::cerr << "Ошибка записи в файл: " << error << std::endl;
    }

    if (bytesWritten != count) {
        std::cerr << "Количество записанных байт не совпадает с ожидаемым. Ожидалось: "
                  << count << ", записано: " << bytesWritten << std::endl;
        return -1;
    }


    std::cout << "Записано байт: " << bytesWritten << std::endl;

    it->second.position += bytesWritten;
    return bytesWritten;
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
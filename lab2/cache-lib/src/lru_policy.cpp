#include "lru_policy.h"
#include <cstring> // Для memcpy
#include <algorithm> // Для std::min
#include <iostream>
#include <string>
#include <windows.h>



void LRUCache::print_cache() {
    std::cout << "=== Cache List ===" << std::endl;
    for (const auto& entry : cache_list_) {
        std::cout << "FD: " << entry.fd
                  << ", Offset: " << entry.offset
                  << ", Dirty: " << (entry.dirty ? "true" : "false")
                  << ", Data (first 16 bytes): ";
        for (size_t i = 0; i < std::min<size_t>(16, entry.data.size()); ++i) {
            std::cout << entry.data[i];
        }
        std::cout << std::endl;
    }

    std::cout << "=== Cache Map ===" << std::endl;
    for (const auto& [key, it] : cache_map_) {
        std::cout << "Key (FD: " << key.first
                  << ", Offset: " << key.second
                  << "), Points to FD: " << it->fd
                  << ", Offset: " << it->offset << std::endl;
    }
}


// Чтение данных из кэша
size_t LRUCache::read(int fd, size_t offset, char* buf, size_t size) {
    size_t total_read = 0;

    while (total_read < size) {
        size_t block_offset = offset / SECTOR_SIZE * SECTOR_SIZE;
        auto it = cache_map_.find({fd, block_offset});

        if (it != cache_map_.end()) {
            CacheEntry& entry = *it->second;
            size_t block_pos = offset % SECTOR_SIZE;
            size_t to_copy = min(size - total_read, SECTOR_SIZE - block_pos);

            std::memcpy(buf + total_read, &entry.data[block_pos], to_copy);

            // Перемещаем блок в начало списка LRU
            cache_list_.splice(cache_list_.begin(), cache_list_, it->second);

            total_read += to_copy;
            offset += to_copy;
        } else {
            break; // Если нет данных в кэше, выходим
        }
    }
    // std::cout << "After cache read:" << std::endl;
    // print_cache();
    return total_read;
}


// Запись данных в кэш
size_t LRUCache::write(int fd, size_t offset, const char* buf, size_t size, bool dirty) {
    size_t total_written = 0;

    while (total_written < size) {
        size_t block_offset = offset / SECTOR_SIZE * SECTOR_SIZE;
        auto it = cache_map_.find({fd, block_offset});
        CacheEntry* entry;

        if (it == cache_map_.end()) {
            entry = new CacheEntry{
                fd,
                block_offset,
                std::vector<char>(SECTOR_SIZE, 0),
                dirty
            };
            used_ += SECTOR_SIZE;
        } else {
            entry = &(*it->second);
            entry->dirty = dirty;
        }

        size_t block_pos = offset % SECTOR_SIZE;
        size_t to_write = min(size - total_written, SECTOR_SIZE - block_pos);

        // std::cout << "Copying to cache: ";
        // for (size_t i = 0; i < to_write; ++i) {
        //     std::cout << buf[total_written + i];
        // }
        // std::cout << std::endl;

        std::memcpy(&entry->data[block_pos], buf + total_written, to_write);

        // std::cout << "Cache block after memcpy (first 16 bytes): ";
        // for (size_t i = 0; i < 16; ++i) {
        //     std::cout << entry->data[i];
        // }
        // std::cout << std::endl;

        if (it == cache_map_.end()) {
            cache_list_.push_front(*entry);
            cache_map_[{fd, block_offset}] = cache_list_.begin();
        }

        total_written += to_write;
        offset += to_write;
        while (used_ > capacity_) {
            evict();
        }
    }
    // std::cout << "After cache write:" << std::endl;
    // print_cache();
    return total_written;
}

// Сброс dirty блоков на диск
void LRUCache::flush(int fd) {
    for (auto it = cache_list_.begin(); it != cache_list_.end();) {
        if (it->fd == fd && it->dirty) {
            write_to_disk(*it);
            it->dirty = false;
        }
        ++it;
    }
}

// Вытеснение блоков
void LRUCache::evict() {
    if (cache_list_.empty()) return;

    auto last = cache_list_.end();
    --last;

    if (last->dirty) {
        write_to_disk(*last);
    }

    used_ -= last->data.size();
    cache_map_.erase({last->fd, last->offset});
    cache_list_.pop_back();
}

// Запись dirty блока на диск
void LRUCache::write_to_disk(const CacheEntry& entry) {
    HANDLE file = get_handle_callback_(entry.fd);
    if (file == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка: дескриптор файла недействителен для fd " << entry.fd << std::endl;
        return;
    }

    DWORD bytesWritten = 0;
    SetFilePointer(file, static_cast<LONG>(entry.offset), NULL, FILE_BEGIN);
    BOOL result = WriteFile(file, entry.data.data(), static_cast<DWORD>(entry.data.size()), &bytesWritten, NULL);

    if (!result || bytesWritten != entry.data.size()) {
        std::cerr << "Ошибка записи данных из кэша на диск." << std::endl;
    }
}

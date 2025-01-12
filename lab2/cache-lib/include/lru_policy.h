#ifndef CACHE_LIBRARY_LRU_POLICY_H
#define CACHE_LIBRARY_LRU_POLICY_H

#include <unordered_map>
#include <list>
#include <vector>
#include <cstddef>
#include <utility> // Для std::pair
#include <functional> // Для std::hash
#include <windows.h>

// Пользовательская хэш-функция для std::pair
namespace std {
    template <>
    struct hash<std::pair<int, size_t>> {
        size_t operator()(const std::pair<int, size_t>& p) const noexcept {
            return hash<int>()(p.first) ^ (hash<size_t>()(p.second) << 1);
        }
    };
}

#define CAPACITY (4*1024*1024)
#define BLOCK_SIZE (12*1024)

typedef HANDLE (*GetHandleCallback)(int fd);


class LRUCache {
public:
    LRUCache(size_t capacity, GetHandleCallback callback)
        : capacity_(capacity), used_(0), get_handle_callback_(callback) {}

    size_t read(int fd, size_t offset, char* buf, size_t size);
    size_t  write(int fd, size_t offset, const char* buf, size_t size, bool dirty);
    void flush(int fd);
    void evict();

    void print_cache(); // Новый метод для вывода содержимого кэша

private:

    struct CacheEntry {
        int fd;
        size_t offset;            // Смещение блока
        std::vector<char> data;   // Данные блока
        bool dirty = false;       // Флаг "грязности"
    };

    size_t capacity_;             // Максимальный размер кэша
    size_t used_;                 // Текущий размер используемого кэша

    std::list<CacheEntry> cache_list_;
    std::unordered_map<std::pair<int, size_t>, std::list<CacheEntry>::iterator> cache_map_;

    GetHandleCallback get_handle_callback_;

    void write_to_disk(const CacheEntry& entry); // Сброс записи на диск
};

#endif // CACHE_LIBRARY_LRU_POLICY_H

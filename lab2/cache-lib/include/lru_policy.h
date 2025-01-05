#ifndef CACHE_LIBRARY_LRU_POLICY_H
#define CACHE_LIBRARY_LRU_POLICY_H

#include <unordered_map>
#include <list>
#include <vector>
#include <cstddef>

class LRUCache {
public:
    LRUCache(size_t capacity) : capacity_(capacity), used_(0) {}

    size_t read(int fd, size_t offset, char *buf, size_t size);
    void write(int fd, size_t offset, const char *buf, size_t size);

private:
    struct CacheEntry {
        int fd;
        size_t offset;
        std::vector<char> data;
    };

    size_t capacity_;
    size_t used_;

    std::list<CacheEntry> cache_list_;
    std::unordered_map<int, std::list<CacheEntry>::iterator> cache_map_;

    void evict();
};

#endif // CACHE_LIBRARY_LRU_POLICY_H
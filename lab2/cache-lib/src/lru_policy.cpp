#include "lru_policy.h"

size_t LRUCache::read(int fd, size_t offset, char *buf, size_t size) {
    for (auto it = cache_list_.begin(); it != cache_list_.end(); ++it) {
        if (it->fd == fd && it->offset <= offset && offset < it->offset + it->data.size()) {
            size_t cache_offset = offset - it->offset;
            size_t to_copy = std::min(size, it->data.size() - cache_offset);
            std::memcpy(buf, &it->data[cache_offset], to_copy);
            cache_list_.splice(cache_list_.begin(), cache_list_, it);
            return to_copy;
        }
    }
    return 0;
}

void LRUCache::write(int fd, size_t offset, const char *buf, size_t size) {
    CacheEntry entry = {fd, offset, std::vector<char>(buf, buf + size)};
    cache_list_.push_front(entry);
    cache_map_[fd] = cache_list_.begin();
    used_ += size;

    while (used_ > capacity_) {
        evict();
    }
}

void LRUCache::evict() {
    if (cache_list_.empty()) return;

    auto last = cache_list_.end();
    --last;
    used_ -= last->data.size();
    cache_map_.erase(last->fd);
    cache_list_.pop_back();
}


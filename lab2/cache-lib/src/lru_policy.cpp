#include "lru_policy.h"
#include <cstring> // Для memcpy
#include <algorithm> // Для std::min

size_t LRUCache::read(int fd, size_t offset, char *buf, size_t size) {
    size_t total_read = 0;

    for (auto it = cache_list_.begin(); it != cache_list_.end(); ++it) {
        if (it->fd == fd && it->offset <= offset && offset < it->offset + it->data.size()) {
            size_t cache_offset = offset - it->offset;
            size_t to_copy = std::min(size, it->data.size() - cache_offset);
            std::memcpy(buf, &it->data[cache_offset], to_copy);
            cache_list_.splice(cache_list_.begin(), cache_list_, it); // Перемещаем в начало списка
            total_read += to_copy;
            offset += to_copy;
            buf += to_copy;
            size -= to_copy;

            if (size == 0) break; // Все данные прочитаны
        }
    }

    return total_read;
}

void LRUCache::write(int fd, size_t offset, const char *buf, size_t size) {
    // Удаление перекрывающихся данных из кэша
    for (auto it = cache_list_.begin(); it != cache_list_.end();) {
        if (it->fd == fd && offset < it->offset + it->data.size() && offset + size > it->offset) {
            // Удаляем существующую запись, если она перекрывается новой
            used_ -= it->data.size();
            cache_map_.erase({it->fd, it->offset});
            it = cache_list_.erase(it);
        } else {
            ++it;
        }
    }

    // Добавление новой записи
    CacheEntry entry = {fd, offset, std::vector<char>(buf, buf + size)};
    cache_list_.push_front(entry);
    cache_map_[{fd, offset}] = cache_list_.begin(); // Учитываем смещение
    used_ += size;

    // Удаление избыточных данных при переполнении
    while (used_ > capacity_) {
        evict();
    }
}

void LRUCache::evict() {
    if (cache_list_.empty()) return;

    auto last = cache_list_.end();
    --last;
    used_ -= last->data.size();
    cache_map_.erase({last->fd, last->offset}); // Учитываем смещение
    cache_list_.pop_back();
}

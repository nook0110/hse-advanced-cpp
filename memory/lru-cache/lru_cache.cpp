#include "lru_cache.h"
LruCache::LruCache(size_t max_size) : max_size_(max_size) {
}

void LruCache::Set(const std::string& key, const std::string& value) {
    if (map_.contains(key)) {
        map_[key].value = value;
        UpdateElement(key);
        return;
    }

    keys_.emplace_front(key);
    map_[key] = {keys_.begin(), value};
    UpdateCacheSize();
}

bool LruCache::Get(const std::string& key, std::string* const value) {
    if (!map_.contains(key)) {
        return false;
    }

    *value = map_[key].value;
    UpdateElement(key);
    return true;
}

void LruCache::UpdateCacheSize() {
    while (keys_.size() > max_size_) {
        map_.extract(keys_.back());
        keys_.pop_back();
    }
}

void LruCache::UpdateElement(const std::string& key) {
    keys_.emplace_front(std::move(*map_[key].it));
    keys_.erase(map_[key].it);
    map_[key].it = keys_.begin();
}

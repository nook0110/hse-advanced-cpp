#pragma once

#include <cstddef>
#include <string>
#include <list>
#include <unordered_map>

class LruCache {
public:
    LruCache(size_t max_size);

    void Set(const std::string& key, const std::string& value);

    bool Get(const std::string& key, std::string* value);

private:
    struct Node {
        std::list<std::string>::iterator it;
        std::string value;
    };

    void UpdateCacheSize();
    void UpdateElement(const std::string& position);

    size_t max_size_;
    std::list<std::string> keys_;
    std::unordered_map<std::string, Node> map_;
};

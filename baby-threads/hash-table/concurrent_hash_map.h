#pragma once

#include <algorithm>
#include <cstddef>
#include <list>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <vector>

template <class K, class V, class Hash = std::hash<K>>
class ConcurrentHashMap {
public:
    ConcurrentHashMap(const Hash& hasher = Hash()) : ConcurrentHashMap(kUndefinedSize, hasher) {
    }

    explicit ConcurrentHashMap(int expected_size, const Hash& hasher = Hash())
        : ConcurrentHashMap(expected_size, kDefaultConcurrencyLevel, hasher) {
    }

    ConcurrentHashMap(int expected_size, int expected_threads_count, const Hash& hasher = Hash())
        : hasher_(hasher) {
        (void)expected_threads_count;

        if (expected_size != kUndefinedSize) {
            table_.reserve(expected_size);
        }
    }

    bool Insert(const K& key, const V& value) {
        std::unique_lock table_lock(mutex_);
        auto& [mutex, data] = table_[hasher_(key)];
        table_lock.unlock();

        std::lock_guard chain_lock(mutex);

        if (std::ranges::find_if(data, [&key](const auto& node) { return node.key == key; }) !=
            data.end()) {
            return false;
        }
        data.emplace_back(key, value);
        return true;
    }

    bool Erase(const K& key) {
        std::unique_lock table_lock(mutex_);
        auto& [mutex, data] = table_[hasher_(key)];
        table_lock.unlock();

        std::lock_guard lock(mutex);

        auto pos = std::ranges::find_if(data, [&key](const auto& node) { return node.key == key; });
        if (pos == data.end()) {
            return false;
        }
        data.erase(pos);
        return true;
    }

    void Clear() {
        std::lock_guard lock(mutex_);

        std::vector<std::unique_lock<std::mutex>> chains;

        for (const auto& [_, chain] : table_) {
            chains.emplace_back(chain.mutex);
        }
        table_.clear();
    }

    std::pair<bool, V> Find(const K& key) const {
        std::unique_lock table_lock(mutex_);
        if (!table_.contains(hasher_(key))) {
            return std::make_pair(false, V());
        }

        auto& [mutex, data] = table_.at(hasher_(key));
        table_lock.unlock();

        std::lock_guard lock(mutex);

        auto it = std::ranges::find_if(data, [&key](const auto& node) { return node.key == key; });
        return it == data.end() ? std::make_pair(false, V()) : std::make_pair(true, it->value);
    }

    const V At(const K& key) const {
        std::unique_lock table_lock(mutex_);
        auto& [mutex, data] = table_.at(hasher_(key));
        table_lock.unlock();

        std::lock_guard lock(mutex);

        auto it = std::ranges::find_if(data, [&key](const auto& node) { return node.key == key; });
        return it->value;
    }

    size_t Size() const {
        std::lock_guard lock(mutex_);

        size_t ans = 0;

        for (const auto& [_, chain] : table_) {
            ans += chain.data.size();
        }

        return ans;
    }

    static const int kDefaultConcurrencyLevel;
    static const int kUndefinedSize;

private:
    Hash hasher_;

    struct Node {
        const K key;
        V value;
    };

    struct Chain {
        mutable std::mutex mutex;
        std::list<Node> data;
    };

    std::unordered_map<size_t, Chain> table_;
    mutable std::mutex mutex_;
};

template <class K, class V, class Hash>
const int ConcurrentHashMap<K, V, Hash>::kDefaultConcurrencyLevel = 8;

template <class K, class V, class Hash>
const int ConcurrentHashMap<K, V, Hash>::kUndefinedSize = -1;

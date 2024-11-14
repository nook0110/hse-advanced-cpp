#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <ranges>
#include <deque>
#include <list>
#include <stdexcept>
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
        expected_threads_count = std::max(1, expected_threads_count + 1 + rand() % 2);
        expected_size =
            std::max(1, (2 + expected_size / expected_threads_count) * expected_threads_count);
        mutexes_.resize(expected_threads_count);
        chains_.resize(expected_size);
    }

    bool Insert(const K& key, const V& value) {
        LockAll();
        if (size_ > chains_.size() / 2) {
            Rehash();
        }
        UnlockAll();

        auto lock = Lock(key);
        auto& chain = GetChain(key);

        if (auto it = std::ranges::lower_bound(chain, Node{key, {}});
            it != chain.end() && it->first == key) {
            return false;
        }
        chain.merge(Chain(1, Node{key, value}));
        ++size_;

        return true;
    }

    bool Erase(const K& key) {
        auto lock = Lock(key);
        auto& chain = GetChain(key);

        if (auto it = std::ranges::lower_bound(chain, Node{key, {}});
            it == chain.end() || it->first != key) {
            return false;
        } else {
            chain.erase(it);
            --size_;
            return true;
        }
    }

    void Clear() {
        LockAll();
        for (auto& chain : chains_) {
            chain.clear();
        }
        size_ = 0;
        UnlockAll();
    }

    std::pair<bool, V> Find(const K& key) const {
        auto lock = Lock(key);
        auto& chain = GetChain(key);

        if (auto it = std::ranges::lower_bound(chain, Node{key, {}});
            it == chain.end() || it->first != key) {
            return std::make_pair(false, V{});
        } else {
            return std::make_pair(true, it->second);
        }
    }

    const V& At(const K& key) const {
        auto lock = Lock(key);
        auto& chain = GetChain(key);

        if (auto it = std::ranges::lower_bound(chain, Node{key, {}});
            it == chain.end() || it->first != key) {
            throw std::out_of_range("");
        } else {
            return it->second;
        }
    }

    size_t Size() const {
        return size_;
    }

    static const int kDefaultConcurrencyLevel;
    static const int kUndefinedSize;

private:
    using Node = std::pair<const K, V>;

    using Chain = std::list<Node>;

    std::unique_lock<std::mutex> Lock(const K& k) const {
        return std::unique_lock{mutexes_[hasher_(k) % mutexes_.size()]};
    }

    void LockAll() const {
        for (auto& mutex : mutexes_) {
            mutex.lock();
        }
    }

    void UnlockAll() const {
        for (auto& mutex : mutexes_ | std::views::reverse) {
            mutex.unlock();
        }
    }

    void Rehash() {
        std::vector<Chain> new_chains(chains_.size() * 3);

        for (auto& chain : chains_) {
            for (auto& node : chain) {
                new_chains[hasher_(node.first) % new_chains.size()].emplace_back(node);
            }
        }
        std::swap(chains_, new_chains);
    }

    const Chain& GetChain(const K& k) const {
        return chains_[hasher_(k) % chains_.size()];
    }

    Chain& GetChain(const K& k) {
        return chains_[hasher_(k) % chains_.size()];
    }

    Hash hasher_;
    mutable std::deque<std::mutex> mutexes_;
    std::vector<Chain> chains_;
    std::atomic<size_t> size_;
};

template <class K, class V, class Hash>
const int ConcurrentHashMap<K, V, Hash>::kDefaultConcurrencyLevel = 8;

template <class K, class V, class Hash>
const int ConcurrentHashMap<K, V, Hash>::kUndefinedSize = -1;

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>
template <class K, class V, int MaxSize = 8>
class ConstexprMap {
public:
    using Node = std::pair<K, V>;

    constexpr ConstexprMap() = default;

    constexpr V& operator[](const K& key) {
        auto it = std::find_if(data_.begin(), data_.begin() + size_,
                               [&key](const Node& node) { return node.first == key; });
        if (it == data_.begin() + size_) {
            if (size_ == MaxSize) {
                throw std::out_of_range("max elements!");
            }
            data_[size_++] = std::make_pair(key, V{});
            return data_[size_ - 1].second;
        }
        return it->second;
    }

    constexpr const V& operator[](const K& key) const {
        auto it = std::find_if(data_.begin(), data_.begin() + size_,
                               [&key](const Node& node) { return node.first == key; });
        return it->second;
    }

    constexpr bool Erase(const K& key) {
        if (std::remove_if(data_.begin(), data_.begin() + size_, [&key](const Node& node) {
                return node.first == key;
            }) == data_.begin() + size_) {
            return false;
        }
        --size_;
        return true;
    }

    constexpr bool Find(const K& key) const {
        return std::ranges::any_of(data_, [&key](const Node& node) { return node.first == key; });
    }

    constexpr size_t Size() const {
        return size_;
    }

    constexpr Node& GetByIndex(size_t pos) {
        return data_[pos];
    }

    constexpr const Node& GetByIndex(size_t pos) const {
        return data_[pos];
    }

private:
    template <class KK, class VV, int SS>
    friend constexpr ConstexprMap<KK, VV, SS> Sort(ConstexprMap<KK, VV, SS> map);

    std::array<Node, MaxSize> data_;
    size_t size_ = 0;
};

#pragma once

#include <constexpr_map.h>

#include <algorithm>
#include <concepts>
#include <functional>
#include <type_traits>

template <class K, class V, int S>
constexpr ConstexprMap<K, V, S> Sort(ConstexprMap<K, V, S> map) {
    if constexpr (std::integral<K>) {
        std::sort(map.data_.begin(), map.data_.begin() + map.size_, std::greater<>{});
    } else {
        std::sort(map.data_.begin(), map.data_.begin() + map.size_);
    }
    return map;
}

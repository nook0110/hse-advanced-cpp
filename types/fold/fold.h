#pragma once

#include <iterator>
#include <utility>
#include <vector>
struct Sum {
    template <typename T, typename U>
    auto operator()(T&& t, U&& u) const {
        return std::forward<T>(t) + std::forward<U>(u);
    }
};

struct Prod {
    template <typename T, typename U>
    auto operator()(T&& t, U&& u) const {
        return std::forward<T>(t) * std::forward<U>(u);
    }
};

struct Concat {
    template <typename T>
    auto operator()(std::vector<T> t, std::vector<T> u) const {
        for (auto& val : u) {
            t.emplace_back(std::move(val));
        }
        return t;
    }
};

template <class Iterator, class T, class BinaryOp>
T Fold(Iterator first, Iterator last, T init, BinaryOp func) {
    for (; first != last; ++first) {
        init = func(std::move(init), *first);
    }

    return init;
}

class Length {
public:
    Length(int* value) : value_(value) {
    }
    template <typename T>
    auto operator()(const T& t, const T&) const {
        ++(*value_);
        return t;
    }

private:
    int* value_;
};

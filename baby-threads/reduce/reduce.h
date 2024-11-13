#pragma once
#include <cstddef>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iterator>
#include <optional>
#include <thread>
#include <vector>

struct ThreadContext {
    size_t step;
    size_t idx;
};

template <class RandomAccessIterator, class T, class Func>
void ReduceThreaded(ThreadContext ctx, RandomAccessIterator first, RandomAccessIterator last,
                    Func func, std::deque<std::optional<T>>& ans) {
    const auto [step, idx] = ctx;
    first += idx;
    if (first >= last) {
        return;
    }
    auto cur_value = *first;

    first += step;
    while (first < last) {
        cur_value = func(cur_value, *first);
        first += step;
    }

    ans[idx] = cur_value;
}

template <class RandomAccessIterator, class T, class Func>
T Reduce(RandomAccessIterator first, RandomAccessIterator last, const T& initial_value, Func func) {
    std::vector<std::thread> threads;
    const auto amount_of_threads = std::thread::hardware_concurrency();
    std::deque<std::optional<T>> answers(amount_of_threads);

    for (size_t i = 0; i < amount_of_threads; ++i) {
        threads.emplace_back(ReduceThreaded<RandomAccessIterator, T, Func>,
                             ThreadContext{amount_of_threads, i}, first, last, func,
                             std::ref(answers));
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto cur_value = initial_value;

    for (const auto& val : answers) {
        if (val) {
            cur_value = func(cur_value, *val);
        }
    }

    return cur_value;
}

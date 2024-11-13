#pragma once
#include <cstddef>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <optional>
#include <thread>
#include <vector>

struct ThreadContext {
    size_t step;
    size_t idx;
};

template <class T>
struct Wrapper {
    std::atomic<T> value;
};

template <class RandomAccessIterator, class T, class Func>
void ReduceThreaded(ThreadContext ctx, RandomAccessIterator first, RandomAccessIterator last,
                    Func func, Wrapper<T>& ans) {
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

    ans.value = cur_value;
}

template <class RandomAccessIterator, class T, class Func>
T Reduce(RandomAccessIterator first, RandomAccessIterator last, const T& initial_value, Func func) {
    if (first == last) {
        return initial_value;
    }
    std::vector<std::thread> threads;
    const auto amount_of_threads =
        std::min<size_t>(std::thread::hardware_concurrency(), std::distance(first, last));
    std::vector<Wrapper<T>> answers(amount_of_threads);

    for (size_t i = 0; i < amount_of_threads; ++i) {
        threads.emplace_back(ReduceThreaded<RandomAccessIterator, T, Func>,
                             ThreadContext{amount_of_threads, i}, first, last, func,
                             std::ref(answers[i]));
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto cur_value = initial_value;

    for (const auto& val : answers) {
        cur_value = func(cur_value, val.value);
    }

    return cur_value;
}

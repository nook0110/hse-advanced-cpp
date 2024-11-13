#pragma once
#include <cstddef>
#include <cmath>
#include <iterator>
#include <mutex>
#include <thread>
#include <vector>

struct ThreadContext {
    size_t step;
    size_t idx;
};

template <class RandomAccessIterator, class T, class Func>
void ReduceThreaded(ThreadContext ctx, RandomAccessIterator first, RandomAccessIterator last,
                    Func func, std::vector<T>& ans, std::mutex& mut) {
    const auto [step, idx] = ctx;
    first += idx;
    if (first >= last) {
        return;
    }
    T cur_value = *first;
    first += step;
    while (first < last) {
        auto val = *first;
        cur_value = func(cur_value, val);
        first += step;
    }

    std::scoped_lock mutex(mut);
    ans[idx] = cur_value;
}

template <class RandomAccessIterator, class T, class Func>
T Reduce(RandomAccessIterator first, RandomAccessIterator last, const T& initial_value, Func func) {
    if (first == last) {
        return initial_value;
    }
    std::vector<std::thread> threads;
    const auto amount_of_threads =
        std::min<size_t>(std::thread::hardware_concurrency(), std::distance(first, last));
    std::vector<T> answers(amount_of_threads);

    std::mutex mut;

    for (size_t i = 0; i < amount_of_threads; ++i) {
        threads.emplace_back(ReduceThreaded<RandomAccessIterator, T, Func>,
                             ThreadContext{amount_of_threads, i}, first, last, func,
                             std::ref(answers), std::ref(mut));
    }
    for (auto& thread : threads) {
        thread.join();
    }

    auto cur_value = initial_value;

    for (const auto& val : answers) {
        cur_value = func(cur_value, val);
    }

    return cur_value;
}

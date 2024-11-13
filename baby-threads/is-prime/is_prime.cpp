#include "is_prime.h"
#include <atomic>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <thread>
#include <vector>

void IsPrimeThreaded(uint64_t x, uint64_t bound, uint64_t step, uint64_t idx,
                     std::atomic_bool& is_prime) {
    for (uint64_t i = idx + 2; i <= bound && is_prime; i += step) {
        if (i >= x) {
            break;
        }
        if (x % i == 0) {
            is_prime = false;
            return;
        }
    }
}

bool IsPrime(uint64_t x) {
    if (x <= 1) {
        return false;
    }
    uint64_t root = sqrt(x);
    auto bound = std::min(root + 6, x);
    std::vector<std::thread> threads;

    std::atomic_bool is_prime = true;

    const auto amount_of_threads = std::thread::hardware_concurrency();

    for (uint64_t i = 0; i < amount_of_threads; ++i) {
        threads.emplace_back(IsPrimeThreaded, x, bound, amount_of_threads, i, std::ref(is_prime));
    }
    for (auto& thread : threads) {
        thread.join();
    }
    return is_prime;
}

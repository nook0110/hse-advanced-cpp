#pragma once

#include <atomic>
#include <cstddef>

struct RWSpinLock {
    void LockRead() {
        std::size_t amount = lock_.load();
        while (amount % 2 != 0 || !lock_.compare_exchange_strong(amount, amount + 2)) {
            amount = lock_.load();
        }
    }

    void UnlockRead() {
        lock_.fetch_sub(2);
    }

    void LockWrite() {
        std::size_t flag = 0;
        while (!lock_.compare_exchange_strong(flag, 1)) {
            flag = 0;
        }
    }

    void UnlockWrite() {
        lock_.store(0);
    }

private:
    std::atomic<std::size_t> lock_;
};

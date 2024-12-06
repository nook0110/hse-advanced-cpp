#pragma once

#include <atomic>
#include <cstddef>

struct RWSpinLock {
    void LockRead() {
        while (lock_.load() & 1) {
        }
        lock_.fetch_add(2);
    }

    void UnlockRead() {
        lock_.fetch_sub(2);
    }

    void LockWrite() {
        while (lock_.load()) {
        }
        lock_.store(1);
    }

    void UnlockWrite() {
        lock_.store(0);
    }

private:
    std::atomic<std::size_t> lock_;
};

#pragma once

#include <atomic>
#include <cstdint>

// Atomically do the following:
//    if (*(uint64_t*)addr == expected_value) {
//        sleep_on_address(addr)
//    }
void FutexWait(void *addr, uint64_t expected_value);

// Wakeup 1 thread sleeping on the given address
void FutexWakeOne(void *addr);

// Wakeup all threads sleeping on the given address
void FutexWakeAll(void *addr);

class Mutex {
public:
    void Lock() {
        int c = 0;
        val_.compare_exchange_strong(c, 1);
        if (c != 0) {
            if (c != 2) {
                c = val_.exchange(2);
            }
            while (c != 0) {
                FutexWait(&val_, 2);
                c = val_.exchange(2);
            }
        }
    }

    void Unlock() {
        if (val_.fetch_sub(1) != 1) {
            val_ = 0;
            FutexWakeOne(&val_);
        }
    }

private:
    std::atomic<int> val_;
};

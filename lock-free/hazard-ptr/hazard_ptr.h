#pragma once

#include <atomic>
#include <functional>
#include <optional>
#include <shared_mutex>
#include <mutex>
#include <memory>

struct ThreadState {
    std::atomic<void*>* ptr;
};

extern thread_local std::atomic<void*> hazard_ptr;
extern std::mutex threads_lock;

template <class T>
T* Acquire(std::atomic<T*>* ptr) {
    auto value = ptr->load();  // (2)

    do {
        hazard_ptr.store(value);

        auto new_value = ptr->load();  // (3)
        if (new_value == value) {      // (1)
            return value;
        }

        value = new_value;
    } while (true);
}

inline void Release() {
    hazard_ptr.store(nullptr);
}

struct RetiredPtr {
    void* value = nullptr;
    std::function<void()> deleter;
    RetiredPtr* next = nullptr;
};
extern std::atomic<RetiredPtr*> free_list;
extern std::atomic<int> approximate_free_list_size;

void ScanFreeList();

inline void Push(void* value, std::function<void()> deleter) {
    RetiredPtr* expected_head = nullptr;
    auto new_head = new RetiredPtr{value, deleter, expected_head};

    while (!free_list.compare_exchange_weak(expected_head, new_head)) {
        new_head->next = expected_head;
    }
}

template <class T, class Deleter = std::default_delete<T>>
void Retire(T* value, Deleter deleter = {}) {
    Push(value, [value, deleter]() { deleter(value); });
    approximate_free_list_size++;

    if (approximate_free_list_size > 100) {
        ScanFreeList();
    }
}

inline void RegisterThread() {
}

inline void UnregisterThread() {
}

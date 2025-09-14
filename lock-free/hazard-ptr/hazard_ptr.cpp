#include "hazard_ptr.h"
#include <algorithm>
#include <unordered_set>
#include <vector>
thread_local std::atomic<void*> hazard_ptr{nullptr};

std::mutex threads_lock;
std::unordered_set<ThreadState*> threads;

std::atomic<RetiredPtr*> free_list;

std::atomic<int> approximate_free_list_size = 0;

std::mutex scan_lock;

void ScanFreeList() {
    approximate_free_list_size.store(0);

    // (1) С помощью мьютекса убеждаемся, что не больше одного потока занимается сканированием.
    // В реальном коде не забудьте использовать guard.
    if (!scan_lock.try_lock()) {
        return;
    }

    // (2) Забираем все указатели из free_list
    RetiredPtr* retired = free_list.exchange(nullptr);

    std::vector<void*> hazard;
    {
        std::unique_lock guard(threads_lock);
        for (const auto& thread : threads) {
            if (auto ptr = thread->ptr->load(); ptr) {
                hazard.push_back(ptr);
            }
        }
    }

    RetiredPtr* cur = retired;
    while (cur) {
        if (std::ranges::count(hazard, cur->value)) {
            cur->deleter();
            cur->value = nullptr;
        } else {
            Push(cur->value, cur->deleter);
        }
        cur = cur->next;
    }

    cur = retired;
    while (cur) {
        auto to_delete = cur;
        cur = cur->next;
        delete to_delete;
    }
}
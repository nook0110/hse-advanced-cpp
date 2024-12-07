#pragma once

#include <atomic>
#include <cstddef>
#include <queue>
#include <vector>

template <class T>
class MPMCBoundedQueue {
public:
    explicit MPMCBoundedQueue(int size) : mask_(size - 1), nodes_(size) {
    }

    bool Enqueue(const T& value) {
        auto pos = end_.load();
        auto idx = pos & mask_;
        Node* node;

        while (true) {
            node = &nodes_[idx];

            size_t age = node->age.load();

            int diff = static_cast<int>(age) - static_cast<int>(pos);

            if (diff == 0 && end_.compare_exchange_weak(pos, pos + 1)) {
                break;
            } else if (diff < 0) {
                return false;
            } else {
                pos = end_.load();
            }
        }

        node->value = value;

        node->age.store(pos + mask_ + 1);

        return true;
    }

    bool Dequeue(T& data) {
        auto pos = begin_.load();
        auto idx = pos & mask_;
        Node* node;

        while (true) {
            node = &nodes_[idx];

            size_t age = node->age.load();

            int diff = static_cast<int>(age) - static_cast<int>(pos) - 1;

            if (diff == 0 && begin_.compare_exchange_weak(pos, pos + 1)) {
                break;
            } else if (diff < 0) {
                return false;
            } else {
                pos = begin_.load();
            }
        }

        data = node->value;

        node->age.store(pos + mask_ + 1);

        return true;
    }

private:
    struct Node {
        T value;
        std::atomic_size_t age;
    };

    std::atomic_size_t begin_ = 0;
    std::atomic_size_t end_ = 1;

    const size_t mask_;

    std::vector<Node> nodes_;
};

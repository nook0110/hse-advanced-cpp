#pragma once

#include <atomic>
#include <optional>
#include <stdexcept>
#include <utility>

template <class T>
class MPSCStack {
public:
    struct Node {
        Node* next = {};
        T value;
    };

    // Push adds one element to stack top.
    //
    // Safe to call from multiple threads.
    void Push(const T& value) {
        Node* expected_head = nullptr;
        auto new_head = new Node{expected_head, value};

        while (!head_.compare_exchange_weak(expected_head, new_head)) {
            new_head->next = expected_head;
        }
    }

    // Pop removes top element from the stack.
    //
    // Not safe to call concurrently.
    std::optional<T> Pop() {
        Node* expected_head = head_.load();

        while (expected_head && !head_.compare_exchange_weak(expected_head, expected_head->next)) {
        }

        if (!expected_head) {
            return {};
        }

        T value = std::move(expected_head->value);
        delete expected_head;
        return value;
    }

    // DequeuedAll Pop's all elements from the stack and calls cb() for each.
    //
    // Not safe to call concurrently with Pop()
    template <class TFn>
    void DequeueAll(const TFn& cb) {
        while (true) {
            auto val = Pop();
            if (!val) {
                break;
            }
            cb(*val);
        }
    }

    ~MPSCStack() {
        while (Pop()) {
        }
    }

    std::atomic<Node*> head_;
};

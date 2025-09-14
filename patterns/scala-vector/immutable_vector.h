#pragma once

#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>
template <class T>
class TrieNode {
public:
    std::shared_ptr<TrieNode> Set(const size_t index, T value) {
        std::shared_ptr<TrieNode> answer = std::make_shared<TrieNode>(*this);
        if (index == 0) {
            answer->data_ = std::move(value);
        } else {
            if (!answer->nodes_[index % 32]) {
                answer->nodes_[index % 32] = std::make_shared<TrieNode>();
            }
            answer->nodes_[index % 32] =
                answer->nodes_[index % 32]->Set(index / 32, std::move(value));
        }

        return answer;
    }

    std::shared_ptr<TrieNode> Pop(const size_t index) {
        std::shared_ptr<TrieNode> answer = std::make_shared<TrieNode>(*this);

        std::shared_ptr<TrieNode> new_node = nullptr;

        if (index / 32 != 0) {
            new_node = nodes_[index % 32]->Pop(index / 32);
        }
        answer->nodes_[index % 32] = new_node;

        return answer;
    }

    T& Get(size_t index) {
        if (index == 0) {
            return data_;
        }
        return nodes_[index % 32]->Get(index / 32);
    }

private:
    T data_;
    std::array<std::shared_ptr<TrieNode>, 32> nodes_;
};

template <class T>
class ImmutableVector {
public:
    ImmutableVector() = default;

    explicit ImmutableVector(size_t count, const T& value = T()) : size_(count) {
        for (size_t i = 0; i < count; ++i) {
            root_ = root_->Set(i, value);
        }
    }

    template <typename Iterator>
    ImmutableVector(Iterator first, Iterator last) : size_(std::distance(first, last)) {
        size_t i = 0;
        for (auto it = first; it != last; ++it) {
            root_ = root_->Set(i, *it);
            ++i;
        }
    }

    ImmutableVector(std::initializer_list<T> l) : size_(l.size()) {
        for (size_t i = 0; i < l.size(); ++i) {
            root_ = root_->Set(i, *(l.begin() + i));
        }
    }

    ImmutableVector Set(size_t index, const T& value) {
        ImmutableVector answer;
        answer.root_ = root_->Set(index, value);
        answer.size_ = size_;
        return answer;
    }
    const T& Get(size_t index) const {
        return root_->Get(index);
    }

    ImmutableVector PushBack(const T& value) {
        ImmutableVector answer;
        answer.root_ = root_->Set(size_, value);
        answer.size_ = size_ + 1;
        return answer;
    }

    ImmutableVector PopBack() {
        ImmutableVector answer;
        answer.root_ = root_->Pop(size_ - 1);
        answer.size_ = size_ - 1;
        return answer;
    }

    size_t Size() const {
        return size_;
    }

private:
    std::shared_ptr<TrieNode<T>> root_ = std::make_shared<TrieNode<T>>();
    size_t size_ = 0;
};

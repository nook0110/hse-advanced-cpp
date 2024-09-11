#pragma once

#include <cassert>
#include <iterator>

class ListHook {
public:
    ListHook() : next_(this), prev_(this){};

    void Swap(ListHook& other) {
        if (!IsLinked()) {
            LinkBefore(&other);
            other.Unlink();
            return;
        }
        const auto next = next_;
        LinkBefore(&other);
        other.Unlink();
        other.LinkBefore(next);
    }

    ListHook(const ListHook&) = delete;

    bool IsLinked() const {
        return next_ != this && prev_ != this;
    }

    void Unlink() {
        if (!IsLinked()) {
            return;
        }

        next_->prev_ = prev_;
        prev_->next_ = next_;

        next_ = this;
        prev_ = this;
    }

    // Must unlink element from list
    ~ListHook() {
        Unlink();
    }

private:
    template <class T>
    friend class List;

    // that helper function might be useful
    void LinkBefore(ListHook* other) {
        if (!IsLinked()) {
            prev_ = other->prev_;
            other->prev_->next_ = this;
        } else {
            assert(!other->IsLinked());  // at least 1 should be unlinked

            other->next_ = this->next_;
            next_->prev_ = other;
        }

        next_ = other;
        other->prev_ = this;
    }

    ListHook* next_;
    ListHook* prev_;
};

template <typename T>
class List {
public:
    class Iterator {
        using IteratorTag = std::bidirectional_iterator_tag;

    public:
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef T* pointer;
        typedef T& reference;
        typedef IteratorTag iterator_category;

        Iterator(pointer ptr) : ptr_(ptr) {
        }

        Iterator& operator++() {
            ptr_ = static_cast<pointer>(ptr_->next_);
            return *this;
        }
        Iterator operator++(int) {
            auto ans = *this;
            ++(*this);
            return ans;
        }

        T& operator*() const {
            return *ptr_;
        }

        T* operator->() const {
            return ptr_;
        }

        bool operator==(const Iterator& rhs) const = default;
        bool operator!=(const Iterator& rhs) const = default;

    private:
        pointer ptr_;
    };

    List() = default;
    List(const List&) = delete;
    List(List&& other) {
        dummy_.Swap(other.dummy_);
    }

    // must unlink all elements from list
    ~List() {
        while (!IsEmpty()) {
            Begin()->Unlink();
        }
    }

    List& operator=(const List&) = delete;
    List& operator=(List&& other) {
        dummy_.Swap(other.dummy_);
        return *this;
    };

    bool IsEmpty() const {
        return !dummy_.IsLinked();
    }
    // that method is allowed to be O(n)
    size_t Size() const {
        size_t size = 0;
        for (auto it =
                 Iterator(static_cast<Iterator::pointer>(const_cast<ListHook*>(dummy_.next_))); // NOLINT
             it !=
             Iterator(static_cast<Iterator::pointer>(const_cast<ListHook*>(&dummy_)));  // NOLINT
             ++it) {
            ++size;
        }
        return size;
    }

    // note that IntrusiveList doesn't own elements,
    // and never copies or moves T
    void PushBack(T* elem) {
        elem->LinkBefore(&dummy_);
    }
    void PushFront(T* elem) {
        dummy_.LinkBefore(elem);
    }

    T& Front() {
        return static_cast<T&>(*dummy_.next_);
    }
    const T& Front() const {
        return static_cast<const T&>(*dummy_.next_);
    }

    T& Back() {
        return static_cast<T&>(*dummy_.prev_);
    }
    const T& Back() const {
        return static_cast<const T&>(*dummy_.prev_);
    }

    void PopBack() {
        dummy_.prev_->Unlink();
    }
    void PopFront() {
        dummy_.next_->Unlink();
    }

    Iterator Begin() {
        return Iterator(static_cast<Iterator::pointer>(dummy_.next_));
    }
    Iterator End() {
        return Iterator(static_cast<Iterator::pointer>(&dummy_));
    }

    // complexity of this function must be O(1)
    Iterator IteratorTo(T* element) {
        return Iterator(element);
    }

private:
    ListHook dummy_;
};

template <typename T>
typename List<T>::Iterator begin(List<T>& list) {  // NOLINT
    return list.Begin();
}

template <typename T>
typename List<T>::Iterator end(List<T>& list) {  // NOLINT
    return list.End();
}

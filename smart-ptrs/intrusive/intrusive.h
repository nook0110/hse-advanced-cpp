#pragma once

#include <cstddef>  // for std::nullptr_t
#include <memory>
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        return ++count_;
    }
    size_t DecRef() {
        return --count_;
    }
    size_t RefCount() const {
        return count_;
    }

    SimpleCounter& operator=(const SimpleCounter&) {
        return *this;
    }
    SimpleCounter& operator=(SimpleCounter&&) {
        return *this;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        counter_.DecRef();
        if (RefCount() == 0) {
            Deleter{}.Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() = default;
    IntrusivePtr(std::nullptr_t) {};
    IntrusivePtr(T* ptr) {
        Reset(ptr);
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        Reset(static_cast<T*>(other.object_));
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        Reset(other.object_);
        other.Reset();
    }

    IntrusivePtr(const IntrusivePtr& other) {
        Reset(static_cast<T*>(other.object_));
    }

    IntrusivePtr(IntrusivePtr&& other) {
        Reset(other.object_);
        other.Reset();
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (this == std::addressof(other)) {
            return *this;
        }
        Reset(other.object_);
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (this == std::addressof(other)) {
            return *this;
        }
        Reset(other.object_);
        other.Reset();
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        Reset();
    }

    // Modifiers
    void Reset() {
        if (object_) {
            object_->DecRef();
        }
        object_ = nullptr;
    }
    void Reset(T* ptr) {
        Reset();
        object_ = ptr;
        if (object_) {
            object_->IncRef();
        }
    }
    void Swap(IntrusivePtr& other) {
        std::swap(object_, other.object_);
    }

    // Observers
    T* Get() const {
        return object_;
    }
    T& operator*() const {
        return *object_;
    }
    T* operator->() const {
        return Get();
    }
    size_t UseCount() const {
        if (!object_) {
            return 0;
        }
        return object_->RefCount();
    }
    explicit operator bool() const {
        return UseCount();
    }

private:
    T* object_ = nullptr;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}

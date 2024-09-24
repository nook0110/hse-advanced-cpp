#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cassert>
#include <concepts>
#include <cstddef>  // std::nullptr_t
#include <cstdio>
#include <memory>
#include <type_traits>
#include <utility>

class ControlBlockBase {
public:
    virtual void DestroyObject() = 0;
    virtual ~ControlBlockBase() = default;

    void Subscribe() {
        ++count_;
    }
    void Unsubscribe() {
        --count_;
        if (!count_) {
            DestroyObject();
            delete this;
        }
    }

    size_t GetCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

template <typename T>
class ControlBlock final : public ControlBlockBase {
public:
    explicit ControlBlock(T* object) : object_(object) {
    }

    void DestroyObject() override {
        delete object_;
    }

private:
    T* object_;
};

template <typename T>
class MakeSharedControlBlock final : public ControlBlockBase {
public:
    template <typename... Args>
    explicit MakeSharedControlBlock(std::in_place_t, Args&&... object)
        : object_(std::forward<Args>(object)...) {
    }

    void DestroyObject() override {
        /* Do nothing */
    }

    T* Get() {
        return std::addressof(object_);
    }

private:
    T object_;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() = default;
    SharedPtr(std::nullptr_t) : SharedPtr() {};
    template <typename U>
    explicit SharedPtr(U* ptr) : object_(ptr) {
        AssignControlBlock(new ControlBlock<U>(ptr));
    }
    explicit SharedPtr(MakeSharedControlBlock<T>* block) : object_(block->Get()) {
        AssignControlBlock(block);
    }

    template <typename U>
    SharedPtr(SharedPtr<U>& other) : object_(other.object_) {
        AssignControlBlock(other.control_block_);
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) : object_(other.object_) {
        AssignControlBlock(other.control_block_);
        other.Reset();
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : object_(ptr) {
        AssignControlBlock(other.control_block_);
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        Swap(other);
        other.Reset();
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        object_ = other.object_;
        AssignControlBlock(other.control_block_);
        other.Reset();
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        object_ = other.object_;
        AssignControlBlock(other.control_block_);
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        object_ = other.object_;
        AssignControlBlock(other.control_block_);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        object_ = nullptr;
        AssignControlBlock();
    }

    template <typename U>
    void Reset(U* ptr) {
        Reset();
        object_ = ptr;
        AssignControlBlock(new ControlBlock<U>(ptr));
    }
    void Swap(SharedPtr& other) {
        std::swap(object_, other.object_);
        std::swap(control_block_, other.control_block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return object_;
    }
    T& operator*() const {
        return *(Get());
    }
    T* operator->() const {
        return Get();
    }
    size_t UseCount() const {
        if (!control_block_) {
            return 0;
        }
        return control_block_->GetCount();
    }
    explicit operator bool() const {
        if (!control_block_) {
            return false;
        }
        return control_block_->GetCount();
    }

private:
    template <typename>
    friend class SharedPtr;

    void AssignControlBlock(ControlBlockBase* block = nullptr) {
        if (control_block_) {
            control_block_->Unsubscribe();
        }
        control_block_ = nullptr;
        control_block_ = block;
        if (control_block_) {
            control_block_->Subscribe();
        }
    }

    ControlBlockBase* control_block_ = nullptr;
    T* object_ = nullptr;
};

template <typename T1, typename T2>
inline bool operator==(const SharedPtr<T1>& left, const SharedPtr<T2>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new MakeSharedControlBlock<T>(std::in_place, std::forward<Args>(args)...));
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};

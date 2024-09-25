#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cassert>
#include <cstdio>
#include <type_traits>
#include <optional>
#include <utility>
#include <cstddef>  // std::nullptr_t

class ControlBlockBase {
public:
    virtual void DestroyObject() = 0;
    virtual ~ControlBlockBase() = default;

    void Subscribe(SharedTag) {
        ++shared_count_;
    }
    void Subscribe(WeakTag) {
        ++weak_count_;
    }
    void Unsubscribe(SharedTag) {
        --shared_count_;
        if (!shared_count_) {
            DestroyObject();
            if (!weak_count_) {
                delete this;
            }
        }
    }
    void Unsubscribe(WeakTag) {
        --weak_count_;
        if (!weak_count_ && !shared_count_) {
            delete this;
        }
    }

    size_t GetCount() const {
        return shared_count_;
    }

private:
    size_t shared_count_ = 0;
    size_t weak_count_ = 0;
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
        : object_(std::in_place, std::forward<Args>(object)...) {
    }

    void DestroyObject() override {
        object_ = std::nullopt;
    }

    T* Get() {
        return std::addressof(*object_);
    }

private:
    std::optional<T> object_;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() = default;
    SharedPtr(std::nullptr_t){};
    template <typename U>
        requires(!std::is_base_of_v<EnableSharedFromThisBase, U>)
    explicit SharedPtr(U* ptr) : object_(ptr) {
        AssignControlBlock(new ControlBlock<U>(ptr));
    }
    template <typename U>
        requires(std::is_base_of_v<EnableSharedFromThisBase, U>)
    explicit SharedPtr(U* ptr) : object_(ptr) {
        AssignControlBlock(ptr->GetControlBlock());
    }
    explicit SharedPtr(MakeSharedControlBlock<T>* block) : object_(block->Get()) {
        AssignControlBlock(block);
    }

    SharedPtr(const SharedPtr& other) : object_(other.object_) {
        AssignControlBlock(other.control_block_);
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : object_(other.object_) {
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
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr{};
        }
        object_ = other.object_;
        AssignControlBlock(other.control_block_);
    }

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
    template <typename>
    friend class WeakPtr;
    template <typename>
    friend class EnableSharedFromThis;

    void AssignControlBlock(ControlBlockBase* block = nullptr) {
        if (control_block_) {
            control_block_->Unsubscribe(SharedTag{});
        }
        control_block_ = nullptr;
        control_block_ = block;
        if (control_block_) {
            control_block_->Subscribe(SharedTag{});
        }
    }

    SharedPtr(ControlBlockBase* block, T* object) : object_(object) {
        AssignControlBlock(block);
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
    requires(!std::is_base_of_v<EnableSharedFromThisBase, T>)
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new MakeSharedControlBlock<T>(std::in_place, std::forward<Args>(args)...));
}

template <typename T, typename... Args>
    requires(std::is_base_of_v<EnableSharedFromThisBase, T>)
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

// Look for usage examples in tests

template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
public:
    EnableSharedFromThis() : control_block_(new ControlBlock<EnableSharedFromThis<T>>(this)) {
    }

    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>{
            GetControlBlock(),
            dynamic_cast<T*>(this),
        };
    }

    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>{
            GetControlBlock(),
            dynamic_cast<const T*>(this),
        };
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return WeakPtr<T>{
            GetControlBlock(),
            dynamic_cast<T*>(this),
        };
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<const T>{
            GetControlBlock(),
            dynamic_cast<const T*>(this),
        };
    }
    virtual ~EnableSharedFromThis() = default;

private:
    template <typename>
    friend class SharedPtr;

    ControlBlockBase* GetControlBlock() const {
        return control_block_;
    }

    ControlBlock<EnableSharedFromThis<T>>* control_block_ = nullptr;
};

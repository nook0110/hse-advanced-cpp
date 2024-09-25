#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() = default;

    WeakPtr(const WeakPtr& other) : object_(other.object_) {
        AssignControlBlock(other.control_block_);
    }
    WeakPtr(WeakPtr&& other) noexcept : WeakPtr(other) {
        other.Reset();
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : object_(other.object_) {
        AssignControlBlock(other.control_block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        object_ = other.object_;
        AssignControlBlock(other.control_block_);
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) noexcept {
        (*this) = other;
        other.Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        object_ = nullptr;
        AssignControlBlock();
    }
    void Swap(WeakPtr& other) {
        std::swap(object_, other.object_);
        std::swap(control_block_, other.control_block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!control_block_) {
            return 0;
        }
        return control_block_->GetCount();
    }
    bool Expired() const {
        return UseCount() == 0;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>{};
        }
        return SharedPtr<T>(*this);
    }

private:
    template <typename>
    friend class SharedPtr;
    template <typename>
    friend class EnableSharedFromThis;

    void AssignControlBlock(ControlBlockBase* block = nullptr) {
        if (control_block_) {
            control_block_->Unsubscribe(WeakTag{});
        }
        control_block_ = nullptr;
        control_block_ = block;
        if (control_block_) {
            control_block_->Subscribe(WeakTag{});
        }
    }

    WeakPtr(ControlBlockBase* block, T* object) : object_(object) {
        AssignControlBlock(block);
    }

    ControlBlockBase* control_block_ = nullptr;
    T* object_ = nullptr;
};

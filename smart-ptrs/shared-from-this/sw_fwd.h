#pragma once

#include <array>
#include <cstddef>
#include <exception>
#include <optional>
#include <type_traits>
#include <utility>

class BadWeakPtr : public std::exception {};

class ControlBlockBase {
public:
    virtual void DestroyObject() = 0;
    virtual ~ControlBlockBase() = default;

    struct SharedTag {};
    struct WeakTag {};

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

class EnableSharedFromThisBase {};

template <typename T>
class EnableSharedFromThis;

template <typename T>
class EnableSharedFromThisControlBlock final : public ControlBlockBase {
public:
    explicit EnableSharedFromThisControlBlock(EnableSharedFromThis<T>* object) : object_(object) {
    }

    void DestroyObject() override {
        delete dynamic_cast<T*>(object_);
    }

private:
    EnableSharedFromThis<T>* object_;
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

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

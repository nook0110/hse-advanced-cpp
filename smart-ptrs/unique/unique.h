#pragma once

#include "compressed_pair.h"

#include <concepts>
#include <cstddef>  // std::nullptr_t
#include <memory>
#include <type_traits>
#include <utility>

// Primary template
template <typename T, typename Deleter = std::default_delete<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter{}) {
    }
    UniquePtr(T* ptr, Deleter deleter) : data_(ptr, std::move(deleter)) {
    }

    template <typename DerivedFromT, typename DerivedFromDeleter>
        requires(std::derived_from<DerivedFromT, T> &&
                 std::is_convertible_v<DerivedFromDeleter, Deleter>)
    UniquePtr(UniquePtr<DerivedFromT, DerivedFromDeleter>&& other) noexcept
        : data_(static_cast<T*>(other.Release()), std::move(other.GetDeleter())) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        Swap(other);
        other.Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == std::addressof(other)) {
            return *this;
        }

        Swap(other);
        other.Reset();
        return *this;
    };
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto released = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return released;
    }

    void Reset(T* ptr = nullptr) {
        auto released_ptr = Release();
        data_.GetFirst() = ptr;
        if (released_ptr) {
            GetDeleter()(released_ptr);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return Get();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *Get();
    }
    T* operator->() const {
        return Get();
    }

private:
    template <typename, typename>
    friend class UniquePtr;

    CompressedPair<T*, Deleter> data_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> : public UniquePtr<T, Deleter> {
public:
    explicit UniquePtr(T* ptr = nullptr) : UniquePtr<T, Deleter>(ptr, Deleter{}) {
    }

    T& operator[](size_t idx) {
        return *(this->data_.GetFirst() + idx);
    }
    const T& operator[](size_t idx) const {
        return *(this->data_.GetFirst() + idx);
    }
};
#pragma once

#include <type_traits>
#include <utility>
#include <cstddef>

// Me think, why waste time write lot code, when few code do trick.

template <class Type>
concept EmptyClassAndNotFinal = std::is_empty_v<Type> && !std::is_final_v<Type>;

template <size_t I, typename Type>
class Wrapper;

template <size_t I, typename NonEmptyType>
    requires(!EmptyClassAndNotFinal<NonEmptyType>)
class Wrapper<I, NonEmptyType> {
public:
    Wrapper() = default;
    template <class... Args>
    Wrapper(std::in_place_t, Args... args) : value_(std::forward<Args>(args)...) {
    }

    NonEmptyType& GetValue() {
        return value_;
    }
    const NonEmptyType& GetValue() const {
        return value_;
    }

private:
    NonEmptyType value_;
};

template <size_t I, class EmptyType>
    requires(EmptyClassAndNotFinal<EmptyType>)
class Wrapper<I, EmptyType> : public EmptyType {
public:
    Wrapper() = default;

    template <class... Args>
    Wrapper(std::in_place_t, Args&&... args) : EmptyType(std::forward<Args>(args)...) {
    }

    EmptyType& GetValue() {
        return *this;
    }
    const EmptyType& GetValue() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : protected Wrapper<0, F>, protected Wrapper<1, S> {
public:
    CompressedPair() = default;
    CompressedPair(const F& f, const S& s) : Wrapper<0, F>(f), Wrapper<1, S>(s) {
    }
    template <class U1, class U2>
    CompressedPair(U1&& f, U2&& s)
        : Wrapper<0, F>(std::in_place, std::forward<U1>(f)),
          Wrapper<1, S>(std::in_place, std::forward<U2>(s)) {
    }

    F& GetFirst() {
        return Wrapper<0, F>::GetValue();
    }
    const F& GetFirst() const {
        return Wrapper<0, F>::GetValue();
    }
    S& GetSecond() {
        return Wrapper<1, S>::GetValue();
    }
    const S& GetSecond() const {
        return Wrapper<1, S>::GetValue();
    }
};
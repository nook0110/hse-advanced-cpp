#pragma once

#include <type_traits>
#include <utility>
#include <cstddef>

// Me think, why waste time write lot code, when few code do trick.

template <class Type>
concept EmptyClassAndNotFinal = std::is_empty_v<Type> && !std::is_final_v<Type>;

template <size_t I, typename Type>
class Wrapper;

template <size_t I, typename Type>
    requires(!EmptyClassAndNotFinal<Type>)
class Wrapper<I, Type> {
public:
    Wrapper() = default;
    template <class... Args>
    Wrapper(std::in_place_t, Args... args) : value_(std::forward<Args>(args)...) {
    }

    Type& GetValue() {
        return value_;
    }
    const Type& GetValue() const {
        return value_;
    }

private:
    Type value_ = Type{};
};

template <size_t I, class Type>
    requires(EmptyClassAndNotFinal<Type>)
class Wrapper<I, Type> : public Type {
public:
    Wrapper() = default;

    template <class... Args>
    Wrapper(std::in_place_t, Args&&... args) : Type(std::forward<Args>(args)...) {
    }

    Type& GetValue() {
        return *this;
    }
    const Type& GetValue() const {
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
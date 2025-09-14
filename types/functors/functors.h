#pragma once

#include <utility>
#include <vector>
#include <functional>

template <class Functor>
class ReverseBinaryFunctor {
public:
    explicit ReverseBinaryFunctor(Functor f) : f_(f){};

    template <class T, class U>
    auto operator()(T&& t, U&& u) const {
        return f_(std::forward<U>(u), std::forward<T>(t));
    }

private:
    Functor f_;
};

template <class Functor>
class ReverseUnaryFunctor {
public:
    explicit ReverseUnaryFunctor(Functor f) : f_(f){};
    template <class T>
    auto operator()(T&& t) const {
        return !f_(std::forward<T>(t));
    }

private:
    Functor f_;
};

template <class Functor>
ReverseUnaryFunctor<Functor> MakeReverseUnaryFunctor(Functor functor) {
    return ReverseUnaryFunctor<Functor>(functor);
}

template <class Functor>
ReverseBinaryFunctor<Functor> MakeReverseBinaryFunctor(Functor functor) {
    return ReverseBinaryFunctor<Functor>(functor);
}

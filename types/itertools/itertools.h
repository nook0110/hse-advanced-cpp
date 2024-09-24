#pragma once

#include <concepts>
#include <iterator>
#include <utility>
template <class Iterator>
class IteratorRange {
public:
    using const_iterator = Iterator;  // NOLINT
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {
    }

    Iterator begin() const {  // NOLINT
        return begin_;
    }

    Iterator end() const {  // NOLINT
        return end_;
    }

private:
    Iterator begin_, end_;
};

template <typename T>
class RangeIter {
public:
    RangeIter(T value, T step = 1) : value_(value), step_(step) {
    }

    RangeIter& operator++() {
        value_ += step_;
        return *this;
    }

    RangeIter operator++(int) {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    bool operator==(const RangeIter& other) const {
        return value_ >= other.value_;
    }
    T operator*() const {
        return value_;
    }

private:
    T value_;
    T step_;
};

template <typename T>
IteratorRange<RangeIter<T>> Range(T from, T to, T step = 1) {
    return IteratorRange(RangeIter<T>(from, step), RangeIter<T>(to, step));
}

template <typename T>
IteratorRange<RangeIter<T>> Range(T to) {
    return Range(T{0}, to);
}

template <typename Iterator1, typename Iterator2>
class ZipIter {
public:
    ZipIter(Iterator1 it1, Iterator2 it2) : it1_(it1), it2_(it2) {
    }

    ZipIter& operator++() {
        ++it1_;
        ++it2_;
        return *this;
    }

    ZipIter operator++(int) {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    auto operator*() const {
        return std::make_pair(*it1_, *it2_);
    }

    bool operator==(const ZipIter& other) const {
        return it1_ == other.it1_ || it2_ == other.it2_;
    }

private:
    Iterator1 it1_;
    Iterator2 it2_;
};

template <typename T, typename U>
IteratorRange<ZipIter<typename T::const_iterator, typename U::const_iterator>> Zip(const T& t,
                                                                                   const U& u) {
    return IteratorRange(
        ZipIter<typename T::const_iterator, typename U::const_iterator>(std::begin(t),
                                                                        std::begin(u)),
        ZipIter<typename T::const_iterator, typename U::const_iterator>(std::end(t), std::end(u)));
}

template <typename Iterator>
class GroupIter {
public:
    GroupIter(Iterator it, Iterator end) : it_(it), end_(end), next_it_(CalcNext()) {
    }

    GroupIter& operator++() {
        it_ = next_it_;
        next_it_ = CalcNext();
        return *this;
    }
    GroupIter operator++(int) {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    IteratorRange<Iterator> operator*() const {
        return IteratorRange<Iterator>(it_, next_it_);
    }

    bool operator==(const GroupIter& other) const {
        return it_ == other.it_;
    }

private:
    Iterator CalcNext() const {
        Iterator next = it_;
        while (next != end_ && *next == *it_) {
            ++next;
        }
        return next;
    }

    Iterator it_;
    Iterator end_;
    Iterator next_it_;
};

template <typename T>
IteratorRange<GroupIter<typename T::const_iterator>> Group(const T& t) {
    return {{std::begin(t), std::end(t)}, {std::end(t), std::end(t)}};
}
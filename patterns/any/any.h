#pragma once

#include <memory>
#include <typeinfo>
#include <utility>

struct ValueBase {
    virtual std::unique_ptr<ValueBase> DeepCopy() const = 0;
    virtual ~ValueBase() = default;
};

template <typename T>
struct Value : public ValueBase {
    Value(T value) : value(std::move(value)) {
    }
    std::unique_ptr<ValueBase> DeepCopy() const final {
        return std::make_unique<Value>(value);
    }
    T value;
};

class Any {
public:
    Any() = default;
    template <class T>
    Any(const T& value) : value_(std::make_unique<Value<T>>(value)) {
    }

    template <class T>
    Any& operator=(const T& value) {
        value_ = std::make_unique<Value<T>>(value);
        return *this;
    }

    Any(const Any& rhs) : value_(rhs.value_->DeepCopy()) {
    }
    Any& operator=(const Any& rhs) {
        Any copy(rhs);
        Swap(copy);
        return *this;
    }
    ~Any() = default;

    bool Empty() const {
        return !value_;
    }

    void Clear() {
        value_.reset();
    }
    void Swap(Any& rhs) {
        std::swap(value_, rhs.value_);
    }

    template <class T>
    const T& GetValue() const {
        auto ptr = dynamic_cast<Value<T>*>(value_.get());
        if (!ptr) {
            throw std::bad_cast();
        }
        return ptr->value;
    }

private:
    std::unique_ptr<ValueBase> value_ = nullptr;
};

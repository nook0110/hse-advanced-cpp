#pragma once

#include <concepts>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <optional>

class MessageException : public std::exception {
public:
    MessageException(std::string msg) : msg_(std::move(msg)) {
    }

private:
    const char* what() const noexcept override {  // NOLINT
        return msg_.c_str();
    }
    std::string msg_;
};

class BadTryAccess : public MessageException {
public:
    struct ObjectEmptyTag {};
    struct NoExceptionTag {};
    BadTryAccess(ObjectEmptyTag) : MessageException("Object is empty") {};
    BadTryAccess(NoExceptionTag) : MessageException("No exception") {};
};

template <class T>
class Try {
public:
    Try() = default;
    Try(T t) : value_(std::move(t)) {
    }

    template <class Exception>
        requires(std::is_base_of_v<std::exception, Exception>)
    Try(Exception exception) : value_(std::make_exception_ptr(exception)) {
    }

    Try(std::exception_ptr exception) : value_(exception) {
    }

    const T& Value() {
        if (!value_) {
            throw BadTryAccess{BadTryAccess::ObjectEmptyTag()};
        }

        if (IsFailed()) {
            std::rethrow_exception(std::get<std::exception_ptr>(*value_));
        }

        return std::get<T>(*value_);
    }

    bool IsFailed() {
        if (!value_) {
            return false;
        }

        return std::holds_alternative<std::exception_ptr>(*value_);
    }

    void Throw() {
        if (!IsFailed()) {
            throw BadTryAccess{BadTryAccess::NoExceptionTag()};
        }
    }

private:
    std::optional<std::variant<T, std::exception_ptr>> value_;
};

template <>
class Try<void> {
public:
    Try() = default;
    template <class Exception>
        requires(std::is_base_of_v<std::exception, Exception>)
    Try(Exception exception) : value_(std::make_exception_ptr(exception)) {
    }
    Try(std::exception_ptr exception) : value_(exception) {
    }

    bool IsFailed() {
        return value_.has_value();
    }

    void Throw() {
        if (!IsFailed()) {
            throw BadTryAccess{BadTryAccess::NoExceptionTag()};
        }
    }

private:
    std::optional<std::exception_ptr> value_;
};

class UnknownException : public std::exception {
    const char* what() const noexcept override {  // NOLINT
        return msg_.c_str();
    }

    std::string msg_ = "Unknown exception";
};
template <class Function, class... Args>
auto TryRun(Function func, Args&&... args) {
    using ReturnType = decltype(func(args...));
    try {
        if constexpr (std::same_as<ReturnType, void>) {
            func(std::forward<Args>(args)...);
            return Try<ReturnType>();
        } else {
            return Try<ReturnType>(func(std::forward<Args>(args)...));
        }
    } catch (std::exception&) {
        return Try<ReturnType>(std::current_exception());
    } catch (const char* msg) {
        return Try<ReturnType>(MessageException(msg));
    } catch (int error_code) {
        return Try<ReturnType>(MessageException(std::strerror(error_code)));
    } catch (...) {
        return Try<ReturnType>(UnknownException{});
    }
    return Try<ReturnType>{};
}

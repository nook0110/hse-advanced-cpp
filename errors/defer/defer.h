#pragma once

#include <optional>

template <typename Callback>
class Defer final {
public:
    Defer(Callback callback) : callback_(std::move(callback)) {
    }

    void Invoke() {
        std::move (*callback_)();
        callback_ = std::nullopt;
    }

    void Cancel() {
        callback_ = std::nullopt;
    }

    ~Defer() {
        if (callback_) {
            Invoke();
        }
    }

private:
    void operator()() && {
        Invoke();
    }

    std::optional<Callback> callback_;
};

#pragma once

#include <utility>
#include <optional>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <utility>
#include <optional>

template <class T>
class UnbufferedChannel {
public:
    void Send(const T& value) {
        std::unique_lock lock(mutex_);

        waiting_sender_.wait(lock, [this]() { return !sending_ || closed_; });

        sending_ = true;

        if (closed_) {
            throw std::runtime_error("wat r u doing bro");
        }

        data_.emplace(value);
        reader_.notify_one();
        sended_sender_.wait(lock, [this]() { return !data_ || closed_; });

        if (closed_ && data_) {
            throw std::runtime_error("wat r u doing bro");
        }

        sending_ = false;
        waiting_sender_.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock lock(mutex_);

        reader_.wait(lock, [this]() { return data_ || closed_; });

        if (closed_) {
            return {};
        }

        auto ans = *data_;
        data_ = {};
        sended_sender_.notify_one();
        return ans;
    }

    void Close() {
        std::unique_lock lock(mutex_);
        closed_ = true;
        NotifyAll();
    }

    bool closed_ = false;

    std::optional<T> data_;
    std::size_t size_;

    std::mutex mutex_;
    std::condition_variable waiting_sender_;
    std::condition_variable sended_sender_;
    std::condition_variable reader_;

    bool sending_ = false;

    void NotifyAll() {
        waiting_sender_.notify_all();
        sended_sender_.notify_all();
        reader_.notify_all();
    }
};

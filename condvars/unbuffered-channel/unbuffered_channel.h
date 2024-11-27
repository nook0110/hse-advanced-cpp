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

        cv_.wait(lock, [this]() { return data_.size() < size_ || closed_; });

        if (closed_) {
            cv_.notify_all();
            throw std::runtime_error("wat r u doing bro");
        }

        data_.emplace(value);
        read = false;
        cv_.notify_all();
        cv_.wait(lock, [this]() { return read || closed_; });
        if(closed_ && !read) {
            cv_.notify_all();
            throw std::runtime_error("wat r u doing bro");
        }
        cv_.notify_all();
    }

    std::optional<T> Recv() {
        std::unique_lock lock(mutex_);

        cv_.wait(lock, [this]() { return !data_.empty() || closed_; });

        if (closed_ && data_.empty()) {
            cv_.notify_all();
            return {};
        }

        auto ans = data_.front();
        data_.pop();
        read = true;
        cv_.notify_all();
        return ans;
    }

    void Close() {
        std::unique_lock lock(mutex_);
        closed_ = true;
        cv_.notify_all();
    }

    bool closed_ = false;

    std::queue<T> data_;
    std::size_t size_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool read = false;
};

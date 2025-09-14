#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>

class DefaultCallback {
public:
    void operator()(int& value) {
        --value;
    }
};

class Semaphore {
public:
    Semaphore(int count) : id_to_enter_(count), count_(count) {
    }

    void Leave() {
        std::unique_lock<std::mutex> lock(mutex_);
        ++count_;
        ++id_to_enter_;
        cv_.notify_all();
    }

    template <class Func>
    void Enter(Func callback) {
        std::unique_lock<std::mutex> lock(mutex_);

        size_t cur_id = (++next_id_);

        while (id_to_enter_ < cur_id || count_ == 0) {
            cv_.wait(lock);
        }

        callback(count_);
    }

    void Enter() {
        DefaultCallback callback;
        Enter(callback);
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    size_t next_id_ = 0;
    size_t id_to_enter_ = 0;
    int count_ = 0;
};

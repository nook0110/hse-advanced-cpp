#pragma once
#include <condition_variable>
#include <mutex>

class RWLock {
public:
    template <class Func>
    void Read(Func func) {
        ReadLock();

        try {
            func();
        } catch (...) {
            ReadUnlock();
            throw;
        }

        ReadUnlock();
    }

    void ReadLock() {
        std::unique_lock lock(mutex_);

        cv_.wait(lock, [this]() { return !writer_; });

        ++blocked_readers_;
    }

    void ReadUnlock() {
        std::unique_lock lock(mutex_);
        --blocked_readers_;
        if (blocked_readers_ == 0) {
            cv_.notify_all();
        }
    }

    template <class Func>
    void Write(Func func) {
        WriteLock();
        func();
        WriteUnlock();
    }

    void WriteLock() {
        std::unique_lock lock(mutex_);

        cv_.wait(lock, [this]() { return !writer_ && blocked_readers_ == 0; });
        writer_ = true;
    }

    void WriteUnlock() {
        std::unique_lock lock(mutex_);
        writer_ = false;
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;

    bool writer_ = false;
    int blocked_readers_ = 0;
};

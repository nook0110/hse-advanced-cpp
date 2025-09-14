#include <iostream>
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>

template <class T>
class TimerQueue {
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

public:
    void Add(const T& item, TimePoint at) {
        std::lock_guard lock(mutex_);
        queue_.push(Item{item, at});
        cv_.notify_all();
    }

    T Pop() {
        std::unique_lock lock(mutex_);

        cv_.wait(lock, [this]() { return !queue_.empty(); });

        cv_.wait_until(lock, queue_.top().time, [this]() {
            auto min_time = queue_.top().time;
            return Clock::now() >= min_time;
        });

        Item item = queue_.top();
        queue_.pop();
        return item.value;
    }

private:
    struct Item {
        T value;
        TimePoint time;

        auto operator<=>(const Item& other) const {
            return time <=> other.time;
        }
    };

    std::priority_queue<Item, std::vector<Item>, std::greater<>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

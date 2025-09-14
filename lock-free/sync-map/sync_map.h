#pragma once

#include <atomic>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "hazard_ptr.h"

template <class K, class V>
class SyncMap {
public:
    struct Snapshot {
        std::shared_ptr<std::unordered_map<K, V>> read_only;

        bool dirty = false;
    };

    bool Lookup(const K& key, V* value) {
        if (snapshot_.load() && !snapshot_.load()->dirty) {
            Snapshot* snap = Acquire(&snapshot_);
            if (!snap->read_only->contains(key)) {
                return false;
            }
            *value = snap->read_only->at(key);
            return true;
        }
        std::lock_guard lock(lock_);
        ++operation_count_;

        if (!mutable_map_->contains(key)) {
            return false;
        }
        *value = mutable_map_->at(key);
        if (operation_count_ > 100) {
            auto new_value = new Snapshot{mutable_map_, false};
            Snapshot* old_value = snapshot_.exchange(new_value);
            if (old_value) {
                Retire(old_value);
            }
            operation_count_ = 0;
        }
        return true;
    }

    bool Insert(const K& key, const V& value) {
        std::lock_guard lock(lock_);
        if (mutable_map_->contains(key)) {
            return false;
        }
        if (snapshot_.load()) {
            snapshot_.load()->dirty = true;
        }
        (*mutable_map_)[key] = value;
        return true;
    }

    SyncMap() = default;

    ~SyncMap() {
        Snapshot* old_value = snapshot_.exchange(nullptr);
        if (old_value) {
            Retire(old_value);
        }
    }

    std::atomic<Snapshot*> snapshot_;

    std::mutex lock_;
    std::shared_ptr<std::unordered_map<K, V>> mutable_map_ =
        std::make_shared<std::unordered_map<K, V>>();
    int operation_count_ = 0;
};

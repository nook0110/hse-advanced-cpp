#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct State {
    void Subscibe() {
        ++ref_count;
    }
    void Unsubscibe() {
        --ref_count;
        if (ref_count == 0) {
            delete this;
        }
    }
    std::unique_ptr<State> GetCopy() const {
        return std::make_unique<State>(1, data);
    }

    size_t ref_count = 1;
    std::vector<std::string> data;
};

class COWVector {
public:
    COWVector() : state_(new State) {
    }
    ~COWVector() {
        state_->Unsubscibe();
    }

    COWVector(const COWVector& other) : state_(other.state_) {
        state_->Subscibe();
    }
    COWVector& operator=(const COWVector& other) {
        state_->Unsubscibe();
        state_ = other.state_;
        state_->Subscibe();
        return *this;
    }

    COWVector(COWVector&& other) {
        Swap(other);
    }

    COWVector& operator=(COWVector&& other) {
        Swap(other);
        return *this;
    }

    size_t Size() const {
        return state_->data.size();
    }

    void Resize(size_t size) {
        GoingToChange();

        state_->data.resize(size);
    }

    const std::string& Get(size_t at) const {
        return state_->data[at];
    }
    const std::string& Back() const {
        return state_->data.back();
    }

    void PushBack(const std::string& value) {
        GoingToChange();
        state_->data.push_back(value);
    }

    void Set(size_t at, const std::string& value) {
        GoingToChange();
        state_->data[at] = value;
    }

    void Swap(COWVector& other) {
        std::swap(state_, other.state_);
    }

private:
    void GoingToChange() {
        if (state_->ref_count > 1) {
            auto new_state = state_->GetCopy();
            state_->Unsubscibe();
            state_ = new_state.release();
        }
    }

    State* state_ = nullptr;
};

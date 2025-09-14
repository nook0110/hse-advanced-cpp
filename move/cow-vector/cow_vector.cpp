#include "cow_vector.h"

void State::Subscibe() {
    ++ref_count;
}
void State::Unsubscibe() {
    --ref_count;
    if (ref_count == 0) {
        delete this;
    }
}
std::unique_ptr<State> State::GetCopy() const {
    return std::make_unique<State>(1, data);
}

COWVector::COWVector() : state_(new State) {
}
COWVector::~COWVector() {
    state_->Unsubscibe();
}

COWVector::COWVector(const COWVector& other) : state_(other.state_) {
    state_->Subscibe();
}
COWVector& COWVector::operator=(const COWVector& other) {
    state_->Unsubscibe();
    state_ = other.state_;
    state_->Subscibe();
    return *this;
}

COWVector::COWVector(COWVector&& other) {
    Swap(other);
}

COWVector& COWVector::operator=(COWVector&& other) {
    Swap(other);
    return *this;
}

size_t COWVector::Size() const {
    return state_->data.size();
}

void COWVector::Resize(size_t size) {
    GoingToChange();

    state_->data.resize(size);
}

const std::string& COWVector::Get(size_t at) const {
    return state_->data[at];
}
const std::string& COWVector::Back() const {
    return state_->data.back();
}

void COWVector::PushBack(const std::string& value) {
    GoingToChange();
    state_->data.push_back(value);
}

void COWVector::Set(size_t at, const std::string& value) {
    GoingToChange();
    state_->data[at] = value;
}

void COWVector::Swap(COWVector& other) {
    std::swap(state_, other.state_);
}

void COWVector::GoingToChange() {
    if (state_->ref_count > 1) {
        auto new_state = state_->GetCopy();
        state_->Unsubscibe();
        state_ = new_state.release();
    }
}

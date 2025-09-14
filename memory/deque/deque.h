#pragma once

#include <cstddef>
#include <initializer_list>
#include <algorithm>
#include <memory>
#include <cassert>
#include <utility>

class Block {
public:
    static constexpr size_t kBlockSize = 512 / sizeof(int);

    Block() = default;

    [[nodiscard]] bool IsFull() const {
        return Size() == kBlockSize;
    }

    [[nodiscard]] bool IsEmpty() const {
        return Size() == 0;
    }

    size_t Size() const {
        return size_;
    }

    size_t GetNextIndex(size_t idx) const {
        return (idx + 1) % kBlockSize;
    }

    size_t GetPreviousIndex(size_t idx) const {
        return (idx + kBlockSize - 1) % kBlockSize;
    }

    void PushFront(int value) {
        ++size_;
        block_[next_front_block_] = value;
        next_front_block_ = GetPreviousIndex(next_front_block_);
    }
    void PushBack(int value) {
        ++size_;
        block_[next_back_block_] = value;
        next_back_block_ = GetNextIndex(next_back_block_);
    }

    void PopFront() {
        assert(Size() != 0);
        --size_;
        next_front_block_ = GetNextIndex(next_front_block_);
    }
    void PopBack() {
        assert(Size() != 0);
        --size_;
        next_back_block_ = GetPreviousIndex(next_back_block_);
    }

    int& operator[](size_t ind) {
        return block_[GetNextIndex(next_front_block_ + ind)];
    }

    int operator[](size_t ind) const {
        return block_[GetNextIndex(next_front_block_ + ind)];
    }

private:
    size_t next_front_block_ = kBlockSize - 1;
    size_t next_back_block_ = 0;
    size_t size_ = 0;
    int block_[kBlockSize];
};

class RingBuffer {
public:
    RingBuffer(size_t size = 0)
        : capacity_(std::max(size_t{2}, size)),
          next_front_block_(capacity_ - 1),
          buffer_(std::make_unique<BlockArray>(capacity_)) {
        for (size_t i = 0; i < size; ++i) {
            PushBack(0);
        }
    }

    RingBuffer(const RingBuffer& other)
        : capacity_(other.capacity_),
          next_front_block_(capacity_ - 1),
          buffer_(std::make_unique<BlockArray>(capacity_)) {
        for (size_t i = 0; i < other.Size(); ++i) {
            PushBack(other[i]);
        }
    }

    RingBuffer(RingBuffer&& other) = default;
    RingBuffer& operator=(RingBuffer&& other) = default;

    void PushBack(int value) {
        if (Size() == 0 || GetBackBlock()->IsFull()) {
            AddBlock(false);
        }
        return GetBackBlock()->PushBack(value);
    }

    void PopBack() {
        if (GetBackBlock()->IsEmpty()) {
            DeleteBlock(false);
        }
        GetBackBlock()->PopBack();
    }

    void PushFront(int value) {
        if (Size() == 0 || GetFrontBlock()->IsFull()) {
            AddBlock(true);
        }
        return GetFrontBlock()->PushFront(value);
    }

    void PopFront() {
        if (GetFrontBlock()->IsEmpty()) {
            DeleteBlock(true);
        }
        GetFrontBlock()->PopFront();
    }

    int& operator[](size_t ind) {
        size_t first_block_filled = GetFrontBlock()->Size();
        if (ind < first_block_filled) {
            return GetFrontBlock()->operator[](ind);
        }
        size_t block_idx =
            ((next_front_block_ + 1) + 1 + (ind - first_block_filled) / Block::kBlockSize) %
            capacity_;
        return GetBlock(block_idx)->operator[]((ind - first_block_filled) % Block::kBlockSize);
    }

    int operator[](size_t ind) const {
        return const_cast<RingBuffer*>(this)->operator[](ind);  // NOLINT
    }

    size_t Size() const {
        if (GetBackBlock().get() == nullptr /* no blocks were allocated */) {
            return 0;
        }

        auto allocated = amount_of_blocks_ * Block::kBlockSize;

        auto excess = Block::kBlockSize - GetFrontBlock()->Size();
        if (GetFrontBlock() != GetBackBlock()) {
            excess += Block::kBlockSize - GetBackBlock()->Size();
        }
        return allocated - excess;
    }

private:
    bool IsFull() const {
        return amount_of_blocks_ == capacity_;
    }

    const std::unique_ptr<Block>& GetFrontBlock() const {
        return GetBlock(GetNextIndex(next_front_block_));
    }

    const std::unique_ptr<Block>& GetBackBlock() const {
        return GetBlock(GetPreviousIndex(next_back_block_));
    }

    const std::unique_ptr<Block>& GetBlock(size_t idx) const {
        return *(buffer_.get() + idx);
    }

    std::unique_ptr<Block>& GetFrontBlock() {
        return GetBlock(GetNextIndex(next_front_block_));
    }

    std::unique_ptr<Block>& GetBackBlock() {
        return GetBlock(GetPreviousIndex(next_back_block_));
    }

    std::unique_ptr<Block>& GetBlock(size_t idx) {
        return *(buffer_.get() + idx);
    }

    size_t GetNextIndex(size_t idx) const {
        return (idx + 1) % capacity_;
    }

    size_t GetPreviousIndex(size_t idx) const {
        return (idx + capacity_ - 1) % capacity_;
    }

    void DeleteBlock(bool front) {
        --amount_of_blocks_;
        if (front) {
            assert(GetFrontBlock()->IsEmpty());
            GetFrontBlock().reset();
            next_front_block_ = GetNextIndex(next_front_block_);
        } else {
            assert(GetBackBlock()->IsEmpty());
            GetBackBlock().reset();
            next_back_block_ = GetPreviousIndex(next_back_block_);
        }
    }

    void AddBlock(bool front) {
        if (IsFull()) {
            Reallocate();
        }

        if (front) {
            assert(Size() == 0 || GetFrontBlock()->IsFull());
            assert(GetBlock(next_front_block_).get() == nullptr);
            GetBlock(next_front_block_) = std::make_unique<Block>();
            next_front_block_ = GetPreviousIndex(next_front_block_);

        } else {
            assert(Size() == 0 || GetBackBlock()->IsFull());
            assert(GetBlock(next_back_block_).get() == nullptr);
            GetBlock(next_back_block_) = std::make_unique<Block>();
            next_back_block_ = GetNextIndex(next_back_block_);
        }

        ++amount_of_blocks_;
    }

    void Reallocate() {
        assert(IsFull());
        auto new_buffer = std::make_unique<BlockArray>(capacity_ * 2);
        if (GetNextIndex(next_front_block_) < GetPreviousIndex(next_back_block_) + 1) {
            std::move(buffer_.get() + GetNextIndex(next_front_block_),
                      buffer_.get() + GetPreviousIndex(next_back_block_) + 1, new_buffer.get());
        } else {
            std::move(buffer_.get() + GetNextIndex(next_front_block_), buffer_.get() + capacity_,
                      new_buffer.get());

            std::move(buffer_.get(), buffer_.get() + GetPreviousIndex(next_back_block_) + 1,
                      new_buffer.get() + (capacity_ - GetNextIndex(next_front_block_)));
        }
        next_back_block_ = capacity_;
        next_front_block_ = 2 * capacity_ - 1;
        capacity_ *= 2;
        buffer_.swap(new_buffer);
    }

    using BlockPtr = std::unique_ptr<Block>;
    using BlockArray = BlockPtr[];

    size_t capacity_ = 1;
    size_t next_front_block_ = 0;
    size_t next_back_block_ = 0;
    size_t amount_of_blocks_ = 0;
    std::unique_ptr<BlockArray> buffer_;
};

class Deque {
public:
    Deque() = default;
    Deque(const Deque& rhs) = default;
    Deque(Deque&& rhs) = default;
    explicit Deque(size_t size) : data_(size) {
    }

    Deque(std::initializer_list<int> list) : data_(list.size()) {
        size_t idx = 0;
        for (auto element : list) {
            data_[idx++] = element;
        }
    }

    Deque& operator=(Deque rhs) {
        Swap(rhs);
        return *this;
    }

    void Swap(Deque& rhs) {
        std::swap(data_, rhs.data_);
    }

    void PushBack(int value) {
        data_.PushBack(value);
    }

    void PopBack() {
        data_.PopBack();
    }

    void PushFront(int value) {
        data_.PushFront(value);
    }

    void PopFront() {
        data_.PopFront();
    }

    int& operator[](size_t ind) {
        return data_[ind];
    }

    int operator[](size_t ind) const {
        return data_[ind];
    }

    size_t Size() const {
        return data_.Size();
    }

    void Clear() {
        data_ = RingBuffer{};
    }

private:
    RingBuffer data_;
};

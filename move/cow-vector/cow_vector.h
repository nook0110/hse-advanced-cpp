#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct State {
    void Subscibe();
    void Unsubscibe();
    std::unique_ptr<State> GetCopy() const;

    size_t ref_count = 1;
    std::vector<std::string> data;
};

class COWVector {
public:
    COWVector();
    ~COWVector();

    COWVector(const COWVector& other);
    COWVector& operator=(const COWVector& other);

    COWVector(COWVector&& other);

    COWVector& operator=(COWVector&& other);

    size_t Size() const;

    void Resize(size_t size);
    const std::string& Get(size_t at) const;
    const std::string& Back() const;

    void PushBack(const std::string& value);

    void Set(size_t at, const std::string& value);

    void Swap(COWVector& other);

private:
    void GoingToChange();

    State* state_ = nullptr;
};

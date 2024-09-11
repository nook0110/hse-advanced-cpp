#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <cstring>

class StringView {
public:
    StringView(const std::string& string, size_t start = 0, size_t len = std::string::npos)
        : data_(string.data() + start), size_(std::min(string.size() - start, len)) {
    }
    StringView(const char* c_string) : data_(c_string), size_(std::strlen(c_string)) {
    }
    StringView(const char* data, size_t size) : data_(data), size_(size) {
    }

    char operator[](const size_t i) const {
        return data_[i];
    }

    size_t Size() const {
        return size_;
    }

private:
    const char* data_;
    size_t size_;
};

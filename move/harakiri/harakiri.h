#pragma once

#include <string>

// Should not allow reuse and yell under sanitizers.
// Fix the interface and implementation.
// AwesomeCallback should add "awesomeness".

class OneTimeCallback {
public:
    OneTimeCallback(std::string s) : s_(std::move(s)) {
    }

    virtual std::string operator()() const&& {
        std::string ans = s_;
        delete this;
        return ans;
    }

    virtual ~OneTimeCallback() = default;

private:
    std::string s_;
};

// Implement ctor, operator(), maybe something else...
class AwesomeCallback : public OneTimeCallback {
public:
    AwesomeCallback(std::string s) : OneTimeCallback(s + "awesomeness"){};

    ~AwesomeCallback() = default;
};

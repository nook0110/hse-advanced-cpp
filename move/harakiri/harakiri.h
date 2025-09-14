#pragma once

#include <string>

// Should not allow reuse and yell under sanitizers.
// Fix the interface and implementation.
// AwesomeCallback should add "awesomeness".

class OneTimeCallback {
public:
    OneTimeCallback(std::string s);

    virtual std::string operator()() const&&;

    virtual ~OneTimeCallback() = default;

private:
    std::string s_;
};

// Implement ctor, operator(), maybe something else...
class AwesomeCallback : public OneTimeCallback {
public:
    AwesomeCallback(std::string s);
    ~AwesomeCallback() = default;
};

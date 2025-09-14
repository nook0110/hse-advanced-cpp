#include "harakiri.h"

// Your code goes here
OneTimeCallback::OneTimeCallback(std::string s) : s_(std::move(s)) {
}

std::string OneTimeCallback::operator()() const&& {
    std::string ans = s_;
    delete this;
    return ans;
}

AwesomeCallback::AwesomeCallback(std::string s) : OneTimeCallback(s + "awesomeness") {};

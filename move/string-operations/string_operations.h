#pragma once

#include <cstring>
#include <string_view>
#include <string>
#include <vector>

bool StartsWith(std::string_view string, std::string_view text);
bool EndsWith(std::string_view string, std::string_view text);
std::string_view StripPrefix(std::string_view string, std::string_view prefix);
std::string_view StripSuffix(std::string_view string, std::string_view suffix);
std::string_view ClippedSubstr(std::string_view s, size_t pos, size_t n = std::string_view::npos);
std::string_view StripAsciiWhitespace(std::string_view text);
std::vector<std::string_view> StrSplit(std::string_view text, std::string_view delim);
std::string ReadN(const std::string& filename, size_t n);
std::string AddSlash(std::string_view path);
std::string_view RemoveSlash(std::string_view path);
std::string_view Dirname(std::string_view path);
std::string_view Basename(std::string_view path);
std::string CollapseSlashes(std::string_view path);
std::string StrJoin(const std::vector<std::string_view>& strings, std::string_view delimiter);

namespace {
inline size_t LengthInStr(const std::string& str) {
    return str.size();
}
inline size_t LengthInStr(const char* str) {
    return strlen(str);
}
inline size_t LengthInStr(std::string_view str) {
    return str.size();
}
template <typename T>
    requires(std::integral<T>)
size_t LengthInStr(T number) {
    if (number == 0) {
        return 1;
    }
    int length = 0;
    if (number < 0) {
        length = 1;
    }
    while (number) {
        number /= 10;
        length++;
    }
    return length;
}
inline void AppendTo(std::string& s, const std::string& str) {
    s += str;
}
inline void AppendTo(std::string& s, const char* str) {
    s += str;
}
inline void AppendTo(std::string& s, std::string_view str) {
    s += str;
}
template <typename T>
    requires(std::integral<T>)
void AppendTo(std::string& s, T number) {
    size_t length = LengthInStr(number);
    size_t s_size = s.size();
    s.resize(s.size() + length, '0');
    if (number < 0) {
        s[s_size] = '-';
    }

    auto cur = s.size();
    while (number) {
        auto next_digit = number % 10;
        if (next_digit < 0) {
            next_digit = -next_digit;
        }
        s[--cur] = '0' + next_digit;
        number /= 10;
    }
}
}  // namespace

template <class... Args>
std::string StrCat(Args&&... args) {
    size_t reserve = (0 + ... + LengthInStr(args));
    std::string ans;
    ans.reserve(reserve);
    (AppendTo(ans, args), ...);
    return ans;
}

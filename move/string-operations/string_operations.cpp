#include "string_operations.h"

#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <numeric>
#include <ranges>
#include <string_view>
#include <string>
#include <vector>

bool StartsWith(std::string_view string, std::string_view text) {
    return std::ranges::equal(string.substr(0, text.size()), text);
}

bool EndsWith(std::string_view string, std::string_view text) {
    if (text.size() > string.size()) {
        return false;
    }

    return std::ranges::equal(string.substr(string.size() - text.size()), text);
}

std::string_view StripPrefix(std::string_view string, std::string_view prefix) {
    if (!StartsWith(string, prefix)) {
        return string;
    }

    string.remove_prefix(prefix.size());
    return string;
}

std::string_view StripSuffix(std::string_view string, std::string_view suffix) {
    if (!EndsWith(string, suffix)) {
        return string;
    }

    string.remove_suffix(suffix.size());
    return string;
}

std::string_view ClippedSubstr(std::string_view s, size_t pos, size_t n) {
    if (n > s.size()) {
        return s;
    }

    return s.substr(pos, n);
}

std::string_view StripAsciiWhitespace(std::string_view text) {
    auto first_nonspace_char =
        std::ranges::find_if_not(text, [](const char c) -> bool { return std::isspace(c); });
    text.remove_prefix(first_nonspace_char - text.begin());
    auto last_nonspace_char = std::ranges::find_if_not(
        std::ranges::reverse_view(text), [](const char c) -> bool { return std::isspace(c); });
    text.remove_suffix(last_nonspace_char - text.rbegin());
    return text;
}

std::vector<std::string_view> StrSplit(std::string_view text, std::string_view delim) {
    std::vector<std::string_view> answer;

    size_t splits = 0;
    {
        auto next = text.find(delim);
        while (next != std::string_view::npos) {
            ++splits;
            next = text.find(delim, next + delim.size());
        }
    }

    answer.reserve(splits + 1);
    {
        auto next = text.find(delim);
        while (next != std::string_view::npos) {
            answer.emplace_back(text.substr(0, next));
            text.remove_prefix(next + delim.size());
            next = text.find(delim);
        }

        answer.emplace_back(text);
    }

    return answer;
}

std::string ReadN(const std::string& filename, size_t n) {
    auto fd = open(filename.data(), O_APPEND);
    if (fd == -1) {
        return {};
    }

    std::string ans(n, ' ');

    if (read(fd, ans.data(), n) == -1) {
        ans = {};
    }

    close(fd);
    return ans;
}

std::string AddSlash(std::string_view path) {
    std::string s;
    s.reserve(path.size() + static_cast<size_t>(!EndsWith(path, "/")));
    s = path;
    if (!EndsWith(path, "/")) {
        s += '/';
    }

    return s;
}

std::string_view RemoveSlash(std::string_view path) {
    if (EndsWith(path, "/") && path.size() > 1) {
        path.remove_suffix(1);
    }

    return path;
}

std::string_view Dirname(std::string_view path) {
    path.remove_suffix(path.size() - path.find_last_of('/') - 1);
    return RemoveSlash(path);
}

std::string_view Basename(std::string_view path) {
    return path.substr(path.find_last_of('/') + 1);
}

std::string CollapseSlashes(std::string_view path) {
    std::string ans;
    ans.reserve(path.size());

    for (const char c : path) {
        if (!ans.empty() && ans.back() == '/' && c == '/') {
            continue;
        }

        ans += c;
    }

    return ans;
}

std::string StrJoin(const std::vector<std::string_view>& strings, std::string_view delimiter) {
    if (strings.empty()) {
        return {};
    }

    auto strings_size =
        std::accumulate(strings.begin(), strings.end(), size_t{0},
                        [](size_t sum, std::string_view string) { return sum + string.size(); });
    auto size = delimiter.size() * (strings.size() - 1) + strings_size;
    std::string ans;
    ans.reserve(size);
    for (size_t i = 0; i < strings.size(); ++i) {
        ans += strings[i];
        if (i != strings.size() - 1) {
            ans += delimiter;
        }
    }

    return ans;
}

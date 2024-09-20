#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <functional>
#include <numeric>
#include <ranges>
#include <string_view>
#include <string>
#include <utility>
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
std::string_view ClippedSubstr(std::string_view s, size_t pos, size_t n = std::string_view::npos) {
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
    s.reserve(path.size() + !path.ends_with('/'));
    s = path;
    if (!path.ends_with('/')) {
        s += '/';
    }
    return s;
}

std::string_view RemoveSlash(std::string_view path) {
    if (path.ends_with('/') && path.size() > 1) {
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
        if (ans.size() > 0 && ans.back() == '/' && c == '/') {
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

size_t LengthInStr(const std::string& str) {
    return str.size();
}

size_t LengthInStr(const char* str) {
    return strlen(str);
}

size_t LengthInStr(std::string_view str) {
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

void AppendTo(std::string& s, const std::string& str) {
    s += str;
}
void AppendTo(std::string& s, const char* str) {
    s += str;
}
void AppendTo(std::string& s, std::string_view str) {
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

template <class... Args>
std::string StrCat(Args&&... args) {
    size_t reserve = (0 + ... + LengthInStr(args));
    std::string ans;
    ans.reserve(reserve);
    (AppendTo(ans, args), ...);
    return ans;
}

/*
`bool StartsWith(STR string, STR text)` — проверяет, что строка `string` начинается с `text`.

`bool EndsWith(STR string, STR text)` — проверяет, что строка `string` оканчивается на `text`.

`STR StripPrefix(STR string, STR prefix)` — возвращает `string` с убранным `prefix`,
если `string` не начинается на `prefix`, возвращает `string`.

`STR StripSuffix(STR string, STR suffix)` — тоже самое, но с суффиксом.

`STR ClippedSubstr(STR s, size_t pos, size_t n = STR::npos)` — тоже самое, что и `s.substr(pos,
n)`, но если `n` больше `s.size()`, то возвращается `s`.

`STR StripAsciiWhitespace(STR)` — `strip` строки, удаляем все символы с обоих концов
вида [isspace](https://en.cppreference.com/w/cpp/string/byte/isspace).

`std::vector<STR> StrSplit(STR text, STR delim)` — делаем `split` строки по `delim`. Подумайте,
прежде чем копипастить из уже имеющейся задачи. Обойдитесь одной аллокацией памяти.

`STR ReadN(STR filename, int n)` — открывает файл и читает `n` байт из filename. Используйте
Linux Syscalls `open`, `read`, `close`. Если открыть или прочитать файл нельзя, возвращает
пустую строчку.

`STR AddSlash(STR path)` — добавляет к `path` файловой системы символ `/`, если его не было.

`STR RemoveSlash(STR path)` — убирает `/` из `path`, если это не сам путь `/` и путь
заканчивается на `/`.

`STR Dirname(STR path)` — известно, что `path` — корректный путь до файла без слеша на конце,
верните папку, в которой этот файл лежит без слеша на конце, если это не корень.

`STR Basename(STR path)` — известно, что `path` — корректный путь до файла, верните его
название.

`STR CollapseSlashes(STR path)` — известно, что `path` — корректный путь, но `/` могут
повторяться, надо убрать все повторения.

`STR StrJoin(const std::vector<STR>& strings, STR delimiter)` — склеить все строки в одну через
`delimiter`. Обойдитесь одной аллокацией памяти.

`STR StrCat(Args...)` — склеить все аргументы в один в их строковом представлении.
Должны поддерживаться числа (`int, long, long long` и их `unsigned` версии), также все строковые
типы (`std::string, std::string_view, const char*`). Аргументов в `StrCat` не больше пяти.
Придумайте как это сделать за одну аллокацию памяти.
*/

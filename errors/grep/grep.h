#pragma once

#include <cstddef>
#include <filesystem>
#include <iterator>
#include <optional>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include "utf8.h"
#include "utf8/checked.h"

using std::optional;

struct GrepOptions {
    optional<size_t> look_ahead_length;
    size_t max_matches_per_line;

    GrepOptions() {
        max_matches_per_line = 10;
    }

    GrepOptions(size_t look_ahead_length) : GrepOptions() {
        this->look_ahead_length = look_ahead_length;
    }

    GrepOptions(optional<size_t> look_ahead_length, size_t max_matches_per_line) {
        this->look_ahead_length = look_ahead_length;
        this->max_matches_per_line = max_matches_per_line;
    }
};

bool IsValidUtf8(const std::filesystem::path& file) {
    std::ifstream opened_file(file);
    if (!opened_file) {
        return false;
    }

    std::istreambuf_iterator<char> it(opened_file.rdbuf());
    std::istreambuf_iterator<char> eos;

    return utf8::is_valid(it, eos);
}

template <class Visitor>
void FileGrep(const std::filesystem::path& file, const std::string& pattern, Visitor visitor,
              const GrepOptions& options) {
    std::ifstream opened_file(file);
    if (!opened_file) {
        return;
    }

    if (!IsValidUtf8(file)) {
        visitor.OnError("Invalid utf-8!");
        return;
    }

    size_t line_idx = 0;
    std::string line;
    while (getline(opened_file, line)) {
        auto begin = line.begin();
        auto end = line.end();

        auto find_next = [&pattern, end](std::string::iterator it) {
            return std::search(it, end, std::boyer_moore_searcher(pattern.begin(), pattern.end()));
        };

        size_t matches = 0;
        for (auto it = find_next(begin); it != end; it = find_next(std::next(it))) {
            if (++matches > options.max_matches_per_line) {
                break;
            }
            std::optional<std::string> context;
            if (options.look_ahead_length) {
                auto context_start = it;
                auto pattern_size = utf8::distance(pattern.begin(), pattern.end());
                try {
                    utf8::advance(context_start, pattern_size, end);
                } catch (const utf8::not_enough_room&) {
                    context_start = end;
                }
                auto context_end = context_start;
                try {
                    utf8::advance(context_end, *options.look_ahead_length, end);
                } catch (const utf8::not_enough_room&) {
                    context_end = end;
                }

                context = std::string(context_start, context_end);
            }

            visitor.OnMatch(file, line_idx + 1, utf8::distance(begin, it) + 1, context);
        }

        line_idx++;
    }

    opened_file.close();
}

template <class Visitor>
void Grep(const std::string& path, const std::string& pattern, Visitor visitor,
          const GrepOptions& options) {
    if (std::filesystem::is_directory(path)) {
        for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(path)) {
            if (!std::filesystem::is_regular_file(dir_entry)) {
                continue;
            }
            FileGrep(dir_entry, pattern, visitor, options);
        }
    }
    if (std::filesystem::is_regular_file(path)) {
        FileGrep(path, pattern, visitor, options);
    }
}

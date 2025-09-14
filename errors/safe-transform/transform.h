#pragma once

#include <cstddef>
#include <list>
template <class Iterator, class UnaryPred, class UnaryOp>
void TransformIf(Iterator begin, Iterator end, UnaryPred p, UnaryOp f) {
    using T = Iterator::value_type;
    struct LogElement {
        Iterator it;
        T value;
    };
    std::list<LogElement> logs;
    bool copy_exception = false;

    auto restore = [&logs, &copy_exception]() {
        for (const auto& log : logs) {
            if (copy_exception == true) {
                return;
            }
            try {
                *log.it = log.value;
            } catch (...) {
                copy_exception = true;
            }
        }
    };

    for (auto it = begin; it != end; it = std::next(it)) {
        bool need_change = false;
        try {
            need_change = p(*it);
        } catch (...) {
            restore();
            throw;
        }
        if (need_change) {
            if (!copy_exception) {
                try {
                    logs.emplace_back(it, *it);
                } catch (...) {
                    copy_exception = true;
                }
            }
            try {
                f(*it);
            } catch (...) {
                restore();
                throw;
            }
        }
    }
}
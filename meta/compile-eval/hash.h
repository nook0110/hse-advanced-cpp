#pragma once

#include <cstdint>
constexpr int hash(const char *s, int p, int mod) {  // NOLINT
    return (*s == '\0' ? 0 : ((static_cast<int64_t>(hash(s + 1, p, mod)) * p) % mod + *s) % mod);
}

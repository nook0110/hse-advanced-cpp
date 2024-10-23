#pragma once
constexpr bool is_prime(int x) {  // NOLINT
    if (x == 1) {
        return false;
    }
    bool ans = true;
    for (int i = 2; i <= x / i; ++i) {
        if (x % i == 0) {
            ans = false;
        }
    }
    return ans;
}

constexpr int next_prime(int x) {  // NOLINT
    return is_prime(x) ? x : next_prime(x + 1);
}

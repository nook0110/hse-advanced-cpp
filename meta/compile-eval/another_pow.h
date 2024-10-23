#pragma once

constexpr int pow(int a, int b) {  // NOLINT
    return b == 0 ? 1 : (pow(a, b / 2) * pow(a, b / 2) * (b % 2 ? a : 1));
}

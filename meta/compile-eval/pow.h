#pragma once

template <unsigned a, unsigned b>
struct Pow {
    static const unsigned value =                                       // NOLINT
        Pow<a, b / 2>::value * Pow<a, b / 2>::value * (b % 2 ? a : 1);  // NOLINT
};

template <unsigned a>
struct Pow<a, 0> {
    static const unsigned value = 1;  // NOLINT
};

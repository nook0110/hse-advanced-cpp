
#include <cmath>
template <bool B, class T, class F>
struct Conditional {
    static const int value = T::value;  // NOLINT
};

template <class T, class F>
struct Conditional<false, T, F> {
    static const int value = F::value;  // NOLINT
};

template <int N, int L = 0, int R = N + 1, int M = (L + R) / 2>
struct Sqrt {
    static const int value =  // NOLINT
        Conditional<(M >= (N + M - 1) / M), Sqrt<N, L, M>, Sqrt<N, M, R> >::value;
};

template <int N, int ANS>
struct Sqrt<N, ANS, ANS + 1> {
    static const int value = ANS + 1;  // NOLINT
};

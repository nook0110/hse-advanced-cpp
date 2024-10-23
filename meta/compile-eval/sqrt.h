
#include <cmath>
template <bool B, class T, int ANS>
struct Conditional {
    static const int value = T::value;  // NOLINT
};

template <class T, int ANS>
struct Conditional<false, T, ANS> {
    static const int value = ANS;  // NOLINT
};

template <int N, int ANS = static_cast<int>(sqrt(N) + 1)>
struct Sqrt {
    static const int value = Conditional<(ANS * ANS >= N), Sqrt<N, ANS - 1>, ANS + 1>::value;  // NOLINT
};

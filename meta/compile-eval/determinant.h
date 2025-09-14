#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>

template <std::size_t N>
constexpr std::array<std::array<int, N - 1>, N - 1> GetSubmatrix(
    const std::array<std::array<int, N>, N>& matrix, size_t remove_j) {
    std::array<std::array<int, N - 1>, N - 1> submatrix = {};
    for (size_t i = 1; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            if (j < remove_j) {
                (&std::get<0>(((&std::get<0>(submatrix))[i - 1])))[j] = matrix[i][j];
            } else if (j > remove_j) {
                (&std::get<0>(((&std::get<0>(submatrix))[i - 1])))[j - 1] = matrix[i][j];
            }
        }
    }
    return submatrix;
}

template <std::size_t N>
constexpr int determinant(const std::array<std::array<int, N>, N>& matrix) {  // NOLINT
    auto det = 0;
    for (size_t col = 0; col < N; ++col) {
        std::array<std::array<int, N - 1>, N - 1> submatrix = GetSubmatrix(matrix, col);
        auto subdeterminant = determinant(submatrix);
        det += (col % 2 == 0 ? 1 : -1) * matrix[0][col] * subdeterminant;
    }
    return det;
}

template <>
constexpr int determinant<1>(const std::array<std::array<int, 1>, 1>& matrix) {  // NOLINT
    return matrix[0][0];
}

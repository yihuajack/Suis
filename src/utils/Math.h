//
// Created by Yihua Liu on 2024/03/28.
//

#ifndef UTILS_MATH_H
#define UTILS_MATH_H

#include <complex>
#include <limits>
#include <valarray>
#include <variant>
#include <vector>

namespace Utils::Math {
    constexpr float TOL = 100;

    // Python sys.float_info.epsilon is DBL_EPSILON
    // For C++, epsilon can be false, 0, FLT_EPSILON, DBL_EPSILON, LDBL_EPSILON, or custom T()
    template<std::floating_point T>
    constexpr T EPSILON = std::numeric_limits<T>::epsilon();

    template<typename T1, typename T2>
    auto real_if_close(const std::valarray<T1> &a, T2 tol = TOL) -> std::variant<std::valarray<T1>, std::valarray<T2>>;

    template<typename T>
    auto real_if_close(const std::complex<T> &a, T tol = TOL) -> std::variant<T, std::complex<T>>;

    template<typename T, std::size_t N>
    auto linspace(T start, T stop) -> std::array<T, N>;

    template<typename T>
    auto linspace(T start, T stop, std::size_t num) -> std::vector<T>;

    template<typename T>
    auto linspace_va(T start, T stop, std::size_t num) -> std::valarray<T>;
}

#endif  // UTILS_MATH_H

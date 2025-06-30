//
// Created by Yihua Liu on 2024/03/28.
//

#ifndef UTILS_MATH_H
#define UTILS_MATH_H

#include <complex>
#include <limits>
#include <ranges>
#include <valarray>
#include <variant>
#include <vector>
#include <QList>

#include "Global.h"

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

    template<typename T, typename SZ_T>
    auto linspace(T start, T stop, SZ_T num) -> QList<T>;

    template<typename T>
    auto linspace_va(T start, T stop, std::size_t num) -> std::valarray<T>;

    template<typename T>
    auto linspace(T start, T stop, T step) -> std::vector<T>;

    // If you do not want to import a heap of headers of instances list QList, put the definition here.
    // Note that the parameter order is different from numpy.interp!
    template<FloatingList U, FloatingList V>
    auto interp1_linear(U &&x, U &&y, V &&xi) -> std::remove_reference_t<U> {
        if (x.size() not_eq y.size()) {
            throw std::invalid_argument("x and y must have the same length");
        }
        if (x.size() < 2) {
            throw std::invalid_argument("x and y must have at least two elements");
        }
        std::remove_reference_t<U> yi(static_cast<typename std::remove_reference_t<U>::value_type>(xi.size()));
        // const typename std::remove_reference_t<V>::value_type xi_val
#ifdef __cpp_lib_ranges_enumerate
        for (const auto [i, xi_val] : std::views::enumerate(xi)) {
#else
        for (auto i = 0; i < xi.size(); ++i) {
            auto xi_val = xi[i];
#endif
            if (xi_val <= x.front()) {
                yi[i] = y.front();
            } else if (xi_val >= x.back()) {
                yi[i] = y.back();
            } else {
                for (size_t j = 0; j < x.size() - 1; ++j) {
                    if (xi_val >= x[j] && xi_val <= x[j + 1]) {
                        yi[i] = std::lerp(y[j], y[j + 1], (xi_val - x[j]) / (x[j + 1] - x[j]));
                        break;
                    }
                }
            }
        }
        return yi;
    }
}

#endif  // UTILS_MATH_H

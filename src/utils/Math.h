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

    template<typename T>
    auto linspace_va(T start, T stop, std::size_t num) -> std::valarray<T>;

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
        for (const typename std::remove_reference_t<V>::value_type xi_val : xi) {
            if (xi_val <= x.front()) {
                yi.push_back(y.front());
            } else if (xi_val >= x.back()) {
                yi.push_back(y.back());
            } else {
                for (size_t i = 0; i < x.size() - 1; ++i) {
                    if (xi_val >= x[i] && xi_val <= x[i+1]) {
                        yi.push_back(std::lerp(y[i], y[i+1], (xi_val - x[i]) / (x[i+1] - x[i])));
                        break;
                    }
                }
            }
        }
        return yi;
    }
}

#endif  // UTILS_MATH_H

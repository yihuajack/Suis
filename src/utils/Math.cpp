//
// Created by Yihua on 2024/03/28.
//

#include "Math.h"
#include <algorithm>
#include <ranges>

/* numpy.real_if_close(a, tol=100)
 * If input is complex with all imaginary parts close to zero, return real parts.
 *
 * "Close to zero" is defined as `tol` * (machine epsilon of the type for `a`).
 *
 * Parameters
 * ----------
 * a : const std::valarray<std::complex<T1>> &
 *     Input array.
 * tol : T2
 *     Tolerance in machine epsilons for the complex part of the elements in the array.
 *     If the tolerance is <=1, then the absolute tolerance is used.
 *
 * Returns
 * -------
 * out : std::valarray<T2>
 *     If `a` is real, the type of `a` is used for the output.
 *     If `a` has complex elements, the returned type is float.
 */
template<typename T1, typename T2>
auto Utils::Math::real_if_close(const std::valarray<T1> &a, T2 tol) -> std::variant<std::valarray<T1>, std::valarray<T2>> {
    // std::is_same_v<T1, float> or std::is_same_v<T1, double> or std::is_same_v<T1, long double>
    // or any extended floating-point types (std::float16_t, std::float32_t, std::float64_t, std::float128_t,
    // or std::bfloat16_t)(since C++23), including any cv-qualified variants.
    if constexpr (not std::is_same_v<T1, std::complex<T2>> or not std::is_floating_point_v<T2>) {
        return a;
    }
    // whether tol should be T2 or float is a question
    if (tol > 1) {
        tol *= EPSILON<T2>;
    }
    // The parameters of the lambda expression (function) for std::valarray<T>::apply cannot be a reference
    // but can be consts if it is not modified.
    // Note that apply() has to be valarray<T> apply( T func(T) ) const;
    // or valarray<T> apply( T func(const T&) ) const;
    // First, you are not allowed to change the original valarray;
    // second, it is impossible to return a valarray that is not of type T
    // Besides, operands to '?:' cannot have different types.
    if (std::ranges::all_of(a, [&tol](const T1 &elem) {
        return std::abs(elem.imag()) < tol;
    })) {
        std::valarray<T2> real_part_array(a.size());
        std::ranges::transform(a, std::begin(real_part_array), [](const std::complex<T2> c_num) {
            return c_num.real();
        });
        return real_part_array;
    }
    return a;
}

// (C2765) Default function arguments cannot be specified in explicit specializations of function templates,
// member function templates, and member functions of class templates when the class is implicitly instantiated.
template auto Utils::Math::real_if_close(const std::valarray<std::complex<double>> &a,
                                         double tol) -> std::variant<std::valarray<std::complex<double>>, std::valarray<double>>;

// real_if_close for singleton
// The compiler takes responsibility to match only std::complex<T>template<typename T>
template<typename T>
auto Utils::Math::real_if_close(const std::complex<T> &a, T tol) -> std::variant<T, std::complex<T>> {
    if constexpr (not std::is_floating_point_v<T>) {
        return a;
    }
    if (tol > 1) {
        tol *= EPSILON<T>;
    }
    if (std::abs(a.imag()) < tol) {
        return a.real();
    }
    return a;
}

// We can write double tol = TOL for gcc but not for MSVC:
// error C2765: real_if_close: an explicit specialization of a function template cannot have any default arguments
template auto Utils::Math::real_if_close(const std::complex<double> &a,
                                         double tol) -> std::variant<double, std::complex<double>>;

template<typename T, std::size_t N>
auto Utils::Math::linspace(const T start, const T stop) -> std::array<T, N> {
    std::array<T, N> result{};
    T step = (stop - start) / static_cast<T>(N - 1);
    T current = start;
    std::ranges::generate(result, [&current, step]() {
        T value = current;
        current += step;
        return value;
    });
    return result;
}

template auto Utils::Math::linspace(const double start, const double stop) -> std::array<double, 357>;

template<typename T>
auto Utils::Math::linspace(const T start, const T stop, const std::size_t num) -> std::vector<T> {
    std::vector<T> result(num);
    T step = (stop - start) / static_cast<T>(num - 1);
    T current = start;
    std::ranges::generate(result, [&current, &step] /* -> T */ {  // (C2760)
        T value = current;
        current += step;
        return value;
    });
    return result;
}

template auto Utils::Math::linspace(double start, double stop, std::size_t num) -> std::vector<double>;

template<typename T>
auto Utils::Math::linspace_va(const T start, const T stop, const std::size_t num) -> std::valarray<T> {
    std::valarray<T> result(num);
    T step = (stop - start) / static_cast<T>(num - 1);
    T current = start;
    std::ranges::generate(result, [&current, &step] /* -> T */ {  // (C2760)
        T value = current;
        current += step;
        return value;
    });
    return result;
}

template auto Utils::Math::linspace_va(double start, double stop, std::size_t num) -> std::valarray<double>;

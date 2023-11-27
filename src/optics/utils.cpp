//
// Created by Yihua on 2023/11/12.
//

#include "tmm.h"
#include "utils.h"

ValueWarning::ValueWarning(const std::string &message) : std::runtime_error(message) {}

template auto complex_to_string_with_name(const std::complex<double> c, const std::string &name) -> std::string;

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
auto real_if_close(const std::valarray<T1> &a, T2 tol = TOL) -> std::variant<std::valarray<T1>, std::valarray<T2>> {
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
    if (std::all_of(std::begin(a), std::end(a), [&tol](const T1 &elem) {
        return std::abs(elem.imag()) < tol;
    })) {
        std::valarray<T2> real_part_array(a.size());
        std::transform(std::begin(a), std::end(a), std::begin(real_part_array), [](const std::complex<T2> c_num) {
            return c_num.real();
        });
        return real_part_array;
    }
    return a;
}

// real_if_close for singleton
// The compiler takes responsibility to match only std::complex<T>template<typename T>
template<typename T>
auto real_if_close(const std::complex<T> &a, T tol) -> std::variant<T, std::complex<T>> {
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

template auto real_if_close(const std::complex<double> &a,
                            double tol = TOL) -> std::variant<double, std::complex<double>>;

template<typename T, std::size_t N>
auto linspace(const T start, const T stop) -> std::array<T, N> {
    std::array<T, N> result{};
    T step = (stop - start) / static_cast<T>(N - 1);
    T current = start;
    std::generate(result.begin(), result.end(), [&current, step]() {
        T value = current;
        current += step;
        return value;
    });
    return result;
}

template auto linspace(const double start, const double stop) -> std::array<double, 357>;

template<typename T>
auto linspace(const T start, const T stop, const std::size_t num) -> std::vector<T> {
    std::vector<T> result(num);
    T step = (stop - start) / static_cast<T>(num - 1);
    T current = start;
    std::generate(result.begin(), result.end(), [&current, step]() {
        T value = current;
        current += step;
        return value;
    });
    return result;
}

template auto linspace(const double start, const double stop, const std::size_t num) -> std::vector<double>;

//
// Created by Yihua on 2023/11/12.
//

// GCC/Clang has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
#include <algorithm>
#include <iostream>
#include <vector>
#include "utils.h"

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
auto real_if_close(const std::valarray<T1> &a, T2 tol) -> std::variant<std::valarray<T1>, std::valarray<T2>> {
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

template auto real_if_close(const std::valarray<std::complex<double>> &a,
                            double tol = TOL) -> std::variant<std::valarray<std::complex<double>>, std::valarray<double>>;

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

// We can write double tol = TOL for gcc but not for MSVC:
// error C2765: real_if_close: an explicit specialization of a function template cannot have any default arguments
template auto real_if_close(const std::complex<double> &a,
                            double tol) -> std::variant<double, std::complex<double>>;

template<typename T, std::size_t N>
auto linspace(const T start, const T stop) -> std::array<T, N> {
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

template auto linspace(const double start, const double stop) -> std::array<double, 357>;

template<typename T>
auto linspace(const T start, const T stop, const std::size_t num) -> std::vector<T> {
    std::vector<T> result(num);
    T step = (stop - start) / static_cast<T>(num - 1);
    T current = start;
    std::ranges::generate(result, [&current, &step] -> T {
        T value = current;
        current += step;
        return value;
    });
    return result;
}

template auto linspace(double start, double stop, std::size_t num) -> std::vector<double>;

template<typename T>
auto linspace_va(const T start, const T stop, const std::size_t num) -> std::valarray<T> {
    std::valarray<T> result(num);
    T step = (stop - start) / static_cast<T>(num - 1);
    T current = start;
    std::ranges::generate(result, [&current, &step] -> T {
        T value = current;
        current += step;
        return value;
    });
    return result;
}

template auto linspace_va(double start, double stop, std::size_t num) -> std::valarray<double>;

void print_container(const std::ranges::common_range auto &container) {
    for (const auto &item: container) {
        std::cout << item << ' ';
    }
    std::cout << '\n';
}

template<TwoDContainer Container>
void print_spec2d_container(const Container &container) {
    for (const auto &row: container) {
        for (const auto &item : row) {
            std::cout << item << ' ';
        }
        std::cout << '\n';
    }
}

// __GNUG__ is equivalent to (__GNUC__ && __cplusplus),
// see https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// See https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c
// https://stackoverflow.com/questions/13777681/demangling-in-msvc
#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
auto demangle(const char *mangled_name) -> std::string {
    int status = 0;
    // Implicit conversion from char * to std::string.
    // If using char *, we need to manually free it, which is not RAII.
    std::string demangled_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
    if (status == 0) {
        std::string result(demangled_name);
        return result;
    }
    // Demangling failed, return the mangled name
    return mangled_name;
}
// Do not mix #if defined and #elifdef!
// Normally, `undname` command is only accessible under Developer Command Prompt/PowerShell for VS
#elif defined(_MSC_VER)
auto demangle(const char *mangled_name) -> std::string {
    // Assemble the command to run the 'undname' utility
    std::string command = "undname -n\"" + std::string(mangled_name) + "\"";
    // Open a pipe to the command
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        return mangled_name; // Return the mangled name if the command fails
    }
    // Read the demangled name from the pipe
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    // Close the pipe
    _pclose(pipe);
    // Remove newline characters from the result
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}
#else
auto demangle(const char *mangled_name) -> std::string {
    throw std::logic_error("Demangling only by gcc, clang, and msvc is supported.");
}
#endif

template<std::ranges::sized_range U>
auto rng2d_transpose(const U &old_rng, const std::size_t num_rows) -> U {
    U new_rng(old_rng.size());
    const std::size_t num_cols = old_rng.size() / num_rows;
    for (std::size_t i = 0; i < num_rows; i++) {
        for (std::size_t j = 0; j < num_cols; j++) {
            new_rng[j * num_rows + i] = old_rng[i * num_cols + j];
        }
    }
    return new_rng;
}

template auto rng2d_transpose(const std::valarray<std::complex<double>> &old_va,
                              std::size_t num_rows) -> std::valarray<std::complex<double>>;
template auto rng2d_transpose(const std::valarray<double> &old_va,
                              std::size_t num_rows) -> std::valarray<double>;
template auto rng2d_transpose(const std::valarray<std::size_t> &old_va,
                              std::size_t num_rows) -> std::valarray<std::size_t>;
template auto rng2d_transpose(const std::vector<double> &old_vec,
                              std::size_t num_rows) -> std::vector<double>;

template<std::ranges::sized_range U>
requires std::ranges::sized_range<std::ranges::range_value_t<U>> and (not std::ranges::range<typename std::ranges::range_value_t<U>::value_type>)
auto rng2l_transpose(const U &old_rng) -> U {
    const std::size_t num_rows = old_rng.size();  // new_rng[0].size()
    const std::size_t num_cols = old_rng[0].size();  // new_rng.size()
    U new_rng(num_cols);
    for (std::size_t i = 0; i < num_cols; i++) {
        new_rng[i] = typename U::value_type(num_rows);
    }
    for (std::size_t i = 0; i < num_rows; i++) {
        for (std::size_t j = 0; j < num_cols; j++) {
            new_rng[j][i] = old_rng[i][j];
        }
    }
    return new_rng;
}

template auto rng2l_transpose(const std::vector<std::valarray<double>> &old_vec) -> std::vector<std::valarray<double>>;
template auto rng2l_transpose(const std::valarray<std::valarray<double>> &old_vec) -> std::valarray<std::valarray<double>>;

template<std::ranges::sized_range U, typename T, std::size_t N>
requires std::is_same_v<std::ranges::range_value_t<U>, std::vector<std::array<T, N>>>
auto vva2_flatten(const U &vvan) -> std::vector<T> {
    // ranges::views::concat is in Range-v3 but still not in C++23, probably will be in C++26,
    // see https://github.com/cplusplus/papers/issues/1204
    // P2542 R0-R7 https://wg21.link/P2542R7 (https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2542r7.html)
    std::vector<T> flattened;  // (vvan.size() * vvan.front().size() * N)
    std::ranges::for_each(vvan, [&](const std::vector<std::array<T, N>> &vectorOfArrays) {
        std::ranges::for_each(vectorOfArrays, [&](const std::array<T, N> &array) {
#ifdef __cpp_lib_containers_ranges
            flattened.append_range(array);
#else
            flattened.insert(flattened.end(), array.begin(), array.end());
#endif
        });
    });
    return flattened;
}

template auto vva2_flatten<std::valarray<std::vector<std::array<std::complex<double>, 2>>>, std::complex<double>, 2>(
    const std::valarray<std::vector<std::array<std::complex<double>, 2>>> &vvan) -> std::vector<std::complex<double>>;

template<std::ranges::sized_range U, typename T>
requires std::is_same_v<std::ranges::range_value_t<U>, std::valarray<T>>
auto vv_flatten(const U &vv) -> std::vector<T> {
    std::vector<T> flattened;
    std::ranges::for_each(vv, [&](const std::valarray<T> &va) {
#ifdef __cpp_lib_containers_ranges
        flattened.append_range(va);
#else
        flattened.insert(flattened.end(), std::begin(va), std::end(va));
#endif
    });
    return flattened;
}

template auto vv_flatten(const std::valarray<std::valarray<double>> &vv) -> std::vector<double>;
template auto vv_flatten(const std::vector<std::valarray<double>> &vv) -> std::vector<double>;
template auto vv_flatten(const std::valarray<std::valarray<std::complex<double>>> &vv) -> std::vector<std::complex<double>>;

#ifdef __cpp_lib_generator
template<std::ranges::sized_range U>
std::generator<const typename inner_type<U>::type&> recursive_iterate(const U &nested_range) {
    for (const typename U::value_type &elem : nested_range) {
        if constexpr (std::ranges::sized_range<typename U::value_type>) {
            recursive_iterate(elem);
        } else {
            co_yield nested_range;
        }
    }
}
#endif

//
// Created by Yihua Liu on 2023/11/12.
//

// GCC/Clang has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
#include <algorithm>
#include <complex>
#include "Range.h"

template<std::ranges::sized_range U>
auto Utils::Range::rng2d_transpose(const U &old_rng, const std::size_t num_rows) -> U {
    U new_rng(old_rng.size());
    const std::size_t num_cols = old_rng.size() / num_rows;
    for (std::size_t i = 0; i < num_rows; i++) {
        for (std::size_t j = 0; j < num_cols; j++) {
            new_rng[j * num_rows + i] = old_rng[i * num_cols + j];
        }
    }
    return new_rng;
}

template auto Utils::Range::rng2d_transpose(const std::valarray<std::complex<double>> &old_va,
                              std::size_t num_rows) -> std::valarray<std::complex<double>>;
template auto Utils::Range::rng2d_transpose(const std::valarray<double> &old_va,
                              std::size_t num_rows) -> std::valarray<double>;
template auto Utils::Range::rng2d_transpose(const std::valarray<std::size_t> &old_va,
                              std::size_t num_rows) -> std::valarray<std::size_t>;
template auto Utils::Range::rng2d_transpose(const std::vector<double> &old_vec,
                              std::size_t num_rows) -> std::vector<double>;

template<std::ranges::sized_range U>
requires std::ranges::sized_range<std::ranges::range_value_t<U>> and (not std::ranges::range<typename std::ranges::range_value_t<U>::value_type>)
auto Utils::Range::rng2l_transpose(const U &old_rng) -> U {
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

template auto Utils::Range::rng2l_transpose(const std::vector<std::valarray<double>> &old_vec) -> std::vector<std::valarray<double>>;
template auto Utils::Range::rng2l_transpose(const std::valarray<std::valarray<double>> &old_vec) -> std::valarray<std::valarray<double>>;

template<std::ranges::sized_range U, typename T, std::size_t N>
requires std::is_same_v<std::ranges::range_value_t<U>, std::vector<std::array<T, N>>>
auto Utils::Range::vva2_flatten(const U &vvan) -> std::vector<T> {
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

template auto Utils::Range::vva2_flatten<std::valarray<std::vector<std::array<std::complex<double>, 2>>>, std::complex<double>, 2>(
    const std::valarray<std::vector<std::array<std::complex<double>, 2>>> &vvan) -> std::vector<std::complex<double>>;

template<std::ranges::sized_range U, typename T>
requires std::is_same_v<std::ranges::range_value_t<U>, std::valarray<T>>
auto Utils::Range::vv_flatten(const U &vv) -> std::vector<T> {
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

template auto Utils::Range::vv_flatten(const std::valarray<std::valarray<double>> &vv) -> std::vector<double>;
template auto Utils::Range::vv_flatten(const std::vector<std::valarray<double>> &vv) -> std::vector<double>;
template auto Utils::Range::vv_flatten(const std::valarray<std::valarray<std::complex<double>>> &vv) -> std::vector<std::complex<double>>;

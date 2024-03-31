//
// Created by Yihua Liu on 2023/11/26.
//

#ifndef UTILS_RANGE_H
#define UTILS_RANGE_H

#include <ranges>
#include <valarray>
#include <vector>

// size_t for C "stddef.h"; std::size_t for C++ <cstddef>
// For local development environment
// size_t for "corecrt.h"; std::size_t for "c++config.h"
// __MINGW_EXTENSION typedef unsigned __int64 size_t; ifdef _WIN64
// typedef __SIZE_TYPE__ 	size_t; in namespace std
// https://stackoverflow.com/questions/42797279/what-to-do-with-size-t-vs-stdsize-t
// using std::size_t;

namespace Utils::Range {
    template<std::ranges::sized_range U>
    auto rng2d_transpose(const U &old_va, std::size_t num_rows) -> U;

    template<std::ranges::sized_range U>
    requires std::ranges::sized_range<std::ranges::range_value_t<U>> and (not std::ranges::range<typename std::ranges::range_value_t<U>::value_type>)
    auto rng2l_transpose(const U &old_rng) -> U;

    template<std::ranges::sized_range U, typename T, std::size_t N>
    requires std::is_same_v<std::ranges::range_value_t<U>, std::vector<std::array<T, N>>>
    auto vva2_flatten(const U &vvan) -> std::vector<T>;

    template<std::ranges::sized_range U, typename T>
    requires std::is_same_v<std::ranges::range_value_t<U>, std::valarray<T>>
    auto vv_flatten(const U &vv) -> std::vector<T>;

// Auxiliary template structs to obtain the innermost range using std::void_t
// https://stackoverflow.com/questions/59503567/how-can-i-deduce-the-inner-type-of-a-nested-stdvector-at-compile-time
// It seems that metaprogramming libraries may achieve this more intuitively like boost::mp11 or newer boost::hana,
// but I have no time to play with them.
// Note that std::complex also has member type value_type = T!
    template<class T, typename = void>
    struct inner_type {
        using type = T;
    };

    template<class T>
    struct inner_type<T, std::void_t<typename T::value_type::value_type>>
            : inner_type<typename T::value_type> {
    };

#ifdef __cpp_lib_generator
#include <generator>

    template<std::ranges::sized_range U>
    std::generator<const typename inner_type<U>::type&> recursive_iterate(const U &nested_range);
#endif
}

#endif  // UTILS_RANGE_H

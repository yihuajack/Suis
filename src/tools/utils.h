//
// Created by Yihua on 2023/11/26.
//

#ifndef UTILS_H
#define UTILS_H

#include <complex>
#include <concepts>
#include <iostream>
#include <limits>
#include <ranges>
#include <type_traits>
#include <valarray>
#include <variant>

// https://stackoverflow.com/questions/72702736/is-there-a-better-way-to-generic-print-containers-in-c20-c23
#ifdef __cpp_lib_print
#include <print>
#endif

constexpr float TOL = 100;

// Python sys.float_info.epsilon is DBL_EPSILON
// For C++, epsilon can be false, 0, FLT_EPSILON, DBL_EPSILON, LDBL_EPSILON, or custom T()
template<std::floating_point T>
constexpr T EPSILON=std::numeric_limits<T>::epsilon();

// size_t for C "stddef.h"; std::size_t for C++ <cstddef>
// For local development environment
// size_t for "corecrt.h"; std::size_t for "c++config.h"
// __MINGW_EXTENSION typedef unsigned __int64 size_t; ifdef _WIN64
// typedef __SIZE_TYPE__ 	size_t; in namespace std
// https://stackoverflow.com/questions/42797279/what-to-do-with-size-t-vs-stdsize-t
// using std::size_t;

template<typename T>
concept Scalar2DContainer = requires (T container) {
    // Check if it's a container
    requires std::ranges::range<T>;

    // Check if it has nested containers (rows)
    requires std::ranges::range<decltype(*std::begin(container))>;

    // Check if the nested containers have elements (columns)
    requires std::ranges::sized_range<decltype(*std::begin(*std::begin(container)))>;
};

template<typename T>
concept IsComplex = std::is_same_v<T, std::complex<typename T::value_type>>;

template<typename T>
concept Complex1DContainer = requires (T container) {
    // Check if it's a container
    requires std::ranges::range<T>;

    // Check if the nested containers have elements of type std::complex
    requires IsComplex<typename std::iterator_traits<decltype(std::begin(container))>::value_type>;

};

// template <typename T>
// concept HasComplexElements = requires (T element) {
//     // Check if the element type has real() and imag() member functions
//     { element.real() } -> std::convertible_to<double>;
//     { element.imag() } -> std::convertible_to<double>;
// };
// requires HasComplexElements<typename std::remove_reference_t<decltype(*std::begin(*std::begin(container)))>>;

template<typename T>
concept Complex2DContainer = requires(T container) {
    requires std::ranges::range<T>;  // T is equal to decltype<container>

    requires Complex1DContainer<typename std::iterator_traits<decltype(container.begin())>::value_type>;
};

template<typename T>
concept TwoDContainer = Scalar2DContainer<T> || Complex2DContainer<T>;

// Inline methods are supposed to be implemented in the header file
// https://stackoverflow.com/questions/1421666/qt-creator-inline-function-used-but-never-defined-why
template<typename T>
inline auto complex_to_string_with_name(const std::complex<T> c, const std::string &name) -> std::string {
    // QDebug cannot overload << and >> for std::complex
    std::ostringstream ss;
    ss << name << ": " << c.real() << " + " << c.imag() << "i";
    return ss.str();
}

template<typename T>
auto real_if_close(const std::complex<T> &a, T tol = TOL) -> std::variant<T, std::complex<T>>;

template<typename T, std::size_t N>
auto linspace(T start, T stop) -> std::array<T, N>;

template<typename T>
auto linspace(T start, T stop, std::size_t num) -> std::vector<T>;

// https://stackoverflow.com/questions/65096563/how-to-template-containers
// https://stackoverflow.com/questions/72792411/how-to-template-on-a-container-type
// https://stackoverflow.com/questions/7728478/c-template-class-function-with-arbitrary-container-type-how-to-define-it
// https://devblogs.microsoft.com/oldnewthing/20190619-00/?p=102599
void print_container(const std::ranges::common_range auto &container) {
    for (const auto &item: container) {
        std::cout << item << ' ';
    }
    std::cout << '\n';
}

// Cannot do by
// template<template<class...> class C, typename T>
// auto print_spec_container(const C<T> &container) -> std::enable_if_t<std::is_same_v<C<T>, std::vector<std::array<std::complex<T>, 2>>>>;
// but can be a generalized template<typename Container, typename T>
template<TwoDContainer Container>
void print_spec2d_container(const Container &container) {
    for (const auto &row: container) {
        for (const auto &item : row) {
            std::cout << item << ' ';
        }
        std::cout << '\n';
    }
};

auto demangle(const char* mangled_name) -> std::string;

template<typename T>
auto va_2d_transpose(const std::valarray<T> &old_va, std::size_t num_rows) -> std::valarray<T>;

template<typename T, std::size_t N>
auto vva2_flatten(const std::vector<std::vector<std::array<T, N>>> &vvan) -> std::vector<T>;

#endif //UTILS_H

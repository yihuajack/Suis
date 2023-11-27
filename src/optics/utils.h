//
// Created by Yihua on 2023/11/26.
//

#ifndef TMM_UTILS_H
#define TMM_UTILS_H

#include <complex>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <valarray>
#include <variant>

class ValueWarning : public std::runtime_error {
public:
    explicit ValueWarning(const std::string &message);
};

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
template<typename Container, typename T>
auto print_spec2d_container(const Container &container) -> std::enable_if_t<std::is_same_v<Container, std::vector<std::array<std::complex<T>, 2>>>> {
    for (const auto &row: container) {
        for (const auto &item : row) {
            std::cout << item << ' ';
        }
        std::cout << '\n';
    }
};

#endif //TMM_UTILS_H

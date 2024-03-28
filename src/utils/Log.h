//
// Created by Yihua on 2024/03/28.
//

#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <ranges>
#include <type_traits>

#include "../Global.h"

// https://stackoverflow.com/questions/72702736/is-there-a-better-way-to-generic-print-containers-in-c20-c23
#ifdef __cpp_lib_print
#include <print>
#endif

namespace Utils::Log {
    template<typename T>
    concept Scalar2DContainer = requires(T container) {
        // Check if it's a container
        requires std::ranges::range<T>;

        // Check if it has nested containers (rows)
        requires std::ranges::range<decltype(*std::begin(container))>;

        // Check if the nested containers have elements (columns)
        requires std::ranges::sized_range<decltype(*std::begin(*std::begin(container)))>;
    };

    template<typename T>
    concept Complex1DContainer = requires(T container) {
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

    // https://stackoverflow.com/questions/65096563/how-to-template-containers
    // https://stackoverflow.com/questions/72792411/how-to-template-on-a-container-type
    // https://stackoverflow.com/questions/7728478/c-template-class-function-with-arbitrary-container-type-how-to-define-it
    // https://devblogs.microsoft.com/oldnewthing/20190619-00/?p=102599
    void print_container(const std::ranges::common_range auto &container);

    // Cannot do by
    // template<template<class...> class C, typename T>
    // auto print_spec_container(const C<T> &container) -> std::enable_if_t<std::is_same_v<C<T>, std::vector<std::array<std::complex<T>, 2>>>>;
    // but can be a generalized template<typename Container, typename T>
    template<TwoDContainer Container>
    void print_spec2d_container(const Container &container);
}
#endif  // UTILS_LOG_H

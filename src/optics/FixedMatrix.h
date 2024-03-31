//
// Created by Yihua Liu on 2023/11/19.
//

#ifndef FIXEDMATRIX_H
#define FIXEDMATRIX_H

#include <array>
#include <cstddef>
#include <stdexcept>

// This matrix class is very coarse. Refer to Bjarne Stroustrup's "The C++ Programming Language" next time.
template<typename T, std::size_t N, std::size_t M>
class FixedMatrix {
private:
    std::array<std::array<T, M>, N> data;
public:
    FixedMatrix() = default;
    explicit FixedMatrix(T num);
    FixedMatrix(const FixedMatrix &other);
    FixedMatrix(FixedMatrix &&other) noexcept;
    explicit FixedMatrix(const std::array<std::array<T, M>, N> &data);
    explicit FixedMatrix(std::array<std::array<T, M>, N> data) noexcept;
    // Different from traditional C-style arrays that can be brace-initialized by {{1, 0}, {0, 1}}
    // 2D std::array has to be brace-initialized by {{{1, 0}, {0, 1}}} or {1, 0, 0, 1}
    // https://stackoverflow.com/questions/12844475/why-cant-simple-initialize-with-braces-2d-stdarray
    // Distinguish std::initializer_list<T> and list-initialization
    // Note that std::initializer_list<T> uses copy-semantics by holding its values as const objects, see
    // https://stackoverflow.com/questions/24109445/stdinitializer-list-and-reference-types
    // Since std::initializer_list may be implemented as a pair of pointers or pointer and length,
    // it is often passed by value, see
    // https://stackoverflow.com/questions/17803475/why-is-stdinitializer-list-often-passed-by-value
    // init_list and row must not be a reference; otherwise,
    // error: cannot bind non-const lvalue reference of type 'std::initializer_list<std::initializer_list<int> >&'
    // to an rvalue of type 'std::initializer_list<std::initializer_list<int> >'
    // error: binding reference of type 'std::initializer_list<int>&'
    // to 'const std::initializer_list<int>' discards qualifiers
    // Hence, we cannot distinguish the move and copy constructor or assignment operator
    // like std::copy(row.begin(), row.end(), data[i].begin());
    // A pitfall is that 2D std::array behaves like traditional 2D C-style array that when moving or copying an
    // initializer list or a vector to the array, the extra element will be added to the next sub-array.
    // Thus, it is encouraged to use the array<array> constructor,
    // as it will alert error: too many initializers for arrays like {0, 1, 2, 3, 4} or {{0, 1}, {2, 3, 4}}.
    // FixedMatrix<int, 2, 2> = {1, 0, 0, 1} will construct by the array<array> [MOVING] constructor;
    // others will construct by the initializer_list<initializer_list> constructor due to automatic brace elision.
    // See Item 7, Chapter 3 of Effective Modern C++
    // Also see https://youtrack.jetbrains.com/issue/CPP-36190/Class-constructor-parameter-used-but-marked-dark-and-unused
    FixedMatrix(std::initializer_list<std::initializer_list<T>> init_list);
    ~FixedMatrix() = default;
    // See Scott Meyer's More Effective C++ "Techniques, Idioms, Patterns" Item 30 Proxy classes
    class RowProxy {
    private:
        std::array<T, M> &row;
    public:
        explicit RowProxy(std::array<T, M> &row);
        auto operator[](std::size_t j) -> T &;
        auto operator[](std::size_t j) const -> const T &;

        constexpr auto begin() noexcept -> std::array<T, M>::iterator;
        constexpr auto begin() const noexcept -> std::array<T, M>::const_iterator;
        constexpr auto cbegin() const noexcept -> std::array<T, M>::const_iterator;
        constexpr auto end() noexcept -> std::array<T, M>::iterator;
        constexpr auto end() const noexcept -> std::array<T, M>::const_iterator;
        constexpr auto cend() const noexcept -> std::array<T, M>::const_iterator;
    };

    // Notice that the implementations of operator[] has already included bounds checking by at() functions.
    // For this operator[] functions and RowProxy's, they cannot be defined as constexpr; otherwise,
    // inline function used but never defined.
    auto operator[](std::size_t i) -> RowProxy;
    auto operator[](std::size_t i) const -> const RowProxy;
    auto operator/(const T &scalar) const -> FixedMatrix<T, N, M>;
    // In-place matrix multiplication is generally not supported.
    void operator/=(const T &scalar);
    auto operator=(T num) -> FixedMatrix&;
    auto operator=(const FixedMatrix &other) -> FixedMatrix&;
    auto operator=(FixedMatrix &&other) noexcept -> FixedMatrix&;
    auto operator=(const std::array<std::array<T, M>, N> &data_) -> FixedMatrix&;
    auto operator=(std::array<std::array<T, M>, N> &&data_) noexcept -> FixedMatrix&;
    auto operator=(std::initializer_list<std::initializer_list<T>> init_list) -> FixedMatrix&;

    constexpr auto begin() noexcept -> std::array<std::array<T, M>, N>::iterator;
    constexpr auto begin() const noexcept -> std::array<std::array<T, M>, N>::const_iterator;
    constexpr auto cbegin() const noexcept -> std::array<std::array<T, M>, N>::const_iterator;
    constexpr auto end() noexcept -> std::array<std::array<T, M>, N>::iterator;
    constexpr auto end() const noexcept -> std::array<std::array<T, M>, N>::const_iterator;
    constexpr auto cend() const noexcept -> std::array<std::array<T, M>, N>::const_iterator;

    // For function marked [[nodiscard]]:
    // https://clang.llvm.org/extra/clang-tidy/checks/modernize/use-nodiscard.html
    auto transpose() const -> FixedMatrix<T, M, N>;
    // template<std::size_t P>
    // auto dot(const FixedMatrix<T, N, M> &matrix1, const FixedMatrix<T, M, P> &matrix2) -> FixedMatrix<T, N, P>;
    auto squeeze() const -> std::array<T, M>;
};

// For friend functions or non-member functions
template<typename T, std::size_t N, std::size_t M, std::size_t P>
// For member functions:
// template<typename T, std::size_t N, std::size_t M>
// template<std::size_t P>
// For a friend version,
// see https://stackoverflow.com/questions/77541238/separating-template-friend-functions-declaration-and-definition
// https://stackoverflow.com/questions/3989678/c-template-friend-operator-overloading
// https://stackoverflow.com/questions/33861151/c-how-to-declare-a-function-template-friend-for-a-class-template
// https://stackoverflow.com/questions/12875679/how-to-explicitly-instantiate-a-template-class-that-has-a-nested-class-with-a-fr
// Probably it is more practical to move the definition of template friend function into here
// or alternatively wrap it with a template-less struct in a namespace with a forward declaration,
// or including the definition in the header file with a forward declaration.
auto dot(const FixedMatrix<T, N, M> &matrix1, const FixedMatrix<T, M, P> &matrix2) -> FixedMatrix<T, N, P>;

template<typename T, std::size_t N, std::size_t M, std::size_t P>
auto operator*(const FixedMatrix<T, N, M> &mat1, const FixedMatrix<T, M, P> &mat2) -> FixedMatrix<T, N, P>;

#endif //FIXEDMATRIX_H

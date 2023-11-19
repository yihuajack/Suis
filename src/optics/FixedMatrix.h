//
// Created by Yihua on 2023/11/19.
//

#ifndef FIXEDMATRIX_H
#define FIXEDMATRIX_H

#include <array>
#include <cstddef>
#include <stdexcept>

template<typename T, std::size_t N, std::size_t M>
class FixedMatrix {
private:
    std::array<std::array<T, M>, N> data;
public:
    FixedMatrix();
    explicit FixedMatrix(T num);
    FixedMatrix(const FixedMatrix &other);
    FixedMatrix(FixedMatrix &&other) noexcept;
    explicit FixedMatrix(const std::array<std::array<T, M>, N> &data);
    explicit FixedMatrix(std::array<std::array<T, M>, N> &&data) noexcept;
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
    FixedMatrix(std::initializer_list<std::initializer_list<T>> init_list);
    ~FixedMatrix() = default;

    class RowProxy {
    private:
        std::array<T, M> &row;
    public:
        explicit RowProxy(std::array<T, M> &row);
        auto operator[](std::size_t j) -> T &;
    };

    auto operator[](std::size_t i) -> RowProxy;
    auto operator/(const T &scalar) const -> FixedMatrix<T, N, M>;
    // In-place matrix multiplication is generally not supported.
    void operator/=(const T &scalar);
    auto operator=(T num) -> FixedMatrix&;
    auto operator=(const FixedMatrix &other) -> FixedMatrix&;
    auto operator=(FixedMatrix &&other) noexcept -> FixedMatrix&;
    auto operator=(const std::array<std::array<T, M>, N> &data_) -> FixedMatrix&;
    auto operator=(std::array<std::array<T, M>, N> &&data_) noexcept -> FixedMatrix&;
    auto operator=([[maybe_unused]] std::initializer_list<std::initializer_list<T>> init_list) -> FixedMatrix&;

    // For function marked [[nodiscard]]:
    // https://clang.llvm.org/extra/clang-tidy/checks/modernize/use-nodiscard.html
    auto transpose() const -> FixedMatrix<T, M, N>;
    template<std::size_t P>
    friend auto dot(const FixedMatrix<T, N, M> &matrix1, const FixedMatrix<T, M, P> &matrix2) -> FixedMatrix<T, N, P>;
    auto squeeze() const -> std::array<T, M>;
};

template<typename T, std::size_t N, std::size_t M, std::size_t P>
auto operator*(const FixedMatrix<T, N, M> &mat1, const FixedMatrix<T, M, P> &mat2) -> FixedMatrix<T, N, P>;

#endif //FIXEDMATRIX_H

//
// Created by Yihua on 2023/11/19.
//

#include <algorithm>
#include <complex>
#include "FixedMatrix.h"

// const-qualification of parameters only has an effect in function definitions
// num must be a reference; otherwise,
// Reference to type 'const std::array<std::array<double, 2>, 2>::value_type' (aka 'const std::array<double, 2>') could
// not bind to an lvalue of type 'double'
template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(const T num) {
    for (std::array<T, M> &row : data) {
        row.fill(num);
    }
}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(const FixedMatrix &other) : data(other.data) {}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(FixedMatrix &&other) noexcept : data(std::move(other.data)) {}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(const std::array<std::array<T, M>, N> &data) : data(data) {}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(std::array<std::array<T, M>, N> data) noexcept : data(std::move(data)) {}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(std::initializer_list<std::initializer_list<T>> init_list) {
    std::size_t i = 0;
    for (std::initializer_list<T> row : init_list) {
        std::move(row.begin(), row.end(), data.at(i).begin());
        ++i;
    }
}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::RowProxy::RowProxy(std::array<T, M> &row) : row(row) {}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::RowProxy::operator[](std::size_t j) -> T & {
    return row.at(j);
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::RowProxy::operator[](std::size_t j) const -> const T & {
    return row.at(j);
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::RowProxy::begin() noexcept -> std::array<T, M>::iterator {
    return row.begin();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::RowProxy::begin() const noexcept -> std::array<T, M>::const_iterator {
    return row.begin();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::RowProxy::cbegin() const noexcept -> std::array<T, M>::const_iterator {
    return row.cbegin();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::RowProxy::end() noexcept -> std::array<T, M>::iterator {
    return row.end();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::RowProxy::end() const noexcept -> std::array<T, M>::const_iterator {
    return row.end();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::RowProxy::cend() const noexcept -> std::array<T, M>::const_iterator {
    return row.cend();
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator[](std::size_t i) -> FixedMatrix::RowProxy {
    return RowProxy(data.at(i));
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator[](std::size_t i) const -> const FixedMatrix::RowProxy {
    return RowProxy(const_cast<std::array<T, M>&>(data.at(i)));
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator/(const T &scalar) const -> FixedMatrix<T, N, M> {
    FixedMatrix<T, N, M> result{};
    for (std::size_t i = 0; i < N; i++) {
        std::transform(data.at(i).begin(), data.at(i).end(), result[i].begin(), [scalar](const T &element) {
            return element / scalar;
        });
    }
    return result;
}

template<typename T, std::size_t N, std::size_t M>
void FixedMatrix<T, N, M>::operator/=(const T &scalar) {
    for (std::array<T, M> &row : data) {
        std::transform(row.begin(), row.end(), row.begin(), [scalar](T &element) {
            return element / scalar;
        });
    }
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator=(const T num) -> FixedMatrix & {
    for (std::array<T, M> &row : data) {
        row.fill(num);
    }
    return *this;
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator=(const FixedMatrix &other) -> FixedMatrix & {
    if (this != &other) {
        data = other.data;
    }
    return *this;
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator=(FixedMatrix &&other) noexcept -> FixedMatrix & {
    if (this != &other) {
        data = std::move(other.data);
    }
    return *this;
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator=(const std::array<std::array<T, M>, N> &data_) -> FixedMatrix & {
    if (&data != &data_) {
        data = data_;
    }
    return *this;
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator=(std::array<std::array<T, M>, N> &&data_) noexcept -> FixedMatrix & {
    if (&data != &data_) {
        data = std::move(data_);
    }
    return *this;
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator=(std::initializer_list<std::initializer_list<T>> init_list) -> FixedMatrix & {
    std::size_t i = 0;
    for (std::initializer_list<T> row : init_list) {
        std::move(row.begin(), row.end(), data.at(i).begin());
        ++i;
    }
    return *this;
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::begin() noexcept -> std::array<std::array<T, M>, N>::iterator {
    return data.begin();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::begin() const noexcept -> std::array<std::array<T, M>, N>::const_iterator {
    return data.begin();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::cbegin() const noexcept -> std::array<std::array<T, M>, N>::const_iterator {
    return data.cbegin();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::end() noexcept -> std::array<std::array<T, M>, N>::iterator {
    return data.end();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::end() const noexcept -> std::array<std::array<T, M>, N>::const_iterator {
    return data.end();
}

template<typename T, std::size_t N, std::size_t M>
constexpr auto FixedMatrix<T, N, M>::cend() const noexcept -> std::array<std::array<T, M>, N>::const_iterator {
    return data.cend();
}

template<typename T, std::size_t N, std::size_t M>
[[nodiscard]] auto FixedMatrix<T, N, M>::transpose() const -> FixedMatrix<T, M, N> {
    FixedMatrix<T, M, N> result{};
    for (std::size_t i = 0; i < N; i++) {
        for (std::size_t j = 0; j < M; j++) {
            result[j][i] = (*this)[i][j];
        }
    }
    return result;
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::squeeze() const -> std::array<T, M> {
    if (N == 1) {
        return data[0];
    }
    throw std::logic_error("FixedMatrix cannot be squeezed");
}

template<typename T, std::size_t N, std::size_t M, std::size_t P>
auto dot(const FixedMatrix<T, N, M> &matrix1, const FixedMatrix<T, M, P> &matrix2) -> FixedMatrix<T, N, P> {
    FixedMatrix<T, N, P> result;
    for (std::size_t i = 0; i < N; i++) {
        for (std::size_t j = 0; j < P; j++) {
            for (std::size_t k = 0; k < M; k++) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
    return result;
}
template<typename T, std::size_t N, std::size_t M, std::size_t P>
auto operator*(const FixedMatrix<T, N, M> &mat1, const FixedMatrix<T, M, P> &mat2) -> FixedMatrix<T, N, P> {
    return dot<P>(mat1, mat2);
}

template class FixedMatrix<std::complex<double>, 1, 2>;
template class FixedMatrix<std::complex<double>, 2, 1>;
template class FixedMatrix<std::complex<double>, 2, 2>;

template auto dot(const FixedMatrix<std::complex<double>, 2, 2> &matrix1,
                  const FixedMatrix<std::complex<double>, 2, 2> &matrix2) -> FixedMatrix<std::complex<double>, 2, 2>;
template auto dot(const FixedMatrix<std::complex<double>, 2, 2> &matrix1,
                  const FixedMatrix<std::complex<double>, 2, 1> &matrix2) -> FixedMatrix<std::complex<double>, 2, 1>;

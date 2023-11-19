//
// Created by Yihua on 2023/11/19.
//

#include <algorithm>
#include "FixedMatrix.h"

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix() : data(N, std::array<T, M>()) {}

// const-qualification of parameters only has an effect in function definitions
template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(const T num) {
    data.fill(num);
}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(const FixedMatrix &other) : data(other.data) {}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(FixedMatrix &&other) noexcept : data(std::move(other.data)) {}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(const std::array<std::array<T, M>, N> &data) : data(data) {}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(std::array<std::array<T, M>, N> &&data) noexcept : data(std::move(data)) {}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::FixedMatrix(std::initializer_list<std::initializer_list<T>> init_list) {
    std::size_t i = 0;
    for (std::initializer_list<T> row : init_list) {
        std::move(row.begin(), row.end(), data[i].begin());
        ++i;
    }
}

template<typename T, std::size_t N, std::size_t M>
FixedMatrix<T, N, M>::RowProxy::RowProxy(std::array<T, M> &row) : row(row) {}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::RowProxy::operator[](std::size_t j) -> T & {
    return row[j];
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator[](std::size_t i) -> FixedMatrix::RowProxy {
    return RowProxy(data[i]);
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator/(const T &scalar) const -> FixedMatrix<T, N, M> {
    FixedMatrix<T, N, M> result;
    for (std::size_t i = 0; i < N; i++) {
        std::transform(data[i].begin(), data[i].end(), result[i].begin(), [scalar](const T &element) {
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
    data.fill(num);
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
    if (data != &data_) {
        data = data_;
    }
    return *this;
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator=(std::array<std::array<T, M>, N> &&data_) noexcept -> FixedMatrix & {
    if (data != &data_) {
        data = std::move(data_);
    }
    return *this;
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::operator=(std::initializer_list<std::initializer_list<T>> init_list) -> FixedMatrix & {
    std::size_t i = 0;
    for (std::initializer_list<T> row : init_list) {
        std::move(row.begin(), row.end(), data[i].begin());
        ++i;
    }
}

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::transpose() const -> FixedMatrix<T, M, N> {
    FixedMatrix<T, M, N> result;
    for (std::size_t i = 0; i < N; i++) {
        for (std::size_t j = 0; j < M; j++) {
            result[j][i] = (*this)[i][j];
        }
    }
    return result;
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

template<typename T, std::size_t N, std::size_t M>
auto FixedMatrix<T, N, M>::squeeze() const -> std::array<T, M> {
    if (N == 1) {
        return data[0];
    }
    throw std::logic_error("FixedMatrix cannot be squeezed");
}

template<typename T, std::size_t N, std::size_t M, std::size_t P>
auto operator*(const FixedMatrix<T, N, M> &mat1, const FixedMatrix<T, M, P> &mat2) -> FixedMatrix<T, N, P> {
    return dot<P>(mat1, mat2);
}

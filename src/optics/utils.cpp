//
// Created by Yihua on 2023/11/12.
//

#include <array>
#include <complex>
#include <stdexcept>
#include <vector>

template<typename T, size_t N, size_t M, size_t P>
auto dot2dArray(const std::array<std::array<std::complex<T>, M>, N> &matrix1,
                const std::array<std::array<std::complex<T>, P>, M> &matrix2) -> std::array<std::array<std::complex<T>, P>, N> {
    std::array<std::array<std::complex<T>, P>, N> result;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < P; j++) {
            for (size_t k = 0; k < M; k++) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
    return result;
}

template<typename T, size_t N, size_t M>
auto transpose2dArray(
        const std::array<std::array<std::complex<T>, M>, N> &array) -> std::array<std::array<std::complex<T>, N>, M> {
    std::array<std::array<std::complex<T>, N>, M> result;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            result[j][i] = array[i][j];
        }
    }
    return result;
}

template<typename T, size_t N>
auto transposeArrayOfVector(
        const std::vector<std::array<std::complex<T>, N>> &array) -> std::array<std::vector<std::complex<T>>, N> {
    std::array<std::vector<std::complex<T>>, N> result;
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < array.size(); j++) {
            result[j][i] = array[i][j];
        }
    }
    return result;
}

template<typename T, size_t N>
auto squeezeArray(const std::array<T, N> &array) -> T {
    if (std::is_array_v<T> and array.size() == 1) {
        return array[0];
    }
    throw std::logic_error("Array cannot be squeezed");
}

// Reference:
// https://stackoverflow.com/questions/53378000/how-to-access-to-subsequence-of-a-valarray-considering-it-as-a-2d-matrix-in-c
// Using std::slice/std::slice_array is not a good idea
template<typename T, size_t N, size_t M>
class ComplexMatrix {
private:
    std::valarray<T> data;
public:
    ComplexMatrix() : data(N * M) {}
    explicit ComplexMatrix(std::valarray<T> data) : data(data) {}
    ComplexMatrix(std::initializer_list<std::initializer_list<T>> &initList) {
        size_t i = 0;
        for (const auto &row : initList) {
            std::move(row.begin(), row.end(), &data[i * M]);
            ++i;
        }
    }

    class RowProxy {
    private:
        std::valarray<T> &elems;
        size_t row;
    public:
        explicit RowProxy(std::valarray<T>& elems, size_t row) : elems(elems), row(row) {}
        auto operator[](size_t j) -> T & {
            return elems[row * M + j];
        }
    };

    auto operator[](size_t i) -> RowProxy {
        return RowProxy(data, M, i);
    }
    auto operator/(const T& scalar) const -> ComplexMatrix<T, N, M> {
        ComplexMatrix<T, N, M> result;
        result.data = data / scalar;
        return result;
    }

    auto squeeze() const -> std::valarray<std::complex<T>> {
        if (N == 1) {
            return data;
        }
        throw std::logic_error("ComplexMatrix cannot be squeezed");
    }
};

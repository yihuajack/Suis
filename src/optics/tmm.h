#ifndef TMM_H
#define TMM_H

#include <array>
#include <complex>
#include <limits>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>

constexpr float TOL=100;
// Python sys.float_info.epsilon is DBL_EPSILON
// For C++, epsilon can be false, 0, FLT_EPSILON, DBL_EPSILON, LDBL_EPSILON, or custom T()
template<typename T>
constexpr T EPSILON=std::numeric_limits<T>::epsilon();

// size_t for C "stddef.h"; std::size_t for C++ <cstddef>
// For local development environment
// size_t for "corecrt.h"; stdd::size_t for "c++config.h"
// __MINGW_EXTENSION typedef unsigned __int64 size_t; ifdef _WIN64
// typedef __SIZE_TYPE__ 	size_t; in namespace std
// https://stackoverflow.com/questions/42797279/what-to-do-with-size-t-vs-stdsize-t
// using std::size_t;

template<typename T>
using coh_tmm_dict = std::unordered_map<std::string, std::variant<char, T, std::complex<T>, std::valarray<std::complex<T>>, std::vector<std::array<std::complex<T>, 2>>>>;
template<typename T>
using inc_group_layer_dict = std::unordered_map<std::string, std::variant<std::size_t, std::vector<std::size_t>, std::vector<long long int>, std::vector<std::vector<T>>, std::vector<std::vector<std::complex<T>>>, std::vector<std::vector<std::size_t>>>>;
template<typename T>
using inc_tmm_dict = std::unordered_map<std::string, std::variant<T, std::size_t, std::vector<std::size_t>, std::vector<long long int>, std::vector<T>, std::vector<std::vector<T>>, std::vector<std::vector<std::complex<T>>>, std::vector<std::vector<std::size_t>>, std::vector<std::array<T, 2>>, std::vector<coh_tmm_dict<T>>>>;

template<typename T>
coh_tmm_dict<T> coh_tmm(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                        std::complex<T> th_0, T lam_vac);

template<typename T>
std::unordered_map<std::string, T> ellips(std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                                          std::complex<T> th_0, T lam_vac);

#endif // TMM_H

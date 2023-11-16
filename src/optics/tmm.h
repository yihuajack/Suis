#ifndef TMM_H
#define TMM_H

#include <limits>

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

#endif // TMM_H

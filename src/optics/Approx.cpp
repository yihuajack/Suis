//
// Created by Yihua on 2023/11/28.
//

#include "Approx.h"

template<typename U, typename T>
ApproxBase<U, T>::ApproxBase(U expected, const T abs, const T rel, const bool nan_ok) : expected(expected), abs(abs), rel(rel), nan_ok(nan_ok) {}

template<typename U, typename T>
auto ApproxBase<U, T>::_approx_scalar(T x) -> ApproxScalar<T> {
    return ApproxScalar<T>();
}

template<typename U, typename T>
auto ApproxBase<U, T>::operator==(const ApproxBase &actual) const -> bool {
    return false;//std::all_of(expected.cbegin(), expected.cend(), );
}

template<typename T>
auto ApproxScalar<T>::operator==(const ApproxScalar &actual) const -> bool {
    if (actual == expected) {
        return true;
    }
    return false;
}

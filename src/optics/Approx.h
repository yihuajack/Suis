//
// Created by Yihua on 2023/11/28.
//

#ifndef APPROXBASE_H
#define APPROXBASE_H

// Forward declaration
template<typename T>
class ApproxScalar;

/*
 * Provide shared utilities for making approximate comparisons between
 * numbers or sequences of numbers.
 */
template<typename U, typename T>
class ApproxBase {
private:
    U expected;
    T abs;
    T rel;
    bool nan_ok{};
public:
    ApproxBase(U expected, T abs, T rel, bool nan_ok);
    // virtual ~ApproxBase() = default;

    virtual auto operator==(const ApproxBase &actual) const -> bool;
    void _yield_comaparisons(const ApproxBase &actual) = 0;
    auto ApproxBase<U, T>::_approx_scalar(T x) -> ApproxScalar<T>;
};

template<typename T>
class ApproxScalar : ApproxBase<T, T> {
private:
    static constexpr T DEFAULT_ABSOLUTE_TOLERANCE = 1e-12;
    static constexpr T DEFAULT_RELATIVE_TOLERANCE = 1e-6;
public:
    virtual auto operator==(const ApproxScalar &actual) const -> bool;
};

#endif //APPROXBASE_H

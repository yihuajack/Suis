//
// Created by Yihua on 2023/11/28.
//

#ifndef APPROX_H
#define APPROX_H

#include <any>
#include <cmath>
#include <complex>
#include <concepts>
#include <ranges>
#include <type_traits>

// https://stackoverflow.com/questions/30736951/templated-class-check-if-complex-at-compile-time
template<typename T>
concept FPScalar = std::floating_point<T> or std::is_same_v<T, std::complex<typename T::value_type>> and std::is_floating_point_v<typename T:: value_type>;

// Forward declaration for _approx_scalar()
template<FPScalar U, std::floating_point T>
class ApproxScalar;

/*
 * Provide shared utilities for making approximate comparisons between
 * numbers or sequences of numbers.
 */
template<typename U, std::floating_point T>
class ApproxBase {
protected:
    U expected;
    T abs;
    T rel;
    bool nan_ok{};
public:
    ApproxBase(U expected, T abs, T rel, bool nan_ok);
    ApproxBase(const ApproxBase &other);
    ApproxBase(ApproxBase &&other) noexcept;
    virtual ~ApproxBase() = default;

    auto operator=(const ApproxBase &other) -> ApproxBase<U, T>&;
    auto operator=(ApproxBase &&other) noexcept -> ApproxBase<U, T>&;
    // 'virtual' cannot be specified on member function template:
    // template <typename V = U>
    // requires std::ranges::sized_range<V>
    virtual auto operator==(const U &actual) const -> bool;
    // _yield_comparisons cannot be a pure virtual member function because ApproxScalar does not implement it.
    virtual auto _yield_comparisons(const U &actual) const -> std::any;
    template<FPScalar V>
    auto _approx_scalar(V x) const -> ApproxScalar<V, T>;
};

/*
 * Perform approximate comparisons where the expected value is a sequence of numbers.
 */
template<std::ranges::sized_range U, std::floating_point T>
class ApproxSequenceLike : public ApproxBase<U, T> {
public:
    using ApproxBase<U, T>::ApproxBase;
    auto operator==(const U &actual) const -> bool override;
    auto _yield_comparisons(const U &actual) const -> std::any override;
};

/*
 * Perform approximate comparisons where the expected value is a single number.
 */
template<FPScalar U, std::floating_point T>
class ApproxScalar : public ApproxBase<U, T> {
private:
    static constexpr T DEFAULT_ABSOLUTE_TOLERANCE = 1e-12;
    static constexpr T DEFAULT_RELATIVE_TOLERANCE = 1e-6;
public:
    // See TCPP 4th ed Chapter 20.3.5.1 Inheriting Constructors
    using ApproxBase<U, T>::ApproxBase;
    // virtual ~ApproxScalar() = default;
    auto operator==(const U &actual) const -> bool override;
    virtual auto tolerance() const -> T;
};

template<std::ranges::sized_range U, std::floating_point T>
auto approx(const U &expected, T rel = NAN, T abs = NAN, bool nan_ok = false) -> ApproxSequenceLike<U, T>;

template<FPScalar U, std::floating_point T>
auto approx(U expected, T rel = NAN, T abs = NAN, bool nan_ok = false) -> ApproxScalar<U, T>;

#endif //APPROX_H

//
// Created by Yihua on 2023/11/27.
//

// GCC has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
// If valarray operations are not needed, then just use vectors, because vectors have at() method.
#include <algorithm>
#include <functional>
#include <numbers>
#include <boost/numeric/ublas/assignment.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "tmm.h"
#include "utils.h"

template<std::floating_point T>
auto list_snell(const std::valarray<std::complex<T>> &n_list, const std::complex<T> th_0,
                const std::size_t num_wl) -> std::valarray<std::complex<T>> {
    const std::size_t num_layers = n_list.size() / num_wl;
    std::valarray<std::complex<T>> angles(num_layers * num_wl);
    for (std::size_t i = 0; i < num_layers; i++) {
        // angles[i] = std::asin(n_list[0] * std::sin(th_0) / n_list[i]); if n_list and angles are vector of valarray
        angles[std::slice(i * num_wl, num_wl, 1)] = std::asin(n_list[std::slice(0, num_wl, 1)] * std::sin(th_0) / n_list[std::slice(i * num_wl, num_wl, 1)]);
    }
    // std::ranges::any_of(n, std::bind_front(std::less<>(),
    //            std::bind_front(std::multiplies<>(),
    //                    std::bind_front<F>(std::real), std::bind_front<F>(std::imag)),
    //            0))
    // valarray is_forward_angle is not realistic.
    for (std::size_t i = 0; i < num_wl; i++) {
        if (not is_forward_angle(n_list[i], angles[i])) {  // n_list[0] and angles[0]
            angles[i] = std::numbers::pi_v<T> - angles[i];
        }
        if (not is_forward_angle(n_list[(num_layers - 1) * num_wl + i], angles[(num_layers - 1) * num_wl + i])) {  // not else if!
            angles[(num_layers - 1) * num_wl + i] = std::numbers::pi_v<T> - angles[(num_layers - 1) * num_wl + i];
        }
    }
    return angles;
}

template<typename T>
auto interface_r(const char polarization, const std::valarray<std::complex<T>> &n_i,
                 const std::valarray<std::complex<T>> &n_f, const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<std::complex<T>> {
    if (polarization == 's') {
        return (n_i * std::cos(th_i) - n_f * std::cos(th_f)) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    }
    if (polarization == 'p') {
        return (n_f * std::cos(th_i) - n_i * std::cos(th_f)) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<typename T>
auto interface_t(const char polarization, const std::valarray<std::complex<T>> &n_i,
                 const std::valarray<std::complex<T>> &n_f, const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<std::complex<T>> {
    if (polarization == 's') {
        return 2 * n_i * std::cos(th_i) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    }
    if (polarization == 'p') {
        return 2 * n_i * std::cos(th_i) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<typename T>
auto interface_t(const char polarization, const std::valarray<std::complex<T>> &n_i,
                 const std::valarray<std::complex<T>> &n_f, const std::complex<T> th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<std::complex<T>> {
    if (polarization == 's') {
        return 2 * n_i * std::cos(th_i) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    }
    if (polarization == 'p') {
        return 2 * n_i * std::cos(th_i) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<typename T>
auto R_from_r(const std::valarray<std::complex<T>> &r) -> std::valarray<T> {
    std::valarray<T> R_array(r.size());
    std::ranges::transform(r, std::begin(R_array), [](const std::complex<T> reflection) {
        return std::norm(reflection);
    });
    return R_array;
}

template<typename T>
auto T_from_t(const char pol, const std::valarray<std::complex<T>> &t, const std::valarray<std::complex<T>> &n_i,
              const std::valarray<std::complex<T>> &n_f, const std::valarray<std::complex<T>> &th_i,
              const std::valarray<std::complex<T>> &th_f) -> std::valarray<T> {
    using F = T (*)(const std::complex<T> &);
    std::valarray<T> t_norm(t.size());
    if (pol == 's') {
        const std::valarray<std::complex<T>> f_prod = n_f * std::cos(th_f);
        const std::valarray<std::complex<T>> i_prod = n_i * std::cos(th_i);
        std::valarray<T> f_prod_r(f_prod.size());
        std::valarray<T> i_prod_r(i_prod.size());
        std::ranges::transform(f_prod, std::begin(f_prod_r), std::bind_front<F>(std::real));
        std::ranges::transform(i_prod, std::begin(i_prod_r), std::bind_front<F>(std::real));
        std::ranges::transform(t, std::begin(t_norm), std::bind_front<F>(std::norm));
        return t_norm * f_prod_r / i_prod_r;
    }
    if (pol == 'p') {
        std::valarray<T> p_res(t.size());
        for (std::size_t i = 0; i < t.size() ; i++) {
            p_res[i] = std::norm(t[i]) * (n_f[i] * std::conj(std::cos(th_f[i]))).real() / (n_i[i] * std::conj(std::cos(th_i[i]))).real();
        }
        return p_res;
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<typename T>
auto T_from_t(const char pol, const std::valarray<std::complex<T>> &t, const std::valarray<std::complex<T>> &n_i,
              const std::valarray<std::complex<T>> &n_f, const std::complex<T> th_i,
              const std::valarray<std::complex<T>> &th_f) -> std::valarray<T> {
    using F = T (*)(const std::complex<T> &);  // F cannot be noexcept
    std::valarray<T> t_norm(t.size());
    // https://stackoverflow.com/questions/62807743/why-use-stdbind-front-over-lambdas-in-c20
    // https://stackoverflow.com/questions/73202679/problem-using-stdtransform-with-lambdas-vs-stdtransform-with-stdbind
    // https://godbolt.org/z/hPx7P6W3E
    // https://stackoverflow.com/questions/6610046/stdfunction-and-stdbind-what-are-they-and-when-should-they-be-used
    // Effective Modern C++ Chapter 6 Lambda Expressions Item 34 Prefer lambdas to std::bind
    // std::ranges::transform(f_prod, std::begin(f_prod_r), std::bind(static_cast<T (*)(const std::complex<T> &)>(std::real), std::placeholders::_1));
    // std::ranges::transform(f_prod, std::begin(f_prod_r), std::bind_front(static_cast<T (*)(const std::complex<T> &)>(std::real)));
    if (pol == 's') {
        const std::valarray<std::complex<T>> f_prod = n_f * std::cos(th_f);
        const std::valarray<std::complex<T>> i_prod = n_i * std::cos(th_i);
        std::valarray<T> f_prod_r(f_prod.size());
        std::valarray<T> i_prod_r(i_prod.size());
        std::ranges::transform(f_prod, std::begin(f_prod_r), std::bind_front<F>(std::real));
        std::ranges::transform(i_prod, std::begin(i_prod_r), std::bind_front<F>(std::real));
        std::ranges::transform(t, std::begin(t_norm), std::bind_front<F>(std::norm));
        return t_norm * f_prod_r / i_prod_r;
    }
    if (pol == 'p') {
        std::valarray<T> p_res(t.size());
        for (std::size_t i = 0; i < t.size() ; i++) {
            p_res[i] = std::norm(t[i]) * (n_f[i] * std::conj(std::cos(th_f[i]))).real() / (n_i[i] * std::conj(std::cos(th_i))).real();
        }
        return p_res;
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<std::floating_point T>
auto interface_R(const char polarization, const std::valarray<std::complex<T>> &n_i,
                 const std::valarray<std::complex<T>> &n_f, const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<T> {
    // Although solcore's tmm is inherited from an old version of sbyrnes321's tmm,
    // they have independent development routes.
    std::valarray<std::complex<T>> r = interface_r(polarization, n_i, n_f, th_i, th_f);
    // If the outgoing angle is pi/2, that means the light is totally internally reflected, and we can set r = R = 1.
    // If not, can get unphysical results for r.

    // Note that while T actually CAN be > 1 (when you have incidence from an absorbing
    // medium), I[solcore] don't think R can ever be > 1.

    // For a given boolean (val)array mask,
    // std::copy_if(numbers.begin(), numbers.end(), std::back_inserter(result), [&mask, i = 0](const int &)
    // mutable {
    //     return mask[i++];
    // });
    // Binary operation
    std::ranges::transform(r, th_f, std::begin(r),
                   [](const std::complex<T> r_value, const std::complex<T> th_f_value) {
                       return (th_f_value.real() > std::numbers::pi_v<T> / 2 - 1e-6) ? 1 : r_value;
                   });
    // An alternative way
    std::ranges::replace_if(r, [&th_i, i = 0](const std::complex<T> &) mutable -> bool {
        return th_i[i++].real() > std::numbers::pi_v<T> / 2 - 1e-6;
    }, 1);

    return R_from_r(r);
}

template auto interface_R(const char polarization, const std::valarray<std::complex<double>> &n_i,
                          const std::valarray<std::complex<double>> &n_f,
                          const std::valarray<std::complex<double>> &th_i,
                          const std::valarray<std::complex<double>> &th_f) -> std::valarray<double>;

template<std::floating_point T>
auto interface_T(const char polarization, const std::valarray<std::complex<T>> &n_i,
                 const std::valarray<std::complex<T>> &n_f, const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<T> {
    std::valarray<std::complex<T>> t = interface_t(polarization, n_i, n_f, th_i, th_f);
    // If the incoming angle is pi/2, that means (most likely) that the light was previously
    // totally internally reflected. That means the light will never reach this interface, and
    // we can safely set t = T = 0; otherwise we get numerical issues which give unphysically large
    // values of T because in T_from_t we divide by cos(th_i) which is ~ 0.
    std::transform(std::begin(t), std::end(t), std::begin(th_i), std::begin(t),
                   [](const std::complex<T> t_value, const std::complex<T> th_i_value) {
                       return (th_i_value.real() > std::numbers::pi_v<T> / 2 - 1e-6) ? 0 : t_value;
                   });

    return T_from_t(polarization, t, n_i, n_f, th_i, th_f);
}

template auto interface_T(const char polarization, const std::valarray<std::complex<double>> &n_i,
                          const std::valarray<std::complex<double>> &n_f,
                          const std::valarray<std::complex<double>> &th_i,
                          const std::valarray<std::complex<double>> &th_f) -> std::valarray<double>;

template<std::floating_point T>
auto interface_T(const char polarization, const std::valarray<std::complex<T>> &n_i,
                 const std::valarray<std::complex<T>> &n_f, const std::complex<T> th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<T> {
    std::valarray<std::complex<T>> t = interface_t(polarization, n_i, n_f, th_i, th_f);
    std::ranges::transform(t, std::begin(t), [&th_i](const std::complex<T> t_value) {
        return (th_i.real() > std::numbers::pi_v<T> / 2 - 1e-6) ? 0 : t_value;
    });

    return T_from_t(polarization, t, n_i, n_f, th_i, th_f);
}

template<typename T>
auto power_entering_from_r(const char pol, std::valarray<std::complex<T>> &r, const std::valarray<std::complex<T>> &n_i,
                           const std::complex<T> th_i) -> std::valarray<T> {
    std::valarray<T> power(r.size());
    // An interesting fact is that r or n_i marked non-const is enough if using ranges::transform,
    // depending on the order in the transform statement.
    // Yet it is better to remain the parameter n_list passed to coh_tmm const.
    if (pol == 's') {
        std::ranges::transform(power, n_i, std::begin(r), [&th_i](const std::complex<T> n_value, const std::complex<T> r_value) {
            return (n_value * std::cos(th_i) * (1.0 + std::conj(r_value)) * (1.0 - r_value)).real() / (n_value * std::cos(th_i)).real();
        });
        return power;
    }
    if (pol == 'p') {
        std::ranges::transform(power, n_i, std::begin(r), [&th_i](const std::complex<T> n_value, const std::complex<T> r_value) {
            return (n_value * std::conj(std::cos(th_i)) * (1.0 + r_value) * (1.0 - std::conj(r_value))).real() /
                   (n_value * std::conj(std::cos(th_i))).real();
        });
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<std::floating_point T>
auto coh_tmm(const char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             const std::complex<T> th_0, const std::valarray<T> &lam_vac) -> coh_tmm_vec_dict<T> {
    // th_0 is std::complex<T>
    // This function is not vectorized for angles; you need to run one angle calculation at a time.
    const std::size_t num_wl = lam_vac.size();
    const std::size_t num_layers = n_list.size() / num_wl;
    if (num_layers not_eq d_list.size()) {
        throw std::invalid_argument("n_list and d_list must have same length");
    }
    if (not std::isinf(d_list[0]) or not std::isinf(d_list[d_list.size() - 1])) {
        throw std::invalid_argument("d_list must start and end with inf!");
    }
    // gcc/clang can properly distinguish pipeline operator and valarray's bitwise or operator, but not for
    // (gcc) error: no match for 'operator|' (operand types are 'std::_Expr<std::__detail::_BinClos<std::__multiplies,
    // std::_Expr, std::_Constant, std::__detail::_SClos<std::_ValArray, std::complex<double> >, std::complex<double> >,
    // std::complex<double> >' and 'std::ranges::views::__adaptor::_Partial<std::ranges::views::_Transform,
    // std::_Bind_front0<double (*)(const std::complex<double>&)> >')
    // (clang) See https://github.com/llvm/llvm-project/issues/76393
    // Also see:
    // https://stackoverflow.com/questions/62110985/add-of-stdvalarray-got-different-sizes-with-different-operand-orders
    // We have to explicitly define:
    std::valarray<std::complex<T>> temp_va = n_list[std::slice(0, num_wl, 1)] * std::sin(th_0);
    if (std::ranges::any_of(temp_va |
                            std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)),
                            // Note the order of front-binding!
                            std::bind_front(std::less_equal<>(), TOL * EPSILON<T>))) {
        throw std::invalid_argument("Error in n0 or th0!");
    }
    const std::valarray<std::complex<T>> th_list = list_snell(n_list, th_0, num_wl);
    std::valarray<std::complex<T>> compvec_lam_vac(n_list.size());
    // #ifdef __cpp_lib_ranges_repeat
    // Till this code is written (Dec. 13, 2023), [P2328R1](https://wg21.link/P2328R1)
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2328r1.html
    // "DR20: views::join should join all views of ranges" is completely supported by GCC libstdc++ 11.2
    // and Clang libc++ 15 and partially supported by MSVC STL 19.30.
    // It can be successfully compiled by Microsoft C/C++ Compiler 19.38.33130,
    // but cannot be successfully compiled by clang version 17.0.6 and libc++ 17.0.6-1:
    // error: no member named 'join' in namespace 'std::ranges::views'
    // Discussion on this issue: https://www.reddit.com/r/cpp_questions/comments/18ag9fy/cant_use_stdviewsjoin_with_libc
    // `-fexperimental-library` flag must be added to make clang compile this.
    // 16 hours before this comment was written (UTC+8 3pm Dec. 13, 2023) llvm-project PR #66033
    // [libc++] P2770R0: "Stashing stashing iterators for proper flattening"
    // removed the _LIBCPP_ENABLE_EXPERIMENTAL macro in __ranges/join_view.h in commit 6a66467.
    // Note that std::(ranges::)views::repeat (std::ranges::repeat_view) is a range factory like
    // std::(ranges::)views::iota (std::ranges::iota_view) rather than a range adaptor.
    // The repeated view is repeating transform_view of ref_view of valarray, so it must be joined (flatten).
    // Numpy can do vectorization automatically, but we have to manually vectorize the valarray lam_vac.
    std::ranges::move(std::views::repeat(lam_vac | std::views::transform([](const T real) {
        return std::complex<T>(real, 0.0);
    }), num_layers) | std::views::join, std::begin(compvec_lam_vac));
    const std::valarray<std::complex<T>> kz_list = 2 * std::numbers::pi_v<T> * n_list * std::cos(th_list) / compvec_lam_vac;
    // Do the same thing to d_list.
    std::valarray<std::complex<T>> compvec_d_list(n_list.size());
    std::ranges::move(std::views::repeat(d_list | std::views::transform([](const T real) {
        return std::complex<T>(real, 0.0);
    }), num_layers) | std::views::join, std::begin(compvec_d_list));
    // A weird exception once occurred Exception 0xc0000005 encountered at address 0x____________:
    // Access violation reading location 0x00000000 at valarray::_Grow(_Count) ->
    // _Myptr = _Allocate_for_op_delete<_Ty>(_Newsize) ->
    // return static_cast<_Ty*>(::operator new(_Bytes));
    std::valarray<std::complex<T>> delta = kz_list * va_2d_transpose(compvec_d_list, num_wl);
    // std::slice_array does not have std::begin() or std::end().
    std::ranges::transform(std::begin(delta) + num_wl, std::begin(delta) + (num_layers - 1) * num_wl, std::begin(delta) + num_wl, [](const std::complex<T> delta_i) {
        return delta_i.imag() > 100 ? std::complex<T>(delta_i.real(), 100) : delta_i;
    });
    std::vector<std::vector<std::valarray<std::complex<T>>>> t_list(num_layers, std::vector<std::valarray<std::complex<T>>>(num_layers, std::valarray<std::complex<T>>(num_wl)));
    std::vector<std::vector<std::valarray<std::complex<T>>>> r_list(num_layers, std::vector<std::valarray<std::complex<T>>>(num_layers, std::valarray<std::complex<T>>(num_wl)));
    for (std::size_t i = 0; i < num_layers - 1; i++) {
        t_list[i][i + 1] = interface_t(pol, n_list[i], n_list[i + 1], th_list[i], th_list[i + 1]);
        r_list[i][i + 1] = interface_r(pol, n_list[i], n_list[i + 1], th_list[i], th_list[i + 1]);
    }
    // boost::numeric::ublas::tensor<std::complex<T>, boost::numeric::ublas::last_order> M_list(boost::numeric::ublas::shape{num_layers, num_wl, 2, 2});
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> M_list(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_layers * num_wl);
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        // Array Layout j=\sum_{r=1}^p{i_r\cdot w_r} with 1\leq i_r\leq n_r for 1\leq r\leq p.
        // First-order layout: w1 = 1, wk = nk-1 * wk-1 => j = i1 + i2n1 + i3n1n2 + ... + ipn1n2...np-1
        // Last-order layout: wp = 1, wk = nk+1 * wk+1 => j = i1n2...np + i2n3...np + ... + ip-1np + ip
        // For C-style 2D array, a[i][j] = a[i * num_cols + j] (stored by rows), i.e., j = i1n2 + i2 (last-order).
        // (two dimensions) format_t | column_major | first-order | MATLAB/Fortran
        // (two dimensions) format_t | row_major    | last-order  | C/C++
        // Default storage_type: std::vector<value_t>
        // boost::numeric::ublas::tensor<std::complex<T>, boost::numeric::ublas::last_order> A(boost::numeric::ublas::shape{num_wl, 2, 2});
        // boost::numeric::ublas::tensor<std::complex<T>, boost::numeric::ublas::last_order> B(boost::numeric::ublas::shape{num_wl, 2, 2});
        // Unfortunately, <boost/numeric/ublas/assignment.hpp> operator<<= only supports vector and matrix.
        // Note on at() method: for a 3D tensor T, T.at(0, 0), T.at(0, 1), T.at(1, 0), and T.at(1, 1) are
        // T.at(0, 0, 0), T.at(0, 1, 0), T.at(1, 0, 0), and T.at(1, 1, 0)
        // Unfortunately, boost ublas tensor does not support range or slice of a tensor,
        // which makes it extremely difficult to use.
        std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> A(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_wl);
        std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> B(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_wl);
        for (std::size_t j = 0; j < num_wl; j++) {
            A[j](0, 0) = std::exp(std::complex<T>(0, -1) * delta[i * num_layers + j]);
            A[j](0, 1) = 0;
            A[j](1, 0) = 0;
            A[j](1, 1) = std::exp(std::complex<T>(0, 1) * delta[i * num_layers + j]);
            B[j](0, 0) = 1;
            B[j](0, 1) = r_list[i][i + 1][j];
            B[j](1, 0) = r_list[i][i + 1][j];
            B[j](1, 1) = 1;
            M_list[i * num_layers + j] = boost::numeric::ublas::prod(A[j], B[j]) / t_list[i][i + 1][j];
        }
        // The matrix multiplication of a matrix M1 and a matrix M2 is einsum('...ij,...jk', A, B)
        // where ellipses are used to enable and control broadcasting.
    }
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> Mtilde(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_wl);
    // https://stackoverflow.com/questions/48821973/constant-matrix-with-boost-ublas
    for (std::size_t i = 0; i < num_wl; i++) {
        Mtilde[i] <<= 1, 0,
                      0, 1;
    }
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        for (std::size_t j = 0; j < num_wl; j++) {
            Mtilde[j] = boost::numeric::ublas::prod(Mtilde[j], M_list[i * num_layers + j]);
        }
    }
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> A(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        A[i] <<= 1,               r_list[0][1][i],
                 r_list[0][1][i], 1;
        Mtilde[i] = boost::numeric::ublas::prod(Mtilde[i], A[i]) / t_list[0][1][i];
    }
    std::valarray<std::complex<T>> r(num_wl);
    std::valarray<std::complex<T>> t(num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        r[i] = Mtilde[i](1, 0) / Mtilde[i](0, 0);
        t[i] = 1.0 / Mtilde[i](0, 0);
    }
    // boost::multi_array<std::complex<T>, 3> vw_list(boost::extents[num_layers][num_wl][2]);
    // It is not necessary to use <boost/multi_array.hpp> here.
    std::vector<std::vector<std::array<std::complex<T>, 2>>> vw_list(num_layers, std::vector<std::array<std::complex<T>, 2>>(num_wl));
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> vw(boost::numeric::ublas::zero_matrix<std::complex<T>>(2, 2), num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        vw[i](0, 0) = t[i];
        vw[i](0, 1) = t[i];
    }
    for (std::size_t i = 0; i < num_wl; i++) {
        vw_list.at(num_layers - 1).at(i).at(0) = vw[i](0, 0);
        vw_list.at(num_layers - 1).at(i).at(1) = vw[i](0, 1);
    }
    for (std::size_t i = num_layers - 2; i > 0; i--) {
        for (std::size_t j = 0; j < num_wl; j++) {
            vw[j] = boost::numeric::ublas::prod(M_list[i * num_layers + j], vw[j]);
            vw_list.at(i).at(j).at(0) = vw[j](0, 1);
            vw_list.at(i).at(j).at(1) = vw[j](1, 1);
        }
    }
    // It should be better if using plain for-loop.
    // boost::multi_array<std::complex<T>, 1> ones(boost::extents[num_wl]);
    // std::ranges::fill(ones, 1);
    // vw_list[boost::indices[num_layers - 1][boost::multi_array_types::index_range()][1]] = ones;
    std::ranges::transform(vw_list.at(num_layers - 1), vw_list.at(num_layers - 1).begin(), [](std::array<std::complex<T>, 2> &subarray) -> std::array<std::complex<T>, 2> {
        subarray.at(1) = 1;
        return subarray;
    });
    const std::valarray<T> R = R_from_r(r);
    const std::valarray<T> Tr = T_from_t(pol, t, n_list[std::slice(0, num_wl, 1)], n_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)], th_0, th_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)]);
    const std::valarray<T> power_entering = power_entering_from_r(pol, r, n_list[std::slice(0, num_wl, 1)], th_0);
    return {{"r", r},
            {"t", t},
            {"R", R},
            {"T", Tr},
            {"power_entering", power_entering},
            {"vw_list", vw_list},
            {"kz_list", kz_list},
            {"th_list", th_list},
            {"pol", pol},
            {"n_list", n_list},
            {"d_list", d_list},
            {"th_0", th_0},
            {"lam_vac", lam_vac}};
}

template auto coh_tmm(const char pol, const std::valarray<std::complex<double>> &n_list,
                      const std::valarray<double> &d_list, const std::complex<double> th_0,
                      const std::valarray<double> &lam_vac) -> coh_tmm_vec_dict<double>;

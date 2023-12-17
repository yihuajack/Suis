//
// Created by Yihua on 2023/11/27.
//

// GCC has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
#include <algorithm>
#include <functional>
#include <numbers>
#include <boost/numeric/ublas/tensor.hpp>
#include "FixedMatrix.h"
#include "tmm.h"
#include "utils.h"

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
auto R_from_r(const std::valarray<std::complex<T>> &r) -> std::valarray<T> {
    std::valarray<T> R_array(r.size());
    std::transform(std::begin(r), std::end(r), std::begin(R_array), [](const std::complex<T> reflection) {
        return std::norm(reflection);
    });
    return R_array;
}

template<typename T>
auto T_from_t(const char pol, const std::valarray<std::complex<T>> &t, const std::valarray<std::complex<T>> &n_i,
              const std::valarray<std::complex<T>> &n_f, const std::valarray<std::complex<T>> &th_i,
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
            p_res[i] = std::norm(t[i]) * (n_f[i] * std::conj(std::cos(th_f[i]))).real() / (n_i[i] * std::conj(std::cos(th_i[i]))).real();
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
    std::transform(std::begin(r), std::end(r), std::begin(th_f), std::begin(r),
                   [](const std::complex<T> r_value, const std::complex<T> th_f_value) {
                       return (th_f_value.real() > std::numbers::pi_v<T> / 2 - 1e-6) ? 1 : r_value;
                   });
    // An alternative way
    std::replace_if(std::begin(r), std::end(r), [&th_i, i = 0](const std::complex<T> &) mutable -> bool {
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
auto coh_tmm(const char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             const std::complex<T> th_0, const std::valarray<T> &lam_vac) -> coh_tmm_dict<T> {
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
    if (std::ranges::any_of(n_list[std::slice(0, num_wl, 1)] * std::sin(th_0) | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)), std::bind_front(std::greater_equal<>(), TOL * EPSILON<T>))) {
        throw std::invalid_argument("Error in n0 or th0!");
    }
    const std::valarray<std::complex<T>> th_list = list_snell(n_list, th_0);
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
    std::valarray<std::complex<T>> delta = kz_list * compvec_d_list;
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
    boost::numeric::ublas::tensor<std::complex<T>, boost::numeric::ublas::last_order> M_list(boost::numeric::ublas::shape{num_layers, num_wl, 2, 2});
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        // Array Layout j=\sum_{r=1}^p{i_r\cdot w_r} with 1\leq i_r\leq n_r for 1\leq r\leq p.
        // First-order layout: w1 = 1, wk = nk-1 * wk-1 => j = i1 + i2n1 + i3n1n2 + ... + ipn1n2...np-1
        // Last-order layout: wp = 1, wk = nk+1 * wk+1 => j = i1n2...np + i2n3...np + ... + ip-1np + ip
        // For C-style 2D array, a[i][j] = a[i * num_cols + j] (stored by rows), i.e., j = i1n2 + i2 (last-order).
        // (two dimensions) format_t column_major first-order MATLAB/Fortran
        // (two dimensions) format_t row_major    last-order  C/C++
        // Default storage_type: std::vector<value_t>
        boost::numeric::ublas::tensor<std::complex<T>, boost::numeric::ublas::last_order> A(boost::numeric::ublas::shape{num_wl, 2, 2});
        boost::numeric::ublas::tensor<std::complex<T>, boost::numeric::ublas::last_order> B(boost::numeric::ublas::shape{num_wl, 2, 2});
        // Unfortunately, <boost/numeric/ublas/assignment.hpp> operator<<= only supports vector and matrix.
        std::vector<std::complex<T>> d(num_wl);
        for (std::size_t j = 0; j < num_wl; j++) {
            A.at(j, 0, 0) = std::exp(std::complex<T>(0, -1) * delta[i * num_layers + j]);
            A.at(j, 0, 1) = 0;
            A.at(j, 1, 0) = 0;
            A.at(j, 1, 1) = std::exp(std::complex<T>(0, 1) * delta[i * num_layers + j]);
            B.at(j, 0, 0) = 1;
            B.at(j, 0, 1) = r_list[i][i + 1][j];
            B.at(j, 1, 0) = r_list[i][i + 1][j];
            B.at(j, 1, 1) = 1;
            d.at(j) = t_list[i][i + 1][j];
        }
        // The matrix multiplication of a matrix M1 and a matrix M2 is einsum('...ij,...jk', A, B)
        // where ellipses are used to enable and control broadcasting.
        M_list(i) = boost::numeric::ublas::prod(A, B, 1);
    }
    FixedMatrix<std::complex<T>, 2, 2> Mtilde = {{1, 0}, {0, 1}};
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        Mtilde = dot(Mtilde, M_list[i]);
    }
    Mtilde = dot(FixedMatrix<std::complex<T>, 2, 2>({{1, r_list[0][1]}, {r_list[0][1], 1}}) / t_list[0][1], Mtilde);
    const std::complex<T> r = Mtilde[1][0] / Mtilde[0][0];
    const std::complex<T> t = 1.0 / Mtilde[0][0];
    std::vector<std::array<std::complex<T>, 2>> vw_list(num_layers, std::array<std::complex<T>, 2>());
    FixedMatrix<std::complex<T>, 2, 1> vw({{t}, {0}});
    vw_list.back() = vw.transpose().squeeze();
    for (std::size_t i = num_layers - 2; i > 0; i--) {
        vw = dot(M_list[i], vw);
        vw_list[i] = vw.transpose().squeeze();
    }
    const T R = R_from_r(r);
    const T Tr = T_from_t(pol, t, n_list[0], n_list[n_list.size() - 1], th_0, th_list[th_list.size() - 1]);
    const T power_entering = power_entering_from_r(pol, r, n_list[0], th_0);
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
                      const std::valarray<double> &lam_vac) -> coh_tmm_dict<double>;

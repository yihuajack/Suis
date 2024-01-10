//
// Created by Yihua on 2023/11/27.
//

// GCC/Clang has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
// If valarray operations are not needed, then just use vectors, because vectors have at() method.
#include <algorithm>
#include <functional>
#include <numbers>
#include <numeric>
#include <boost/numeric/ublas/assignment.hpp>  // operator<<=
#include <boost/numeric/ublas/matrix.hpp>
#include "tmm.h"
#include "utils.h"

using namespace std::complex_literals;

template<typename T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto snell(const std::valarray<std::complex<T>> &n_1, const std::valarray<std::complex<T>> &n_2,
           const TH_T &th_1, const std::size_t num_wl) -> std::valarray<std::complex<T>> {
    std::valarray<std::complex<T>> th_2_guess = std::asin(n_1 * std::sin(th_1) / n_2);
    for (std::size_t i = 0; i < num_wl; i++) {
        if (not is_forward_angle(n_2[i], th_2_guess[i])) {
            th_2_guess[i] = std::numbers::pi_v<T> - th_2_guess[i];
        }
    }
    return th_2_guess;
}

template<std::floating_point T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto list_snell(const std::valarray<std::complex<T>> &n_list, const TH_T &th_0,
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
auto R_from_r(const std::valarray<std::complex<T>> &r) -> std::valarray<T> {
    std::valarray<T> R_array(r.size());
    std::ranges::transform(r, std::begin(R_array), [](const std::complex<T> reflection) {
        return std::norm(reflection);
    });
    return R_array;
}

template<typename T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto T_from_t(const char pol, const std::valarray<std::complex<T>> &t, const std::valarray<std::complex<T>> &n_i,
              const std::valarray<std::complex<T>> &n_f, const TH_T &th_i,
              const std::valarray<std::complex<T>> &th_f) -> std::valarray<T> {
    using F = T (*)(const std::complex<T> &);  // F cannot be noexcept
    using G = std::complex<T> (*)(const std::complex<T> &);
    // https://stackoverflow.com/questions/62807743/why-use-stdbind-front-over-lambdas-in-c20
    // https://stackoverflow.com/questions/73202679/problem-using-stdtransform-with-lambdas-vs-stdtransform-with-stdbind
    // https://godbolt.org/z/hPx7P6W3E
    // https://stackoverflow.com/questions/6610046/stdfunction-and-stdbind-what-are-they-and-when-should-they-be-used
    // Effective Modern C++ Chapter 6 Lambda Expressions Item 34 Prefer lambdas to std::bind
    // std::ranges::transform(f_prod, std::begin(f_prod_r), std::bind(static_cast<T (*)(const std::complex<T> &)>(std::real), std::placeholders::_1));
    // std::ranges::transform(f_prod, std::begin(f_prod_r), std::bind_front(static_cast<T (*)(const std::complex<T> &)>(std::real)));
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
        std::valarray<std::complex<T>> f_prod = n_f * std::cos(th_f);
        std::valarray<std::complex<T>> i_prod = n_i * std::cos(th_i);
        std::valarray<T> f_prod_cr(f_prod.size());
        std::valarray<T> i_prod_cr(i_prod.size());
        std::ranges::transform(f_prod, std::begin(f_prod), std::bind_front<G>(std::conj));
        std::ranges::transform(i_prod, std::begin(i_prod), std::bind_front<G>(std::conj));
        std::ranges::transform(f_prod, std::begin(f_prod_cr), std::bind_front<F>(std::real));
        std::ranges::transform(i_prod, std::begin(i_prod_cr), std::bind_front<F>(std::real));
        std::ranges::transform(t, std::begin(t_norm), std::bind_front<F>(std::norm));
        return t_norm * f_prod_cr / i_prod_cr;
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

template<typename T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto power_entering_from_r(const char pol, const std::valarray<std::complex<T>> &r,
                           const std::valarray<std::complex<T>> &n_i, const TH_T &th_i) -> std::valarray<T> {
    std::valarray<T> power(r.size());
    using F = T (*)(const std::complex<T> &);
    using G = std::complex<T> (*)(const std::complex<T> &);
    std::valarray<std::complex<T>> r_conj(r.size());
    std::ranges::transform(r, std::begin(r_conj), std::bind_front<G>(std::conj));
    std::valarray<T> real_numerator(n_i.size());
    std::valarray<T> real_denominator(n_i.size());
    if (pol == 's') {
        std::valarray<std::complex<T>> complex_denominator = n_i * std::cos(th_i);
        std::valarray<std::complex<T>> complex_numerator = complex_denominator * (1 + r_conj) * (1 - r);
        std::ranges::transform(complex_numerator, std::begin(real_numerator), std::bind_front<F>(std::real));
        std::ranges::transform(complex_denominator, std::begin(real_denominator), std::bind_front<F>(std::real));
        return real_numerator / real_denominator;
    }
    if (pol == 'p') {
        std::valarray<std::complex<T>> complex_denominator(n_i.size());
        if constexpr (std::is_same_v<TH_T, std::valarray<std::complex<T>>>) {
            std::ranges::transform(n_i, th_i, std::begin(complex_denominator), [](const std::complex<T> n, std::complex<T> th) {
                return n * std::conj(std::cos(th));
            });
            // A std::bind() functional version:
            // std::ranges::transform(n_i, th_i, std::begin(complex_denominator),
            //                        std::bind(std::multiplies<>(),
            //                                  std::placeholders::_1,
            //                                  std::bind<G>(std::conj, std::bind<G>(std::cos, std::placeholders::_2))));
        } else {
            complex_denominator = n_i * std::conj(std::cos(th_i));
        }
        std::valarray<std::complex<T>> complex_numerator = complex_denominator * (1 + r) * (1 - r_conj);
        std::ranges::transform(complex_numerator, std::begin(real_numerator), std::bind_front<F>(std::real));
        std::ranges::transform(complex_denominator, std::begin(real_denominator), std::bind_front<F>(std::real));
        return real_numerator / real_denominator;
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<typename T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto coh_tmm(const char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             const TH_T &th_0, const std::valarray<T> &lam_vac) -> coh_tmm_vec_dict<T> {
    // th_0 is std::complex<T>
    // This function is not vectorized for angles; you need to run one angle calculation at a time.
    const std::size_t num_wl = lam_vac.size();
    const std::size_t num_elems = n_list.size();
    const std::size_t num_layers = num_elems / num_wl;
    if constexpr (std::is_same_v<TH_T, std::valarray<std::complex<T>>>) {
        if (th_0.size() not_eq lam_vac.size()) {
            throw std::invalid_argument("th_0 and lam_vac have different sizes;"
                                        "you need to run one angle calculation at a time.");
        }
    }
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
    // However, MSVC can properly distinguish it.
    // For compatibility, we have to explicitly convert it to a valarray:
#ifdef _MSC_VER
    if (std::ranges::any_of(n_list[std::slice(0, num_wl, 1)] * std::sin(th_0) |
#else
    if (std::ranges::any_of(std::valarray<std::complex<T>>(n_list[std::slice(0, num_wl, 1)] * std::sin(th_0)) |
#endif
        std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)),
            // Note the order of front-binding!
                            std::bind_front(std::less_equal<>(), TOL * EPSILON<T>))) {
        throw std::invalid_argument("Error in n0 or th0!");
    }
    const std::valarray<std::complex<T>> th_list = list_snell(n_list, th_0, num_wl);
    std::valarray<std::complex<T>> compvec_lam_vac(num_elems);
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
    std::ranges::move(std::views::repeat(lam_vac | std::views::transform([](const T real) -> std::complex<T> {
        return real;
    }), num_layers) | std::views::join, std::begin(compvec_lam_vac));
    const std::valarray<std::complex<T>> kz_list = 2 * std::numbers::pi_v<T> * n_list * std::cos(th_list) / compvec_lam_vac;
    // Do the same thing to d_list.
    std::valarray<std::complex<T>> compvec_d_list(num_elems);
    std::ranges::move(std::views::repeat(d_list | std::views::transform([](const T real) -> std::complex<T> {
        return real;
    }), num_wl) | std::views::join, std::begin(compvec_d_list));
    // Note that if num_layers * d_list.size() greater than compvec_d_list.size(), by address sanitizer (ASAN),
    // there will be a heap-buffer-overflow. Shadow bytes around the buggy address contain fa (Heap left redzone).
    // Without ASAN, the problem emerges until executing the next line:
    // (msvc) Exception 0x80000003 encountered at address 0x____________:
    // Access violation reading location 0x00000000 at valarray::_Grow(_Count) ->
    // _Myptr = _Allocate_for_op_delete<_Ty>(_Newsize) ->
    // return static_cast<_Ty*>(::operator new(_Bytes));
    // where _Count is 10, _Bytes is 100 for example.
    // gdb SIGTRAP (Trace/breakpoint trap)
    // valarray constructor _M_data(__valarray_get_storage<_Tp>(_M_size))
    // return static_cast<_Tp*>(operator new(__n * sizeof(_Tp))); -> malloc -> RtlAllocateHeap -> EtwLogTraceEvent ->
    // RtlRegisterSecureMemoryCacheCallback -> RtlAllocateHeap -> .misaligned_access -> RtlIsZeroMemory
    // Return code -1073740940 (0xC0000374) (STATUS_HEAP_CORRUPTION)
    // Transpose num_wl * num_layers to num_layers * num_wl
    std::valarray<std::complex<T>> delta = kz_list * va_2d_transpose(compvec_d_list, num_wl);
    // std::slice_array does not have std::begin() or std::end().
    std::ranges::transform(std::begin(delta) + num_wl, std::begin(delta) + (num_layers - 1) * num_wl, std::begin(delta) + num_wl, [](const std::complex<T> delta_i) {
        return delta_i.imag() > 100 ? delta_i.real() + 100i : delta_i;
    });
    std::vector<std::vector<std::valarray<std::complex<T>>>> t_list(num_layers, std::vector<std::valarray<std::complex<T>>>(num_layers, std::valarray<std::complex<T>>(num_wl)));
    std::vector<std::vector<std::valarray<std::complex<T>>>> r_list(num_layers, std::vector<std::valarray<std::complex<T>>>(num_layers, std::valarray<std::complex<T>>(num_wl)));
    for (std::size_t i = 0; i < num_layers - 1; i++) {
#ifdef _MSC_VER
        t_list.at(i).at(i + 1) = interface_t(pol, n_list[std::slice(i * num_wl, num_wl, 1)],
                                             n_list[std::slice((i + 1) * num_wl, num_wl, 1)],
                                             th_list[std::slice(i * num_wl, num_wl, 1)],
                                             th_list[std::slice((i + 1) * num_wl, num_wl, 1)]);
        r_list.at(i).at(i + 1) = interface_r(pol, n_list[std::slice(i * num_wl, num_wl, 1)],
                                             n_list[std::slice((i + 1) * num_wl, num_wl, 1)],
                                             th_list[std::slice(i * num_wl, num_wl, 1)],
                                             th_list[std::slice((i + 1) * num_wl, num_wl, 1)]);
#else
        t_list.at(i).at(i + 1) = interface_t(pol, std::valarray<std::complex<T>>(n_list[std::slice(i * num_wl, num_wl, 1)]),
                                             std::valarray<std::complex<T>>(n_list[std::slice((i + 1) * num_wl, num_wl, 1)]),
                                             std::valarray<std::complex<T>>(th_list[std::slice(i * num_wl, num_wl, 1)]),
                                             std::valarray<std::complex<T>>(th_list[std::slice((i + 1) * num_wl, num_wl, 1)]));
        r_list.at(i).at(i + 1) = interface_r(pol, std::valarray<std::complex<T>>(n_list[std::slice(i * num_wl, num_wl, 1)]),
                                             std::valarray<std::complex<T>>(n_list[std::slice((i + 1) * num_wl, num_wl, 1)]),
                                             std::valarray<std::complex<T>>(th_list[std::slice(i * num_wl, num_wl, 1)]),
                                             std::valarray<std::complex<T>>(th_list[std::slice((i + 1) * num_wl, num_wl, 1)]));
#endif
    }
    // boost::numeric::ublas::tensor<std::complex<T>, boost::numeric::ublas::last_order> M_list(boost::numeric::ublas::shape{num_layers, num_wl, 2, 2});
    // boost 1.74.0 or to 1.81.0 does not accept the following code:
    // /usr/include/boost/numeric/ublas/storage.hpp:79:30: error: ‘class std::allocator<std::complex<double> >’ has no member named ‘construct’
    //   79 |                       alloc_.construct(d, value_type());
    //      |                       ~~~~~~~^~~~~~~~~
    // We have to initialize by zero_matrix if not setting every value.
    // The default matrix constructor does not initialize to 0 + 0i but around 1e-6 + 1e-6i
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> M_list(boost::numeric::ublas::zero_matrix<std::complex<T>>(2, 2), num_elems);
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
            A[j](0, 0) = std::exp(-1i * delta[i * num_wl + j]);
            A[j](0, 1) = 0;
            A[j](1, 0) = 0;
            A[j](1, 1) = std::exp(1i * delta[i * num_wl + j]);
            B[j](0, 0) = 1;
            B[j](0, 1) = r_list.at(i).at(i + 1)[j];
            B[j](1, 0) = r_list.at(i).at(i + 1)[j];
            B[j](1, 1) = 1;
            M_list[i * num_wl + j] = boost::numeric::ublas::prod(A[j], B[j]) / t_list.at(i).at(i + 1)[j];
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
            Mtilde[j] = boost::numeric::ublas::prod(Mtilde[j], M_list[i * num_wl + j]);
        }
    }
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> A(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        A[i] <<= 1                    , r_list.at(0).at(1)[i],
                 r_list.at(0).at(1)[i], 1;
        Mtilde[i] = boost::numeric::ublas::prod(A[i], Mtilde[i]) / t_list.at(0).at(1)[i];
    }
    std::valarray<std::complex<T>> r(num_wl);
    std::valarray<std::complex<T>> t(num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        r[i] = Mtilde[i](1, 0) / Mtilde[i](0, 0);
        t[i] = 1.0 / Mtilde[i](0, 0);
    }
    // boost::multi_array<std::complex<T>, 3> vw_list(boost::extents[num_layers][num_wl][2]);
    // It is not necessary to use <boost/multi_array.hpp> here.
    std::valarray<std::vector<std::array<std::complex<T>, 2>>> vw_list(std::vector<std::array<std::complex<T>, 2>>(num_wl), num_layers);
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> vw(boost::numeric::ublas::zero_matrix<std::complex<T>>(2, 2), num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        vw[i](0, 0) = t[i];
        vw[i](0, 1) = t[i];
    }
    for (std::size_t i = 0; i < num_wl; i++) {
        vw_list[num_layers - 1].at(i).at(0) = vw[i](0, 0);
        vw_list[num_layers - 1].at(i).at(1) = vw[i](0, 1);
    }
    for (std::size_t i = num_layers - 2; i > 0; --i) {
        for (std::size_t j = 0; j < num_wl; j++) {
            vw[j] = boost::numeric::ublas::prod(M_list[i * num_wl + j], vw[j]);
            vw_list[i].at(j).at(0) = vw[j](0, 1);
            vw_list[i].at(j).at(1) = vw[j](1, 1);
        }
    }
    // It should be better if using plain for-loop.
    // boost::multi_array<std::complex<T>, 1> ones(boost::extents[num_wl]);
    // std::ranges::fill(ones, 1);
    // vw_list[boost::indices[num_layers - 1][boost::multi_array_types::index_range()][1]] = ones;
    std::ranges::transform(vw_list[num_layers - 1], vw_list[num_layers - 1].begin(), [](std::array<std::complex<T>, 2> &subarray) -> std::array<std::complex<T>, 2> {
        subarray.at(1) = 0;
        return subarray;
    });
    const std::valarray<T> R = R_from_r(r);
    // libstdc++/libc++ replacement types do not match `const std::valarray<T> &`
    // libstdc++:
    // _Expr<_SClos<_ValArray, _Tp>, _Tp>, _Expr<_GClos<_ValArray, _Tp>, _Tp>, _Expr<_IClos<_ValArray, _Tp>, _Tp>
    // libc++: __val_expr<__slice_expr<const valarray&> >, __val_expr<__indirect_expr<const valarray&> >,
    // __val_expr<__mask_expr<const valarray&> >, __val_expr<__indirect_expr<const valarray&> >
    // See https://en.cppreference.com/w/cpp/numeric/valarray/operator_at and
    // Working Draft, Standard for Programming Language C++ (N4950)
    // See https://github.com/llvm/llvm-project/issues/76450 and https://gcc.gnu.org/bugzilla/show_bug.cgi?id=113160
#ifdef _MSC_VER
    const std::valarray<T> Tr = T_from_t(pol, t, n_list[std::slice(0, num_wl, 1)],
                                         n_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)],
                                         th_0,
                                         th_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)]);
    const std::valarray<T> power_entering = power_entering_from_r(pol, r, n_list[std::slice(0, num_wl, 1)], th_0);
#else
    const std::valarray<T> Tr = T_from_t(pol, t, std::valarray<std::complex<T>>(n_list[std::slice(0, num_wl, 1)]),
                                         std::valarray<std::complex<T>>(n_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)]),
                                         th_0,
                                         std::valarray<std::complex<T>>(th_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)]));
    const std::valarray<T> power_entering = power_entering_from_r(pol, r, std::valarray<std::complex<T>>(n_list[std::slice(0, num_wl, 1)]), th_0);
#endif
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
                      const std::valarray<double> &d_list, const std::complex<double> &th_0,
                      const std::valarray<double> &lam_vac) -> coh_tmm_vec_dict<double>;

template<std::floating_point T>
auto coh_tmm_reverse(const char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                     const std::complex<T> th_0, const std::valarray<T> &lam_vac) -> coh_tmm_vec_dict<T> {
    const std::size_t num_wl = lam_vac.size();
    const std::size_t num_layers = d_list.size();
#ifdef _MSC_VER
    const std::valarray<std::complex<T>> th_f = snell(n_list[std::slice(0, num_wl, 1)],
                                                n_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)],
                                                th_0, num_wl);
#else
    const std::valarray<std::complex<T>> th_f = snell(std::valarray<std::complex<T>>(n_list[std::slice(0, num_wl, 1)]),
                                                      std::valarray<std::complex<T>>(n_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)]),
                                                      th_0, num_wl);
#endif
    std::valarray<std::complex<T>> reversed_n_list(n_list.size());
    std::complex<T> *rev_nl_it = std::begin(reversed_n_list);
    // General Method: use views and iterators.
    for (const auto &row : n_list | std::views::chunk(num_wl) | std::views::reverse) {
        std::ranges::move(row, rev_nl_it);
        std::ranges::advance(rev_nl_it, num_wl);
    }
    // Valarray Method: use slices and indices.
    // [Pseudocode]
    // reversed_n_list[std::slice((num_layers - 1 - i) * num_wl, num_wl, 1)] = n_list[std::slice(i * num_wl, num_wl, 1)];
    std::valarray<T> reversed_d_list(d_list.size());
    std::ranges::reverse_copy(d_list, std::begin(reversed_d_list));
    return coh_tmm(pol, reversed_n_list, reversed_d_list, th_f, lam_vac);
}

template auto coh_tmm_reverse(const char pol, const std::valarray<std::complex<double>> &n_list,
                              const std::valarray<double> &d_list, const std::complex<double> th_0,
                              const std::valarray<double> &lam_vac) -> coh_tmm_vec_dict<double>;

template<typename T>
auto ellips(const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list, const std::complex<T> th_0,
            const std::valarray<T> &lam_vac) -> std::unordered_map<std::string, std::valarray<T>> {
    const coh_tmm_vec_dict<T> s_data = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const coh_tmm_vec_dict<T> p_data = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    const std::valarray<std::complex<T>> rs = std::get<std::valarray<std::complex<T>>>(s_data.at("r"));
    const std::valarray<std::complex<T>> rp = std::get<std::valarray<std::complex<T>>>(p_data.at("r"));
    std::valarray<T> psi(rs.size());
    std::valarray<T> Delta(rs.size());
    std::ranges::transform(rs, rp, std::begin(psi), [](const std::complex<T> rs_v, const std::complex<T> rp_v) -> T {
        return std::atan(std::abs(rp_v / rs_v));
    });
    std::ranges::transform(rs, rp, std::begin(Delta), [](const std::complex<T> rs_v, const std::complex<T> rp_v) -> T {
        return std::arg(-rp_v / rs_v);
    });
    return {{"psi", psi}, {"Delta", Delta}};
}

template auto ellips(const std::valarray<std::complex<double>> &n_list, const std::valarray<double> &d_list,
                     const std::complex<double> th_0,
                     const std::valarray<double> &lam_vac) -> std::unordered_map<std::string, std::valarray<double>>;

/*
 * This function is vectorized.
 * Calculates reflected and transmitted power for unpolarized light.
 */
template<typename T>
auto unpolarized_RT(const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                    const std::complex<T> th_0,
                    const std::valarray<T> &lam_vac) -> std::unordered_map<std::string, std::valarray<T>> {
    const coh_tmm_vec_dict<T> s_data = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const coh_tmm_vec_dict<T> p_data = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    const std::valarray<T> R = (std::get<std::valarray<T>>(s_data.at("R")) + std::get<std::valarray<T>>(p_data.at("R"))) / 2;
    const std::valarray<T> Tr = (std::get<std::valarray<T>>(s_data.at("T")) + std::get<std::valarray<T>>(p_data.at("T"))) / 2;
    return {{"R", R}, {"T", Tr}};
}

template auto unpolarized_RT(const std::valarray<std::complex<double>> &n_list, const std::valarray<double> &d_list,
                             const std::complex<double> th_0,
                             const std::valarray<double> &lam_vac) -> std::unordered_map<std::string, std::valarray<double>>;

/*
 * This function is vectorized.
 * Starting with output of coh_tmm(), calculate the Poynting vector
 * and absorbed energy density a distance "dist" into layer number "layer"
 */
template<typename T>
auto position_resolved(const std::valarray<std::size_t> &layer, const std::valarray<T> &distance,
                       const coh_tmm_vec_dict<T> &coh_tmm_data) -> std::unordered_map<std::string, std::variant<std::valarray<T>, std::valarray<std::complex<T>>>> {
    const std::size_t num_layers = layer.size();
    boost::numeric::ublas::matrix<T> vw_mat;
    std::valarray<std::vector<std::array<std::complex<T>, 2>>> vw_list = std::get<std::valarray<std::vector<std::array<std::complex<T>, 2>>>>(coh_tmm_data.at("vw_list"));
    const std::size_t num_wl = vw_list[0].size();
    const std::valarray<std::vector<std::array<std::complex<T>, 2>>> vw_list_l = vw_list[layer];
    std::valarray<std::complex<T>> v(1, num_layers * num_wl);
    std::valarray<std::complex<T>> w(num_layers * num_wl);
    std::ranges::move(std::views::repeat(std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("r")), num_layers) | std::views::join, std::begin(w));
    for (std::size_t i = 0; i < num_layers; i++) {
        if (layer[i] > 0) {
            for (std::size_t j = 0; j < num_wl; j++) {
                v[i * num_wl + j] = vw_list_l[i].at(j).at(0);
                w[i * num_wl + j] = vw_list_l[i].at(j).at(1);
            }
        }
    }
    const std::valarray<std::complex<T>> kz_list = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("kz_list"));
#ifdef _MSC_VER
    auto kz_view = layer * num_wl |
#else
    auto kz_view = std::valarray(layer * num_wl) |
#endif
                   std::views::transform([&kz_list, num_wl](const std::size_t layer_index) {
                       return std::ranges::subrange(std::begin(kz_list) + layer_index, std::begin(kz_list) + layer_index + num_wl);
                   }) | std::views::join;
    std::valarray<std::complex<T>> kz(num_layers * num_wl);
    std::ranges::move(kz_view, std::begin(kz));
    const std::valarray<std::complex<T>> th_list = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("th_list"));
#ifdef _MSC_VER
    auto th_view = layer * num_wl |
#else
    auto th_view = std::valarray(layer * num_wl) |
#endif
                   std::views::transform([&th_list, num_wl](const std::size_t layer_index) {
                       return std::ranges::subrange(std::begin(th_list) + layer_index, std::begin(th_list) + layer_index + num_wl);
                   }) | std::views::join;
    std::valarray<std::complex<T>> th(num_layers * num_wl);
    std::ranges::move(th_view, std::begin(th));
    const std::valarray<std::complex<T>> n_list = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("n_list"));
#ifdef _MSC_VER
    auto n_view = layer * num_wl |
#else
    auto n_view = std::valarray(layer * num_wl) |
#endif
                   std::views::transform([&n_list, num_wl](const std::size_t layer_index) {
                       return std::ranges::subrange(std::begin(n_list) + layer_index, std::begin(n_list) + layer_index + num_wl);
                   }) | std::views::join;
    std::valarray<std::complex<T>> n(num_layers * num_wl);
    std::ranges::move(n_view, std::begin(n));
    const std::valarray<std::complex<T>> n_0 = n_list[std::slice(0, num_wl, 1)];
    const std::complex<T> th_0 = std::get<std::complex<T>>(coh_tmm_data.at("th_0"));
    const char pol = std::get<char>(coh_tmm_data.at("pol"));
    // std::ranges::any_of(layer, std::bind_front(std::greater_equal<>(), 1)) or
    // std::ranges::any_of(layer, std::bind_front(std::greater<>(), 0)) or
    std::valarray<bool> cond = (layer < 1 or 0 > distance or distance > std::get<std::valarray<T>>(coh_tmm_data.at("d_list"))[layer]) and (layer not_eq 0 or distance > 0);
    bool *pos = std::ranges::find(cond, true);
    // If std::cend(cond): Substitution failed: expression ::std::end(_Cont) is ill-formed.
    // error C2672: “std::cend”: 未找到匹配的重载函数
    // MSVC's <xutility> cbegin() and cend() requires const _Container& _Cont
    // while begin() and end() requires _Ty (&_Array)[_Size].
    // libstdc++ begin() and end(): valarray<_Tp>& or const valarray<_Tp>&
    // However, libstdc++ can accept std::cbegin(cond) or std::cend(cond),
    // although the C++ standard does not list the two non-member functions.
    if (pos not_eq std::end(cond)) {
        throw std::runtime_error("Position cannot be resolved at layer " + std::to_string(std::ranges::distance(std::begin(cond), pos)));
    }
    std::valarray<std::complex<T>> comp_dist(num_layers * num_wl);
    std::ranges::move(std::views::repeat(distance | std::views::transform([](const T real) -> std::complex<T> {
        return real;
    }), num_wl) | std::views::join, std::begin(comp_dist));
    comp_dist = va_2d_transpose(comp_dist, num_wl);
    const std::valarray<std::complex<T>> Ef = v * std::exp(1i * kz * comp_dist);
    const std::valarray<std::complex<T>> Eb = w * std::exp(-1i * kz * comp_dist);
    std::valarray<T> poyn(num_layers * num_wl);
    if (pol == 's') {
        for (std::size_t i = 0; i < num_layers; i++) {
            for (std::size_t j = 0; j < num_wl; j++) {
                poyn[i * num_wl + j] = (n[i * num_wl + j] * std::cos(th[i * num_wl + j]) *
                                        std::conj(Ef[i * num_wl + j] + Eb[i * num_wl + j]) *
                                        (Ef[i * num_wl + j] - Eb[i * num_wl + j])).real() /
                                       (n_0[j] * std::cos(th_0)).real();
            }
        }
    } else if (pol == 'p') {
        for (std::size_t i = 0; i < num_layers; i++) {
            for (std::size_t j = 0; j < num_wl; j++) {
                poyn[i * num_wl + j] = (n[i * num_wl + j] * std::conj(std::cos(th[i * num_wl + j])) *
                                        (Ef[i * num_wl + j] + Eb[i * num_wl + j]) *
                                        std::conj(Ef[i * num_wl + j] - Eb[i * num_wl + j])).real() /
                                       (n_0[j] * std::conj(std::cos(th_0))).real();
            }
        }
    }
    std::valarray<T> absor(num_layers * num_wl);
    if (pol == 's') {
        for (std::size_t i = 0; i < num_layers; i++) {
            for (std::size_t j = 0; j < num_wl; j++) {
                absor[i * num_wl + j] = (n[i * num_wl + j] * std::cos(th[i * num_wl + j]) *
                                         kz[i * num_wl + j] * std::norm(Ef[i * num_wl + j] + Eb[i * num_wl + j])).imag() /
                                        (n_0[j] * std::cos(th_0)).real();
            }
        }
    } else if (pol == 'p') {
        for (std::size_t i = 0; i < num_layers; i++) {
            for (std::size_t j = 0; j < num_wl; j++) {
                absor[i * num_wl + j] = (n[i * num_wl + j] * std::conj(std::cos(th[i * num_wl + j])) *
                                         (kz[i * num_wl + j] * std::norm(Ef[i * num_wl + j] - Eb[i * num_wl + j]) -
                                          std::conj(kz[i * num_wl + j]) * std::norm(Ef[i * num_wl + j] + Eb[i * num_wl + j]))).imag() /
                                        (n_0[j] * std::conj(std::cos(th_0))).real();
            }
        }
    }
    const std::valarray<std::complex<T>> Ex = pol == 's' ? std::valarray<std::complex<T>>{0} : (Ef - Eb) * std::cos(th);
    const std::valarray<std::complex<T>> Ey = pol == 's' ? Ef + Eb : std::valarray<std::complex<T>>{0};
    const std::valarray<std::complex<T>> Ez = pol == 's' ? std::valarray<std::complex<T>>{0} : -(Ef + Eb) * std::sin(th);
    return {{"poyn", poyn}, {"absor", absor}, {"Ex", Ex}, {"Ey", Ey}, {"Ez", Ez}};
}

template auto position_resolved(const std::valarray<std::size_t> &layer, const std::valarray<double> &distance,
                                const coh_tmm_vec_dict<double> &coh_tmm_data) -> std::unordered_map<std::string, std::variant<std::valarray<double>, std::valarray<std::complex<double>>>>;

/*
 * This function is vectorized.
 * d_list is a list of thicknesses of layers, all of which are finite.

 * distance is the distance from the front of the whole multilayer structure
 * (i.e., from the start of layer 0.)

 * Function returns [layer, z], where:

 * - layer is what number layer you are at.
 * - z is the distance into that layer.

 * For large distance, layer = len(d_list), even though d_list[layer] does not
 * exist in this case.
 * For negative distance, return [-1, distance]
 */
template<typename T>
auto find_in_structure(const std::valarray<T> &d_list,
                       const std::vector<T> &dist) -> std::pair<std::valarray<typename std::iterator_traits<T *>::difference_type>, std::vector<T>> {
    if (std::isinf(std::abs(d_list.sum()))) {
        throw std::runtime_error("This function expects finite arguments");
    }
    // distance >= 0 because d_list and dist have different sizes!
    if (std::ranges::any_of(dist, std::bind_front(std::greater<>(), 0))) {
        throw std::runtime_error("Any dist < 0. Should return [-1, distance].");
    }
    const std::size_t dlist_size = d_list.size();
    const std::size_t dist_size = dist.size();
    std::valarray<std::size_t> layers(dlist_size);
    // See N4950 Page 1145 Chapter 26.5.7.2 Range conversions ranges::to [range.utility.conv.to]
    // > The range conversion functions construct an object (usually a container) from a range, by using a constructor
    // taking a range, a from_range_t tagged constructor, or a constructor taking a pair of iterators, or by inserting
    // each element of the range into the default-constructed object.
    // Since GCC/libstdc++ 14.0, std::ranges::to() is available.
    // using C = std::valarray<double>; (or std::vector<double>)
    // using R = std::ranges::take_view<std::ranges::ref_view<C>>;  (or transform_view, etc.)
    // (or std::ranges::take_view<std::ranges::ref_view<std::vector<double, std::allocator<double>>>>)
    // (2.1) First see if the non-recursive case applies -- the conversion target is either:
    // - a range with a convertible value type;
    // - a non-range type which might support being created from the input argument(s) (e.g. an `optional`).
    // C does satisfy std::ranges::input_range, but
    // std::convertible_to<std::ranges::range_reference_t<R>, std::ranges::range_value_t<C>> is true.
    // i.e. !std::ranges::input_range<Container> || std::convertible_to<std::ranges::range_reference_t<Range>, std::ranges::range_value_t<Container>> is false.
    // (2.1.1) Case 1 -- construct directly from the given range.
    // C(std::forward<R>(r), std::forward<Args>(args)...)
    // std::constructible_from<C, R, Args...> is false.
    // If we write va | std::ranges::to<std::valarray<double>>() then it is true.
    // (2.1.2) Case 2 -- construct using the `from_range_t` tagged constructor.
    // C(from_range, std::forward<R>(r), std::forward<Args>(args)...)
    // std::constructible_from<C, std::from_range_t, R, Args...> is true for vector, false for valarray, because
    // std::vector<T, Alloator> constructor etc. use std::from_range_t but std::valarray<T> constructor does not.
    // See https://en.cppreference.com/w/cpp/ranges/from_range.
    // (2.1.3) Case 3 -- construct from a begin-end iterator pair.
    // C(ranges::begin(r), ranges::end(r), std::forward<Args>(args)...)
    // template <class Container, class Range, class... Args>
    // concept constructible_from_iter_pair =
    // std::ranges::common_range<Range> &&  // (2.1.3.1)
    // requires { typename std::iterator_traits<std::ranges::iterator_t<Range>>::iterator_category; } &&
    // std::derived_from<typename std::iterator_traits<std::ranges::iterator_t<Range>>::iterator_category, std::input_iterator_tag> &&  // (2.1.3.2)
    // std::constructible_from<Container, std::ranges::iterator_t<Range>, std::ranges::sentinel_t<Range>, Args...>;  // (2.1.3.3)
    // std::constructible_from<C, from_range_t, R, _Args...> is true for vector, false for valarray.
    // (2.1.4) Case 4 -- default-construct (or construct from the extra arguments) and insert, reserving the size if possible.
    // C c(std::forward<Args>(args)...);
    // if constexpr (sized_range<R> && reservable-container <C>)
    // c.reserve(static_cast<range_size_t<C>>(ranges::size(r)));
    // ranges::copy(r, container-inserter <range_reference_t<R>>(c));
    // template <class Container, class Ref>
    // constexpr bool container_insertable = requires(Container& c, Ref&& ref) {
    //     requires(
    //     requires { c.push_back(std::forward<Ref>(ref)); } ||
    //     requires { c.insert(c.end(), std::forward<Ref>(ref)); });
    // };
    // std::constructible_from<C, Args> && container_insertable<C, std::ranges::range_reference_t<R>> is true for vector, false for valarray:
    // (gcc 14.0.0) error: 'class std::valarray<double>' has no member named 'insert'
    // (gcc 14.0.0) error: 'class std::valarray<double>' has no member named 'end'
    // (2.2) Try the recursive case.
    // std::ranges::input_range<std::ranges::range_reference_t<R> is false.
    // Also see <range/v3/range/conversion.hpp>
    // ranges::detail::convertible_to_cont<R, C> is true for vector, false for valarray.
    // - ranges::detail::range_and_not_view<C> is true
    // - std::move_constructible<C> is true (move_constructible = constructible_from<T, T> && convertible_to<T, T>)
    // - ranges::detail::convertible_to_cont_impl_concept<R, C> is true for vector, false for valarray.
    // -- std::constructible_from<std::ranges::range_value_t<R>, std::ranges::range_reference_t<C>>
    // -- std::constructible_from<C, ranges::detail::range_cpp17_iterator_t<R>, ranges::detail::range_cpp17_iterator_t<R>> is true for vector, false for valarray.
    // See https://github.com/ericniebler/range-v3/issues/1803 and https://eel.is/c++draft/range.utility.conv.
    // For linear algebra support to the C++ standard library, see [P1385](https://wg21.link/P1385R7)
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1385r7.pdf
    // https://github.com/cplusplus/papers/issues/169

    // Reference implementation on GitHub: https://github.com/BobSteagall/wg21
    // on Conan: https://conan.io/center/recipes/wg21-linear_algebra.
    // It aims to interface [P1673R13](https://wg21.link/p1673)
    // [A free function linear algebra interface based on the BLAS](https://isocpp.org/files/papers/P1673R13.html)
    // Reference implementation on GitHub: https://github.com/kokkos/stdBLAS
    // Hope we can use the feature test macro __cpp_lib_linalg = 202311L in the near future.
    std::vector<T> d_array(dlist_size + 1, 0);
    std::ranges::move(d_list, d_array.begin() + 1);
    std::valarray<T> cum_sum(dlist_size + 1);
    std::partial_sum(d_array.cbegin(), d_array.cend(), std::begin(cum_sum));
    std::valarray<typename std::iterator_traits<T *>::difference_type> layer(dist_size);
    std::vector<T> distance = dist;
    // dist -= cum_sum[layer - 1];
    for (std::size_t i = 0; i < dist_size; i++) {
        // lower_bound for searchsorted left
        // upper_bound for searchsorted right
        const T *it = std::ranges::upper_bound(cum_sum, dist.at(i));
        if (dist.at(i) < 0) {
            layer[i] = -1;
        } else {
            layer[i] = std::ranges::distance(std::begin(cum_sum), it);
            distance.at(i) -= layer[i] == 0 ? cum_sum[dlist_size] : cum_sum[layer[i] - 1];
        }
    }
    return std::pair(layer, distance);
}

template auto find_in_structure(const std::valarray<double> &d_list,
                                const std::vector<double> &dist) -> std::pair<std::valarray<std::iterator_traits<double *>::difference_type>, std::vector<double>>;

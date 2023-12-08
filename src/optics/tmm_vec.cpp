//
// Created by Yihua on 2023/11/27.
//

// GCC has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
#include <algorithm>
#include <numbers>
#include "FixedMatrix.h"
#include "tmm.h"
#include "utils.h"

template<typename T>
auto interface_r(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::valarray<std::complex<T>> &th_i,
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
auto interface_t(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::valarray<std::complex<T>> &th_i,
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
auto interface_R(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::valarray<std::complex<T>> &th_i, const std::valarray<std::complex<T>> &th_f) -> T {
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
    // An alternative equivalent way
    std::replace_if(std::begin(r), std::end(r), [&th_i, i = 0](const int &) mutable {
        return th_i[i++].real() > std::numbers::pi_v<T> / 2 - 1e-6;
    }, 1);

    return R_from_r(r);
}

template<typename T>
auto interface_T(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<T> {
    std::valarray<std::complex<T>> t = interface_t(polarization, n_i, n_f, th_i, th_f);
    // If the incoming angle is pi/2, that means (most likely) that the light was previously
    // totally internally reflected. That means the light will never reach this interface, and
    // we can safely set t = T = 0; otherwise we get numerical issues which give unphysically large
    // values of T because in T_from_t we divide by cos(th_i) which is ~ 0.
    std::transform(std::cbegin(t), std::cend(t), std::begin(th_i), std::begin(t),
                   [](const std::complex<T> t_value, const std::complex<T> th_i_value) {
                       return (th_i_value.real() > std::numbers::pi_v<T> / 2 - 1e-6) ? 0 : t_value;
                   });

    return T_from_t(polarization, t, n_i, n_f, th_i, th_f);
}

template<typename T>
auto coh_tmm(const char pol, const std::valarray<std::valarray<std::complex<T>>> &n_list, const std::vector<T> &d_list,
             const std::complex<T> th_0, const T lam_vac) -> stack_coh_tmm_dict<T> {
    if (n_list.size() not_eq d_list.size()) {
        throw std::logic_error("n_list and d_list must have same length");
    }
    if (not std::isinf(d_list[0]) or not std::isinf(d_list[d_list.size() - 1])) {
        throw std::runtime_error("d_list must start and end with inf!");
    }
    if (std::abs((n_list[0] * std::sin(th_0)).imag()) >= TOL * EPSILON<T> or not is_forward_angle(n_list[0], th_0)) {
        throw std::runtime_error("Error in n0 or th0!");
    }
    const std::size_t num_layers = n_list.size();
    const std::valarray<std::complex<T>> th_list = list_snell(n_list, th_0);
    const std::valarray<std::complex<T>> kz_list = 2 * std::numbers::pi_v<T> * n_list * std::cos(th_list) / lam_vac;
    std::valarray<std::complex<T>> d_list_comp_temp(d_list.size());
    std::transform(std::begin(d_list), std::end(d_list), std::begin(d_list_comp_temp), [](const T real) {
        return std::complex<T>(real, 0.0);
    });
    std::valarray<std::complex<T>> delta = kz_list * d_list_comp_temp;
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        if (delta[i].imag() > 35) {
            delta[i] = std::complex<T>(delta[i].real(), 35);
            try {
                throw std::runtime_error("Warning: Layers that are almost perfectly opaque "
                                         "are modified to be slightly transmissive, "
                                         "allowing 1 photon in 10^30 to pass through. It's "
                                         "for numerical stability. This warning will not "
                                         "be shown again.");
            } catch (const std::runtime_error &coh_value_warning) {
                // Do nothing.
            }
        }
    }
    std::vector<std::vector<std::complex<T>>> t_list(num_layers, std::vector<std::complex<T>>(num_layers));
    std::vector<std::vector<std::complex<T>>> r_list(num_layers, std::vector<std::complex<T>>(num_layers));
    for (std::size_t i = 0; i < num_layers - 1; i++) {
        t_list[i][i + 1] = interface_t(pol, n_list[i], n_list[i + 1], th_list[i], th_list[i + 1]);
        r_list[i][i + 1] = interface_r(pol, n_list[i], n_list[i + 1], th_list[i], th_list[i + 1]);
    }
    std::vector<FixedMatrix<std::complex<T>, 2, 2>> M_list(num_layers, FixedMatrix<std::complex<T>, 2, 2>());
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        M_list[i] = dot(FixedMatrix<std::complex<T>, 2, 2>({{std::exp(std::complex<T>(0, -1) * delta[i]), 0}, {0, std::exp(std::complex<T>(0, 1) * delta[i])}}),
                        FixedMatrix<std::complex<T>, 2, 2>({{1, r_list[i][i + 1]}, {r_list[i][i + 1], 1}})) / t_list[i][i + 1];
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

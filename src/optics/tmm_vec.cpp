//
// Created by Yihua Liu on 2023/11/27.
//

// GCC/Clang has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
// If valarray operations are not needed, then just use vectors, because vectors have at() method.
#include <algorithm>
#include <functional>
#include <numbers>
#include <numeric>
#ifdef _MSC_VER  // Silence the warning from boost uBLAS
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#endif
// https://github.com/boostorg/ublas/issue/95 C++20: 'class std::allocator' has no member named 'construct'
// https://github.com/boostorg/ublas/pull/103 Support the C++ allocator model
#include <boost/numeric/ublas/assignment.hpp>  // operator<<=
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include "tmm.h"
#include "src/utils/Math.h"
#include "src/utils/Range.h"

using namespace std::complex_literals;

template<typename T>
AbsorpAnalyticVecFn<T>::AbsorpAnalyticVecFn(const AbsorpAnalyticVecFn &other) : A3(other.A3), a1(other.a1),
                                                                                   a3(other.a3), A1(other.A1),
                                                                                   A2(other.A2), d(other.d) {}

template<typename T>
void AbsorpAnalyticVecFn<T>::fill_in(const coh_tmm_vec_dict<T> &coh_tmm_data, const std::valarray<std::ptrdiff_t> &f_layer) {
    const char pol = std::get<char>(coh_tmm_data.at("pol"));
    const std::size_t num_layers = f_layer.size();
    const std::valarray<std::vector<std::array<std::complex<T>, 2>>> vw_list = std::get<std::valarray<std::vector<std::array<std::complex<T>, 2>>>>(coh_tmm_data.at("vw_list"));
    std::valarray<std::size_t> layer(num_layers);
    // layer = f_layer % num_layers
    std::ranges::transform(f_layer, std::begin(layer), [num_layers](const std::ptrdiff_t li) -> std::size_t {
        return (num_layers + li % num_layers) % num_layers;
    });
    const std::size_t num_wl = vw_list[0].size();
    const std::size_t num_elems = num_layers * num_wl;
    std::valarray<std::size_t> l_indices(num_elems);
    // Do not confuse with std::views::slide!
    // If we write std::views::zip(std::views::repeat(std::views::iota(0U, num_wl), num_layers) | std::views::join,
    // then we have to transpose the second view first, which is less convenient; thus,
    // If we write as below, we can just transpose the manipulated l_indices.
#if (defined __cpp_lib_ranges_zip) && (defined __cpp_lib_ranges_repeat)
    std::ranges::move(std::views::transform(std::views::zip(
        std::views::iota(0U, num_layers) | std::views::transform([num_wl](std::size_t x) -> std::ranges::repeat_view<std::size_t, std::size_t> {
                                     return std::views::repeat(x, num_wl);
                                 }) | std::views::join,
#ifdef _MSC_VER
        std::views::repeat(layer * 2, num_wl) | std::views::join),
#else
        std::views::repeat(std::valarray(layer * 2), num_wl) | std::views::join),
#endif
        [](const std::pair<std::size_t const &, std::size_t const &> &pair) -> std::size_t {
            return pair.first + pair.second;
        }), std::begin(l_indices));
#else
    for (std::size_t i = 0; i < num_elems; ++i) {
        l_indices[i] = i / num_wl + 2 * layer[i % num_layers];
    }
#endif
    l_indices = Utils::Range::rng2d_transpose(l_indices, num_layers);
    const std::valarray<std::vector<std::array<std::complex<T>, 2>>> vw_list_l = vw_list[layer];
    std::valarray<std::complex<T>> v(num_layers * num_wl);
    std::valarray<std::complex<T>> w(num_layers * num_wl);
    for (std::size_t i = 0; i < num_layers; i++) {
        if (layer[i] > 0) {
            for (std::size_t j = 0; j < num_wl; j++) {
                v[i * num_wl + j] = vw_list_l[i].at(j).at(0);
                w[i * num_wl + j] = vw_list_l[i].at(j).at(1);
            }
        }
    }
    const std::valarray<std::complex<T>> kz = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("kz_list"))[l_indices];
    const std::valarray<std::complex<T>> n = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("n_list"))[l_indices];
    const std::valarray<std::complex<T>> n_0 = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("n_list"))[std::slice(0, num_wl, 1)];
    const std::complex<T> th_0 = std::get<std::complex<T>>(coh_tmm_data.at("th_0"));
    const std::valarray<std::complex<T>> th = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("th_list"))[l_indices];
    d = std::get<std::valarray<T>>(coh_tmm_data.at("d_list"))[layer];
    a1.resize(num_elems);
    a3.resize(num_elems);
#ifdef _MSC_VER
    std::ranges::move(2 * kz | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)), std::begin(a1));
    std::ranges::move(2 * kz | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::real)), std::begin(a3));
#elif (defined __GNUC__ && __GNUC__ < 13)
    for (auto i = 0; i < kz.size(); ++i) {
        a1[i] = 2 * std::imag(kz[i]);
        a3[i] = 2 * std::real(kz[i]);
    }
#else
    std::ranges::move(std::valarray<std::complex<T>>(2 * kz) | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)), std::begin(a1));
    std::ranges::move(std::valarray<std::complex<T>>(2 * kz) | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::real)), std::begin(a3));
#endif
    a1[a1 < 1e-30] = 0;
    a3[a3 < 1e-30] = 0;
    A1.resize(num_elems);
    A2.resize(num_elems);
    A3.resize(num_elems);
    if (pol == 's') {
        std::valarray<T> temp(num_elems);
        for (std::size_t i = 0; i < num_layers; i++) {
            for (std::size_t j = 0; j < num_wl; j++) {
                temp[i * num_wl + j] = (n[i * num_wl + j] * std::cos(th[i * num_wl + j]) * kz[i * num_wl + j]).imag() / (n_0[j] * std::cos(th_0)).real();
            }
        }
        // Remember that we cannot pass std::views::transform(std::bind_front(std::multiplies<>(), temp)) to the pipeline
        std::ranges::move(w | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::norm)), std::begin(A1));
        A1 *= temp;
        std::ranges::move(v | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::norm)), std::begin(A2));
        A2 *= temp;
        std::ranges::move(w | std::views::transform(std::bind_front<std::complex<T> (*)(const std::complex<T> &)>(std::conj)), std::begin(A3));
        std::valarray<std::complex<T>> c_temp(num_elems);
        std::ranges::move(temp | std::views::transform([](const T real) -> std::complex<T> {
            return real;
        }), std::begin(c_temp));
        A3 *= c_temp;
        A3 *= v;
    } else {  // pol == 'p'
        std::valarray<T> temp(num_elems);
        for (std::size_t i = 0; i < num_layers; i++) {
            for (std::size_t j = 0; j < num_wl; j++) {
                temp[i * num_wl + j] = 2 * kz[i * num_wl + j].imag() * (n[i * num_wl + j] * std::cos(std::conj(th[i * num_wl + j]))).real() / (n_0[j] * std::conj(std::cos(th_0))).real();
            }
        }
        std::ranges::move(w | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::norm)), std::begin(A1));
        A1 *= temp;
        std::ranges::move(v | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::norm)), std::begin(A2));
        A2 *= temp;
        for (std::size_t i = 0; i < num_layers; i++) {
            for (std::size_t j = 0; j < num_wl; j++) {
                A3[i * num_wl + j] = v[i * num_wl + j] * std::conj(w[i * num_wl + j]) * -2.0 * kz[i * num_wl + j].real() * (n[i * num_wl + j] * std::cos(std::conj(th[i * num_wl + j]))).imag() / (n_0[j] * std::conj(std::cos(th_0))).real();
            }
        }
    }
}

template<typename T>
void AbsorpAnalyticVecFn<T>::fill_in(const coh_tmm_vecn_dict<T> &coh_tmm_data, const std::ptrdiff_t f_layer) {
    const char pol = std::get<char>(coh_tmm_data.at("pol"));
    const std::valarray<std::vector<std::array<std::complex<T>, 2>>> vw_list = std::get<std::valarray<std::vector<std::array<std::complex<T>, 2>>>>(coh_tmm_data.at("vw_list"));
    const std::size_t num_wl = vw_list[0].front().size();
    // Warning: Do not directly use operator% like f_layer % vw_list.size()!
    // See https://stackoverflow.com/questions/7594508/why-does-the-modulo-operator-result-in-negative-values
    // std::div(f_layer, vw_list.size()).rem also gives negative results
    // f_layer is signed while num_layers is unsigned!
    const std::ptrdiff_t num_layers = vw_list.size();
    const std::size_t layer = (num_layers + f_layer % num_layers) % num_layers;
    const std::vector<std::array<std::complex<T>, 2>> &vw_list_l = vw_list[layer];
    std::valarray<std::complex<T>> v(num_wl);
    std::valarray<std::complex<T>> w(num_wl);
    if (layer > 0) {
        for (std::size_t i = 0; i < num_wl; i++) {
            v[i] = vw_list_l.at(i).front();
            w[i] = vw_list_l.at(i).back();
        }
    }
    const std::valarray<std::complex<T>> kz = std::get<std::vector<std::valarray<std::complex<T>>>>(coh_tmm_data.at("kz_list")).at(layer);
    const std::valarray<std::complex<T>> n = std::get<std::vector<std::valarray<std::complex<T>>>>(coh_tmm_data.at("n_list")).at(layer);
    const std::valarray<std::complex<T>> th = std::get<std::vector<std::valarray<std::complex<T>>>>(coh_tmm_data.at("th_list")).at(layer);
    d = std::get<std::vector<T>>(coh_tmm_data.at("d_list")).at(layer);
    const std::valarray<std::complex<T>> n_0 = std::get<std::vector<std::valarray<std::complex<T>>>>(coh_tmm_data.at("n_list")).front();
    const std::valarray<std::complex<T>> th_0 = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("th_0"));
    a1.resize(num_wl);
    a3.resize(num_wl);
#ifdef _MSC_VER
    std::ranges::move(2 * kz | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)), std::begin(a1));
    std::ranges::move(2 * kz | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::real)), std::begin(a3));
#elif (defined __GNUC__ && __GNUC__ < 13)  // P2387R3
    for (std::size_t i = 0; i < num_wl; ++i) {
        a1[i] = 2 * std::imag(kz[i]);
        a3[i] = 2 * std::real(kz[i]);
    }
#else
    std::ranges::move(std::valarray<std::complex<T>>(2 * kz) | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)), std::begin(a1));
    std::ranges::move(std::valarray<std::complex<T>>(2 * kz) | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::real)), std::begin(a3));
#endif
    a1[a1 < 1e-30] = 0;
    a3[a3 < 1e-30] = 0;
    A1.resize(num_wl);
    A2.resize(num_wl);
    A3.resize(num_wl);
    if (pol == 's') {
        std::valarray<T> temp(num_wl);
        for (std::size_t i = 0; i < num_wl; i++) {
            temp[i] = (n[i] * std::cos(th[i]) * kz[i]).imag() / (n_0[i] * std::cos(th_0[i])).real();
        }
        std::ranges::move(w | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::norm)), std::begin(A1));
        A1 *= temp;
        std::ranges::move(v | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::norm)), std::begin(A2));
        A2 *= temp;
        std::ranges::move(w | std::views::transform(std::bind_front<std::complex<T> (*)(const std::complex<T> &)>(std::conj)), std::begin(A3));
        std::valarray<std::complex<T>> c_temp(num_wl);
        std::ranges::move(temp | std::views::transform([](const T real) -> std::complex<T> {
            return real;
        }), std::begin(c_temp));
        A3 *= c_temp;
        A3 *= v;
    } else {
        std::valarray<T> temp(num_wl);
            for (std::size_t i = 0; i < num_wl; i++) {
                temp[i] = 2 * kz[i].imag() * (n[i] * std::cos(std::conj(th[i]))).real() / (n_0[i] * std::conj(std::cos(th_0[i]))).real();
            }
        std::ranges::move(w | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::norm)), std::begin(A1));
        A1 *= temp;
        std::ranges::move(v | std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::norm)), std::begin(A2));
        A2 *= temp;
        for (std::size_t i = 0; i < num_wl; i++) {
            A3[i] = v[i] * std::conj(w[i]) * -2.0 * kz[i].real() * (n[i] * std::cos(std::conj(th[i]))).imag() / (n_0[i] * std::conj(std::cos(th_0[i]))).real();
        }
    }
}

template<typename T>
[[nodiscard]] auto AbsorpAnalyticVecFn<T>::run(const T z) const -> std::valarray<std::complex<T>> {
    const std::size_t num_wl = a1.size();
    std::valarray<std::complex<T>> result(num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        result[i] = A1[i] * std::exp(a1[i] * z) + A2[i] * std::exp(-a1[i] * z) + A3[i] * std::exp(1i * a3[i] * z) + std::conj(A3[i]) * std::exp(-1i * a3[i] * z);
    }
    return result;
}

template<typename T>
[[nodiscard]] auto AbsorpAnalyticVecFn<T>::run(const std::valarray<T> &z) const -> std::valarray<std::valarray<std::complex<T>>> {
    const std::size_t num_layers = z.size();
    const std::size_t num_wl = a1.size();
    std::valarray<std::valarray<std::complex<T>>> result(std::valarray<std::complex<T>>(num_wl), num_layers);
    for (std::size_t i = 0; i < num_layers; i++) {
        for (std::size_t j = 0; j < num_wl; j++) {  // result[i][j] not [j][i]!
            result[i][j] = ((A1[j] < 1e-100) ? 0 : A1[j] * std::exp(a1[j] * z[i])) + A2[j] * std::exp(-a1[j] * z[i])
                    + A3[j] * std::exp(1i * a3[j] * z[i]) + std::conj(A3[j]) * std::exp(-1i * a3[j] * z[i]);
        }
    }
    return result;
}

template<typename T>
template<typename FAC_T>
// Note that std::is_same_v takes into account const/volatile qualifications and
// std::remove_cv_ref_t<T> is equivalent to std::remove_cv_t<std::remove_reference_t<T>>
requires std::is_same_v<FAC_T, T> or std::is_same_v<std::remove_cvref_t<FAC_T>, std::valarray<T>>
void AbsorpAnalyticVecFn<T>::scale(FAC_T &&factor) {
    A1 *= factor;
    A2 *= factor;
    if constexpr (std::is_same_v<FAC_T, T>) {
        A3 *= factor;
    } else {
        if (A3.size() not_eq factor.size()) {
            throw std::runtime_error("A3 and factor have different sizes.");
        }
        std::ranges::transform(A3, factor, std::begin(A3), [](const std::complex<T> A3v, const T fac_v) {
            return A3v * fac_v;
        });
    }
    const std::size_t Asz = A1.size();
    std::valarray<bool> infA(Asz);
    std::ranges::transform(A1, std::begin(infA), std::bind_front<bool (*)(const T)>(std::isinf));
    A1[infA] = 0;
    std::ranges::transform(A2, std::begin(infA), std::bind_front<bool (*)(const T)>(std::isinf));
    A2[infA] = 0;
    std::valarray<T> absA3(Asz);
    std::ranges::transform(A3, std::begin(absA3), std::bind_front<T (*)(const std::complex<T> &)>(std::abs));
    std::ranges::transform(absA3, std::begin(infA), std::bind_front<bool (*)(const T)>(std::isinf));
    A3[infA] = 0;
}

template<typename T>
void AbsorpAnalyticVecFn<T>::add(const AbsorpAnalyticVecFn &b) {
    // std::ranges::all_of is unary.
    if (std::ranges::equal(b.a1, a1, [](const T ba1v, const T a1v) -> bool {
        return ba1v != a1v;
    }) or std::ranges::equal(b.a3, a3, [](const T ba1v, const T a1v) -> bool {
        return ba1v != a1v;
    })) {
        throw std::runtime_error("Incompatible absorption analytical functions!");
    }
    A1 += b.A1;
    A2 += b.A2;
    A3 += b.A3;
}

template<typename T>
void AbsorpAnalyticVecFn<T>::flip() {
    if (const T *p_dval = std::get_if<T>(&d)) {
        std::valarray<T> newA1 = A2 * std::exp(-a1 * *p_dval);
        newA1[A2 == 0] = 0;
        std::valarray<T> newA2 = A1 * std::exp(a1 * *p_dval);
        newA2[A1 == 0] = 0;
        A1 = std::move(newA1);
        A2 = std::move(newA2);
        for (std::size_t i = 0; i < A3.size(); i++) {
            A3[i] = std::conj(A3[i] * std::exp(1i * a3[i] * *p_dval));
        }
    } else if (const std::valarray<T> *p_dva = std::get_if<std::valarray<T>>(&d)) {
        std::valarray<T> newA1 = A2 * std::exp(-a1 * *p_dva);
        newA1[A2 == 0] = 0;
        std::valarray<T> newA2 = A1 * std::exp(a1 * *p_dva);
        newA2[A1 == 0] = 0;
        A1 = std::move(newA1);
        A2 = std::move(newA2);
        for (std::size_t i = 0; i < (*p_dva).size(); i++) {
            A3[i] = std::conj(A3[i] * std::exp(1i * a3[i] * (*p_dva)[i]));
        }
    } else {
        throw std::runtime_error("d of AbsorpAnalyticVecFn is neither T nor std::valarray<T>.");
    }
}

template class AbsorpAnalyticVecFn<double>;
template void AbsorpAnalyticVecFn<double>::scale(double &&factor);

template<typename T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto snell(const std::valarray<std::complex<T>> &n_1, const std::valarray<std::complex<T>> &n_2,
           const TH_T &th_1) -> std::valarray<std::complex<T>> {
    const std::size_t num_wl = n_1.size();
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

template<std::floating_point T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto list_snell(const std::vector<std::valarray<std::complex<T>>> &n_list,
                const TH_T &th_0) -> std::vector<std::valarray<std::complex<T>>> {
    const std::size_t num_layers = n_list.size();
    const std::size_t num_wl = n_list.front().size();
    if constexpr (std::is_same_v<TH_T, std::valarray<std::complex<T>>>) {
        if (th_0.size() not_eq num_wl) {
            throw std::runtime_error("n_list elements' size mismatches th_0's size.");
        }
    }
    std::vector<std::valarray<std::complex<T>>> angles(num_layers, std::valarray<std::complex<T>>(num_wl));
    for (std::size_t i : std::views::iota(0U, num_layers)) {
        angles.at(i) = std::asin(n_list.front() * std::sin(th_0) / n_list.at(i));
    }
    for (std::size_t i : std::views::iota(0U, num_wl)) {
        if (not is_forward_angle(n_list.front()[i], angles.front()[i])) {
            angles.front()[i] = std::numbers::pi_v<T> - angles.front()[i];
        }
        if (not is_forward_angle(n_list.back()[i], angles.back()[i])) {
            angles.back()[i] = std::numbers::pi_v<T> - angles.back()[i];
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
                       return th_i_value.real() > std::numbers::pi_v<T> / 2 - 1e-6 ? 0 : t_value;
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
#if (defined __GNUC__ && __GNUC__ < 13)  // P2387R3
    for (std::size_t i = 0; i < num_wl; ++i) {
        if constexpr (std::same_as<TH_T, std::complex<T>>) {
            if (std::abs(std::imag(n_list[i] * std::sin(th_0))) > Utils::Math::TOL * Utils::Math::EPSILON<T>) {
                throw std::invalid_argument("Error in n0 or th0!");
            }
        } else {
            if (std::abs(std::imag(n_list[i] * std::sin(th_0[i]))) > Utils::Math::TOL * Utils::Math::EPSILON<T>) {
                throw std::invalid_argument("Error in n0 or th0!");
            }
        }
    }
#else
#ifdef _MSC_VER
    if (std::ranges::any_of(n_list[std::slice(0, num_wl, 1)] * std::sin(th_0) |
#else
    if (std::ranges::any_of(std::valarray<std::complex<T>>(n_list[std::slice(0, num_wl, 1)] * std::sin(th_0)) |
#endif
        std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)),
            // Note the order of front-binding!
                            std::bind_front(std::less_equal<>(), Utils::Math::TOL * Utils::Math::EPSILON<T>))) {
        throw std::invalid_argument("Error in n0 or th0!");
    }
#endif
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
#ifdef __cpp_lib_ranges_repeat
    std::ranges::move(std::views::repeat(lam_vac | std::views::transform([](const T real) -> std::complex<T> {
        return real;
    }), num_layers) | std::views::join, std::begin(compvec_lam_vac));
#else
    for (std::size_t i = 0; i < num_elems; ++i) {
        compvec_lam_vac[i] = std::real(lam_vac[i % num_wl]);
    }
#endif
    const std::valarray<std::complex<T>> kz_list = 2 * std::numbers::pi_v<T> * n_list * std::cos(th_list) / compvec_lam_vac;
    // Do the same thing to d_list.
    std::valarray<std::complex<T>> compvec_d_list(num_elems);
#ifdef __cpp_lib_ranges_repeat
    std::ranges::move(std::views::repeat(d_list | std::views::transform([](const T real) -> std::complex<T> {
        return real;
    }), num_wl) | std::views::join, std::begin(compvec_d_list));
#else
    for (std::size_t i = 0; i < num_elems; ++i) {
        compvec_d_list[i] = std::real(d_list[i % num_layers]);
    }
#endif
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
    std::valarray<std::complex<T>> delta = kz_list * Utils::Range::rng2d_transpose(compvec_d_list, num_wl);
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
        A[i] <<= 1                          , r_list.at(0).at(1)[i],
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

template auto coh_tmm(char pol, const std::valarray<std::complex<double>> &n_list,
                      const std::valarray<double> &d_list, const std::complex<double> &th_0,
                      const std::valarray<double> &lam_vac) -> coh_tmm_vec_dict<double>;

template<typename T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto coh_tmm(const char pol, const std::vector<std::valarray<std::complex<T>>> &n_list, const std::vector<T> &d_list,
             const TH_T &th_0, const std::valarray<T> &lam_vac) -> coh_tmm_vecn_dict<T> {
    const std::size_t num_wl = lam_vac.size();
    const std::size_t num_layers = n_list.size();
    if constexpr (std::is_same_v<TH_T, std::valarray<std::complex<T>>>) {
        if (th_0.size() not_eq lam_vac.size()) {
            throw std::invalid_argument("th_0 and lam_vac have different sizes;"
                                        "you need to run one angle calculation at a time.");
        }
    }
    if (num_layers not_eq d_list.size()) {
        throw std::invalid_argument("n_list and d_list must have same length");
    }
    if (not std::isinf(d_list.front()) or not std::isinf(d_list.back())) {
        throw std::invalid_argument("d_list must start and end with inf!");
    }
#if (defined __GNUC__ && __GNUC__ < 13)
    std::valarray<T> test_va(num_wl);
    for (std::size_t i = 0; i < num_wl; ++i) {
        if constexpr (std::same_as<TH_T, std::complex<T>>) {
            if (std::abs(std::imag(n_list.front()[i] * std::sin(th_0))) > Utils::Math::TOL * Utils::Math::EPSILON<T>) {
                throw std::invalid_argument("Error in n0 or th0!");
            }
        } else {
            if (std::abs(std::imag(n_list.front()[i] * std::sin(th_0[i]))) > Utils::Math::TOL * Utils::Math::EPSILON<T>) {
                throw std::invalid_argument("Error in n0 or th0!");
            }
        }
    }
#else
#ifdef _MSC_VER
    if (std::ranges::any_of(n_list.front() * std::sin(th_0) |
#else
    if (std::ranges::any_of(std::valarray<std::complex<T>>(n_list.front() * std::sin(th_0)) |
#endif
                            std::views::transform(std::bind_front<T (*)(const std::complex<T> &)>(std::imag)),
            // Note the order of front-binding!
                            std::bind_front(std::less_equal<>(), Utils::Math::TOL * Utils::Math::EPSILON<T>))) {
        throw std::invalid_argument("Error in n0 or th0!");
    }
#endif
    std::vector<std::valarray<std::complex<T>>> th_list = list_snell(n_list, th_0);
    std::valarray<std::complex<T>> comp_lam_vac(num_wl);
    std::ranges::transform(lam_vac, std::begin(comp_lam_vac), [](const T real) -> std::complex<T> {
        return real;
    });
    std::vector<std::valarray<std::complex<T>>> kz_list(num_layers, std::valarray<std::complex<T>>(num_wl));
    for (std::size_t i : std::views::iota(0U, num_layers)) {
        kz_list.at(i) = 2 * std::numbers::pi_v<T> * n_list.at(i) * std::cos(th_list.at(i)) / comp_lam_vac;
    }
    std::valarray<std::complex<T>> comp_d_list(num_layers);
    std::ranges::transform(d_list, std::begin(comp_d_list), [](const T real) -> std::complex<T> {
        return real;
    });
    std::vector<std::valarray<std::complex<T>>> delta(num_layers, std::valarray<std::complex<T>>(num_wl));
    for (std::size_t i = 0; i < num_layers; i++) {
        delta.at(i) = kz_list.at(i) * comp_d_list[i];
    }
    for (std::size_t i : std::views::iota(1U, num_layers - 1)) {
        if (std::ranges::any_of(delta.at(i), [](const std::complex<T> delta_i) -> bool {
            return delta_i.imag() > 35;
        })) {
            std::ranges::transform(delta.at(i), std::begin(delta.at(i)), [](const std::complex<T> delta_i) {
                return delta_i.real() + 35i;
            });
            try {
                throw std::runtime_error("Warning: Layers that are almost perfectly opaque "
                                         "are modified to be slightly transmissive, "
                                         "allowing 1 photon in 10^30 to pass through. It's "
                                         "for numerical stability. This warning will not "
                                         "be shown again.");
            } catch (const std::runtime_error &coh_value_warning) {
                std::cerr << coh_value_warning.what() << '\n';
            }
        }
    }
    std::vector<std::vector<std::valarray<std::complex<T>>>> t_list(num_layers, std::vector<std::valarray<std::complex<T>>>(num_layers, std::valarray<std::complex<T>>(num_wl)));
    std::vector<std::vector<std::valarray<std::complex<T>>>> r_list(num_layers, std::vector<std::valarray<std::complex<T>>>(num_layers, std::valarray<std::complex<T>>(num_wl)));
    for (std::size_t i = 0; i < num_layers - 1; i++) {
        t_list.at(i).at(i + 1) = interface_t(pol, n_list.at(i), n_list.at(i + 1), th_list.at(i), th_list.at(i + 1));
        r_list.at(i).at(i + 1) = interface_r(pol, n_list.at(i), n_list.at(i + 1), th_list.at(i), th_list.at(i + 1));
    }
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> M_list(boost::numeric::ublas::zero_matrix<std::complex<T>>(2, 2), num_layers * num_wl);
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> A(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_wl);
        std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> B(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_wl);
        for (std::size_t j = 0; j < num_wl; j++) {
            A[j](0, 0) = std::exp(-1i * delta.at(i)[j]);
            A[j](0, 1) = 0;
            A[j](1, 0) = 0;
            A[j](1, 1) = std::exp(1i * delta.at(i)[j]);
            B[j](0, 0) = 1;
            B[j](0, 1) = r_list.at(i).at(i + 1)[j];
            B[j](1, 0) = r_list.at(i).at(i + 1)[j];
            B[j](1, 1) = 1;
            M_list[i * num_wl + j] = boost::numeric::ublas::prod(A[j], B[j]) / t_list.at(i).at(i + 1)[j];
        }
    }
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> Mtilde(boost::numeric::ublas::matrix<std::complex<T>>(2, 2), num_wl);
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
    std::valarray<std::vector<std::array<std::complex<T>, 2>>> vw_list(std::vector<std::array<std::complex<T>, 2>>(num_wl), num_layers);
    std::valarray<boost::numeric::ublas::matrix<std::complex<T>>> vw(boost::numeric::ublas::zero_matrix<std::complex<T>>(2, 2), num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        vw[i](0, 0) = t[i];  // vw[i](1, 0) = 0
        vw[i](0, 1) = t[i];  // vw[i](1, 1) = 0
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
    std::ranges::transform(vw_list[num_layers - 1], vw_list[num_layers - 1].begin(), [](std::array<std::complex<T>, 2> &subarray) -> std::array<std::complex<T>, 2> {
        subarray.at(1) = 0;
        return subarray;
    });
    const std::valarray<T> R = R_from_r(r);
    const std::valarray<T> Tr = T_from_t(pol, t, n_list.front(), n_list.at(num_layers - 1), th_0, th_list.at(num_layers - 1));
    const std::valarray<T> power_entering = power_entering_from_r(pol, r, n_list.front(), th_0);
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

template auto coh_tmm(char pol, const std::vector<std::valarray<std::complex<double>>> &n_list,
                      const std::vector<double> &d_list, const std::complex<double> &th_0,
                      const std::valarray<double> &lam_vac) -> coh_tmm_vecn_dict<double>;

template<std::floating_point T>
auto coh_tmm_reverse(const char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                     const std::complex<T> th_0, const std::valarray<T> &lam_vac) -> coh_tmm_vec_dict<T> {
    const std::size_t num_wl = lam_vac.size();
    const std::size_t num_layers = d_list.size();
#ifdef _MSC_VER
    const std::valarray<std::complex<T>> th_f = snell(n_list[std::slice(0, num_wl, 1)],
                                                n_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)],
                                                th_0);
#else
    const std::valarray<std::complex<T>> th_f = snell(std::valarray<std::complex<T>>(n_list[std::slice(0, num_wl, 1)]),
                                                      std::valarray<std::complex<T>>(n_list[std::slice((num_layers - 1) * num_wl, num_wl, 1)]),
                                                      th_0);
#endif
    std::valarray<std::complex<T>> reversed_n_list(num_layers * num_wl);
    std::complex<T> *rev_nl_it = std::begin(reversed_n_list);
    // General Method: use views and iterators.
#ifdef __cpp_lib_ranges_chunk
    for (const auto &row : n_list | std::views::chunk(num_wl) | std::views::reverse) {
#else
    for (std::size_t i = num_layers - 1; i >= 0; --i) {
        std::vector<std::complex<T>> row(num_wl);
        for (std::size_t j = 0; j < num_wl; ++j) {
            row.at(j) = n_list[i * num_wl + j];
        }
#endif
        std::ranges::move(row, rev_nl_it);
        std::ranges::advance(rev_nl_it, num_wl);
    }
    // Valarray Method: use slices and indices.
    // [Pseudocode]
    // reversed_n_list[std::slice((num_layers - 1 - i) * num_wl, num_wl, 1)] = n_list[std::slice(i * num_wl, num_wl, 1)];
    std::valarray<T> reversed_d_list(num_layers);
    std::ranges::reverse_copy(d_list, std::begin(reversed_d_list));
    return coh_tmm(pol, reversed_n_list, reversed_d_list, th_f, lam_vac);
}

template auto coh_tmm_reverse(const char pol, const std::valarray<std::complex<double>> &n_list,
                              const std::valarray<double> &d_list, const std::complex<double> th_0,
                              const std::valarray<double> &lam_vac) -> coh_tmm_vec_dict<double>;

template<std::floating_point T>
auto coh_tmm_reverse(const char pol, const std::vector<std::valarray<std::complex<T>>> &n_list,
                     const std::vector<T> &d_list, const std::valarray<std::complex<T>> &th_0,
                     const std::valarray<T> &lam_vac) -> coh_tmm_vecn_dict<T> {
    const std::size_t num_wl = lam_vac.size();
    const std::size_t num_layers = d_list.size();  // == n_list.size()
    const std::valarray<std::complex<T>> th_f = snell(n_list.front(), n_list.back(), th_0);
    std::vector<std::valarray<std::complex<T>>> reversed_n_list(num_layers, std::valarray<std::complex<T>>(num_wl));
    std::ranges::reverse_copy(n_list, reversed_n_list.begin());
    std::vector<T> reversed_d_list(num_layers);
    std::ranges::reverse_copy(d_list, reversed_d_list.begin());
    return coh_tmm(pol, reversed_n_list, reversed_d_list, th_f, lam_vac);
}

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
    const std::valarray<std::vector<std::array<std::complex<T>, 2>>> vw_list_l = std::get<std::valarray<std::vector<std::array<std::complex<T>, 2>>>>(coh_tmm_data.at("vw_list"))[layer];
    const std::size_t num_wl = vw_list_l[0].size();
    std::valarray<std::complex<T>> v(1, num_layers * num_wl);
    std::valarray<std::complex<T>> w(num_layers * num_wl);
#ifdef __cpp_lib_ranges_repeat
    std::ranges::move(std::views::repeat(std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("r")), num_layers) | std::views::join, std::begin(w));
#else
    const std::valarray<std::complex<T>> r = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("r"));
    for (std::size_t i = 0; i < num_layers * num_wl; ++i) {
        w[i] = r[i % num_wl];
    }
#endif
    for (std::size_t i = 0; i < num_layers; ++i) {
        if (layer[i] > 0) {
            for (std::size_t j = 0; j < num_wl; ++j) {
                v[i * num_wl + j] = vw_list_l[i].at(j).at(0);
                w[i * num_wl + j] = vw_list_l[i].at(j).at(1);
            }
        }
    }
    const std::valarray<std::complex<T>> kz_list = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("kz_list"));
    std::valarray<std::complex<T>> kz(num_layers * num_wl);
#if (defined __GNUC__ && __GNUC__ < 13)
    for (std::size_t i = 0; i < num_layers; ++i) {
        for (std::size_t j = 0; j < num_wl; ++j) {
            kz[i * num_wl + j] = kz_list[layer[i] * num_wl + j];
        }
    }
#else
#ifdef _MSC_VER
    auto kz_view = layer * num_wl |
#else
    auto kz_view = std::valarray(layer * num_wl) |
#endif
                   std::views::transform([&kz_list, num_wl](const std::size_t layer_index) {
                       return std::ranges::subrange(std::begin(kz_list) + layer_index, std::begin(kz_list) + layer_index + num_wl);
                   }) | std::views::join;
    std::ranges::move(kz_view, std::begin(kz));
#endif
    const std::valarray<std::complex<T>> th_list = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("th_list"));
    std::valarray<std::complex<T>> th(num_layers * num_wl);
#if (defined __GNUC__ && __GNUC__ < 13)
    for (std::size_t i = 0; i < num_layers; ++i) {
        for (std::size_t j = 0; j < num_wl; ++j) {
            th[i * num_wl + j] = th_list[layer[i] * num_wl + j];
        }
    }
#else
#ifdef _MSC_VER
    auto th_view = layer * num_wl |
#else
    auto th_view = std::valarray(layer * num_wl) |
#endif
                   std::views::transform([&th_list, num_wl](const std::size_t layer_index) {
                       return std::ranges::subrange(std::begin(th_list) + layer_index, std::begin(th_list) + layer_index + num_wl);
                   }) | std::views::join;
    std::ranges::move(th_view, std::begin(th));
#endif
    const std::valarray<std::complex<T>> n_list = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("n_list"));
    std::valarray<std::complex<T>> n(num_layers * num_wl);
#if (defined __GNUC__ && __GNUC__ < 13)
    for (std::size_t i = 0; i < num_layers; ++i) {
        for (std::size_t j = 0; j < num_wl; ++j) {
            n[i * num_wl + j] = n_list[layer[i] * num_wl + j];
        }
    }
#else
#ifdef _MSC_VER
    auto n_view = layer * num_wl |
#else
    auto n_view = std::valarray(layer * num_wl) |
#endif
                   std::views::transform([&n_list, num_wl](const std::size_t layer_index) {
                       return std::ranges::subrange(std::begin(n_list) + layer_index, std::begin(n_list) + layer_index + num_wl);
                   }) | std::views::join;
    std::ranges::move(n_view, std::begin(n));
#endif
    const std::valarray<std::complex<T>> n_0 = n_list[std::slice(0, num_wl, 1)];
    const std::complex<T> th_0 = std::get<std::complex<T>>(coh_tmm_data.at("th_0"));
    const char pol = std::get<char>(coh_tmm_data.at("pol"));
    // std::ranges::any_of(layer, std::bind_front(std::greater_equal<>(), 1)) or
    // std::ranges::any_of(layer, std::bind_front(std::greater<>(), 0)) or
    std::valarray<bool> cond = (layer < 1 or 0 > distance or distance > std::get<std::valarray<T>>(coh_tmm_data.at("d_list"))[layer]) and (layer not_eq 0 or distance > 0);
    bool *pos = std::ranges::find(cond, true);
    // If std::cend(cond): Substitution failed: expression ::std::end(_Cont) is ill-formed.
    // error C2672: “std::cend”: no matching overloaded function found
    // MSVC's <xutility> cbegin() and cend() requires const _Container& _Cont
    // while begin() and end() requires _Ty (&_Array)[_Size].
    // libstdc++ begin() and end(): valarray<_Tp>& or const valarray<_Tp>&
    // However, libstdc++ can accept std::cbegin(cond) or std::cend(cond),
    // although the C++ standard does not list the two non-member functions.
    if (pos not_eq std::end(cond)) {
        throw std::runtime_error("Position cannot be resolved at layer " + std::to_string(std::ranges::distance(std::begin(cond), pos)));
    }
    std::valarray<std::complex<T>> comp_dist(num_layers * num_wl);
#ifdef __cpp_lib_ranges_repeat
    std::ranges::move(std::views::repeat(distance | std::views::transform([](const T real) -> std::complex<T> {
        return real;
    }), num_wl) | std::views::join, std::begin(comp_dist));
#else
    for (std::size_t i = 0; i < num_layers * num_wl; ++i) {
        comp_dist[i] = std::real(distance[i % num_layers]);
    }
#endif
    comp_dist = Utils::Range::rng2d_transpose(comp_dist, num_wl);
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

template<typename T>
auto position_resolved(const std::size_t layer, const T distance,
                       const coh_tmm_vec_dict<T> &coh_tmm_data) -> std::unordered_map<std::string, std::variant<std::valarray<T>, std::valarray<std::complex<T>>>> {
    const std::vector<std::array<std::complex<T>, 2>> vw_list_l = std::get<std::valarray<std::vector<std::array<std::complex<T>, 2>>>>(coh_tmm_data.at("vw_list"))[layer];
    const std::size_t num_wl = vw_list_l.size();
    std::valarray<std::complex<T>> v(1, num_wl);
    std::valarray<std::complex<T>> w(num_wl);
    if (layer > 0) {
        for (std::size_t i = 0; i < num_wl; i++) {
            v[i] = vw_list_l.at(i).at(0);
            w[i] = vw_list_l.at(i).at(1);
        }
    } else {
        w = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("r"));
    }
    const std::valarray<std::complex<T>> kz = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("kz_list"))[std::slice(layer * num_wl, num_wl, 1)];
    const std::valarray<std::complex<T>> th = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("th_list"))[std::slice(layer * num_wl, num_wl, 1)];
    const std::valarray<std::complex<T>> n = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("n_list"))[std::slice(layer * num_wl, num_wl, 1)];
    const std::valarray<std::complex<T>> n_0 = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("n_list"))[std::slice(0, num_wl, 1)];
    const std::complex<T> th_0 = std::get<std::complex<T>>(coh_tmm_data.at("th_0"));
    const char pol = std::get<char>(coh_tmm_data.at("pol"));
    if ((layer < 1 or 0 > distance or distance > std::get<std::valarray<T>>(coh_tmm_data.at("d_list"))[layer]) and (layer not_eq 0 or distance > 0)) {
        throw std::runtime_error("Position cannot be resolved at layer " + std::to_string(layer));
    }
    const std::valarray<std::complex<T>> Ef = v * std::exp(1i * kz * distance);
    const std::valarray<std::complex<T>> Eb = w * std::exp(-1i * kz * distance);
    std::valarray<T> poyn(num_wl);
    if (pol == 's') {
        for (std::size_t i = 0; i < num_wl; i++) {
            poyn[i] = (n[i] * std::cos(th[i]) * std::conj(Ef[i] + Eb[i]) * (Ef[i] - Eb[i])).real() /
                            (n_0[i] * std::cos(th_0)).real();
        }
    } else if (pol == 'p') {
        for (std::size_t i = 0; i < num_wl; i++) {
            poyn[i] = (n[i] * std::conj(std::cos(th[i])) * (Ef[i] + Eb[i]) * std::conj(Ef[i] - Eb[i])).real() /
                            (n_0[i] * std::conj(std::cos(th_0))).real();
        }
    }
    std::valarray<T> absor(num_wl);
    if (pol == 's') {
        for (std::size_t i = 0; i < num_wl; i++) {
            absor[i] = (n[i] * std::cos(th[i]) * kz[i] * std::norm(Ef[i] + Eb[i])).imag() /
                            (n_0[i] * std::cos(th_0)).real();
        }
    } else if (pol == 'p') {
        for (std::size_t i = 0; i < num_wl; i++) {
            absor[i] = (n[i] * std::conj(std::cos(th[i])) * (kz[i] * std::norm(Ef[i] - Eb[i]) - std::conj(kz[i]) * std::norm(Ef[i] + Eb[i]))).imag() /
                            (n_0[i] * std::conj(std::cos(th_0))).real();
        }
    }
    const std::valarray<std::complex<T>> Ex = pol == 's' ? std::valarray<std::complex<T>>{0} : (Ef - Eb) * std::cos(th);
    const std::valarray<std::complex<T>> Ey = pol == 's' ? Ef + Eb : std::valarray<std::complex<T>>{0};
    const std::valarray<std::complex<T>> Ez = pol == 's' ? std::valarray<std::complex<T>>{0} : -(Ef + Eb) * std::sin(th);
    return {{"poyn", poyn}, {"absor", absor}, {"Ex", Ex}, {"Ey", Ey}, {"Ez", Ez}};
}

template<typename T>
auto position_resolved(const std::size_t layer, const T distance,
                       const coh_tmm_vecn_dict<T> &coh_tmm_data) -> std::unordered_map<std::string, std::variant<std::valarray<T>, std::valarray<std::complex<T>>>> {
    const std::vector<std::array<std::complex<T>, 2>> vw_list_l = std::get<std::valarray<std::vector<std::array<std::complex<T>, 2>>>>(coh_tmm_data.at("vw_list"))[layer];
    const std::size_t num_wl = vw_list_l.size();
    std::valarray<std::complex<T>> v(1, num_wl);
    std::valarray<std::complex<T>> w(num_wl);
    if (layer > 0) {
        for (std::size_t i = 0; i < num_wl; i++) {
            v[i] = vw_list_l.at(i).at(0);
            w[i] = vw_list_l.at(i).at(1);
        }
    } else {
        w = std::get<std::valarray<std::complex<T>>>(coh_tmm_data.at("r"));
    }
    const std::valarray<std::complex<T>> kz = std::get<std::vector<std::valarray<std::complex<T>>>>(coh_tmm_data.at("kz_list")).at(layer);
    const std::valarray<std::complex<T>> th = std::get<std::vector<std::valarray<std::complex<T>>>>(coh_tmm_data.at("th_list")).at(layer);
    const std::valarray<std::complex<T>> n = std::get<std::vector<std::valarray<std::complex<T>>>>(coh_tmm_data.at("n_list")).at(layer);
    const std::valarray<std::complex<T>> n_0 = std::get<std::vector<std::valarray<std::complex<T>>>>(coh_tmm_data.at("n_list")).front();
    const std::complex<T> th_0 = std::get<std::complex<T>>(coh_tmm_data.at("th_0"));
    const char pol = std::get<char>(coh_tmm_data.at("pol"));
    if ((layer < 1 or 0 > distance or distance > std::get<std::valarray<T>>(coh_tmm_data.at("d_list"))[layer]) and (layer not_eq 0 or distance > 0)) {
        throw std::runtime_error("Position cannot be resolved at layer " + std::to_string(layer));
    }
    const std::valarray<std::complex<T>> Ef = v * std::exp(1i * kz * distance);
    const std::valarray<std::complex<T>> Eb = w * std::exp(-1i * kz * distance);
    std::valarray<T> poyn(num_wl);
    if (pol == 's') {
        for (std::size_t i = 0; i < num_wl; i++) {
            poyn[i] = (n[i] * std::cos(th[i]) * std::conj(Ef[i] + Eb[i]) * (Ef[i] - Eb[i])).real() /
                      (n_0[i] * std::cos(th_0)).real();
        }
    } else if (pol == 'p') {
        for (std::size_t i = 0; i < num_wl; i++) {
            poyn[i] = (n[i] * std::conj(std::cos(th[i])) * (Ef[i] + Eb[i]) * std::conj(Ef[i] - Eb[i])).real() /
                      (n_0[i] * std::conj(std::cos(th_0))).real();
        }
    }
    std::valarray<T> absor(num_wl);
    if (pol == 's') {
        for (std::size_t i = 0; i < num_wl; i++) {
            absor[i] = (n[i] * std::cos(th[i]) * kz[i] * std::norm(Ef[i] + Eb[i])).imag() /
                       (n_0[i] * std::cos(th_0)).real();
        }
    } else if (pol == 'p') {
        for (std::size_t i = 0; i < num_wl; i++) {
            absor[i] = (n[i] * std::conj(std::cos(th[i])) * (kz[i] * std::norm(Ef[i] - Eb[i]) - std::conj(kz[i]) * std::norm(Ef[i] + Eb[i]))).imag() /
                       (n_0[i] * std::conj(std::cos(th_0))).real();
        }
    }
    const std::valarray<std::complex<T>> Ex = pol == 's' ? std::valarray<std::complex<T>>{0} : (Ef - Eb) * std::cos(th);
    const std::valarray<std::complex<T>> Ey = pol == 's' ? Ef + Eb : std::valarray<std::complex<T>>{0};
    const std::valarray<std::complex<T>> Ez = pol == 's' ? std::valarray<std::complex<T>>{0} : -(Ef + Eb) * std::sin(th);
    return {{"poyn", poyn}, {"absor", absor}, {"Ex", Ex}, {"Ey", Ey}, {"Ez", Ez}};
}

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
                       const std::valarray<T> &dist) -> std::pair<std::valarray<typename std::iterator_traits<T *>::difference_type>, std::valarray<T>> {
    if (std::isinf(std::abs(d_list.sum()))) {
        throw std::runtime_error("This function expects finite arguments");
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
    // std::vector<T> d_array(dlist_size + 1, 0);
    // std::ranges::move(d_list, d_array.begin() + 1);
    std::valarray<T> cum_sum(dlist_size);
    // MSVC-specific: std::begin()/std::end() instead of std::cbegin()/std::cend() (C2672)
    std::partial_sum(std::begin(d_list), std::end(d_list), std::begin(cum_sum));
    std::valarray<typename std::iterator_traits<T *>::difference_type> layer(-1, dist_size);
    std::valarray<T> distance = dist;
    // dist -= cum_sum[layer - 1];
    for (std::size_t i = 0; i < dist_size; i++) {
        // lower_bound for searchsorted left
        // upper_bound for searchsorted right
        const T *it = std::ranges::upper_bound(cum_sum, dist[i]);
        // layer[dist < 0] = -1;
        if (dist[i] >= 0) {
            layer[i] = std::ranges::distance(std::begin(cum_sum), it);
            distance[i] -= layer[i] == 0 ? cum_sum[dlist_size - 1] : cum_sum[layer[i] - 1];
        }
    }
    return std::pair(layer, distance);
}

template auto find_in_structure(const std::valarray<double> &d_list,
                                const std::valarray<double> &dist) -> std::pair<std::valarray<std::iterator_traits<double *>::difference_type>, std::valarray<double>>;

template<typename T>
auto find_in_structure_inf(const std::valarray<T> &d_list,
                           const std::valarray<T> &dist) -> std::pair<std::valarray<std::size_t>, std::valarray<T>> {
#ifdef _MSC_VER
    std::pair<std::valarray<typename std::iterator_traits<T *>::difference_type>, std::valarray<T>> found = find_in_structure(d_list[std::slice(1, d_list.size() - 2, 1)], dist);
#else
    std::pair<std::valarray<typename std::iterator_traits<T *>::difference_type>, std::valarray<T>> found = find_in_structure(std::valarray<T>(d_list[std::slice(1, d_list.size() - 2, 1)]), dist);
#endif
    std::valarray<std::size_t> layer(dist.size());  // constructor of valarray<size_t> would be ambiguous
    for (std::size_t i = 0; i < dist.size(); i++) {
        if (dist[i] < 0) {
            layer[i] = 0;
            found.second[i] = dist[i];
        } else {
            layer[i] = found.first[i];  // found.first[i] + 1 : found.first[i], type conversion
        }
    }
    return std::pair(layer, found.second);
}

template auto find_in_structure_inf(const std::valarray<double> &d_list,
                                    const std::valarray<double> &dist) -> std::pair<std::valarray<std::size_t>, std::valarray<double>>;

template<std::floating_point T>
auto layer_starts(const std::valarray<T> &d_list) -> std::valarray<T> {
    std::valarray<T> final_answer(0.0, d_list.size());
    final_answer[0] = -INFINITY;
    final_answer[1] = 0;
    for (std::size_t i = 2; i < d_list.size(); i++) {
        final_answer[i] = final_answer[i - 1] + d_list[i - 1];
    }
    return final_answer;
}

template auto layer_starts(const std::valarray<double> &d_list) -> std::valarray<double>;

template<typename T>
auto absorp_in_each_layer(const coh_tmm_vec_dict<T> &coh_tmm_data) -> std::valarray<std::valarray<T>> {  // public
    const std::size_t num_layers = std::get<std::valarray<T>>(coh_tmm_data.at("d_list")).size();
    const std::size_t num_lam_vac = std::get<std::valarray<T>>(coh_tmm_data.at("lam_vac")).size();
    std::valarray<std::valarray<T>> power_entering_each_layer(std::valarray<T>(num_lam_vac), num_layers);
    power_entering_each_layer[0] = 1;
    power_entering_each_layer[1] = std::get<std::valarray<T>>(coh_tmm_data.at("power_entering"));
    power_entering_each_layer[num_layers - 1] = std::get<std::valarray<T>>(coh_tmm_data.at("T"));
    for (std::size_t i = 2; i < num_layers - 1; i++) {
        power_entering_each_layer[i] = std::get<std::valarray<T>>(position_resolved(i, 0.0, coh_tmm_data).at("poyn"));
    }
    std::valarray<std::valarray<T>> final_answer(std::valarray<T>(num_lam_vac), num_layers);
    std::adjacent_difference(std::next(std::begin(power_entering_each_layer)), std::end(power_entering_each_layer),
                             std::begin(final_answer), [](const std::valarray<T> &x, const std::valarray<T> &y) {
                return y - x;
            });
    final_answer[0] = power_entering_each_layer[0] - power_entering_each_layer[1];
    final_answer[num_layers - 1] = power_entering_each_layer[num_layers - 1];
    for (std::size_t i = 0; i < num_layers; i++) {
        final_answer[i][final_answer[i] < std::valarray<T>(0.0, num_lam_vac)] = 0;
    }
    return final_answer;
}

template auto absorp_in_each_layer(const coh_tmm_vec_dict<double> &coh_tmm_data) -> std::valarray<std::valarray<double>>;

template<typename T>
auto absorp_in_each_layer(const coh_tmm_vecn_dict<T> &coh_tmm_data) -> std::valarray<std::valarray<T>> {  // private
    const std::size_t num_layers = std::get<std::vector<T>>(coh_tmm_data.at("d_list")).size();
    const std::size_t num_lam_vac = std::get<std::valarray<T>>(coh_tmm_data.at("lam_vac")).size();
    std::valarray<std::valarray<T>> power_entering_each_layer(std::valarray<T>(num_lam_vac), num_layers);
    power_entering_each_layer[0] = 1;
    power_entering_each_layer[1] = std::get<std::valarray<T>>(coh_tmm_data.at("power_entering"));
    power_entering_each_layer[num_layers - 1] = std::get<std::valarray<T>>(coh_tmm_data.at("T"));
    for (std::size_t i = 2; i < num_layers - 1; i++) {
        power_entering_each_layer[i] = std::get<std::valarray<T>>(position_resolved(i, 0.0, coh_tmm_data).at("poyn"));
    }
    std::valarray<std::valarray<T>> final_answer(std::valarray<T>(num_lam_vac), num_layers);
    std::adjacent_difference(std::next(std::begin(power_entering_each_layer)), std::end(power_entering_each_layer),
                             std::begin(final_answer), [](const std::valarray<T> &x, const std::valarray<T> &y) {
                return y - x;
            });
    final_answer[0] = power_entering_each_layer[0] - power_entering_each_layer[1];
    final_answer[num_layers - 1] = power_entering_each_layer[num_layers - 1];
    for (std::size_t i = 0; i < num_layers; i++) {
        final_answer[i][final_answer[i] < std::valarray<T>(0.0, num_lam_vac)] = 0;
    }
    return final_answer;
}

template auto absorp_in_each_layer(const coh_tmm_vecn_dict<double> &coh_tmm_data) -> std::valarray<std::valarray<double>>;

template<typename T>
auto inc_group_layers(const std::vector<std::valarray<std::complex<T>>> &n_list, const std::valarray<T> &d_list,
                      const std::valarray<LayerType> &c_list) -> inc_tmm_vec_dict<T> {
    const std::size_t num_layers = n_list.size();
    if (not std::isinf(d_list[0]) or not std::isinf(d_list[d_list.size() - 1])) {
        throw std::runtime_error("d_list must start and end with inf!");
    }
    if (c_list[0] not_eq LayerType::Incoherent or c_list[c_list.size() - 1] not_eq LayerType::Incoherent) {
        throw std::runtime_error("c_list should start and end with Incoherent");
    }
    if (d_list.size() not_eq c_list.size()) {
        throw std::logic_error("List sizes do not match!");
    }
    std::size_t inc_index = 0;
    std::size_t stack_index = 0;
    std::vector<std::vector<T>> stack_d_list;
    std::vector<std::vector<std::valarray<std::complex<T>>>> stack_n_list;
    std::vector<std::size_t> all_from_inc;
    std::vector<std::ptrdiff_t> inc_from_all;
    std::vector<std::vector<std::size_t>> all_from_stack;
    std::vector<std::vector<std::size_t>> stack_from_all;
    std::vector<std::ptrdiff_t> inc_from_stack;
    std::vector<std::ptrdiff_t> stack_from_inc;
    bool stack_in_progress = false;
    std::vector<T> ongoing_stack_d_list;
    std::vector<std::valarray<std::complex<T>>> ongoing_stack_n_list;
    std::size_t within_stack_index = 0;
    for (std::size_t alllayer_index = 0; alllayer_index < num_layers; alllayer_index++) {
        if (c_list[alllayer_index] == LayerType::Coherent) {
            inc_from_all.push_back(-1);
            if (not stack_in_progress) {
                stack_in_progress = true;
                ongoing_stack_d_list = {INFINITY, d_list[alllayer_index]};
                ongoing_stack_n_list = {n_list.at(alllayer_index - 1), n_list.at(alllayer_index)};
                stack_from_all.push_back({stack_index, 1});
                all_from_stack.push_back({alllayer_index - 1, alllayer_index});
                inc_from_stack.push_back(inc_index - 1);
                within_stack_index = 1;
            } else {  // another coherent layer in the same stack
                ongoing_stack_d_list.push_back(d_list[alllayer_index]);
                ongoing_stack_n_list.push_back(n_list.at(alllayer_index));
                within_stack_index++;
                stack_from_all.push_back({stack_index, within_stack_index});
                all_from_stack.back().push_back(alllayer_index);
            }
        } else if (c_list[alllayer_index] == LayerType::Incoherent) {
            stack_from_all.emplace_back();
            inc_from_all.push_back(inc_index);
            all_from_inc.push_back(alllayer_index);
            if (not stack_in_progress) {
                stack_from_inc.push_back(-1);
            } else {
                stack_in_progress = false;
                stack_from_inc.push_back(stack_index);
                ongoing_stack_d_list.push_back(INFINITY);
                stack_d_list.push_back(ongoing_stack_d_list);
                ongoing_stack_n_list.push_back(n_list.at(alllayer_index));
                stack_n_list.push_back(ongoing_stack_n_list);
                all_from_stack.back().push_back(alllayer_index);
                stack_index++;
            }
            inc_index++;
        } else {
            throw std::invalid_argument("Error: c_list entries must be Incoherent or Coherent!");
        }
    }
    return {{"stack_d_list", stack_d_list},
            {"stack_n_list", stack_n_list},
            {"all_from_inc", all_from_inc},
            {"inc_from_all", inc_from_all},
            {"all_from_stack", all_from_stack},
            {"stack_from_all", stack_from_all},
            {"inc_from_stack", inc_from_stack},
            {"stack_from_inc", stack_from_inc},
            {"num_stacks", all_from_stack.size()},
            {"num_inc_layers", all_from_inc.size()},
            {"num_layers", n_list.size()}};
}

template auto inc_group_layers(const std::vector<std::valarray<std::complex<double>>> &n_list,
                               const std::valarray<double> &d_list,
                               const std::valarray<LayerType> &c_list) -> inc_tmm_vec_dict<double>;

/*
 * This function is vectorized.
 * Incoherent, or partly incoherent partly coherent, transfer matrix method.

 * See coh_tmm for definitions of pol, n_list, d_list, th_0, lam_vac.

 * c_list is "coherency list". Each entry should be 'i' for incoherent or 'c'
 * for 'coherent'.

 * If an incoherent layer has real refractive index (no absorption), then its
 * thickness doesn't affect the calculation results.

 * See https://arxiv.org/abs/1603.02720 for a physics background and some
 * of the definitions.

 * Outputs the following as a dictionary:

 * - R--reflected wave power (as a fraction of incident)
 * - T--transmitted wave power (as a fraction of incident)
 * - VW_list-- the n'th element is [V_n, W_n], the forward- and backward-traveling
 *   intensities, respectively, at the beginning of the n'th incoherent medium.
 * - coh_tmm_data_list--n'th element is coh_tmm_data[n], the output of
 *   the coh_tmm program for the n'th "stack" (group of one or more
 *   consecutive coherent layers).
 * - coh_tmm_bdata_list--n'th element is coh_tmm_bdata[n], the output of the
 *   coh_tmm program for the n'th stack, but with the layers of the stack
 *   in reverse order.
 * - stackFB_list--n'th element is [F, B], where F is light traveling forward
 *   towards the n'th stack and B is light traveling backwards towards the n'th
 *   stack.
 * - num_layers-- total number both coherent and incoherent.
 * - power_entering_list--n'th element is the normalized Poynting vector
 *   crossing the interface into the n'th incoherent layer from the previous
 *   (coherent or incoherent) layer.
 * - Plus, all the outputs of inc_group_layers
 */
template<std::floating_point T>
auto inc_tmm(const char pol, const std::vector<std::valarray<std::complex<T>>> &n_list, const std::valarray<T> &d_list,
             const std::valarray<LayerType> &c_list, const std::complex<T> th_0,
             const std::valarray<T> &lam_vac) -> inc_tmm_vec_dict<T> {
    const std::size_t num_layers = n_list.size();
    const std::size_t num_wl = lam_vac.size();
    if (std::holds_alternative<std::valarray<std::complex<T>>>(Utils::Math::real_if_close<std::complex<T>, T>(std::valarray<std::complex<T>>(n_list.front() * std::sin(th_0))))) {
        throw std::runtime_error("Error in n0 or th0!");
    }
    inc_tmm_vec_dict<T> group_layer_data = inc_group_layers(n_list, d_list, c_list);
    const std::size_t num_inc_layers = std::get<std::size_t>(group_layer_data.at("num_inc_layers"));
    const std::size_t num_stacks = std::get<std::size_t>(group_layer_data.at("num_stacks"));
    const std::vector<std::vector<std::valarray<std::complex<T>>>> stack_n_list = std::get<std::vector<std::vector<std::valarray<std::complex<T>>>>>(group_layer_data.at("stack_n_list"));
    const std::vector<std::vector<T>> stack_d_list = std::get<std::vector<std::vector<T>>>(group_layer_data.at("stack_d_list"));
    const std::vector<std::vector<std::size_t>> all_from_stack = std::get<std::vector<std::vector<std::size_t>>>(group_layer_data.at("all_from_stack"));
    const std::vector<std::size_t> all_from_inc = std::get<std::vector<std::size_t>>(group_layer_data.at("all_from_inc"));
    const std::vector<std::ptrdiff_t> stack_from_inc = std::get<std::vector<std::ptrdiff_t>>(group_layer_data.at("stack_from_inc"));
    const std::vector<std::ptrdiff_t> inc_from_stack = std::get<std::vector<std::ptrdiff_t>>(group_layer_data.at("inc_from_stack"));

    std::vector<std::valarray<std::complex<T>>> th_list = list_snell(n_list, th_0);

    std::vector<coh_tmm_vecn_dict<T>> coh_tmm_data_list;
    std::vector<coh_tmm_vecn_dict<T>> coh_tmm_bdata_list;
    for (std::size_t i : std::views::iota(0U, num_stacks)) {
        coh_tmm_data_list.emplace_back(coh_tmm(pol, stack_n_list.at(i), stack_d_list.at(i), th_list.at(all_from_stack.at(i).front()), lam_vac));
        coh_tmm_bdata_list.emplace_back(coh_tmm_reverse(pol, stack_n_list.at(i), stack_d_list.at(i), th_list.at(all_from_stack.at(i).front()), lam_vac));
    }
    std::vector<std::valarray<T>> P_list(num_inc_layers, std::valarray<T>(num_wl));
    std::size_t all_inc_i = 0;
    for (std::size_t inc_index : std::views::iota(1U, num_inc_layers - 1)) {
        all_inc_i = all_from_inc.at(inc_index);
        for (std::size_t i : std::views::iota(0U, num_wl)) {
            P_list.at(inc_index)[i] = std::exp(-4 * std::numbers::pi_v<T> * d_list[all_inc_i] * (n_list.at(all_inc_i)[i] * std::cos(th_list.at(all_inc_i)[i])).imag() / lam_vac[i]);
        }
    }
    for (std::valarray<T> &Pv : P_list) {
        Pv[Pv < 1e-30] = 1e-30;
    }
    std::vector<std::vector<std::valarray<T>>> T_list(num_inc_layers, std::vector<std::valarray<T>>(num_inc_layers, std::valarray<T>(num_wl)));
    std::vector<std::vector<std::valarray<T>>> R_list(num_inc_layers, std::vector<std::valarray<T>>(num_inc_layers, std::valarray<T>(num_wl)));
    std::size_t alllayer_index = 0;
    std::ptrdiff_t nextstack_index = 0;
    for (std::size_t inc_index : std::views::iota(0U, num_inc_layers - 1)) {
        alllayer_index = all_from_inc.at(inc_index);
        nextstack_index = stack_from_inc.at(inc_index + 1);
        if (nextstack_index == -1) {
            R_list.at(inc_index).at(inc_index + 1) = interface_R(pol, n_list.at(alllayer_index),
                                                                 n_list.at(alllayer_index + 1),
                                                                 th_list.at(alllayer_index),
                                                                 th_list.at(alllayer_index + 1));
            T_list.at(inc_index).at(inc_index + 1) = interface_T(pol, n_list.at(alllayer_index),
                                                                 n_list.at(alllayer_index + 1),
                                                                 th_list.at(alllayer_index),
                                                                 th_list.at(alllayer_index + 1));
            R_list.at(inc_index + 1).at(inc_index) = interface_R(pol, n_list.at(alllayer_index + 1),
                                                                 n_list.at(alllayer_index),
                                                                 th_list.at(alllayer_index + 1),
                                                                 th_list.at(alllayer_index));
            T_list.at(inc_index + 1).at(inc_index) = interface_T(pol, n_list.at(alllayer_index + 1),
                                                                 n_list.at(alllayer_index),
                                                                 th_list.at(alllayer_index + 1),
                                                                 th_list.at(alllayer_index));
        } else {
            R_list.at(inc_index).at(inc_index + 1) = std::get<std::valarray<T>>(coh_tmm_data_list.at(nextstack_index).at("R"));
            T_list.at(inc_index).at(inc_index + 1) = std::get<std::valarray<T>>(coh_tmm_data_list.at(nextstack_index).at("T"));
            R_list.at(inc_index + 1).at(inc_index) = std::get<std::valarray<T>>(coh_tmm_bdata_list.at(nextstack_index).at("R"));
            T_list.at(inc_index + 1).at(inc_index) = std::get<std::valarray<T>>(coh_tmm_bdata_list.at(nextstack_index).at("T"));
        }
    }
    std::valarray<boost::numeric::ublas::matrix<T>> L0(boost::numeric::ublas::matrix<T>(2, 2, NAN), num_wl);
    std::vector<std::valarray<boost::numeric::ublas::matrix<T>>> L_list{std::move(L0)};
    std::valarray<boost::numeric::ublas::matrix<T>> Ltilde(boost::numeric::ublas::zero_matrix<T>(2, 2), num_wl);
    for (std::size_t i : std::views::iota(0U, num_wl)) {
        Ltilde[i] <<= 1 / T_list[0][1][i]              , -R_list[1][0][i] / T_list[0][1][i],
                      R_list[0][1][i] / T_list[0][1][i], (T_list[1][0][i] * T_list[0][1][i] - R_list[1][0][i] * R_list[0][1][i]) / T_list[0][1][i];
        // We can write Ltilde[i] = Ltilde[i] / T_list[0][1][i] but not Ltilde = Ltilde / T_list[0][1]
    }
    boost::numeric::ublas::matrix<T> L(2, 2);
    boost::numeric::ublas::matrix<T> L1(2, 2);
    boost::numeric::ublas::matrix<T> L2(2, 2);
    // Q: Why not boost::numeric::ublas::matrix<std::valarray<T>> instead?
    // A: Though we can add two matrices of valarray, but we cannot do matrix multiplication.
    // template<class E1, class E2>
    // static BOOST_UBLAS_INLINE result_type apply (const matrix_expression<E1> &e1, const matrix_expression<E2> &e2,
    //                                              size_type i, size_type j) {
    //     size_type size = BOOST_UBLAS_SAME (e1 ().size2 (), e2 ().size1 ());
    //     result_type t = result_type (0);
    //     for (size_type k = 0; k < size; ++ k)
    //         t += e1 () (i, k) * e2 () (k, j);
    //         return t;
    // }
    // where result_type is matrix_matrix_binary_functor<M1, M2, TV>::result_type's template class TV.
    // For GCC/Clang, result_type (0) is
    // std::_Expr<std::__detail::_BinClos<std::__plus, std::_ValArray, std::_ValArray, double, double>, double>::_Expr(int)
    // so error: no matching function for call to it
    // t is ‘boost::numeric::ublas::matrix_matrix_prod<boost::numeric::ublas::matrix<std::valarray<double> >,
    // boost::numeric::ublas::matrix<std::valarray<double> >,
    // std::_Expr<std::__detail::_BinClos<std::__plus, std::_ValArray, std::_ValArray, double, double>, double> >::result_type’
    // {aka ‘boost::numeric::ublas::matrix_matrix_binary_functor<boost::numeric::ublas::matrix<std::valarray<double> >,
    // boost::numeric::ublas::matrix<std::valarray<double> >,
    // std::_Expr<std::__detail::_BinClos<std::__plus, std::_ValArray, std::_ValArray, double, double>, double> >::result_type’}
    // and RHS is ‘std::_Expr<std::__detail::_BinClos<std::__multiplies, std::_ValArray, std::_ValArray, double, double>, double>’
    // so error: no match for ‘operator+=’
    // For MSVC, the code can be compiled because valarray expressions are all valarrays, but
    // it does not allocate memory for the element valarray, so the element valarray is empty of size 0.
    // -> template<class E1, class E2>
    // typename matrix_matrix_binary_traits<typename E1::value_type, E1, typename E2::value_type, E2>::result_type
    // prod (const matrix_expression<E1> &e1, const matrix_expression<E2> &e2) in matrix_expression.hpp
    // where e1 and e2 are
    // const boost::numeric::ublas::matrix_expression<boost::numeric::ublas::matrix<std::valarray<double>,
    // boost::numeric::ublas::basic_row_major<unsigned __int64, __int64>,
    // boost::numeric::ublas::unbounded_array<std::valarray<double>, std::allocator<std::valarray<double>>>>> &
    // It is even impossible to enclose std::vector because it will fail scalar_binary_functor's result_type.
    // boost::numeric::ublas::vector will fail at result_type (0).
    for (std::size_t i : std::views::iota(1U, num_inc_layers - 1)) {
        std::valarray<boost::numeric::ublas::matrix<T>> Li(boost::numeric::ublas::matrix<T>(2, 2), num_wl);
        for (std::size_t j : std::views::iota(0U, num_wl)) {
            L1 <<= 1 / P_list.at(i)[j], 0,
                   0                     , P_list.at(i)[j];
            L2 <<= 1                              , -R_list.at(i + 1).at(i)[j],
                   R_list.at(i).at(i + 1)[j], T_list.at(i + 1).at(i)[j] * T_list.at(i).at(i + 1)[j] - R_list.at(i + 1).at(i)[j] * R_list.at(i).at(i + 1)[j];
            L = boost::numeric::ublas::prod(L1, L2);
            L /= T_list.at(i).at(i + 1)[j];
            Li[j] = L;
            Ltilde[j] = boost::numeric::ublas::prod(Ltilde[j], L);
        }
        L_list.emplace_back(Li);
    }
    std::valarray<T> Tr(num_wl);
    std::valarray<T> R(num_wl);
    for (std::size_t i : std::views::iota(0U, num_wl)) {
        Tr[i] = 1 / Ltilde[i](0, 0);
        R[i] = Ltilde[i](1, 0) / Ltilde[i](0, 0);
    }
    std::valarray<std::array<std::valarray<T>, 2>> VW_list(std::array<std::valarray<T>, 2>{std::valarray<T>(num_wl), std::valarray<T>(num_wl)}, num_inc_layers);
    VW_list[0] = std::array<std::valarray<T>, 2>{std::valarray<T>(NAN, num_wl), std::valarray<T>(NAN, num_wl)};
    std::valarray<boost::numeric::ublas::vector<T>> VW(boost::numeric::ublas::zero_vector<T>(2), num_wl);
    for (std::size_t i = 0; i < num_wl; i++) {
        VW[i](0) = Tr[i];
    }
    for (std::size_t i = 0; i < num_wl; i++) {
        VW_list[num_inc_layers - 1].at(0)[i] = VW[i](0);
        VW_list[num_inc_layers - 1].at(1)[i] = VW[i](1);
    }
    for (std::size_t i = num_inc_layers - 2; i > 0; --i) {
        for (std::size_t j = 0; j < num_wl; j++) {
            VW[j] = boost::numeric::ublas::prod(L_list.at(i)[j], VW[j]);
            VW_list[i].at(0)[j] = VW[j](0);
            VW_list[i].at(1)[j] = VW[j](1);
        }
    }
    std::valarray<std::array<std::valarray<T>, 2>> stackFB_list(inc_from_stack.size());
    std::valarray<T> F(num_wl);
    std::valarray<T> B(num_wl);
#ifdef __cpp_lib_ranges_enumerate
    for (auto const [i, prev_inc_index] : std::views::enumerate(inc_from_stack)) {
#else
    for (auto i = 0; i < inc_from_stack.size(); ++i) {
        auto prev_inc_index = inc_from_stack[i];
#endif
#ifdef __cpp_lib_ranges_iota
        for (std::size_t j : std::views::iota(0U, num_wl)) {
#else
        for (std::size_t j = 0; j < num_wl; ++j) {
#endif
            F[j] = (prev_inc_index == 0) ? 1 : VW_list[prev_inc_index].at(0)[j] * P_list.at(prev_inc_index)[j];
            B[j] = VW_list[prev_inc_index + 1].at(1)[j];
        }
        stackFB_list[i] = {F, B};
    }
    std::vector<std::valarray<T>> power_entering_list{std::valarray<T>(1, num_wl)};
#ifdef __cpp_lib_ranges_enumerate
    for (auto const [i, prev_stack_index] : stack_from_inc | std::views::drop(1) | std::views::enumerate) {
#else
    for (auto i = 0; i < stack_from_inc.size() - 1; ++i) {
        auto prev_stack_index = stack_from_inc[i + 1];
#endif
        if (prev_stack_index == -1) {
#ifdef _MSC_VER
            power_entering_list.emplace_back(i == 0 ? T_list[0][1] - VW_list[1][1] * T_list[1][0] :
                                             VW_list[i][0] * P_list[i] * T_list[i][i + 1] -
                                             VW_list[i + 1][1] * T_list[i + 1][i]);
#else
            power_entering_list.emplace_back(i == 0 ? std::valarray<T>(T_list[0][1] - VW_list[1][1] * T_list[1][0]) :
                                             std::valarray<T>(VW_list[i][0] * P_list[i] * T_list[i][i + 1] -
                                             VW_list[i + 1][1] * T_list[i + 1][i]));
#endif
        } else {
            power_entering_list.emplace_back(stackFB_list[prev_stack_index][0] * std::get<std::valarray<T>>(coh_tmm_data_list.at(prev_stack_index).at("T")) -
                    stackFB_list[prev_stack_index][1] * std::get<std::valarray<T>>(coh_tmm_bdata_list.at(prev_stack_index).at("power_entering")));
        }
    }
    // despite checking in interface_T and interface_R, still sometimes end up with
    // unphysical R or T values of incident from medium with n > 1
    R[R > 1] = 1;
    Tr[Tr < 0] = 0;
    group_layer_data.merge(inc_tmm_vec_dict<T>{{"T", Tr},
                                               {"R", R},
                                               {"VW_list", VW_list},
                                               {"coh_tmm_data_list", coh_tmm_data_list},
                                               {"coh_tmm_bdata_list", coh_tmm_bdata_list},
                                               {"stackFB_list", stackFB_list},
                                               {"power_entering_list", power_entering_list}});
    return group_layer_data;
}

template auto inc_tmm(char pol, const std::vector<std::valarray<std::complex<double>>> &n_list,
                      const std::valarray<double> &d_list, const std::valarray<LayerType> &c_list,
                      std::complex<double> th_0,
                      const std::valarray<double> &lam_vac) -> inc_tmm_vec_dict<double>;

template<typename T>
auto inc_absorp_in_each_layer(const inc_tmm_vec_dict<T> &inc_data) -> std::vector<std::valarray<T>> {
    const std::vector<std::ptrdiff_t> stack_from_inc = std::get<std::vector<std::ptrdiff_t>>(inc_data.at("stack_from_inc"));
    const std::vector<std::valarray<T>> power_entering_list = std::get<std::vector<std::valarray<T>>>(inc_data.at("power_entering_list"));
    const std::valarray<std::array<std::valarray<T>, 2>> stackFB_list = std::get<std::valarray<std::array<std::valarray<T>, 2>>>(inc_data.at("stackFB_list"));
    std::vector<std::valarray<T>> absorp_list;
    const std::size_t num_wl = power_entering_list.front().size();
    for (std::size_t i = 0; i < power_entering_list.size() - 1; i++) {
        if (stack_from_inc.at(i + 1) == -1) {
            absorp_list.emplace_back(power_entering_list.at(i) - power_entering_list.at(i + 1));
        } else {
            const std::size_t j = stack_from_inc.at(i + 1);
            const coh_tmm_vecn_dict<T> coh_tmm_data = std::get<std::vector<coh_tmm_vecn_dict<T>>>(inc_data.at("coh_tmm_data_list")).at(j);
            const coh_tmm_vecn_dict<T> coh_tmm_bdata = std::get<std::vector<coh_tmm_vecn_dict<T>>>(inc_data.at("coh_tmm_bdata_list")).at(j);
            const std::valarray<T> power_exiting = stackFB_list[j].front() * std::get<std::valarray<T>>(coh_tmm_data.at("power_entering")) - stackFB_list[j].back() * std::get<std::valarray<T>>(coh_tmm_bdata.at("T"));
            absorp_list.emplace_back(power_entering_list.at(i) - power_exiting);
            const std::valarray<std::valarray<T>> fcoh_absorp = stackFB_list[j].front() * absorp_in_each_layer(coh_tmm_data);
            const std::valarray<std::valarray<T>> bcoh_absorp = stackFB_list[j].back() * absorp_in_each_layer(coh_tmm_bdata);
            const std::size_t num_layers = fcoh_absorp.size();  // == d_list.size()
            const std::valarray<std::valarray<T>> stack_absorp = fcoh_absorp[std::slice(1, num_layers - 2, 1)] + bcoh_absorp[std::slice(num_layers - 2, num_layers - 2, -1)];
#ifdef __cpp_lib_containers_ranges
            absorp_list.append_range(stack_absorp);
#else
            absorp_list.insert(absorp_list.end(), std::cbegin(stack_absorp), std::cend(stack_absorp));
#endif
        }
    }
    absorp_list.push_back(std::get<std::valarray<T>>(inc_data.at("T")));
    for (std::valarray<T> &absorp : absorp_list) {
        absorp[absorp < 0] = 0;
    }
    return absorp_list;
}

template auto inc_absorp_in_each_layer(const inc_tmm_vec_dict<double> &inc_data) -> std::vector<std::valarray<double>>;

template<typename T>
auto inc_find_absorp_analytic_fn(const std::size_t layer, const inc_tmm_vec_dict<T> &inc_data) -> AbsorpAnalyticVecFn<T> {
    std::vector<std::size_t> j = std::get<std::vector<std::vector<std::size_t>>>(inc_data.at("stack_from_all")).at(layer);
    if (j.empty()) {
        throw std::runtime_error("Layer must be coherent for this function!");
    }
    std::size_t stackindex = j.front();
    std::size_t withinstackindex = j.back();
    AbsorpAnalyticVecFn<T> forwardfunc;
    forwardfunc.fill_in(std::get<std::vector<coh_tmm_vecn_dict<T>>>(inc_data.at("coh_tmm_data_list")).at(stackindex), withinstackindex);
    forwardfunc.scale(std::get<std::valarray<std::array<std::valarray<T>, 2>>>(inc_data.at("stackFB_list"))[stackindex].front());
    AbsorpAnalyticVecFn<T> backfunc;
    backfunc.fill_in(std::get<std::vector<coh_tmm_vecn_dict<T>>>(inc_data.at("coh_tmm_bdata_list")).at(stackindex), -1 - withinstackindex);
    backfunc.scale(std::get<std::valarray<std::array<std::valarray<T>, 2>>>(inc_data.at("stackFB_list"))[stackindex].back());
    backfunc.flip();
    forwardfunc.add(backfunc);
    return forwardfunc;
}

template auto inc_find_absorp_analytic_fn(std::size_t layer, const inc_tmm_vec_dict<double> &inc_data) -> AbsorpAnalyticVecFn<double>;

// helper type for the visitor
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

/*
 * This function is vectorized. Analogous to position_resolved, but
 * for layers (incoherent or coherent) in (partly) incoherent stacks.
 * This is a new function, not from Steven Byrnes' tmm package.
 * It assumes that in incoherent layers, we can assume the absorption has
 * a Beer-Lambert profile (this is not really correct; actually, the absorption
 * profile will depend on the coherence length, as discussed in the
 * documentation for the tmm package). This is an approximation in order
 * to be able to generate absorption profiles for partly coherent stacks.
 * Starting with output of inc_tmm(), calculate the Poynting vector
 * and absorbed energy density a distance "dist" into layer number "layer"
 * */
template<typename T>
auto inc_position_resolved(std::valarray<std::size_t> &&layer, const std::valarray<T> &dist,
                           const inc_tmm_vec_dict<T> &inc_tmm_data, const std::valarray<LayerType> &coherency_list,
                           const std::valarray<std::valarray<T>> &alphas,
                           const T zero_threshold) -> std::valarray<std::valarray<T>> {
    // If duplicate elements exist, after unique, the last elements will be indeterminate!
    const std::vector<std::valarray<T>> A_per_layer = inc_absorp_in_each_layer(inc_tmm_data);
    const std::size_t num_layers = A_per_layer.size();
    const std::size_t num_wl = A_per_layer.front().size();  // == alphas[0].size()
    std::vector<std::valarray<T>> fraction_reaching(num_layers, std::valarray<T>(num_wl));
    fraction_reaching.front() = 1 - A_per_layer.front();
    std::variant<std::valarray<std::valarray<T>>, std::valarray<std::valarray<std::complex<T>>>> A_layer;
    // Cumulative sum but cannot use std::accumulate
    std::valarray<std::valarray<T>> A_local(std::valarray<T>(num_wl), dist.size());  // Note the order of dims
    for (std::size_t i : std::views::iota(1U, num_layers)) {
        for (std::size_t j : std::views::iota(0U, num_wl)) {
            T cumsum_axis0 = A_per_layer.front()[j];
            for (std::size_t k : std::views::iota(1U, i + 1)) {
                cumsum_axis0 += A_per_layer.at(k)[j];
            }
            fraction_reaching.at(i)[j] = 1 - cumsum_axis0;
        }
    }
    // const std::ranges::subrange<std::size_t*, std::size_t*, (std::ranges::subrange_kind)1> layers = std::ranges::unique(layer);
    // Do not directly use the return value of std::ranges::unique() without erasing!
    std::vector<std::size_t> layers;
    std::ranges::unique_copy(layer, std::back_inserter(layers));
#ifdef __cpp_lib_ranges_enumerate
    for (const auto [i, l] : std::views::enumerate(layers)) {  // unique layer indices
#else
    for (std::size_t i = 0; i < layers.size(); ++i) {
        std::size_t l = layers.at(i);
#endif
        if (coherency_list[l] == LayerType::Coherent) {
            AbsorpAnalyticVecFn<T> fn = inc_find_absorp_analytic_fn(l, inc_tmm_data);
            A_layer = fn.run(dist[layer == l]);
        } else {
#ifdef _MSC_VER
            A_layer = beer_lambert(alphas[l], fraction_reaching[i], dist[layer == l] * 1e9, A_per_layer[l]);
#else
            A_layer = beer_lambert(alphas[l], fraction_reaching[i],
                                   std::valarray<T>(dist[layer == l] * 1e9), A_per_layer[l]);
#endif
        }
        // vaT (beer_lambert) is num_wl * num_llayers while vaCT (fn.run) is num_llayers * num_wl
        auto use_vaT = [i, zero_threshold, &fraction_reaching, num_wl](std::valarray<std::valarray<T>> &vaT) -> std::valarray<std::valarray<T>> {
            // std::mask_array<std::valarray<T>> vaT_cond = vaT[fraction_reaching[i] < zero_threshold];
            // Unfortunately, we cannot write vaT[fraction_reaching[i] < zero_threshold] = std::valarray<T>(0.0, vaT[0].size());
            vaT = Utils::Range::rng2l_transpose(vaT);
            // Now vaT is num_llayers * num_wl
            for (std::size_t j : std::views::iota(0U, vaT.size())) {
                vaT[j][fraction_reaching.at(i) < zero_threshold] = 0;
            }
            return vaT;
        };
        auto use_vaCT = [i, zero_threshold, &fraction_reaching, num_wl](std::valarray<std::valarray<std::complex<T>>> &vaCT) -> std::valarray<std::valarray<T>> {
            vaCT[fraction_reaching[i] < zero_threshold] = std::valarray<std::complex<T>>(0.0, vaCT[0].size());
            const std::size_t num_llayers = vaCT.size();
            std::valarray<std::valarray<T>> Areal_layer(std::valarray<T>(num_wl), num_llayers);
            for (std::size_t j : std::views::iota(0U, num_llayers)) {
                for (std::size_t k : std::views::iota(0U, num_wl)) {
                    Areal_layer[j][k] = (fraction_reaching.at(i)[k] < zero_threshold) ? 0 : vaCT[j][k].real();
                }
            }
            return Areal_layer;
        };
#if (__cpp_lib_variant >= 202306L)
        std::valarray<std::valarray<T>> Areal_layer = A_layer.std::visit(overloads{use_vaT, use_vaCT});
#else
        std::valarray<std::valarray<T>> Areal_layer = std::visit(overloads{use_vaT, use_vaCT}, A_layer);
#endif
        A_local[layer == l] = Areal_layer;
    }
    return A_local;
}

template auto inc_position_resolved(std::valarray<std::size_t> &&layer, const std::valarray<double> &dist,
                                    const inc_tmm_vec_dict<double> &inc_tmm_data,
                                    const std::valarray<LayerType> &coherency_list,
                                    const std::valarray<std::valarray<double>> &alphas,
                                    double zero_threshold) -> std::valarray<std::valarray<double>>;

template<typename T>
auto beer_lambert(const std::valarray<T> &alphas, const std::valarray<T> &fraction, const std::valarray<T> &dist,
                  const std::valarray<T> &A_total) -> std::valarray<std::valarray<T>> {
    const std::size_t sz_d = dist.size();
    const std::size_t sz_alpha = alphas.size();
    std::valarray<std::valarray<T>> expn(std::valarray<T>(sz_d), sz_alpha);
    // We are doing outer products here, similar to AbsorpAnalyticVecFn<T>::run()
    for (std::size_t i = 0; i < sz_alpha; i++) {
        for (std::size_t j = 0; j < sz_d; j++) {
            expn[i][j] = std::exp(-alphas[i] * dist[j]);
        }
    }
    const std::valarray<T> A_integrated = fraction * (1 - std::exp(-alphas * std::ranges::max(dist)));
    // Check std::ranges::contains(A_integrated, 0))
    std::valarray<T> scale = A_total / A_integrated;
    std::ranges::replace_if(scale, [](const T sc) -> bool {
        return std::isnan(sc) or std::isinf(sc);  // 0/0 is nan; otherwise /0 is inf.
    }, 0);
    std::valarray<std::valarray<T>> output(std::valarray<T>(sz_d), sz_alpha);
    for (std::size_t i = 0; i < sz_alpha; i++) {
        output[i] = scale[i] * fraction[i] * alphas[i] * expn[i];  // Not dividing 1e9 here
    }
    return output;
}

template auto beer_lambert(const std::valarray<double> &alphas, const std::valarray<double> &fraction,
                           const std::valarray<double> &dist,
                           const std::valarray<double> &A_total) -> std::valarray<std::valarray<double>>;

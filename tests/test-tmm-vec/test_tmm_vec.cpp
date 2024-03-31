//
// Created by Yihua Liu on 2023/11/27.
//

// GCC/Clang has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
#include <algorithm>
#include <cassert>
#include <numbers>
#include <functional>
#include "../../src/optics/tmm.h"
#include "../../src/utils/Approx.h"
#include "../../src/utils/Math.h"
#include "../../src/utils/Range.h"

using namespace std::complex_literals;

void test_snell() {
    // assert() does not support comma operators. However, you can have a look at
    // P2264 R1-[P2264R5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2264r5.html)
    // https://github.com/cplusplus/papers/issues/957
    // Make assert() macro user friendly for C and C++
    // assert(snell(3.0 + 0i, 2.0 + 0i, 0.7 + 0i) == approx<std::complex<double>, double>(1.3105496419558818));
    const ApproxScalar<std::complex<double>, double> snell_approx = approx<std::complex<double>, double>(1.3105496419558818);
    assert(snell(3.0 + 0i, 2.0 + 0i, 0.7 + 0i) == snell_approx);
}

void test_list_snell() {
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> list_snell_approx = approx<std::valarray<std::complex<double>>, double>(std::valarray<std::complex<double>>{0.3, 0.19831075, 0.14830313});
    const std::valarray<std::complex<double>> list_snell_res = list_snell(std::valarray<std::complex<double>>{2, 3, 4}, 0.3 + 0i);
    assert(list_snell_res == list_snell_approx);
}

void test_interface_r() {
    constexpr std::complex<double> th1 = 0.2;
    constexpr std::complex<double> n1 = 2;
    constexpr std::complex<double> n2 = 3;
    const std::complex<double> th2 = std::asin(n1 / n2 * std::sin(th1));
    const ApproxScalar<std::complex<double>, double> s_approx = approx<std::complex<double>, double>(-0.20541108217641596);
    const ApproxScalar<std::complex<double>, double> p_approx = approx<std::complex<double>, double>(0.19457669033430525);
    assert(interface_r('s', n1, n2, th1, th2) == s_approx);
    assert(interface_r('p', n1, n2, th1, th2) == p_approx);
}

void test_interface_t() {
    constexpr std::complex<double> th1 = 0.2;
    constexpr std::complex<double> n1 = 2;
    constexpr std::complex<double> n2 = 3;
    const std::complex<double> th2 = std::asin(n1 / n2 * std::sin(th1));
    const std::complex<double> s_res = interface_t('s', n1, n2, th1, th2);
    const std::complex<double> p_res = interface_t('p', n1, n2, th1, th2);
    const ApproxScalar<std::complex<double>, double> s_approx = approx<std::complex<double>, double>(0.7945889178235841);
    const ApproxScalar<std::complex<double>, double> p_approx = approx<std::complex<double>, double>(0.7963844602228701);
    assert(s_res == s_approx);
    assert(p_res == p_approx);
}

void test_R_from_r() {
    assert(R_from_r(std::numbers::sqrt2 + 1i * std::numbers::sqrt2) == 4.0);
}

void test_T_from_t() {
    constexpr std::complex<double> th1 = 0.2;
    constexpr std::complex<double> n1 = 2;
    constexpr std::complex<double> n2 = 3;
    const std::complex<double> th2 = std::asin(n1 / n2 * std::sin(th1));
    constexpr std::complex<double> ts = 0.7945889178235841;
    constexpr std::complex<double> tp = 0.7963844602228701;
    const ApproxScalar<std::complex<double>, double> Ts_approx = approx<std::complex<double>, double>(0.9578062873191139);
    const ApproxScalar<std::complex<double>, double> Tp_approx = approx<std::complex<double>, double>(0.962139911578548);
    assert(T_from_t('s', ts, n1, n2, th1, th2) == Ts_approx);
    assert(T_from_t('s', tp, n1, n2, th1, th2) == Tp_approx);
}

void test_power_entering_from_r() {
    constexpr std::complex<double> rs = -0.20541108217641596;
    constexpr std::complex<double> rp = 0.19457669033430525;

    constexpr std::complex<double> n1 = 2;
    constexpr std::complex<double> th1 = 0.2;

    const ApproxScalar<double, double> ps_approx = approx<double, double>(0.9578062873191138);
    const ApproxScalar<double, double> pp_approx = approx<double, double>(0.962139911578548);
    assert(power_entering_from_r('s', rs, n1, th1) == ps_approx);
    assert(power_entering_from_r('p', rp, n1, th1) == pp_approx);
}

void test_interface_R() {
    const std::valarray<std::complex<double>> th1 = {0.2, std::numbers::pi / 2};
    const std::valarray<std::complex<double>> n1 = {2, 2};
    const std::valarray<std::complex<double>> n2 = {3, 3};
    const std::valarray<std::complex<double>> th2 = std::asin(n1 / n2 * std::sin(th1));
    const ApproxSequenceLike<std::valarray<double>, double> interface_Rs_approx = approx<std::valarray<double>, double>({0.042193712680886314, 1});
    const ApproxSequenceLike<std::valarray<double>, double> interface_Rp_approx = approx<std::valarray<double>, double>({0.037860088421452116, 1});
    assert(interface_R('s', n1, n2, th1, th2) == interface_Rs_approx);
    assert(interface_R('p', n1, n2, th1, th2) == interface_Rp_approx);
}

void test_interface_T() {
    const std::valarray<std::complex<double>> th1 = {0.2, std::numbers::pi / 2};
    const std::valarray<std::complex<double>> n1 = {2, 2};
    const std::valarray<std::complex<double>> n2 = {3, 3};
    const std::valarray<std::complex<double>> th2 = std::asin(n1 / n2 * std::sin(th1));
    const ApproxSequenceLike<std::valarray<double>, double> interface_Ts_approx = approx<std::valarray<double>, double>({0.9578062873191139, 0});
    const ApproxSequenceLike<std::valarray<double>, double> interface_Tp_approx = approx<std::valarray<double>, double>({0.962139911578548, 0});
    assert(interface_T('s', n1, n2, th1, th2) == interface_Ts_approx);
    assert(interface_T('p', n1, n2, th1, th2) == interface_Tp_approx);
}

// Test for coh_tmm
void test_coh_tmm_exceptions() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, {10, 200, 187.3, 1973.5, INFINITY}, th_0, lam_vac);
    result = coh_tmm('s', n_list, d_list, 0.2 + 0.2i, lam_vac);
}

void test_coh_tmm_s_r() {
    // An interesting discussion on VLA and std::dynarray:
    // https://stackoverflow.com/questions/1887097/why-arent-variable-length-arrays-part-of-the-c-standard/21519062
    // https://stackoverflow.com/questions/61141691/are-there-plans-to-implement-stddynarray-in-next-c-standard
    // EWG 13 N3639, N3497, N3467, N3412 Runtime-sized arrays with automatic storage duration
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> sr_approx = approx<std::valarray<std::complex<double>>, double>({0.14017645 - 0.2132843i, 0.22307786 - 0.10704008i});
    assert(std::get<std::valarray<std::complex<double>>>(result.at("r")) == sr_approx);
}

void test_coh_tmm_s_t() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> st_approx = approx<std::valarray<std::complex<double>>, double>({1.78669633e-05 - 9.79824244e-06i, -8.86075993e-02 - 4.05953564e-01i});
    assert(std::get<std::valarray<std::complex<double>>>(result.at("t")) == st_approx);
}

void test_coh_tmm_s_R() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> sR_approx = approx<std::valarray<double>, double>({0.06513963, 0.06122131});
    assert(std::get<std::valarray<double>>(result.at("R")) == sR_approx);
}

void test_coh_tmm_s_T() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> sT_approx = approx<std::valarray<double>, double>({1.15234466e-09, 4.13619185e-01});
    assert(std::get<std::valarray<double>>(result.at("T")) == sT_approx);
}

void test_coh_tmm_s_power_entering() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> spower_approx = approx<std::valarray<double>, double>({0.93486037, 0.93877869});
    assert(std::get<std::valarray<double>>(result.at("power_entering")) == spower_approx);
}

void test_coh_tmm_s_vw_list() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::vector<std::complex<double>>, double> svwl_approx = approx<std::vector<std::complex<double>>, double>({
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            1.18358724e+00 - 2.33272105e-01i,
            -4.34107939e-02 + 1.99878010e-02i,
            1.03160316e+00 - 7.28921467e-02i,
            1.91474694e-01 - 3.41479380e-02i,
            -8.59535500e-02 + 1.06568462e-01i,
            -1.36521327e-09 + 2.83859953e-10i,
            6.08369346e-01 + 5.06683493e-01i,
            1.75320349e-01 - 9.58306162e-02i,
            -1.23112929e-05 + 1.37276841e-05i,
            -1.94390395e-06 + 2.16097082e-06i,
            -6.54156818e-02 + 3.57104644e-01i,
            3.38453387e-02 + 4.04808706e-02i,
            1.78669633e-05 - 9.79824244e-06i,
            0.00000000e+00 + 0.00000000e+00i,
            -8.86075993e-02 - 4.05953564e-01i,
            0.00000000e+00 + 0.00000000e+00i
    });
    const std::vector<std::complex<double>> s_vw_list = Utils::Range::vva2_flatten<std::valarray<std::vector<std::array<std::complex<double>, 2>>>, std::complex<double>, 2>(std::get<std::valarray<std::vector<std::array<std::complex<double>, 2>>>>(result.at("vw_list")));
    assert(s_vw_list == svwl_approx);
}

void test_coh_tmm_p_power_entering() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> ppower_approx = approx<std::valarray<double>, double>({0.96244989, 0.94994018});
    assert(std::get<std::valarray<double>>(result.at("power_entering")) == ppower_approx);
}

void test_coh_tmm_p_vw_list() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::vector<std::complex<double>>, double> pvwl_approx = approx<std::vector<std::complex<double>>, double>({
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            1.17017431e+00 - 2.43748228e-01i,
            4.40679361e-02 - 1.53940000e-02i,
            1.02922989e+00 - 7.82628087e-02i,
            -1.84573000e-01 + 1.79809491e-02i,
            -8.59886075e-02 + 1.13689959e-01i,
            1.39851113e-09 - 3.01497601e-10i,
            6.07730278e-01 + 5.07144030e-01i,
            -1.68609283e-01 + 8.64966880e-02i,
            -1.23967610e-05 + 1.45623920e-05i,
            1.93199813e-06 - 2.24107827e-06i,
            -6.52504472e-02 + 3.60299246e-01i,
            -3.33430797e-02 - 3.97852657e-02i,
            1.82536479e-05 - 1.06422631e-05i,
            0.00000000e+00 + 0.00000000e+00i,
            -9.02947159e-02 - 4.09448171e-01i,
            0.00000000e+00 + 0.00000000e+00i
    });
    const std::vector<std::complex<double>> p_vw_list = Utils::Range::vva2_flatten<std::valarray<std::vector<std::array<std::complex<double>, 2>>>, std::complex<double>, 2>(std::get<std::valarray<std::vector<std::array<std::complex<double>, 2>>>>(result.at("vw_list")));
    assert(p_vw_list == pvwl_approx);
}

void test_coh_tmm_kz_list() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> kzl_approx = approx<std::valarray<std::complex<double>>, double>({
            0.02250959+0.00000000e+00i, 0.00440866+0.00000000e+00i,
            0.01435451+0.00687561i, 0.00404247+0.00074813i,
            0.03118008+0.04748033i, 0.00515452+0.00110011i,
            0.07823055+0.00000000e+00i, 0.01413365+0.00000000e+00i,
            0.06246792+0.01579948i, 0.01056188+0.00035793i
    }, 1e-5);
    assert(std::get<std::valarray<std::complex<double>>>(result.at("kz_list")) == kzl_approx);
}

void test_coh_tmm_th_list() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> thl_approx = approx<std::valarray<std::complex<double>>, double>({
            0.3+0.i, 0.3+0.i,
            0.38659626-0.16429512i, 0.3162772 -0.05459799i,
            0.06789345-0.10235287i, 0.24849917-0.0507924i,
            0.08877261+0.i        , 0.09619234+0.i,
            0.10445527-0.02621521i, 0.12826687-0.00429919i
    });
    assert(std::get<std::valarray<std::complex<double>>>(result.at("th_list")) == thl_approx);
}

void test_coh_tmm_inputs() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    assert(std::get<char>(result.at("pol")) == 's');
    // Method 1: indirect way using operator- of valarray.
#ifdef _MSC_VER
    assert(std::ranges::all_of(std::get<std::valarray<std::complex<double>>>(result.at("n_list")) - n_list, std::bind_front(std::equal_to<>(), 0.0)));
#else
    assert(std::ranges::all_of(std::valarray<std::complex<double>>(std::get<std::valarray<std::complex<double>>>(result.at("n_list")) - n_list), std::bind_front(std::equal_to<>(), 0.0)));
#endif
    // Method 2: direct way to compare two ranges. There is also a traditional version std::equal.
    assert(std::ranges::equal(std::get<std::valarray<double>>(result.at("d_list")), d_list));
    assert(std::get<std::complex<double>>(result.at("th_0")) == th_0);
    assert(std::ranges::equal(std::get<std::valarray<double>>(result.at("lam_vac")), lam_vac));
}
// end of tests for coh_tmm

void test_coh_tmm_reverse() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm_reverse('s', n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> r_approx = approx<std::valarray<std::complex<double>>, double>({0.44495594 - 0.08125146i, 0.13483962 - 0.37537464i});
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> t_approx = approx<std::valarray<std::complex<double>>, double>({5.64612420e-05 - 1.46509665e-05i, -1.94928443e-01 - 9.82812305e-01i});
    const ApproxSequenceLike<std::valarray<double>, double> R_approx = approx<std::valarray<double>, double>({0.20458758, 0.15908784});
    const ApproxSequenceLike<std::valarray<double>, double> T_approx = approx<std::valarray<double>, double>({1.22605928e-09, 4.19050975e-01});
    const ApproxSequenceLike<std::valarray<double>, double> power_approx = approx<std::valarray<double>, double>({0.75431194, 0.84091216});
    const ApproxSequenceLike<std::vector<std::complex<double>>, double> vwl_approx = approx<std::vector<std::complex<double>>, double>({
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            0.00000000e+00 + 0.00000000e+00i,
            9.35877457e-01 + 4.78628162e-02i,
            5.09078478e-01 - 1.29114277e-01i,
            8.90678139e-01 - 4.74324242e-02i,
            2.44161483e-01 - 3.27942215e-01i,
            -1.22416652e+00 - 1.12459390e-01i,
            -1.29313936e-08 + 6.59495568e-09i,
            -1.13691217e+00 + 6.36142041e-01i,
            -2.07647974e-02 - 4.70097431e-02i,
            -2.63661542e-04 + 6.60611182e-05i,
            -4.98027806e-07 - 4.91628647e-06i,
            -1.06956388e+00 - 5.45729218e-01i,
            5.53546526e-02 + 6.76305892e-02i,
            5.64612420e-05 - 1.46509665e-05i,
            0.00000000e+00 + 0.00000000e+00i,
            -1.94928443e-01 - 9.82812305e-01i,
            0.00000000e+00 + 0.00000000e+00i
    });
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> kzl_approx = approx<std::valarray<std::complex<double>>, double>({
            0.06246792+0.01579948i, 0.01056179+0.i        ,
            0.07823055+0.i        , 0.01413365+0.i        ,
            0.03118008+0.04748033i, 0.00515452+0.00110011i,
            0.01435451+0.00687561i, 0.00404247+0.00074813i,
            0.02250959+0.i        , 0.00440866+0.i
    }, 1e-5);
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> thl_approx = approx<std::valarray<std::complex<double>>, double>({
            0.10445527-0.02621521i, 0.12841137+0.i        ,
            0.08877261+0.i        , 0.09619234+0.i        ,
            0.06789345-0.10235287i, 0.24849917-0.0507924i ,
            0.38659626-0.16429512i, 0.3162772 -0.05459799i,
            0.3       +0.i        , 0.3       +0.i
    });
    assert(std::get<std::valarray<std::complex<double>>>(result.at("r")) == r_approx);
    assert(std::get<std::valarray<std::complex<double>>>(result.at("t")) == t_approx);
    assert(std::get<std::valarray<double>>(result.at("R")) == R_approx);
    assert(std::get<std::valarray<double>>(result.at("T")) == T_approx);
    assert(std::get<std::valarray<double>>(result.at("power_entering")) == power_approx);
    const std::vector<std::complex<double>> vw_list = Utils::Range::vva2_flatten<std::valarray<std::vector<std::array<std::complex<double>, 2>>>, std::complex<double>, 2>(std::get<std::valarray<std::vector<std::array<std::complex<double>, 2>>>>(result.at("vw_list")));
    assert(vw_list == vwl_approx);
    assert(std::get<std::valarray<std::complex<double>>>(result.at("th_list")) == thl_approx);
    assert(std::get<std::valarray<std::complex<double>>>(result.at("kz_list")) == kzl_approx);
}

void test_ellips_psi() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const std::unordered_map<std::string, std::valarray<double>> psi_result = ellips(n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> psi_approx = approx<std::valarray<double>, double>({0.64939282, 0.73516374});
    assert(psi_result.at("psi") == psi_approx);
}

void test_ellips_Delta() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const std::unordered_map<std::string, std::valarray<double>> Delta_result = ellips(n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> Delta_approx = approx<std::valarray<double>, double>({0.64939282, 0.73516374});
    assert(Delta_result.at("psi") == Delta_approx);
}

void test_unpolarized_RT_R() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const std::unordered_map<std::string, std::valarray<double>> RT_result = unpolarized_RT(n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> RT_R_approx = approx<std::valarray<double>, double>({0.05134487, 0.05564057});
    assert(RT_result.at("R") == RT_R_approx);
}

void test_unpolarized_RT_T() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = Utils::Range::rng2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const std::unordered_map<std::string, std::valarray<double>> RT_result = unpolarized_RT(n_list, d_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> RT_T_approx = approx<std::valarray<double>, double>({1.19651603e-09, 4.17401823e-01});
    std::valarray<double> temp = RT_result.at("T");
    assert(RT_result.at("T") == RT_T_approx);
}

void test_position_resolved_s_poyn() {
    const coh_tmm_vec_dict<double> coh_tmm_data = {
            {"r", std::valarray<std::complex<double>>{0.14017645 - 0.2132843i, 0.22307786 - 0.10704008i}},
            {"t", std::valarray<std::complex<double>>{1.78669633e-05 - 9.79824244e-06i, -8.86075993e-02 - 4.05953564e-01i}},
            {"R", std::valarray<double>{0.06513963, 0.06122131}},
            {"T", std::valarray<double>{1.15234466e-09, 4.13619185e-01}},
            {"power_entering", std::valarray<double>{0.93486037, 0.93877869}},
            {"vw_list", std::valarray<std::vector<std::array<std::complex<double>, 2>>>{
                    {{0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i}},
                    {{1.18358724e+00 - 2.33272105e-01i,
                      -4.34107939e-02 + 1.99878010e-02i},
                     {1.03160316e+00 - 7.28921467e-02i,
                      1.91474694e-01 - 3.41479380e-02i}},
                    {{-8.59535500e-02 + 1.06568462e-01i,
                      -1.36521327e-09 + 2.83859953e-10i},
                     {6.08369346e-01 + 5.06683493e-01i,
                      1.75320349e-01 - 9.58306162e-02i}},
                    {{-1.23112929e-05 + 1.37276841e-05i,
                      -1.94390395e-06 + 2.16097082e-06i},
                     {-6.54156818e-02 + 3.57104644e-01i,
                      3.38453387e-02 + 4.04808706e-02i}},
                    {{1.78669633e-05 - 9.79824244e-06i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {-8.86075993e-02 - 4.05953564e-01i,
                      0.00000000e+00 + 0.00000000e+00i}}
            }},
            {"kz_list", std::valarray<std::complex<double>>{
                    0.02250959+0.i        , 0.00440866+0.i        ,
                    0.01435451+0.00687561i, 0.00404247+0.00074813i,
                    0.03118008+0.04748033i, 0.00515452+0.00110011i,
                    0.07823055+0.i        , 0.01413365+0.i        ,
                    0.06246792+0.01579948i, 0.01056188+0.00035793i
            }},
            {"th_list", std::valarray<std::complex<double>>{
                    0.3       +0.i        , 0.3       +0.i        ,
                    0.38659626-0.16429512i, 0.3162772 -0.05459799i,
                    0.06789345-0.10235287i, 0.24849917-0.0507924i ,
                    0.08877261+0.i        , 0.09619234+0.i        ,
                    0.10445527-0.02621521i, 0.12826687-0.00429919i
            }},
            {"pol", 's'},
            {"n_list", std::valarray<std::complex<double>>{
                    1.5+0.i , 1.3+0.i ,
                    1. +0.4i, 1.2+0.2i,
                    2. +3.i , 1.5+0.3i,
                    5. +0.i , 4. +0.i ,
                    4. +1.i , 3. +0.1i
            }},
            {"d_list", std::valarray<double>{INFINITY, 200, 187.3, 1973.5, INFINITY}},
            {"th_0", 0.3},
            {"lam_vac", std::valarray<double>{400, 1770}}
    };
    const std::unordered_map<std::string, std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> pr_result = position_resolved({0, 1, 1, 2, 2, 3, 3, 4, 4},
                                                                                                                                                  {20, 10, 200, 20, 50.8, 10, 2000, 0, 200},
                                                                                                                                                  coh_tmm_data);
    const ApproxSequenceLike<std::valarray<double>, double> poyn_approx = approx<std::valarray<double>, double>(Utils::Range::rng2d_transpose(std::valarray<double>{
            0.00000000e+00, 8.24769450e-01, 2.59652919e-02, 3.88663596e-03,
            2.08618804e-04, 1.15234465e-09, 1.15234465e-09, 1.15234466e-09,
            2.07458676e-12,
            0.00000000e+00, 9.18232659e-01, 6.12787705e-01, 5.74755463e-01,
            5.25103871e-01, 4.13619185e-01, 4.13619185e-01, 4.13619186e-01,
            3.58444455e-01
    }, 2));
    assert(std::get<std::valarray<double>>(pr_result.at("poyn")) == poyn_approx);
}

void test_position_resolved_s_absor() {
    const coh_tmm_vec_dict<double> coh_tmm_data = {
            {"r", std::valarray<std::complex<double>>{0.14017645 - 0.2132843i, 0.22307786 - 0.10704008i}},
            {"t", std::valarray<std::complex<double>>{1.78669633e-05 - 9.79824244e-06i, -8.86075993e-02 - 4.05953564e-01i}},
            {"R", std::valarray<double>{0.06513963, 0.06122131}},
            {"T", std::valarray<double>{1.15234466e-09, 4.13619185e-01}},
            {"power_entering", std::valarray<double>{0.93486037, 0.93877869}},
            {"vw_list", std::valarray<std::vector<std::array<std::complex<double>, 2>>>{
                    {{0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i}},
                    {{1.18358724e+00 - 2.33272105e-01i,
                      -4.34107939e-02 + 1.99878010e-02i},
                     {1.03160316e+00 - 7.28921467e-02i,
                      1.91474694e-01 - 3.41479380e-02i}},
                    {{-8.59535500e-02 + 1.06568462e-01i,
                      -1.36521327e-09 + 2.83859953e-10i},
                     {6.08369346e-01 + 5.06683493e-01i,
                      1.75320349e-01 - 9.58306162e-02i}},
                    {{-1.23112929e-05 + 1.37276841e-05i,
                      -1.94390395e-06 + 2.16097082e-06i},
                     {-6.54156818e-02 + 3.57104644e-01i,
                      3.38453387e-02 + 4.04808706e-02i}},
                    {{1.78669633e-05 - 9.79824244e-06i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {-8.86075993e-02 - 4.05953564e-01i,
                      0.00000000e+00 + 0.00000000e+00i}}
            }},
            {"kz_list", std::valarray<std::complex<double>>{
                    0.02250959+0.i        , 0.00440866+0.i        ,
                    0.01435451+0.00687561i, 0.00404247+0.00074813i,
                    0.03118008+0.04748033i, 0.00515452+0.00110011i,
                    0.07823055+0.i        , 0.01413365+0.i        ,
                    0.06246792+0.01579948i, 0.01056188+0.00035793i
            }},
            {"th_list", std::valarray<std::complex<double>>{
                    0.3       +0.i        , 0.3       +0.i        ,
                    0.38659626-0.16429512i, 0.3162772 -0.05459799i,
                    0.06789345-0.10235287i, 0.24849917-0.0507924i ,
                    0.08877261+0.i        , 0.09619234+0.i        ,
                    0.10445527-0.02621521i, 0.12826687-0.00429919i
            }},
            {"pol", 's'},
            {"n_list", std::valarray<std::complex<double>>{
                    1.5+0.i , 1.3+0.i ,
                    1. +0.4i, 1.2+0.2i,
                    2. +3.i , 1.5+0.3i,
                    5. +0.i , 4. +0.i ,
                    4. +1.i , 3. +0.1i
            }},
            {"d_list", std::valarray<double>{INFINITY, 200, 187.3, 1973.5, INFINITY}},
            {"th_0", 0.3},
            {"lam_vac", std::valarray<double>{400, 1770}}
    };
    const std::unordered_map<std::string, std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> pr_result = position_resolved({0, 1, 1, 2, 2, 3, 3, 4, 4},
                                                                                                                                                  {20, 10, 200, 20, 50.8, 10, 2000, 0, 200},
                                                                                                                                                  coh_tmm_data);
    const ApproxSequenceLike<std::valarray<double>, double> absorption_approx = approx<std::valarray<double>, double>(Utils::Range::rng2d_transpose(std::valarray<double>{
            0.00000000e+00, 1.02697381e-02, 1.64378595e-04, 3.69077590e-04,
            1.98105861e-05, 0.00000000e+00, 0.00000000e+00, 3.64128877e-11,
            6.55547748e-14,
            0.00000000e+00, 2.04057536e-03, 1.07421979e-03, 1.78805499e-03,
            1.43715316e-03, 0.00000000e+00, 0.00000000e+00, 2.96091644e-04,
            2.56594499e-04
    }, 2));
    assert(std::get<std::valarray<double>>(pr_result.at("absor")) == absorption_approx);
}

void test_position_resolved_p_poyn() {
    const coh_tmm_vec_dict<double> coh_tmm_data = {
            {"r", std::valarray<std::complex<double>>{-0.12140058 + 0.15103645i, -0.21104259 + 0.07430242i}},
            {"t", std::valarray<std::complex<double>>{1.82536479e-05 - 1.06422631e-05i, -9.02947159e-02 - 4.09448171e-01i}},
            {"R", std::valarray<double>{0.03755011, 0.05005982}},
            {"T", std::valarray<double>{1.24068740e-09, 4.21184461e-01}},
            {"power_entering", std::valarray<double>{0.96244989, 0.94994018}},
            {"vw_list", std::valarray<std::vector<std::array<std::complex<double>, 2>>>{
                    {{0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i}},
                    {{1.17017431e+00 - 2.43748228e-01i,
                      4.40679361e-02 - 1.53940000e-02i},
                     {1.02922989e+00 - 7.82628087e-02i,
                      -1.84573000e-01 + 1.79809491e-02i}},
                    {{-8.59886075e-02 + 1.13689959e-01i,
                      1.39851113e-09 - 3.01497601e-10i},
                     {6.07730278e-01 + 5.07144030e-01i,
                      -1.68609283e-01 + 8.64966880e-02i}},
                    {{-1.23967610e-05 + 1.45623920e-05i,
                      1.93199813e-06 - 2.24107827e-06i},
                     {-6.52504472e-02 + 3.60299246e-01i,
                      -3.33430797e-02 - 3.97852657e-02i}},
                    {{1.82536479e-05 - 1.06422631e-05i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {-9.02947159e-02 - 4.09448171e-01i,
                      0.00000000e+00 + 0.00000000e+00i}}
            }},
            {"kz_list", std::valarray<std::complex<double>>{
                    0.02250959+0.i        , 0.00440866+0.i        ,
                    0.01435451+0.00687561i, 0.00404247+0.00074813i,
                    0.03118008+0.04748033i, 0.00515452+0.00110011i,
                    0.07823055+0.i        , 0.01413365+0.i        ,
                    0.06246792+0.01579948i, 0.01056188+0.00035793i
            }},
            {"th_list", std::valarray<std::complex<double>>{
                    0.3       +0.i        , 0.3       +0.i        ,
                    0.38659626-0.16429512i, 0.3162772 -0.05459799i,
                    0.06789345-0.10235287i, 0.24849917-0.0507924i ,
                    0.08877261+0.i        , 0.09619234+0.i        ,
                    0.10445527-0.02621521i, 0.12826687-0.00429919i
            }},
            {"pol", 'p'},
            {"n_list", std::valarray<std::complex<double>>{
                    1.5+0.i , 1.3+0.i ,
                    1. +0.4i, 1.2+0.2i,
                    2. +3.i , 1.5+0.3i,
                    5. +0.i , 4. +0.i ,
                    4. +1.i , 3. +0.1i
            }},
            {"d_list", std::valarray<double>{INFINITY, 200, 187.3, 1973.5, INFINITY}},
            {"th_0", 0.3},
            {"lam_vac", std::valarray<double>{400, 1770}}
    };
    const std::unordered_map<std::string, std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> pr_result = position_resolved({0, 1, 1, 2, 2, 3, 3, 4, 4},
                                                                                                                                                  {20, 10, 200, 20, 50.8, 10, 2000, 0, 200},
                                                                                                                                                  coh_tmm_data);
    const ApproxSequenceLike<std::valarray<double>, double> poyn_approx = approx<std::valarray<double>, double>(Utils::Range::rng2d_transpose(std::valarray<double>{
            0.00000000e+00, 8.45520745e-01, 2.87382013e-02, 4.30170256e-03,
            2.30897897e-04, 1.24068740e-09, 1.24068740e-09, 1.24068740e-09,
            2.23363177e-12,
            0.00000000e+00, 9.30639570e-01, 6.33536065e-01, 5.95958829e-01,
            5.45941168e-01, 4.21184460e-01, 4.21184460e-01, 4.21184461e-01,
            3.65000560e-01
    }, 2));
    assert(std::get<std::valarray<double>>(pr_result.at("poyn")) == poyn_approx);
}

void test_position_resolved_p_absor() {
    const coh_tmm_vec_dict<double> coh_tmm_data = {
            {"r", std::valarray<std::complex<double>>{-0.12140058 + 0.15103645i, -0.21104259 + 0.07430242i}},
            {"t", std::valarray<std::complex<double>>{1.82536479e-05 - 1.06422631e-05i, -9.02947159e-02 - 4.09448171e-01i}},
            {"R", std::valarray<double>{0.03755011, 0.05005982}},
            {"T", std::valarray<double>{1.24068740e-09, 4.21184461e-01}},
            {"power_entering", std::valarray<double>{0.96244989, 0.94994018}},
            {"vw_list", std::valarray<std::vector<std::array<std::complex<double>, 2>>>{
                    {{0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i}},
                    {{1.17017431e+00 - 2.43748228e-01i,
                      4.40679361e-02 - 1.53940000e-02i},
                     {1.02922989e+00 - 7.82628087e-02i,
                      -1.84573000e-01 + 1.79809491e-02i}},
                    {{-8.59886075e-02 + 1.13689959e-01i,
                      1.39851113e-09 - 3.01497601e-10i},
                     {6.07730278e-01 + 5.07144030e-01i,
                      -1.68609283e-01 + 8.64966880e-02i}},
                    {{-1.23967610e-05 + 1.45623920e-05i,
                      1.93199813e-06 - 2.24107827e-06i},
                     {-6.52504472e-02 + 3.60299246e-01i,
                      -3.33430797e-02 - 3.97852657e-02i}},
                    {{1.82536479e-05 - 1.06422631e-05i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {-9.02947159e-02 - 4.09448171e-01i,
                      0.00000000e+00 + 0.00000000e+00i}}
            }},
            {"kz_list", std::valarray<std::complex<double>>{
                    0.02250959+0.i        , 0.00440866+0.i        ,
                    0.01435451+0.00687561i, 0.00404247+0.00074813i,
                    0.03118008+0.04748033i, 0.00515452+0.00110011i,
                    0.07823055+0.i        , 0.01413365+0.i        ,
                    0.06246792+0.01579948i, 0.01056188+0.00035793i
            }},
            {"th_list", std::valarray<std::complex<double>>{
                    0.3       +0.i        , 0.3       +0.i        ,
                    0.38659626-0.16429512i, 0.3162772 -0.05459799i,
                    0.06789345-0.10235287i, 0.24849917-0.0507924i ,
                    0.08877261+0.i        , 0.09619234+0.i        ,
                    0.10445527-0.02621521i, 0.12826687-0.00429919i
            }},
            {"pol", 'p'},
            {"n_list", std::valarray<std::complex<double>>{
                    1.5+0.i , 1.3+0.i ,
                    1. +0.4i, 1.2+0.2i,
                    2. +3.i , 1.5+0.3i,
                    5. +0.i , 4. +0.i ,
                    4. +1.i , 3. +0.1i
            }},
            {"d_list", std::valarray<double>{INFINITY, 200, 187.3, 1973.5, INFINITY}},
            {"th_0", 0.3},
            {"lam_vac", std::valarray<double>{400, 1770}}
    };
    const std::unordered_map<std::string, std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> pr_result = position_resolved({0, 1, 1, 2, 2, 3, 3, 4, 4},
                                                                                                                                                  {20, 10, 200, 20, 50.8, 10, 2000, 0, 200},
                                                                                                                                                  coh_tmm_data);
    const ApproxSequenceLike<std::valarray<double>, double> absor_approx = approx<std::valarray<double>, double>(Utils::Range::rng2d_transpose(std::valarray<double>{
            0.00000000e+00, 1.08969642e-02, 5.17505408e-04, 4.08492602e-04,
            2.19262267e-05, 0.00000000e+00, 0.00000000e+00, 3.92044315e-11,
            7.05804410e-14,
            0.00000000e+00, 1.91824256e-03, 1.12566565e-03, 1.77891531e-03,
            1.46979454e-03, 0.00000000e+00, 0.00000000e+00, 3.01509108e-04,
            2.61289301e-04
    }, 2));
    assert(std::get<std::valarray<double>>(pr_result.at("absor")) == absor_approx);
}

void test_find_in_structure_exception() {
    const std::pair<std::valarray<std::iterator_traits<double *>::difference_type>, std::valarray<double>> result = find_in_structure<double>(
            {INFINITY, 100, 200, INFINITY}, {0, 100, 200});
}

void test_find_in_structure() {
    // Approx only applies to std::floating_point
    const std::pair<std::valarray<std::iterator_traits<double *>::difference_type>, std::valarray<double>> result = find_in_structure({200, 187.3, 1973.5}, Utils::Math::linspace_va<double>(0, 700, 10));
    const std::valarray<std::iterator_traits<double *>::difference_type> layer_approx{1, 1, 1, 2, 2, 3, 3, 3, 3, 3};
    assert(std::ranges::equal(result.first, layer_approx));
    const ApproxSequenceLike<std::valarray<double>, double> dist_approx = approx<std::valarray<double>, double>({0., 77.77777778, 155.55555556, 33.33333333, 111.11111111,
                                                         1.58888889, 79.36666667,157.14444444, 234.92222222, 312.7});
    assert(result.second == dist_approx);
}

void test_find_in_structure_inf() {
    const std::pair<std::valarray<std::size_t>, std::valarray<double>> result = find_in_structure_inf({INFINITY, 200, 187.3, 1973.5, INFINITY}, Utils::Math::linspace_va<double>(0, 700, 10));
    const std::valarray<std::iterator_traits<double *>::difference_type> layer_approx{1, 1, 1, 2, 2, 3, 3, 3, 3, 3};
    assert(std::ranges::equal(result.first, layer_approx));
    const ApproxSequenceLike<std::valarray<double>, double> dist_approx = approx<std::valarray<double>, double>({0., 77.77777778, 155.55555556, 33.33333333, 111.11111111,
                                                         1.58888889, 79.36666667,157.14444444, 234.92222222, 312.7});
    assert(result.second == dist_approx);
}

void test_layer_starts() {
    const std::valarray<double> layer_starts_result = layer_starts(std::valarray<double>{INFINITY, 200, 187.3, 1973.5, INFINITY});
    const std::valarray<double> layer_starts_approx = {-INFINITY, 0., 200., 387.3, 2360.8};
    assert(std::ranges::equal(layer_starts_result, layer_starts_approx));
}

// tests for absorp_analytic_fn

void test_fill_in_s() {
    const coh_tmm_vec_dict<double> coh_tmm_data = {
            {"r", std::valarray<std::complex<double>>{0.14017645 - 0.2132843i, 0.22307786 - 0.10704008i}},
            {"t", std::valarray<std::complex<double>>{1.78669633e-05 - 9.79824244e-06i, -8.86075993e-02 - 4.05953564e-01i}},
            {"R", std::valarray<double>{0.06513963, 0.06122131}},
            {"T", std::valarray<double>{1.15234466e-09, 4.13619185e-01}},
            {"power_entering", std::valarray<double>{0.93486037, 0.93877869}},
            {"vw_list", std::valarray<std::vector<std::array<std::complex<double>, 2>>>{
                    {{0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i}},
                    {{1.18358724e+00 - 2.33272105e-01i,
                      -4.34107939e-02 + 1.99878010e-02i},
                     {1.03160316e+00 - 7.28921467e-02i,
                      1.91474694e-01 - 3.41479380e-02i}},
                    {{-8.59535500e-02 + 1.06568462e-01i,
                      -1.36521327e-09 + 2.83859953e-10i},
                     {6.08369346e-01 + 5.06683493e-01i,
                      1.75320349e-01 - 9.58306162e-02i}},
                    {{-1.23112929e-05 + 1.37276841e-05i,
                      -1.94390395e-06 + 2.16097082e-06i},
                     {-6.54156818e-02 + 3.57104644e-01i,
                      3.38453387e-02 + 4.04808706e-02i}},
                    {{1.78669633e-05 - 9.79824244e-06i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {-8.86075993e-02 - 4.05953564e-01i,
                      0.00000000e+00 + 0.00000000e+00i}}
            }},
            {"kz_list", std::valarray<std::complex<double>>{
                    0.02250959+0.i        , 0.00440866+0.i        ,
                    0.01435451+0.00687561i, 0.00404247+0.00074813i,
                    0.03118008+0.04748033i, 0.00515452+0.00110011i,
                    0.07823055+0.i        , 0.01413365+0.i        ,
                    0.06246792+0.01579948i, 0.01056188+0.00035793i
            }},
            {"th_list", std::valarray<std::complex<double>>{
                    0.3       +0.i        , 0.3       +0.i        ,
                    0.38659626-0.16429512i, 0.3162772 -0.05459799i,
                    0.06789345-0.10235287i, 0.24849917-0.0507924i ,
                    0.08877261+0.i        , 0.09619234+0.i        ,
                    0.10445527-0.02621521i, 0.12826687-0.00429919i
            }},
            {"pol", 's'},
            {"n_list", std::valarray<std::complex<double>>{
                    1.5+0.i , 1.3+0.i ,
                    1. +0.4i, 1.2+0.2i,
                    2. +3.i , 1.5+0.3i,
                    5. +0.i , 4. +0.i ,
                    4. +1.i , 3. +0.1i
            }},
            {"d_list", std::valarray<double>{INFINITY, 200, 187.3, 1973.5, INFINITY}},
            {"th_0", 0.3},
            {"lam_vac", std::valarray<double>{400, 1770}}
    };
    AbsorpAnalyticVecFn<double> a;
    a.fill_in(coh_tmm_data, {1, 2});
    std::vector<std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> re_result(5);
    re_result.front() = a.a1;
    re_result.at(1) = a.a3;
    re_result.at(2) = a.A1;
    re_result.at(3) = a.A2;
    re_result.back() = a.A3;
    std::vector<std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> re_approx(5);
    re_approx.front() = std::valarray{0.01375122, 0.00149626, 0.09496066, 0.00220022};
    re_approx.at(1) = std::valarray{0.02870902, 0.00808494, 0.06236016, 0.01030904};
    re_approx.at(2) = std::valarray{2.00290348e-05, 5.19001441e-05, 2.55761667e-19, 1.02694503e-04};
    re_approx.at(3) = std::valarray{0.01276183, 0.00146736, 0.00246567, 0.00161252};
    re_approx.back() = std::valarray{-4.91455269e-04 - 1.18654706e-04i,  2.74416636e-04 + 2.91821819e-05i,
        1.94145098e-11 - 1.59280064e-11i,  1.49469559e-04 + 3.78492113e-04i};
    for (const std::pair<std::variant<std::valarray<double>, std::valarray<std::complex<double>>> &, std::variant<std::valarray<double>, std::valarray<std::complex<double>>> &> &elem : std::views::zip(re_result, re_approx)) {
        const std::variant<std::valarray<double>, std::valarray<std::complex<double>>> r = elem.first;
        const std::variant<std::valarray<double>, std::valarray<std::complex<double>>> e = elem.second;
        const std::valarray<double> *prvt = std::get_if<std::valarray<double>>(&r);
        const std::valarray<double> *pevt = std::get_if<std::valarray<double>>(&e);
        const std::valarray<std::complex<double>> *prvct = std::get_if<std::valarray<std::complex<double>>>(&r);
        const std::valarray<std::complex<double>> *pevct = std::get_if<std::valarray<std::complex<double>>>(&e);
        if (prvt and pevt) {
            assert(std::ranges::equal(*prvt, *pevt, [](const double rt, const double et) -> bool {
                return rt == approx(et, 1e-5);
            }));
        } else if (prvct and pevct) {
            assert(std::ranges::equal(*prvct, *pevct, [](const std::complex<double> rct, const std::complex<double> ect) -> bool {
                return rct == approx(ect, 1e-5);
            }));
        } else {
            throw std::runtime_error("r and e type mismatches.");
        }
    }
}

void test_fill_in_p() {
    const coh_tmm_vec_dict<double> coh_tmm_data = {
            {"r", std::valarray<std::complex<double>>{-0.12140058 + 0.15103645i, -0.21104259 + 0.07430242i}},
            {"t", std::valarray<std::complex<double>>{1.82536479e-05 - 1.06422631e-05i, -9.02947159e-02 - 4.09448171e-01i}},
            {"R", std::valarray<double>{0.03755011, 0.05005982}},
            {"T", std::valarray<double>{1.24068740e-09, 4.21184461e-01}},
            {"power_entering", std::valarray<double>{0.96244989, 0.94994018}},
            {"vw_list", std::valarray<std::vector<std::array<std::complex<double>, 2>>>{
                    {{0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {0.00000000e+00 + 0.00000000e+00i,
                      0.00000000e+00 + 0.00000000e+00i}},
                    {{1.17017431e+00 - 2.43748228e-01i,
                      4.40679361e-02 - 1.53940000e-02i},
                     {1.02922989e+00 - 7.82628087e-02i,
                      -1.84573000e-01 + 1.79809491e-02i}},
                    {{-8.59886075e-02 + 1.13689959e-01i,
                      1.39851113e-09 - 3.01497601e-10i},
                     {6.07730278e-01 + 5.07144030e-01i,
                      -1.68609283e-01 + 8.64966880e-02i}},
                    {{-1.23967610e-05 + 1.45623920e-05i,
                      1.93199813e-06 - 2.24107827e-06i},
                     {-6.52504472e-02 + 3.60299246e-01i,
                      -3.33430797e-02 - 3.97852657e-02i}},
                    {{1.82536479e-05 - 1.06422631e-05i,
                      0.00000000e+00 + 0.00000000e+00i},
                     {-9.02947159e-02 - 4.09448171e-01i,
                      0.00000000e+00 + 0.00000000e+00i}}
            }},
            {"kz_list", std::valarray<std::complex<double>>{
                    0.02250959+0.i        , 0.00440866+0.i        ,
                    0.01435451+0.00687561i, 0.00404247+0.00074813i,
                    0.03118008+0.04748033i, 0.00515452+0.00110011i,
                    0.07823055+0.i        , 0.01413365+0.i        ,
                    0.06246792+0.01579948i, 0.01056188+0.00035793i
            }},
            {"th_list", std::valarray<std::complex<double>>{
                    0.3       +0.i        , 0.3       +0.i        ,
                    0.38659626-0.16429512i, 0.3162772 -0.05459799i,
                    0.06789345-0.10235287i, 0.24849917-0.0507924i ,
                    0.08877261+0.i        , 0.09619234+0.i        ,
                    0.10445527-0.02621521i, 0.12826687-0.00429919i
            }},
            {"pol", 'p'},
            {"n_list", std::valarray<std::complex<double>>{
                    1.5+0.i , 1.3+0.i ,
                    1. +0.4i, 1.2+0.2i,
                    2. +3.i , 1.5+0.3i,
                    5. +0.i , 4. +0.i ,
                    4. +1.i , 3. +0.1i
            }},
            {"d_list", std::valarray<double>{INFINITY, 200, 187.3, 1973.5, INFINITY}},
            {"th_0", 0.3},
            {"lam_vac", std::valarray<double>{400, 1770}}
    };
    AbsorpAnalyticVecFn<double> a;
    a.fill_in(coh_tmm_data, {1, 2});
    std::vector<std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> re_result(5);
    re_result.front() = a.a1;
    re_result.at(1) = a.a3;
    re_result.at(2) = a.A1;
    re_result.at(3) = a.A2;
    re_result.back() = a.A3;
    std::vector<std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> re_approx(5);
    re_approx.front() = std::valarray{0.01375122, 0.00149626, 0.09496066, 0.00220022};
    re_approx.at(1) = std::valarray{0.02870902, 0.00808494, 0.06236016, 0.01030904};
    re_approx.at(2) = std::valarray{2.01486783e-05, 4.74646908e-05, 2.74885296e-19, 9.28559634e-05};
    re_approx.at(3) = std::valarray{0.01321129, 0.00147049, 0.00272899, 0.00162005};
    re_approx.back() = std::valarray{-3.47185512e-04 - 4.56403179e-05i,  2.11762306e-04 + 4.49397818e-06i,
        2.01399935e-11 - 1.73429018e-11i,  1.32514817e-04 + 3.12222809e-04i};
    for (const std::pair<std::variant<std::valarray<double>, std::valarray<std::complex<double>>> &, std::variant<std::valarray<double>, std::valarray<std::complex<double>>> &> &elem : std::views::zip(re_result, re_approx)) {
        const std::variant<std::valarray<double>, std::valarray<std::complex<double>>> r = elem.first;
        const std::variant<std::valarray<double>, std::valarray<std::complex<double>>> e = elem.second;
        const std::valarray<double> *prvt = std::get_if<std::valarray<double>>(&r);
        const std::valarray<double> *pevt = std::get_if<std::valarray<double>>(&e);
        const std::valarray<std::complex<double>> *prvct = std::get_if<std::valarray<std::complex<double>>>(&r);
        const std::valarray<std::complex<double>> *pevct = std::get_if<std::valarray<std::complex<double>>>(&e);
        if (prvt and pevt) {
            assert(std::ranges::equal(*prvt, *pevt, [](const double rt, const double et) -> bool {
                return rt == approx(et, 1e-5);
            }));
        } else if (prvct and pevct) {
            assert(std::ranges::equal(*prvct, *pevct, [](const std::complex<double> rct, const std::complex<double> ect) -> bool {
                return rct == approx(ect, 1e-5);
            }));
        } else {
            throw std::runtime_error("r and e type mismatches.");
        }
    }
}

void test_copy() {
    AbsorpAnalyticVecFn<double> a;
    a.a1 = {1};
    a.a3 = {0.5};
    a.A1 = {2};
    a.A2 = {7};
    a.A3 = {5.0 + 3i};
    a.d = std::valarray<double>{7, 3};
    AbsorpAnalyticVecFn<double> b(a);
    std::vector<std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> azip(5);
    azip.front() = a.a1;
    azip.at(1) = a.a3;
    azip.at(2) = a.A1;
    azip.at(3) = a.A2;
    azip.back() = a.A3;
    std::vector<std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> bzip(5);
    bzip.front() = b.a1;
    bzip.at(1) = b.a3;
    bzip.at(2) = b.A1;
    bzip.at(3) = b.A2;
    bzip.back() = b.A3;
    for (const std::pair<std::variant<std::valarray<double>, std::valarray<std::complex<double>>> &, std::variant<std::valarray<double>, std::valarray<std::complex<double>>> &> &elem : std::views::zip(azip, bzip)) {
        const std::variant<std::valarray<double>, std::valarray<std::complex<double>>> avar = elem.first;
        const std::variant<std::valarray<double>, std::valarray<std::complex<double>>> bvar = elem.second;
        const std::valarray<double> *prvt = std::get_if<std::valarray<double>>(&avar);
        const std::valarray<double> *pevt = std::get_if<std::valarray<double>>(&bvar);
        const std::valarray<std::complex<double>> *prvct = std::get_if<std::valarray<std::complex<double>>>(&avar);
        const std::valarray<std::complex<double>> *pevct = std::get_if<std::valarray<std::complex<double>>>(&bvar);
        if (prvt and pevt) {
            assert(std::ranges::equal(*prvt, *pevt, [](const double rt, const double et) -> bool {
                return rt == approx(et, 1e-5);
            }));
        } else if (prvct and pevct) {
            assert(std::ranges::equal(*prvct, *pevct, [](const std::complex<double> rct, const std::complex<double> ect) -> bool {
                return rct == approx(ect, 1e-5);
            }));
        } else {
            throw std::runtime_error("a and b type mismatches.");
        }
    }
}

void test_run_array() {
    AbsorpAnalyticVecFn<double> a;
    a.a1 = {0.01375122, 0.00149626};
    a.a3 = {0.02870902, 0.00808494};
    a.A1 = {2.00290348e-05, 5.19001441e-05};
    a.A2 = {0.01276183, 0.00146736};
    a.A3 = {-4.91455269e-04 - 1.18654706e-04i,  2.74416636e-04 + 2.91821819e-05i};
    a.d = {200.};
    const ApproxSequenceLike<std::vector<std::complex<double>>, double> run_approx = approx<std::vector<std::complex<double>>, double>({
        0.01179895 + 0.i, 0.00772895 + 0.i, 0.00570666 + 0.i, 0.0043161 + 0.i, 0.00277534 + 0.i, 0.00118025 + 0.i, 0.00016438 + 0.i,
        0.00206809 + 0.i, 0.00196401 + 0.i, 0.00182646 + 0.i, 0.00166052 + 0.i, 0.00147356 + 0.i, 0.00127472 + 0.i, 0.00107422 + 0.i
    }, 1e-4);
    const std::vector<std::complex<double>> run_result = Utils::Range::vv_flatten<std::valarray<std::valarray<std::complex<double>>>, std::complex<double>>(a.run(Utils::Math::linspace_va<double>(0, 200, 7)));
    assert(run_result == run_approx);
}

void test_run() {
    AbsorpAnalyticVecFn<double> a;
    a.a1 = {0.01375122, 0.00149626};
    a.a3 = {0.02870902, 0.00808494};
    a.A1 = {2.00290348e-05, 5.19001441e-05};
    a.A2 = {0.01276183, 0.00146736};
    a.A3 = {-4.91455269e-04 - 1.18654706e-04i,  2.74416636e-04 + 2.91821819e-05i};
    a.d = {200.};
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> run_approx = approx<std::valarray<std::complex<double>>, double>({
        0.00016438 + 0.i, 0.00107422 + 0.i
    }, 1e-4);
    assert(a.run(200.0) == run_approx);
}

void test_scale() {
    AbsorpAnalyticVecFn<double> a;
    a.a1 = {0.01375122, 0.00149626};
    a.a3 = {0.02870902, 0.00808494};
    a.A1 = {2.00290348e-05, 5.19001441e-05};
    a.A2 = {0.01276183, 0.00146736};
    a.A3 = {-4.91455269e-04 - 1.18654706e-04i,  2.74416636e-04 + 2.91821819e-05i};
    a.d = {200.};
    a.scale(0.7);
    const ApproxSequenceLike<std::valarray<double>, double> a1_approx = approx<std::valarray<double>, double>({0.01375122, 0.00149626}, 1e-5);
    const ApproxSequenceLike<std::valarray<double>, double> a3_approx = approx<std::valarray<double>, double>({0.02870902, 0.00808494}, 1e-5);
    const ApproxSequenceLike<std::valarray<double>, double> A1_approx = approx<std::valarray<double>, double>({0.7 * 2.00290348e-05, 0.7 * 5.19001441e-05}, 1e-5);
    const ApproxSequenceLike<std::valarray<double>, double> A2_approx = approx<std::valarray<double>, double>({0.7 * 0.01276183, 0.7 * 0.00146736}, 1e-5);
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> A3_approx = approx<std::valarray<std::complex<double>>, double>({0.7 * (-4.91455269e-04 - 1.18654706e-04i), 0.7 * (2.74416636e-04 + 2.91821819e-05i)}, 1e-5);
    const ApproxSequenceLike<std::valarray<double>, double> d_approx = approx<std::valarray<double>, double>({200}, 1e-5);
    assert(a.a1 == a1_approx);
    assert(a.a3 == a3_approx);
    assert(a.A1 == A1_approx);
    assert(a.A2 == A2_approx);
    assert(a.A3 == A3_approx);
    assert(std::get<std::valarray<double>>(a.d) == d_approx);
}

void test_add() {
    AbsorpAnalyticVecFn<double> a;
    a.a1 = {0.01375122, 0.00149626};
    a.a3 = {0.02870902, 0.00808494};
    a.A1 = {2.00290348e-05, 5.19001441e-05};
    a.A2 = {0.01276183, 0.00146736};
    a.A3 = {-4.91455269e-04 - 1.18654706e-04i,  2.74416636e-04 + 2.91821819e-05i};
    a.d = {200.};
    AbsorpAnalyticVecFn<double> b;
    b.a1 = {0.01375122, 0.00149626};
    b.a3 = {0.02870902, 0.00808494};
    b.A1 = {2.e-05, 5e-05};
    b.A2 = {0.05, 0.003};
    b.A3 = {4.81455269e-04 - 1 - 04i,  3e-04 + 3 - 05i};
    b.d = {200.};
    a.add(b);
    const ApproxSequenceLike<std::valarray<double>, double> a1_approx = approx<std::valarray<double>, double>({0.01375122, 0.00149626}, 1e-5);
    const ApproxSequenceLike<std::valarray<double>, double> a3_approx = approx<std::valarray<double>, double>({0.02870902, 0.00808494}, 1e-5);
    const ApproxSequenceLike<std::valarray<double>, double> A1_approx = approx<std::valarray<double>, double>({2.00290348e-05 + 2.e-05, 5.19001441e-05 + 5e-05}, 1e-5);
    const ApproxSequenceLike<std::valarray<double>, double> A2_approx = approx<std::valarray<double>, double>({0.01276183 + 0.05, 0.00146736 + 0.003}, 1e-5);
    const ApproxSequenceLike<std::valarray<std::complex<double>>, double> A3_approx = approx<std::valarray<std::complex<double>>, double>({-4.91455269e-04 - 1.18654706e-04i + 4.81455269e-04 - 1.0 - 04i,
                                                                                                                                           2.74416636e-04 + 2.91821819e-05i + 3e-04 + 3.0 - 05i}, 1e-5);
    const ApproxSequenceLike<std::valarray<double>, double> d_approx = approx<std::valarray<double>, double>({200}, 1e-5);
    assert(a.a1 == a1_approx);
    assert(a.a3 == a3_approx);
    assert(a.A1 == A1_approx);
    assert(a.A2 == A2_approx);
    assert(a.A3 == A3_approx);
    assert(std::get<std::valarray<double>>(a.d) == d_approx);
}

void test_add_exception() {
    AbsorpAnalyticVecFn<double> a;
    a.a1 = {0.01375122, 0.00149626};
    a.a3 = {0.02870902, 0.00808494};
    a.A1 = {2.00290348e-05, 5.19001441e-05};
    a.A2 = {0.01276183, 0.00146736};
    a.A3 = {-4.91455269e-04 - 1.18654706e-04i,  2.74416636e-04 + 2.91821819e-05i};
    a.d = {200.};
    AbsorpAnalyticVecFn<double> b;
    b.a1 = {7, 2};
    b.a3 = {0.02870902, 0.00808494};
    b.A1 = {2.e-05, 5e-05};
    b.A2 = {0.05, 0.003};
    b.A3 = {4.81455269e-04 - 1 - 04i,  3e-04 + 3 - 05i};
    b.d = {200.};
    a.add(b);
}

void test_absorp_in_each_layer() {
    const coh_tmm_vec_dict<double> coh_tmm_data = {
            {"r", std::valarray<std::complex<double>>{0.14017645 - 0.2132843i, 0.22307786 - 0.10704008i}},
            {"t", std::valarray<std::complex<double>>{1.78669633e-05 - 9.79824244e-06i, -8.86075993e-02 - 4.05953564e-01i}},
            {"R", std::valarray<double>{0.06513963, 0.06122131}},
            {"T", std::valarray<double>{1.15234466e-09, 4.13619185e-01}},
            {"power_entering", std::valarray<double>{0.93486037, 0.93877869}},
            {"vw_list", std::valarray<std::vector<std::array<std::complex<double>, 2>>>{
                    {{0.00000000e+00 + 0.00000000e+00i,
                             0.00000000e+00 + 0.00000000e+00i},
                            {0.00000000e+00 + 0.00000000e+00i,
                                    0.00000000e+00 + 0.00000000e+00i}},
                    {{1.18358724e+00 - 2.33272105e-01i,
                             -4.34107939e-02 + 1.99878010e-02i},
                            {1.03160316e+00 - 7.28921467e-02i,
                                    1.91474694e-01 - 3.41479380e-02i}},
                    {{-8.59535500e-02 + 1.06568462e-01i,
                             -1.36521327e-09 + 2.83859953e-10i},
                            {6.08369346e-01 + 5.06683493e-01i,
                                    1.75320349e-01 - 9.58306162e-02i}},
                    {{-1.23112929e-05 + 1.37276841e-05i,
                             -1.94390395e-06 + 2.16097082e-06i},
                            {-6.54156818e-02 + 3.57104644e-01i,
                                    3.38453387e-02 + 4.04808706e-02i}},
                    {{1.78669633e-05 - 9.79824244e-06i,
                             0.00000000e+00 + 0.00000000e+00i},
                            {-8.86075993e-02 - 4.05953564e-01i,
                                    0.00000000e+00 + 0.00000000e+00i}}
            }},
            {"kz_list", std::valarray<std::complex<double>>{
                    0.02250959+0.i        , 0.00440866+0.i        ,
                    0.01435451+0.00687561i, 0.00404247+0.00074813i,
                    0.03118008+0.04748033i, 0.00515452+0.00110011i,
                    0.07823055+0.i        , 0.01413365+0.i        ,
                    0.06246792+0.01579948i, 0.01056188+0.00035793i
            }},
            {"th_list", std::valarray<std::complex<double>>{
                    0.3       +0.i        , 0.3       +0.i        ,
                    0.38659626-0.16429512i, 0.3162772 -0.05459799i,
                    0.06789345-0.10235287i, 0.24849917-0.0507924i ,
                    0.08877261+0.i        , 0.09619234+0.i        ,
                    0.10445527-0.02621521i, 0.12826687-0.00429919i
            }},
            {"pol", 's'},
            {"n_list", std::valarray<std::complex<double>>{
                    1.5+0.i , 1.3+0.i ,
                    1. +0.4i, 1.2+0.2i,
                    2. +3.i , 1.5+0.3i,
                    5. +0.i , 4. +0.i ,
                    4. +1.i , 3. +0.1i
            }},
            {"d_list", std::valarray<double>{INFINITY, 200, 187.3, 1973.5, INFINITY}},
            {"th_0", 0.3},
            {"lam_vac", std::valarray<double>{400, 1770}}
    };
    const std::vector<double> ab_result = Utils::Range::vv_flatten<std::valarray<std::valarray<double>>, double>(absorp_in_each_layer(coh_tmm_data));
    const ApproxSequenceLike<std::vector<double>, double> ab_approx = approx<std::vector<double>, double>({
            6.51396300e-02, 6.12213100e-02,
            9.08895166e-01, 3.25991032e-01,
            2.59652025e-02, 1.99168474e-01,
            0, 0,
            1.15234466e-09, 4.13619185e-01
        }, 1e-6, 1e-10);
    assert(ab_approx == ab_result);
}

void test_inc_group_layers() {
    const std::vector<std::valarray<std::complex<double>>> n_list = {{1.5, 1.3},
                                                                     {1.0 + 0.4i, 1.2 + 0.2i},
                                                                     {2.0 + 3i, 1.5 + 0.3i},
                                                                     {5, 4},
                                                                     {4.0 + 1i, 3.0 + 0.1i}};
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent};
    const inc_tmm_vec_dict<double> result = inc_group_layers(n_list, d_list, c_list);
    const std::vector<std::vector<double>> stack_d_list = std::get<std::vector<std::vector<double>>>(result.at("stack_d_list"));
    assert(stack_d_list.size() == 1 and std::ranges::equal(stack_d_list.front(), std::valarray<double>{INFINITY, 200, 187.3, INFINITY}));
    const std::vector<std::vector<std::valarray<std::complex<double>>>> stack_n_list = std::get<std::vector<std::vector<std::valarray<std::complex<double>>>>>(result.at("stack_n_list"));
    assert(stack_n_list.size() == 1 and stack_n_list.front().size() == 4);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> snl_approx0 = approx<std::valarray<std::complex<double>>, double>({1.5, 1.3});
    assert(stack_n_list.front().front() == snl_approx0);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> snl_approx1 = approx<std::valarray<std::complex<double>>, double>({1.0 + 0.4i, 1.2 + 0.2i});
    assert(stack_n_list.front().at(1) == snl_approx1);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> snl_approx2 = approx<std::valarray<std::complex<double>>, double>({2.0 + 3i, 1.5 + 0.3i});
    assert(stack_n_list.front().at(2) == snl_approx2);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> snl_approx3 = approx<std::valarray<std::complex<double>>, double>({5, 4});
    assert(stack_n_list.front().back() == snl_approx3);
    assert(std::ranges::equal(std::get<std::vector<std::size_t>>(result.at("all_from_inc")), std::vector<std::size_t>{0, 3, 4}));
    assert(std::ranges::equal(std::get<std::vector<std::ptrdiff_t>>(result.at("inc_from_all")), std::vector<std::ptrdiff_t>{0, -1, -1, 1, 2}));
    const std::vector<std::vector<std::size_t>> all_from_stack = std::get<std::vector<std::vector<std::size_t>>>(result.at("all_from_stack"));
    assert(all_from_stack.size() == 1 and std::ranges::equal(all_from_stack.front(), std::vector<std::size_t >{0, 1, 2, 3}));
    assert(std::ranges::equal(std::get<std::vector<std::ptrdiff_t>>(result.at("inc_from_stack")), std::vector<std::ptrdiff_t>{0}));
    assert(std::ranges::equal(std::get<std::vector<std::ptrdiff_t>>(result.at("stack_from_inc")), std::vector<std::ptrdiff_t>{-1, 0, -1}));
    assert(std::get<std::size_t>(result.at("num_stacks")) == 1);
    assert(std::get<std::size_t>(result.at("num_inc_layers")) == 3);
    assert(std::get<std::size_t>(result.at("num_layers")) == 5);
}

// tests for inc_tmm

void test_inc_tmm_exception() {
    const inc_tmm_dict<double> exc = inc_tmm('s', {1.0 + 0.5i, 2, 3}, {INFINITY, 20, INFINITY}, {LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent}, 0.5 + 0i, 500.0);
}

void test_inc_tmm_s_R() {
    const std::vector<std::valarray<std::complex<double>>> n_list = {{1.5, 1.3},
                                                                     {1.0 + 0.4i, 1.2 + 0.2i},
                                                                     {2.0 + 3i, 1.5 + 0.3i},
                                                                     {5, 4},
                                                                     {4.0 + 1i, 3.0 + 0.1i}};
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent};
    const std::valarray<double> lam_vac = {400, 1770};
    const inc_tmm_vec_dict<double> result = inc_tmm('s', n_list, d_list, c_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> sR_approx = approx<std::valarray<double>, double>({0.06513963, 0.09735299});
    assert(std::get<std::valarray<double>>(result.at("R")) == sR_approx);
}

void test_inc_tmm_s_R_incfirst() {
    // testing the case where the coherent stack DOES NOT start right after the semi-infinite layer
    const std::vector<std::valarray<std::complex<double>>> n_list = {{1.5, 1.3},
                                                                     {1.0 + 0.4i, 1.2 + 0.2i},
                                                                     {2.0 + 3i, 1.5 + 0.3i},
                                                                     {5, 4},
                                                                     {4.0 + 1i, 3.0 + 0.1i}};
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Incoherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent};
    const std::valarray<double> lam_vac = {400, 1770};
    const inc_tmm_vec_dict<double> result = inc_tmm('s', n_list, d_list, c_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> sR_approx = approx<std::valarray<double>, double>({0.08254069, 0.07335674});
    assert(std::get<std::valarray<double>>(result.at("R")) == sR_approx);
}

void test_inc_tmm_s_T() {
    const std::vector<std::valarray<std::complex<double>>> n_list = {{1.5, 1.3},
                                                                     {1.0 + 0.4i, 1.2 + 0.2i},
                                                                     {2.0 + 1i, 1.5 + 0.3i},
                                                                     {5, 4},
                                                                     {4.0 + 1i, 3.0 + 0.1i}};
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 100.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent};
    const std::valarray<double> lam_vac = {400, 1770};
    const inc_tmm_vec_dict<double> result = inc_tmm('s', n_list, d_list, c_list, th_0, lam_vac);
    const ApproxSequenceLike<std::valarray<double>, double> sT_approx = approx<std::valarray<double>, double>({1.22080402e-04, 3.88581656e-01});
    assert(std::get<std::valarray<double>>(result.at("T")) == sT_approx);
}

void test_inc_tmm_s_power_entering_list() {
    const std::vector<std::valarray<std::complex<double>>> n_list = {{1.5, 1.3},
                                                                     {1.0 + 0.4i, 1.2 + 0.2i},
                                                                     {2.0 + 3i, 1.5 + 0.3i},
                                                                     {5, 4},
                                                                     {4.0 + 1i, 3.0 + 0.1i}};
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent};
    const std::valarray<double> lam_vac = {400, 1770};
    const inc_tmm_vec_dict<double> result = inc_tmm('s', n_list, d_list, c_list, th_0, lam_vac);
    const ApproxSequenceLike<std::vector<double>, double> sp_approx = approx<std::vector<double>, double>(
            Utils::Range::rng2d_transpose(
                    std::vector<double>{1.00000000e+00, 1.09570873e-09, 1.09570873e-09,
                                        1.00000000e+00, 3.87024124e-01, 3.87024124e-01}
                    , 2));
    const std::vector<double> sp_result = Utils::Range::vv_flatten<std::vector<std::valarray<double>>, double>(std::get<std::vector<std::valarray<double>>>(result.at("power_entering_list")));
    assert(sp_result == sp_approx);
}

void test_inc_tmm_s_VW_list() {
    const std::vector<std::valarray<std::complex<double>>> n_list = {{1.5, 1.3},
                                                                     {1.0 + 0.4i, 1.2 + 0.2i},
                                                                     {2.0 + 1i, 1.5 + 0.3i},
                                                                     {5, 4},
                                                                     {4.0 + 1i, 3.0 + 0.1i}};
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 100.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent};
    const std::valarray<double> lam_vac = {400, 1770};
    const inc_tmm_vec_dict<double> result = inc_tmm('s', n_list, d_list, c_list, th_0, lam_vac);
    std::valarray<std::array<std::valarray<double>, 2>> VW_list(std::array<std::valarray<double>, 2>{std::valarray<double>(2), std::valarray<double>(2)}, 4);
    VW_list[0].front() = {NAN, NAN};
    VW_list[0].back() = {NAN, NAN};
    VW_list[1].front() = {4.99455825e-02, 7.30898359e-01};
    VW_list[1].back() = {6.91491754e-08, 7.46196606e-02};
    VW_list[2].front() = {1.25191080e-04, 3.96967199e-01};
    VW_list[2].back() = {3.11067761e-06, 8.38554284e-03};
    VW_list[3].front() = {1.22080402e-04, 3.88581656e-01};
    VW_list[3].back() = {0.00000000e+00, 0.00000000e+00};
    const ApproxNestedRange<std::valarray<std::array<std::valarray<double>, 2>>, double> VWl_approx = approx<std::valarray<std::array<std::valarray<double>, 2>>, double>(VW_list, NAN, NAN, true);
    const std::valarray<std::array<std::valarray<double>, 2>> VWl_result = std::get<std::valarray<std::array<std::valarray<double>, 2>>>(result.at("VW_list"));
    assert(VWl_result == VWl_approx);
}

void test_inc_absorp_in_each_layer() {
    std::valarray<std::array<std::valarray<double>, 2>> VW_list(std::array<std::valarray<double>, 2>{std::valarray<double>(2), std::valarray<double>(2)}, 4);
    VW_list[0].front() = {NAN, NAN};
    VW_list[0].back() = {NAN, NAN};
    VW_list[1].front() = {4.99455825e-02, 7.30898359e-01};
    VW_list[1].back() = {6.91491754e-08, 7.46196606e-02};
    VW_list[2].front() = {1.25191080e-04, 3.96967199e-01};
    VW_list[2].back() = {3.11067761e-06, 8.38554284e-03};
    VW_list[3].front() = {1.22080402e-04, 3.88581656e-01};
    VW_list[3].back() = {0.00000000e+00, 0.00000000e+00};
    std::valarray<std::vector<std::array<std::complex<double>, 2>>> vw_list(std::vector<std::array<std::complex<double>, 2>>(2, std::array<std::complex<double>, 2>()), 3);
    vw_list[0] = {{0, 0}, {0, 0}};
    vw_list[1] = {{1.18009942 - 0.22823678i, -0.02194584 + 0.01826407i}, {1.0438036 - 0.08762503i,  0.00681749 - 0.09631804i}};
    vw_list[2] = {{-0.16908171 + 0.0889725i, 0}, {0.59979328 + 0.5149352i, 0}};
    std::vector<coh_tmm_vecn_dict<double>> coh_tmm_data_list;
    coh_tmm_data_list.emplace_back(coh_tmm_vecn_dict<double>{
            {"r", std::valarray{0.15815358 - 0.2099727i, 0.05062109 - 0.18394307i}},
            {"t", std::valarray{-0.16908171 + 0.0889725i, 0.59979328 + 0.5149352i}},
            {"R", std::valarray{0.06910109, 0.03639755}},
            {"T", std::valarray{0.04994557, 0.73063361}},
            {"power_entering", std::valarray{0.93089891, 0.96360245}},
            {"vw_list", vw_list},  // if std::move(vw_list) here, vw_list below will be dead.
            {"kz_list", std::vector<std::valarray<std::complex<double>>>{{0.02250959, 0.00440866},
                                                                         {0.01435451 + 0.00687561i, 0.00404247 + 0.00074813i},
                                                                         {0.03079749 + 0.01602339i, 0.00515452 + 0.00110011i}}},
            {"th_list", std::vector<std::valarray<std::complex<double>>>{{0.3, 0.3},
                                                                         {0.38659626 - 0.16429512i, 0.3162772 - 0.05459799i},
                                                                         {0.17752825 - 0.08995035i, 0.24849917 - 0.0507924i}}},
            {"pol", 's'},
            {"n_list", std::vector<std::valarray<std::complex<double>>>{{1.5, 1.3},
                                                                        {1. + 0.4i, 1.2 + 0.2i},
                                                                        {2. + 1.i, 1.5 + 0.3i}}},
            {"d_list", std::vector<double>{INFINITY, 200, INFINITY}},
            {"th_0", std::valarray<std::complex<double>>{0.3, 0.3}},
            {"lam_vac", std::valarray<double>{400, 1770}}
    });
    std::vector<coh_tmm_vecn_dict<double>> coh_tmm_bdata_list;
    vw_list[1] = {{1.37311562 + 0.0051288i, -0.00368788 + 0.02468907i}, {1.13241606 + 0.01868049i, -0.07494402 - 0.03433257i}};
    vw_list[2] = {{-0.29467151 + 0.00137131i, 0}, {0.57277308 + 0.75172209i, 0}};
    coh_tmm_bdata_list.emplace_back(coh_tmm_vecn_dict<double>{
            {"r", std::valarray{0.36942774 + 0.02981787i, 0.05747204 - 0.01565208i}},
            {"t", std::valarray{-0.29467151 + 0.00137131i,  0.57277308 + 0.75172209i}},
            {"R", std::valarray{0.13736596, 0.00354802}},
            {"T", std::valarray{0.06346552, 0.76391471}},
            {"power_entering", std::valarray{0.89366146, 0.98977083}},
            {"vw_list", std::move(vw_list)},
            {"kz_list", std::vector<std::valarray<std::complex<double>>>{{0.03079749 + 1.60233896e-02i, 0.00515452 + 1.10011324e-03i},
                                                                         {0.01435451 + 6.87561247e-03i, 0.00404247 + 7.48130214e-04i},
                                                                         {0.02250959 + 1.34865520e-19i, 0.00440866 - 1.52390418e-20i}}},
            {"th_list", std::vector<std::valarray<std::complex<double>>>{{0.17752825 - 8.99503525e-02i, 0.24849917 - 5.07924048e-02i},
                                                                         {0.38659626 - 1.64295119e-01i, 0.3162772 - 5.45979936e-02i},
                                                                         {0.3 - 1.93687955e-17i, 0.3 + 1.11743051e-17i}}},
            {"pol", 's'},
            {"n_list", std::vector<std::valarray<std::complex<double>>>{{2. + 1.i, 1.5 + 0.3i},
                                                                        {1. + 0.4i, 1.2 + 0.2i},
                                                                        {1.5, 1.3}}},
            {"d_list", std::vector<double>{INFINITY, 200, INFINITY}},
            {"th_0", std::valarray<std::complex<double>>{0.17752825 - 0.08995035i, 0.24849917 - 0.0507924i}},
            {"lam_vac", std::valarray<double>{400, 1770}}
    });
    std::valarray<std::array<std::valarray<double>, 2>> stackFB_list(std::array<std::valarray<double>, 2>{std::valarray<double>(2), std::valarray<double>(2)}, 1);
    stackFB_list[0] = std::array<std::valarray<double>, 2>{std::valarray<double>{1, 1}, std::valarray<double>{6.91491754e-08, 7.46196606e-02}};  // diff
    const inc_tmm_vec_dict<double> inc_data = {
            {"T", std::valarray{1.22080402e-04, 3.88581656e-01}},
            {"R", std::valarray{0.0691011, 0.0934006}},
            {"VW_list", VW_list},
            {"coh_tmm_data_list", std::move(coh_tmm_data_list)},
            {"coh_tmm_bdata_list", std::move(coh_tmm_bdata_list)},
            {"stackFB_list", std::move(stackFB_list)},
            {"power_entering_list", Utils::Range::rng2l_transpose(std::vector<std::valarray<double>>{
                    {1, 4.99455112e-02, 1.22080402e-04, 1.22080402e-04},
                    {1, 6.56777243e-01, 3.88581656e-01, 3.88581656e-01}})},
            {"stack_d_list", std::vector<std::vector<double>>{{INFINITY, 200, INFINITY}}},
            {"stack_n_list", std::vector<std::vector<std::valarray<std::complex<double>>>>{{{1.5, 1.3},
                                                                                            {1.0 + 0.4i, 1.2 + 0.2i},
                                                                                            {2.0 + 1i, 1.5 + 0.3i}}}},
            {"all_from_inc", std::vector<std::size_t>{0, 2, 3, 4}},
            {"inc_from_all", std::vector<std::ptrdiff_t>{0, -1, 1, 2, 3}},
            {"all_from_stack", std::vector<std::vector<std::size_t>>{{0, 1, 2}}},
            {"stack_from_all", std::vector<std::vector<std::size_t>>{{}, {0, 1}, {}, {}, {}}},
            {"inc_from_stack", std::vector<std::ptrdiff_t>{0}},
            {"stack_from_inc", std::vector<std::ptrdiff_t>{-1, 0, -1, -1}},
            {"num_stacks", 1U},
            {"num_inc_layers", 4U},
            {"num_layers", 5U}
    };
    const ApproxNestedRange<std::vector<std::valarray<double>>, double> absorp_approx = approx<std::vector<std::valarray<double>>, double>(std::vector<std::valarray<double>>{{6.91010944e-02, 9.34006064e-02},
                                                                                                                                                                              {8.80953397e-01, 2.49822147e-01},
                                                                                                                                                                              {4.98234308e-02, 2.68195587e-01},
                                                                                                                                                                              {0, 0},
                                                                                                                                                                              {1.22080402e-04, 3.88581656e-01}});
    assert(inc_absorp_in_each_layer(inc_data) == absorp_approx);
}

void test_inc_find_absorp_analytic_fn_exception() {
    const inc_tmm_vec_dict<double> inc_data = {{"stack_from_all", std::vector<std::vector<std::size_t>>{{}, {0, 1}, {}, {}, {}}}};
    inc_find_absorp_analytic_fn(2, inc_data);
}

void test_inc_find_absorp_analytic_fn() {
    const std::vector<std::valarray<std::complex<double>>> n_list = {{1, 1}, {2, 3}, {2, 3}, {1, 1}};
    const std::valarray<double> d_list = {INFINITY, 100, 1000, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {100, 500};
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent};
    const inc_tmm_vec_dict<double> inc_tmm_data = inc_tmm('s', n_list, d_list, c_list, th_0, lam_vac);
    AbsorpAnalyticVecFn<double> a = inc_find_absorp_analytic_fn(1, inc_tmm_data);
    std::vector<std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> re_result(5);
    re_result.front() = a.a1;
    re_result.at(1) = a.a3;
    re_result.at(2) = a.A1;
    re_result.at(3) = a.A2;
    re_result.back() = a.A3;
    std::vector<std::variant<std::valarray<double>, std::valarray<std::complex<double>>>> re_approx(5);
    // Must explicitly tell valarray value type otherwise all the 5 are double because re_approx value type is variant.
    re_approx.front() = std::valarray{0.0, 0.0};
    re_approx.at(1) = std::valarray{0.24856865, 0.07503152};
    re_approx.at(2) = std::valarray{0.0, 0.0};
    re_approx.at(3) = std::valarray{0.0, 0.0};
    re_approx.back() = std::valarray<std::complex<double>>{0.0, 0.0};
    for (const std::pair<std::variant<std::valarray<double>, std::valarray<std::complex<double>>> &, std::variant<std::valarray<double>, std::valarray<std::complex<double>>> &> &elem : std::views::zip(re_result, re_approx)) {
        const std::variant<std::valarray<double>, std::valarray<std::complex<double>>> r = elem.first;
        const std::variant<std::valarray<double>, std::valarray<std::complex<double>>> e = elem.second;
        const std::valarray<double> *prvt = std::get_if<std::valarray<double>>(&r);
        const std::valarray<double> *pevt = std::get_if<std::valarray<double>>(&e);
        const std::valarray<std::complex<double>> *prvct = std::get_if<std::valarray<std::complex<double>>>(&r);
        const std::valarray<std::complex<double>> *pevct = std::get_if<std::valarray<std::complex<double>>>(&e);
        if (prvt and pevt) {
            assert(std::ranges::equal(*prvt, *pevt, [](const double rt, const double et) -> bool {
                return rt == approx(et, 1e-5);
            }));
        } else if (prvct and pevct) {
            assert(std::ranges::equal(*prvct, *pevct, [](const std::complex<double> rct, const std::complex<double> ect) -> bool {
                return rct == approx(ect, 1e-5);
            }));
        } else {
            throw std::runtime_error("r and e type mismatches.");
        }
    }
}

void test_inc_position_resolved() {
    const std::vector<std::valarray<std::complex<double>>> n_list = {{1, 1}, {2.0 + 0.5i, 3}, {2, 3.0 + 1i}, {1, 1}};
    const std::valarray<double> d_list = {INFINITY, 100, 1000, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {100, 500};
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent};
    const inc_tmm_vec_dict<double> inc_tmm_data = inc_tmm('s', n_list, d_list, c_list, th_0, lam_vac);
    const std::valarray<double> dist = Utils::Math::linspace_va(0.0, 1100.0, 12.0);
    // layer is going to be uniqued later
    auto [layer, d_in_layer] = find_in_structure_inf(d_list, dist);
    std::valarray<std::valarray<double>> alphas(std::valarray<double>(2), 4);
    for (std::size_t i : std::views::iota(0U, 4U)) {
        for (std::size_t j : std::views::iota(0U, 2U)) {
            alphas[i][j] = 4 * std::numbers::pi * n_list[i][j].imag() / lam_vac[j];
        }
    }
    const ApproxNestedRange<std::valarray<std::valarray<double>>, double> incpr_approx = approx<std::valarray<std::valarray<double>>, double>(
            Utils::Range::rng2l_transpose(std::valarray<std::valarray<double>>{
        {0, 5.41619013e-02, 1.11248375e-04, 7.66705892e-03,
         4.38516795e+00, 2.50714861e+03, 1.43341858e+06, 8.19532120e+08,
         4.68553223e+11, 2.67887148e+14, 1.53159813e+17, 0},
        {0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 1.74764534e-13}}));
    assert(inc_position_resolved(std::forward<std::valarray<std::size_t>>(layer), d_in_layer, inc_tmm_data, c_list, alphas) == incpr_approx);
}

void test_beer_lambert() {
    const std::valarray<double> alphas = Utils::Math::linspace_va(0.0, 1.0, 5.0);
    const std::valarray<double> fraction = Utils::Math::linspace_va(0.2, 1.0, 5.0);
    const std::valarray<double> A_total = {0, 9.99999989e-09, 2.99999992e-08, 5.99999978e-08, 9.99999950e-08};
    const std::valarray<double> dist = Utils::Math::linspace_va(0.0, 100e-9, 4.0);
    const ApproxNestedRange<std::valarray<std::valarray<double>>, double> bl_approx = approx<std::valarray<std::valarray<double>>, double>(std::valarray<std::valarray<double>>{{0, 0, 0, 0},
                                                                                                                                                                                {0.1, 0.1, 0.1, 0.1},
                                                                                                                                                                                {0.3, 0.3, 0.29999999, 0.29999999},
                                                                                                                                                                                {0.6, 0.59999999, 0.59999997, 0.59999996},
                                                                                                                                                                                {1, 0.99999997, 0.99999993, 0.9999999}});
    assert(beer_lambert(alphas, fraction, dist, A_total) == bl_approx);
}

void runall() {
    test_snell();
    test_list_snell();
    test_interface_r();
    test_interface_t();
    test_R_from_r();
    test_T_from_t();
    test_power_entering_from_r();
    test_interface_R();
    test_interface_T();
    test_coh_tmm_s_r();
    test_coh_tmm_s_t();
    test_coh_tmm_s_R();
    test_coh_tmm_s_T();
    test_coh_tmm_s_power_entering();
    test_coh_tmm_s_vw_list();
    test_coh_tmm_p_power_entering();
    test_coh_tmm_p_vw_list();
    test_coh_tmm_kz_list();
    test_coh_tmm_th_list();
    test_coh_tmm_inputs();
    test_ellips_psi();
    test_ellips_Delta();
    test_unpolarized_RT_R();
    test_find_in_structure();
    test_find_in_structure_inf();
    test_layer_starts();
    test_fill_in_s();
    test_fill_in_p();
    test_copy();
    test_run_array();
    test_run();
    test_scale();
    test_add();
    test_absorp_in_each_layer();
    test_inc_group_layers();
    test_inc_tmm_s_R();
    test_inc_tmm_s_R_incfirst();
    test_inc_tmm_s_T();
    test_inc_tmm_s_power_entering_list();
    test_inc_tmm_s_VW_list();
    test_inc_absorp_in_each_layer();
    test_inc_find_absorp_analytic_fn();
    test_inc_position_resolved();
    test_beer_lambert();
}

void run_all_except() {
    test_coh_tmm_exceptions();
    test_unpolarized_RT_T();  // Relative error: [7.12e-4, 1.86e-5].
    test_position_resolved_s_poyn();  // Will fail layer/distance[0] and [6].
    test_position_resolved_s_absor();  // Same as above
    test_position_resolved_p_poyn();  // Same as above
    test_position_resolved_p_absor();  // Same as above
    test_find_in_structure_exception();
    test_add_exception();
    test_inc_tmm_exception();
    test_inc_find_absorp_analytic_fn_exception();
}

auto main() -> int {
    runall();
}

//
// Created by Yihua on 2023/11/27.
//

// GCC/Clang has already forwarded <algorithm> from <valarray>, but it is not the case for MSVC.
#include <algorithm>
#include <cassert>
#include <numbers>
#include <functional>
#include "tmm.h"
#include "utils.h"
#include "Approx.h"

using namespace std::complex_literals;

void test_snell() {
    // assert() does not support comma operators. However, you can have a look at
    // P2264 R1-[P2264R5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2264r5.html)
    // https://github.com/cplusplus/papers/issues/957
    // Make assert() macro user friendly for C and C++
    // assert(snell(3.0 + 0i, 2.0 + 0i, 0.7 + 0i) == approx<std::complex<double>, double>(1.3105496419558818));
    ApproxScalar<std::complex<double>, double> snell_approx = approx<std::complex<double>, double>(1.3105496419558818);
    assert(snell(3.0 + 0i, 2.0 + 0i, 0.7 + 0i) == snell_approx);
}

void test_list_snell() {
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> list_snell_approx = approx<std::valarray<std::complex<double>>, double>(std::valarray<std::complex<double>>{0.3, 0.19831075, 0.14830313});
    std::valarray<std::complex<double>> list_snell_res = list_snell(std::valarray<std::complex<double>>{2, 3, 4}, 0.3 + 0i);
    assert(list_snell_res == list_snell_approx);
}

void test_interface_r() {
    std::complex<double> th1 = 0.2;
    std::complex<double> n1 = 2;
    std::complex<double> n2 = 3;
    std::complex<double> th2 = std::asin(n1 / n2 * std::sin(th1));
    ApproxScalar<std::complex<double>, double> s_approx = approx<std::complex<double>, double>(-0.20541108217641596);
    ApproxScalar<std::complex<double>, double> p_approx = approx<std::complex<double>, double>(0.19457669033430525);
    assert(interface_r('s', n1, n2, th1, th2) == s_approx);
    assert(interface_r('p', n1, n2, th1, th2) == p_approx);
}

void test_interface_t() {
    std::complex<double> th1 = 0.2;
    std::complex<double> n1 = 2;
    std::complex<double> n2 = 3;
    std::complex<double> th2 = std::asin(n1 / n2 * std::sin(th1));
    std::complex<double> s_res = interface_t('s', n1, n2, th1, th2);
    std::complex<double> p_res = interface_t('p', n1, n2, th1, th2);
    ApproxScalar<std::complex<double>, double> s_approx = approx<std::complex<double>, double>(0.7945889178235841);
    ApproxScalar<std::complex<double>, double> p_approx = approx<std::complex<double>, double>(0.7963844602228701);
    assert(s_res == s_approx);
    assert(p_res == p_approx);
}

void test_R_from_r() {
    assert(R_from_r(std::numbers::sqrt2 + 1i * std::numbers::sqrt2) == 4.0);
}

void test_T_from_t() {
    std::complex<double> th1 = 0.2;
    std::complex<double> n1 = 2;
    std::complex<double> n2 = 3;
    std::complex<double> th2 = std::asin(n1 / n2 * std::sin(th1));
    std::complex<double> ts = 0.7945889178235841;
    std::complex<double> tp = 0.7963844602228701;
    ApproxScalar<std::complex<double>, double> Ts_approx = approx<std::complex<double>, double>(0.9578062873191139);
    ApproxScalar<std::complex<double>, double> Tp_approx = approx<std::complex<double>, double>(0.962139911578548);
    assert(T_from_t('s', ts, n1, n2, th1, th2) == Ts_approx);
    assert(T_from_t('s', tp, n1, n2, th1, th2) == Tp_approx);
}

void test_power_entering_from_r() {
    std::complex<double> rs = -0.20541108217641596;
    std::complex<double> rp = 0.19457669033430525;

    std::complex<double> n1 = 2;
    std::complex<double> th1 = 0.2;

    ApproxScalar<double, double> ps_approx = approx<double, double>(0.9578062873191138);
    ApproxScalar<double, double> pp_approx = approx<double, double>(0.962139911578548);
    assert(power_entering_from_r('s', rs, n1, th1) == ps_approx);
    assert(power_entering_from_r('p', rp, n1, th1) == pp_approx);
}

void test_interface_R() {
    std::valarray<std::complex<double>> th1 = {0.2, std::numbers::pi / 2};
    std::valarray<std::complex<double>> n1 = {2, 2};
    std::valarray<std::complex<double>> n2 = {3, 3};
    std::valarray<std::complex<double>> th2 = std::asin(n1 / n2 * std::sin(th1));
    ApproxSequenceLike<std::valarray<double>, double> interface_Rs_approx = approx<std::valarray<double>, double>({0.042193712680886314, 1});
    ApproxSequenceLike<std::valarray<double>, double> interface_Rp_approx = approx<std::valarray<double>, double>({0.037860088421452116, 1});
    assert(interface_R('s', n1, n2, th1, th2) == interface_Rs_approx);
    assert(interface_R('p', n1, n2, th1, th2) == interface_Rp_approx);
}

void test_interface_T() {
    std::valarray<std::complex<double>> th1 = {0.2, std::numbers::pi / 2};
    std::valarray<std::complex<double>> n1 = {2, 2};
    std::valarray<std::complex<double>> n2 = {3, 3};
    std::valarray<std::complex<double>> th2 = std::asin(n1 / n2 * std::sin(th1));
    ApproxSequenceLike<std::valarray<double>, double> interface_Ts_approx = approx<std::valarray<double>, double>({0.9578062873191139, 0});
    ApproxSequenceLike<std::valarray<double>, double> interface_Tp_approx = approx<std::valarray<double>, double>({0.962139911578548, 0});
    assert(interface_T('s', n1, n2, th1, th2) == interface_Ts_approx);
    assert(interface_T('p', n1, n2, th1, th2) == interface_Tp_approx);
}

// Test for coh_tmm
void test_coh_tmm_s_r() {
    // An interesting discussion on VLA and std::dynarray:
    // https://stackoverflow.com/questions/1887097/why-arent-variable-length-arrays-part-of-the-c-standard/21519062
    // https://stackoverflow.com/questions/61141691/are-there-plans-to-implement-stddynarray-in-next-c-standard
    // EWG 13 N3639, N3497, N3467, N3412 Runtime-sized arrays with automatic storage duration
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> sr_approx = approx<std::valarray<std::complex<double>>, double>(std::valarray<std::complex<double>>{0.14017645 - 0.2132843i, 0.22307786 - 0.10704008i});
    assert(std::get<std::valarray<std::complex<double>>>(result.at("r")) == sr_approx);
}

void test_coh_tmm_s_t() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> st_approx = approx<std::valarray<std::complex<double>>, double>(std::valarray<std::complex<double>>{1.78669633e-05 - 9.79824244e-06i, -8.86075993e-02 - 4.05953564e-01i});
    assert(std::get<std::valarray<std::complex<double>>>(result.at("t")) == st_approx);
}

void test_coh_tmm_s_R() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<double>, double> sR_approx = approx<std::valarray<double>, double>(std::valarray<double>{0.06513963, 0.06122131});
    assert(std::get<std::valarray<double>>(result.at("R")) == sR_approx);
}

void test_coh_tmm_s_T() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<double>, double> sT_approx = approx<std::valarray<double>, double>(std::valarray<double>{1.15234466e-09, 4.13619185e-01});
    assert(std::get<std::valarray<double>>(result.at("T")) == sT_approx);
}

void test_coh_tmm_s_power_entering() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<double>, double> spower_approx = approx<std::valarray<double>, double>(std::valarray<double>{0.93486037, 0.93877869});
    assert(std::get<std::valarray<double>>(result.at("power_entering")) == spower_approx);
}

void test_coh_tmm_s_vw_list() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::vector<std::complex<double>>, double> svwl_approx = approx<std::vector<std::complex<double>>, double>(std::vector<std::complex<double>>{
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
    const std::vector<std::complex<double>> s_vw_list = vva2_flatten<std::valarray<std::vector<std::array<std::complex<double>, 2>>>, std::complex<double>, 2>(std::get<std::valarray<std::vector<std::array<std::complex<double>, 2>>>>(result.at("vw_list")));
    assert(s_vw_list == svwl_approx);
}

void test_coh_tmm_p_power_entering() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<double>, double> ppower_approx = approx<std::valarray<double>, double>(std::valarray<double>{0.96244989, 0.94994018});
    assert(std::get<std::valarray<double>>(result.at("power_entering")) == ppower_approx);
}

void test_coh_tmm_p_vw_list() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::vector<std::complex<double>>, double> pvwl_approx = approx<std::vector<std::complex<double>>, double>(std::vector<std::complex<double>>{
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
    const std::vector<std::complex<double>> p_vw_list = vva2_flatten<std::valarray<std::vector<std::array<std::complex<double>, 2>>>, std::complex<double>, 2>(std::get<std::valarray<std::vector<std::array<std::complex<double>, 2>>>>(result.at("vw_list")));
    assert(p_vw_list == pvwl_approx);
}

void test_coh_tmm_kz_list() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> kzl_approx = approx<std::valarray<std::complex<double>>, double>(std::valarray<std::complex<double>>{
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
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> thl_approx = approx<std::valarray<std::complex<double>>, double>(std::valarray<std::complex<double>>{
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
    n_list = va_2d_transpose(n_list, 2);
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
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const coh_tmm_vec_dict<double> result = coh_tmm_reverse('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> r_approx = approx<std::valarray<std::complex<double>>, double>({0.44495594 - 0.08125146i, 0.13483962 - 0.37537464i});
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> t_approx = approx<std::valarray<std::complex<double>>, double>({5.64612420e-05 - 1.46509665e-05i, -1.94928443e-01 - 9.82812305e-01i});
    ApproxSequenceLike<std::valarray<double>, double> R_approx = approx<std::valarray<double>, double>({0.20458758, 0.15908784});
    ApproxSequenceLike<std::valarray<double>, double> T_approx = approx<std::valarray<double>, double>({1.22605928e-09, 4.19050975e-01});
    ApproxSequenceLike<std::valarray<double>, double> power_approx = approx<std::valarray<double>, double>({0.75431194, 0.84091216});
    ApproxSequenceLike<std::vector<std::complex<double>>, double> vwl_approx = approx<std::vector<std::complex<double>>, double>({
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
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> kzl_approx = approx<std::valarray<std::complex<double>>, double>({
            0.06246792+0.01579948i, 0.01056179+0.i        ,
            0.07823055+0.i        , 0.01413365+0.i        ,
            0.03118008+0.04748033i, 0.00515452+0.00110011i,
            0.01435451+0.00687561i, 0.00404247+0.00074813i,
            0.02250959+0.i        , 0.00440866+0.i
    }, 1e-5);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> thl_approx = approx<std::valarray<std::complex<double>>, double>({
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
    const std::vector<std::complex<double>> vw_list = vva2_flatten<std::valarray<std::vector<std::array<std::complex<double>, 2>>>, std::complex<double>, 2>(std::get<std::valarray<std::vector<std::array<std::complex<double>, 2>>>>(result.at("vw_list")));
    assert(vw_list == vwl_approx);
    assert(std::get<std::valarray<std::complex<double>>>(result.at("th_list")) == thl_approx);
    assert(std::get<std::valarray<std::complex<double>>>(result.at("kz_list")) == kzl_approx);
}

void test_ellips_psi() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const std::unordered_map<std::string, std::valarray<double>> psi_result = ellips(n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<double>, double> psi_approx = approx<std::valarray<double>, double>(std::valarray<double>{0.64939282, 0.73516374});
    assert(psi_result.at("psi") == psi_approx);
}

void test_ellips_Delta() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const std::unordered_map<std::string, std::valarray<double>> Delta_result = ellips(n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<double>, double> Delta_approx = approx<std::valarray<double>, double>(std::valarray<double>{0.64939282, 0.73516374});
    assert(Delta_result.at("psi") == Delta_approx);
}

void test_unpolarized_RT_R() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const std::unordered_map<std::string, std::valarray<double>> RT_result = unpolarized_RT(n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<double>, double> RT_R_approx = approx<std::valarray<double>, double>(std::valarray<double>{0.05134487, 0.05564057});
    assert(RT_result.at("R") == RT_R_approx);
}

void test_unpolarized_RT_T() {
    std::valarray<std::complex<double>> n_list = {1.5, 1.0 + 0.4i, 2.0 + 3i, 5, 4.0 + 1i,
                                                  1.3, 1.2 + 0.2i, 1.5 + 0.3i, 4, 3.0 + 0.1i};
    n_list = va_2d_transpose(n_list, 2);
    const std::valarray<double> d_list = {INFINITY, 200, 187.3, 1973.5, INFINITY};
    constexpr std::complex<double> th_0 = 0.3;
    const std::valarray<double> lam_vac = {400, 1770};
    const std::unordered_map<std::string, std::valarray<double>> RT_result = unpolarized_RT(n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<double>, double> RT_T_approx = approx<std::valarray<double>, double>(std::valarray<double>{1.19651603e-09, 4.17401823e-01});
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
    ApproxSequenceLike<std::valarray<double>, double> poyn_approx = approx<std::valarray<double>, double>(va_2d_transpose(std::valarray<double>{
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
    ApproxSequenceLike<std::valarray<double>, double> absorption_approx = approx<std::valarray<double>, double>(va_2d_transpose(std::valarray<double>{
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
            {"r", std::valarray<std::complex<double>>{-0.12140058 + 0.15103645j, -0.21104259 + 0.07430242i}},
            {"t", std::valarray<std::complex<double>>{1.82536479e-05 - 1.06422631e-05j, -9.02947159e-02 - 4.09448171e-01i}},
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
    ApproxSequenceLike<std::valarray<double>, double> poyn_approx = approx<std::valarray<double>, double>(va_2d_transpose(std::valarray<double>{
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
            {"r", std::valarray<std::complex<double>>{-0.12140058 + 0.15103645j, -0.21104259 + 0.07430242i}},
            {"t", std::valarray<std::complex<double>>{1.82536479e-05 - 1.06422631e-05j, -9.02947159e-02 - 4.09448171e-01i}},
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
    ApproxSequenceLike<std::valarray<double>, double> absor_approx = approx<std::valarray<double>, double>(va_2d_transpose(std::valarray<double>{
            0.00000000e+00, 1.08969642e-02, 5.17505408e-04, 4.08492602e-04,
            2.19262267e-05, 0.00000000e+00, 0.00000000e+00, 3.92044315e-11,
            7.05804410e-14,
            0.00000000e+00, 1.91824256e-03, 1.12566565e-03, 1.77891531e-03,
            1.46979454e-03, 0.00000000e+00, 0.00000000e+00, 3.01509108e-04,
            2.61289301e-04
    }, 2));
    assert(std::get<std::valarray<double>>(pr_result.at("absor")) == absor_approx);
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
    test_unpolarized_RT_T();  // Error: [7.12e-4, 1.86e-5].
    test_position_resolved_s_poyn();  // Will fail layer/distance[0] and [6].
    test_position_resolved_s_absor();  // Same as above
    test_position_resolved_p_poyn();  // Same as above
    test_position_resolved_p_absor();  // Same as above
}

auto main() -> int {
    test_position_resolved_p_absor();
}

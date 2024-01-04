//
// Created by Yihua on 2023/11/27.
//

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
    const std::vector<std::complex<double>> s_vw_list = vva2_flatten<std::complex<double>, 2>(std::get<std::vector<std::vector<std::array<std::complex<double>, 2>>>>(result.at("vw_list")));
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
    const std::vector<std::complex<double>> p_vw_list = vva2_flatten<std::complex<double>, 2>(std::get<std::vector<std::vector<std::array<std::complex<double>, 2>>>>(result.at("vw_list")));
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
    assert(std::get<char>(result.at("pol")) == 's');
}

auto main() -> int {
    test_coh_tmm_inputs();
}

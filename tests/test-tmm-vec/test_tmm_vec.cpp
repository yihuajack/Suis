//
// Created by Yihua on 2023/11/27.
//

#include <cassert>
#include <numbers>
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
void test_coh_tmm_exceptions() {
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
    const coh_tmm_dict<double> result = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    ApproxSequenceLike<std::valarray<std::complex<double>>, double> rs_approx = approx<std::valarray<std::complex<double>>, double>(std::valarray<std::complex<double>>{0.14017645 - 0.2132843i, 0.22307786 - 0.10704008i});
    assert(result.at("r") == rs_approx);
}

auto main() -> int {
    test_interface_T();
}

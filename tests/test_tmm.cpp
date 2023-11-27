/*
 * Tests to ensure tmm package was coded correctly.
 * Use run_all() to run them all in order.
 */
#include <numeric>
#include <unordered_set>
// Using target_include_directories(tmm PRIVATE ${CMAKE_SOURCE_DIR}/../../src/optics)
// We can directly include tmm.h rather than #include "../../src/optics/tmm.h"
// See https://stackoverflow.com/questions/31969547/what-is-the-difference-between-include-directories-and-target-include-directorie
#include "tmm.h"
#include "utils.h"

// "5 * degree" is 5 degrees expressed in radians
// "1.2 / degree" is 1.2 radians expressed in degrees
static constexpr double degree = M_PI / 180;

void basic_test();
void position_resolved_test();
void position_resolved_test2();
void absorp_analytic_fn_test();
void incoherent_test();
void RT_test();
void coh_overflow_test();
void inc_overflow_test();

void runall() {
    basic_test();
    position_resolved_test();
    position_resolved_test2();
    absorp_analytic_fn_test();
    incoherent_test();
    RT_test();
    coh_overflow_test();
    inc_overflow_test();
}

static inline auto df(std::complex<double> a, std::complex<double> b) -> double {  // difference fraction
    return std::abs(a - b) / std::max(std::abs(a), std::abs(b));
}

/*
 * Compare with program I[Steven] wrote previously in Mathematica.
 * Also confirms that I[Steven] don't accidentally mess up the program by editing.
 */
void basic_test() {
    const std::valarray<std::complex<double>> n_list = {1, 2 + 4I, 3 + 0.3I, 1 + 0.1I};
    const std::valarray<double> d_list = {INFINITY, 2, 3, INFINITY};
    constexpr std::complex<double> th_0 = 0.1;
    constexpr double lam_vac = 100;

    // Clang-Tidy: Do not use 'std::endl' with streams; use '\n' instead
    std::cout << "The following should all be zero (within rounding errors):\n";

    const coh_tmm_dict<double> s_data = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    std::cout << df(std::get<std::complex<double>>(s_data.at("r")), -0.60331226568845775 - 0.093522181653632019I) << '\n';
    std::cout << df(std::get<std::complex<double>>(s_data.at("t")), 0.44429533471192989 + 0.16921936169383078I) << '\n';
    std::cout << df(std::get<double>(s_data.at("R")), 0.37273208839139516) << '\n';
    std::cout << df(std::get<double>(s_data.at("T")), 0.22604491247079261) << '\n';
    const coh_tmm_dict<double> p_data = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    std::cout << df(std::get<std::complex<double>>(p_data.at("r")), 0.60102654255772481 + 0.094489146845323682I) << '\n';
    std::cout << df(std::get<std::complex<double>>(p_data.at("t")), 0.4461816467503148 + 0.17061408427088917I) << '\n';
    std::cout << df(std::get<double>(p_data.at("R")), 0.37016110373044969) << '\n';
    std::cout << df(std::get<double>(p_data.at("T")), 0.22824374314132009) << '\n';
    const std::unordered_map<std::string, double> ellips_data = ellips(n_list, d_list, th_0, lam_vac);
    std::cout << df(ellips_data.at("psi"), 0.78366777347038352) << '\n';
    std::cout << df(ellips_data.at("Delta"), 0.0021460774404193292) << '\n';
}

/*
 * Compare with program I[Steven] wrote previously in Mathematica.
 * Also, various consistency checks.
 */
void position_resolved_test() {
    const std::valarray<double> d_list = {INFINITY, 100, 300, INFINITY};  // in nm
    const std::valarray<std::complex<double>> n_list = {1, 2.2 + 0.2I, 3.3 + 0.3I, 1};
    constexpr std::complex<double> th_0 = M_PI / 4;
    constexpr double lam_vac = 400;
    std::size_t layer = 1;
    double dist = 37;  // dist may have the same type with d_list[i]
    std::cout << "The following should all be zero (within rounding errors):\n";

    char pol = 'p';
    coh_tmm_dict<double> coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
    std::cout << df(std::get<std::valarray<std::complex<double>>>(coh_tmm_data.at("kz_list"))[1], 0.0327410685922732 + 0.003315885921866465I) << '\n';
    std::unordered_map<std::string, std::variant<double, std::complex<double>>> data = position_resolved(layer, dist, coh_tmm_data);
    std::cout << df(std::get<double>(data.at("poyn")), 0.7094950598055798) << '\n';
    std::cout << df(std::get<double>(data.at("absor")), 0.005135049118053356) << '\n';
    std::cout << df(1, absorp_in_each_layer(coh_tmm_data).sum()) << '\n';

    pol = 's';
    coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
    std::cout << df(std::get<std::valarray<std::complex<double>>>(coh_tmm_data.at("kz_list"))[1], 0.0327410685922732 + 0.003315885921866465I) << '\n';
    data = position_resolved(layer, dist, coh_tmm_data);
    std::cout << df(std::get<double>(data.at("poyn")), 0.5422594735025152) << '\n';
    std::cout << df(std::get<double>(data.at("absor")), 0.004041912286816303) << '\n';
    std::cout << df(1, absorp_in_each_layer(coh_tmm_data).sum()) << '\n';

    std::unordered_map<std::string, std::variant<double, std::complex<double>>> data1;
    std::unordered_map<std::string, std::variant<double, std::complex<double>>> data2;
    // Poynting vector derivative should equal absorption
    for (const char polarization : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(polarization, n_list, d_list, th_0, lam_vac);
        data1 = position_resolved(layer, dist, coh_tmm_data);
        data2 = position_resolved(layer, dist + 0.001, coh_tmm_data);
        std::cout << "Finite difference should approximate derivative. Difference is " << df((std::get<double>(data1.at("absor")) + std::get<double>(data2.at("absor"))) / 2,
                                                                                             (std::get<double>(data1.at("poyn")) - std::get<double>(data2.at("poyn"))) / 0.001) << '\n';
    }

    // Poynting vector at the end should equal T
    layer = 2;
    dist = 300;
    for (const char polarization : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(polarization, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        std::cout << df(std::get<double>(data.at("poyn")), std::get<double>(coh_tmm_data.at("T"))) << '\n';
    }

    // Poynting vector at start should equal power_entering
    layer = 1;
    dist = 0;
    for (const char polarization : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(polarization, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        std::cout << df(std::get<double>(data.at("poyn")), std::get<double>(coh_tmm_data.at("power_entering"))) << '\n';
    }

    double poyn1 = NAN;
    double poyn2 = NAN;
    // Poynting vector should be continuous
    for (const char polarization : std::unordered_set<char>{'s', 'p'}) {
        layer = 1;
        dist = 100;
        coh_tmm_data = coh_tmm(polarization, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        poyn1 = std::get<double>(data.at("poyn"));
        layer = 2;
        dist = 0;
        coh_tmm_data = coh_tmm(polarization, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        poyn2 = std::get<double>(data.at("poyn"));
        std::cout << df(poyn1, poyn2) << '\n';
    }
}

/*
 * Similar to position_resolved_test(), but with initial and final medium
 * having a complex refractive index.
 */
void position_resolved_test2() {
    const std::valarray<double> d_list = {INFINITY, 100, 300, INFINITY};  // In nm
    // "00" is before the 0'th layer.
    // This is an easy way to generate th0,
    // ensuring that n0 * sin(th0) is real.
    constexpr std::complex<double> n00 = 1;
    constexpr std::complex<double> th00 = M_PI / 4;
    constexpr std::complex<double> n0 = 1 + 0.1I;
    const std::complex<double> th_0 = snell(n00, n0, th00);
    const std::valarray<std::complex<double>> n_list = {n0, 2.2 + 0.2I, 3.3 + 0.3I, 1 + 0.4I};
    constexpr double lam_vac = 400;
    std::size_t layer = 1;
    double dist = 37;
    std::cout << "The following should all be zero (within rounding errors):\n";

    coh_tmm_dict<double> coh_tmm_data;
    std::unordered_map<std::string, std::variant<double, std::complex<double>>> data;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        std::cout << df(1, absorp_in_each_layer(coh_tmm_data).sum()) << '\n';
    }

    std::unordered_map<std::string, std::variant<double, std::complex<double>>> data1;
    std::unordered_map<std::string, std::variant<double, std::complex<double>>> data2;
    // Poynting vector derivative should equal absorption
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        data1 = position_resolved(layer, dist, coh_tmm_data);
        data2 = position_resolved(layer, dist + 0.001, coh_tmm_data);
        std::cout << "Finite difference should approximate derivative. Difference is " << df((std::get<double>(data1.at("absor")) + std::get<double>(data2.at("absor"))) / 2,
                                                                                             (std::get<double>(data1.at("poyn")) - std::get<double>(data2.at("poyn"))) / 0.001) << '\n';
    }

    // Poynting vector at the end should equal T
    layer = 2;
    dist = 300;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        std::cout << df(std::get<double>(data.at("poyn")), std::get<double>(coh_tmm_data.at("T"))) << '\n';
    }

    // Poynting vector at start should equal power_entering
    layer = 1;
    dist = 0;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        std::cout << df(std::get<double>(data.at("poyn")), std::get<double>(coh_tmm_data.at("power_entering"))) << '\n';
    }

    double poyn1 = NAN;
    double poyn2 = NAN;
    // Poynting vector should be continuous
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        layer = 1;
        dist = 100;
        coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        poyn1 = std::get<double>(data.at("poyn"));
        layer = 2;
        dist = 0;
        coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        poyn2 = std::get<double>(data.at("poyn"));
        std::cout << df(poyn1, poyn2) << '\n';
    }
}

/*
 * Test absorp_analytic_fn functions
 */
void absorp_analytic_fn_test() {
    const std::valarray<double> d_list = {INFINITY, 100, 300, INFINITY};  // in nm
    const std::valarray<std::complex<double>> n_list = {1, 2.2 + 0.2I, 3.3 + 0.3I, 1};
    constexpr std::complex<double> th_0 = M_PI / 4;
    constexpr double lam_vac = 400;
    constexpr std::size_t layer = 1;
    const double d = d_list[layer];
    constexpr double dist = 37;
    std::cout << "The following should all be zero (within rounding errors):\n";

    coh_tmm_dict<double> coh_tmm_data;
    double expected_absorp = NAN;
    double dist_from_other_side = NAN;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        expected_absorp = std::get<double>(position_resolved(layer, dist, coh_tmm_data)["absor"]);
        AbsorpAnalyticFn<double> absorp_fn;
        absorp_fn.fill_in(coh_tmm_data, layer);
        std::cout << df(absorp_fn.run(dist), expected_absorp) << '\n';
        const AbsorpAnalyticFn<double> absorp_fn2 = absorp_fn.flip();
        dist_from_other_side = d - dist;
        std::cout << df(absorp_fn2.run(dist_from_other_side), expected_absorp) << '\n';
    }
}

/*
 * test inc_tmm(). To do: Add more tests.
 */
void incoherent_test() {
    std::cout << "The following should all be zero (within rounding errors):\n";

    // 3-incoherent-layer test, real refractive indices (so that R and T are the
    // same in both directions)
    std::complex<double> n0 = 1;  // avoid implicit conversion from double to std::complex<double>
    std::complex<double> n1 = 2;
    std::complex<double> n2 = 3;
    std::valarray<std::complex<double>> n_list = {n0, n1, n2};
    std::valarray<double> d_list = {INFINITY, 567, INFINITY};
    std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent};
    std::complex<double> th0 = M_PI / 3;
    const std::complex<double> th1 = snell(n0, n1, th0);
    std::complex<double> th2 = snell(n0, n2, th0);
    constexpr double lam_vac = 400;

    inc_tmm_dict<double> inc_data;
    double R0 = NAN;
    double R1 = NAN;
    double T0 = NAN;
    double RR = NAN;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        inc_data = inc_tmm(pol, n_list, d_list, c_list, th0, lam_vac);
        R0 = std::norm(interface_r(pol, n0, n1, th0, th1));
        R1 = std::norm(interface_r(pol, n1, n2, th1, th2));
        T0 = 1 - R0;
        RR = R0 + R1 * std::pow(T0, 2) / (1 - R0 * R1);
        std::cout << df(std::get<double>(inc_data.at("R")), RR) << '\n';
        std::cout << df(std::get<double>(inc_data.at("R")) + std::get<double>(inc_data.at("T")), 1) << '\n';
    }
    // One finite layer with incoherent layers on both sides.
    // Should agree with coherent program
    n0 = std::complex<double>(1, 0.1);
    n1 = std::complex<double>(2, 0.2);
    n2 = std::complex<double>(3, 0.4);
    n_list = {n0, n1, n2};
    d_list = {INFINITY, 100, INFINITY};
    c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Incoherent};
    constexpr std::complex<double> n00 = 1;
    constexpr std::complex<double> th00 = M_PI / 3;
    th0 = snell(n00, n0, th00);
    coh_tmm_dict<double> coh_data;
    std::vector<double> inc_each;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        inc_data = inc_tmm(pol, n_list, d_list, c_list, th0, lam_vac);
        coh_data = coh_tmm(pol, n_list, d_list, th0, lam_vac);
        std::cout << df(std::get<double>(inc_data.at("R")), std::get<double>(coh_data.at("R"))) << '\n';
        std::cout << df(std::get<double>(inc_data.at("T")), std::get<double>(coh_data.at("T"))) << '\n';
        inc_each = inc_absorp_in_each_layer(inc_data);
        std::cout << df(1, std::accumulate(inc_each.cbegin(), inc_each.cend(), 0.0)) << '\n';
    }
    // One finite layer with three incoherent layers.
    // Should agree with manual calculation + coherent program
    n2 = std::complex<double>(3, 0.004);
    constexpr std::complex<double> n3 = 4 + 0.2I;
    constexpr double d1 = 100;
    constexpr double d2 = 10000;
    n_list = {n0, n1, n2, n3};
    d_list = {INFINITY, d1, d2, INFINITY};
    c_list = {LayerType::Incoherent, LayerType::Coherent, LayerType::Incoherent, LayerType::Incoherent};
    th0 = snell(n00, n0, th00);
    std::complex<double> th3;
    coh_tmm_dict<double> coh_bdata;
    double R02 = NAN;
    double R20 = NAN;
    double T02 = NAN;
    double T20 = NAN;
    double P2 = NAN;
    double R23 = NAN;
    double T23 = NAN;
    double Tr = NAN;
    double R = NAN;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        inc_data = inc_tmm(pol, n_list, d_list, c_list, th0, lam_vac);
        coh_data = coh_tmm(pol, {n0, n1, n2}, {INFINITY, d1, INFINITY}, th0, lam_vac);
        th2 = snell(n0, n2, th0);
        th3 = snell(n0, n3, th0);
        coh_bdata = coh_tmm(pol, {n2, n1, n0}, {INFINITY, d1, INFINITY}, th2, lam_vac);
        R02 = std::get<double>(coh_data.at("R"));
        R20 = std::get<double>(coh_bdata.at("R"));
        T02 = std::get<double>(coh_data.at("T"));
        T20 = std::get<double>(coh_bdata.at("T"));
        P2 = std::exp(-4 * M_PI * d2 * (n2 * std::cos(th2)).imag() / lam_vac);  // fraction passing through
        R23 = interface_R(pol, n2, n3, th2, th3);
        T23 = interface_T(pol, n2, n3, th2, th3);
        // T = T02 * P2 * T23 + T02 * P2 * R23 * P2 * R20 * P2 * T23 + ...
        Tr = T02 * P2 * T23 / (1 - R23 * P2 * R20 * P2);
        // R = R02
        //     + T02 * P2 * R23 * P2 * T20
        //     + T02 * P2 * R23 * P2 * R20 * P2 * R23 * P2 * T20 + ...
        R = R02 + T02 * P2 * R23 * P2 * T20 / (1 - R20 * P2 * R23 * P2);
        std::cout << df(std::get<double>(inc_data.at("T")), Tr) << '\n';
        std::cout << df(std::get<double>(inc_data.at("R")), R) << '\n';
    }
    // The coherent program with a thick but randomly-varying-thickness substrate
    // should agree with the incoherent program.
    constexpr std::complex<double> nair = 1 + 0.1I;
    constexpr std::complex<double> nfilm = 2 + 0.2I;
    constexpr std::complex<double> nsub = 3;
    constexpr std::complex<double> nf = 3 + 0.4I;
    n_list = {nair, nfilm, nsub, nf};
    std::valarray<double> d_list_inc;
    double dsub = NAN;
    const std::array<double, 357> ls_arr = linspace<double, 357>(10000.0, 30000.0);
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        d_list_inc = {INFINITY, 100, 1, INFINITY};  // sub thickness doesn't matter here
        inc_data = inc_tmm(pol, n_list, d_list_inc, c_list, th0, lam_vac);
        std::array<double, 357> coh_Rs{};
        std::array<double, 357> coh_Ts{};
        dsub = NAN;
        for (std::size_t i = 0; i < 357; i++) {
            dsub = ls_arr.at(i);
            d_list = {INFINITY, 100, dsub, INFINITY};
            coh_data = coh_tmm(pol, n_list, d_list, th0, lam_vac);
            coh_Rs.at(i) = std::get<double>(coh_data.at("R"));
            coh_Ts.at(i) = std::get<double>(coh_data.at("T"));
        }
        std::cout << "Coherent with random thickness should agree with incoherent. "
                     "Discrepancy is: " << df(std::accumulate(coh_Rs.cbegin(), coh_Rs.cend(), 0.0) / 357, std::get<double>(inc_data.at("R"))) << '\n';
        std::cout << "Coherent with random thickness should agree with incoherent. "
                     "Discrepancy is: " << df(std::accumulate(coh_Ts.cbegin(), coh_Ts.cend(), 0.0) / 357, std::get<double>(inc_data.at("T"))) << '\n';
    }
    // The coherent program with a thick substrate and randomly varying wavelength
    // should agree with the incoherent program.
    n0 = 1;
    n_list = {n0, 2 + 0.0002I, 3 + 0.0001I, 3 + 0.4I};
    th0 = snell(n00, n0, th00);
    d_list = {INFINITY, 10000, 10200, INFINITY};
    c_list = {LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent};
    std::valarray<double> inc_absorp;
    std::valarray<double> coh_absorp;
    constexpr std::size_t num_pts = 234;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        inc_absorp = {0, 0, 0, 0};
        coh_absorp = {0, 0, 0, 0};
        for (const double pt_lam_vac : linspace(40.0, 50.0, num_pts)) {
            inc_data = inc_tmm(pol, n_list, d_list, c_list, th0, pt_lam_vac);
            inc_each = inc_absorp_in_each_layer(inc_data);
            std::transform(inc_each.cbegin(), inc_each.cend(), std::begin(inc_absorp), std::begin(inc_absorp), std::plus<double>());
            coh_data = coh_tmm(pol, n_list, d_list, th0, pt_lam_vac);
            coh_absorp += absorp_in_each_layer(coh_data);
        }
        inc_absorp /= num_pts;
        coh_absorp /= num_pts;
        std::cout << "Coherent with random wavelength should agree with incoherent. "
                     "The two rows of this array should be the same:\n";
        print_container(inc_absorp);
        print_container(coh_absorp);
    }
}

/*
 * Tests of formulas for R and T
 */
void RT_test() {
    std::cout << "The following should all be zero (within rounding errors):\n";

    // When ni is real [see manual], R+T should equal 1
    std::complex<double> ni = 2;
    constexpr std::complex<double> nf = 3 + 0.2I;
    std::complex<double> thi = M_PI / 5;
    std::complex<double> thf = snell(ni, nf, thi);
    double T = NAN;
    double R = NAN;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        T = interface_T(pol, ni, nf, thi, thf);
        R = interface_R(pol, ni, nf, thi, thf);
        std::cout << df(1, R + T) << '\n';
    }

    // For a single interface, power_entering should equal T
    ni = std::complex<double>(2, 0.1);
    constexpr std::complex<double> n00 = 1;
    thf = snell(ni, nf, thi);
    thf = snell(ni, nf, thi);
    std::complex<double> r;
    double pe = NAN;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        r = interface_r(pol, ni, nf, thi, thf);
        pe = power_entering_from_r(pol, r, ni, thi);
        T = interface_T(pol, ni, nf, thi, thf);
        std::cout << df(pe, T) << '\n';
    }
}

/*
 * Test whether very, very opaque layers will break the coherent program
 */
void coh_overflow_test() {
    const std::valarray<std::complex<double>> n_list = {1, 2 + 0.1I, 1 + 3I, 4, 5};
    const std::valarray<double> d_list = {INFINITY, 50, 1e5, 50, INFINITY};
    constexpr double lam = 200;
    const double alpha_d = n_list[2].imag() * 4 * M_PI * d_list[2] / lam;
    std::cout << "Very opaque layer: Calculation should involve e^(-" << alpha_d << ")!\n";
    coh_tmm_dict<double> data = coh_tmm('s', n_list, d_list, std::complex<double>(0, 0), lam);
    const std::valarray<std::complex<double>> n_list2 = std::valarray(n_list[std::slice(0, 3, 1)]);
    std::valarray<double> d_list2 = std::valarray(d_list[std::slice(0, 3, 1)]);
    d_list2[d_list2.size() - 1] = INFINITY;
    const coh_tmm_dict<double> data2 = coh_tmm('s', n_list2, d_list2, std::complex<double>(0, 0), lam);
    std::cout << "First entries of the following two lists should agree:\n";
    print_spec2d_container<std::vector<std::array<std::complex<double>, 2>>, double>(std::get<std::vector<std::array<std::complex<double>, 2>>>(data.at("vw_list")));
    print_spec2d_container<std::vector<std::array<std::complex<double>, 2>>, double>(std::get<std::vector<std::array<std::complex<double>, 2>>>(data2.at("vw_list")));
}

/*
 * Test whether very, very opaque layers will break the coherent program
 */
void inc_overflow_test() {
    const std::valarray<std::complex<double>> n_list = {1, 2, 1 + 3I, 4, 5};
    const std::valarray<double> d_list = {INFINITY, 50, 1e5, 50, INFINITY};
    const std::valarray<LayerType> c_list = {LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent, LayerType::Incoherent};
    constexpr double lam = 200;
    const double alpha_d = n_list[2].imag() * 4 * M_PI * d_list[2] / lam;
    std::cout << "Very opaque layer: Calculation should involve e^(-" << alpha_d << ")!\n";
    inc_tmm_dict<double> data = inc_tmm('s', n_list, d_list, c_list, std::complex<double>(0, 0), lam);
    const std::valarray<std::complex<double>> n_list2 = std::valarray(n_list[std::slice(0, 3, 1)]);
    std::valarray<double> d_list2 = std::valarray(d_list[std::slice(0, 3, 1)]);
    d_list2[d_list2.size() - 1] = INFINITY;
    const std::valarray<LayerType> c_list2 = std::valarray(c_list[std::slice(0, 3, 1)]);
    const inc_tmm_dict<double> data2 = inc_tmm('s', n_list2, d_list2, c_list2, std::complex<double>(0, 0), lam);
    std::cout << "First entries of the following two lists should agree:\n";
    print_container(std::get<std::vector<double>>(data.at("power_entering_list")));
    print_container(std::get<std::vector<double>>(data2.at("power_entering_list")));
}

auto main() -> int {
    runall();
}

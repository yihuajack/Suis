/*
 * Tests to ensure tmm package was coded correctly.
 * Use run_all() to run them all in order.
 */
#include <iostream>
#include <unordered_set>
// Using target_include_directories(tmm PRIVATE ${CMAKE_SOURCE_DIR}/../../src/optics)
// We can directly include tmm.h rather than #include "../../src/optics/tmm.h"
// See https://stackoverflow.com/questions/31969547/what-is-the-difference-between-include-directories-and-target-include-directorie
#include "tmm.h"

// "5 * degree" is 5 degrees expressed in radians
// "1.2 / degree" is 1.2 radians expressed in degrees
static constexpr double degree = M_PI / 180;

void basic_test();
void position_resolved_test();
void position_resolved_test2();
void absorp_analytic_fn_test();

void runall() {
    basic_test();
    position_resolved_test();
    position_resolved_test2();
    absorp_analytic_fn_test();
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

    // Poynting vector derivative should equal absorption
    for (const char polarization : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_data = coh_tmm(polarization, n_list, d_list, th_0, lam_vac);
        const std::unordered_map<std::string, std::variant<double, std::complex<double>>> data1 = position_resolved(layer, dist, coh_tmm_data);
        const std::unordered_map<std::string, std::variant<double, std::complex<double>>> data2 = position_resolved(layer, dist + 0.001, coh_tmm_data);
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

    // Poynting vector should be continuous
    for (const char polarization : std::unordered_set<char>{'s', 'p'}) {
        layer = 1;
        dist = 100;
        coh_tmm_data = coh_tmm(polarization, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        const double poyn1 = std::get<double>(data.at("poyn"));
        layer = 2;
        dist = 0;
        coh_tmm_data = coh_tmm(polarization, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        const double poyn2 = std::get<double>(data.at("poyn"));
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

    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        const coh_tmm_dict<double> coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        const std::unordered_map<std::string, std::variant<double, std::complex<double>>> data = position_resolved(layer, dist, coh_tmm_data);
        std::cout << df(1, absorp_in_each_layer(coh_tmm_data).sum()) << '\n';
    }

    // Poynting vector derivative should equal absorption
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        const coh_tmm_dict<double> coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        const std::unordered_map<std::string, std::variant<double, std::complex<double>>> data1 = position_resolved(layer, dist, coh_tmm_data);
        const std::unordered_map<std::string, std::variant<double, std::complex<double>>> data2 = position_resolved(layer, dist + 0.001, coh_tmm_data);
        std::cout << "Finite difference should approximate derivative. Difference is " << df((std::get<double>(data1.at("absor")) + std::get<double>(data2.at("absor"))) / 2,
                                                                                             (std::get<double>(data1.at("poyn")) - std::get<double>(data2.at("poyn"))) / 0.001) << '\n';
    }

    // Poynting vector at the end should equal T
    layer = 2;
    dist = 300;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        const coh_tmm_dict<double> coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        const std::unordered_map<std::string, std::variant<double, std::complex<double>>> data = position_resolved(layer, dist, coh_tmm_data);
        std::cout << df(std::get<double>(data.at("poyn")), std::get<double>(coh_tmm_data.at("T"))) << '\n';
    }

    // Poynting vector at start should equal power_entering
    layer = 1;
    dist = 0;
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        const coh_tmm_dict<double> coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        const std::unordered_map<std::string, std::variant<double, std::complex<double>>> data = position_resolved(layer, dist, coh_tmm_data);
        std::cout << df(std::get<double>(data.at("poyn")), std::get<double>(coh_tmm_data.at("power_entering"))) << '\n';
    }

    // Poynting vector should be continuous
    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        layer = 1;
        dist = 100;
        coh_tmm_dict<double> coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        std::unordered_map<std::string, std::variant<double, std::complex<double>>> data = position_resolved(layer, dist, coh_tmm_data);
        const double poyn1 = std::get<double>(data.at("poyn"));
        layer = 2;
        dist = 0;
        coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        data = position_resolved(layer, dist, coh_tmm_data);
        const double poyn2 = std::get<double>(data.at("poyn"));
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

    for (const char pol : std::unordered_set<char>{'s', 'p'}) {
        coh_tmm_dict<double> coh_tmm_data = coh_tmm(pol, n_list, d_list, th_0, lam_vac);
        const double expected_absorp = std::get<double>(position_resolved(layer, dist, coh_tmm_data)["absor"]);
        AbsorpAnalyticFn<double> absorp_fn;
        absorp_fn.fill_in(coh_tmm_data, layer);
        std::cout << df(absorp_fn.run(dist), expected_absorp) << '\n';
        const AbsorpAnalyticFn<double> absorp_fn2 = absorp_fn.flip();
        const double dist_from_other_side = d - dist;
        std::cout << df(absorp_fn2.run(dist_from_other_side), expected_absorp) << '\n';
    }
}

auto main() -> int {
    absorp_analytic_fn_test();
}

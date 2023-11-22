/*
 * Tests to ensure tmm package was coded correctly.
 * Use run_all() to run them all in order.
 */
#include <iostream>
// Using target_include_directories(tmm PRIVATE ${CMAKE_SOURCE_DIR}/../../src/optics)
// We can directly include tmm.h rather than #include "../../src/optics/tmm.h"
// See https://stackoverflow.com/questions/31969547/what-is-the-difference-between-include-directories-and-target-include-directorie
#include "../../src/optics/FixedMatrix.h"
#include "../../src/optics/tmm.h"

// "5 * degree" is 5 degrees expressed in radians
// "1.2 / degree" is 1.2 radians expressed in degrees
static constexpr double degree = M_PI / 180;

void basic_test();

void runall() {
    basic_test();
}

static inline auto df(std::complex<double> a, std::complex<double> b) -> double {  // difference fraction
    return std::abs(a - b) / std::max(std::abs(a), std::abs(b));
}

/*
 * Compare with program I[Steven] wrote previously in Mathematica.
 * Also confirms that I[Steven] don't accidentally mess up the program by editing.
 */
void basic_test() {
    std::valarray<std::complex<double>> n_list = {1, 2 + 4I, 3 + 0.3I, 1 + 0.1I};
    std::valarray<double> d_list = {INFINITY, 2, 3, INFINITY};
    std::complex<double> th_0 = 0.1;
    double lam_vac = 100;

    // Clang-Tidy: Do not use 'std::endl' with streams; use '\n' instead
    std::cout << "The following should all be zero (within rounding errors):\n";

    coh_tmm_dict<double> s_data = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    std::cout << df(std::get<std::complex<double>>(s_data["r"]), -0.60331226568845775 - 0.093522181653632019I) << '\n';
    std::cout << df(std::get<std::complex<double>>(s_data["t"]), 0.44429533471192989 + 0.16921936169383078I) << '\n';
    std::cout << df(std::get<double>(s_data["R"]), 0.37273208839139516) << '\n';
    std::cout << df(std::get<double>(s_data["T"]), 0.22604491247079261) << '\n';
    coh_tmm_dict<double> p_data = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    std::cout << df(std::get<std::complex<double>>(p_data["r"]), 0.60102654255772481 + 0.094489146845323682I) << '\n';
    std::cout << df(std::get<std::complex<double>>(p_data["t"]), 0.4461816467503148 + 0.17061408427088917I) << '\n';
    std::cout << df(std::get<double>(p_data["R"]), 0.37016110373044969) << '\n';
    std::cout << df(std::get<double>(p_data["T"]), 0.22824374314132009) << '\n';
    std::unordered_map<std::string, double> ellips_data = ellips(n_list, d_list, th_0, lam_vac);
    std::cout << df(ellips_data["psi"], 0.78366777347038352) << '\n';
    std::cout << df(ellips_data["Delta"], 0.0021460774404193292) << '\n';
}

int main() {
    void runall();
}
//
// Created by Yihua on 2023/11/27.
//

#include <cassert>
#include "tmm.h"
#include "Approx.h"

using namespace std::complex_literals;

void test_snell() {
    // assert() does not support comma operators. However, you can have a look at
    // P2264 R1-[P2264R5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2264r5.html)
    // https://github.com/cplusplus/papers/issues/957
    // Make assert() macro user friendly for C and C++
    // assert(snell(3.0 + 0i, 2.0 + 0i, 0.7 + 0i) == approx<std::complex<double>, double>(1.3105496419558818));
    ApproxScalar<std::complex<double>, double> snell_approx = approx<std::complex<double>, double>(1.3105496419558818);
    std::complex<double> snell_result = snell(3.0 + 0i, 2.0 + 0i, 0.7 + 0i);
    assert(snell_result == snell_approx);
}

auto main() -> int {
    test_snell();
}

//
// Created by Yihua on 2023/11/27.
//

#include <iostream>
#include <list>
#include "tmm.h"
#include "Approx.h"

void test_snell() {
    auto v1 = std::vector<double>{1, 2, 3};
    auto v2 = std::list<short>{1, 2, 3, 4};
    auto v3 = std::to_array({1, 2, 3, 4, 5});

    bool result_bool = approx<double>(1.0) == 0.99;
    bool result = approx<std::vector<double>, double>(v1) == std::vector<double>{1.01, 2, 3};

    // We can write std::__cxx11::list<> for gcc but not for msvc:
    // error C3083: the symbol to the left of a "__cxx11":"::" must be a type
    // std::ranges::zip_view<std::ranges::ref_view<std::vector<float, std::allocator<float>>>,
    //         std::ranges::ref_view<std::list<short, std::allocator<short>>>,
    //                 std::ranges::ref_view<std::array<int, 5>>> sum = std::views::zip(v1, v2, v3);

    std::cout << std::boolalpha << result_bool << '\n';
    std::cout << result << '\n';
}

auto main() -> int {
    test_snell();
}

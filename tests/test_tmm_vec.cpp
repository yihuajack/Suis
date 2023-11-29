//
// Created by Yihua on 2023/11/27.
//

#include <iostream>
#include <list>
#include <ranges>
#include "tmm.h"
#include "utils.h"

void test_snell() {
    auto v1 = std::vector<float>{1, 2, 3};
    auto v2 = std::list<short>{1, 2, 3, 4};
    auto v3 = std::to_array({1, 2, 3, 4, 5});

    auto add = [](auto a, auto b, auto c) { return a + b + c; };

    // We can write std::__cxx11::list<> for gcc but not for msvc:
    // error C3083: the symbol to the left of a "__cxx11":"::" must be a type
    std::ranges::zip_view<std::ranges::ref_view<std::vector<float, std::allocator<float>>>,
            std::ranges::ref_view<std::list<short, std::allocator<short>>>,
                    std::ranges::ref_view<std::array<int, 5>>>  sum = std::views::zip(v1, v2, v3);
}

auto main() -> int {
    test_snell();
}

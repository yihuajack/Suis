//
// Created by Yihua Liu on 2024/08/09.
//

#ifndef SUISAPP_GETVARSUB_H
#define SUISAPP_GETVARSUB_H

#include <algorithm>
#include <execution>

#include "Global.h"

// Builds individual variable arrays on the sub-interval xmesh
template<Vector V>
V getvar_sub(const V& var) {
    if (var.size() < 2) {
        return V{};  // Return empty vector if not enough data points
    }
    V result(var.size() - 1);
    std::transform(std::execution::par,
        var.begin(), var.end() - 1, var.begin() + 1, result.begin(),
                           [](const typename V::value_type& a, const typename V::value_type& b) {
                               return (a + b) / 2.0;
                           });
    return result;
}

template <Vector V>
auto getvar_sub(const V& var_mat) -> std::vector<decltype(getvar_sub(var_mat.front()))> {
    std::vector<decltype(getvar_sub(var_mat.front()))> result;

    for (const auto& row : var_mat) {
        result.push_back(getvar_sub(row));
    }

    return result;
}

#endif  // SUISAPP_GETVARSUB_H

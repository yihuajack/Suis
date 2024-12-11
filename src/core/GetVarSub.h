//
// Created by Yihua Liu on 2024/08/09.
//

#ifndef SUISAPP_GETVARSUB_H
#define SUISAPP_GETVARSUB_H

#include <algorithm>

#include "Global.h"

// Builds individual variable arrays on the sub-interval xmesh
template<Vector V>
V getvar_sub(const V& var) {
    V result(var.size() - 1);
    std::ranges::transform(var.begin(), var.end() - 1, var.begin() + 1, result.begin(), [](typename V::value_type a, typename V::value_type b) {
        return (a + b) / 2.0;
    });
    return result;
}

#endif  // SUISAPP_GETVARSUB_H

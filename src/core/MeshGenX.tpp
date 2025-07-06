//
// Created by Yihua Liu on 2025-07-01.
//

#ifndef SUISAPP_MESHGENX_TPP
#define SUISAPP_MESHGENX_TPP

#ifndef SUISAPP_PARAMETERCLASS_H
#error __FILE__ should only be included from ParameterClass.h.
#endif // SUISAPP_PARAMETERCLASS_H

template <typename C, typename R>
concept HasAppendRange = requires (C c, R r) {
    { c.append_range(r) } -> std::same_as<void>;
};

template <typename C, typename R>
concept HasAppend = requires(C c, R r) {
    { c.append(r) } -> std::same_as<void>;
};

template<template <typename...> class L, typename F_T, typename STR_T>
L<F_T> ParameterClass<L, F_T, STR_T>::meshgen_x() {
    // Linearly spaced
    L<F_T> x;  // pcum().back()
    L<F_T> dcum = dcum0();
    if (xmesh_type) {  // linear
        for (SZ_T i : std::views::iota(0, col_size())) {
            if constexpr (HasAppendRange<L<F_T>, L<F_T>>) {
                x.append_range(Utils::Math::linspace(dcum.at(i), dcum.at(i + 1) - d.at(i) / layer_points.at(i), layer_points.at(i)));
            } else if constexpr (HasAppend<L<F_T>, L<F_T>>) {
                x.append(Utils::Math::linspace(dcum.at(i), dcum.at(i + 1) - d.at(i) / layer_points.at(i), layer_points.at(i)));
            } else {
                static_assert([] {
                    return false;
                }(), "Container does not support append_range or append.");
            }
        }
        x.emplace_back(dcum.back());
    } else {  // erf-linear
        for (SZ_T i : std::views::iota(0, col_size())) {
            L<F_T> x_layer;
            if (layer_type.at(i) == "layer" or layer_type.at(i) == "active") {
                const std::vector<F_T> parr = Utils::Math::linspace(-0.5, 1 / std::numbers::pi_v<F_T>, 0.5);
                const std::size_t sz_parr = parr.size();
                x_layer.resize(sz_parr);
                const F_T x_layer0 = std::erf(2 * std::numbers::pi_v<F_T> * xmesh_coeff.front() * parr.front());
                x_layer.front() = 0;
                for (std::size_t j : std::views::iota(1U, sz_parr)) {
                    x_layer[j] = std::erf(2 * std::numbers::pi_v<F_T> * xmesh_coeff.at(j) * parr.at(j)) - x_layer0;
                }
                const F_T max_x_layer = std::ranges::max(x_layer);
                for (std::size_t j : std::views::iota(1U, sz_parr)) {
                    x_layer[j] = dcum.at(i) + x_layer.at(j) / max_x_layer * d.at(i);
                }
            } else if (layer_type.at(i) == "junction" or layer_type.at(i) == "interface") {
                x_layer = Utils::Math::linspace(dcum.at(i), dcum.at(i + 1) - d.at(i) / layer_points.at(i), layer_points.at(i));
            }
            if constexpr (HasAppendRange<L<F_T>, L<F_T>>) {
                x.append_range(x_layer.begin(), x_layer.end() - 1);
            } else if constexpr (HasAppend<L<F_T>, L<F_T>>) {
                x.append(x_layer.begin(), x_layer.end() - 1);
            } else {
                static_assert([] {
                    return false;
                }(), "Container does not support append_range or append.");
            }
            x.emplace_back(dcum.back());
        }
    }
    return x;
}

#endif  // SUISAPP_MESHGENX_TPP

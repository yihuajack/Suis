//
// Created by Yihua Liu on 2024-7-4.
//

#ifndef SUISAPP_TRANSFERMATRIX_H
#define SUISAPP_TRANSFERMATRIX_H

#include <numbers>
#include <ranges>
#include <unordered_map>
#include <variant>

#include "tmm.h"
#include "OpticStack.h"

/*
 * R: std::valarray<T>
 * A: std::valarray<T>
 * T: std::valarray<T>
 * A_per_layer: std::valarray<std::valarray<T>> (coh) / std::vector<std::valarray<T>> (inc)
 */
template<typename T>
using rat_dict = std::unordered_map<std::string, std::variant<std::valarray<T>, std::valarray<std::valarray<T>>,
        std::vector<std::valarray<T>>>>;

/*
 * Calculates the reflected, absorbed, and transmitted intensity of the structure
    for the wavelengths and angles defined.

    :param structure: A Structure object with layers and materials or a
        OpticStack object.
    :param wavelength: Wavelengths (in m) in which calculate the data. An array.
    :param angle: Angle (in degrees) of the incident light.
        Default: 0 (normal incidence).
    :param pol: Polarization of the light: 's', 'p', or 'u'. Default: 'u' (unpolarized).
    :param coherent: If the light is coherent or not. If not, a coherency list
        must be added.
    :param coherency_list: A list indicating in which layers light should be treated as
        coherent ('c') and in which incoherent ('i'). It needs as many elements as
        layers in the structure.
    :param no_back_reflection: If reflection from the back must be suppressed.
        Default=True.
    :return: A dictionary with the R, A, and T at the specified wavelengths and angle.
 */
template<typename U>
rat_dict<typename std::remove_reference_t<U>::value_type> calculate_rat(std::unique_ptr<OpticStack<std::remove_reference_t<U>>> stack,
                                                                        U &&wavelength,
                                                                        double angle = 0,
                                                                        char pol = 'u',
                                                                        bool coherent = true,
                                                                        const std::vector<char> &coherency_list = {}) {
    using T = typename std::remove_reference_t<U>::value_type;
    constexpr double degree = std::numbers::pi_v<typename std::remove_reference_t<U>::value_type> / 180;
    std::valarray<LayerType> coherency_va(stack->num_mat_layers + 2);
    if (not coherent) {
        if (not coherency_list.empty()) {
            if (coherency_list.size() not_eq stack->num_mat_layers) {
                const std::string error_info = "Error: The coherency list must have as many elements " +
                                               std::to_string(coherency_list.size()) +
                                               " as the number of layers " + std::to_string(stack->num_mat_layers);
                throw std::runtime_error(error_info);
            }
            coherency_va[0] = LayerType::Incoherent;
            for (const auto [i, layer_type] : std::views::enumerate(coherency_list)) {
                coherency_va[i] = layer_type == 'c' ? LayerType::Coherent : LayerType::Incoherent;
            }
            coherency_va[stack->num_mat_layers + 1] = LayerType::Incoherent;
            if (stack->no_back_reflection) {
                coherency_va.resize(stack->num_mat_layers + 3);
                coherency_va[stack->num_mat_layers + 2] = LayerType::Incoherent;
            }
        } else {
            const std::string error_info = "Error: For incoherent or partly incoherent calculations you must "
                                           "supply the coherency_list parameter with as many elements as the "
                                           "number of layers in the structure";
            throw std::runtime_error(error_info);
        }
    }
    rat_dict<T> rat_out;
    std::valarray<T> lam_vac(wavelength.size());
    std::ranges::copy(wavelength, std::begin(lam_vac));
    if (pol == 's' or pol == 'p') {
        if (coherent) {
            // Don't want to add a template for get_widths() to deal with std::vector<T> so just use the
            // std::valarray<T> version of coh_tmm
            const coh_tmm_vec_dict<T> out = coh_tmm(pol,
                                                    stack->template get_indices<std::valarray<std::complex<T>>>(std::forward<U>(wavelength)),
                                                    stack->template get_widths<std::valarray<T>>(),
                                                    std::complex<T>(angle * degree),
                                                    lam_vac);
            rat_out.emplace("R", std::get<std::valarray<T>>(out.at("R")));
            rat_out.emplace("T", std::get<std::valarray<T>>(out.at("T")));
            rat_out.emplace("A", 1 - std::get<std::valarray<T>>(out.at("R")) - std::get<std::valarray<T>>(out.at("T")));
            rat_out.emplace("A_per_layer", absorp_in_each_layer(out));
        } else {
            const inc_tmm_vec_dict<double> out = inc_tmm(pol,
                                                         stack->template get_indices<std::vector<std::valarray<std::complex<T>>>>(std::forward<U>(wavelength)),
                                                         stack->template get_widths<std::valarray<T>>(),
                                                         coherency_va,
                                                         std::complex<T>(angle * degree),
                                                         lam_vac);
            rat_out.emplace("R", std::get<std::valarray<T>>(out.at("R")));
            rat_out.emplace("T", std::get<std::valarray<T>>(out.at("T")));
            rat_out.emplace("A", 1 - std::get<std::valarray<T>>(out.at("R")) - std::get<std::valarray<T>>(out.at("T")));
            rat_out.emplace("A_per_layer", inc_absorp_in_each_layer(out));
        }
    } else {
        if (coherent) {
            const coh_tmm_vecn_dict<double> out_p = coh_tmm('p',
                                                            stack->template get_indices<std::vector<std::valarray<std::complex<T>>>>(std::forward<U>(wavelength)),
                                                            stack->template get_widths<std::vector<T>>(),
                                                            std::complex<T>(angle * degree),
                                                            lam_vac);
            const coh_tmm_vecn_dict<double> out_s = coh_tmm('s',
                                                            stack->template get_indices<std::vector<std::valarray<std::complex<T>>>>(std::forward<U>(wavelength)),
                                                            stack->template get_widths<std::vector<T>>(),
                                                            std::complex<T>(angle * degree),
                                                            lam_vac);
            rat_out.emplace("R", (std::get<std::valarray<T>>(out_p.at("R")) + std::get<std::valarray<T>>(out_s.at("R"))) / 2);
            rat_out.emplace("T", (std::get<std::valarray<T>>(out_p.at("T")) + std::get<std::valarray<T>>(out_s.at("T"))) / 2);
            rat_out.emplace("A", 1 - std::get<std::valarray<T>>(rat_out.at("R")) - std::get<std::valarray<T>>(rat_out.at("T")));
            rat_out.emplace("A_per_layer", (absorp_in_each_layer(out_p) + absorp_in_each_layer(out_s)) / std::valarray<T>(2));
            // A_per_layer_s and A_per_layer_p
        } else {
            const inc_tmm_vec_dict<double> out_p = inc_tmm('p',
                                                           stack->template get_indices<std::vector<std::valarray<std::complex<T>>>>(std::forward<U>(wavelength)),
                                                           stack->template get_widths<std::valarray<T>>(),
                                                           coherency_va,
                                                           std::complex<T>(angle * degree),
                                                           lam_vac);
            const inc_tmm_vec_dict<double> out_s = inc_tmm('s',
                                                           stack->template get_indices<std::vector<std::valarray<std::complex<T>>>>(std::forward<U>(wavelength)),
                                                           stack->template get_widths<std::valarray<T>>(),
                                                           coherency_va,
                                                           std::complex<T>(angle * degree),
                                                           lam_vac);
            rat_out.emplace("R", (std::get<std::valarray<T>>(out_p.at("R")) + std::get<std::valarray<T>>(out_s.at("R"))) / 2);
            rat_out.emplace("T", (std::get<std::valarray<T>>(out_p.at("T")) + std::get<std::valarray<T>>(out_s.at("T"))) / 2);
            rat_out.emplace("A", 1 - std::get<std::valarray<T>>(rat_out.at("R")) - std::get<std::valarray<T>>(rat_out.at("T")));
            std::vector<std::valarray<T>> A_per_layer;
            for (const auto [p, s] : std::views::zip(inc_absorp_in_each_layer(out_p), inc_absorp_in_each_layer(out_s))) {
                A_per_layer.emplace_back((p + s) / 2);
            }
            rat_out.emplace("A_per_layer", A_per_layer);
        }
    }
    return rat_out;
}

#endif  // SUISAPP_TRANSFERMATRIX_H

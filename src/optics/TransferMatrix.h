//
// Created by Yihua Liu on 2024-7-8.
//

#ifndef SUISAPP_TRANSFERMATRIX_H
#define SUISAPP_TRANSFERMATRIX_H

#include <unordered_map>
#include <variant>

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

template<typename U>
rat_dict<typename U::value_type> calculate_rat(OpticStack<U> *structure, U &&wavelength, double angle = 0,
                                               char pol = 'u', bool coherent = true,
                                               const std::vector<char> &coherency_list = {});

#endif  // SUISAPP_TRANSFERMATRIX_H

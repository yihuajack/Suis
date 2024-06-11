//
// Created by Yihua Liu on 2024-06-11.
//

#include "CompOpticMaterial.h"

CompOpticMaterial::CompOpticMaterial(QString mat_name, std::vector<std::pair<double, std::vector<double>>>& n_wl,
                                     std::vector<std::pair<double, std::vector<double>>>& n_data,
                                     std::vector<std::pair<double, std::vector<double>>>& k_wl,
                                     std::vector<std::pair<double, std::vector<double>>>& k_data) : mat_name(std::move(mat_name)),
                                                                                                    n_wl(std::move(n_wl)),
                                                                                                    n_data(std::move(n_data)),
                                                                                                    k_wl(std::move(k_wl)),
                                                                                                    k_data(std::move(k_data)) {}

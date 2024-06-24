//
// Created by Yihua Liu on 2024/3/31.
//

#include "OpticMaterial.h"

OpticMaterial::OpticMaterial(QString mat_name, QList<std::pair<double, QList<double>>>& n_wl,
                             QList<std::pair<double, QList<double>>>& n_data,
                             QList<std::pair<double, QList<double>>>& k_wl,
                             QList<std::pair<double, QList<double>>>& k_data) : mat_name(std::move(mat_name)),
                                                                                         n_wl(std::move(n_wl)),
                                                                                         n_data(std::move(n_data)),
                                                                                         k_wl(std::move(k_wl)),
                                                                                         k_data(std::move(k_data)) {}

QString OpticMaterial::name() const {
    return mat_name;
}

QList<double> OpticMaterial::nWl() const {
    return n_wl.back().second;
}

QList<double> OpticMaterial::nData() const {
    return n_data.back().second;
}

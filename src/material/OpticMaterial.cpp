//
// Created by Yihua Liu on 2024/3/31.
//

#include "OpticMaterial.h"

OpticMaterial::OpticMaterial(QString mat_name, std::vector<double>& n_wl, std::vector<double>& n_vec,
                             std::vector<double>& k_wl, std::vector<double>& k_vec) : mat_name(std::move(mat_name)),
                                                                                      n_wl(std::move(n_wl)), n_data(std::move(n_vec)), k_wl(std::move(k_wl)),
                                                                                      k_data(std::move(k_vec)) {}

QString OpticMaterial::matName() const {
    return mat_name;
}

void OpticMaterial::setMatName(const QString &name) {
    if (mat_name not_eq name) {
        mat_name = name;
        emit matNameChanged();
    }
}

QList<double> OpticMaterial::nWl() const {
    return QList<double>{n_wl.cbegin(), n_wl.cend()};
}

QList<double> OpticMaterial::nData() const {
    return QList<double>{n_data.cbegin(), n_data.cend()};
}

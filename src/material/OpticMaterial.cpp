//
// Created by Yihua Liu on 2024/3/31.
//

#include "OpticMaterial.h"

template<typename T>
QString OpticMaterial<T>::name() const {
    return mat_name;
}

template<typename T>
T OpticMaterial<T>::nWl() const {
    return n_wl.back().second;
}

template<typename T>
T OpticMaterial<T>::nData() const {
    return n_data.back().second;
}

template<typename T>
T OpticMaterial<T>::kWl() const {
    return k_wl.back().second;
}

template<typename T>
T OpticMaterial<T>::kData() const {
    return k_data.back().second;
}

template class OpticMaterial<QList<double>>;

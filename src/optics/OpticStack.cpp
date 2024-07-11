//
// Created by Yihua Liu on 2024-7-4.
//

#include "OpticStack.h"

/*
 * Returns the complex refractive index of the stack.

    :param wl: Wavelength of the light in m.
    :return: A list with the complex refractive index of each layer, including the
    semi-infinite front and back layers and, optionally, the back absorbing layer
    used to suppress back surface reflection.
 */
template<FloatingList T>
template<typename U>
requires std::same_as<U, std::valarray<std::complex<typename T::value_type>>>
U OpticStack<T>::get_indices(T &&wavelength) {
    const std::size_t sz_wl = wavelength.size();
    U indices(1, sz_wl * num_mat_layers);
    if (incidence) {
        const QList<double> n_data = incidence->nData();
        const QList<double> k_data = incidence->kData();
        if (sz_wl not_eq n_data.size() or sz_wl not_eq k_data.size()) {
            qWarning("n_data size does not match k_data size");
            return {};
        }
        for (qsizetype i = 0; i < sz_wl; i++) {
            indices[i] = {n_data.at(i), k_data.at(i)};
        }
    }
    if (substrate) {
        const QList<double> n_data = substrate->nData();
        const QList<double> k_data = substrate->kData();
        if (sz_wl not_eq n_data.size() or sz_wl not_eq k_data.size()) {
            qWarning("n_data size does not match k_data size");
            return {};
        }
        for (qsizetype i = 0; i < sz_wl; i++) {
            indices[(num_mat_layers - 1) * sz_wl + i] = {n_data.at(i), k_data.at(i)};
        }
    }
    for (std::size_t i = 0; i < structure.size(); i++) {
        for (qsizetype j = 0; j < sz_wl; j++) {
            const QList<double> n_data = structure.at(i).first->nData();
            const QList<double> k_data = structure.at(i).first->kData();
            indices[(i + 1) * sz_wl + j] = {n_data.at(i), k_data.at(i)};
        }
    }
    // substrate irrelevant if no_back_reflection = True
    if (no_back_reflection) {
        for (qsizetype i = 0; i < sz_wl; i++) {
            const T absorbing_k = k_absorbing(std::forward<T>(wavelength));
            indices[(num_mat_layers - 1) * sz_wl + i] = absorbing_k[i];
        }
    }
    return indices;
}

template<FloatingList T>
template<typename U>
requires std::same_as<U, std::vector<std::valarray<std::complex<typename T::value_type>>>>
U OpticStack<T>::get_indices(T &&wavelength) {
    const std::size_t sz_wl = wavelength.size();
    U indices(num_mat_layers, std::valarray<std::complex<typename T::value_type>>(1, sz_wl));
    if (incidence) {
        const QList<double> n_data = incidence->nData();
        const QList<double> k_data = incidence->kData();
        if (sz_wl not_eq n_data.size() or sz_wl not_eq k_data.size()) {
            qWarning("n_data size does not match k_data size");
            return {};
        }
        for (qsizetype i = 0; i < sz_wl; i++) {
            indices.front()[i] = {n_data.at(i), k_data.at(i)};
        }
    }
    if (substrate) {
        const QList<double> n_data = substrate->nData();
        const QList<double> k_data = substrate->kData();
        if (sz_wl not_eq n_data.size() or sz_wl not_eq k_data.size()) {
            qWarning("n_data size does not match k_data size");
            return {};
        }
        for (qsizetype i = 0; i < sz_wl; i++) {
            indices.back()[i] = {n_data.at(i), k_data.at(i)};
        }
    }
    for (std::size_t i = 0; i < structure.size(); i++) {
        for (qsizetype j = 0; j < sz_wl; j++) {
            const QList<double> n_data = structure.at(i).first->nData();
            const QList<double> k_data = structure.at(i).first->kData();
            indices.at(i + 1)[j] = {n_data.at(i), k_data.at(i)};
        }
    }
    // substrate irrelevant if no_back_reflection = True
    if (no_back_reflection) {
        for (qsizetype i = 0; i < sz_wl; i++) {
            const T absorbing_k = k_absorbing(std::forward<T>(wavelength));
            indices.back()[i] = absorbing_k[i];
        }
    }
    return indices;
}

/*
 * Returns the widths of the layers of the stack.

        :return: A list with the widths each layer, including the semi-infinite front
        and back layers and, optionally, the back absorbing layer used to suppress
        back surface reflection, defined as 1 mm thick.
 */
// Template argument for template type parameter must be a type; omitted 'typename' is a Microsoft extension
template<FloatingList T>
template<FloatingList U>
requires std::same_as<typename U::value_type, typename T::value_type>
U OpticStack<T>::get_widths() {
    std::size_t sz_struct = structure.size();
    if (no_back_reflection) {
        // std::valarray and std::vector have different constructors for (val, count)
        U widths(sz_struct + 3);
        widths[0] = INFINITY;
        for (std::size_t i = 1; i <= sz_struct; i++) {
            widths[i] = structure.at(i).second;
        }
        widths[sz_struct + 1] = 1e-3;
        widths[sz_struct + 2] = INFINITY;
        return widths;
    } else {
        U widths(sz_struct + 2);
        widths[0] = INFINITY;
        for (std::size_t i = 1; i <= sz_struct; i++) {
            widths[i] = structure.at(i).second;
        }
        widths[sz_struct + 1] = INFINITY;
        return widths;
    }
}

/*
 * k value of the back highly absorbing layer. It is the maximum between the
        bottom layer of the stack or a finite, small value that will absorb all light
        within the absorbing layer thickness.

        :param wl: Wavelength of the light in m.
        :return: The k value at each wavelength.
 */
template<FloatingList T>
T OpticStack<T>::k_absorbing(T &&wavelength) {
    T k_absorbing(wavelength.size());
    std::ranges::transform(wavelength, structure.back().first->kData(), std::begin(k_absorbing), [](T::value_type wl, T::value_type k) -> T::value_type {
        return std::max(k * 1e3, k);
    });
    return k_absorbing;
}

template class OpticStack<QList<double>>;
template std::valarray<std::complex<double>> OpticStack<QList<double>>::get_indices(QList<double> &&wavelength);
template std::vector<std::valarray<std::complex<double>>> OpticStack<QList<double>>::get_indices(QList<double> &&wavelength);
template std::valarray<double> OpticStack<QList<double>>::get_widths();
template std::vector<double> OpticStack<QList<double>>::get_widths();

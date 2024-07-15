//
// Created by Yihua Liu on 2024-7-4.
//

#include "OpticStack.h"

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
            widths[i] = structure.at(i - 1).second;
        }
        widths[sz_struct + 1] = 1e-3;
        widths[sz_struct + 2] = INFINITY;
        return widths;
    } else {
        U widths(sz_struct + 2);
        widths[0] = INFINITY;
        for (std::size_t i = 1; i <= sz_struct; i++) {
            widths[i] = structure.at(i - 1).second;
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
template std::valarray<double> OpticStack<QList<double>>::get_widths();
template std::vector<double> OpticStack<QList<double>>::get_widths();

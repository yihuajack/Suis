//
// Created by Yihua Liu on 2024/3/31.
//

#ifndef SUISAPP_OPTIC_MATERIAL_H
#define SUISAPP_OPTIC_MATERIAL_H

#include <concepts>
#include <QList>
#include <QString>

#include "Global.h"
#include "utils/math.h"

template<typename T1, typename T2>
concept Pair = requires(T1 a) {
    { a.first };
    { a.second } -> std::convertible_to<T2>;
};

// Compound requirements uses return-type-requirement -> type-constraint
template<typename T1, typename T2>
concept Vector = requires(T1 a) {
    { Pair<decltype(a.front()), T2> };
    { Pair<decltype(a.back()), T2> };
};

// It seems that there is no need to make it a QObject
// See https://doc.qt.io/qt-6/qtquick-modelviewsdata-cppmodels.html
template<FloatingList T>
class OpticMaterial {
public:
    // It seems that it is hard to put the definition of the constructor in the source file,
    // different from AbsorpAnalyticVecFn<T>::scale()
    template<typename U, typename V>
    requires Vector<V, T> and Vector<U, T>
    OpticMaterial(QString mat_name, U&& n_wl, V&& n_data, U&& k_wl, V&& k_data) : mat_name(std::move(mat_name)),
                                                                                  n_wl(std::forward<U>(n_wl)),
                                                                                  n_data(std::forward<V>(n_data)),
                                                                                  k_wl(std::forward<U>(k_wl)),
                                                                                  k_data(std::forward<V>(k_data)) {}

    [[nodiscard]] QString name() const;
    [[nodiscard]] T nWl() const;
    [[nodiscard]] T nData() const;
    [[nodiscard]] T kWl() const;
    [[nodiscard]] T kData() const;

    // The original Python implementation does really late evaluations. When executing calculate_rat, it evaluates
    // the get_indices() function, which evaluates the interpolation methods depending on wavelengths n_interpolated
    // and k_interpolated of the material class. In the interpolation methods, it loads n_data (a vstack of wl and n)
    // and k_data (a vstack of wl and k) from the TXT files and then does interpolation. In our implementation, we
    // advance file reading before the constructor of the material class, but still do not evaluate the interpolation
    // until it is needed.
    template<FloatingList U>
    T n_interpolated(U &&x) {
        return Utils::Math::interp1_linear(n_wl.back().second, n_data.back().second, std::forward<U>(x));
    }

    template<FloatingList U>
    T k_interpolated(U &&x) {
        return Utils::Math::interp1_linear(k_wl.back().second, k_data.back().second, std::forward<U>(x));
    }

private:
    QString mat_name;
    // No matter using the raw data or the interpolated data, we have to store the raw data.
    QList<std::pair<double, T>> n_wl;
    QList<std::pair<double, T>> n_data;
    QList<std::pair<double, T>> k_wl;
    QList<std::pair<double, T>> k_data;
};

#endif  // SUISAPP_OPTIC_MATERIAL_H

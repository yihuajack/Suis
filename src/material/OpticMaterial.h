//
// Created by Yihua Liu on 2024/3/31.
//

#ifndef SUISAPP_OPTIC_MATERIAL_H
#define SUISAPP_OPTIC_MATERIAL_H

#include <QDebug>
#include <QList>
#include <QString>

#include "Global.h"
#include "utils/Math.h"

enum class DbType {
    UNDEFINED,
    SOLCORE,
    SOPRA,
    DF,
    GCL
};

template<typename T1, typename T2>
concept Pair = requires(T1 a) {
    { a.first };
    { a.second } -> std::convertible_to<T2>;
};

// Compound requirements uses return-type-requirement -> type-constraint
template<typename T1, typename T2>
concept PairVector = requires(T1 a) {
    { Pair<decltype(a.front()), T2> };
    { Pair<decltype(a.back()), T2> };
};

// It seems that there is no need to make it a QObject
// See https://doc.qt.io/qt-6/qtquick-modelviewsdata-cppmodels.html
template<FloatingList T>
class OpticMaterial {
public:
    OpticMaterial(QString mat_name, const DbType db_type, QString path) : mat_name(std::move(mat_name)),
                                                                          db_type(db_type),
                                                                          path(std::move(path)) {}

    [[nodiscard]] QString name() const;
    [[nodiscard]] T wl() const;
    [[nodiscard]] T nData() const;
    [[nodiscard]] T kData() const;

    // The original Python implementation does really late evaluations. When executing calculate_rat, it evaluates
    // the get_indices() function, which evaluates the interpolation methods depending on wavelengths n_interpolated
    // and k_interpolated of the material class. In the interpolation methods, it loads n_data (a vstack of wl and n)
    // and k_data (a vstack of wl and k) from the TXT files and then does interpolation.
    void load_nk();

    template<FloatingList U>
    T n_interpolated(U &&x) {
        if (wavelengths.empty() or n_data.empty()) {
            try {
                load_nk();
            } catch (std::runtime_error& e) {
                qWarning() << "Material" << mat_name << "does not have n-data defined. Returning \"ones\": " << e.what();
                T ret(x.size(), 1);
                return ret;
            }
        }
        return Utils::Math::interp1_linear(wavelengths.back().second, n_data.back().second, std::forward<U>(x));
    }

    template<FloatingList U>
    T k_interpolated(U &&x) {
        if (wavelengths.empty() or k_data.empty()) {
            try {
                load_nk();
            } catch (std::runtime_error& e) {
                qWarning() << "Material" << mat_name << "does not have k-data defined. Returning \"zeros\": " << e.what();
                T ret(x.size(), 0);
                return ret;
            }
        }
        return Utils::Math::interp1_linear(wavelengths.back().second, k_data.back().second, std::forward<U>(x));
    }

private:
    QString mat_name;
    DbType db_type;
    QString path;
    // Design tradeoff: one-time file I/O and no searching time cost but higher memory space cost
    // Alternative design: lazy loading n/k data when interpolation needed
    // No matter using the raw data or the interpolated data, we have to store the raw data.
    QList<std::pair<double, T>> wavelengths;
    QList<std::pair<double, T>> n_data;
    QList<std::pair<double, T>> k_data;
};

#endif  // SUISAPP_OPTIC_MATERIAL_H

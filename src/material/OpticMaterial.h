//
// Created by Yihua Liu on 2024/3/31.
//

#ifndef SUISAPP_OPTIC_MATERIAL_H
#define SUISAPP_OPTIC_MATERIAL_H

#include <QList>
#include <QString>

// It seems that there is no need to make it a QObject
// See https://doc.qt.io/qt-6/qtquick-modelviewsdata-cppmodels.html
class OpticMaterial {
    // Q_PROPERTY(QString mat_name READ matName WRITE setMatName NOTIFY matNameChanged)
    // Q_PROPERTY(QList<double> n_wl READ nWl NOTIFY nWlChanged)  // last list (fraction = 1)

public:
    OpticMaterial(QString mat_name, QList<std::pair<double, QList<double>>>& n_wl,
                  QList<std::pair<double, QList<double>>>& n_data,
                  QList<std::pair<double, QList<double>>>& k_wl,
                  QList<std::pair<double, QList<double>>>& k_data);

    [[nodiscard]] QString name() const;

    [[nodiscard]] QList<double> nWl() const;
    [[nodiscard]] QList<double> nData() const;

private:
    QString mat_name;
    // No matter using the raw data or the interpolated data, we have to store the raw data.
    QList<std::pair<double, QList<double>>> n_wl;
    QList<std::pair<double, QList<double>>> n_data;
    QList<std::pair<double, QList<double>>> k_wl;
    QList<std::pair<double, QList<double>>> k_data;
};


#endif  // SUISAPP_OPTIC_MATERIAL_H

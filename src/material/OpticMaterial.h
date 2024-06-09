//
// Created by Yihua Liu on 2024/3/31.
//

#ifndef SUISAPP_OPTIC_MATERIAL_H
#define SUISAPP_OPTIC_MATERIAL_H

#include <vector>
#include <QFileInfo>
#include <QString>

enum class DbType {
    DF, SOLCORE
};

class OpticMaterial {
public:
    OpticMaterial(const DbType& db_type, QString mat_name, const QFileInfo& mat_f);

private:
    QString mat_name;
    std::vector<double> n_wl;
    std::vector<double> n_data;
    std::vector<double> k_wl;
    std::vector<double> k_data;

    void readMat(const DbType& db_type, const QFileInfo& mat_f);
};


#endif  // SUISAPP_OPTIC_MATERIAL_H

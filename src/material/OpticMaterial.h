//
// Created by Yihua Liu on 2024/3/31.
//

#ifndef SUISAPP_OPTIC_MATERIAL_H
#define SUISAPP_OPTIC_MATERIAL_H

#include <vector>
#include <QString>

enum class DbType {
    SOLCORE_DB
};

class OpticMaterial {
public:
    OpticMaterial(DbType db_type, QString mat_name, const QString& mat_path);

private:
    QString mat_name;
    std::vector<double> n_wl;
    std::vector<double> n_data;
    std::vector<double> k_wl;
    std::vector<double> k_data;

    void readMat(const QString& path);
};


#endif  // SUISAPP_OPTIC_MATERIAL_H

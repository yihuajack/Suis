//
// Created by Yihua Liu on 2024/3/31.
//

#ifndef SUISAPP_OPTIC_MATERIAL_H
#define SUISAPP_OPTIC_MATERIAL_H

#include <vector>
#include <QString>

class OpticMaterial {
public:
    OpticMaterial(QString mat_name, std::vector<double>& n_wl, std::vector<double>& n_vec,
                  std::vector<double>& k_wl, std::vector<double>& k_vec);

private:
    QString mat_name;
    std::vector<double> n_wl;
    std::vector<double> n_data;
    std::vector<double> k_wl;
    std::vector<double> k_data;
};


#endif  // SUISAPP_OPTIC_MATERIAL_H

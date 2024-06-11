//
// Created by Yihua Liu on 2024-06-11.
//

#ifndef SUISAPP_COMPOPTICMATERIAL_H
#define SUISAPP_COMPOPTICMATERIAL_H

#include <vector>
#include <QString>

class CompOpticMaterial {
public:
    CompOpticMaterial(QString mat_name, std::vector<std::pair<double, std::vector<double>>>& n_wl,
                      std::vector<std::pair<double, std::vector<double>>>& n_data,
                      std::vector<std::pair<double, std::vector<double>>>& k_wl,
                      std::vector<std::pair<double, std::vector<double>>>& k_data);

private:
    QString mat_name;
    std::vector<std::pair<double, std::vector<double>>> n_wl;
    std::vector<std::pair<double, std::vector<double>>> n_data;
    std::vector<std::pair<double, std::vector<double>>> k_wl;
    std::vector<std::pair<double, std::vector<double>>> k_data;
};


#endif  // SUISAPP_COMPOPTICMATERIAL_H

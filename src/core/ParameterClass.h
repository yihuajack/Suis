//
// Created by Yihua Liu on 2024-7-15.
//

#ifndef SUISAPP_PARAMETERCLASS_H
#define SUISAPP_PARAMETERCLASS_H


class ParameterClass {
public:
    QList<QString> layer_type;
    QList<QString> material;
    QList<double> d;
    QList<qsizetype> layer_points;
    QList<double> xmesh_coeff;
    QList<double> Phi_EA;
    bool side{};
};


#endif  // SUISAPP_PARAMETERCLASS_H

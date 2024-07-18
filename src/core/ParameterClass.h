//
// Created by Yihua Liu on 2024-7-15.
//

#ifndef SUISAPP_PARAMETERCLASS_H
#define SUISAPP_PARAMETERCLASS_H

#include <utility>

template<template <typename...> class L, typename T, typename STR_T, typename VAR_T>
struct ParameterClass {
public:
    static const L<T>::size_type size = 25;  // Only for get()

    L<STR_T> layer_type;
    L<STR_T> material;
    L<T> d;
    L<typename L<T>::size_type> layer_points;
    L<T> xmesh_coeff;
    L<T> Phi_EA;
    L<T> Phi_IP;
    L<T> Et;
    L<T> EF0;
    T Phi_left{};
    T Phi_right{};
    L<T> Nc;
    L<T> Nv;
    L<T> Nani;
    L<T> Ncat;
    L<T> a_max;
    L<T> c_max;
    L<T> mu_n;
    L<T> mu_p;
    L<T> mu_a;
    L<T> mu_c;
    L<T> epp;
    L<T> g0;
    L<T> B;
    L<T> taun;
    L<T> taup;
    L<T> sn;
    T sn_l{};
    T sn_r{};
    L<T> sp;
    T sp_l{};
    T sp_r{};
    bool optical_model{};  // Should be better using enums
    bool side{};
    STR_T xmesh_type;

    L<STR_T> headers = {
            "layer_type",
            "material",
            "d",
            "layer_points",
            "xmesh_coeff",
            "Phi_EA",
            "Phi_IP",
            "Et",
            "EF0",
            "Nc",
            "Nv",
            "Nani",
            "Ncat",
            "a_max",
            "c_max",
            "mu_n",
            "mu_p",
            "mu_a",
            "mu_c",
            "epp",
            "g0",
            "B",
            "taun",
            "taup",
            "sn",
            "sp",
    };

    [[nodiscard]] VAR_T get(const int index) const {  // maybe there are better ways to do this...
        switch (index) {
            case 0:
                return layer_type;
            case 1:
                return material;
            case 2:
                return VAR_T::fromValue(d);
            case 3:
                return VAR_T::fromValue(layer_points);
            case 4:
                return VAR_T::fromValue(xmesh_coeff);
            case 5:
                return VAR_T::fromValue(Phi_EA);
            case 6:
                return VAR_T::fromValue(Phi_IP);
            case 7:
                return VAR_T::fromValue(Et);
            case 8:
                return VAR_T::fromValue(EF0);
            case 9:
                return VAR_T::fromValue(Nc);
            case 10:
                return VAR_T::fromValue(Nv);
            case 11:
                return VAR_T::fromValue(Nani);
            case 12:
                return VAR_T::fromValue(Ncat);
            case 13:
                return VAR_T::fromValue(a_max);
            case 14:
                return VAR_T::fromValue(c_max);
            case 15:
                return VAR_T::fromValue(mu_n);
            case 16:
                return VAR_T::fromValue(mu_p);
            case 17:
                return VAR_T::fromValue(mu_a);
            case 18:
                return VAR_T::fromValue(mu_c);
            case 19:
                return VAR_T::fromValue(epp);
            case 20:
                return VAR_T::fromValue(g0);
            case 21:
                return VAR_T::fromValue(B);
            case 22:
                return VAR_T::fromValue(taun);
            case 23:
                return VAR_T::fromValue(taup);
            case 24:
                return VAR_T::fromValue(sn);
            case 25:
                return VAR_T::fromValue(sp);
            default:
                return {};
        }
    };

    void set(const VAR_T &cell, const int row, const int col) {
        switch (row) {
            case 0:
                layer_type[col] = cell.toString();
                return;  // or break;
            case 1:
                material[col] = cell.toString();
                return;
            case 2:
                d[col] = cell.toDouble();
                return;
            case 3:
                layer_points[col] = cell.toLongLong();
                return;
            case 4:
                xmesh_coeff[col] = cell.toDouble();
                return;
            case 5:
                Phi_EA[col] = cell.toDouble();
                return;
            case 6:
                Phi_IP[col] = cell.toDouble();
                return;
            case 7:
                Et[col] = cell.toDouble();
                return;
            case 8:
                EF0[col] = cell.toDouble();
                return;
            case 9:
                Nc[col] = cell.toDouble();
                return;
            case 10:
                Nv[col] = cell.toDouble();
                return;
            case 11:
                Nani[col] = cell.toDouble();
                return;
            case 12:
                Ncat[col] = cell.toDouble();
                return;
            case 13:
                a_max[col] = cell.toDouble();
                return;
            case 14:
                c_max[col] = cell.toDouble();
                return;
            case 15:
                mu_n[col] = cell.toDouble();
                return;
            case 16:
                mu_p[col] = cell.toDouble();
                return;
            case 17:
                mu_a[col] = cell.toDouble();
                return;
            case 18:
                mu_c[col] = cell.toDouble();
                return;
            case 19:
                epp[col] = cell.toDouble();
                return;
            case 20:
                g0[col] = cell.toDouble();
                return;
            case 21:
                B[col] = cell.toDouble();
                return;
            case 22:
                taun[col] = cell.toDouble();
                return;
            case 23:
                taup[col] = cell.toDouble();
                return;
            case 24:
                sn[col] = cell.toDouble();
                return;
            case 25:
                sp[col] = cell.toDouble();
                return;
            default:
                return;
        }
    };
};

#endif  // SUISAPP_PARAMETERCLASS_H

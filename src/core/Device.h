//
// Created by Yihua Liu on 2024/08/11.
//

#ifndef SUISAPP_DEVICE_H
#define SUISAPP_DEVICE_H

#include "Global.h"

template <Vector V>
struct Device {
    V mu_c;
    V mu_a;
    V sn;
    V sp;
    V mu_n;
    V mu_p;
    V Phi_EA;
    V Phi_IP;
    V EF0;
    V EF0_zerointerface;
    V Nc;
    V Nv;
    V n0;
    V p0;
    V Nani;
    V Ncat;
    V a_max;
    V c_max;
    V g0;
    V B;
    V NA;
    V ND;
    V gradEA;
    V gradIP;
    V gradNc;
    V gradNv;
    V taun_vsr;
    V taup_vsr;
    V alpha0;
    V beta0;
    V alpha0_xn;
    V beta0_xp;
    V dint;
    V int_switch;
    V bulk_switch;
    V vsr_zone;
    V srh_zone;
    V Field_switch;
    V taun;
    V taup;
    V epp;
    V ni;
    V nt;
    V pt;
    V xprime;
    V xprime_n;
    V xprime_p;
    V sign_xn;
    V sign_xp;
};

#endif  // SUISAPP_DEVICE_H

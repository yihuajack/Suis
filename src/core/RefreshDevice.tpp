//
// Created by Yihua Liu on 2025-07-02.
//

#ifndef SUISAPP_REFRESHDEVICE_TPP
#define SUISAPP_REFRESHDEVICE_TPP

#ifndef SUISAPP_PARAMETERCLASS_H
#error __FILE__ should only be included from ParameterClass.h.
#endif // SUISAPP_PARAMETERCLASS_H

// Rebuilds important device properties
template<template <typename...> class L, typename F_T, typename STR_T>
void ParameterClass<L, F_T, STR_T>::refresh_device() {
    xx = meshgen_x();
    x_sub = getvar_sub(xx);
}

#endif  // SUISAPP_REFRESHDEVICE_TPP

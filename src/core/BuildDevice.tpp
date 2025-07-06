//
// Created by Yihua Liu on 2024/08/09.
//

// https://stackoverflow.com/questions/54362798/template-tpp-file-include-guards
#ifndef SUISAPP_BUILDDEVICE_TPP
#define SUISAPP_BUILDDEVICE_TPP

#ifndef SUISAPP_PARAMETERCLASS_H
#error __FILE__ should only be included from ParameterClass.h.
#endif // SUISAPP_PARAMETERCLASS_H

#include "Device.h"

template<template <typename...> class L, typename F_T, typename STR_T>
Device<L<F_T>> ParameterClass<L, F_T, STR_T>::build_device(const bool meshoption) {  // 1 for 'whole', 0 for 'sub'
    L<F_T> xmesh = meshoption ? xx : x_sub;
    return xmesh;
}

#endif  // SUISAPP_BUILDDEVICE_TPP

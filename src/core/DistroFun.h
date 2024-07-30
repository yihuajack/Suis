//
// Created by Yihua Liu on 2024-7-21.
//

#ifndef SUISAPP_DISTROFUN_H
#define SUISAPP_DISTROFUN_H

#include <numbers>
#include <boost/math/quadrature/gauss.hpp>

enum class PROB_DISTRO {
    FERMI, BLAKEMORE, BOLTZMANN
};

template<template <typename...> class L, typename F_T, typename STR_T>
class ParameterClass;  // forward declaration

// Calculates carrier densities for different distribution functions
template<template <typename...> class L, typename T, typename STR_T>
class DistroFun {
    using SZ_T = L<T>::size_type;
public:
    static constexpr SZ_T uplimit = 10;  // Upper limit for the integration [eV]

    static L<T> nfun(const L<T> &Nc, const L<T> &Ec, const L<T> &Efn, const ParameterClass<L, T, STR_T> &par) {
        const T kT = ParameterClass<L, T, STR_T>::kB * par.T;
        L<T> n(Nc.size());
        switch (par.prob_distro_function) {
            case PROB_DISTRO::FERMI: {
                // Fermi dirac integral for obtaining electron densities
                // Nc = conduction band density of states
                // Ec = conduction band energy
                // Ef = Fermi level
                // T = temperature
                // See Schubert 2015, pp. 130
                for (SZ_T i = 0; i < Nc.size(); i++) {
                    auto fn = [&Efn, &Ec, i](T E) -> T {
                        return std::sqrt(E / kT) / (1 + std::exp((E - Efn.at(i) + Ec.at(i)) / kT));;
                    };
                    if (not std::isnan(Nc.at(i))) {  // ignores interfaces
                        n[i] = std::real((2 * Nc.at(i) / (kT * std::sqrt(std::numbers::pi_v<T>))) * boost::math::quadrature::gauss<T, 15>::integrate(fn, 0, uplimit));
                    }
                }
                return n;
            }
            case PROB_DISTRO::BLAKEMORE: {
                for (SZ_T i = 0; i < Nc.size(); i++) {
                    T eta_n = (Efn.at(i) - Ec.at(i)) / kT;
                    n[i] = Nc.at(i) * (1 / std::exp(-eta_n) + par.gamma);
                }
                return n;
            }
            case PROB_DISTRO::BOLTZMANN: {
                for (SZ_T i = 0; i < Nc.size(); i++) {
                    n[i] = Nc.at(i) * std::exp((Efn.at(i) - Ec.at(i)) / kT);
                }
                return n;
            }
            default:
                return n;
        }
    }

    static L<T> pfun(const L<T> &Nv, const L<T> &Ev, const L<T> &Efp,
                     const ParameterClass<L, T, STR_T> &par) {
        const T kT = ParameterClass<L, T, STR_T>::kB * par.T;
        L<T> p(Nv.size());
        switch (par.prob_distro_function) {
            case PROB_DISTRO::FERMI: {
                // Fermi dirac integral for obtaining electron densities
                // Nc = conduction band density of states
                // Ec = conduction band energy
                // Ef = Fermi level
                // T = temperature
                // See Schubert 2015, pp. 130
                // Reflecting the energy makes the integral easier for some
                // reason - doesn't seem to like integrating from negative
                // infinity...
                for (SZ_T i = 0; i < Nv.size(); i++) {
                    auto fp = [&Efp, &Ev, i](T E) -> T {
                        return std::sqrt(E / kT) / (1 + std::exp((E + Efp.at(i) - Ev.at(i)) / kT));;
                    };
                    if (not std::isnan(Nv.at(i))) {  // ignores interfaces
                        p[i] = std::real((2 * Nv.at(i) / (kT * std::sqrt(std::numbers::pi_v<T>))) * boost::math::quadrature::gauss<T, 15>::integrate(fp, 0, uplimit));
                    }
                }
                return p;
            }
            case PROB_DISTRO::BLAKEMORE: {
                for (SZ_T i = 0; i < Nv.size(); i++) {
                    T eta_p = (Ev.at(i) - Efp.at(i)) / kT;
                    p[i] = Nv.at(i) * (1 / std::exp(-eta_p) + par.gamma);
                }
                return p;
            }
            case PROB_DISTRO::BOLTZMANN: {
                for (SZ_T i = 0; i < Nv.size(); i++) {
                    p[i] = Nv.at(i) * std::exp((Ev.at(i) - Efp.at(i)) / kT);
                }
                return p;
            }
            default:
                return p;
        }
    }
};


#endif //SUISAPP_DISTROFUN_H

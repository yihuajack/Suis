//
// Created by Yihua Liu on 2024-7-21.
//

#ifndef SUISAPP_DISTFUN_H
#define SUISAPP_DISTFUN_H

#include <numbers>
#include <boost/math/quadrature/gauss.hpp>

enum class PROB_DIST {
    FERMI, BLAKEMORE, BOLTZMANN
};

template<template <typename...> class L, typename F_T, typename STR_T>
class ParameterClass;  // forward declaration

// Calculates carrier densities for different distribution functions
template<template <typename...> class L, typename T, typename STR_T>
class DistFun {
    using SZ_T = L<T>::size_type;
public:
    static constexpr SZ_T uplimit = 10;  // Upper limit for the integration [eV]

    static L<T> nfun(const L<T> &Nc, const L<T> &Ec, const L<T> &Efn, const PROB_DIST prob_dist_function,
                     const T temperature, const T gamma) {
        const T kT = ParameterClass<L, T, STR_T>::kB * temperature;
        L<T> n(Nc.size());
        switch (prob_dist_function) {
            case PROB_DIST::FERMI: {
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
                        n[i] = std::real(2 * Nc.at(i) / (kT * std::sqrt(std::numbers::pi_v<T>)) * boost::math::quadrature::gauss<T, 15>::integrate(fn, 0, uplimit));
                    }
                }
                return n;
            }
            case PROB_DIST::BLAKEMORE: {
                for (SZ_T i = 0; i < Nc.size(); i++) {
                    T eta_n = (Efn.at(i) - Ec.at(i)) / kT;
                    n[i] = Nc.at(i) / (std::exp(-eta_n) + gamma);
                }
                return n;
            }
            case PROB_DIST::BOLTZMANN: {
                for (SZ_T i = 0; i < Nc.size(); i++) {
                    n[i] = Nc.at(i) * std::exp((Efn.at(i) - Ec.at(i)) / kT);
                }
                return n;
            }
            default:
                throw std::runtime_error("Invalid probability distribution function");
        }
    }

    static T nfun(const T &Nc, const T &Ec, const T &Efn, const PROB_DIST prob_dist_function,
                     const T temperature, const T gamma) {
        const T kT = ParameterClass<L, T, STR_T>::kB * temperature;
        switch (prob_dist_function) {
            case PROB_DIST::FERMI: {
                auto fn = [&Efn, &Ec](T E) -> T {
                    return std::sqrt(E / kT) / (1 + std::exp((E - Efn + Ec) / kT));;
                };
                return std::real(2 * Nc / (kT * std::sqrt(std::numbers::pi_v<T>)) * boost::math::quadrature::gauss<T, 15>::integrate(fn, 0, uplimit));
            }
            case PROB_DIST::BLAKEMORE: {
                T eta_n = (Efn - Ec) / kT;
                return Nc / (std::exp(-eta_n) + gamma);
            }
            case PROB_DIST::BOLTZMANN: {
                return Nc * std::exp((Efn - Ec) / kT);
            }
            default:
                throw std::runtime_error("Invalid probability distribution function");
        }
    }

    static L<T> pfun(const L<T> &Nv, const L<T> &Ev, const L<T> &Efp, const PROB_DIST prob_dist_function,
                     const T temperature, const T gamma) {
        const T kT = ParameterClass<L, T, STR_T>::kB * temperature;
        L<T> p(Nv.size());
        switch (prob_dist_function) {
            case PROB_DIST::FERMI: {
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
            case PROB_DIST::BLAKEMORE: {
                for (SZ_T i = 0; i < Nv.size(); i++) {
                    T eta_p = (Ev.at(i) - Efp.at(i)) / kT;
                    p[i] = Nv.at(i)  / (std::exp(-eta_p) + gamma);
                }
                return p;
            }
            case PROB_DIST::BOLTZMANN: {
                for (SZ_T i = 0; i < Nv.size(); i++) {
                    p[i] = Nv.at(i) * std::exp((Ev.at(i) - Efp.at(i)) / kT);
                }
                return p;
            }
            default:
                return p;
        }
    }

    static T pfun(const T &Nv, const T &Ev, const T &Efp, const PROB_DIST prob_dist_function,
                  const T temperature, const T gamma) {
        const T kT = ParameterClass<L, T, STR_T>::kB * temperature;
        switch (prob_dist_function) {
            case PROB_DIST::FERMI: {
                auto fp = [&Efp, &Ev](T E) -> T {
                    return std::sqrt(E / kT) / (1 + std::exp((E + Efp - Ev) / kT));;
                };
                return std::real(2 * Nv / (kT * std::sqrt(std::numbers::pi_v<T>)) * boost::math::quadrature::gauss<T, 15>::integrate(fp, 0, uplimit));
            }
            case PROB_DIST::BLAKEMORE: {
                T eta_p = (Ev - Efp) / kT;
                return Nv / (std::exp(-eta_p) + gamma);
            }
            case PROB_DIST::BOLTZMANN: {
                return Nv * std::exp((Ev - Efp) / kT);
            }
            default:
                throw std::runtime_error("Invalid probability distribution function");
        }
    }
};


#endif //SUISAPP_DISTFUN_H

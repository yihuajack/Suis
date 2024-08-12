//
// Created by Yihua Liu on 2024-7-15.
//

#ifndef SUISAPP_PARAMETERCLASS_H
#define SUISAPP_PARAMETERCLASS_H

#include <iostream>
#include <print>
#include <ranges>
#include <set>
#include <utility>

#include "Device.h"
#include "DistFun.h"
#include "GetVarSub.h"
#include "utils/math.h"

enum class SpatialCoordinate {
    CARTESIAN, CYLINDRICAL_POLAR, SPHERICAL_POLAR
};

enum class TMESH_TYPE {
    LINEAR, LOG10, LOG10_F_T
};

enum class FUN_TYPE {
    CONSTANT, SWEEP_AND_STILL, SIN, SWEEP
};

template<template <typename...> class L, typename F_T, typename STR_T>
class ParameterClass {
    using SZ_T = L<F_T>::size_type;
    // using CHAR_T = STR_T::value_type;
public:
    ParameterClass(const L<L<STR_T>> &csv_data, const std::map<STR_T, typename L<F_T>::size_type> &properties) {
        import_properties(csv_data, properties);

        const SZ_T c_sz = col_size();
        const L<F_T> N_D = ND();
        const L<F_T> N_A = NA();
        for (SZ_T i = 0; i < c_sz; i++) {
            // Warn if doping density exceeds eDOS
            if (N_D.at(i) > Nc.at(i) or N_A.at(i) > Nc.at(i)) {
                throw std::runtime_error("Doping density must be less than eDOS. For consistent values ensure "
                                         "electrode work functions are within the band gap and check expressions for "
                                         "doping density in Dependent variables.");
            }
            // Warn if trap energies are outside of band gap energies
            if (Et.at(i) >= Phi_EA.at(i) or Et.at(i) <= Phi_IP.at(i)) {
                throw std::runtime_error("Trap energies must exist within layer band gap.");
            }
            if (a_max.at(i) <= 0) {
                throw std::runtime_error("Maximum anion density (a_max) cannot have zero or negative entries - "
                                         "choose a low value rather than zero e.g. 1");
            }
            // Warn if c_max is set to zero in any layers - leads to infinite diffusion rate
            if (c_max.at(i) <= 0) {
                throw std::runtime_error("Maximum cation density (c_max) cannot have zero or negative entries "
                                         "- choose a low value rather than zero e.g. 1");
            }
            // Warn if electrode work functions are outside of boundary layer bandgap
            if (Phi_left < Phi_IP.front() or Phi_left > Phi_EA.front()) {
                throw std::out_of_range("Left-hand work function (Phi_left) out of range: value must exist "
                                        "within left-hand layer band gap");
            }
            if (Phi_right < Phi_IP.back() or Phi_right > Phi_EA.back()) {
                throw std::out_of_range("Right-hand work function (Phi_right) out of range: value must exist "
                                        "within right-hand layer band gap");
            }
        }
        // Warn if property array do not have the correct number of layers.
        // The layer thickness array is used to define the number of layers
        if (Phi_EA.size() not_eq c_sz) {
            throw std::length_error("Electron Affinity array (Phi_EA) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the "
                                    "thickness array (d), except SRH properties for interfaces which should have "
                                    "length(d)-1 elements.");
        } else if (Phi_IP.size() not_eq c_sz) {
            throw std::length_error("Ionization Potential array (Phi_IP) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the "
                                    "thickness array (d), except SRH properties for interfaces which should have "
                                    "length(d)-1 elements.");
        } else if (mu_n.size() not_eq c_sz) {
            throw std::length_error("Electron mobility array (mu_n) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the "
                                    "thickness array (d), except SRH properties for interfaces which should have "
                                    "length(d)-1 elements.");
        } else if (mu_p.size() not_eq c_sz) {
            throw std::length_error("Hole mobility array (mu_p) does not have the correct number of elements. "
                                    "Property arrays must have the same number of elements as the thickness array (d), "
                                    "except SRH properties for interfaces which should have length(d)-1 elements.");
        } else if (mu_a.size() not_eq c_sz) {
            throw std::length_error("Anion mobility array (mu_a) does not have the correct number of elements. "
                                    "Property arrays must have the same number of elements as the thickness array (d), "
                                    "except SRH properties for interfaces which should have length(d)-1 elements.");
        } else if (mu_c.size() not_eq c_sz) {
            throw std::length_error("Cation mobility array (mu_c) does not have the correct number of elements."
                                    " Property arrays must have the same number of elements as the thickness array (d),"
                                    " except SRH properties for interfaces which should have length(d)-1 elements.");
        } else if (N_A.size() not_eq c_sz) {
            throw std::length_error("Acceptor density array (NA) does not have the correct number of elements. "
                                    "Property arrays must have the same number of elements as the thickness array (d), "
                                    "except SRH properties for interfaces which should have length(d)-1 elements.");
        } else if (N_D.size() not_eq c_sz) {
            throw std::length_error("Donor density array (ND) does not have the correct number of elements. "
                                    "Property arrays must have the same number of elements as the thickness array (d), "
                                    "except SRH properties for interfaces which should have length(d)-1 elements.");
        } else if (Nc.size() not_eq c_sz) {
            throw std::length_error("Effective density of states array (Nc) does not have the correct number of"
                                    " elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        } else if (Nv.size() not_eq c_sz) {
            throw std::length_error("Effective density of states array (Nv) does not have the correct number of"
                                    " elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        } else if (Nani.size() not_eq c_sz) {
            throw std::length_error("Background anion density (Nani) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        } else if (Ncat.size() not_eq c_sz) {
            throw std::length_error("Background cation density (Ncat) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        } else if (a_max.size() not_eq c_sz) {
            throw std::length_error("Ion density of states array (a_max) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        } else if (c_max.size() not_eq c_sz) {
            throw std::length_error("Ion density of states array (c_max) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        } else if (epp.size() not_eq c_sz) {
            throw std::length_error("Relative dielectric constant array (epp) does not have the correct number "
                                    "of elements. Property arrays must have the same number of elements as the "
                                    "thickness array (d), except SRH properties for interfaces which should have "
                                    "length(d)-1 elements.");
        } else if (B.size() not_eq c_sz) {
            throw std::length_error("Radiative recombination coefficient array (B) does not have the correct "
                                    "number of elements. Property arrays must have the same number of elements as the "
                                    "thickness array (d), except SRH properties for interfaces which should have "
                                    "length(d)-1 elements.");
        } else if (EF0.size() not_eq c_sz) {
            throw std::length_error("Equilibrium Fermi level array (EF0) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        } else if (g0.size() not_eq c_sz) {
            throw std::length_error("Uniform generation array (g0) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        } else if (taun.size() not_eq c_sz) {
            throw std::length_error("Bulk SRH electron time constants array (taun_bulk) does not have the "
                                    "correct number of elements. Property arrays must have the same number of elements "
                                    "as the thickness array (d), except SRH properties for interfaces which should have"
                                    " length(d)-1 elements.");
        } else if (taup.size() not_eq c_sz) {
            throw std::length_error("Bulk SRH hole time constants array (taup_bulk) does not have the correct "
                                    "number of elements. Property arrays must have the same number of elements as the "
                                    "thickness array (d), except SRH properties for interfaces which should have "
                                    "length(d)-1 elements.");
        } else if (Et.size() not_eq c_sz) {
            throw std::length_error("Bulk SRH trap energy array (Et) does not have the correct number of "
                                    "elements. Property arrays must have the same number of elements as the thickness "
                                    "array (d), except SRH properties for interfaces which should have length(d)-1 "
                                    "elements.");
        }
        // Device and generation builder
        // Import variables and structure, xx, gx1, gx2, and dev must be
        // refreshed when to rebuild the device for example when
        // changing device thickness on the fly. These are not present
        // in the dependent variables as it is too costly to have them
        // continuously called.
    }

    static constexpr F_T kB = 8.617330350e-5;  // Boltzmann constant [eV K^-1]
    static constexpr F_T epp0 = 552434;  // Epsilon_0 [e^2 eV^-1 cm^-1]
    static constexpr SZ_T q = 1;  // Charge of the species in units of e.
    static constexpr F_T e = 1.60217662e-19;  // Elementary charge in Coulombs.

    // Temperature [K]
    F_T T = 300;

    static constexpr SZ_T size = 25;  // Only for get(); error C2864

    // Spatial mesh
    // Device Dimensions [cm]
    // The spatial mesh is a linear piece-wise mesh and is built by the
    // MESHGEN_X function using 2 arrays DCELL and PCELL,
    // which define the thickness and number of points of each layer
    // respectively.
    L<F_T> d = {400e-7};  // Layer and subsection thickness array
    L<SZ_T> layer_points = {400};  // Points array

    // Layer description
    // Define the layer type for each of the layers in the device. The
    // options are:
    // LAYER = standard layer
    // ACTIVE = standard layer but the properties of this layer are
    // flagged such that they can easily be accessed
    // JUNCTION = a region with graded properties between two materials
    // (either LAYER or ACTIVE type)
    // with different properties
    L<STR_T> layer_type = {"active"};
    // STACK is used for reading the optical properties' library.
    // The names here do not influence the electrical properties of the
    // device. See INDEX OF REFRACTION LIBRARY for choices - names must be entered
    // exactly as given in the column headings with the '_n', '_k' omitted
    L<STR_T> material = {"MAPICl"};
    L<std::tuple<F_T, F_T, F_T>> layer_color = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    // Define spatial coordinate system - typically this will be kept at
    // 0 for most applications
    // m=0 cartesian
    // m=1 cylindrical polar coordinates
    // m=2 spherical polar coordinates
    SpatialCoordinate m = SpatialCoordinate::CARTESIAN;

    // Spatial mesh
    // xmesh_type specification - see MESHGEN_X.
    bool xmesh_type = true;
    L<F_T> xmesh_coeff = {0.7};
    // Time mesh
    // The time mesh is dynamically generated by ODE15s- the mesh
    // defined by MESHGEN_T only defines the values of the points that
    // are read out and so does not influence convergence. Defining an
    // unnecessarily high number of points however can be expensive owing
    // to interpolation of the solution.
    TMESH_TYPE tmesh_type = TMESH_TYPE::LOG10;  // Mesh type- for use with meshgen_t
    F_T t0 = 1e-16;  // Initial log mesh time value
    F_T tmax = 1e-12;  // Max time value
    SZ_T tpoints = 100;  // Number of time points

    // GENERAL CONTROL PARAMETERS
    bool mobset = true;  // Switch on/off electron hole mobility - MUST BE SET TO ZERO FOR INITIAL SOLUTION
    bool mobseti = true;  // Switch on/off ionic carrier mobility - MUST BE SET TO ZERO FOR INITIAL SOLUTION
    bool SRHset = true;  // Switch on/off SRH recombination - recommend setting to zero for initial solution
    bool radset = true;  // Switch on/off band-to-band recombination
    SZ_T N_max_variables = 5;  // Total number of allowable variables
    // 'Fermi' = Fermi-Dirac, 'Blakemore' = Blakemore approximation, 'Boltzmann' = Boltzmann statistics
    PROB_DIST prob_dist_function = PROB_DIST::BLAKEMORE;
    F_T gamma_Blakemore = 0.27;  // Blakemore coefficient
    F_T Fermi_limit = 0.2;  // Max allowable limit for Fermi levels beyond the bands [eV]
    SZ_T Fermi_Dn_points = 400;  // No. of points in the Fermi-Dirac look-up table
    bool intgradfun = false;  // Interface gradient function 'linear' = linear, 'erf' = 'error function'

    // Generation
    // optical_model = Optical Model
    // 0 = Uniform Generation
    // 1 = Beer Lambert
    bool optical_model = true;  // Should be better using enums
    F_T int1 = 0;  // Light intensity source 1 (multiples of g0 or 1 sun for Beer-Lambert)
    F_T int2 = 0;  // Light intensity source 2 (multiples of g0 or 1 sun for Beer-Lambert)
    L<F_T> g0 = {2.6409e+21};  // Uniform generation rate [cm-3s-1]
    STR_T light_source1 = "AM15";
    STR_T light_source2 = "laser";
    F_T laser_lambda1 = 0;
    F_T laser_lambda2 = 638;
    FUN_TYPE g1_fun_type = FUN_TYPE::CONSTANT;
    FUN_TYPE g2_fun_type = FUN_TYPE::CONSTANT;
    L<F_T> g1_fun_arg = {0};
    L<F_T> g2_fun_arg = {0};
    bool side = false;  // illumination side 1 = left, 2 = right
    STR_T refrlib;
    // default: Approximate Uniform generation rate @ 1 Sun for 510 nm active layer thickness

    // Pulse settings
    F_T pulsepow = 10;  // Pulse power [mW cm-2] OM2 (Beer-Lambert and Transfer Matrix only)

    // LAYER MATERIAL PROPERTIES
    // Numerical values should be given as a row vector with the number of
    // entries equal to the number of layers specified in STACK

    // Energy levels [eV]
    L<F_T> Phi_EA = {0};  // Electron affinity
    L<F_T> Phi_IP = {-1};  // Ionization potential

    // Equilibrium Fermi energies [eV]
    // These define the doping density in each layer- see NA and ND calculations in methods
    L<F_T> EF0 = {-0.5};

    // SRH trap energies [eV]
    // These must exist within the energy gap of the appropriate layers
    // and define the variables PT and NT in the expression:
    // U = (np-ni^2)/(taun(p+pt) +taup(n+nt))
    L<F_T> Et = {-0.5};
    F_T ni_eff = 0;  // Effective intrinsic carrier density used for surface recombination equivalence

    // Electrode Fermi energies [eV]
    // Fermi energies of the metal electrode. These define the built-in voltage, Vbi
    // and the boundary carrier concentrations n0_l, p0_l, n0_r, and
    // p0_r
    F_T Phi_left = -0.6;
    F_T Phi_right = -0.4;

    // Effective Density Of States (eDOS) [cm-3]
    L<F_T> Nc = {1e19};
    L<F_T> Nv = {1e19};
    // PEDOT eDOS: https://aip.scitation.org/doi/10.1063/1.4824104
    // MAPI eDOS: F. Brivio, K. T. Butler, A. Walsh and M. van Schilfgaarde, Phys. Rev. B, 2014, 89, 155204.
    // PCBM eDOS:

    // Mobile ions
    SZ_T N_ionic_species = 1;
    L<F_T> Nani;  // Mobile ion defect density [cm-3] - A. Walsh et al. Angewandte Chemie, 2015, 127, 1811.
    L<F_T> Ncat;  // Mobile ion defect density [cm-3] - A. Walsh et al. Angewandte Chemie, 2015, 127, 1811.
    SZ_T z_c = 1;  // Integer charge state for cations
    SZ_T z_a = -1;  // Integer charge state for anions
    // Limits the density of ions - Approximate density of iodide sites [cm-3]
    L<F_T> a_max = {1.21e22};  // P. Calado thesis
    L<F_T> c_max = {1.21e22};

    F_T K_a = 1;  // Coefficients to easily accelerate ions
    F_T K_c = 1;  // Coefficients to easily accelerate ions

    // Mobilities   [cm2V-1s-1]
    L<F_T> mu_n = {1};  // electron mobility
    L<F_T> mu_p = {1};  // hole mobility
    L<F_T> mu_c = {1e-10};
    L<F_T> mu_a = {1e-12};
    // PTPD h+ mobility: https://pubs.rsc.org/en/content/articlehtml/2014/ra/c4ra05564k
    // PEDOT mu_n = 0.01 cm2V-1s-1 https://aip.scitation.org/doi/10.1063/1.4824104
    // TiO2 mu_n = 0.09 cm2V-1s-1 Bak2008
    // Spiro mu_p = 0.02 cm2V-1s-1 Hawash2018
    // Relative dielectric constants
    L<F_T> epp = {10};
    F_T epp_factor = 1e6;  // a factor required to prevent singular matrix - still under investigation
    // Recombination
    // Radiative recombination, r_rad = k(np - ni^2)
    // [cm3 s-1] Radiative Recombination coefficient
    L<F_T> B = {3.6e-12};

    // SRH time constants for each layer [s]
    L<F_T> taun = {1e6};  // [s] SRH time constant for electrons
    L<F_T> taup = {1e6};  // [s] SRH time constant for holes
    // Surface recombination and extraction coefficients [cm s-1]
    // Descriptions given in the comments considering that holes are
    // extracted at left boundary, electrons at right boundary
    F_T sn_l = 1e7;  // electron surface recombination velocity left boundary
    F_T sn_r = 1e7;  // electron extraction velocity right boundary
    F_T sp_l = 1e7;  // hole extraction left boundary
    F_T sp_r = 1e7;  // hole surface recombination velocity right boundary

    // Volumetric surface recombination
    bool vsr_mode = false;  // Either 1 for volumetric surface recombination approximation or 0 for off
    bool vsr_check = true;  // Perform check for self-consitency at the end of DF
    L<F_T> sn = {0};  // Electron interfacial surface recombination velocity [cm s-1]
    L<F_T> sp = {0};  // Hole interfacial surface recombination velocities [cm s-1]
    F_T frac_vsr_zone = 0.1;  // recombination zone thickness [fraction of interface thickness]
    L<char> vsr_zone_loc;  // recombination zone location either: 'L', 'C', 'R', or 'auto'.
    // IMPORT_PROPERTIES deals with the choice of value.
    // auto is temporary
    F_T AbsTol_vsr = 1e10;  // The integrated interfacial recombination flux above which a warning can be flagged [cm-2 s-1]
    F_T RelTol_vsr = 0.05;  // Fractional error between abrupt and volumetric surface recombination models above which a warning is flagged

    // Series resistance
    F_T Rs = 0;
    F_T Rs_initial = 0;  // Switch to allow linear ramp of Rs on first application

    // Defect recombination rate coefficient
    // Currently not used
    F_T k_defect_p = 0;
    F_T k_defect_n = 0;

    // Dynamically created variables

    // Voltage function parameters
    FUN_TYPE V_fun_type = FUN_TYPE::CONSTANT;
    L<F_T> V_fun_arg = {0};

    // Define the default relative tolerance for the pdepe solver
    // 1e-3 is the default, can be decreased if more precision is needed
    // Solver options
    F_T MaxStepFactor = 1;  // Multiplier for easy access to maximum time step
    F_T RelTol = 1e-3;
    F_T AbsTol = 1e-6;

    // Impedance parameters
    // J_E_func, J_E_func_tilted, and E2_func
    L<F_T> xx;
    L<F_T> x_sub;

    SZ_T col_size() const {
        return layer_type.size();
    }

    /* Getters */
    F_T gamma() const {
        switch (prob_dist_function) {
            case PROB_DIST::BOLTZMANN:
                return 0;
            case PROB_DIST::BLAKEMORE:
                return gamma_Blakemore;
            default:
                throw std::invalid_argument("prob_dist_function is neither Boltzmann not Blakemore");
        }
    }
    // Get active layer indexes from layer_type
    SZ_T active_layer() const {
        auto val_it = std::ranges::find(layer_type, "active");
        if (val_it == layer_type.cend()) {
            // If no flag is give assume active layer is middle
            std::println("No designated 'active' layer - assigning middle layer to be active");
            return std::round(col_size() / 2);
        }
        return std::distance(layer_type.cbegin(), val_it);
    }

    // Active layer thickness
    // d_active/d_midactive: active_layer is not a list

    // Band gap energies    [eV]
    L<F_T> Eg() const {
        L<F_T> value(col_size());
        for (SZ_T i = 0; i < col_size(); i++) {
            value[i] = Phi_EA.at(i) - Phi_IP.at(i);
        }
        return value;
    };

    // Built-in voltage Vbi based on difference in boundary work functions
    L<F_T> Vbi() const {
        L<F_T> value(col_size());
        for (SZ_T i = 0; i < col_size(); i++) {
            value[i] = Phi_right.at(i) - Phi_left.at(i);
        }
        return value;
    };

    // Intrinsic Fermi Energies
    // Currently uses Boltzmann stats as approximation should always be
    L<F_T> Efi() const {
        L<F_T> value(col_size());
        for (SZ_T i = 0; i < col_size(); i++) {
            value[i] = 0.5 * (Phi_EA.at(i) - Phi_IP.at(i)) + kB * T;
        }
        return value;
    };

    // Donor densities
    L<F_T> ND() const {
        return DistFun<L, F_T, STR_T>::nfun(Nc, Phi_EA, EF0, prob_dist_function, T, gamma());
    }

    // Acceptor densities
    L<F_T> NA() const {
        return DistFun<L, F_T, STR_T>::pfun(Nv, Phi_IP, EF0, prob_dist_function, T, gamma());
    }

    // Intrinsic carrier densities (Boltzmann)
    L<F_T> ni() const {
        L<F_T> value(col_size());
        for (SZ_T i = 0; i < col_size(); i++) {
            value[i] = std::sqrt(Nc.at(i), Nv.at(i)) * std::exp(-Eg() / (2 * kB * T));
        }
        return value;
    }

    // Equilibrium electron densities
    L<F_T> n0() const {  // identical to ND(), no internal support for in-class function aliases, copy to avoid potential overhead
        return DistFun<L, F_T, STR_T>::nfun(Nc, Phi_EA, EF0, prob_dist_function, T, gamma());
    }

    // Equilibrium hole densities
    L<F_T> p0() const {  // identical to NA()
        return DistFun<L, F_T, STR_T>::pfun(Nv, Phi_IP, EF0, prob_dist_function, T, gamma());
    }

    // Boundary electron and hole densities
    // Uses metal Fermi energies to calculate boundary densities
    // Electrons left boundary
    F_T n0_l() const {
        return DistFun<L, F_T, STR_T>::nfun(Nc.front(), Phi_EA.front(), Phi_left, prob_dist_function, T, gamma());
    }

    // Electrons right boundary
    F_T n0_r() const {
        return DistFun<L, F_T, STR_T>::nfun(Nc.back(), Phi_EA.back(), Phi_right, prob_dist_function, T, gamma());
    }

    // Holes left boundary
    F_T p0_l() const {
        return DistFun<L, F_T, STR_T>::pfun(Nv.front(), Phi_IP.front(), Phi_left, prob_dist_function, T, gamma());
    }

    // Holes right boundary
    F_T p0_r() const {
        return DistFun<L, F_T, STR_T>::pfun(Nv.back(), Phi_IP.back(), Phi_right, prob_dist_function, T, gamma());
    }

    // SRH trap energy coefficients
    L<F_T> nt() {
        return DistFun<L, F_T, STR_T>::nfun(Nc, Phi_EA, Et, prob_dist_function, T, gamma());
    }

    L<F_T> pt() {
        return DistFun<L, F_T, STR_T>::nfun(Nv, Phi_IP, Et, prob_dist_function, T, gamma());
    }

    // Thickness and point arrays
    L<F_T> dcum() const {
        return std::partial_sum(d.begin(), d.end());
    }

    L<SZ_T> pcum() const {
        return std::partial_sum(layer_points.begin(), layer_points.end());
    }

    L<F_T> dcum0() const {
        return {0, std::partial_sum(d.begin(), d.end())};
    }

    L<SZ_T> pcum0() const {
        return {1, std::partial_sum(layer_points.begin(), layer_points.end())};
    }

    // interface switch for zeroing field in interfaces
    // int_switch all ones

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

    template<typename VAR_T>
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

    template<typename VAR_T>
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

    template<typename T>
    static L<T> import_single_property(const L<L<STR_T>> &data,
                                       const std::map<STR_T, SZ_T> &property_in,
                                       const std::set<STR_T> &possible_headers,
                                       SZ_T start_row,
                                       SZ_T end_row) {
        for (const STR_T &possible_header : possible_headers) {
            const std::map<STR_T, SZ_T>::const_iterator it = property_in.find(possible_header);
            if (it not_eq property_in.cend()) {
                const SZ_T index = it->second;
                L<T> property(end_row - start_row + 1);
                for (SZ_T rc = start_row; rc <= end_row; rc++) {
                    if constexpr (std::same_as<T, STR_T>) {
                        property[rc - start_row] = data.at(rc).at(index);
                    } else if constexpr (std::same_as<T, F_T>) {
                        STR_T double_str = data.at(rc).at(index);
                        property[rc - start_row] = double_str.isEmpty() ? NAN : double_str.toDouble();
                    } else if constexpr (std::same_as<T, SZ_T>) {
                        property[rc - start_row] = data.at(rc).at(index).toLongLong();
                    } else {
                        static_assert(false, "Invalid type for import_single_property");
                    }
                }
                return property;
            }
        }
        throw std::out_of_range("No column headings match: " + possible_headers.cbegin()->toStdString() +
                                ", using default in PC.");
    }

private:
    /*
     * A function to IMPORT_PROPERTIES from a text file LOCATED at FILEPATH. Each of the listed properties
     * is checked to see if it is available in the .CSV file. If it is available, the existing properties
     * are overwritten otherwise a warning is displayed. Some entries have
     * nested try-catch statements for backwards compatibility with older
     * variable names stored in .csv files. The object properties will still be
     * imported with the latest nomenclature.
     *
     * IMPORT_SINGLE_PROPERTY checks for the existence of column headers in the .CSV
     * with POSSIBLE_HEADERS. Multiple names for variables in the .CSV are given
     * for backwards compatibility, however once read-in the properties take the
     * naming convention of the current version i.e. this is simply to allow old
     * parameter files to be read-in.
     */
    void import_properties(const L<L<STR_T>> &csv_data, const std::map<STR_T, SZ_T> &properties) {
        SZ_T start_row;
        SZ_T end_row;
        SZ_T maxRow = csv_data.size();
        const std::map<STR_T, SZ_T>::const_iterator pit_lt = properties.find("layer_type");
        if (pit_lt == properties.cend()) {
            throw std::runtime_error("No layer type (layer_type) defined in .csv. "
                     "layer_type must be defined when using .csv input file");
        }
        SZ_T layer_type_index = pit_lt->second;  // ELECTRODE, LAYER, INTERFACE, ACTIVE
        bool has_electrodes = false;
        if (csv_data.at(1).at(layer_type_index) == "electrode") {
            start_row = 2;
        } else {  // no front surface electrode
            start_row = 1;
            throw std::runtime_error("Missing front surface electrode; RAT calculation is disabled.");
        }
        if (csv_data.at(maxRow - 1).at(layer_type_index) == "electrode") {
            end_row = maxRow - 2;
            has_electrodes = start_row == 2;
        } else {  // no back surface electrode
            end_row = maxRow - 1;
            throw std::runtime_error("Missing back surface electrode; RAT calculation is disabled.");
        }
        layer_type.resize(end_row - start_row + 1);
        for (const auto [i, rc] : std::views::enumerate(std::views::iota(start_row, end_row + 1))) {
            layer_type[static_cast<SZ_T>(i)] = csv_data.at(rc).at(layer_type_index);
        }
        // Material name array
        // This material list contains both layers and interfaces (typical structure has a size of 5).
        material = import_single_property<STR_T>(csv_data, properties, {"material", "stack"}, start_row, end_row);
        // Layer thickness array
        d = import_single_property<F_T>(csv_data, properties, {"dcell", "d", "thickness"}, start_row, end_row);
        const SZ_T sz_mat = material.size();
        if (d.size() not_eq sz_mat) {
            throw std::runtime_error("Size of thickness does not match size of material!");
        }
        // Layer points array
        layer_points = import_single_property<SZ_T>(csv_data, properties, {"layer_points", "points"}, start_row, end_row);;
        // Spatial mesh coefficient for non-linear meshes
        xmesh_coeff = import_single_property<F_T>(csv_data, properties, {"xmesh_coeff"}, start_row, end_row);
        // Electron affinity array
        Phi_EA = import_single_property<F_T>(csv_data, properties, {"Phi_EA", "EA"}, start_row, end_row);
        // Ionization potential array
        Phi_IP = import_single_property<F_T>(csv_data, properties, {"Phi_IP", "IP"}, start_row, end_row);
        // SRH Trap Energy
        Et = import_single_property<F_T>(csv_data, properties, {"Et", "Et_bulk"}, start_row, end_row);
        // Equilibrium Fermi energy array
        EF0 = import_single_property<F_T>(csv_data, properties, {"EF0", "E0"}, 1, maxRow - 1);
        EF0 = EF0.mid(start_row, end_row - start_row + 1);
        Phi_left = EF0.front();
        Phi_right = EF0.back();
        // Conduction band effective density of states
        Nc = import_single_property<F_T>(csv_data, properties, {"Nc", "Ncb", "NC", "NCB"}, start_row, end_row);
        // Valence band effective density of states
        Nv = import_single_property<F_T>(csv_data, properties, {"Nv", "Ncb", "NV", "NVB"}, start_row, end_row);
        // Intrinsic anion density
        Nani = import_single_property<F_T>(csv_data, properties, {"Nani"}, start_row, end_row);
        // Intrinsic cation density
        Ncat = import_single_property<F_T>(csv_data, properties, {"Ncat", "Nion"}, start_row, end_row);
        // Limiting density of anion states
        a_max = import_single_property<F_T>(csv_data, properties, {"a_max", "amax", "DOSani"}, start_row, end_row);
        // Limiting density of cation states
        c_max = import_single_property<F_T>(csv_data, properties, {"c_max", "cmax", "DOScat"}, start_row, end_row);
        // Electron mobility
        mu_n = import_single_property<F_T>(csv_data, properties, {"mu_n", "mun", "mue", "mu_e"}, start_row, end_row);
        // Hole mobility
        mu_p = import_single_property<F_T>(csv_data, properties, {"mu_p", "mup", "muh", "mu_h"}, start_row, end_row);
        // Anion mobility
        mu_a = import_single_property<F_T>(csv_data, properties, {"mu_a", "mua", "mu_ani", "muani"}, start_row, end_row);
        // Cation mobility
        mu_c = import_single_property<F_T>(csv_data, properties, {"mu_c", "muc", "mu_cat", "mucat"}, start_row, end_row);
        // Relative dielectric constant
        epp = import_single_property<F_T>(csv_data, properties, {"epp", "eppr"}, start_row, end_row);
        // Uniform volumetric generation rate
        g0 = import_single_property<F_T>(csv_data, properties, {"g0", "G0"}, start_row, end_row);
        // Band-to-band recombination coefficient
        B = import_single_property<F_T>(csv_data, properties, {"B", "krad", "kbtb"}, start_row, end_row);
        // Electron SRH time constant
        taun = import_single_property<F_T>(csv_data, properties, {"taun", "taun_SRH"}, start_row, end_row);
        // Hole SRH time constant
        taup = import_single_property<F_T>(csv_data, properties, {"taup", "taup_SRH"}, start_row, end_row);
        // Electron and hole surface recombination velocities
        if (has_electrodes) {
            sn = import_single_property<F_T>(csv_data, properties, {"sn"}, 1, maxRow - 1);
            sn = sn.mid(start_row, end_row - start_row + 1);
            sn_l = sn.front();
            sn_r = sn.back();
            sp = import_single_property<F_T>(csv_data, properties, {"sp"}, 1, maxRow - 1);
            sp = sp.mid(start_row, end_row - start_row + 1);
            sp_l = sp.front();
            sp_r = sp.back();
        } else {
            sn = import_single_property<F_T>(csv_data, properties, {"sn"}, start_row, end_row);
            sp = import_single_property<F_T>(csv_data, properties, {"sp"}, start_row, end_row);
        }
        STR_T optical_model_str;
        typename std::map<STR_T, SZ_T>::const_iterator pit = properties.find("optical_model");
        if (pit not_eq properties.cend()) {
            optical_model_str = csv_data.at(1).at(pit->second);
        } else {
            pit = properties.find("OM");
            if (pit not_eq properties.cend()) {
                optical_model_str = csv_data.at(1).at(pit->second);
            }
        }
        if (optical_model_str == "uniform" or optical_model_str.toDouble() == 0) {
            optical_model = false;
        } else if (optical_model_str == "Beer-Lambert" or optical_model_str.toDouble() == 1) {
            optical_model = true;
        } else {
            std::cerr << "optical_model not recognized - defaulting to 'Beer-Lambert'\n";
        }
        // Illumination side
        const STR_T side_str = csv_data.at(1).at(properties.at("side"));  // Row first!
        if (side_str == "right" or side_str.toDouble() == 2) {  // not toInt() or toUInt()!
            side = true;
        } else if (side_str == "left" or side_str.toDouble() == 1) {
            side = false;
        } else {
            // Warn if xmesh_type is not correct
            throw std::runtime_error("Side property is not correctly specified.");
        }
        // Spatial mesh
        const STR_T xmesh_type_str = csv_data.at(1).at(properties.at({"xmesh_type"}));
        if (xmesh_type_str == "linear") {
            xmesh_type = false;
        } else if (xmesh_type_str == "erf-linear") {
            xmesh_type = true;
        } else {
            throw std::runtime_error("PAR.xmesh_type should either be 'linear' or 'erf-linear'. "
                                     "MESHGEN_X cannot generate a mesh if this is not the case.");
        }
        // Recombination zone location
        if (layer_type.contains("Interface") or layer_type.contains("junction")) {
            L<char> vsr_zone_loc_auto = locate_vsr_zone();
            const std::map<STR_T, SZ_T>::const_iterator pit_vzl = properties.find("vsr_zone_loc");
            L<STR_T> vsr_zone_loc_user(end_row - start_row + 1);
            if (pit_vzl not_eq properties.cend()) {
                for (SZ_T rc = start_row; rc <= end_row; rc++) {
                    vsr_zone_loc_user[rc - start_row] = csv_data.at(rc).at(pit_vzl->second);
                }
                for (SZ_T i = 0; i < col_size(); i++) {
                    const STR_T &user_vzl = vsr_zone_loc_user.at(i);
                    if (user_vzl == "L") {  // toAscii() toLatin1()
                        vsr_zone_loc[i] = 'L';
                    } else if (user_vzl == "C") {
                        vsr_zone_loc[i] = 'C';
                    } else if (user_vzl == "R") {
                        vsr_zone_loc[i] = 'R';
                    } else if (user_vzl == "auto") {
                        vsr_zone_loc[i] = vsr_zone_loc_auto.at(i);
                    }
                }
            } else {
                std::cerr << "Recombination zone location (vsr_zone_loc) not defined in .csv. Using auto defined\n";
                vsr_zone_loc = vsr_zone_loc_auto;
            }
        }
    }

    L<char> locate_vsr_zone() {
        // A function to locate the recombination zone within the interfacial regions based on the minority carrier
        // densities at equilibrium intelligent location
        // QList<bool> int_logical(par.col_size());
        L<SZ_T> loc;  // interface layer locations
        for (SZ_T i = 0; i < col_size(); i++) {
            if (layer_type.at(i) == "interface") {
                loc.emplace_back(i);
            }
        }
        vsr_zone_loc.resize(col_size());
        L<F_T> n_0 = n0();
        L<F_T> p_0 = p0();
        for (const SZ_T i : loc) {
            // Gradient coefficients for surface recombination equivalence
            F_T alpha0 = ((Phi_EA.at(i - 1) - Phi_EA.at(i + 1)) / kB * T + (std::log(Nc.at(i + 1)) - std::log(Nc.at(i - 1)))) / d.at(i);
            F_T beta0 = ((Phi_IP.at(i - 1) - Phi_IP.at(i + 1)) / kB * T + (std::log(Nv.at(i + 1)) - std::log(Nv.at(i - 1)))) / d.at(i);
            if (alpha0 <= 0 and beta0 > 0 or alpha0 < 0 and beta0 >= 0) {
                if (n_0.at(i + 1) > p_0.at(i - 1)) {
                    vsr_zone_loc[i] = 'R';
                } else if (n_0.at(i + 1) < p_0.at(i - 1)) {
                    vsr_zone_loc[i] = 'L';
                } else {
                    vsr_zone_loc[i] = 'C';
                }
            } else if (alpha0 >= 0 and beta0 < 0 or alpha0 > 0 and beta0 <= 0) {
                if (p_0.at(i + 1) > n_0.at(i - 1)) {
                    vsr_zone_loc[i] = 'R';
                } else if (p_0.at(i + 1) < n_0.at(i - 1)) {
                    vsr_zone_loc[i] = 'L';
                } else {
                    vsr_zone_loc[i] = 'C';
                }
            } else if (alpha0 <= 0 and beta0 < 0 or alpha0 < 0 and beta0 <= 0) {
                vsr_zone_loc[i] = 'L';
            } else if (alpha0 >= 0 and beta0 > 0 or alpha0 > 0 and beta0 >= 0) {
                vsr_zone_loc[i] = 'R';
            } else {
                vsr_zone_loc[i] = 'C';
            }
        }
        return vsr_zone_loc;
    }

    // Generates the spatial mesh dependent on option defined by XMESH_TYPE
    L<F_T> meshgen_x() {
        // Linearly spaced
        L<F_T> x;  // pcum().back()
        L<F_T> dcum = dcum0();
        if (xmesh_type) {  // linear
            for (SZ_T i : std::views::iota(0U, col_size())) {
                x.append_range(Utils::Math::linspace(dcum.at(i), dcum.at(i + 1) - d.at(i) / layer_points.at(i), layer_points.at(i)));
            }
            x.emplace_back(dcum.back());
        } else {  // erf-linear
            for (SZ_T i : std::views::iota(0U, col_size())) {
                std::vector<F_T> x_layer;
                if (layer_type.at(i) == "layer" or layer_type.at(i) == "active") {
                    std::vector<F_T> parr = Utils::Math::linspace(-0.5, 1 / std::numbers::pi_v<F_T>, 0.5);
                    std::size_t sz_parr = parr.size();
                    x_layer.resize(sz_parr);
                    F_T x_layer0 = std::erf(2 * std::numbers::pi_v<F_T> * xmesh_coeff.front() * parr.front());
                    x_layer.front() = 0;
                    for (std::size_t j : std::views::iota(1U, sz_parr)) {
                        x_layer[i] = std::erf(2 * std::numbers::pi_v<F_T> * xmesh_coeff.at(i) * parr.at(i)) - x_layer0;
                    }
                    F_T max_x_layer = std::ranges::max(x_layer);
                    for (std::size_t j : std::views::iota(1U, sz_parr)) {
                        x_layer[i] = dcum.at(i) + x_layer[i] / max_x_layer * d.at(i);
                    }
                } else if (layer_type.at(i) == "junction" or layer_type.at(i) == "interface") {
                    x_layer = Utils::Math::linspace(dcum.at(i), dcum.at(i + 1) - d.at(i) / layer_points.at(i), layer_points.at(i));
                }
                x.append_range(x_layer.begin(), x_layer.end() - 1);
                x.emplace_back(dcum.back());
            }
        }
        return x;
    }

    // BUILD_DEVICE calls BUILD_PROPERTY for each device property. BUILD_PROPERTY then defines the
    // properties at each point on the grid defined by MESHOPTION
    Device<L<F_T>> build_device(bool meshoption);

    // Rebuilds important device properties
    void refresh_device() {
        xx = meshgen_x();
        x_sub = getvar_sub(xx);
    }
};

#include "BuildDevice.tpp"

#endif  // SUISAPP_PARAMETERCLASS_H

#include <iostream>
#include "equilibrate.h"

struct EqSol {
public:
    double u = 0;
};

void equilibrate(Parameter &par, bool electronic_only) {
    // Initial arguments
    // Setting sol.u = 0 enables a parameters structure to be read into
    // DF but indicates that the initial conditions should be the
    // analytical solutions
    struct EqSol sol;
    sol.u = 0;
    // Store the original parameter set
    Parameter par_origin = par;
    // Start with zero SRH recombination
    par.SRHset = false;
    // Radiative rec could initially be set to zero in addition if required
    par.radset = true;
    // Start with no ionic carriers
    par.N_ionic_species = 0;
    // Switch off volumetric surface recombination check
    par.vsr_check = false;

    /* General initial parameters */
    // Set applied bias to zero
    par.V_fun_type = VFunType::CONSTANT;
    par.V_fun_arg = 0;

    // Set light intensities to zero
    par.int1 = 0;
    par.int2 = 0;
    par.g1_fun_type = G1FunType::CONSTANT;
    par.g2_fun_type = G2FunType::CONSTANT;

    // Time mesh
    par.tmesh_type = TMeshType::LOG10;
    par.tpoints = 10;

    // Series resistance
    par.Rs = 0;

    /* Switch off mobilities */
    par.mobset = false;
    par.mobseti = false;

    /* Initial solution with zero mobility */
    std::cout << "Initial solution, zero mobility" << '\n';
}

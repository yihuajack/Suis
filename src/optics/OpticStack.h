//
// Created by Yihua Liu on 2024-7-4.
//

#ifndef SUISAPP_OPTICSTACK_H
#define SUISAPP_OPTICSTACK_H

#include <string>
#include <vector>

#include "material/MaterialDbModel.h"

/*
 * Class that contains an optical structure: a sequence of layers with a thickness
and a complex refractive index.

It serves as an intermediate step between layers and materials and the
stack of thicknesses and n and k values necessary to run calculations involving TMM.
When creating an OpticStack object, the thicknesses of all the layers forming the
structure and the optical data of the materials of the layers are extracted
and arranged in such a way they can be easily and fast read by the TMM functions.

In addition to a structure with Layers, it can also take a list where each
element represent a layer written as a list and contains the layer thickness and
the dielectric model, the raw n and k data as a function
of wavelengths, or a whole Device structure as the type used in the PDD model.

In summary, this class accepts:

    - A structure with layers
    - A list where each element is [thickness, DielectricModel]
    - A list where each element is [thickness, wavelength, n, k]
    - A list mixing the above:
        [ [thickness, DielectricModel],
          [thickness, wavelength, n, k],
          Layer,
          Layer ]

This allows for maximum flexibility when creating the optical model, allowing to
construct the stack with experimental data, modelled data and known material
properties from the database.

Yet another way of defining the layers mixes experimental data with a DielectricModel
within the same layer but in spectral distinct regions. The syntax for the
layer is:

layer = [thickness, wavelength, n, k, DielectricModel, mixing]

where mixing is a list containing three elements:
[the mixing point (nm), the mixing width (nm), zero or one]
depending on if the mixing function should be increasing with the wavelength or
decreasing. If increasing (zero), the Dielectric model will be used at long
wavelengths and the experimental data at short wavelengths. If decreasing (one)
the opposite is done. The mixing point and mixing width control how smooth is the
transition between one and the other type of data.

Extra layers such as he semi-infinite, air-like first and last medium, and a back
highly absorbing layer are included at runtime to fulfill the requirements of the
TMM solver or to solve some of its limitations.
 */
template<typename T>
class OpticStack {
public:
    OpticStack(const std::vector<std::pair<std::string, double>> &structure, bool no_back_reflection = false,
               const std::string &substrate = "", const std::string &incidence = "");

private:
    std::vector<std::pair<OpticMaterial<T>, double>> structure;
    OpticMaterial<T> substrate;
    OpticMaterial<T> incidence;
};


#endif  //SUISAPP_OPTICSTACK_H

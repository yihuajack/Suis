//
// Created by Yihua Liu on 2024-7-4.
//

#include <memory>

#include "OpticStack.h"

/*
 * Calculates the reflected, absorbed and transmitted intensity of the structure
    for the wavelengths and angles defined.

    :param structure: A solcore Structure object with layers and materials or a
        OpticStack object.
    :param wavelength: Wavelengths (in nm) in which calculate the data. An array.
    :param angle: Angle (in degrees) of the incident light.
        Default: 0 (normal incidence).
    :param pol: Polarisation of the light: 's', 'p' or 'u'. Default: 'u' (unpolarised).
    :param coherent: If the light is coherent or not. If not, a coherency list
        must be added.
    :param coherency_list: A list indicating in which layers light should be treated as
        coherent ('c') and in which incoherent ('i'). It needs as many elements as
        layers in the structure.
    :param no_back_reflection: If reflection from the back must be supressed.
        Default=True.
    :return: A dictionary with the R, A and T at the specified wavelengths and angle.
 */
template<typename T>
void calculate_rat(OpticStack<T> &structure, std::vector<double> &wavelength, double angle = 0, char pol = 'u',
                   bool coherent = true) {
    const std::size_t num_wl = wavelength.size();
}

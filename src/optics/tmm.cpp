#include <complex>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>
#include "tmm.h"

template <typename T>
static inline std::string complex_to_string_with_name(const std::complex<T> c, const std::string& name) {
    // QDebug cannot overload << and >> for std::complex
    std::ostringstream ss;
    ss << name << ": " << c.real() << " + " << c.imag() << "i";
    return ss.str();
}

/*
 * if a wave is traveling at angle theta from normal in a medium with index n,
 * calculate whether or not this is the forward-traveling wave (i.e., the one
 * going from front to back of the stack, like the incoming or outgoing waves,
 * but unlike the reflected wave). For real n & theta, the criterion is simply
 * -pi/2 < theta < pi/2, but for complex n & theta, it's more complicated.
 * See https://arxiv.org/abs/1603.02720 appendix D. If theta is the forward
 * angle, then (pi-theta) is the backward angle and vice-versa.
 */
template <typename T>
bool is_forward_angle(const std::complex<T> n, const std::complex<T> theta) {
    bool answer = false;
    if (n.real() * n.imag() >= 0) {
        throw std::runtime_error("For materials with gain, it's ambiguous which beam is incoming vs outgoing\n" +
                                 complex_to_string_with_name(n, "n") + "\t" + complex_to_string_with_name(theta, "angle"));
    } else {
        std::complex<double> ncostheta = n * std::cos(theta);
        answer = std::abs(ncostheta.imag()) > 100 * EPSILON ? ncostheta.imag() > 0 : ncostheta.real() > 0;
        if ((answer && (ncostheta.imag() > -100 * EPSILON ||
                        ncostheta.real() > -100 * EPSILON ||
                        std::real(n * std::cos(std::conj(theta))) > -100 * EPSILON)) ||
            (!answer && (ncostheta.imag() < 100 * EPSILON ||
                         ncostheta.real() < 100 * EPSILON ||
                         std::real(n * std::cos(std::conj(theta))) < 100 * EPSILON))) {
            throw std::runtime_error("It's not clear which beam is incoming vs outgoing. Weird index maybe?\n" +
                                     complex_to_string_with_name(n, "n") + "\t" + complex_to_string_with_name(theta, "angle"));
        }
    }
    return answer;
}

/*
 * return angle theta in layer 2 with refractive index n_2, assuming
 * it has angle th_1 in layer with refractive index n_1. Use Snell's law. Note
 * that "angles" may be complex!!
 */
template <typename T>
std::complex<T> snell(const std::complex<T> n_1, const std::complex<T> n_2, const std::complex<T> th_1) {
    std::complex<T> th_2_guess = std::asin(n_1 * std::sin(th_1) / n_2);
    return is_forward_angle(n_2, th_2_guess) ? th_2_guess : M_PI - th_2_guess;
}

/* return list of angle theta in each layer based on angle th_0 in layer 0,
 * using Snell's law. n_list is index of refraction of each layer. Note that
 * "angles" may be complex!!
 */
template <typename T>
std::valarray<std::complex<T>> list_snell(const std::valarray<std::complex<T>>& n_list, const std::complex<T> th_0) {
    std::valarray<std::complex<T>> angles = std::asin(n_list[0] * std::sin(th_0) / n_list);
    if (!is_forward_angle(n_list[0], angles[0])) {
        angles[0] = M_PI - angles[0];
    }
    if (!is_forward_angle(n_list[n_list.size() - 1], angles[angles.size() - 1])) {
        angles[angles.size() - 1] = M_PI - angles[angles.size() - 1];
    }
    return angles;
}

/*
 * reflection amplitude (from Fresnel equations)
 *
 * polarization is either "s" or "p" for polarization
 *
 * n_i, n_f are (complex) refractive index for incident and final
 *
 * th_i, th_f are (complex) propagation angle for incident and final
 * (in radians, where 0=normal). "th" stands for "theta".
 */
template <typename T>
std::complex<T> interface_r(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i, const std::complex<T> th_f) {
    if (polarization == 's') {
        return (n_i * std::cos(th_i) - n_f * std::cos(th_f)) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    } else if (polarization == 'p') {
        return (n_f * std::cos(th_i) - n_i * std::cos(th_f)) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    } else {
        throw std::invalid_argument("Polarization must be 's' or 'p'");
    }
}

/*
 * transmission amplitude (from Fresnel equations)
 *
 * polarization is either "s" or "p" for polarization
 *
 * n_i, n_f are (complex) refractive index for incident and final
 *
 * th_i, th_f are (complex) propagation angle for incident and final
 * (in radians, where 0=normal). "th" stands for "theta".
 */
template <typename T>
std::complex<T> interface_t(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i, const std::complex<T> th_f) {
    if (polarization == 's') {
        return 2 * n_i * std::cos(th_i) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    } else if (polarization == 'p') {
        return 2 * n_i * std::cos(th_i) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    } else {
        throw std::invalid_argument("Polarization must be 's' or 'p'");
    }
}

/*
 * Calculate reflected power R, starting with reflection amplitude r.
 */
template <typename T>
T R_from_r(const std::complex<T> r) {
    return std::norm(r);
}

/*
 * Calculate transmitted power T, starting with transmission amplitude t.
 *
 * n_i,n_f are refractive indices of incident and final medium.
 *
 * th_i, th_f are (complex) propagation angles through incident & final medium
 * (in radians, where 0=normal). "th" stands for "theta".
 *
 * In the case that n_i, n_f, th_i, th_f are real, formulas simplify to
 * T=|t|^2 * (n_f cos(th_f)) / (n_i cos(th_i)).
 *
 * See manual for discussion of formulas
 */
template <typename T>
T T_from_t(const char pol, const std::complex<T> t, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i, const std::complex<T> th_f) {
    if (pol == 's') {
        return std::abs(t * t) * (n_f * std::cos(th_f)).real() / (n_i * std::cos(th_i)).real();
    } else if (pol == 'p') {
        return std::abs(t * t) * (n_f * std::conj(std::cos(th_f))).real() / (n_i * std::conj(std::cos(th_i))).real();
    } else {
        throw std::invalid_argument("Polarization must be 's' or 'p'");
    }
}

/*
 * Calculate the power entering the first interface of the stack, starting with
 * reflection amplitude r. Normally this equals 1-R, but in the unusual case
 * that n_i is not real, it can be a bit different than 1-R. See manual.
 *
 * n_i is refractive index of incident medium.
 *
 * th_i is (complex) propegation angle through incident medium
 * (in radians, where 0=normal). "th" stands for "theta".
 */
template <typename T>
T power_entering_from_r(const char pol, const std::complex<T> r, const std::complex<T> n_i, const std::complex<T> th_i) {
    if (pol == 's') {
        return (n_i * std::cos(th_i) * (1 + std::conj(r)) * (1 - r)).real() / (n_i * std::cos(th_i)).real();
    } else if (pol == 'p') {
        return (n_i * std::conj(std::cos(th_i)) * (1 + r) * (1 - std::conj(r))).real() / (n_i * std::conj(std::cos(th_i))).real();
    } else {
        throw std::invalid_argument("Polarization must be 's' or 'p'");
    }
}

/*
 * Fraction of light intensity reflected at an interface.
 */
template <typename T>
T interface_R(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i, const std::complex<T> th_f) {
    return R_from_r(interface_r(polarization, n_i, n_f, th_i, th_f));
}

/*
 * Fraction of light intensity transmitted at an interface.
 */
template <typename T>
T interface_T(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i, const std::complex<T> th_f) {
    return T_from_t(interface_t(polarization, n_i, n_f, th_i, th_f), n_i, n_f, th_i, th_f);
}

/*
 * Main "coherent transfer matrix method" calc. Given parameters of a stack,
 * calculates everything you could ever want to know about how light
 * propagates in it. (If performance is an issue, you can delete some of the
 * calculations without affecting the rest.)
 *
 * pol is light polarization, "s" or "p".
 *
 * n_list is the list of refractive indices, in the order that the light would
 * pass through them. The 0'th element of the list should be the semi-infinite
 * medium from which the light enters, the last element should be the semi-
 * infinite medium to which the light exits (if any exits).
 *
 * th_0 is the angle of incidence: 0 for normal, pi/2 for glancing.
 * Remember, for a dissipative incoming medium (n_list[0] is not real), th_0
 * should be complex so that n0 sin(th0) is real (intensity is constant as
 * a function of lateral position).
 *
 * d_list is the list of layer thicknesses (front to back). Should correspond
 * one-to-one with elements of n_list. First and last elements should be "inf".
 *
 * lam_vac is vacuum wavelength of the light.
 *
 * Outputs the following as a dictionary (see manual for details)
 *
 * - r--reflection amplitude
 * - t--transmission amplitude
 * - R--reflected wave power (as fraction of incident)
 * - T--transmitted wave power (as fraction of incident)
 * - power_entering--Power entering the first layer, usually (but not always)
 *   equal to 1-R (see manual).
 * - vw_list-- n'th element is [v_n,w_n], the forward- and backward-traveling
 *   amplitudes, respectively, in the n'th medium just after interface with
 *   (n-1)st medium.
 * - kz_list--normal component of complex angular wavenumber for
 *   forward-traveling wave in each layer.
 * - th_list--(complex) propagation angle (in radians) in each layer
 * - pol, n_list, d_list, th_0, lam_vac--same as input
 */
template <typename T>
std::unordered_map<std::string, std::variant<std::complex<T>>> coh_tmm(const char polarization, const std::valarray<std::complex<T>>& n_list, const std::valarray<T>& d_list, const std::complex<T> th_0, const T lam_vac) {
    if (n_list.size() != d_list.size()) {
        throw std::logic_error("n_list and d_list must have same length");
    }
    if (!isinf(d_list[0]) || !isinf(d_list[d_list.size() - 1])) {
        throw std::runtime_error("d_list must start and end with inf!");
    }
    if (std::abs((n_list.front() * std::sin(th_0)).imag()) < 100 * EPSILON || is_forward_angle(n_list.front(), th_0)) {
        throw std::runtime_error("Error in n0 or th0!");
    }
    size_t num_layers = n_list.size();
    std::valarray<std::complex<T>> th_list = list_snell(n_list, th_0);
    std::valarray<std::complex<T>> kz_list = 2 * M_PI * n_list * std::cos(th_list) / lam_vac;
    result["kz_list"] = kz_list;
    std::vector<std::complex<T>> r_list;
    std::vector<std::complex<T>> t_list;
    std::vector<std::complex<T>> vw_list;
    std::complex<T> prev_kz = kz_list.front();
    for (size_t i = 1; i < n_list.size(); i++) {
        std::complex<T> this_kz = kz_list[i];
        std::complex<T> delta = this_kz * d_list[i];
        std::complex<T> phase = std::exp(std::complex<T>(0, 1) * delta);
        std::complex<T> interface_r = coh_interface_r(polarization, n_list[i - 1], n_list[i], th_list[i - 1], th_list[i]);
        std::complex<T> interface_t = coh_interface_t(polarization, n_list[i - 1], n_list[i], th_list[i - 1], th_list[i]);
        std::complex<T> r = interface_r + interface_t * interface_t * prev_kz * phase / (1 - interface_r * interface_r * prev_kz * this_kz * phase);
        std::complex<T> t = interface_t * phase * (1 - interface_r * interface_r) / (1 - interface_r * interface_r * prev_kz * this_kz * phase);
        r_list.push_back(r);
        t_list.push_back(t);
        vw_list.push_back({1, r});
        prev_kz = this_kz;
    }
    std::unordered_map<std::string, std::vector<std::complex<T>>> result;
    result["r"] = r;
    result["t"] = t;
    result["R"] = R_from_r(r);
    result["T"] = T_from_t(pol, t, n_list.front(), n_list.back(), th_0, th_list.back());
    result["power_entering"] = {power_entering_from_r(polarization, r_list.front(), n_list.front(), th_list.front())};
    return result;
}

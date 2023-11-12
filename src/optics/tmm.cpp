#include <algorithm>
#include <array>
#include <complex>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>
#include "tmm.h"

template<typename T>
using coh_tmm_dict = std::unordered_map<std::string, std::variant<char, T, std::complex<T>, std::valarray<std::complex<T>>, std::vector<std::valarray<std::complex<T>>>>>;

class ValueWarning : public std::runtime_error {
public:
    explicit ValueWarning(const std::string &message) : std::runtime_error(message) {}
};

template<typename T>
static inline auto complex_to_string_with_name(const std::complex<T> c, const std::string &name) -> std::string {
    // QDebug cannot overload << and >> for std::complex
    std::ostringstream ss;
    ss << name << ": " << c.real() << " + " << c.imag() << "i";
    return ss.str();
}

// Reference:
// https://stackoverflow.com/questions/53378000/how-to-access-to-subsequence-of-a-valarray-considering-it-as-a-2d-matrix-in-c
// Using std::slice/std::slice_array is not a good idea
template<typename T, size_t N, size_t M>
class ComplexMatrix {
private:
    std::valarray<T> data;
public:
    ComplexMatrix() : data(N * M) {}
    explicit ComplexMatrix(std::valarray<T> data) : data(data) {}
    ComplexMatrix(std::initializer_list<std::initializer_list<T>> &initList) {
        size_t i = 0;
        for (const auto &row : initList) {
            std::move(row.begin(), row.end(), &data[i * M]);
            ++i;
        }
    }

    class RowProxy {
    private:
        std::valarray<T> &elems;
        size_t row;
    public:
        explicit RowProxy(std::valarray<T>& elems, size_t row) : elems(elems), row(row) {}
        auto operator[](size_t j) -> T & {
            return elems[row * M + j];
        }
    };

    auto operator[](size_t i) -> RowProxy {
        return RowProxy(data, M, i);
    }
    auto operator/(const T& scalar) const -> ComplexMatrix<T, N, M> {
        ComplexMatrix<T, N, M> result;
        result.data = data / scalar;
        return result;
    }

    // For function marked [[nodiscard]]:
    // https://clang.llvm.org/extra/clang-tidy/checks/modernize/use-nodiscard.html
    auto transpose() const -> ComplexMatrix<T, M, N> {
        ComplexMatrix<T, M, N> result;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                result[j][i] = (*this)[i][j];
            }
        }
        return result;
    }
    template<size_t P>
    friend auto dot(const ComplexMatrix<T, N, M> &matrix1,
                    const ComplexMatrix<T, M, P> &matrix2) -> ComplexMatrix<T, N, P> {
        ComplexMatrix<T, N, P> result;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < P; j++) {
                for (size_t k = 0; k < M; k++) {
                    result[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
        }
        return result;
    }
    auto squeeze() const -> std::valarray<std::complex<T>> {
        if (N == 1) {
            return data;
        }
        throw std::logic_error("ComplexMatrix cannot be squeezed");
    }
};

/*
 * If a wave is traveling at angle theta from normal in a medium with index n,
 * calculate whether this is the forward-traveling wave (i.e., the one
 * going from front to back of the stack, like the incoming or outgoing waves,
 * but unlike the reflected wave). For real n & theta, the criterion is simply
 * -pi/2 < theta < pi/2, but for complex n & theta, it's more complicated.
 * See https://arxiv.org/abs/1603.02720 appendix D. If theta is the forward
 * angle, then (pi-theta) is the backward angle and vice-versa.
 */
template<typename T>
auto is_forward_angle(const std::complex<T> n, const std::complex<T> theta) -> bool {
    if (n.real() * n.imag() >= 0) {
        throw std::runtime_error("For materials with gain, it's ambiguous which "
                                 "beam is incoming vs outgoing. See "
                                 "https://arxiv.org/abs/1603.02720 Appendix C.\n" +
                                 complex_to_string_with_name(n, "n") + "\t" +
                                 complex_to_string_with_name(theta, "angle"));
    }
    std::complex<double> ncostheta = n * std::cos(theta);
    bool answer = std::abs(ncostheta.imag()) > 100 * EPSILON ? ncostheta.imag() > 0 : ncostheta.real() > 0;
    if ((answer and (ncostheta.imag() > -100 * EPSILON or
                     ncostheta.real() > -100 * EPSILON or
                     std::real(n * std::cos(std::conj(theta))) > -100 * EPSILON)) or
        (not answer and (ncostheta.imag() < 100 * EPSILON or
                         ncostheta.real() < 100 * EPSILON or
                         std::real(n * std::cos(std::conj(theta))) < 100 * EPSILON))) {
        throw std::runtime_error("It's not clear which beam is incoming vs outgoing. Weird"
                                 " index maybe?\n" +
                                 complex_to_string_with_name(n, "n") + "\t" +
                                 complex_to_string_with_name(theta, "angle"));
    }
    return answer;
}

/*
 * return angle theta in layer 2 with refractive index n_2, assuming
 * it has angle th_1 in layer with refractive index n_1. Use Snell's law. Note
 * that "angles" may be complex!!
 */
template<typename T>
auto snell(const std::complex<T> n_1, const std::complex<T> n_2, const std::complex<T> th_1) -> std::complex<T> {
    std::complex<T> th_2_guess = std::asin(n_1 * std::sin(th_1) / n_2);
    return is_forward_angle(n_2, th_2_guess) ? th_2_guess : M_PI - th_2_guess;
}

/* return list of angle theta in each layer based on angle th_0 in layer 0,
 * using Snell's law. n_list is index of refraction of each layer. Note that
 * "angles" may be complex!!
 */
template<typename T>
auto list_snell(const std::valarray<std::complex<T>> &n_list, const std::complex<T> th_0) -> std::valarray<std::complex<T>> {
    std::valarray<std::complex<T>> angles = std::asin(n_list[0] * std::sin(th_0) / n_list);
    if (not is_forward_angle(n_list[0], angles[0])) {
        angles[0] = M_PI - angles[0];
    }
    if (not is_forward_angle(n_list[n_list.size() - 1], angles[angles.size() - 1])) {
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
template<typename T>
auto interface_r(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i,
            const std::complex<T> th_f) -> std::complex<T> {
    if (polarization == 's') {
        return (n_i * std::cos(th_i) - n_f * std::cos(th_f)) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    }
    if (polarization == 'p') {
        return (n_f * std::cos(th_i) - n_i * std::cos(th_f)) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
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
template<typename T>
auto interface_t(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i,
            const std::complex<T> th_f) -> std::complex<T> {
    if (polarization == 's') {
        return 2 * n_i * std::cos(th_i) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    }
    if (polarization == 'p') {
        return 2 * n_i * std::cos(th_i) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

/*
 * Calculate reflected power R, starting with reflection amplitude r.
 */
template<typename T>
auto R_from_r(const std::complex<T> r) -> T {
    return std::norm(r);
}

/*
 * Calculate transmitted power T, starting with transmission amplitude t.
 *
 * n_i, n_f are refractive indices of incident and final medium.
 *
 * th_i, th_f are (complex) propagation angles through incident & final medium
 * (in radians, where 0=normal). "th" stands for "theta".
 *
 * In the case that n_i, n_f, th_i, th_f are real, formulas simplify to
 * T=|t|^2 * (n_f cos(th_f)) / (n_i cos(th_i)).
 *
 * See manual for discussion of formulas
 */
template<typename T>
auto T_from_t(const char pol, const std::complex<T> t, const std::complex<T> n_i, const std::complex<T> n_f,
              const std::complex<T> th_i, const std::complex<T> th_f) -> T {
    if (pol == 's') {
        return std::abs(t * t) * (n_f * std::cos(th_f)).real() / (n_i * std::cos(th_i)).real();
    }
    if (pol == 'p') {
        return std::abs(t * t) * (n_f * std::conj(std::cos(th_f))).real() / (n_i * std::conj(std::cos(th_i))).real();
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

/*
 * Calculate the power entering the first interface of the stack, starting with
 * reflection amplitude r. Normally this equals 1-R, but in the unusual case
 * that n_i is not real, it can be a bit different from 1-R. See manual.
 *
 * n_i is refractive index of incident medium.
 *
 * th_i is (complex) propagation angle through incident medium
 * (in radians, where 0=normal). "th" stands for "theta".
 */
template<typename T>
auto power_entering_from_r(const char pol, const std::complex<T> r, const std::complex<T> n_i,
                           const std::complex<T> th_i) -> T {
    if (pol == 's') {
        return (n_i * std::cos(th_i) * (1 + std::conj(r)) * (1 - r)).real() / (n_i * std::cos(th_i)).real();
    }
    if (pol == 'p') {
        return (n_i * std::conj(std::cos(th_i)) * (1 + r) * (1 - std::conj(r))).real() /
               (n_i * std::conj(std::cos(th_i))).real();
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

/*
 * Fraction of light intensity reflected at an interface.
 */
template<typename T>
auto interface_R(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i,
            const std::complex<T> th_f) -> T {
    return R_from_r(interface_r(polarization, n_i, n_f, th_i, th_f));
}

/*
 * Fraction of light intensity transmitted at an interface.
 */
template<typename T>
auto interface_T(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f, const std::complex<T> th_i,
            const std::complex<T> th_f) -> T {
    return T_from_t(interface_t(polarization, n_i, n_f, th_i, th_f), n_i, n_f, th_i, th_f);
}

/*
 * Main "coherent transfer matrix method" calc. Given parameters of a stack,
 * calculate everything you could ever want to know about how light
 * propagates in it. (If performance is an issue, you can delete some
 * calculations without affecting the rest.)
 *
 * pol is light polarization, "s" or "p".
 *
 * n_list is the list of refractive indices, in the order that the light would
 * pass through them. The 0'th element of the list should be the semi-infinite
 * medium from which the light enters, the last element should be the semi-infinite
 * medium to which the light exits (if any exits).
 *
 * th_0 is the angle of incidence: 0 for normal, pi/2 for glancing.
 * Remember, for a dissipative incoming medium (n_list[0] is not real), th_0
 * should be complex so that n0 sin(th0) is real (intensity is constant as
 * a function of lateral position).
 *
 * d_list is the list of layer thicknesses (front to back). Should correspond
 * one-to-one with elements of n_list. The first and last elements should be "inf".
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
 * - vw_list-- n'th element is [v_n, w_n], the forward- and backward-traveling
 *   amplitudes, respectively, in the n'th medium just after interface with
 *   (n-1)st medium.
 * - kz_list--normal component of complex angular wave number for
 *   forward-traveling wave in each layer.
 * - th_list--(complex) propagation angle (in radians) in each layer
 * - pol, n_list, d_list, th_0, lam_vac--same as input
 */
template<typename T>
auto coh_tmm(const char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             const std::complex<T> th_0,
             const T lam_vac) -> coh_tmm_dict<T> {
    // Input tests
    if (n_list.size() not_eq d_list.size()) {
        throw std::logic_error("n_list and d_list must have same length");
    }
    if (not isinf(d_list[0]) or not isinf(d_list[d_list.size() - 1])) {
        throw std::runtime_error("d_list must start and end with inf!");
    }
    if (std::abs((n_list.front() * std::sin(th_0)).imag()) < 100 * EPSILON or is_forward_angle(n_list.front(), th_0)) {
        throw std::runtime_error("Error in n0 or th0!");
    }
    size_t num_layers = n_list.size();
    // th_list is a list with, for each layer, the angle that the light travels
    // through the layer. Computed with Snell's law. Note that the "angles" may be
    // complex!
    std::valarray<std::complex<T>> th_list = list_snell(n_list, th_0);
    // kz is the z-component of (complex) angular wave-vector for forward-moving
    // wave. Positive imaginary part means decaying.
    std::valarray<std::complex<T>> kz_list = 2 * M_PI * n_list * std::cos(th_list) / lam_vac;
    // delta is the total phase accrued by traveling through a given layer.
    // Ignore warning about inf multiplication
    std::valarray<std::complex<T>> delta = kz_list * d_list;
    // For a very opaque layer, reset delta to avoid divide-by-0 and similar
    // errors. The criterion imag(delta) > 35 corresponds to single-pass
    // transmission < 1e-30 --- small enough that the exact value doesn't
    // matter.
    for (size_t i = 1; i < num_layers - 1; i++) {
        if (delta[i].imag() > 35) {
            delta[i] = delta[i].real() + 35I;
            throw ValueWarning("Warning: Layers that are almost perfectly opaque "
                               "are modified to be slightly transmissive, "
                               "allowing 1 photon in 10^30 to pass through. It's "
                               "for numerical stability. This warning will not "
                               "be shown again.");
        }
    }
    // t_list[i, j] and r_list[i, j] are transmission and reflection amplitudes,
    // respectively, coming from i, going to j. Only need to calculate this when
    // j=i+1. (2D array is overkill but helps avoid confusion.)
    std::vector<std::vector<std::complex<T>>> t_list(num_layers, std::vector<std::complex<T>>(num_layers));
    std::vector<std::vector<std::complex<T>>> r_list(num_layers, std::vector<std::complex<T>>(num_layers));
    for (size_t i = 0; i < num_layers - 1; i++) {
        t_list[i][i + 1] = interface_t(pol, n_list[i], n_list[i + 1], th_list[i], th_list[i + 1]);
        r_list[i][i + 1] = interface_r(pol, n_list[i], n_list[i + 1], th_list[i], th_list[i + 1]);
    }
    // At the interface between the (n-1)st and nth material, let v_n be the
    // amplitude of the wave on the nth side heading forwards (away from the
    // boundary), and let w_n be the amplitude on the nth side heading backwards
    // (towards the boundary).
    // Then (v_n, w_n) = M_n (v_{n+1}, w_{n+1}).
    // M_n is M_list[n].
    // M_0 and M_{num_layers-1} are not defined.
    // My M is a bit different from Sernelius's, but Mtilde is the same.
    // std::vector<std::array<std::array<std::complex<T>, 2>, 2>> M_list;
    std::vector<ComplexMatrix<T, 2, 2>> M_list;
    for (size_t i = 1; i < num_layers - 1; i++) {
        M_list.push_back(dot(ComplexMatrix({{std::exp(-1I * delta[i]), 0}, {0, std::exp(1I * delta[i])}}),
                                                        ComplexMatrix({{1, r_list[i][i + 1]}, {r_list[i][i + 1], 1}})));
    }
    // std::array<std::array<std::complex<T>, 2>, 2> Mtilde = {{1, 0}, {0, 1}};
    ComplexMatrix<T, 2, 2> Mtilde = {{1, 0}, {0, 1}};
    for (size_t i = 1; i < num_layers - 1; i++) {
        Mtilde = dot(Mtilde, M_list[i]);
    }
    Mtilde = dot(ComplexMatrix({{1, r_list[0][1]}, {r_list[0][1], 1}}) / t_list[0][1], Mtilde);
    // Net complex transmission and reflection amplitudes
    std::complex<T> r = Mtilde[1][0] / Mtilde[0][0];
    std::complex<T> t = 1 / Mtilde[0][0];
    // vw_list[n] = [v_n, w_n]. v_0 and w_0 are undefined because the 0th medium
    // has no left interface.
    // std::vector<std::array<std::complex<T>, 2>> vw_list(num_layers, std::array<std::complex<T>, 2>());
    // std::array<std::array<std::complex<T>, 1>, 2> vw;
    std::vector<std::valarray<std::complex<T>>> vw_list(num_layers, std::valarray<std::complex<T>>());
    ComplexMatrix<T, 2, 1> vw;
    vw_list.back() = vw.transpose().squeeze();
    for (size_t i = num_layers - 2; i > 0; i--) {
        vw = dot(M_list[i], vw);
        vw_list[i] = vw.transpose().squeeze();
    }
    // Net transmitted and reflected power, as a proportion of the incoming light
    // power.
    T R = R_from_r(r);
    T Tr = T_from_t(pol, t, n_list[0], n_list[n_list.size() - 1], th_0, th_list[th_list.size() - 1]);
    T power_entering = power_entering_from_r(pol, r, n_list[0], th_0);
    std::unordered_map<std::string, std::vector<std::complex<T>>> result;
    result["r"] = r;
    result["t"] = t;
    result["R"] = R;
    result["T"] = Tr;
    result["power_entering"] = power_entering;
    result["vw_list"] = vw_list;
    result["kz_list"] = kz_list;
    result["th_list"] = th_list;
    result["pol"] = pol;
    result["n_list"] = n_list;
    result["d_list"] = d_list;
    result["th_0"] = th_0;
    result["lam_vac"] = lam_vac;
    return result;
}

/*
 * Reverses the order of the stack then runs coh_tmm.
 */
template<typename T>
auto coh_tmm_reverse(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                     const std::complex<T> th_0,
                     const T lam_vac) -> coh_tmm_dict<T> {
    std::complex<T> th_f = snell(n_list[0], n_list[n_list.size() - 1], th_0);
    return coh_tmm(pol, std::reverse(n_list.begin(), n_list.end()), std::reverse(d_list.begin(), d_list.end()), th_f,
                   lam_vac);
}

/*
 * Calculates ellipsometric parameters, in radians.
 *
 * Warning: Conventions differ.
 * You may need to subtract pi/2 or whatever.
 */
template<typename T>
auto ellips(std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
            const std::complex<T> th_0,
            const T lam_vac) -> std::unordered_map<std::string, std::variant<T, std::complex<T>>> {
    coh_tmm_dict<T> s_data = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    coh_tmm_dict<T> p_data = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    std::complex<T> rs = s_data["r"];
    std::complex<T> rp = s_data["r"];
    return {{"psi", std::atan(std::abs(rp / rs))}, {"Delta", std::arg(-rp / rs)}};
}

/*
 * Calculates reflected and transmitted power for unpolarized light.
 */
template<typename T>
auto unpolarized_RT(std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                    const std::complex<T> th_0, const T lam_vac) -> T {
    coh_tmm_dict<T> s_data = coh_tmm('s', n_list, d_list, th_0, lam_vac);
    coh_tmm_dict<T> p_data = coh_tmm('p', n_list, d_list, th_0, lam_vac);
    T R = (s_data["R"] + p_data["R"]) / 2;
    T Tr = (s_data["T"] + p_data["T"]) / 2;
    return {{"R", R}, {"T", Tr}};
}

/*
 * Starting with output of coh_tmm(), calculate the Poynting vector,
 * absorbed energy density, and E-field at a specific location.
 * The location is defined by (layer, distance), defined the same way as in
 * find_in_structure_with_inf(...).

 * Returns a dictionary containing:

 * - poyn - the component of Poynting vector normal to the interfaces
 * - absor - the absorbed energy density at that point
 * - Ex and Ey and Ez - the electric field amplitudes, where
 *   z is normal to the interfaces and the light rays are in the x,z plane.

 * The E-field is in units where the incoming |E|=1; see
 * https://arxiv.org/pdf/1603.02720.pdf for formulas.
 */
template<typename T>
auto position_resolved(size_t layer, T distance,
                       const coh_tmm_dict<T> &coh_tmm_data) -> std::unordered_map<std::string, std::variant<T, std::complex<T>>> {
    std::complex<T> v = (layer > 0) ? coh_tmm_data["vw_list"][layer][0] : 1;
    std::complex<T> w = (layer > 0) ? coh_tmm_data["vw_list"][layer][1] : coh_tmm_data["r"];
    std::complex<T> kz = coh_tmm_data["kz_list"][layer];
    std::complex<T> th = coh_tmm_data["th_list"][layer];
    std::complex<T> n = coh_tmm_data["n_list"][layer];
    std::complex<T> n_0 = coh_tmm_data["n_list"][0];
    std::complex<T> th_0 = coh_tmm_data["th_0"];
    char pol = coh_tmm_data["pol"];
    if (not ((layer >= 1 and 0 <= distance and distance <= coh_tmm_data["d_list"][layer]) or (layer == 0 and distance <= 0))) {
        throw std::runtime_error("Position cannot be resolved at layer " + std::to_string(layer));
    }
    // The amplitude of forward-moving wave is Ef, backwards is Eb
    std::complex<T> Ef = v * std::exp(1I * kz * distance);
    std::complex<T> Eb = w * std::exp(-1I * kz * distance);
    // Poynting vector
    T poyn = pol == 's' ? (n * std::cos(th) * std::conj(Ef + Eb) * (Ef - Eb)).real() / (n_0 * std::cos(th_0)).real() :
             (n * std::conj(std::cos(th)) * (Ef + Eb) * std::conj(Ef - Eb)).real() / (n_0 * std::conj(std::cos(th_0))).real();
    // Absorbed energy density
    T absor = pol == 's' ? (n * std::cos(th) * kz * std::pow(std::abs(Ef + Eb), 2)).imag() / (n_0 * std::cos(th_0)).real() :
              (n * std::conj(std::cos(th)) * (kz * std::pow(std::abs(Ef - Eb), 2) - std::conj(kz) * std::pow(std::abs(Ef + Eb), 2))).imag() / (n_0 * std::conj(std::cos(th_0))).real();
    // Electric field
    std::complex<T> Ex = pol == 's' ? 0 : (Ef - Eb) * std::cos(th);
    std::complex<T> Ey = pol == 's' ? Ef + Eb : 0;
    std::complex<T> Ez = pol == 's' ? 0 : -(Ef + Eb) * std::sin(th);
    return {{"poyn", poyn}, {"absor", absor}, {"Ex", Ex}, {"Ey", Ey}, {"Ez", Ez}};
}

/*
 * d_list is a list of thicknesses of layers, all of which are finite.

 * distance is the distance from the front of the whole multilayer structure
 * (i.e., from the start of layer 0.)

 * Function returns [layer, z], where:

 * - layer is what number layer you are at.
 * - z is the distance into that layer.

 * For large distance, layer = len(d_list), even though d_list[layer] does not
 * exist in this case.
 * For negative distance, return [-1, distance]
 */
template<typename T>
auto find_in_structure(const std::valarray<std::complex<T>> &d_list, T distance) -> std::pair<size_t, T> {
    if (std::isinf(d_list.sum())) {
        throw std::runtime_error("This function expects finite arguments");
    }
    if (distance < 0) {
        throw std::runtime_error("Should return [-1, distance]");
    }
    size_t layer = 0;
    while (layer < d_list.size() and distance >= d_list[layer]) {
        distance -= d_list[layer];
        layer++;
    }
    return std::pair(layer, distance);
}

/*
 * d_list is list of thicknesses of layers [inf, blah, blah, ..., blah, inf]

 * distance is the distance from the front of the whole multilayer structure
 * (i.e., from the start of layer 1.)

 * Function returns [layer, z], where:

 * - layer is what number layer you are at.
 * - z is the distance into that layer.

 * For distance < 0, returns [0, distance].
 * So the first interface can be described as either [0,0] or [1,0].
 */
template<typename T>
auto find_in_structure_inf(const std::valarray<std::complex<T>> &d_list, T distance) -> std::pair<size_t, T> {
    if (distance < 0) {
        return std::pair(0, distance);
    }
    std::pair<size_t, T> found = find_in_structure(d_list[std::slice(1, d_list.size() - 2, 1)], distance);
}
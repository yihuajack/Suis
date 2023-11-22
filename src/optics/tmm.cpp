#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include "FixedMatrix.h"
#include "tmm.h"

class ValueWarning : public std::runtime_error {
public:
    explicit ValueWarning(const std::string &message) : std::runtime_error(message) {}
};

enum class LayerType { Coherent, InCoherent };

template<typename T>
static inline auto complex_to_string_with_name(const std::complex<T> c, const std::string &name) -> std::string {
    // QDebug cannot overload << and >> for std::complex
    std::ostringstream ss;
    ss << name << ": " << c.real() << " + " << c.imag() << "i";
    return ss.str();
}

/* numpy.real_if_close(a, tol=100)
 * If input is complex with all imaginary parts close to zero, return real parts.
 *
 * "Close to zero" is defined as `tol` * (machine epsilon of the type for `a`).
 *
 * Parameters
 * ----------
 * a : const std::valarray<std::complex<T1>> &
 *     Input array.
 * tol : T2
 *     Tolerance in machine epsilons for the complex part of the elements in the array.
 *     If the tolerance is <=1, then the absolute tolerance is used.
 *
 * Returns
 * -------
 * out : std::valarray<T2>
 *     If `a` is real, the type of `a` is used for the output.
 *     If `a` has complex elements, the returned type is float.
 */
template<typename T1, typename T2>
auto real_if_close(const std::valarray<T1> &a, T2 tol = TOL) -> std::variant<std::valarray<T1>, std::valarray<T2>> {
    // std::is_same_v<T1, float> or std::is_same_v<T1, double> or std::is_same_v<T1, long double>
    // or any extended floating-point types (std::float16_t, std::float32_t, std::float64_t, std::float128_t,
    // or std::bfloat16_t)(since C++23), including any cv-qualified variants.
    if constexpr (not std::is_same_v<T1, std::complex<T2>> or not std::is_floating_point_v<T2>) {
        return a;
    }
    // whether tol should be T2 or float is a question
    if (tol > 1) {
        tol *= EPSILON<T2>;
    }
    // The parameters of the lambda expression (function) for std::valarray<T>::apply cannot be a reference
    // but can be consts if it is not modified.
    // Note that apply() has to be valarray<T> apply( T func(T) ) const;
    // or valarray<T> apply( T func(const T&) ) const;
    // First, you are not allowed to change the original valarray;
    // second, it is impossible to return a valarray that is not of type T
    // Besides, operands to '?:' cannot have different types.
    if (std::all_of(std::begin(a), std::end(a), [&tol](const T1 &elem) {
        return std::abs(elem.imag()) < tol;
    })) {
        std::valarray<T2> real_part_array(a.size());
        std::transform(std::begin(a), std::end(a), std::begin(real_part_array), [](const std::complex<T2> &c_num) {
            return c_num.real();
        });
        return real_part_array;
    }
    return a;
}

// real_if_close for singleton
// The compiler takes responsibility to match only std::complex<T>
template<typename T>
auto real_if_close(const std::complex<T> &a, T tol = TOL) -> std::variant<T, std::complex<T>> {
    if constexpr (not std::is_floating_point_v<T>) {
        return a;
    }
    if (tol > 1) {
        tol *= EPSILON<T>;
    }
    if (std::abs(a.imag()) < tol) {
        return a.real();
    }
    return a;
}

/*
 * Absorption in a given layer is a pretty simple analytical function:
 * The sum of four exponentials.

 * a(z) = A1*exp(a1*z) + A2*exp(-a1*z)
 *        + A3*exp(1j*a3*z) + conj(A3)*exp(-1j*a3*z)

 * where a(z) is absorption at depth z, with z=0 being the start of the layer,
 * and A1,A2,a1,a3 are real numbers, with a1>0, a3>0, and A3 is complex.
 * The class stores these five parameters, as well as d, the layer thickness.

 * This gives absorption as a fraction of intensity coming towards the first
 * layer of the stack.
 */
template<typename T>
class AbsorpAnalyticFn {
private:
    std::complex<T> d, A3;
    T a1, a3, A1, A2;;
public:
    AbsorpAnalyticFn() = default;

    /*
     * fill in the absorption analytic function starting from coh_tmm_data
     * (the output of coh_tmm), for absorption in the layer with index
     * "layer".
     */
    void fill_in(coh_tmm_dict<T> coh_tmm_data, std::size_t layer);
    /*
     * Calculates absorption at a given depth z, where z=0 is the start of the
     * layer.
     */
    auto run(T z) const -> std::complex<T>;
    /*
     * Flip the function front-to-back, to describe a(d-z) instead of a(z),
     * where d is layer thickness.
     */
    void flip();
    /*
     * multiplies the absorption at each point by "factor".
     */
    void scale(T factor);
    /*
     * adds another compatible absorption analytical function
     */
    AbsorpAnalyticFn add(const AbsorpAnalyticFn &b);
};

template<typename T>
void AbsorpAnalyticFn<T>::fill_in(coh_tmm_dict<T> coh_tmm_data, std::size_t layer) {
    char pol = coh_tmm_data["pol"];
    std::complex<T> v = coh_tmm_data["vw_list"][layer][0];
    std::complex<T> w = coh_tmm_data["vw_list"][layer][1];
    std::complex<T> kz = coh_tmm_data["kz_list"][layer];
    std::complex<T> n = coh_tmm_data["n_list"][layer];
    std::complex<T> n_0 = coh_tmm_data["n_list"][0];
    std::complex<T> th_0 = coh_tmm_data["th_0"];
    std::complex<T> th = coh_tmm_data["th"][layer];
    d = coh_tmm_data["d_list"][layer];

    a1 = (2 * kz).imag();
    a3 = (2 * kz).real();

    if (pol == 's') {
        T temp = (n * std::cos(th) * kz).imag() / (n_0 * std::cos(th_0)).real();
        A1 = temp * std::norm(w);
        A2 = temp * std::norm(v);
        A3 = temp * v * std::conj(w);
    } else {
        T temp = 2 * kz.imag() * (n * std::cos(std::conj(th))).real() / (n_0 * std::conj(std::cos(th_0))).real();
        A1 = temp * std::norm(w);
        A2 = temp * std::norm(v);
        A3 = v * std::conj(w) * -2 * kz.real() * (n * std::cos(std::conj(th))).imag() / (n_0 * std::conj(std::cos(th_0))).real();
    }
}

template<typename T>
auto AbsorpAnalyticFn<T>::run(T z) const -> std::complex<T> {
    return A1 * std::exp(a1 * z) + A2 * std::exp(-a1 * z) + A3 * std::exp(1I * a3 * z) + std::conj(A3) * std::exp(-1I * a3 * z);
}

template<typename T>
void AbsorpAnalyticFn<T>::flip() {
    T newA1 = A2 * std::exp(-a1 * d);
    T newA2 = A1 * std::exp(-a1 * d);
    A1, A2 = newA1, newA2;
    A3 = std::conj(A3 * std::exp(1I * a3 * d));
}

template<typename T>
void AbsorpAnalyticFn<T>::scale(T factor) {
    A1 *= factor;
    A2 *= factor;
    A3 *= factor;
}

template<typename T>
auto AbsorpAnalyticFn<T>::add(const AbsorpAnalyticFn &b) -> AbsorpAnalyticFn<T> {
    if (b.a1 not_eq a1 or b.a3 not_eq a3) {
        throw std::runtime_error("Incompatible absorption analytical functions!");
    }
    A1 += b.A1;
    A2 += b.A2;
    A3 += b.A3;
    return *this;
}

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
    bool answer = std::abs(ncostheta.imag()) > TOL * EPSILON<T> ? ncostheta.imag() > 0 : ncostheta.real() > 0;
    if ((answer and (ncostheta.imag() > -TOL * EPSILON<T> or
                     ncostheta.real() > -TOL * EPSILON<T> or
                     std::real(n * std::cos(std::conj(theta))) > -TOL * EPSILON<T>)) or
        (not answer and (ncostheta.imag() < TOL * EPSILON<T> or
                         ncostheta.real() < TOL * EPSILON<T> or
                         std::real(n * std::cos(std::conj(theta))) < TOL * EPSILON<T>))) {
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

/* return list of angle theta in each layer based on angle th_0 in layer 0, using Snell's law.
 * n_list is index of refraction of each layer.
 * Note that "angles" may be complex!!
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
auto interface_r(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::complex<T> th_i, const std::complex<T> th_f) -> std::complex<T> {
    if (polarization == 's') {
        return (n_i * std::cos(th_i) - n_f * std::cos(th_f)) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    }
    if (polarization == 'p') {
        return (n_f * std::cos(th_i) - n_i * std::cos(th_f)) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<typename T>
auto interface_r(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<std::complex<T>> {
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
auto interface_t(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::complex<T> th_i, const std::complex<T> th_f) -> std::complex<T> {
    if (polarization == 's') {
        return 2 * n_i * std::cos(th_i) / ((n_i * std::cos(th_i) + n_f * std::cos(th_f)));
    }
    if (polarization == 'p') {
        return 2 * n_i * std::cos(th_i) / ((n_f * std::cos(th_i) + n_i * std::cos(th_f)));
    }
    throw std::invalid_argument("Polarization must be 's' or 'p'");
}

template<typename T>
auto interface_t(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<std::complex<T>> {
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

template<typename T>
auto interface_R(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::valarray<std::complex<T>> &th_i, const std::valarray<std::complex<T>> &th_f) -> T {
    // Although solcore's tmm is inherited from an old version of sbyrnes321's tmm,
    // they have independent development routes.
    std::valarray<std::complex<T>> r = interface_r(polarization, n_i, n_f, th_i, th_f);
    // If the outgoing angle is pi/2, that means the light is totally internally reflected, and we can set r = R = 1.
    // If not, can get unphysical results for r.

    // Note that while T actually CAN be > 1 (when you have incidence from an absorbing
    // medium), I[solcore] don't think R can ever be > 1.

    // For a given boolean (val)array mask,
    // std::copy_if(numbers.begin(), numbers.end(), std::back_inserter(result), [&mask, i = 0](const int &)
    // mutable {
    //     return mask[i++];
    // });
    // Binary operation
    std::transform(std::begin(r), std::end(r), std::begin(th_f), std::begin(r),
                   [](std::complex<T> r_value, std::complex<T> th_f_value) {
        return (th_f_value.real() > M_PI / 2 - 1e-6) ? 1 : r_value;
    });
    // An alternative equivalent way
    std::replace_if(std::begin(r), std::end(r), [&th_i, i = 0](const int &) mutable {
        return th_i[i++].real() > M_PI / 2 - 1e-6;
    }, 1);

    return R_from_r(r);
}

/*
 * Fraction of light intensity transmitted at an interface.
 */
template<typename T>
auto interface_T(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::complex<T> th_i, const std::complex<T> th_f) -> T {
    return T_from_t(interface_t(polarization, n_i, n_f, th_i, th_f), n_i, n_f, th_i, th_f);
}

template<typename T>
auto interface_T(const char polarization, const std::complex<T> n_i, const std::complex<T> n_f,
                 const std::complex<T> &th_i, const std::complex<T> &th_f) -> std::valarray<T> {
    std::valarray<std::complex<T>> t = interface_t(polarization, n_i, n_f, th_i, th_f);
    // If the incoming angle is pi/2, that means (most likely) that the light was previously
    // totally internally reflected. That means the light will never reach this interface, and
    // we can safely set t = T = 0; otherwise we get numerical issues which give unphysically large
    // values of T because in T_from_t we divide by cos(th_i) which is ~ 0.
    std::transform(std::begin(t), std::end(t), std::begin(th_i), std::begin(t),
                   [](std::complex<T> t_value, std::complex<T> th_i_value) {
                       return (th_i_value.real() > M_PI / 2 - 1e-6) ? 0 : t_value;
                   });

    return T_from_t(polarization, t, n_i, n_f, th_i, th_f);
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
auto coh_tmm(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             std::complex<T> th_0, const T lam_vac) -> coh_tmm_dict<T> {
    // Input tests
    if (n_list.size() not_eq d_list.size()) {
        throw std::logic_error("n_list and d_list must have same length");
    }
    if (not std::isinf(d_list[0]) or not std::isinf(d_list[d_list.size() - 1])) {
        throw std::runtime_error("d_list must start and end with inf!");
    }
    if (std::abs((n_list.front() * std::sin(th_0)).imag()) < TOL * EPSILON<T> or is_forward_angle(n_list.front(), th_0)) {
        throw std::runtime_error("Error in n0 or th0!");
    }
    std::size_t num_layers = n_list.size();
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
    for (std::size_t i = 1; i < num_layers - 1; i++) {
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
    for (std::size_t i = 0; i < num_layers - 1; i++) {
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
    std::vector<FixedMatrix<std::complex<T>, 2, 2>> M_list;
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        M_list.push_back(dot(FixedMatrix({{std::exp(-1I * delta[i]), 0}, {0, std::exp(1I * delta[i])}}),
                             FixedMatrix({{1, r_list[i][i + 1]}, {r_list[i][i + 1], 1}})));
    }
    // std::array<std::array<std::complex<T>, 2>, 2> Mtilde = {{1, 0}, {0, 1}};
    FixedMatrix<std::complex<T>, 2, 2> Mtilde = {{1, 0}, {0, 1}};
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        Mtilde = dot(Mtilde, M_list[i]);
    }
    Mtilde = dot(FixedMatrix({{1, r_list[0][1]}, {r_list[0][1], 1}}) / t_list[0][1], Mtilde);
    // Net complex transmission and reflection amplitudes
    std::complex<T> r = Mtilde[1][0] / Mtilde[0][0];
    std::complex<T> t = 1 / Mtilde[0][0];
    // vw_list[n] = [v_n, w_n]. v_0 and w_0 are undefined because the 0th medium
    // has no left interface.
    std::vector<std::array<std::complex<T>, 2>> vw_list(num_layers, std::array<std::complex<T>, 2>());
    // std::array<std::array<std::complex<T>, 1>, 2> vw;
    FixedMatrix<std::complex<T>, 2, 1> vw({{t}, {0}});
    vw_list.back() = vw.transpose().squeeze();
    for (std::size_t i = num_layers - 2; i > 0; i--) {
        vw = dot(M_list[i], vw);
        vw_list[i] = vw.transpose().squeeze();
    }
    // Net transmitted and reflected power, as a proportion of the incoming light
    // power.
    T R = R_from_r(r);
    T Tr = T_from_t(pol, t, n_list[0], n_list[n_list.size() - 1], th_0, th_list[th_list.size() - 1]);
    T power_entering = power_entering_from_r(pol, r, n_list[0], th_0);
    return {{"r", r},
            {"t", t},
            {"R", R},
            {"T", Tr},
            {"power_entering", power_entering},
            {"vw_list", vw_list},
            {"kz_list", kz_list},
            {"th_list", th_list},
            {"pol", pol},
            {"n_list", n_list},
            {"d_list", d_list},
            {"th_0", th_0},
            {"lam_vac", lam_vac}};
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
auto ellips(std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list, const std::complex<T> th_0,
            const T lam_vac) -> std::unordered_map<std::string, T> {
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
auto position_resolved(std::size_t layer, T distance,
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
auto find_in_structure(const std::valarray<std::complex<T>> &d_list, T distance) -> std::pair<std::size_t, T> {
    if (std::isinf(d_list.sum())) {
        throw std::runtime_error("This function expects finite arguments");
    }
    if (distance < 0) {
        throw std::runtime_error("Should return [-1, distance]");
    }
    std::size_t layer = 0;
    while (layer < d_list.size() and distance >= d_list[layer]) {
        distance -= d_list[layer];
        layer++;
    }
    return std::pair(layer, distance);
}

/*
 * d_list is a list of thicknesses of layers [inf, blah, blah, ..., blah, inf]

 * distance is the distance from the front of the whole multilayer structure
 * (i.e., from the start of layer 1.)

 * Function returns [layer, z], where:

 * - layer is what number layer you are at.
 * - z is the distance into that layer.

 * For distance < 0, returns [0, distance].
 * So the first interface can be described as either [0,0] or [1,0].
 */
template<typename T>
auto find_in_structure_inf(const std::valarray<std::complex<T>> &d_list, T distance) -> std::pair<std::size_t, T> {
    if (distance < 0) {
        return std::pair(0, distance);
    }
    std::pair<std::size_t, T> found = find_in_structure(d_list[std::slice(1, d_list.size() - 2, 1)], distance);
    return std::pair(found.first + 1, found.second);
}

/*
 * Gives the location of the start of any given layer, relative to the front
 * of the whole multilayer structure.
 * (i.e., the start of layer 1)
 *
 * d_list is a list of thicknesses of layers [inf, blah, blah, ..., blah, inf]
 */
template<typename T>
auto layer_starts(const std::valarray<std::complex<T>> &d_list) -> std::valarray<std::complex<T>> {
    std::valarray<std::complex<T>> final_answer(d_list.size());
    final_answer[0] = -INFINITY;
    final_answer[1] = 0;
    for (std::size_t i = 2; i < d_list.size(); i++) {
        final_answer[i] = final_answer[i - 1] + d_list[i - 1];
    }
}

/*
 * An array listing what proportion of light is absorbed in each layer.
 *
 * Assumes the final layer eventually absorbs all transmitted light.
 *
 * Assumes the initial layer eventually absorbs all reflected light.
 *
 * Entries of the array should sum to 1.
 *
 * coh_tmm_data is output of coh_tmm()
 */
template<typename T>
auto absorp_in_each_layer(const coh_tmm_dict<T> &coh_tmm_data) -> std::valarray<T> {
    std::size_t num_layers = coh_tmm_data["d_list"].size();
    std::valarray<T> power_entering_each_layer(num_layers);
    power_entering_each_layer[0] = 1;
    power_entering_each_layer[1] = coh_tmm_data["power_entering"];
    power_entering_each_layer[num_layers - 1] = coh_tmm_data["T"];
    for (std::size_t i = 2; i < num_layers - 1; i++) {
        power_entering_each_layer[i] = position_resolved(i, 0, coh_tmm_data)["poyn"];
    }
    std::valarray<T> final_answer(num_layers);
    // std::valarray does not have begin() end(), use std::begin() std::end() instead
    std::adjacent_difference(power_entering_each_layer.begin(), power_entering_each_layer.end(), std::begin(final_answer));
    final_answer[num_layers - 1] = power_entering_each_layer[num_layers - 1];
    return final_answer;
}

/*
 * Helper function for inc_tmm. Groups and sorts layer information.

 * See coh_tmm for definitions of n_list, d_list.

 * c_list is "coherency list". Each entry should be 'i' for incoherent or 'c'
 * for 'coherent'.

 * A "stack" is a group of one or more consecutive coherent layers. A "stack
 * index" labels the stacks 0, 1, 2, ... The "within-stack index" counts the
 * coherent layers within stack 1, 2, 3, ... [index 0 is the incoherent layer
 * before the stack starts]

 * An "incoherent layer index" labels the incoherent layers 0, 1, 2, ...

 * An "alllayer index" labels all layers (all elements of d_list) 0, 1, 2, ...

 * Returns info about how the layers relate:

 * - stack_d_list[i] = list of thicknesses of each coherent layer in the i'th
 *   stack, plus starting and ending with "inf"
 * - stack_n_list[i] = list of refractive index of each coherent layer in the
 *   i'th stack, plus the two surrounding incoherent layers
 * - all_from_inc[i] = j means that the layer with incoherent index i has
 *   alllayer index j
 * - inc_from_all[i] = j means that the layer with alllayer index i has
 *   incoherent index j. If j = nan then the layer is coherent.
 * - all_from_stack[i1][i2] = j means that the layer with stack index i1 and
 *   within-stack index i2 has alllayer index j
 * - stack_from_all[i] = [j1 j2] means that the layer with alllayer index i is
 *   part of stack j1 with withinstack-index j2. If stack_from_all[i] = nan
 *   then the layer is incoherent
 * - inc_from_stack[i] = j means that the i'th stack comes after the layer
 *   with incoherent index j, and before the layer with incoherent index j+1.
 * - stack_from_inc[i] = j means that the layer with incoherent index i comes
 *   immediately after the j'th stack. If j=-1, it is not immediately
 *   following a stack.
 * - num_stacks = number of stacks
 * - num_inc_layers = number of incoherent layers
 * - num_layers = number of layers in total
 */
template<typename T>
auto inc_group_layers(const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                      const std::vector<LayerType> &c_list) -> inc_group_layer_dict<T> {
    // isinf() and std::inf() are different
    // isinf() is in "math.h": #define isinf(x) (fpclassify(x) == FP_INFINITE)
    // std::isinf() is in <cmath>: true if infinite, false otherwise
    // (INFINITY, std::numeric_limits<double>::infinity(), std::exp(800), etc.)
    if (not std::isinf(d_list[0]) or not std::isinf(d_list[d_list.size() - 1])) {
        throw std::runtime_error("d_list must start and end with inf!");
    }
    if (c_list.front() not_eq LayerType::InCoherent or c_list.back() not_eq LayerType::InCoherent) {
        throw std::runtime_error("c_list should start and end with Incoherent");
    }
    if (n_list.size() not_eq d_list.size() or d_list.size() not_eq c_list.size()) {
        throw std::logic_error("List sizes do not match!");
    }
    std::size_t inc_index = 0;
    std::size_t stack_index = 0;
    std::vector<std::vector<T>> stack_d_list;
    std::vector<std::vector<std::complex<T>>> stack_n_list;
    std::vector<std::size_t> all_from_inc;
    std::vector<long long int> inc_from_all;
    std::vector<std::vector<std::size_t>> all_from_stack;
    // Although we know the size of the inner vector is 2 if valid, but we want to make it empty if invalid
    std::vector<std::vector<std::size_t>> stack_from_all;
    std::vector<long long int> inc_from_stack;
    std::vector<long long int> stack_from_inc;
    bool stack_in_progress = false;
    std::vector<T> ongoing_stack_d_list;
    std::vector<std::complex<T>> ongoing_stack_n_list;
    std::size_t within_stack_index = 0;
    for (std::size_t alllayer_index = 0; alllayer_index < n_list.size(); alllayer_index++) {
        if (c_list[alllayer_index] == LayerType::Coherent) {  // coherent layer
            // NAN has to be a float, double, or long double
            // Use -1 instead of NAN or std::nan/nanf/nanl
            inc_from_all.push_back(-1);
            if (not stack_in_progress) {  // this layer is starting new stack
                stack_in_progress = true;
                ongoing_stack_d_list = {INFINITY, d_list[alllayer_index]};
                ongoing_stack_n_list = {n_list[alllayer_index - 1], n_list[alllayer_index]};
                stack_from_all.push_back({stack_index, 1});
                all_from_stack.push_back({alllayer_index - 1, alllayer_index});
                inc_from_stack.push_back(inc_index - 1);  // narrowing conversion
                within_stack_index = 1;
            } else {  // another coherent layer in the same stack
                ongoing_stack_d_list.push_back(d_list[alllayer_index]);
                ongoing_stack_n_list.push_back(n_list[alllayer_index]);
                within_stack_index++;
                stack_from_all.push_back({stack_index, within_stack_index});
                all_from_stack.back().push_back(alllayer_index);
            }
        } else if (c_list[alllayer_index] == LayerType::InCoherent) {  // incoherent layer
            stack_from_all.emplace_back();  // push_back({})
            inc_from_all.push_back(inc_index);  // narrowing conversion
            all_from_inc.push_back(alllayer_index);
            if (not stack_in_progress) {  // the previous layer was also incoherent
                stack_from_inc.push_back(-1);
            } else {  // the previous layer was coherent
                stack_in_progress = false;
                stack_from_inc.push_back(stack_index);
                ongoing_stack_d_list.push_back(INFINITY);
                stack_d_list.push_back(ongoing_stack_d_list);
                ongoing_stack_n_list.push_back(n_list[alllayer_index]);
                stack_n_list.push_back(ongoing_stack_n_list);
                all_from_stack.back().push_back(alllayer_index);
                stack_index++;
            }
            inc_index++;
        } else {
            throw std::invalid_argument("Error: c_list entries must be Incoherent or Coherent!");
        }
    }
    return {{"stack_d_list", stack_d_list},
            {"stack_n_list", stack_n_list},
            {"all_from_inc", all_from_inc},
            {"inc_from_all", inc_from_all},
            {"all_from_stack", all_from_stack},
            {"stack_from_all", stack_from_all},
            {"inc_from_stack", inc_from_stack},
            {"stack_from_inc", stack_from_inc},
            {"num_stacks", all_from_stack.size()},
            {"num_inc_layers", all_from_inc.size()},
            {"num_layers", n_list.size()}};
}

/*
 * Incoherent, or partly incoherent partly coherent, transfer matrix method.

 * See coh_tmm for definitions of pol, n_list, d_list, th_0, lam_vac.

 * c_list is "coherency list". Each entry should be 'i' for incoherent or 'c'
 * for 'coherent'.

 * If an incoherent layer has real refractive index (no absorption), then its
 * thickness doesn't affect the calculation results.

 * See https://arxiv.org/abs/1603.02720 for a physics background and some
 * of the definitions.

 * Outputs the following as a dictionary:

 * - R--reflected wave power (as a fraction of incident)
 * - T--transmitted wave power (as a fraction of incident)
 * - VW_list-- the n'th element is [V_n, W_n], the forward- and backward-traveling
 *   intensities, respectively, at the beginning of the n'th incoherent medium.
 * - coh_tmm_data_list--n'th element is coh_tmm_data[n], the output of
 *   the coh_tmm program for the n'th "stack" (group of one or more
 *   consecutive coherent layers).
 * - coh_tmm_bdata_list--n'th element is coh_tmm_bdata[n], the output of the
 *   coh_tmm program for the n'th stack, but with the layers of the stack
 *   in reverse order.
 * - stackFB_list--n'th element is [F, B], where F is light traveling forward
 *   towards the n'th stack and B is light traveling backwards towards the n'th
 *   stack.
 * - num_layers-- total number both coherent and incoherent.
 * - power_entering_list--n'th element is the normalized Poynting vector
 *   crossing the interface into the n'th incoherent layer from the previous
 *   (coherent or incoherent) layer.
 * - Plus, all the outputs of inc_group_layers
 */
template<typename T>
auto inc_tmm(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             const std::vector<LayerType> &c_list, const std::complex<T> th_0,
             const T lam_vac) -> inc_tmm_dict<T> {
    // Input tests
    if (std::holds_alternative<std::complex<T>>(real_if_close(n_list[0] * std::sin(th_0)))) {
        throw std::runtime_error("Error in n0 or th0!");
    }
    inc_group_layer_dict<T> group_layer_data = inc_group_layers(n_list, d_list, c_list);
    std::size_t num_inc_layers = group_layer_data["num_inc_layers"];
    std::size_t num_stacks = group_layer_data["num_stacks"];
    std::vector<std::vector<std::complex<T>>> stack_n_list = group_layer_data["stack_n_list"];
    std::vector<std::vector<T>> stack_d_list = group_layer_data["stack_d_list"];
    std::vector<std::vector<std::size_t>> all_from_stack = group_layer_data["all_from_stack"];
    std::vector<std::size_t> all_from_inc = group_layer_data["all_from_inc"];
    std::vector<long long int> stack_from_inc = group_layer_data["stack_from_inc"];
    std::vector<long long int> inc_from_stack = group_layer_data["inc_from_stack"];

    // th_list is a list with, for each layer, the angle that the light travels through the layer.
    // Computed with Snell's law.
    // Note that the "angles" may be complex!
    std::valarray<std::complex<T>> th_list = list_snell(n_list, th_0);

    // coh_tmm_data_list[i] is the output of coh_tmm for the i'th stack
    std::vector<coh_tmm_dict<T>> coh_tmm_data_list;
    // coh_tmm_bdata_list[i] is the same stack as coh_tmm_data_list[i] but
    // with the order of layers reversed
    std::vector<coh_tmm_dict<T>> coh_tmm_bdata_list;
    for (std::size_t i = 0; i < num_stacks; i++) {
        coh_tmm_data_list.push_back(coh_tmm(pol, stack_n_list[i],
                                            stack_d_list[i],
                                            th_list[all_from_stack[i][0]],
                                            lam_vac));
        coh_tmm_bdata_list.push_back(coh_tmm_reverse(pol, stack_n_list[i],
                                                     stack_d_list[i],
                                                     th_list[all_from_stack[i][0]],
                                                     lam_vac));
    }
    // P_list[i] is a fraction not absorbed in a single pass through i'th incoherent
    // layer.
    std::valarray<std::complex<T>> P_list(num_inc_layers);
    for (std::size_t inc_index = 1; inc_index < num_inc_layers - 1; inc_index++) {  // skip 0'th and last (infinite)
        std::size_t i = all_from_inc[inc_index];
        P_list[inc_index] = std::exp(-4 * M_PI * d_list[i] * (n_list[i] * std::cos(th_list[i])).imag() / lam_vac);
        // For a very opaque layer, reset P to avoid divide-by-0 and similar
        // errors.
        if (P_list[inc_index] < 1e-30) {
            P_list[inc_index] = 1e-30;
        }
    }
    // T_list[i, j] and R_list[i, j] are transmission and reflection powers,
    // respectively, coming from the i'th incoherent layer, going to the j'th incoherent layer.
    // Only need to calculate this when j=i+1 or j=i-1.
    // (2D array is overkill but helps avoid confusion.)
    // initialize these arrays

    // https://stackoverflow.com/questions/17663186/initializing-a-two-dimensional-stdvector
    std::vector<std::vector<T>> T_list(num_inc_layers, std::vector<T>(num_inc_layers));
    std::vector<std::vector<T>> R_list(num_inc_layers, std::vector<T>(num_inc_layers));
    for (std::size_t inc_index = 0; inc_index < num_inc_layers - 1; inc_index++) {  // looking at interface i -> i+1
        std::size_t alllayer_index = all_from_inc[inc_index];
        long long int nextstack_index = stack_from_inc[inc_index + 1];
        if (nextstack_index == -1) {  // the next layer is incoherent
            R_list[inc_index][inc_index + 1] = interface_R(pol, n_list[alllayer_index],
                                                           n_list[alllayer_index + 1],
                                                           th_list[alllayer_index],
                                                           th_list[alllayer_index + 1]);
            T_list[inc_index][inc_index + 1] = interface_T(pol, n_list[alllayer_index + 1],
                                                           n_list[alllayer_index],
                                                           th_list[alllayer_index + 1],
                                                           th_list[alllayer_index]);
            R_list[inc_index + 1][inc_index] = interface_R(pol, n_list[alllayer_index + 1],
                                                           n_list[alllayer_index],
                                                           th_list[alllayer_index + 1],
                                                           th_list[alllayer_index]);
            T_list[inc_index + 1][inc_index] = interface_T(pol, n_list[alllayer_index + 1],
                                                           n_list[alllayer_index],
                                                           th_list[alllayer_index + 1],
                                                           th_list[alllayer_index]);
        } else {  // the next layer is coherent
            R_list[inc_index][inc_index + 1] = coh_tmm_data_list[nextstack_index]["R"];
            T_list[inc_index][inc_index + 1] = coh_tmm_data_list[nextstack_index]["T"];
            R_list[inc_index + 1][inc_index] = coh_tmm_bdata_list[nextstack_index]["R"];
            T_list[inc_index + 1][inc_index] = coh_tmm_bdata_list[nextstack_index]["T"];
        }
    }
    // L is the transfer matrix from the i'th to (i+1)st incoherent layer, see
    // manual
    std::vector<FixedMatrix<T, 2, 2>> L_list{FixedMatrix<std::complex<T>, 2, 2>(NAN)};  // L_0 is not defined because 0'th layer has no beginning.
    FixedMatrix<T, 2, 2> Ltilde = FixedMatrix({{1, -R_list[1][0]}, {R_list[0][1], T_list[1][0] * T_list[0][1] - R_list[1][0] * R_list[0][1]}}) / T_list[0][1];
    for (std::size_t i = 1; i < num_inc_layers - 1; i++) {
        FixedMatrix<T, 2, 2> L = dot(FixedMatrix({{1 / P_list[i]}, {0, P_list[i]}}),
                                     FixedMatrix({{1, -R_list[i + 1][i]}, {R_list[i][i + 1], T_list[i + 1][i] * T_list[i][i + 1] - R_list[i + 1][i] * R_list[i][i + 1]}})) / T_list[i][i + 1];
        L_list.push_back(L);
        Ltilde *= L;
    }
    T Tr = 1 / Ltilde[0][0];
    // Top-level comma expression in array subscript is deprecated in C++20 and unsupported in C++23
    T R = Ltilde[1][0] / Ltilde[0][0];

    // VW_list[n] = [V_n, W_n], the forward- and backward-moving intensities
    // at the beginning of the n'th incoherent layer.
    // VW_list[0] is undefined because 0'th layer has no beginning.

    std::vector<std::array<T, 2>> VW_list(num_inc_layers, std::array<std::complex<T>, 2>());
    VW_list.front() = {NAN, NAN};
    FixedMatrix<T, 2, 1> VW({{Tr}, {0}});
    VW_list.back() = VW.transpose().squeeze();
    for (std::size_t i = num_inc_layers - 2; i > 0; i--) {
        VW = dot(L_list[i], VW);
        VW_list[i] = VW.transpose().squeeze();
    }

    // stackFB_list[n]=[F, B] means that F is light traveling forward towards n'th
    // stack and B is light traveling backwards towards n'th stack.
    // Reminder: inc_from_stack[i] = j means that the i'th stack comes after the
    // layer with incoherent index j.
    std::vector<std::array<T, 2>> stackFB_list;
    for (long long int prev_inc_index : inc_from_stack) {
        T F = (prev_inc_index == 0) ? 1 : VW_list[prev_inc_index][0] * P_list[prev_inc_index];
        T B = VW_list[prev_inc_index + 1][1];
        stackFB_list.push_back(std::array<T, 2>{F, B});
    }
    // power_entering_list[i] is the normalized Poynting vector crossing the
    // interface into the i'th incoherent layer from the previous (coherent or incoherent) layer.
    // See the manual.
    std::vector<T> power_entering_list{1};  // "1" by convention for infinite 0th layer.
    for (std::size_t i = 1; i < num_inc_layers; i++) {
        long long int prev_stack_index = stack_from_inc[i];
        if (prev_stack_index == -1) {
            // case where this layer directly follows another incoherent layer
            power_entering_list.push_back(i == 1 ? T_list[0][1] - VW_list[1][1] * T_list[1][0] :  // special case because VW_list[0] & A_list[0] are undefined
                                                   VW_list[i - 1][0] * P_list[i - 1] * T_list[i - 1][i] - VW_list[i][1] * T_list[i][i - 1]);
        } else {  // case where this layer follows a coherent stack
            power_entering_list.push_back(stackFB_list[prev_stack_index][0] * coh_tmm_data_list[prev_stack_index]["T"] - stackFB_list[prev_stack_index][1] * coh_tmm_bdata_list[prev_stack_index]["power_entering"]);
        }
    }
    // Since group_layer_data does not have duplicate keys, the merging order does not matter.
    return group_layer_data.merge({{"T", Tr},
            {"R", R},
            {"VW_list", VW_list},
            {"coh_tmm_data_list", coh_tmm_data_list},
            {"coh_tmm_bdata_list", coh_tmm_bdata_list},
            {"stackFB_list", stackFB_list},
            {"power_entering_list", power_entering_list}});
}

/*
 * A list saying what proportion of light is absorbed in each layer.
 * Assumes all reflected light is eventually absorbed in the 0'th medium, and
 * all transmitted light is eventually absorbed in the final medium.
 * Returns a list [layer0absorp, layer1absorp, ...].
 * Entries should sum to 1.
 * inc_data is output of incoherent_main()
 */
template<typename T>
auto inc_absorp_in_each_layer(const inc_tmm_dict<T> &inc_data) {
    // Reminder: inc_from_stack[i] = j means that the i'th stack comes after the
    // layer with incoherent index j.
    // Reminder: stack_from_inc[i] = j means that the layer
    // with incoherent index i comes immediately after the j'th stack (or j=nan
    // if it's not immediately following a stack).

    std::vector<long long int> stack_from_inc = inc_data["stack_from_inc"];
    std::vector<T> power_entering_list = inc_data["power_entering_list"];
    // stackFB_list[n]=[F, B] means that F is light traveling forward towards n'th
    // stack and B is light traveling backwards towards n'th stack.
    std::vector<std::array<T, 2>> stackFB_list = inc_data["stackFB_list"];
    std::vector<T> absorp_list;

    // loop through incoherent layers, excluding the final layer
    for (std::size_t i = 0; i < power_entering_list.size() - 1; i++) {
        if (stack_from_inc[i + 1] == -1) {
            // case that incoherent layer i is right before another incoherent layer
            absorp_list.push_back(power_entering_list[i] - power_entering_list[i + 1]);
        } else {  // incoherent layer i is immediately before a coherent stack
            std::size_t j = stack_from_inc[i + 1];
            coh_tmm_dict<T> coh_tmm_data = inc_data["coh_tmm_data_list"][j];
            coh_tmm_dict<T> coh_tmm_bdata = inc_data["coh_tmm_bdata_list"][j];
            // First, power in the incoherent layer...
            T power_exiting = stackFB_list[j][0] * coh_tmm_data["power_entering"] - stackFB_list[j][1] * coh_tmm_bdata["T"];
            absorp_list.push_back(power_entering_list[i] - power_exiting);
            // Next, power in the coherent stack...
            std::valarray<T> coh_absorp = stackFB_list[j][0] * absorp_in_each_layer(coh_tmm_data);
            std::valarray<T> rcoh_absorp = stackFB_list[j][1] * absorp_in_each_layer(coh_tmm_bdata);
            std::valarray<T> stack_absorp = coh_absorp[std::slice(1, coh_absorp.size() - 2, 1)] + rcoh_absorp[std::slice(rcoh_absorp.size() - 2, rcoh_absorp.size() - 2, -1)];
            // absorp_list.insert(absorp_list.end(), std::begin(stack_absorp), std::end(stack_absorp));
            // At the time of writing, the C++23 feature "Constructing containers and strings from ranges with
            // ranges::to(), tagged constructors, insert and assign member functions"
            // [P1206R7](https://wg21.link/P1206R7)
            // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1206r7.pdf
            // https://github.com/microsoft/STL/pull/2806
            // is supported by Clang libc++ 17 and MSVC STL 19.34,
            // but is not supported by GCC libstdc++ 13.2.0 yet.
#ifdef __cpp_lib_containers_ranges
            absorp_list.append_range(stack_absorp);
#else
            absorp_list.insert(absorp_list.end(), std::begin(stack_absorp), std::end(stack_absorp));
#endif
        }
    }
    // final semi-infinite layer
    absorp_list.push_back(inc_data["T"]);
    return absorp_list;
}

/*
 * Outputs an absorp_analytic_fn object for a coherent layer within a
 * partly incoherent stack.

 * inc_data is output of incoherent_main()
 */
template<typename T>
auto inc_find_absorp_analytic_fn(std::size_t layer, const inc_tmm_dict<T> &inc_data) -> AbsorpAnalyticFn<T> {
    std::vector<std::size_t> j = inc_data["stack_from_all"][layer];
    if (j.empty()) {
        throw std::runtime_error("Layer must be coherent for this function!");
    }
    // Recall that for incoherent layers stack_from_all[layer] is empty.
    // For coherent layers stack_from_all[layer] the size is 2.
    std::size_t stackindex = j.front();
    std::size_t withinstackindex = j.back();
    AbsorpAnalyticFn<T> forwardfunc;
    forwardfunc.fill_in(inc_data["coh_tmm_data_list"][stackindex], withinstackindex);
    forwardfunc.scale(inc_data["stackFB_list"][stackindex][0]);
    AbsorpAnalyticFn<T> backfunc;
    backfunc.fill_in(inc_data["coh_tmm_bdata_list"][stackindex], -1 - withinstackindex);
    backfunc.scale(inc_data["stackFB_list"][stackindex][1]);
    backfunc.flip();
    return forwardfunc.add(backfunc);
}

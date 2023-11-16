#include <algorithm>
#include <array>
#include <complex>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>
#include "tmm.h"

template<typename T>
using coh_tmm_dict = std::unordered_map<std::string, std::variant<char, T, std::complex<T>, std::valarray<std::complex<T>>, std::vector<std::array<std::complex<T>, 2>>>>;
template<typename T>
using inc_group_layer_dict = std::unordered_map<std::string, std::variant<std::vector<std::vector<T>>, std::vector<std::vector<std::complex<T>>>, std::vector<std::size_t>, std::vector<long long int>, std::vector<std::vector<std::size_t>>, std::size_t>>;

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
    for (const T1 &elem : a) {
        if (std::abs(a.imag()) < tol) {
            return a.apply([](T1 x) -> T2
                {
                    return x.real();
                });
        }
    }
    return std::all_of(std::begin(a), std::end(a), [&tol](const T1 &elem) { return std::abs(elem.imag()) < tol; }) ;
}

template<typename T, std::size_t N, std::size_t M>
class ComplexMatrix {
private:
    std::array<std::array<T, M>, N> data;
public:
    ComplexMatrix() : data(N, std::array<T, M>()) {}
    ComplexMatrix(const ComplexMatrix &other) : data(other.data) {}
    ComplexMatrix(ComplexMatrix &&other) noexcept : data(std::move(other.data)) {}
    explicit ComplexMatrix(const std::array<std::array<T, M>, N> &data) : data(data) {}
    explicit ComplexMatrix(std::array<std::array<T, M>, N> &&data) noexcept : data(std::move(data)) {}
    // Different from traditional C-style arrays that can be brace-initialized by {{1, 0}, {0, 1}}
    // 2D std::array has to be brace-initialized by {{{1, 0}, {0, 1}}} or {1, 0, 0, 1}
    // https://stackoverflow.com/questions/12844475/why-cant-simple-initialize-with-braces-2d-stdarray
    // Distinguish std::initializer_list<T> and list-initialization
    // Note that std::initializer_list<T> uses copy-semantics by holding its values as const objects, see
    // https://stackoverflow.com/questions/24109445/stdinitializer-list-and-reference-types
    // Since std::initializer_list may be implemented as a pair of pointers or pointer and length,
    // it is often passed by value, see
    // https://stackoverflow.com/questions/17803475/why-is-stdinitializer-list-often-passed-by-value
    // init_list and row must not be a reference; otherwise,
    // error: cannot bind non-const lvalue reference of type 'std::initializer_list<std::initializer_list<int> >&'
    // to an rvalue of type 'std::initializer_list<std::initializer_list<int> >'
    // error: binding reference of type 'std::initializer_list<int>&'
    // to 'const std::initializer_list<int>' discards qualifiers
    // Hence, we cannot distinguish the move and copy constructor or assignment operator
    // like std::copy(row.begin(), row.end(), data[i].begin());
    // A pitfall is that 2D std::array behaves like traditional 2D C-style array that when moving or copying an
    // initializer list or a vector to the array, the extra element will be added to the next sub-array.
    // Thus, it is encouraged to use the array<array> constructor,
    // as it will alert error: too many initializers for arrays like {0, 1, 2, 3, 4} or {{0, 1}, {2, 3, 4}}.
    // ComplexMatrix<int, 2, 2> = {1, 0, 0, 1} will construct by the array<array> [MOVING] constructor;
    // others will construct by the initializer_list<initializer_list> constructor due to automatic brace elision.
    ComplexMatrix([[maybe_unused]] std::initializer_list<std::initializer_list<T>> init_list) {
        std::size_t i = 0;
        for (std::initializer_list<T> row : init_list) {
            std::move(row.begin(), row.end(), data[i].begin());
            ++i;
        }
    }
    ~ComplexMatrix() = default;

    class RowProxy {
    private:
        std::array<T, M> &row;
    public:
        explicit RowProxy(std::array<T, M> &row) : row(row) {}
        auto operator[](std::size_t j) -> T & {
            return row[j];
        }
    };

    auto operator[](std::size_t i) -> RowProxy {
        return RowProxy(data[i]);
    }
    auto operator/(const T& scalar) const -> ComplexMatrix<T, N, M> {
        ComplexMatrix<T, N, M> result;
        result.data = data / scalar;
        return result;
    }
    auto operator=(const ComplexMatrix &other) -> ComplexMatrix& {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }
    auto operator=(ComplexMatrix &&other) noexcept -> ComplexMatrix& {
        if (this != &other) {
            data = std::move(other.data);
        }
        return *this;
    }
    auto operator=(const std::array<std::array<T, M>, N> &data_) -> ComplexMatrix& {
        if (data != &data_) {
            data = data_;
        }
        return *this;
    }
    auto operator=(std::array<std::array<T, M>, N> &&data_) noexcept -> ComplexMatrix& {
        if (data != &data_) {
            data = std::move(data_);
        }
        return *this;
    }
    auto operator=([[maybe_unused]] std::initializer_list<std::initializer_list<T>> init_list) -> ComplexMatrix& {
        std::size_t i = 0;
        for (std::initializer_list<T> row : init_list) {
            std::move(row.begin(), row.end(), data[i].begin());
            ++i;
        }
    }

    // For function marked [[nodiscard]]:
    // https://clang.llvm.org/extra/clang-tidy/checks/modernize/use-nodiscard.html
    auto transpose() const -> ComplexMatrix<T, M, N> {
        ComplexMatrix<T, M, N> result;
        for (std::size_t i = 0; i < N; i++) {
            for (std::size_t j = 0; j < M; j++) {
                result[j][i] = (*this)[i][j];
            }
        }
        return result;
    }
    template<std::size_t P>
    friend auto dot(const ComplexMatrix<T, N, M> &matrix1,
                    const ComplexMatrix<T, M, P> &matrix2) -> ComplexMatrix<T, N, P> {
        ComplexMatrix<T, N, P> result;
        for (std::size_t i = 0; i < N; i++) {
            for (std::size_t j = 0; j < P; j++) {
                for (std::size_t k = 0; k < M; k++) {
                    result[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
        }
        return result;
    }
    auto squeeze() const -> std::array<std::complex<T>, M> {
        if (N == 1) {
            return data[0];
        }
        throw std::logic_error("ComplexMatrix cannot be squeezed");
    }
};

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
    /*
     * fill in the absorption analytic function starting from coh_tmm_data
     * (the output of coh_tmm), for absorption in the layer with index
     * "layer".
     */
    void fill_in(coh_tmm_dict<T> coh_tmm_data, std::size_t layer) {
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
    /*
     * Calculates absorption at a given depth z, where z=0 is the start of the
     * layer.
     */
    auto run(T z) const -> std::complex<T> {
        return A1 * std::exp(a1 * z) + A2 * std::exp(-a1 * z) + A3 * std::exp(1I * a3 * z) + std::conj(A3) * std::exp(-1I * a3 * z);
    }
    /*
     * Flip the function front-to-back, to describe a(d-z) instead of a(z),
     * where d is layer thickness.
     */
    void flip() {
        T newA1 = A2 * std::exp(-a1 * d);
        T newA2 = A1 * std::exp(-a1 * d);
        A1, A2 = newA1, newA2;
        A3 = std::conj(A3 * std::exp(1I * a3 * d));
    }
    /*
     * multiplies the absorption at each point by "factor".
     */
    void scale(T factor) {
        A1 *= factor;
        A2 *= factor;
        A3 *= factor;
    }
    /*
     * adds another compatible absorption analytical function
     */
    void add(const AbsorpAnalyticFn &b) {
        if (b.a1 not_eq a1 or b.a3 not_eq a3) {
            throw std::runtime_error("Incompatible absorption analytical functions!");
        }
        A1 += b.A1;
        A2 += b.A2;
        A3 += b.A3;
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
    std::vector<ComplexMatrix<T, 2, 2>> M_list;
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        M_list.push_back(dot(ComplexMatrix({{std::exp(-1I * delta[i]), 0}, {0, std::exp(1I * delta[i])}}),
                                                        ComplexMatrix({{1, r_list[i][i + 1]}, {r_list[i][i + 1], 1}})));
    }
    // std::array<std::array<std::complex<T>, 2>, 2> Mtilde = {{1, 0}, {0, 1}};
    ComplexMatrix<T, 2, 2> Mtilde = {{1, 0}, {0, 1}};
    for (std::size_t i = 1; i < num_layers - 1; i++) {
        Mtilde = dot(Mtilde, M_list[i]);
    }
    Mtilde = dot(ComplexMatrix({{1, r_list[0][1]}, {r_list[0][1], 1}}) / t_list[0][1], Mtilde);
    // Net complex transmission and reflection amplitudes
    std::complex<T> r = Mtilde[1][0] / Mtilde[0][0];
    std::complex<T> t = 1 / Mtilde[0][0];
    // vw_list[n] = [v_n, w_n]. v_0 and w_0 are undefined because the 0th medium
    // has no left interface.
    std::vector<std::array<std::complex<T>, 2>> vw_list(num_layers, std::array<std::complex<T>, 2>());
    // std::array<std::array<std::complex<T>, 1>, 2> vw;
    ComplexMatrix<T, 2, 1> vw;
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
 * d_list is list of thicknesses of layers [inf, blah, blah, ..., blah, inf]
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
 * coherent layers within the stack 1, 2, 3, ... [index 0 is the incoherent layer
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
 *   immediately after the j'th stack. If j=nan, it is not immediately
 *   following a stack.
 * - num_stacks = number of stacks
 * - num_inc_layers = number of incoherent layers
 * - num_layers = number of layers in total
 */
template<typename T>
auto inc_group_layer(const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
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
    std::vector<std::size_t> stack_from_inc;
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
                stack_from_inc.push_back(stack_index);
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
 * Incoherent, or partly-incoherent-partly-coherent, transfer matrix method.

 * See coh_tmm for definitions of pol, n_list, d_list, th_0, lam_vac.

 * c_list is "coherency list". Each entry should be 'i' for incoherent or 'c'
 * for 'coherent'.

 * If an incoherent layer has real refractive index (no absorption), then its
 * thickness doesn't affect the calculation results.

 * See https://arxiv.org/abs/1603.02720 for physics background and some
 * of the definitions.

 * Outputs the following as a dictionary:

 * - R--reflected wave power (as fraction of incident)
 * - T--transmitted wave power (as fraction of incident)
 * - VW_list-- n'th element is [V_n,W_n], the forward- and backward-traveling
 *   intensities, respectively, at the beginning of the n'th incoherent medium.
 * - coh_tmm_data_list--n'th element is coh_tmm_data[n], the output of
 *   the coh_tmm program for the n'th "stack" (group of one or more
 *   consecutive coherent layers).
 * - coh_tmm_bdata_list--n'th element is coh_tmm_bdata[n], the output of the
 *   coh_tmm program for the n'th stack, but with the layers of the stack
 *   in reverse order.
 * - stackFB_list--n'th element is [F,B], where F is light traveling forward
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
             const std::vector<LayerType> &c_list, const std::complex<T> th_0, const T lam_vac) {
    // Input tests

}

#ifndef TMM_H
#define TMM_H

#include <array>
#include <complex>
#include <limits>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>

constexpr float TOL=100;
// Python sys.float_info.epsilon is DBL_EPSILON
// For C++, epsilon can be false, 0, FLT_EPSILON, DBL_EPSILON, LDBL_EPSILON, or custom T()
template<typename T>
constexpr T EPSILON=std::numeric_limits<T>::epsilon();

// size_t for C "stddef.h"; std::size_t for C++ <cstddef>
// For local development environment
// size_t for "corecrt.h"; std::size_t for "c++config.h"
// __MINGW_EXTENSION typedef unsigned __int64 size_t; ifdef _WIN64
// typedef __SIZE_TYPE__ 	size_t; in namespace std
// https://stackoverflow.com/questions/42797279/what-to-do-with-size-t-vs-stdsize-t
// using std::size_t;

template<typename T>
using coh_tmm_dict = std::unordered_map<std::string, std::variant<char, T, std::complex<T>, std::valarray<T>, std::valarray<std::complex<T>>, std::vector<std::array<std::complex<T>, 2>>>>;
template<typename T>
using inc_group_layer_dict = std::unordered_map<std::string, std::variant<std::size_t, std::vector<std::size_t>, std::vector<long long int>, std::vector<std::vector<T>>, std::vector<std::vector<std::complex<T>>>, std::vector<std::vector<std::size_t>>>>;
template<typename T>
using inc_tmm_dict = std::unordered_map<std::string, std::variant<T, std::size_t, std::vector<std::size_t>, std::vector<long long int>, std::vector<T>, std::vector<std::vector<T>>, std::vector<std::vector<std::complex<T>>>, std::vector<std::vector<std::size_t>>, std::vector<std::array<T, 2>>, std::vector<coh_tmm_dict<T>>>>;

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
    std::complex<T> A3;
    T a1, a3, A1, A2, d;
public:
    AbsorpAnalyticFn() = default;

    /*
     * fill in the absorption analytic function starting from coh_tmm_data
     * (the output of coh_tmm), for absorption in the layer with index
     * "layer".
     */
    void fill_in(const coh_tmm_dict<T> &coh_tmm_data, std::size_t layer);
    /*
     * Calculates absorption at a given depth z, where z=0 is the start of the
     * layer.
     */
    auto run(T z) const -> std::complex<T>;
    /*
     * Flip the function front-to-back, to describe a(d-z) instead of a(z),
     * where d is layer thickness.
     */
    auto flip() -> AbsorpAnalyticFn;
    /*
     * multiplies the absorption at each point by "factor".
     */
    void scale(T factor);
    /*
     * adds another compatible absorption analytical function
     */
    auto add(const AbsorpAnalyticFn &b) -> AbsorpAnalyticFn;
};

template<typename T>
auto snell(std::complex<T> n_1, std::complex<T> n_2, std::complex<T> th_1) -> std::complex<T>;

template<typename T>
auto coh_tmm(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             std::complex<T> th_0, T lam_vac) -> coh_tmm_dict<T>;

template<typename T>
auto ellips(const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list, std::complex<T> th_0,
            T lam_vac) -> std::unordered_map<std::string, T>;

template<typename T>
auto position_resolved(std::size_t layer, T distance,
                       const coh_tmm_dict<T> &coh_tmm_data) -> std::unordered_map<std::string, std::variant<T, std::complex<T>>>;

template<typename T>
auto absorp_in_each_layer(const coh_tmm_dict<T> &coh_tmm_data) -> std::valarray<T>;

#endif // TMM_H

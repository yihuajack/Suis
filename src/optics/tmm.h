#ifndef TMM_H
#define TMM_H

#include <array>
#include <complex>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>

/*
 * r: std::complex<T>
 * t: std::complex<T>
 * R: T
 * T: T
 * power_entering: T
 * vw_list: std::vector<std::array<std::complex<T>, 2>>
 * kz_list: std::valarray<std::complex<T>>
 * th_list: std::valarray<std::complex<T>>
 * pol: char
 * n_list: std::valarray<std::complex<T>>
 * d_list: std::valarray<T>
 * th_0: std::complex<T>
 * lam_vac: T
 */
template<typename T>
using coh_tmm_dict = std::unordered_map<std::string, std::variant<char, T, std::complex<T>, std::valarray<T>,
        std::valarray<std::complex<T>>, std::vector<std::array<std::complex<T>, 2>>>>;
/*
 * r: std::complex<T>
 * t: std::complex<T>
 * R: T
 * T: T
 * power_entering: T
 * vw_list: std::vector<std::array<std::complex<T>, 2>>
 * kz_list: std::valarray<std::complex<T>>
 * th_list: std::valarray<std::complex<T>>
 * pol: char
 * n_list: std::vector<std::complex<T>>
 * d_list: std::vector<T>
 * th_0: std::complex<T>
 * lam_vac: T
 */
template<typename T>
using stack_coh_tmm_dict = std::unordered_map<std::string, std::variant<char, T, std::complex<T>, std::vector<T>,
        std::vector<std::complex<T>>, std::valarray<std::complex<T>>, std::vector<std::array<std::complex<T>, 2>>>>;
/*
 * stack_d_list: std::vector<std::vector<T>>
 * stack_n_list: std::vector<std::vector<std::complex<T>>>
 * all_from_inc: std::vector<std::size_t>
 * inc_from_all: std::vector<std::ptrdiff_t>
 * all_from_stack: std::vector<std::vector<std::size_t>>
 * stack_from_all: std::vector<std::vector<std::size_t>>
 * inc_from_stack: std::vector<std::ptrdiff_t>
 * stack_from_inc: std::vector<std::ptrdiff_t>
 * num_stacks: std::size_t
 * num_inc_layers: std::size_t
 * num_layers: std::size_t
 * T: T
 * R: T
 * VW_list: std::valarray<std::array<T, 2>>
 * coh_tmm_data_list: std::vector<stack_coh_tmm_dict<T>>
 * coh_tmm_bdata_list: std::vector<coh_tmm_dict<T>>
 * stackFB_list: std::valarray<std::array<T, 2>>
 * power_entering_list: std::vector<T>
 */
template<typename T>
using inc_tmm_dict = std::unordered_map<std::string, std::variant<T, std::size_t, std::vector<std::size_t>,
        std::vector<std::ptrdiff_t>, std::vector<T>, std::vector<coh_tmm_dict<T>>, std::vector<stack_coh_tmm_dict<T>>,
        std::vector<std::vector<T>>, std::vector<std::vector<std::complex<T>>>, std::vector<std::vector<std::size_t>>,
        std::valarray<std::array<T, 2>>>>;
/*
 * r: std::valarray<std::complex<T>>
 * t: std::valarray<std::complex<T>>
 * R: std::valarray<T>
 * T: std::valarray<T>
 * power_entering: std::valarray<T>
 * vw_list: std::valarray<std::vector<std::array<std::complex<T>, 2>>>
 * kz_list: std::valarray<std::complex<T>>
 * th_list: std::valarray<std::complex<T>>
 * pol: char
 * n_list: std::valarray<std::complex<T>>
 * d_list: std::valarray<T>
 * th_0: std::complex<T>
 * lam_vac: std::valarray<T>
 */
template<typename T>
using coh_tmm_vec_dict = std::unordered_map<std::string, std::variant<char, std::complex<T>, std::valarray<T>,
        std::valarray<std::complex<T>>, std::valarray<std::vector<std::array<std::complex<T>, 2>>>>>;
/*
 * r: std::valarray<std::complex<T>>
 * t: std::valarray<std::complex<T>>
 * R: std::valarray<T>
 * T: std::valarray<T>
 * power_entering: std::valarray<T>
 * vw_list: std::valarray<std::vector<std::array<std::complex<T>, 2>>>
 * kz_list: std::vector<std::valarray<std::complex<T>>>
 * th_list: std::vector<std::valarray<std::complex<T>>>
 * pol: char
 * n_list: std::vector<std::valarray<std::complex<T>>>
 * d_list: std::vector<T>
 * th_0: std::complex<T>/std::valarray<std::complex<T>>
 * lam_vac: std::valarray<T>
 */
template<typename T>
using coh_tmm_vecn_dict = std::unordered_map<std::string, std::variant<char, std::complex<T>, std::valarray<T>,
        std::vector<T>, std::valarray<std::complex<T>>, std::vector<std::valarray<std::complex<T>>>,
        std::valarray<std::vector<std::array<std::complex<T>, 2>>>>>;
/*
 * stack_d_list: std::vector<std::vector<T>>
 * stack_n_list: std::vector<std::vector<std::valarray<std::complex<T>>>>
 * all_from_inc: std::vector<std::size_t>
 * inc_from_all: std::vector<std::ptrdiff_t>
 * all_from_stack: std::vector<std::vector<std::size_t>>
 * stack_from_all: std::vector<std::vector<std::size_t>>
 * inc_from_stack: std::vector<std::ptrdiff_t>
 * stack_from_inc: std::vector<std::ptrdiff_t>
 * num_stacks: std::size_t
 * num_inc_layers: std::size_t
 * num_layers: std::size_t
 * Tr: std::valarray<T>
 * R: std::valarray<T>
 * VW_list: std::valarray<std::array<std::valarray<T>, 2>>
 * coh_tmm_data_list: std::vector<coh_tmm_vecn_dict<T>>
 * coh_tmm_bdata_list: std::vector<coh_tmm_vecn_dict<T>>
 * stackFB_list: std::valarray<std::array<std::valarray<T>, 2>>
 * power_entering_list: std::vector<std::valarray<T>>
 */
template<typename T>
using inc_tmm_vec_dict = std::unordered_map<std::string, std::variant<std::valarray<T>, std::size_t,
        std::vector<std::size_t>, std::vector<std::ptrdiff_t>, std::vector<std::valarray<T>>,
        std::vector<coh_tmm_vecn_dict<T>>, std::vector<std::vector<T>>, std::vector<std::vector<std::size_t>>,
        std::vector<std::vector<std::valarray<std::complex<T>>>>, std::valarray<std::array<std::valarray<T>, 2>>>>;

enum class LayerType { Coherent, Incoherent };

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

/*
 * This function (specifically, 'run') is vectorized.
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
class AbsorpAnalyticVecFn {
    std::valarray<std::complex<T>> A3;
    std::valarray<T> a1, a3, A1, A2;
    // Warning: the size of d may be 1 filled by coh_tmm_vecn_dict, different from the other 4.
    // The behavior of operator_arith3 is undefined when the two arguments are valarrays with different sizes.
    std::variant<T, std::valarray<T>> d;
public:
    AbsorpAnalyticVecFn() = default;

    AbsorpAnalyticVecFn(const AbsorpAnalyticVecFn<T> &other);

    /*
     * fill in the absorption analytic function starting from coh_tmm_data
     * (the output of coh_tmm), for absorption in the layer with index
     * "layer".
     */
    void fill_in(const coh_tmm_vec_dict<T> &coh_tmm_data, const std::valarray<std::ptrdiff_t> &layer);
    void fill_in(const coh_tmm_vecn_dict<T> &coh_tmm_data, std::ptrdiff_t layer);
    /*
     * Calculates absorption at a given depth z, where z=0 is the start of the
     * layer.
     */
    auto run(T z) const -> std::valarray<std::complex<T>>;
    auto run(const std::valarray<T> &z) const -> std::valarray<std::valarray<std::complex<T>>>;
    /*
     * Flip the function front-to-back, to describe a(d-z) instead of a(z),
     * where d is layer thickness.
     */
    void flip();
    /*
     * multiplies the absorption at each point by "factor".
     */
    template<typename FAC_T>
    requires std::is_same_v<FAC_T, T> or std::is_same_v<std::remove_cvref_t<FAC_T>, std::valarray<T>>
    void scale(FAC_T &&factor);
    /*
     * adds another compatible absorption analytical function
     */
    void add(const AbsorpAnalyticVecFn &b);

    friend void test_fill_in_s();
    friend void test_fill_in_p();
    friend void test_copy();
    friend void test_run_array();
    friend void test_run();
    friend void test_scale();
    friend void test_add();
    friend void test_add_exception();
    friend void test_inc_find_absorp_analytic_fn();
};

template<std::floating_point T>
auto is_forward_angle(std::complex<T> n, std::complex<T> theta) -> bool;

template<std::floating_point T>
auto snell(std::complex<T> n_1, std::complex<T> n_2, std::complex<T> th_1) -> std::complex<T>;

template<std::floating_point T>
auto list_snell(const std::valarray<std::complex<T>> &n_list, std::complex<T> th_0) -> std::valarray<std::complex<T>>;

template<typename T>
auto interface_r(char polarization, std::complex<T> n_i, std::complex<T> n_f, std::complex<T> th_i,
                 std::complex<T> th_f) -> std::complex<T>;

template<typename T>
auto interface_t(char polarization, std::complex<T> n_i, std::complex<T> n_f, std::complex<T> th_i,
                 std::complex<T> th_f) -> std::complex<T>;

template<typename T>
auto R_from_r(std::complex<T> r) -> T;

template<typename T>
auto T_from_t(char pol, std::complex<T> t, std::complex<T> n_i, std::complex<T> n_f, std::complex<T> th_i,
              std::complex<T> th_f) -> T;

template<typename T>
auto power_entering_from_r(char pol, std::complex<T> r, std::complex<T> n_i, std::complex<T> th_i) -> T;

template<typename T>
auto interface_R(char polarization, std::complex<T> n_i, std::complex<T> n_f, std::complex<T> th_i,
                 std::complex<T> th_f) -> T;

template<std::floating_point T>
auto interface_R(char polarization, const std::valarray<std::complex<T>> &n_i,
                 const std::valarray<std::complex<T>> &n_f, const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<T>;

template<typename T>
auto interface_T(char polarization, std::complex<T> n_i, std::complex<T> n_f, std::complex<T> th_i,
                 std::complex<T> th_f) -> T;

template<std::floating_point T>
auto interface_T(char polarization, const std::valarray<std::complex<T>> &n_i,
                 const std::valarray<std::complex<T>> &n_f, const std::valarray<std::complex<T>> &th_i,
                 const std::valarray<std::complex<T>> &th_f) -> std::valarray<T>;

template<std::floating_point T>
auto coh_tmm(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             std::complex<T> th_0, T lam_vac) -> coh_tmm_dict<T>;

template<typename T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto coh_tmm(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             const TH_T &th_0, const std::valarray<T> &lam_vac) -> coh_tmm_vec_dict<T>;

template<typename T, typename TH_T>
requires std::is_same_v<TH_T, std::valarray<std::complex<T>>> || std::is_same_v<TH_T, std::complex<T>>
auto coh_tmm(char pol, const std::vector<std::valarray<std::complex<T>>> &n_list, const std::vector<T> &d_list,
             const TH_T &th_0, const std::valarray<T> &lam_vac) -> coh_tmm_vecn_dict<T>;

template<std::floating_point T>
auto coh_tmm_reverse(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
                     std::complex<T> th_0, const std::valarray<T> &lam_vac) -> coh_tmm_vec_dict<T>;

template<typename T>
auto ellips(const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list, std::complex<T> th_0,
            T lam_vac) -> std::unordered_map<std::string, T>;

template<typename T>
auto ellips(const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list, std::complex<T> th_0,
            const std::valarray<T> &lam_vac) -> std::unordered_map<std::string, std::valarray<T>>;

template<typename T>
auto unpolarized_RT(const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list, std::complex<T> th_0,
                    const std::valarray<T> &lam_vac) -> std::unordered_map<std::string, std::valarray<T>>;

template<typename T, typename COH_TMM_T>
requires std::is_same_v<COH_TMM_T, coh_tmm_dict<T>> or std::is_same_v<COH_TMM_T, stack_coh_tmm_dict<T>>
auto position_resolved(std::size_t layer, T distance,
                       const COH_TMM_T &coh_tmm_data) -> std::unordered_map<std::string, std::variant<T, std::complex<T>>>;

template<typename T>
auto position_resolved(const std::valarray<std::size_t> &layer, const std::valarray<T> &distance,
                       const coh_tmm_vec_dict<T> &coh_tmm_data) -> std::unordered_map<std::string, std::variant<std::valarray<T>, std::valarray<std::complex<T>>>>;

template<typename T>
auto position_resolved(std::size_t layer, const std::valarray<T> &distance,
                       const coh_tmm_vec_dict<T> &coh_tmm_data) -> std::unordered_map<std::string, std::variant<std::valarray<T>, std::valarray<std::complex<T>>>>;

template<typename T>
auto find_in_structure(const std::valarray<T> &d_list,
                       const std::valarray<T> &dist) -> std::pair<std::valarray<typename std::iterator_traits<T *>::difference_type>, std::valarray<T>>;

template<typename T>
auto find_in_structure_inf(const std::valarray<T> &d_list,
                           const std::valarray<T> &dist) -> std::pair<std::valarray<std::size_t>, std::valarray<T>>;

template<std::floating_point T>
auto layer_starts(const std::valarray<T> &d_list) -> std::valarray<T>;

template<typename T>
auto absorp_in_each_layer(const coh_tmm_dict<T> &coh_tmm_data) -> std::valarray<T>;

template<typename T>
auto absorp_in_each_layer(const coh_tmm_vec_dict<T> &coh_tmm_data) -> std::valarray<std::valarray<T>>;

template<typename T>
auto absorp_in_each_layer(const coh_tmm_vecn_dict<T> &coh_tmm_data) -> std::valarray<std::valarray<T>>;

template<typename T>
auto inc_group_layers(const std::vector<std::valarray<std::complex<T>>> &n_list, const std::valarray<T> &d_list,
                      const std::valarray<LayerType> &c_list) -> inc_tmm_vec_dict<T>;

template<std::floating_point T>
auto inc_tmm(char pol, const std::valarray<std::complex<T>> &n_list, const std::valarray<T> &d_list,
             const std::valarray<LayerType> &c_list, std::complex<T> th_0, T lam_vac) -> inc_tmm_dict<T>;

template<std::floating_point T>
auto inc_tmm(char pol, const std::vector<std::valarray<std::complex<T>>> &n_list, const std::valarray<T> &d_list,
             const std::valarray<LayerType> &c_list, std::complex<T> th_0,
             const std::valarray<T> &lam_vac) -> inc_tmm_vec_dict<T>;

template<typename T>
auto inc_absorp_in_each_layer(const inc_tmm_dict<T> &inc_data) -> std::vector<T>;

template<typename T>
auto inc_absorp_in_each_layer(const inc_tmm_vec_dict<T> &inc_data) -> std::vector<std::valarray<T>>;

template<typename T>
auto inc_find_absorp_analytic_fn(std::size_t layer, const inc_tmm_vec_dict<T> &inc_data) -> AbsorpAnalyticVecFn<T>;

template<typename T>
auto inc_position_resolved(std::valarray<std::size_t> &&layer, const std::valarray<T> &dist,
                           const inc_tmm_vec_dict<T> &inc_tmm_data, const std::valarray<LayerType> &coherency_list,
                           const std::valarray<std::valarray<T>> &alphas,
                           T zero_threshold = 1e-6) -> std::valarray<std::valarray<T>>;

template<typename T>
auto beer_lambert(const std::valarray<T> &alphas, const std::valarray<T> &fraction, const std::valarray<T> &dist,
                  const std::valarray<T> &A_total) -> std::valarray<std::valarray<T>>;

#endif // TMM_H

#include <QtDebug>
#include <complex>
#include <sstream>
#include "tmm.h"

template <typename T>
std::string complex_to_string(const std::complex<T>& value) {
    std::ostringstream ss;
    ss << value.real() << " + " << value.imag() << "i";
    return ss.str();
}

template <typename T>
static inline void qwarning_complex_with_name(const std::complex<T> c, const QString& name) {
    // QDebug cannot overload << and >> for std::complex
    qWarning() << name << ": " << complex_to_string(c);
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
bool is_forward_angle(std::complex<T> n, std::complex<T> theta) {
    bool answer = false;
    if (n.real() * n.imag() >= 0) {
        qWarning() << "For materials with gain, it's ambiguous which beam is incoming vs outgoing";
        QWARNING_COMPLEX(n);
        qwarning_complex_with_name(theta, "angle");
    } else {
        std::complex<double> ncostheta = n * std::cos(theta);
        answer = std::abs(ncostheta.imag()) > 100 * EPSILON ? ncostheta.imag() > 0 : ncostheta.real() > 0;
        if ((answer && (ncostheta.imag() > -100 * EPSILON ||
                        ncostheta.real() > -100 * EPSILON ||
                        std::real(n * std::cos(std::conj(theta))) > -100 * EPSILON)) ||
            (!answer && (ncostheta.imag() < 100 * EPSILON ||
                         ncostheta.real() < 100 * EPSILON ||
                         std::real(n * std::cos(std::conj(theta))) < 100 * EPSILON))) {
            qWarning() << "It's not clear which beam is incoming vs outgoing. Weird index maybe?";
            QWARNING_COMPLEX(n);
            qwarning_complex_with_name(theta, "angle");
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
std::complex<T> snell(std::complex<T> n_1, std::complex<T> n_2, std::complex<T> th_1) {
    std::complex<T> th_2_guess = std::asin(n_1 * std::sin(th_1) / n_2);
    return is_forward_angle(n_2, th_2_guess) ? th_2_guess : M_PI - th_2_guess;
}

/* return list of angle theta in each layer based on angle th_0 in layer 0,
 * using Snell's law. n_list is index of refraction of each layer. Note that
 * "angles" may be complex!!
 */
template <typename T>
QList<std::complex<T>> list_snell(const QList<std::complex<T>>& n_list, std::complex<T> th_0) {
    QList<std::complex<T>> angles;
    for (size_t i = 0; i < n_list.size() - 1; i++) {
        angles.push_back(std::asin(n_list.first() * std::sin(th_0) / n_list[i]));
    }
    if (!is_forward_angle(n_list.first(), angles.first())) {
        angles.first() = M_PI - angles.first();
    }
    if (!is_forward_angle(n_list.last(), angles.last())) {
        angles.last() = M_PI - angles.last();
    }
    return angles;
}

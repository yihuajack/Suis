// <gsl/gsl_complex_math.h> has already included <gsl/gsl_complex.h>
#ifdef DEBUG
#include <iostream>
#define qWarning() std::cerr
#else
#include <QtDebug>
#endif
#include <sstream>
#include <gsl/gsl_math.h>
#include <gsl/gsl_complex_math.h>
#include "tmm.h"

std::string complex_to_string(const gsl_complex& value) {
    std::ostringstream ss;
    ss << GSL_REAL(value) << " + " << GSL_IMAG(value) << "i";
    return ss.str();
}

static inline void qwarning_complex_with_name(const gsl_complex c, const QString& name) {
    qWarning() << name << ": " << complex_to_string(c);
}

bool is_forward_angle(gsl_complex n, gsl_complex theta) {
    bool answer = false;
    if (GSL_REAL(n) * GSL_IMAG(n) >= 0) {
        qWarning() << "For materials with gain, it's ambiguous which beam is incoming vs outgoing";
        QWARNING_COMPLEX(n);
        qwarning_complex_with_name(theta, "angle");
    } else {
        gsl_complex ncostheta = gsl_complex_mul(n, gsl_complex_cos(theta));
        answer = std::abs(GSL_IMAG(ncostheta)) > 100 * EPSILON ? GSL_IMAG(ncostheta) > 0 : GSL_REAL(ncostheta) > 0;
        if ((answer && (GSL_IMAG(ncostheta) > -100 * EPSILON ||
                        GSL_REAL(ncostheta) > -100 * EPSILON ||
                        GSL_REAL(gsl_complex_mul(n, gsl_complex_cos(gsl_complex_conjugate(theta))))) > -100 * EPSILON) ||
            (!answer && (GSL_IMAG(ncostheta) < 100 * EPSILON ||
                         GSL_REAL(ncostheta) < 100 * EPSILON ||
                         GSL_REAL(gsl_complex_mul(n, gsl_complex_cos(gsl_complex_conjugate(theta))))) < 100 * EPSILON)) {
            qWarning() << "It's not clear which beam is incoming vs outgoing. Weird index maybe?";
            QWARNING_COMPLEX(n);
            qwarning_complex_with_name(theta, "angle");
        }
    }
    return answer;
}

gsl_complex snell(gsl_complex n_1, gsl_complex n_2, gsl_complex th_1) {
    gsl_complex th_2_guess = gsl_complex_arcsin(gsl_complex_div(gsl_complex_mul(n_1, gsl_complex_sin(th_1)), n_2));
    return is_forward_angle(n_2, th_2_guess) ? th_2_guess : gsl_complex_sub(gsl_complex_rect(M_PI, 0), th_2_guess);
}
QList<gsl_complex> list_snell(const QList<gsl_complex>& n_list, gsl_complex th_0) {
    QList<gsl_complex> angles;
    for (size_t i = 0; i < n_list.size() - 1; i++) {
        angles.push_back(gsl_complex_arcsin(gsl_complex_div(gsl_complex_mul(n_list.front(), gsl_complex_sin(th_0)), n_list[i])));
    }
    if (!is_forward_angle(n_list.front(), angles.front())) {
        angles.front() = gsl_complex_sub(gsl_complex_rect(M_PI, 0), angles.front());
    }
    if (!is_forward_angle(n_list.back(), angles.back())) {
        angles.back() = gsl_complex_sub(gsl_complex_rect(M_PI, 0), angles.back());
    }
    return angles;
}

#ifdef DEBUG
int main() {
    gsl_complex n, theta;
    GSL_SET_COMPLEX(&n, 1, 0);
    GSL_SET_COMPLEX(&theta, 0, 1);
    bool answer = is_forward_angle(n, theta);
    std::cout << "Is forward angle: " << answer << std::endl;
}
#endif

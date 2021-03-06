#ifndef MPFRHELPER_H
#define MPFRHELPER_H

#include <boost/multiprecision/mpfr.hpp>

namespace MathOps
{

template<typename T>
T get_constant_pi() { return boost::math::constants::pi<T>(); }

template<typename T>
T get_constant_e() { return boost::math::constants::e<T>(); }

inline boost::multiprecision::mpfr_float modf(boost::multiprecision::mpfr_float x, boost::multiprecision::mpfr_float &integral) { return boost::multiprecision::modf(x, &integral); }
inline boost::multiprecision::mpfr_float isnan(boost::multiprecision::mpfr_float x) { return boost::multiprecision::isnan(x); }
inline boost::multiprecision::mpfr_float abs(boost::multiprecision::mpfr_float x) { return boost::multiprecision::abs(x); }

inline boost::multiprecision::mpfr_float log(boost::multiprecision::mpfr_float x) { return boost::multiprecision::log(x); }
inline boost::multiprecision::mpfr_float log10(boost::multiprecision::mpfr_float x) { return boost::multiprecision::log10(x); }
inline boost::multiprecision::mpfr_float sqrt(boost::multiprecision::mpfr_float x) { return boost::multiprecision::sqrt(x); }
inline boost::multiprecision::mpfr_float pow(boost::multiprecision::mpfr_float a, boost::multiprecision::mpfr_float b) { return boost::multiprecision::pow(a, b); }

inline boost::multiprecision::mpfr_float sin(boost::multiprecision::mpfr_float x) { return boost::multiprecision::sin(x); }
inline boost::multiprecision::mpfr_float asin(boost::multiprecision::mpfr_float x) { return boost::multiprecision::asin(x); }
inline boost::multiprecision::mpfr_float cos(boost::multiprecision::mpfr_float x) { return boost::multiprecision::cos(x); }
inline boost::multiprecision::mpfr_float acos(boost::multiprecision::mpfr_float x) { return boost::multiprecision::acos(x); }
inline boost::multiprecision::mpfr_float tan(boost::multiprecision::mpfr_float x) { return boost::multiprecision::tan(x); }
inline boost::multiprecision::mpfr_float atan(boost::multiprecision::mpfr_float x) { return boost::multiprecision::atan(x); }

inline boost::multiprecision::mpfr_float sinh(boost::multiprecision::mpfr_float x) { return boost::multiprecision::sinh(x); }
inline boost::multiprecision::mpfr_float asinh(boost::multiprecision::mpfr_float x) { return boost::multiprecision::asinh(x); }
inline boost::multiprecision::mpfr_float cosh(boost::multiprecision::mpfr_float x) { return boost::multiprecision::cosh(x); }
inline boost::multiprecision::mpfr_float acosh(boost::multiprecision::mpfr_float x) { return boost::multiprecision::acosh(x); }
inline boost::multiprecision::mpfr_float tanh(boost::multiprecision::mpfr_float x) { return boost::multiprecision::tanh(x); }
inline boost::multiprecision::mpfr_float atanh(boost::multiprecision::mpfr_float x) { return boost::multiprecision::atanh(x); }

} /* namespace MathOps */

#endif /* MPFRHELPER_H */

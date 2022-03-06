/*  Copyright 1992-2021 Intel Corporation.                                 */
/*                                                                         */
/*  This software and the related documents are Intel copyrighted          */
/*  materials, and your use of them is governed by the express license     */
/*  under which they were provided to you ("License"). Unless the License  */
/*  provides otherwise, you may not use, modify, copy, publish,            */
/*  distribute, disclose or transmit this software or the related          */
/*  documents without Intel's prior written permission.                    */
/*                                                                         */
/*  This software and the related documents are provided as is, with no    */
/*  express or implied warranties, other than those that are expressly     */
/*  stated in the License.                                                 */

#ifndef __HLS_MATH_H__
#define __HLS_MATH_H__

/* This file is used to overlay math.h on linux and windows.
The table below describes what is currently available on each platform
We are currently supporting gcc 4.4.7 and MSVC 10 respectively, so that 
is the versions we are currenlty trying to mimic.
All floating point functions that are available, has an f at the end for 
single precison floating point, so sin(double), but sinf(float)
Y = Supported
N = Not supported yet
E = Support in extendedmath.h
- = Not available on that platform with the supported underlying compiler, 
 ****************************************************************
 * Function Name            Linux    Windows  Notes        *
 * abs                          -    N        Integer math *
 * labs                         -    N        Integer math *
 * atof                         -    N                     *
 * cos(float/double)            Y    Y                     *
 * sin(float/double)            Y    Y                     *
 * tan(float/double)            Y    Y                     *
 * acos(float/double)           Y    Y                     *
 * asin(float/double)           Y    Y                     *
 * atan(float/double)           Y    Y                     *
 * atan2(float/double)          Y    Y                     *
 * cosh(float/double)           Y    Y                     *
 * sinh(float/double)           Y    Y                     *
 * tanh(float/double)           Y    Y                     *
 * acosh(float/double)          Y    Y        C99/C++11    *
 * asinh(float/double)          Y    Y        C99/C++11    *
 * atanh(float/double)          Y    Y        C99/C++11    *
 * exp(float/double)            Y    Y                     *
 * frexp(float/double)          Y    Y                     *
 * ldexp(float/double)          Y    Y                     *
 * log(float/double)            Y    Y                     *
 * log10(float/double)          Y    Y                     *
 * modf(float/double)           Y    Y                     *
 * exp2(float/double)           Y    Y        C99/C++11    *
 * exp10(float/double)          Y    E                     *
 * expm1(float/double)          Y    Y        C99/C++11    *
 * ilogb(float/double)          Y    Y        C++11        *
 * log1p(float/double)          Y    Y        C99/C++11    * 
 * log2(float/double)           Y    Y        C99/C++11    *
 * logb(float/double)           Y    Y        C++11        *
 * scalbn(float/double)         N    N        C99/C++11    *
 * scalbln(float/double)        N    N        C99/C++11    *
 * pow(float/double)            Y    Y                     *
 * sqrt(float/double)           Y    Y                     *
 * cbrt(float/double)           Y    Y        C99/C++11    *
 * hypot(float/double)          Y    Y        C99/C++11    *
 * erf(float/double)            Y    Y        C99/C++11    *
 * erfc(float/double)           Y    Y        C99/C++11    *
 * tgamma(float/double)         Y    Y        C99/C++11    *
 * lgamma(float/double)         Y    Y        C99/C++11    *
 * lgamma_r(float/double)       Y    E        C99/C++11    *
 * ceil(float/double)           Y    Y                     *
 * floor(float/double)          Y    Y                     *
 * fmod(float/double)           Y    Y                     *
 * trunc(float/double)          Y    Y        C++11        *
 * round(float/double)          Y    Y        C++11        *
 * lround(float/double)         N    N        C++11        *
 * llround(float/double)        N    N        C++11        *
 * rint(float/double)           Y    Y        C99/C++11    *
 * lrint(float/double)          N    N        C++11        *
 * llrint(float/double)         N    N        C++11        *
 * nearbyint(float/double)      N    N        C++11        *
 * remainder(float/double)      Y    Y        C++11        *
 * drem                         N    -        obsolete name for remanider *
 * remquo(float/double)         Y    Y        C++11        *
 * significand                  N    -        non standard *
 * copysign(float/double)       Y    Y        C99/C++11    *
 * nan(float/double)            N    N        C99/C++11    *
 * nextafter(float/double)      Y    Y        C99/C++11    *
 * nexttoward(float/double)     N    N        C99/C++11    *
 * fdim(float/double)           Y    Y        C++11        *
 * fmin(float/double)           Y    Y        C++11        *
 * fmax(float/double)           Y    Y        C++11        *
 * fabs(float/double)           Y    Y                     *
 * fma(float/double)            Y    Y        C++11        *
 * fpclassify(float/double)     Y    N        C++11        *
 * isfinite(float/double)       Y    Y        C++11        *
 * finite(float/double)         Y    E        BSD name for isfinite _finite for windows    *
 * isinf(float/double)          Y    Y        C++11        *
 * isnan(float/double)          Y    Y        C++11        *
 * isnormal(float/double)       Y    N        C++11        *
 * signbit(float/double)        Y    N        C++11        *
 * isgreater(float/double)      N    -        C++11        *
 * isgreaterequal(float/double) N    -        C++11        *
 * isless(float/double)         N    -        C++11        *
 * islessequal(float/double)    N    -        C++11        *
 * islessgreater(float/double)  N    -        C++11        *
 * isunordered(float/double)    Y    N        C++11        *
 ****************************************************************/

#include <math.h>

#if defined(__INTELFPGA_COMPILER__)
#ifdef HLS_SYNTHESIS
#define __PUREF __attribute__((const))

// FPGA, remap to current internal OpenCL names
extern "C" {

/********** Supported trig builtins **********/
double __PUREF __acl__cosfd(double __x);
#define cos(__x) __acl__cosfd(__x)

float __PUREF __acl__flush_denorm_cosf(float __x);
#define cosf(__x) __acl__flush_denorm_cosf(__x)

double __PUREF __acl__sinfd(double __x);
#define sin(__x) __acl__sinfd(__x)

float __PUREF __acl__flush_denorm_sinf(float __x);
#define sinf(__x) __acl__flush_denorm_sinf(__x)

double __PUREF __acl__wrapper_tanfd(double __x);
#define tan(__x) __acl__wrapper_tanfd(__x)

float __PUREF __acl__flush_denorm_tanf(float __x);
#define tanf(__x) __acl__flush_denorm_tanf(__x)

double __PUREF __acl__acosfd(double __x);
#define acos(__x) __acl__acosfd(__x)

float __PUREF __acl__flush_denorm_acosf(float __x);
#define acosf(__x) __acl__flush_denorm_acosf(__x)

double __PUREF __acl__asinfd(double __x);
#define asin(__x) __acl__asinfd(__x)

float __PUREF __acl__flush_denorm_asinf(float __x);
#define asinf(__x) __acl__flush_denorm_asinf(__x)

double __PUREF __acl__atanfd(double __x);
#define atan(__x) __acl__atanfd(__x)

float __PUREF __acl__flush_denorm_atanf(float __x);
#define atanf(__x) __acl__flush_denorm_atanf(__x)

double __PUREF __acl__atan2fd(double __x, double __y);
#define atan2(__x, __y) __acl__atan2fd(__x, __y)

float __PUREF __acl__atan2f(float __x, float __y);
#define atan2f(__x, __y) __acl__atan2f(__x, __y)

double __PUREF __acl__coshfd(double __x);
#define cosh(__x) __acl__coshfd(__x)

float __PUREF __acl__coshf(float __x);
#define coshf(__x) __acl__coshf(__x)

double __PUREF __acl__sinhfd(double __x);
#define sinh(__x) __acl__sinhfd(__x)

float __PUREF __acl__sinhf(float __x);
#define sinhf(__x) __acl__sinhf(__x)

double __PUREF __acl__tanhfd(double __x);
#define tanh(__x) __acl__tanhfd(__x)

float __PUREF __acl__tanhf(float __x);
#define tanhf(__x) __acl__tanhf(__x)

double __PUREF __acl__acoshfd(double __x);
#define acosh(__x) __acl__acoshfd(__x)

double __PUREF __acl__asinhfd(double __x);
#define asinh(__x) __acl__asinhfd(__x)

double __PUREF __acl__atanhfd(double __x);
#define atanh(__x) __acl__atanhfd(__x)

double __PUREF __acl__expfd(double __x);
#define exp(__x) __acl__expfd(__x)

float __PUREF __acl__flush_denorm_expf(float __x);
#define expf(__x) __acl__flush_denorm_expf(__x)

double __PUREF __acl_frexpd(double __x, int* __y);
#define frexp(__x, __y)  __acl_frexpd(__x, __y)

float __PUREF __acl_frexpf(float __x, int* __y);
#define frexpf(__x, __y)  __acl_frexpf(__x, __y)

double __PUREF __acl__ldexpfd(double __x, int __y);
#define ldexp(__x, __y) __acl__ldexpfd(__x, __y)

float __PUREF __acl__ldexpf(float __x, int __y);
#define ldexpf(__x, __y) __acl__ldexpf(__x, __y)
 
double __PUREF __acl__logfd(double __x);
#define log(__x) __acl__logfd(__x)

float __PUREF __acl__flush_denorm_logf(float __x);
#define logf(__x) __acl__flush_denorm_logf(__x)

double __PUREF __acl__log10fd(double __x);
#define log10(__x) __acl__log10fd(__x)

float __PUREF __acl__log10f(float __x);
#define log10f(__x) __acl__log10f(__x)

double __PUREF __acl_modfd(double __x, double * __y);
#define modf(__x, __y) __acl_modfd(__x, __y)

float __PUREF __acl_modff(float __x, float* __y);
#define modff(__x, __y) __acl_modff(__x, __y)

double __PUREF __acl__exp2fd(double __x);
#define exp2(__x) __acl__exp2fd(__x)

float __PUREF __acl__exp2f(float __x);
#define exp2f(__x) __acl__exp2f(__x)

#if defined (__linux__)

double __PUREF __acl__exp10fd(double __x);
#define exp10(__x) __acl__exp10fd(__x)

float __PUREF __acl__exp10f(float __x);
#define exp10f(__x) __acl__exp10f(__x)

#endif //linux

inline double __x_expm1 (double __x) {
  return __acl__expfd(__x) - 1.0;
}
#define expm1(__x) __x_expm1(__x)

float __PUREF __acl__expm1f(float);      
#define expm1f(__x) __acl__expm1f(__x)

int __PUREF __acl__ilogbfd(double __x);
#define ilogb(__x) __acl__ilogbfd(__x)

int __PUREF __acl__ilogbf(float __x);
#define ilogbf(__x) __acl__ilogbf(__x)

double __PUREF __acl__ln1pxfd(double __x);
#define log1p(__x) __acl__ln1pxfd(__x)

float __PUREF __acl__ln1pxf(float __x);
#define log1pf(__x) __acl__ln1pxf(__x)

double __PUREF __acl__log2fd(double __x);
#define log2(__x) __acl__log2fd(__x)

float __PUREF __acl__log2f(float __x);
#define log2f(__x) __acl__log2f(__x)

double __PUREF __acl__powfd(double __x, double __y);
#define pow(__x,__y) __acl__powfd(__x, __y)

float __PUREF __acl__powf(float __x, float __y);
#define powf(__x,__y) __acl__powf(__x, __y)

double __PUREF __acl__sqrtfd(double  __x);
#define sqrt(__x) __acl__sqrtfd(__x)

float __PUREF __acl__flush_denorm_sqrtf(float __x);
#define sqrtf(__x) __acl__flush_denorm_sqrtf(__x)

// Only have the float version built-in.
inline double __x_cbrt(double __x) {
  return exp(log(__x) / 3.0);
}
#define cbrt(__x) __x_cbrt(__x)

float __PUREF __acl__cbrtf(float);
#define cbrtf(__x) __acl__cbrtf(__x)

inline double __x_hypot2(double __x, double __y) {
  return sqrt(__x*__x + __y*__y);
}

inline double __x_hypot3(double __x, double __y, double __z) {
  return sqrt(__x*__x + __y*__y + __z*__z);
}

// Pick hypot function based on # args (2 or 3)
// hypot for float and long double use double implementation
#define GET_HYPOT(_1, _2, _3, NAME, ...) NAME
#define hypot(...) GET_HYPOT(__VA_ARGS__, __x_hypot3, __x_hypot2)(__VA_ARGS__)

inline long double __x_hypotl(long double __x, long double __y) {
  return sqrt(__x*__x + __y*__y);
}
#define hypotl(__x,__y) __x_hypotl(__x,__y)

float __PUREF __acl__hypotf(float, float);
#define hypotf(__x,__y) __acl__hypotf(__x,__y)

/************ Error & Gamma Functions *********/
double __PUREF __acl__ceilfd(double __x);
#define ceil(__x) __acl__ceilfd(__x)

float __PUREF __acl__ceilf(float __x);
#define ceilf(__x) __acl__ceilf(__x)

double __PUREF __acl__floorfd(double __x);
#define floor(__x) __acl__floorfd(__x)

float __PUREF __acl__floorf(float __x);
#define floorf(__x) __acl__floorf(__x)

double __PUREF __acl__fmodfd(double __x, double __y);
#define fmod(__x,__y) __acl__fmodfd(__x, __y)

float __PUREF __acl__fmodf(float __x, float __y);
#define fmodf(__x,__y) __acl__fmodf(__x, __y)

double __PUREF __acl__truncfd(double __x);
#define trunc(__x) __acl__truncfd(__x)

float __PUREF __acl__truncf(float __x);
#define truncf(__x) __acl__truncf(__x)

double __PUREF __acl__roundfd(double __x);
#define round(__x) __acl__roundfd(__x)

float __PUREF __acl__roundf(float __x);
#define roundf(__x) __acl__roundf(__x)

double __PUREF __acl__rintfd(double __x);
#define rint(__x) __acl__rintfd(__x)

float __PUREF __acl__rintf(float __x);
#define rintf(__x) __acl__rintf(__x)

double __PUREF __acl__copysignfd(double __x, double __y);
#define copysign(__x, __y) __acl__copysignfd(__x, __y)

float __PUREF __acl__copysignf(float __x, float __y);
#define copysignf(__x, __y) __acl__copysignf(__x, __y)

double __PUREF __acl__fdimfd(double __x, double __y);
#define fdim(__x,__y) __acl__fdimfd(__x, __y)

float __PUREF __acl__fdimf(float __x, float __y);
#define fdimf(__x,__y) __acl__fdimf(__x, __y)

double __PUREF __acl__fminfd(double __x, double __y);
#define fmin(__x,__y) __acl__fminfd(__x, __y)

float __PUREF __acl__fminf(float __x, float __y);
#define fminf(__x,__y) __acl__fminf(__x, __y)

double __PUREF __acl__fmaxfd(double __x, double __y);
#define fmax(__x,__y) __acl__fmaxfd(__x, __y)

float __PUREF __acl__fmaxf(float __x, float __y);
#define fmaxf(__x,__y) __acl__fmaxf(__x, __y)

double __PUREF __acl__fabsfd(double __x);
#define fabs(__x) __acl__fabsfd(__x)

float __PUREF __acl__fabsf(float __x);
#define fabsf(__x) __acl__fabsf(__x)

/********** Classification ******************/
#if defined (__linux__)
long __PUREF __acl__isfinitefd(double __x);
int __PUREF __acl__isfinitef(float __x);
#define isfinite(__x) (sizeof (__x) == sizeof (float) ? __acl__isfinitef(__x) :  __acl__isfinitefd(__x))

#define finite(__x) __acl__isfinitefd(__x)
#define finitef(__x) __acl__isfinitef(__x)

long __PUREF __acl__isinffd(double __x);
int __PUREF __acl__isinff(float __x);
#define isinf(__x) (sizeof (__x) == sizeof (float) ? __acl__isinff(__x) :  __acl__isinffd(__x))

long __PUREF __acl__isnanfd(double __x);
int __PUREF __acl__isnanf(float __x);
#define isnan(__x) (sizeof (__x) == sizeof (float) ? __acl__isnanf(__x) :  __acl__isnanfd(__x))

inline int __x_fpclassifyfd(double __x) {
  if (isnan(__x)) return 0; // FP_NAN
  if (isinf(__x)) return 1; // FP_INFINITE
  unsigned long in_bits = *((unsigned long*)&__x);
  if ((in_bits == 0x0000000000000000) || (in_bits == 0x8000000000000000)) return 2; // FP_ZERO
  if ((in_bits & 0x7ff0000000000000) == 0) return 3; // FP_SUBNORMAL
  return 4; // FP_NORMAL
}
inline int __x_fpclassifyf(float __x) {
  if (isnan(__x)) return 0; // FP_NAN
  if (isinf(__x)) return 1; // FP_INFINITE
  unsigned in_bits = *((unsigned*)&__x);
  if ((in_bits == 0x00000000) || (in_bits == 0x80000000)) return 2; // FP_ZERO
  if ((in_bits & 0x7f800000) == 0) return 3; // FP_SUBNORMAL
  return 4; // FP_NORMAL
}
#define fpclassify(__x) (sizeof (__x) == sizeof (float) ?  __x_fpclassifyf(__x) :  __x_fpclassifyfd(__x))

inline int __x_signbitfd(double __x) { unsigned long xbits = *((unsigned long*)&__x); return (int)(xbits >> 63); }
inline int __x_signbitf(float __x) { unsigned xbits = *((unsigned*)&__x); return xbits >> 31; }
#define signbit(__x) (sizeof (__x) == sizeof (float) ?  __x_signbitf(__x) :  __x_signbitfd(__x))

#define isnormal(x) (fpclassify (x) == FP_NORMAL)

#endif //linux

#if defined (_MSC_VER)
  // Pull in CPP Library version information. 
  long long __PUREF __acl__isnanfd(double __x);
  int __PUREF __acl__isnanf(float __x);
  long long __PUREF __acl__isfinitefd(double __x);
  int __PUREF __acl__isfinitef(float __x);
  
  #if (_MSC_VER >= 1900)
    // Support isnan(x) for MSVC 2015+  in fpga components.
    // MSVC handles _isnanf and _isnanfd in 2 different places (math.h and float.h).
    // So it is messy to support them. Use isnan instead.
    #ifndef isnan 
      #define isnan(x) \
        (sizeof (x) == sizeof (float) ? __acl__isnanf(x) :  __acl__isnanfd(x))
    #endif // #ifndef isnan

    #ifndef isfinite
      #define isfinite(x) \
        (sizeof (x) == sizeof (float) ? __acl__isfinitef(x) : __acl__isfinitefd(x))
    #endif // #ifndef isfinite

    #ifndef isinf
      #define isinf(x) \
        (sizeof (x) == sizeof (float) ? !__acl__isfinitef(x) : !__acl__isfinitefd(x))
    #endif // #ifndef isinf

  #endif // #if (_MSC_VER >= 1900)

#endif // #if defined(MSC_VER)

/********** Other builtins **********/
/* They have to be put in the end because they are dependent on functions above */
float __PUREF __acl__flush_denorm(float __x);
inline float __PUREF __acl__flush_denorm_signed_zero(float __x) {
  unsigned in_bits = *((unsigned*)&__x);
  if (((in_bits & 0x7f800000) == 0) && (in_bits != 0x80000000)) in_bits = 0;
  return *((float*)&in_bits);
}

inline float __x_acoshf(float __x) {
  unsigned float_nan = 0x7fffffff;
  return (float)(isnan(__x) ? (__x) : 
          (((__x) < 1.0f) ? *((float*)&float_nan) : 
           __acl__flush_denorm((float)(log(((double)(__x)) + sqrt(((double)(__x)) + 1.0) * sqrt(((double)(__x))-1.0))))));
}
#define acoshf(__x) __x_acoshf(__x)

inline float __x_asinhf(float __x) {
  float absx = fabsf(__x); 
  double myx = (double) __x; 
  return (float) ((isnan(__x) || isinf(__x)) ? (__x) : 
          __acl__flush_denorm((absx < 0x1.0p-28f) ? (__x) : 
           ((absx > 0x1.0p+28f) ? copysignf(((float)(log((double)(absx)) + 0.693147180559945309417232121458176568)), __x) : 
            ((absx > 2.0f) ? copysignf(((float)(log(((double)(absx)) + sqrt(myx * myx + 1.0)))), __x) : 
             copysignf(((float)(log1p(absx + myx * myx / (1.0 + sqrt(myx * myx + 1.0))))), __x)))));
}
#define asinhf(__x) __x_asinhf(__x)

inline float __x_atanhf(float __x) {
  unsigned float_nan = 0x7fffffff;
  float absx = fabsf(__acl__flush_denorm_signed_zero(__x));
  double __dx = (double)(absx); 
  double __dxs = (double)(__acl__flush_denorm_signed_zero(__x)); 
  return (float)__acl__flush_denorm_signed_zero((isnan(__x) ? *((float*)&float_nan) : 
          ((*((unsigned*)&absx) > 0x3f800000) ? *((float*)&float_nan) : 
           ((*((unsigned*)&absx) < 0x3f000000) ? (float) copysign(ldexp(log1p( 2.0 * ( __dx + __dx*__dx / (1.0-__dx) )), -1), __dxs) : 
            (float)copysign(ldexp(log1p(2.0 * __dx / (1.0-__dx)), -1), __dxs)))));
}
#define atanhf(__x) __x_atanhf(__x)

inline double __x_erffd(double __x) { double x_sqr = __x * __x; return copysign(sqrt(1.0 - exp(-x_sqr * (((4.0 / 0X1.921FB54442D18P+1) + 0.147*x_sqr) / (1.0+0.147*x_sqr)))),__x); }
#define erf(__x) __x_erffd(__x)

inline float __x_erff(float __x) { float x_sqr = (__x) * (__x); return copysignf(sqrtf(1.0f - expf(-x_sqr * (((4.0f * 0X1.45F306DC9C883P-2f) + 0.147f*x_sqr) / (1.0f+0.147f*x_sqr)))),__x); }
#define erff(__x) __x_erff(__x)

inline double __x_erfcfd(double __x) { return 1.0 - erf(__x); }
#define erfc(__x) __x_erfcfd(__x)

inline float __x_erfcf(float __x) { return 1.0f - erff(__x); }
#define erfcf(__x) __x_erfcf(__x)

inline double __x_fmafd(double __x, double __y, double __z) {
  if (!isnan(__x) && !isinf(__x) && !isnan(__y) && !isinf(__y) && isinf(__z)) return __z;
  if (isnan(__x)) return __x;
  if (isnan(__y)) return __y;
  if (isnan(__z)) return __z;
  return __x*__y+__z;
}
#define fma(__x,__y,__z) __x_fmafd(__x,__y,__z)

float __PUREF __acl__fused_mad(float __x, float __y, float __z);
inline float __x_fmaf(float __x, float __y, float __z) {
  if (!isnan(__x) && !isinf(__x) && !isnan(__y) && !isinf(__y) && isinf(__z)) return __z;
  if (isnan(__x)) return __x;
  if (isnan(__y)) return __y;
  if (isnan(__z)) return __z;
  float ix = __acl__flush_denorm(__x);
  float iy = __acl__flush_denorm(__y);
  float iz = __acl__flush_denorm(__z);
  return __acl__fused_mad(ix, iy, iz);
}
#define fmaf(__x,__y,__z) __x_fmaf(__x,__y,__z)

// Every invoking of Gamma function uses Stirling's approximation
inline double __x_lgammafd(double __x) { return (__x-0.5)*log(__x) - (__x) + 0.91893853320467; }
#define lgamma(__x) __x_lgammafd(__x)

inline float __x_lgammaf(float __x) { return isinf(__x) ? (__x) : ((fabsf(__x)-0.5f)*logf(fabsf(__x)) - fabsf(__x) + 0.91893853320467f); }
#define lgammaf(__x) __x_lgammaf(__x)

#if defined (__linux__)

inline double __x_lgammafd_r(double __x, int *__sign) {
  double i = lgamma(__x);
  *__sign = (int)((i < 0.0) ? 1 : 0);
  return i;
}
#define lgamma_r(__x,__sign) __x_lgammafd_r(__x,__sign)

inline float __x_lgammaf_r(float __x, int *__sign) {
  float i = lgammaf(__x);
  *__sign = (int)((i < 0.0) ? 1 : 0);
  return i;
}
#define lgammaf_r(__x,__sign) __x_lgammaf_r(__x,__sign)

#endif

inline double __x_tgammafd(double __x) { return exp(lgamma(__x)); }
#define tgamma(__x) __x_tgammafd(__x)

inline float __x_tgammaf(float __x) { return expf(lgammaf(__x)); }
#define tgammaf(__x) __x_tgammaf(__x)

#if defined (__linux__)
inline double __x_logbfd(double __x) {
  if (isnan(__x) || isinf(__x)) return __x;
  long exponent = *((long*)&__x);
  exponent = (exponent >> 52) & 0x00000000000007ff;
  if (exponent == 0) return exponent;
  return ((double)(exponent - 1023));
}

inline double __x_nextafterfd(double __x, double __y) {
  if (isnan(__x)) return __x;
  if (isnan(__y)) return __y;
  unsigned long temp = 9223372036854775808U;
  unsigned long x_int = *((unsigned long*)&__x);
  unsigned long y_int = *((unsigned long*)&__y);
  if (x_int & temp) x_int = temp - x_int;
  if (y_int & temp) y_int = temp - y_int;
  if (x_int==y_int) return __x;
  x_int += (x_int < y_int) ? 1 : -1;
  x_int = (x_int < 0) ? temp - x_int : x_int;
  return *((double*)&x_int);
}

inline double __x_remquofd(double __x, double __y, int *__quo) {
  double fx = __x;
  double fy = __y;
  double xr = fabs(fx);
  double yr = fabs(fy);
  unsigned long mask_x = *((unsigned long*)&xr);
  unsigned long mask_y = *((unsigned long*)&yr);
  unsigned long sign_x = *((unsigned long*)&fx) >> 63;
  unsigned long sign_y = *((unsigned long*)&fy) >> 63;
  if (isnan(fx) || isnan(fy) || isinf(fx) || (*((unsigned long*)&yr) == 0))
  { *__quo = 0; return __builtin_nan(""); } 
  if (isinf(fy) || (mask_x == 0)) { *__quo = 0; return fx; }
  if (mask_x == mask_y) { *__quo = (sign_x == sign_y) ? 1 : -1; return copysign((double)0.0,fx); }
  int ex = ilogb(fx);
  int ey = ilogb(fy);
  unsigned q = 0;
  double xxp = ldexp(xr, 1);
  if (ex-ey >= 0) { 
    int i;
    yr = ldexp(yr, -ey);
    xr = ldexp(xr, -ex);
    for(i = ex-ey; i > 0; i--) {
      q <<= 1;
      if(xr >= yr) {
        xr -= yr;
        q += 1;
      }
      xr += xr;
    }
    q <<= 1;
    if( xr > yr ) {
      xr -= yr;
      q += 1;
    }
    xxp = ldexp(xr, ey+1);
    xr = ldexp(xr, ey);
  }
  double abs_fy = fabs(fy);
  if ((abs_fy < xxp) || ((abs_fy == xxp) && (q & 0x00000001))) {
    double abs_xr = fabs(xr);
    bool adjust = (*((unsigned long*)&abs_xr) == 0);
    double yyp = ldexp(fabs(fy),1);
    xxp -= yyp;
    xr = adjust ? ldexp(xxp,-1) : xxp; q += 1; }
  q = q & 0x07f;
  if (sign_x != sign_y) q = -q;
  if ((sign_x == 1) && (mask_x != 0)) xr = -xr;
  *__quo = q;
  return xr; 
}
#endif
#if defined (_MSC_VER)
inline double __x_logbfd(double __x) {
  if (isnan(__x) || isinf(__x)) return __x;
  long long exponent = *((long long*)&__x);
  exponent = (exponent >> 52) & 0x00000000000007ff;
  if (exponent == 0) return exponent;
  return ((double)(exponent - 1023));
}

inline double __x_nextafterfd(double __x, double __y) {
  if (isnan(__x)) return __x;
  if (isnan(__y)) return __y;
  unsigned long long temp = 9223372036854775808U;
  unsigned long long x_int = *((unsigned long long*)&__x);
  unsigned long long y_int = *((unsigned long long*)&__y);
  if (x_int & temp) x_int = temp - x_int;
  if (y_int & temp) y_int = temp - y_int;
  if (x_int==y_int) return __x;
  x_int += (x_int < y_int) ? 1 : -1;
  x_int = (x_int < 0) ? temp - x_int : x_int;
  return *((double*)&x_int);
}

inline double __x_remquofd(double __x, double __y, int *__quo) {
  double fx = __x;
  double fy = __y;
  double xr = fabs(fx);
  double yr = fabs(fy);
  unsigned long long mask_x = *((unsigned long long*)&xr);
  unsigned long long mask_y = *((unsigned long long*)&yr);
  unsigned long long sign_x = *((unsigned long long*)&fx) >> 63;
  unsigned long long sign_y = *((unsigned long long*)&fy) >> 63;
  if (isnan(fx) || isnan(fy) || isinf(fx) || (*((unsigned long long*)&yr) == 0))
  { *__quo = 0; return __builtin_nan(""); } 
  if (isinf(fy) || (mask_x == 0)) { *__quo = 0; return fx; }
  if (mask_x == mask_y) { *__quo = (sign_x == sign_y) ? 1 : -1; return copysign((double)0.0,fx); }
  int ex = ilogb(fx);
  int ey = ilogb(fy);
  unsigned q = 0;
  double xxp = ldexp(xr, 1);
  if (ex-ey >= 0) { 
    int i;
    yr = ldexp(yr, -ey);
    xr = ldexp(xr, -ex);
    for(i = ex-ey; i > 0; i--) {
      q <<= 1;
      if(xr >= yr) {
        xr -= yr;
        q += 1;
      }
      xr += xr;
    }
    q <<= 1;
    if( xr > yr ) {
      xr -= yr;
      q += 1;
    }
    xxp = ldexp(xr, ey+1);
    xr = ldexp(xr, ey);
  }
  double abs_fy = fabs(fy);
  if ((abs_fy < xxp) || ((abs_fy == xxp) && (q & 0x00000001))) {
    double abs_xr = fabs(xr);
    bool adjust = (*((unsigned long long*)&abs_xr) == 0);
    double yyp = ldexp(fabs(fy),1);
    xxp -= yyp;
    xr = adjust ? ldexp(xxp,-1) : xxp; q += 1; }
  q = q & 0x07f;
  if (sign_x != sign_y) q = -q;
  if ((sign_x == 1) && (mask_x != 0)) xr = -xr;
  *__quo = q;
  return xr; 
}
#endif
#define logb(__x) __x_logbfd(__x)
#define nextafter(__x,__y) __x_nextafterfd(__x,__y)
#define remquo(__x,__y,__quo) __x_remquofd(__x,__y,__quo)

inline double __x_remainderfd(double __x, double __y) { int __i; return remquo(__x, __y, &__i); }
#define remainder(__x,__y) __x_remainderfd(__x,__y)

inline float __x_logbf(float __x) {
  if (isnan(__x) || isinf(__x)) return __x;
  int exponent = *((int*)&__x);
  exponent = (exponent >> 23) & 0x000000ff;
  if (exponent == 0) return exponent;
  return ((float)(exponent - 127));
}
#define logbf(__x) __x_logbf(__x)

inline float __x_nextafterf(float __x, float __y) {
  if (isnan(__x)) return __x;
  if (isnan(__y)) return __y;
  unsigned int temp = 2147483648U;
  unsigned int x_int = *((unsigned int*)&__x);
  unsigned int y_int = *((unsigned int*)&__y);
  if (x_int & temp) x_int = temp - x_int;
  if (y_int & temp) y_int = temp - y_int;
  if (x_int==y_int) return __x;
  x_int += (x_int < y_int) ? 1 : -1;
  x_int = (x_int < 0) ? temp - x_int : x_int;
  return *((float*)&x_int);
}
#define nextafterf(__x,__y) __x_nextafterf(__x,__y)

float __PUREF __acl__fadd_with_denorms(float a, float b);
inline float __x_remquof(float __x, float __y, int *__quo) {
  float fx = __acl__flush_denorm(__x);
  float fy = __acl__flush_denorm(__y);
  float xr = fabsf(fx);
  float yr = fabsf(fy);
  unsigned mask_x = *((unsigned*)&xr);
  unsigned mask_y = *((unsigned*)&yr);
  unsigned sign_x = *((unsigned*)&fx) >> 31;
  unsigned sign_y = *((unsigned*)&fy) >> 31;
  if (isnan(fx) || isnan(fy) || isinf(fx) || (*((unsigned*)&yr) == 0))
  { *__quo = 0; return __builtin_nanf(""); } 
  if (isinf(fy) || (mask_x == 0)) { *__quo = 0; return fx; }
  if (mask_x == mask_y) { *__quo = (sign_x == sign_y) ? 1 : -1; return copysignf((float)0.0f,fx); }
  int ex = ilogbf(fx);
  int ey = ilogbf(fy);
  unsigned q = 0;
  float xxp = ldexpf(xr, 1);
  if (ex-ey >= 0) { 
    int i;
    yr = ldexpf(yr, -ey);
    xr = ldexpf(xr, -ex);
    for(i = ex-ey; i > 0; i--) {
      q <<= 1;
      if(xr >= yr) {
        xr -= yr;
        q += 1;
      }
      xr += xr;
    }
    q <<= 1;
    if( xr > yr ) {
      xr -= yr;
      q += 1;
    }
    xxp = ldexpf(xr, ey+1);
    xr = ldexpf(xr, ey);
  }
  float abs_fy = fabsf(fy);
  if ((abs_fy < xxp) || ((abs_fy == xxp) && (q & 0x00000001))) {
    float abs_xr = fabsf(xr);
    bool adjust = (*((unsigned*)&abs_xr) == 0);
    float yyp = ldexpf(abs_fy,1);
    xxp = __acl__fadd_with_denorms(adjust ? xxp : xr, adjust ? (-yyp) : (-abs_fy));
    xr = adjust ? ldexpf(xxp,-1) : xxp; q += 1; }
  q = q & 0x07f;
  if (sign_x != sign_y) q = -q;
  if ((sign_x == 1) && (mask_x != 0)) xr = -xr;
  *__quo = q;
  return __acl__flush_denorm(xr);
}
#define remquof(__x,__y,__quo) __x_remquof(__x,__y,__quo)

inline float __x_remainderf(float __x, float __y) { int __i; return __acl__flush_denorm(remquof(__x, __y, &__i)); }
#define remainderf(__x,__y) __x_remainderf(__x,__y)

}
#endif //#ifdef HLS_SYNTHESIS
#endif //__INTELFPGA_COMPILER__
#endif //__HLS_MATH_H__


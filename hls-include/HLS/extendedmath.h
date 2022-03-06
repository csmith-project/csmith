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

/**********************************************************************
 *                                                                    *
 *  Math function that is not supported by the standard Math headers  *
 *                                                                    *
 **********************************************************************/
#ifndef __HLS_EXTENDEDMATH_H__
#define __HLS_EXTENDEDMATH_H__

/* The table below describes what is currently available on each platform
We are currently supporting gcc 5.4.0 and MSVC 10 respectively, so that 
is the versions we are currenlty trying to mimic.
All floating point functions that are available, has an f at the end for 
single precison floating point, so sin(double), but sinf(float)
E = Support in extendedmath.h
Y = Support in math.h
N = Not supported yet
 ****************************************************************
 * Function Name            Linux    Windows  Notes        *
 * sincos(float/double)         E    E                     *
 * rsqrt(float/double)          E    E                     *
 * sinpi(float/double)          E    E                     *
 * cospi(float/double)          E    E                     *
 * tanpi(float/double)          E    E                     *
 * asinpi(float/double)         E    E                     *
 * acospi(float/double)         E    E                     *
 * atanpi(float/double)         E    E                     *
 * powr(float/double)           E    E                     *
 * pown(float/double)           E    E                     *
 * popcountc(unsigned char)     E    E                     *
 * popcounts(unsigned short)    E    E                     *
 * popcount(unsigned)           E    E                     *
 * popcountl(unsigned long)     E    E                     *
 * popcountll(unsigned longlong)E    E                     *
 * exp10(float/double)          Y    E                     *
 * finite(float/double)         Y    E                     *
 * lgamma_r(float/double)       Y    E                     *
 * fract(float/double)          E    E                     *
 * isordered(float/double)      E    E                     *
 * maxmag(float/double)         E    E                     *
 * minmag(float/double)         E    E                     *
 * rootn(float/double)          E    E                     *
 * mad(float/double)            E    E                     *
 * oclnan(float/double)         E    E                     *
 ****************************************************************/

#ifndef _HLS_EMBEDDED_PROFILE
#include "HLS/internal/_hls.h"
#endif
#include "HLS/math.h"

// iff we're in the Microsoft compilation flow ...
#if defined (_MSC_VER)
  // Pull in CPP Library version information. 
  #include <yvals.h>
  #include <intrin.h>
  // Check if _CPPLIB_VER is at least 650, the version for
  // Visual Studio 2015 header files, where _finite() and _isnan()
  // are defined.
  #if (_CPPLIB_VER >= 650)

    #ifndef M_PI
      #define M_PI 3.14159265358979323846
    #endif // #ifndef M_PI

    // Note that MSVC long is 32 bit not 64 bit. long long is 64bit.
    // __acl__isfinitefd returns a 64 bit so it needs to be declared
    // with long long, so it can be properly used.
    #ifdef HLS_SYNTHESIS
    extern "C" {
      long long __PUREF __acl__isfinitefd(double __x);
      #define finite(__x) __acl__isfinitefd(__x)
      
      int __PUREF __acl__isfinitef(float __x);
      #define finitef(__x) __acl__isfinitef(__x)
    } //Extern "C"
    #else 
      // If not in synthesis flow, use windows built-ins.
      // Cast finite() result to 64 bit so it is consistent with fpga flow.
      #define finitef(__x) _finitef(__x)
      #define finite(__x) (long long)_finite(__x)
    #endif

  #endif // #if (_CPPLIB_VER >= 650)

#endif // #if defined(MSC_VER)

/********* Extended Math Constants ******/

// Constants for inverse of pi
static const float M_PI_INV_F = 0.3183098861838f;
static const double M_PI_INV_D = 0.318309886183790671537767;


/********* Extended Math Functions ******/

// Mapping powr and pown back to pow with a corner case for powr of x < 0.
inline double __acl__powr_approx(double x, double y) {
  if (x < 0) {
    unsigned long long res = 0x7fffffffffffffffULL;
    return *((double *) &res);
  } else {
    return pow(x,y);
  }
}

inline float __acl__powrf_approx(float x, float y) {
  if (x < 0) {
    unsigned int res = 0x7fffffff;
    return *((float *) &res);
  } else {
    return powf(x,y);
  }
}

extern "C" {

#if defined(HLS_SYNTHESIS) || defined(_MSC_VER)
// This follows the GNU extension syntax
inline void sincosf(float arg, float *sinval, float *cosval) {
  *sinval = sinf(arg);
  *cosval = cosf(arg);
}

inline void sincos(double arg, double *sinval, double *cosval) {
  *sinval = sin(arg);
  *cosval = cos(arg);
}
#endif

// Here are definitions of math functions that are not declared in system math.h
// Implementations work on both emulator and simulator
inline float flush_denorm(float __x) {
  unsigned in_bits = *((unsigned*)&__x);
  if ((in_bits & 0x7f800000) == 0) in_bits = 0;
  return *((float*)&in_bits);
}
  // if compiling for FPGA
#if defined(HLS_SYNTHESIS)

inline float maxmagf(float __x, float __y) { float abs_x = fabsf(__x); float abs_y = fabsf(__y);
  return ((abs_x < abs_y) ? ( __y ) : ((abs_y < abs_x) ? (__x) : (__acl__fmaxf(__x, __y)))); }
inline double maxmag(double __x, double __y) { double abs_x = fabs(__x); double abs_y = fabs(__y);
  return ((abs_x < abs_y) ? ( __y ) : ((abs_y < abs_x) ? (__x) : (__acl__fmaxfd(__x, __y))));}

inline float minmagf(float __x, float __y) { float abs_x = fabsf(__x); float abs_y = fabsf(__y);
  return ((abs_x > abs_y) ? ( __y ) : ((abs_y > abs_x) ? (__x) : (__acl__fminf(__x, __y)))); }
inline double minmag(double __x, double __y) { double abs_x = fabs(__x); double abs_y = fabs(__y);
  return ((abs_x > abs_y) ? ( __y ) : ((abs_y > abs_x) ? (__x) : (__acl__fminfd(__x, __y)))); }

#else //if linux or Windows x86

inline float maxmagf(float __x, float __y) { float abs_x = fabsf(__x); float abs_y = fabsf(__y);
  return ((abs_x < abs_y) ? ( __y ) : ((abs_y < abs_x) ? (__x) : (fmax(__x, __y)))); }
inline double maxmag(double __x, double __y) { double abs_x = fabs(__x); double abs_y = fabs(__y);
  return ((abs_x < abs_y) ? ( __y ) : ((abs_y < abs_x) ? (__x) : (fmaxf(__x, __y))));}

inline float minmagf(float __x, float __y) { float abs_x = fabsf(__x); float abs_y = fabsf(__y);
  return ((abs_x > abs_y) ? ( __y ) : ((abs_y > abs_x) ? (__x) : (fmin(__x, __y)))); }
inline double minmag(double __x, double __y) { double abs_x = fabs(__x); double abs_y = fabs(__y);
  return ((abs_x > abs_y) ? ( __y ) : ((abs_y > abs_x) ? (__x) : (fminf(__x, __y)))); }

#endif

inline float madf(float __x, float __y, float __z) { return __z + __x * __y; }
inline double mad(double __x, double __y, double __z) { return __z + __x * __y; }

inline float rootnf(float __x, int __y) {
  float fx = flush_denorm(__x);
  float absx = fabsf(fx);
  int signfx = *((unsigned*)&fx) >> 31;
  unsigned float_mask = 0x7fffffff;
  if (__y == 0) return *((float*)&float_mask);
  if (((__y&1) == 0) && ((*((unsigned*)&fx) != 0) && (signfx == 1)))  return *((float*)&float_mask);
  if (*((unsigned*)&fx) == 0) {
    if ( (__y & 0x80000001) == 0) return 0.0f;
    if ( (__y & 0x80000001) == 1) return fx;
    unsigned _float_mask = 0x7f800000;
    if ( (__y & 0x80000001) == 0x80000000) return *((float*)&_float_mask);
    if ( (__y & 0x80000001) == 0x80000001) return copysignf(*((float*)&_float_mask), __x);
  }
  double result = exp2( log2(((double)(absx))) / ((double)(__y)) );
  float final_result = flush_denorm(((float)(result)));
  return copysignf(final_result, __x);
}

inline double rootn(double __x, int __y) { return exp(log(__x) / (__y)); }

inline float oclnanf(unsigned __x) { unsigned mask = __x & 4194303; unsigned exp_mask = 2143289344; unsigned res = (mask|exp_mask); return *((float*)&res); }

inline float fractf(float __x, float *__ptr) {
  float i, f;
  if (isnan(__x)) { *__ptr = __x; return __x; }
  if (isinf(__x)) { *__ptr = __x; return copysignf((float)0.0f, __x); }
  f = modff(__x, &i);
  unsigned signf = *((unsigned*)&f) >> 31;
  float maskf = fabsf(f);
  unsigned mask = *((unsigned*)&maskf);
  if ((signf == 1) && (mask != 0)) {
    f = 1.0f + f;
    i -= 1.0f;
    if (*((unsigned*)&f) == 0x3f800000) { unsigned bits = 0x3F7FFFFF; f = *((float*)&bits); }
  }
  *__ptr = i;
  return f; 
}
#if defined (__linux__)
inline double oclnan(unsigned long __x) { unsigned long mask = __x & 2251799813685247; unsigned long exp_mask = 9221120237041090560; unsigned long res = (mask|exp_mask); return *((double*)&res); }

inline double fract(double __x, double *__ptr) {
  double i, f;
  if (isnan(__x)) { *__ptr = __x; return __x; }
  if (isinf(__x)) { *__ptr = __x; return copysign((double)0.0, __x); }
  f = modf(__x, &i);
  unsigned long signf = *((unsigned long*)&f) >> 63;
  double maskd = fabs(f);
  unsigned long mask = *((unsigned long*)&maskd);
  if ((signf == 1) && (mask != 0)) {
    f = 1.0 + f;
    i -= 1.0;
    if (*((unsigned long long*)&f) == 0x3ff0000000000000ULL) { unsigned long long bits = 0x3FEFFFFFFFFFFFFFULL; f = *((double*)&bits); }
  }
  *__ptr = i;
  return f;
}
#endif // __linux__
#if defined (_MSC_VER)
inline double oclnan(unsigned long long __x) { unsigned long long mask = __x & 2251799813685247; unsigned long long exp_mask = 9221120237041090560; unsigned long long res = (mask|exp_mask); return *((double*)&res); }

inline double fract(double __x, double *__ptr) {
  double i, f;
  if (isnan(__x)) { *__ptr = __x; return __x; }
  if (isinf(__x)) { *__ptr = __x; return copysign((double)0.0, __x); }
  f = modf(__x, &i);
  unsigned long long signf = *((unsigned long long*)&f) >> 63;
  double maskd = fabs(f);
  unsigned long long mask = *((unsigned long long*)&maskd);
  if ((signf == 1) && (mask != 0)) {
    f = 1.0 + f;
    i -= 1.0;
    if (*((unsigned long long*)&f) == 0x3ff0000000000000ULL) { unsigned long long bits = 0x3FEFFFFFFFFFFFFFULL; f = *((double*)&bits); }
  }
  *__ptr = i;
  return f;
}
#endif // _MSC_VER

#ifdef HLS_SYNTHESIS
// FPGA, remap to current internal names
float __PUREF __acl__rsqrtf(float __x);
#define rsqrtf(__x) __acl__rsqrtf(__x)
double __PUREF __acl__rsqrtfd(double __x);
#define rsqrt(__x) __acl__rsqrtfd(__x)

float __PUREF __acl__sinpif(float __x);
#define sinpif(__x) __acl__sinpif(__x)
double __PUREF __acl__sinpifd(double __x);
#define sinpi(__x) __acl__sinpifd(__x)

float __PUREF __acl__cospif(float __x);
#define cospif(__x) __acl__cospif(__x)
double __PUREF __acl__cospifd(double __x);
#define cospi(__x) __acl__cospifd(__x)

float __acl__tanpif(float __x);
#define tanpif(__x) __acl__tanpif(__x)
#define tanpi(__x) (tan((__x) * (double)M_PI))

float __PUREF __acl__asinpif(float __x);
#define asinpif(__x) __acl__asinpif(__x)
double __PUREF __acl__asinpifd(double __x);
#define asinpi(__x) __acl__asinpifd(__x)

float __PUREF __acl__acospif(float __x);
#define acospif(__x) __acl__acospif(__x)
double __PUREF __acl__acospifd(double __x);
#define acospi(__x) __acl__acospifd(__x)

float __PUREF __acl__atanpif(float __x);
#define atanpif(__x) __acl__atanpif(__x)
double __PUREF __acl__atanpifd(double __x);
#define atanpi(__x) __acl__atanpifd(__x)

float __PUREF __acl__powrf(float __x, float __y);
#define powrf(__x,__y) __acl__powrf(__x, __y)
double __PUREF __acl__powrfd(double __x, double __y);
#define powr(__x,__y) __acl__powrfd(__x, __y)

float __PUREF __acl__pownf(float __x, int __y);
#define pownf(__x,__y) __acl__pownf(__x, __y)
double __PUREF __acl__pownfd(double __x, int __y);
#define pown(__x,__y) __acl__pownfd(__x, __y)

int __PUREF __acl__isorderedf(float __x, float __y);
#define isorderedf(__x, __y) __acl__isorderedf(__x, __y)
int __PUREF __acl__isorderedfd(double __x, double __y);
#define isordered(__x, __y) __acl__isorderedfd(__x, __y)

#else // HLS_SYNTHESIS
// Yuck, need to wrap in a function so it can be substituted everywere
inline float rsqrtf(float __x) {
  return 1.0f/sqrtf(__x);
}
inline double rsqrt(double __x) {
  return 1.0/sqrt(__x);
}

#define sinpif(__x) (sinf((__x) * (float)M_PI))
#define sinpi(__x) (sin((__x) * (double)M_PI))

#define cospif(__x) (cosf((__x) * (float)M_PI))
#define cospi(__x) (cos((__x) * (double)M_PI))

#define tanpif(__x) (tanf((__x) * (float)M_PI))
#define tanpi(__x) (tan((__x) * (double)M_PI))

#define asinpif(__x) (asinf(__x) / (float)M_PI)
#define asinpi(__x) (asin(__x) / (double)M_PI)

#define acospif(__x) (acosf(__x) / (float)M_PI)
#define acospi(__x) (acos(__x) / (double)M_PI)

#define atanpif(__x) (atanf(__x) / (float)M_PI)
#define atanpi(__x) (atan(__x) / (double)M_PI)

#define powrf(__x,__y) (__acl__powrf_approx(__x,__y))
#define powr(__x,__y) (__acl__powr_approx(__x,__y))

#define pownf(__x,__y) (powf(__x,(float)__y))
#define pown(__x,__y) (pow(__x,(double)__y))

#define isorderedf(__x,__y) (!isnan(__x) && !isnan(__y))
#define isordered(__x,__y) (!isnan(__x) && !isnan(__y))

#endif // HLS_SYNTHESIS

#if defined(_MSC_VER)
#define popcountc(__x)  ((unsigned char)     __popcnt  ((unsigned)__x))
#define popcounts(__x)  ((unsigned short)    __popcnt  ((unsigned)__x))
#define popcount(__x)   ((unsigned)          __popcnt  ((unsigned)__x))
#define popcountl(__x)  ((unsigned long)     __popcnt  ((unsigned)__x))
#define popcountll(__x) ((unsigned long long)__popcnt64((unsigned long long)__x))

#else
#define popcountc(__x)  ((unsigned char)     __builtin_popcount ((unsigned)__x))
#define popcounts(__x)  ((unsigned short)    __builtin_popcount ((unsigned)__x))
#define popcount(__x)   ((unsigned)          __builtin_popcount ((unsigned)__x))
#define popcountl(__x)  ((unsigned long)     __builtin_popcountl((unsigned long)__x))
#define popcountll(__x) ((unsigned long long)__builtin_popcountl((unsigned long)__x))

#endif

#if defined(_MSC_VER)

inline double lgamma_r(double __x, int *__sign) {
  double i = lgamma(__x);
  *__sign = (int)((i < 0.0) ? 1 : 0);
  return i;
}

inline float lgammaf_r(float __x, int *__sign) {
  float i = lgammaf(__x);
  *__sign = (int)((i < 0.0) ? 1 : 0);
  return i;
}

#if defined(HLS_SYNTHESIS)
// Windows FPGA flow
double __PUREF __acl__exp10fd(double __x);
#define exp10(__x) __acl__exp10fd(__x)
float __PUREF __acl__exp10f(float __x);
#define exp10f(__x) __acl__exp10f(__x)
#else
// Windows x86 flow
#define exp10(__x) exp( __x * log(10.0))
#define exp10f(__x) expf( __x * logf(10.0f))
#endif // HLS_SYNTHESIS
#endif // MSC_VER

} //Extern "C"
#endif //__HLS_EXTENDEDMATH_H__


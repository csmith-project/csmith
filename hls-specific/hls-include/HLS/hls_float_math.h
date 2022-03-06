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

#ifndef __HLS_FLOAT_MATH__0H__
#define __HLS_FLOAT_MATH__0H__

#include "HLS/hls_float.h"

namespace ihc {

// Currently, the math functions for hls_float only supports up to double
// precision
template <int E, int M> static constexpr void validateFPSupport() {
  constexpr int MaxSupportedExponentWidth = 11;
  constexpr int MaxSupportedMantissaWidth = 52;
  static_assert(
      E <= MaxSupportedExponentWidth && M <= MaxSupportedMantissaWidth,
      "Math function does not support this exponent or mantissa bit widths");
}

template <int E, int M> static constexpr void validateEarlyFPSupport() {
  constexpr int MaxSupportedExponentWidth = 15;
  constexpr int MaxSupportedMantissaWidth = 63;
  static_assert(
      E <= MaxSupportedExponentWidth && M <= MaxSupportedMantissaWidth,
      "Math function does not support this exponent or mantissa bit widths");
}
/////////////////////// Commonly Used Math Operations ////////////////////////

// computes the square root of x -> x^(1/2)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_sqrt(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::sqrt_vpfp_impl(x);
}

// computes the cube root of x -> x^(1/3)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_cbrt(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::cbrt_vpfp_impl(x);
}

// computes the reciprocal(inverse) of x -> x^(-1)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_recip(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::recip_vpfp_impl(x);
}

// computes the reciprocal(inverse) square root of x -> x^(-1/2)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_rsqrt(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::rsqrt_vpfp_impl(x);
}

// computes the hypotenuse of x and y -> srqt(x^2 + y^2)
template <int E1, int M1, int E2, int M2, ihc::fp_config::FP_Round Rnd1,
          ihc::fp_config::FP_Round Rnd2>
hls_float<E1, M1> ihc_hypot(hls_float<E1, M1, Rnd1> const &x,
                            hls_float<E2, M2, Rnd2> const &y) {
  static_assert(
      E1 == E2 && M1 == M2,
      "hypot requires both input arguments to have the same precision");

  validateFPSupport<E1, M1>();
  return internal::hypot_vpfp_impl(x, y);
}

////////////////// Exponential and Logarithmic Functions /////////////////////

// computes e to the power of x -> e^x
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_exp(hls_float<E, M, Rnd> const &x) {
  validateEarlyFPSupport<E, M>();
  return internal::exp_vpfp_impl(x);
}

// computes 2 to the power of x -> 2^x
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_exp2(hls_float<E, M, Rnd> const &x) {
  validateEarlyFPSupport<E, M>();
  return internal::exp2_vpfp_impl(x);
}

// computes 10 to the power of x -> 10^x
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_exp10(hls_float<E, M, Rnd> const &x) {
  validateEarlyFPSupport<E, M>();
  return internal::exp10_vpfp_impl(x);
}

// computes (e to the power of x) -1 -> e^x -1
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_expm1(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::expm1_vpfp_impl(x);
}

// computes the natural log of x -> ln(x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_log(hls_float<E, M, Rnd> const &x) {
  validateEarlyFPSupport<E, M>();
  return internal::log_vpfp_impl(x);
}

// computes the base 2 log of x -> log2(x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_log2(hls_float<E, M, Rnd> const &x) {
  validateEarlyFPSupport<E, M>();
  return internal::log2_vpfp_impl(x);
}

// computes the base 10 log of x -> log10(x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_log10(hls_float<E, M, Rnd> const &x) {
  validateEarlyFPSupport<E, M>();
  return internal::log10_vpfp_impl(x);
}

// computes the natural log of (1+x) -> ln(1+x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_log1p(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::log1p_vpfp_impl(x);
}

///////////////////////// Power Functions ////////////////////////////////

// computes x to the power of y, both types are hls_float -> x^y
template <int E1, int M1, int E2, int M2, ihc::fp_config::FP_Round Rnd1,
          ihc::fp_config::FP_Round Rnd2>
hls_float<E1, M1> ihc_pow(hls_float<E1, M1, Rnd1> const &x,
                          hls_float<E2, M2, Rnd2> const &y) {
  static_assert(E1 == E2 && M1 == M2,
                "pow requires both input arguments to have the same precision");
  validateFPSupport<E1, M1>();
  return internal::pow_vpfp_impl(x, y);
}

// computes x to the power of n, where n is an arbitrary precision ac_int -> x^n
template <int E, int M, ihc::fp_config::FP_Round Rnd, int W, bool S>
hls_float<E, M, Rnd> ihc_pown(hls_float<E, M, Rnd> const &x,
                              ac_int<W, S> const &n) {
  validateFPSupport<E, M>();
  return internal::pown_vpfp_impl(x, n);
}

// overload pown where n is an C++ native integral type. To do so,
// we construct an ac_int that is equivalent to the implementation
// of the native int type
template <int E, int M, ihc::fp_config::FP_Round Rnd, typename T,
          typename = std::enable_if_t<std::is_integral<T>::value>>
hls_float<E, M, Rnd> ihc_pown(hls_float<E, M, Rnd> const &x, T n) {
  validateFPSupport<E, M>();
  constexpr int bit_width = sizeof(T) * 8;
  constexpr bool signedness = std::is_signed<T>::value;
  ac_int<bit_width, signedness> equivalent(n);
  return internal::pown_vpfp_impl(x, equivalent);
}

// computes x to the power of y, with the restriction that x>=0 -> x^y, x>=0
// undefined behavior if x < 0
template <int E1, int M1, int E2, int M2, ihc::fp_config::FP_Round Rnd1,
          ihc::fp_config::FP_Round Rnd2>
hls_float<E1, M1> ihc_powr(hls_float<E1, M1, Rnd1> const &x,
                           hls_float<E2, M2, Rnd2> const &y) {
  static_assert(
      E1 == E2 && M1 == M2,
      "powr requires both input arguments to have the same precision");
  validateFPSupport<E1, M1>();
  return internal::powr_vpfp_impl(x, y);
}

/////////////////////// Trigonometric Functions /////////////////////////////

// computes the sine of x -> sin(x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_sin(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::sin_vpfp_impl(x);
}

// computes the sine of pi*x -> sin( pi*x )
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_sinpi(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::sinpi_vpfp_impl(x);
}

// computes the cosine of -> cos(x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_cos(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::cos_vpfp_impl(x);
}

// computes the cosine of pi*x -> cos( pi*x )
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_cospi(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::cospi_vpfp_impl(x);
}

// computes both the sine and cosine of x, the result for sine is returned
// the result for cosine is stored in the second argument, cos_value
template <int E1, int M1, int E2, int M2, ihc::fp_config::FP_Round Rnd1,
          ihc::fp_config::FP_Round Rnd2>
hls_float<E1, M1> ihc_sincos(hls_float<E1, M1, Rnd1> const &x,
                             hls_float<E2, M2, Rnd2> &cos_value) {
  static_assert(E1 == E2 && M1 == M2,
                "The result of cosine provided in the second argument must be "
                "the same precision as the first argument \"x\"");
  validateFPSupport<E1, M1>();
  return internal::sincos_vpfp_impl(x, cos_value);
}

// computes both the sine and cosine of pi*x, the result for sine is returned
// the result for cosine is stored in the second argument, cos_value
template <int E1, int M1, int E2, int M2, ihc::fp_config::FP_Round Rnd1,
          ihc::fp_config::FP_Round Rnd2>
hls_float<E1, M1> ihc_sincospi(hls_float<E1, M1, Rnd1> const &x,
                               hls_float<E2, M2, Rnd2> &cos_value) {
  static_assert(E1 == E2 && M1 == M2,
                "The result of cosine provided in the second argument must be "
                "the same precision as the first argument \"x\"");
  validateFPSupport<E1, M1>();
  return internal::sincospi_vpfp_impl(x, cos_value);
}

// computes the inverse sine of x -> arcsin(x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_asin(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::asin_vpfp_impl(x);
}

// computes the inverse of sine of pi * x  -> arcsin(x)/pi
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_asinpi(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::asinpi_vpfp_impl(x);
}

// computes the inverse cosine of x -> arccos(x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_acos(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::acos_vpfp_impl(x);
}

// computes the inverse of cosine of pi * x  -> arccos(x)/pi
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_acospi(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::acospi_vpfp_impl(x);
}

// computes the inverse tangent of x -> arctan(x)
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_atan(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::atan_vpfp_impl(x);
}

// computes the inverse of tangent of pi * x  -> arctan(x)/pi
template <int E, int M, ihc::fp_config::FP_Round Rnd>
hls_float<E, M, Rnd> ihc_atanpi(hls_float<E, M, Rnd> const &x) {
  validateFPSupport<E, M>();
  return internal::atanpi_vpfp_impl(x);
}

// computes the inverse tangent of x/y -> arctan(x/y)
template <int E1, int M1, int E2, int M2, ihc::fp_config::FP_Round Rnd1,
          ihc::fp_config::FP_Round Rnd2>
hls_float<E1, M1> ihc_atan2(hls_float<E1, M1, Rnd1> const &x,
                            hls_float<E2, M2, Rnd2> const &y) {
  static_assert(
      E1 == E2 && M1 == M2,
      "atan2 requires both input arguments to have the same precision");
  validateFPSupport<E1, M1>();
  return internal::atan2_vpfp_impl(x, y);
}

} // namespace ihc

#endif // __HLS_FLOAT_MATH__0H__

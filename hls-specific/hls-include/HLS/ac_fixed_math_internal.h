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

#ifndef __HLS_AC_FIXED_MATH_INTERNAL_H__
#define __HLS_AC_FIXED_MATH_INTERNAL_H__

#include "HLS/ac_fixed.h"

// Emulation flow signatures need to be tagged with SYCL_EXTERNAL when using
// this header with the SYCL compiler.
#ifdef __SYCL_COMPILER_VERSION
#define CONDITIONAL_SYCL_EXTERNAL SYCL_EXTERNAL
#else
#define CONDITIONAL_SYCL_EXTERNAL
#endif // __SYCL_COMPILER_VERSION

// Declartion of X86 emulation flow signitures
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_sqrt_x86(int w, int i,
                                                              int s,
                                                              long long x,
                                                              long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL
__ac_fixed_reciprocal_x86(int w, int i, int s, long long x, long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL
__ac_fixed_reciprocal_sqrt_x86(int w, int i, int s, long long x, long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_sin_x86(int w, int i,
                                                             int s, long long x,
                                                             long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_cos_x86(int w, int i,
                                                             int s, long long x,
                                                             long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_sinpi_x86(int w, int i,
                                                               int s,
                                                               long long x,
                                                               long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_cospi_x86(int w, int i,
                                                               int s,
                                                               long long x,
                                                               long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_exp_x86(int w, int i,
                                                             int s, long long x,
                                                             long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_log_x86(int w, int i,
                                                             int s, long long x,
                                                             long long &r);
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_sincos_x86(
    int w, int i, int s, long long x, long long &rs, long long &rc);
extern "C" void CONDITIONAL_SYCL_EXTERNAL __ac_fixed_sincospi_x86(
    int w, int i, int s, long long x, long long &rs, long long &rc);

namespace ac_fixed_math_private {
// Check Here for Type propatation
template <int W, int I, bool S> struct rt {
  enum {
    // sqrt
    sqrt_w = W,
    sqrt_i = I,
    sqrt_s = S,
    // reciprocal
    rcpl_w = W,
    rcpl_i = I,
    rcpl_s = S,
    // reciprocal_sqrt
    rcpl_sqrt_w = W,
    rcpl_sqrt_i = I,
    rcpl_sqrt_s = S,
    // helper
    i_sincos = S ? 2 : 1,
    w_sincos = W - I + i_sincos,
    // sin
    sin_w = w_sincos,
    sin_i = i_sincos,
    sin_s = S,
    // cos
    cos_w = w_sincos,
    cos_i = i_sincos,
    cos_s = S,
    // sinpi
    sinpi_w = W - I + 2,
    sinpi_i = 2,
    sinpi_s = true,
    // cospi
    cospi_w = sinpi_w,
    cospi_i = sinpi_i,
    cospi_s = sinpi_s,
    // exp
    exp_w = W,
    exp_i = I,
    exp_s = true,
    // log
    log_w = W,
    log_i = I,
    log_s = true,
  };
  // typedef ac_fixed<div_w, div_i, div_s> div;
  typedef ac_fixed<W, I, S> input_t;
  typedef ac_fixed<sqrt_w, sqrt_i, sqrt_s> sqrt_r;
  typedef ac_fixed<rcpl_w, rcpl_i, rcpl_s> rcpl_r;
  typedef ac_fixed<rcpl_sqrt_w, rcpl_sqrt_i, rcpl_sqrt_s> rcpl_sqrt_r;
  typedef ac_fixed<sin_w, sin_i, sin_s> sin_r;
  typedef ac_fixed<cos_w, cos_i, cos_s> cos_r;
  typedef ac_fixed<sinpi_w, sinpi_i, sinpi_s> sinpi_r;
  typedef ac_fixed<cospi_w, cospi_i, cospi_s> cospi_r;
  typedef ac_fixed<exp_w, exp_i, exp_s> exp_r;
  typedef ac_fixed<log_w, log_i, log_s> log_r;
};
}; // namespace ac_fixed_math_private

#endif //__HLS_AC_FIXED_MATH_INTERNAL_H__

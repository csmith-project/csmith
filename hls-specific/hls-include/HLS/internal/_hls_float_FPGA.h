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

#ifndef _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_FPGA
#define _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_FPGA

#include "_hls_float_common_inc.h"

// Dummy definitions for math functions to be used when compiling the host code.
// These are needed when compiling with the dpcpp compiler. SYCL requires
// linking the host code with a definition even if the functions are not being
// used on the host.
#if defined(__SYCL_COMPILER_VERSION) && !defined(__SYCL_DEVICE_ONLY__)
template <int Bits> using ap_int = _ExtInt(Bits);
template <unsigned int Bits> using ap_uint = unsigned _ExtInt(Bits);

// In the following built-ins width of arbitrary precision integer type for
// a floating point variable should be equal to sum of corresponding
// exponent width E, mantissa width M and 1 for sign bit. I.e. WA = EA + MA + 1.
template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatCastINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatCastFromIntINTEL(
    ap_int<WA> A, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatCastToIntINTEL(
    ap_int<WA> A, int32_t MA, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatAddINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatSubINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatMulINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatDivINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

// Comparison built-ins don't use Subnormal Support, Rounding Mode and
// Rounding Accuracy.
template <int WA, int WB>
SYCL_EXTERNAL bool __spirv_ArbitraryFloatGTINTEL(ap_int<WA> A, int32_t MA,
                                                 ap_int<WB> B, int32_t MB) {
  return ap_int<2>(0);
}

template <int WA, int WB>
SYCL_EXTERNAL bool __spirv_ArbitraryFloatGEINTEL(ap_int<WA> A, int32_t MA,
                                                 ap_int<WB> B, int32_t MB) {
  return ap_int<2>(0);
}

template <int WA, int WB>
SYCL_EXTERNAL bool __spirv_ArbitraryFloatLTINTEL(ap_int<WA> A, int32_t MA,
                                                 ap_int<WB> B, int32_t MB) {
  return ap_int<2>(0);
}

template <int WA, int WB>
SYCL_EXTERNAL bool __spirv_ArbitraryFloatLEINTEL(ap_int<WA> A, int32_t MA,
                                                 ap_int<WB> B, int32_t MB) {
  return ap_int<2>(0);
}

template <int WA, int WB>
SYCL_EXTERNAL bool __spirv_ArbitraryFloatEQINTEL(ap_int<WA> A, int32_t MA,
                                                 ap_int<WB> B, int32_t MB) {
  return ap_int<2>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatRecipINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatRSqrtINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatCbrtINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatHypotINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatSqrtINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatLogINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatLog2INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatLog10INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatLog1pINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatExpINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatExp2INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatExp10INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatExpm1INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatSinINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatCosINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

// Result value contains both values of sine and cosine and so has the size of
// 2 * Wout where Wout is equal to (1 + Eout + Mout).
template <int WA, int Wout>
SYCL_EXTERNAL ap_int<2 * Wout> __spirv_ArbitraryFloatSinCosINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatSinPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatCosPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

// Result value contains both values of sine(A*pi) and cosine(A*pi) and so has
// the size of 2 * Wout where Wout is equal to (1 + Eout + Mout).
template <int WA, int Wout>
SYCL_EXTERNAL ap_int<2 * Wout> __spirv_ArbitraryFloatSinCosPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatASinINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatASinPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatACosINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatACosPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatATanINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatATanPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatATan2INTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatPowINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout> __spirv_ArbitraryFloatPowRINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}

// PowN built-in calculates `A^B` where `A` is arbitrary precision floating
// point number and `B` is arbitrary precision integer, i.e. its width doesn't
// depend on sum of exponent and mantissa.
template <int WA, int WB, int Wout>
SYCL_EXTERNAL ap_int<Wout>
__spirv_ArbitraryFloatPowNINTEL(ap_int<WA> A, int32_t MA, ap_int<WB> B,
                                int32_t Mout, int32_t EnableSubnormals = 0,
                                int32_t RoundingMode = 0,
                                int32_t RoundingAccuracy = 0) {
  return ap_int<Wout>(0);
}
#endif // __SYCL_COMPILER_VERSION && !__SYCL_DEVICE_ONLY__

// When compiling with dpcpp these will be included from the dpcpp headers
#ifndef __SYCL_COMPILER_VERSION
template <int Bits> using ap_int = _ExtInt(Bits);
template <unsigned int Bits> using ap_uint = unsigned _ExtInt(Bits);

// In the following built-ins width of arbitrary precision integer type for
// a floating point variable should be equal to sum of corresponding
// exponent width E, mantissa width M and 1 for sign bit. I.e. WA = EA + MA + 1.
template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatCastINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatCastFromIntINTEL(
    ap_int<WA> A, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatCastToIntINTEL(
    ap_int<WA> A, int32_t MA, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int WB, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatAddINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int WB, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatSubINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int WB, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatMulINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int WB, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatDivINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) noexcept;

// Comparison built-ins don't use Subnormal Support, Rounding Mode and
// Rounding Accuracy.
template <int WA, int WB>
extern bool __spirv_ArbitraryFloatGTINTEL(ap_int<WA> A, int32_t MA,
                                          ap_int<WB> B, int32_t MB) noexcept;

template <int WA, int WB>
extern bool __spirv_ArbitraryFloatGEINTEL(ap_int<WA> A, int32_t MA,
                                          ap_int<WB> B, int32_t MB) noexcept;

template <int WA, int WB>
extern bool __spirv_ArbitraryFloatLTINTEL(ap_int<WA> A, int32_t MA,
                                          ap_int<WB> B, int32_t MB) noexcept;

template <int WA, int WB>
extern bool __spirv_ArbitraryFloatLEINTEL(ap_int<WA> A, int32_t MA,
                                          ap_int<WB> B, int32_t MB) noexcept;

template <int WA, int WB>
extern bool __spirv_ArbitraryFloatEQINTEL(ap_int<WA> A, int32_t MA,
                                          ap_int<WB> B, int32_t MB) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatRecipINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatRSqrtINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatCbrtINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int WB, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatHypotINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatSqrtINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatLogINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatLog2INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatLog10INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatLog1pINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatExpINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatExp2INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatExp10INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatExpm1INTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatSinINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatCosINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

// Result value contains both values of sine and cosine and so has the size of
// 2 * Wout where Wout is equal to (1 + Eout + Mout).
template <int WA, int Wout>
extern ap_int<2 * Wout> __spirv_ArbitraryFloatSinCosINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatSinPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatCosPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

// Result value contains both values of sine(A*pi) and cosine(A*pi) and so has
// the size of 2 * Wout where Wout is equal to (1 + Eout + Mout).
template <int WA, int Wout>
extern ap_int<2 * Wout> __spirv_ArbitraryFloatSinCosPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatASinINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatASinPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatACosINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatACosPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatATanINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatATanPiINTEL(
    ap_int<WA> A, int32_t MA, int32_t Mout, int32_t EnableSubnormals = 0,
    int32_t RoundingMode = 0, int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int WB, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatATan2INTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int WB, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatPowINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) noexcept;

template <int WA, int WB, int Wout>
extern ap_int<Wout> __spirv_ArbitraryFloatPowRINTEL(
    ap_int<WA> A, int32_t MA, ap_int<WB> B, int32_t MB, int32_t Mout,
    int32_t EnableSubnormals = 0, int32_t RoundingMode = 0,
    int32_t RoundingAccuracy = 0) noexcept;

// PowN built-in calculates `A^B` where `A` is arbitrary precision floating
// point number and `B` is arbitrary precision integer, i.e. its width doesn't
// depend on sum of exponent and mantissa.
template <int WA, int WB, int Wout>
extern ap_int<Wout>
__spirv_ArbitraryFloatPowNINTEL(ap_int<WA> A, int32_t MA, ap_int<WB> B,
                                int32_t Mout, int32_t EnableSubnormals = 0,
                                int32_t RoundingMode = 0,
                                int32_t RoundingAccuracy = 0) noexcept;

#endif // __SYCL_COMPILER_VERSION

namespace ihc {

namespace internal {

using RD_t = fp_config::FP_Round;

///////////////////////////// Binary Operators ///////////////////////////////
template <int RoundingAccuracy, int EnableSubNormals, int Eout, int Mout,
          RD_t Rndout, int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
inline void hls_vpfp_add(hls_float<Eout, Mout, Rndout> &ret,
                         const hls_float<E1, M1, Rnd1> &arg1,
                         const hls_float<E2, M2, Rnd2> &arg2) {
  ac_int<Eout + Mout + 1, false> temp;
  temp._set_value_internal(
      __spirv_ArbitraryFloatAddINTEL<1 + E1 + M1, 1 + E2 + M2, 1 + Eout + Mout>(
          arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2, Mout,
          EnableSubNormals,
          Rnd1 /*Using the rounding mode of the left most operand*/,
          RoundingAccuracy));
  ret.set_bits(temp);
}

template <int RoundingAccuracy, int EnableSubNormals, int Eout, int Mout,
          RD_t Rndout, int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
inline void hls_vpfp_sub(hls_float<Eout, Mout, Rndout> &ret,
                         const hls_float<E1, M1, Rnd1> &arg1,
                         const hls_float<E2, M2, Rnd2> &arg2) {
  ac_int<Eout + Mout + 1, false> temp;
  temp._set_value_internal(
      __spirv_ArbitraryFloatSubINTEL<1 + E1 + M1, 1 + E2 + M2, 1 + Eout + Mout>(
          arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2, Mout,
          EnableSubNormals, Rnd1, RoundingAccuracy));
  ret.set_bits(temp);
}

template <int RoundingAccuracy, int EnableSubNormals, int Eout, int Mout,
          RD_t Rndout, int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
inline void hls_vpfp_mul(hls_float<Eout, Mout, Rndout> &ret,
                         const hls_float<E1, M1, Rnd1> &arg1,
                         const hls_float<E2, M2, Rnd2> &arg2) {
  ac_int<Eout + Mout + 1, false> temp;
  temp._set_value_internal(
      __spirv_ArbitraryFloatMulINTEL<1 + E1 + M1, 1 + E2 + M2, 1 + Eout + Mout>(
          arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2, Mout,
          EnableSubNormals, Rnd1, RoundingAccuracy));
  ret.set_bits(temp);
}

template <int RoundingAccuracy, int EnableSubNormals, int Eout, int Mout,
          RD_t Rndout, int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
inline void hls_vpfp_div(hls_float<Eout, Mout, Rndout> &ret,
                         const hls_float<E1, M1, Rnd1> &arg1,
                         const hls_float<E2, M2, Rnd2> &arg2) {
  ac_int<Eout + Mout + 1, false> temp;
  temp._set_value_internal(
      __spirv_ArbitraryFloatDivINTEL<1 + E1 + M1, 1 + E2 + M2, 1 + Eout + Mout>(
          arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2, Mout,
          EnableSubNormals, Rnd1, RoundingAccuracy));
  ret.set_bits(temp);
}

//////////////////////////// Conversions /////////////////////////////////

// between hls_float
template <int Ein, int Min, int Eout, int Mout, RD_t Rin, RD_t Rout,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
inline void hls_vpfp_cast(const hls_float<Ein, Min, Rin> &from,
                          hls_float<Eout, Mout, Rout> &to) {
  if (Ein == Eout && Min == Mout) {
    to.set_bits(from.get_bits());
  }
  // 4 == RoundtoZero in hls_float.h
  else if (Rout == 4) {
    // We decide to implement RoundToZero in source because
    // the logic is fairly simple and can be easily const-propagated
    hls_vpfp_trunc_convert(from, to);
  } else {
    // for the other rounding blocks we rely on dsdk::A_CAST_BLOCK to handle it
    // for us. This is because the logic is slightly more complicated, the dsdk
    // implementation is more robustly tested and optimized compared to source
    // level implementation.

    ac_int<Eout + Mout + 1, false> temp;

    temp._set_value_internal(
        __spirv_ArbitraryFloatCastINTEL<1 + Ein + Min, 1 + Eout + Mout>(
            from._get_bits_ap_uint(), Min, Mout, EnableSubNormals, Rout,
            RoundingAccuracy));

    to.set_bits(temp);
  }
}

// from integer type to hls_float conversions
template <typename T, int Eout, int Mout, RD_t Rout, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
inline void hls_vpfp_cast_integral(const T &from,
                                   hls_float<Eout, Mout, Rout> &to) {
  static_assert(std::is_integral<T>::value,
                "this function only supports casting from integer types");
  // cast the input integer to an ac_int
  ac_int<sizeof(T) * 8, std::is_signed<T>::value> from_cast(from);

  ac_int<Eout + Mout + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatCastFromIntINTEL<(int)sizeof(T) * 8,
                                             1 + Eout + Mout>(
          from_cast._get_value_internal(), Mout, EnableSubNormals, Rout,
          RoundingAccuracy));

  to.set_bits(temp);
}

// from hls_float to integral type conversions
template <typename T, int Ein, int Min, RD_t Rin, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
inline void hls_vpfp_cast_integral(const hls_float<Ein, Min, Rin> &from,
                                   T &to) {
  static_assert(std::is_integral<T>::value,
                "this function only supports casting to integer types");
  // construct an equivalent ac_int representation of the same integer
  ac_int<sizeof(T) * 8, std::is_signed<T>::value> to_cast;

  to_cast._set_value_internal(
      __spirv_ArbitraryFloatCastToIntINTEL<1 + Ein + Min, (int)sizeof(T) * 8>(
          from._get_bits_ap_uint(), Min, EnableSubNormals, Rin,
          RoundingAccuracy));

  to = (T)to_cast;
}

/////////////////////////// Relational Operator //////////////////////////////

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
inline bool hls_vpfp_gt(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  return __spirv_ArbitraryFloatGTINTEL<1 + E1 + M1, 1 + E2 + M2>(
      arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2);
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
inline bool hls_vpfp_lt(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  return __spirv_ArbitraryFloatLTINTEL<1 + E1 + M1, 1 + E2 + M2>(
      arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2);
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
inline bool hls_vpfp_eq(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  return __spirv_ArbitraryFloatEQINTEL<1 + E1 + M1, 1 + E2 + M2>(
      arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2);
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
inline bool hls_vpfp_ge(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  return __spirv_ArbitraryFloatGEINTEL<1 + E1 + M1, 1 + E2 + M2>(
      arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2);
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
inline bool hls_vpfp_le(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  return __spirv_ArbitraryFloatLEINTEL<1 + E1 + M1, 1 + E2 + M2>(
      arg1._get_bits_ap_uint(), M1, arg2._get_bits_ap_uint(), M2);
}

/////////////////////// Commonly Used Math Operations ////////////////////////

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> sqrt_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatSqrtINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));
  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> cbrt_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatCbrtINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));
  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> recip_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatRecipINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> rsqrt_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatRSqrtINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
hls_float<E1, M1> hypot_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                  hls_float<E2, M2, Rnd2> const &y) {
  hls_float<E1, M1> ret;
  ac_int<E1 + M1 + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatHypotINTEL<1 + E1 + M1, 1 + E2 + M2, 1 + E1 + M1>(
          x._get_bits_ap_uint(), M1, y._get_bits_ap_uint(), M2, M1,
          EnableSubNormals, Rnd1, RoundingAccuracy));
  ret.set_bits(temp);
  return ret;
}

////////////////// Exponential and Logarithmic Functions /////////////////////

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> exp_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(__spirv_ArbitraryFloatExpINTEL<1 + E + M, 1 + E + M>(
      x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd, RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> exp2_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatExp2INTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> exp10_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatExp10INTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> expm1_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatExpm1INTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> log_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(__spirv_ArbitraryFloatLogINTEL<1 + E + M, 1 + E + M>(
      x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd, RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> log2_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatLog2INTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> log10_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatLog10INTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> log1p_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatLog1pINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

///////////////////////// Power Functions ////////////////////////////////

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
hls_float<E1, M1> pow_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                hls_float<E2, M2, Rnd2> const &y) {
  hls_float<E1, M1> ret;
  ac_int<E1 + M1 + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatPowINTEL<1 + E1 + M1, 1 + E2 + M2, 1 + E1 + M1>(
          x._get_bits_ap_uint(), M1, y._get_bits_ap_uint(), M2, M1,
          EnableSubNormals,
          Rnd1 /*Using the rounding mode of the left most operand*/,
          RoundingAccuracy));
  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int W, bool S, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> pown_vpfp_impl(hls_float<E, M, Rnd> const &x,
                                    ac_int<W, S> const &n) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatPowNINTEL<1 + E + M, W, 1 + E + M>(
          x._get_bits_ap_uint(), M, n._get_value_internal(), M,
          EnableSubNormals, Rnd, RoundingAccuracy));
  ret.set_bits(temp);
  return ret;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
hls_float<E1, M1> powr_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                 hls_float<E2, M2, Rnd2> const &y) {
  hls_float<E1, M1> ret;
  ac_int<E1 + M1 + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatPowRINTEL<1 + E1 + M1, 1 + E2 + M2, 1 + E1 + M1>(
          x._get_bits_ap_uint(), M1, y._get_bits_ap_uint(), M2, M1,
          EnableSubNormals,
          Rnd1 /*Using the rounding mode of the left most operand*/,
          RoundingAccuracy));
  ret.set_bits(temp);
  return ret;
}

/////////////////////// Trigonometric Functions /////////////////////////////

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
inline hls_float<E, M, Rnd> sin_vpfp_impl(const hls_float<E, M, Rnd> &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(__spirv_ArbitraryFloatSinINTEL<1 + E + M, 1 + E + M>(
      x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd, RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> sinpi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatSinPiINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> cos_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(__spirv_ArbitraryFloatCosINTEL<1 + E + M, 1 + E + M>(
      x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd, RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> cospi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatCosPiINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
hls_float<E1, M1> sincos_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                   hls_float<E2, M2, Rnd2> &cos_value) {
  static_assert(E1 == E2,
                "Exponent width of the cos value argument is different from "
                "width of the input to the sincos function call.");
  static_assert(M1 == M2,
                "Mantissa width of the cos value argument is different from "
                "width of the input to the sincos function call.");

  hls_float<E1, M1> ret;
  const int FPWidth = E1 + M1 + 1;
  // contains both values
  ac_int<FPWidth + FPWidth, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatSinCosINTEL<1 + E1 + M1, 1 + E2 + M2>(
          x._get_bits_ap_uint(), M1, M2, EnableSubNormals, Rnd1,
          RoundingAccuracy));
  ret.set_bits(temp.template slc<FPWidth>(0));
  cos_value.set_bits(temp.template slc<FPWidth>(FPWidth));
  return ret;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
hls_float<E1, M1> sincospi_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                     hls_float<E2, M2, Rnd2> &cos_value) {
  static_assert(E1 == E2,
                "Exponent width of the cos value argument is different from "
                "width of the input to the sincospi function call.");
  static_assert(M1 == M2,
                "Mantissa width of the cos value argument is different from "
                "width of the input to the sincospi function call.");

  hls_float<E1, M1> ret;
  const int FPWidth = E1 + M1 + 1;
  // contains both values
  ac_int<FPWidth + FPWidth, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatSinCosPiINTEL<1 + E1 + M1, 1 + E2 + M2>(
          x._get_bits_ap_uint(), M1, M2, EnableSubNormals, Rnd1,
          RoundingAccuracy));
  ret.set_bits(temp.template slc<FPWidth>(0));
  cos_value.set_bits(temp.template slc<FPWidth>(FPWidth));
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> asin_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatASinINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> asinpi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatASinPiINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> acos_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatACosINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> acospi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatACosPiINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> atan_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatATanINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E, int M, RD_t Rnd, int RoundingAccuracy = 0,
          int EnableSubNormals = 0>
hls_float<E, M, Rnd> atanpi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> ret;
  ac_int<E + M + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatATanPiINTEL<1 + E + M, 1 + E + M>(
          x._get_bits_ap_uint(), M, M, EnableSubNormals, Rnd,
          RoundingAccuracy));

  ret.set_bits(temp);
  return ret;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int RoundingAccuracy = 0, int EnableSubNormals = 0>
hls_float<E1, M1> atan2_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                  hls_float<E2, M2, Rnd2> const &y) {
  hls_float<E1, M1> ret;
  ac_int<E1 + M1 + 1, false> temp;

  temp._set_value_internal(
      __spirv_ArbitraryFloatATan2INTEL<1 + E1 + M1, 1 + E2 + M2, 1 + E1 + M1>(
          x._get_bits_ap_uint(), M1, y._get_bits_ap_uint(), M2, M1,
          EnableSubNormals,
          Rnd1 /*Using the rounding mode of the left most operand*/,
          RoundingAccuracy));
  ret.set_bits(temp);
  return ret;
}

} // namespace internal
} //  namespace ihc
#endif // _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_FPGA_

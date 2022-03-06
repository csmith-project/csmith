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

#ifndef _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_INTER
#define _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_INTER

#include <stddef.h>

// Emulation flow signatures need to be tagged tiwh SYCL_EXTERNAL when using
// this header with the SYCL compiler.
#ifdef __SYCL_COMPILER_VERSION
#define CONDITIONAL_SYCL_EXTERNAL SYCL_EXTERNAL
#else
#define CONDITIONAL_SYCL_EXTERNAL
#endif // __SYCL_COMPILER_VERSION

#ifndef __MSVC_DECLSPEC__
#ifdef _MSC_VER
// The dllexport attribute is set in the library implementation. Here we specify
// the dllimport attribute.
#define __MSVC_DECLSPEC__ __declspec(dllimport)
#else
#define __MSVC_DECLSPEC__
#endif
#endif

namespace ihc {
namespace internal {

////////////////////////////////////////////////////////////////////////////////
// Intermediate Class Type
////////////////////////////////////////////////////////////////////////////////

/// Intermediate class to represent an arbitrary precision floating point type
/// between the HLD compiler and the low-level implementation for x86
class __MSVC_DECLSPEC__ hls_vpfp_intermediate_ty {
public:
  using inter_storage_ty = unsigned char; /// Underlying data type

private:
  const size_t
      m_mantissa_bits; /// Number of bits needed to represent the mantissa
  const size_t
      m_exponent_bits; /// Number of bits needed to represent the exponent
  inter_storage_ty *m_mantissa; /// Stores the actual mantissa
  inter_storage_ty *m_exponent; /// Stores the actual exponent
  bool m_sign_bit;              /// Contains the sign bit

public:
  /// Constructor that takes in a fixed size for the float type
  /// Allocates the memory needed to store the data type. Note that the data
  /// type cannot be changed after the fact.
  CONDITIONAL_SYCL_EXTERNAL hls_vpfp_intermediate_ty() = delete;
  CONDITIONAL_SYCL_EXTERNAL hls_vpfp_intermediate_ty(const size_t E,
                                                     const size_t M);
  CONDITIONAL_SYCL_EXTERNAL
  hls_vpfp_intermediate_ty(const hls_vpfp_intermediate_ty &input);
  /// Destructor - cleans up the memory used by the underlying data
  CONDITIONAL_SYCL_EXTERNAL ~hls_vpfp_intermediate_ty();
  CONDITIONAL_SYCL_EXTERNAL hls_vpfp_intermediate_ty &
  operator=(const hls_vpfp_intermediate_ty &input);
  /// Return a handle to the underlying exponent array
  CONDITIONAL_SYCL_EXTERNAL inter_storage_ty *get_exponent() const {
    return m_exponent;
  }
  /// Return a handle to the underlying mantissa array
  CONDITIONAL_SYCL_EXTERNAL inter_storage_ty *get_mantissa() const {
    return m_mantissa;
  }
  /// Returns the sign bit
  CONDITIONAL_SYCL_EXTERNAL bool get_sign_bit() const { return m_sign_bit; }
  /// Returns the number of bits needed to store the exponent
  CONDITIONAL_SYCL_EXTERNAL size_t get_exponent_bits() const {
    return m_exponent_bits;
  }
  /// Returns the number of words needed to store the exponent
  CONDITIONAL_SYCL_EXTERNAL size_t get_exponent_words() const {
    return getWordSize(m_exponent_bits);
  }
  /// Returns the number of bits needed to store the mantissa
  CONDITIONAL_SYCL_EXTERNAL size_t get_mantissa_bits() const {
    return m_mantissa_bits;
  }
  /// Returns the number of words needed to store the mantissa
  CONDITIONAL_SYCL_EXTERNAL size_t get_mantissa_words() const {
    return getWordSize(m_mantissa_bits);
  }
  /// Copies the data from the input array to the internal exponent array
  CONDITIONAL_SYCL_EXTERNAL void set_exponent(const inter_storage_ty *input);
  /// Copies the data from the input array to the internal mantissa array
  CONDITIONAL_SYCL_EXTERNAL void set_mantissa(const inter_storage_ty *input);
  /// Sets the sign bit
  CONDITIONAL_SYCL_EXTERNAL void set_sign_bit(bool input) {
    m_sign_bit = input;
  }
  /// Returns true if the store floating point number represents NaN
  CONDITIONAL_SYCL_EXTERNAL bool isNaN() const;
  /// Returns true if the store floating point number represents Inf. The sign
  /// bit has to be checked to see if this is Inf or -Inf
  CONDITIONAL_SYCL_EXTERNAL bool isInf() const;
  /// Returns true if this is a subnormal number
  CONDITIONAL_SYCL_EXTERNAL bool isSubnormal() const;
  /// Returns true if the value is zero (pos or neg)
  CONDITIONAL_SYCL_EXTERNAL bool isZero() const;

private:
  /// Returns the number of words of the underlying data type needed to store
  /// a given number of bits.
  CONDITIONAL_SYCL_EXTERNAL size_t getWordSize(size_t num_bits) const;
  /// Returns true if the exponent is all 1's (barring any padding bits)
  CONDITIONAL_SYCL_EXTERNAL bool isExponentFF() const;
  /// Returns true if the mantissa is all 0's (barring any padding bits)
  CONDITIONAL_SYCL_EXTERNAL bool isExponentZero() const;
  /// Returns true if the mantissa is all 1's (barring any padding bits)
  CONDITIONAL_SYCL_EXTERNAL bool isMantissaZero() const;
};

////////////////////////////////////////////////////////////////////////////////
// Library Function Signatures
////////////////////////////////////////////////////////////////////////////////

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL int
hls_inter_get_str(char *allocated_buffer, size_t buffer_size,
                  const hls_vpfp_intermediate_ty &input, int base,
                  int rounding);

/////////////////////// Basic Arithmetic Operations ////////////////////////////

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_add(hls_vpfp_intermediate_ty &result,
              const hls_vpfp_intermediate_ty &input_A,
              const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_sub(hls_vpfp_intermediate_ty &result,
              const hls_vpfp_intermediate_ty &input_A,
              const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_div(hls_vpfp_intermediate_ty &result,
              const hls_vpfp_intermediate_ty &input_A,
              const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_mul(hls_vpfp_intermediate_ty &result,
              const hls_vpfp_intermediate_ty &input_A,
              const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

////////////////////////// Comparison Operations ///////////////////////////////
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL bool
hls_inter_gt(const hls_vpfp_intermediate_ty &input_A,
             const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL bool
hls_inter_ge(const hls_vpfp_intermediate_ty &input_A,
             const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL bool
hls_inter_lt(const hls_vpfp_intermediate_ty &input_A,
             const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL bool
hls_inter_le(const hls_vpfp_intermediate_ty &input_A,
             const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL bool
hls_inter_eq(const hls_vpfp_intermediate_ty &input_A,
             const hls_vpfp_intermediate_ty &input_B, const bool subnormal);

/////////////////////// Commonly Used Math Operations //////////////////////////
/// computes the square root of x -> x^(1/2)
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_sqrt(hls_vpfp_intermediate_ty &result,
               const hls_vpfp_intermediate_ty &input,
               const bool subnormal = false);

/// computes the cube root of x -> x^(1/3)
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_cbrt(hls_vpfp_intermediate_ty &result,
               const hls_vpfp_intermediate_ty &input,
               const bool subnormal = false);

/// computes the reciprocal of x -> 1/x
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_recip(hls_vpfp_intermediate_ty &result,
                const hls_vpfp_intermediate_ty &input,
                const bool subnormal = false);

/// computes the reciprocal sqrt of x -> 1/sqrt(x)
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_rsqrt(hls_vpfp_intermediate_ty &result,
                const hls_vpfp_intermediate_ty &input,
                const bool subnormal = false);

/// computes the hypotenuse of x and y -> srqt(x^2 + y^2)
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void hls_inter_hypot(
    hls_vpfp_intermediate_ty &result, const hls_vpfp_intermediate_ty &input_A,
    const hls_vpfp_intermediate_ty &input_B, const bool subnormal = false);

////////////////// Exponential and Logarithmic Functions /////////////////////
/// computes e to the power of x -> e^x
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_exp(hls_vpfp_intermediate_ty &result,
              const hls_vpfp_intermediate_ty &input_A,
              const bool subnormal = false);

/// computes 2 to the power of x -> 2^x
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_exp2(hls_vpfp_intermediate_ty &result,
               const hls_vpfp_intermediate_ty &input_A,
               const bool subnormal = false);

/// computes 10 to the power of x -> 10^x
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_exp10(hls_vpfp_intermediate_ty &result,
                const hls_vpfp_intermediate_ty &input_A,
                const bool subnormal = false);

/// computes (e to the power of x) -1 -> e^x -1
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_expm1(hls_vpfp_intermediate_ty &result,
                const hls_vpfp_intermediate_ty &input_A,
                const bool subnormal = false);

/// computes the natural log of x -> ln(x)
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_log(hls_vpfp_intermediate_ty &result,
              const hls_vpfp_intermediate_ty &input_A,
              const bool subnormal = false);

/// computes the base 2 log of x -> log2(x)
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_log2(hls_vpfp_intermediate_ty &result,
               const hls_vpfp_intermediate_ty &input_A,
               const bool subnormal = false);

/// computes the base 10 log of x -> log10(x)
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_log10(hls_vpfp_intermediate_ty &result,
                const hls_vpfp_intermediate_ty &input_A,
                const bool subnormal = false);

/// computes the natural log of (1+x) -> ln(1+x)
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_log1p(hls_vpfp_intermediate_ty &result,
                const hls_vpfp_intermediate_ty &input_A,
                const bool subnormal = false);

///////////////////////// Power Functions ///////////////////////////////////
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void hls_inter_pow(
    hls_vpfp_intermediate_ty &result, const hls_vpfp_intermediate_ty &input_A,
    const hls_vpfp_intermediate_ty &input_B, const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_pown(hls_vpfp_intermediate_ty &result,
               const hls_vpfp_intermediate_ty &input_A, const long int input_B,
               const bool subnormal = false);

/////////////////////// Trigonometric Functions /////////////////////////////
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_sin(hls_vpfp_intermediate_ty &result,
              const hls_vpfp_intermediate_ty &input_A,
              const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_sinpi(hls_vpfp_intermediate_ty &result,
                const hls_vpfp_intermediate_ty &input_A,
                const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_cos(hls_vpfp_intermediate_ty &result,
              const hls_vpfp_intermediate_ty &input_A,
              const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_cospi(hls_vpfp_intermediate_ty &result,
                const hls_vpfp_intermediate_ty &input_A,
                const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_asin(hls_vpfp_intermediate_ty &result,
               const hls_vpfp_intermediate_ty &input_A,
               const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_asinpi(hls_vpfp_intermediate_ty &result,
                 const hls_vpfp_intermediate_ty &input_A,
                 const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_acos(hls_vpfp_intermediate_ty &result,
               const hls_vpfp_intermediate_ty &input_A,
               const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_acospi(hls_vpfp_intermediate_ty &result,
                 const hls_vpfp_intermediate_ty &input_A,
                 const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_atan(hls_vpfp_intermediate_ty &result,
               const hls_vpfp_intermediate_ty &input_A,
               const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_atanpi(hls_vpfp_intermediate_ty &result,
                 const hls_vpfp_intermediate_ty &input_A,
                 const bool subnormal = false);

extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void hls_inter_atan2(
    hls_vpfp_intermediate_ty &result, const hls_vpfp_intermediate_ty &input_A,
    const hls_vpfp_intermediate_ty &input_B, const bool subnormal = false);

/////////////////////// Conversion Functions ////////////////////////////////
extern "C" __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL void
hls_inter_convert(hls_vpfp_intermediate_ty &result,
                  const hls_vpfp_intermediate_ty &input, const int rounding = 0,
                  const bool subnormal = true);

extern void __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL
hls_convert_int_to_hls_inter(hls_vpfp_intermediate_ty &result,
                             const long long int &input);

extern void __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL
hls_convert_int_to_hls_inter(hls_vpfp_intermediate_ty &result,
                             const unsigned long long int &input);

extern void __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL
hls_convert_hls_inter_to_int(long long int &result,
                             const hls_vpfp_intermediate_ty &input,
                             bool emit_warnings = false);

extern void __MSVC_DECLSPEC__ CONDITIONAL_SYCL_EXTERNAL
hls_convert_hls_inter_to_int(unsigned long long int &result,
                             const hls_vpfp_intermediate_ty &input,
                             bool emit_warnings = false);

} // namespace internal
} // namespace ihc

#endif

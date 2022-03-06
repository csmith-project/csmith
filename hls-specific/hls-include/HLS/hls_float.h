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

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//   HLS Variable Precision IEEE-754 Style Floating Point Numbers          //
//                                                                         //
//   Author: Xin Li                                                        //
//                                                                         //
// //////////////////////////////////////////////////////////////////////////

//  Source:          hls_float.h
//  Description:     fast arbitrary-precision floating point types
//                     - IEEE-754 style floating point type with E bit
//                       exponent and M bit mantissa:  ihc::hls_float<E, M>
//
//  Created by:      Xin Li
//
//  Notes:
//  Compiler support: Works with the Intel High Level Synthesis Compiler (i++)
//

#ifndef __HLS_FLOAT__0H__
#define __HLS_FLOAT__0H__

#include "HLS/ac_int.h"

#if (defined(E) || defined(M))
#error Either E or M is already defined. Definition conflicts with their usage as template parameters.
#error DO NOT use defines before including third party header files.
#endif

#if !defined(__EMULATION_FLOW__)
#include "HLS/internal/_hls_float_FPGA.h"
#else
#include "HLS/internal/_hls_float_X86-64.h"
#endif

// This internal flag skips the restriction on the discrete set of FP
// parameterizations that i++ supports. No functional correctness is guaranteed
// for the extra parameterizations, extra validation effort it required from the
// user.
#if defined(HLS_EXTENDED_FP_OPS)
#warning HLS_EXTENDED_FP_OPS is defined, Intel HLS Compiler does not provide functional guanrantees for undocumented features
#define HLS_EXTENDED_FP_OPS_ENABLED 1
#else
#define HLS_EXTENDED_FP_OPS_ENABLED 0
#endif

#if defined(VPF_MAX)
#undef VPF_MAX
#endif
#define VPF_MAX(a, b) ((a) > (b) ? (a) : (b))

#if defined(DEBUG_HLS_FLOAT_WARN) && defined(HLS_X86)
#include <iostream>
#endif

namespace ihc {

// hls_float representations of C++ native types
using FPsingle = hls_float<8, 23, fp_config::FP_Round::RNE>;
using FPdouble = hls_float<11, 52, fp_config::FP_Round::RNE>;
using FPhalf = hls_float<5, 10, fp_config::FP_Round::RNE>;
using bfloat16 = hls_float<8, 7, fp_config::FP_Round::RNE>;
using bfloat19 = hls_float<8, 10, fp_config::FP_Round::RNE>;

#ifdef _MSC_VER
using FPLdouble = FPdouble;
// for windows, the long double type is the same as double
// so the bit width is 64
constexpr unsigned hls_long_double_bit_width = 64;
#else // Linux
using FPLdouble = hls_float<15, 63, fp_config::FP_Round::RNE>;
// for GCC on Linux with X86-64 machine, the long double type is Intel 80 bit
// fp80, which has one explicit fraction bit that IEEE format does not require,
// so the bit width is 79
constexpr unsigned hls_long_double_bit_width = 79;
#endif

// Current list of supported FP parameterizations. These choices of exponent and
// mantissa are validated from hardware. The other parameterizations were not
// thoroughly tested and are thus disallowed. If you wish to use other
// parameterizations of E and M than the list below, compile with
// -DHLS_EXTENDED_FP_OPS
template <int E, int M> struct HLS_FP_Supported {
  static const int value = isPrecisionSupported<
      E, M,

      // Supported Types
      Precision<8, 7>,  // same E and M widths as bfloat
      Precision<5, 10>, // same E and M widths as half type
      Precision<8, 10>, Precision<8, 17>,
      Precision<8, 23>, // same E and M as single precision float type
      Precision<8, 26>, Precision<10, 35>, Precision<11, 44>,
      Precision<11, 52>, // same E and M as double precision double type
      Precision<15, 63>, // Extended double precision

      // Used for explicit specialization to mark the template finish
      UnsupportedFP>::value;
};

/////////////////////////////////////////////////////////////////////////////
///////////////// Architecture Agnostic Class Definition ////////////////////

template <int E, int M, fp_config::FP_Round Rnd> class hls_float {

  static_assert(HLS_EXTENDED_FP_OPS_ENABLED || HLS_FP_Supported<E, M>::value,
                "The parameterization for E and M is not supported");

  static_assert(Rnd == fp_config::FP_Round::RNE ||
                    Rnd == fp_config::FP_Round::RZERO,
                "The compiler currently only supports round to nearest(even) "
                "and to zero");

  // bit representation of the floating point number using an unsigned ac_int
  ac_int<E + M + 1, false> fp_bit_value;

public:
  ///////////////////// BIT FIELD SET/GET Functions ////////////////////////////

  constexpr void set_exponent(ac_int<E, false> bits) {
    fp_bit_value.template set_slc<E>(M, bits);
  }

  constexpr void set_mantissa(ac_int<M, false> bits) {
    fp_bit_value.template set_slc<M>(0, bits);
  }

  constexpr void set_sign_bit(ac_int<1, false> bits) {
    fp_bit_value.template set_slc<1>(E + M, bits);
  }

  constexpr void set_bits(ac_int<E + M + 1, false> bits) {
    fp_bit_value = bits;
  }

  constexpr ac_int<E, false> get_exponent() const {
    return fp_bit_value.template slc<E>(M);
  }

  constexpr ac_int<M, false> get_mantissa() const {
    return fp_bit_value.template slc<M>(0);
  }

  constexpr ac_int<1, false> get_sign_bit() const {
    return fp_bit_value.template slc<1>(E + M);
  }

  constexpr ac_int<E + M + 1, false> get_bits() const { return fp_bit_value; }

  //////////////////////// Commonly Used Constants ////////////////////////

  constexpr static hls_float<E, M, Rnd> pos_inf() {
    ac_int<E, false> bit_exponent(0);
    bit_exponent = bit_exponent.bit_complement();
    hls_float<E, M, Rnd> temp;
    temp.set_bits(0);
    temp.set_exponent(bit_exponent);
    return temp;
  }

  constexpr static hls_float<E, M, Rnd> neg_inf() {
    ac_int<E, false> bit_exponent(0);
    bit_exponent = bit_exponent.bit_complement();
    hls_float<E, M, Rnd> temp;
    temp.set_bits(0);
    temp.set_exponent(bit_exponent);
    temp.set_sign_bit(ac_int<1, false>(1));
    return temp;
  }

  constexpr static hls_float<E, M, Rnd> nan() {
    ac_int<E, false> bit_exponent(0);
    bit_exponent = bit_exponent.bit_complement();
    hls_float<E, M, Rnd> temp;
    // sets mantissa LSB to 1
    temp.set_bits(1);
    temp.set_exponent(bit_exponent);
    return temp;
  }

  constexpr static hls_float<E, M, Rnd>
  from_exponent_mantissa_sign(ac_int<E, false> exponent,
                              ac_int<M, false> mantissa,
                              ac_int<1, false> sign) {
    auto ans = hls_float<E, M, Rnd>::nan();
    ans.set_exponent(exponent);
    ans.set_mantissa(mantissa);
    ans.set_sign_bit(sign);
    return ans;
  }
  /////////////////////////// Utility Functions //////////////////////////////

  bool constexpr is_zero() const {
    return !(get_exponent()) && !(get_mantissa());
  }

  bool constexpr is_inf() const {
    return !(get_exponent().bit_complement()) && !(get_mantissa());
  }

  bool constexpr is_nan() const {
    return !(get_exponent().bit_complement()) && get_mantissa();
  }

  hls_float<E, M, Rnd> next_after(hls_float<E, M, Rnd> to) const {
    if (get_sign_bit() && !to.get_sign_bit()) {
      // neg to pos
      return get_next();
    } else if (!get_sign_bit() && to.get_sign_bit()) {
      // pos to neg
      return get_before();
    } else {
      // same sign
      bool both_pos = !get_sign_bit();
      if (get_exponent() > to.get_exponent()) {
        // bigger exponent to smaller
        return both_pos ? get_before() : get_next();
      } else if (get_exponent() < to.get_exponent()) {
        // smaller exponent to bigger
        return both_pos ? get_next() : get_before();
      } else {
        // same exponent
        if (get_mantissa() > to.get_mantissa()) {
          // bigger mantissa to smaller
          return both_pos ? get_before() : get_next();
        } else if (get_mantissa() < to.get_mantissa()) {
          // smaler mantissa to bigger
          return both_pos ? get_next() : get_before();
        } else {
          // same mantissa, all the same
          return to;
        }
      }
    }
  }

  //////////////////////////// Default Rules ////////////////////////////////
  static constexpr auto wE = E;
  static constexpr auto wM = M;
  static constexpr auto wTotal = E + M + 1;
  static constexpr auto DefaultAccuracy = fp_config::FP_Accuracy::DEFAULT;
  static constexpr auto DefaultSubnormal = fp_config::FP_Subnormal::AUTO;
  using RD_t = fp_config::FP_Round;

  // promotion rules for operation with another precision
  template <int E2, int M2, RD_t ret_RD = RD_t::RNE> struct return_type {
    enum {
      op_e = E == E2 ? E : VPF_MAX(E, E2),
      op_m = E == E2  ? VPF_MAX(M, M2)
             : E > E2 ? M
                      : M2,
    };
    // currently, all binary operations by default promotes the operands to the
    // input type with larger representable range, and the output type is also
    // specified as such
    typedef hls_float<op_e, op_m, ret_RD> mul;
    typedef hls_float<op_e, op_m, ret_RD> add;
    typedef hls_float<op_e, op_m, ret_RD> sub;
    typedef hls_float<op_e, op_m, ret_RD> div;
    typedef hls_float<op_e, op_m, ret_RD> cmp;
  };

  ///////////////////////////// CONSTRUCTORS ///////////////////////////////

  // default constructor
  constexpr hls_float<E, M, Rnd>() {}

  // constructor from a different hls_float
  template <int E2, int M2, RD_t R2>
  hls_float<E, M, Rnd>(hls_float<E2, M2, R2> const &rhs) {
    // constructor from other different type, result to float casts
    internal::hls_vpfp_cast(rhs, *this);
  }

  // construction from native float type, treat float as
  // bit accurate instance of hls_float<8, 23>
  hls_float<E, M, Rnd>(float number) {
    bitCaster<float> Caster = {.original = number};
    FPsingle s;
    s.set_bits(Caster.casted);
    internal::hls_vpfp_cast(s, *this);
  }

  // construction from native double types, treat double as
  // bit accurate instance of hls_float<11, 52>
  hls_float<E, M, Rnd>(double number) {
    bitCaster<double> Caster = {.original = number};
    FPdouble d;
    d.set_bits(Caster.casted);
    internal::hls_vpfp_cast(d, *this);
  }

  // construction from native long double types,
  // note that long double is not the same for different OSs
  // Linux: 80 bit Intel FP80, 1 bit explicit mantissa
  // Windows: double
  hls_float<E, M, Rnd>(long double number) {
    FPLdouble ld;
    ld.set_bits(LdoubleToHLSLdouble(number));
    internal::hls_vpfp_cast(FPLdouble(ld), *this);
  }

  // construction from native integral types
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  hls_float<E, M, Rnd>(T number) {

#if defined(DEBUG_HLS_FLOAT_WARN) && defined(HLS_X86)
    std::cout << "warning: initialization from integral value: \"" << number
              << "\" to " << type_name()
              << " may result in loss of information\n";
#endif
    internal::hls_vpfp_cast_integral(number, *this);
  }

  ///////////////////////// ASSIGNMENT OPERATORS ///////////////////////////

  // assignment of the same type
  hls_float<E, M, Rnd> &operator=(const hls_float<E, M, Rnd> &rhs) {
    this->fp_bit_value = rhs.get_bits();
    return *this;
  }

  // assignment of hls_float with a different precision
  template <int E2, int M2, RD_t R2>
  hls_float<E, M, Rnd> &operator=(const hls_float<E2, M2, R2> &rhs) {
    internal::hls_vpfp_cast(rhs, *this);
    return *this;
  }

  // Allow assignment from all C++ native types. Before conversion, double is
  // bit-casted to hls_float<11, 52> float is bit casted to hls_float<8, 23>,
  // long double is bit converted to hls_float<15, 63> on Linux
  hls_float<E, M, Rnd> &operator=(float number) {
    bitCaster<float> Caster = {.original = number};
    FPsingle s;
    s.set_bits(Caster.casted);
    internal::hls_vpfp_cast(s, *this);
    return *this;
  }

  hls_float<E, M, Rnd> &operator=(double number) {
    bitCaster<double> Caster = {.original = number};
    FPdouble d;
    d.set_bits(Caster.casted);
    internal::hls_vpfp_cast(d, *this);
    return *this;
  }

  hls_float<E, M, Rnd> &operator=(long double number) {
    FPLdouble ld;
    ld.set_bits(LdoubleToHLSLdouble(number));
    internal::hls_vpfp_cast(ld, *this);
    return *this;
  }

  // C++ native integral types conversion
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  hls_float<E, M, Rnd> &operator=(T number) {

#if defined(DEBUG_HLS_FLOAT_WARN) && defined(HLS_X86)
    std::cout << "assignment of integral types of value: \"" << number
              << "\" to " << type_name()
              << " may result in loss of information\n";
#endif

    internal::hls_vpfp_cast_integral(number, *this);
    return *this;
  }

  /////////////////////////// CASTING OPERATORS ////////////////////////////

  template <int E2, int M2, RD_t Rounding>
  hls_float<E2, M2, Rounding> convert_to() const {
    hls_float<E2, M2, Rounding> temp;
    internal::hls_vpfp_cast(*this, temp);
    return temp;
  }

  template <int E2, int M2, RD_t R2,
            typename = std::enable_if_t<E2 != E || M2 != M>>
  explicit operator hls_float<E2, M2, R2>() const {
    hls_float<E2, M2, R2> temp;
    internal::hls_vpfp_cast(*this, temp);
    return temp;
  }

  // cast to floating point types will be the same as casting to
  // their hls_float equivalent
  operator float() const {
    FPsingle f;
    internal::hls_vpfp_cast(*this, f);
    bitCaster<float> rev_cast = {.casted = f.get_bits()};
    return rev_cast.original;
  }
  operator double() const {
    FPdouble d;
    internal::hls_vpfp_cast(*this, d);
    bitCaster<double> rev_cast = {.casted = d.get_bits()};
    return rev_cast.original;
  }

  operator long double() const {
    FPLdouble casted_ldouble;
    internal::hls_vpfp_cast(*this, casted_ldouble);
    return HLSLDoubleToLongDouble(casted_ldouble.get_bits());
  }

  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  operator T() const {
    T rep = {};
    internal::hls_vpfp_cast_integral(*this, rep);
    return rep;
  }

  /////////////////// ARITHMETIC (ASSIGNMENT) OPERATORS ////////////////////

  template <int E2, int M2, RD_t R2>
  typename return_type<E2, M2>::add
  operator+(const hls_float<E2, M2, R2> &rhs) const {
    // default promotion rules to unify across all operations
    using arg1_t = typename return_type<E2, M2, Rnd>::add;
    using arg2_t = typename return_type<E2, M2, R2>::add;
    typename return_type<E2, M2>::add ret;
    constexpr bool unify = !(E2 == E && M2 == M);
    if (unify) {
      arg1_t arg1(*this);
      arg2_t arg2(rhs);
      internal::hls_vpfp_add<DefaultAccuracy, DefaultSubnormal>(ret, arg1,
                                                                arg2);
    } else {
      internal::hls_vpfp_add<DefaultAccuracy, DefaultSubnormal>(ret, *this,
                                                                rhs);
    }
    return ret;
  }

  template <int E2, int M2, RD_t R2>
  typename return_type<E2, M2>::sub
  operator-(const hls_float<E2, M2, R2> &rhs) const {
    // default promotion rules to unify across all operations
    using arg1_t = typename return_type<E2, M2, Rnd>::sub;
    using arg2_t = typename return_type<E2, M2, R2>::sub;
    typename return_type<E2, M2>::sub ret;
    constexpr bool unify = !(E2 == E && M2 == M);
    if (unify) {
      arg1_t arg1(*this);
      arg2_t arg2(rhs);
      internal::hls_vpfp_sub<DefaultAccuracy, DefaultSubnormal>(ret, arg1,
                                                                arg2);
    } else {
      internal::hls_vpfp_sub<DefaultAccuracy, DefaultSubnormal>(ret, *this,
                                                                rhs);
    }
    return ret;
  }

  template <int E2, int M2, RD_t R2>
  typename return_type<E2, M2>::mul
  operator*(const hls_float<E2, M2, R2> &rhs) const {
    // default promotion rules to unify across all operations
    using arg1_t = typename return_type<E2, M2, Rnd>::mul;
    using arg2_t = typename return_type<E2, M2, R2>::mul;
    typename return_type<E2, M2>::mul ret;
    constexpr bool unify = !(E2 == E && M2 == M);
    if (unify) {
      arg1_t arg1(*this);
      arg2_t arg2(rhs);
      internal::hls_vpfp_mul<DefaultAccuracy, DefaultSubnormal>(ret, arg1,
                                                                arg2);
    } else {
      internal::hls_vpfp_mul<DefaultAccuracy, DefaultSubnormal>(ret, *this,
                                                                rhs);
    }
    return ret;
  }

  template <int E2, int M2, RD_t R2>
  typename return_type<E2, M2>::div
  operator/(const hls_float<E2, M2, R2> &rhs) const {
    // default promotion rules to unify across all operations
    using arg1_t = typename return_type<E2, M2, Rnd>::div;
    using arg2_t = typename return_type<E2, M2, R2>::div;
    typename return_type<E2, M2>::div ret;
    constexpr bool unify = !(E2 == E && M2 == M);
    if (unify) {
      arg1_t arg1(*this);
      arg2_t arg2(rhs);
      internal::hls_vpfp_div<DefaultAccuracy, DefaultSubnormal>(ret, arg1,
                                                                arg2);
    } else {
      internal::hls_vpfp_div<DefaultAccuracy, DefaultSubnormal>(ret, *this,
                                                                rhs);
    }
    return ret;
  }

  template <int E2, int M2, RD_t R2>
  hls_float<E, M, Rnd> &operator+=(const hls_float<E2, M2, R2> &rhs) {
    constexpr bool unify = !(E2 == E && M2 == M);
    if (unify)
      *this = this->operator+(rhs);
    else {
      internal::hls_vpfp_add<DefaultAccuracy, DefaultSubnormal>(*this, *this,
                                                                rhs);
    }
    return *this;
  }

  template <int E2, int M2, RD_t R2>
  hls_float<E, M, Rnd> &operator-=(const hls_float<E2, M2, R2> &rhs) {
    constexpr bool unify = !(E2 == E && M2 == M);
    if (unify)
      *this = this->operator-(rhs);
    else {
      internal::hls_vpfp_sub<DefaultAccuracy, DefaultSubnormal>(*this, *this,
                                                                rhs);
    }
    return *this;
  }

  template <int E2, int M2, RD_t R2>
  hls_float<E, M, Rnd> &operator*=(const hls_float<E2, M2, R2> &rhs) {
    constexpr bool unify = !(E2 == E && M2 == M);
    if (unify)
      *this = this->operator*(rhs);
    else {
      internal::hls_vpfp_mul<DefaultAccuracy, DefaultSubnormal>(*this, *this,
                                                                rhs);
    }
    return *this;
  }

  template <int E2, int M2, RD_t R2>
  hls_float<E, M, Rnd> &operator/=(const hls_float<E2, M2, R2> &rhs) {
    constexpr bool unify = !(E2 == E && M2 == M);
    if (unify)
      *this = this->operator/(rhs);
    else {
      internal::hls_vpfp_div<DefaultAccuracy, DefaultSubnormal>(*this, *this,
                                                                rhs);
    }
    return *this;
  }

  ///////////////////////// COMPARISON OPERATORS ///////////////////////////

  template <int E2, int M2, RD_t R2>
  bool operator>(const hls_float<E2, M2, R2> &rhs) const {
    using opt_type1 = typename return_type<E2, M2, Rnd>::cmp;
    using opt_type2 = typename return_type<E2, M2, R2>::cmp;
    opt_type1 arg1(*this);
    opt_type2 arg2(rhs);
    return internal::hls_vpfp_gt(arg1, arg2);
  }

  template <int E2, int M2, RD_t R2>
  bool operator<(const hls_float<E2, M2, R2> &rhs) const {
    using opt_type1 = typename return_type<E2, M2, Rnd>::cmp;
    using opt_type2 = typename return_type<E2, M2, R2>::cmp;
    opt_type1 arg1(*this);
    opt_type2 arg2(rhs);
    return internal::hls_vpfp_lt(arg1, arg2);
  }

  template <int E2, int M2, RD_t R2>
  bool operator==(const hls_float<E2, M2, R2> &rhs) const {
    using opt_type1 = typename return_type<E2, M2, Rnd>::cmp;
    using opt_type2 = typename return_type<E2, M2, R2>::cmp;
    opt_type1 arg1(*this);
    opt_type2 arg2(rhs);
    return internal::hls_vpfp_eq(arg1, arg2);
  }

  template <int E2, int M2, RD_t R2>
  bool operator!=(const hls_float<E2, M2, R2> &rhs) const {
    using opt_type1 = typename return_type<E2, M2, Rnd>::cmp;
    using opt_type2 = typename return_type<E2, M2, R2>::cmp;
    bool nan = is_nan() || rhs.is_nan();
    opt_type1 arg1(*this);
    opt_type2 arg2(rhs);
    return nan ? false : !internal::hls_vpfp_eq(arg1, arg2);
  }

  template <int E2, int M2, RD_t R2>
  bool operator>=(const hls_float<E2, M2, R2> &rhs) const {
    using opt_type1 = typename return_type<E2, M2, Rnd>::cmp;
    using opt_type2 = typename return_type<E2, M2, R2>::cmp;
    opt_type1 arg1(*this);
    opt_type2 arg2(rhs);
    return internal::hls_vpfp_ge(arg1, arg2);
  }

  template <int E2, int M2, RD_t R2>
  bool operator<=(const hls_float<E2, M2, R2> &rhs) const {
    using opt_type1 = typename return_type<E2, M2, Rnd>::cmp;
    using opt_type2 = typename return_type<E2, M2, R2>::cmp;
    opt_type1 arg1(*this);
    opt_type2 arg2(rhs);
    return internal::hls_vpfp_le(arg1, arg2);
  }

  ///////////////////// UNARY OPERATORS OPERATORS //////////////////////////

  constexpr hls_float<E, M, Rnd> operator+() const { return *this; }
  constexpr hls_float<E, M, Rnd> operator-() const {
    hls_float<E, M, Rnd> ret = *this;

    auto sign = get_sign_bit();
    sign = ~sign;
    ret.set_sign_bit(sign);
    return ret;
  }
  constexpr hls_float<E, M, Rnd> abs() const {
    hls_float<E, M, Rnd> ret = *this;
    ret.set_sign_bit(0);
    return ret;
  }

  ////////////////////////// EXPLICT FUNCTIONS //////////////////////////////

  template <fp_config::FP_Accuracy Accuracy = DefaultAccuracy,
            fp_config::FP_Subnormal Subnormal = DefaultSubnormal, int E1,
            int M1, int E2, int M2, RD_t R1, RD_t R2>
  hls_float<E, M> static add(const hls_float<E1, M1, R1> &arg1,
                             const hls_float<E2, M2, R2> &arg2) {
    static_assert(!((Accuracy == fp_config::FP_Accuracy::LOW ||
                     Accuracy == fp_config::FP_Accuracy::MEDIUM) &&
                    Subnormal == fp_config::FP_Subnormal::ON),
                  "Subnormals are not supported in low accuracy modes");
    hls_float<E, M> ret;
    hls_float<E, M, R1> op1(arg1);
    hls_float<E, M, R2> op2(arg2);
    internal::hls_vpfp_add<Accuracy, Subnormal>(ret, op1, op2);
    return ret;
  }

  template <fp_config::FP_Accuracy Accuracy = DefaultAccuracy,
            fp_config::FP_Subnormal Subnormal = DefaultSubnormal, int E1,
            int M1, int E2, int M2, RD_t R1, RD_t R2>
  hls_float<E, M> static sub(const hls_float<E1, M1, R1> &arg1,
                             const hls_float<E2, M2, R2> &arg2) {
    static_assert(!((Accuracy == fp_config::FP_Accuracy::LOW ||
                     Accuracy == fp_config::FP_Accuracy::MEDIUM) &&
                    Subnormal == fp_config::FP_Subnormal::ON),
                  "Subnormals are not supported in low accuracy modes");
    hls_float<E, M> ret;
    hls_float<E, M, R1> op1(arg1);
    hls_float<E, M, R2> op2(arg2);
    internal::hls_vpfp_sub<Accuracy, Subnormal>(ret, op1, op2);
    return ret;
  }

  template <fp_config::FP_Accuracy Accuracy = DefaultAccuracy,
            fp_config::FP_Subnormal Subnormal = DefaultSubnormal, int E1,
            int M1, int E2, int M2, RD_t R1, RD_t R2>
  hls_float<E, M> static mul(const hls_float<E1, M1, R1> &arg1,
                             const hls_float<E2, M2, R2> &arg2) {
    static_assert(!((Accuracy == fp_config::FP_Accuracy::LOW ||
                     Accuracy == fp_config::FP_Accuracy::MEDIUM) &&
                    Subnormal == fp_config::FP_Subnormal::ON),
                  "Subnormals are not supported in low accuracy modes");
    hls_float<E, M> ret;
    internal::hls_vpfp_mul<Accuracy, Subnormal>(ret, arg1, arg2);
    return ret;
  }

  template <fp_config::FP_Accuracy Accuracy = DefaultAccuracy,
            fp_config::FP_Subnormal Subnormal = DefaultSubnormal, int E1,
            int M1, int E2, int M2, RD_t R1, RD_t R2>
  hls_float<E, M> static div(const hls_float<E1, M1, R1> &arg1,
                             const hls_float<E2, M2, R2> &arg2) {
    static_assert(!((Accuracy == fp_config::FP_Accuracy::LOW ||
                     Accuracy == fp_config::FP_Accuracy::MEDIUM) &&
                    Subnormal == fp_config::FP_Subnormal::ON),
                  "Subnormals are not supported in low accuracy modes");
    hls_float<E, M> ret;
    hls_float<E, M, R1> op1(arg1);
    hls_float<E, M, R2> op2(arg2);
    internal::hls_vpfp_div<Accuracy, Subnormal>(ret, op1, op2);
    return ret;
  }

#ifndef _HLS_EMBEDDED_PROFILE
  inline std::string type_name() const {

    std::string r = "hls_float<";
#ifdef __EMULATION_FLOW__
    r += std::to_string(E) + ',';
    r += std::to_string(M) + ',';
    switch (Rnd) {
    case RD_t::RNE:
      r += "RNE";
      break;
    case RD_t::RNA:
      r += "RNA";
      break;
    case RD_t::RUP:
      r += "RUP";
      break;
    case RD_t::RDOWN:
      r += "RDOWN";
      break;
    case RD_t::RZERO:
      r += "RZERO";
      break;
    default:
      break;
    }
#endif
    r += '>';
    return r;
  }

  std::string get_str(int base = 10,
                      fp_config::FP_Round rnd = RD_t::RNE) const {
#ifdef __EMULATION_FLOW__
    return internal::get_str_x86(*this, base, rnd);
#else
    return "";
#endif
  }
#endif

  template <int APWidth> using _fp_as_ap_uint = unsigned _ExtInt(APWidth);
  const _fp_as_ap_uint<E + M + 1> _get_bits_ap_uint() const {
    return fp_bit_value._get_value_internal();
  }

private:
  hls_float<E, M, Rnd> get_next() const {
    // some corner cases
    if (is_nan()) {
      return hls_float<E, M, Rnd>::nan();
    } else if (is_inf()) {
      // -inf -> most negative number, +inf -> nan
      if (!get_sign_bit()) {
        return hls_float<E, M, Rnd>::nan();
      }
      ac_int<E + M + 1, false> most_neg = 0;
      // generate all ones except for the LSB of exp
      most_neg = most_neg.bit_complement();
      most_neg.template set_slc<1>(M, ac_int<1, false>(0));
      hls_float<E, M, Rnd> temp;
      temp.set_bits(most_neg);
      return temp;
    } else if (is_zero()) {
      hls_float<E, M, Rnd> temp;
      temp.set_bits(1);
      return temp;
    } else {
      if (!get_sign_bit()) {
        ac_int<M + 1, false> new_m = get_mantissa();
        ac_int<E, false> new_e = get_exponent();
        // increment mantissa
        new_m++;
        // overflow makes exponent goes up by 1
        new_e += new_m.template slc<1>(M);

        hls_float<E, M, Rnd> temp;
        temp.set_exponent(new_e);
        temp.set_mantissa(new_m.template slc<M>(0));
        temp.set_sign_bit(0);
        return temp;
      } else {
        ac_int<M + 1, false> new_m =
            get_mantissa() | (ac_int<M + 1, false>(1) << M);
        ac_int<E, false> new_e = get_exponent();
        // decrement the mantissa
        new_m--;
        // if we underflow we get an 0 in MSB that we used to set 1
        new_e -= new_m.template slc<1>(M).bit_complement();

        hls_float<E, M, Rnd> temp;
        temp.set_exponent(new_e);
        temp.set_mantissa(new_m.template slc<M>(0));
        temp.set_sign_bit(1);
        return temp;
      }
    }
  }

  hls_float<E, M, Rnd> get_before() const {
    // some corner cases
    if (is_nan()) {
      return hls_float<E, M, Rnd>::nan();
    } else if (is_inf()) {
      // -inf -> nan, +inf -> most positive number
      if (get_sign_bit()) {
        return hls_float<E, M, Rnd>::nan();
      }
      // most positive number
      ac_int<E + M + 1, false> most_pos = 0;
      // generate all ones except for the LSB of exp and sign
      most_pos = most_pos.bit_complement();
      most_pos.template set_slc<1>(M, ac_int<1, false>(0));

      hls_float<E, M, Rnd> temp;
      temp.set_bits(most_pos);
      temp.set_sign_bit(0);
      return temp;
    } else if (is_zero()) {
      hls_float<E, M, Rnd> temp;
      temp.set_bits(1);
      temp.set_sign_bit(1);
      return temp;
    } else {
      if (get_sign_bit()) {
        ac_int<M + 1, false> new_m = get_mantissa();
        ac_int<E, false> new_e = get_exponent();
        // increment mantissa
        new_m++;
        // overflow makes exponent goes up by 1
        new_e += new_m.template slc<1>(M);
        // this naturally overflows to +inf after the largest
        hls_float<E, M, Rnd> temp;
        temp.set_exponent(new_e);
        temp.set_mantissa(new_m.template slc<M>(0));
        temp.set_sign_bit(1);
        return temp;
      } else {
        ac_int<M + 1, false> new_m =
            get_mantissa() | (ac_int<M + 1, false>(1) << M);
        ac_int<E, false> new_e = get_exponent();
        // decrement the mantissa
        new_m--;
        // if we underflow we get an 0 in MSB that we used to set 1
        new_e -= new_m.template slc<1>(M).bit_complement();

        hls_float<E, M, Rnd> temp;
        temp.set_exponent(new_e);
        temp.set_mantissa(new_m.template slc<M>(0));
        temp.set_sign_bit(0);
        return temp;
      }
    }
  }
  // these two functions convert to and from x86 fp80 to Intel HLS equivalent
  // extended double precision type note that this does not encode unnormal
  // numbers, i.e. exponent non-zero, bit 63 zero, and mantissa anything such
  // number indicates a normalized value that has undergone precision loss
  ac_int<hls_long_double_bit_width, false> constexpr LdoubleToHLSLdouble(
      long double x) {

    bitCaster<long double> Caster = {.original = (long double)x};
#ifdef _MSC_VER
    // trivial bit cast to double equivalent
    static_assert(sizeof(x) * 8 == hls_long_double_bit_width, "");
    return Caster.casted;
#else
    ac_int<hls_long_double_bit_width, false> rep;
    // drop the explicit 1 bit mantissa
    // set the mantissa bits first
    constexpr int wM = 63;
    constexpr int wE = 15;

    rep.template set_slc<wM>(0, Caster.casted.template slc<wM>(0));
    // set slice for wE + 1(sign) bits, from mantissa width + 1
    rep.template set_slc<wE + 1>(wM,
                                 Caster.casted.template slc<wE + 1>(wM + 1));
    return rep;
#endif
  }

  long double constexpr HLSLDoubleToLongDouble(
      const ac_int<hls_long_double_bit_width, false> x) {

#ifdef _MSC_VER
    // trivial bit cast to double equivalent
    static_assert(sizeof(x) * 8 == hls_long_double_bit_width, "");
    bitCaster<long double> revCaster = {.casted = x};
    return revCaster.original;
#else
    // x is a 79 bit floating point representation with 16 bytes memory
    // footprint, need to inflate it
    ac_int<sizeof(long double) * 8, false> rep;

    constexpr int wM = 63;
    constexpr int wE = 15;

    rep.template set_slc<wM>(0, x.template slc<wM>(0));
    // the extra bit on long double is always set to 1
    rep.template set_slc<1>(wM, ac_int<1, false>(1));
    rep.template set_slc<wE + 1>(wM + 1, x.template slc<wE + 1>(wM));

    bitCaster<long double> revCaster = {.casted = rep};
    return revCaster.original;
#endif
  }

}; // class hls_float

#ifndef _HLS_EMBEDDED_PROFILE
template <int E, int M, fp_config::FP_Round Rnd>
std::ostream &operator<<(std::ostream &out,
                         const hls_float<E, M, Rnd> &number) {
#ifdef __EMULATION_FLOW__
  out << number.get_str();
#endif
  return out;
}
#endif // linux

// free function for defining operations of hls_float with other
// C++ floating point native types
//   <>_OP:  the operation being defined
//   C_TYPE: the type of the other C++ native operand that is not
//           of type hls_float
//   E_t:    the equivalent exponent width of the operand
//   M_t:    the equivalent mantissa width of the operand
//   RTYPE:  return type of the template return type deduction for
//           that specific operation

#define HLS_FLOAT_BIN_OP_WITH_TYPE(BIN_OP, C_TYPE, E_t, M_t, RTYPE)            \
  template <int E, int M, fp_config::FP_Round Rnd>                             \
  inline typename hls_float<E, M>::template return_type<E_t, M_t>::RTYPE       \
  operator BIN_OP(C_TYPE i_op, const hls_float<E, M, Rnd> &op) {               \
    return hls_float<E_t, M_t, Rnd>(i_op).operator BIN_OP(op);                 \
  }                                                                            \
  template <int E, int M, fp_config::FP_Round Rnd>                             \
  inline typename hls_float<E, M>::template return_type<E_t, M_t>::RTYPE       \
  operator BIN_OP(const hls_float<E, M, Rnd> &op, C_TYPE i_op) {               \
    return op.operator BIN_OP(hls_float<E_t, M_t, Rnd>(i_op));                 \
  }

#define HLS_FLOAT_REL_OP_WITH_TYPE(REL_OP, C_TYPE, E_t, M_t, RTYPE)            \
  template <int E, int M, fp_config::FP_Round Rnd>                             \
  inline bool operator REL_OP(C_TYPE i_op, const hls_float<E, M, Rnd> &op) {   \
    return hls_float<E_t, M_t, Rnd>(i_op).operator REL_OP(op);                 \
  }                                                                            \
  template <int E, int M, fp_config::FP_Round Rnd>                             \
  inline bool operator REL_OP(const hls_float<E, M, Rnd> &op, C_TYPE i_op) {   \
    return op.operator REL_OP(hls_float<E_t, M_t, Rnd>(i_op));                 \
  }

#define HLS_FLOAT_ASSIGN_OP_WITH_TYPE(ASSIGN_OP, C_TYPE, E_t, M_t)             \
  template <int E, int M, fp_config::FP_Round Rnd>                             \
  inline hls_float<E, M, Rnd> &operator ASSIGN_OP(hls_float<E, M, Rnd> &op,    \
                                                  C_TYPE op2) {                \
    return op.operator ASSIGN_OP(hls_float<E_t, M_t, Rnd>(op2));               \
  }

#define FLOAT_OPS_WITH_HLS_FLOAT(C_TYPE, E, M)                                 \
  HLS_FLOAT_BIN_OP_WITH_TYPE(*, C_TYPE, E, M, mul)                             \
  HLS_FLOAT_BIN_OP_WITH_TYPE(+, C_TYPE, E, M, add)                             \
  HLS_FLOAT_BIN_OP_WITH_TYPE(-, C_TYPE, E, M, sub)                             \
  HLS_FLOAT_BIN_OP_WITH_TYPE(/, C_TYPE, E, M, div)                             \
                                                                               \
  HLS_FLOAT_REL_OP_WITH_TYPE(==, C_TYPE, E, M, cmp)                            \
  HLS_FLOAT_REL_OP_WITH_TYPE(!=, C_TYPE, E, M, cmp)                            \
  HLS_FLOAT_REL_OP_WITH_TYPE(>, C_TYPE, E, M, cmp)                             \
  HLS_FLOAT_REL_OP_WITH_TYPE(>=, C_TYPE, E, M, cmp)                            \
  HLS_FLOAT_REL_OP_WITH_TYPE(<, C_TYPE, E, M, cmp)                             \
  HLS_FLOAT_REL_OP_WITH_TYPE(<=, C_TYPE, E, M, cmp)                            \
                                                                               \
  HLS_FLOAT_ASSIGN_OP_WITH_TYPE(+=, C_TYPE, E, M)                              \
  HLS_FLOAT_ASSIGN_OP_WITH_TYPE(-=, C_TYPE, E, M)                              \
  HLS_FLOAT_ASSIGN_OP_WITH_TYPE(*=, C_TYPE, E, M)                              \
  HLS_FLOAT_ASSIGN_OP_WITH_TYPE(/=, C_TYPE, E, M)

// free function for treating operation of hls_float with other
// C++ integral native types
//   <>_OP:  the operation being defined

#define INTEGRAL_BIN_OPS_WITH_HLS_FLOAT(BIN_OP)                                \
  template <int E, int M, fp_config::FP_Round Rnd, typename T,                 \
            typename = std::enable_if_t<std::is_integral<T>::value>>           \
  inline hls_float<E, M> operator BIN_OP(const hls_float<E, M, Rnd> &lhs,      \
                                         const T &rhs) {                       \
    return lhs.operator BIN_OP(hls_float<E, M, Rnd>(rhs));                     \
  }                                                                            \
  template <int E, int M, fp_config::FP_Round Rnd, typename T,                 \
            typename = std::enable_if_t<std::is_integral<T>::value>>           \
  inline hls_float<E, M> operator BIN_OP(const T &lhs,                         \
                                         const hls_float<E, M, Rnd> &rhs) {    \
    return hls_float<E, M, Rnd>(lhs).operator BIN_OP(rhs);                     \
  }

#define INTEGRAL_REL_OPS_WITH_HLS_FLOAT(REL_OP)                                \
  template <int E, int M, fp_config::FP_Round Rnd, typename T,                 \
            typename = std::enable_if_t<std::is_integral<T>::value>>           \
  inline bool operator REL_OP(const hls_float<E, M, Rnd> &lhs, const T &rhs) { \
    return lhs.operator REL_OP(hls_float<E, M, Rnd>(rhs));                     \
  }                                                                            \
  template <int E, int M, fp_config::FP_Round Rnd, typename T,                 \
            typename = std::enable_if_t<std::is_integral<T>::value>>           \
  inline bool operator REL_OP(const T &lhs, const hls_float<E, M, Rnd> &rhs) { \
    return hls_float<E, M, Rnd>(lhs).operator REL_OP(rhs);                     \
  }

#define INTEGRAL_ASSIGN_OPS_WITH_HLS_FLOAT(ASSIGN_OP)                          \
  template <int E, int M, fp_config::FP_Round Rnd, typename T,                 \
            typename = std::enable_if_t<std::is_integral<T>::value>>           \
  inline hls_float<E, M, Rnd> &operator ASSIGN_OP(hls_float<E, M, Rnd> &lhs,   \
                                                  const T &rhs) {              \
    return lhs.operator ASSIGN_OP(hls_float<E, M, Rnd>(rhs));                  \
  }

#define DECL_HLS_FLOAT_INEGRRAL_ARITHMEIC()
INTEGRAL_BIN_OPS_WITH_HLS_FLOAT(+)
INTEGRAL_BIN_OPS_WITH_HLS_FLOAT(-)
INTEGRAL_BIN_OPS_WITH_HLS_FLOAT(*)
INTEGRAL_BIN_OPS_WITH_HLS_FLOAT(/)

INTEGRAL_REL_OPS_WITH_HLS_FLOAT(>)
INTEGRAL_REL_OPS_WITH_HLS_FLOAT(<)
INTEGRAL_REL_OPS_WITH_HLS_FLOAT(==)
INTEGRAL_REL_OPS_WITH_HLS_FLOAT(!=)
INTEGRAL_REL_OPS_WITH_HLS_FLOAT(>=)
INTEGRAL_REL_OPS_WITH_HLS_FLOAT(<=)

INTEGRAL_ASSIGN_OPS_WITH_HLS_FLOAT(+=)
INTEGRAL_ASSIGN_OPS_WITH_HLS_FLOAT(-=)
INTEGRAL_ASSIGN_OPS_WITH_HLS_FLOAT(*=)
INTEGRAL_ASSIGN_OPS_WITH_HLS_FLOAT(/=)

// macros expansion for operation for floating point types, arg are all
// explicitly defined onwards
FLOAT_OPS_WITH_HLS_FLOAT(float, FPsingle::wE, FPsingle::wM)
FLOAT_OPS_WITH_HLS_FLOAT(double, FPdouble::wE, FPdouble::wM)
FLOAT_OPS_WITH_HLS_FLOAT(long double, FPLdouble::wE, FPLdouble::wM)
// macros expansion for operation for integral types, simply cast the type to
// the other hls_float operand
DECL_HLS_FLOAT_INEGRRAL_ARITHMEIC()

} // namespace ihc

#endif // __HLS_FLOAT__0H__

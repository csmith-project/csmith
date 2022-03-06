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

#ifndef _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_X86_64
#define _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_X86_64

#include "HLS/internal/_hls_float_interface.h"
#include "_hls_float_common_inc.h"

#include <cassert>
#include <iostream>
#include <limits>

namespace ihc {

namespace internal {

using RD_t = fp_config::FP_Round;

namespace {
// Copy an ac_int to an unsigned char array
template <int N>
void copy_ac_int_to_array(const ac_int<N, false> &ac_val,
                          hls_vpfp_intermediate_ty::inter_storage_ty *arr_val,
                          int num_chars) {
  assert(num_chars == (N / 8 + (N % 8 > 0)));

  for (int idx = 0; (idx * 8) < N; idx++) {
    arr_val[idx] = ac_val.template slc<8>(8 * idx);
  }
}

// Copy an unsigned char array into an ac_int
template <int N>
void copy_array_to_ac_int(
    const hls_vpfp_intermediate_ty::inter_storage_ty *arr_val,
    ac_int<N, false> &ac_val, int num_chars) {
  assert(num_chars == (N / 8 + (N % 8 > 0)));
  static_assert(sizeof(hls_vpfp_intermediate_ty::inter_storage_ty) == 1,
                "Currently, only char arrays are supported.");
  for (int idx = 0; (idx * 8) < N; idx++) {
    if (N - idx * 8 >= 8) {
      // Set 8 bits at a time to set the underlying ac_int
      ac_val.set_slc(8 * idx, ac_int<8, false>(arr_val[idx]));
    } else {
      // In case we have fewer than 8 bits left, set the remaining bits.
      // Note that the last_size is set to 1 in case N % 8 == 0 since we would
      // get a compile time error otherwise. In this case, this code is never
      // exercised.
      constexpr int last_size = N % 8 ? N % 8 : 1;
      constexpr unsigned bit_mask = (1 << last_size) - 1;
      ac_val.set_slc(8 * idx,
                     ac_int<last_size, false>(arr_val[idx] & bit_mask));
    }
  }
}

// Convert hls_float --> vpfp_intermediate_ty
template <int E, int M, RD_t Rnd>
void convert_hls_float_to_vpfp_inter(const hls_float<E, M, Rnd> &input,
                                     hls_vpfp_intermediate_ty &output) {
  constexpr int m_chars =
      M / (8 * sizeof(hls_vpfp_intermediate_ty::inter_storage_ty)) +
      ((M % (8 * sizeof(hls_vpfp_intermediate_ty::inter_storage_ty))) > 0);
  constexpr int e_chars =
      E / (8 * sizeof(hls_vpfp_intermediate_ty::inter_storage_ty)) +
      ((E % (8 * sizeof(hls_vpfp_intermediate_ty::inter_storage_ty))) > 0);

  copy_ac_int_to_array(input.get_exponent(), output.get_exponent(), e_chars);
  copy_ac_int_to_array(input.get_mantissa(), output.get_mantissa(), m_chars);

  output.set_sign_bit(input.get_sign_bit());
}

// Convert vpfp_intermediate_ty -> hls_float
template <int E, int M, RD_t Rnd>
void convert_vpfp_inter_to_hls_float(const hls_vpfp_intermediate_ty &input,
                                     hls_float<E, M, Rnd> &output) {

  ac_int<M, false> mantissa;
  ac_int<E, false> exponent;

  copy_array_to_ac_int(input.get_mantissa(), mantissa,
                       input.get_mantissa_words());
  copy_array_to_ac_int(input.get_exponent(), exponent,
                       input.get_exponent_words());

  output.set_exponent(exponent);
  output.set_mantissa(mantissa);
  output.set_sign_bit(input.get_sign_bit());
}

} // namespace

// between hls_float type conversions
template <int Ein, int Min, int Eout, int Mout, RD_t Rin, RD_t Rout>
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
    hls_vpfp_intermediate_ty input_m(Ein, Min), output_m(Eout, Mout);
    convert_hls_float_to_vpfp_inter(from, input_m);
    // Flush subnormals when the output size is smaller than the input type,
    // but not the other way around
    if (Eout > Ein) {
      hls_inter_convert(output_m, input_m, Rout, true);
    } else {
      hls_inter_convert(output_m, input_m, Rout, false);
    }
    convert_vpfp_inter_to_hls_float(output_m, to);
  }
}

// from integer type to hls_float conversions
template <typename T, int Eout, int Mout, RD_t Rout>
inline void hls_vpfp_cast_integral(const T &from,
                                   hls_float<Eout, Mout, Rout> &to) {
  static_assert(std::is_integral<T>::value,
                "this function only supports casting from integer types");
  static_assert(
      sizeof(from) <= sizeof(long long int),
      "Conversions to hls_float from integer types only works up to 64 bits.");
  hls_vpfp_intermediate_ty output_m(Eout, Mout);
  if (std::is_signed<T>::value) {
    long long int from_converted = from;
    hls_convert_int_to_hls_inter(output_m, from_converted);
    convert_vpfp_inter_to_hls_float(output_m, to);
  } else {
    unsigned long long int from_converted = from;
    hls_convert_int_to_hls_inter(output_m, from_converted);
    convert_vpfp_inter_to_hls_float(output_m, to);
  }
}

// from hls_float to integral type conversions
template <typename T, int Eout, int Mout, RD_t Rout>
inline void hls_vpfp_cast_integral(const hls_float<Eout, Mout, Rout> &from,
                                   T &to) {
  static_assert(std::is_integral<T>::value,
                "this function only supports casting to integer types");
  static_assert(
      sizeof(to) <= sizeof(long long int),
      "Conversions to hls_float from integer types only works up to 64 bits.");
  hls_vpfp_intermediate_ty input_m(Eout, Mout);
#if defined(DEBUG_HLS_FLOAT_WARN) && defined(HLS_X86)
  bool emit_warnings = true;
#else
  bool emit_warnings = false;
#endif

  if (std::is_signed<T>::value) {
    long long int to_converted;
    convert_hls_float_to_vpfp_inter(from, input_m);
    hls_convert_hls_inter_to_int(to_converted, input_m, emit_warnings);

    // Saturate to the possibly smaller data type
    if (sizeof(to) < sizeof(long long int)) {
      const T min_val = std::numeric_limits<T>::min();
      const T max_val = std::numeric_limits<T>::max();
      if (to_converted < min_val) {
        to_converted = min_val;
      } else if (to_converted > max_val) {
        to_converted = max_val;
      }
    }
    to = to_converted;
  } else {
    unsigned long long int to_converted;
    convert_hls_float_to_vpfp_inter(from, input_m);
    hls_convert_hls_inter_to_int(to_converted, input_m, emit_warnings);

    // Saturate to the possibly smaller data type
    if (sizeof(to) < sizeof(long long int)) {
      const T min_val = std::numeric_limits<T>::min();
      const T max_val = std::numeric_limits<T>::max();
      if (to_converted < min_val) {
        to_converted = min_val;
      } else if (to_converted > max_val) {
        to_converted = max_val;
      }
    }
    to = to_converted;
  }
}

///////////////////////////// Binary Operators ///////////////////////////////

template <int AccuracyLevel, int SubnormalSupport, int Eout, int Mout,
          RD_t Rndout, int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
inline void hls_vpfp_add(hls_float<Eout, Mout, Rndout> &ret,
                         const hls_float<E1, M1, Rnd1> &arg1,
                         const hls_float<E2, M2, Rnd2> &arg2) {
  hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2), result_m(Eout, Mout);
  convert_hls_float_to_vpfp_inter(arg1, arg1_m);
  convert_hls_float_to_vpfp_inter(arg2, arg2_m);
  hls_inter_add(result_m, arg1_m, arg2_m,
                fp_config::isSubnormalOn(SubnormalSupport));
  convert_vpfp_inter_to_hls_float(result_m, ret);
}

template <int AccuracyLevel, int SubnormalSupport, int Eout, int Mout,
          RD_t Rndout, int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
inline void hls_vpfp_sub(hls_float<Eout, Mout, Rndout> &ret,
                         const hls_float<E1, M1, Rnd1> &arg1,
                         const hls_float<E2, M2, Rnd2> &arg2) {
  hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2), result_m(Eout, Mout);
  convert_hls_float_to_vpfp_inter(arg1, arg1_m);
  convert_hls_float_to_vpfp_inter(arg2, arg2_m);
  hls_inter_sub(result_m, arg1_m, arg2_m,
                fp_config::isSubnormalOn(SubnormalSupport));
  convert_vpfp_inter_to_hls_float(result_m, ret);
}

template <int AccuracyLevel, int SubnormalSupport, int Eout, int Mout,
          RD_t Rndout, int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
inline void hls_vpfp_mul(hls_float<Eout, Mout, Rndout> &ret,
                         const hls_float<E1, M1, Rnd1> &arg1,
                         const hls_float<E2, M2, Rnd2> &arg2) {
  hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2), result_m(Eout, Mout);
  convert_hls_float_to_vpfp_inter(arg1, arg1_m);
  convert_hls_float_to_vpfp_inter(arg2, arg2_m);
  hls_inter_mul(result_m, arg1_m, arg2_m,
                fp_config::isSubnormalOn(SubnormalSupport));
  convert_vpfp_inter_to_hls_float(result_m, ret);
}

template <int AccuracyLevel, int SubnormalSupport, int Eout, int Mout,
          RD_t Rndout, int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
inline void hls_vpfp_div(hls_float<Eout, Mout, Rndout> &ret,
                         const hls_float<E1, M1, Rnd1> &arg1,
                         const hls_float<E2, M2, Rnd2> &arg2) {
  if (arg1.is_nan() || arg2.is_nan()) {
    // anything that involves nan
    ret = hls_float<Eout, Mout, Rndout>::nan();
  } else if (arg2.is_zero() && !arg1.is_zero()) {
    // none zero over non zero
    ret = arg1.get_sign_bit() ^ arg2.get_sign_bit()
              ? hls_float<Eout, Mout, Rndout>::neg_inf()
              : hls_float<Eout, Mout, Rndout>::pos_inf();
  } else {
    hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2),
        result_m(Eout, Mout);
    convert_hls_float_to_vpfp_inter(arg1, arg1_m);
    convert_hls_float_to_vpfp_inter(arg2, arg2_m);
    hls_inter_div(result_m, arg1_m, arg2_m,
                  fp_config::isSubnormalOn(SubnormalSupport));
    convert_vpfp_inter_to_hls_float(result_m, ret);
  }
}

/////////////////////////// Relational Operator //////////////////////////////

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int AccuracyLevel = 0, int SubnormalSupport = 0>
inline bool hls_vpfp_gt(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2);
  convert_hls_float_to_vpfp_inter(arg1, arg1_m);
  convert_hls_float_to_vpfp_inter(arg2, arg2_m);
  return hls_inter_gt(arg1_m, arg2_m,
                      fp_config::isSubnormalOn(SubnormalSupport));
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int AccuracyLevel = 0, int SubnormalSupport = 0>
inline bool hls_vpfp_lt(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2);
  convert_hls_float_to_vpfp_inter(arg1, arg1_m);
  convert_hls_float_to_vpfp_inter(arg2, arg2_m);
  return hls_inter_lt(arg1_m, arg2_m,
                      fp_config::isSubnormalOn(SubnormalSupport));
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int AccuracyLevel = 0, int SubnormalSupport = 0>
inline bool hls_vpfp_eq(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2);
  convert_hls_float_to_vpfp_inter(arg1, arg1_m);
  convert_hls_float_to_vpfp_inter(arg2, arg2_m);
  return hls_inter_eq(arg1_m, arg2_m,
                      fp_config::isSubnormalOn(SubnormalSupport));
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int AccuracyLevel = 0, int SubnormalSupport = 0>
inline bool hls_vpfp_ge(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2);
  convert_hls_float_to_vpfp_inter(arg1, arg1_m);
  convert_hls_float_to_vpfp_inter(arg2, arg2_m);
  return hls_inter_ge(arg1_m, arg2_m,
                      fp_config::isSubnormalOn(SubnormalSupport));
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2,
          int AccuracyLevel = 0, int SubnormalSupport = 0>
inline bool hls_vpfp_le(const hls_float<E1, M1, Rnd1> &arg1,
                        const hls_float<E2, M2, Rnd2> &arg2) {
  hls_vpfp_intermediate_ty arg1_m(E1, M1), arg2_m(E2, M2);
  convert_hls_float_to_vpfp_inter(arg1, arg1_m);
  convert_hls_float_to_vpfp_inter(arg2, arg2_m);
  return hls_inter_le(arg1_m, arg2_m,
                      fp_config::isSubnormalOn(SubnormalSupport));
}

/////////////////////////////// To STRING ////////////////////////////////////
template <int E, int M, RD_t Rnd>
std::string get_str_x86(hls_float<E, M, Rnd> x, int base, int rounding) {
  // When the base is not recognized, return an empty string.
  if (base < 2 || base > 16) {
    return "";
  }
  hls_vpfp_intermediate_ty argx(E, M);
  convert_hls_float_to_vpfp_inter(x, argx);

  // at least 7 for -@inf@[null]
  // the internal get_str returns us the mantissa and where the decimal point
  // should be normally we need mantissa + 1 at least, but has to be greater
  // than 7
  size_t buffer_size = M + 2 > 7 ? M + 2 : 7;
  char *c_str_raw = new char[buffer_size];
  int dec_pos = hls_inter_get_str(c_str_raw, buffer_size, argx, base, rounding);
  // just to make sure we null terminate, forever
  c_str_raw[buffer_size - 1] = '\0';

  std::string ret = c_str_raw;
  delete[] c_str_raw;
  size_t special_char = ret.find("@");

  bool needs_trimming = false;
  if (special_char != std::string::npos) {
    // special encodings
    std::string new_s;
    for (char &c : ret) {
      if (c != '@') {
        new_s += c;
      }
    }
    ret = new_s;
  } else if (x.is_zero()) {
    ret = x.get_sign_bit() ? "-0.0" : "0.0";
  } else {
    bool neg = ret[0] == '-';
    if (neg)
      ret = ret.substr(1);
    // normal numbers
    if (dec_pos <= 0) {
      std::string pre = "0.";
      for (int i = 0; i > dec_pos; --i) {
        pre += '0';
      }
      ret = pre + ret;
      needs_trimming = true;
    } else if (dec_pos > ret.length()) {
      for (int i = ret.length(); i < dec_pos; ++i) {
        ret += '0';
      }
    } else {
      ret.insert(dec_pos, ".");
      needs_trimming = true;
    }
    std::string lead = neg ? "-" : "";
    ret = lead + ret;
  }
  if (needs_trimming) {
    size_t cut_off = ret.length() - 1;
    // trim trailing zeros
    while (cut_off > 0 && ret[cut_off] == '0' && ret[cut_off - 1] == '0') {
      cut_off--;
    }
    ret = ret.substr(0, cut_off + 1);
  }
  return ret;
}

/////////////////////// Commonly Used Math Operations ////////////////////////

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> sqrt_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_sqrt(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> cbrt_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_cbrt(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> recip_vpfp_impl(hls_float<E, M, Rnd> const &x) {

  if (x.is_zero()) {
    return x.get_sign_bit() ? hls_float<E, M, Rnd>::neg_inf()
                            : hls_float<E, M, Rnd>::pos_inf();
  }

  hls_float<E, M> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_recip(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> rsqrt_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  if (x.is_zero()) {
    return x.get_sign_bit() ? hls_float<E, M, Rnd>::neg_inf()
                            : hls_float<E, M, Rnd>::pos_inf();
  }
  hls_float<E, M> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_rsqrt(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
hls_float<E1, M1> hypot_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                  hls_float<E2, M2, Rnd2> const &y) {
  hls_float<E1, M1> result;

  hls_vpfp_intermediate_ty x_m(E1, M1), y_m(E2, M2), result_m(E1, M1);
  convert_hls_float_to_vpfp_inter(x, x_m);
  convert_hls_float_to_vpfp_inter(y, y_m);
  hls_inter_hypot(result_m, x_m, y_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

////////////////// Exponential and Logarithmic Functions /////////////////////

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> exp_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_exp(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> exp2_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_exp2(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> exp10_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_exp10(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> expm1_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_expm1(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> log_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_log(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> log2_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_log2(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> log10_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_log10(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> log1p_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_log1p(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

///////////////////////// Power Functions ////////////////////////////////

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
hls_float<E1, M1> pow_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                hls_float<E2, M2, Rnd2> const &y) {
  hls_float<E1, M1> result;
  // a few cases here will return nan:
  // inf to the power of zero
  // x and y is nan
  if (x.is_nan() && y.is_nan()) {
    return hls_float<E1, M1, Rnd1>::nan();
  }
  // anything to the power of 0 is 1
  else if (y.is_zero()) {
    return 1.0;
  } else {
    hls_vpfp_intermediate_ty x_m(E1, M1), y_m(E2, M2), result_m(E1, M1);
    convert_hls_float_to_vpfp_inter(x, x_m);
    convert_hls_float_to_vpfp_inter(y, y_m);
    hls_inter_pow(result_m, x_m, y_m);
    convert_vpfp_inter_to_hls_float(result_m, result);
  }
  // for infinity, handle the corner case which we get infinity but should be
  // -inf this happens when the exponent (y) is odd negative integer, x is less
  // than 0, and -0
  if (result.is_inf() && !result.get_sign_bit() && x.get_sign_bit() &&
      y.get_sign_bit()) {
    bool is_float_int_odd = false;
    ac_int<E2, false> exp = y.get_exponent();
    ac_int<M2, false> man = y.get_mantissa();
    ac_int<E2 + 1, true> exp_real =
        ac_int<E2 + 1, true>(exp) -
        ac_int<E2 + 1, true>((ac_int<E2, false>(1) << E2 - 1) - 1);
    if (exp_real >= 0 && exp_real <= M2) {
      // so the assumption here is that, as long as the mantissa is even
      ac_int<M2, false> mask = ((ac_int<M2, false>(1) << (M2 - exp_real)) - 1);
      bool is_int = (man & mask) == 0;
      bool is_odd = false;
      if (man == 0 && exp_real == 0) {
        // this is the number 1
        is_odd = true;
      } else {
        is_odd = man.template slc<1>((unsigned)(M2 - exp_real));
      }
      is_float_int_odd = is_int && is_odd;
    }
    if (is_float_int_odd) {
      return hls_float<E1, M1>::neg_inf();
    }
  }

  return result;
}

template <int E, int M, RD_t Rnd, int W, bool S>
hls_float<E, M, Rnd> pown_vpfp_impl(hls_float<E, M, Rnd> const &x,
                                    ac_int<W, S> const &n) {
  hls_float<E, M> result;
  static_assert(W <= 64, "pown only supports integers of sizes up to 64 bits ");
  if (n == 0) {
    // any number to the power of 0 is 1 - including nan
    return 1.0;
  } else {
    // normal cases
    hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
    convert_hls_float_to_vpfp_inter(x, x_m);
    hls_inter_pown(result_m, x_m, (long int)n);
    convert_vpfp_inter_to_hls_float(result_m, result);
  }
  // for infinity, handle the corner case which it is power to a negative number
  if (x.get_sign_bit()) {
    if (n % 2 == 0) {
      result.set_sign_bit(0);
    } else {
      result.set_sign_bit(1);
    }
  }

  return result;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
hls_float<E1, M1> powr_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                 hls_float<E2, M2, Rnd2> const &y) {
  hls_float<E1, M1> ret;
  // inf to the inf gives inf
  // inf to the -inf gives 0
  if (x.is_inf() && !x.get_sign_bit() && y.is_inf()) {
    ret = y.get_sign_bit() ? hls_float<E1, M1>(0.0)
                           : hls_float<E1, M1>::pos_inf();
  }
  // a few cases here will return nan:
  // negative x (not including -0)
  // inf to the power of zero
  // x or y is nan
  // 0 to the power of 0
  // 1 to the power of inf
  else if ((x.get_sign_bit() && !x.is_zero()) || (x.is_inf() && y.is_zero()) ||
           x.is_nan() || y.is_nan() || (x.is_zero() && y.is_zero()) ||
           (y.is_inf() && x.get_mantissa() == 0 &&
            x.get_exponent().template slc<E1 - 1>(0).bit_complement() == 0)) {

    ret = hls_float<E1, M1, Rnd1>::nan();
  } else {
    hls_vpfp_intermediate_ty x_m(E1, M1), y_m(E2, M2), result_m(E1, M1);
    convert_hls_float_to_vpfp_inter(x, x_m);
    convert_hls_float_to_vpfp_inter(y, y_m);
    hls_inter_pow(result_m, x_m, y_m, false); // no subnormal support
    convert_vpfp_inter_to_hls_float(result_m, ret);
  }

  return ret;
}

/////////////////////// Trigonometric Functions /////////////////////////////

template <int E, int M, RD_t Rnd>
inline hls_float<E, M, Rnd> sin_vpfp_impl(const hls_float<E, M, Rnd> &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_sin(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> sinpi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_sinpi(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> cos_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_cos(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> cospi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_cospi(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
hls_float<E1, M1> sincos_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                   hls_float<E2, M2, Rnd2> &cos_value) {
  cos_value = cos_vpfp_impl(x);
  return sin_vpfp_impl(x);
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
hls_float<E1, M1> sincospi_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                     hls_float<E2, M2, Rnd2> &cos_value) {
  cos_value = cospi_vpfp_impl(x);
  return sinpi_vpfp_impl(x);
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> asin_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_asin(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> asinpi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_asinpi(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> acos_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_acos(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> acospi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_acospi(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> atan_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M, Rnd> result;

  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_atan(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E, int M, RD_t Rnd>
hls_float<E, M, Rnd> atanpi_vpfp_impl(hls_float<E, M, Rnd> const &x) {
  hls_float<E, M> result;
  hls_vpfp_intermediate_ty x_m(E, M), result_m(E, M);
  convert_hls_float_to_vpfp_inter(x, x_m);
  hls_inter_atanpi(result_m, x_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

template <int E1, int M1, int E2, int M2, RD_t Rnd1, RD_t Rnd2>
hls_float<E1, M1> atan2_vpfp_impl(hls_float<E1, M1, Rnd1> const &x,
                                  hls_float<E2, M2, Rnd2> const &y) {
  hls_float<E1, M1> result;
  hls_vpfp_intermediate_ty x_m(E1, M1), y_m(E2, M2), result_m(E1, M1);

  convert_hls_float_to_vpfp_inter(x, x_m);
  convert_hls_float_to_vpfp_inter(y, y_m);
  hls_inter_atan2(result_m, x_m, y_m);
  convert_vpfp_inter_to_hls_float(result_m, result);

  return result;
}

} // namespace internal
} //  namespace ihc
#endif // _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_X86_64

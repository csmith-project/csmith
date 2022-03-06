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

#ifndef _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_COMMON_INC
#define _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_COMMON_INC

#include <type_traits>

namespace ihc {

/////////////////////////// Utility Functions/Types //////////////////////////

template <int E, int M> struct Precision {};
using UnsupportedFP = Precision<-1, -1>;

template <int E, int M, typename P, typename... Args>
struct isPrecisionSupported {
  static const int value = std::is_same<Precision<E, M>, P>::value
                               ? 1
                               : isPrecisionSupported<E, M, Args...>::value;
};

template <int E, int M, typename P>
struct isPrecisionSupported<E, M, P, UnsupportedFP> {
  static const int value = std::is_same<Precision<E, M>, P>::value;
};

template <typename T> union bitCaster {
  T original;
  ac_int<sizeof(T) * 8, false> casted;
};


///////////////////////////  Control of FP Operations //////////////////////////

namespace fp_config {

  // Options for subnormal handling on floating-point operations.
  // Emulation support will always have subnormal support.
  // Note the compiler will error out for unsupported subnormal handling
  // for a given FPGA architecture, operation, and numeric type.
  enum FP_Subnormal {
      // pick the implementation that is most native to target FPGA
      // i.e. Prefer harden DSP over soft IP.
      AUTO = 0,
      ON,      // FP operations will not flush subnormal to 0, more area
      OFF      // FP operations will flush subnormal numbers to 0
  };

  // Options for tuning the area/accuracy trade-offs on various floating point.
  // This option is also ignored for emulation, and the result for emulation
  // always provides the most accurate result a hls_float<E, M> can represent
  enum FP_Accuracy {
      DEFAULT = 0, // use compiler flag (-fp-accuracy) if set, otherwise high
      LOW,         // low area, low accuracy on FP operations
      MEDIUM,
      HIGH         // high area, high accuracy on FP operations
  };

  // Option for the adjusting the rounding modes:
  // This option is currently only supported on dot-product.
  // The choice of rounding does not have any effect for emulation.
  // NOTE: These flag require compiler support, so please make sure
  // your change here is in sync of the compiler support that implements this
  enum FP_Round {
    RNE = 0,    // Round to nearest, ties break to even
    RNA,        // Round to nearest, ties break away
    RUP,        // Round towards positive infinity
    RDOWN,      // Round towards negative infinity
    RZERO       // Round towards zero
  };

  // Interprets the subnormal enum to return a boolean as to whether or not to
  // enable subnormal support for the emulation flow
  inline bool isSubnormalOn(int S) {
    switch (S) {
    case fp_config::FP_Subnormal::ON:
      return true;
    case fp_config::FP_Subnormal::AUTO:
    case fp_config::FP_Subnormal::OFF:
    default:
      return false;
    }
  }
} // namespace fp_config

/////////////////////////////////////////////////////////////////////////////
///////// THE ONLY declaration of the class with default parameter //////////
/////////////////////////////////////////////////////////////////////////////
template <int E, int M, 
          fp_config::FP_Round Rnd = fp_config::FP_Round::RNE>
class hls_float;
/////////////////////////////////////////////////////////////////////////////


// source level implementations that are common for both flows
namespace internal {
template <int Ein, int Min, int Eout, int Mout,
          fp_config::FP_Round Rin, fp_config::FP_Round Rout> 
inline void hls_vpfp_trunc_convert(const hls_float<Ein, Min, Rin> &from,
                                   hls_float<Eout, Mout, Rout> &to) {
  // We decide to implement RoundToZero in source because
  // the logic is fairly simple and can be easily const-propagated
  if (from.is_nan()) {
    to  = hls_float<Eout, Mout, Rout>::nan();
    return;
  }
  auto from_mantissa = from.get_mantissa();
  auto from_exp      = from.get_exponent();

  bool overflow = false;
  bool underflow = false;

  ac_int<Ein, false>  old_offset = ac_int<Ein, false>(1) << Ein-1;
  ac_int<Eout, false> new_offset = ac_int<Eout, false>(1) << Eout-1;

  ac_int<Eout, false> to_exp;
  ac_int<Mout, false> to_mantissa;
  // prepare exponent bits
  if (from_exp == 0 ) {
    // subnormal exp are always 0
    to_exp = 0;
  }
  else if (Ein > Eout) {

    // this case is slightly tricker since we can run into overflow and underflow
    // problems as the old exponent width is able to represent more
    // This line is purely to suppress the ap_int error
    constexpr unsigned EinBits = Ein > 1 ? Ein+1 : 2;
    ac_int<EinBits, true> temp = from_exp;

    // calculate the theoretical new exponent value, this may or may not fit.
    temp -= (old_offset - new_offset);

    // this is used to suppress the ap_int warning
    constexpr unsigned upper_bits = Ein-Eout > 1 ? Ein-Eout : 1;
    if (temp < 0) {
      underflow = true;
    } else if (ac_int<Ein, false>(temp). template slc<upper_bits>(Eout) == 0) {
      // we are sure that temp is >0 here, so stripping the sign is fine
      to_exp = temp.template slc<Eout>(0);
    }
    else {
      overflow = true;
    }
  }
  else if (Ein < Eout){
    // we are always safe to do this
    to_exp = from_exp + ( new_offset - old_offset);
  }
  else {
    to_exp = from_exp;
  }

  // prepare mantissa bits
  if (Min > Mout) {
    to_mantissa = from_mantissa.template slc<Mout>(Min - Mout);
  }
  else if (Min  < Mout) {
    // zero extend it
    to_mantissa = from_mantissa;
    to_mantissa <<= (Mout - Min);
  }
  else {
    to_mantissa = from_mantissa;
  }
  constexpr auto ff = ac_int<Eout,false>(0).bit_complement();
  constexpr auto zeroE = ac_int<Eout,false>(0);
  constexpr auto zeroM = ac_int<Mout,false>(0);
  to.set_exponent( overflow ? ff : (underflow ? zeroE : to_exp));
  to.set_mantissa( overflow || underflow ? zeroM  : to_mantissa);
  to.set_sign_bit(from.get_sign_bit());

}

} // namespace internal
} // namespace ihc

#endif // #ifndef _INTEL_IHC_HLS_INTERNAL_HLS_FLOAT_COMMON_INC

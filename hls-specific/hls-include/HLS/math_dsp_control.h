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
 *  Library function wrapper for DSP usage control on math functions  *
 *                                                                    *
 **********************************************************************/

#ifndef __HLS_MATH_DSP_CONTROL_H__
#define __HLS_MATH_DSP_CONTROL_H__

// This macro will be used to enable code that is meant for HLS compiler's
// emulation flow and SYCL compiler's emulation flow
#if defined(HLS_X86) ||                                                        \
    (defined(__SYCL_COMPILER_VERSION) && defined(FPGA_EMULATOR))
#define __EMULATION_FLOW__
#endif

namespace ihc {

enum class Preference { DSP, Softlogic, Compiler_default };

#if !defined(__EMULATION_FLOW__)

template <typename Function>
void math_prefer_dsp_propagate(Function f)
__attribute__((prefer_dsp))
__attribute__((propagate_dsp_preference))
{
  f();
}

template <typename Function>
void math_prefer_dsp_no_propagate(Function f)
__attribute__((prefer_dsp))
{
  f();
}

template <typename Function>
void math_prefer_softlogic_propagate(Function f)
__attribute__((prefer_softlogic))
__attribute__((propagate_dsp_preference))
{
  f();
}

template <typename Function>
void math_prefer_softlogic_no_propagate(Function f)
__attribute__((prefer_softlogic))
{
  f();
}

template <Preference my_preference = Preference::DSP,
          bool propagate = true, 
          typename Function>
void math_dsp_control(Function f) {
  if (my_preference == Preference::DSP) {
    if (propagate) {
      math_prefer_dsp_propagate(f);
    } else {
      math_prefer_dsp_no_propagate(f);
    }
  } else if (my_preference == Preference::Softlogic) {
    if (propagate) {
      math_prefer_softlogic_propagate(f);
    } else {
      math_prefer_softlogic_no_propagate(f);
    }
  } else { // my_preference == Preference::Compiler_default
    math_prefer_dsp_no_propagate([&]() { f(); });
  }
}

#else

template <Preference my_preference = Preference::DSP,
          bool propagate = true, 
          typename Function>
void math_dsp_control(Function f) {
  f();
}

#endif // !defined(__EMULATION_FLOW__)

} // namespace ihc

#endif // __HLS_MATH_DSP_CONTROL_H__

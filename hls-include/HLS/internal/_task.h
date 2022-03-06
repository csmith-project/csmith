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

/********************************************************
 *                                                      *
 *  Asynchronous Execution Support for System of Tasks  *
 *                                                      *
 ********************************************************/

#ifndef _INTEL_IHC_HLS_TASK
#define _INTEL_IHC_HLS_TASK

#include "HLS/internal/_hls.h"
#include "HLS/internal/_function_traits.h"

#include <type_traits>
#include <utility>

// Make sure the macros to take the user calls into implementations
// is not in effect for the source code of the implementation
#undef launch
#undef collect

// Bring in the platform specific '_task' for composition
#if defined(__INTELFPGA_COMPILER__)  && !defined(HLS_X86)
#include "HLS/internal/_task_FPGA.h"
#else
#include "HLS/internal/_task_X86-64.h"
#endif

namespace ihc {
  namespace internal {
    // Signature for void func()
    using VoidFuncTy = void ();

    // The task is a singleton that is shared between
    // a calculation and its result:
    // * Launch the calculations
    //   task<function>::launch(args...);
    // * Get the results
    //   [ret = ]task<function>::collect();
    //      > blocking until launch is finished
    //      > returns result for non-void function
    //
    // When the main program exits, any pending
    // launches will still be processed in their
    // respective thread
    //
    // The task is implemented as an
    // Adaptor Design Pattern
    // The X86-64 or FPGA implementation is adapted to the desired API
    // This adaptor is straight forward:
    // Both architecture specific implementations are supposed to
    // be interface compatible with this adaptor
    template<auto& f, bool detach = false>
    class task {
    public:
      using F = decltype(f);
      using T = typename ihc::function_traits<F>::return_type;

      // Launch the callable
      template<int capacity, typename ... Args>
      static void launch(Args&& ... args) {
        _t.template launch<capacity>(std::forward<Args>(args)...);
      }

      // Get the result
      template<int capacity>
      static T collect() {
        // Restore void if needed
        return static_cast<T>(_t.template collect<capacity>());
      }

    private:
      // Constructor
      task() {};
      // Destructor
      // Can't be explicit for FPGA target in Intel(R) HLS Compiler
      //~_task() {}

      // Composition (_task is architecture specific)
      // Singleton
      // Anywhere in the function hierarchy where "f" gets called,
      // this specific task (with the corresponding thread and queue)
      // needs to be used
      static internal::_task<f, detach> _t;
    }; // class task

    template <auto& f, bool detach>
    internal::_task<f, detach> task<f, detach>::_t;


    // launch_always_run_helper utilities
    template <VoidFuncTy &f> void launch_always_run() {
      while (1) {
        f();
      }
    }

    template <VoidFuncTy &f> class launch_always_run_helper {
    public:
      launch_always_run_helper() {
        task<launch_always_run<f>, true>::template launch<0>();
      }
    };
  } // namespace internal

  // Launch (through singleton)
  template <auto &f, int capacity = 0, typename... Args>
  void launch(Args &&... args) {
    internal::task<f>::template launch<capacity>(std::forward<Args>(args)...);
  }

  // Collect (through singleton)
  template <auto &f, int capacity = 0>
  typename ihc::function_traits<decltype(f)>::return_type collect() {
    using T = typename ihc::function_traits<decltype(f)>::return_type;
    // Restore void if needed
    return static_cast<T>(internal::task<f>::template collect<capacity>());
  }

  /// Launches a task function at when the component starts up.
  /// The task function to be launched must take no input arguments and have
  /// a void return.
  template <internal::VoidFuncTy &f> void launch_always_run() {
    static internal::launch_always_run_helper<f> t;
  }

} // namespace ihc

// For backwards compatibility
#define launch(x, ...) _Pragma\
    ("GCC warning \"'launch' macro has been deprecated. Use launch<task> instead.\"")\
    launch <x, 0>(__VA_ARGS__)
#define collect(x) _Pragma\
    ("GCC warning \"'collect' macro has been deprecated.\
 Use collect<task> instead.\"")\
    collect<x, 0>()

#endif // _INTEL_IHC_HLS_TASK

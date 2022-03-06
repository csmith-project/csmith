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

/*********************************************************************
 *                                                                   *
 *  Internals of Asynchronous Execution Support for System of Tasks  *
 *  DO NOT REFERENCE DIRECTLY                                        *
 *                                                                   *
 *********************************************************************/

#ifndef _INTEL_IHC_HLS_INTERNAL__TASK_FPGA
#define _INTEL_IHC_HLS_INTERNAL__TASK_FPGA
#include "HLS/internal/_function_traits.h"

namespace ihc {
  namespace internal {
    // The task is a singleton that is shared between
    // a calculation and its result:
    // * Identify the functionality
    //   task<function>::instance()
    // * Launch the calculations
    //   task<function>::instance().launch(args...);
    // * Get the results
    //   [ret = ]task<function>::instance().collect();
    //      > blocking until launch is finished
    //      > returns result for non-void function
    //
    // When the main program exits, any pending
    // launches will still be processed in their
    // respective thread
    template<auto& f, bool detach = false>
    class _task {
    public:
      using F = decltype(f);
      using T = typename ihc::function_traits<F>::return_type;

      // Constructor
      _task() {}

      // Disable copy-assignment operator
      _task& operator=(const _task& rhs) = delete;

      // Disable copy constructor
      _task(const _task& other) = delete;

      // Destructor
      // Can't be explicit for FPGA target in Intel(R) HLS Compiler
      //~_task() {}

      template<int capacity, typename ... Args>
      void launch(Args&&... args) {
        __builtin_intel_hls_enqueue(capacity, f, std::forward<Args>(args)...);
      } // launch

      template<int capacity>
      T collect() {
        return static_cast<T>(__builtin_intel_hls_get(capacity, f));
      }

    }; // class _task

  } //namespace internal
} // namespace ihc

#endif // _INTEL_IHC_HLS_INTERNAL__TASK_FPGA

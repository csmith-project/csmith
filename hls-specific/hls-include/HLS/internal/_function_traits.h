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
 *  Metaprogramming to extract the return type from a function type  *
 *                                                                   *
 *********************************************************************/
#ifndef _INTEL_IHC_HLS_FUNCTION_TRAITS
#define _INTEL_IHC_HLS_FUNCTION_TRAITS

namespace ihc {
  template<typename F>
  struct function_traits {
    using return_type = F;
  };

  template<typename R, typename... Args>
  struct function_traits<R(*)(Args...)>
  {
    using return_type = R;
  };

  template<typename R, typename... Args>
  struct function_traits<R(&)(Args...)>
  {
    using return_type = R;
  };
}

#endif // _INTEL_IHC_HLS_FUNCTION_TRAITS

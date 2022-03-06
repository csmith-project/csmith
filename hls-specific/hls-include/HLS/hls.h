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

/*********************************************
 *                                           *
 *  Basic Support for Intel's HLS Tool       *
 *                                           *
 *********************************************/
#ifndef __HLS_H__
#define __HLS_H__

#include "HLS/internal/_stream.h"
#include "HLS/internal/_memory.h"
#include "HLS/internal/_task.h"
#include "HLS/internal/_hls.h"
#include "HLS/pipes.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4265) // has virtual functions, but destructor is not virtual
#pragma warning(disable:4505) // unreferenced local function has been removed
#endif

#ifdef __INTELFPGA_COMPILER__
// Memory attributes
#define hls_register                                  __attribute__((__register__))
#define hls_memory                                    __attribute__((__memory__))
#define hls_memory_impl(__x)                          __attribute__((__memory__(__x)))
#define hls_force_pow2_depth(__x)                        __attribute__((__force_pow2_depth__(__x)))
#define hls_numbanks(__x)                             __attribute__((__numbanks__(__x)))
#define hls_bankwidth(__x)                            __attribute__((__bankwidth__(__x)))
#define hls_singlepump                                __attribute__((__singlepump__))
#define hls_doublepump                                __attribute__((__doublepump__))
#define hls_bankbits(__x, ...)                        __attribute__((__bank_bits__(__x, ##__VA_ARGS__)))
#define hls_merge(__x, __y)                           __attribute__((merge(__x, __y)))
#define hls_init_on_reset                             __attribute__((__static_array_reset__(1)))
#define hls_init_on_powerup                           __attribute__((__static_array_reset__(0)))
#define hls_simple_dual_port_memory                   __attribute__((simple_dual_port))
#define hls_max_replicates(__x)                       __attribute__((max_replicates(__x)))
#define hls_private_copies(__x)                   __attribute__((__private_copies__(__x)))

// Interface synthesis attributes
#define hls_avalon_agent_component             __attribute__((component_interface("avalon_mm_agent"))) __attribute__((stall_free_return))
#define hls_avalon_agent_register_argument     __attribute__((argument_interface("avalon_mm_agent")))
#define hls_avalon_agent_memory_argument(__x)  __attribute__((local_mem_size(__x))) __attribute__((agent_memory_argument))
#define hls_avalon_slave_component             __attribute__((component_interface("avalon_mm_agent"))) __attribute__((stall_free_return))
#define hls_avalon_slave_register_argument     __attribute__((argument_interface("avalon_mm_agent")))
#define hls_avalon_slave_memory_argument(__x)  __attribute__((local_mem_size(__x))) __attribute__((agent_memory_argument))
#define hls_avalon_streaming_component         __attribute__((component_interface("avalon_streaming")))
#define hls_always_run_component               __attribute__((component_interface("always_run"))) __attribute__((stall_free_return))
#define hls_conduit_argument                   __attribute__((argument_interface("wire")))
#define hls_stable_argument                    __attribute__((stable_argument))
#define hls_stall_free_return                  __attribute__((stall_free_return))
#define hls_readonly                          __attribute__((readwrite_mode("readonly")))
#define hls_writeonly                         __attribute__((readwrite_mode("writeonly")))

// Component attributes
#define hls_max_concurrency(__x)               __attribute__((max_concurrency(__x)))
#define hls_scheduler_target_fmax_mhz(__x)     __attribute__((scheduler_target_fmax_mhz(__x)))
#define hls_component_ii(__x)                  __attribute__((hls_ii(__x)))
#define hls_disable_component_pipelining       __attribute__((hls_force_loop_pipelining("off")))

// Cluster attributes
#define hls_use_stall_enable_clusters          __attribute__((use_stall_enable_clusters))

// fpga_reg support
#define hls_fpga_reg(__x)                      __fpga_reg(__x)

#else // __INTELFPGA_COMPILER__
#define hls_register
#define hls_memory
#define hls_memory_impl(__x)
#define hls_force_pow2_depth(__x)
#define hls_numbanks(__x)
#define hls_bankwidth(__x)
#define hls_singlepump
#define hls_doublepump
#define hls_bankbits(__x, ...)
#define hls_merge(__x, __y)
#define hls_init_on_reset
#define hls_init_on_powerup

#define hls_simple_dual_port_memory
#define hls_max_replicates(__x)
#define hls_private_copies(__x)

#define hls_avalon_agent_component
#define hls_avalon_agent_register_argument
#define hls_avalon_agent_memory_argument(__x)
#define hls_avalon_slave_component
#define hls_avalon_slave_register_argument
#define hls_avalon_slave_memory_argument(__x)

#define hls_avalon_streaming_component
#define hls_always_run_component
#define hls_conduit_argument
#define hls_stable_argument
#define hls_stall_free_return
#define hls_writeonly
#define hls_readonly

#define hls_max_concurrency(__x)
#define hls_scheduler_target_fmax_mhz(__x)
#define hls_component_ii(__x)
#define hls_disable_component_pipelining

#define hls_use_stall_enable_clusters

#define hls_fpga_reg(__x) __x
#endif // __INTELFPGA_COMPILER__

////////////////////////////////////////////////////////////////////////////////
// HLS Cosimulation Support API
////////////////////////////////////////////////////////////////////////////////

#define ihc_hls_enqueue(retptr, func, ...) \
  { \
    if (__ihc_hls_async_call_capable()){ \
      __ihc_enqueue_handle=(retptr); \
      (void) (*(func))(__VA_ARGS__); \
      __ihc_enqueue_handle=0; \
    } else { \
      *(retptr) = (*(func))(__VA_ARGS__); \
    } \
  }

#define ihc_hls_enqueue_noret(func, ...) \
  { \
  __ihc_enqueue_handle=& __ihc_enqueue_handle; \
  (*(func))(__VA_ARGS__); \
  __ihc_enqueue_handle=0; \
  }

#define ihc_hls_component_run_all(component_address) \
  __ihc_hls_component_run_all((void*) (component_address))

#define ihc_hls_set_component_wait_cycle(component_address, num_wait_cycles) \
  __ihc_hls_set_component_wait_cycle((void*) (component_address), num_wait_cycles)

// When running a simulation, this function will issue a reset to all components
// in the testbench
// Returns: 0 if reset did not occur (ie. if the component target is x86)
//          1 if reset occurred (ie. if the component target is an FPGA)
extern "C" int ihc_hls_sim_reset(void);

////////////////////////////////////////////////////////////////////////////////
// HLS Component Built-Ins
////////////////////////////////////////////////////////////////////////////////

//Builtin memory fence function call
#ifdef HLS_X86
inline void ihc_fence() {}

#else
extern "C" void __acl_mem_fence(unsigned int);
inline void ihc_fence() {
  // fence on all types of fences from OpenCL
  __acl_mem_fence(-1);
}
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

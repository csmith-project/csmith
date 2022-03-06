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

#ifndef __HLS_INTERNAL_PIPES__
#define __HLS_INTERNAL_PIPES__

namespace ihc {
namespace internal {

// Struct representing layout of pipe storage
struct ConstantPipeStorage {
  int  _PacketSize;
  int  _PacketAlignment;
  int  _Capacity;
};

#ifdef HLS_X86
template <typename T>
extern void HLS_CreatePipeFromPipeStorage_read(
    const ConstantPipeStorage *Storage) noexcept {};

template <typename T>
extern void HLS_CreatePipeFromPipeStorage_write(
    const ConstantPipeStorage *Storage) noexcept {};

#else // FPGA
// Pipe types
template<typename T>
using RPipe = __attribute__((pipe("read_only"))) const T;

template<typename T>
using WPipe = __attribute__((pipe("write_only"))) const T;

template <typename T>
extern RPipe<T> HLS_CreatePipeFromPipeStorage_read(
    const ConstantPipeStorage *Storage) noexcept;

template <typename T>
extern WPipe<T> HLS_CreatePipeFromPipeStorage_write(
    const ConstantPipeStorage *Storage) noexcept;
#endif

} // namespace internal
} // namespace ihc
#endif // __HLS_INTERNAL_PIPES__

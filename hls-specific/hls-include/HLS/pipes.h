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

#ifndef __HLS_PIPES__
#define __HLS_PIPES__

#ifdef CL_SYCL_LANGUAGE_VERSION
// Defer to SYCL pipes implementation
#include "CL/sycl/pipes.hpp"
namespace ihc {
template<class name, class T, unsigned capacity = 0>
using pipe = cl::sycl::pipe<name, T, capacity>;
}
#else // HLS

#include "HLS/internal/_hls.h"
#include "HLS/internal/_pipes.h"

namespace ihc {

template<class name, class T, unsigned capacity = 0>
class pipe {
public:
  // Non-blocking
  static T read(bool &success);
  static void write(const T &data, bool &success);
  // Blocking
  static T read();
  static void write(const T &data);

private:
  static constexpr int _size = sizeof(T);
  static constexpr int _alignment = alignof(T);
  static constexpr int _capacity = capacity;
  static constexpr internal::ConstantPipeStorage _storage = {_size, _alignment,
                                                             _capacity};
#ifdef HLS_X86
private:
  static internal::stream<T> _stream;
#endif
};

#ifdef HLS_X86
template<class name, class T, unsigned capacity>
internal::stream<T> pipe<name, T, capacity>::_stream;

template<class name, class T, unsigned capacity>
T pipe<name, T, capacity>::read(bool &success) {
  // Need to call dummy function CreatePipeFromPipeStorage
  // so _storage object is visible to ReplaceComponentHLSSim pass.
  internal::HLS_CreatePipeFromPipeStorage_read<T>(&_storage);
  return _stream.tryRead(success);
}

template<class name, class T, unsigned capacity>
void pipe<name, T, capacity>::write(const T &data, bool &success) {
  internal::HLS_CreatePipeFromPipeStorage_write<T>(&_storage);
  success = _stream.tryWrite(data);
}

template<class name, class T, unsigned capacity>
T pipe<name, T, capacity>::read() {
  internal::HLS_CreatePipeFromPipeStorage_read<T>(&_storage);
  return _stream.read(/*wait*/ true);
}

template<class name, class T, unsigned capacity>
void pipe<name, T, capacity>::write(const T &data) {
  internal::HLS_CreatePipeFromPipeStorage_write<T>(&_storage);
  _stream.write(data);
}

#else // FPGA
template<class name, class T, unsigned capacity>
T pipe<name, T, capacity>::read(bool &success) {
  internal::RPipe<T> p =
      internal::HLS_CreatePipeFromPipeStorage_read<T>(&_storage);
  T tempData;
  success = !static_cast<bool>(
      __builtin_intel_hls_read_pipe(p, &tempData, _size, _alignment));
  return tempData;
}

template<class name, class T, unsigned capacity>
void pipe<name, T, capacity>::write(const T &data, bool &success) {
  internal::WPipe<T> p =
      internal::HLS_CreatePipeFromPipeStorage_write<T>(&_storage);
  success = !static_cast<bool>(
      __builtin_intel_hls_write_pipe(p, &data, _size, _alignment));
}

template<class name, class T, unsigned capacity>
T pipe<name, T, capacity>::read() {
  internal::RPipe<T> p =
      internal::HLS_CreatePipeFromPipeStorage_read<T>(&_storage);
  T tempData;
  __builtin_intel_hls_read_pipe_blocking(p, &tempData, _size,
                                         _alignment);
  return tempData;
}

template<class name, class T, unsigned capacity>
void pipe<name, T, capacity>::write(const T &data) {
  internal::WPipe<T> p =
      internal::HLS_CreatePipeFromPipeStorage_write<T>(&_storage);
  __builtin_intel_hls_write_pipe_blocking(p, &data, _size, _alignment);
}
#endif

} // namespace ihc
#endif
#endif // __HLS_PIPES__

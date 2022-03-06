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

#ifndef __IHC_MEMORY_H__
#define __IHC_MEMORY_H__

#include "HLS/internal/_hls.h"
#include "HLS/lsu.h"
#include <type_traits>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(                                                               \
    disable : 4265) // has virtual functions, but destructor is not virtual
#pragma warning(disable : 4505) // unreferenced local function has been removed
#endif

namespace ihc {

////////////////////////////////
/// memory host interface  ///
////////////////////////////////

template <int _N> struct dwidth {
  static constexpr int value = _N;
  static constexpr int defaultValue = 64;
};

template <int _N> struct awidth {
  static constexpr int value = _N;
  static constexpr int defaultValue = 64;
};

template <int _N> struct latency {
  static constexpr int value = _N;
  static constexpr int defaultValue = 1;
};

template <int _N> struct readwrite_mode {
  // Should be enum readwrite_t but we don't know how to make GetValue generic
  static constexpr enum readwrite_t value = (readwrite_t)_N;
  static constexpr enum readwrite_t defaultValue = readwrite;
};

template <int _N> struct maxburst {
  static constexpr int value = _N;
  static constexpr int defaultValue = 1;
};

template <int _N> struct align {
  static constexpr int value = _N;
  static constexpr int defaultValue = -1;
};

template <int _N> struct aspace {
  static constexpr int value = _N;
  static constexpr int defaultValue = 1;
};

template <int _N> struct waitrequest {
  static constexpr int value = _N;
  static constexpr int defaultValue = false;
};

template <typename _DT, int _dwidth, int _awidth, int _aspace, int _latency,
          int _maxburst, int _align, int _readwrite_mode, int _waitrequest>
class mm_host_checker {
  static_assert(_aspace > 0, "mm_host 'aspace' parameter must be at least 1");
};

template <typename _DT, int _dwidth, int _awidth, int _aspace, int _latency,
          int _maxburst, int _align, int _readwrite_mode, int _waitrequest>
class mm_master_checker {
  static_assert(_aspace > 0, "mm_master 'aspace' parameter must be at least 1");
};

template <typename _DT, class... _Params>
class mm_host final
#ifdef HLS_X86
    : public internal::memory_base
#endif // HLS_X86

{
public:
#ifdef HLS_X86
  template <typename _T>
  explicit mm_host(_T *data, std::size_t size = 0, bool use_socket = false)
      : internal::memory_base(_aspace, _awidth, _dwidth, _latency,
                              _readwrite_mode, true, _maxburst, _align,
                              _waitrequest, data, size, sizeof(_DT),
                              use_socket) {
    mSize = size;
    mUse_socket = use_socket;
    if (size > 0 && size % sizeof(_DT) != 0) {
      __ihc_hls_runtime_error_x86(
          "The buffer size must be a multiple of the type size");
    }
  }

#else
  template <typename _T>
  explicit mm_host(_T *data, std::size_t size = 0, bool use_socket = false);

#endif // HLS_X86

  // The copy constructor and assignment operator are needed in the testbench
  // but illegal in a component
  mm_host(const mm_host &other);
  mm_host &operator=(const mm_host &other);

  // Clean up any derived mm_hosts when this object is destroyed.
  ~mm_host();

  //////////////////////////////////////////////////////////////////////////////
  // The following operators apply to the mm_host object and are only
  // supported in the testbench:
  //   mm_host()
  //   getInterfaceAtIndex()
  //////////////////////////////////////////////////////////////////////////////
  // The following operators apply to the base pointer and should only be used
  // in the component:
  //   operator[]()
  //   operator*()
  //   operator->()
  //   operator _T()
  //   operator+()
  //   operator&()
  //   operator|()
  //   operator^()
  //////////////////////////////////////////////////////////////////////////////

  _DT &operator[](int index);
  _DT &operator*();
  _DT *operator->();
  template <typename _T> operator _T();
  _DT *operator+(int index);
  template <typename _T> _DT *operator&(_T value);
  template <typename _T> _DT *operator|(_T value);
  template <typename _T> _DT *operator^(_T value);

  // This function is only supported in the testbench:
  mm_host<_DT, _Params...> &getInterfaceAtIndex(int index);

#ifdef HLS_X86
private:
  std::vector<internal::memory_base *> new_hosts;
#else // Fpga

#endif // HLS_X86

private:
  static constexpr int _dwidth = ihc::GetValue<ihc::dwidth, _Params...>::value;
  static constexpr int _awidth = ihc::GetValue<ihc::awidth, _Params...>::value;
  static constexpr int _aspace = ihc::GetValue<ihc::aspace, _Params...>::value;
  static constexpr int _latency = ihc::GetValue<ihc::latency, _Params...>::value;
  static constexpr int _maxburst = ihc::GetValue<ihc::maxburst, _Params...>::value;
  static constexpr int _align = (ihc::GetValue<ihc::align, _Params...>::value == -1)
                                    ? alignof(_DT)
                                    : ihc::GetValue<ihc::align, _Params...>::value;
  static constexpr int _readwrite_mode =
      ihc::GetValue<ihc::readwrite_mode, _Params...>::value;
  static constexpr bool _waitrequest =
      ihc::GetValue<ihc::waitrequest, _Params...>::value;
  static constexpr mm_host_checker<_DT, _dwidth, _awidth, _aspace, _latency,
                                     _maxburst, _align, _readwrite_mode,
                                     _waitrequest>
      checker{};
  _DT __hls_mm_host_aspace(_aspace) * mPtr;
  int mSize;
  bool mUse_socket;
};

template <typename _DT, class... _Params>
class mm_master final
#ifdef HLS_X86
    : public internal::memory_base
#endif // HLS_X86

{
public:
#ifdef HLS_X86
  template <typename _T>
  explicit mm_master(_T *data, std::size_t size = 0, bool use_socket = false)
      : internal::memory_base(_aspace, _awidth, _dwidth, _latency,
                              _readwrite_mode, true, _maxburst, _align,
                              _waitrequest, data, size, sizeof(_DT),
                              use_socket) {
    mSize = size;
    mUse_socket = use_socket;
    if (size > 0 && size % sizeof(_DT) != 0) {
      __ihc_hls_runtime_error_x86(
          "The buffer size must be a multiple of the type size");
    }
  }

#else
  template <typename _T>
  [[deprecated("Use mm_host instead.")]]
  explicit mm_master(_T *data, std::size_t size = 0, bool use_socket = false);
#endif // HLS_X86

  // The copy constructor and assignment operator are needed in the testbench
  // but illegal in a component
  mm_master(const mm_master &other);
  mm_master &operator=(const mm_master &other);

  // Clean up any derived mm_masters when this object is destroyed.
  ~mm_master();

  //////////////////////////////////////////////////////////////////////////////
  // The following operators apply to the mm_master object and are only
  // supported in the testbench:
  //   mm_master()
  //   getInterfaceAtIndex()
  //////////////////////////////////////////////////////////////////////////////
  // The following operators apply to the base pointer and should only be used
  // in the component:
  //   operator[]()
  //   operator*()
  //   operator->()
  //   operator _T()
  //   operator+()
  //   operator&()
  //   operator|()
  //   operator^()
  //////////////////////////////////////////////////////////////////////////////

  _DT &operator[](int index);
  _DT &operator*();
  _DT *operator->();
  template <typename _T> operator _T();
  _DT *operator+(int index);
  template <typename _T> _DT *operator&(_T value);
  template <typename _T> _DT *operator|(_T value);
  template <typename _T> _DT *operator^(_T value);

  // This function is only supported in the testbench:
  mm_master<_DT, _Params...> &getInterfaceAtIndex(int index);

#ifdef HLS_X86
private:
  std::vector<internal::memory_base *> new_masters;
#else // Fpga

#endif // HLS_X86

private:
  static constexpr int _dwidth = ihc::GetValue<ihc::dwidth, _Params...>::value;
  static constexpr int _awidth = ihc::GetValue<ihc::awidth, _Params...>::value;
  static constexpr int _aspace = ihc::GetValue<ihc::aspace, _Params...>::value;
  static constexpr int _latency = ihc::GetValue<ihc::latency, _Params...>::value;
  static constexpr int _maxburst = ihc::GetValue<ihc::maxburst, _Params...>::value;
  static constexpr int _align = (ihc::GetValue<ihc::align, _Params...>::value == -1)
                                    ? alignof(_DT)
                                    : ihc::GetValue<ihc::align, _Params...>::value;
  static constexpr int _readwrite_mode =
      ihc::GetValue<ihc::readwrite_mode, _Params...>::value;
  static constexpr bool _waitrequest =
      ihc::GetValue<ihc::waitrequest, _Params...>::value;
  static constexpr mm_master_checker<_DT, _dwidth, _awidth, _aspace, _latency,
                                     _maxburst, _align, _readwrite_mode,
                                     _waitrequest>
      checker{};
  _DT __hls_mm_master_aspace(_aspace) * mPtr;
  int mSize;
  bool mUse_socket;
};

////////////////////////////////////////////////////////////////////////////////
// Implementations, no declarations below
////////////////////////////////////////////////////////////////////////////////
#ifdef HLS_X86
//////////////////
/// mm_host  ///
//////////////////

// The copy constructor and assignment operator are needed in the testbench
// necessary to ensure but illegal in a component
template <typename _DT, class... _Params>
mm_host<_DT, _Params...>::mm_host(const mm_host &other)
    : internal::memory_base(_aspace, _awidth, _dwidth, _latency,
                            static_cast<readwrite_t>(_readwrite_mode), true,
                            _maxburst, _align, _waitrequest, other.get_base(),
                            other.get_size(), sizeof(_DT),
                            other.uses_socket()) {
  mPtr = other.mPtr;
  mSize = other.mSize;
  mUse_socket = other.mUse_socket;
  mem = other.mem;
}

template <typename _DT, class... _Params>
mm_master<_DT, _Params...>::mm_master(const mm_master &other)
    : internal::memory_base(_aspace, _awidth, _dwidth, _latency,
                            static_cast<readwrite_t>(_readwrite_mode), true,
                            _maxburst, _align, _waitrequest, other.get_base(),
                            other.get_size(), sizeof(_DT),
                            other.uses_socket()) {
  mPtr = other.mPtr;
  mSize = other.mSize;
  mUse_socket = other.mUse_socket;
  mem = other.mem;
}

template <typename _DT, class... _Params>
mm_host<_DT, _Params...> &
mm_host<_DT, _Params...>::operator=(const mm_host &other) {
  mPtr = other.mPtr;
  mSize = other.mSize;
  mUse_socket = other.m_Use_socket;
  mem = other.mem;
}

template <typename _DT, class... _Params>
mm_master<_DT, _Params...> &
mm_master<_DT, _Params...>::operator=(const mm_master &other) {
  mPtr = other.mPtr;
  mSize = other.mSize;
  mUse_socket = other.m_Use_socket;
  mem = other.mem;
}

// Clean up any derived mm_hosts when this object is destroyed.
template <typename _DT, class... _Params>
mm_host<_DT, _Params...>::~mm_host() {
  for (std::vector<internal::memory_base *>::iterator it = new_hosts.begin(),
                                                      ie = new_hosts.end();
       it != ie; it++) {
    delete *it;
  }
  new_hosts.clear();
}

// Clean up any derived mm_masters when this object is destroyed.
template <typename _DT, class... _Params>
mm_master<_DT, _Params...>::~mm_master() {
  for (std::vector<internal::memory_base *>::iterator it = new_masters.begin(),
                                                      ie = new_masters.end();
       it != ie; it++) {
    delete *it;
  }
  new_masters.clear();
}

template <typename _DT, class... _Params>
_DT &mm_host<_DT, _Params...>::operator[](int index) {
  assert(size == 0 || index * data_size < size);
  return ((_DT *)mem)[index];
}

template <typename _DT, class... _Params>
_DT &mm_master<_DT, _Params...>::operator[](int index) {
  assert(size == 0 || index * data_size < size);
  return ((_DT *)mem)[index];
}

template <typename _DT, class... _Params>
_DT &mm_host<_DT, _Params...>::operator*() {
  return ((_DT *)mem)[0];
}

template <typename _DT, class... _Params>
_DT &mm_master<_DT, _Params...>::operator*() {
  return ((_DT *)mem)[0];
}

template <typename _DT, class... _Params>
_DT *mm_host<_DT, _Params...>::operator->() {
  return (_DT *)mem;
}

template <typename _DT, class... _Params>
_DT *mm_master<_DT, _Params...>::operator->() {
  return (_DT *)mem;
}

template <typename _DT, class... _Params>
template <typename _T>
mm_host<_DT, _Params...>::operator _T() {
  return (_T)((unsigned long long)mem);
}

template <typename _DT, class... _Params>
template <typename _T>
mm_master<_DT, _Params...>::operator _T() {
  return (_T)((unsigned long long)mem);
}

template <typename _DT, class... _Params>
_DT *mm_host<_DT, _Params...>::operator+(int index) {
  assert(size == 0 || index * data_size < size);
  return &((_DT *)mem)[index];
}

template <typename _DT, class... _Params>
_DT *mm_master<_DT, _Params...>::operator+(int index) {
  assert(size == 0 || index * data_size < size);
  return &((_DT *)mem)[index];
}

// Bitwise operators
template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_host<_DT, _Params...>::operator&(_T value) {
  return (_DT *)((unsigned long long)mem & (unsigned long long)value);
}

// Bitwise operators
template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_master<_DT, _Params...>::operator&(_T value) {
  return (_DT *)((unsigned long long)mem & (unsigned long long)value);
}

template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_host<_DT, _Params...>::operator|(_T value) {
  return (_DT *)((unsigned long long)mem | (unsigned long long)value);
}

template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_master<_DT, _Params...>::operator|(_T value) {
  return (_DT *)((unsigned long long)mem | (unsigned long long)value);
}

template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_host<_DT, _Params...>::operator^(_T value) {
  return (_DT *)((unsigned long long)mem ^ (unsigned long long)value);
}

template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_master<_DT, _Params...>::operator^(_T value) {
  return (_DT *)((unsigned long long)mem ^ (unsigned long long)value);
}

// Function for creating new mm_host at an offset
template <typename _DT, class... _Params>
mm_host<_DT, _Params...> &
mm_host<_DT, _Params...>::getInterfaceAtIndex(int index) {
  assert(mSize == 0 || index * data_size < mSize);
  // This new object is cleaned up when this' destructor is called.
  mm_host<_DT, _Params...> *temp = new mm_host(
      &(((_DT *)mem)[index]), mSize - index * sizeof(_DT), mUse_socket);
  new_hosts.push_back(temp);
  return *temp;
}

// Function for creating new mm_master at an offset
template <typename _DT, class... _Params>
mm_master<_DT, _Params...> &
mm_master<_DT, _Params...>::getInterfaceAtIndex(int index) {
  assert(mSize == 0 || index * data_size < mSize);
  // This new object is cleaned up when this' destructor is called.
  mm_master<_DT, _Params...> *temp = new mm_master(
      &(((_DT *)mem)[index]), mSize - index * sizeof(_DT), mUse_socket);
  new_masters.push_back(temp);
  return *temp;
}

#else // fpga path. Ignore the class just return a consistent pointer/reference

//////////////////
/// mm_host  ///
//////////////////

template <typename _DT, class... _Params>
_DT &mm_host<_DT, _Params...>::operator[](int index) {
  return *(_DT *)__builtin_intel_hls_mm_host_load(
      mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency, _maxburst,
      _align, _readwrite_mode, _waitrequest, index);
}

template <typename _DT, class... _Params>
_DT &mm_master<_DT, _Params...>::operator[](int index) {
  return *(_DT *)__builtin_intel_hls_mm_host_load(
      mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency, _maxburst,
      _align, _readwrite_mode, _waitrequest, index);
}

template <typename _DT, class... _Params>
_DT &mm_host<_DT, _Params...>::operator*() {
  return *(_DT *)__builtin_intel_hls_mm_host_load(
      mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency, _maxburst,
      _align, _readwrite_mode, _waitrequest, (int)0);
}

template <typename _DT, class... _Params>
_DT &mm_master<_DT, _Params...>::operator*() {
  return *(_DT *)__builtin_intel_hls_mm_host_load(
      mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency, _maxburst,
      _align, _readwrite_mode, _waitrequest, (int)0);
}

template <typename _DT, class... _Params>
_DT *mm_host<_DT, _Params...>::operator->() {
  return (_DT *)__builtin_intel_hls_mm_host_load(
      mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency, _maxburst,
      _align, _readwrite_mode, _waitrequest, (int)0);
}

template <typename _DT, class... _Params>
_DT *mm_master<_DT, _Params...>::operator->() {
  return (_DT *)__builtin_intel_hls_mm_host_load(
      mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency, _maxburst,
      _align, _readwrite_mode, _waitrequest, (int)0);
}

template <typename _DT, class... _Params>
_DT *mm_host<_DT, _Params...>::operator+(int index) {
  return (_DT *)__builtin_intel_hls_mm_host_load(
             mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency,
             _maxburst, _align, _readwrite_mode, _waitrequest, (int)0) +
         index;
}

template <typename _DT, class... _Params>
_DT *mm_master<_DT, _Params...>::operator+(int index) {
  return (_DT *)__builtin_intel_hls_mm_host_load(
             mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency,
             _maxburst, _align, _readwrite_mode, _waitrequest, (int)0) +
         index;
}

template <typename _DT, class... _Params>
template <typename _T>
mm_host<_DT, _Params...>::operator _T() {
  return (_T)((unsigned long long)__builtin_intel_hls_mm_host_load(
      mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency, _maxburst,
      _align, _readwrite_mode, _waitrequest, (int)0));
}

template <typename _DT, class... _Params>
template <typename _T>
mm_master<_DT, _Params...>::operator _T() {
  return (_T)((unsigned long long)__builtin_intel_hls_mm_host_load(
      mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency, _maxburst,
      _align, _readwrite_mode, _waitrequest, (int)0));
}

// Bitwise operators
template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_host<_DT, _Params...>::operator&(_T value) {
  return (
      _DT *)(((unsigned long long)__builtin_intel_hls_mm_host_load(
                 mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency,
                 _maxburst, _align, _readwrite_mode, _waitrequest, (int)0)) &
             (unsigned long long)value);
}

// Bitwise operators
template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_master<_DT, _Params...>::operator&(_T value) {
  return (
      _DT *)(((unsigned long long)__builtin_intel_hls_mm_host_load(
                 mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency,
                 _maxburst, _align, _readwrite_mode, _waitrequest, (int)0)) &
             (unsigned long long)value);
}

template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_host<_DT, _Params...>::operator|(_T value) {
  return (
      _DT *)(((unsigned long long)__builtin_intel_hls_mm_host_load(
                 mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency,
                 _maxburst, _align, _readwrite_mode, _waitrequest, (int)0)) |
             (unsigned long long)value);
}

template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_master<_DT, _Params...>::operator|(_T value) {
  return (
      _DT *)(((unsigned long long)__builtin_intel_hls_mm_host_load(
                 mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency,
                 _maxburst, _align, _readwrite_mode, _waitrequest, (int)0)) |
             (unsigned long long)value);
}

template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_host<_DT, _Params...>::operator^(_T value) {
  return (
      _DT *)(((unsigned long long)__builtin_intel_hls_mm_host_load(
                 mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency,
                 _maxburst, _align, _readwrite_mode, _waitrequest, (int)0)) ^
             (unsigned long long)value);
}

template <typename _DT, class... _Params>
template <typename _T>
_DT *mm_master<_DT, _Params...>::operator^(_T value) {
  return (
      _DT *)(((unsigned long long)__builtin_intel_hls_mm_host_load(
                 mPtr, mSize, mUse_socket, _dwidth, _awidth, _aspace, _latency,
                 _maxburst, _align, _readwrite_mode, _waitrequest, (int)0)) ^
             (unsigned long long)value);
}

#endif // HLS_X86
} // namespace ihc

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif //  __IHC_MEMORY_H__

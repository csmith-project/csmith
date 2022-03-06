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

#ifndef __IHC_STREAM_H__
#define __IHC_STREAM_H__

#include "HLS/internal/_hls.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(                                                               \
    disable : 4265) // has virtual functions, but destructor is not virtual
#pragma warning(disable : 4505) // unreferenced local function has been removed
#endif

namespace ihc {
/////////////////////////////
/// streaming interfaces  ///
/////////////////////////////

template <int _N> struct buffer {
  static constexpr int value = _N;
  static constexpr int defaultValue = 0;
};

template <int _N> struct readyLatency {
  static constexpr int value = _N;
  static constexpr int defaultValue = 0;
};

template <int _N> struct bitsPerSymbol {
  static constexpr int value = _N;
  static constexpr int defaultValue = 0;
};

template <int _N> struct usesPackets {
  static constexpr bool value = _N;
  static constexpr bool defaultValue = false;
};

template <int _N> struct usesValid {
  static constexpr int value = _N;
  static constexpr int defaultValue = true;
};

template <int _N> struct usesReady {
  static constexpr int value = _N;
  static constexpr int defaultValue = true;
};

template <int _N> struct usesEmpty {
  static constexpr int value = _N;
  static constexpr int defaultValue = false;
};

template <int _N> struct firstSymbolInHighOrderBits {
  static constexpr int value = _N;
  static constexpr int defaultValue = false;
};

/////////////////////////
/// Assert and Checks ///
/////////////////////////
template <typename _T, int _buffer, int _readyLatency, int _bitsPerSymbol,
          int _firstSymbolInHighOrderBits, int _usesPackets, int _usesEmpty,
          int _usesValid, int _usesReady>
class stream_checker {
private:
  static_assert((_bitsPerSymbol > 0 && _bitsPerSymbol < 8 * sizeof(_T)) ||
                    !_usesEmpty,
                "Stream parameter bitsPerSymbol must be set to a value "
                "strictly grater than 0 and strictly smaller than the width of "
                "the stream data bus when usesEmpty is set to true");

public:
  void checkUsesPackets() const {
    static_assert(_usesPackets,
                  "Using startofpacket and endofpacket requires a stream "
                  "with the parameterization: usesPackets<true>");
  }

  void checkUsesEmpty() const {
    static_assert(_usesEmpty,
                  "Empty based stream operations require a stream with the "
                  "parameterization: usesEmpty<true>");
  }

  void checkIfUsesEmptyThenUsesPackets() const {
    static_assert(_usesPackets || !_usesEmpty,
                  "Empty based stream operations require a stream with the "
                  "parametrizations: "
                  "usesPackets<true>, usesEmpty<true>");
  }
};

/////////////////////////
/// Class Definitions ///
/////////////////////////
template <typename _T, class... _Params>
class stream_in final : public internal::stream<_T, _Params...> {
public:
  stream_in();
  stream_in(const stream_in &) = delete;
  stream_in(const stream_in &&) = delete;
  stream_in &operator=(const stream_in &) = delete;
  stream_in &operator=(const stream_in &&) = delete;
  _T read(bool wait = false);
  void write(const _T &arg);
  _T tryRead(bool &success);
  bool tryWrite(const _T &arg);

  // for packet based stream
  _T read(bool &sop, bool &eop, bool wait = false);
  _T read(bool &sop, bool &eop, int &empty, bool wait = false);
  void write(const _T &arg, bool sop, bool eop);
  void write(const _T &arg, bool sop, bool eop, int empty);
  _T tryRead(bool &success, bool &sop, bool &eop);
  _T tryRead(bool &success, bool &sop, bool &eop, int &empty);
  bool tryWrite(const _T &arg, bool sop, bool eop);
  bool tryWrite(const _T &arg, bool sop, bool eop, int empty);
  void setStallCycles(unsigned average_stall, unsigned stall_delta = 0);
  void setValidCycles(unsigned average_valid, unsigned valid_delta = 0);

private:
  static constexpr int _buffer = ihc::GetValue<ihc::buffer, _Params...>::value;
  static constexpr int _readyLatency =
      ihc::GetValue<ihc::readyLatency, _Params...>::value;
  static constexpr int _bitsPerSymbol =
      ihc::GetValue<ihc::bitsPerSymbol, _Params...>::value;
  static constexpr bool _firstSymbolInHighOrderBits =
      ihc::GetValue<ihc::firstSymbolInHighOrderBits, _Params...>::value;
  static constexpr bool _usesPackets =
      ihc::GetValue<ihc::usesPackets, _Params...>::value;
  static constexpr bool _usesEmpty =
      ihc::GetValue<ihc::usesEmpty, _Params...>::value;
  static constexpr bool _usesValid =
      ihc::GetValue<ihc::usesValid, _Params...>::value;
  static constexpr bool _usesReady =
      ihc::GetValue<ihc::usesReady, _Params...>::value;
  static constexpr stream_checker<_T, _buffer, _readyLatency, _bitsPerSymbol,
                                  _firstSymbolInHighOrderBits, _usesPackets,
                                  _usesEmpty, _usesValid, _usesReady>
      checker{};
};

template <typename _T, class... _Params>
class stream_out final : public internal::stream<_T, _Params...> {

public:
  stream_out();
  stream_out(const stream_out &) = delete;
  stream_out(const stream_out &&) = delete;
  stream_out &operator=(const stream_out &) = delete;
  stream_out &operator=(const stream_out &&) = delete;
  _T read(bool wait = false);
  void write(const _T &arg);
  _T tryRead(bool &success);
  bool tryWrite(const _T &arg);

  // for packet based stream
  _T read(bool &sop, bool &eop, bool wait = false);
  _T read(bool &sop, bool &eop, int &empty, bool wait = false);
  void write(const _T &arg, bool sop, bool eop);
  void write(const _T &arg, bool sop, bool eop, int empty);
  _T tryRead(bool &success, bool &sop, bool &eop);
  _T tryRead(bool &success, bool &sop, bool &eop, int &empty);
  bool tryWrite(const _T &arg, bool sop, bool eop);
  bool tryWrite(const _T &arg, bool sop, bool eop, int empty);
  void setStallCycles(unsigned average_stall, unsigned stall_delta = 0);
  void setReadyCycles(unsigned average_ready, unsigned ready_delta = 0);

private:
  static constexpr int _buffer = ihc::GetValue<ihc::buffer, _Params...>::value;
  static constexpr int _readyLatency =
      ihc::GetValue<ihc::readyLatency, _Params...>::value;
  static constexpr int _bitsPerSymbol =
      ihc::GetValue<ihc::bitsPerSymbol, _Params...>::value;
  static constexpr bool _firstSymbolInHighOrderBits =
      ihc::GetValue<ihc::firstSymbolInHighOrderBits, _Params...>::value;
  static constexpr bool _usesPackets =
      ihc::GetValue<ihc::usesPackets, _Params...>::value;
  static constexpr bool _usesEmpty =
      ihc::GetValue<ihc::usesEmpty, _Params...>::value;
  static constexpr bool _usesValid =
      ihc::GetValue<ihc::usesValid, _Params...>::value;
  static constexpr bool _usesReady =
      ihc::GetValue<ihc::usesReady, _Params...>::value;
  static constexpr stream_checker<_T, _buffer, _readyLatency, _bitsPerSymbol,
                                  _firstSymbolInHighOrderBits, _usesPackets,
                                  _usesEmpty, _usesValid, _usesReady>
      checker{};
};

// Bi-directional inter-task stream
template <typename _T, class... _Params>
class stream final : public internal::stream<_T, _Params...> {
public:
  stream();
  stream(const stream &) = delete;
  stream(const stream &&) = delete;
  stream &operator=(const stream &) = delete;
  stream &operator=(const stream &&) = delete;
  _T read(bool wait = true);
  void write(const _T &arg);
  _T tryRead(bool &success);
  bool tryWrite(const _T &arg);

  // for packet based stream
  _T read(bool &sop, bool &eop, bool wait = true);
  _T read(bool &sop, bool &eop, int &empty, bool wait = true);
  void write(const _T &arg, bool sop, bool eop);
  void write(const _T &arg, bool sop, bool eop, int empty);
  _T tryRead(bool &success, bool &sop, bool &eop);
  _T tryRead(bool &success, bool &sop, bool &eop, int &empty);
  bool tryWrite(const _T &arg, bool sop, bool eop);
  bool tryWrite(const _T &arg, bool sop, bool eop, int empty);

private:
  static constexpr int _buffer = ihc::GetValue<ihc::buffer, _Params...>::value;
  static constexpr int _readyLatency =
      ihc::GetValue<ihc::readyLatency, _Params...>::value;
  static constexpr int _bitsPerSymbol =
      ihc::GetValue<ihc::bitsPerSymbol, _Params...>::value;
  static constexpr bool _firstSymbolInHighOrderBits =
      ihc::GetValue<ihc::firstSymbolInHighOrderBits, _Params...>::value;
  static constexpr bool _usesPackets =
      ihc::GetValue<ihc::usesPackets, _Params...>::value;
  static constexpr bool _usesEmpty =
      ihc::GetValue<ihc::usesEmpty, _Params...>::value;
  static constexpr bool _usesValid =
      ihc::GetValue<ihc::usesValid, _Params...>::value;
  static constexpr bool _usesReady =
      ihc::GetValue<ihc::usesReady, _Params...>::value;
  static_assert(_usesValid,
                "Bi-directional stream interfaces must use Valid signal");
  static_assert(_usesReady,
                "Bi-directional stream interfaces must use Ready signal");
  static constexpr stream_checker<_T, _buffer, _readyLatency, _bitsPerSymbol,
                                  _firstSymbolInHighOrderBits, _usesPackets,
                                  _usesEmpty, _usesValid, _usesReady>
      checker{};
};

////////////////////////////////////////////////////////////////////////////////
// Implementations, no declarations below
////////////////////////////////////////////////////////////////////////////////
#ifdef HLS_X86

///////////////////
/// stream_in   ///
///////////////////

template <typename _T, class... _Params>
stream_in<_T, _Params...>::stream_in() {}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::tryRead(bool &success) {
  return internal::stream<_T, _Params...>::tryRead(success);
}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::read(bool wait /*=false*/) {
  _T elem = internal::stream<_T, _Params...>::read(wait);
  return elem;
}

template <typename _T, class... _Params>
bool stream_in<_T, _Params...>::tryWrite(const _T &arg) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg);
  }
  return success;
}

template <typename _T, class... _Params>
void stream_in<_T, _Params...>::write(const _T &arg) {
  internal::stream<_T, _Params...>::write(arg);
}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop) {
  return internal::stream<_T, _Params...>::tryRead(success, sop, eop);
}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop,
                                      int &empty) {
  return internal::stream<_T, _Params...>::tryRead(success, sop, eop, empty);
}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::read(bool &sop, bool &eop, bool wait /*=false*/) {
  _T elem = internal::stream<_T, _Params...>::read(sop, eop, wait);
  return elem;
}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::read(bool &sop, bool &eop, int &empty,
                                   bool wait /*=false*/) {
  _T elem = internal::stream<_T, _Params...>::read(sop, eop, empty, wait);
  return elem;
}

template <typename _T, class... _Params>
bool stream_in<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg, sop, eop);
  }
  return success;
}

template <typename _T, class... _Params>
bool stream_in<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop,
                                         int empty) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg, sop, eop, empty);
  }
  return success;
}

template <typename _T, class... _Params>
void stream_in<_T, _Params...>::write(const _T &arg, bool sop, bool eop) {
  internal::stream<_T, _Params...>::write(arg, sop, eop);
}

template <typename _T, class... _Params>
void stream_in<_T, _Params...>::write(const _T &arg, bool sop, bool eop,
                                      int empty) {
  internal::stream<_T, _Params...>::write(arg, sop, eop, empty);
}

template <typename _T, class... _Params>
void stream_in<_T, _Params...>::setStallCycles(unsigned average_stall,
                                               unsigned stall_delta) {
  if (stall_delta > average_stall) {
    __ihc_hls_runtime_error_x86("The stall delta in setStallCycles cannot be "
                                "larger than the average stall value");
  }
  internal::stream<_T, _Params...>::setStallCycles(average_stall, stall_delta);
}

template <typename _T, class... _Params>
void stream_in<_T, _Params...>::setValidCycles(unsigned average_valid,
                                               unsigned valid_delta) {
  if (average_valid == 0) {
    __ihc_hls_runtime_error_x86(
        "The valid average in setValidCycles must be at least 1");
  }
  if (valid_delta > average_valid) {
    __ihc_hls_runtime_error_x86("The valid delta in setValidCycles cannot be "
                                "larger than the average valid value");
  }
  internal::stream<_T, _Params...>::setReadyorValidCycles(average_valid,
                                                          valid_delta);
}

///////////////////
/// stream_out  ///
///////////////////

template <typename _T, class... _Params>
stream_out<_T, _Params...>::stream_out() {}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::tryRead(bool &success) {
  return internal::stream<_T, _Params...>::tryRead(success);
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::read(bool wait /*=false*/) {
  _T elem = internal::stream<_T, _Params...>::read(wait);
  return elem;
}

template <typename _T, class... _Params>
void stream_out<_T, _Params...>::write(const _T &arg) {
  internal::stream<_T, _Params...>::write(arg);
}

template <typename _T, class... _Params>
bool stream_out<_T, _Params...>::tryWrite(const _T &arg) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg);
  }
  return success;
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop) {
  return internal::stream<_T, _Params...>::tryRead(success, sop, eop);
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop,
                                       int &empty) {
  return internal::stream<_T, _Params...>::tryRead(success, sop, eop, empty);
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::read(bool &sop, bool &eop,
                                    bool wait /*=false*/) {
  _T elem = internal::stream<_T, _Params...>::read(sop, eop, wait);
  return elem;
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::read(bool &sop, bool &eop, int &empty,
                                    bool wait /*=false*/) {
  _T elem = internal::stream<_T, _Params...>::read(sop, eop, empty, wait);
  return elem;
}

template <typename _T, class... _Params>
void stream_out<_T, _Params...>::write(const _T &arg, bool sop, bool eop) {
  internal::stream<_T, _Params...>::write(arg, sop, eop);
}

template <typename _T, class... _Params>
void stream_out<_T, _Params...>::write(const _T &arg, bool sop, bool eop,
                                       int empty) {
  internal::stream<_T, _Params...>::write(arg, sop, eop, empty);
}

template <typename _T, class... _Params>
bool stream_out<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg, sop, eop);
  }
  return success;
}

template <typename _T, class... _Params>
bool stream_out<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop,
                                          int empty) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg, sop, eop, empty);
  }
  return success;
}

template <typename _T, class... _Params>
void stream_out<_T, _Params...>::setStallCycles(unsigned average_stall,
                                                unsigned stall_delta) {
  if (stall_delta > average_stall) {
    __ihc_hls_runtime_error_x86("The stall delta in setStallCycles cannot be "
                                "larger than the average stall value");
  }
  internal::stream<_T, _Params...>::setStallCycles(average_stall, stall_delta);
}

template <typename _T, class... _Params>
void stream_out<_T, _Params...>::setReadyCycles(unsigned average_ready,
                                                unsigned ready_delta) {
  if (average_ready == 0) {
    __ihc_hls_runtime_error_x86(
        "The ready average in setReadCycles must be at least 1");
  }
  if (ready_delta > average_ready) {
    __ihc_hls_runtime_error_x86("The ready delta in setReadyCycles cannot be "
                                "larger than the average ready value");
  }
  internal::stream<_T, _Params...>::setReadyorValidCycles(average_ready,
                                                          ready_delta);
}

///////////////////
///// stream  /////
///////////////////

template <typename _T, class... _Params> stream<_T, _Params...>::stream() {}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::tryRead(bool &success) {
  return internal::stream<_T, _Params...>::tryRead(success);
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::read(bool wait /*=true*/) {
  _T elem = internal::stream<_T, _Params...>::read(wait);
  return elem;
}

template <typename _T, class... _Params>
void stream<_T, _Params...>::write(const _T &arg) {
  internal::stream<_T, _Params...>::write(arg);
}

template <typename _T, class... _Params>
bool stream<_T, _Params...>::tryWrite(const _T &arg) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg);
  }
  return success;
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop) {
  return internal::stream<_T, _Params...>::tryRead(success, sop, eop);
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop,
                                   int &empty) {
  return internal::stream<_T, _Params...>::tryRead(success, sop, eop, empty);
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::read(bool &sop, bool &eop, bool wait /*=true*/) {
  _T elem = internal::stream<_T, _Params...>::read(sop, eop, wait);
  return elem;
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::read(bool &sop, bool &eop, int &empty,
                                bool wait /*=true*/) {
  _T elem = internal::stream<_T, _Params...>::read(sop, eop, empty, wait);
  return elem;
}

template <typename _T, class... _Params>
void stream<_T, _Params...>::write(const _T &arg, bool sop, bool eop) {
  internal::stream<_T, _Params...>::write(arg, sop, eop);
}

template <typename _T, class... _Params>
void stream<_T, _Params...>::write(const _T &arg, bool sop, bool eop,
                                   int empty) {
  internal::stream<_T, _Params...>::write(arg, sop, eop, empty);
}

template <typename _T, class... _Params>
bool stream<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg, sop, eop);
  }
  return success;
}

template <typename _T, class... _Params>
bool stream<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop,
                                      int empty) {
  bool success = true; /* stl::queue has no full */
  if (success) {
    write(arg, sop, eop, empty);
  }
  return success;
}

#else // fpga path. Ignore the class just return a consistent pointer/reference

///////////////////
/// stream_in   ///
///////////////////

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::tryRead(bool &success) {
  checker.checkIfUsesEmptyThenUsesPackets();
  bool sop = false;
  bool eop = false;
  int emp = 0;
  return *__builtin_intel_hls_instream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, &sop,
      &eop, &emp, &success);
}
template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::read(bool wait) {
  (void)wait;
  checker.checkIfUsesEmptyThenUsesPackets();
  bool sop = false;
  bool eop = false;
  int emp = 0;
  return *__builtin_intel_hls_instream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, &sop,
      &eop, &emp);
}

template <typename _T, class... _Params>
void stream_in<_T, _Params...>::write(const _T &arg) {
  __builtin_intel_hls_instream_write(&arg, (__int64)this, _buffer,
                                     _readyLatency, _bitsPerSymbol,
                                     _firstSymbolInHighOrderBits, _usesPackets,
                                     _usesEmpty, _usesValid, false, false, 0);
}

template <typename _T, class... _Params>
bool stream_in<_T, _Params...>::tryWrite(const _T &arg) {
  return __builtin_intel_hls_instream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, false,
      false, 0);
}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop) {
  checker.checkUsesPackets();
  int emp = 0;
  return *__builtin_intel_hls_instream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, &sop,
      &eop, &emp, &success);
}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop,
                                      int &empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();

  return *__builtin_intel_hls_instream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, &sop,
      &eop, &empty, &success);
}

template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::read(bool &sop, bool &eop, bool wait) {
  (void)wait;
  checker.checkUsesPackets();
  int emp = 0;
  return *__builtin_intel_hls_instream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, &sop,
      &eop, &emp);
}
template <typename _T, class... _Params>
_T stream_in<_T, _Params...>::read(bool &sop, bool &eop, int &empty,
                                   bool wait) {
  (void)wait;
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  return *__builtin_intel_hls_instream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, &sop,
      &eop, &empty);
}

template <typename _T, class... _Params>
bool stream_in<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop) {
  checker.checkUsesPackets();
  return __builtin_intel_hls_instream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, sop,
      eop, 0);
}

template <typename _T, class... _Params>
bool stream_in<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop,
                                         int empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  return __builtin_intel_hls_instream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesValid, sop,
      eop, empty);
}

template <typename _T, class... _Params>
void stream_in<_T, _Params...>::write(const _T &arg, bool sop, bool eop) {
  checker.checkUsesPackets();
  __builtin_intel_hls_instream_write(&arg, (__int64)this, _buffer,
                                     _readyLatency, _bitsPerSymbol,
                                     _firstSymbolInHighOrderBits, _usesPackets,
                                     _usesEmpty, _usesValid, sop, eop, 0);
}

template <typename _T, class... _Params>
void stream_in<_T, _Params...>::write(const _T &arg, bool sop, bool eop,
                                      int empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  __builtin_intel_hls_instream_write(&arg, (__int64)this, _buffer,
                                     _readyLatency, _bitsPerSymbol,
                                     _firstSymbolInHighOrderBits, _usesPackets,
                                     _usesEmpty, _usesValid, sop, eop, empty);
}

///////////////////
/// stream_out  ///
///////////////////

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::tryRead(bool &success) {
  bool sop = false;
  bool eop = false;
  int emp = 0;
  return *__builtin_intel_hls_outstream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &emp, &success);
}
template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::read(bool wait) {
  (void)wait;
  bool sop = false;
  bool eop = false;
  int emp = 0;
  return *__builtin_intel_hls_outstream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &emp);
}
template <typename _T, class... _Params>
void stream_out<_T, _Params...>::write(const _T &arg) {
  checker.checkIfUsesEmptyThenUsesPackets();
  __builtin_intel_hls_outstream_write(&arg, (__int64)this, _buffer,
                                      _readyLatency, _bitsPerSymbol,
                                      _firstSymbolInHighOrderBits, _usesPackets,
                                      _usesEmpty, _usesReady, false, false, 0);
}

template <typename _T, class... _Params>
bool stream_out<_T, _Params...>::tryWrite(const _T &arg) {
  checker.checkIfUsesEmptyThenUsesPackets();
  return __builtin_intel_hls_outstream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, false,
      false, 0);
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop) {
  checker.checkIfUsesEmptyThenUsesPackets();
  int emp = 0;
  return *__builtin_intel_hls_outstream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &emp, &success);
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop,
                                       int &empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  return *__builtin_intel_hls_outstream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &empty, &success);
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::read(bool &sop, bool &eop, bool wait) {
  (void)wait;
  checker.checkUsesPackets();
  int emp;
  return *__builtin_intel_hls_outstream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &emp);
}

template <typename _T, class... _Params>
_T stream_out<_T, _Params...>::read(bool &sop, bool &eop, int &empty,
                                    bool wait) {
  (void)wait;
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  return *__builtin_intel_hls_outstream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &empty);
}

template <typename _T, class... _Params>
void stream_out<_T, _Params...>::write(const _T &arg, bool sop, bool eop) {
  checker.checkUsesPackets();
  __builtin_intel_hls_outstream_write(&arg, (__int64)this, _buffer,
                                      _readyLatency, _bitsPerSymbol,
                                      _firstSymbolInHighOrderBits, _usesPackets,
                                      _usesEmpty, _usesReady, sop, eop, 0);
}

template <typename _T, class... _Params>
void stream_out<_T, _Params...>::write(const _T &arg, bool sop, bool eop,
                                       int empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  __builtin_intel_hls_outstream_write(&arg, (__int64)this, _buffer,
                                      _readyLatency, _bitsPerSymbol,
                                      _firstSymbolInHighOrderBits, _usesPackets,
                                      _usesEmpty, _usesReady, sop, eop, empty);
}

template <typename _T, class... _Params>
bool stream_out<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop) {
  checker.checkUsesPackets();
  return __builtin_intel_hls_outstream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, sop,
      eop, 0);
}

template <typename _T, class... _Params>
bool stream_out<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop,
                                          int empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();

  return __builtin_intel_hls_outstream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, sop,
      eop, empty);
}

///////////////////
///// stream  /////
///////////////////

template <typename _T, class... _Params>
_T stream<_T, _Params...>::tryRead(bool &success) {
  bool sop = false;
  bool eop = false;
  int emp = 0;
  return *__builtin_intel_hls_instream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &emp, &success);
}
template <typename _T, class... _Params>
_T stream<_T, _Params...>::read(bool wait) {
  (void)wait;
  bool sop = false;
  bool eop = false;
  int emp = 0;
  return *__builtin_intel_hls_instream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &emp);
}
template <typename _T, class... _Params>
void stream<_T, _Params...>::write(const _T &arg) {
  checker.checkIfUsesEmptyThenUsesPackets();
  __builtin_intel_hls_outstream_write(&arg, (__int64)this, _buffer,
                                      _readyLatency, _bitsPerSymbol,
                                      _firstSymbolInHighOrderBits, _usesPackets,
                                      _usesEmpty, _usesReady, false, false, 0);
}

template <typename _T, class... _Params>
bool stream<_T, _Params...>::tryWrite(const _T &arg) {
  checker.checkIfUsesEmptyThenUsesPackets();
  return __builtin_intel_hls_outstream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, false,
      false, 0);
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop) {
  checker.checkUsesPackets();
  int emp = 0;
  return *__builtin_intel_hls_instream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &emp, &success);
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::tryRead(bool &success, bool &sop, bool &eop,
                                   int &empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  return *__builtin_intel_hls_instream_tryRead(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &empty, &success);
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::read(bool &sop, bool &eop, bool wait) {
  (void)wait;
  checker.checkUsesPackets();
  int emp;
  return *__builtin_intel_hls_instream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &emp);
}

template <typename _T, class... _Params>
_T stream<_T, _Params...>::read(bool &sop, bool &eop, int &empty, bool wait) {
  (void)wait;
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  return *__builtin_intel_hls_instream_read(
      (_T *)0, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, &sop,
      &eop, &empty);
}

template <typename _T, class... _Params>
void stream<_T, _Params...>::write(const _T &arg, bool sop, bool eop) {
  checker.checkUsesPackets();
  __builtin_intel_hls_outstream_write(&arg, (__int64)this, _buffer,
                                      _readyLatency, _bitsPerSymbol,
                                      _firstSymbolInHighOrderBits, _usesPackets,
                                      _usesEmpty, _usesReady, sop, eop, 0);
}

template <typename _T, class... _Params>
void stream<_T, _Params...>::write(const _T &arg, bool sop, bool eop,
                                   int empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  __builtin_intel_hls_outstream_write(&arg, (__int64)this, _buffer,
                                      _readyLatency, _bitsPerSymbol,
                                      _firstSymbolInHighOrderBits, _usesPackets,
                                      _usesEmpty, _usesReady, sop, eop, empty);
}

template <typename _T, class... _Params>
bool stream<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop) {
  checker.checkUsesPackets();
  return __builtin_intel_hls_outstream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, sop,
      eop, 0);
}

template <typename _T, class... _Params>
bool stream<_T, _Params...>::tryWrite(const _T &arg, bool sop, bool eop,
                                      int empty) {
  checker.checkUsesPackets();
  checker.checkUsesEmpty();
  return __builtin_intel_hls_outstream_tryWrite(
      &arg, (__int64)this, _buffer, _readyLatency, _bitsPerSymbol,
      _firstSymbolInHighOrderBits, _usesPackets, _usesEmpty, _usesReady, sop,
      eop, empty);
}

#endif // HLS_X86
} // namespace ihc

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // __IHC_STREAM_H__

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

/*****************************************************
 *                                                   *
 *  Internals of Basic Support for Intel's HLS Tool  *
 *  DO NOT REFERENCE DIRECTLY                        *
 *                                                   *
 *****************************************************/

#ifndef __HLS_INTERNAL_H__
#define __HLS_INTERNAL_H__

#ifdef __INTELFPGA_COMPILER__
   // Compiling for FPGA or x86 using FPGA compiler
#  undef component
#  define component __attribute__((ihc_component)) __attribute__((noinline))
#else
#  ifndef component
#    define component
#  endif
#  ifndef HLS_X86
#    define HLS_X86
#  endif
#endif

#include <type_traits>
#ifdef HLS_X86
#include <queue>
#include <string.h> //memcpy
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _HLS_EMBEDDED_PROFILE
#include "HLS/internal/emulator_threadsupport.h"
#else
#include <mutex>
#include <condition_variable>
#endif
#endif

#ifdef __INTELFPGA_COMPILER__
#define __hls_mm_master_aspace(__x) __attribute__((address_space(__x + 1024)))
#define __hls_mm_host_aspace(__x) __attribute__((address_space(__x + 1024)))
#else
#define __hls_mm_master_aspace(__x)
#define __hls_mm_host_aspace(__x)
#endif

#ifdef _HLS_EMBEDDED_PROFILE
#define _VIRTUAL_
#else
#define _VIRTUAL_ virtual
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4265) // has virtual functions, but destructor is not virtual
#pragma warning(disable:4505) // unreferenced local function has been removed
#endif

#if defined(_MSC_VER)
  #define WINDOWSWEAK __declspec(selectany)
  #define LINUXWEAK
#else
  #define WINDOWSWEAK
  #define LINUXWEAK __attribute__((weak))
#endif

WINDOWSWEAK void *__ihc_enqueue_handle LINUXWEAK;

extern "C" unsigned long long ihc_hls_get_sim_time(void);
extern "C" int __ihc_hls_get_clock_period_ps(void);
extern "C" int __ihc_hls_component_start_sim_task(const char *component_name);
extern "C" int __ihc_hls_component_wait_sim_task(const char *component_name, unsigned int wait_cycles);
extern "C" void __ihc_hls_stream_sink_wait_sim_task(void* obj_ptr);
extern "C" void __ihc_hls_component_run_all(const void* component_address);
extern "C" void __ihc_hls_set_component_wait_cycle(const void * component_address, unsigned int num_wait_cycles);

// Cosim internal support calls
extern "C" int __ihc_hls_async_call_capable(void);
// Print error message and bail
extern "C" void __ihc_hls_runtime_error(const char *msg);

#ifdef HLS_X86
inline void __ihc_hls_runtime_error_x86(const char *msg){
#ifndef _HLS_EMBEDDED_PROFILE
  printf("Error: %s\n",msg);
  assert(0);
#endif
}
#endif

enum readwrite_t {readwrite = 0, readonly = 1, writeonly = 2};

// ignore __fpga_reg in emulation flow
#ifdef HLS_X86
  #define __fpga_reg(x) (x)
#elif defined(__clang__)
#if __has_builtin(__builtin_fpga_reg)
  #ifndef __fpga_reg
    #define __fpga_reg __builtin_fpga_reg
  #endif
#endif
#endif

namespace ihc {

template <template <int> class _Type, class _T>
struct MatchType : std::is_same<_Type<_T::value>, _T> {};

template <template <int> class _Type, class... _T> struct GetValue {
  // any value is ok here, so '0' is fine for an arbitrary instantiation
  enum { value = _Type<0>::defaultValue };
  // only when _T is empty
};

template <template <int> class _Type, class _T1, class... _T>
struct GetValue<_Type, _T1, _T...> {
  enum {
    value = std::conditional<MatchType<_Type, _T1>::value, _T1,
                             GetValue<_Type, _T...>>::type::value
  };
};

namespace internal {

using namespace std;

// Interface parameter base types used to define interfaces
class param {};
class notinit:public param {};

class memory_base {
private:
#ifdef HLS_X86
  // Attributes
  int aspace;
  int awidth;
  int dwidth;
  int latency;
  readwrite_t readwrite_mode;
  bool byteenabled;
  int maxburst;
  int align;
  bool waitrequest;
  size_t sim_base;

protected:
  // Extra cosim book keeping
  size_t data_size; //size of the underlying type used by the interface
  void *mem;
  size_t size;
  bool use_socket; // if true, all reads and writes will go through the IPC socket instead of shared memory

  //description of test data buffer accessed through this interface
  void set_mem(void *x) {mem=x;};
  void set_size(size_t x) {size=x;};
  void set_data_size(size_t datasize) {data_size = datasize;}

#endif
public:
#ifdef HLS_X86
  memory_base(int aspace, int awidth, int dwidth, int latency,
              int readwrite_mode, bool byteenabled, int maxburst, int align,
              bool waitrequest, void *mem, size_t size, size_t data_size,
              bool use_socket)
      : aspace(aspace), awidth(awidth), dwidth(dwidth), latency(latency),
        readwrite_mode(static_cast<readwrite_t>(readwrite_mode)),
        byteenabled(byteenabled), maxburst(maxburst), align(align),
        waitrequest(waitrequest), sim_base(0), data_size(data_size), mem(mem), size(size),
        use_socket(use_socket)  {}
  memory_base(int aspace, int awidth, int dwidth, int latency,
              readwrite_t readwrite_mode, bool byteenabled, int maxburst,
              int align, bool waitrequest);
  // set_parameters, set_base, and get_sim_base are defined in the hls_cosim library
  void set_parameters(int aspace, int awidth, int dwidth, int latency,
                      readwrite_t readwrite_mode, bool byteenabled,
                      int maxburst, int align, bool waitrequest);
  void set_base(void *ptr, size_t datasize, size_t size, bool use_socket);
  int get_interface_aspace() const {return aspace;}
  void * get_base() const {return mem;}
  size_t get_size() const {return size;}
  int get_awidth() const {return awidth;}
  int get_dwidth() const {return dwidth;}
  int get_latency() const {return latency;}
  readwrite_t get_readwrite_mode() const {return readwrite_mode;}
  bool get_byteenabled() const {return byteenabled;}
  int get_maxburst() const {return maxburst;}
  int get_align() const {return align;}
  bool get_waitrequest() const {return waitrequest;}

  size_t get_data_size() const {return data_size;}

  void set_sim_base(size_t addr) {sim_base = addr;}
  size_t get_sim_base();

  void set_use_socket(bool socket) {use_socket = socket;}
  bool uses_socket() const {return use_socket;}
#endif
};

#ifdef HLS_X86
class stream_abstract_base {
  bool stable; // does the data on this interface change between function calls?
  bool implicit; // is this interface synchronous with the component's function call interface?
  bool global; // is this an implicit interface that carries a global variable's address?
  size_t data_size; //size of the underlying type used by the interface
protected:
  unsigned m_average_stall;
  unsigned m_stall_delta;
  unsigned m_average_RoV;
  unsigned m_RoV_delta;
  unsigned m_remaining_period;
  unsigned m_period_threshold;
public:
stream_abstract_base():stable(false), implicit(false), global(false), m_average_stall(0), m_stall_delta(0), m_average_RoV(1), m_RoV_delta(0), m_remaining_period(1), m_period_threshold(1) { assert(1==0);}
stream_abstract_base(size_t data_size):stable(false), implicit(false), global(false), data_size(data_size), m_average_stall(0), m_stall_delta(0), m_average_RoV(1), m_RoV_delta(0), m_remaining_period(1), m_period_threshold(1){}
  bool is_stable() {return stable;}
  void set_stable() {stable = true;}
  bool is_implicit() {return implicit;}
  bool is_global() {return global;}
  void set_implicit() {implicit = true;}
  void set_global() {global = true;}
  size_t get_data_size() {return data_size;}
  void set_data_size(size_t datasize) {data_size = datasize;}
  unsigned get_average_stall() {return m_average_stall;}
  unsigned get_stall_delta() {return m_stall_delta;}
  unsigned get_average_RoV() {return m_average_RoV;}
  unsigned get_RoV_delta() {return m_RoV_delta;}
#ifndef _HLS_EMBEDDED_PROFILE
  virtual bool _internal_cosim_empty() = 0;
  virtual void read_by_ptr(void *data) = 0;
  virtual void read_by_ptr_pkt(void *data, bool* sop, bool* eop) = 0;
  virtual void read_by_ptr_pkt_e(void *data, bool* sop, bool* eop, void* empty) = 0;
  virtual void write_by_ptr(void *data) = 0;
  virtual void write_by_ptr_pkt(void *data, bool* sop, bool* eop) = 0;
  virtual void write_by_ptr_pkt_e(void *data, bool* sop, bool* eop, void* empty) = 0;
  virtual void front_by_ptr(void *data) = 0;
  virtual void front_by_ptr_pkt(void *data, bool* sop, bool* eop) = 0;
  virtual void front_by_ptr_pkt_e(void *data, bool* sop, bool* eop, void* empty) = 0;
  virtual size_t get_size() = 0;
#endif
};
#endif

template<typename T, class ... Params>
class stream
#ifdef HLS_X86
  :  public stream_abstract_base
#endif
{

#ifdef HLS_X86
  std::queue<T> q_;
  std::queue<std::pair<bool,bool> > qp_;
  std::queue<int> qpe_;
  mutex m_;
  condition_variable cv_;
#endif
public:
  stream();
  // do not allow copy and assignment of streams
  stream(const stream&) = delete;
  stream& operator=(const stream&) = delete;

  bool _internal_cosim_empty();
#ifdef HLS_X86
  _VIRTUAL_ T read(bool wait=false);
  _VIRTUAL_ void write(const T& arg);
  _VIRTUAL_ T tryRead(bool &success);
  _VIRTUAL_ bool tryWrite(const T& arg);

  // for packet based stream
  _VIRTUAL_ T read(bool& sop, bool& eop, bool wait=false);
  _VIRTUAL_ T read(bool& sop, bool& eop, int& empty, bool wait=false);
  _VIRTUAL_ void write(const T& arg, bool sop, bool eop);
  _VIRTUAL_ void write(const T& arg, bool sop, bool eop, int empty);
  _VIRTUAL_ T tryRead(bool &success, bool& sop, bool& eop);
  _VIRTUAL_ T tryRead(bool &success, bool& sop, bool& eop, int& empty);
  _VIRTUAL_ bool tryWrite(const T& arg, bool sop, bool eop);
  _VIRTUAL_ bool tryWrite(const T& arg, bool sop, bool eop, int empty);
  _VIRTUAL_ void setStallCycles(unsigned average_stall, unsigned stall_delta=0);
  _VIRTUAL_ void setReadyorValidCycles(unsigned average_RoV, unsigned RoV_delta=0);
#else
  T read();
  void write(const T& arg);
  T tryRead(bool &success);
  bool tryWrite(const T& arg);

  // for packet based stream
  T read(bool& sop, bool& eop);
  T read(bool& sop, bool& eop, int& empty);
  void write(const T& arg, bool sop, bool eop);
  void write(const T& arg, bool sop, bool eop, int empty);
  T tryRead(bool &success, bool& sop, bool& eop);
  T tryRead(bool &success, bool& sop, bool& eop, int& empty);
  bool tryWrite(const T& arg, bool sop, bool eop);
  bool tryWrite(const T& arg, bool sop, bool eop, int empty);
#endif
   T _internal_cosim_front();
   T _internal_cosim_front(bool& sop, bool& eop);
   T _internal_cosim_front(bool& sop, bool& eop, int& empty);

#ifdef HLS_X86
  void read_by_ptr(void *data);
  void read_by_ptr_pkt(void *data, bool* sop, bool* eop);
  void read_by_ptr_pkt_e(void *data, bool* sop, bool* eop, void* empty);
  void write_by_ptr(void *data);
  void write_by_ptr_pkt(void *data, bool* sop, bool*eop);
  void write_by_ptr_pkt_e(void *data, bool* sop, bool*eop, void* empty);
  void front_by_ptr(void *data);
  void front_by_ptr_pkt(void *data, bool* sop, bool* eop);
  void front_by_ptr_pkt_e(void *data, bool* sop, bool* eop, void* empty);
  size_t get_size() {return q_.size();}

  // control stall/valid behaviour
  bool stall();
  void setStallPeriod();
  //Middle Square Weyl Sequence RNG
   unsigned msws();
#endif
};

template <typename T, class ... Params>
  stream<T,Params...>::stream()
#ifdef HLS_X86
 :stream_abstract_base(sizeof(T))
#endif
{
}

  ////////////////
 /// stream   ///
////////////////

#ifdef HLS_X86
template<typename T, class ... Params>
bool stream<T,Params...>::_internal_cosim_empty() {
  bool empty=q_.empty();
  return empty;
}

template<typename T, class ... Params>
T stream<T,Params...>::tryRead(bool &success) {
  unique_lock<mutex> _{ m_ };
  success = !_internal_cosim_empty();
  if (success) {
    T arg = q_.front();
    q_.pop();

    // unused sideband signals
    qp_.pop();
    qpe_.pop();
    return arg;
  } else {
    return T();
  }
}

template<typename T, class ... Params>
T stream<T,Params...>::tryRead(bool &success, bool& sop, bool& eop) {
  unique_lock<mutex> _{ m_ };
  success = !_internal_cosim_empty();
  if (success) {
    T arg = q_.front();
    q_.pop();
    std::pair<bool,bool> p = qp_.front();
    sop = p.first;
    eop = p.second;

    qp_.pop();
    qpe_.pop();

    return arg;
  } else {
    return T();
  }
}

template<typename T, class ... Params>
T stream<T,Params...>::tryRead(bool &success, bool& sop, bool& eop, int& empty) {
  unique_lock<mutex> _{ m_ };
  success = !_internal_cosim_empty();
  if (success) {
    T arg = q_.front();
    q_.pop();
    std::pair<bool,bool> p = qp_.front();
    empty = qpe_.front();
    sop = p.first;
    eop = p.second;

    qp_.pop();
    qpe_.pop();

    return arg;
  } else {
    return T();
  }
}

template<typename T, class ... Params>
T stream<T,Params...>::read(bool wait) {
  unique_lock<mutex> lock{ m_ };
  bool empty = _internal_cosim_empty();
  // in the case of emulation, we either error out or wait until wait is available
  if(empty && !wait) __ihc_hls_runtime_error_x86("Cannot do a blocking read from an empty stream on an x86 target");
  // wait until the queue is no longer empty
  cv_.wait(lock, [this](){return  !this->_internal_cosim_empty();});
  T arg = q_.front();
  q_.pop();

  // unused sideband signals
  qp_.pop();
  qpe_.pop();
  return arg;
}

template<typename T, class ... Params>
T stream<T,Params...>::read(bool& sop, bool& eop, bool wait) {
  unique_lock<mutex> lock{ m_ };
  bool empty = _internal_cosim_empty();
  // in the case of emulation, we either error out or wait until wait is available
  if(empty && !wait) __ihc_hls_runtime_error_x86("Cannot do a blocking read from an empty stream on an x86 target");
  // wait until the queue is no longer empty
  cv_.wait(lock, [this]{return  !this->_internal_cosim_empty();});
  T arg = q_.front();
  q_.pop();
  std::pair<bool,bool> p = qp_.front();
  sop = p.first;
  eop = p.second;

  qp_.pop();
  // unused sideband signal
  qpe_.pop();
  return arg;
}

template<typename T, class ... Params>
T stream<T,Params...>::read(bool& sop, bool& eop, int& empty, bool wait) {
  unique_lock<mutex> lock{ m_ };
  bool empty_ = _internal_cosim_empty();
  // in the case of emulation, we either error out or wait until wait is available
  if(empty_ && !wait) __ihc_hls_runtime_error_x86("Cannot do a blocking read from an empty stream on an x86 target");
  // wait until the queue is no longer empty
  cv_.wait(lock, [this]{return  !this->_internal_cosim_empty();});
  T arg = q_.front();
  q_.pop();

  std::pair<bool,bool> p = qp_.front();
  empty = qpe_.front();
  sop = p.first;
  eop = p.second;

  qp_.pop();
  qpe_.pop();
  return arg;
}

template<typename T, class ... Params>
T stream<T,Params...>::_internal_cosim_front() {
  unique_lock<mutex> _{ m_ };
  bool empty = _internal_cosim_empty();
  if(empty) __ihc_hls_runtime_error_x86("Cannot do a blocking read from an empty stream on an x86 target");
  T arg = q_.front();
  return arg;
}

template<typename T, class ... Params>
T stream<T,Params...>::_internal_cosim_front(bool& sop, bool& eop) {
  unique_lock<mutex> _{ m_ };
  bool empty = _internal_cosim_empty();
  if(empty) __ihc_hls_runtime_error_x86("Cannot do a blocking read from an empty stream on an x86 target");

  T arg = q_.front();
  std::pair<bool,bool> p = qp_.front();

  sop = p.first;
  eop = p.second;
  return arg;
}

template<typename T, class ... Params>
T stream<T,Params...>::_internal_cosim_front(bool& sop, bool& eop, int& empty) {
  unique_lock<mutex> _{ m_ };
  bool empty_ = _internal_cosim_empty();
  if(empty_) __ihc_hls_runtime_error_x86("Cannot do a blocking read from an empty stream on an x86 target");

  T arg = q_.front();
  std::pair<bool,bool> p = qp_.front();
  empty = qpe_.front();
  sop = p.first;
  eop = p.second;
  return arg;
}

template<typename T, class ... Params>
bool stream<T,Params...>::tryWrite(const T& arg) {
   bool success = true; /* stl::queue has no full */
   if (success) {
      write(arg);
   }
   return success;
}

template<typename T, class ... Params>
bool stream<T,Params...>::tryWrite(const T& arg, bool sop, bool eop) {
    bool success = true; /* stl::queue has no full */
    if (success) {
        write(arg, sop, eop);
    }
    return success;
}

template<typename T, class ... Params>
bool stream<T,Params...>::tryWrite(const T& arg, bool sop, bool eop, int empty) {
    bool success = true; /* stl::queue has no full */
    if (success) {
        write(arg, sop, eop, empty);
    }
    return success;
}

template<typename T, class ... Params>
void stream<T,Params...>::write(const T& arg) {
    {
        unique_lock<mutex> _{ m_ };
        q_.push(arg);

        // sideband signals
        qp_.push(std::pair<bool,bool>(false,false));
        qpe_.push(0);
    }
    cv_.notify_one();
}

template<typename T, class ... Params>
void stream<T,Params...>::write(const T& arg, bool sop, bool eop) {
    {
        unique_lock<mutex> _{ m_ };
        q_.push(arg);

        // sideband signals
        qp_.push(std::pair<bool,bool>(sop, eop));
        qpe_.push(0);
    }
    cv_.notify_one();
}

template<typename T, class ... Params>
void stream<T,Params...>::write(const T& arg, bool sop, bool eop, int empty) {
    {
        unique_lock<mutex> _{ m_ };
        q_.push(arg);

        // sideband signals
        qp_.push(std::pair<bool,bool>(sop, eop));
        qpe_.push(empty);
    }
    cv_.notify_one();
}

template<typename T, class ... Params>
void stream<T,Params...>::read_by_ptr(void *data) {
    T elem = read();
    memcpy(data, &elem, sizeof(T));
}

template<typename T, class ... Params>
void stream<T,Params...>::read_by_ptr_pkt(void *data, bool* sop, bool* eop) {
    T elem = read(*sop, *eop);
    memcpy(data, &elem, sizeof(T));
}

template<typename T, class ... Params>
void stream<T,Params...>::read_by_ptr_pkt_e(void *data, bool* sop, bool* eop, void* empty) {
    int temp_empty;
    T elem = read(*sop, *eop, temp_empty);
    memcpy(data, &elem, sizeof(T));
    memcpy(empty, &temp_empty, sizeof(int));
}

template<typename T, class ... Params>
void stream<T,Params...>::front_by_ptr_pkt_e(void *data, bool* sop, bool* eop, void* empty) {
    int temp_empty;
    T elem = _internal_cosim_front(*sop, *eop, temp_empty);
    memcpy(data, &elem, sizeof(T));
    memcpy(empty, &temp_empty, sizeof(int));
}

template<typename T, class ... Params>
void stream<T,Params...>::front_by_ptr_pkt(void *data, bool* sop, bool* eop) {
    T elem = _internal_cosim_front(*sop, *eop);
    memcpy(data, &elem, sizeof(T));
}


template<typename T, class ... Params>
void stream<T,Params...>::front_by_ptr(void *data) {
    T elem = _internal_cosim_front();
    memcpy(data, &elem, sizeof(T));
}

template<typename T, class ... Params>
void stream<T,Params...>::write_by_ptr_pkt(void *data, bool* sop, bool* eop) {
    T elem;
    memcpy(&elem, data, sizeof(T));
    write(elem, *sop, *eop);
}

template<typename T, class ... Params>
void stream<T,Params...>::write_by_ptr_pkt_e(void *data, bool* sop, bool* eop, void* empty) {
    T elem;
    int temp_empty;
    memcpy(&elem, data, sizeof(T));
    memcpy(&temp_empty, empty, sizeof(int));
    write(elem, *sop, *eop, temp_empty);
}

template<typename T, class ... Params>
void stream<T,Params...>::write_by_ptr(void *data) {
    T elem;
    memcpy(&elem, data, sizeof(T));
    write(elem);
}

template<typename T, class ... Params>
void stream<T,Params...>::setStallPeriod() {
    if (m_stall_delta == 0) {
        m_remaining_period = m_average_stall;
    } else {
        m_remaining_period = (m_average_stall - m_stall_delta) + msws()%(m_stall_delta*2+1);
    }
    if (m_RoV_delta == 0) {
        m_period_threshold = m_average_RoV;
    } else {
        m_period_threshold = (m_average_RoV - m_RoV_delta) + msws()%(m_RoV_delta*2+1);
    }
    m_remaining_period +=  m_period_threshold;
}

template<typename T, class ... Params>
  void stream<T, Params...>::setStallCycles(unsigned average_stall, unsigned stall_delta) {
    m_average_stall = average_stall;
    m_stall_delta = stall_delta;
    setStallPeriod();
}

template<typename T, class ... Params>
  void stream<T, Params...>::setReadyorValidCycles(unsigned average_RoV, unsigned RoV_delta) {
    m_average_RoV = average_RoV;
    m_RoV_delta = RoV_delta;
    setStallPeriod();
}

template<typename T, class ... Params>
bool stream<T, Params...>::stall() {
  if (m_remaining_period == 0) {
    setStallPeriod();
  }
  m_remaining_period--;
  if (m_remaining_period < m_period_threshold) {
    return false;
  } else {
    return true;
  }
}

template<typename T, class ... Params>
unsigned stream<T, Params...>::msws() {
  static unsigned long long x = 0, w = 0;
  const unsigned long long s = 0xb5ad4eceda1ce2a9ULL;

  x *= x;
  x += (w += s);
  return (unsigned) (x = (x>>32) | (x<<32));
}

#endif
} //namespace internal
} //namespace ihc

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif


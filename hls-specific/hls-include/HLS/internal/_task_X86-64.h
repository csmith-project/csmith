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

#ifndef _INTEL_IHC_HLS_INTERNAL__TASK_X86_64
#define _INTEL_IHC_HLS_INTERNAL__TASK_X86_64
#include "HLS/internal/_function_traits.h"

#include <deque>
#ifdef _HLS_EMBEDDED_PROFILE
  #include "HLS/internal/emulator_threadsupport.h"
  #ifndef _MSC_VER
    #include <functional>
  #else
    #include "HLS/internal/_function.h"
  #endif
#else
  #include <thread>
  #include <mutex>
  #include <condition_variable>
  #include <functional>
#endif

namespace ihc {
  namespace internal {
    using namespace std;
    // Force void functions to return a value (bool true)
    // This goes into a queue of finished results,
    // to emulate the 'collect' functionality
    template<typename T>
    struct execute {
      T operator()(function<T()>& fnc) const {
        return fnc();
      }
    };
    template<>
    struct execute<void> {
      bool operator()(function <void()>& fnc) const {
        fnc();
        return true;
      }
    };

    template<auto& f, bool detach>
    class _task;

    template<auto& f, bool detach = false>
    void monitorwrapper(_task<f,detach> *);

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
      // using F: typename X is different between compilers!
      // Using R: avoid specialization for void functions
      using F = decltype(f);
      using T = typename ihc::function_traits<F>::return_type;
      using R = typename std::conditional<std::is_void<T>::value, bool, T>::type;

      friend void monitorwrapper<f,detach>(_task<f,detach> *);

      // Constructor: kick of job monitor in separate thread
      _task() = default;

      // Disable copy-assignment operator
      _task& operator=(const _task& rhs) = delete;

      // Disable copy constructor
      _task(const _task& other) = delete;

      // Disable move-assignment operator
      _task& operator=(const _task&& rhs) = delete;

      // Disable move constructor
      _task(const _task&& other) = delete;

      // Destructor
      // Exit from the main program triggers this
      // Force execution of all remaining launches
      ~_task() {
        // Flush the monitor
        _q_monitor = false;
        // Notify the monitor (if waiting)
        _q_fnc_cv.notify_one();
        if (!detach) {
          // Terminate thread
          _thread.join();
        } else {
          _thread.detach();
        }
      }

      template<int capacity, typename ... Args>
      void launch(Args&&... args) {
        auto fnc = new(function<T()>) {[=](){ return f(args...); }};
        // Avoid contention (push/pop)
        {
          unique_lock<mutex> lock{ _q_fnc_m };
          _q_fnc.emplace_back(fnc);
        }
        // Notify the monitor
        _q_fnc_cv.notify_one();
      } // launch

      template<int capacity>
      T collect() {
        unique_lock<mutex> _q_ret_lock{ _q_ret_m };
        // Guard spurious wake up with predicate
        // Release lock...
        _q_ret_cv.wait(_q_ret_lock, [&]() {return !_q_ret.empty(); });
        // Notification from push
        // ...Re-acquire lock...
        // ...thread-safe pop from deque is possible
        auto ret = _q_ret.front();
        _q_ret.pop_front();
        // Restore void if needed
        return static_cast<T>(ret);
      }

    private:
      // The private monitor method will execute launches
      // of function "f" in a separate thread;
      // The monitor launches multiple
      // launches of this function in series:
      // MAIN--launch--q_fnc       /--collect--collect-->
      // THREAD ........\----fnc--q_ret       /
      //                \..............--fnc--q_ret
      //
      // Different functions create different tasks, with
      // their own monitor (thread)
      // This means that different functions get executed
      // in separate threads in parallel:
      // MAIN    -- launch1
      // THREAD1   |  \-----fnc1--
      // MAIN      |-- launch2
      // THREAD2   |     \-----fnc2--
      std::deque < function<T()>* > _q_fnc;
      mutex _q_fnc_m;               // Access to launch queue
      condition_variable _q_fnc_cv; // Notification of of input
                                         // launch->monitor (available)
                                         // ~_task->monitor (flush)
      bool _q_monitor{ true };           // Activate the monitor

      std::deque <R> _q_ret;
      mutex _q_ret_m;               // Access to result queue
      condition_variable _q_ret_cv; // Notification of output
                                         // monitor->collect
      thread _thread{ monitorwrapper<f,detach>, this };

      void monitor() {
        // Keep monitoring the job deque
        // This will be invoked in a separate thread,
        // which sleeps when the job deque is empty
        R ret;
        unique_lock<mutex> _q_fnc_lock{ _q_fnc_m, defer_lock };
        execute<T> execute;
        while (_q_monitor || !_q_fnc.empty()) {
          // Acquire lock
          _q_fnc_lock.lock();
          // Guard spurious wake up with predicate
          // Release lock...
          _q_fnc_cv.wait(_q_fnc_lock, [this]() { return !_q_monitor || !_q_fnc.empty(); });
          // Notification from push
          // ...Re-acquire lock...
          // ...thread-safe pop from deque is possible
          if (!_q_fnc.empty()) {
            auto* fnc = _q_fnc.front();
            _q_fnc.pop_front();
            // Free lock, so more push activities can happen
            _q_fnc_lock.unlock();
            // Force a type for void functions, to emulate the collect queue
            ret = execute(*fnc);
            delete (fnc);
            // Avoid contention (push/pop)
            {
              lock_guard<mutex> q_ret_lock{ _q_ret_m };
              _q_ret.emplace_back(ret);
            }
            _q_ret_cv.notify_one(); // Notify to collect    (if blocking)
          }
          else {
            // !_q_monitor
            break;
          }
        } // while(_run)
      } // monitor

    }; // class _task

    template<auto& f, bool detach>
    void monitorwrapper(_task<f,detach> * t){
      if (t)
        t->monitor();
    }

  } //namespace internal
} // namespace ihc

#endif // _INTEL_IHC_HLS_INTERNAL__TASK_X86_64

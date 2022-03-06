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

/* Wrapper library to keep c++ library functions out of emulator code */
#ifndef _INTEL_IHC_EMULATOR_SUPPORT
#define _INTEL_IHC_EMULATOR_SUPPORT

/* Note: _ihc_* functions are part of the emulator runtime and the signatures must match what we define there */

extern "C" void *_ihc_mutex_create();
extern "C" int _ihc_mutex_delete(void *handle);
extern "C" int _ihc_mutex_lock(void *handle);
extern "C" int _ihc_mutex_unlock(void *handle);

extern "C" void* _ihc_cond_create();
extern "C" int _ihc_cond_delete(void *cv);
extern "C" int _ihc_cond_notify_one(void *cv);
extern "C" int _ihc_cond_wait(void *m,void *cv);

extern "C" void *_ihc_pthread_create(void *(*func)(void *),void *arg);
extern "C" int _ihc_pthread_join(void *handle);
extern "C" int _ihc_pthread_detach(void *handle);

class thread {
  void *_thread;
 public:
  template < class Function, class Arg >
    explicit thread(Function&& f, Arg&& arg){ 
      _thread=_ihc_pthread_create((void *(*)(void *))&f,arg); 
    }
  void join()  { _ihc_pthread_join(_thread); }
  void detach()  { _ihc_pthread_detach(_thread); }
};

class mutex {
  void *pm_ref;
 public:
  mutex()      { pm_ref=_ihc_mutex_create(); }
  mutex(const mutex&) = delete;
  ~mutex()     { _ihc_mutex_delete(pm_ref); }
  void lock()  { _ihc_mutex_lock(pm_ref); }
  void unlock(){ _ihc_mutex_unlock(pm_ref); }
  void *ref()  { return pm_ref; }
};

constexpr bool defer_lock = true;

template <class im_type>
  class unique_lock : std::false_type {};

template <>
  class unique_lock<mutex> : std::true_type {
 public:
  mutex &_im;
  unique_lock(mutex &im, bool deferred=false):_im{im} { if(!deferred) lock(); }
  unique_lock(const unique_lock&) = delete;
  ~unique_lock()                            { unlock(); }
  void lock()                               { _im.lock(); }
  void unlock()                             { _im.unlock(); }
};

template <class im_type>
  class lock_guard : std::false_type {};

template <>
  class lock_guard<mutex> : std::true_type {
  mutex &_im;
 public:
  lock_guard(mutex &im): _im{im} { _im.lock(); }
  lock_guard(const lock_guard&) = delete;
 ~lock_guard()       { _im.unlock(); }
};

class condition_variable{
  void *cond=nullptr;
 public:
  condition_variable()        { cond =_ihc_cond_create();}
  condition_variable(const condition_variable&) = delete; 
  template<typename im_type, typename pred_t>
    void wait(unique_lock<im_type> &lock, pred_t fn){ 
      while(!(fn())){
        _ihc_cond_wait(lock._im.ref(), cond);
      }
  }
  void notify_one()                {_ihc_cond_notify_one(cond);}
};

#endif //_INTEL_IHC_EMULATOR_SUPPORT

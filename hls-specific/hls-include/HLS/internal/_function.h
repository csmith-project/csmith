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

/*****************************************************************
 *                                                               *
 *  Metaprogramming to generalize lambdas in a function wrapper  *
 *    for System of Tasks emulation                              *
 *                                                               *
 *****************************************************************/

#ifndef _INTEL_IHC_HLS_FUNCTION
#define _INTEL_IHC_HLS_FUNCTION

namespace ihc {
namespace internal {

class lambda_container {
public:
  void *get() { return &lptr; }
  template <typename T> const T &get() const {
    return *(static_cast<const T *>(get()));
  }

private:
  void *lptr = nullptr;
  const void *get() const { return &lptr; }
};

template <typename Res> class function;
template <typename Res> class function<Res()> {
public:
  template <typename Sig> function(Sig lambda) {
    m_invoker = &access_methods<Sig>::invoke;
    access_methods<Sig>::init_lambda(m_lc, std::move(lambda));
  }

  explicit operator bool() const { return true; }

  Res operator()() { return (*m_invoker)(m_lc); }

private:
  lambda_container m_lc;
  Res (*m_invoker)(const lambda_container &) = nullptr;

  template <typename Sig> class access_methods {
  public:
    static Res invoke(const lambda_container &lc) {
      return (*get_pointer(lc))();
    }

    static void init_lambda(lambda_container &lc, Sig &&lambda) {
      new (lc.get()) Sig(std::move(lambda));
    }

  private:
    static Sig *get_pointer(const lambda_container &lc) {
      const Sig &l_fnc = lc.get<Sig>();
      return const_cast<Sig *>(&l_fnc);
    }
  };
};

} // namespace internal
} // namespace ihc

#endif // _INTEL_IHC_HLS_FUNCTION

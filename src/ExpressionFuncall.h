// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2015 The University of Utah
// All rights reserved.
//
// This file is part of `csmith', a random generator of C programs.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef EXPRESSION_FUNCALL_H
#define EXPRESSION_FUNCALL_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>

#include "Expression.h"
#include "FunctionInvocation.h"

class CGContext;
class FunctionInvocation;
class ExpressionVariable;

/*
 *
 */
class ExpressionFuncall : public Expression {
public:
  // Factory method.
  static Expression *make_random(CGContext &cg_context, const Type *type,
                                 const CVQualifiers *qfer = nullptr);

  explicit ExpressionFuncall(const FunctionInvocation &fi);

  virtual ~ExpressionFuncall(void) override;

  virtual Expression *clone() const override;

  virtual CVQualifiers get_qualifiers(void) const override;

  virtual void get_eval_to_subexps(vector<const Expression *> &subs) const override {
    subs.push_back(this);
  }

  virtual const FunctionInvocation *get_invoke(void) const override { return &invoke; };

  virtual const Type &get_type(void) const override;

  virtual void get_called_funcs(
      std::vector<const FunctionInvocationUser *> &funcs) const override;

  virtual unsigned int get_complexity(void) const override;

  virtual bool visit_facts(vector<const Fact *> &inputs,
                           CGContext &cg_context) const override;

  virtual bool has_uncertain_call_recursive(void) const override;

  virtual bool compatible(const Variable *) const override;

  virtual bool compatible(const Expression *) const override;

  virtual bool use_var(const Variable *v) const override;

  virtual bool equals(int num) const override { return invoke.equals(num); }
  virtual bool is_0_or_1(void) const override { return invoke.is_0_or_1(); }

  virtual std::vector<const ExpressionVariable *>
  get_dereferenced_ptrs(void) const override;
  virtual void
  get_referenced_ptrs(std::vector<const Variable *> &ptrs) const override;

  void Output(std::ostream &) const override ;
  virtual void indented_output(std::ostream &out, int indent) const override;

private:
  const FunctionInvocation &invoke;

  explicit ExpressionFuncall(const ExpressionFuncall &efun);

  // unimplementable
  ExpressionFuncall &operator=(const ExpressionFuncall &ec);
};

///////////////////////////////////////////////////////////////////////////////

#endif // EXPRESSION_FUNCALL_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

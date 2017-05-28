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

#ifndef EXPRESSION_COMMA_H
#define EXPRESSION_COMMA_H

///////////////////////////////////////////////////////////////////////////////
#include <ostream>
#include "Expression.h"

/*
 *
 */
class ExpressionComma : public Expression
{
public:
	// Factory method.
	static ExpressionComma *make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer=0);

	virtual ~ExpressionComma(void);

	Expression* clone(void) const;

	virtual CVQualifiers get_qualifiers(void) const { return rhs.get_qualifiers();}

	virtual void get_eval_to_subexps(vector<const Expression*>& subs) const;

	virtual const Type &get_type(void) const { return rhs.get_type();}

	virtual void get_called_funcs(std::vector<const FunctionInvocationUser*>& funcs) const { lhs.get_called_funcs(funcs); rhs.get_called_funcs(funcs);}

	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;

	virtual bool has_uncertain_call_recursive(void) const { return lhs.has_uncertain_call_recursive() || rhs.has_uncertain_call_recursive();}

	virtual bool use_var(const Variable* v) const { return lhs.use_var(v) || rhs.use_var(v);}

	virtual bool equals(int num) const { return rhs.equals(num);}
	virtual bool is_0_or_1(void) const { return rhs.is_0_or_1();}

	virtual std::vector<const ExpressionVariable*> get_dereferenced_ptrs(void) const;
	virtual void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const { lhs.get_referenced_ptrs(ptrs); rhs.get_referenced_ptrs(ptrs);}
	virtual unsigned int get_complexity(void) const { return get_lhs()->get_complexity() + get_rhs()->get_complexity();}

	const Expression* get_lhs(void) const { return &lhs;}
	const Expression* get_rhs(void) const { return &rhs;}
	void Output(std::ostream &) const;
	virtual void indented_output(std::ostream &out, int indent) const;

private:
	const Expression& lhs;
	const Expression& rhs;
	explicit ExpressionComma(const Expression& l, const Expression& r);
};

///////////////////////////////////////////////////////////////////////////////

#endif // EXPRESSION_COMMA_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

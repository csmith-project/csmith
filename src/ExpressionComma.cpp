// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2015, 2017 The University of Utah
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ExpressionComma.h"

#include "Common.h"
#include "CGContext.h"
#include "CGOptions.h"
#include "Reducer.h"
#include "Type.h"
#include "util.h"

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */

// Needs cast only if it is NULL
void
cast_if_needed(Expression* exp)
{
	const Type& exp_type = exp->get_type();
	if((exp->term_type == eConstant) && (exp_type.eType == ePointer) && exp->equals(0)) {
		exp->cast_type = &exp_type;
	}
}

ExpressionComma*
ExpressionComma::make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer)
{
	Expression* lhs = Expression::make_random(cg_context, NULL, NULL, false, true);
	Expression* rhs = Expression::make_random(cg_context, type, qfer, false, false);
	// typecast, if needed.
	if(CGOptions::lang_cpp())
		cast_if_needed(rhs);
	ExpressionComma* ec = new ExpressionComma(*lhs, *rhs);
	return ec;
}

ExpressionComma::ExpressionComma(const Expression& l, const Expression& r)
	: Expression(eCommaExpr),
	  lhs(l),
	  rhs(r)
{
	// nothing else to do
}

/*
 *
 */
ExpressionComma::~ExpressionComma(void)
{
	delete &lhs;
	delete &rhs;
}

Expression*
ExpressionComma::clone(void) const
{
	return new ExpressionComma(*lhs.clone(), *rhs.clone());
}

bool
ExpressionComma::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	if (!lhs.visit_facts(inputs, cg_context)) {
		return false;
	}
	return rhs.visit_facts(inputs, cg_context);
}

vector<const ExpressionVariable*>
ExpressionComma::get_dereferenced_ptrs(void) const
{
	vector<const ExpressionVariable*> ptrs1 = lhs.get_dereferenced_ptrs();
	vector<const ExpressionVariable*> ptrs2 = rhs.get_dereferenced_ptrs();
	ptrs1.insert(ptrs1.end(), ptrs2.begin(), ptrs2.end());
	return ptrs1;
}

void
ExpressionComma::get_eval_to_subexps(vector<const Expression*>& subs) const
{
	vector<const Expression*> exps;
	get_rhs()->get_eval_to_subexps(exps);
	subs.insert(subs.end(), exps.begin(), exps.end());
}

void
ExpressionComma::Output(std::ostream &out) const
{
	output_cast(out);
	Reducer* reducer = CGOptions::get_reducer();
	if (reducer && reducer->output_expr(this, out)) {
		return;
	}
	out << "(";
	lhs.Output(out);
	out << " , ";
	rhs.Output(out);
	out << ")";
}

void
ExpressionComma::indented_output(std::ostream &out, int indent) const
{
	output_tab(out, indent);
	Output(out);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

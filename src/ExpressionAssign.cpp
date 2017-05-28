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

#include "ExpressionAssign.h"
#include <cassert>
#include "Common.h"
#include "CGContext.h"
#include "CGOptions.h"
#include "FactMgr.h"
#include "Function.h"
#include "Bookkeeper.h"
#include "Variable.h"
#include "StringUtils.h"
#include "Block.h"

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
Expression *
ExpressionAssign::make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer)
{
	CVQualifiers qf;
	if (qfer == NULL) {
		qf = CVQualifiers::random_qualifiers(type, Effect::WRITE, cg_context, true);
		qfer = &qf;
	}
	StatementAssign* sa = StatementAssign::make_random(cg_context, type, qfer);
	FactMgr* fm = get_fact_mgr(&cg_context);
	FactMgr::update_fact_for_assign(sa, fm->global_facts);
	ExpressionAssign* ea = new ExpressionAssign(sa);
	return ea;
}

ExpressionAssign::ExpressionAssign(const StatementAssign* sa)
: Expression(eAssignment),
  assign(sa)
{
}

/*
 *
 */
ExpressionAssign::~ExpressionAssign(void)
{
	/*if (assign) {
		delete assign;
		assign = NULL;
	}*/
}

Expression*
ExpressionAssign::clone() const
{
	return new ExpressionAssign(assign);
}

CVQualifiers
ExpressionAssign::get_qualifiers(void) const
{
	return assign->get_lhs()->get_qualifiers();
}

/*
 * return if a variable is referenced in this expression
 */
bool
ExpressionAssign::use_var(const Variable* v) const
{
	if (assign->get_lhs()->use_var(v) || assign->get_expr()->use_var(v)) {
		return true;
	}
	return false;
}

bool
ExpressionAssign::equals(int num) const
{
	return assign->is_simple_assign() && assign->get_expr()->equals(num);
}

bool
ExpressionAssign::is_0_or_1(void) const
{
	return assign->is_simple_assign() && assign->get_expr()->is_0_or_1();
}

void
ExpressionAssign::get_eval_to_subexps(vector<const Expression*>& subs) const
{
	vector<const Expression*> exps;
	get_lhs()->get_eval_to_subexps(exps);
	subs.insert(subs.end(), exps.begin(), exps.end());
}

bool
ExpressionAssign::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	return assign->visit_facts(inputs, cg_context);
}

void
ExpressionAssign::Output(std::ostream &out) const
{
	output_cast(out);
	Reducer* reducer = CGOptions::get_reducer();
	if (reducer && reducer->output_expr(this, out)) {
		return;
	}
	out << "(";
	assign->OutputAsExpr(out);
	out << ")";
}

void
ExpressionAssign::indented_output(std::ostream &out, int indent) const
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

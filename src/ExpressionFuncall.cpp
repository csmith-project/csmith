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

#include "ExpressionFuncall.h"
#include <cassert>

#include "Common.h"

#include "CGContext.h"
#include "CGOptions.h"
#include "ExpressionVariable.h"
#include "Function.h"
#include "FunctionInvocation.h"
#include "FunctionInvocationUser.h"
#include "VariableSelector.h"
#include "FactMgr.h"
#include "Error.h"
#include "Bookkeeper.h"
#include "StringUtils.h"
#include "Reducer.h"
#include "Block.h"
#include "random.h"

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
static bool
ExpressionFunctionProbability(const CGContext &/*cg_context*/)
{
	if (Function::reach_max_functions_cnt() && !CGOptions::builtins()) {
		return true;
	}
	return rnd_flipcoin(80);
}

/*
 *
 */
Expression *
ExpressionFuncall::make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer)
{
	Expression *e = 0;
	bool std_func = ExpressionFunctionProbability(cg_context);
	ERROR_GUARD(NULL);
    // unary/binary "functions" produce scalar types only
	if (type && (type->eType != eSimple || type->simple_type == eVoid))
		std_func = false;

	Effect effect_accum = cg_context.get_accum_effect();
	Effect effect_stm = cg_context.get_effect_stm();
	FactMgr* fm = get_fact_mgr(&cg_context);
	vector<const Fact*> facts_copy = fm->global_facts;
	FunctionInvocation *fi = FunctionInvocation::make_random(std_func, cg_context, type, qfer);
	ERROR_GUARD(NULL);

	if (fi->failed) {
		// if it's a invalid invocation, (see FunctionInvocationUser::revisit)
		// restore the env, and replace invocation with a simple var
		cg_context.reset_effect_accum(effect_accum);
		cg_context.reset_effect_stm(effect_stm);
		fm->restore_facts(facts_copy);
		e = ExpressionVariable::make_random(cg_context, type, qfer);
		delete fi;
	}
	else {
		e = new ExpressionFuncall(*fi);
	}
	return e;
}

/*
 *
 */
Expression *
ExpressionFuncall::clone() const
{
	const FunctionInvocation *fi = (this->invoke).clone();
	return new ExpressionFuncall(*fi);
}

/*
 *
 */
ExpressionFuncall::ExpressionFuncall(const FunctionInvocation &fi)
	: Expression(eFunction),
	  invoke(fi)
{
}

/*
 *
 */
ExpressionFuncall::~ExpressionFuncall(void)
{
	delete &invoke;
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
const Type &
ExpressionFuncall::get_type(void) const
{
	return invoke.get_type();
}

void
ExpressionFuncall::get_called_funcs(std::vector<const FunctionInvocationUser*>& funcs) const
{
	invoke.get_called_funcs(funcs);
}

unsigned int
ExpressionFuncall::get_complexity(void) const
{
	unsigned int comp = 0;
	const FunctionInvocation* invoke = get_invoke();
	if (invoke->invoke_type == eFuncCall) {
		comp++;  // function call itself counts as 1 complexity
	}
	for (size_t i=0; i<invoke->param_value.size(); i++) {
		comp += invoke->param_value[i]->get_complexity();
	}
	return comp;
}

bool
ExpressionFuncall::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	return invoke.visit_facts(inputs, cg_context);
}

std::vector<const ExpressionVariable*>
ExpressionFuncall::get_dereferenced_ptrs(void) const
{
	// return a empty vector by default
	std::vector<const ExpressionVariable*> refs;
	for (size_t i=0; i<invoke.param_value.size(); i++) {
		// the parameters might has pointer dereferences
		const Expression* value = invoke.param_value[i];
		vector<const ExpressionVariable*> deref_ptrs = value->get_dereferenced_ptrs();
		refs.insert(refs.end(),  deref_ptrs.begin(), deref_ptrs.end());
	}
	return refs;
}

// find pointers used in the expression, recursively go into callee if this is a call
void
ExpressionFuncall::get_referenced_ptrs(std::vector<const Variable*>& ptrs) const
{
	for (size_t i=0; i<invoke.param_value.size(); i++) {
		// the parameters might has pointer references
		const Expression* value = invoke.param_value[i];
		value->get_referenced_ptrs(ptrs);
	}
	if (invoke.invoke_type == eFuncCall) {
		const FunctionInvocationUser* fiu = dynamic_cast<const FunctionInvocationUser*>(&invoke);
		assert(fiu);
		//cout << "follow " << fiu->get_func()->name << endl;
		add_variables_to_set(ptrs, fiu->get_func()->get_referenced_ptrs());
	}
}

bool
ExpressionFuncall::has_uncertain_call_recursive(void) const
{
	return invoke.has_uncertain_call_recursive();
}

/*
 * return the const/volatile qualifiers for function calls
 */
CVQualifiers
ExpressionFuncall::get_qualifiers(void) const
{
	return invoke.get_qualifiers();
}

/*
 * return if a variable is referenced in this expression
 */
bool
ExpressionFuncall::use_var(const Variable* v) const
{
	size_t i;
	for (i=0; i<invoke.param_value.size(); i++) {
		if (invoke.param_value[i]->use_var(v)) {
			return true;
		}
	}
	return false;
}

/*
 *
 */
bool
ExpressionFuncall::compatible(const Variable *v) const
{
	return invoke.compatible(v);
}

bool ExpressionFuncall::compatible(const Expression * /*exp*/) const
{
	return false;
}

/*
 *
 */
void
ExpressionFuncall::Output(std::ostream &out) const
{
	output_cast(out);
	Reducer* reducer = CGOptions::get_reducer();
	if (reducer && reducer->output_expr(this, out)) {
		return;
	}
	invoke.Output(out);
}

void
ExpressionFuncall::indented_output(std::ostream &out, int indent) const
{
	invoke.indented_output(out, indent);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

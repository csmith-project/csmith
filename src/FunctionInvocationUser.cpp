// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2015, 2017 The University of Utah
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

//
// This file was derived from a random program generator written by Bryan
// Turner.  The attributions in that file was:
//
// Random Program Generator
// Bryan Turner (bryan.turner@pobox.com)
// July, 2005
//

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef WIN32
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif

#include "FunctionInvocationUser.h"

#include <cassert>
#include <fstream>

#include "Common.h"
#include "CGContext.h"
#include "Expression.h"
#include "ExpressionVariable.h"
#include "ExpressionFuncall.h"
#include "Lhs.h"
#include "Function.h"
#include "Type.h"
#include "Variable.h"
#include "FactMgr.h"
#include "Statement.h"
#include "StatementExpr.h"
#include "StatementAssign.h"
#include "Block.h"
#include "Fact.h"
#include "SafeOpFlags.h"
#include "Error.h"

using namespace std;

static vector<bool> needcomma;  // Flag to track output of commas

static vector<const FunctionInvocationUser*> invocations;   // list of function calls
static vector<const Fact*> return_facts;              // list of return facts

const Fact*
get_return_fact_for_invocation(const FunctionInvocationUser* fiu, const Variable* var, enum eFactCategory cat)
{
	assert(invocations.size() == return_facts.size());
	for (size_t i=0; i<return_facts.size(); i++) {
		if (invocations[i] == fiu) {
			const Fact* fact = return_facts[i];
			if (fact->eCat == cat && fact->get_var() == var) {
				return fact;
			}
		}
	}
	return 0;
}

void
add_return_fact_for_invocation(const FunctionInvocationUser* fiu, const Fact* f)
{
	size_t i;
	assert(invocations.size() == return_facts.size());
	for (i=0; i<invocations.size(); i++) {
		if (invocations[i] == fiu && return_facts[i]->is_related(*f)) {
			return_facts[i] = f;
			return;
		}
	}
	invocations.push_back(fiu);
	return_facts.push_back(f);
}

/*
 * find the functions from a list of function calls.
 */
void
calls_to_funcs(const vector<const FunctionInvocationUser*>& calls, vector<const Function*>& funcs)
{
	size_t i;
	for (i=0; i<calls.size(); i++) {
		const Function* func = calls[i]->get_func();
		if (find_function_in_set(funcs, func) == -1) {
			funcs.push_back(func);
		}
	}
}

/*
 * find the functions from a list of function calls. If this is a new function, find the invocations
 * inside it recursively
 */
void
calls_to_funcs_recursive(const vector<const FunctionInvocationUser*>& calls, vector<const Function*>& funcs)
{
	size_t i;
	for (i=0; i<calls.size(); i++) {
		const Function* func = calls[i]->get_func();
		if (find_function_in_set(funcs, func) == -1) {
			funcs.push_back(func);
			// find the calls made this function and add callees recursively
			vector<const FunctionInvocationUser*> calls;
			func->body->get_called_funcs(calls);
			calls_to_funcs_recursive(calls, funcs);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

/*
 * XXX: replace with a useful constructor.
 */
FunctionInvocationUser::FunctionInvocationUser(Function *target,
						   bool isBackLink, const SafeOpFlags *flags)
	: FunctionInvocation(eFuncCall, flags),
	  func(target),
	  isBackLink(isBackLink)
{
	// Nothing else to do.  Caller must build useful params.
}

/*
 * copy constructor
 */
FunctionInvocationUser::FunctionInvocationUser(const FunctionInvocationUser &fiu)
	: FunctionInvocation(fiu),
	  func(fiu.func),
	  isBackLink(fiu.isBackLink)
{
}

/*
 *
 */
FunctionInvocationUser::~FunctionInvocationUser(void)
{
	// Nothing to do.  This object does not own `*func'.
}

FunctionInvocation *
FunctionInvocationUser::clone() const
{
	assert(this->op_flags == NULL);

	return new FunctionInvocationUser(*this);
}

/* build parameters first, then the function body. this way the generation order is in sync
   with execution order, and the dataflow analyzer doesn't need to visit the function twice
 */
FunctionInvocationUser*
FunctionInvocationUser::build_invocation_and_function(CGContext &cg_context, const Type* type, const CVQualifiers* qfer)
{
	assert(type);		// return type must be provided
	FactMgr* caller_fm = get_fact_mgr(&cg_context);
	Effect running_eff_context(cg_context.get_effect_context());
	Function* func = Function::make_random_signature(cg_context, type, qfer);

	if (func->name == "func_51")
		BREAK_NOP;		// for debugging
	vector<const Expression*> param_values;
	size_t i;
	for (i = 0; i < func->param.size(); i++) {
		Effect param_eff_accum;
		CGContext param_cg_context(cg_context, running_eff_context, &param_eff_accum);
		Variable* v = func->param[i];
		// to avoid too much function invocations as parameters
		Expression *p = Expression::make_random_param(param_cg_context, v->type, &v->qfer);
		// typecast, if needed.
		p->check_and_set_cast(v->type);
		param_values.push_back(p);
		// Update the "running effect context": the context that we must use
		// when we generate subsequent parameters within this invocation.
		running_eff_context.add_effect(param_eff_accum);
		// Update the total effect of this invocation, too.
		cg_context.merge_param_context(param_cg_context);
	}

	FunctionInvocationUser* fiu = new FunctionInvocationUser(func, false, NULL);
	fiu->param_value = param_values;
	// hand-over from caller to callee
	FactMgr* fm = get_fact_mgr_for_func(func);
	fm->global_facts = caller_fm->global_facts;
	fm->caller_to_callee_handover(fiu, fm->global_facts);

	// create function body
	Effect effect_accum;
	func->generate_body_with_known_params(cg_context, effect_accum);

	// post creation processing
	FactVec ret_facts = fm->map_facts_out[func->body];
	func->body->add_back_return_facts(fm, ret_facts);
	fiu->save_return_fact(ret_facts);

	// remove facts related to passing parameters
	//FactMgr::update_facts_for_oos_vars(func->param, fm->global_facts);
	fm->setup_in_out_maps(true);
	// hand-over from callee to caller: points-to facts
	renew_facts(caller_fm->global_facts, ret_facts);

	// hand-over from callee to caller: effects
	func->accum_eff_context.add_external_effect(cg_context.get_effect_context());
	Effect& func_effect = func->feffect;
	func_effect.add_external_effect(effect_accum, cg_context.call_chain);
	cg_context.add_visible_effect(effect_accum, cg_context.get_current_block());

	// hand-over from callee to caller: new global variables
	Function* caller_func = cg_context.get_current_func();
	caller_func->new_globals.insert(caller_func->new_globals.end(), func->new_globals.begin(), func->new_globals.end());
	// include facts for globals just created
	for (i=0; i<func->new_globals.size(); i++) {
		const Variable* var = func->new_globals[i];
		caller_fm->add_new_var_fact_and_update_inout_maps(NULL, var);
	}

	func->visited_cnt = 1;
	return fiu;
}

/*
 * Internal helper function.
 */
bool
FunctionInvocationUser::build_invocation(Function *target, CGContext &cg_context)
{
	unsigned int i;
	func = target;			// XXX: unnecessary; done by constructor
	Effect running_eff_context(cg_context.get_effect_context());
	FactMgr* fm = get_fact_mgr(&cg_context);
	// XXX DEBUGGING
	if (func->name == "func_36" && cg_context.get_current_func()->name=="func_22") {
	 	i = 0; // Set breakpoint here.
	}

	for (i = 0; i < func->param.size(); i++) {
		Effect param_eff_accum;
		CGContext param_cg_context(cg_context, running_eff_context, &param_eff_accum);
		Variable* v = func->param[i];
		// to avoid too much function invocations as parameters
		Expression *p = Expression::make_random_param(param_cg_context, v->type, &v->qfer);
		ERROR_GUARD(false);
		// typecast, if needed.
		p->check_and_set_cast(v->type);
		param_value.push_back(p);
		// Update the "running effect context": the context that we must use
		// when we generate subsequent parameters within this invocation.
		running_eff_context.add_effect(param_eff_accum);
		// Update the total effect of this invocation, too.
		cg_context.merge_param_context(param_cg_context);
	}

	// no need to validate func_1 as it has no parameters and it's called only once
	// in addition, the hack (calling func_1 in a func_1 context) we used would
	// ruin DFA
	failed = false;
	if (target != GetFirstFunction() && (target->fact_changed || target->union_field_read || target->is_pointer_referenced())) {
		// revisit with a new context
		Effect effect_accum;
		// retrive the context effect in prev. visits, and include them for this visit
		Effect effect_context = cg_context.get_effect_context();
		effect_context.add_effect(func->accum_eff_context);
		CGContext new_context(cg_context, func, effect_context, &effect_accum);
		failed = !revisit(fm->global_facts, new_context);
		// incorporate facts from revisit
		if (!failed) {
			assert(cg_context.get_current_block());
			cg_context.add_visible_effect(*new_context.get_effect_accum(), cg_context.get_current_block());
			Effect& func_effect = func->feffect;
			func_effect.add_external_effect(*new_context.get_effect_accum(), cg_context.call_chain);
		}
	}
	else {
		// if the function neither change pointer facts, nor dereference pointer (which means
		// the read/write set are static, no need to re-analyze
		cg_context.add_external_effect(func->get_feffect());
	}
	return !failed;
}

/*
 * return true if the invocation is valid (not violating fixed facts), false other wise
 *
 * side effects: update input facts and FactMgr in cg_context if the invocation is found valid
 */
bool
FunctionInvocationUser::revisit(std::vector<const Fact*>& inputs, CGContext& cg_context) const
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	fm->clear_map_visited();
	if (func->visited_cnt++ == 0) {
		fm->setup_in_out_maps(true);
	}
	// for debugging
	if (func->name=="func_10" && func->visited_cnt==19) {
		//cout << func->visited_cnt << endl;
		//func->Output(cout);
		//cout << endl;
	}

	// make copies so we can back up if fail
	vector<const Fact*> inputs_copy = inputs;

	// add facts related to pass parameters
	fm->caller_to_callee_handover(this, inputs);

	map<const Statement*, FactVec> facts_in_copy = fm->map_facts_in;
	map<const Statement*, FactVec> facts_out_copy = fm->map_facts_out;
	map<const Statement*, Effect>  stm_effect_copy = fm->map_stm_effect;
	map<const Statement*, Effect>  accum_effect_copy = fm->map_accum_effect;
	// TODO: revisit only if "contingent variable" has been changed?
	if (!func->body->visit_facts(inputs, cg_context)) {
		// restore facts and effect
		fm->map_facts_in = facts_in_copy;
		fm->map_facts_out = facts_out_copy;
		fm->map_stm_effect = stm_effect_copy;
		fm->map_accum_effect = accum_effect_copy;
		inputs = inputs_copy;
		return false;
	}
	cg_context.add_effect(fm->map_stm_effect[func->body]);
	FactVec ret_facts;
	func->body->add_back_return_facts(fm, ret_facts);
	save_return_fact(ret_facts);
	// incorporate early return facts
	merge_facts(inputs, ret_facts);

	// remove facts related to passing parameters
	FactMgr::update_facts_for_oos_vars(func->param, inputs);

	fm->setup_in_out_maps(false);
	// remember the effect context during this visit to this function
	func->accum_eff_context.add_external_effect(cg_context.get_effect_context());
	// update the original facts with new facts changed by function call
	renew_facts(inputs_copy, inputs);
	inputs = inputs_copy;
	return true;
}

/*
 * save the return fact for later use
 */
void
FunctionInvocationUser::save_return_fact(const vector<const Fact*>& facts) const
{
	size_t i;
	for (i=0; i<facts.size(); i++) {
		if (func->rv->match(facts[i]->get_var())) {
			add_return_fact_for_invocation(this, facts[i]);
		}
	}
}

/*
 * Release all dynamic memory
 */
void
FunctionInvocationUser::doFinalization(void)
{
	invocations.clear();
	return_facts.clear();
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
const Type &
FunctionInvocationUser::get_type(void) const
{
	return *(func->return_type);
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
static int
OutputActualParamExpression(const Expression *expr, std::ostream *pOut)
{
	std::ostream &out = *pOut;
	if (needcomma.back()) {
		out << ", ";
	}
	needcomma.back() = true;
	expr->Output(out);
    // for MSVC: must return something to be able to pass to a "map" function
    return 0;
}

/*
 *
 */
static void
OutputExpressionVector(const vector<const Expression*> &var, std::ostream &out)
{
	needcomma.push_back(false);
	for_each(var.begin(), var.end(),
			 std::bind2nd(std::ptr_fun(OutputActualParamExpression), &out));
	needcomma.pop_back();
}

/*
 *
 */
void
FunctionInvocationUser::Output(std::ostream &out) const
{
	out << func->name << "(";
	OutputExpressionVector(param_value, out);
	out << ")";
}

/*
 *
 */
void
FunctionInvocationUser::indented_output(std::ostream &out, int indent) const
{
	if (has_simple_params()) {
		output_tab(out, indent);
		Output(out);
		return;
	}
	output_tab(out, indent);
	out << func->name;
	outputln(out);
	output_open_encloser("(", out, indent);
	size_t i;
	for (i=0; i<param_value.size(); i++) {
		if (i > 0) outputln(out);
		param_value[i]->indented_output(out, indent);
		out << ",";
	}
	output_close_encloser(")", out, indent);
}
///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

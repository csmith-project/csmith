// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2015, 2017 The University of Utah
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

#ifdef WIN32
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif

#include <cassert>
#include "StatementIf.h"
#include <iostream>
#include "CGOptions.h"
#include "Block.h"
#include "Type.h"
#include "Function.h"
#include "Expression.h"
#include "FactMgr.h"
#include "Bookkeeper.h"
#include "Error.h"
#include "DepthSpec.h"
#include "util.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
StatementIf *
StatementIf::make_random(CGContext &cg_context)
{
	DEPTH_GUARD_BY_TYPE_RETURN(dtStatementIf, NULL);
	FactMgr* fm = get_fact_mgr(&cg_context);
	FactVec pre_facts;
	Effect pre_effect;
	// func_1 hacking, save the env in case we need to re-analyze
	if (cg_context.get_current_func()->name == "func_1" && !(cg_context.flags & IN_LOOP)) {
		pre_effect = cg_context.get_accum_effect();
		pre_facts = fm->global_facts;
	}
	cg_context.get_effect_stm().clear();
	Expression *expr = Expression::make_random(cg_context, get_int_type(), NULL, false, !CGOptions::const_as_condition());
	ERROR_GUARD(NULL);
	// func_1 hacking, re-analyze for multiple function calls
	if (cg_context.get_current_func()->name == "func_1" && !(cg_context.flags & IN_LOOP)) {
		if (expr->has_uncertain_call_recursive()) {
			fm->makeup_new_var_facts(pre_facts, fm->global_facts);
			cg_context.reset_effect_accum(pre_effect);
			cg_context.curr_blk = cg_context.get_current_block();
			bool ok = expr->visit_facts(pre_facts, cg_context);
			if (!ok) {
			//	print_facts(pre_facts);
			//	expr->indented_output(cout, 0);
			}
			assert(ok);
			fm->global_facts = pre_facts;
		}
	}
	Effect eff = cg_context.get_effect_stm();

	// this will save global_facts to map_facts_in[if_true], and update
	// facts for new variables created while generating if_true
	Block *if_true = Block::make_random(cg_context);
	ERROR_GUARD_AND_DEL1(NULL, expr);

	// generate false branch with the same env as true branch
	fm->global_facts = fm->map_facts_in[if_true];
	Block *if_false = Block::make_random(cg_context);
	ERROR_GUARD_AND_DEL2(NULL, expr, if_true);

	StatementIf* si = new StatementIf(cg_context.get_current_block(), *expr, *if_true, *if_false);
	// compute accumulated effect for this statement
	si->set_accumulated_effect_after_block(eff, if_true, cg_context);
	si->set_accumulated_effect_after_block(eff, if_false, cg_context);
    return si;
}

/*
 *
 */
StatementIf::StatementIf(Block* b, const Expression &test,
						 const Block &if_true, const Block &if_false)
	: Statement(eIfElse, b),
	  test(test),
	  if_true(if_true),
	  if_false(if_false)
{
	// Nothing else to do.
}

/*
 *
 */
StatementIf::StatementIf(const StatementIf &si)
	: Statement(si.get_type(), si.parent),
	  test(si.test),
	  if_true(si.if_true),
	  if_false(si.if_false)
{
	// Nothing else to do.
}

/*
 *
 */
StatementIf::~StatementIf(void)
{
	delete &test;
	delete &if_true;
	delete &if_false;
}

/*
 *
 */
void
StatementIf::Output(std::ostream &out, FactMgr* fm, int indent) const
{
	output_condition(out, fm, indent);
	output_branches(out, fm, indent);
}

void
StatementIf::output_condition(std::ostream &out, FactMgr* /*fm*/, int indent) const
{
	output_tab(out, indent);
	out << "if (";
	test.Output(out);
	out << ")";
	outputln(out);
}

void
StatementIf::output_branches(std::ostream &out, FactMgr* fm, int indent) const
{
	if_true.Output(out, fm, indent);
	output_tab(out, indent);
	out << "else";
	outputln(out);
	if_false.Output(out, fm, indent);
}

bool
StatementIf::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	vector<const Fact*> inputs_copy = inputs;
	// evaludate condition first
	if (!test.visit_facts(inputs, cg_context)) {
		return false;
	}
	Effect eff = cg_context.get_effect_stm();
	FactVec inputs_true = inputs;
	if (!if_true.visit_facts(inputs_true, cg_context)) {
		return false;
	}
	FactVec inputs_false = inputs;
	if (!if_false.visit_facts(inputs_false, cg_context)) {
		return false;
	}
	// compute accumulated effect for this statement
	set_accumulated_effect_after_block(eff, &if_true, cg_context);
	set_accumulated_effect_after_block(eff, &if_false, cg_context);

	// facts changed in whichever branch that must return are not considered
	// to be the ouput facts of this statement (rather they are the output
	// facts of this function)
	bool true_must_return = if_true.must_return();
	bool false_must_return = if_false.must_return();
	if (true_must_return && false_must_return) {
		inputs = inputs_copy;
	}
	else if (true_must_return) {
		inputs = inputs_false;
	}
	else if (false_must_return) {
		inputs = inputs_true;
	}
	else {
		inputs = inputs_true;
		merge_facts(inputs, inputs_false);
	}
	return true;
}

bool
StatementIf::must_return(void) const
{
	return if_true.must_return() && if_false.must_return();
}

bool
StatementIf::must_jump(void) const
{
	return if_true.must_jump() && if_false.must_jump();
}

void
StatementIf::combine_branch_facts(vector<const Fact*>& pre_facts) const
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	FactVec& outputs = fm->global_facts;
	fm->makeup_new_var_facts(pre_facts, fm->map_facts_out[&if_true]);
	fm->makeup_new_var_facts(pre_facts, fm->map_facts_out[&if_false]);

	bool true_must_return = if_true.must_return();
	bool false_must_return = if_false.must_return();
	// take return statement into consideration to achieve better precision
	if (true_must_return && false_must_return) {
		outputs = pre_facts;
	}
	else if (true_must_return) {
		// since false branch is created after true branch, it's output should
		// have all the variables created in true branch already
		outputs = fm->map_facts_out[&if_false];
	}
	else if (false_must_return) {
		outputs = fm->map_facts_out[&if_true];
		// if skip the outcome from false branch, don't forget facts of those variables
		// created in false branch
		fm->makeup_new_var_facts(outputs, fm->map_facts_in[&if_false]);
	}
	else {
		outputs = fm->map_facts_out[&if_true];
		merge_facts(outputs, fm->map_facts_out[&if_false]);
	}
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

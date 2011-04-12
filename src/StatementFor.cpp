// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011 The University of Utah
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

#include "StatementFor.h"
#include <cassert>

#include "Common.h"
#include "Block.h"
#include "CGContext.h"
#include "CGOptions.h"
#include "Constant.h"
#include "ExpressionFuncall.h"
#include "ExpressionVariable.h"
#include "Function.h"
// `FunctionInvocation' for `eBinaryOps'.  Yuck.
#include "FunctionInvocation.h"
#include "FunctionInvocationBinary.h"
#include "VariableSelector.h"
#include "FactMgr.h"
#include "Lhs.h"
#include "SafeOpFlags.h"
#include "Error.h"
#include "PartialExpander.h"
#include "Bookkeeper.h"
#include "DepthSpec.h"
#include "StatementBreak.h"
#include "CFGEdge.h"

#include "random.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 * Randomly determine the parameters for a counting loop: initial value, limit
 * value, increment value, test operator, and increment operator.
 */
static void
make_random_loop_control(int &init, int &limit, int &incr,
						 eBinaryOps &test_op,
						 eAssignOps &incr_op, 
						 bool iv_signed)
{
	// We don't have to put error guards here because we are trying
	// to get pure random numbers, and in this case, we cannot get
	// errors
	init  = pure_rnd_flipcoin(50) ? 0 : (pure_rnd_upto(60)-30); 
	limit = iv_signed ? (pure_rnd_upto(60) - 30) : (pure_rnd_upto(60) + 1);  

	eBinaryOps t_ops[] = { eCmpLt, eCmpLe, eCmpGt, eCmpGe, eCmpEq, eCmpNe };
	test_op = t_ops[pure_rnd_upto(sizeof(t_ops)/sizeof(*t_ops))];
	ERROR_RETURN();

	if (pure_rnd_flipcoin(50)) {
		ERROR_RETURN();
		// Do `+=' or `-=' by an increment between 0 and 9 inclusive.
		// make sure the limit can be reached without wrap-around
		incr_op = (limit >= init) ? eAddAssign : eSubAssign; 
		incr = pure_rnd_upto(10);
		if (incr == 0) incr = 1;
	} else {
		ERROR_RETURN();
		// Do `++' or `--', pre- or post-.
		// make sure the limit can be reached without wrap-around
		if (limit >= init) {
			incr_op = pure_rnd_flipcoin(50) ? ePreIncr : ePostIncr;
		} else {
			incr_op = pure_rnd_flipcoin(50) ? ePreDecr : ePostDecr;
		}
		incr = 1;
	}
	ERROR_RETURN();
}

/*
 *
 */
StatementFor *
StatementFor::make_random(CGContext &cg_context)
{
	DEPTH_GUARD_BY_TYPE_RETURN(dtStatementFor, NULL);
	// Generate a somewhat-trivial `for' loop.
	Function *curr_func = cg_context.get_current_func();
	assert(curr_func);
	FactMgr* fm = get_fact_mgr(&cg_context);
	assert(fm);
	Block* blk = cg_context.get_current_block();
	assert(blk);

	// save a copy of facts env and context
	vector<const Fact*> facts_copy = fm->global_facts;
	cg_context.get_effect_stm().clear();

	// Select the loop control variable, avoid volatile
	vector<const Variable*> invalid_vars;
	Variable *var = NULL;
	do {
		var = VariableSelector::SelectLoopCtrlVar(cg_context, invalid_vars); 
		ERROR_GUARD(NULL);
		if (var->is_volatile()) {
			invalid_vars.push_back(var);
		} else {
			break;
		}
	} while (true); 

	assert(cg_context.read_indices(var, fm->global_facts));
	cg_context.write_var(var);
	cg_context.read_var(var);

	// Select the loop parameters: init, limit, increment, etc.
	int        init_n, limit_n, incr_n;
	eBinaryOps test_op;
	eAssignOps incr_op;
	
	assert(var->type);
	make_random_loop_control(init_n, limit_n, incr_n, test_op, incr_op, var->type->is_signed());
	ERROR_GUARD(NULL);

	// Build the IR for the subparts of the loop.
	Constant * c_init = Constant::make_int(init_n);
	ERROR_GUARD(NULL);

	// if we chose control variable wisely, this should never return false
	assert(var);
	Lhs* lhs = new Lhs(*var);
	ERROR_GUARD_AND_DEL1(NULL, c_init);
	SafeOpFlags *flags1 = SafeOpFlags::make_random(sOpAssign);
	ERROR_GUARD_AND_DEL2(NULL, c_init, lhs);

	StatementAssign *init = new StatementAssign(*lhs, *c_init, flags1);
	ERROR_GUARD_AND_DEL3(NULL, c_init, lhs, flags1);
	assert(init->visit_facts(fm->global_facts, cg_context));

	assert(var);
	ExpressionVariable *v = new ExpressionVariable(*var);
	Bookkeeper::record_volatile_access(v->get_var(), v->get_indirect_level(), false);
	Bookkeeper::record_volatile_access(v->get_var(), v->get_indirect_level(), true);

	Constant *c_limit = Constant::make_int(limit_n);
	ERROR_GUARD_AND_DEL2(NULL, init, v);

	FunctionInvocation *invocation = FunctionInvocation::make_binary(cg_context, test_op, v, c_limit);
	ERROR_GUARD_AND_DEL3(NULL, init, v, c_limit);

	Expression *test = new ExpressionFuncall(*invocation);

	// canonize before validation
	//const ExpressionVariable exp_var(*var);
	//const FunctionInvocationBinary fb(eAdd, &exp_var, c_incr);
	//const ExpressionFuncall funcall(fb); 
	Lhs *lhs1 = dynamic_cast<Lhs*>(lhs->clone());
	//SafeOpFlags *flags2 = SafeOpFlags::make_random(sOpAssign);
	ERROR_GUARD_AND_DEL3(NULL, init, test, lhs1);

	Constant * c_incr = Constant::make_int(incr_n);
	ERROR_GUARD_AND_DEL3(NULL, init, test, lhs1);

	StatementAssign *incr = StatementAssign::make_possible_compound_assign(cg_context, *lhs1, incr_op, *c_incr);
	//StatementAssign *incr = new StatementAssign(*lhs1, incr_op, *c_incr, flags2);

	// With some probability, generate the loop under the restriction that we
	// not read and/or write to the loop control variable.
	CGContext::VariableSet body_no_read_vars(cg_context.get_no_read_vars());
	CGContext::VariableSet body_no_write_vars(cg_context.get_no_write_vars());
	if (rnd_flipcoin(90)) {
		body_no_write_vars.push_back(var->get_collective());
	}
	ERROR_GUARD_AND_DEL3(NULL, init, test, incr);
	Effect eff = cg_context.get_effect_stm();

	// TODO: actually do something with the no-read set.
	CGContext body_cg_context(cg_context.get_current_func(),
							  cg_context.stmt_depth,
							  cg_context.expr_depth,
							  (cg_context.flags | IN_LOOP),
							  cg_context.call_chain,
							  cg_context.curr_blk,
							  cg_context.focus_var,
							  body_no_read_vars,
							  body_no_write_vars,
							  cg_context.get_effect_context(),
							  cg_context.get_effect_accum());
	
	// for debugging
	//static int g = 0;
	Block *body = Block::make_random(body_cg_context, true);
	ERROR_GUARD_AND_DEL3(NULL, init, test, incr);

	StatementFor* sf = new StatementFor(*init, *test, *incr, *body);
	// if the control reached the end of this for-loop with must-return body, it means
	// the loop is never entered. restore facts to pre-loop env
	fm->global_facts = fm->map_facts_in[sf->get_body()];
	if (sf->get_body()->must_return()) {
		fm->restore_facts(facts_copy);
	}	
	// add forward edges introduced by "break"
	for (size_t i=0; i<sf->get_body()->break_stms.size(); i++) {
		const StatementBreak* stm = dynamic_cast<const StatementBreak*>(sf->get_body()->break_stms[i]);
		fm->create_cfg_edge(stm, sf, true, false);
		merge_jump_facts(fm->global_facts, fm->map_facts_out[stm]);
	}
	// compute accumulated effect
	sf->set_accumulated_effect_after_block(eff, body, cg_context);
	return sf;
}

/*
 *
 */
StatementFor::StatementFor(const StatementAssign &init,
						   const Expression &test,
						   const StatementAssign &incr,
						   const Block &body)
	: Statement(eFor),
	  init(init),
	  test(test),
	  incr(incr),
	  body(body)
{
	// Nothing else to do.
}

#if 0
/*
 * unimplement it
 */
StatementFor::StatementFor(const StatementFor &sf)
	: Statement(sf.get_type()),
	  init(sf.init),
	  test(sf.test),
	  incr(sf.incr),
	  body(sf.body)
{
	// Nothing else to do.
}
#endif

/*
 *
 */
StatementFor::~StatementFor(void)
{
	delete &init;
	delete &test;
	delete &incr;
	delete &body;
}

void
StatementFor::output_header(std::ostream& out, int indent) const
{
	output_tab(out, indent);
	out << "for (";
	init.OutputAsExpr(out);
	out << "; ";
	test.Output(out);
	out << "; ";
	incr.OutputAsExpr(out);
	out << ")";
	outputln(out);
}

/*
 *
 */
void
StatementFor::Output(std::ostream &out, FactMgr* fm, int indent) const
{
	output_header(out, indent);
	body.Output(out, fm, indent);
}

bool 
StatementFor::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{   
	// walk the initializing statement
	if (!init.visit_facts(inputs, cg_context)) {
		return false;
	} 
	//print_facts(inputs);
	FactVec facts_copy = inputs;
	Effect eff = cg_context.get_effect_stm();
	if (!body.visit_facts(inputs, cg_context)) {
		return false;
	}
	FactMgr* fm = get_fact_mgr(&cg_context);
	// if body must return, means the control reached end of for-loop with pre-loop env
	if (body.must_return()) {
		inputs = facts_copy;
	} else {
		inputs = fm->map_facts_in[&body];
	}
	 
	// include the facts from "break" statements 
	// find edges leading to the end of this statement, and merge
	size_t i;
	vector<const CFGEdge*> edges;
	find_edges_in(edges, true, false);
	for (i=0; i<edges.size(); i++) { 
		const Statement* src = edges[i]->src;
		merge_jump_facts(inputs, fm->map_facts_out[src]);
	}
	// compute accumulated effect
	set_accumulated_effect_after_block(eff, &body, cg_context);
	return true;
} 

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

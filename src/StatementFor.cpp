// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2014, 2015, 2016, 2017 The University of Utah
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
#include "ArrayVariable.h"
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
		incr = pure_rnd_upto(10);
    // avoid an infinite loop due to inexact division, e.g,
    // init = 0, limit = 3, step = 2, test_op = eCmpNe
    if (test_op == eCmpNe && incr > 1)
      limit = (limit - init) / incr * incr + init;
    incr_op = (limit >= init) ? eAddAssign : eSubAssign;
    if (incr == 0) incr = 1;

    // A rare case that could cause wrap around: distance between init
    // and limit is multiple of incr, and the incr goes to wrong direction.
    // For example: init = 8, limit = 1, incr = -7. test_op is >=
    if (CGOptions::fast_execution() && (limit - init) % incr == 0 && 
      (test_op == eCmpGe || test_op == eCmpLe)) {
      limit = (incr_op == eAddAssign) ? limit + 1 : limit - 1;
    }
	} else {
		ERROR_RETURN();
		// Do `++' or `--', pre- or post-.
		// make sure the limit can be reached without wrap-around
		if ((limit < init) || ((limit == init) && (test_op == eCmpGe))) {
			incr_op = pure_rnd_flipcoin(50) ? ePreDecr : ePostDecr;
		} else {
			incr_op = pure_rnd_flipcoin(50) ? ePreIncr : ePostIncr;
		}
		if (((incr_op == ePreIncr) && !CGOptions::pre_incr_operator())
			|| ((incr_op == ePostIncr) && !CGOptions::post_incr_operator())) {

			incr_op = eAddAssign;
		}
		if (((incr_op == ePreDecr) && !CGOptions::pre_decr_operator())
			|| ((incr_op == ePostDecr) && !CGOptions::post_decr_operator())) {

			incr_op = eSubAssign;
		}
		incr = 1;
	}
	ERROR_RETURN();
}

/*
 * Randomly determine the parameters for an array-travering loop
 */
static unsigned int
make_random_array_control(unsigned int bound, int &init, int &limit, int &incr, eBinaryOps &test_op, eAssignOps &incr_op, bool is_signed)
{
	// choose either increment or decrement
	test_op = is_signed ? (rnd_flipcoin(50) ? eCmpLe : eCmpGe) : eCmpLe;
	if (test_op == eCmpLe) {
		// increment, start near index 0
		init  = pure_rnd_flipcoin(50) ? 0 : pure_rnd_upto(bound/2);
		limit = bound;
		incr_op = eAddAssign;
		incr = pure_rnd_flipcoin(50) ? 1 : pure_rnd_upto(bound/4);
		if (incr == 0) incr = 1;
		bound = ((bound - init) / incr) * incr + init;
	} else {
		// decrement, start near last index
		init = pure_rnd_flipcoin(50) ? (bound) : (bound - pure_rnd_upto(bound/2));
		limit = pure_rnd_flipcoin(50) ? 0 : pure_rnd_upto(bound/2);
		incr_op = eSubAssign;
		incr = pure_rnd_flipcoin(50) ? 1 : pure_rnd_upto(bound/4);
		if (incr == 0) incr = 1;
		bound = init;
	}
	return bound;
}

const Variable*
StatementFor::make_iteration(CGContext& cg_context, StatementAssign*& init, Expression*& test, StatementAssign*& incr, unsigned int& bound)
{
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

	bool read = cg_context.read_indices(var, fm->global_facts);
	assert(read);
	cg_context.write_var(var);
	cg_context.read_var(var);

	// Select the loop parameters: init, limit, increment, etc.
	int        init_n, limit_n, incr_n;
	eBinaryOps test_op;
	eAssignOps incr_op;
	bound = INVALID_BOUND;

	// choose a random array from must use variables, and find the dimension with shortest length
	// JYTODO: be more aggressive?
	if (cg_context.rw_directive) {
		vector<const Variable*> arrays;
		cg_context.rw_directive->find_must_use_arrays(arrays);
		if (!arrays.empty()) {
			const ArrayVariable* av = dynamic_cast<const ArrayVariable*>(VariableSelector::choose_ok_var(arrays));
			assert(av);
			for (size_t i=0; i<av->get_dimension(); i++) {
				if (av->get_sizes()[i] < bound) {
					bound = av->get_sizes()[i];
				}
			}
		}
	}
	if (bound != INVALID_BOUND) {
		bound = make_random_array_control(--bound, init_n, limit_n, incr_n, test_op, incr_op, var->type->is_signed());
	} else {
		assert(var->type);
		make_random_loop_control(init_n, limit_n, incr_n, test_op, incr_op, var->type->is_signed());
	}
	ERROR_GUARD(NULL);

	// Build the IR for the subparts of the loop.
	Constant * c_init = Constant::make_int(init_n);
	ERROR_GUARD(NULL);

	// if we chose control variable wisely, this should never return false
	assert(var);
	Lhs* lhs = new Lhs(*var);
	ERROR_GUARD_AND_DEL1(NULL, c_init);
	eBinaryOps bop = StatementAssign::compound_to_binary_ops(incr_op);
	SafeOpFlags *flags1 = SafeOpFlags::make_random_binary(var->type, var->type, var->type, sOpAssign, bop);
	ERROR_GUARD_AND_DEL2(NULL, c_init, lhs);

	init = new StatementAssign(cg_context.get_current_block(), *lhs, *c_init, eSimpleAssign, flags1);
	ERROR_GUARD_AND_DEL3(NULL, c_init, lhs, flags1);
	bool visited = init->visit_facts(fm->global_facts, cg_context);
	assert(visited);

	assert(var);
	ExpressionVariable *v = new ExpressionVariable(*var);
	Bookkeeper::record_volatile_access(v->get_var(), v->get_indirect_level(), false);
	Bookkeeper::record_volatile_access(v->get_var(), v->get_indirect_level(), true);

	Constant *c_limit = Constant::make_int(limit_n);
	ERROR_GUARD_AND_DEL2(NULL, init, v);

	FunctionInvocation *invocation = FunctionInvocation::make_binary(cg_context, test_op, v, c_limit);
	ERROR_GUARD_AND_DEL3(NULL, init, v, c_limit);

	test = new ExpressionFuncall(*invocation);

	// canonize before validation
	//const ExpressionVariable exp_var(*var);
	//const FunctionInvocationBinary fb(eAdd, &exp_var, c_incr);
	//const ExpressionFuncall funcall(fb);
	Lhs *lhs1 = dynamic_cast<Lhs*>(lhs->clone());
	//SafeOpFlags *flags2 = SafeOpFlags::make_random(sOpAssign);
	ERROR_GUARD_AND_DEL3(NULL, init, test, lhs1);

	Constant * c_incr = Constant::make_int(incr_n);
	ERROR_GUARD_AND_DEL3(NULL, init, test, lhs1);

	if (bound != INVALID_BOUND) {
		incr = new StatementAssign(cg_context.get_current_block(), *lhs1, *c_incr, incr_op);
	} else {
		incr = StatementAssign::make_possible_compound_assign(cg_context, &(v->get_type()), *lhs1, incr_op, *c_incr);
	}
	return var;
}

/*
 * Generate a somewhat-trivial `for' loop
 */
StatementFor *
StatementFor::make_random(CGContext &cg_context)
{
	FactMgr* fm = get_fact_mgr(&cg_context);
	assert(fm);
	cg_context.get_effect_stm().clear();

	StatementAssign* init = NULL;
	StatementAssign* incr = NULL;
	Expression* test = NULL;
	unsigned int bound = 0;
	const Variable* iv = make_iteration(cg_context, init, test, incr, bound);
	// record the effect and facts before loop body
	Effect pre_effects = cg_context.get_effect_stm();
	vector<const Fact*> pre_facts = fm->global_facts;

	// create CGContext for body
	CGContext body_cg_context(cg_context, cg_context.rw_directive, iv, bound);
	Block *body = Block::make_random(body_cg_context, true);
	ERROR_GUARD_AND_DEL3(NULL, init, test, incr);

	StatementFor* sf = new StatementFor(cg_context.get_current_block(), *init, *test, *incr, *body);
	sf->post_loop_analysis(cg_context, pre_facts, pre_effects);
	return sf;
}

/* generate loop traversing array(s) */
StatementFor *
StatementFor::make_random_array_loop(const CGContext &cg_context)
{
	// select the number of arrays to manipulate, default maximum = 4;
	unsigned int aryno = rnd_upto(CGOptions::max_array_num_in_loop());
	// choose arrays to manipulate, create new ones if necessary
	VariableSet must_reads, must_writes;
	for (size_t i=0; i<aryno; i++) {
		const ArrayVariable* av = VariableSelector::select_array(cg_context);
		// random access choice: 0 = must read, 1 = must write, 2 = both
		int access = rnd_upto(3);
		if (access == 0 || access == 2) {
			add_variable_to_set(must_reads, (const Variable*)av);
		}
		if (access == 1 || access == 2) {
			add_variable_to_set(must_writes, (const Variable*)av);
		}
	}
	// create read/write directive from existing context and incoming directives
	VariableSet all_must_reads, all_must_writes, no_reads, no_writes;
	if (cg_context.rw_directive) {
		combine_variable_sets(cg_context.rw_directive->must_read_vars, must_reads, all_must_reads);
		combine_variable_sets(cg_context.rw_directive->must_write_vars, must_writes, all_must_writes);
		no_reads = cg_context.rw_directive->no_read_vars;
		no_writes = cg_context.rw_directive->no_write_vars;
	} else {
		all_must_reads = must_reads;
		all_must_writes = must_writes;
	}
	RWDirective rwd(no_reads, no_writes, all_must_reads, all_must_writes);
	// create CGContext for loop
	CGContext loop_cg_context(cg_context, &rwd, NULL, 0);
	StatementFor* sf = make_random(loop_cg_context);
	return sf;
}

void
StatementFor::post_loop_analysis(CGContext& cg_context, vector<const Fact*>& pre_facts, Effect& pre_effect)
{
	FactMgr* fm = get_fact_mgr(&cg_context);
	assert(fm);
	// if the control reached the end of this for-loop with must-return body, it means
	// the loop is never entered. restore facts to pre-loop env
	fm->global_facts = fm->map_facts_in[&body];
	if (body.must_return()) {
		fm->restore_facts(pre_facts);
	}
	// add forward edges introduced by "break"
	for (size_t i=0; i<body.break_stms.size(); i++) {
		const StatementBreak* stm = dynamic_cast<const StatementBreak*>(body.break_stms[i]);
		fm->create_cfg_edge(stm, this, true, false);
		FactMgr::merge_jump_facts(fm->global_facts, fm->map_facts_out[stm]);
	}
	// compute accumulated effect
	set_accumulated_effect_after_block(pre_effect, &body, cg_context);
}

/*
 *
 */
StatementFor::StatementFor(Block* b, const StatementAssign &init,
						   const Expression &test,
						   const StatementAssign &incr,
						   const Block &body)
	: Statement(eFor, b),
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

	const Variable* iv = init.get_lhs()->get_var();
	// the indction variable should be scalar, and shouldn't be the IV of an outer loop
	assert(iv->type->eType == eSimple);
	assert(cg_context.iv_bounds.find(iv) == cg_context.iv_bounds.end());
	// give an arbitrary bound that we don't check against
	cg_context.iv_bounds[iv] = 0;

	if (!body.visit_facts(inputs, cg_context)) {
		// remove IV from context
		cg_context.iv_bounds.erase(iv);
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
		FactMgr::merge_jump_facts(inputs, fm->map_facts_out[src]);
	}
	// compute accumulated effect
	set_accumulated_effect_after_block(eff, &body, cg_context);
	// remove IV from context
	cg_context.iv_bounds.erase(iv);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

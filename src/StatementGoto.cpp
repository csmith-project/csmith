// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2015, 2017 The University of Utah
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

#include "StatementGoto.h"
#include <iostream>
#include <cassert>
#include "CGOptions.h"
#include "CGContext.h"
#include "Block.h"
#include "Function.h"
#include "Expression.h"
#include "ExpressionVariable.h"
#include "Variable.h"
#include "VariableSelector.h"
#include "Type.h"
#include "FactMgr.h"
#include "random.h"
#include "Error.h"
#include "Bookkeeper.h"

using namespace std;

std::map<const Statement*, string> StatementGoto::stm_labels;

///////////////////////////////////////////////////////////////////////////////
/*
 *
 */
StatementGoto *
StatementGoto::make_random(CGContext &cg_context)
{
	//static int g = 0;
	//int h = g++;
	Block* curr_blk = cg_context.get_current_block();
	Function* func = cg_context.get_current_func();
	FactMgr* fm = get_fact_mgr(&cg_context);

	// find blocks that are good for backward or forward jump (a good block is the one
	// doesn't create un-initialized variables with the jump)
	bool back_edge = false;
	vector<Block*> blks = func->blocks;
	Block* ok_blk = 0;
	bool select_back_edge = rnd_flipcoin(40);
	ERROR_GUARD(NULL);
	if (select_back_edge) {
		// create a backward "goto", this creates a loop when other_stm leads to stm
		back_edge = true;
		ok_blk = find_good_jump_block(blks, curr_blk, true);
	}
	if (ok_blk == 0) {
		back_edge = false;
		blks = func->blocks;
		ok_blk = find_good_jump_block(blks, curr_blk, false);
	}
	if (ok_blk == NULL) return NULL;

	const Statement* stm = 0;
	const Statement* other_stm = 0;
	if (!curr_blk->stms.empty()) {
		stm = curr_blk->stms.back();
	}
	// find statements that are good for backward/forward jumps (a good statement has
	// the type other than break/continue/goto/return) (relax this constriction?)
	size_t i;
	vector<const Statement*> ok_stms;
	for (i=0; i<ok_blk->stms.size(); i++) {
		const Statement* s = ok_blk->stms[i];
		if (s != stm) {
			// forward goto shouldn't be inserted after a return statement
			// disallow this for backward goto as well for now
			if (!s->must_return()) {
				ok_stms.push_back(s);
			}
		}
	}

	if (ok_stms.size() > 0) {
		size_t stm_id = rnd_upto(ok_stms.size());
		ERROR_GUARD(NULL);
		other_stm = ok_stms[stm_id];
		cg_context.get_effect_stm().clear();
		//Expression* test = Expression::make_random(cg_context, get_int_type(), true, true, eVariable);
		// use a variable that is already read in the context to avoid introducing conflict by the condition
		const Variable* cond_var = NULL;
		if (back_edge) {
			cond_var = VariableSelector::choose_visible_read_var(curr_blk,
				cg_context.get_effect_accum()->get_read_vars(), get_int_type(), fm->global_facts);
		} else {
			// travel in time, find a suitable variable read at generation time of the other statement
			cond_var = VariableSelector::choose_visible_read_var(ok_blk,
				fm->map_accum_effect[other_stm].get_read_vars(), get_int_type(), fm->map_facts_out[other_stm]);
		}
		if (cond_var == 0) {
			return NULL;
		}
		Expression* test = new ExpressionVariable(*cond_var);

		StatementGoto* sg = 0;
		vector<const Variable*> skipped_vars;

		if (back_edge) {
			sg = new StatementGoto(curr_blk, *test, other_stm, skipped_vars);
			fm->create_cfg_edge(sg, other_stm, false, true);
			// find the least incomplete block that contains both stm and other_stm,
			// which is the block that needs to be re-analyzed due to this "goto" once
			// done with creation
			Block* b = curr_blk;
			while (!b->contains_stmt(other_stm)) {
				b = b->parent;
			}
			assert(b);
			b->need_revisit = true;
			Bookkeeper::backward_jump_cnt++;
			return sg;
		}
		else {
			// create a forward "goto", insert after "other_stm"
			// make sure the jump doesn't cause trouble to the existing DFA analyzer
			FactVec goto_out, stm_in, stm_out;
			bool ok = true;
			bool found_new_facts = false;
			// JYTODO: don't assume facts_in == facts_out for control statements
			FactVec& goto_in = other_stm->is_ctrl_stmt() ? fm->map_facts_in[other_stm] : fm->map_facts_out[other_stm];
			FactMgr::update_facts_for_dest(goto_in, goto_out, stm);
			stm_in = fm->map_facts_in[stm];
			Effect pre_effect = cg_context.get_accum_effect();
			// merge the effect from goto src
			cg_context.add_effect(fm->map_accum_effect[other_stm]);
			if (FactMgr::merge_jump_facts(stm_in, goto_out)) {
				stm_out = stm_in;
				found_new_facts = true;
				map<const Statement*, FactVec> facts_in_copy, facts_out_copy;
				fm->backup_stm_fact_maps(stm, facts_in_copy, facts_out_copy);
				ok = stm->stm_visit_facts(stm_out, cg_context);
				if (!ok) {
					fm->restore_stm_fact_maps(stm, facts_in_copy, facts_out_copy);
					cg_context.reset_effect_accum(pre_effect);
					return NULL;
				}
				// in cases where "stm" contains "other_stm", the above "stm_visit_facts" will cause "map_facts_in[other_stm]" to be updated
				if (stm->contains_stmt(other_stm)) {
					FactMgr::update_facts_for_dest(goto_in, goto_out, stm);
				}
			}

			Block* other_blk = other_stm->parent;
			sg = new StatementGoto(other_blk, *test, stm, skipped_vars);
			for (i=0; i<other_blk->stms.size(); i++) {
				if (other_blk->stms[i] == other_stm) {
					// note we don't return goto statement for forward edges, instead the
					// statement is inserted into an existing block
					other_blk->stms.insert(other_blk->stms.begin()+i+1, sg);
					break;
				}
			}
			// take care in/out facts for newly created goto statement, and the jump destination
			fm->set_fact_in(sg, goto_in);
			fm->map_facts_out[sg] = goto_out;
			fm->map_visited[sg] = true;
			if (found_new_facts) {
				fm->set_fact_in(stm, stm_in);
				fm->set_fact_out(stm, stm_out);
			}
			fm->create_cfg_edge(sg, stm, false, false);
			fm->global_facts = fm->map_facts_out[stm];
			// special handling for control statements: their output facts has been altered for oos variables
			// use the input facts intead (warning: this rely on the assumption that these statements doesn't
			// change fact env.
			if (stm->is_ctrl_stmt() || stm->eType == eReturn) {
				fm->global_facts = fm->map_facts_in[stm];
			}
			Bookkeeper::forward_jump_cnt++;
		}
	}
	return NULL;
}

/*
 *
 */
StatementGoto::StatementGoto(Block* b, const Expression &test, const Statement* dest, const std::vector<const Variable*>& vars)
	: Statement(eGoto, b),
	  test(test),
	  dest(dest),
	  init_skipped_vars(vars)
{
	if (stm_labels.find(dest) != stm_labels.end()){
		label = stm_labels[dest];
	}
	else {
		label = gensym("lbl_");
		stm_labels[dest] = label;
	}
}

/*
 *
 */
StatementGoto::StatementGoto(const StatementGoto &sg)
: Statement(sg.get_type(), sg.parent),
	  test(sg.test),
	  dest(sg.dest),
	  label(sg.label),
	  init_skipped_vars(sg.init_skipped_vars)
{
	// Nothing else to do.
}

/*
 *
 */
StatementGoto::~StatementGoto(void)
{
	delete &test;
}

/*
 *
 */
void
StatementGoto::Output(std::ostream &out, FactMgr* /*fm*/, int indent) const
{
	output_tab(out, indent);
	out << "if (";
	test.Output(out);
	out << ")";
	outputln(out);
	output_tab(out, indent+1);
	out << "goto " << label << ";";
	outputln(out);
}

/*
 *
 */
void
StatementGoto::output_skipped_var_inits(std::ostream &out, int indent) const
{
	size_t i;
	for (i=0; i<init_skipped_vars.size(); i++) {
		output_tab(out, indent);
		const Variable* v = init_skipped_vars[i];
		out << v->get_actual_name() << " = ";
		assert(v->init);
		v->init->Output(out);
		out << ";";
		outputln(out);
	}
}

/*
 * find all the local variables whose initialization has been skipped due to this jump
 */
bool
StatementGoto::has_init_skipped_vars(const Block* src_blk, const Statement* dest)
{
	vector<Variable*> local_vars;
	vector<const Variable*> skipped_vars;
	Block* b = dest->parent;
	for (; b && b != src_blk; b = b->parent) {
		local_vars.insert(local_vars.end(), b->local_vars.begin(), b->local_vars.end());
	}
	size_t i;
	skipped_vars.clear();
	for (i=0; i<local_vars.size(); i++) {
		const Variable* v = local_vars[i];
		//b==parent means the jump destination is inside a block that also
		//contains "goto", in that case, all locals in sub-blocks have been skipped
		if (b==src_blk || !v->is_visible_local(src_blk)) {
			// if v is a const, this re-initialize strategy would fail, therefore we have
			// to disallow this goto
			// similarly, we can not do this for struct/union variables
			//if (v->is_const() || !v->type->fields.empty()) {
			//	return false;
			//}
			//skipped_vars.push_back(v);
			return true;
		}
	}
	return false;
}

Block*
StatementGoto::find_good_jump_block(vector<Block*>& blocks, const Block* blk, bool as_dest)
{
	if (blocks.empty()) return NULL;
	// if the current block is in array-traversing loop, we don't want any jumps into it
	if (blk->in_array_loop && !as_dest) return NULL;
	// if the current block hasn't generated any statement, we can not create a "goto" targeted to this block
	if (blk->stms.empty() && !as_dest) return NULL;
	const Statement* last_stm = blk->get_last_stm();
	if (last_stm && last_stm->must_return() && as_dest) return NULL;

	size_t index = rnd_upto(blocks.size());
	ERROR_GUARD(NULL);
	// disallow jumping to a block that is inside an array traversing loop
	if (blocks[index]->in_array_loop && as_dest) {
		blocks.erase(blocks.begin() + index);
		return find_good_jump_block(blocks, blk, as_dest);
	}
	// if block has no statement, delete it and throw dice again
	if (blocks[index]->stms.empty()) {
		blocks.erase(blocks.begin() + index);
		return find_good_jump_block(blocks, blk, as_dest);
	}
	// if the block has only return statement, we can not use it as the jump source,
	// so delete and throw dice again
	if (blocks[index]->stms.size()==1 && blocks[index]->stms[0]->eType==eReturn && !as_dest) {
		blocks.erase(blocks.begin() + index);
		return find_good_jump_block(blocks, blk, as_dest);
	}

	Block* b = blocks[index];
	if (b == blk) {
		return b;
	}
	if ((as_dest && has_init_skipped_vars(blk, b->stms[0])) ||
		(!as_dest && has_init_skipped_vars(b, blk->stms[0])))
	{
		blocks.erase(blocks.begin() + index);
		return find_good_jump_block(blocks, blk, as_dest);
	}
	return b;
}

/*
 * return true if condition is always true
 */
bool
StatementGoto::must_jump(void) const
{
	return test.not_equals(0);
}

bool
StatementGoto::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	// evaludate condition first
	if (!test.visit_facts(inputs, cg_context)) {
		return false;
	}
	size_t i;
	for (i=0; i<init_skipped_vars.size(); i++) {
		if (!cg_context.check_write_var(init_skipped_vars[i], inputs)) {
			return false;
		}
	}
	FactMgr* fm = get_fact_mgr(&cg_context);
	/* in a case like the following
	      goto ll;
		  a = &b;
	      if ( ...) {
	 ll:     ...
	      }
	 and a revisit to the function caused output env {a -> {b, c}} for "goto ll",
	 a subsequent revisit that caused output env {a -> {b}} for "goto ll" would
	 bypass analyzing if statement and returns {a -> {b, c}} (from the last round)
	 even it should be {a -> {b}}. (this is determined by Statement::contains_unfixed_goto)

	 To address this problem, if a goto statement's output env is a subset of last
	 round output (but not exact match), the jump target (and it's enclosing
	 statements) is forced to re-analyze by resetting the target's input and output env
	 (re-analysis is enforced through Statement::contains_unfixed_goto)
	 */
	if (!fm->map_visited[this] &&
		!fm->map_visited[dest] &&
		!same_facts(inputs, fm->map_facts_out[this]) &&
		subset_facts(inputs, fm->map_facts_out[this])) {
			//print_facts(inputs);
			//cout << endl;
			//print_facts(fm->map_facts_out[this]);
			fm->map_facts_in[dest].clear();
			fm->map_facts_out[dest].clear();
	}
	fm->map_stm_effect[this] = cg_context.get_effect_stm();
	return true;
}

void
StatementGoto::doFinalization(void)
{
	stm_labels.clear();
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2015, 2016, 2017 The University of Utah
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
#include "Block.h"

#include <cassert>
#include <sstream>

#include "CGContext.h"
#include "CGOptions.h"
#include "Function.h"
#include "FunctionInvocationUser.h"
#include "Statement.h"
#include "StatementGoto.h"
#include "Variable.h"
#include "VariableSelector.h"
#include "FactMgr.h"
#include "random.h"
#include "util.h"
#include "DepthSpec.h"
#include "Error.h"
#include "CFGEdge.h"
#include "Expression.h"
#include "VectorFilter.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
Block* find_block_by_id(int blk_id)
{
	const vector<Function*>& funcs = get_all_functions();
	size_t i, j;
	for (i=0; i<funcs.size(); i++) {
		Function* f = funcs[i];
		if (f->is_builtin)
			continue;
		for (j=0; j<f->blocks.size(); j++) {
			if (f->blocks[j]->stm_id == blk_id) {
				return f->blocks[j];
			}
		}
	}
	return NULL;
}

/*
 *
 */
static unsigned int
BlockProbability(Block &block)
{
	vector<unsigned int> v;
	v.push_back(block.block_size() - 1);
	VectorFilter filter(v, NOT_FILTER_OUT);
	filter.disable(fDefault);
	return rnd_upto(block.block_size(), &filter);
}

Block *
Block::make_dummy_block(CGContext &cg_context)
{
	Function *curr_func = cg_context.get_current_func();
	assert(curr_func);

	Block *b = new Block(cg_context.get_current_block(), 0);
	b->func = curr_func;
	b->in_array_loop = !(cg_context.iv_bounds.empty());
	curr_func->blocks.push_back(b);
	curr_func->stack.push_back(b);
	FactMgr* fm = get_fact_mgr_for_func(curr_func);
	fm->set_fact_in(b, fm->global_facts);
	Effect pre_effect = cg_context.get_accum_effect();
	b->post_creation_analysis(cg_context, pre_effect);
	curr_func->stack.pop_back();
	return b;
}

/*
 *
 */
Block *
Block::make_random(CGContext &cg_context, bool looping)
{
	//static int bid = 0;
	DEPTH_GUARD_BY_TYPE_RETURN(dtBlock, NULL);

	Function *curr_func = cg_context.get_current_func();
	assert(curr_func);

	Block *b = new Block(cg_context.get_current_block(), CGOptions::max_block_size());
	b->func = curr_func;
	b->looping = looping;
	// if there are induction variables, we are in a loop that traverses array(s)
	b->in_array_loop = !(cg_context.iv_bounds.empty());
	//b->stm_id = bid++;

	// Push this block onto the variable scope stack.
	curr_func->stack.push_back(b);
	curr_func->blocks.push_back(b);

	// record global facts at this moment so that subsequent statement
	// inside the block doesn't ruin it
	FactMgr* fm = get_fact_mgr_for_func(curr_func);
	fm->set_fact_in(b, fm->global_facts);
	Effect pre_effect = cg_context.get_accum_effect();

	unsigned int max = BlockProbability(*b);
	if (Error::get_error() != SUCCESS) {
		curr_func->stack.pop_back();
		delete b;
		return NULL;
	}
	unsigned int i;
	if (b->stm_id == 1)
		BREAK_NOP;			// for debugging
	for (i = 0; i <= max; ++i) {
		Statement *s = Statement::make_random(cg_context);
		// In the exhaustive mode, Statement::make_random could return NULL;
		if (!s)
			break;
		b->stms.push_back(s);
		if (s->must_return()) {
			break;
		}
	}

	if (Error::get_error() != SUCCESS) {
		curr_func->stack.pop_back();
		delete b;
		return NULL;
	}

	// append nested loop if some must-read/write variables hasn't been accessed
	if (b->need_nested_loop(cg_context) && cg_context.blk_depth < CGOptions::max_blk_depth()) {
		b->append_nested_loop(cg_context);
	}

	// perform DFA analysis after creation
	b->post_creation_analysis(cg_context, pre_effect);

	if (Error::get_error() != SUCCESS) {
		curr_func->stack.pop_back();
		delete b;
		return NULL;
	}

	curr_func->stack.pop_back();
	if (Error::get_error() != SUCCESS) {
		//curr_func->stack.pop_back();
		delete b;
		return NULL;
	}

	// ISSUE: in the exhaustive mode, do we need a return statement here
	// if the last statement is not?
	Error::set_error(SUCCESS);
	return b;
}

/*
 *
 */
Block::Block(Block* b, int block_size)
	: Statement(eBlock, b),
	  need_revisit(false),
	  depth_protect(false),
	  block_size_(block_size)
{

}

#if 0
/*
 * ISSUE:I guess we don't need it.
 */
Block::Block(const Block &b)
	: Statement(eBlock),
	  stms(b.stms),
	  local_vars(b.local_vars),
	  depth_protect(b.depth_protect)
{
	// Nothing else to do.
}
#endif

/*
 *
 */
Block::~Block(void)
{
	vector<Statement *>::iterator i;
	for(i = stms.begin(); i != stms.end(); ++i) {
		delete (*i);
	}
	stms.clear();

	vector<Statement *>::iterator j;
	for(j = deleted_stms.begin(); j != deleted_stms.end(); ++j) {
		delete (*j);
	}
	deleted_stms.clear();

	local_vars.clear();
	macro_tmp_vars.clear();
}


std::string
Block::create_new_tmp_var(enum eSimpleType type) const
{
	string var_name = gensym("t_");
	macro_tmp_vars[var_name] = type;
	return var_name;
}

void
Block::OutputTmpVariableList(std::ostream &out, int indent) const
{
	std::map<string, enum eSimpleType>::const_iterator i;
	for (i = macro_tmp_vars.begin(); i != macro_tmp_vars.end(); ++i) {
		std::string name = (*i).first;
		enum eSimpleType type = (*i).second;
		output_tab(out, indent);
		Type::get_simple_type(type).Output(out);
		out << " " << name << " = 0;" << std::endl;
	}
}

/*
 *
 */
static void
OutputStatementList(const vector<Statement*> &stms, std::ostream &out, FactMgr* fm, int indent)
{
	size_t i;
	for (i=0; i<stms.size(); i++) {
		const Statement* stm = stms[i];
		stm->pre_output(out, fm, indent);
		stm->Output(out, fm, indent);
		stm->post_output(out, fm, indent);
	}
}

/*
 *
 */
void
Block::Output(std::ostream &out, FactMgr* fm, int indent) const
{
	output_tab(out, indent);
	out << "{ ";
	std::ostringstream ss;
	ss << "block id: " << stm_id;
	output_comment_line(out, ss.str());

	if (CGOptions::depth_protect()) {
		out << "DEPTH++;" << endl;
	}

	indent++;
	if (CGOptions::math_notmp())
		OutputTmpVariableList(out, indent);

	OutputVariableList(local_vars, out, indent);
	OutputStatementList(stms, out, fm, indent);

	if (CGOptions::depth_protect()) {
		out << "DEPTH--;" << endl;
	}
	indent--;

	output_tab(out, indent);
	out << "}";
	outputln(out);
}

/* find the last effective statement for this block, note
 * a return statement terminates the block before reaching the
 * the last statement
 */
const Statement*
Block::get_last_stm(void) const
{
	const Statement* s = 0;
	for (size_t i=0; i<stms.size(); i++) {
		s = stms[i];
		if (s->eType == eReturn) {
			break;
		}
	}
	return s;
}

/*
 * return a random parent block (including itself) or the global block (if 0 is returned)
 */
Block*
Block::random_parent_block(void)
{
	vector<Block*> blks;
	if (CGOptions::global_variables()) {
		blks.push_back(NULL);
	}
	Block* tmp = this;
	while (tmp) {
		blks.push_back(tmp);
		tmp = tmp->parent;
	}
	int index = rnd_upto(blks.size());
	ERROR_GUARD(NULL);
	return blks[index];
}

/*
 * return true if there is no way out of this block other than function return
 */
bool
Block::must_return(void) const
{
	if (stms.size() > 0 && break_stms.size() == 0 && get_last_stm()->must_return()) {
		vector<const CFGEdge*> edges;
		if (find_edges_in(edges, false, true)) {
			// if there is a statement goes back to block (most likely "continue")
			// then this block has a chance to escape the return statement at the end
			for (size_t i=0; i<edges.size(); i++) {
				if (edges[i]->src != this) {
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

/*
 * return true if there is no way out of this block other than jump
 */
bool
Block::must_jump(void) const
{
	if (stms.size() > 0 && break_stms.size() == 0 && get_last_stm()->must_jump()) {
		return true;
	}
	return false;
}

bool
Block::must_break_or_return(void) const
{
	if (stms.size() > 0 && get_last_stm()->must_return()) {
		vector<const CFGEdge*> edges;
		if (find_edges_in(edges, false, true)) {
			// if there is a statement goes back to block (most likely "continue")
			// then this block has a chance to escape the return statement at the end
			for (size_t i=0; i<edges.size(); i++) {
				if (edges[i]->src != this) {
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

/*
 * check if there is a control flow edge from the tail to the head of the block
 */
bool
Block::from_tail_to_head(void) const
{
	if (looping && stms.size() > 0) {
		const Statement* s = get_last_stm();
		//if (s->is_ctrl_stmt() || s->must_return()) {
		if (s->must_jump()) {
			return false;
		}
		return true;
	}
	return false;
}

Statement*
Block::append_return_stmt(CGContext& cg_context)
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	FactVec pre_facts = fm->global_facts;
	cg_context.get_effect_stm().clear();
	Statement* sr = Statement::make_random(cg_context, eReturn);
	ERROR_GUARD(NULL);
	stms.push_back(sr);
	fm->makeup_new_var_facts(pre_facts, fm->global_facts);
	bool visited = sr->visit_facts(fm->global_facts, cg_context);
	assert(visited);

	fm->set_fact_in(sr, pre_facts);
	fm->set_fact_out(sr, fm->global_facts);
	fm->map_accum_effect[sr] = *(cg_context.get_effect_accum());
	fm->map_visited[sr] = true;
	//sr->post_creation_analysis(pre_facts, cg_context);
	fm->map_accum_effect[this] = *(cg_context.get_effect_accum());
	fm->map_stm_effect[this].add_effect(fm->map_stm_effect[sr]);
	return sr;
}

bool
Block::need_nested_loop(const CGContext& cg_context)
{
	size_t i;
	const Statement* s = get_last_stm();
	if (looping && (s == NULL || !s->must_jump()) && cg_context.rw_directive) {
		RWDirective* rwd = cg_context.rw_directive;
		for (i=0; i<rwd->must_read_vars.size(); i++) {
			size_t dimen = rwd->must_read_vars[i]->get_dimension();
			if (dimen > cg_context.iv_bounds.size()) {
				return true;
			} else if (dimen == cg_context.iv_bounds.size() && rnd_flipcoin(10)) {
				return true;
			}
		}
		for (i=0; i<rwd->must_write_vars.size(); i++) {
			size_t dimen = rwd->must_write_vars[i]->get_dimension();
			if (dimen > cg_context.iv_bounds.size()) {
				return true;
			} else if (dimen == cg_context.iv_bounds.size() && rnd_flipcoin(10)) {
				return true;
			}
		}
	}
	return false;
}

Statement*
Block::append_nested_loop(CGContext& cg_context)
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	FactVec pre_facts = fm->global_facts;
	cg_context.get_effect_stm().clear();

	Statement* sf = Statement::make_random(cg_context, eFor);
	ERROR_GUARD(NULL);
	stms.push_back(sf);
	fm->makeup_new_var_facts(pre_facts, fm->global_facts);
	//assert(sf->visit_facts(fm->global_facts, cg_context));

	fm->set_fact_in(sf, pre_facts);
	fm->set_fact_out(sf, fm->global_facts);
	fm->map_accum_effect[sf] = *(cg_context.get_effect_accum());
	fm->map_visited[sf] = true;
	//sf->post_creation_analysis(pre_facts, cg_context);
	fm->map_accum_effect[this] = *(cg_context.get_effect_accum());
	fm->map_stm_effect[this].add_effect(fm->map_stm_effect[sf]);
	return sf;
}

/* return true is var is local variable of this block or parent block,
 * or var is parameter of function
 */
bool
Block::is_var_on_stack(const Variable* var) const
{
    size_t i;
    for (i=0; i<func->param.size(); i++) {
        if (func->param[i]->match(var)) {
            return true;
        }
    }
	const Block* b = this;
	while (b) {
		if (find_variable_in_set(b->local_vars, var) != -1) {
			return true;
		}
		b = b->parent;
	}
    return false;
}

std::vector<const ExpressionVariable*>
Block::get_dereferenced_ptrs(void) const
{
	// return a empty vector by default
	std::vector<const ExpressionVariable*> empty;
	return empty;
}

bool
Block::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	int dummy;
	FactMgr* fm = get_fact_mgr(&cg_context);
	vector<const Fact*> dummy_facts;
	Effect pre_effect = cg_context.get_accum_effect();
	if (!find_fixed_point(inputs, dummy_facts, cg_context, dummy, false)) {
		cg_context.reset_effect_accum(pre_effect);
		return false;
	}
	inputs = fm->map_facts_out[this];
	fm->map_visited[this] = true;
	return true;
}

/*
 * return true if there are back edges leading to statement
 * inside this block (but not in sub-blocks)
 */
bool
Block::contains_back_edge(void) const
{
	if (func != 0) {
		FactMgr* fm = get_fact_mgr_for_func(func);
		size_t i;
		for (i=0; i<fm->cfg_edges.size(); i++) {
			const CFGEdge* edge = fm->cfg_edges[i];
			if (edge->back_link && edge->dest->parent == this) {
				return true;
			}
		}
	}
	return false;
}


/**************************************************************************************************
 * DFA analysis for a block:
 *
 * we must considers all kinds of blocks: block for for-loops; block for if-true and if-false; block for
 * function body; block that loops; block has jump destination insdie; block being a jump destination itself
 * (in the case of "continue" in for-loops). All of them must be taken care in this function.
 *
 * params:
 *    inputs: the inputs env before entering block
 *    cg_context: code generation context
 *    fail_index: records which statement in this block caused analyzer to fail
 *    visit_one: when is true, the statements in this block must be visited at least once
 ****************************************************************************************************/
bool
Block::find_fixed_point(vector<const Fact*> inputs, vector<const Fact*>& post_facts, CGContext& cg_context, int& fail_index, bool visit_once) const
{
	FactMgr* fm = get_fact_mgr(&cg_context);
	// include outputs from all back edges leading to this block
	size_t i;
	static int g = 0;
	vector<const CFGEdge*> edges;
	int cnt = 0;
	do {
		// if we have never visited the block, force the visitor to go through all statements at least once
		if (fm->map_visited[this]) {
			if (cnt++ > 7) {
				// takes too many iterations to reach a fixed point, must be something wrong
				assert(0);
			}
			find_edges_in(edges, false, true);
			for (i=0; i<edges.size(); i++) {
				const Statement* src = edges[i]->src;
				//assert(fm->map_visited[src]);
				merge_facts(inputs, fm->map_facts_out[src]);
			}
		}
		if (!visit_once) {
			int shortcut = shortcut_analysis(inputs, cg_context);
			if (shortcut == 0) return true;
		}
		//if (shortcut == 1) return false;

		FactVec outputs = inputs;
		// add facts for locals
		for (i=0; i<local_vars.size(); i++) {
			const Variable* v = local_vars[i];
			FactMgr::add_new_var_fact(v, outputs);
		}

		// revisit statements with new inputs
		for (i=0; i<stms.size(); i++) {
			int h = g++;
			if (h == 2585)
				BREAK_NOP;		// for debugging
			if (!stms[i]->analyze_with_edges_in(outputs, cg_context)) {
				fail_index = i;
				return false;
			}
		}
		fm->set_fact_in(this, inputs);
		post_facts = outputs;
		FactMgr::update_facts_for_oos_vars(local_vars, outputs);
		fm->set_fact_out(this, outputs);
		fm->map_visited[this] = true;
		// compute accumulated effect
		set_accumulated_effect(cg_context);
		visit_once = false;
	} while (true);
	return true;
}

void
Block::set_accumulated_effect(CGContext& cg_context) const
{
	Effect eff;
	FactMgr* fm = get_fact_mgr(&cg_context);
	for (size_t i=0; i<stms.size(); i++) {
		Statement* s = stms[i];
		eff.add_effect(fm->map_stm_effect[s]);
	}
	//cg_context.get_effect_stm() = eff;
	fm->map_stm_effect[this] = eff;
}

/*
 * remove a statement from this block. this may trigger other events: deleting it from
 * break_stms, deleting CFG edges linked to it, etc
 */
size_t
Block::remove_stmt(const Statement* s)
{
	int i, len, cnt;
	cnt = 0;
	assert(func);
	FactMgr* fm = get_fact_mgr_for_func(func);
	vector<const Statement*> cfg_stms;
	vector<int> types;
	types.push_back(eContinue);
	types.push_back(eBreak);
	types.push_back(eGoto);
	//if (func->name == "func_109")
	//	s->Output(cout, fm);
	if (s->find_typed_stmts(cfg_stms, types)) {
		// remove from the break_stms list if it is or contains a break
		Block* b;
		for (b = this; b && !b->looping; b = b->parent) {
			/* Empty. */
		}
		if (b != 0) {
			len = b->break_stms.size();
			for (i=0; i<len; i++) {
				if (find_stm_in_set(cfg_stms, b->break_stms[i]) >= 0) {
					b->break_stms.erase(b->break_stms.begin() + i);
					i--;
					len--;
				}
			}
		}
		// remove any CFG edges that has s (or flow-control statements inside s) as src
		len = fm->cfg_edges.size();
		for (i=0; i<len; i++) {
			const CFGEdge* edge = fm->cfg_edges[i];
			if (find_stm_in_set(cfg_stms, edge->src) >= 0) {
				fm->cfg_edges.erase(fm->cfg_edges.begin() + i);
				delete edge;
				i--;
				len--;
			}
		}
	}

	// remove any CFG edges that has s (or statements inside s) as dest
	len = fm->cfg_edges.size();
	for (i=0; i<len; i++) {
		const CFGEdge* edge = fm->cfg_edges[i];
		const Statement* src = edge->src;
		if (s->contains_stmt(edge->dest)) {
			fm->cfg_edges.erase(fm->cfg_edges.begin() + i);
			delete edge;
			i--;
			len--;
			// delete the source statement (most likely goto) as well
			if (src->eType == eGoto) {
				int deleted = src->parent->remove_stmt(src);
				if (src->parent == this) {
					cnt += deleted;
				}
				if ((int)fm->cfg_edges.size() != len) {
					// re-iterate all edges from the beginning
					i = -1;
					len = fm->cfg_edges.size();
				}
			}
		}
	}

	// delete all the blocks inside s
	len = func->blocks.size();
	for (i=0; i<len; i++) {
		Block* b = func->blocks[i];
		if (s->contains_stmt(b)) {
			func->blocks.erase(func->blocks.begin() + i);
			i--;
			len--;
		}
	}
	// delete the statment itself
	for (i=0; i<(int)stms.size(); i++) {
		if (stms[i] == s) {
			deleted_stms.push_back(stms[i]);
			stms.erase(stms.begin() + i);
			cnt++;
			break;
		}
	}
	return cnt;
}

/**********************************************************************
 * once generated the loop body, verify whether some statement caused
 * the analyzer to fail during the 2nd iteration of the loop body (in
 * most case, a null/dead pointer dereference would do it), if so, delete
 * the statement in which analyzer fails and all subsequent statemets
 *
 * also performs effect analysis
 *********************************************************************/
void
Block::post_creation_analysis(CGContext& cg_context, const Effect& pre_effect)
{
	int index;
	FactMgr* fm = get_fact_mgr(&cg_context);
	fm->map_visited[this] = true;
	// compute accumulated effect
	set_accumulated_effect(cg_context);
	//fm->print_facts(fm->global_facts);
	vector<const Fact*> post_facts = fm->global_facts;
	FactMgr::update_facts_for_oos_vars(local_vars, fm->global_facts);
	fm->remove_rv_facts(fm->global_facts);
	fm->set_fact_out(this, fm->global_facts);

	// find out if fixed-point-searching is required
	bool is_loop_body = !must_break_or_return() && looping;
	bool self_back_edge = false;
	if (is_loop_body || need_revisit || has_edge_in(false, true)) {
		if (is_loop_body && from_tail_to_head()) {
			self_back_edge = true;
			fm->create_cfg_edge(this, this, false, true);
		}
		vector<const Fact*> facts_copy = fm->map_facts_in[this];
		// reset the accumulative effect
		cg_context.reset_effect_accum(pre_effect);
		while (!find_fixed_point(facts_copy, post_facts, cg_context, index, need_revisit)) {
			size_t i, len;
			len = stms.size();
			for (i=index; i<len; i++) {
				remove_stmt(stms[i]);
				i = index-1;
				len = stms.size();
			}
			// if we delete some statements, next visit must go through statements (no shortcut)
			need_revisit = true;
			// clean up in/out map from previous analysis that might include facts caused by deleted statements
			fm->reset_stm_fact_maps(this);
			// sometimes a loop would emerge after we delete the "return" statement in body
			if (!self_back_edge && from_tail_to_head()) {
				self_back_edge = true;
				fm->create_cfg_edge(this, this, false, true);
			}
			// reset incoming effects
			cg_context.reset_effect_accum(pre_effect);
		}
		fm->global_facts = fm->map_facts_out[this];
	}
	// make sure we add back return statement for blocks that require it and had such statement deleted
	// only do this for top-level block of a function which requires a return statement
	if (parent == 0 && func->need_return_stmt() && !must_return()) {
		fm->global_facts = post_facts;
		Statement* sr = append_return_stmt(cg_context);
		fm->set_fact_out(this, fm->map_facts_out[sr]);
	}
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

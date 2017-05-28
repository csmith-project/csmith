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

#include "Statement.h"
#include <cassert>
#include <map>
#include <iostream>
#include "Common.h"

#include "CGContext.h"
#include "CGOptions.h"
#include "Function.h"
#include "Expression.h"
#include "ExpressionFuncall.h"
#include "FunctionInvocation.h"
#include "FunctionInvocationUser.h"
#include "FactPointTo.h"

#include "Block.h" // temporary; don't want to depend on subclases!
#include "StatementAssign.h" // temporary; don't want to depend on subclases!
#include "StatementExpr.h" // temporary; don't want to depend on subclases!
#include "StatementFor.h" // temporary; don't want to depend on subclases!
#include "StatementIf.h" // temporary; don't want to depend on subclases!
#include "StatementReturn.h" // temporary; don't want to depend on subclases!
#include "StatementBreak.h"
#include "StatementContinue.h"
#include "StatementGoto.h"
#include "StatementArrayOp.h"
#include "Variable.h"
#include "ArrayVariable.h"
#include "VectorFilter.h"
#include "ProbabilityTable.h"
#include "PartialExpander.h"
#include "random.h"
#include "Fact.h"
#include "FactMgr.h"
#include "CFGEdge.h"
#include "Error.h"
#include "DepthSpec.h"
#include "OutputMgr.h"
#include "util.h"
#include "StringUtils.h"
#include "VariableSelector.h"

using namespace std;
const Statement* Statement::failed_stm;

///////////////////////////////////////////////////////////////////////////////
class StatementFilter : public Filter
{
public:
	explicit StatementFilter(const CGContext &cg_context);

	virtual ~StatementFilter(void);

	virtual bool filter(int v) const;
private:
	const CGContext &cg_context_;
};

StatementFilter::StatementFilter(const CGContext &cg_context)
	: cg_context_(cg_context)
{

}

StatementFilter::~StatementFilter(void)
{

}

// use a table to define probabilities of different kinds of statements
// Must initialize it before use
ProbabilityTable<unsigned int, ProbName> *Statement::stmtTable_ = NULL;

void
Statement::InitProbabilityTable()
{
	if (Statement::stmtTable_)
		return;

	Statement::stmtTable_ = new ProbabilityTable<unsigned int, ProbName>();
	Statement::stmtTable_->initialize(pStatementProb);
}

eStatementType
Statement::number_to_type(unsigned int value)
{
	assert(Statement::stmtTable_);
	assert(value < 100);
	ProbName pname = Statement::stmtTable_->get_value(value);
	eStatementType type = static_cast<eStatementType>(Probabilities::pname_to_type(pname));
	return type;
}

bool StatementFilter::filter(int value) const
{
	assert(value != -1);

	if (!this->valid_filter())
		return false;

	eStatementType type = Statement::number_to_type(value);

	// If expand_check returns false, we filter out v.
	if (!PartialExpander::expand_check(type))
		return true;

	const Type* return_type = cg_context_.get_current_func()->return_type;
	bool no_return = (return_type->eType == eSimple && return_type->simple_type == eVoid);

	if (type == eBlock) {
		return true;
	}
	if ((type == eReturn) && no_return) {
		return true;
	}

	if ( (type == eBreak || type == eContinue) && !(cg_context_.flags & IN_LOOP) ) {
		return true;
	}

	// Limit Function complexity (depth of nested control structures)
	if (cg_context_.blk_depth >= CGOptions::max_blk_depth()) {
		return Statement::is_compound(type);
	}
	else if (Function::reach_max_functions_cnt()) { // Limit # of functions..
		if (type != eInvoke)
			return false;
		else
			return true;
	}
	else {
		return false;
	}

	return false;
}

int find_stm_in_set(const vector<const Statement*>& set, const Statement* s)
{
    size_t i;
    for (i=0; i<set.size(); i++) {
        if (set[i] == s) {
            return i;
        }
    }
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
//
// TODO: Build probability tables for each decision point
//       IE: Return statements less common than assignment statements, etc.
//       Allow MIN, MAX and pluggable density functions (uniform, normal, guassian, ...)
//
// Like this:
/*
unsigned int probabilityStatement[eStatementType] =
{
	100, 	//	eAssign,
	75, 	//	eIfElse,
	30,		//	eFor,		// Make this a generic loop construct (while/for/do)
	30,		//	eInvoke,
	10,		//	eReturn,
}

probability probStatement
{
	2, // Min
	0, // Max
	probabilityStatement // Pluggable Function (custom probability table, in this case)
}
*/

static eStatementType
StatementProbability(const StatementFilter *filter)
{
	int value = rnd_upto(100, filter);
	ERROR_GUARD(MAX_STATEMENT_TYPE);
	assert(value != -1);
	assert(value >= 0 && value < 100);
	return Statement::number_to_type(value);
}

int Statement::sid = 0;
/*
 *
 */
Statement *
Statement::make_random(CGContext &cg_context,
					   eStatementType t)
{
	DEPTH_GUARD_BY_TYPE_RETURN_WITH_FLAG(dtStatement, t, NULL);
	// Should initialize table first
	Statement::InitProbabilityTable();

	if ((CGOptions::stop_by_stmt() >= 0) && (sid >= CGOptions::stop_by_stmt())) {
		t = eReturn;
	}

	// Add more statements:
	// for
	// while
	// library call (malloc, free, str*, mem*, etc..)?
	// switch?
	// ..?
	if (t == MAX_STATEMENT_TYPE) {
		StatementFilter filter(cg_context);
		t = StatementProbability(&filter);
		ERROR_GUARD(NULL);
	}
	FactMgr* fm = get_fact_mgr(&cg_context);
	FactVec pre_facts = fm->global_facts;
	Effect pre_effect = cg_context.get_accum_effect();
	cg_context.get_effect_stm().clear();
	cg_context.expr_depth = 0;
	if (is_compound(t)) {
		cg_context.blk_depth++;
	}
	// XXX: interim ickiness
	Statement *s = 0;

	switch (t) {
	default:
		assert(!"unknown Statement type");
		break;
	case eAssign:
		s = StatementAssign::make_random(cg_context);
		break;
	case eBlock:
		s = Block::make_random(cg_context);
		break;
	case eFor:
		s = StatementFor::make_random(cg_context);
		break;
	case eIfElse:
		s = StatementIf::make_random(cg_context);
		break;
	case eInvoke:
		s = StatementExpr::make_random(cg_context);
		break;
	case eReturn:
		s = StatementReturn::make_random(cg_context);
		break;
	case eBreak:
		s = StatementBreak::make_random(cg_context);
		break;
	case eContinue:
		s = StatementContinue::make_random(cg_context);
		break;
	case eGoto:
		s = StatementGoto::make_random(cg_context);
		break;
	case eArrayOp:
		s = StatementArrayOp::make_random(cg_context);
		break;
	}

	ERROR_GUARD(NULL);
	if (is_compound(t)) {
		cg_context.blk_depth--;
	}
	// sometimes make_random may return 0 for various reasons. keep generating
	if (s == 0) {
		return make_random(cg_context);
	}
	s->func = cg_context.get_current_func();
	s->parent = cg_context.get_current_block();
	s->post_creation_analysis(pre_facts, pre_effect, cg_context);
	return s;
}

std::vector<const ExpressionVariable*>
Statement::get_dereferenced_ptrs(void) const
{
	// return a empty vector by default
	std::vector<const ExpressionVariable*> empty;
	return empty;
}

void
Statement::get_referenced_ptrs(std::vector<const Variable*>& ptrs) const
{
	size_t i;
	vector<const Expression*> exprs;
	vector<const Block*> blks;
	get_exprs(exprs);
	get_blocks(blks);
	for (i=0; i<exprs.size(); i++) {
		exprs[i]->get_referenced_ptrs(ptrs);
	}
	for (i=0; i<blks.size(); i++) {
		for (size_t j=0; j<blks[i]->stms.size(); j++) {
			const Statement* s = blks[i]->stms[j];
			s->get_referenced_ptrs(ptrs);
		}
	}
}

int
Statement::get_blk_depth(void) const
{
	int depth = 0;
	for (const Block* b = parent; b != NULL; b = b->parent) {
		depth++;
	}
	return depth;
}

bool
Statement::is_ptr_used(void) const
{
	vector<const Variable*> ptrs;
	get_referenced_ptrs(ptrs);
	return !ptrs.empty();
}

/*
 *
 */
Statement::Statement(eStatementType st, Block* b)
	: eType(st),
	func(b ? b->func : 0),
	parent(b)
{
	stm_id = Statement::sid;
	Statement::sid++;
}

/*
 *
 */
Statement::~Statement(void)
{
	// Nothing to do.
}

/*
 * return true if statement is contained in block b
 */
bool
Statement::in_block(const Block* b) const
{
	const Block* tmp = parent;
	while (tmp) {
		if (tmp == b) return true;
		tmp = tmp->parent;
	}
	return false;
}

/*
 * return true if this statement dominates s
 */
bool
Statement::dominate(const Statement* s) const
{
	if (s->parent == this) {
		return true;
	}
	if (parent == s->parent) {
		return stm_id <= s->stm_id;
	}
	const Statement* container = s->find_container_stm();
	if (container == 0) {
		container = s->parent;
	}
	if (container != 0) {
		return dominate(container);
	}
	return false;
}

/*
 * find the parent for-statement or while-statement (to be implemented)
 * that contains this statement or block
 */
const Statement*
Statement::find_container_stm(void) const
{
	const Block* b = (eType == eBlock) ? (const Block*)this : parent;
	if (b != 0 && b->parent != 0) {
		for (size_t i=0; i<b->parent->stms.size(); i++) {
			const Statement* s = b->parent->stms[i];
			vector<const Block*> blks;
			s->get_blocks(blks);
			if (std::find(blks.begin(), blks.end(), b) != blks.end()) {
				return s;
			}
		}
	}
	return 0;
}

/*
 * return true if there is CFG edge(s) leading to this statement matching given attributes
 */
bool
Statement::has_edge_in(bool post_dest, bool back_link) const
{
	if (func != 0) {
		FactMgr* fm = get_fact_mgr_for_func(func);
		assert(fm);
		size_t i;
		for (i=0; i<fm->cfg_edges.size(); i++) {
			const CFGEdge* e = fm->cfg_edges[i];
			if (e->dest == this && e->back_link == back_link && e->post_dest == post_dest) {
				return true;
			}
		}
	}
	return false;
}

/*
 * find CFG edges leading to this statement, found edges are stored in pass-in param "edges"
 * return the number of edges found
 */
int
Statement::find_edges_in(vector<const CFGEdge*>& edges, bool post_dest, bool back_link) const
{
	edges.clear();
	if (func != 0) {
		FactMgr* fm = get_fact_mgr_for_func(func);
		assert(fm);
		size_t i;
		for (i=0; i<fm->cfg_edges.size(); i++) {
			const CFGEdge* e = fm->cfg_edges[i];
			if (e->dest == this && e->back_link == back_link && e->post_dest == post_dest) {
				edges.push_back(e);
			}
		}
	}
	return edges.size();
}

/*
 * return the label if this statement is the destination of a "goto" statement
 */
std::string
Statement::find_jump_label(void) const
{
	if (func != 0) {
		FactMgr* fm = get_fact_mgr_for_func(func);
		assert(fm);
		size_t i;
		for (i=0; i<fm->cfg_edges.size(); i++) {
			const CFGEdge* e = fm->cfg_edges[i];
			if (e->dest == this && e->src->eType == eGoto) {
				const StatementGoto* sg = dynamic_cast<const StatementGoto*>(e->src);
				assert(sg);
				return sg->label;
			}
		}
	}
	return "";
}

/*
 * find all "goto" statements that jumps to this statement
 */
int
Statement::find_jump_sources(std::vector<const StatementGoto*>& gotos) const
{
	if (func != 0) {
		FactMgr* fm = get_fact_mgr_for_func(func);
		assert(fm);
		size_t i;
		gotos.clear();
		for (i=0; i<fm->cfg_edges.size(); i++) {
			const CFGEdge* e = fm->cfg_edges[i];
			if (e->dest == this && e->src->eType == eGoto) {
				const StatementGoto* sg = dynamic_cast<const StatementGoto*>(e->src);
				assert(sg);
				gotos.push_back(sg);
			}
		}
	}
	return gotos.size();
}

/*
 * a helper function for StatementFor and StatementIf
 * include effect caused by block, and update the effect map for this statement in fact manager
 */
void
Statement::set_accumulated_effect_after_block(Effect& eff, const Block* b, CGContext& cg_context) const
{
	FactMgr* fm = get_fact_mgr(&cg_context);
	eff.add_effect(fm->map_stm_effect[b]);
	fm->map_stm_effect[this] = eff;
}

/*
 * add back return_facts for skipped statement (see validate_and_update_facts)
 */
void
Statement::add_back_return_facts(FactMgr* fm, std::vector<const Fact*>& facts) const
{
	if (eType == eReturn) {
		merge_facts(facts, fm->map_facts_out[this]);
	} else {
		vector<const Block*> blks;
		get_blocks(blks);
		for (size_t i=0; i<blks.size(); i++) {
			for (size_t j=0; j<blks[i]->stms.size(); j++) {
				blks[i]->stms[j]->add_back_return_facts(fm, facts);
			}
		}
	}
}

/* return code:
 *    0 means we successfully take a shortcut
 *    1 means the shortcut fails due to effect conflict
 *    2 means there is no shortcut
 */
int
Statement::shortcut_analysis(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	// the output facts of control statement (break/continue/goto) has removed local facts
	// thus can not take this shortcut. (The facts we get should represent all variables
	// visible in subsequent statement)
	if (same_facts(inputs, fm->map_facts_in[this]) && !is_ctrl_stmt() && !contains_unfixed_goto())
	{
		//cg_context.get_effect_context().Output(cout);
		//print_facts(inputs);
		//fm->map_stm_effect[this].Output(cout);
		if (cg_context.in_conflict(fm->map_stm_effect[this])) {
			return 1;
		}
		inputs = fm->map_facts_out[this];
		cg_context.add_effect(fm->map_stm_effect[this]);
		fm->map_accum_effect[this] = *(cg_context.get_effect_accum());
		return 0;
	}
	return 2;
}

/***************************************************************************************
 * for a given input env, abstract a given statement, generate an output env, and
 * update both input/output env for this statement
 *
 * shortcut: if this input env matches previous input env, use previous output env directly
 ***************************************************************************************/
bool
Statement::validate_and_update_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	int shortcut = shortcut_analysis(inputs, cg_context);
	if (shortcut==0) {
		/* mark the goto statements inside this statement as visited
		   this is to fix scenario like the following:

		   lbl:  s1
		   for (...) {
		      goto lbl;
		   }

		   where the "for" statement is bypassed, but the output from "goto lbl"
		   must be feed into s1 in order to achieve a fixed point */
		for (size_t i=0; i<fm->cfg_edges.size(); i++) {
			const Statement* s = fm->cfg_edges[i]->src;
			if (s->eType == eGoto && contains_stmt(s)) {
				fm->map_visited[s] = true;
			}
		}
		return true;
	}
	if (shortcut==1) return false;

	vector<const Fact*> inputs_copy = inputs;
	if (!stm_visit_facts(inputs, cg_context)) {
		return false;
	}
	fm->set_fact_in(this, inputs_copy);
	fm->set_fact_out(this, inputs);
	return true;
}

bool
Statement::stm_visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	cg_context.get_effect_stm().clear();
	cg_context.curr_blk = parent;
	FactMgr* fm = get_fact_mgr(&cg_context);
	//static int g = 0;
	//int h = g++;
	bool ok = visit_facts(inputs, cg_context);


	if (!ok && !is_compound(eType)) {
		failed_stm = this;
	}
	//if (!FactPointTo::is_valid_ptr("g_75", inputs))
	//	Output(cout, fm);
	fm->remove_rv_facts(inputs);
	fm->map_accum_effect[this] = *(cg_context.get_effect_accum());
	fm->map_visited[this] = true;
	return ok;
}

/*
 * find all the control flow manipulate statements, i.e. break/continue/goto
 * (maybe return?) contained in this statement recursively
 */
int
Statement::find_typed_stmts(vector<const Statement*>& stms, const vector<int>& stmt_types) const
{
	if (std::find(stmt_types.begin(), stmt_types.end(), eType) != stmt_types.end()) {
		stms.push_back(this);
	}
	vector<const Block*> blks;
	get_blocks(blks);
	for (size_t i=0; i<blks.size(); i++) {
		for (size_t j=0; j<blks[i]->stms.size(); j++) {
		const Statement* s  = blks[i]->stms[j];
			s->find_typed_stmts(stms, stmt_types);
		}
	}
	return stms.size();
}

bool
Statement::is_1st_stm(void) const
{
	return parent && parent->stms.size() && parent->stms[0] == this;
}

bool
Statement::is_jump_target_from_other_blocks(void) const
{
	vector<const StatementGoto*> gotos;
	if (find_jump_sources(gotos)) {
		size_t i;
		for (i=0; i<gotos.size(); i++) {
			if (gotos[i]->parent != this->parent) {
				return true;
			}
		}
	}
	return false;
}

bool
Statement::read_union_field(void) const
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	assert(fm);
	if (fm->map_stm_effect[this].union_field_is_read()) {
		return true;
	}
	vector<const FunctionInvocationUser*> calls;
	get_called_funcs(calls);
	for (size_t i=0; i<calls.size(); i++) {
		if (calls[i]->get_func()->union_field_read) {
			return true;
		}
	}
	return false;
}

/*
 * return true if s is contained inside this statement
 */
bool
Statement::contains_stmt(const Statement* s) const
{
	if (this == s) {
		return true;
	}
	if (eType == eBlock) {
		for (const Block* tmp = s->parent; tmp; tmp = tmp->parent) {
			if (tmp == (const Block*)this) {
				return true;
			}
		}
		return false;
	}
	vector<const Block*> blks;
	get_blocks(blks);
	for (size_t i=0; i<blks.size(); i++) {
		if (blks[i]->contains_stmt(s)) {
			return true;
		}
	}
	return false;
}

int
Statement::find_contained_labels(vector<string>& labels) const
{
	string label = find_jump_label();
	if (label != "") {
		labels.push_back(label);
	}

	vector<const Block*> blks;
	get_blocks(blks);
	size_t i, j;
	for (i=0; i<blks.size(); i++) {
		for (j=0; j<blks[i]->stms.size(); j++) {
			const Statement* s = blks[i]->stms[j];
			s->find_contained_labels(labels);
		}
	}
	return labels.size();
}

/*
 * find all the functions directly called in this statement
 */
const FunctionInvocation*
Statement::get_direct_invocation(void) const
{
	if (eType == eAssign) {
		const Expression* e = ((const StatementAssign*)this)->get_expr();
		if (e->term_type == eFunction) {
			return ((const ExpressionFuncall*)e)->get_invoke();
		}
	}
	else if (eType == eInvoke) {
		return ((const StatementExpr*)this)->get_invoke();
	}
	else if (eType == eIfElse) {
		const StatementIf* si = (const StatementIf*)this;
		const Expression* e = si->get_test();
		if (e->term_type == eFunction) {
			return ((const ExpressionFuncall*)e)->get_invoke();
		}
	}
	return NULL;
}

/*
 * find all the function calls in this statement
 */
void
Statement::get_called_funcs(std::vector<const FunctionInvocationUser*>& funcs) const
{
	size_t i;
	vector<const Expression*> exprs;
	vector<const Block*> blks;
	get_exprs(exprs);
	get_blocks(blks);
	for (i=0; i<exprs.size(); i++) {
		exprs[i]->get_called_funcs(funcs);
	}
	for (i=0; i<blks.size(); i++) {
		for (size_t j=0; j<blks[i]->stms.size(); j++) {
			const Statement* s = blks[i]->stms[j];
			s->get_called_funcs(funcs);
		}
	}
}

/*
 * return true if there are some goto statement contained in this statement
 * that hasn't reached a fixed point
 */
bool
Statement::contains_unfixed_goto(void) const
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	assert(fm);
	size_t i, j;
	for (i=0; i<fm->cfg_edges.size(); i++) {
		const CFGEdge* edge = fm->cfg_edges[i];
		/* the following for-loop has to be analyzed at least once
		   label: ...
		   ...
		   for (...) {
		      goto label;
		   }
		*/
		if (edge->src->eType == eGoto && contains_stmt(edge->src) && !fm->map_visited[edge->src] && !contains_stmt(edge->dest)) {
			return true;
		}
		if (edge->src->eType == eGoto && fm->map_visited[edge->src] && contains_stmt(edge->dest)) {
			// take care the special case caused by StatementGoto::visit_facts
			if (!fm->map_facts_out[edge->src].empty() && fm->map_facts_in[edge->dest].empty()) {
				return true;
			}
			for (j=0; j<fm->map_facts_in[edge->dest].size(); j++) {
				const Fact* f = fm->map_facts_in[edge->dest][j];
				// ignore return variable facts
				if (!f->get_var()->is_rv()) {
					const Fact* jump_src_f = find_related_fact(fm->map_facts_out[edge->src], f);
					if (jump_src_f && !f->imply(*jump_src_f)) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool
Statement::analyze_with_edges_in(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	FactMgr* fm = get_fact_mgr(&cg_context);
	size_t i;
	vector<const CFGEdge*> edges;
	if (find_jump_label()=="lbl_101")
		i = 0;
	// consider output from back edges. we should not merge them if this is the first time
	if (fm->map_visited[this] && has_edge_in(false, true)) {
		find_edges_in(edges, false, true);
		for (i=0; i<edges.size(); i++) {
			const Statement* src = edges[i]->src;
			if (fm->map_visited[src]) {
				FactMgr::merge_jump_facts(inputs, fm->map_facts_out[src]);
				cg_context.add_effect(fm->map_accum_effect[src]);
			}
		}
	}
	// always consider output from forward edges
	if (has_edge_in(false, false)) {
		find_edges_in(edges, false, false);
		for (i=0; i<edges.size(); i++) {
			const Statement* src = edges[i]->src;
			if (fm->map_visited[src]) {
				FactMgr::merge_jump_facts(inputs, fm->map_facts_out[src]);
				cg_context.add_effect(fm->map_accum_effect[src]);
			}
		}
	}
	return validate_and_update_facts(inputs, cg_context);
}

/****************************************************************************
 * Entry point to pointer analysis and other DFA analysis for newly
 * created statement. remember some analysis has already been done during the
 * statement generation, some analysis work is only possible with a complete
 * statement, we do it here
 ****************************************************************************/
void
Statement::post_creation_analysis(vector<const Fact*>& pre_facts, const Effect& pre_effect, CGContext& cg_context) const
{
	FactMgr* fm = get_fact_mgr_for_func(func);
	if (eType == eIfElse) {
		((const StatementIf*)this)->combine_branch_facts(pre_facts);
	} else {
		fm->makeup_new_var_facts(pre_facts, fm->global_facts);
	}

	// save the effect for this statement if this is a simple statement
	// for compound statements, it's effect is saved in make_random
	if (!is_compound(eType)) {
		fm->map_stm_effect[this] = cg_context.get_effect_stm();
	}

	bool special_handled = false;
	// special handling for non-looping statements in func_1, which we never re-visit to
	// save run-time
	if (cg_context.get_current_func()->name == "func_1" && !(cg_context.flags & IN_LOOP) ) {
		if (has_uncertain_call_recursive()) {
			FactVec outputs = pre_facts;
			cg_context.reset_effect_accum(pre_effect);
			//if (stm_id == 573)
				/*if (this->eType == eAssign) {
					((const StatementAssign*)this)->get_rhs()->indented_output(cout, 0);
				}
				cout << endl;
				Output(cout, fm);*/
			//}
			if (!validate_and_update_facts(outputs, cg_context)) {
				assert(0);
			}
			fm->global_facts = outputs;
			special_handled = true;
		}
	}

	if (!special_handled) {
		// for if...else..., we don't want to walk through the true branch and false branch again
		// compute the output with consideration of return statement(s) in both branches
		if (eType == eAssign) {
			const StatementAssign* sa = (const StatementAssign*)this;
			// abstract fact for assignment itself. No analysis on function calls
			// on RHS since they are already handled during statement generation
			FactMgr::update_fact_for_assign(sa, fm->global_facts);
		}
		else if (eType == eReturn) {
			const StatementReturn* sr = (const StatementReturn*)this;
			FactMgr::update_fact_for_return(sr, fm->global_facts);
		}
	}
	fm->remove_rv_facts(fm->global_facts);
	fm->set_fact_in(this, pre_facts);
	fm->set_fact_out(this, fm->global_facts);
	fm->map_accum_effect[this] = *(cg_context.get_effect_accum());
	fm->map_visited[this] = true;
}

/*
 * return: 1 means this is a goto target, 0 otherwise
 */
int
Statement::pre_output(std::ostream &out, FactMgr* /* fm */, int indent) const
{
	// output label if this is a goto target
	vector<const StatementGoto*> gotos;
	if (find_jump_sources(gotos)) {
		assert(gotos.size() > 0);
		out << gotos[0]->label << ":" << endl;
		return 1;
		//for (j=0; j<gotos.size(); j++) {
		//	gotos[j]->output_skipped_var_inits(out, indent);
		//}
	}
	// compute checksum and output, for Yang's delta
	output_hash(out, indent);
	return 0;
}

void
Statement::post_output(std::ostream &out, FactMgr* fm, int indent) const
{
	// don't print facts after block because it would mess up "if ... else ..."
	if (fm && CGOptions::paranoid() && !CGOptions::concise() && eType != eBlock) {
		fm->output_assertions(out, this, indent, true);
	}
}

void
Statement::output_hash(std::ostream &out, int indent) const
{
	// compute checksum and print out the value
	if (CGOptions::step_hash_by_stmt()) {
		OutputMgr::OutputStepHashFuncInvocation(out, indent, stm_id);
	}
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cassert>

#include "CGContext.h"
#include "CGOptions.h"
#include "Effect.h"
#include "Variable.h"
#include "Function.h"
#include "Block.h"
#include "Type.h"
#include "ExpressionVariable.h"
#include "Fact.h"
#include "FactMgr.h"
#include "FactPointTo.h"
#include "FactUnion.h"
#include "Lhs.h"
#include "Statement.h"
#include "Bookkeeper.h"
#include "ArrayVariable.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 * Declare "the" empty set of variables.
 */
const VariableSet CGContext::empty_variable_set(0);
const CGContext CGContext::empty_context(0, Effect::get_empty_effect(), 0);

///////////////////////////////////////////////////////////////////////////////
/*
 * A convenience constructor. Context is created when entering functions
 */
CGContext::CGContext(Function *current_func, const Effect &eff_context, Effect *eff_accum)
	: current_func(current_func),
	  blk_depth(0),
	  expr_depth(0),
	  flags(0),
	  curr_blk(0),
	  rw_directive(NULL),
	  iv_bounds(),
	  curr_rhs(NULL),
	  effect_context(eff_context),
	  effect_accum(eff_accum)
{
	// Nothing else to do.
}

// adapt current context to create context of parameters
CGContext::CGContext(const CGContext &cgc, const Effect &eff_context, Effect *eff_accum)
	: current_func(cgc.current_func),
	  blk_depth(cgc.blk_depth),
	  expr_depth(cgc.expr_depth),
	  flags(cgc.flags),
	  call_chain(cgc.call_chain),
	  curr_blk(cgc.curr_blk),
	  rw_directive(cgc.rw_directive),
	  iv_bounds(cgc.iv_bounds),
	  curr_rhs(NULL),
	  effect_context(eff_context),
	  effect_accum(eff_accum),
	  effect_stm()
{
	// Nothing else to do.
}

// adapt current context to create context of callee
CGContext::CGContext(const CGContext &cgc, Function* f, const Effect &eff_context, Effect *eff_accum)
	: current_func(f),
	  blk_depth(0),
	  expr_depth(0),
	  flags(cgc.flags),
	  call_chain(cgc.call_chain),
	  curr_blk(cgc.curr_blk),
	  rw_directive(cgc.rw_directive),
	  iv_bounds(cgc.iv_bounds),
	  curr_rhs(NULL),
	  effect_context(eff_context),
	  effect_accum(eff_accum)
{
	extend_call_chain(cgc);
}

// adapt current context to create context of loop body
CGContext::CGContext(const CGContext &cgc, RWDirective* rwd, const Variable* iv, unsigned int bound)
	: current_func(cgc.current_func),
	  blk_depth(cgc.blk_depth),
	  expr_depth(0),
	  flags(cgc.flags | IN_LOOP),
	  call_chain(cgc.call_chain),
	  curr_blk(cgc.curr_blk),
	  rw_directive(rwd),
	  iv_bounds(cgc.iv_bounds),
	  curr_rhs(NULL),
	  effect_context(cgc.effect_context),
	  effect_accum(cgc.effect_accum)
{
	// add loop induction variable
	if (iv) {
		iv_bounds[iv] = bound;
	}
}

/*
 *
 */
CGContext::~CGContext(void)
{
	// Nothing else to do.
}

/*
 *
 */
bool
CGContext::is_nonreadable(const Variable *v) const
{
	if (rw_directive) {
		VariableSet::size_type len = rw_directive->no_read_vars.size();
		VariableSet::size_type i;
		for (i = 0; i < len; ++i) {
			if (rw_directive->no_read_vars[i]->match(v)) {
				return true;
			}
		}
	}
	return false;
}

/*
 *
 */
bool
CGContext::is_nonwritable(const Variable *v) const
{
	if (rw_directive) {
		VariableSet::size_type len = rw_directive->no_write_vars.size();
		for (size_t i = 0; i < len; ++i) {
			const Variable* no_write_var = rw_directive->no_write_vars[i];
			if (no_write_var->loose_match(v) || v->loose_match(no_write_var)) {
				return true;
			}
		}
	}
	// not writing to loop IVs (to avoid infinite loops)
	map<const Variable*, unsigned int>::const_iterator iter;
	for (iter = iv_bounds.begin(); iter != iv_bounds.end(); ++iter) {
		if (v->loose_match(iter->first)) {
			return true;
		}
	}
	return false;
}

bool CGContext::check_deref_volatile(const Variable *v, int deref_level)
{
        assert(v && "NULL Variable!");
        if (!CGOptions::strict_volatile_rule())
                return true;
	int level = deref_level;

	if (!effect_context.is_side_effect_free()) {
		while (level > 0) {
			if (v->is_volatile_after_deref(level))
				return false;
			level--;
		}
	}
	if (effect_accum)
		effect_accum->access_deref_volatile(v, deref_level);

	effect_stm.access_deref_volatile(v, deref_level);
	return true;
}

/*
 *
 */
void
CGContext::read_var(const Variable *v)
{
	v = v->get_collective();
	if (is_nonreadable(v)) {
		assert(!"attempted read from a nonreadable variable");
	}
	if (effect_accum) {
		effect_accum->read_var(v);
	}
	// track effect for single statement
	effect_stm.read_var(v);
	sanity_check();
}

/*
 *
 */
bool
CGContext::check_read_var(const Variable *v, const std::vector<const Fact*>& facts)
{
	if (!read_indices(v, facts)) {
		return false;
	}
	v = v->get_collective();
	if (FactUnion::is_nonreadable_field(v, facts)) {
		return false;
	}
	if (is_nonreadable(v)) {
		return false;
	}
	if (effect_context.is_written_partially(v)) {
		return false;
	}
	if (v->is_volatile() && !effect_context.is_side_effect_free()) {
		return false;
	}
	if (v->is_pointer() && FactPointTo::is_dangling_ptr(v, facts)) {
		return false;
	}
	read_var(v);
	return true;
}

bool CGContext::read_pointed(const ExpressionVariable* v, const std::vector<const Fact*>& facts)
{
	size_t i;
	Effect effect_accum_copy = *effect_accum;
	int indirect = v->get_indirect_level();
	assert(indirect > 0);
	incr_counter(Bookkeeper::dereference_level_cnts, indirect);

	bool allow_null_ptr = CGOptions::null_pointer_dereference_prob() > 0;
	bool allow_dead_ptr = CGOptions::dead_pointer_dereference_prob() > 0;
	if (!read_indices(v->get_var(), facts)) {
		return false;
	}
	vector<const Variable*> tmp;
	tmp.push_back(v->get_var()->get_collective());
	// recursively trace the pointer(s) to find real variables they point to
	while (indirect-- > 0) {
		tmp = FactPointTo::merge_pointees_of_pointers(tmp, facts);
		// make sure there is no null/dead pointers
		if (tmp.size()==0 ||
			(!allow_null_ptr && is_variable_in_set(tmp, FactPointTo::null_ptr)) ||
			(!allow_dead_ptr && is_variable_in_set(tmp, FactPointTo::garbage_ptr))) {
			*effect_accum = effect_accum_copy;
			return false;
		}
		// make sure the remaining pointee are readable in context
		for (i=0; i<tmp.size(); i++) {
			const Variable* pointee = tmp[i];
			if (!FactPointTo::is_special_ptr(pointee)) {
				if (!check_read_var(pointee, facts)) {
					*effect_accum = effect_accum_copy;
					return false;
				}
			}
		}
	}
	return true;
}

bool CGContext::write_pointed(const Lhs* v, const std::vector<const Fact*>& facts)
{
	size_t i;
	Effect effect_accum_copy = *effect_accum;
	int indirect = v->get_indirect_level();
	assert(indirect > 0);
	incr_counter(Bookkeeper::dereference_level_cnts, indirect);
	//vector<const Variable*> tmp = FactPointTo::merge_pointees_of_pointer(v->get_var(), indirect, facts);
	if (!read_indices(v->get_var(), facts)) {
		return false;
	}

	vector<const Variable*> tmp;
	tmp.push_back(v->get_var()->get_collective());

	bool allow_null_ptr = CGOptions::null_pointer_dereference_prob() > 0;
	bool allow_dead_ptr = CGOptions::dead_pointer_dereference_prob() > 0;
	// recursively trace the pointer(s) to find real variables they point to
	while (indirect-- > 0) {
		tmp = FactPointTo::merge_pointees_of_pointers(tmp, facts);
		// make sure there is no null/dead pointers
		if (tmp.size()==0 ||
			(!allow_null_ptr && is_variable_in_set(tmp, FactPointTo::null_ptr)) ||
			(!allow_dead_ptr && is_variable_in_set(tmp, FactPointTo::garbage_ptr))) {
			*effect_accum = effect_accum_copy;
			return false;
		}
		// make sure the remaining pointee are readable or writable(if it is the ultimate pointee) in context
		for (i=0; i<tmp.size(); i++) {
			const Variable* pointee = tmp[i];
			if (!FactPointTo::is_special_ptr(pointee)) {
				bool succ = false;
				if (indirect==0) {
					succ = check_write_var(pointee, facts);
				}
				else {
					succ = check_read_var(pointee, facts);
				}
				if (!succ) {
					*effect_accum = effect_accum_copy;
					return false;
				}
			}
		}
	}
	return true;
}

/*
 *
 */
void
CGContext::write_var(const Variable *v)
{
	v = v->get_collective();
	if (is_nonwritable(v)) {
		assert(!"attempted write to a nonwritable variable");
	}
	if (effect_accum) {
		effect_accum->write_var(v);
	}
	// track effect for single statement
	effect_stm.write_var(v);
	sanity_check();
}

/*
 *
 */
bool
CGContext::check_write_var(const Variable *v, const std::vector<const Fact*>& facts)
{
	if (!read_indices(v, facts)) {
		return false;
	}
	v = v->get_collective();
	if (is_nonwritable(v) || v->is_const()) {
		return false;
	}
	if (effect_context.is_written_partially(v)) {
		return false;
	}
	if (effect_context.is_read_partially(v)) {
		return false;
	}
	if (v->is_volatile() && !effect_context.is_side_effect_free()) {
		return false;
	}
	if ((flags & NO_DANGLING_PTR) && v->is_pointer() && FactPointTo::is_dangling_ptr(v, facts)) {
		return false;
	}
	write_var(v);
	return true;
}

/*
 *
 */
bool
CGContext::read_indices(const Variable* v, const vector<const Fact*>& facts)
{
	size_t i;
	vector<const Variable*> vars;
	if (v->isArray) {
		vector<const Fact*> facts_copy = facts;
		const ArrayVariable* av = (const ArrayVariable*)v;
		for (i=0; i<av->get_indices().size(); i++) {
			const Expression* e = av->get_indices()[i];
			if (!e->visit_facts(facts_copy, *this)) {
				return false;
			}
		}
		if (av->field_var_of) {
			return read_indices(av->field_var_of, facts);
		}
		return true;
	}
	else if	(v->is_array_field()) {
		// find the parent that is an array
		for (; v && !v->isArray; v = v->field_var_of) {
			/* Empty. */
		}
		assert(v);
		return read_indices(v, facts);
	}
	return true;
}

/*
 *
 */
void
CGContext::add_effect(const Effect &e, bool include_lhs_effects)
{
	if (effect_accum) {
		effect_accum->add_effect(e, include_lhs_effects);
	}
	effect_stm.add_effect(e);
	sanity_check();
}

void
CGContext::merge_param_context(const CGContext& param_cg_context, bool include_lhs_effects)
{
	add_effect(*param_cg_context.get_effect_accum(), include_lhs_effects);
	expr_depth = param_cg_context.expr_depth;
}

/*
 *
 */
void
CGContext::add_external_effect(const Effect &e)
{
	if (effect_accum) {
		effect_accum->add_external_effect(e);
	}
	effect_stm.add_external_effect(e);
	sanity_check();
}

/*
 *
 */
void
CGContext::add_visible_effect(const Effect &e, const Block* b)
{
	std::vector<const Block*> callers = call_chain;
	callers.push_back(b);
	if (effect_accum) {
		effect_accum->add_external_effect(e, callers);
	}
	effect_stm.add_external_effect(e, callers);
	sanity_check();
}

// --------------------------------------------------------------
 /* find the scope of a variable in relative to the current context
  * return: -1 if is global variable
  *          0 if from current function parameters
  *          1 if from top level block of same function
  *          ...
  *          n if from block of depth n
  *          INVISIBLE if not visible in current context
  *          INACTIVE if not visible in current context, and not
  *          active on stack frame neither
  **************************************************************/
int
CGContext::find_variable_scope(const Variable* var) const
{
	if (var->is_global()) {
		return -1;
	}
	Function *func = get_current_func();
	assert(func);

	int i;
	for (i=0; i<(int)func->param.size(); i++) {
		if (func->param[i]->match(var)) {
			return 0;
		}
	}

	// check if visible in current function
	const Block* b = get_current_block();
	i = 1;
	do {
		if (find_variable_in_set(b->local_vars, var) != -1) {
			return i;
		}
		b = b->parent;
		i++;
	} while (b);

	// check if exist on one of the stack frames
	for (i=call_chain.size()-1; i>=0; i--) {
		b = call_chain[i];
		do {
			if (find_variable_in_set(b->local_vars, var) != -1) {
				return INVISIBLE;
			}
			b = b->parent;
			i++;
		} while (b);
	}

	return INACTIVE;
}

void CGContext::extend_call_chain(const CGContext& cg_context)
{
	call_chain = cg_context.call_chain;
	const Block* b = cg_context.get_current_block();
	if (b==0) {
		b = cg_context.curr_blk;
	}
	if (b) {
		call_chain.push_back(b);
	}
}

void
CGContext::output_call_chain(std::ostream &out)
{
	size_t i;
	for (i=0; i<call_chain.size(); i++) {
		const Block* b = call_chain[i];
		if (i > 0) {
			out << " -> ";
		}
		out << "b" << b << " in " << b->func->name;
	}
	out << endl;
}

bool
CGContext::is_frame_var(const Variable* v) const
{
	const Block* b = get_current_block();
	assert(b);
	if (v->is_visible_local(b)) return true;

	for (size_t i=0; i<call_chain.size(); i++) {
		const Block* b = call_chain[i];
		if (v->is_visible_local(b)) {
			return true;
		}
	}
	return false;
}

Block *
CGContext::get_current_block(void) const
{
    Function *func = get_current_func();
    if (func) {
        if (func->stack.size() > 0) {
            return func->stack.back();
        }
    }
    return 0;
}

bool
CGContext::allow_volatile(void) const
{
	return effect_context.is_side_effect_free();
}

bool
CGContext::allow_const(Effect::Access access) const
{
	return access != Effect::WRITE;
}

bool
CGContext::accept_type(const Type* t) const
{
	return get_effect_context().is_side_effect_free() || !t->is_volatile_struct_union();
}

/* return true if an incoming effect is in conflict with current context */
bool
CGContext::in_conflict(const Effect& eff) const
{
	size_t i;
	const std::vector<const Variable *>& rvars = eff.get_read_vars();
	const std::vector<const Variable *>& wvars = eff.get_write_vars();
	for (i=0; i<rvars.size(); i++) {
		const Variable* v = rvars[i];
		if (is_nonreadable(v)) {
			return true;
		}
		if (effect_context.is_written_partially(v)) {
			return true;
		}
		// Yang: do we need to consider deref level here?
		if (v->is_volatile() && !effect_context.is_side_effect_free()) {
			return true;
		}
	}

	for (i=0; i<wvars.size(); i++) {
		const Variable* v = wvars[i];
		if (is_nonwritable(v) || v->is_const()) {
			return true;
		}
		if (effect_context.is_written_partially(v)) {
			return true;
		}
		if (effect_context.is_read_partially(v)) {
			return true;
		}
		if (v->is_volatile() && !effect_context.is_side_effect_free()) {
			return true;
		}
	}
	return false;
}

void
CGContext::find_reachable_frame_vars(vector<const Fact*>& facts, VariableSet& frame_vars) const
{
	size_t i, j;
	for (i=0; i<facts.size(); i++) {
		if (facts[i]->eCat == ePointTo) {
			const FactPointTo* fp = (const FactPointTo*)(facts[i]);
			for (j=0; j<fp->get_point_to_vars().size(); j++) {
				const Variable* v = fp->get_point_to_vars()[j];
				if (is_frame_var(v)) {
					frame_vars.push_back(v);
				}
			}
		}
	}
}

void
CGContext::get_external_no_reads_writes(VariableSet& no_reads, VariableSet& no_writes, const VariableSet& frame_vars) const
{
	no_reads.clear();
	no_writes.clear();
	size_t i;

	if (rw_directive) {
		for (i=0; i<rw_directive->no_read_vars.size(); i++) {
			const Variable* v = rw_directive->no_read_vars[i];
			if (v->is_global() || find_variable_in_set(frame_vars, v) != -1) {
				no_reads.push_back(v);
			}
		}
		for (i=0; i<rw_directive->no_write_vars.size(); i++) {
			const Variable* v = rw_directive->no_write_vars[i];
			if (v->is_global() || find_variable_in_set(frame_vars, v) != -1) {
				no_writes.push_back(v);
			}
		}
	}
	// convert global IVs into non-writables
	map<const Variable*, unsigned int>::const_iterator iter;
	for (iter = iv_bounds.begin(); iter != iv_bounds.end(); ++iter) {
		if (iter->first->is_global() || find_variable_in_set(frame_vars, iter->first) != -1) {
			no_writes.push_back(iter->first);
		}
	}
}

void
RWDirective::find_must_use_arrays(vector<const Variable*>& avs) const
{
	avs.clear();
	size_t i;
	for (i=0; i<must_read_vars.size(); i++) {
		const Variable* v = must_read_vars[i];
		if (v->isArray && !is_variable_in_set(avs, v)) {
			avs.push_back(v);
		}
	}
	for (i=0; i<must_write_vars.size(); i++) {
		const Variable* v = must_write_vars[i];
		if (v->isArray && !is_variable_in_set(avs, v)) {
			avs.push_back(v);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
void
CGContext::sanity_check(void)
{
#if 0
	// XXX: too strong.  See note in `TODO'.
	if (effect_accum && !effect_context.is_side_effect_free()) {
		assert(effect_accum->is_side_effect_free());
	}
#endif // 0
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

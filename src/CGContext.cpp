// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011 The University of Utah
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

#include <cassert>

#include "CGContext.h"
#include "Effect.h"
#include "Variable.h"
#include "Function.h"
#include "Block.h"
#include "Type.h"
#include "ExpressionVariable.h"
#include "Fact.h"
#include "FactMgr.h"
#include "FactPointTo.h"
#include "Lhs.h"
#include "Statement.h"
#include "Bookkeeper.h"
#include "ArrayVariable.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 * Declare "the" empty set of variables.
 */
const CGContext::VariableSet CGContext::empty_variable_set(0);
const CGContext CGContext::empty_context(0, Effect::get_empty_effect(), 0);

///////////////////////////////////////////////////////////////////////////////

#if 0
/*
 * 
 */
CGContext::CGContext(void)
	: current_func(0),
	  stmt_depth(0),
	  expr_depth(0),
	  flags(0),
	  curr_blk(0),
	  no_read_vars(),
	  no_write_vars(),
	  effect_context(),
	  effect_accum(0)
{
	// Nothing else to do.
}
#endif

/*
 *
 */
CGContext::CGContext(Function *current_func,
					 int stmt_depth,
					 int expr_depth,
					 unsigned int flags,
					 const std::vector<const Block*>& callers,
					 const Block* blk,
					 ArrayVariable* fvar,
					 const VariableSet &no_read_vars,
					 const VariableSet &no_write_vars,
					 const Effect &eff_context,
					 Effect *eff_accum)
	: current_func(current_func),
	  stmt_depth(stmt_depth),
	  expr_depth(expr_depth),
	  flags(flags),
	  call_chain(callers),
	  curr_blk(blk),
	  focus_var(fvar),
	  no_read_vars(no_read_vars),
	  no_write_vars(no_write_vars),
	  effect_context(eff_context),
	  effect_accum(eff_accum),
	  effect_stm()
{
	// Nothing else to do.
}

/*
 * A convenience constructor.  Set `stmt_depth' and `flags' to zero, and set
 * `no_read_vars' and `no_write_vars' each to be an empty set.
 */
CGContext::CGContext(Function *current_func,
					 const Effect &eff_context,
					 Effect *eff_accum)
	: current_func(current_func),
	  stmt_depth(0),
	  expr_depth(0),
	  flags(0),
	  curr_blk(0),
	  focus_var(0),
	  no_read_vars(CGContext::empty_variable_set),
	  no_write_vars(CGContext::empty_variable_set),
	  effect_context(eff_context),
	  effect_accum(eff_accum)
{
	// Nothing else to do.
}

/*
 * 
 */
CGContext::CGContext(const CGContext &cgc)
	: current_func(cgc.current_func),
	  stmt_depth(cgc.stmt_depth),
	  expr_depth(cgc.expr_depth),
	  flags(cgc.flags),
	  call_chain(cgc.call_chain),
	  curr_blk(cgc.curr_blk),
	  focus_var(cgc.focus_var),
	  no_read_vars(cgc.no_read_vars),
	  no_write_vars(cgc.no_write_vars),
	  effect_context(cgc.effect_context),
	  effect_accum(cgc.effect_accum)
{
	// Nothing else to do.
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
	VariableSet::size_type len = no_read_vars.size();
	VariableSet::size_type i;

	for (i = 0; i < len; ++i) {
		if (no_read_vars[i]->match(v)) {
			return true;
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
	VariableSet::size_type len = no_write_vars.size();
	VariableSet::size_type i;

	for (i = 0; i < len; ++i) {
		if (no_write_vars[i]->match(v)) {
			return true;
		}
	}
	return false;
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
	if (is_nonreadable(v)) {
		return false;
	} 
	if (effect_context.is_written(v) || effect_context.field_is_written(v)) {
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

	if (!read_indices(v->get_var(), facts)) {
		return false;
	}
	vector<const Variable*> tmp;
	tmp.push_back(v->get_var()->get_collective());
	// recursively trace the pointer(s) to find real variables they point to 
	while (indirect-- > 0) {
		tmp = FactPointTo::merge_pointees_of_pointers(tmp, facts);
		// make sure there is no null/dead pointers
		if (tmp.size()==0 || find_variable_in_set(tmp, FactPointTo::null_ptr)!=-1 || find_variable_in_set(tmp, FactPointTo::garbage_ptr) != -1) {
			*effect_accum = effect_accum_copy;
			return false;
		}
		// make sure the remaining pointee are readable in context
		for (i=0; i<tmp.size(); i++) {
			const Variable* pointee = tmp[i];
			if (pointee != FactPointTo::tbd_ptr) {
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
	// recursively trace the pointer(s) to find real variables they point to 
	while (indirect-- > 0) {
		tmp = FactPointTo::merge_pointees_of_pointers(tmp, facts);
		// make sure there is no null/dead pointers
		if (tmp.size()==0 || find_variable_in_set(tmp, FactPointTo::null_ptr)!=-1 || find_variable_in_set(tmp, FactPointTo::garbage_ptr) != -1) {
			*effect_accum = effect_accum_copy;
			return false;
		}
		// make sure the remaining pointee are readable or writable(if it is the ultimate pointee) in context
		for (i=0; i<tmp.size(); i++) {
			const Variable* pointee = tmp[i];
			if (pointee != FactPointTo::tbd_ptr) {
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
	if (effect_context.is_written(v) || effect_context.field_is_written(v)) {
		return false;
	}
	if (effect_context.is_read(v) || effect_context.field_is_read(v)) {
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
		if (av->isFieldVarOf_) {
			return read_indices(av->isFieldVarOf_, facts);
		}
		return true;
	}
	else if	(v->is_array_field()) {
		// find the parent that is an array
		for (; v && !v->isArray; v = v->isFieldVarOf_) {
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
CGContext::add_effect(const Effect &e)
{
	if (effect_accum) {
		effect_accum->add_effect(e);
	}
	effect_stm.add_effect(e);
	sanity_check();
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
	if ((cg_context.flags & IN_LOOP) || (cg_context.flags & IN_CALL_CHAIN_LOOP)) {
		flags |= IN_CALL_CHAIN_LOOP;
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
	return get_effect_context().is_side_effect_free() || !t->is_volatile_struct();
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
		if (effect_context.is_written(v) || effect_context.field_is_written(v)) {
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
		if (effect_context.is_written(v) || effect_context.field_is_written(v)) {
			return true;
		}
		if (effect_context.is_read(v) || effect_context.field_is_read(v)) {
			return true;
		}
		if (v->is_volatile() && !effect_context.is_side_effect_free()) {
			return true;
		}
	}
	return false;
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

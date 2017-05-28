// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2014, 2015, 2017 The University of Utah
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

#include "Lhs.h"
#include <cassert>
#include <iostream>
#include "CGContext.h"
#include "CGOptions.h"
#include "Function.h"
#include "Variable.h"
#include "Fact.h"
#include "Type.h"
#include "FactPointTo.h"
#include "FactMgr.h"
#include "VariableSelector.h"
#include "ExpressionVariable.h"
#include "Bookkeeper.h"
#include "Error.h"
#include "DepthSpec.h"
#include "ArrayVariable.h"
#include "random.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
Lhs *
Lhs::make_random(CGContext &cg_context, const Type* t, const CVQualifiers* qfer, bool compound_assign, bool no_signed_overflow)
{
	Function *curr_func = cg_context.get_current_func();
	FactMgr* fm = get_fact_mgr_for_func(curr_func);
	vector<const Variable*> dummy;
	//static int cnt = 0;				// for debug

	// save effects, in case we need to backtrack
	Effect effect_accum = cg_context.get_accum_effect();
	Effect effect_stm = cg_context.get_effect_stm();

	do {
		DEPTH_GUARD_BY_TYPE_RETURN(dtLhs, NULL);
		const Variable* var = 0;
		// try to use one of the "must_use" variables
		var = VariableSelector::select_must_use_var(Effect::WRITE, cg_context, t, qfer);
		if (var == NULL) {
			bool flag = rnd_flipcoin(SelectDerefPointerProb);
			if (flag) {
				var = VariableSelector::select_deref_pointer(Effect::WRITE, cg_context, t, qfer, dummy);
				ERROR_GUARD(NULL);
				if (var) {
					int deref_level = var->type->get_indirect_level() - t->get_indirect_level();
					assert(!var->qfer.is_const_after_deref(deref_level));
				}
			}
		}
		if (var==0) {
			CVQualifiers new_qfer(*qfer);
			if (!(new_qfer.wildcard)) {
				new_qfer.restrict(Effect::WRITE, cg_context);
			}
			var = VariableSelector::select(Effect::WRITE, cg_context, t, &new_qfer, dummy, eDerefExact);
			ERROR_GUARD(NULL);
			int deref_level = var->type->get_indirect_level() - t->get_indirect_level();
			assert(!var->qfer.is_const_after_deref(deref_level));
		}
		ERROR_GUARD(NULL);
		assert(var);
		bool valid = FactPointTo::opportunistic_validate(var, t, fm->global_facts) && !cg_context.get_effect_stm().is_written(var);
		// we don't want signed integer for some operations, such as ++/-- which has potential of overflowing
		// it's possible for unsigned bitfield to overflow: consider a 31-bit unsigned field that is promoted to 32-bit signed int before arithematics
		if (valid && t->eType == eSimple && no_signed_overflow && (var->type->get_base_type()->is_signed() || var->isBitfield_)) {
			valid = false;
		}
		if (valid && CGOptions::ccomp() && var->isBitfield_ && t->is_long_long()) {
			valid = false;
		}
		if (!t->is_float() && var->type->is_float()) {
			valid = false;
		}
		if (valid) {
			assert(var);
			Lhs tmp(*var, t, compound_assign);
			if (tmp.visit_facts(fm->global_facts, cg_context)) {
				// bookkeeping
				int deref_level = tmp.get_indirect_level();
				if (deref_level > 0) {
					incr_counter(Bookkeeper::write_dereference_cnts, deref_level);
				}
				Bookkeeper::record_volatile_access(var, deref_level, true);
				return new Lhs(*var, t, compound_assign);
			}
			// restore the effects
			cg_context.reset_effect_accum(effect_accum);
			cg_context.reset_effect_stm(effect_stm);
		}
		dummy.push_back(var);
	} while (true);

	assert(0);
	return 0;
}

/*
 *
 */
Lhs::Lhs(const Variable &v)
	: Expression(eLhs),
      var(v),
      type(v.type),
	  for_compound_assign(false)
{
}

/*
 * copy constructor
 */
Lhs::Lhs(const Lhs &lhs)
	: Expression(eLhs),
	  var(lhs.var),
	  type(lhs.type),
	  for_compound_assign(lhs.for_compound_assign)
{
}

/*
 *
 */
Lhs::Lhs(const Variable &v, const Type* t, bool compound_assign)
	: Expression(eLhs),
      var(v),
      type(t),
	  for_compound_assign(compound_assign)
{
}

/*
 *
 */
Lhs::~Lhs(void)
{
	// Nothing to do.
}

///////////////////////////////////////////////////////////////////////////////

Expression *
Lhs::clone() const
{
	return new Lhs(*this);
}

/*
 *
 */
const Type &
Lhs::get_type(void) const
{
	return *(type);
}

void
Lhs::get_lvars(const vector<const Fact*>& facts, vector<const Variable*>& vars) const
{
	vars = FactPointTo::merge_pointees_of_pointer(get_var()->get_collective(), get_indirect_level(), facts);
}

/*
 *
 */
int
Lhs::get_indirect_level(void) const
{
	return var.type->get_indirect_level() - type->get_indirect_level();
}

/*
 *
 */
CVQualifiers
Lhs::get_qualifiers(void) const
{
	int indirect = get_indirect_level();
	CVQualifiers qfer = var.qfer.indirect_qualifiers(indirect);
	assert(!qfer.is_const());
	return qfer;
}

/*
 *
 */
void
Lhs::Output(std::ostream &out) const
{
	ExpressionVariable ev(var, type);
	if (var.is_volatile() && CGOptions::wrap_volatiles()) {
		out << "VOL_LVAL(";
		ev.Output(out);
		out << ", ";
		type->Output(out);
		out << ")";
	} else {
		ev.Output(out);
	}
}

bool
Lhs::is_volatile(void) const
{
	int indirect = get_indirect_level();
	return var.is_volatile_after_deref(indirect);
}

std::vector<const ExpressionVariable*>
Lhs::get_dereferenced_ptrs(void) const
{
	// return a empty vector by default
	std::vector<const ExpressionVariable*> refs;
	if (get_indirect_level() > 0) {
		refs.push_back(new ExpressionVariable(var, type));
	}
	return refs;
}

void
Lhs::get_referenced_ptrs(std::vector<const Variable*>& ptrs) const
{
	if (var.is_pointer()) {
		ptrs.push_back(&var);
	}
}

bool
Lhs::ptr_modified_in_rhs(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	int indirect = get_indirect_level();
	assert(indirect > 0);
	if (cg_context.get_effect_stm().is_written(&var)) {
		return true;
	}
	vector<const Variable*> tmp;
	tmp.push_back(get_var()->get_collective());
	// recursively trace the pointer(s) to find real variables they point to
	// only dereferenced pointers (not including the target variables) need to
	// be checked with context derived from RHS
	while (indirect-- > 1) {
		tmp = FactPointTo::merge_pointees_of_pointers(tmp, inputs);
		for (size_t i=0; i<tmp.size(); i++) {
			const Variable* v = tmp[i];
			if (cg_context.get_effect_stm().is_written(v)) {
				return true;
			}
		}
	}
	return false;
}

bool
Lhs::visit_indices(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	string dummy;
	const ArrayVariable* av = get_var()->get_array(dummy);
	if (av == 0) return true;

	// use RHS accumulated effects as context effects to validate index expressions
	Effect eff = cg_context.get_effect_context();
	eff.add_effect(cg_context.get_effect_stm());
	CGContext rhs_context(cg_context.get_current_func(), eff, 0);
	for (size_t i=0; i<av->get_indices().size(); i++) {
		const Expression* e = av->get_indices()[i];
		if (!e->visit_facts(inputs, rhs_context)) {
			return false;
		}
	}
	return true;
}

// conservatively assume two fields overlap if they are both part of the same union variable
bool
have_overlapping_fields(const Expression* e1, const Expression* e2, const vector<const Fact*>& facts)
{
	vector<const Variable*> vars1, vars2;
	if (FactPointTo::find_union_pointees(facts, e1, vars1)) {
		FactPointTo::find_union_pointees(facts, e2, vars2);
		for (size_t i=0; i<vars2.size(); i++) {
			if (is_variable_in_set(vars1, vars2[i])) {
				return true;
			}
		}
	}
	return false;
}

bool
Lhs::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	bool valid = false;
	const Variable* v = get_var();
	// if LHS is for compound assignments, it's should be validated against a read first
	if (for_compound_assign) {
		ExpressionVariable ev(*v, type);
		if (!ev.visit_facts(inputs, cg_context)) {
			return false;
		}
	}
	if (!visit_indices(inputs, cg_context)) {
		return false;
	}
	// avoid a.x = a.y (or any RHS that evaluates to a.y) where x and y are partially overlapping fields
	if (cg_context.curr_rhs) {
		vector<const Expression*> subs;
		cg_context.curr_rhs->get_eval_to_subexps(subs);
		for (size_t i=0; i<subs.size(); i++) {
			if (subs[i]->term_type == eVariable || subs[i]->term_type == eLhs) {
				if (have_overlapping_fields(subs[i], this, inputs)) {
					return false;
				}
			}
		}
	}

	int deref_level = get_indirect_level();
	if (deref_level > 0) {
		if (!FactPointTo::is_valid_ptr(v, inputs)) {
			return false;
		}
		if (ptr_modified_in_rhs(inputs, cg_context)) {
			return false;
		}
		valid = cg_context.check_read_var(v, inputs) && cg_context.write_pointed(this, inputs) &&
			cg_context.check_deref_volatile(v, deref_level);
	}
	else {
		valid = cg_context.check_write_var(v, inputs);
	}
	if (valid) {
		if (cg_context.get_effect_accum()) {
			Effect* eff = cg_context.get_effect_accum();
			eff->set_lhs_write_vars(eff->get_write_vars());
		}
	}
	return valid;
}

bool
Lhs::compatible(const Expression *exp) const
{
	assert(exp);
	return exp->compatible(&var);
}

bool
Lhs::compatible(const Variable *v) const
{
	return var.compatible(v);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2014, 2015, 2017 The University of Utah
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

#include "ExpressionVariable.h"
#include <cassert>

#include "CGContext.h"
#include "CGOptions.h"
#include "Function.h"
#include "VariableSelector.h"
#include "Fact.h"
#include "FactPointTo.h"
#include "FactMgr.h"
#include "Bookkeeper.h"
#include "Error.h"
#include "DepthSpec.h"
#include "ArrayVariable.h"

#include "random.h"


///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
ExpressionVariable *
ExpressionVariable::make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer, bool as_param, bool as_return)
{
	DEPTH_GUARD_BY_TYPE_RETURN(dtExpressionVariable, NULL);
	Function *curr_func = cg_context.get_current_func();
	FactMgr* fm = get_fact_mgr_for_func(curr_func);
	vector<const Variable*> dummy;

	// save current effects, in case we need to reset
	Effect eff_accum = cg_context.get_accum_effect();
	Effect eff_stmt = cg_context.get_effect_stm();

	ExpressionVariable *ev = 0;
	do {
		const Variable* var = 0;
		// try to use one of must_read_vars in CGContext
		var = VariableSelector::select_must_use_var(Effect::READ, cg_context, type, qfer);
		if (var == NULL) {
			var = VariableSelector::select(Effect::READ, cg_context, type, qfer, dummy, eFlexible);
		}
		ERROR_GUARD(NULL);
		if (!var)
			continue;
		if (!type->is_float() && var->type->is_float())
			continue;
		// forbid a parameter to take the address of an argument
		// this is to simplify the path shortcutting delta
		if (as_param && var->is_argument() && var->type->is_dereferenced_from(type)) {
			continue;
		}
		if (!CGOptions::addr_taken_of_locals()
			&& var->type->is_dereferenced_from(type)
			&& (var->is_argument() || var->is_local())) {
			continue;
		}

		// forbid a escaping pointer to take the address of an argument or a local variable
		int indirection = var->type->get_indirect_level() - type->get_indirect_level();
		if (as_return && CGOptions::no_return_dead_ptr() &&
			FactPointTo::is_pointing_to_locals(var, cg_context.get_current_block(), indirection, fm->global_facts)) {
			continue;
		}
		int valid = FactPointTo::opportunistic_validate(var, type, fm->global_facts);
		if (valid) {
			ExpressionVariable tmp(*var, type);
			if (tmp.visit_facts(fm->global_facts, cg_context)) {
				ev = tmp.get_indirect_level() == 0 ? new ExpressionVariable(*var) : new ExpressionVariable(*var, type);
				cg_context.curr_blk = cg_context.get_current_block();
				break;
			}
			else {
				cg_context.reset_effect_accum(eff_accum);
				cg_context.reset_effect_stm(eff_stmt);
			}
		}
		dummy.push_back(var);
	} while (true);

	// statistics
	int deref_level = ev->get_indirect_level();
	if (deref_level > 0) {
		incr_counter(Bookkeeper::read_dereference_cnts, deref_level);
	}
	if (deref_level < 0) {
		Bookkeeper::record_address_taken(ev->get_var());
	}
	Bookkeeper::record_volatile_access(ev->get_var(), deref_level, false);
	return ev;
}

/*
 *
 */
ExpressionVariable::ExpressionVariable(const Variable &v)
	: Expression(eVariable),
      var(v),
      type(v.type)
{
}

/*
 *
 */
ExpressionVariable::ExpressionVariable(const Variable &v, const Type* t)
	: Expression(eVariable),
	  var(v),
	  type(t)
{
}

/*
 *
 */
ExpressionVariable::ExpressionVariable(const ExpressionVariable &expr)
	: Expression(eVariable),
	  var(expr.var),
	  type(expr.type)
{
	// Nothing to do
}

Expression *
ExpressionVariable::clone() const
{
	return new ExpressionVariable(*this);
}

/*
 *
 */
ExpressionVariable::~ExpressionVariable(void)
{
	// Nothing to do.
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
const Type &
ExpressionVariable::get_type(void) const
{
	return *(type);
}

/*
 *
 */
int
ExpressionVariable::get_indirect_level(void) const
{
	return var.type->get_indirect_level() - type->get_indirect_level();
}

/*
 *
 */
CVQualifiers
ExpressionVariable::get_qualifiers(void) const
{
	int indirect = get_indirect_level();
	return var.qfer.indirect_qualifiers(indirect);
}

/*
 *
 */
void
ExpressionVariable::Output(std::ostream &out) const
{
	output_cast(out);
	Reducer* reducer = CGOptions::get_reducer();
	if (reducer && reducer->output_expr(this, out)) {
		return;
	}
	int i;
    int indirect_level = get_indirect_level();
    if (indirect_level > 0) {
        out << "(";
		for (i=0; i<indirect_level; i++) {
			out << "*";
		}
	}
	else if (indirect_level < 0) {
		assert(indirect_level == -1);
		out << "&";
    }
	var.Output(out);
    if (indirect_level > 0) {
        out << ")";
    }
}

std::vector<const ExpressionVariable*>
ExpressionVariable::get_dereferenced_ptrs(void) const
{
	// return a empty vector by default
	std::vector<const ExpressionVariable*> refs;
	if (get_indirect_level() > 0) {
		refs.push_back(this);
	}
	return refs;
}

void
ExpressionVariable::get_referenced_ptrs(std::vector<const Variable*>& ptrs) const
{
	if (var.is_pointer()) {
		ptrs.push_back(&var);
	}
}

bool
ExpressionVariable::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	int deref_level = get_indirect_level();
	const Variable* v = get_var();
	if (deref_level > 0) {
		if (!FactPointTo::is_valid_ptr(v, inputs)) {
			return false;
		}
		// Yang: do we need to consider the deref_level?
		bool valid = cg_context.check_read_var(v, inputs) && cg_context.read_pointed(this, inputs) &&
			cg_context.check_deref_volatile(v, deref_level);
		return valid;
	}
	// we filter out bitfield
	if (deref_level < 0) {
		if (v->isBitfield_)
			return false;
		// it's actually valid to take address of a null/dead pointer
		return true;
	}
	else {
		return cg_context.check_read_var(v, inputs);
	}
}

/*
 *
 */
bool
ExpressionVariable::compatible(const Expression *exp) const
{
	assert(exp);

	//if (!(exp->term_type == eVariable))
		//return false;

	return exp->compatible(&var);
}

/*
 *
 */
bool
ExpressionVariable::compatible(const Variable *v) const
{
	assert(v);

	return this->var.compatible(v);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

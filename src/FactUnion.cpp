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

#include "FactUnion.h"
#include <iostream>
#include "CGOptions.h"
#include "Fact.h"
#include "Type.h"
#include "Common.h"
#include "Function.h"
#include "FactMgr.h"
#include "FactPointTo.h"
#include "Lhs.h"
#include "ExpressionVariable.h"
#include "FunctionInvocationUser.h"
#include "ExpressionAssign.h"
#include "ExpressionComma.h"
#include <cassert>

const int  FactUnion::TOP = -2;
const int  FactUnion::BOTTOM = -1;

/*
 * constructor
 */
FactUnion::FactUnion(const Variable* v) :
    Fact(eUnionWrite),
    var(v),
    last_written_fid(TOP)
{
	// nothing else to do
}

FactUnion::FactUnion(const Variable* v, int fid) :
    Fact(eUnionWrite),
    var(v),
    last_written_fid(fid)
{
	// nothing else to do
}

const Type*
FactUnion::get_last_written_type(void) const
{
	assert(var->type && var->type->eType == eUnion);
	if (is_top() || is_bottom()) return NULL;
	assert (last_written_fid >= 0 && last_written_fid < (int)(var->field_vars.size()));
	return var->field_vars[last_written_fid]->type;
}

std::vector<const Fact*>
FactUnion::rhs_to_lhs_transfer(const std::vector<const Fact*>& facts, const vector<const Variable*>& lvars, const Expression* rhs)
{
	vector<const Fact*> empty;
	// assert all possible LHS are unions
	for (size_t i=0; i<lvars.size(); i++) {
		assert(lvars[i]->type->eType == eUnion);
	}
	assert(rhs != NULL);
	if (rhs->term_type == eConstant) {
		return make_facts(lvars, 0);
	}
    else if (rhs->term_type == eVariable) {
        const ExpressionVariable* expvar = (const ExpressionVariable*)rhs;
		int indirect = expvar->get_indirect_level();
        assert (indirect >= 0);

		vector<const Variable*> rvars = FactPointTo::merge_pointees_of_pointer(expvar->get_var()->get_collective(), indirect, facts);
		const FactUnion* rhs_fact = dynamic_cast<const FactUnion*>(join_var_facts(facts, rvars));
		if (rhs_fact) {
			return make_facts(lvars, rhs_fact->get_last_written_fid());
		}
	}
    else if (rhs->term_type == eFunction) {
        const FunctionInvocation* fi = rhs->get_invoke();
		if (fi->invoke_type == eFuncCall) {
			const FunctionInvocationUser* fiu = dynamic_cast<const FunctionInvocationUser*>(fi);
			// find the fact regarding return variable
			const FactUnion* rv_fact = dynamic_cast<const FactUnion*>(get_return_fact_for_invocation(fiu, fiu->get_func()->rv, eUnionWrite));
			assert(rv_fact);
			return make_facts(lvars, rv_fact->get_last_written_fid());
        }
    }
	else if (rhs->term_type == eAssignment) {
		const ExpressionAssign* ea = dynamic_cast<const ExpressionAssign*>(rhs);
		return rhs_to_lhs_transfer(facts, lvars, ea->get_rhs());
	}
	else if (rhs->term_type == eCommaExpr) {
		const ExpressionComma* ec = dynamic_cast<const ExpressionComma*>(rhs);
		return rhs_to_lhs_transfer(facts, lvars, ec->get_rhs());
	}
	return empty;
}

/* draw facts from an assignment */
std::vector<const Fact*>
FactUnion::abstract_fact_for_assign(const std::vector<const Fact*>& facts, const Lhs* lhs, const Expression* rhs)
{
	std::vector<const Fact*> ret_facts;
	if (rhs == NULL) return ret_facts;
	// find all the pointed variables on LHS
	std::vector<const Variable*> lvars = FactPointTo::merge_pointees_of_pointer(lhs->get_var()->get_collective(), lhs->get_indirect_level(), facts);
	if (lhs->get_type().eType == eUnion) {
		return rhs_to_lhs_transfer(facts, lvars, rhs);
	}

	for (size_t i=0; i<lvars.size(); i++) {
		const Variable* v = lvars[i];
		const FactUnion* fu = 0;
		if (v->is_union_field()) {
			if (lvars.size() > 1) {
				// if writing to an union field is uncertain (due to dereference of a pointer which may points to an
				// union field or something else), We mark the union as unreadable
				fu = make_fact(v->field_var_of, BOTTOM);
			} else {
				fu = make_fact(v->field_var_of, v->get_field_id());
			}
		} else if (v->is_inside_union_field() && (v->type->has_padding() || v->is_packed_after_bitfield())) {
			fu = make_fact(v->get_container_union(), BOTTOM);
		}

		if (fu) {
			ret_facts.push_back(fu);
		}
	}
    return ret_facts;
}

Fact*
FactUnion::clone(void) const
{
	FactUnion *fact = new FactUnion(var, last_written_fid);
	facts_.push_back(fact);
	return fact;
}

FactUnion *
FactUnion::make_fact(const Variable* v, int fid)
{
	assert(v == NULL || v->type->eType == eUnion);
	FactUnion *fact = new FactUnion(v, fid);
	facts_.push_back(fact);
	return fact;
}

vector<const Fact*>
FactUnion::make_facts(const vector<const Variable*>& vars, int fid)
{
	size_t i;
	vector<const Fact*> facts;
	for (i=0; i<vars.size(); i++) {
		facts.push_back(make_fact(vars[i], fid));
	}
	return facts;
}

bool
FactUnion::is_nonreadable_field(const Variable *v, const std::vector<const Fact*>& facts)
{
	if (v->is_inside_union_field()) {
		for (; v && !v->is_union_field(); v = v->field_var_of) {
			/* Empty */
		}
		assert(v->is_union_field());
		FactUnion tmp(v->field_var_of, v->get_field_id());
		const FactUnion* fu = dynamic_cast<const FactUnion*>(find_related_fact(facts, &tmp));
		if (fu==NULL || !tmp.imply(*fu)) {
			return true;
		}
	}
	return false;
}

bool
FactUnion::equal(const Fact& f) const
{
    if (is_related(f)) {
        const FactUnion& fact = (const FactUnion&)f;
		return last_written_fid == fact.get_last_written_fid();
    }
    return false;
}

/*
 * return 1 if changed, 0 otherwise. currently facts from diff. categories are not joined
 */
int
FactUnion::join(const Fact& f)
{
	if (is_related(f)) {
		if (imply(f)) return 0;
		if (f.imply(*this)) {
			last_written_fid = ((const FactUnion&)f).get_last_written_fid();
		} else {
			set_bottom();
		}
		return 1;
	}
	return 0;
}

/*
 * join facts about a list of vars, return the merged facts
 */
Fact*
FactUnion::join_var_facts(const vector<const Fact*>& facts, const vector<const Variable*>& vars) const
{
	FactUnion* fu = 0;
	for (size_t i=0; i<vars.size(); i++) {
		FactUnion tmp(vars[i]);
		const FactUnion* exist_fact = dynamic_cast<const FactUnion*>(find_related_fact(facts, &tmp));
		if (exist_fact) {
			if (fu == 0) {
				fu = dynamic_cast<FactUnion*>(exist_fact->clone());
			} else {
				// hack: both facts have to be the property of the same variable to be able to merge
				fu->set_var(exist_fact->get_var());
				fu->join(*exist_fact);
			}
		}
	}
	return fu;
}

/*
 * return true if this fact is lower than the other fact in lattice
 * i.e. merging with the other fact causes no change to this fact
 */
bool
FactUnion::imply(const Fact& f) const
{
	if (is_related(f)) {
		if (is_bottom()) return true;			// bottom implies every node
		const FactUnion& fu = (const FactUnion&)f;
		if (fu.is_bottom()) return false;
		if (equal(fu)) return true;

		// type sensitive lattice: bottom -> size-determined types -> wider size determined types
		//								  \-> char* -> other pointer types
		if (CGOptions::union_read_type_sensitive()) {
			const Type* t = get_last_written_type();
			const Type* other_t = fu.get_last_written_type();
			if (t->eType == ePointer && other_t->eType == ePointer) {
				if (t->is_pointer_to_char()) {
					return true;
				}
				return false;
			}
			if (t->SizeInBytes() == SIZE_UNKNOWN || other_t->SizeInBytes() == SIZE_UNKNOWN) {
				return false;
			}
			if (t->SizeInBytes() <= other_t->SizeInBytes()) {
				// only safe is the other field is not bitfield
				if (!var->type->is_bitfield(fu.get_last_written_fid())) {
					return true;
				}
			}
		}
	}
	return false;
}

bool
FactUnion::is_field_readable(const Variable* v, int fid, const vector<const Fact*>& facts)
{
	assert(v->type->eType == eUnion && fid >=0 && fid < (int)(v->type->fields.size()));
	FactUnion tmp(v, fid);
	const FactUnion* fu = dynamic_cast<const FactUnion*>(find_related_fact(facts, &tmp));
	return (fu && tmp.imply(*fu)) ;
}

void
FactUnion::Output(std::ostream &out) const
{
	var->Output(out);
	out << " last written field: " << last_written_fid;
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

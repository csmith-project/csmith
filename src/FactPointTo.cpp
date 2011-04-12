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
 
#include "FactPointTo.h"
#include <iostream>
#include "Fact.h"
#include "Type.h"
#include "VariableSelector.h"
#include "ArrayVariable.h"
#include "StatementAssign.h"
#include "StatementReturn.h"
#include "Block.h"
#include "Constant.h"
#include "Function.h"
#include "ExpressionVariable.h"
#include "ExpressionFuncall.h"
#include "FunctionInvocation.h"
#include "FunctionInvocationUser.h"
#include "FactMgr.h"
#include "Lhs.h"

#include <assert.h>

using namespace std; 
 
// ISSUE: don't delete those vars, which are global static variables, and
// the order of init and deconstruct is not specified by C++ standard. Just
// let them be. 
const Variable* FactPointTo::null_ptr = VariableSelector::make_dummy_static_variable("null");
const Variable* FactPointTo::garbage_ptr = VariableSelector::make_dummy_static_variable("garbage");
const Variable* FactPointTo::tbd_ptr = VariableSelector::make_dummy_static_variable("tbd");
vector<const Variable*> FactPointTo::all_ptrs;
vector<vector<const Variable*> > FactPointTo::all_aliases;
std::vector<FactPointTo*> FactPointTo::facts_;

bool
FactPointTo::is_null() const 
{
    size_t i;
    for (i=0; i<point_to_vars.size(); i++) {
        if (point_to_vars[i] == null_ptr) {
            return true;
        }
    }
    return false;
}

bool
FactPointTo::is_tbd_only() const 
{
    return point_to_vars.size()==1 && point_to_vars[0] == tbd_ptr;
}

bool
FactPointTo::is_dead() const 
{
    return (find_variable_in_set(point_to_vars, garbage_ptr) != -1);
}

bool 
FactPointTo::has_invisible(const Function* func, const Statement* stm) const
{
    size_t i;
	if (!func->is_var_visible(var, stm)) {
		return true;
	}
    for (i=0; i<point_to_vars.size(); i++) {
		const Variable* v = point_to_vars[i];
		if (v != null_ptr && v != garbage_ptr && v != tbd_ptr) {
			if (!func->is_var_visible(v, stm)) {
				return true;
			}
		}
    }
    return false;
}

/* mark a variable in point_to_vars as dead, note
   this create a new fact, and if that variable is
   not in point-to set, return null to indicate no
   update is neccessary
 */
FactPointTo*
FactPointTo::mark_dead_var(const Variable* v)
{
	vector<const Variable*> var_set = point_to_vars; 
	int pos = find_variable_in_set(var_set, v);
	if (pos == -1) {
		pos = find_field_variable_in_set(var_set, v);
	}
	if (pos >= 0) { 
		if (find_variable_in_set(var_set, garbage_ptr) >= 0) {
			// if there is already a garbage pointer, delete this variable
			var_set.erase(var_set.begin() + pos);
		}
		else { 
			// otherwise, replace this var with a garbage pointer
			var_set[pos] = garbage_ptr;
		}
		return FactPointTo::make_fact(var, var_set);
	} 
	return 0;
}

/* mark any local variable in point_to_vars as dead, note
   this create a new fact, and if no local variable is in 
   point-to set, return null to indicate no update is neccessary
 */
FactPointTo*
FactPointTo::mark_func_end(const Statement* stm)
{
	vector<const Variable*> var_set = point_to_vars;
	size_t len = var_set.size();
	bool changed = false;
	bool has_garbage_ptr = (find_variable_in_set(var_set, garbage_ptr) >= 0);
	const Function* func = stm->func;

	for (size_t i=0; i<len; i++) {
		const Variable* v = var_set[i];
		if (func->is_var_on_stack(v, stm)) {
			if (has_garbage_ptr) {
				var_set.erase(var_set.begin() + i);
				i--;
				len--;
			}
			else {
				var_set[i] = garbage_ptr;
				has_garbage_ptr = true;
			}
			changed = true;
		}
	}
	if (changed) { 
		return FactPointTo::make_fact(var, var_set);
	} 
	return 0;
}

int
FactPointTo::size() const 
{
	return point_to_vars.size();
}

std::vector<const Fact*>
FactPointTo::abstract_fact_for_assign(const std::vector<const Fact*>& facts, const Lhs* lhs, const Expression* rhs)
{   
	std::vector<const Fact*> ret_facts;
	// return empty set if lhs is not pointer
	if (lhs->get_type().eType != ePointer) {
		return ret_facts;
	}
	// find all the pointed variables on LHS
	std::vector<const Variable*> lvars = merge_pointees_of_pointer(lhs->get_var()->get_collective(), lhs->get_indirect_level(), facts);
	  
    if (rhs->term_type == eConstant) { 
        string v = ((const Constant*)rhs)->get_value(); 
        if (v.compare("0") == 0) {
            return FactPointTo::make_facts(lvars, null_ptr); 
        }
        else {
            // what does it mean to set a pointer to a non-zero constant?
            // there could be multiple interpretations. Right now we just choose
            // the simplest one: treat it as zero
            return FactPointTo::make_facts(lvars, null_ptr); 
        }
    } 
    else if (rhs->term_type == eVariable) {
        const ExpressionVariable* expvar = (const ExpressionVariable*)rhs;
		int indirect = expvar->get_indirect_level();
        if (indirect < 0) {
            // taking address of another variable. multi-level indirection is not allowed
			assert(indirect == -1);
			return FactPointTo::make_facts(lvars, expvar->get_var()->get_collective()); 
        }
		else if (indirect > 0) { 
			vector<const Variable*> var_set = merge_pointees_of_pointer(expvar->get_var()->get_collective(), indirect+1, facts);
			return FactPointTo::make_facts(lvars, var_set);
		}
        else {
			const Variable* pointer = expvar->get_var()->get_collective();
            FactPointTo dummy(pointer);
            const Fact* exist_fact = find_related_fact(facts, &dummy);
            // we are setting a pointer to another pointer that hasn't
            // been initialized in this function, most likely it's a argument
            if (exist_fact == 0) {
                assert(0); //return pointer->is_argument() ? FactPointTo::make_facts(lvars, tbd_ptr) : FactPointTo::make_facts(lvars, garbage_ptr);
            } 
            // otherwise use the analysis for the existing pointer
            else {
                FactPointTo* f2 = (FactPointTo*)exist_fact;
                return FactPointTo::make_facts(lvars, f2->get_point_to_vars());
            } 
        }
    }
    else if (rhs->term_type == eFunction) {
        const ExpressionFuncall* funcall = (const ExpressionFuncall*)rhs;
        const FunctionInvocation* fi = funcall->get_invoke();
        switch (fi->invoke_type) {
            case eBinaryPrim:
            case eUnaryPrim:
                break;              // TODO: support pointer arithmatics
            case eFuncCall: {
				const FunctionInvocationUser* fiu = dynamic_cast<const FunctionInvocationUser*>(fi);
				// find the fact regarding return variable
				FactPointTo* rv_fact = (FactPointTo*)get_return_fact_for_invocation(fiu); 
				assert(rv_fact);
				return FactPointTo::make_facts(lvars, rv_fact->get_point_to_vars());
            }
        }
    } 
    return ret_facts;
}

Fact* 
FactPointTo::abstract_fact_for_return(const std::vector<const Fact*>& facts, const ExpressionVariable* var, const Function* func)
{ 
    if (func->return_type->eType == ePointer) {
		int indirect_level = var->get_indirect_level();
		// if > -2, means we "return &(&v)", which shouldn't happen in current impl.
	    assert(indirect_level > -2);  
		if (indirect_level == -1) {
			return FactPointTo::make_fact(func->rv, var->get_var()->get_collective());
		}
		if (indirect_level == 0) {
			FactPointTo dummy(var->get_var()->get_collective());
			const Fact* exist_fact = find_related_fact(facts, &dummy);
              
			if (exist_fact == 0) { 
				// we are returning a pointer we knew nothing, something is wrong
				assert(0);
			}
			else { 
				return FactPointTo::make_fact(func->rv, ((FactPointTo*)exist_fact)->get_point_to_vars());
			}
		}
		else {
			// for one or more level(s) of dereferences 
			// this is a much complicated case: we are returning a pointer in the form of *p (which implies 
			// p is a double pointer), we need to find out all variables p point to, and merge all variables
			// these variables in turn point to 
			vector<const Variable*> var_set = merge_pointees_of_pointer(var->get_var()->get_collective(), indirect_level+1, facts);			
			return FactPointTo::make_fact(func->rv, var_set);
		}

    }
    return 0;
}

Fact*
FactPointTo::clone(void) const
{
	FactPointTo *fact = new FactPointTo(var, point_to_vars);
	facts_.push_back(fact);
	return fact;
}

FactPointTo *
FactPointTo::make_fact(const Variable *v)
{
	FactPointTo *fact = new FactPointTo(v);
	facts_.push_back(fact);
	return fact;
}

FactPointTo *
FactPointTo::make_fact(const Variable* v, const vector<const Variable*>& set)
{
	FactPointTo *fact = new FactPointTo(v, set);
	facts_.push_back(fact);
	return fact;
}

FactPointTo *
FactPointTo::make_fact(const Variable* v, const Variable* point_to)
{
	FactPointTo *fact = new FactPointTo(v, point_to);
	facts_.push_back(fact);
	return fact;
}

vector<const Fact*>
FactPointTo::make_facts(vector<const Variable*> vars, const vector<const Variable*>& set)
{
	size_t i;
	vector<const Fact*> facts;
	for (i=0; i<vars.size(); i++) {
		// if type is null, means they are special variables (most likely tbd_ptr) we don't care
		if (vars[i]->type != 0) {
			facts.push_back(make_fact(vars[i], set));
		}
	}
	return facts;
}

vector<const Fact*>
FactPointTo::make_facts(vector<const Variable*> vars, const Variable* point_to)
{
	size_t i;
	vector<const Fact*> facts;
	for (i=0; i<vars.size(); i++) {
		// if type is null, means they are special variables (most likely tbd_ptr) we don't care
		if (vars[i]->type != 0) {
			facts.push_back(make_fact(vars[i], point_to));
		}
	}
	return facts;
}

///////////////////////////////////////////////////////////////////////////////
/*
 * 
 */
FactPointTo::FactPointTo(const Variable* v) :
    Fact(ePointTo),
    var(v)
{
    // every pointer starts from un-initialized state
    point_to_vars.push_back(garbage_ptr);
}
 
/*
 * 
 */
FactPointTo::FactPointTo(const Variable* v, const vector<const Variable*>& set) :
    Fact(ePointTo),
    var(v),
    point_to_vars(set)
{
}
 
/*
 * 
 */
FactPointTo::FactPointTo(const Variable* v, const Variable* point_to) :
    Fact(ePointTo),
    var(v)
{
    point_to_vars.push_back(point_to);
}

#if 0
/*
 * 
 */
FactPointTo::FactPointTo(const FactPointTo& f) :
    Fact(ePointTo),
    var(f.get_var()),
    point_to_vars(f.get_point_to_vars())
{
}
#endif

/*
 * 
 */
FactPointTo::~FactPointTo(void)
{
	// Nothing else to do.
}
 
/*
 * return 1 if both facts are concerning about the nullness of the same pointer
 */
bool 
FactPointTo::is_related(const Fact& f) const
{
    if (eCat == f.eCat) {
        const FactPointTo& fact = (const FactPointTo&)f;
        return (var == fact.get_var());
    }
    return false;
}

/*
 * return 1 if given facts is for a variable in this point-to set
 */
bool 
FactPointTo::is_relevant(const Fact& f) const 
{
	if (eCat == f.eCat) {
		return is_variable_in_set(point_to_vars, f.get_var());
	}
	return false;
}

/*
 * return true if ptr is either null nore dangling in the given context
 */ 
bool 
FactPointTo::is_valid_ptr(const Variable* p, const std::vector<const Fact*>& facts)
{ 
	FactPointTo fp(p);
	const FactPointTo* fact = (const FactPointTo*)find_related_fact(facts, &fp); 
	return (fact && !fact->is_null() && !fact->is_dead());
}

/*
 * return true if ptr is either null nore dangling in the given context
 */ 
bool 
FactPointTo::is_valid_ptr(const char* name, const std::vector<const Fact*>& facts)
{ 
	size_t i;
	for (i=0; i<facts.size(); i++) {
		if (facts[i]->get_var()->name == name) {
			if (facts[i]->eCat == ePointTo) { 
				const FactPointTo* fact = (const FactPointTo*)(facts[i]); 
				return (!fact->is_null() && !fact->is_dead());
			}
		}
	}
	return true;
}

/*
 * return true if ptr is dangling in the given context
 */ 
bool 
FactPointTo::is_dangling_ptr(const Variable* p, const std::vector<const Fact*>& facts)
{
	FactPointTo fp(p);
	const FactPointTo* fact = (const FactPointTo*)find_related_fact(facts, &fp);
	return (fact && fact->is_dead());
}

/*
 * return true if expression is a dangling expression
 */
bool 
FactPointTo::is_dangling_expr(const Expression* e, const std::vector<const Fact*>& facts)
{
	const Type& type = e->get_type(); 
	if (type.eType == ePointer) {
		if (e->term_type == eVariable) {
			const ExpressionVariable* ev = (const ExpressionVariable*)e;
			if (ev->get_indirect_level()==0) {
				FactPointTo fp(ev->get_var());
				const FactPointTo* fact = (const FactPointTo*)find_related_fact(facts, &fp);
				return (fact && fact->is_dead());
			}
		}
		else if (e->term_type == eFunction) {
			const ExpressionFuncall* ef = (const ExpressionFuncall*)e;
			if (ef->get_invoke()->invoke_type == eFuncCall) {
				const FunctionInvocationUser* fiu = (const FunctionInvocationUser*)(ef->get_invoke());
				const Fact* fact = get_return_fact_for_invocation(fiu);
				if (fact) {
					assert(fact->eCat == ePointTo);
					return ((const FactPointTo*)fact)->is_dead();
				}
			}
		}
	}
	return false;
}

/* return true if the variable has any chance to be a local variable after dereference */
bool FactPointTo::is_pointing_to_locals(const Variable* v, const Block* b, int indirection, const vector<const Fact*>& facts)
{
	if (indirection == -1) {
		return v->is_visible_local(b);
	}
	if (!v->is_pointer()) return false;
	if (v->isArray) {
		v = v->get_collective();
	}
	vector<const Variable*> pointees;
	if (indirection == 0) { 
		FactPointTo f(v);
		const FactPointTo* ft = dynamic_cast<const FactPointTo*>(find_related_fact(facts, &f));
		if (ft) {
			pointees = ft->get_point_to_vars();
		}
	} else {
		pointees = merge_pointees_of_pointer(v, indirection, facts);
	}
	for (size_t i=0; i<pointees.size(); i++) {
		const Variable* pointee = pointees[i];
		if (pointee->is_visible_local(b)) {
			return true;
		}
		// recursively find if any points-to locations are local
		if (pointee->is_pointer()) {
			for (int j=0; j<pointee->type->get_indirect_level(); j++) {
				vector<const Variable*> vars = merge_pointees_of_pointer(pointee, j+1, facts);
				for (size_t k=0; k<vars.size(); k++) {
					if (vars[k]->is_visible_local(b)) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

std::string 
FactPointTo::point_to_str(const Variable* v)
{
	if (v == null_ptr) {
		return "0";
	}
	else if (v == tbd_ptr) {
		return "tbd";
	}
	else if (v == garbage_ptr) {
		return "garbage";
	}
	string s = "&";
	s += v->name;
	return s;
}

bool 
FactPointTo::equal(const Fact& f) const
{
    if (eCat == f.eCat) {
        const FactPointTo& fact = (const FactPointTo&)f;
        return (var == fact.get_var() && equal_variable_sets(point_to_vars, fact.get_point_to_vars()));
    }
    return false;
}

/*
 * return 1 if changed, 0 otherwise
 */
int 
FactPointTo::join(const Fact& f)
{
    // right now, only consider facts of same category
    // intersect diff. categories of facts later?
    int changed = 0;
    if (is_related(f)) {
        const FactPointTo& fact = (const FactPointTo&)f;  
        const vector<const Variable*>& vars = fact.get_point_to_vars();
        for (size_t i=0; i<vars.size(); i++) {
            const Variable* v = vars[i];
            if (!is_variable_in_set(point_to_vars, v)) {
                point_to_vars.push_back(v);
                changed = 1;
            }
        } 
    }
    return changed;
}

/*
 * join two facts from two visits to the same function
 * return 1 if changed, 0 otherwise
 */
int 
FactPointTo::join_visits(const Fact& f)
{
	// ignore tbd fact from either visit
    int changed = 0;
    if (is_related(f)) {
        const FactPointTo& fact = (const FactPointTo&)f;
		if (!fact.is_tbd_only()) {
			const vector<const Variable*>& vars = fact.get_point_to_vars();
			if (is_tbd_only()) {
				point_to_vars.clear();
			}
			for (size_t i=0; i<vars.size(); i++) {
				const Variable* v = vars[i];
				if (!is_variable_in_set(point_to_vars, v)) {
					point_to_vars.push_back(v);
					changed = 1;
				}
			}
        } 
    }
    return changed;
}

/*
 * return false if point-to already contains point-to-set in f, true otherwise
 */
bool 
FactPointTo::conflict_with(const Fact& f) const
{
    // right now, only consider facts of same category.
    // compare diff. categories of facts later?
    if (is_related(f)) {
        const FactPointTo& fact = (const FactPointTo&)f; 
        if (sub_variable_sets(fact.get_point_to_vars(), point_to_vars)) {
            return false;
        }
    }
    return true;
}
 
void output_var(const Variable* var, std::ostream &out)
{
	var->Output(out);
	// for array of pointers, only asserting the first pointer in array probably is enough?
	if (var->isArray)
	{
		size_t i;
		const ArrayVariable* av = (const ArrayVariable*)var;
		for (i=0; i<av->get_dimension(); i++) {
			out << "[0]";
		}
	}
}

/*
 * 
 */
void 
FactPointTo::Output(std::ostream &out) const
{ 
	for (size_t i=0; i<point_to_vars.size(); i++) { 
		if (i > 0) {
            out << " || ";
        } 
		const Variable* pointee = point_to_vars[i];
		if (pointee->isArray || pointee->is_array_field()) {
			//const ArrayVariable* av = (const ArrayVariable*)pointee;
			out << "(";
			output_var(var, out);
			out << " >= &";
			pointee->OutputLowerBound(out);
			out << " && ";
			output_var(var, out);
			out << " <= &";
			pointee->OutputUpperBound(out);
			out <<  ")";
			continue;
		}
        output_var(var, out);
        out << " == "; 
		if (pointee == garbage_ptr) {
			out << "dangling";
		}
		else if (pointee == tbd_ptr) {
			out << "tbd";
		}
		else if (pointee == null_ptr) {
			out << "0";
		}
		else {
			out << "&";
			pointee->Output(out);
		}
    }
}

/*
 * output assertion to random program, to check the correctness of compiler 
 */
void 
FactPointTo::OutputAssertion(std::ostream &out) const
{
	if (point_to_vars.size() > 0) { 
		// put in comment if the assertion doesn't make sense to compiler
		if (is_variable_in_set(point_to_vars, garbage_ptr) ||
			is_variable_in_set(point_to_vars, tbd_ptr)) {  
				out << "//";
		}
        out << "assert ("; 
        Output(out);
        out << ");" << endl;
    }
}

std::vector<const Variable*>
FactPointTo::merge_pointees_of_pointer(const Variable* ptr, int indirect, const std::vector<const Fact*>& facts)
{
	vector<const Variable*> tmp;
	tmp.push_back(ptr);
	// recursively trace the pointer(s) to find real variables they point to 
	while (indirect-- > 0) {
		tmp = FactPointTo::merge_pointees_of_pointers(tmp, facts);
	}
	return tmp;
}

std::vector<const Variable*> 
FactPointTo::merge_pointees_of_pointers(const std::vector<const Variable*>& ptrs, const std::vector<const Fact*>& facts)
{
	size_t i, j;
	vector<const Variable*> pointee_vars;
	for (i=0; i<ptrs.size(); i++) {
		const Variable* p = ptrs[i];
		FactPointTo dummy(p);
		const FactPointTo* exist_fact = (const FactPointTo*)find_related_fact(facts, &dummy);
		// I can not think of a reason this is null
		// well...this actually happens when p is a parameter of function f, and we are in the middle of creating f 
		// assert(exist_fact);
		if (exist_fact) {
			for (j=0; j<exist_fact->get_point_to_vars().size(); j++) {
				const Variable* pointee = exist_fact->get_point_to_vars()[j];
				add_variable_to_set(pointee_vars, pointee);
			}
		}
	}
	return pointee_vars;
}

/* 
 * check if one of the array indices is based on given variable, if yes, modifying
 * given variable would render the point-to fact undeterministic. For example, if
 * pointer p points to a[j], and j is modified, now instead of a single array member,
 * p could be pointing to any member in array a. We use constant -1 to denote this
 * fact
 */
const FactPointTo*
FactPointTo::update_with_modified_index(const Variable* index_var) const
{
	size_t j, k;
	vector<const Variable*> pointees = point_to_vars;
	bool changed = false;
	if (index_var->name == "p_4") 
		j = 0;
	for (j=0; j<point_to_vars.size(); j++) {
		const Variable* v = point_to_vars[j];
		while (v->isFieldVarOf_) {
			v = v->isFieldVarOf_;
		}
		// if v is an itemized array variable, check it's indices
		if (v->isArray && v->get_collective() != v) {
			const ArrayVariable* av = (const ArrayVariable*)v;
			ArrayVariable* new_av = 0;
			vector<size_t> modified;
			for (k=0; k<av->get_indices().size(); k++) {
				const Expression* exp = av->get_indices()[k];
				if (exp->use_var(index_var)) {
					modified.push_back(k);
				}
			}
			if (!modified.empty()) {
				new_av = new ArrayVariable(*av);
				for (k=0; k<modified.size(); k++) {
					Constant* neg1 = new Constant(get_int_type(), "-1");
					new_av->set_index(modified[k], neg1);
				}
			}
			if (new_av) {
				pointees[j] = new_av;
				changed = true;
			}
		}
	}
	if (changed) {
		return make_fact(var, pointees);
	}
	return this;
}

void
FactPointTo::update_facts_with_modified_index(std::vector<const Fact*>& facts, const Variable* index_var)
{
	size_t i;
	for (i=0; i<facts.size(); i++) {
		if (facts[i]->eCat == ePointTo) {
			const FactPointTo* fp = (const FactPointTo*)facts[i];
			const FactPointTo* new_fp = fp->update_with_modified_index(index_var);
			if (new_fp != fp) {
				facts[i] = new_fp;
			}
		}
	}
}

void 
FactPointTo::update_ptr_aliases(const vector<Fact*>& facts, vector<const Variable*>& ptrs, vector<vector<const Variable*> >& aliases)
{
	size_t i, j;
	for (j=0; j<facts.size(); j++) {
		if (facts[j]->eCat == ePointTo) {
			const FactPointTo* f = (const FactPointTo*)(facts[j]);
			// don't include rv facts
			if (f->get_var()->type != 0) {
				int pos = find_variable_in_set(ptrs, f->get_var());
				if (pos == -1) {
					ptrs.push_back(f->get_var());
					vector<const Variable*> set = f->get_point_to_vars();
					aliases.push_back(set);
					assert(ptrs.size() == aliases.size());
				}
				else {
					// merge the old alias set with new alias set
					for (i=0; i<f->get_point_to_vars().size() ; i++) {
						const Variable* v = f->get_point_to_vars()[i];
						if (find_variable_in_set(aliases[pos], v) == -1) {
							aliases[pos].push_back(v);
						}
					}
				}
			}
		}
	}
}

void
FactPointTo::aggregate_all_pointto_sets(void) 
{
	size_t i;
	const vector<Function*>& funcs = get_all_functions();  
	for (i=0; i<funcs.size(); i++) { 
		FactMgr* fm = get_fact_mgr_for_func(funcs[i]);
		map<const Statement*, vector<Fact*> >::iterator iter; 
		for(iter = fm->map_facts_out_final.begin(); iter != fm->map_facts_out_final.end(); ++iter) { 
			update_ptr_aliases(iter->second, all_ptrs, all_aliases);
		} 
	}
	assert(all_ptrs.size() == all_aliases.size());
}

void
FactPointTo::doFinalization()
{
	std::vector<FactPointTo*>::iterator i;
	for( i = FactPointTo::facts_.begin(); i != FactPointTo::facts_.end(); ++i) {
		delete (*i);
	}
	FactPointTo::facts_.clear();
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

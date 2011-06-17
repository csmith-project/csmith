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
#ifdef WIN32 
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif

#include "FactMgr.h" 

#include <cassert>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include "Fact.h"
#include "FactPointTo.h"
#include "FactUnion.h"
#include "Variable.h"
#include "Block.h"
#include "StatementFor.h"
#include "StatementIf.h"
#include "StatementReturn.h"
#include "StatementAssign.h"
#include "StatementExpr.h"
#include "StatementGoto.h"
#include "StatementArrayOp.h"
#include "Function.h"
#include "FunctionInvocation.h"
#include "FunctionInvocationUser.h"
#include "CGOptions.h"
#include "Expression.h"
#include "ExpressionVariable.h"
#include "Lhs.h"
#include "CFGEdge.h"

using namespace std; 
 
std::vector<Fact*> FactMgr::meta_facts;

void
FactMgr::add_new_local_var_fact(const Block* blk, const Variable* var)
{
	// if blk == 0, we add fact to all blocks of this function
	// otherwise, only add fact to blocks equal or below blk (variable is declared in blk) 
	if (blk==0) {
		add_new_global_var_fact(var);
	}
	else {
		assert(blk->func == func); 
		size_t j, k; 
		const vector<Fact*>& meta_facts = FactMgr::meta_facts;
		// TODO: consider facts related to struct or arrays
		if (var->type->eType == eSimple || var->type->eType == ePointer || var->type->eType == eUnion) { 
			FactVec facts;
			for (j=0; j<meta_facts.size(); j++) {
				Lhs lhs(*var);
				facts = meta_facts[j]->abstract_fact_for_assign(facts, &lhs, var->init);
				for (k=0; k<facts.size(); k++) {
					const Fact* f = facts[k];
					// sometimes the facts is already included during the creation of child
					// functions. so need this check
					if (find_related_fact(global_facts, f) == 0) {
						global_facts.push_back(f);
					} 
					if (find_related_fact(shadow_facts, f) == 0) {
						shadow_facts.push_back(f);
					} 
					map<const Statement*, FactVec>::iterator iter;
					for(iter = map_facts_in.begin(); iter != map_facts_in.end(); ++iter) {  
						const Statement* stm = iter->first;
						if (stm && stm->in_block(blk)) {
							iter->second.push_back(f);
						}
					}
					for(iter = map_facts_out.begin(); iter != map_facts_out.end(); ++iter) {  
						const Statement* stm = iter->first;
						assert(stm);
						add_fact_out(stm, f);
					}
				}
			}
		}
	}
}

void 
FactMgr::add_param_facts(const vector<const Expression*>& param_values, FactVec& facts)
{ 
	for (size_t i=0; i<func->param.size(); i++) {
		const Variable* var = func->param[i];
		const Expression* value = param_values[i]; 
		Lhs lhs(*var); 
		update_fact_for_assign(&lhs, value, facts);
	}
}

void 
add_new_var_fact(const Variable* v, FactVec& facts)
{
	size_t j, k;
	const vector<Fact*>& meta_facts = FactMgr::meta_facts;
	assert(v);
	// TODO: consider facts related to struct or arrays
	if (v->type && (v->type->eType == eSimple || v->type->eType == ePointer || v->type->eType == eUnion) && v->init) { 
		FactVec tmp_facts;
		for (j=0; j<meta_facts.size(); j++) {
			Lhs lhs(*v);
			tmp_facts = meta_facts[j]->abstract_fact_for_assign(tmp_facts, &lhs, v->init);
			for (k=0; k<tmp_facts.size(); k++) {
				const Fact* f = tmp_facts[k];
				// sometimes the facts is already included during the creation of child
				// functions. so need this check
				if (find_related_fact(facts, f) == 0) {
					facts.push_back(f);
				}
			}
		}
	}
}

void 
FactMgr::add_new_global_var_fact(const Variable* v)
{
	size_t j, k;
	assert(v->is_global());
	if (v->name == "g_39") 
		j = 0;
	const vector<Fact*>& meta_facts = FactMgr::meta_facts;
	// TODO: consider facts related to struct or arrays
	if (v->type->eType == eSimple || v->type->eType == ePointer || v->type->eType == eUnion) { 
		FactVec facts;
		for (j=0; j<meta_facts.size(); j++) {
			Lhs lhs(*v);
			facts = meta_facts[j]->abstract_fact_for_assign(facts, &lhs, v->init);
			for (k=0; k<facts.size(); k++) {
				const Fact* f = facts[k];
				// sometimes the facts is already included during the creation of child
				// functions. so need this check
				if (find_related_fact(global_facts, f) == 0) {
					global_facts.push_back(f);
				} 
				if (find_related_fact(shadow_facts, f) == 0) {
					shadow_facts.push_back(f);
				} 
				map<const Statement*, FactVec>::iterator iter;
				for(iter = map_facts_in.begin(); iter != map_facts_in.end(); ++iter) {  
					iter->second.push_back(f);
				}
				for(iter = map_facts_out.begin(); iter != map_facts_out.end(); ++iter) { 
					//const Statement* stm = iter->first;
					iter->second.push_back(f);
				}
			}
		}
	}
}

/* update facts env for out-of-scope variables */
void 
update_facts_for_oos_vars(const vector<Variable*>& vars, FactVec& facts)
{
	//print_facts(facts);
	size_t i, j;
	// remove all facts related to vars, as they become irrelevant going out of scope
	for (i=0; i<vars.size(); i++) {
		const Variable* var = vars[i];
		size_t len = facts.size();
		for (j=0; j<len; j++) {
			//print_facts(facts);
			// remove all facts related to this variable
			const Fact* f = facts[j];
			if (f->get_var() == var) {
				facts.erase(facts.begin() + j);
				len--;
				j--;
			}
		}
	}
	// mark any remaining facts that may point to a out-of-scope variable as "point to garbage"
	for (i=0; i<vars.size(); i++) {
		const Variable* var = vars[i];
		for (j=0; j<facts.size(); j++) {
			if (facts[j]->eCat == ePointTo) {
				FactPointTo* f = (FactPointTo*)(facts[j]);
				FactPointTo* new_fact = f->mark_dead_var(var);
				if (new_fact) {
					facts[j] = new_fact;
				}
			}
		}
	}
}

void 
update_facts_for_oos_vars(const vector<const Variable*>& vars, FactVec& facts)
{
	//print_facts(facts);
	size_t i, j;
	// remove all facts related to vars, as they become irrelevant going out of scope
	for (i=0; i<vars.size(); i++) {
		const Variable* var = vars[i];
		size_t len = facts.size();
		for (j=0; j<len; j++) {
			//print_facts(facts);
			// remove all facts related to this variable
			const Fact* f = facts[j];
			if (f->get_var() == var) {
				facts.erase(facts.begin() + j);
				len--;
				j--;
			}
		}
	}
	// mark any remaining facts that may point to a out-of-scope variable as "point to garbage"
	for (i=0; i<vars.size(); i++) {
		const Variable* var = vars[i];
		for (j=0; j<facts.size(); j++) {
			if (facts[j]->eCat == ePointTo) {
				FactPointTo* f = (FactPointTo*)(facts[j]);
				FactPointTo* new_fact = f->mark_dead_var(var);
				if (new_fact) {
					facts[j] = new_fact;
				}
			}
		}
	}
}

/*
 * remove facts concerning variables local to a function
 * hint: relevant facts are those concerns variable visible at the end of this function
 */
void 
remove_function_local_facts(std::vector<const Fact*>& inputs, const Statement* stm)
{
	size_t i;
	size_t len = inputs.size();
	assert(stm->func);
	const Function* func = stm->func;
	// remove irrelevant facts
	for (i=0; i<len; i++) {
		const Variable* v = inputs[i]->get_var();
		// if it's fact for a local variable of this function, or return variable
		// of another function, we are not interested in them after exit function
		if (func->is_var_on_stack(v, stm) || (v->is_rv() && v != func->rv)) { 
			inputs.erase(inputs.begin() + i);
			i--;
			len--; 
		}
	}
	// mark any remaining facts that may point to a local vars of this 
	// function as "point to garbage"  
	for (i=0; i<inputs.size(); i++) {
		if (inputs[i]->eCat == ePointTo) {
			FactPointTo* f = (FactPointTo*)(inputs[i]);
			FactPointTo* new_fact = f->mark_func_end(stm);
			if (new_fact) {
				inputs[i] = new_fact;
			}
		}
	}
}

void
FactMgr::setup_in_out_maps(bool first_time)
{
	if (first_time) {
		// first time revisit, create map_facts_in_final and map_facts_out_final with cloned facts 
		map<const Statement*, vector<const Fact*> >::const_iterator iter;
		for(iter = map_facts_in.begin(); iter != map_facts_in.end(); ++iter) {
			const Statement* stm = iter->first;
			const vector<const Fact*>& facts1 = iter->second;
			map_facts_in_final[stm] = copy_facts(facts1);
		}    
		for(iter = map_facts_out.begin(); iter != map_facts_out.end(); ++iter) {
			const Statement* stm = iter->first;
			const vector<const Fact*>& facts1 = iter->second; 
			map_facts_out_final[stm] = copy_facts(facts1);
		}    
	}
	else {  
		// not the 1st time revisit
		// combine facts_in and facts_out from this invocation with facts from previous invocations
		map<const Statement*, vector<Fact*> >::iterator iter;
		for(iter = map_facts_in_final.begin(); iter != map_facts_in_final.end(); ++iter) {
			const Statement* stm = iter->first;
			vector<Fact*>& facts1 = iter->second;
			const FactVec& facts2 = map_facts_in[stm]; 
			combine_facts(facts1, facts2);
		}    
		for(iter = map_facts_out_final.begin(); iter != map_facts_out_final.end(); ++iter) {
			const Statement* stm = iter->first;
			vector<Fact*>& facts1 = iter->second;
			const FactVec& facts2 = map_facts_out[stm];
			combine_facts(facts1, facts2);
		}     
	}
	//JYTODO: beef up the sanity check
	//sanity_check_map();
}

void 
FactMgr::set_fact_in(const Statement* s, const FactVec& facts)
{
	map_facts_in[s] = facts;
}

/* 
 * special treatment for "contine" and "break", their output env should has no facts
 * of local variables in the loop. For return, there should be no fact for local varibles
 * of the function
 */
void 
FactMgr::set_fact_out(const Statement* s, const FactVec& facts)
{
	if (s->eType == eContinue || s->eType == eBreak) {
		FactVec facts_copy = facts;
		remove_loop_local_facts(s, facts_copy);
		map_facts_out[s] = facts_copy;
	}
	else if (s->eType == eGoto) {
		const StatementGoto* sg = dynamic_cast<const StatementGoto*>(s);
		FactVec facts_copy; 
		update_facts_for_dest(facts, facts_copy, sg->dest); 
		map_facts_out[s] = facts_copy;
	}
	else if (s->eType == eReturn || s->parent==NULL) {
		FactVec facts_copy = facts;
		remove_function_local_facts(facts_copy, s);
		map_facts_out[s] = facts_copy;
	} 
	else {
		map_facts_out[s] = facts;
	}
}

/*
 * add a fact to facts_out of a statement, discard the fact
 * if it becomes irrelevant at the end of this statement.
 * such as local facts after return
 */
void 
FactMgr::add_fact_out(const Statement* stm, const Fact* fact)
{
	const Variable* var = fact->get_var();
	if (func->is_var_visible(var, stm)) {
		if (stm->eType == eReturn && !var->is_global()) return;
		if (stm->eType == eBreak || stm->eType == eContinue) {
			Block* b;
			for (b=stm->parent; b && !b->looping; b=b->parent) {
				/* Empty. */
			}
			if (!func->is_var_visible(var, b)) {
				return;
			}
		}
		if (stm->eType == eGoto) {
			const StatementGoto* sg = (const StatementGoto*)stm;
			if (!func->is_var_visible(var, sg->dest)) {
				return;
			}
		}
		map_facts_out[stm].push_back(fact);
	}
}

/*
 * add:			parameter facts
 * remove:		facts concerning local variables
 * exception:	facts can be indirectly accessed through pointers
 *				for example: { int i; func(&i)}. The facts of i will not be removed
 */
void 
FactMgr::caller_to_callee_handover(const FunctionInvocationUser* fiu, std::vector<const Fact*>& inputs)
{
	// add parameter facts
	add_param_facts(fiu->param_value, inputs);

	size_t i, j, cnt;
	std::vector<const Fact*> keep_facts;
	size_t len = inputs.size();
	// move global facts and parameter facts to a separate "keep" list
	for (i=0; i<len; i++) {
		const Variable* v = inputs[i]->get_var();
		if (v->is_global() || find_variable_in_set(func->param, v) >=0) {
			keep_facts.push_back(inputs[i]);
			inputs.erase(inputs.begin() + i);
			i--;
			len--;
		}
	}
	// find all the facts for variables that might be pointed to by variables we already found.
	// this include variables on stack (most likely locals of callers) but invisible to 
	// this function
	do {
		cnt = keep_facts.size(); 
		for (i=0; i<len; i++) {
			const Fact* f = inputs[i]; 
			// const Variable* v = f->get_var(); 
			for (j=0; j<keep_facts.size(); j++) {
				if (keep_facts[j]->eCat == ePointTo) {
					const FactPointTo* fp = dynamic_cast<const FactPointTo*>(keep_facts[j]);
					if (fp->point_to(f->get_var())) {
						keep_facts.push_back(f); 
						inputs.erase(inputs.begin() + i);
						i--;
						len--;
						break;
					}
				}
			}
		}
	} while (keep_facts.size() > cnt); 
	inputs = keep_facts;
}

/*
 * remove facts related to return variables of other functions
 */
void FactMgr::remove_rv_facts(FactVec& facts)
{
	size_t len = facts.size();
	for (size_t i=0; i<len; i++) { 
		const Fact* f = facts[i]; 
		if (f->get_var()->is_rv() && f->get_var() != func->rv) {
			facts.erase(facts.begin() + i);
			len--;
			i--; 
		}
	}
}

bool 
update_fact_for_assign(const Lhs* lhs, const Expression* rhs, FactVec& inputs)
{
	bool changed = false;
    for (size_t i=0; i<FactMgr::meta_facts.size(); i++) {
        vector<const Fact*> facts = FactMgr::meta_facts[i]->abstract_fact_for_assign(inputs, lhs, rhs);
		if (facts.size() == 1 && !facts[0]->get_var()->isArray) { 
			// for must-point-to fact concerning no-array variable, just renew the old fact
			renew_fact(inputs, facts[0]); 
		}
		else {                   
			// for may-point-to facts (which means pointer on LHS is uncertain), merge them with old facts
			for (size_t j=0; j<facts.size(); j++) {
				const Fact* f = facts[j];
				merge_fact(inputs, f);
			} 
		}
		if (facts.size() > 0) {
			changed = true;
		}
    }
	return changed;
}

bool 
update_fact_for_assign(const StatementAssign* sa, FactVec& inputs)
{
	if (update_fact_for_assign(sa->get_lhs(), sa->get_rhs(), inputs)) { 
		sa->func->fact_changed = true;
		return true;
    }
	return false;
}

void 
update_fact_for_return(const StatementReturn* sr, FactVec& inputs)
{
	size_t i;
    for (i=0; i<FactMgr::meta_facts.size(); i++) {
        Fact* f = FactMgr::meta_facts[i]->abstract_fact_for_return(inputs, sr->get_var(), sr->func);
		if (f) {
			// merge with other return statements
			merge_fact(inputs, f);
			sr->func->fact_changed = true;
		}
    }
	// incorporate current facts into return facts
	FactMgr* fm = get_fact_mgr_for_func(sr->func);
	fm->set_fact_out(sr, inputs);
	//merge_facts(fm->return_facts, fm->map_facts_out[sr]);
}

void
update_facts_for_dest(const FactVec& facts_in, FactVec& facts_out, const Statement* dest)
{
	size_t i, j;
	vector<const Variable*> oos_vars;
	const Function* func = dest->func;
	assert(func);
	// find all the variales that are out-of-scope after jump to destination
	// oos variables are those not on stack and not global
	for (i=0; i<facts_in.size(); i++) { 
		const Fact* f = facts_in[i];
		const Variable* var = f->get_var();
		// return variable, target site don't need them
		if (!var || var->is_rv()) continue; 
		if (func->is_var_oos(var, dest)) {
			if (find_variable_in_set(oos_vars, var) == -1) {
				oos_vars.push_back(var);
			}
		}
		if (f->eCat == ePointTo) {
			const FactPointTo* fp = dynamic_cast<const FactPointTo*>(f);
			for (j=0; j<fp->get_point_to_vars().size(); j++) {
				const Variable* v = fp->get_point_to_vars()[j];
				if (!FactPointTo::is_special_ptr(v) && func->is_var_oos(v, dest)) {
					if (find_variable_in_set(oos_vars, v) == -1) {
						oos_vars.push_back(v);
					}
				}
			}
		}
		merge_fact(facts_out, f);
	}
	update_facts_for_oos_vars(oos_vars, facts_out);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * 
 */
FactMgr::FactMgr(const Function* f)
: func(f)
{ 
}

/*
 * 
 */
FactMgr::~FactMgr(void)
{
	size_t i;
	for (i=0; i<cfg_edges.size(); i++) {
		delete cfg_edges[i];
	}
	cfg_edges.clear();
}

void
FactMgr::add_interested_facts(int interests)
{
 	// create meta facts for subsequent iterations
    if (interests & ePointTo) {
        //meta_facts.push_back(new FactPointTo(0));
        //meta_facts.push_back(FactPointTo::make_fact(0));
        FactPointTo *fp = FactPointTo::make_fact(0);
        meta_facts.push_back(fp);
	}
	if (interests & eUnionWrite) {
		FactUnion *fu = FactUnion::make_fact(0, 0);
		meta_facts.push_back(fu);
    } 
} 

void 
FactMgr::backup_facts(void) 
{
	shadow_facts = global_facts;
}

void 
FactMgr::restore_facts(void)
{
	global_facts = shadow_facts;
}

void 
FactMgr::restore_facts(vector<const Fact*>& old_facts)
{
	makeup_new_var_facts(old_facts, global_facts);
	global_facts = old_facts;
}

void 
FactMgr::makeup_new_var_facts(vector<const Fact*>& old_facts, const vector<const Fact*>& new_facts)
{
    size_t i; 
    for (i=0; i<new_facts.size(); i++) { 
		const Fact* f = new_facts[i];
		// if there are variable facts not present in old facts,
		// mean they are variables created after old_facts,
		// manually add them
		if (find_related_fact(old_facts, f) == 0) { 
			add_new_var_fact(f->get_var(), old_facts);
		}
	}
}

void 
FactMgr::clear_map_visited(void)
{
	map<const Statement*, bool>::iterator iter;
	for(iter = map_visited.begin(); iter != map_visited.end(); ++iter) {  
		iter->second = false;
	}
}

void
FactMgr::backup_stm_fact_maps(const Statement* stm, map<const Statement*, FactVec>& facts_in, map<const Statement*, FactVec>& facts_out)
{
	vector<const Block*> blks;
	stm->get_blocks(blks);
	for (size_t i=0; i<blks.size(); i++) { 
		const Block* b = blks[i];
		facts_in[b] = map_facts_in[b];
		facts_out[b] = map_facts_out[b];
		for (size_t j=0; j<b->stms.size(); j++) {
			backup_stm_fact_maps(b->stms[j], facts_in, facts_out);
		}
	}
	facts_in[stm] = map_facts_in[stm];
	facts_out[stm] = map_facts_out[stm];
}

void
FactMgr::restore_stm_fact_maps(const Statement* stm, map<const Statement*, FactVec>& facts_in, map<const Statement*, FactVec>& facts_out)
{
	vector<const Block*> blks;
	stm->get_blocks(blks);
	for (size_t i=0; i<blks.size(); i++) { 
		const Block* b = blks[i];
		map_facts_in[b] = facts_in[b];
		map_facts_out[b] = facts_out[b];
		for (size_t j=0; j<b->stms.size(); j++) {
			restore_stm_fact_maps(b->stms[j], facts_in, facts_out);
		}
	}
	map_facts_in[stm] = facts_in[stm];
	map_facts_out[stm] = facts_out[stm];
}

/*
 * reset input/output env of this statement and all statements included to empty
 */
void
FactMgr::reset_stm_fact_maps(const Statement* stm)
{
	FactVec empty;
	vector<const Block*> blks;
	stm->get_blocks(blks);
	for (size_t i=0; i<blks.size(); i++) { 
		const Block* b = blks[i];
		map_facts_in[b] = empty;
		map_facts_out[b] = empty;
		for (size_t j=0; j<b->stms.size(); j++) {
			reset_stm_fact_maps(b->stms[j]);
		}
	}
	map_facts_in[stm] = empty;
	map_facts_out[stm] = empty;
}

int 
find_fact(const FactVec& facts, const Fact* fact)
{
    size_t i; 
    for (i=0; i<facts.size(); i++) {
        if (fact->equal(*facts[i])) {
            return i;
        }
    }
    return -1;
}

const Fact* 
find_related_fact(const FactVec& facts, const Fact* new_fact)
{
    size_t i; 
    for (i=0; i<facts.size(); i++) {
        if (new_fact->is_related(*facts[i])) {
            return facts[i];
        }
    }
    return 0;
}

const Fact* 
find_related_fact(const vector<Fact*>& facts, const Fact* new_fact)
{
    size_t i; 
    for (i=0; i<facts.size(); i++) {
        if (new_fact->is_related(*facts[i])) {
            return facts[i];
        }
    }
    return 0;
}

// TODO: we really need to free the memory properly while maintain the memory in compact
// way, i.e., don't allocate a Fact object unless it's absolutely necessary
bool 
merge_fact(FactVec& facts, const Fact* new_fact)
{ 
    bool changed = false; 
    size_t i; 
    for (i=0; i<facts.size(); i++) {
        const Fact* f = facts[i]; 
        if (f->is_related(*new_fact)) {
            if (!f->imply(*new_fact)) {
				Fact* copy_fact = new_fact->clone();
                copy_fact->join(*f);    
                facts[i] = copy_fact;
                changed = true;
				// TODO: release the memory for f???
            } 
            else {
                //delete new_fact;   // new fact is useless, unsafe to do so???
            }
            break;
        }
    }  
    if (i == facts.size()) {// if not found 
        facts.push_back(new_fact);
        changed = true;
    } 
    return changed;
}

bool
renew_fact(FactVec& facts, const Fact* new_fact)
{ 
    size_t i; 
    for (i=0; i<facts.size(); i++) {
        if (new_fact->is_related(*facts[i])) { 
            if (new_fact->equal(*facts[i])) {
                return false;
            }
            facts[i] = new_fact; 
            break;
        }
    } 
    // if not found, append the new fact
    if (i==facts.size()) {
        facts.push_back(new_fact);
    }
    return true;
}
   
bool
merge_facts(FactVec& facts, const FactVec& new_facts)
{ 
    size_t i;
    bool changed = false;
    for (i=0; i<new_facts.size(); i++) {
		//new_facts[i]->Output(cout);
        if (merge_fact(facts, new_facts[i])) {
            changed = true;
        }
    } 
    return changed;
}

bool
merge_jump_facts(FactVec& facts, const FactVec& jump_facts)
{ 
    size_t i;
    bool changed = false;
    for (i=0; i<facts.size(); i++) {
		const Fact* f = facts[i]; 
		if (!f->get_var()->is_rv()) {
			const Fact* jump_f = find_related_fact(jump_facts, f);
			// this should not happen: jump over initializers
			if (jump_f == 0) {
				if (f->eCat == ePointTo) {
					jump_f = FactPointTo::make_fact(f->get_var(), FactPointTo::garbage_ptr);
				} 
				else if (f->eCat == eUnionWrite) {
					jump_f = FactUnion::make_fact(f->get_var(), FactUnion::BOTTOM);
				}
			}
			if (jump_f && merge_fact(facts, jump_f)) {
				changed = true;
			}
		}
    } 
    return changed;
}

bool
merge_prev_facts(FactVec& facts, FactVec& old_facts)
{ 
    size_t i; 
    for (i=0; i<facts.size(); i++) { 
		const Fact* f = facts[i];
		// if there are variable facts not present in old facts,
		// mean they are variables created after old_facts,
		// manually add them before we do a merge
		if (find_related_fact(old_facts, f) == 0) {
			add_new_var_fact(f->get_var(), old_facts);
		}
	}
	return merge_facts(facts, old_facts);
}

bool
renew_facts(FactVec& facts, const FactVec& new_facts)
{ 
    size_t i;
    bool changed = false;
    for (i=0; i<new_facts.size(); i++) {
        if (renew_fact(facts, new_facts[i])) {
            changed = true;
        }
    } 
    return changed;
}

vector<Fact*>
copy_facts(const FactVec& facts_in)
{ 
    size_t i; 
	vector<Fact*> facts_out;
    for (i=0; i<facts_in.size(); i++) {
		Fact* f = facts_in[i]->clone();
        facts_out.push_back(f);
    } 
	return facts_out;
}

/*************************************************************
 * combine facts obtained from two visits to the same function
 *************************************************************/
void 
combine_facts(vector<Fact*>& facts1, const FactVec& facts2)
{
    size_t i, j;  
    for (i=0; i<facts2.size(); i++) {
		const Fact* new_fact = facts2[i];
		for (j=0; j<facts1.size(); j++) {
			Fact* old_fact = facts1[j];
			if (old_fact->is_related(*new_fact)) {
				old_fact->join_visits(*new_fact);
				break;
			}
		}
	}
}

bool 
same_facts(const FactVec& facts1, const FactVec& facts2)
{
	if (facts1.size() == facts2.size()) {
		size_t i;
		for (i=0; i<facts1.size(); i++) {
			if (find_fact(facts2, facts1[i]) == -1) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool 
subset_facts(const FactVec& facts1, const FactVec& facts2)
{
	if (facts1.size() == facts2.size()) {
		size_t i;
		for (i=0; i<facts1.size(); i++) {
			const Fact* f1 = facts1[i];
			const Fact* f2 = find_related_fact(facts2, f1);
			if (f2 == 0 || !f2->imply(*f1)) {
				return false;
			} 
		}
		return true;
	}
	return false;
}

/*
 * create a control flow graph edge introduced by break/continue/goto
 */
void 
FactMgr::create_cfg_edge(const Statement* src, const Statement* dest, bool post_dest, bool is_back_link)
{
	CFGEdge* edge = new CFGEdge(src, dest, post_dest, is_back_link);
	cfg_edges.push_back(edge);
}

void
remove_loop_local_facts(const Statement* s, FactVec& facts)
{
	// filter out out-of-scope facts 
	const Block* b = (s->eType==eBlock) ? (const Block*)s : s->parent; 
	vector<Variable*> local_vars = b->local_vars;
	while (b && !b->looping) {
		b = b->parent;
		local_vars.insert(local_vars.end(), b->local_vars.begin(), b->local_vars.end());
	} 
	update_facts_for_oos_vars(local_vars, facts);
}

void
FactMgr::output_assertions(std::ostream &out, const Statement* stm, int indent, bool post_condition)
{
	vector<Fact*> facts;  
	if (!post_condition) {
		facts = map_facts_in_final[stm];
	} else {
		find_updated_final_facts(stm, facts);
	}
	if (facts.empty()) return;

	if (stm->eType == eFor || stm->eType == eIfElse) {
		output_tab(out, indent);
		std::ostringstream ss; 
		ss << "facts after " << (stm->eType == eFor ? "for loop" : "branching");
		output_comment_line(out, ss.str());
	}
	if (stm->eType == eAssign || stm->eType == eInvoke || stm->eType == eReturn) {
		output_tab(out, indent);
		std::ostringstream ss; 
		ss << "statement id: " << stm->stm_id;
		output_comment_line(out, ss.str());
	}
	for (size_t i=0; i<facts.size(); i++) {
		const Fact* f = facts[i];
		const Effect& eff = func->feffect;
		const Variable* v = f->get_var();
		assert(v);
		// don't print facts regarding global variables that are neither read or written in this function
		if (v->is_global() && !eff.is_read(v) && !eff.is_written(v)) {
			continue;
		}
		output_tab(out, indent); 
		f->OutputAssertion(out, stm);
	}
}

void 
FactMgr::find_updated_facts(const Statement* stm, vector<const Fact*>& facts)
{
	const FactVec& facts_in = map_facts_in[stm]; 
	const FactVec& facts_out = map_facts_out[stm]; 
	  
	for (size_t i=0; i<facts_out.size(); i++) {
		const Fact* f = facts_out[i];
		const Fact* prev_f = find_related_fact(facts_in, f);
		assert(prev_f);
		if (!f->equal(*prev_f)) {
			facts.push_back(f);
		}
	}
}

void 
FactMgr::find_updated_final_facts(const Statement* stm, vector<Fact*>& facts)
{
	vector<Fact*>& facts_in = map_facts_in_final[stm]; 
	vector<Fact*>& facts_out = map_facts_out_final[stm]; 
	  
	for (size_t i=0; i<facts_out.size(); i++) {
		Fact* f = facts_out[i];
		// sometimes there is no pre-facts for return variables, so we don't
		// check the difference
		if (f->get_var() == func->rv) {
			facts.push_back(f);
		} 
		else {
			const Fact* prev_f = find_related_fact(facts_in, f);
			assert(prev_f);
			if (!f->equal(*prev_f)) {
				facts.push_back(f);
			}
		}
	}
}

void
FactMgr::find_dangling_global_ptrs(Function* f)
{
	for (size_t i=0; i<global_facts.size(); i++) { 
		if (global_facts[i]->eCat == ePointTo) {
			FactPointTo* fp = (FactPointTo*)(global_facts[i]);
			const Variable* v = fp->get_var(); 
			// const pointers should never be dangling
			if (v->is_const() || !v->is_global()) continue;   
			if (fp->is_dead()) {
				f->dead_globals.push_back(v);
			}
		}
	}
}

void
print_facts(const FactVec& facts)
{
	for (size_t i=0; i<facts.size(); i++) {
		const Fact* f = facts[i];
		f->OutputAssertion(cout);
	}
}

void
print_var_fact(const FactVec& facts, const char* vname)
{
	for (size_t i=0; i<facts.size(); i++) {
		const Fact* f = facts[i];
		if (f->get_var()->name == vname) {
			f->OutputAssertion(cout);
		}
	}
}

void
FactMgr::sanity_check_map() const
{
	map<const Statement*, vector<const Fact*> >::const_iterator iter; 
	for(iter = map_facts_in.begin(); iter != map_facts_in.end(); ++iter) {
		const Statement* stm = iter->first;
		const vector<const Fact*>& facts = iter->second;
		for (size_t i=0; i<facts.size(); i++) {
			const Variable* v = facts[i]->get_var();
			if (!v->is_visible(stm->parent)) {
				// exception: the input facts to a function body could include the parameter facts 
				if (stm->parent == 0 && find_variable_in_set(func->param, v) != -1) {
					continue;
				}
				//assert(0);
			}
		}
	} 
		
	for(iter = map_facts_out.begin(); iter != map_facts_out.end(); ++iter) {
		const Statement* stm = iter->first;
		const vector<const Fact*>& facts = iter->second;
		for (size_t i=0; i<facts.size(); i++) {
			const Variable* v = facts[i]->get_var();
			if (!v->is_visible(stm->parent) && v != func->rv) {
				//assert(0);
			}
		}
	}
}

void
FactMgr::doFinalization()
{
	Fact::doFinalization();
	meta_facts.clear();
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

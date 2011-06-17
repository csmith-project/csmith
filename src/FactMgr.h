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

#ifndef FACT_MGR_H
#define FACT_MGR_H

class StatementAssign;
class StatementIf;
class StatementReturn;
class StatementFor;
class StatementExpr;
class Block;
class Expression;
class Fact;
class Variable;
class Expression;
class Statement;
class Function;
class FunctionInvocationUser;
class CGContext;
class Lhs;
class CFGEdge;

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <vector>
#include <map>
#include "Effect.h"
using namespace std; 

typedef std::vector<const Fact*> FactVec;
typedef FactVec* FactVecP;

///////////////////////////////////////////////////////////////////////////////

class FactMgr
{
public:
	FactMgr(const Function* f);

	FactMgr(const Function* f, const FactVec& facts);
    
	static void doFinalization();

	static void add_interested_facts(int interests);

	~FactMgr(void); 

	//FactMgr* clone(void);
	
	bool validate_fact(const Fact* f, const FactVec& facts);

	bool validate_assign(const Lhs* v, const Expression* e);

	void backup_facts(void);

	void restore_facts(void);

	void restore_facts(vector<const Fact*>& old_facts);

	void makeup_new_var_facts(vector<const Fact*>& old_facts, const vector<const Fact*>& new_facts);

	/* add fact related to newly created var to all statements  this var is visible */
	void add_new_local_var_fact(const Block* blk, const Variable* var);
 
	void add_new_global_var_fact(const Variable* v);

	void setup_in_out_maps(bool first_time);

	void set_fact_in(const Statement* s, const FactVec& facts);
	void set_fact_out(const Statement* s, const FactVec& facts);
	void add_fact_out(const Statement* stm, const Fact* fact);

	void create_cfg_edge(const Statement* src, const Statement* dest, bool post_stm_edge, bool back_link);

	void clear_map_visited(void);
	void backup_stm_fact_maps(const Statement* stm, map<const Statement*, FactVec>& facts_in, map<const Statement*, FactVec>& facts_out);
	void restore_stm_fact_maps(const Statement* stm, map<const Statement*, FactVec>& facts_in, map<const Statement*, FactVec>& facts_out);
	void reset_stm_fact_maps(const Statement* stm);

	void output_assertions(std::ostream &out, const Statement* stm, int indent, bool post_condition);
	void find_updated_final_facts(const Statement* stm, vector<Fact*>& facts);
	void find_updated_facts(const Statement* stm, vector<const Fact*>& facts);

	void find_dangling_global_ptrs(Function* f);

	/* add paramters facts to env */
	void add_param_facts(const vector<const Expression*>& param_values, FactVec& facts);
	void caller_to_callee_handover(const FunctionInvocationUser* fiu, std::vector<const Fact*>& inputs);

	/* remove facts related to return variables (except rv of this function) from env */
	void remove_rv_facts(FactVec& facts);

	void sanity_check_map() const;

	static std::vector<Fact*> meta_facts; 

	std::map<const Statement*, FactVec> map_facts_in;
	std::map<const Statement*, FactVec> map_facts_out;
	std::map<const Statement*, std::vector<Fact*> > map_facts_in_final;
	std::map<const Statement*, std::vector<Fact*> > map_facts_out_final;
	std::map<const Statement*, Effect> map_stm_effect;
	std::map<const Statement*, Effect> map_accum_effect;
	std::map<const Statement*, bool> map_visited;
	std::vector<const CFGEdge*> cfg_edges;
	FactVec global_facts;
	FactVec fixed_facts;
	//FactVec return_facts;
	FactVec shadow_facts;

	const Function* func;
};

/******************* Fact Manipulating Functions **********************/

/* find a fact from facts env */
int find_fact(const FactVec& facts, const Fact* fact);  

/* find a specific type of fact (same variable most likely) from facts env */
const Fact* find_related_fact(const FactVec& facts, const Fact* new_fact);  
const Fact* find_related_fact(const vector<Fact*>& facts, const Fact* new_fact);

/* merge a fact into env */
bool merge_fact(FactVec& facts, const Fact* new_fact);

/* merge two facts env */
bool merge_facts(FactVec& facts, const FactVec& new_facts);

/* merge two facts env */
bool merge_jump_facts(FactVec& facts, const FactVec& jump_facts);

/* merge one env with a prev env that may miss new variables created recently */
bool merge_prev_facts(FactVec& facts, FactVec& old_facts);

/* check if two facts env are identical */
bool same_facts(const FactVec& facts1, const FactVec& facts2);

/* check if one facts env is a subset of the other */
bool subset_facts(const FactVec& facts1, const FactVec& facts2);

/* renew a fact in env (append is absent) */
bool renew_fact(FactVec& facts, const Fact* new_fact);

/* renew facts in new_facts into existing facts env */
bool renew_facts(FactVec& facts, const FactVec& new_facts);

/* clone an env */
vector<Fact*> copy_facts(const FactVec& facts_in);

/* combine facts in two env, discard facts not exist in one of them */
void combine_facts(vector<Fact*>& facts1, const FactVec& facts2);

/* print facts env */
void print_facts(const FactVec& facts);

/* print fact(s) in env regarding a given variable */
void print_var_fact(const FactVec& facts, const char* vname);

/* add a new variable fact to env */
void add_new_var_fact(const Variable* v, FactVec& facts);

/* remove facts related to certain variables from env */
void update_facts_for_oos_vars(const vector<Variable*>& vars, FactVec& facts);
void update_facts_for_oos_vars(const vector<const Variable*>& vars, FactVec& facts);

/* remove facts localized to a given function up to a given return statement */
void remove_function_local_facts(std::vector<const Fact*>& inputs, const Statement* stm);

void remove_loop_local_facts(const Statement* s, FactVec& facts);

/* update fact(s) from assignments */
bool update_fact_for_assign(const StatementAssign* sa, FactVec& inputs);
bool update_fact_for_assign(const Lhs* lhs, const Expression* rhs, FactVec& inputs);

/* update facts(s) from return statement */
void update_fact_for_return(const StatementReturn* sa, FactVec& inputs);

/* update fact(s) for a jump destination */
void update_facts_for_dest(const FactVec& facts_in, FactVec& facts_out, const Statement* dest);

///////////////////////////////////////////////////////////////////////////////

#endif // FACT_MGR_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

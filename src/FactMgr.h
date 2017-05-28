// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2015 The University of Utah
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
#include "Fact.h"
using namespace std;

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

	void restore_facts(vector<const Fact*>& old_facts);

	void makeup_new_var_facts(vector<const Fact*>& old_facts, const vector<const Fact*>& new_facts);

	void add_new_var_fact_and_update_inout_maps(const Block* blk, const Variable* var);

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

	static void remove_loop_local_facts(const Statement* s, FactVec& facts);
	/* remove facts localized to a given function up to a given return statement */
	static void remove_function_local_facts(std::vector<const Fact*>& inputs, const Statement* stm);
	static bool merge_jump_facts(FactVec& facts, const FactVec& jump_facts);
	/* add a new variable fact to env */
	static void add_new_var_fact(const Variable* v, FactVec& facts);
	static const vector<const Fact*>& get_program_end_facts(void);

	/* remove facts related to certain variables from env */
	static void update_facts_for_oos_vars(const vector<Variable*>& vars, FactVec& facts);
	static void update_facts_for_oos_vars(const vector<const Variable*>& vars, FactVec& facts);

	/* update fact(s) from assignments */
	static bool update_fact_for_assign(const StatementAssign* sa, FactVec& inputs);
	static bool update_fact_for_assign(const Lhs* lhs, const Expression* rhs, FactVec& inputs);

	/* update facts(s) from return statement */
	static void update_fact_for_return(const StatementReturn* sa, FactVec& inputs);

	/* update fact(s) for a jump destination */
	static void update_facts_for_dest(const FactVec& facts_in, FactVec& facts_out, const Statement* dest);

	void sanity_check_map() const;

	static std::vector<Fact*> meta_facts;

	// maps to track facts and effects at historical generation points.
	// they are used for bypassing analyzing statements if possible
	std::map<const Statement*, FactVec> map_facts_in;
	std::map<const Statement*, FactVec> map_facts_out;
	std::map<const Statement*, std::vector<Fact*> > map_facts_in_final;
	std::map<const Statement*, std::vector<Fact*> > map_facts_out_final;
	std::map<const Statement*, Effect> map_stm_effect;
	std::map<const Statement*, Effect> map_accum_effect;
	std::map<const Statement*, bool> map_visited;

	std::vector<const CFGEdge*> cfg_edges;
	FactVec global_facts;

	const Function* func;
};

///////////////////////////////////////////////////////////////////////////////

#endif // FACT_MGR_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2015 The University of Utah
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

#ifndef VARIABLE_SELECTOR_H
#define VARIABLE_SELECTOR_H

///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
using namespace std;
#include "Variable.h"
#include "Type.h"

class CGContext;
class Expression;
class Function;
class Block;
class Lhs;
class Fact;
class CVQualifiers;
class ArrayVariable;

enum eVariableScope
{
	eGlobal,
	eParentLocal,
	eParentParam,
	eNewValue,
	MAX_VAR_SCOPE
};

class VariableSelector
{
	friend class ArrayVariable;
public:
	VariableSelector(void) {};
	static Variable* new_variable(const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer);
	// ISSUE: use it only when you want to create a static variable
	static Variable *make_dummy_static_variable(const string &name);

	static Variable* select(Effect::Access access, const CGContext &cg_context,
             const Type* type, const CVQualifiers* qfer,
			 const vector<const Variable*>& invalid_vars, eMatchType mt,
			 eVariableScope scope=MAX_VAR_SCOPE);
	static Variable* choose_ok_var(const vector<Variable *> &vars);
	static const Variable* choose_ok_var(const vector<const Variable *> &vars);
	static const Variable* choose_visible_read_var(const Block* b, vector<const Variable*> written_vars, const Type* type, const vector<const Fact*>& facts);
	static Variable* choose_var(vector<Variable *> vars, Effect::Access access,
		   const CGContext &cg_context, const Type* type, const CVQualifiers* qfer,
		   eMatchType mt, const vector<const Variable*>& invalid_vars, bool no_bitfield = false, bool no_expand_struct = false);
	static Variable *select_deref_pointer(Effect::Access access, const CGContext &cg_context, const Type* type,
		   const CVQualifiers* qfer, const vector<const Variable*>& invalid_vars);
	static Variable *SelectLoopCtrlVar(const CGContext &cg_context, const vector<const Variable*>& invalid_vars);
	static Block* expand_block_for_goto(Block* b, const CGContext& cg_context);
	static Block* lower_block_for_vars(const vector<Block*>& blks, vector<const Variable*>& vars);

	static Expression* make_init_value(Effect::Access access, const CGContext &cg_context, const Type* t, const CVQualifiers* qfer, Block* b);
	static ArrayVariable* create_mutated_array_var(const ArrayVariable* av, const vector<const Expression*>& new_indices);
	static const Variable* select_must_use_var(Effect::Access access, CGContext &cg_context, const Type* type, const CVQualifiers* qfer);

	static ArrayVariable* select_array(const CGContext &cg_context);
	static ArrayVariable*  itemize_array(CGContext &cg_context, const ArrayVariable* av);
	//static const ArrayVariable* select_random_focus_var(Effect::Access access, const CGContext &cg_context, const Type* type, const CVQualifiers* qfer, const vector<const Variable*>& invalid_vars, eMatchType mt);

	~VariableSelector(void);
	///////////////////////////////////////////////////////////////////////

	static void GenerateParameterVariable(Function &curFunc);
	static Variable* GenerateParameterVariable(const Type *type, const CVQualifiers *qfer);
	static std::vector<Variable *>* GetGlobalVariables(void) {return &GlobalList;}
	static void doFinalization(void);
	static void expand_struct_union_vars(vector<const Variable *>& vars, const Type* type);

	static ProbabilityTable<unsigned int, eVariableScope> * scopeTable_;
	static void InitScopeTable();

	static vector<Variable*> find_all_visible_vars(const Block* b);
	static void get_all_local_vars(const Block* b, vector<const Variable *> &vars);
	static const Variable* find_var_by_name(string name);

private:
	static ArrayVariable* create_array_and_itemize(Block* blk, string name, const CGContext& cg_context, const Type* t, const Expression* init, const CVQualifiers* qfer);

	static ArrayVariable* create_random_array(const CGContext& cg_context);

	static Variable* eager_create_global_struct(Effect::Access access, const CGContext &cg_context,
					const Type* type, const CVQualifiers* qfer,
					eMatchType mt, const vector<const Variable*>& invalid_vars);

	static Variable* eager_create_local_struct(Block &block, Effect::Access access, const CGContext &cg_context,
					const Type* type, const CVQualifiers* qfer,
					eMatchType mt, const vector<const Variable*>& invalid_vars);

	static Variable* SelectGlobal(Effect::Access access, const CGContext &cg_context, const Type* type,
			const CVQualifiers* qfer, eMatchType mt, const vector<const Variable*>& invalid_vars);

	static Variable* SelectParentLocal(Effect::Access access, const CGContext &cg_context, const Type* type,
			const CVQualifiers* qfer, eMatchType mt, const vector<const Variable*> &invalid_vars);

	static Variable* SelectParentParam(Effect::Access access, const CGContext &cg_context, const Type* type,
			const CVQualifiers* qfer, eMatchType mt, const vector<const Variable*> &invalid_vars);

	static Variable* GenerateNewVariable(Effect::Access access, const CGContext &cg_context, const Type* type, const CVQualifiers* qfer);

	static Variable* GenerateNewGlobal(Effect::Access access, const CGContext &cg_context, const Type* t, const CVQualifiers* qfer);

	static Variable* GenerateNewNonArrayGlobal(Effect::Access access, const CGContext &cg_context, const Type* t, const CVQualifiers* qfer);

	static Variable* GenerateNewParentLocal(Block &block, Effect::Access access, const CGContext &cg_context, const Type* type, const CVQualifiers* qfer);

	static void get_all_array_vars(vector<const Variable*> &array_vars);

	static void find_all_non_bitfield_visible_vars(const Block* b, vector<Variable*> &vars);

	static void find_all_non_array_visible_vars(const Block* b, vector<Variable*> &vars);

	static void expand_struct_union_vars(vector<Variable *>& vars, const Type* type);

	static bool has_dereferenceable_var(const vector<Variable *>& vars, const Type* type, const CGContext& cg_context);

	static bool has_eligible_volatile_var(const vector<Variable *>& vars, const Type* type, const CVQualifiers* qfer, Effect::Access access, const CGContext& cg_context);

	static bool is_eligible_var(const Variable* var, int deref_level, Effect::Access access, const CGContext& cg_context);

	static Variable * create_and_initialize(Effect::Access access, const CGContext &cg_context, const Type* t,
					const CVQualifiers* qfer, Block *blk, std::string name);

	// all variables generated
	static vector<Variable*> AllVars;

	// All globals, including volatiles.
	static vector<Variable*> GlobalList;

	// All the non-volatile globals.
	static vector<Variable*> GlobalNonvolatilesList;

	// flag that indicates whether a new variable has been created
	static bool var_created;
};

void OutputGlobalVariables(std::ostream &);
void OutputGlobalVariablesDecls(std::ostream &, std::string prefix = "");
void HashGlobalVariables(std::ostream &);

///////////////////////////////////////////////////////////////////////////////

#endif // VARIABLE_SELECTOR_H

// Local VariableSelectors:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.


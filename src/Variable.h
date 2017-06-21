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

//
// This file was derived from a random program generator written by Bryan
// Turner.  The attributions in that file was:
//
// Random Program Generator
// Bryan Turner (bryan.turner@pobox.com)
// July, 2005
//

#ifndef VARIABLE_H
#define VARIABLE_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include "Effect.h"
#include "Type.h"
#include "CVQualifiers.h"
#include "StringUtils.h"

class CGContext;
class Expression;
class Function;
class Block;
class Lhs;
class ArrayVariable;

class Variable
{
	friend class VariableSelector;
	friend class ArrayVariable;
public:
	static Variable *CreateVariable(const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer);
	static Variable *CreateVariable(const std::string &name, const Type *type,
			 bool isConst, bool isVolatile,
			 bool isAuto, bool isStatic, bool isRegister, bool isBitfield, const Variable* isFieldVarOf);
	static Variable *CreateVariable(const std::string &name, const Type *type,
			 const vector<bool>& isConsts, const vector<bool>& isVolatiles,
			 bool isAuto, bool isStatic, bool isRegister, bool isBitfield, const Variable* isFieldVarOf);

	static void doFinalization(void);

	virtual ~Variable(void);
	virtual bool is_global(void) const;
	virtual bool is_local(void) const;
	virtual bool is_visible_local(const Block* blk) const;
	virtual size_t get_dimension(void) const { return 0;}
	bool is_visible(const Block* blk) const { return is_global() || is_visible_local(blk);}
	bool is_argument(void) const;
	bool is_tmp_var(void) const;
	bool is_const(void) const;
	bool is_volatile(void) const;
	bool is_access_once(void) const { return isAccessOnce; }
	bool is_const_after_deref(int deref_level) const;
	bool is_volatile_after_deref(int deref_level) const;
	bool is_packed_aggregate_field_var() const;
	bool has_field_var(const Variable* v) const;
	bool is_field_var(void) const { return field_var_of != 0; };
	const Variable* get_top_container(void) const;
	const Variable* get_container_union(void) const;
	int  get_field_id(void) const;
	bool is_union_field(void) const { return field_var_of != 0 && field_var_of->type->eType == eUnion; };
	bool is_inside_union_field(void) const { return is_union_field() || (field_var_of && field_var_of->is_inside_union_field()); }
	bool is_packed_after_bitfield(void) const;
	bool is_array_field(void) const;
	bool is_virtual(void) const;
	bool is_aggregate(void) const { return type && type->is_aggregate(); }
	bool match(const Variable* v) const;
	bool loose_match(const Variable* v) const;
	bool is_pointer(void) const { return type && type->eType == ePointer;}
	bool is_rv(void) const { return name.find("_rv") != string::npos; }
	int get_seq_num(void) const;
	void find_pointer_fields(vector<const Variable*>& ptr_fields) const;

	virtual std::string get_actual_name() const;
	std::string to_string(void) const;

	// ISSUE: we treat volatiles specially
	bool compatible(const Variable *v) const;
	const Variable* get_named_var(void) const;
	const Variable* match_var_name(const string& vname) const;
	virtual void hash(std::ostream& out) const;
	virtual const Variable* get_collective(void) const;
	virtual const ArrayVariable* get_array(string& field) const;
	virtual int get_index_vars(vector<const Variable*>& /* vars */) const { return 0;}

	///////////////////////////////////////////////////////////////////////

	virtual void Output(std::ostream &) const;
	int output_runtime_value(ostream &out, string prefix, string suffix, int indent, bool multi_lines=false) const;
	int output_addressable_name(ostream &out, int indent) const;
	int output_volatile_address(ostream &out, int indent, const string &fp_string, vector<string> &seen_names) const;
	int output_volatile_fprintf(ostream &out, int indent, const string &name,
		const string &sizeof_string, const string &fp_string) const;
	bool is_seen_name(vector<std::string> &seen_names, const std::string &name) const;
	bool is_valid_volatile(void) const;
	int output_value_dump(ostream &out, string prefix, int indent) const;
	void OutputAddrOf(std::ostream &) const;
	void OutputForComment(std::ostream &) const;
	virtual void OutputDef(std::ostream &out, int indent) const;
	virtual void OutputDecl(std::ostream &) const;
	virtual void output_qualified_type(std::ostream &out) const;
	virtual void OutputLowerBound(std::ostream &) const;
	virtual void OutputUpperBound(std::ostream &) const;

	static size_t GetMaxArrayDimension(const vector<Variable*>& vars);

	vector<Variable *> field_vars;    // field variables for struct/unions
	const std::string name;
	const Type *type;
	const Expression *init;

	// Storage-class specifiers.
	const bool isAuto;
	// bool isExtern;
	const bool isStatic;
	const bool isRegister;
	const bool isBitfield_; // expanded from a full-bitfield struct var
	bool isAddrTaken;
	bool isAccessOnce;
	const Variable* field_var_of; //expanded from a struct/union
	const bool isArray;
	const CVQualifiers qfer;
	static std::vector<const Variable*> &get_new_ctrl_vars();
	static std::vector<const Variable*> &get_last_ctrl_vars();

	static const char sink_var_name[];

private:
	Variable(const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer);
	Variable(const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer, const Variable* isFieldVarOf, bool isArray);
	Variable(const std::string &name, const Type *type,
			 const vector<bool>& isConsts, const vector<bool>& isVolatiles,
			 bool isAuto, bool isStatic, bool isRegister, bool isBitfield, const Variable* isFieldVarOf);

	static std::vector<const Variable*>& new_ctrl_vars(void);
	static std::vector< std::vector<const Variable*>* > ctrl_vars_vectors;
	static unsigned long ctrl_vars_count;

	void create_field_vars(const Type* type);
};

void OutputVariableList(const std::vector<Variable*> &var, std::ostream &out, int indent = 0);
void OutputVariableDeclList(const std::vector<Variable*> &var, std::ostream &out, std::string prefix = "", int indent = 0);
void OutputArrayInitializers(const vector<Variable*>& vars, std::ostream &out, int indent);
void OutputArrayCtrlVars(const vector<const Variable*>& ctrl_vars, std::ostream &out, size_t dimen, int indent);
void OutputVolatileAddress(const vector<Variable*> &vars, std::ostream &out, int indent, const string &fp_string);
void MapVariableList(const vector<Variable*> &var, std::ostream &out, int (*func)(Variable *var, std::ostream *pOut));
int HashVariable(Variable *var, std::ostream *pOut);

int find_variable_in_set(const vector<const Variable*>& set, const Variable* v);
int find_variable_in_set(const vector<Variable*>& set, const Variable* v);
int find_field_variable_in_set(const vector<const Variable*>& set, const Variable* v);
bool is_variable_in_set(const vector<const Variable*>& set, const Variable* v);
bool add_variable_to_set(vector<const Variable*>& set, const Variable* v);
bool add_variables_to_set(vector<const Variable*>& set, const vector<const Variable*>& new_set);
bool equal_variable_sets(const vector<const Variable*>& set1, const vector<const Variable*>& set2);
bool sub_variable_sets(const vector<const Variable*>& set1, const vector<const Variable*>& set2);
void combine_variable_sets(const vector<const Variable*>& set1, const vector<const Variable*>& set2, vector<const Variable*>& set_all);
void remove_field_vars(vector<const Variable*>& set);

///////////////////////////////////////////////////////////////////////////////

#endif // VARIABLE_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

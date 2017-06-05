// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2014, 2015, 2016, 2017 The University of Utah
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

#ifdef WIN32
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif

#include "ArrayVariable.h"
#include <cassert>

#include "Common.h"
#include "CGContext.h"
#include "CGOptions.h"

#include "Block.h"
#include "Constant.h"
#include "Effect.h"
#include "Error.h"
#include "Expression.h"
#include "ExpressionFuncall.h"
#include "ExpressionVariable.h"
#include "FactMgr.h"
#include "FactUnion.h"
#include "Function.h"
#include "FunctionInvocation.h"
#include "Type.h"
#include "VariableSelector.h"
#include "random.h"
#include "util.h"

using namespace std;

/*
 * count the "key" variable of an binary/unary operation.
 * return 0 for constants, 2 for function calls
 */
static int count_expr_key_var(const Expression* e)
{
	if (e->term_type == eVariable) {
		return 1;
	}
	else if (e->term_type == eConstant) {
		return 0;
	}
	else if (e->term_type == eFunction) {
		const ExpressionFuncall* ef = dynamic_cast<const ExpressionFuncall*>(e);
		assert(ef);
		const FunctionInvocation* fi = ef->get_invoke();
		// for calls
		if (fi->invoke_type == eFuncCall) {
			return 2;
		}
		// for unary operations
		if (fi->param_value.size()==1) {
			return count_expr_key_var(fi->param_value[0]);
		}
		// for binary operations
		assert(fi->param_value.size() == 2);
		return count_expr_key_var(fi->param_value[0]) + count_expr_key_var(fi->param_value[1]);
	}
	// shouldn't be here
	assert(0);
	return 0;
}

/*
 * find the "key" variable of an expression.
 * return NULL if expression is a function call, constant, or a binary
 * operation involves at least two variables
 */
static const Variable* find_expr_key_var(const Expression* e)
{
	if (e->term_type == eVariable) {
		return ((const ExpressionVariable*)e)->get_var();
	}
	else if (e->term_type == eFunction) {
		const ExpressionFuncall* ef = dynamic_cast<const ExpressionFuncall*>(e);
		assert(ef);
		const FunctionInvocation& fi = *(ef->get_invoke());
		if (fi.invoke_type == eBinaryPrim || fi.invoke_type == eUnaryPrim) {
			if (fi.param_value.size()==1) {
				return find_expr_key_var(fi.param_value[0]);
			}
			assert(fi.param_value.size() == 2);
			const Variable* v0 = find_expr_key_var(fi.param_value[0]);
			const Variable* v1 = find_expr_key_var(fi.param_value[1]);
			if (v0 == NULL && v1 != NULL) return v1;
			if (v0 != NULL && v1 == NULL) return v0;
		}
	}
	return NULL;
}

//=======================================================================================
ArrayVariable *
ArrayVariable::CreateArrayVariable(const CGContext& cg_context, Block* blk, const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer, const Variable* isFieldVarOf)
{
	assert(type);
	if (type->eType == eSimple)
		assert(type->simple_type != eVoid);

	// quick way to choose a random array dimension: 1d 60%, 2d 30%, and son on
	int num = rnd_upto(99)+1;
	ERROR_GUARD(NULL);
	int dimension = 0;
	int step = 100;
	for (; num > 0; num -= step) {
		dimension++;
		step /= 2;
		if (step == 0) step = 1;
	}
	if (dimension > CGOptions::max_array_dimensions()) {
		dimension = CGOptions::max_array_dimensions();
	}
	vector<unsigned int> sizes;
	int total_size = 1;
	for (int i=0; i<dimension; i++) {
		unsigned int dimen_size = rnd_upto(CGOptions::max_array_length_per_dimension()) + 1;
		ERROR_GUARD(NULL);
		if (total_size * dimen_size > (unsigned int)CGOptions::max_array_length()) {
			dimen_size = CGOptions::max_array_length() / total_size;
		}
		if (dimen_size) {
			total_size *= dimen_size;
			sizes.push_back(dimen_size);
		}
	}
	ArrayVariable *var = new ArrayVariable(blk, name, type, init, qfer, sizes, isFieldVarOf);
	ERROR_GUARD_AND_DEL1(NULL, var);
	if (type->is_aggregate()) {
		var->create_field_vars(type);
	}
	// create a list of alternative initial values
	unsigned int init_num = pure_rnd_upto(total_size / 2);
	if (0) {   // keep the code for comparing the bug finding power with the else branch
		if (type->eType == eSimple || type->eType == eStruct) {
			unsigned int init_num = pure_rnd_upto(total_size - 1);
			for (size_t i=0; i<init_num; i++) {
				Expression* e = Constant::make_random(type);
				var->add_init_value(e);
			}
		}
	} else {
		for (size_t i=0; i<init_num; i++) {
			Expression* e = NULL;
			if (type->eType != ePointer || CGOptions::strict_const_arrays()) {
				e = Constant::make_random(type);
			} else {
				e = VariableSelector::make_init_value(Effect::READ, cg_context, type, qfer, blk);
			}
			var->add_init_value(e);
		}
	}

	// add it to global list or local variable list
	blk? blk->local_vars.push_back(var) : VariableSelector::GetGlobalVariables()->push_back(var);
	return var;
}

/*
 *
 */
ArrayVariable::ArrayVariable(Block* blk, const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer, const vector<unsigned int>& sizes, const Variable* isFieldVarOf)
	: Variable(name, type, init, qfer, isFieldVarOf, true),
	  collective(NULL),
	  parent(blk),
	  sizes(sizes)
{
	// nothing else to do
}

ArrayVariable::ArrayVariable(const ArrayVariable& av)
	: Variable(av.name, av.type, av.init, &(av.qfer), av.field_var_of, true),
	collective(av.collective),
	parent(av.parent),
	sizes(av.sizes),
	indices(av.indices),
	init_values(av.init_values)
{
	// nothing else to do
}
/*
 *
 */
ArrayVariable::~ArrayVariable(void)
{
	// use Variable's destructor for collective variable
	if (collective != 0) {
		size_t i;
		for (i=0; i<indices.size(); i++) {
			delete indices[i];
		}
		init = NULL;   // set to NULL to avoid being deleted twice
	}
}

void
ArrayVariable::add_index(const Expression* e)
{
	indices.push_back(e);
}

void
ArrayVariable::set_index(size_t index, const Expression* e)
{
	indices[index] = e;
}

unsigned long
ArrayVariable::get_size(void) const
{
	size_t i;
	unsigned long len = 1;
	for (i=0; i<sizes.size(); i++) {
		len *= sizes[i];
	}
	return len;
}

unsigned long
ArrayVariable::size_in_bytes(void) const
{
	unsigned long len = type->SizeInBytes();
	size_t i;
	for (i=0; i<sizes.size(); i++) {
		len *= sizes[i];
	}
	return len;
}

ArrayVariable*
ArrayVariable::itemize(void) const
{
	size_t i;
	assert(collective == 0);
	ArrayVariable* av = new ArrayVariable(*this);
	VariableSelector::AllVars.push_back(av);
	for (i=0; i<sizes.size(); i++) {
		int index = rnd_upto(sizes[i]);
		av->add_index(new Constant(get_int_type(), StringUtils::int2str(index)));
	}
	av->collective = this;
	// only expand struct/union for itemized array variable
	if (type->is_aggregate()) {
		av->create_field_vars(type);
	}
	return av;
}

ArrayVariable*
ArrayVariable::itemize(const vector<int>& const_indices) const
{
	size_t i;
	assert(collective == 0);
	assert(const_indices.size() == sizes.size());
	ArrayVariable* av = new ArrayVariable(*this);
	VariableSelector::AllVars.push_back(av);
	for (i=0; i<sizes.size(); i++) {
		int index = const_indices[i];
		av->add_index(new Constant(get_int_type(), StringUtils::int2str(index)));
	}
	av->collective = this;
	// only expand struct/union for itemized array variable
	if (type->is_aggregate()) {
		av->create_field_vars(type);
	}
	return av;
}

ArrayVariable*
ArrayVariable::itemize(const std::vector<const Variable*>& indices, Block* blk) const
{
	size_t i;
	// Looks like this function is dead.
	assert(0 && "Invoke a dead function?");
	assert(collective == 0);
	ArrayVariable* av = new ArrayVariable(*this);
	VariableSelector::AllVars.push_back(av);
	for (i=0; i<sizes.size(); i++) {
		av->add_index(new ExpressionVariable(*indices[i]));
	}
	av->collective = this;
	av->parent = blk;
	// only expand struct/union for itemized array variable
	if (type->is_aggregate()) {
		av->create_field_vars(type);
	}
	blk->local_vars.push_back(av);
	return av;
}

ArrayVariable*
ArrayVariable::itemize(const std::vector<const Expression*>& indices, Block* blk) const
{
	size_t i;
	assert(collective == 0);
	ArrayVariable* av = new ArrayVariable(*this);
	VariableSelector::AllVars.push_back(av);
	for (i=0; i<sizes.size(); i++) {
		av->add_index(indices[i]);
	}
	av->collective = this;
	av->parent = blk;
	// only expand struct/union for itemized array variable
	if (type->is_aggregate()) {
		av->create_field_vars(type);
	}
	blk->local_vars.push_back(av);
	return av;
}

ArrayVariable*
ArrayVariable::rnd_mutate(void)
{
	assert(0 && "invalid call to rnd_mutate");
	bool use_existing = rnd_flipcoin(20);
	ERROR_GUARD(NULL);
	size_t i;
	if (use_existing) {
		vector<Variable*> ok_vars;
		for (i=0; i<parent->local_vars.size(); i++) {
			if (is_variant(parent->local_vars[i])) {
				ok_vars.push_back(parent->local_vars[i]);
			}
		}
		Variable* v = VariableSelector::choose_ok_var(ok_vars);
		ERROR_GUARD(NULL);
		if (v) {
			ArrayVariable* av = dynamic_cast<ArrayVariable*>(v);
			return av;
		}
	}
	vector<const Expression*> new_indices;
	vector<bool> mutate_flags;
	bool no_mutate = true;
	for (i=0; i<get_dimension(); i++) {
		bool mutate = rnd_flipcoin(10);
		ERROR_GUARD(0);
		mutate_flags.push_back(mutate);
		if (mutate) {
			no_mutate = false;
		}
	}
	// if no mutation, return self
	if (no_mutate) {
		return this;
	}
	for (i=0; i<get_dimension(); i++) {
		if (mutate_flags[i]) {
			const Expression* e = indices[i];
			assert(e->term_type == eVariable);
			const ExpressionVariable* ev = dynamic_cast<const ExpressionVariable*>(e);
			// create a mutated index from the original by adding an constant offset
			FunctionInvocation* fi = new FunctionInvocationBinary(eAdd, 0);
        	fi->add_operand(new ExpressionVariable(*(ev->get_var())));
			int offset = rnd_upto(sizes[i]);
			if (offset == 0) offset = 1;	// give offset 1 more chance
			ERROR_GUARD(NULL);
			ostringstream oss;
			oss << offset;
        	fi->add_operand(new Constant(get_int_type(), oss.str()));
        	Expression* mutated_e = new ExpressionFuncall(*fi);
			new_indices.push_back(mutated_e);
		}
		else {
			new_indices.push_back(indices[i]->clone());
		}
	}
	// if index of at least one dimension mutated, return the new variable
	return VariableSelector::create_mutated_array_var(this, new_indices);
}

bool
ArrayVariable::is_variant(const Variable* v) const
{
	if (v->isArray) {
		const ArrayVariable* av = dynamic_cast<const ArrayVariable*>(v);
		assert(av);
		if (av->collective != 0 && collective == av->collective && av->indices.size() == this->indices.size() ) {
			size_t i;
			for (i=0; i<indices.size(); i++) {
				const Expression* e = indices[i];
				const Expression* other_e = av->indices[i];
				if (count_expr_key_var(e) != 1 ||
					count_expr_key_var(other_e) != 1 ||
					find_expr_key_var(e) != find_expr_key_var(other_e)) {
					return false;
				}
			}
			return true;
		}
	}
	return false;
}

// --------------------------------------------------------------
bool
ArrayVariable::is_global(void) const
{
	return parent == 0;
}

// -------------------------------------------------------------
bool
ArrayVariable::is_visible_local(const Block* blk) const
{
	const Block* b = blk;
	while (b) {
		if (b == parent) {
			return true;
		}
		b = b->parent;
	}
    return false;
}

bool
ArrayVariable::no_loop_initializer(void) const
{
	// don't use loop initializer if we are doing test case reduction
	// if (CGOptions::get_reducer()) return true;
	// can not use loop initializer if either array member are structs, or they are constants, or it has > 1 initial values
	return type->eType==eStruct || type->eType==eUnion || is_const() || is_global() || (init_values.size() > 0);
}

// print the initializer recursively for multi-dimension arrays
// this is based on John's idea
string
ArrayVariable::build_init_recursive(size_t dimen, const vector<string>& init_strings) const
{
	assert (dimen < get_dimension());
	static unsigned seed = 0xABCDEF;
	string ret = "{";
	for (size_t i=0; i<sizes[dimen]; i++) {
		if (dimen == sizes.size() - 1) {
			// use magic number to choose an initial value
			size_t rnd_index = ((seed * seed + (i+7) * (i+13)) * 52369) % (init_strings.size());
			ret += init_strings[rnd_index];
			seed++;
		 } else {
			ret += build_init_recursive(dimen + 1, init_strings);
		 }
		 if (i != sizes[dimen]-1) ret += ",";
	}
	ret += "}";
	return ret;
}

// build the string initializer in form of "{...}"
string
ArrayVariable::build_initializer_str(const vector<string>& init_strings) const
{
	string str, str_dimen;
	if (CGOptions::force_non_uniform_array_init()) {
		return build_init_recursive(0, init_strings);
	}

	for (int i=sizes.size()-1; i>=0; i--) {
		size_t len = sizes[i];
		str_dimen = "{";
		for (size_t j=0; j<len; j++) {
			// for last dimension, use magic number to choose an initial value
			if (i == ((int)sizes.size()) - 1) {
				unsigned int rnd_index = ((i + (j+7) * (j+13)) * 52369) % (init_strings.size());
				str_dimen += init_strings[rnd_index];
			} else {
				str_dimen += str;
			}
			str_dimen += ((j<len-1) ? ", " : "");
		}
		str_dimen += "}";
		str = str_dimen;
	}
	return str;
}

// --------------------------------------------------------------
void
ArrayVariable::OutputDef(std::ostream &out, int indent) const
{
	if (collective == 0) {
		output_tab(out, indent);
		if (!no_loop_initializer() ) {
			// don't print definition for array, rather use a loop initializer
			OutputDecl(out);
			out << ";";
			outputln(out);
		}
		else {
			// use string initializer for arrays
			// create the strings for initial values
			size_t i;
			vector<string> init_strings;
			assert(init);
			init_strings.push_back(init->to_string());
			for (i=0; i<init_values.size(); i++) {
				init_strings.push_back(init_values[i]->to_string());
			}

			// force global variables to be static if necessary
			if (CGOptions::force_globals_static() && is_global()) {
				out << "static ";
			}

			// print type, name, and dimensions
			output_qualified_type(out);
			out << get_actual_name();
			for (i=0; i<sizes.size(); i++) {
				out << "[" << sizes[i] << "]";
			}
			out << " = " << build_initializer_str(init_strings) << ";";
			outputln(out);
		}
	}
}

void ArrayVariable::OutputDecl(std::ostream &out) const
{
	// force global variables to be static if necessary
	if (CGOptions::force_globals_static() && is_global()) {
		out << "static ";
	}
	output_qualified_type(out);
	out << get_actual_name();
	size_t i;
	for (i=0; i<sizes.size(); i++) {
		out << "[" << sizes[i] << "]";
	}
}

// --------------------------------------------------------------
void
ArrayVariable::Output(std::ostream &out) const
{
	if (collective == 0) {
		out << get_actual_name();
	}
	// for itemized array variables, output the modularized index
	else {
		out << get_actual_name();
		assert(!indices.empty());
		size_t i;
		for (i=0; i<indices.size(); i++) {
			if (1) { //indices[i]->less_than(sizes[i])) {
				out << "[";
				indices[i]->Output(out);
				out << "]";
			}
			else {
				out << "[";
				const Type* t = &(indices[i]->get_type());
				// cast signed variable to it's corresponding unsigned type, just in case we get a negative index
				if (t->is_signed()) {
					const Type* unsigned_type = t->to_unsigned();
					assert(unsigned_type);
					out << "(";
					unsigned_type->Output(out);
					out << ")";
				}
				out << "(";
				indices[i]->Output(out);
				out << ") % " << sizes[i] << "]";
			}
		}
	}
}

void
ArrayVariable::OutputUpperBound(std::ostream &out) const
{
	out << name;
	size_t i;
	for (i=0; i<get_dimension(); i++) {
		out << "[" << (sizes[i] - 1) << "]";
	}
}

// --------------------------------------------------------------
void
ArrayVariable::OutputLowerBound(std::ostream &out) const
{
	out << name;
	size_t i;
	for (i=0; i<get_dimension(); i++) {
		out << "[0]";
	}
}

// --------------------------------------------------------------
void
ArrayVariable::output_with_indices(std::ostream &out, const std::vector<const Variable*>& cvs) const
{
	size_t i;
	out << get_actual_name();
	for (i=0; i<get_dimension(); i++) {
		out << "[";
		cvs[i]->Output(out);
		out << "]";
	}
}

void
ArrayVariable::output_checksum_with_indices(std::ostream &out,
					const std::vector<const Variable*>& cvs,
					string field_name) const
{
	out << "printf(\"...checksum after hashing ";

	size_t i;
	out << get_actual_name();
	for (i=0; i<get_dimension(); i++) {
		out << "[%d]";
	}
	out << field_name;

	out << " : %X\\n\"";
	for (i=0; i<get_dimension(); i++) {
		out << " ,";
		cvs[i]->Output(out);
	}

	out <<", crc32_context ^ 0xFFFFFFFFUL);";
}

// --------------------------------------------------------------
void
ArrayVariable::output_init(std::ostream &out, const Expression* init, const vector<const Variable*>& cvs, int indent) const
{
	if (collective != 0) return;
	size_t i;

	for (i=0; i<get_dimension(); i++) {
		if (i > 0) {
			output_tab(out, indent);
			out << "{";
			outputln(out);
			indent++;
		}
		output_tab(out, indent);
		out << "for (";
		out << cvs[i]->get_actual_name();
		out << " = 0; ";
		out << cvs[i]->get_actual_name();
		out << " < " << sizes[i] << "; ";
		out << cvs[i]->get_actual_name();
		if (CGOptions::post_incr_operator()) {
			out << "++)";
		}
		else {
			out << " = " << cvs[i]->get_actual_name() << " + 1)";
		}
		outputln(out);
	}
	output_tab(out, indent+1);
	output_with_indices(out, cvs);
	out << " = ";
	init->Output(out);
	out << ";";
	outputln(out);
	// output the closing bracelets
	for (i=1; i<get_dimension(); i++) {
		indent--;
		output_tab(out, indent);
		out << "}";
		outputln(out);
	}
}

// --------------------------------------------------------------
void
ArrayVariable::output_addr_checks(std::ostream &out, const Variable* var, string field_name, int indent) const
{
	size_t i;
	vector<const Variable *> &ctrl_vars = Variable::get_new_ctrl_vars();
	// declare control variables
	OutputArrayCtrlVars(ctrl_vars, out, get_dimension(), indent);
	for (i=0; i<get_dimension(); i++) {
		output_tab(out, indent);
		out << "for (";
		out << ctrl_vars[i]->get_actual_name();
		out << " = 0; ";
		out << ctrl_vars[i]->get_actual_name();
		out << " < " << sizes[i] << "; ";
		out << ctrl_vars[i]->get_actual_name();
		if (CGOptions::post_incr_operator()) {
			out << "++)";
		}
		else {
			out << " = " << ctrl_vars[i]->get_actual_name() << " + 1)";
		}
		outputln(out);
		output_open_encloser("{", out, indent);
	}
	output_tab(out, indent);
	out << "if (";
	var->Output(out);
	out << " == &";
	output_with_indices(out, ctrl_vars);
	out << field_name;
	out << ")" << endl;
	output_open_encloser("{", out, indent);
	output_tab(out, indent);
	out << "printf(\"   ";
	var->Output(out);
	out << " = &";
	out << get_actual_name();
	for (i=0; i<get_dimension(); i++) {
		out << "[%d]";
	}
	out << ";\\n\"";
	for (i=0; i<get_dimension(); i++) {
		out << ", ";
		out << ctrl_vars[i]->get_actual_name();
	}
	out << ");" << endl;
	output_tab(out, indent);
	out << "break;";
	output_close_encloser("}", out, indent);
	// output the closing bracelets
	for (i=0; i<get_dimension(); i++) {
		output_close_encloser("}", out, indent);
	}
}

string
ArrayVariable::make_print_index_str(const vector<const Variable *> &cvs) const
{
	size_t i;
	string str = "printf(\"index = ";
	for (i=0; i<get_dimension(); i++) {
		str += "[%d]";
	}
	str += "\\n\", ";
	for (i=0; i<get_dimension(); i++) {
		if (i) {
			str += ", ";
		}
		str += cvs[i]->get_actual_name();
	}
	str += ");";
	return str;
}

/* -------------------------------------------------------------
 *  hash all array items
 ***************************************************************/
void
ArrayVariable::hash(std::ostream& out) const
{
	if (collective != 0) return;
	vector<string> field_names;
	vector<const Type *> field_types;
	vector<int> included_fields;
	// for unions, find the fields that we don't want to hash due to union field read rules. So FactUnion.cpp
	vector<int> excluded_fields;
	if (type->eType == eUnion) {
		FactMgr* fm = get_fact_mgr_for_func(GetFirstFunction());
		assert(fm);
		for (size_t i=0; i<field_vars.size(); i++) {
			if (!FactUnion::is_field_readable(this, i, fm->global_facts)) {
				excluded_fields.push_back(i);
			}
		}
	}
 	type->get_int_subfield_names("", field_names, field_types, excluded_fields);
	assert(field_names.size() == field_types.size());
	// if not a suitable type for hashing, give up
	if (field_names.size() == 0) return;

	size_t i, j;
	int indent = 1;
	//ISSUE: ugly hack to make sure we use the latest ctrl_vars, which is generated
	// from the call of OutputArrayInitializers in OutputMgr.cpp
	const vector<const Variable*>& cvs = Variable::get_last_ctrl_vars();
	for (i=0; i<get_dimension(); i++) {
		output_tab(out, indent);
		out << "for (";
		out << cvs[i]->get_actual_name();
		out << " = 0; ";
		out << cvs[i]->get_actual_name();
		out << " < " << sizes[i] << "; ";
		out << cvs[i]->get_actual_name();
		if (CGOptions::post_incr_operator()) {
			out << "++)";
		}
		else {
			out << " = " << cvs[i]->get_actual_name() << " + 1)";
		}
		outputln(out);
		output_open_encloser("{", out, indent);
	}
	string vname;
	ostringstream oss;
	output_with_indices(oss, cvs);
	vname = oss.str();
	if (CGOptions::compute_hash()) {
		for (j=0; j<field_names.size(); j++) {
			if (field_types[j]->eType == eSimple && field_types[j]->simple_type == eFloat) {
				output_tab(out, indent);
				out << "transparent_crc_bytes(&" << vname << field_names[j] << ", ";
				out << "sizeof(" << vname << field_names[j] << "), ";
				out << "\"" << vname << field_names[j] << "\", print_hash_value);" << endl;
			} else {
				output_tab(out, indent);
				out << "transparent_crc(" << vname << field_names[j] << ", \"";
				out << vname << field_names[j] << "\", print_hash_value);" << endl;
			}
		}
		// print the index value
		if (CGOptions::hash_value_printf()) {
			output_tab(out, indent);
			out << "if (print_hash_value) " << make_print_index_str(cvs) << endl;
		}
	}
	else {
		if (type->eType == eSimple) {
			output_tab(out, indent);
			out << Variable::sink_var_name << " = ";
			output_with_indices(out, cvs);
			out << ";";
		}
	}
	// output the closing bracelets
	for (i=0; i<get_dimension(); i++) {
		output_close_encloser("}", out, indent);
	}
	outputln(out);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

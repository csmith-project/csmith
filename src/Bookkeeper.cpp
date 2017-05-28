// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2015, 2017 The University of Utah
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

#include "Bookkeeper.h"
#include <cassert>
#include <iostream>
#include "Variable.h"
#include "Type.h"
#include "Function.h"
#include "Expression.h"
#include "ExpressionVariable.h"
#include "Fact.h"
#include "FactPointTo.h"
#include "FactMgr.h"
#include "CVQualifiers.h"
#include "Statement.h"
#include "Block.h"
#include "CGOptions.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

// counter for all levels of struct depth
std::vector<int> Bookkeeper::struct_depth_cnts;
int Bookkeeper::union_var_cnt = 0;
std::vector<int> Bookkeeper::expr_depth_cnts;
std::vector<int> Bookkeeper::blk_depth_cnts;
std::vector<int> Bookkeeper::dereference_level_cnts;
int Bookkeeper::address_taken_cnt = 0;
std::vector<int> Bookkeeper::read_dereference_cnts;
std::vector<int> Bookkeeper::write_dereference_cnts;
int Bookkeeper::cmp_ptr_to_null = 0;
int Bookkeeper::cmp_ptr_to_ptr = 0;
int Bookkeeper::cmp_ptr_to_addr = 0;
int Bookkeeper::read_volatile_cnt = 0;
int Bookkeeper::write_volatile_cnt = 0;
int Bookkeeper::read_non_volatile_cnt = 0;
int Bookkeeper::write_non_volatile_cnt = 0;
int Bookkeeper::read_volatile_thru_ptr_cnt = 0;
int Bookkeeper::write_volatile_thru_ptr_cnt = 0;
int Bookkeeper::pointer_avail_for_dereference = 0;
int Bookkeeper::volatile_avail = 0;
int Bookkeeper::structs_with_bitfields = 0;
std::vector<int> Bookkeeper::vars_with_bitfields;
std::vector<int> Bookkeeper::vars_with_full_bitfields;
int Bookkeeper::vars_with_bitfields_address_taken_cnt = 0;
int Bookkeeper::bitfields_in_total = 0;
int Bookkeeper::unamed_bitfields_in_total = 0;
int Bookkeeper::const_bitfields_in_total = 0;
int Bookkeeper::volatile_bitfields_in_total = 0;
int Bookkeeper::lhs_bitfields_structs_vars_cnt = 0;
int Bookkeeper::rhs_bitfields_structs_vars_cnt = 0;
int Bookkeeper::lhs_bitfield_cnt = 0;
int Bookkeeper::rhs_bitfield_cnt = 0;
int Bookkeeper::forward_jump_cnt = 0;
int Bookkeeper::backward_jump_cnt = 0;
int Bookkeeper::use_new_var_cnt = 0;
int Bookkeeper::use_old_var_cnt = 0;
bool Bookkeeper::rely_on_int_size = false;
bool Bookkeeper::rely_on_ptr_size = false;

/*
 *
 */
Bookkeeper::Bookkeeper()
{
}

/*
 *
 */
Bookkeeper::~Bookkeeper(void)
{
	// Nothing to do.
}

static void
formated_output(std::ostream &out, const char* msg, int num)
{
	out << "XXX " << msg << num << endl;
}

static void
formated_outputf(std::ostream &out, const char* msg, double num)
{
	out << "XXX " << msg << num << endl;
}

void
Bookkeeper::doFinalization()
{
	Bookkeeper::struct_depth_cnts.clear();
	Bookkeeper::expr_depth_cnts.clear();
	Bookkeeper::dereference_level_cnts.clear();
	Bookkeeper::address_taken_cnt = 0;
	Bookkeeper::write_dereference_cnts.clear();
	Bookkeeper::read_dereference_cnts.clear();
	Bookkeeper::cmp_ptr_to_null = 0;
	Bookkeeper::cmp_ptr_to_ptr = 0;
	Bookkeeper::cmp_ptr_to_addr = 0;

}

int
Bookkeeper::stat_blk_depths_for_stmt(const Statement* s)
{
	size_t i, j;
	int cnt = 0;
	if (s->eType != eBlock) {
		incr_counter(blk_depth_cnts, s->get_blk_depth() -1);
		cnt++;
	}
	vector<const Block*> blks;
	s->get_blocks(blks);
	for (i=0; i<blks.size(); i++) {
		for (j=0; j<blks[i]->stms.size(); j++) {
			cnt += stat_blk_depths_for_stmt(blks[i]->stms[j]);
		}
	}
	return cnt;
}

int
Bookkeeper::stat_blk_depths(void)
{
	const vector<Function*>& funcs = get_all_functions();
	int cnt = 0;
	for (size_t i=0; i<funcs.size(); i++) {
		if (funcs[i]->is_builtin)
			continue;
		cnt += stat_blk_depths_for_stmt(funcs[i]->body);
	}
	return cnt;
}

void
Bookkeeper::output_stmts_statistics(std::ostream &out)
{
	size_t i;
	int stmt_cnt = stat_blk_depths();
	formated_output(out, "stmts: ", stmt_cnt);
	formated_output(out, "max block depth: ", (blk_depth_cnts.size() - 1));
	out << "breakdown:" << endl;
	for (i=0; i<blk_depth_cnts.size(); i++) {
		if (blk_depth_cnts[i]) {
			out << "   depth: " << i << ", occurrence: " << blk_depth_cnts[i] << endl;
		}
	}
}

void
Bookkeeper::output_statistics(std::ostream &out)
{
	output_struct_union_statistics(out);
	out << endl;
	output_expr_statistics(out);
	out << endl;
	output_pointer_statistics(out);
	out << endl;
	output_volatile_access_statistics(out);
	out << endl;
	output_jump_statistics(out);
	out << endl;
	output_stmts_statistics(out);
	out << endl;
	output_var_freshness(out);
	if (rely_on_int_size) {
		out << "FYI: the random generator makes assumptions about the integer size. See ";
		out << PLATFORM_CONFIG_FILE << " for more details." << endl;
	}
	if (rely_on_ptr_size) {
		out << "FYI: the random generator makes assumptions about the pointer size. See ";
		out << PLATFORM_CONFIG_FILE << " for more details." << endl;
	}
}

void
Bookkeeper::output_struct_union_statistics(std::ostream &out)
{
	formated_output(out, "max struct depth: ", (struct_depth_cnts.size()-1));
	out << "breakdown:" << endl;
	for (size_t i=0; i<struct_depth_cnts.size(); i++) {
		out << "   depth: " << i << ", occurrence: " << struct_depth_cnts[i] << endl;
	}
	formated_output(out, "total union variables: ", union_var_cnt);
	Bookkeeper::output_bitfields(out);
}

void
Bookkeeper::stat_expr_depths_for_stmt(const Statement* s)
{
	size_t i, j;
	vector<const Expression*> exprs;
	vector<const Block*> blks;
	s->get_exprs(exprs);
	for (i=0; i<exprs.size(); i++) {
		incr_counter(expr_depth_cnts, exprs[i]->get_complexity());
	}
	s->get_blocks(blks);
	for (i=0; i<blks.size(); i++) {
		for (j=0; j<blks[i]->stms.size(); j++) {
			stat_expr_depths_for_stmt(blks[i]->stms[j]);
		}
	}
}

void
Bookkeeper::stat_expr_depths(void)
{
	const vector<Function*>& funcs = get_all_functions();
	for (size_t i=0; i<funcs.size(); i++) {
		if (funcs[i]->is_builtin)
			continue;
		stat_expr_depths_for_stmt(funcs[i]->body);
	}
}

void
Bookkeeper::output_expr_statistics(std::ostream &out)
{
	size_t i;
	stat_expr_depths();
	formated_output(out, "max expression depth: ", (expr_depth_cnts.size() - 1));
	out << "breakdown:" << endl;
	for (i=0; i<expr_depth_cnts.size(); i++) {
		if (expr_depth_cnts[i]) {
			out << "   depth: " << i << ", occurrence: " << expr_depth_cnts[i] << endl;
		}
	}
}

void
Bookkeeper::output_pointer_statistics(std::ostream &out)
{
	size_t i;
	int total_alias_cnt = 0;
	int total_has_null_ptr = 0;
	int point_to_scalar = 0;
	int point_to_struct = 0;
	int point_to_pointer = 0;
	const vector<const Variable*>& ptrs = FactPointTo::all_ptrs;
	const vector<vector<const Variable*> >& aliases = FactPointTo::all_aliases;
	for (i=0; i<ptrs.size(); i++) {
		total_alias_cnt += aliases[i].size();
		if (find_variable_in_set(aliases[i], FactPointTo::null_ptr) >= 0) {
			total_has_null_ptr++;
		}
		const Variable* var = ptrs[i];
		const Type* t = var->type;
		assert(t->eType == ePointer);
		if (t->get_indirect_level() > 1) {
			point_to_pointer++;
		}
		else if (t->ptr_type->eType == eSimple) {
			point_to_scalar++;
		}
		else if (t->ptr_type->eType == eStruct) {
			point_to_struct++;
		}
	}

	formated_output(out, "total number of pointers: ", ptrs.size());
	if (ptrs.size() > 0) {
		out << endl;
		formated_output(out, "times a variable address is taken: ", address_taken_cnt);
		formated_output(out, "times a pointer is dereferenced on RHS: ", calc_total(read_dereference_cnts));
		out << "breakdown:" << endl;
		for (i=1; i<read_dereference_cnts.size(); i++) {
			out << "   depth: " << i << ", occurrence: " << read_dereference_cnts[i] << endl;
		}
		formated_output(out, "times a pointer is dereferenced on LHS: ", calc_total(write_dereference_cnts));
		out << "breakdown:" << endl;
		for (i=1; i<write_dereference_cnts.size(); i++) {
			out << "   depth: " << i << ", occurrence: " << write_dereference_cnts[i] << endl;
		}
		formated_output(out, "times a pointer is compared with null: ", cmp_ptr_to_null);
		formated_output(out, "times a pointer is compared with address of another variable: ", cmp_ptr_to_addr);
		formated_output(out, "times a pointer is compared with another pointer: ", cmp_ptr_to_ptr);
		formated_output(out, "times a pointer is qualified to be dereferenced: ", pointer_avail_for_dereference);

		// if there are dereferenced pointers
		if (dereference_level_cnts.size()) {
			out << endl;
			formated_output(out, "max dereference level: ", dereference_level_cnts.size()-1);
			out << "breakdown:" << endl;
			for (i=0; i<dereference_level_cnts.size(); i++) {
				out << "   level: " << i << ", occurrence: " << dereference_level_cnts[i] << endl;
			}
		}
		formated_output(out, "number of pointers point to pointers: ", point_to_pointer);
		formated_output(out, "number of pointers point to scalars: ", point_to_scalar);
		formated_output(out, "number of pointers point to structs: ", point_to_struct);
		out.precision(3);
		formated_outputf(out, "percent of pointers has null in alias set: ", total_has_null_ptr*100.0/ptrs.size());
		formated_outputf(out, "average alias set size: ", total_alias_cnt*1.0 / ptrs.size());
	}
}

void
Bookkeeper::record_address_taken(const Variable *var)
{
	assert(var);
	assert(var->type);
	const Type *type = var->type;

	// explicitly removing const-ness is a little bit ugly,
	// but changing record_address_taken interface involves
	// a lot of code change...
	Variable *addrTakenVar = const_cast<Variable*>(var);
        addrTakenVar->isAddrTaken =  true;

	Bookkeeper::address_taken_cnt++;
	if (type->has_bitfields())
		Bookkeeper::vars_with_bitfields_address_taken_cnt++;
}

void
Bookkeeper::record_bitfields_reads(const Variable *var)
{
	assert(var);
	assert(var->type);
	const Type *type = var->type;

	if (type->has_bitfields())
		Bookkeeper::rhs_bitfields_structs_vars_cnt++;
	if (var->isBitfield_)
		Bookkeeper::rhs_bitfield_cnt++;
}

void
Bookkeeper::record_bitfields_writes(const Variable *var)
{
	assert(var);
	assert(var->type);
	const Type *type = var->type;

	if (type->has_bitfields())
		Bookkeeper::lhs_bitfields_structs_vars_cnt++;
	if (var->isBitfield_)
		Bookkeeper::lhs_bitfield_cnt++;
}

/*
 * record the LHS/RHS types of comparisons between pointers
 */
void
Bookkeeper::record_pointer_comparisons(const Expression* lhs, const Expression* rhs)
{
	if (lhs->term_type != eFunction && rhs->term_type != eFunction) {
		assert(lhs->get_type().eType == ePointer && rhs->get_type().eType == ePointer);
		if ((lhs->term_type == eVariable && rhs->term_type == eConstant) ||
			(rhs->term_type == eVariable && lhs->term_type == eConstant)) {
			cmp_ptr_to_null++;
		}
		else if (lhs->term_type==eVariable && rhs->term_type==eVariable) {
			const ExpressionVariable* left = (ExpressionVariable*)lhs;
			const ExpressionVariable* right = (ExpressionVariable*)rhs;
			if (left->get_indirect_level() == right->get_indirect_level()) {
				cmp_ptr_to_ptr++;
			}
			else {
				cmp_ptr_to_addr++;
			}
		}
	}
}

/*
 * count volatile/non-volatile reads/writes, specifically access thru pointers
 */
void
Bookkeeper::record_volatile_access(const Variable* var, int deref_level, bool write)
{
	assert(var);
	int i;
	write ? record_bitfields_writes(var) : record_bitfields_reads(var);;
	for (i=0; i<=deref_level; i++) {
		if (write) {
			if (var->qfer.is_volatile_after_deref(i)) {
				if (i) {
					Bookkeeper::write_volatile_thru_ptr_cnt++;
				}
				Bookkeeper::write_volatile_cnt++;
				//var->OutputDef(cout);
			}
			else {
				Bookkeeper::write_non_volatile_cnt++;
			}
		}
		else {
			if (var->qfer.is_volatile_after_deref(i)) {
				if (i) {
					Bookkeeper::read_volatile_thru_ptr_cnt++;
				}
				Bookkeeper::read_volatile_cnt++;
			}
			else {
				Bookkeeper::read_non_volatile_cnt++;
			}
		}
	}
}

void
Bookkeeper::output_volatile_access_statistics(std::ostream &out)
{
	// size_t i;
	formated_output(out, "times a non-volatile is read: ", read_non_volatile_cnt);
	formated_output(out, "times a non-volatile is write: ", write_non_volatile_cnt);
	formated_output(out, "times a volatile is read: ", read_volatile_cnt);
	formated_output(out, "   times read thru a pointer: ", read_volatile_thru_ptr_cnt);
	formated_output(out, "times a volatile is write: ", write_volatile_cnt);
	formated_output(out, "   times written thru a pointer: ", write_volatile_thru_ptr_cnt);
	double percentage = (read_non_volatile_cnt + write_non_volatile_cnt) * 100.0 /
		                (read_non_volatile_cnt + write_non_volatile_cnt + read_volatile_cnt + write_volatile_cnt);

	formated_outputf(out, "times a volatile is available for access: ", volatile_avail);
	out.precision(3);
	formated_outputf(out, "percentage of non-volatile access: ", percentage);
}

void
Bookkeeper::output_bitfields(std::ostream &out)
{
	if (CGOptions::bitfields()) {
		out << std::endl;
		//formated_output(out, "structs with full-bitfields: ", structs_with_bitfields);
		formated_output(out, "non-zero bitfields defined in structs: ", bitfields_in_total);
		formated_output(out, "zero bitfields defined in structs: ", unamed_bitfields_in_total);
		formated_output(out, "const bitfields defined in structs: ", const_bitfields_in_total);
		formated_output(out, "volatile bitfields defined in structs: ", volatile_bitfields_in_total);
		Bookkeeper::output_counters(out, "structs with bitfields in the program: ", "indirect level", vars_with_bitfields);
		Bookkeeper::output_counters(out, "full-bitfields structs in the program: ", "indirect level", vars_with_full_bitfields);
		formated_output(out, "times a bitfields struct's address is taken: ", vars_with_bitfields_address_taken_cnt);
		formated_output(out, "times a bitfields struct on LHS: ", lhs_bitfields_structs_vars_cnt);
		formated_output(out, "times a bitfields struct on RHS: ", rhs_bitfields_structs_vars_cnt);
		formated_output(out, "times a single bitfield on LHS: ", lhs_bitfield_cnt);
		formated_output(out, "times a single bitfield on RHS: ", rhs_bitfield_cnt);
	}
}

void
Bookkeeper::record_vars_with_bitfields(const Type *type)
{
	assert(type);
	const Type *base_type = type->get_base_type();
	if (!base_type->is_aggregate() ||
		(!base_type->has_bitfields()))
		return;

	int level = type->get_indirect_level();
	incr_counter(Bookkeeper::vars_with_bitfields, level);
	if (type->is_full_bitfields_struct())
		incr_counter(Bookkeeper::vars_with_full_bitfields, level);
}

void
Bookkeeper::record_type_with_bitfields(const Type *typ)
{
	if (!typ->is_aggregate()) return;

	if (typ->has_bitfields()) {
		Bookkeeper::structs_with_bitfields++;
		size_t len = typ->bitfields_length_.size();
		assert(len == typ->fields.size());
		for (size_t i = 0; i < len; ++i) {
			if (!typ->is_bitfield(i))
				continue;

			Bookkeeper::bitfields_in_total++;
			if (typ->bitfields_length_[i] == 0)
				Bookkeeper::unamed_bitfields_in_total++;

			CVQualifiers qual = typ->qfers_[i];
			if (qual.is_const())
				Bookkeeper::const_bitfields_in_total++;
			if (qual.is_volatile())
				Bookkeeper::volatile_bitfields_in_total++;
		}
	}
}

void
Bookkeeper::output_jump_statistics(std::ostream &out)
{
	formated_output(out, "forward jumps: ", forward_jump_cnt);
	formated_output(out, "backward jumps: ", backward_jump_cnt);
}

void
Bookkeeper::output_var_freshness(std::ostream &out)
{
	int total = use_new_var_cnt + use_old_var_cnt;
	formated_outputf(out, "percentage a fresh-made variable is used: ", use_new_var_cnt * 100.0 / total);
	formated_outputf(out, "percentage an existing variable is used: ", use_old_var_cnt * 100.0 / total);
}

void
Bookkeeper::output_counters(std::ostream &out, const char* prefix_msg,
		const char* breakdown_msg, const std::vector<int> &counters, int starting_pos)
{
	assert(prefix_msg && breakdown_msg);
	formated_output(out, prefix_msg, calc_total(counters));
	out << "breakdown:" << std::endl;
	for (size_t i=starting_pos; i<counters.size(); i++) {
		out << "   " << breakdown_msg << ": " << i << ", occurrence: " << counters[i] << endl;
	}
}

void incr_counter(std::vector<int>& counters, int pos)
{
	size_t len = counters.size();
	size_t index = (size_t)pos;
	if (index >= len) {
		counters.resize(index+1);
		for (size_t i= len; i<=index; i++) {
			counters[i] = 0;
		}
	}
	counters[index]++;
}

int calc_total(const std::vector<int>& counters)
{
	size_t i;
	int total = 0;
	for (i=0; i<counters.size(); i++) {
		total += counters[i];
	}
	return total;
}

///////////////////////////////////////////////////////////////////////////////

// Local Bookkeepers:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

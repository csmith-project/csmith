// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2015 The University of Utah
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

#ifndef BOOKKEEPER_H
#define BOOKKEEPER_H

///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
using namespace std;

class Variable;
class Expression;
class Statement;
class Fact;
class Type;

class Bookkeeper
{
public:
	Bookkeeper(void);
	~Bookkeeper(void);

	static void doFinalization();

	static void output_statistics(std::ostream &out);

	static void output_struct_union_statistics(std::ostream &out);

	static void output_expr_statistics(std::ostream &out);

	static void output_pointer_statistics(std::ostream &out);

	static void output_jump_statistics(std::ostream &out);

	static void output_stmts_statistics(std::ostream &out);

	static void output_volatile_access_statistics(std::ostream &out);

	static void output_counters(std::ostream &out, const char* prefix_msg,
		const char* breakdown_msg, const std::vector<int> &counters, int starting_pos = 0);

	static void update_ptr_aliases(const vector<Fact*>& facts, vector<const Variable*>& ptrs, vector<vector<const Variable*> >& aliases);

	static void record_address_taken(const Variable *var);

	static void record_pointer_comparisons(const Expression* lhs, const Expression* rhs);

	static void record_volatile_access(const Variable* var, int deref_level, bool write);

	static void record_type_with_bitfields(const Type* typ);

	static void record_vars_with_bitfields(const Type *type);

	static void record_bitfields_writes(const Variable *var);

	static void record_bitfields_reads(const Variable *var);

	static void output_bitfields(std::ostream &out);

	static void output_var_freshness(std::ostream &out);

	static void stat_expr_depths_for_stmt(const Statement* s);
	static void stat_expr_depths(void);

	static int  stat_blk_depths_for_stmt(const Statement* s);
	static int  stat_blk_depths(void);

	static std::vector<int> struct_depth_cnts;

	static int union_var_cnt;

	static std::vector<int> expr_depth_cnts;

	static std::vector<int> blk_depth_cnts;

	static std::vector<int> dereference_level_cnts;

	static int address_taken_cnt;

	static std::vector<int> write_dereference_cnts;

	static std::vector<int> read_dereference_cnts;

	static int cmp_ptr_to_null;
	static int cmp_ptr_to_ptr;
	static int cmp_ptr_to_addr;

	static int read_volatile_cnt;
	static int read_volatile_thru_ptr_cnt;
	static int write_volatile_cnt;
	static int write_volatile_thru_ptr_cnt;
	static int read_non_volatile_cnt;
	static int write_non_volatile_cnt;

	static int pointer_avail_for_dereference;
	static int volatile_avail;

	static int structs_with_bitfields;
	static std::vector<int> vars_with_bitfields;
	static std::vector<int> vars_with_full_bitfields;
	static int vars_with_bitfields_address_taken_cnt;
	static int bitfields_in_total;
	static int unamed_bitfields_in_total;
	static int const_bitfields_in_total;
	static int volatile_bitfields_in_total;
	static int lhs_bitfields_structs_vars_cnt;
	static int rhs_bitfields_structs_vars_cnt;
	static int lhs_bitfield_cnt;
	static int rhs_bitfield_cnt;

	static int forward_jump_cnt;
	static int backward_jump_cnt;

	static int use_new_var_cnt;
	static int use_old_var_cnt;

	static bool rely_on_int_size;
	static bool rely_on_ptr_size;
};

void incr_counter(std::vector<int>& counters, int index);
int calc_total(const std::vector<int>& counters);

///////////////////////////////////////////////////////////////////////////////

#endif // BOOKKEEPER_H

// Local Bookkeepers:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

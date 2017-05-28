// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2015 The University of Utah
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

#ifndef CGCONTEXT_H
#define CGCONTEXT_H

///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
#include "Effect.h"
using namespace std;

// Flags..
#define NO_RETURN (1)
#define IN_LOOP (2)
#define DO_HASH (4)
#define NO_DANGLING_PTR (8)
#define INVISIBLE (9999)
#define INACTIVE (8888)

class Effect;
class Function;
class Variable;
class Fact;
class Block;
class Type;
class Lhs;
class Expression;
class FactMgr;
class Statement;
class ExpressionVariable;
class ArrayVariable;

typedef std::vector<const Variable *> VariableSet;

class RWDirective
{
public:
	RWDirective(const VariableSet& no_reads, const VariableSet& no_writes, VariableSet& reads, VariableSet& writes)
		: no_read_vars(no_reads),
		  no_write_vars(no_writes),
		  must_read_vars(reads),
		  must_write_vars(writes)
	{};

	void find_must_use_arrays(std::vector<const Variable*>& avs) const;

	// The set of variables that should not be read/written. Currently not used
	// could be useful for generating multi-thread functions with no data racing
	const VariableSet &no_read_vars;
	const VariableSet &no_write_vars;
	// The set of variables that must be read/written.  Again, this is not
	// about generating conforming code; it is simply about directing the
	// code generator.
	//
	VariableSet& must_read_vars;
	VariableSet& must_write_vars;
};

/*
 *
 */
class CGContext
{
public:
	static const VariableSet empty_variable_set;

	// original constructor, created at the beginning of generating a function
	CGContext(Function *current_func, const Effect &eff_context, Effect *eff_accum);
	// create a CGContext for parameters from an existing CGContext
	CGContext(const CGContext &cgc, const Effect &eff_context, Effect *eff_accum);
	// create a CGContext for callees from an existing CGContext
	CGContext(const CGContext &cgc, Function* f, const Effect &eff_context, Effect *eff_accum);
	// create a CGContext for loops from an existing CGContext
	CGContext(const CGContext &cgc, RWDirective* lc, const Variable* iv, unsigned int bound);

	~CGContext(void);

	void output_call_chain(std::ostream &out);

	bool is_frame_var(const Variable* v) const;

	bool in_conflict(const Effect& eff) const;

	bool accept_type(const Type* t) const;

	Function *get_current_func(void) const			{ return current_func; }

	Block *get_current_block(void) const;

	static const CGContext& get_empty_context(void) { return empty_context;}

	const Effect &get_effect_context(void) const	{ return effect_context; }
	Effect *get_effect_accum(void) const			{ return effect_accum; }
	Effect get_accum_effect(void) const				{ Effect e; return effect_accum ? *effect_accum : e; }
	Effect& get_effect_stm(void) 					{ return effect_stm; }

	void find_reachable_frame_vars(vector<const Fact*>& facts, VariableSet& frame_vars) const;
	void get_external_no_reads_writes(VariableSet& no_reads, VariableSet& no_writes, const VariableSet& frame_vars) const;

	bool is_nonreadable(const Variable *v) const;
	bool is_nonwritable(const Variable *v) const;

	bool check_deref_volatile(const Variable *v, int deref_level);
	void read_var(const Variable *v);
	void write_var(const Variable *v);
	bool check_read_var(const Variable *v, const std::vector<const Fact*>& facts);
	bool check_write_var(const Variable *v, const std::vector<const Fact*>& facts);
	bool read_indices(const Variable* v, const vector<const Fact*>& facts);
	bool read_pointed(const ExpressionVariable* v, const std::vector<const Fact*>& facts);
	bool write_pointed(const Lhs* v, const std::vector<const Fact*>& facts);
	void add_effect(const Effect &e, bool include_lhs_effects=false);
	void add_external_effect(const Effect &e);
	void add_visible_effect(const Effect &e, const Block* b);
	void merge_param_context(const CGContext& param_cg_context, bool include_lhs_effects=false);

    int find_variable_scope(const Variable* var) const;

	void extend_call_chain(const CGContext& cg_context);

	void reset_effect_accum(const Effect& e) { if (effect_accum) *effect_accum = e;}
	void reset_effect_stm(const Effect& e) { effect_stm = e;}

	bool allow_volatile() const;
	bool allow_const(Effect::Access access) const;

private:
	Function * const current_func; // may be null.


public: // XXX
	int blk_depth;
	int expr_depth;
	unsigned int flags;
	std::vector<const Block*> call_chain;
	const Block* curr_blk;
	RWDirective* rw_directive;
	// induction variables for loops, with each IV controls one nested loop
	map<const Variable*, unsigned int> iv_bounds;

	const Expression* curr_rhs;   // only used in the context of LHS

private:
	const Effect &effect_context;
	Effect *effect_accum; // may be null!
	Effect effect_stm;
	// TODO: move `Function::flags' to here.
	// TODO: move `Function::...' to here?

private:
	CGContext &operator=(const CGContext &cgc);	// unimplementable
	void sanity_check(void);
	static const CGContext empty_context;
};

///////////////////////////////////////////////////////////////////////////////

#endif // CGCONTEXT_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

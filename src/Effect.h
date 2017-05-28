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

#ifndef EFFECT_H
#define EFFECT_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <vector>

class Variable;
class Block;
class ExpressionVariable;

///////////////////////////////////////////////////////////////////////////////

class Effect
{
public:
	Effect(void);
	Effect(const Effect &e);
	~Effect(void);

	Effect &operator=(const Effect &e);

	void read_var(const Variable *v);
	void access_deref_volatile(const Variable *v, int deref_level);
	void write_var(const Variable *v);
	void write_var_set(const std::vector<const Variable *>& vars);
	void add_effect(const Effect &e, bool include_lhs_effects = false);
	void add_external_effect(const Effect &e, std::vector<const Block*> call_chain);
	void add_external_effect(const Effect &e);
	void clear(void);

	bool is_pure(void) const				{ return pure; }
	bool is_side_effect_free(void) const	{ return side_effect_free; }

	bool is_read(const Variable *v) const;
	bool is_read(std::string vname) const;
	bool is_written(const Variable *v) const;
	bool is_written(std::string vname) const;
	bool field_is_read(const Variable *v) const;
	bool field_is_written(const Variable *v) const;
	bool sibling_union_field_is_read(const Variable *v) const;
	bool sibling_union_field_is_written(const Variable *v) const;
	bool is_read_partially(const Variable* v) const;
	bool is_written_partially(const Variable* v) const;
	bool union_field_is_read(void) const;
	bool has_race_with(const Effect &e) const;
	bool is_empty(void) const;
	void consolidate(void);

	static const Effect &get_empty_effect(void)	{ return Effect::empty_effect; }

	const std::vector<const Variable *>& get_read_vars(void) const { return read_vars;}
	const std::vector<const Variable *>& get_write_vars(void) const { return write_vars;}
	const std::vector<const Variable *>& get_lhs_write_vars(void) const { return lhs_write_vars;}
	void set_lhs_write_vars(const std::vector<const Variable *>& vars)  { lhs_write_vars = vars;}

	typedef enum {
		READ, WRITE
	} Access;

	void Output(std::ostream &out) const;
	bool has_global_effect(void) const;
	void update_purity(void);

private:
	std::vector<const Variable *> read_vars;
	std::vector<const Variable *> write_vars;
	std::vector<const Variable *> lhs_write_vars;

	bool pure;
	bool side_effect_free;

	static const Effect empty_effect;
};

///////////////////////////////////////////////////////////////////////////////

#endif // EFFECT_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

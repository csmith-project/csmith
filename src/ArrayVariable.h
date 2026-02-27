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

#ifndef ARRAY_VARIABLE_H
#define ARRAY_VARIABLE_H

#define INVALID_BOUND 0xFFFFFFFF

///////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <string>
#include "Variable.h"
class Expression;
class Block;
using namespace std;

class ArrayVariable : public Variable
{
public:
	static ArrayVariable* CreateArrayVariable(const CGContext& cg_context, Block* blk, const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer, const Variable* isFieldVarOf);
	ArrayVariable(const ArrayVariable& av);
	virtual ~ArrayVariable(void);

	void add_index(const Expression* e);
	void set_index(size_t index, const Expression* e);
	virtual size_t get_dimension(void) const { return sizes.size();}
	unsigned long size_in_bytes(void) const;
	unsigned long get_size(void) const;
	std::vector<unsigned int> get_sizes(void) const { return sizes;}
	const std::vector<const Expression*>& get_indices(void) const { return indices;}
	const std::vector<const Expression*>& get_more_init_values(void) const { return init_values;}
	bool no_loop_initializer(void) const;

	ArrayVariable* itemize(void) const;
	ArrayVariable* itemize(const vector<int>& const_indices) const;
	ArrayVariable* itemize(const std::vector<const Variable*>& indices, Block* blk) const;
	ArrayVariable* itemize(const std::vector<const Expression*>& indices, Block* blk) const;
	ArrayVariable* rnd_mutate(void);
	bool is_variant(const Variable* v) const;
	virtual bool is_global(void) const;
	virtual bool is_visible_local(const Block* blk) const;

	string make_print_index_str(const vector<const Variable*> &cvs) const;
	virtual void Output(std::ostream &) const;
	virtual void OutputDef(std::ostream &out, int indent) const;
	virtual void OutputDecl(std::ostream &) const;
	virtual void hash(std::ostream& out) const;
	virtual const Variable* get_collective(void) const { return collective ? collective : this;}
	virtual const ArrayVariable* get_array(string& /*field*/) const { return this;}
	virtual void OutputLowerBound(std::ostream &) const;
	virtual void OutputUpperBound(std::ostream &) const;
	void output_with_indices(std::ostream &out, const std::vector<const Variable*>& cvs) const;
	void output_checksum_with_indices(std::ostream &out, const std::vector<const Variable*>& cvs, string field_name) const;
	void output_init(std::ostream &out, const Expression* init, const vector<const Variable*>& cvs, int indent) const;
	void output_addr_checks(std::ostream &out, const Variable* var, string field_name, int indent) const;
	void add_init_value(const Expression* e) { init_values.push_back(e);}
	const vector<const Expression*>& get_init_values(void) const { return init_values;}
	string build_initializer_str(const vector<string>& init_strings) const;
	string build_init_recursive(size_t dimen, const vector<string>& init_strings) const;

	const ArrayVariable* collective;
	Block* parent;
private:
	ArrayVariable(Block* blk, const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer, const vector<unsigned int>& sizes, const Variable* isFieldVarOf);


	const std::vector<unsigned int> sizes;
	std::vector<const Expression*> indices;
	std::vector<const Expression*> init_values;
};

///////////////////////////////////////////////////////////////////////////////

#endif // ARRAY_VARIABLE_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

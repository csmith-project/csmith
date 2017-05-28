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

#ifndef STATEMENT_ARRAY_OP_H
#define STATEMENT_ARRAY_OP_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <vector>
#include "Statement.h"
#include "StatementAssign.h"

class Block;
class CGContext;
class Expression;
class FactMgr;
class ArrayVariable;

/*
 *
 */
class StatementArrayOp : public Statement
{
public:
	// Factory method.
	static Statement *make_random(CGContext &cg_context);
	static StatementArrayOp *make_random_array_init(CGContext &cg_context);
	StatementArrayOp(Block* b, const ArrayVariable* av,
				   const std::vector<const Variable*>& cvs,
				   const std::vector<int>& inits,
				   const std::vector<int>& incrs,
				   const Block *body);
	StatementArrayOp(Block* b, const ArrayVariable* av,
				   const std::vector<const Variable*>& cvs,
				   const std::vector<int>& inits,
				   const std::vector<int>& incrs,
				   const Expression *e);
	virtual ~StatementArrayOp(void);

	void output_header(std::ostream& out, int& indent) const;
	virtual void get_exprs(std::vector<const Expression*>& exps) const { if (init_value) exps.push_back(init_value);}
	virtual void get_blocks(std::vector<const Block*>& blks) const { if (body) blks.push_back(body);}
	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;
	virtual void Output(std::ostream &out, FactMgr* fm, int indent = 0) const;

	const ArrayVariable* array_var;
	const std::vector<const Variable*> ctrl_vars;
	const std::vector<int> inits;
	const std::vector<int> incrs;
	const Block *body;
	const Expression *init_value;
private:
	static void make_random_iter_ctrl(int size, int &init, int &incr);
};

///////////////////////////////////////////////////////////////////////////////

#endif // STATEMENT_ARRAY_OP_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

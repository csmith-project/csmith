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

#ifndef STATEMENT_GOTO_H
#define STATEMENT_GOTO_H

///////////////////////////////////////////////////////////////////////////////
#include <ostream>
#include <string>
#include <map>
#include "Statement.h"
using namespace std;

class Expression;
class Block;
class Variable;
class CGContext;
class Statement;
/*
 *
 */
class StatementGoto : public Statement
{
public:
	// Factory method.
	static StatementGoto *make_random(CGContext &cg_context);

	StatementGoto(Block* b, const Expression &test, const Statement* dest, const std::vector<const Variable*>& vars);
	StatementGoto(const StatementGoto &sg);
	virtual ~StatementGoto(void);
	//
	static bool has_init_skipped_vars(const Block* src_blk, const Statement* dest);
	static Block* find_good_jump_block(vector<Block*>& blocks, const Block* blk, bool as_dest);

	virtual bool must_jump(void) const;
	virtual void get_exprs(std::vector<const Expression*>& exps) const {exps.push_back(&test);}
	virtual void get_blocks(std::vector<const Block*>& /* blks */) const {};
	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;
	virtual void Output(std::ostream &out, FactMgr* fm, int indent = 0) const;
	void output_skipped_var_inits(std::ostream &out, int indent) const;

	static void doFinalization(void);

	const Expression &test;
	const Statement* dest;
	std::string label;
	std::vector<const Variable*> init_skipped_vars;
	static std::map<const Statement*, std::string> stm_labels;
};

///////////////////////////////////////////////////////////////////////////////

#endif // STATEMENT_CONTINUE_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

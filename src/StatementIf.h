// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011 The University of Utah
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

#ifndef STATEMENT_IF_H
#define STATEMENT_IF_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>

#include "Statement.h"

class Block;
class CGContext;
class Expression;
class FactMgr;

/*
 *
 */
class StatementIf : public Statement
{
public:
	// Factory method.
	static StatementIf *make_random(CGContext &cg_context);

	StatementIf(Block* b, const Expression &test,
				const Block &if_true, const Block &if_false);
	StatementIf(const StatementIf &si);
	virtual ~StatementIf(void);

	virtual void get_exprs(std::vector<const Expression*>& exps) const {exps.push_back(&test);}
	virtual void get_blocks(std::vector<const Block*>& blks) const { blks.push_back(&if_true); blks.push_back(&if_false);}

	const Block* get_true_branch(void) const { return &if_true; };
	const Block* get_false_branch(void) const { return &if_false; };
	const Expression* get_test(void) const { return &test; };

	void combine_branch_facts(vector<const Fact*>& pre_facts) const;

	virtual bool must_return(void) const;
	virtual bool must_jump(void) const;
	//
	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;

	virtual void Output(std::ostream &out, FactMgr* fm, int indent = 0) const;
	void output_condition(std::ostream &out, FactMgr* fm, int indent = 0) const;
	void output_branches(std::ostream &out, FactMgr* fm, int indent = 0) const;

private:
	const Expression &test;
	const Block &if_true;
	const Block &if_false;

	//

	StatementIf &operator=(const StatementIf &si); // unimplementable
};

///////////////////////////////////////////////////////////////////////////////

#endif // STATEMENT_IF_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

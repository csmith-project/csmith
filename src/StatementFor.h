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

#ifndef STATEMENT_FOR_H
#define STATEMENT_FOR_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>

#include "Statement.h"
#include "StatementAssign.h"

class Block;
class CGContext;
class Expression;
class FactMgr;

/*
 *
 */
class StatementFor : public Statement
{
public:
	// Factory method.
	static StatementFor *make_random(CGContext &cg_context);

	static StatementFor *make_random_array_loop(const CGContext &cg_context);

	static const Variable* make_iteration(CGContext& cg_context, StatementAssign*& begin, Expression*& exit_cond, StatementAssign*& step, unsigned int& bound);

	StatementFor(Block* b,
				 const StatementAssign &init,
				 const Expression &test,
				 const StatementAssign &incr,
				 const Block &body);
	virtual ~StatementFor(void);

	void post_loop_analysis(CGContext& cg_context, vector<const Fact*>& pre_facts, Effect& pre_effect);
	const StatementAssign* get_init(void) const { return &init;};
	const Expression* get_test(void) const { return &test; };
	const StatementAssign* get_incr(void) const { return &incr; };
	const Block* get_body(void) const { return &body; };
	void output_header(std::ostream& out, int indent) const;
	//
	virtual void get_blocks(std::vector<const Block*>& blks) const { blks.push_back(&body);}

	virtual void get_exprs(std::vector<const Expression*>& exps) const {exps.push_back(&test);}

	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;

	virtual void Output(std::ostream &out, FactMgr* fm, int indent = 0) const;

private:
	// XXX --- `init' and `incr' have the "wrong types."  Should be exprs.
	const StatementAssign &init;
	const Expression      &test;
	const StatementAssign &incr;

	const Block &body;

	//

	StatementFor(const StatementFor &sf);
	StatementFor &operator=(const StatementFor &sf); // unimplementable
};

///////////////////////////////////////////////////////////////////////////////

#endif // STATEMENT_FOR_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

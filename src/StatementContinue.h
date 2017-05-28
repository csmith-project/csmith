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

#ifndef STATEMENT_CONTINUE_H
#define STATEMENT_CONTINUE_H

///////////////////////////////////////////////////////////////////////////////
#include <ostream>
#include "Statement.h"

class Expression;
class Block;
class CGContext;
/*
 *
 */
class StatementContinue : public Statement
{
public:
	// Factory method.
	static StatementContinue *make_random(CGContext &cg_context);

	StatementContinue(Block* parent, const Expression &test, const Block& b);
	StatementContinue(const StatementContinue &sc);
	virtual ~StatementContinue(void);
	//
	virtual bool must_jump(void) const;
	virtual void get_blocks(std::vector<const Block*>& /* blks */) const {};
	virtual void get_exprs(std::vector<const Expression*>& exps) const {exps.push_back(&test);}
	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;
	virtual void Output(std::ostream &out, FactMgr* fm, int indent = 0) const;

	const Expression &test;
	const Block& loop_blk;
};

///////////////////////////////////////////////////////////////////////////////

#endif // STATEMENT_CONTINUE_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

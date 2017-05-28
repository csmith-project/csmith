// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2015, 2017 The University of Utah
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

#include "StatementContinue.h"
#include <iostream>
#include <cassert>
#include "CGOptions.h"
#include "CGContext.h"
#include "Block.h"
#include "Type.h"
#include "Function.h"
#include "Expression.h"
#include "FactMgr.h"
#include "Bookkeeper.h"
#include "Error.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
StatementContinue *
StatementContinue::make_random(CGContext &cg_context)
{
	//static int g =0;
	FactMgr* fm = get_fact_mgr(&cg_context);
	// find the closest looping parent block: the one "continue"
	// would apply to
	//int h = g++;
	const Block* b = cg_context.get_current_block();
	const Statement* prev_stm = b->get_last_stm();
	// don't generate "continue" as the first statement in a block
	if (prev_stm == 0) {
		return 0;
	}
	while (b && !b->looping) {
		b = b->parent;
	}
	assert(b);
	cg_context.get_effect_stm().clear();
	Expression *expr = Expression::make_random(cg_context, get_int_type(), 0, true, true, eVariable);
	ERROR_GUARD(NULL);
	StatementContinue* sc = new StatementContinue(cg_context.get_current_block(), *expr, *b);
	fm->create_cfg_edge(sc, b, false, true);
    return sc;
}

/*
 *
 */
StatementContinue::StatementContinue(Block* parent, const Expression &test, const Block &b)
	: Statement(eContinue, parent),
	  test(test),
	  loop_blk(b)
{
	// Nothing else to do.
}

/*
 *
 */
StatementContinue::StatementContinue(const StatementContinue &sc)
: Statement(sc.get_type(), sc.parent),
	  test(sc.test),
	  loop_blk(sc.loop_blk)
{
	// Nothing else to do.
}

/*
 *
 */
StatementContinue::~StatementContinue(void)
{
	delete &test;
}

/*
 * return true if condition is always true
 */
bool
StatementContinue::must_jump(void) const
{
	return test.not_equals(0);
}

/*
 *
 */
void
StatementContinue::Output(std::ostream &out, FactMgr* /*fm*/, int indent) const
{
	output_tab(out, indent);
	out << "if (";
	test.Output(out);
	out << ")";
	outputln(out);
	output_tab(out, indent+1);
	out << "continue;";
	outputln(out);
}

bool
StatementContinue::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	// evaludate condition first
	if (!test.visit_facts(inputs, cg_context)) {
		return false;
	}
	FactMgr* fm = get_fact_mgr(&cg_context);
	fm->map_stm_effect[this] = cg_context.get_effect_stm();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

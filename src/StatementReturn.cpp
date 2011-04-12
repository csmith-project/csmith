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

#include <cassert>

#include "CGContext.h"
#include "CGOptions.h"
#include "Function.h"
#include "StatementReturn.h"
#include "Variable.h"
#include "ExpressionVariable.h"
#include "FactMgr.h"
#include "Bookkeeper.h"
#include "Error.h"
#include "util.h"
#include "DepthSpec.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
StatementReturn *
StatementReturn::make_random(CGContext &cg_context)
{
	DEPTH_GUARD_BY_TYPE_RETURN(dtStatementReturn, NULL);
	Function *curr_func = cg_context.get_current_func();
	assert(curr_func);
	FactMgr* fm = get_fact_mgr(&cg_context);
	assert(fm); 

	cg_context.expr_depth = 0;
	ExpressionVariable* ev = ExpressionVariable::make_random(cg_context, curr_func->return_type, &curr_func->rv->qfer, false, true); 
	// XXX
	ERROR_GUARD(NULL);
	incr_counter(Bookkeeper::expr_depth_cnts, cg_context.expr_depth);

	StatementReturn* sr = new StatementReturn(*ev);
	return sr;
}

std::vector<const ExpressionVariable*> 
StatementReturn::get_dereferenced_ptrs(void) const
{ 
	return var.get_dereferenced_ptrs();
}

bool 
StatementReturn::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{ 
	if (!var.visit_facts(inputs, cg_context)) {
		return false;
	}
	update_fact_for_return(this, inputs); 
	FactMgr* fm = get_fact_mgr(&cg_context);
	fm->map_stm_effect[this] = cg_context.get_effect_stm();
	return true;
}

/*
 *
 */
StatementReturn::StatementReturn(const ExpressionVariable &v)
	: Statement(eReturn),
	  var(v)
{
	// Nothing else to do.
}

/*
 *
 */
StatementReturn::StatementReturn(const StatementReturn &sr)
	: Statement(sr.get_type()),
	  var(sr.var)
{
	// Nothing else to do.
}

/*
 *
 */
StatementReturn::~StatementReturn(void)
{
	delete &var;
}

/*
 *
 */
void
StatementReturn::Output(std::ostream &out, FactMgr* /*fm*/, int indent) const
{
	output_tab(out, indent);
	// XXX --- Fix this.  Outputting two stmts instead of one is bad mojo.
	if (CGOptions::depth_protect()) {
		out << "DEPTH--;" << endl;
	} 
	out << "return "; 
	var.Output(out);
	out << ";";
	outputln(out);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

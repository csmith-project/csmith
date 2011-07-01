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

#include "ExpressionComma.h"
#include <cassert>
#include "Common.h"
#include "CGContext.h"
#include "CGOptions.h"
#include "FactMgr.h"
#include "Bookkeeper.h"
#include "StringUtils.h"
#include "Block.h"

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
Expression *
ExpressionComma::make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer)
{
	Expression *e = 0;
	//++cg_context.stmt_depth;
	//bool std_func = ExpressionFunctionProbability(cg_context);
	//ERROR_GUARD(NULL);
 //   // unary/binary "functions" produce scalar types only
	//if (type && (type->eType != eSimple || type->simple_type == eVoid))
	//	std_func = false;

	//Effect effect_accum = cg_context.get_accum_effect();
	//Effect effect_stm = cg_context.get_effect_stm(); 
	//FactMgr* fm = get_fact_mgr(&cg_context);
	//vector<const Fact*> facts_copy = fm->global_facts; 
	//FunctionInvocation *fi = FunctionInvocation::make_random(std_func, cg_context, type, qfer);
	//ERROR_GUARD(NULL);

	//if (fi->failed) { 
	//	// if it's a invalid invocation, (see FunctionInvocationUser::revisit) 
	//	// restore the env, and replace invocation with a simple var
	//	cg_context.reset_effect_accum(effect_accum);
	//	cg_context.reset_effect_stm(effect_stm);
	//	fm->restore_facts(facts_copy);
	//	e = ExpressionVariable::make_random(cg_context, type, qfer);
	//	delete fi;
	//}
	//else {
	//	e = new ExpressionComma(*fi);
	//}
	//--cg_context.stmt_depth;
	return e;
}

/*
 *
 */
ExpressionComma::~ExpressionComma(void)
{
	delete &lhs;
	delete &rhs;
}

bool 
ExpressionComma::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const 
{
	if (!lhs.visit_facts(inputs, cg_context)) {
		return false;
	}
	return rhs.visit_facts(inputs, cg_context);
}

vector<const ExpressionVariable*> 
ExpressionComma::get_dereferenced_ptrs(void) const
{
	vector<const ExpressionVariable*> ptrs1 = lhs.get_dereferenced_ptrs();
	vector<const ExpressionVariable*> ptrs2 = rhs.get_dereferenced_ptrs();
	ptrs1.insert(ptrs1.end(), ptrs2.begin(), ptrs2.end());
	return ptrs1;
}
	
void
ExpressionComma::Output(std::ostream &out) const
{
	Reducer* reducer = CGOptions::get_reducer();
	if (reducer && reducer->output_expr(this, out)) {
		return;
	} 
	lhs.Output(out);
	rhs.Output(out);
}

void 
ExpressionComma::indented_output(std::ostream &out, int indent) const 
{ 
	output_tab(out, indent);
	Output(out);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

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

//
// This file was derived from a random program generator written by Bryan
// Turner.  The attributions in that file was:
//
// Random Program Generator
// Bryan Turner (bryan.turner@pobox.com)
// July, 2005
//

#include "Expression.h"
#include <cassert>

#include "Common.h"
#include "CGContext.h"
#include "CGOptions.h"
#include "Effect.h"
#include "Function.h"
#include "VectorFilter.h"
#include "Variable.h"
#include "Constant.h" // temporary; don't want to depend on subclass!
#include "ExpressionFuncall.h" // temporary; don't want to depend on subclass!
#include "ExpressionVariable.h" // temporary; don't want to depend on subclass!
#include "ExpressionAssign.h"
#include "ExpressionComma.h"
#include "Error.h"
#include "ProbabilityTable.h"
#include "PartialExpander.h"
#include "random.h"
#include "CVQualifiers.h"
#include "DepthSpec.h"

int eid = 0;

ProbabilityTable<unsigned int, int> *Expression::exprTable_ = NULL;

void
Expression::InitExprProbabilityTable()
{
	if (Expression::exprTable_)
		return;

	Expression::exprTable_ = new ProbabilityTable<unsigned int, int>();
	Expression::exprTable_->add_elem(40, (int)eFunction);
	Expression::exprTable_->add_elem(50, (int)eAssignment);
	Expression::exprTable_->add_elem(60, (int)eCommaExpr);
	Expression::exprTable_->add_elem(70, (int)eConstant);
	Expression::exprTable_->add_elem(100, (int)eVariable);
}

ProbabilityTable<unsigned int, int> *Expression::paramTable_ = NULL;
void
Expression::InitParamProbabilityTable()
{
	if (Expression::paramTable_)
		return;

	Expression::paramTable_ = new ProbabilityTable<unsigned int, int>();
	Expression::paramTable_->add_elem(30, (int)eFunction);
	Expression::paramTable_->add_elem(40, (int)eConstant);
	Expression::paramTable_->add_elem(50, (int)eAssignment);
	Expression::paramTable_->add_elem(60, (int)eCommaExpr);
	Expression::paramTable_->add_elem(100, (int)eVariable);
}

void
Expression::InitProbabilityTables()
{
	Expression::InitExprProbabilityTable();
	Expression::InitParamProbabilityTable();
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
static eTermType
ExpressionTypeProbability(const CGContext &cg_context, const VectorFilter *filter)
{
	if (cg_context.expr_depth > CGOptions::max_expr_depth())
		return eVariable;

	if (PartialExpander::direct_expand_check(eInvoke))
		return eFunction;

	assert(filter);

	int i = rnd_upto(100, filter);
	ERROR_GUARD(MAX_TERM_TYPES);
	return (eTermType)(filter->lookup(i));
}

unsigned int
Expression::func_count(void) const
{
	std::vector<const FunctionInvocationUser*> funcs;
	get_called_funcs(funcs);
	return funcs.size();
}

std::string
Expression::to_string(void) const
{
	ostringstream oss;
	Output(oss); 
	return oss.str();
}

std::vector<const ExpressionVariable*> 
Expression::get_dereferenced_ptrs(void) const
{
	// return a empty vector by default
	std::vector<const ExpressionVariable*> empty;
	return empty;
}

void 
Expression::indented_output(std::ostream &out, int indent) const 
{ 
	output_tab(out, indent); 
	Output(out);
}

/*
 *
 */
Expression *
Expression::make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer, bool no_func, bool no_const, enum eTermType tt)
{
	DEPTH_GUARD_BY_TYPE_RETURN_WITH_FLAG(dtExpression, tt, NULL);
	Expression::InitProbabilityTables();

	Expression *e = 0;  
	if (type == NULL) {
		do { 
			type = cg_context.get_effect_context().is_side_effect_free() ? Type::choose_random_nonvoid() : Type::choose_random_nonvoid_nonvolatile();
		} while (type->eType == eStruct && tt == eConstant);
	}
	assert(!(no_func && tt == eFunction));
	assert(!(no_const && tt == eConstant));
	// constant struct variables can not be a subexpression?
	assert(!(type->eType == eStruct && tt == eConstant));
	 
	// if no term type is provided, choose a random term type with restrictions
	if (tt == MAX_TERM_TYPES) {
		VectorFilter filter(Expression::exprTable_);
		if (no_func || 
			(!CGOptions::return_structs() && type->eType == eStruct) ||
			(!CGOptions::return_unions() && type->eType == eUnion)) {
			filter.add(eFunction);
		}
		// struct constants can't be subexpressions (union constant can't either?) 
		if (no_const || type->eType == eStruct || type->eType == eUnion) {
			filter.add(eConstant);
		}
		if (type->is_const_struct_union()) {
			filter.add(eAssignment);
		}
		tt = ExpressionTypeProbability(cg_context, &filter); 
	}
	    
	ERROR_GUARD(NULL);

	switch (tt) {
	case eConstant:
		if (type->eType == eSimple)
			assert(type->simple_type != eVoid);
		e = Constant::make_random(type);
		break;
	case eVariable:
		e = ExpressionVariable::make_random(cg_context, type, qfer);
		break;
	case eFunction:
		cg_context.expr_depth++;
		e = ExpressionFuncall::make_random(cg_context, type, qfer);
		break;
	case eAssignment:
		e = ExpressionAssign::make_random(cg_context, type, qfer);
		break;
	case eCommaExpr: 
		e = ExpressionComma::make_random(cg_context, type, qfer);
		break;
	default: break;
	}
#if 0
	if (!cg_context.get_effect_context().is_side_effect_free()) {
		assert(e->effect.is_side_effect_free());
	}
#endif 

	ERROR_GUARD(NULL);
	return e;
}

/*
 *
 */
Expression *
Expression::make_random_param(CGContext &cg_context, const Type* type, const CVQualifiers* qfer, enum eTermType tt)
{
	DEPTH_GUARD_BY_TYPE_RETURN_WITH_FLAG(dtExpressionRandomParam, tt, NULL);
	Expression::InitProbabilityTables();
	Expression *e = 0;  
	assert(type);
	// if a term type is provided, no need to choose random term type
	if (tt == MAX_TERM_TYPES) {
		VectorFilter filter(Expression::paramTable_);
		filter.add(eConstant);   // no constants as function call parameters
		if ((!CGOptions::return_structs() && type->eType == eStruct) ||
			(!CGOptions::return_unions() && type->eType == eUnion)) {
			filter.add(eFunction);
		}
		if (type->is_const_struct_union()) {
			filter.add(eAssignment);
		}
		tt = ExpressionTypeProbability(cg_context, &filter);
	}
	 
	ERROR_GUARD(NULL);

	switch (tt) {
	case eConstant:
		if (type->eType == eSimple)
			assert(type->simple_type != eVoid);
		e = Constant::make_random(type);
		break;
	case eVariable:
		e = ExpressionVariable::make_random(cg_context, type, qfer, true);
		break; 
	case eFunction:
		cg_context.expr_depth++;
		e = ExpressionFuncall::make_random(cg_context, type, qfer); 
		break;
	case eAssignment:
		e = ExpressionAssign::make_random(cg_context, type, qfer);
		break;
	case eCommaExpr: 
		e = ExpressionComma::make_random(cg_context, type, qfer);
		break;
	default: break;
	} 

	ERROR_GUARD(NULL);
	return e;
}

/*
 *
 */
Expression::Expression(eTermType e) :
    term_type(e),
	expr_id(eid++)
{
	// Nothing to do.
}

Expression::Expression(const Expression &expr)
	: term_type(expr.term_type)
{
	
}
/*
 *
 */
Expression::~Expression(void)
{
	// Nothing to do.
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

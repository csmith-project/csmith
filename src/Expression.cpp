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

#include "Error.h"
#include "ProbabilityTable.h"
#include "PartialExpander.h"
#include "random.h"
#include "CVQualifiers.h"
#include "DepthSpec.h"

int eid = 0;

class ExpressionFilter : public Filter {
public:
	ExpressionFilter();

	virtual ~ExpressionFilter();

	virtual bool filter(int v) const = 0;

	eTermType number_to_termtype(unsigned int v, ProbabilityTable<unsigned int, eTermType> *table) const;
};

ExpressionFilter::ExpressionFilter()
{

}

ExpressionFilter::~ExpressionFilter()
{

}

ProbabilityTable<unsigned int, eTermType> *Expression::exprTable_ = NULL;

void
Expression::InitExprProbabilityTable()
{
	if (Expression::exprTable_)
		return;

	Expression::exprTable_ = new ProbabilityTable<unsigned int, eTermType>();
	Expression::exprTable_->add_elem(60, eFunction);
	Expression::exprTable_->add_elem(70, eConstant);
	Expression::exprTable_->add_elem(100, eVariable);
}

ProbabilityTable<unsigned int, eTermType> *Expression::paramTable_ = NULL;
void
Expression::InitParamProbabilityTable()
{
	if (Expression::paramTable_)
		return;

	Expression::paramTable_ = new ProbabilityTable<unsigned int, eTermType>();
	Expression::paramTable_->add_elem(30, eFunction);
	Expression::paramTable_->add_elem(40, eConstant);
	Expression::paramTable_->add_elem(100, eVariable);
}

void
Expression::InitProbabilityTables()
{
	Expression::InitExprProbabilityTable();
	Expression::InitParamProbabilityTable();
}

eTermType
ExpressionFilter::number_to_termtype(unsigned int i, ProbabilityTable<unsigned int, eTermType> *table) const
{
	assert(table);
	return table->get_value(i);
}

/////////////////////////////////////////////////////////
class DefaultExpressionFilter : public ExpressionFilter {
public:
	DefaultExpressionFilter(const Type &type, bool no_func, bool no_const);

	virtual ~DefaultExpressionFilter();

	virtual bool filter(int v) const;

private:
	const Type &type_;

	const bool no_func_;
	const bool no_const_;
};


DefaultExpressionFilter::DefaultExpressionFilter(const Type &type, bool no_func, bool no_const)
	: type_(type),
	  no_func_(no_func),
	  no_const_(no_const)
{

}

DefaultExpressionFilter::~DefaultExpressionFilter()
{

}

bool
DefaultExpressionFilter::filter(int v) const
{
	if (!this->valid_filter())
		return false;

	if (Expression::exprTable_->filter(v))
		return true;

	eTermType tt = number_to_termtype(v, Expression::exprTable_);
	if (!CGOptions::return_structs() && tt == eFunction && type_.eType == eStruct)
		return true;
	if (!CGOptions::return_unions() && tt == eFunction && type_.eType == eUnion)
		return true;
	if (tt == eConstant && no_const_) {
		return true;
	}
	if (!(no_func_ && tt == eFunction) &&
			!(type_.eType == eStruct && tt == eConstant)) {
		return false;
	}
	return true;
}

///////////////////////////////////////////////////
class ParamExpressionFilter : public ExpressionFilter {
public:
	ParamExpressionFilter(const Type* t) : type_(t) {} 
	virtual bool filter(int v) const;
private:
	const Type* type_;
};

bool
ParamExpressionFilter::filter(int v) const
{
	if (!this->valid_filter())
		return false;

	if (Expression::exprTable_->filter(v))
		return true;

	eTermType tt = number_to_termtype(v, Expression::exprTable_);
	if (tt == eConstant) {
		return true;
	}
	if (tt == eFunction) {
		if (type_->eType == eStruct && !CGOptions::return_structs() ||
			type_->eType == eUnion && !CGOptions::return_unions())
			return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
static eTermType
ExpressionTypeProbability(const CGContext &cg_context, const ExpressionFilter *filter)
{
	if (cg_context.expr_depth > CGOptions::max_expr_depth())
		return eVariable;

	if (PartialExpander::direct_expand_check(eInvoke))
		return eFunction;

	assert(filter);

	int i = rnd_upto(100, filter);
	ERROR_GUARD(MAX_TERM_TYPES);
	return filter->number_to_termtype(i, Expression::exprTable_);
}

/*
 *
 */
static eTermType
ParameterTypeProbability(const CGContext &cg_context)
{
	if (cg_context.expr_depth > CGOptions::max_expr_depth())
		return eVariable;

	int i = rnd_upto(100);  
	ERROR_GUARD(MAX_TERM_TYPES);
	return Expression::paramTable_->get_value(i);
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
Expression::make_random(CGContext &cg_context, const Type* type, bool no_func, bool no_const, enum eTermType tt)
{
	DEPTH_GUARD_BY_TYPE_RETURN_WITH_FLAG(dtExpression, tt, NULL);
	Expression::InitProbabilityTables();

	Expression *e = 0;  
	assert(type);
	assert(!(no_func && tt == eFunction));
	assert(!(no_const && tt == eConstant));
	// constant struct variables can not be a subexpression?
	assert(!(type->eType == eStruct && tt == eConstant));
	 
	// if no term type is provided, choose a random term type with restrictions
	if (tt == MAX_TERM_TYPES) {
		DefaultExpressionFilter filter(*type, no_func, no_const);
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
		e = ExpressionVariable::make_random(cg_context, type);
		break;
#if 0
	case eBinaryExpr:
		e = ExpressionBinary::make_random(cg_context);
		break;
#endif // 0
	case eFunction:
		cg_context.expr_depth++;
		e = ExpressionFuncall::make_random(cg_context, type);
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
		ParamExpressionFilter filter(type);
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

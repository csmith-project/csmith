// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2015 The University of Utah
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

#ifndef EXPRESSION_H
#define EXPRESSION_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include "CGContext.h"
#include "CVQualifiers.h"
#include "ProbabilityTable.h"
#include <vector>
#include <string>
using namespace std;

class CGContext;
class Type;
class FunctionInvocation;
class FunctionInvocationUser;
class ExpressionVariable;
class CVQualifiers;

#if 0
/*
 *
 */
enum eBinaryOp
{
	eAdd,
	eSub
};
#define MAX_BINARY_OPS ((eBinaryOp) (eSub+1))
#endif // 0

/*
 *
 */
enum eTermType
{
	eConstant,
	eVariable,
	// eUnaryExpr,
	// eBinaryExpr,
	eFunction,
	eAssignment,
	eCommaExpr,
	eLhs
};
#define MAX_TERM_TYPES ((eTermType) (eCommaExpr+1))

template <class Key, class Value>
class ProbabilityTable;

/*
 *
 */
class Expression
{
public:
	// Factory method.
	static Expression *make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer=0, bool no_func = false, bool no_const = false, enum eTermType tt=MAX_TERM_TYPES);

	static Expression *make_random_param(CGContext &cg_context, const Type* type, const CVQualifiers* qfer, enum eTermType tt=MAX_TERM_TYPES);

	static void InitProbabilityTables();

	Expression(eTermType e);

	Expression(const Expression &expr);

	virtual ~Expression(void);

	virtual Expression *clone() const = 0;

	virtual const Type &get_type(void) const = 0;

	virtual CVQualifiers get_qualifiers(void) const = 0;

	virtual void get_eval_to_subexps(vector<const Expression*>& subs) const = 0;

	virtual void get_called_funcs(std::vector<const FunctionInvocationUser*>& /*funcs*/ ) const {};

	virtual const FunctionInvocation* get_invoke(void) const {return NULL;};

	virtual bool visit_facts(vector<const Fact*>& /*inputs*/, CGContext& /*cg_context*/) const {return true;};

	virtual std::vector<const ExpressionVariable*> get_dereferenced_ptrs(void) const;
	virtual void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const = 0;

	virtual bool has_uncertain_call_recursive(void) const {return false;}

	virtual unsigned int get_complexity(void) const = 0;
	virtual bool less_than(int /*num*/) const { return false;}
	virtual bool not_equals(int /*num*/) const { return false;}
	virtual bool equals(int /*num*/) const { return false;}
	virtual bool is_0_or_1(void) const { return false;}

	virtual bool use_var(const Variable* /* v */) const { return false;}

	virtual void Output(std::ostream &) const = 0;
	virtual void indented_output(std::ostream &out, int indent) const;

#if 0
	void OutputBinaryOp(std::ostream &) const;
#endif

	unsigned int func_count(void) const;

	std::string to_string(void) const;

	static void record_dereference_level(int level);

	virtual bool compatible(const Expression *) const { return false;}

	virtual bool compatible(const Variable *) const { return false;}

	void check_and_set_cast(const Type* t);
	void output_cast(std::ostream& out) const;

	enum eTermType term_type;
	int expr_id;

	const Type* cast_type;

private:
	static void InitExprProbabilityTable();
	static void InitParamProbabilityTable();

	static DistributionTable exprTable_;
	static DistributionTable paramTable_;
};

///////////////////////////////////////////////////////////////////////////////

#endif // EXPRESSION_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

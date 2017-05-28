// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2014, 2015 The University of Utah
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

#ifndef FUNCTION_INVOCATION_H
#define FUNCTION_INVOCATION_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <vector>
#include "util.h"
#include "CVQualifiers.h"
using namespace std;

class CGContext;
class Function;
class Expression;
class FunctionInvocationUser;
class Type;
class Fact;
class SafeOpFlags;
class Variable;
class CVQualifiers;

enum eUnaryOps
{
	ePlus,
	eMinus,
	eNot,
	eBitNot
#if 0 // TODO --- to be implemented
	ePreInc,
	ePreDec,
	ePostInc,
	ePostDec
#endif // 0
};
#define MAX_UNARY_OP ((eUnaryOps) (eBitNot+1))
#if 0 // TODO --- to be implemented
#  define MAX_UNARY_OP ((eUnaryOps) (ePostDec+1))
#endif

enum eBinaryOps
{
	eAdd,
	eSub,
	eMul,
	eDiv,
	eMod,
	eCmpGt,
	eCmpLt,
	eCmpGe,
	eCmpLe,
	eCmpEq,
	eCmpNe,
	eAnd,
	eOr,
	eBitXor,
	eBitAnd,
	eBitOr,
	eRShift,
	eLShift
};
#define MAX_BINARY_OP ((eBinaryOps) (eLShift+1))

enum eInvocationType
{
    eBinaryPrim,
    eUnaryPrim,
    eFuncCall
};

class FunctionInvocation
{
public:
	FunctionInvocation(eInvocationType e, const SafeOpFlags *flags);

	virtual ~FunctionInvocation(void);

	virtual FunctionInvocation *clone() const = 0;

	static FunctionInvocation *make_random(bool,
										   CGContext &cg_context,
                                           const Type* type,
										   const CVQualifiers* qfer);
	static FunctionInvocation *make_random(Function *target,
										   CGContext &cg_context);

	static FunctionInvocation *make_random_unary(CGContext &cg_context, const Type* type);

	static FunctionInvocation *make_random_binary(CGContext &cg_context, const Type* type);

	static FunctionInvocation * make_random_binary_ptr_comparison(CGContext &cg_context);

	static FunctionInvocation *make_unary(CGContext &cg_context, eUnaryOps op,
										  Expression *operand);
	static FunctionInvocation *make_binary(CGContext &cg_context, eBinaryOps op,
										   Expression *lhs,
										   Expression *rhs);

	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;

	virtual void get_called_funcs(std::vector<const FunctionInvocationUser*>& funcs ) const;

	virtual bool has_uncertain_call(void) const;

	vector<intvec> permute_param_oders(void) const;

	bool visit_unordered_params(vector<const Fact*>& inputs, CGContext& cg_context) const;

	bool has_uncertain_call_recursive(void) const;

	bool has_simple_params(void) const;

	CVQualifiers get_qualifiers(void) const;

	void add_operand(const Expression* e);

	static bool IsOrderedStandardFunc(eBinaryOps eFunc);

	static bool BinaryOpWorksForFloat(eBinaryOps op);

	static bool UnaryOpWorksForFloat(eUnaryOps op);

	virtual const Type &get_type(void) const = 0;

	virtual bool compatible(const Variable *) const { return false; }

	virtual bool is_0_or_1(void) const { return false;}

	virtual bool equals(int /*num*/) const { return false;}

	virtual void Output(std::ostream &) const = 0;

	virtual void indented_output(std::ostream &out, int indent) const = 0;

	virtual bool safe_invocation() const = 0;

	eInvocationType invoke_type;

	std::vector<const Expression*> param_value;

	bool failed;	// indicates whether this invocation has failed to pass pointer/effect analysis
	bool ptr_cmp;	// indicates whether this is a pointer comparison
protected:
	explicit FunctionInvocation(const FunctionInvocation &fi);

	const SafeOpFlags *op_flags;

private:
	// unimplemented
	FunctionInvocation &operator=(const FunctionInvocation &fi);
};

///////////////////////////////////////////////////////////////////////////////

#endif // FUNCTION_INVOCATION_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

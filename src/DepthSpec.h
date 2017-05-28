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

#ifndef DEPTH_SPEC_H
#define DEPTH_SPEC_H

// Specify depth for each term
// They are all under change

#define ATOMIC_DEPTH_INCR 1

#define GOOD_DEPTH 0
#define BAD_DEPTH -1

enum dType {
	dtFirstFunction,
	dtFunction,
	dtBlock,
	dtStatement,
	dtStatementAssign,
	dtStatementFor,
	dtStatementIf,
	dtStatementExpr,
	dtStatementReturn,
	dtFunctionInvocationUnary,
	dtFunctionInvocationBinary,
	dtFunctionInvocationRandomUnary,
	dtFunctionInvocationRandomRegularBinary,
	dtFunctionInvocationRandomBinaryPointer,
	dtFunctionInvocationRandomBinary,
	dtFunctionInvocationRandom,
	dtFunctionInvocationStdFunc,
	dtFunctionUserBuildInvocation,
	dtExpression,
	dtExpressionRandomParam,
	dtExpressionFuncall,
	dtExpressionVariable,
	dtLhs,
	dtReturnType,
	dtRandomTypeFromType,
	dtFunctionGenerateBody,
	dtGenerateParamList,
	dtTypeChooseSimple,
	dtSelectLValue,
	dtSelectVariable,
	dtSelectDerefPointer,
	dtSelectExistingVariable,
	dtInitVariable,
	dtSelectGlobal,
	dtGenerateNewGlobal,
	dtSelectParentLocal,
	dtGenerateNewParentLocal,
	dtGenerateNewVariable,
	dtInitPointerValue,
	dtSafeOpFlags,
};

#define DEPTH_GUARD_BY_DEPTH_NORETURN(d) \
	if (DepthSpec::depth_guard_by_depth(d) != GOOD_DEPTH) \
		return;

#define DEPTH_GUARD_BY_DEPTH_RETURN(d, rv) \
	if (DepthSpec::depth_guard_by_depth(d) != GOOD_DEPTH) \
		return rv;

#define DEPTH_GUARD_BY_DEPTH_RETURN_DEL1(d, t1, rv) \
	if (DepthSpec::depth_guard_by_depth(d) != GOOD_DEPTH) { \
		if (t1) delete t1; \
		return rv; \
	}

#define DEPTH_GUARD_BY_DEPTH_RETURN_DEL2(d, t1, t2, rv) \
	if (DepthSpec::depth_guard_by_depth(d) != GOOD_DEPTH) { \
		if (t1) delete t1; \
		if (t2) delete t2; \
		return rv; \
	}

#define DPETH_GUARD_BY_TYPE_NORETURN(t) \
	if (DepthSpec::depth_guard_by_type(t) != GOOD_DEPTH) \
		return;

#define DPETH_GUARD_BY_TYPE_NORETURN_WITH_FLAG(t, flag) \
	if (DepthSpec::depth_guard_by_type(t, flag) != GOOD_DEPTH) \
		return;

#define DEPTH_GUARD_BY_TYPE_RETURN(t, rv) \
	if (DepthSpec::depth_guard_by_type(t) != GOOD_DEPTH) \
		return rv;

#define DEPTH_GUARD_BY_TYPE_RETURN_WITH_FLAG(t, flag, rv) \
	if (DepthSpec::depth_guard_by_type(t, flag) != GOOD_DEPTH) \
		return rv;

#define GETTER_DECL(type) \
	static int \
	type##_minimal_depth(int flag = -1);

class DepthSpec
{
public:
	static int depth_guard_by_type(enum dType ty, int extra_flag = 0);

	static int depth_guard_by_depth(int depth_needed);

	static void depth_print();

//private:

	const static int dtVariableSelection_minimal_depth_;

	const static int dtLoopControl_minimal_depth_;

	const static int dtTypeNonVoidSimple_minimal_depth_;

	const static int dtTypeChooseRandom_minimal_depth_;

	const static int dtChooseRandomPointerType_minimal_depth_;

	const static int dtConstant_minimal_depth_;

	const static int dtInitVariable_minimal_depth_;

	static int dtFunction_minimal_depth_(int flag = 0);
	static int dtStatement_minimal_depth_(int flag = 0);

	GETTER_DECL(dtLoopControl)
	GETTER_DECL(dtTypeNonVoidSimple)
	GETTER_DECL(dtTypeChooseRandom)
	GETTER_DECL(dtChooseRandomPointerType)
	GETTER_DECL(dtVariableSelection)
	GETTER_DECL(dtConstant)
	GETTER_DECL(dtInitVariable)

	GETTER_DECL(dtFirstFunction)
	GETTER_DECL(dtFunction)
	GETTER_DECL(dtBlock)
	GETTER_DECL(dtStatement)
	GETTER_DECL(dtStatementAssign)
	GETTER_DECL(dtStatementFor)
	GETTER_DECL(dtStatementIf)
	GETTER_DECL(dtStatementExpr)
	GETTER_DECL(dtStatementReturn)
	GETTER_DECL(dtFunctionInvocationUnary)
	GETTER_DECL(dtFunctionInvocationBinary)
	GETTER_DECL(dtFunctionInvocationRandomUnary)
	GETTER_DECL(dtFunctionInvocationRandomRegularBinary)
	GETTER_DECL(dtFunctionInvocationRandomBinaryPointer)
	GETTER_DECL(dtFunctionInvocationRandomBinary)
	GETTER_DECL(dtFunctionInvocationRandom)
	GETTER_DECL(dtFunctionInvocationStdFunc)
	GETTER_DECL(dtFunctionUserBuildInvocation)
	GETTER_DECL(dtExpression)
	GETTER_DECL(dtExpressionRandomParam)
	GETTER_DECL(dtExpressionVariable)
	GETTER_DECL(dtExpressionFuncall)
	GETTER_DECL(dtLhs)
	GETTER_DECL(dtReturnType)
	GETTER_DECL(dtRandomTypeFromType)
	GETTER_DECL(dtFunctionGenerateBody)
	GETTER_DECL(dtGenerateParamList)
	GETTER_DECL(dtTypeChooseSimple)
	GETTER_DECL(dtSelectLValue)
	GETTER_DECL(dtSelectVariable)
	GETTER_DECL(dtSelectDerefPointer)
	GETTER_DECL(dtSelectExistingVariable)
	GETTER_DECL(dtSelectGlobal)
	GETTER_DECL(dtGenerateNewGlobal)
	GETTER_DECL(dtSelectParentLocal)
	GETTER_DECL(dtGenerateNewParentLocal)
	GETTER_DECL(dtGenerateNewVariable)
	GETTER_DECL(dtInitPointerValue)
	GETTER_DECL(dtSafeOpFlags)

private:
	DepthSpec() {}

	~DepthSpec() {}

	static int backtracking(int depth_needed);
};

#endif //DEPTH_SPEC_H

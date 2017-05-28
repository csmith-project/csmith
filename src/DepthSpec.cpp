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

#include "DepthSpec.h"

#include <algorithm>
#include <vector>
#include <cassert>
#include <iostream>

#include "CGOptions.h"
#include "DFSRndNumGenerator.h"
#include "Statement.h"
#include "RandomNumber.h"
#include "SafeOpFlags.h"
#include "VariableSelector.h"
#include "Expression.h"

using namespace std;

// Atomic default values
const int DepthSpec::dtVariableSelection_minimal_depth_ = 1;

const int DepthSpec::dtLoopControl_minimal_depth_ = 3;

const int DepthSpec::dtTypeNonVoidSimple_minimal_depth_ = 1;

const int DepthSpec::dtTypeChooseRandom_minimal_depth_ = 1;

const int DepthSpec::dtChooseRandomPointerType_minimal_depth_ = 1;

const int DepthSpec::dtConstant_minimal_depth_ = 0;

const int DepthSpec::dtInitVariable_minimal_depth_ = 1;

#define DEFINE_DEFAULT_GETTER(type)	\
	int \
	DepthSpec::type##_minimal_depth(int) { return type##_minimal_depth_; }	\

// Don't define a default one if you define your own!
DEFINE_DEFAULT_GETTER(dtLoopControl)
DEFINE_DEFAULT_GETTER(dtTypeNonVoidSimple)
DEFINE_DEFAULT_GETTER(dtTypeChooseRandom)
DEFINE_DEFAULT_GETTER(dtChooseRandomPointerType)
DEFINE_DEFAULT_GETTER(dtVariableSelection)
DEFINE_DEFAULT_GETTER(dtConstant)
DEFINE_DEFAULT_GETTER(dtInitVariable)

// non-default depth

#if 0
int DepthSpec::dtFunction_minimal_depth(int flag)
{
	return flag ? dtFunction_minimal_depth_() :
			(dtFunction_minimal_depth_() + dtReturnType_minimal_depth());
}
#endif

int DepthSpec::dtFunction_minimal_depth_(int)
{
	return dtGenerateParamList_minimal_depth() + dtFunctionGenerateBody_minimal_depth();
}

int DepthSpec::dtFunction_minimal_depth(int)
{
	return dtFunction_minimal_depth_();
}

int DepthSpec::dtFirstFunction_minimal_depth(int)
{
	return dtReturnType_minimal_depth() + dtFunctionGenerateBody_minimal_depth();
}

int DepthSpec::dtStatement_minimal_depth_(int)
{
/*
	vector<int> vs;
	vs.push_back(dtStatementAssign_minimal_depth());
	vs.push_back(dtStatementFor_minimal_depth());
	vs.push_back(dtStatementIf_minimal_depth());
	vs.push_back(dtStatementExpr_minimal_depth());
	vs.push_back(dtStatementReturn_minimal_depth());

	return *max_element(vs.begin(), vs.end());
*/
	return dtStatementReturn_minimal_depth() + 1;
}

int DepthSpec::dtBlock_minimal_depth(int)
{
	return dtStatement_minimal_depth_() + 1;
}

int DepthSpec::dtStatement_minimal_depth(int flag)
{
	return (flag == MAX_STATEMENT_TYPE) ? (dtStatement_minimal_depth_() + 1) : dtStatement_minimal_depth_();
}

int DepthSpec::dtStatementAssign_minimal_depth(int)
{
	int extra = 0;
	if (CGOptions::compound_assignment())
		extra = 1;
	return dtSelectLValue_minimal_depth() + dtExpression_minimal_depth() +
		dtLhs_minimal_depth() + dtSafeOpFlags_minimal_depth() + extra;
}

int DepthSpec::dtStatementFor_minimal_depth(int)
{
	return (dtSelectLValue_minimal_depth()
		+ dtLoopControl_minimal_depth()
		+ dtSafeOpFlags_minimal_depth()
		+ dtFunctionInvocationBinary_minimal_depth()
		+ dtSafeOpFlags_minimal_depth()
		+ ATOMIC_DEPTH_INCR
		+ dtBlock_minimal_depth());
}

int DepthSpec::dtStatementIf_minimal_depth(int)
{
	return (dtExpression_minimal_depth() + 2 * dtBlock_minimal_depth());
}

int DepthSpec::dtStatementExpr_minimal_depth(int)
{
	return dtFunctionInvocationRandom_minimal_depth();
}

int DepthSpec::dtStatementReturn_minimal_depth(int)
{
	return dtExpressionVariable_minimal_depth();
}

int DepthSpec::dtFunctionInvocationUnary_minimal_depth(int)
{
	return dtSafeOpFlags_minimal_depth();
}

int DepthSpec::dtFunctionInvocationBinary_minimal_depth(int)
{
	return dtSafeOpFlags_minimal_depth();
}

int DepthSpec::dtFunctionInvocationRandomUnary_minimal_depth(int)
{
	return ATOMIC_DEPTH_INCR + dtSafeOpFlags_minimal_depth()
		+ dtExpression_minimal_depth();
}

int DepthSpec::dtFunctionInvocationRandomRegularBinary_minimal_depth(int)
{
	return ATOMIC_DEPTH_INCR + dtSafeOpFlags_minimal_depth()
		+ 2 * + dtExpression_minimal_depth();
}

int DepthSpec::dtFunctionInvocationRandomBinaryPointer_minimal_depth(int)
{
	return 1 + dtSafeOpFlags_minimal_depth() +
		dtChooseRandomPointerType_minimal_depth() +
		2 * dtExpression_minimal_depth();

}

int DepthSpec::dtFunctionInvocationRandomBinary_minimal_depth(int)
{
	vector<int> vs;
	vs.push_back(dtFunctionInvocationRandomRegularBinary_minimal_depth());
	vs.push_back(dtFunctionInvocationRandomBinaryPointer_minimal_depth());
	return 1 + *min_element(vs.begin(), vs.end());
}

int DepthSpec::dtFunctionUserBuildInvocation_minimal_depth(int flag)
{
	return dtExpressionRandomParam_minimal_depth(flag);
}

int DepthSpec::dtFunctionInvocationStdFunc_minimal_depth(int)
{
	vector<int> vs;
	vs.push_back(dtFunctionInvocationRandomUnary_minimal_depth());
	vs.push_back(dtFunctionInvocationRandomBinary_minimal_depth());

	return *min_element(vs.begin(), vs.end()) + 1;
}

int DepthSpec::dtFunctionInvocationRandom_minimal_depth(int)
{
	vector<int> vs;
	vs.push_back(dtFunctionInvocationRandomUnary_minimal_depth());
	vs.push_back(dtFunctionInvocationRandomBinary_minimal_depth());
	vs.push_back(dtFunctionUserBuildInvocation_minimal_depth());

	return *min_element(vs.begin(), vs.end());
}

int DepthSpec::dtExpressionRandomParam_minimal_depth(int flag)
{
	int rv = dtConstant_minimal_depth(flag);
	return (flag == MAX_TERM_TYPES) ? (rv + 1) : rv;
}

int DepthSpec::dtExpression_minimal_depth(int flag)
{
	int rv = dtConstant_minimal_depth(flag);
	return (flag == MAX_TERM_TYPES) ? (rv + 1) : rv;
	//vector<int> vs;
	//vs.push_back(dtConstant_minimal_depth());
	//vs.push_back(dtExpressionVariable_minimal_depth());
	//vs.push_back(dtExpressionFuncall_minimal_depth());

	//return (*max_element(vs.begin(), vs.end()) + 1);
}

int DepthSpec::dtExpressionVariable_minimal_depth(int)
{
	return dtSelectVariable_minimal_depth();
}

int DepthSpec::dtExpressionFuncall_minimal_depth(int)
{
	return dtFunctionInvocationRandom_minimal_depth();
}

int DepthSpec::dtLhs_minimal_depth(int)
{
	return 1;
}

int DepthSpec::dtReturnType_minimal_depth(int)
{
	return dtTypeChooseRandom_minimal_depth();
}

int DepthSpec::dtRandomTypeFromType_minimal_depth(int)
{
	return dtTypeChooseRandom_minimal_depth();
}

int DepthSpec::dtFunctionGenerateBody_minimal_depth(int)
{
	return dtBlock_minimal_depth();
}

int DepthSpec::dtGenerateParamList_minimal_depth(int)
{
	return ATOMIC_DEPTH_INCR + dtSelectVariable_minimal_depth();
}

int DepthSpec::dtTypeChooseSimple_minimal_depth(int flag)
{
	return dtTypeNonVoidSimple_minimal_depth(flag);
}

int DepthSpec::dtSelectLValue_minimal_depth(int flag)
{
	return dtSelectVariable_minimal_depth(flag);
}

int DepthSpec::dtSelectVariable_minimal_depth(int flag)
{
	return (flag == MAX_VAR_SCOPE) ? dtVariableSelection_minimal_depth(flag) + 1 :
					dtVariableSelection_minimal_depth(flag);
}

int DepthSpec::dtSelectDerefPointer_minimal_depth(int)
{
	return 0;
}

int DepthSpec::dtSelectExistingVariable_minimal_depth(int flag)
{
	return dtVariableSelection_minimal_depth(flag);
}

int dtGenerateNewGlobal_minimal_depth_(int flag)
{

	return DepthSpec::dtInitVariable_minimal_depth(flag);
}

int DepthSpec::dtSelectGlobal_minimal_depth(int flag)
{
	return dtInitVariable_minimal_depth(flag) + dtGenerateNewGlobal_minimal_depth_(flag);
}

int DepthSpec::dtGenerateNewGlobal_minimal_depth(int flag)
{

	return 1 + dtGenerateNewGlobal_minimal_depth_(flag);
}

int DepthSpec::dtSelectParentLocal_minimal_depth(int flag)
{
	return 1 + dtRandomTypeFromType_minimal_depth(flag);
}

int dtGenerateNewParentLocal_minimal_depth_(int flag)
{
	return DepthSpec::dtInitVariable_minimal_depth(flag);
}

int DepthSpec::dtGenerateNewParentLocal_minimal_depth(int flag)
{
	return 2 + dtGenerateNewParentLocal_minimal_depth_(flag);
}

int DepthSpec::dtGenerateNewVariable_minimal_depth(int)
{
	int depth = dtGenerateNewParentLocal_minimal_depth() <= dtGenerateNewGlobal_minimal_depth()
			? dtGenerateNewParentLocal_minimal_depth() : dtGenerateNewGlobal_minimal_depth();
	return depth + 1;
}

int DepthSpec::dtInitPointerValue_minimal_depth(int)
{
	int depth = dtGenerateNewParentLocal_minimal_depth() <= dtGenerateNewGlobal_minimal_depth()
			? dtGenerateNewParentLocal_minimal_depth() : dtGenerateNewGlobal_minimal_depth();
	return dtRandomTypeFromType_minimal_depth() + depth;
}

int
DepthSpec::dtSafeOpFlags_minimal_depth(int flag)
{
	return (flag == sOpBinary) ? 2 : 3;
}

//cout << "max_depth = " << max_depth << ", cur_depth = " << cur_depth << ", minimal = " << type##_minimal_depth(flag) << std::endl;
#define DEPTH_GUARD_CASE(type, flag) case type: \
	{ \
		assert(CGOptions::dfs_exhaustive()); \
		int depth_needed = type##_minimal_depth(flag); \
		return DepthSpec::backtracking(depth_needed); \
	}

int
DepthSpec::backtracking(int depth_needed)
{
	DFSRndNumGenerator *impl = dynamic_cast<DFSRndNumGenerator*>(RandomNumber::GetRndNumGenerator());
	if(impl->eager_backtracking(depth_needed))
		return BAD_DEPTH;
	else
		return GOOD_DEPTH;
}

int
DepthSpec::depth_guard_by_depth(int depth_needed)
{
	if (!CGOptions::dfs_exhaustive()) {
		return GOOD_DEPTH;
	}
	return DepthSpec::backtracking(depth_needed);
}

int DepthSpec::depth_guard_by_type(enum dType ty, int extra_flag)
{
	if (!CGOptions::dfs_exhaustive()) {
		return GOOD_DEPTH;
	}

	switch (ty) {
		DEPTH_GUARD_CASE(dtFunction, extra_flag)
		DEPTH_GUARD_CASE(dtFirstFunction, extra_flag)
		DEPTH_GUARD_CASE(dtBlock, extra_flag)
		DEPTH_GUARD_CASE(dtStatement, extra_flag)
		DEPTH_GUARD_CASE(dtStatementAssign, extra_flag)
		DEPTH_GUARD_CASE(dtStatementFor, extra_flag)
		DEPTH_GUARD_CASE(dtStatementIf, extra_flag)
		DEPTH_GUARD_CASE(dtStatementExpr, extra_flag)
		DEPTH_GUARD_CASE(dtStatementReturn, extra_flag)
		DEPTH_GUARD_CASE(dtFunctionInvocationUnary, extra_flag)
		DEPTH_GUARD_CASE(dtFunctionInvocationBinary, extra_flag)
		DEPTH_GUARD_CASE(dtFunctionInvocationRandomUnary, extra_flag)
		DEPTH_GUARD_CASE(dtFunctionInvocationRandomBinary, extra_flag)
		DEPTH_GUARD_CASE(dtFunctionInvocationRandom, extra_flag)
		DEPTH_GUARD_CASE(dtFunctionInvocationStdFunc, extra_flag)
		DEPTH_GUARD_CASE(dtFunctionUserBuildInvocation, extra_flag)
		DEPTH_GUARD_CASE(dtExpression, extra_flag)
		DEPTH_GUARD_CASE(dtExpressionRandomParam, extra_flag)
		DEPTH_GUARD_CASE(dtExpressionVariable, extra_flag)
		DEPTH_GUARD_CASE(dtExpressionFuncall, extra_flag)
		DEPTH_GUARD_CASE(dtLhs, extra_flag)
		DEPTH_GUARD_CASE(dtReturnType, extra_flag)
		DEPTH_GUARD_CASE(dtRandomTypeFromType, extra_flag)
		DEPTH_GUARD_CASE(dtFunctionGenerateBody, extra_flag)
		DEPTH_GUARD_CASE(dtGenerateParamList, extra_flag)
		DEPTH_GUARD_CASE(dtTypeChooseSimple, extra_flag)
		DEPTH_GUARD_CASE(dtSelectLValue, extra_flag)
		DEPTH_GUARD_CASE(dtSelectVariable, extra_flag)
		DEPTH_GUARD_CASE(dtSelectDerefPointer, extra_flag)
		DEPTH_GUARD_CASE(dtSelectExistingVariable, extra_flag)
		DEPTH_GUARD_CASE(dtInitVariable, extra_flag)
		DEPTH_GUARD_CASE(dtSelectGlobal, extra_flag)
		DEPTH_GUARD_CASE(dtGenerateNewGlobal, extra_flag)
		DEPTH_GUARD_CASE(dtSelectParentLocal, extra_flag)
		DEPTH_GUARD_CASE(dtGenerateNewParentLocal, extra_flag)
		DEPTH_GUARD_CASE(dtGenerateNewVariable, extra_flag)
		DEPTH_GUARD_CASE(dtInitPointerValue, extra_flag)
		DEPTH_GUARD_CASE(dtSafeOpFlags, extra_flag)
		default:
			assert(0);
			break;
	}

	return GOOD_DEPTH;
}

#define PRTDEPTH(type) \
	cout << #type << " : size = " << type##_minimal_depth() << endl;

void DepthSpec::depth_print()
{
	PRTDEPTH(dtSafeOpFlags);
	PRTDEPTH(dtLoopControl);
	PRTDEPTH(dtTypeNonVoidSimple);
	PRTDEPTH(dtTypeChooseRandom);
	PRTDEPTH(dtChooseRandomPointerType);
	PRTDEPTH(dtVariableSelection);
	PRTDEPTH(dtConstant);

	PRTDEPTH(dtFirstFunction)
	PRTDEPTH(dtFunction)
	PRTDEPTH(dtBlock)
	PRTDEPTH(dtStatement)
	PRTDEPTH(dtStatementAssign)
	PRTDEPTH(dtStatementFor)
	PRTDEPTH(dtStatementIf)
	PRTDEPTH(dtStatementExpr)
	PRTDEPTH(dtStatementReturn)
	PRTDEPTH(dtFunctionInvocationUnary)
	PRTDEPTH(dtFunctionInvocationBinary)
	PRTDEPTH(dtFunctionInvocationRandomUnary)
	PRTDEPTH(dtFunctionInvocationRandomBinary)
	PRTDEPTH(dtFunctionInvocationRandom)
	PRTDEPTH(dtFunctionInvocationStdFunc)
	PRTDEPTH(dtFunctionUserBuildInvocation)
	PRTDEPTH(dtExpression)
	PRTDEPTH(dtExpressionRandomParam)
	PRTDEPTH(dtExpressionVariable)
	PRTDEPTH(dtExpressionFuncall)
	PRTDEPTH(dtLhs)
	PRTDEPTH(dtReturnType)
	PRTDEPTH(dtRandomTypeFromType)
	PRTDEPTH(dtFunctionGenerateBody)
	PRTDEPTH(dtGenerateParamList)
	PRTDEPTH(dtTypeChooseSimple)
	PRTDEPTH(dtSelectLValue)
	PRTDEPTH(dtSelectDerefPointer)
	PRTDEPTH(dtSelectExistingVariable)
	PRTDEPTH(dtInitVariable)
	PRTDEPTH(dtGenerateNewGlobal)
	PRTDEPTH(dtSelectGlobal)
	PRTDEPTH(dtGenerateNewParentLocal)
	PRTDEPTH(dtSelectParentLocal)
	PRTDEPTH(dtGenerateNewVariable)
	PRTDEPTH(dtInitPointerValue)
}

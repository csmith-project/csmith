// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2014, 2015, 2017 The University of Utah
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "FunctionInvocation.h"

#include <cassert>

#include "Common.h"
#include "CGContext.h"
#include "Effect.h"
#include "Expression.h"
#include "ExpressionVariable.h"
#include "ExpressionFuncall.h"
#include "Function.h"
#include "FunctionInvocationBinary.h"
#include "FunctionInvocationUnary.h"
#include "FunctionInvocationUser.h"
#include "Type.h"
#include "Block.h"
#include "VectorFilter.h"
#include "FactMgr.h"
#include "random.h"
#include "Variable.h"
#include "Bookkeeper.h"
#include "SafeOpFlags.h"
#include "CVQualifiers.h"
#include "Error.h"
#include "Probabilities.h"
#include "CompatibleChecker.h"
#include "DepthSpec.h"
#include "Constant.h"
#include "CGOptions.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 * XXX
 */
FunctionInvocation *
FunctionInvocation::make_random(bool is_std_func,
				CGContext &cg_context,
				const Type* type,
				const CVQualifiers* qfer)
{
	FunctionInvocation *fi = 0;
	// If we are looking for a program-defined function, try to find one.
	if (!is_std_func) {
		Function* callee = NULL;
		if (pure_rnd_flipcoin(50)) {
			callee = Function::choose_func(get_all_functions(), cg_context, type, qfer);
		}
		if (callee != NULL) {
			FunctionInvocationUser *fiu = new FunctionInvocationUser(callee, true, NULL);
			fiu->build_invocation(callee, cg_context);
			fi = fiu;
			if (!fiu->failed) {
				cg_context.get_current_func()->fact_changed |= fiu->func->fact_changed;
			}
		}
		else if (!Function::reach_max_functions_cnt()) {
			fi = FunctionInvocationUser::build_invocation_and_function(cg_context, type, qfer);
		} else {
			// we can not find/create a function because we reach the limit, so give up
			fi = new FunctionInvocationUser(NULL, false, NULL);
			fi->failed = true;
			return fi;
		}
	}
	// now use standard functions, i.e., binary/unary operators to create an invocation
	if (fi == NULL) {
		int rnd_flag = rnd_flipcoin(StdUnaryFuncProb);
		if (rnd_flag) {
			fi = make_random_unary(cg_context, type);
		} else {
			fi = make_random_binary(cg_context, type);
		}
	}
	assert(fi != 0);
	return fi;
}

/*
 * TODO: FIX!  This is a bogus constructor, used only by the `OutputMain'.
 * The problem is, there is no representation for the calling function,
 * so we "just use" `*target'.  Bah!
 */
FunctionInvocation *
FunctionInvocation::make_random(Function *target,
								CGContext &cg_context)
{
	FunctionInvocationUser *fi = new FunctionInvocationUser(target, true, NULL);
	fi->build_invocation(target, cg_context);
	ERROR_GUARD_AND_DEL1(NULL, fi);
	assert(!fi->failed);
	return fi;
}

/*
 *
 */
FunctionInvocation *
FunctionInvocation::make_random_unary(CGContext &cg_context, const Type* type)
{
	DEPTH_GUARD_BY_TYPE_RETURN(dtFunctionInvocationRandomUnary, NULL);
	assert(type);
	eUnaryOps op;
	do {
		op = (eUnaryOps)(rnd_upto(MAX_UNARY_OP, UNARY_OPS_PROB_FILTER));
	} while (type->is_float() && !UnaryOpWorksForFloat(op));
	ERROR_GUARD(NULL);
	SafeOpFlags *flags = NULL;
	flags = SafeOpFlags::make_random_unary(type, NULL, op);
	ERROR_GUARD(NULL);
	type = flags->get_lhs_type();
	assert(type);

	FunctionInvocation *fi = FunctionInvocationUnary::CreateFunctionInvocationUnary(cg_context, op, flags);

	Expression *operand = Expression::make_random(cg_context, type);
	ERROR_GUARD_AND_DEL1(NULL, fi);

	fi->param_value.push_back(operand);
	return fi;
}

/*
 *
 */
FunctionInvocation *
FunctionInvocation::make_random_binary(CGContext &cg_context, const Type* type)
{
	DEPTH_GUARD_BY_TYPE_RETURN(dtFunctionInvocationRandomBinary, NULL);
	if (rnd_flipcoin(10) && Type::has_pointer_type()) {
		ERROR_GUARD(NULL);
		return make_random_binary_ptr_comparison(cg_context);
	}

	eBinaryOps op;
	do {
		op = (eBinaryOps)(rnd_upto(MAX_BINARY_OP, BINARY_OPS_PROB_FILTER));
	} while (type->is_float() && !BinaryOpWorksForFloat(op));
	ERROR_GUARD(NULL);
	assert(type);
	SafeOpFlags *flags = SafeOpFlags::make_random_binary(type, NULL, NULL, sOpBinary, op);
	assert(flags);
	ERROR_GUARD(NULL);
	FunctionInvocationBinary *fi = FunctionInvocationBinary::CreateFunctionInvocationBinary(cg_context, op, flags);

	Effect lhs_eff_accum;
	CGContext lhs_cg_context(cg_context, cg_context.get_effect_context(), &lhs_eff_accum);

	// Generate an expression with the correct type required by safe math operands
	const Type* lhs_type = flags->get_lhs_type();
	const Type* rhs_type = flags->get_rhs_type();
	assert(lhs_type && rhs_type);
	if (!BinaryOpWorksForFloat(op)) {
		assert(!lhs_type->is_float() && "lhs_type is float!");
		assert(!rhs_type->is_float() && "rhs_type is float!");
	}

	Expression *lhs = Expression::make_random(lhs_cg_context, lhs_type);
	ERROR_GUARD_AND_DEL1(NULL, fi);
	Expression *rhs = 0;

	cg_context.merge_param_context(lhs_cg_context, true);
	FactMgr* fm = get_fact_mgr(&cg_context);
	vector<const Fact*> facts_copy = fm->global_facts;

#if 0
	if (lhs->term_type == eVariable) {
		lhs_eff_accum.read_deref_volatile((ExpressionVariable*)lhs);
	}
#endif

	// If we are guaranteed that the LHS will be evaluated before the RHS,
	// or if the LHS is pure (not merely side-effect-free),
	// then we can generate the RHS under the original effect context.
	if (IsOrderedStandardFunc(op)) { // || lhs_eff_accum.is_pure()) { TODO: need more thoughts on the purity issue.
		rhs = Expression::make_random(cg_context, rhs_type);
	}
	else {
		// Otherwise, the RHS must be generated under the combined effect
		// of the original effect and the LHS effect.
		Effect rhs_eff_context(cg_context.get_effect_context());
		rhs_eff_context.add_effect(lhs_eff_accum, true);
		Effect rhs_eff_accum;

		CGContext rhs_cg_context(cg_context, rhs_eff_context, &rhs_eff_accum);
		if (op == eLShift || op == eRShift) {
			eTermType tt = MAX_TERM_TYPES;
			bool not_constant = rnd_flipcoin(ShiftByNonConstantProb);
			// avoid shifting negative or too much
			if (!not_constant) {
				rhs = Constant::make_random_upto(lhs_type->SizeInBytes() * 8);
			} else {
				rhs = Expression::make_random(rhs_cg_context, rhs_type, NULL, false, true, tt);
			}
		}
		else {
			rhs = Expression::make_random(rhs_cg_context, rhs_type);
			// avoid divide by zero or possible zero (reached by pointer comparison)
			if ((op == eMod || op == eDiv) && (rhs->equals(0) || rhs->is_0_or_1()) &&
				!lhs_type->is_float() && !rhs_type->is_float()) {
				VectorFilter f;
				f.add(eMod).add(eDiv).add(eLShift).add(eRShift);
				op = (eBinaryOps)(rnd_upto(MAX_BINARY_OP, &f));
				fi->set_operation(op);
			}
		}
		cg_context.merge_param_context(rhs_cg_context, true);
	}

	ERROR_GUARD_AND_DEL2(NULL, fi, lhs);
	if (!BinaryOpWorksForFloat(op)) {
		assert(!lhs->get_type().is_float() && "lhs is of float!");
		assert(!rhs->get_type().is_float() && "rhs is of float!");
	}

	if (CompatibleChecker::compatible_check(lhs, rhs)) {
		Error::set_error(COMPATIBLE_CHECK_ERROR);
		delete lhs;
		delete rhs;
		delete fi;
		return NULL;
	}

	// ordered operators such as "||" or "&&" may skip the 2nd parameter
	if (IsOrderedStandardFunc(op)) {
		fm->makeup_new_var_facts(facts_copy, fm->global_facts);
		merge_facts(fm->global_facts, facts_copy);
	}
	// TODO: fix `rhs' for eLShift and eRShift and ...
	// Currently, the "fix" is handled in `FunctionInvocationBinary::Output'.
	fi->param_value.push_back(lhs);
	fi->param_value.push_back(rhs);
	return fi;
}

/*
 *
 */
FunctionInvocation *
FunctionInvocation::make_random_binary_ptr_comparison(CGContext &cg_context)
{
	eBinaryOps op = rnd_flipcoin(50) ? eCmpEq : eCmpNe;
	ERROR_GUARD(NULL);
	SafeOpFlags *flags = SafeOpFlags::make_random_binary(get_int_type(), NULL, NULL, sOpBinary, op);
	ERROR_GUARD(NULL);

	FunctionInvocation *fi = FunctionInvocationBinary::CreateFunctionInvocationBinary(cg_context, op, flags);
	const Type* type = Type::choose_random_pointer_type();
	ERROR_GUARD_AND_DEL1(NULL, fi);

	Effect lhs_eff_accum;
	CGContext lhs_cg_context(cg_context, cg_context.get_effect_context(), &lhs_eff_accum);
	lhs_cg_context.flags |= NO_DANGLING_PTR;
	Expression *lhs = Expression::make_random(lhs_cg_context, type, 0, true);
	ERROR_GUARD_AND_DEL1(NULL, fi);
	cg_context.merge_param_context(lhs_cg_context, true);

	// now focus on RHS ...
	enum eTermType tt = MAX_TERM_TYPES;
	// if LHS is const, there is no need for RHS to be const as well
	if (lhs->term_type == eConstant) {
		tt = eVariable;
	}
	Expression *rhs = 0;

	// If we are guaranteed that the LHS will be evaluated before the RHS,
	// or if the LHS is pure (not merely side-effect-free),
	// then we can generate the RHS under the original effect context.
	if (IsOrderedStandardFunc(op)) { //  lhs_eff_accum.is_pure())  JYTODO: purity needs to be redefined
		// although we don't need care about other side effect, we do
		// need to pass in NO_DANGLING_PTR flag
		unsigned int old_flag = cg_context.flags;
		cg_context.flags |= NO_DANGLING_PTR;
		rhs = Expression::make_random(cg_context, type, 0, true, false, tt);
		cg_context.flags = old_flag;
	} else {
		// Otherwise, the RHS must be generated under the combined effect
		// of the original effect and the LHS effect.
		Effect rhs_eff_context(cg_context.get_effect_context());
		rhs_eff_context.add_effect(lhs_eff_accum);
		Effect rhs_eff_accum;

		CGContext rhs_cg_context(cg_context, rhs_eff_context, &rhs_eff_accum);
		rhs_cg_context.flags |= NO_DANGLING_PTR;
		rhs = Expression::make_random(rhs_cg_context, type, 0, true, false, tt);
		cg_context.merge_param_context(rhs_cg_context, true);
	}
	ERROR_GUARD_AND_DEL2(NULL, fi, lhs);

	// typecast, if needed.
	rhs->check_and_set_cast(&lhs->get_type());

	// TODO: fix `rhs' for eLShift and eRShift and ...
	// Currently, the "fix" is handled in `FunctionInvocationBinary::Output'.
	fi->param_value.push_back(lhs);
	fi->param_value.push_back(rhs);
	fi->ptr_cmp = true;

	// bookkeeping for pointers
	Bookkeeper::record_pointer_comparisons(lhs, rhs);
	return fi;
}

/*
 *
 */
void
FunctionInvocation::add_operand(const Expression* e)
{
    param_value.push_back(e);
}

void
FunctionInvocation::get_called_funcs(std::vector<const FunctionInvocationUser*>& funcs) const
{
	// find calls in parameters
	for (size_t i=0; i<param_value.size(); i++) {
		const Expression* value = param_value[i];
		value->get_called_funcs(funcs);
	}
	if (invoke_type == eFuncCall) {
		const FunctionInvocationUser* func_call = (const FunctionInvocationUser*)this;
		funcs.push_back(func_call);
	}
}

bool
FunctionInvocation::has_uncertain_call(void) const
{
	// if there are more than two function calls in two separate parameters,
	// we judge both calls as uncertain because the evaluation order can be
	// either left-to-right or right-to-left
	int has_func_param_cnt = 0;
	size_t i;
	for (i=0; i<param_value.size(); i++) {
		if (param_value[i]->func_count() > 0) {
			has_func_param_cnt++;
		}
	}
	return has_func_param_cnt >= 2;
}

bool
FunctionInvocation::has_uncertain_call_recursive(void) const
{
	size_t i;
	for (i=0; i<param_value.size(); i++) {
		const Expression* e = param_value[i];
		if (e->term_type == eFunction) {
			const ExpressionFuncall* ef = (const ExpressionFuncall*)e;
			if (ef->has_uncertain_call_recursive()) {
				return true;
			}
		}
	}
	return has_uncertain_call();
}

bool
FunctionInvocation::has_simple_params(void) const
{
	size_t i;
	for (i=0; i<param_value.size(); i++) {
		const Expression* e = param_value[i];
		if (e->term_type == eFunction) {
			return false;
		}
	}
	return true;
}

vector<intvec>
FunctionInvocation::permute_param_oders(void) const
{
	vector<intvec> ret;
	intvec ret_base;    // the ordered sequence
	vector<int> base;
	size_t i, j;
	// shortcut for 2 parameters
	if (param_value.size() == 2) {
		ret_base.push_back(0);
		ret_base.push_back(1);
		ret.push_back(ret_base);
		ret_base.clear();
		ret_base.push_back(1);
		ret_base.push_back(0);
		ret.push_back(ret_base);
		return ret;
	}
	// get initial order, mark those paramters that invoke function call
	for (i=0; i<param_value.size(); i++) {
		if (param_value[i]->func_count() > 0) {
			base.push_back(i);
		}
		ret_base.push_back(i);
	}
	// permute
	vector<intvec> permuted = permute(base);
	for (i=0; i<permuted.size(); i++) {
		intvec new_seq = permuted[i];
		intvec tmp = ret_base;
		for (j=0; j<new_seq.size(); j++) {
			// plug back the new sequence into initial ordered sequence
			int orig_pos = base[j];
			int new_pos = new_seq[j];
			tmp[orig_pos] = new_pos;
		}
		ret.push_back(tmp);
	}
	return ret;
}

bool
FunctionInvocation::visit_unordered_params(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	vector<const Fact*> inputs_copy = inputs;
	vector<const Fact*> tmp;
	vector<intvec> orders = permute_param_oders();
	size_t i, j;
	assert(orders.size() > 0);
	// visit function calls with all possible orders
	for (i=0; i<orders.size(); i++) {
		intvec& order = orders[i];
		inputs = inputs_copy;
		for (j=0; j<order.size(); j++) {
			int param_id = order[j];
			const Expression* value = param_value[param_id];
			if (!value->visit_facts(inputs, cg_context)) {
				return false;
			}
		}
		if (i==0) {
			tmp = inputs;
		}
		else {
			merge_facts(tmp, inputs);
		}
	}
	inputs = tmp;
	return true;
}

CVQualifiers
FunctionInvocation::get_qualifiers(void) const
{
	CVQualifiers qfer;
	if (invoke_type == eFuncCall) {
		const FunctionInvocationUser* func_call = dynamic_cast<const FunctionInvocationUser*>(this);
		assert(func_call->get_func());
		assert(func_call->get_func()->rv);
		qfer = func_call->get_func()->rv->qfer;
	}
	// for binary and unary operations, they only yield integers right now (no pointer arithmatic
	// supported yet!), we assume they return non-const non-volatile int
	else {
		qfer.add_qualifiers(false, false);
	}
	return qfer;
}

bool
FunctionInvocation::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	bool unordered = false; //has_uncertain_call();
	bool ok = false;
	bool is_func_call = (invoke_type == eFuncCall);
	static int g = 0;
	Effect running_eff_context(cg_context.get_effect_context());
	if (!unordered) {
		// unsigned int flags = ptr_cmp ? (cg_context.flags | NO_DANGLING_PTR) : cg_context.flags;
		for (size_t i=0; i<param_value.size(); i++) {
			Effect param_eff_accum;
			int h = g++;
			CGContext param_cg_context(cg_context, running_eff_context, &param_eff_accum);
			// the parameters might be function calls
			const Expression* value = param_value[i];
			if (h == 236)
				BREAK_NOP;	// for debugging
			if (!value->visit_facts(inputs, param_cg_context)) {
				return false;
			}
			// Update the "running effect context": the context that we must use
			// when we generate subsequent parameters within this invocation.
			running_eff_context.add_effect(param_eff_accum);
			// Update the total effect of this invocation, too.
			cg_context.merge_param_context(param_cg_context, !is_func_call);
		}
		ok = true;
	}
	else {
		ok = visit_unordered_params(inputs, cg_context);
	}
	if (ok && is_func_call) {
		// make a copy of env
		vector<const Fact*> inputs_copy = inputs;
		const FunctionInvocationUser* func_call = dynamic_cast<const FunctionInvocationUser*>(this);
		Effect effect_accum;
		//CGContext new_context(func_call->func, cg_context.get_effect_context(), &effect_accum);
		CGContext new_context(cg_context, func_call->func, cg_context.get_effect_context(), &effect_accum);
		ok = func_call->revisit(inputs, new_context);
		if (ok) {
			assert(cg_context.curr_blk);
			//cg_context.add_external_effect(*new_context.get_effect_accum());
			cg_context.add_visible_effect(*new_context.get_effect_accum(), cg_context.curr_blk);
			Effect& func_effect = func_call->func->feffect;
			func_effect.add_external_effect(*new_context.get_effect_accum(), cg_context.call_chain);
		}
	}
	return ok;
}

/*
 * Build an "invocation" of a binary operation.
 */
FunctionInvocation *
FunctionInvocation::make_binary(CGContext &cg_context, eBinaryOps op,
				Expression *lhs, Expression *rhs)
{
	DEPTH_GUARD_BY_TYPE_RETURN(dtFunctionInvocationBinary, NULL);
	SafeOpFlags *flags = SafeOpFlags::make_random_binary(NULL, &(lhs->get_type()), &(rhs->get_type()), sOpBinary, op);
	ERROR_GUARD(NULL);
	FunctionInvocation *fi = FunctionInvocationBinary::CreateFunctionInvocationBinary(cg_context, op, flags);
	fi->param_value.push_back(lhs);
	fi->param_value.push_back(rhs);

	return fi;
}

/*
 *
 */
///////////////////////////////////////////////////////////////////////////////

FunctionInvocation::FunctionInvocation(eInvocationType e, const SafeOpFlags *flags)
	: invoke_type(e),
	  failed(false),
	  ptr_cmp(false),
	  op_flags(flags)
{
	// Nothing to do
}

/*
 * copy constructor
 */
FunctionInvocation::FunctionInvocation(const FunctionInvocation &fi)
	: invoke_type(fi.invoke_type),
	  failed(fi.failed),
	  ptr_cmp(fi.ptr_cmp)
{
	std::vector<const Expression*>::const_iterator i;
	for (i = fi.param_value.begin(); i != fi.param_value.end(); ++i) {
		const Expression *expr = (*i)->clone();
		param_value.push_back(expr);
	}
	//assert(fi.op_flags);
	op_flags = fi.op_flags ? fi.op_flags->clone() : 0;
}

/*
 *
 */
FunctionInvocation::~FunctionInvocation(void)
{
	std::vector<const Expression*>::const_iterator i;
	for (i = param_value.begin(); i != param_value.end(); ++i) {
		delete (*i);
	}
	param_value.clear();

	if (op_flags)
		delete op_flags;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Return true if `eFunc' defines and order for the evaluation of its
 * arguments.
 */
bool
FunctionInvocation::IsOrderedStandardFunc(eBinaryOps eFunc)
{
	return ((eFunc == eAnd) || (eFunc == eOr));
}

/*
 * Return true if `op' is suitable as a floating point binary operator
 */
bool
FunctionInvocation::BinaryOpWorksForFloat(eBinaryOps op)
{
	switch (op) {
		case eAdd:
		case eSub:
		case eMul:
		case eDiv:
		case eCmpGt:
		case eCmpLt:
		case eCmpGe:
		case eCmpLe:
		case eCmpEq:
		case eCmpNe: // fall-through
			return true;
		default:
			return false;
	}
}

bool
FunctionInvocation::UnaryOpWorksForFloat(eUnaryOps op)
{
	switch (op) {
		case ePlus:
		case eMinus:
		case eNot: // fall-through
			return true;
		default:
			return false;
	}
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

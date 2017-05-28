// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2014, 2015, 2017 The University of Utah
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

#include "StatementAssign.h"
#include <cassert>
#include <iostream>
#include "Common.h"
#include "CGContext.h"
#include "CGOptions.h"
#include "Expression.h"
#include "Variable.h"
#include "Type.h"
#include "Block.h"
#include "Fact.h"
#include "FactMgr.h"
#include "Function.h"
#include "FunctionInvocation.h"
#include "ExpressionVariable.h"
#include "ExpressionFuncall.h"
#include "Lhs.h"
#include "Bookkeeper.h"
#include "SafeOpFlags.h"
#include "Error.h"
#include "ProbabilityTable.h"
#include "DepthSpec.h"
#include "CompatibleChecker.h"
#include "Constant.h"
#include "VectorFilter.h"

#include "random.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//
// use a table to define probabilities of different kinds of statements
// Must initialize it before use
DistributionTable StatementAssign::assignOpsTable_;

void
StatementAssign::InitProbabilityTable()
{
	assignOpsTable_.add_entry((int)eSimpleAssign, 70);
	assignOpsTable_.add_entry((int)eBitAndAssign, 10);
	assignOpsTable_.add_entry((int)eBitXorAssign, 10);
	assignOpsTable_.add_entry((int)eBitOrAssign,  10);
	if (CGOptions::pre_incr_operator())
		assignOpsTable_.add_entry((int)ePreIncr, 5);
	if (CGOptions::pre_decr_operator())
		assignOpsTable_.add_entry((int)ePreDecr, 5);
	if (CGOptions::post_incr_operator())
		assignOpsTable_.add_entry((int)ePostIncr, 5);
	if (CGOptions::post_decr_operator()) {
		assignOpsTable_.add_entry((int)ePostDecr, 5);
	}
}

eAssignOps
StatementAssign::AssignOpsProbability(const Type* type)
{
	if (!CGOptions::compound_assignment()) {
		return eSimpleAssign;
	}
	// First, floating point values do not apply to |=, &= and ^=.
	// Second, similar to signed integers, we don't generate pre- or post-
	// operators for floating point values. Instead, we will wrap all
	// of these operations into safe_float_math later.
	if (type && (type->eType != eSimple || type->get_base_type()->is_float())) {
		return eSimpleAssign;
	}

	VectorFilter filter(&assignOpsTable_);
	if (type && type->is_signed()) {
		filter.add(ePreIncr).add(ePreDecr).add(ePostIncr).add(ePostDecr);
	}

	int value = rnd_upto(filter.get_max_prob(), &filter);
	return (eAssignOps)(filter.lookup(value));
}

/*
 *
 */
StatementAssign *
StatementAssign::make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qf)
{
	// decide assignment operator
	eAssignOps op = AssignOpsProbability(type);
	// bool stand_alone_assign = false;

	// decide type
	if (type == NULL) {
		// stand_alone_assign = true;
		type = Type::SelectLType(!cg_context.get_effect_context().is_side_effect_free(), op);
	}
	assert(!type->is_const_struct_union());

	FactMgr* fm = get_fact_mgr(&cg_context);
	assert(fm);
	// pre-generation initializations
	Lhs *lhs = NULL;
	Expression *e = NULL;
	Effect running_eff_context(cg_context.get_effect_context());
	Effect rhs_accum, lhs_accum;
	CGContext rhs_cg_context(cg_context, running_eff_context, &rhs_accum);
	CVQualifiers qfer;
	if (qf) qfer = *qf;

	if (need_no_rhs(op)) {
		e = Constant::make_int(1);
		// if we are creating standalone statements like x++, any qualifers fit
		if (qf == NULL) qfer.wildcard = true;
	}
	else if (CGOptions::strict_volatile_rule()) {
		if (type->is_volatile_struct_union())
			return NULL;

		e = Expression::make_random(rhs_cg_context, type, qf);
		ERROR_GUARD_AND_DEL1(NULL, e);
		if (!qf) {
			qfer = e->get_qualifiers();
			// lhs should not has "const" qualifier
			qfer.accept_stricter = true;
		}

		// for compound assignment, generate LHS in the effect context of RHS
		if (op != eSimpleAssign) {
			running_eff_context.add_effect(rhs_accum);
			// for now, just use non-volatile as LHS for compound assignments
			qfer.set_volatile(false);
		}
		running_eff_context.add_effect(rhs_accum);
		// if the rhs is volatile, almost came from dereferencing a pointer,
		// then make sure lhs is not a vol
		if (qfer.get_volatiles().size() && qfer.is_volatile())
			qfer.set_volatile(false);
	}
	else {
		e = Expression::make_random(rhs_cg_context, type, qf);
		ERROR_GUARD_AND_DEL1(NULL, e);
		if (!qf) {
			qfer = e->get_qualifiers();
			// lhs should not has "const" qualifier
			qfer.accept_stricter = true;
		}

		// for compound assignment, generate LHS in the effect context of RHS
		if (op != eSimpleAssign) {
			running_eff_context.add_effect(rhs_accum);
			// for now, just use non-volatile as LHS for compound assignments
			qfer.set_volatile(false);
		}
	}
	cg_context.merge_param_context(rhs_cg_context, true);
	running_eff_context.write_var_set(rhs_accum.get_lhs_write_vars());

	CGContext lhs_cg_context(cg_context, running_eff_context, &lhs_accum);
	lhs_cg_context.get_effect_stm() = rhs_cg_context.get_effect_stm();
	lhs_cg_context.curr_rhs = e;

	bool prev_flag = CGOptions::match_exact_qualifiers(); // keep a copy of previous flag
	if (qf) CGOptions::match_exact_qualifiers(true);      // force exact qualifier match when selecting vars
	if (CGOptions::strict_float()) {
		lhs = Lhs::make_random(lhs_cg_context, &e->get_type(), &qfer, op != eSimpleAssign, need_no_rhs(op));
	}
	else {
		lhs = Lhs::make_random(lhs_cg_context, type, &qfer, op != eSimpleAssign, need_no_rhs(op));
	}
	if (qf) CGOptions::match_exact_qualifiers(prev_flag); // restore flag
	ERROR_GUARD_AND_DEL2(NULL, e, lhs);

	// typecast, if needed.
	e->check_and_set_cast(type);
	if (CGOptions::ccomp() && lhs->get_var()->isBitfield_) {
		e->cast_type = type;
	}
	// e can be of float type. So, we reset its
	if ((lhs->get_var()->type->get_base_type()->is_float() || e->get_type().get_base_type()->is_float())
	    && !StatementAssign::AssignOpWorksForFloat(op)) {
		op = eSimpleAssign;
	}

	if (CompatibleChecker::compatible_check(e, lhs)) {
		Error::set_error(COMPATIBLE_CHECK_ERROR);
		delete e;
		delete lhs;
		return NULL;
	}

	cg_context.merge_param_context(lhs_cg_context, true);
	ERROR_GUARD_AND_DEL2(NULL, e, lhs);
	StatementAssign *stmt_assign = make_possible_compound_assign(cg_context, type, *lhs, op, *e);
	ERROR_GUARD_AND_DEL2(NULL, e, lhs);
	return stmt_assign;
}

bool
StatementAssign::safe_assign(eAssignOps op)
{
	switch (op) {
	case eBitAndAssign: // fall-through
	case eBitXorAssign: // fall-through
	case eBitOrAssign:
		return true;
	default:
		return false;
	}
}

StatementAssign *
StatementAssign::make_possible_compound_assign(CGContext &cg_context,
				 const Type *type,
				 const Lhs &l,
				 eAssignOps op,
				 const Expression &e)
{
	eBinaryOps bop = compound_to_binary_ops(op);
	const Expression *rhs = NULL;
	SafeOpFlags *fs = NULL;
	std::string tmp1;
	std::string tmp2;

	if (bop != MAX_BINARY_OP) {
		SafeOpFlags *local_fs  = NULL;
		FunctionInvocation* fi = NULL;
		if (safe_assign(op)) {
			local_fs = SafeOpFlags::make_dummy_flags();
			fi = new FunctionInvocationBinary(bop, local_fs);
		}
		else {
			local_fs = SafeOpFlags::make_random_binary(type, &(l.get_type()), &(l.get_type()), sOpAssign, bop);
			ERROR_GUARD(NULL);
			fi = FunctionInvocationBinary::CreateFunctionInvocationBinary(cg_context, bop, local_fs);
			tmp1 = dynamic_cast<FunctionInvocationBinary*>(fi)->get_tmp_var1();
			tmp2 = dynamic_cast<FunctionInvocationBinary*>(fi)->get_tmp_var2();
		}
		fs = local_fs->clone();
        	fi->add_operand(new ExpressionVariable(*(l.get_var()), &l.get_type()));
        	fi->add_operand(e.clone());
        	rhs = new ExpressionFuncall(*fi);
        }
	else {
		rhs = &e;
#if 0
		if (e.term_type == eFunction) {
			const ExpressionFuncall* func = dynamic_cast<const ExpressionFuncall*>(&e);
			if (!func->get_invoke().safe_invocation()) {
				fs = SafeOpFlags::make_dummy_flags();
				fs = NULL;
			}
		}
#endif
		if (op != eSimpleAssign) {
			fs = SafeOpFlags::make_random_binary(type, &(l.get_type()), &(rhs->get_type()), sOpAssign, bop);
			bool op1 = fs->get_op1_sign();
			bool op2 = fs->get_op2_sign();
			enum SafeOpSize size = fs->get_op_size();

			eSimpleType type1 = SafeOpFlags::flags_to_type(op1, size);
			eSimpleType type2 = SafeOpFlags::flags_to_type(op2, size);

			const Block *blk = cg_context.get_current_block();
			assert(blk);

			tmp1 = blk->create_new_tmp_var(type1);
			tmp2 = blk->create_new_tmp_var(type2);
			ERROR_GUARD(NULL);
		}
	}
	StatementAssign *sa = new StatementAssign(cg_context.get_current_block(), l, op, e, rhs, fs, tmp1, tmp2);
	return sa;
}

eBinaryOps
StatementAssign::compound_to_binary_ops(eAssignOps op)
{
	eBinaryOps bop = MAX_BINARY_OP;
	switch (op)
	{
	case eAddAssign: bop = eAdd; break;
	case eSubAssign: bop = eSub; break;
	case eMulAssign: bop = eMul; break;
	case eDivAssign: bop = eDiv; break;
	case eRemAssign: bop = eMod; break;
	case eBitAndAssign:	bop = eBitAnd; break;
	case eBitXorAssign:	bop = eBitXor; break;
	case eBitOrAssign:  bop = eBitOr; break;
	case ePreDecr:   bop = eSub; break;
	case ePostDecr:  bop = eSub; break;
	case ePreIncr:   bop = eAdd; break;
	case ePostIncr:  bop = eAdd; break;
	case eLShiftAssign: bop = eLShift; break;
	case eRShiftAssign: bop = eRShift; break;
	default: bop = MAX_BINARY_OP; break;
	}
	return bop;
}

bool
StatementAssign::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	vector<const Fact*> inputs_copy = inputs;
	// LHS and RHS can be evaludated in arbitrary order, try RHS first
	Effect running_eff_context(cg_context.get_effect_context());
	Effect rhs_accum, lhs_accum;
	CGContext rhs_cg_context(cg_context, running_eff_context, &rhs_accum);
	if (!expr.visit_facts(inputs, rhs_cg_context)) {
		return false;
	}

	// for compound assignment, LHS needs to be evaluated in the effect context of RHS
	// Yang: do we also need to consider strict_volatile_rule here?
	// if ((op != eSimpleAssign) || (CGOptions::strict_volatile_rule())) {
	if (op != eSimpleAssign) {
		running_eff_context.add_effect(rhs_accum);
	}
	cg_context.merge_param_context(rhs_cg_context, true);
	running_eff_context.write_var_set(rhs_accum.get_lhs_write_vars());

	CGContext lhs_cg_context(cg_context, running_eff_context, &lhs_accum);
	lhs_cg_context.get_effect_stm() = rhs_cg_context.get_effect_stm();
	lhs_cg_context.curr_rhs = &expr;
	if (!lhs.visit_facts(inputs, lhs_cg_context)) {
		return false;
	}
	cg_context.merge_param_context(lhs_cg_context, true);
	//cg_context.get_effect_stm() = lhs_cg_context.get_effect_stm();
	FactMgr::update_fact_for_assign(this, inputs);
	// save effect
	FactMgr* fm = get_fact_mgr(&cg_context);
	fm->map_stm_effect[this] = cg_context.get_effect_stm();
	return true;
}

std::vector<const ExpressionVariable*>
StatementAssign::get_dereferenced_ptrs(void) const
{
	return expr.get_dereferenced_ptrs();
}

bool
StatementAssign::has_uncertain_call_recursive(void) const
{
	return expr.has_uncertain_call_recursive();
}

/*
 *
 */
StatementAssign::StatementAssign(Block* b, const Lhs &l,
				 const Expression &e,
				 eAssignOps op,
				 const SafeOpFlags *flags)
	: Statement(eAssign, b),
	  op(op),
	  lhs(l),
	  expr(e),
	  rhs(&expr),
	  op_flags(flags)
{
	// Nothing else to do.
}

/*
 *
 */
StatementAssign::StatementAssign(Block* b, const Lhs &l,
				 eAssignOps op,
				 const Expression &e,
				 const Expression *er,
				 const SafeOpFlags *flags,
				 std::string &tmp_name1,
				 std::string &tmp_name2)
	: Statement(eAssign, b),
	  op(op),
	  lhs(l),
	  expr(e),
	  rhs(er),
	  op_flags(flags),
	  tmp_var1(tmp_name1),
	  tmp_var2(tmp_name2)
{
}

#if 0
/*
 *
 */
StatementAssign::StatementAssign(const StatementAssign &sa)
	: Statement(sa.get_type()),
	  op(sa.op),
	  lhs(sa.lhs),
	  rhs(sa.rhs),
	  expr(sa.expr)
{
	op_flags = sa.op_flags.clone();
}
#endif

/*
 *
 */
StatementAssign::~StatementAssign(void)
{
	if (rhs != &expr) {
		delete rhs;
	}
	delete &lhs;
	delete &expr;
	if (op_flags)
		delete op_flags;
}

/*
 * output assign operator
 */
void
StatementAssign::output_op(std::ostream &out) const
{
	switch (op) {
	case eSimpleAssign: out << "="; break;
	case eMulAssign:	out << "*="; break;
	case eDivAssign:	out << "/="; break;
	case eRemAssign:	out << "%="; break;
	case eAddAssign:	out << "+="; break;
	case eSubAssign:	out << "-="; break;
	case eLShiftAssign:	out << "<<="; break;
	case eRShiftAssign:	out << ">>="; break;
	case eBitAndAssign:	out << "&="; break;
	case eBitXorAssign:	out << "^="; break;
	case eBitOrAssign:	out << "|="; break;

	case ePreIncr:		out << "++"; break;
	case ePreDecr:		out << "--"; break;
	case ePostIncr:		out << "++"; break;
	case ePostDecr:		out << "--"; break;
	}
}

/*
 *
 */
void
StatementAssign::Output(std::ostream &out, FactMgr* /*fm*/, int indent) const
{
	output_tab(out, indent);
	OutputAsExpr(out);
	out << ";";
	outputln(out);
}

void
StatementAssign::OutputSimple(std::ostream &out) const
{
	switch (op) {
	default:
		lhs.Output(out);
		out << " ";
		output_op(out);
		out << " ";
		expr.Output(out);
		break;

	case ePreIncr:
	case ePreDecr:
		output_op(out);
		lhs.Output(out);
		break;

	case ePostIncr:
	case ePostDecr:
		lhs.Output(out);
		output_op(out);
		break;
	}
}

/*
 *
 */
void
StatementAssign::OutputAsExpr(std::ostream &out) const
{
	if (CGOptions::avoid_signed_overflow() && op_flags) {
		switch (op) {

		case eSimpleAssign:
		case eBitAndAssign:
		case eBitXorAssign:
		case eBitOrAssign:
		{
			eBinaryOps bop = compound_to_binary_ops(op);
			lhs.Output(out);
			out << " ";
			if (CGOptions::ccomp() && (bop != MAX_BINARY_OP) && (lhs.is_volatile())) {
				out << "=" << " ";
				lhs.Output(out);
				out << " " << FunctionInvocationBinary::get_binop_string(bop) << " ";
				expr.Output(out);
			}
			else {
				output_op(out);
				out << " ";
				expr.Output(out);
			}
			break;
		}

		case ePreIncr:
			out << "++"; lhs.Output(out);
			break;
		case ePreDecr:
			out << "--"; lhs.Output(out);
			break;
		case ePostIncr:
			lhs.Output(out); out << "++";
			break;
		case ePostDecr:	lhs.Output(out);
			out << "--";
			break;

		case eAddAssign:
		case eSubAssign:
			{
				enum eBinaryOps bop = compound_to_binary_ops(op);
				assert(op_flags);
				string fname = op_flags->to_string(bop);
				int id = SafeOpFlags::to_id(fname);
				// don't use safe math wrapper if this function is specified in "--safe-math-wrapper"
				if (!CGOptions::safe_math_wrapper(id)) {
					OutputSimple(out);
					return;
				}
				lhs.Output(out);
				out << " = " << fname << "(";
				if (CGOptions::math_notmp()) {
					out << tmp_var1 << ", ";
				}

				lhs.Output(out);
				out << ", ";
				if (CGOptions::math_notmp()) {
					out << tmp_var2 << ", ";
				}

				if (op == eAddAssign ||
					op == eSubAssign) {
					expr.Output(out);
				} else {
					out << (CGOptions::mark_mutable_const() ? "(1)" : "1");
				}
				if (CGOptions::identify_wrappers()) {
					out << ", " << id;
				}
				out << ")";
			}
			break;

		default:
			assert(false);
			break;
		}
	} else {
		OutputSimple(out);
	}
}

bool
StatementAssign::AssignOpWorksForFloat(eAssignOps op)
{
	switch (op) {
		case eSimpleAssign:
		case eMulAssign:
		case eDivAssign:
		case eAddAssign:
		case eSubAssign:
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

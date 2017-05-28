// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2017 The University of Utah
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

#include "FunctionInvocationBinary.h"
#include <cassert>

#include "Common.h"

#include "CGOptions.h"
#include "Expression.h"
#include "FactMgr.h"
#include "Type.h"
#include "SafeOpFlags.h"
#include "CGContext.h"
#include "Block.h"
#include "random.h"

using namespace std;

static vector<bool> needcomma;  // Flag to track output of commas

///////////////////////////////////////////////////////////////////////////////

FunctionInvocationBinary *
FunctionInvocationBinary::CreateFunctionInvocationBinary(CGContext &cg_context,
						eBinaryOps op,
						SafeOpFlags *flags)
{
	FunctionInvocationBinary *fi = NULL;
	assert(flags);

	if (flags && FunctionInvocationBinary::safe_ops(op)) {
		bool op1 = flags->get_op1_sign();
		bool op2 = flags->get_op2_sign();
		enum SafeOpSize size = flags->get_op_size();

		eSimpleType type1 = SafeOpFlags::flags_to_type(op1, size);
		eSimpleType type2 = SafeOpFlags::flags_to_type(op2, size);

		const Block *blk = cg_context.get_current_block();
		assert(blk);

		std::string tmp_var1 = blk->create_new_tmp_var(type1);
		std::string tmp_var2;
		if (op == eLShift || op == eRShift)
			tmp_var2 = blk->create_new_tmp_var(type2);
		else
			tmp_var2 = blk->create_new_tmp_var(type1);

		fi = new FunctionInvocationBinary(op, flags, tmp_var1, tmp_var2);
	}
	else {
		fi = new FunctionInvocationBinary(op, flags);
	}
	return fi;
}


/*
 * XXX: replace with a useful constructor.
 */
FunctionInvocationBinary::FunctionInvocationBinary(eBinaryOps op, const SafeOpFlags *flags)
	: FunctionInvocation(eBinaryPrim, flags),
	  eFunc(op),
	  tmp_var1(""),
	  tmp_var2("")
{
	// Nothing else to do.  Caller must build useful params.
}

FunctionInvocationBinary::FunctionInvocationBinary(eBinaryOps op, const SafeOpFlags *flags,
						std::string &name1, std::string &name2)
	: FunctionInvocation(eBinaryPrim, flags),
	  eFunc(op),
	  tmp_var1(name1),
	  tmp_var2(name2)
{
	// Nothing else to do.  Caller must build useful params.
}

/*
 * copy constructor
 */
FunctionInvocationBinary::FunctionInvocationBinary(const FunctionInvocationBinary &fbinary)
	: FunctionInvocation(fbinary),
	  eFunc(fbinary.eFunc),
	  tmp_var1(fbinary.tmp_var1),
	  tmp_var2(fbinary.tmp_var2)
{
	// Nothing to do
}

FunctionInvocationBinary::FunctionInvocationBinary(eBinaryOps op , const Expression* exp1, const Expression* exp2, const SafeOpFlags *flags)
	: FunctionInvocation(eBinaryPrim, flags),
	  eFunc(op)
{
	param_value.clear();
	add_operand(exp1);
	add_operand(exp2);
}

/*
 *
 */
FunctionInvocationBinary::~FunctionInvocationBinary(void)
{
	// Nothing to do.
}

/*
 *
 */
FunctionInvocation *
FunctionInvocationBinary::clone() const
{
	return new FunctionInvocationBinary(*this);
}
///////////////////////////////////////////////////////////////////////////////

bool
FunctionInvocationBinary::safe_ops(eBinaryOps op)
{
	switch(op) {
	case eAdd:
	case eSub:
	case eMul:
	case eMod:
	case eDiv:
	case eLShift:
	case eRShift:
		return true;
	default:
		return false;
	}
}

/* do some constant folding */
bool
FunctionInvocationBinary::equals(int num) const
{
	assert(param_value.size() == 2);
	if (num == 0) {
		if (param_value[0]->equals(0) &&
			(eFunc==eMul || eFunc==eDiv || eFunc==eMod || eFunc==eLShift || eFunc==eRShift || eFunc==eAnd || eFunc==eBitAnd)) {
			return true;
		}
		if (param_value[1]->equals(0) && (eFunc==eMul || eFunc==eAnd || eFunc==eBitAnd)) {
			return true;
		}
		if (param_value[0] == param_value[1] && (eFunc==eSub || eFunc==eCmpGt || eFunc==eCmpLt || eFunc==eCmpNe)) {
			return true;
		}
		if ((param_value[1]->equals(1) || param_value[1]->equals(-1)) && eFunc==eMod) {
			return true;
		}
	}
	return false;
}

bool
FunctionInvocationBinary::is_0_or_1(void) const
{
	return eFunc==eCmpGt || eFunc==eCmpLt || eFunc==eCmpGe || eFunc==eCmpLe || eFunc==eCmpEq || eFunc==eCmpNe;
}

bool
FunctionInvocationBinary::is_return_type_float() const
{
	assert(op_flags);
	return op_flags->get_op_size() == sFloat;
}

/*
 * XXX --- we should memoize the types of "standard functions."
 */
const Type &
FunctionInvocationBinary::get_type(void) const
{
	if (is_return_type_float())
		return Type::get_simple_type(eFloat);
	switch (eFunc) {
	default:
		assert(!"invalid operator in FunctionInvocationBinary::get_type()");
		break;

	case eAdd:
	case eSub:
	case eMul:
	case eDiv:
	case eMod:
	case eBitXor:
	case eBitAnd:
	case eBitOr:
		{
			const Type &l_type = param_value[0]->get_type();
			const Type &r_type = param_value[1]->get_type();
			// XXX --- not really right!
			if ((l_type.is_signed()) && (r_type.is_signed())) {
				return Type::get_simple_type(eInt);
			} else {
				return Type::get_simple_type(eUInt);
			}
		}
		break;

	case eCmpGt:
	case eCmpLt:
	case eCmpGe:
	case eCmpLe:
	case eCmpEq:
	case eCmpNe:
	case eAnd:
	case eOr:
		return Type::get_simple_type(eInt);
		break;

	case eRShift:
	case eLShift:
		{
			const Type &l_type = param_value[0]->get_type();
			// XXX --- not really right!
			if (l_type.is_signed()) {
				return Type::get_simple_type(eInt);
			} else {
				return Type::get_simple_type(eUInt);
			}
		}
		break;
	}
	assert(0);
	return Type::get_simple_type(eInt);
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
static void
OutputStandardFuncName(eBinaryOps eFunc, std::ostream &out)
{
	switch (eFunc) {
		// Math Ops
	case eAdd:		out << "+";     break;
	case eSub:		out << "-";     break;
	case eMul:		out << "*";     break;
	case eDiv:		out << "/";     break;
	case eMod:		out << "%";     break;

		// Logical Ops
	case eAnd:		out << "&&";	break;
	case eOr:		out << "||";	break;
	case eCmpEq:	out << "==";	break;
	case eCmpNe:	out << "!=";	break;
	case eCmpGt:	out << ">";		break;
	case eCmpLt:	out << "<";		break;
	case eCmpLe:	out << "<=";	break;
	case eCmpGe:	out << ">=";	break;

		// Bitwise Ops
	case eBitAnd:	out << "&";		break;
	case eBitOr:	out << "|";		break;
	case eBitXor:	out << "^";		break;
	case eLShift:	out << "<<";    break;
	case eRShift:	out << ">>";    break;
	}
}

std::string
FunctionInvocationBinary::get_binop_string(eBinaryOps bop)
{
	string op_string;
	switch (bop)
	{
	case eAdd: op_string = "+"; break;
	case eSub: op_string = "-"; break;
	case eMul: op_string = "*"; break;
	case eDiv: op_string = "/"; break;
	case eMod: op_string = "%"; break;
	case eBitAnd:	op_string = "&"; break;
	case eBitXor:	op_string = "^"; break;
	case eBitOr:  op_string = "|"; break;
	default: assert(0); break;
	}
	return op_string;
}

/*
 *
 */
void
FunctionInvocationBinary::Output(std::ostream &out) const
{
	bool need_cast = false;
	out << "(";
	// special case for mutated array subscripts, see ArrayVariable::rnd_mutate
	// the rational is we don't need overflow check for this addition because
	// the induction variable is small --- less than the size of array, which
	// has a small upper bound
	if (eFunc == eAdd && op_flags == 0) {
		param_value[0]->Output(out);
		out << " + ";
		param_value[1]->Output(out);
	}
	else {
		switch (eFunc) {
		case eAdd:
		case eSub:
		case eMul:
		case eMod:
		case eDiv:
		case eLShift:
		case eRShift:
			if (CGOptions::avoid_signed_overflow()) {
				string fname = op_flags->to_string(eFunc);
				int id = SafeOpFlags::to_id(fname);
				// don't use safe math wrapper if this function is specified in "--safe-math-wrapper"
				if (CGOptions::safe_math_wrapper(id)) {
					out << fname << "(";
					if (CGOptions::math_notmp()) {
						out << tmp_var1 << ", ";
					}
					param_value[0]->Output(out);
					out << ", ";

					if (CGOptions::math_notmp()) {
						out << tmp_var2 << ", ";
					}
					param_value[1]->Output(out);
					if (CGOptions::identify_wrappers()) {
						out << ", " << id;
					}
					out << ")";
					break;
				}
			}
			need_cast = true;
			// fallthrough!

		default:
			// explicit type casting for op1
			if (need_cast) {
				out << "(";
				op_flags->OutputSize(out);
				out << ")";
			}
			param_value[0]->Output(out);
			out << " ";
			OutputStandardFuncName(eFunc, out);
			out << " ";
			// explicit type casting for op2
			if (need_cast) {
				out << "(";
				op_flags->OutputSize(out);
				out << ")";
			}
			param_value[1]->Output(out);
			break;
		}
	}
	out << ")";
}

/*
 *
 */
void
FunctionInvocationBinary::indented_output(std::ostream &out, int indent) const
{
	if (has_simple_params()) {
		output_tab(out, indent);
		Output(out);
		return;
	}
	output_open_encloser("(", out, indent);
	// special case for mutated array subscripts, see ArrayVariable::rnd_mutate
	// the rational is we don't need overflow check for this addition because
	// the induction variable is small --- less than the size of array, which
	// by default is 10 at most
	if (eFunc == eAdd && op_flags == 0) {
		param_value[0]->indented_output(out, indent);
		out << " + ";
		outputln(out);
		param_value[1]->indented_output(out, indent);
	}
	else {
		switch (eFunc) {
		case eAdd:
		case eSub:
		case eMul:
		case eMod:
		case eDiv:
		case eLShift:
		case eRShift:
			if (CGOptions::avoid_signed_overflow()) {
				output_tab(out, indent);
				out << op_flags->to_string(eFunc);
				outputln(out);
				output_open_encloser("(", out, indent);
				if (CGOptions::math_notmp()) {
					output_tab(out, indent);
					out << tmp_var1 << ", ";
				}
				outputln(out);
				param_value[0]->indented_output(out, indent);
				out << ", ";
				outputln(out);
				if (CGOptions::math_notmp()) {
					output_tab(out, indent);
					out << tmp_var2 << ", ";
				}
				outputln(out);
				param_value[1]->indented_output(out, indent);
				output_close_encloser(")", out, indent);
				break;
			}
			// fallthrough!

		default:
			param_value[0]->indented_output(out, indent);
			out << " ";
			OutputStandardFuncName(eFunc, out);
			out << " ";
			outputln(out);
			param_value[1]->indented_output(out, indent);
			break;
		}
	}
	output_close_encloser(")", out, indent);
}

bool
FunctionInvocationBinary::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	bool skippable = IsOrderedStandardFunc(eFunc);
	assert(param_value.size() == 2);
	if (skippable) {
		const Expression* value = param_value[0];
		if (value->visit_facts(inputs, cg_context)) {
			vector<const Fact*> inputs_copy = inputs;
			value = param_value[1];
			if (value->visit_facts(inputs, cg_context)) {
				// the second parameter may or may not be evaludated, thus need to
				// merge with the post-param0 env.
				merge_facts(inputs, inputs_copy);
				return true;
			}
		}
		return false;
	}
	// for other binary invocations, use the standard visitor
	return FunctionInvocation::visit_facts(inputs, cg_context);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

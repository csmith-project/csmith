// -*- mode: C++ -*-
//
// Copyright (c) 2009, 2010, 2011, 2014, 2015, 2017 The University of Utah
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

#include "FunctionInvocationUnary.h"
#include <cassert>

#include "Common.h"
#include "CGOptions.h"
#include "Expression.h"
#include "Type.h"
#include "Block.h"
#include "SafeOpFlags.h"
#include "CGContext.h"
#include "random.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

FunctionInvocationUnary *
FunctionInvocationUnary::CreateFunctionInvocationUnary(
		CGContext &cg_context, eUnaryOps op, SafeOpFlags *flags)
{
	FunctionInvocationUnary *fi = NULL;
	if (flags) {
		bool op1 = flags->get_op1_sign();
		enum SafeOpSize size = flags->get_op_size();

		eSimpleType type = SafeOpFlags::flags_to_type(op1, size);
		const Block *blk = cg_context.get_current_block();
		assert(blk);

		std::string tmp_var = blk->create_new_tmp_var(type);
		fi = new FunctionInvocationUnary(op, flags, tmp_var);
	}
	else {
		fi = new FunctionInvocationUnary(op, flags);
	}
	assert(fi);
	return fi;
}

FunctionInvocationUnary::FunctionInvocationUnary(eUnaryOps op, SafeOpFlags *flags)
	: FunctionInvocation(eUnaryPrim, flags),
	  eFunc(op),
	  tmp_var("")
{
	// Nothing else to do.  Caller must build useful params.
}

/*
 * XXX: replace with a useful constructor.
 */
FunctionInvocationUnary::FunctionInvocationUnary(eUnaryOps op, SafeOpFlags *flags, std::string &name)
	: FunctionInvocation(eUnaryPrim, flags),
	  eFunc(op),
	  tmp_var(name)
{
	// Nothing else to do.  Caller must build useful params.
}

/*
 * copy constructor
 */
FunctionInvocationUnary::FunctionInvocationUnary(const FunctionInvocationUnary &funary)
	: FunctionInvocation(funary),
	  eFunc(funary.eFunc),
	  tmp_var(funary.tmp_var)
{
	// Nothing to do
}

/*
 *
 */
FunctionInvocationUnary::~FunctionInvocationUnary(void)
{
	// Nothing to do.
}

/*
 *
 */
FunctionInvocation *
FunctionInvocationUnary::clone() const
{
	return new FunctionInvocationUnary(*this);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * XXX --- we should memoize the types of "standard functions."
 */
const Type &
FunctionInvocationUnary::get_type(void) const
{
	switch (eFunc) {
	default:
		assert(!"invalid operator in FunctionInvocationUnary::get_type()");
		break;

	case ePlus:
	case eMinus:
	case eBitNot:
		return param_value[0]->get_type();
		break;

	case eNot:
		return Type::get_simple_type(eInt);
		break;
	}
	assert(0);
	return Type::get_simple_type(eInt);
}

/*
 *
 */
bool
FunctionInvocationUnary::compatible(const Variable *v) const
{
	if (!param_value.empty())
		return param_value[0]->compatible(v);
	return false;
}

/* do some constant folding */
bool
FunctionInvocationUnary::equals(int num) const
{
	assert(!param_value.empty());
	if (num == 0 && eFunc == eNot && param_value[0]->not_equals(0)) {
		return true;
	}
	if (num == 1 && eFunc == eNot && param_value[0]->equals(0)) {
		return true;
	}
	if (eFunc == eMinus && param_value[0]->equals(num * -1)) {
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
static void
OutputStandardFuncName(eUnaryOps eFunc, std::ostream &out)
{
	switch (eFunc) {
		// Math Ops
	case ePlus:		out << "+";	break;
	case eMinus:	out << "-";	break;

		// Logical Ops
	case eNot:		out << "!";	break;

		// Bitwise Ops
	case eBitNot:	out << "~";	break;
	}
}

bool
FunctionInvocationUnary::safe_invocation() const
{
	return (eFunc != eMinus);
}

/*
 *
 */
void
FunctionInvocationUnary::Output(std::ostream &out) const
{
	bool need_cast = false;
	out << "(";
	switch (eFunc) {
	default:
		assert(!"invalid operator in FunctionInvocationUnary::Output()");
		break;

	case eMinus:
		if (CGOptions::avoid_signed_overflow()) {
			assert(op_flags);
			if (op_flags->get_op_size() != sFloat) {
				string fname = op_flags->to_string(eFunc);
				int id = SafeOpFlags::to_id(fname);
				// don't use safe math wrapper if this function is specified in "--safe-math-wrapper"
				if (CGOptions::safe_math_wrapper(id)) {
					out << fname << "(";
					if (CGOptions::math_notmp()) {
						out << tmp_var << ", ";
					}
					param_value[0]->Output(out);
					if (CGOptions::identify_wrappers()) {
						out << ", " << id;
					}
					out << ")";
					break;
				}
			}
			else {
				OutputStandardFuncName(eFunc, out);
				param_value[0]->Output(out);
				break;
			}
		}
		need_cast = true;
		// Fallthrough!

	case ePlus:
	case eNot:
	case eBitNot:
		OutputStandardFuncName(eFunc, out);
		// explicit type casting for op1
		if (need_cast) {
			out << "(";
			op_flags->OutputSize(out);
			out << ")";
		}
		param_value[0]->Output(out);
		break;
	}
	out << ")";
}

/*
 *
 */
void
FunctionInvocationUnary::indented_output(std::ostream &out, int indent) const
{
	out << "(";
	switch (eFunc) {
	default:
		assert(!"invalid operator in FunctionInvocationUnary::Output()");
		break;

	case eMinus:
		if (CGOptions::avoid_signed_overflow()) {
			out << op_flags->to_string(eFunc);
			output_open_encloser("(", out, indent);
			param_value[0]->indented_output(out, indent);
			output_close_encloser(")", out, indent);
			break;
		}
		// Fallthrough!

	case ePlus:
	case eNot:
	case eBitNot:
		OutputStandardFuncName(eFunc, out);
		param_value[0]->indented_output(out, indent);
		break;
	}
	out << ")";
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

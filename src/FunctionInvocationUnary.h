// -*- mode: C++ -*-
//
// Copyright (c) 2009, 2010, 2011, 2015 The University of Utah
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

#ifndef FUNCTION_INVOCATION_UNARY_H
#define FUNCTION_INVOCATION_UNARY_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <string>

#include "FunctionInvocation.h"

class Type;
class CGContext;

class FunctionInvocationUnary : public FunctionInvocation
{
	friend class FunctionInvocation; /* XXX --- yuck! */

public:
	static FunctionInvocationUnary *CreateFunctionInvocationUnary(
				CGContext &cg_context,
				eUnaryOps op,
				SafeOpFlags *flags);

	virtual ~FunctionInvocationUnary(void);

	virtual FunctionInvocation* clone() const;

	virtual const Type &get_type(void) const;

	virtual bool compatible(const Variable *v) const;

	virtual void Output(std::ostream &) const;

	virtual void indented_output(std::ostream &out, int indent) const;

	virtual bool safe_invocation() const;

	virtual bool equals(int num) const;

	virtual bool is_0_or_1(void) const { return eFunc == eNot;}

private:
	eUnaryOps eFunc;

	std::string tmp_var;

private:
	FunctionInvocationUnary(eUnaryOps op, SafeOpFlags *flags, std::string &name);

	FunctionInvocationUnary(eUnaryOps op, SafeOpFlags *flags);

	explicit FunctionInvocationUnary(const FunctionInvocationUnary &funary);

	// unimplemented
	FunctionInvocationUnary &operator=(const FunctionInvocationUnary &fi);
};

///////////////////////////////////////////////////////////////////////////////

#endif // FUNCTION_INVOCATION_UNARY_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

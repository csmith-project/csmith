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

#ifndef FUNCTION_INVOCATION_BINARY_H
#define FUNCTION_INVOCATION_BINARY_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <string>

#include "FunctionInvocation.h"

class Type;
class Variable;
class CGContext;

class FunctionInvocationBinary : public FunctionInvocation
{
	friend class FunctionInvocation; /* XXX --- yuck! */

public:

	static FunctionInvocationBinary *CreateFunctionInvocationBinary(CGContext &cg_context, eBinaryOps op, SafeOpFlags *flags);

	FunctionInvocationBinary(eBinaryOps op, const SafeOpFlags *flags);

	FunctionInvocationBinary(eBinaryOps op, const SafeOpFlags *flags, std::string &name1, std::string &name2);

	FunctionInvocationBinary(eBinaryOps op, const Expression* exp1, const Expression* exp2, const SafeOpFlags *flags);

	virtual ~FunctionInvocationBinary(void);

	virtual FunctionInvocation * clone() const;

	virtual bool compatible(const Variable *) const { return false; }

	virtual const Type &get_type(void) const;

	virtual void Output(std::ostream &) const;

	virtual void indented_output(std::ostream &out, int indent) const;

	virtual bool safe_invocation() const { return false; }

	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;

	eBinaryOps get_operation(void) const {return eFunc;}
	void set_operation(eBinaryOps op) { eFunc = op;}

	std::string get_tmp_var1() { return tmp_var1; }

	std::string get_tmp_var2() { return tmp_var2; }

	static std::string get_binop_string(eBinaryOps bop);

	virtual bool equals(int num) const ;
	virtual bool is_0_or_1(void) const;

private:
	eBinaryOps eFunc;

	std::string tmp_var1;

	std::string tmp_var2;

private:

	bool is_return_type_float() const;

	static bool safe_ops(eBinaryOps op);

	// unimplemented
	FunctionInvocationBinary &operator=(const FunctionInvocationBinary &fi);

	explicit FunctionInvocationBinary(const FunctionInvocationBinary &fbinary);

};

///////////////////////////////////////////////////////////////////////////////

#endif // FUNCTION_INVOCATION_BINARY_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

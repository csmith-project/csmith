// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011 The University of Utah
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

#ifndef FUNCTION_INVOCATION_USER_H
#define FUNCTION_INVOCATION_USER_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <vector>
#include "FunctionInvocation.h"
#include "Type.h"
#include "Fact.h"

class CGContext;
class Function;
class Statement;
class Variable;
class SafeOpFlags;
class Variable;

class FunctionInvocationUser: public FunctionInvocation
{
	friend class FunctionInvocation;

public:
	// factory method
	static FunctionInvocationUser* build_invocation_and_function(CGContext &cg_context, const Type* type, const CVQualifiers* qfer);

	virtual ~FunctionInvocationUser(void);

	virtual FunctionInvocation* clone() const;

	virtual bool compatible(const Variable *) const { return false; }

	virtual const Type &get_type(void) const;

	virtual void Output(std::ostream &) const;

	virtual void indented_output(std::ostream &out, int indent) const;

	virtual bool safe_invocation() const { return true; }

	const Function* get_func(void) const { return func; };

	bool revisit(std::vector<const Fact*>& inputs, CGContext& cg_context) const;

	void save_return_fact(const vector<const Fact*>& facts) const;

	static void doFinalization(void);

	FunctionInvocationUser(Function *target, bool isBackLink, const SafeOpFlags *flags);

private:
	Function *func;
	bool isBackLink;

private:
	explicit FunctionInvocationUser(const FunctionInvocationUser &fiu);

	// unimplemented
	//FunctionInvocationUser &operator=(const FunctionInvocationUser &fi);

	bool build_invocation(Function *target, CGContext &cg_context);
};

const Fact* get_return_fact_for_invocation(const FunctionInvocationUser* fiu, const Variable* var, enum eFactCategory cat);

void calls_to_funcs(const vector<const FunctionInvocationUser*>& calls, vector<const Function*>& funcs);
void calls_to_funcs_recursive(const vector<const FunctionInvocationUser*>& calls, vector<const Function*>& funcs);

///////////////////////////////////////////////////////////////////////////////

#endif // FUNCTION_INVOCATION_USER_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

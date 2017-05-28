// -*- mode: C++ -*-
//
// Copyright (c) 2008, 2009, 2010, 2011, 2015, 2017 The University of Utah
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

#include "AbsExtension.h"
#include <sstream>
#include <cassert>
#include "Type.h"
#include "Function.h"
#include "FunctionInvocationUser.h"
#include "ExpressionVariable.h"
#include "ExtensionValue.h"
#include "Effect.h"
#include "Variable.h"
#include "VariableSelector.h"
#include "CGContext.h"

using namespace std;

const char *AbsExtension::tab_ = "    ";
const char *AbsExtension::base_name_ = "x";

void
AbsExtension::Initialize(unsigned int num, vector<ExtensionValue *> &values)
{
	for (unsigned int i = 0; i < num; i++) {
		const Type* type = Type::choose_random_simple();
		std::ostringstream s;
		s << AbsExtension::base_name_ << i;
		ExtensionValue *sv = new ExtensionValue(type, s.str());
		assert(sv);
		values.push_back(sv);
	}
}

FunctionInvocation*
AbsExtension::MakeFuncInvocation(Function *func, std::vector<ExtensionValue *> &values)
{
	FunctionInvocationUser *fi = new FunctionInvocationUser(func, true, NULL);

	vector<ExtensionValue *>::iterator i;
	for (i = values.begin(); i != values.end(); ++i) {
		assert(*i);
		CVQualifiers qfer = (*i)->get_qfer();
		Variable *v = VariableSelector::new_variable((*i)->get_name(), (*i)->get_type(),
								NULL, &qfer);
		Expression *p = new ExpressionVariable(*v, (*i)->get_type());
		fi->param_value.push_back(p);
	}
	return fi;
}

void
AbsExtension::GenerateFirstParameterList(Function &curFunc, std::vector<ExtensionValue *> &values)
{
	vector<ExtensionValue *>::iterator i;
	for (i = values.begin(); i != values.end(); ++i) {
		assert(*i);
		CVQualifiers qfer = (*i)->get_qfer();
		Variable * v = VariableSelector::GenerateParameterVariable((*i)->get_type(), &qfer);
		assert(v);
		curFunc.param.push_back(v);
	}
}

void
AbsExtension::default_output_definitions(ostream &out, std::vector<ExtensionValue *> &values, bool init_flag)
{
	std::vector<ExtensionValue*>::iterator i;
	for (i = values.begin(); i != values.end(); ++i) {
		assert(*i);
		out << AbsExtension::tab_;
		(*i)->get_type()->Output(out);
		out << " " << (*i)->get_name();
		if (init_flag)
			out << " = 0";
		out << ";" << endl;
	}
	out << endl;
}

void
AbsExtension::OutputFirstFunInvocation(std::ostream &out, FunctionInvocation *invoke)
{
	assert(invoke);
	out << "    ";
        invoke->Output(out);
        out << ";" << endl;
}

AbsExtension::AbsExtension()
{

}

AbsExtension::~AbsExtension()
{

}


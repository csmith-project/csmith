// -*- mode: C++ -*-
//
// Copyright (c) 2008, 2009, 2010, 2011, 2017 The University of Utah
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

#include "CrestExtension.h"
#include <cassert>
#include "Type.h"
#include "ExtensionValue.h"

using namespace std;

std::string CrestExtension::input_base_name_ = "CREST_";

CrestExtension::CrestExtension()
{

}

CrestExtension::~CrestExtension()
{
	values_.clear();
}

void
CrestExtension::GenerateValues()
{
	// Nothing to do
}

std::string
CrestExtension::type_to_string(const Type *t)
{
	assert(t->eType == eSimple);
	switch (t->simple_type) {
	case eChar: return "char";
	case eUChar: return "unsigned_char";
	case eShort: return "short";
	case eUShort: return "unsigned_short";
	case eInt: return "int";
	case eUInt: return "unsigned_int";
	case eLong: return "int";
	case eULong: return "unsigned_int";
	default: assert(0); break;
	}
	assert(0);
	return "";
}

void
CrestExtension::output_symbolics(ostream &out)
{
	std::vector<ExtensionValue*>::iterator i;
	for (i = values_.begin(); i != values_.end(); ++i) {
		assert(*i);
		out << AbsExtension::tab_;
		out << CrestExtension::input_base_name_ << type_to_string((*i)->get_type());
		out << "(" << (*i)->get_name() << ");" << std::endl;
	}
}

void
CrestExtension::OutputInit(std::ostream &out)
{
	out << "int main(void)" << endl;
	out << "{" << endl;
	AbsExtension::default_output_definitions(out, values_, false);
	output_symbolics(out);
}

void
CrestExtension::OutputHeader(std::ostream &out)
{
	out << "#include \"crest.h\"" << endl;
}

void
CrestExtension::OutputTail(std::ostream &out)
{
	out << AbsExtension::tab_ << "return 0;" << endl;
}

void
CrestExtension::OutputFirstFunInvocation(std::ostream &out, FunctionInvocation *invoke)
{
	AbsExtension::OutputFirstFunInvocation(out, invoke);
}


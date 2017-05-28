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

#include "SplatExtension.h"
#include <cassert>
#include "Type.h"
#include "Constant.h"
#include "ExtensionValue.h"

using namespace std;

SplatExtension::SplatExtension()
{

}

SplatExtension::~SplatExtension()
{
	std::vector<ExtensionValue*>::iterator i;
	for (i = values_.begin(); i != values_.end(); ++i) {
		delete (*i);
	}
	values_.clear();
}

void
SplatExtension::GenerateValues()
{
	vector<ExtensionValue *>::iterator i;
	for (i = values_.begin(); i != values_.end(); ++i) {
		const Type *type = (*i)->get_type();
		Constant *c =  Constant::make_random(type);
		(*i)->set_value(c);
	}
}

void
SplatExtension::output_symbolics(ostream &out)
{
	std::vector<ExtensionValue*>::iterator i;
	out << "#ifdef __SPLAT__" << endl;
	for (i = values_.begin(); i != values_.end(); ++i) {
		assert(*i);
		out << AbsExtension::tab_;
		int bits = (*i)->get_type()->SizeInBytes() * 8;
		out << "setInput(&" << (*i)->get_name() << ", " << bits << ", ";
		Constant *value = (*i)->get_value();
		assert(value);
		value->Output(out);
		out << ");" << endl;
	}
	out << "#endif" << endl << endl;
}

void
SplatExtension::OutputHeader(std::ostream &out)
{
	out << "#ifdef __SPLAT__" << endl;
	out << "#include \"../includes/instrument.h\"" << endl;
	out << "#endif" << endl;
}

void
SplatExtension::OutputTail(std::ostream &out)
{
	out << "#ifdef __SPLAT__" << endl;
	out << AbsExtension::tab_ << "__splat_exit(ExitNoBug);" << endl;
	out << "#else" << endl;
	out << AbsExtension::tab_ << "return 0;" << endl;
	out << "#endif" << endl;
}

void
SplatExtension::OutputInit(std::ostream &out)
{
	out << "#ifdef __SPLAT__" << endl;
	out << "void __splat_instrumentedCode()" << endl;
	out << "#else" << endl;
	out << "int main(void)" << endl;
	out << "#endif" << endl;
	out << "{" << endl;
	AbsExtension::default_output_definitions(out, values_, true);
	output_symbolics(out);
}

void
SplatExtension::OutputFirstFunInvocation(std::ostream &out, FunctionInvocation *invoke)
{
	AbsExtension::OutputFirstFunInvocation(out, invoke);
}


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

#include "ExtensionMgr.h"
#include <vector>
#include <cassert>
#include "CGOptions.h"
#include "SplatExtension.h"
#include "KleeExtension.h"
#include "CrestExtension.h"
#include "CoverageTestExtension.h"
#include "ExtensionValue.h"
#include "Function.h"
#include "FunctionInvocation.h"
#include "CGContext.h"

using namespace std;

AbsExtension *ExtensionMgr::extension_ = NULL;

void
ExtensionMgr::CreateExtension()
{
	int params_size = CGOptions::func1_max_params();
	if (CGOptions::splat()) {
		extension_ = dynamic_cast<AbsExtension*>(new SplatExtension());
	}
	else if (CGOptions::klee()) {
		extension_ = dynamic_cast<AbsExtension*>(new KleeExtension());
	}
	else if (CGOptions::crest()) {
		extension_ = dynamic_cast<AbsExtension*>(new CrestExtension());
	}
	else if (CGOptions::coverage_test()) {
		extension_ = dynamic_cast<AbsExtension*>(new CoverageTestExtension(CGOptions::coverage_test_size()));
	}
	else {
		return;
	}
	assert(extension_);
	std::vector<ExtensionValue *> &values = ExtensionMgr::extension_->get_values();
	ExtensionMgr::extension_->Initialize(params_size, values);
}

void
ExtensionMgr::DestroyExtension()
{
	delete ExtensionMgr::extension_;
	ExtensionMgr::extension_ = NULL;
}

AbsExtension *
ExtensionMgr::GetExtension()
{
	assert(ExtensionMgr::extension_);
	return ExtensionMgr::extension_;
}

void
ExtensionMgr::GenerateFirstParameterList(Function &curFunc)
{
	if (ExtensionMgr::extension_ == NULL)
		return;
	std::vector<ExtensionValue *> &values = ExtensionMgr::extension_->get_values();
	ExtensionMgr::extension_->GenerateFirstParameterList(curFunc, values);
}

void
ExtensionMgr::GenerateValues()
{
	if (ExtensionMgr::extension_ == NULL)
		return;
	ExtensionMgr::extension_->GenerateValues();
}

FunctionInvocation *
ExtensionMgr::MakeFuncInvocation(Function *curFunc, CGContext &cg_context)
{
	if (ExtensionMgr::extension_ == NULL) {
		return FunctionInvocation::make_random(curFunc, cg_context);
	}
	else {
		std::vector<ExtensionValue *> &values = ExtensionMgr::extension_->get_values();
		return ExtensionMgr::extension_->MakeFuncInvocation(curFunc, values);
	}
}

void
ExtensionMgr::OutputHeader(std::ostream &out)
{
	if (ExtensionMgr::extension_ == NULL) {
		return;
	}
	else {
		ExtensionMgr::extension_->OutputHeader(out);
	}
}

void
ExtensionMgr::OutputTail(ostream &out)
{
	if (ExtensionMgr::extension_ == NULL) {
		out << "    return 0;" << endl;
	}
	else {
		ExtensionMgr::extension_->OutputTail(out);
	}
}

void
ExtensionMgr::OutputInit(std::ostream &out)
{
	if (ExtensionMgr::extension_ == NULL) {
		if (CGOptions::accept_argc()) {
			out << "int main (int argc, char* argv[])" << endl;
		}
		else {
			out << "int main (void)" << endl;
		}
		out << "{" << endl;
	}
	else {
		ExtensionMgr::extension_->OutputInit(out);
	}
}

void
ExtensionMgr::OutputFirstFunInvocation(std::ostream &out, FunctionInvocation *invoke)
{
	assert(invoke);
	if (ExtensionMgr::extension_ == NULL) {
        	out << "    ";
        	invoke->Output(out);
        	out << ";" << endl;
	}
	else {
		ExtensionMgr::extension_->OutputFirstFunInvocation(out, invoke);
	}
}

////////////////////////////////////////////////////////////////////////////

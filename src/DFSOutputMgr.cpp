// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2015, 2017 The University of Utah
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

#include "DFSOutputMgr.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include "CGOptions.h"
#include "Function.h"
#include "Type.h"
#include "VariableSelector.h"

using namespace std;

DFSOutputMgr *DFSOutputMgr::instance_ = NULL;

DFSOutputMgr::DFSOutputMgr()
{

}

DFSOutputMgr::~DFSOutputMgr()
{

}

DFSOutputMgr *
DFSOutputMgr::CreateInstance()
{
	if (DFSOutputMgr::instance_)
		return DFSOutputMgr::instance_;

	DFSOutputMgr::instance_ = new DFSOutputMgr();
	std::string s_output = CGOptions::struct_output();
	if (s_output.empty())
		instance_->struct_output_ = DEFAULT_STRUCT_OUTPUT;
	else
		instance_->struct_output_ = s_output;

	assert(DFSOutputMgr::instance_);
	return DFSOutputMgr::instance_;
}

void
DFSOutputMgr::OutputHeader(int argc, char *argv[], unsigned long seed)
{
	if (!CGOptions::compact_output())
		OutputMgr::OutputHeader(argc, argv, seed);
}

void
DFSOutputMgr::OutputStructUnions(ostream& /*out*/)
{
	ofstream o_struct(struct_output_.c_str());
	OutputStructUnionDeclarations(o_struct);
	o_struct.close();
}

void
DFSOutputMgr::Output()
{
	std::ostream &out = get_main_out();

	OutputGlobalVariables(out);
	if (!CGOptions::compact_output())
		OutputForwardDeclarations(out);
	OutputFunctions(out);

	if (CGOptions::step_hash_by_stmt()) {
		OutputMgr::OutputHashFuncDef(out);
		OutputMgr::OutputStepHashFuncDef(out);
	}

	if (!CGOptions::compact_output())
		OutputMain(out);
}

std::ostream &
DFSOutputMgr::get_main_out()
{
	return std::cout;
}

void
DFSOutputMgr::outputln(ostream &out)
{
	if (!CGOptions::compact_output())
		out << std::endl;
}

void
DFSOutputMgr::output_comment_line(ostream &out, const std::string &comment)
{
	if (!CGOptions::compact_output())
		OutputMgr::output_comment_line(out, comment);
}

void
DFSOutputMgr::output_tab(ostream &out, int indent)
{
	if (!CGOptions::compact_output())
		OutputMgr::output_tab(out, indent);
}


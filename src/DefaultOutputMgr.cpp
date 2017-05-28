// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2015, 2017 The University of Utah
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

#include "DefaultOutputMgr.h"

#include <cassert>
#include <sstream>
#include "Common.h"
#include "CGOptions.h"
#include "platform.h"
#include "Bookkeeper.h"
#include "Function.h"
#include "FunctionInvocation.h"
#include "CGContext.h"
#include "VariableSelector.h"
#include "Type.h"
#include "random.h"
#include "DeltaMonitor.h"
#include "Error.h"

static std::string filename_prefix = "rnd_output";

static std::string global_header = "rnd_globals";

#ifndef WIN32
static std::string dir_sep = "/";
#else
static std::string dir_sep = "\\";
#endif

using namespace std;

DefaultOutputMgr *DefaultOutputMgr::instance_ = NULL;

DefaultOutputMgr *
DefaultOutputMgr::CreateInstance()
{
	if (DefaultOutputMgr::instance_)
		return DefaultOutputMgr::instance_;

	std::string ofile_str = CGOptions::output_file();
	if (!ofile_str.empty()) {
		ofstream *ofile = new ofstream(ofile_str.c_str());
		DefaultOutputMgr::instance_ = new DefaultOutputMgr(ofile);
	}
	else {
		DefaultOutputMgr::instance_ = new DefaultOutputMgr();
	}
	// DefaultOutputMgr::instance_ = new DefaultOutputMgr();
	assert(DefaultOutputMgr::instance_);
	DefaultOutputMgr::instance_->init();
	return DefaultOutputMgr::instance_;
}

ofstream *
DefaultOutputMgr::open_one_output_file(int num)
{
	std::ostringstream ss;
	ss << CGOptions::split_files_dir() << dir_sep << filename_prefix << num << ".c";
	ofstream *ofile = new ofstream(ss.str().c_str());
	return ofile;
}

void
DefaultOutputMgr::init()
{
	int max_files = CGOptions::max_split_files();

	if (max_files == 0)
		return;

	for (int i = 0; i < max_files; ++i) {
		ofstream *out = open_one_output_file(i);
		outs.push_back(out);
	}
}

bool
DefaultOutputMgr::create_output_dir(std::string dir)
{
	return create_dir(dir.c_str());
}

void
DefaultOutputMgr::OutputGlobals()
{
	std::ostringstream ss;
	ss << CGOptions::split_files_dir() << dir_sep << global_header << ".h";

	ofstream ofile(ss.str().c_str());
	ofile << "#ifndef RND_GLOBALS_H" << std::endl;
	ofile << "#define RND_GLOBALS_H" << std::endl;
	ofile << "#include \"safe_math.h\"" << std::endl;

	string prefix = "extern ";
	OutputGlobalVariablesDecls(ofile, prefix);

	OutputStructUnionDeclarations(ofile);
	ofile << "#endif" << std::endl;
	ofile.close();
}

void
DefaultOutputMgr::OutputAllHeaders()
{
	for (unsigned i = 1; i < outs.size(); ++i) {
		ofstream *out = outs[i];
		*out << "#include <stdint.h>" << std::endl;

		if (CGOptions::paranoid())
			*out << "#include <assert.h>" << std::endl;

		*out << "#include <limits.h>" << std::endl;
		*out << "#include \"" << global_header << ".h\"" << std::endl;
		*out << std::endl;
	}

	*outs[0] << "#include \"" << global_header << ".h\"" << std::endl;

	vector<ofstream *>::iterator j;
	for (j = outs.begin(); j != outs.end(); ++j) {
		ofstream *out = (*j);
		string prefix = "extern ";
		// OutputGlobalVariablesDecls(*out, prefix);
		OutputForwardDeclarations(*out);
		*out << std::endl;
	}
}

void
DefaultOutputMgr::RandomOutputVarDefs()
{
	vector<Variable*> *globals = VariableSelector::GetGlobalVariables();
	size_t size = outs.size();
	vector<Variable*>::iterator i;
	for (i = globals->begin(); i != globals->end(); ++i) {
		int index = pure_rnd_upto(size);
		ostream *out = outs[index];
		(*i)->OutputDef(*out, 0);
	}
}

void
DefaultOutputMgr::RandomOutputFuncDefs()
{
	const vector<Function*> &funcs = get_all_functions();
	size_t size = outs.size();
	vector<Function*>::const_iterator i;
	for (i = funcs.begin(); i != funcs.end(); ++i) {
		if ((*i)->is_builtin)
			continue;
		int index = pure_rnd_upto(size);
		(*i)->Output(*outs[index]);
	}
}

void
DefaultOutputMgr::RandomOutputDefs()
{
	RandomOutputVarDefs();
	RandomOutputFuncDefs();
}

void
DefaultOutputMgr::OutputHeader(int argc, char *argv[], unsigned long seed)
{
	OutputMgr::OutputHeader(argc, argv, seed);
}

void
DefaultOutputMgr::Output()
{
	std::ostream &out = get_main_out();
	if (DeltaMonitor::is_running() && (Error::get_error() != SUCCESS)) {
		out << "Delta reduction error!\n";
	}
	if (is_split()) {
		OutputGlobals();
		OutputAllHeaders();
		RandomOutputDefs();
	}
	else {
		OutputStructUnionDeclarations(out);
		OutputGlobalVariables(out);
		OutputForwardDeclarations(out);
		OutputFunctions(out);
	}
	if (CGOptions::step_hash_by_stmt()) {
		OutputMgr::OutputHashFuncDef(out);
		OutputMgr::OutputStepHashFuncDef(out);
	}

	if (!CGOptions::nomain())
		OutputMain(out);
	OutputTail(out);
	DeltaMonitor::Output(out);
}

std::ostream &
DefaultOutputMgr::get_main_out()
{
	if (is_split())
		return *(outs[0]);
	else if (ofile_) {
		return *ofile_;
	}
	else {
		return std::cout;
	}
}

bool
DefaultOutputMgr::is_split()
{
	return (CGOptions::max_split_files() > 0);
}

void
DefaultOutputMgr::outputln(ostream &out)
{
	out << std::endl;
}

void
DefaultOutputMgr::output_comment_line(ostream &out, const std::string &comment)
{
	OutputMgr::output_comment_line(out, comment);
}

void
DefaultOutputMgr::output_tab(ostream &out, int indent)
{
	OutputMgr::output_tab(out, indent);
}

//////////////////////////////////////////////////////////////////
DefaultOutputMgr::DefaultOutputMgr(std::ofstream *ofile)
	: ofile_(ofile)
{

}

DefaultOutputMgr::DefaultOutputMgr()
	: ofile_(NULL)
{

}

DefaultOutputMgr::~DefaultOutputMgr()
{
	std::vector<std::ofstream *>::iterator out;
	for (out = outs.begin(); out != outs.end(); ++out) {
		(*out)->close();
		delete (*out);
	}
	outs.clear();
	if (ofile_)
		ofile_->close();
	delete ofile_;
}


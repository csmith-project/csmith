// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013 The University of Utah
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

#ifndef OUTPUT_MGR_H
#define OUTPUT_MGR_H

#include <ostream>
#include <vector>
#include <string>
using namespace std;

class Variable;

#define TAB "    "    // to beautify output: 1 tab is 4 spaces

class OutputMgr {
public:
	OutputMgr();

	virtual ~OutputMgr() = 0;

	static void OutputHashFuncDef(std::ostream &out);

	static void OutputHashFuncDecl(std::ostream &out);

	static void OutputHashFuncInvocation(std::ostream &out, int indent);

	static void OutputStepHashFuncInvocation(std::ostream &out, int indent, int stmt_id);

	static void OutputStepHashFuncDecl(std::ostream &out);

	static void OutputStepHashFuncDef(std::ostream &out);

	static void really_outputln(std::ostream &out);

	static void set_curr_func(const std::string &fname);

	virtual void OutputHeader(int argc, char *argv[], unsigned long seed) = 0;

	virtual void OutputStructUnions(ostream& /* out */) {};

	virtual void Output() = 0;

	virtual void outputln(ostream &out) {out << std::endl;}

	virtual void output_comment_line(ostream &out, const std::string &comment);

	virtual void output_tab(ostream &out, int indent);

	void OutputPtrResets(ostream &out, const vector<const Variable*>& ptrs);

	static const char *hash_func_name;

	static const char *step_hash_func_name;

	static vector<std::string> monitored_funcs_;

protected:
	virtual std::ostream &get_main_out() = 0;

	static void output_tab_(ostream &out, int indent);

	void OutputTail(std::ostream &out);

	void OutputMain(std::ostream &out);

private:

	static bool is_monitored_func(void);

	static std::string curr_func_;

};

#endif // OUTPUT_MGR_H

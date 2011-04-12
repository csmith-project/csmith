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

#ifndef DEFAULT_OUTPUT_MGR_H
#define DEFAULT_OUTPUT_MGR_H

#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include "OutputMgr.h"

using namespace std;

class DefaultOutputMgr : public OutputMgr {
public:
	static DefaultOutputMgr *CreateInstance();

	virtual ~DefaultOutputMgr();

	static bool create_output_dir(std::string dir);

	virtual void OutputHeader(int argc, char *argv[], unsigned long seed);

	virtual void Output();

	virtual void outputln(ostream &out);

	virtual void output_comment_line(ostream &out, const std::string &comment);

	virtual void output_tab(ostream &out, int indent);

private:
	explicit DefaultOutputMgr(std::ofstream *ofile);

	DefaultOutputMgr();

	virtual std::ostream &get_main_out();

	bool is_split();

	std::ofstream* open_one_output_file(int num);

	void init();

	void OutputGlobals();

	void OutputAllHeaders();

	void RandomOutputDefs();

	void RandomOutputVarDefs();

	void RandomOutputFuncDefs();

	static DefaultOutputMgr *instance_;

	std::vector<std::ofstream* > outs;

	std::ofstream *ofile_;
};

#endif // DEFAULT_OUTPUT_MGR_H

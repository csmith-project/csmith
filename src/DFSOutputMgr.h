// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2015 The University of Utah
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

#ifndef DFS_OUTPUT_MGR_H
#define DFS_OUTPUT_MGR_H

#include "OutputMgr.h"

#define DEFAULT_STRUCT_OUTPUT "csmith_structs.h"

class DFSOutputMgr : public OutputMgr {
public:
	static DFSOutputMgr *CreateInstance();

	virtual ~DFSOutputMgr();

	virtual void OutputHeader(int argc, char *argv[], unsigned long seed);

	virtual void OutputStructUnions(ostream& /*out*/);

	virtual void Output();

	virtual void outputln(ostream &out);

	virtual void output_comment_line(ostream &out, const std::string &comment);

	virtual void output_tab(ostream &out, int indent);

private:
	DFSOutputMgr();

	virtual std::ostream &get_main_out();

	static DFSOutputMgr *instance_;

	std::string struct_output_;
};

#endif // DFS_OUTPUT_MGR_H

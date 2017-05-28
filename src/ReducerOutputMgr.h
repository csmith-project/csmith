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

#ifndef REDUCER_OUTPUT_MGR_H
#define REDUCER_OUTPUT_MGR_H

#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include "OutputMgr.h"
#include "util.h"

class Reducer;
class FactMgr;
class Block;
class Function;
class Statement;
class StatementIf;
class Expression;
class FunctionInvocation;
class FunctionInvocationBinary;

using namespace std;

class ReducerOutputMgr : public OutputMgr {
public:
	ReducerOutputMgr();
	virtual ~ReducerOutputMgr();
	virtual void OutputHeader(int argc, char *argv[], unsigned long seed);
	virtual void Output();

	virtual std::ostream &get_main_out();

	void rewrite_func_call(const Statement* stm, const FunctionInvocation* invoke, string id, std::ostream& out, int indent);
	int rewrite_func_calls(const Statement* stm, std::ostream &out, int indent);
	void output_alt_exprs(const Statement* stm, std::ostream &out, int indent);

	void output_write_var_values(string title, const Statement* stm, std::ostream &out, FactMgr* fm, int indent, bool cover_block_writes=false);
	void output_memory_addrs(const Statement* stm, std::ostream& out, int indent);
	void output_global_values(string header, std::ostream& out, int indent);
	void output_tail(ostream& out);
	void output_crc_lines(std::ostream& out);

	void output_vars(const vector<Variable*> &vars, std::ostream &out, int indent);
	void output_var(const Variable* v, std::ostream &out, int indent);
	int output_func_header(const Function* f, std::ostream& out);
	int output_main_func(std::ostream& out);
	int output_func(const Function* f, std::ostream& out);
	int output_funcs(std::ostream& out);
	int output_block(const Block* blk, std::ostream& out, int indent, bool no_bracelet=false);
	void output_if_stm(const StatementIf* si, std::ostream &out, int indent);
	void output_reduced_stm(const Statement* stm, std::ostream &out, int indent);
	void output_stm(const Statement* stm, std::ostream &out, FactMgr* fm, int indent);
	void output_pre_stm_assigns(const Statement* stm, std::ostream &out, int indent);
	void output_block_entry_msg(const Block* blk, std::ostream &out, int indent);
	void output_pre_stm_values(const Statement* stm, std::ostream &out, FactMgr* fm, int indent);
	void output_post_stm_values(const Statement* stm, std::ostream &out, FactMgr* fm, int indent);
	void output_global_state_for_func(const Function* f, std::ostream &out, int indent);
	void output_artificial_globals(ostream& out);

	virtual void OutputStructUnions(ostream& out);
	void OutputGlobals(ostream& out);

private:
	void OutputGlobals();
	void limit_binarys(vector<const FunctionInvocationBinary*>& binarys, vector<int>& ids);
	std::ofstream *ofile_;
	Reducer* reducer;
};

#endif // REDUCER_OUTPUT_MGR_H

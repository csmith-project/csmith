// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2015, 2017 The University of Utah
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

// util.cpp --- various utility functions

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util.h"

#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include "OutputMgr.h"
#include "AbsProgramGenerator.h"
#include "CGOptions.h"

using namespace std;
///////////////////////////////////////////////////////////////////////////////

static int gensym_count = 0;

void
reset_gensym()
{
	gensym_count = 0;
}

/*
 *
 */
string
gensym(const char* basename)
{
	ostringstream ss; //(basename, ios_base::ate); somehow this yields weird result on windows
	ss << basename;
	ss << ++gensym_count;
	return ss.str();
}

/*
 *
 */
string
gensym(const string& basename)
{
	ostringstream ss; //(basename, ios_base::ate);
	ss << basename;
	ss << ++gensym_count;
	return ss.str();
}

/*
 * permute an integer array
 * return: all possible permutations of input integer array
 */
vector<intvec> permute(intvec in)
{
	vector<intvec> out;
	/* basic cases */
	if (in.size()==0) return out;
	if (in.size() == 1) {
		out.push_back(in);
		return out;
	}
	/* extended cases: divide the array into head and tail, permute
	   tail, then insert head into all possible positions in tail */
	int head = in[0];
	intvec tail = in;
	tail.erase(tail.begin());
	vector<intvec> tails = permute(tail);
	size_t i, j;
	for (i=0; i<tails.size(); i++) {
		intvec one_tail = tails[i];
		for (j=0; j<=one_tail.size(); j++) {
			intvec tmp = one_tail;
			tmp.insert(tmp.begin()+j, head);
			out.push_back(tmp);
		}
	}
	return out;
}

/*
 * given a multi-dimensional array sizes, expand it into all possible array indices
 * for example: a (2, 2, 2) array will be expanded into the following array of arrays
 *     (0, 0, 0), (0, 0, 1), (0, 1, 0), (0, 1, 1),
 *     (1, 0, 0), (1, 0, 1), (1, 1, 0), (1, 1, 1)
 * return: the size of the expanded array of arrays
 */
int expand_within_ranges(vector<unsigned int> in, vector<intvec>& out)
{
	int i, j;
	int dimension = static_cast<int>(in.size());
	intvec limits(dimension);
	// figure out the maximum number the remaining indices can represent
	// like in decimal, the max for each digit starting from right is 10, 100, 1000...
	limits[dimension - 1] = in[dimension - 1];
	for (i = dimension - 2; i>=0; i--) {
		limits[i] = limits[i+1] * in[i];
	}
	out.clear();
	// limit[0] is the maximum number can be represented by all dimensions combined
	for (i=0; i<limits[0]; i++) {
		intvec tmp;
		int num = i;
		// calculate the index for each dimension
		for (j=0; j<dimension-1; j++) {
			tmp.push_back(num / limits[j+1]);
			num = num % limits[j+1];
		}
		tmp.push_back(num);
		out.push_back(tmp);
	}
	return out.size();
}

void really_outputln(std::ostream &out)
{
	OutputMgr::really_outputln(out);
}

void outputln(std::ostream &out)
{
	OutputMgr *output = AbsProgramGenerator::GetOutputMgr();
	assert(output);
	output->outputln(out);
}

void output_print_str(std::ostream& out, std::string str, std::string str_value, int indent)
{
	output_tab(out, indent);
	out << "printf(\"";
	out << str;
	out << "\"";
	if (!str_value.empty()) {
		out << ", ";
		out << str_value;
	}
	out << ");";
}

void output_open_encloser(const char* symbol, std::ostream &out, int& indent)
{
	output_tab(out, indent);
	out << symbol;
	outputln(out);
	indent++;
}

void output_close_encloser(const char* symbol, std::ostream &out, int& indent, bool no_newline)
{
	if (!no_newline) {
		outputln(out);
	}
	indent--;
	output_tab(out, indent);
	out << symbol;
}

void output_comment_line(std::ostream &out, const std::string &comment)
{
	OutputMgr *output = AbsProgramGenerator::GetOutputMgr();
	assert(output);
	output->output_comment_line(out, comment);
}

void output_tab(std::ostream &out, int indent)
{
	OutputMgr *output = AbsProgramGenerator::GetOutputMgr();
	assert(output);
	output->output_tab(out, indent);
}
///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

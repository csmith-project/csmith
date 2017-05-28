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

#ifndef DFS_RNDNUM_GENERATOR_H
#define DFS_RNDNUM_GENERATOR_H

#include <vector>
#include <string>

#include "Common.h"
#include "CommonMacros.h"
#include "AbsRndNumGenerator.h"

class Sequence;
class Filter;

class DFSRndNumGenerator : public AbsRndNumGenerator
{
public:
	virtual ~DFSRndNumGenerator();

	static DFSRndNumGenerator *make_rndnum_generator();

	virtual std::string get_prefixed_name(const std::string &name);

	virtual std::string& trace_depth();

	virtual void get_sequence(std::string &sequence);

	virtual unsigned int rnd_upto(const unsigned int n, const Filter *f = NULL, const std::string *where = NULL);

	virtual bool rnd_flipcoin(const unsigned int p, const Filter *f = NULL, const std::string *where = NULL);

	virtual std::string RandomHexDigits( int num );

	virtual std::string RandomDigits( int num );

	virtual enum RNDNUM_GENERATOR kind() { return rDFSRndNumGenerator; }

	bool eager_backtracking(int depth_needed);

	int get_decision_depth() { return decision_depth_; }

	void reset_state(void);

	int get_current_pos(void) { return current_pos_; }

	void set_current_pos(int pos) { current_pos_ = pos; }

	bool get_all_done(void) { return all_done_; }

private:
	// Forward declaration of nested class SearchState;
	class SearchState;

	// ------------------------------------------------------------------------------------------
	DFSRndNumGenerator(Sequence *concrete_seq);

	int revisit_node(SearchState *state, int local_current_pos,
						int bound, const Filter *f, const string *where);

	void initialize_sequence(const std::vector<int> &v);

	void dumpCurrentState(int bound, const std::string &where);

	virtual unsigned long genrand(void);

	bool filter_invalid_nums(vector<int> *invalid_nums, int v);

	int random_choice(int bound, const Filter *f = NULL, const std::string *where = NULL, std::vector<int> *invalid_nums = NULL);

	void init_states(int size);

	void log_depth(int d, const std::string *where = NULL, const char *log = NULL);

	// ----------------------------------------------------------------------------------------
	static DFSRndNumGenerator *impl_;

	//static std::string name_prefix;

	std::string trace_string_;

	// The current decision depth which should be less than SEARCH_STATE_SIZE
	// It represents the current depth where we are making random choices
	int decision_depth_;

	// The current position at states.
	int current_pos_;

	bool all_done_;

	Sequence *seq_;

	bool use_debug_sequence_;

	// Holds the vector representation of all DFS nodes.
	std::vector<SearchState*> states_;

	// disallow copy and assignment constructors
	DISALLOW_COPY_AND_ASSIGN(DFSRndNumGenerator);
};

#endif //DFS_RNDNUM_GENERATOR_H

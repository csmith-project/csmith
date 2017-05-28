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

#include "SimpleDeltaSequence.h"

#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include "SequenceLineParser.h"
#include "DeltaMonitor.h"
#include "CGOptions.h"

using namespace std;

// Represent the <value, bound> pair
class SimpleDeltaSequence::ValuePair
{
public:
	ValuePair(int value, int bound);

	~ValuePair();

	int get_value() const { return value_; }

	int get_bound() const { return bound_; }

	void set_value(int value) { value_ = value; }

	void set_bound(int bound) { bound_ = bound; }

	void set_value_bound(int value, int bound) {
		value_ = value;
		bound_ = bound;
	}

private:
	int value_;

	int bound_;
};

SimpleDeltaSequence::ValuePair::ValuePair(int value, int bound)
	: value_(value),
	  bound_(bound)
{

}

SimpleDeltaSequence::ValuePair::~ValuePair( )
{

}

///////////////////////////////////////////////////////////////////
const char SimpleDeltaSequence::default_sep_char = ',';

SimpleDeltaSequence *SimpleDeltaSequence::impl_ = NULL;

SimpleDeltaSequence::SimpleDeltaSequence(const char sep_char)
	: sep_char_(sep_char),
	  current_pos_(0)
{
	// Nothing to do
}

SimpleDeltaSequence::~SimpleDeltaSequence()
{
	// Nothing to do
}

/*
 * Create singleton instance.
 */
SimpleDeltaSequence*
SimpleDeltaSequence::CreateInstance(const char sep_char)
{
	if (impl_)
		return impl_;

	impl_ = new SimpleDeltaSequence(sep_char);
	assert(impl_);

	return impl_;
}

/*
 *
 */
bool
SimpleDeltaSequence::empty_line(const std::string &line)
{
	if (line.empty())
		return true;
	size_t found = line.find_first_not_of("\t\n ");
	return (found == string::npos);
}

void
SimpleDeltaSequence::init_sequence()
{
	const std::string &fname = DeltaMonitor::get_input();
	assert(!fname.empty());

	std::string line;
	ifstream seqf(fname.c_str());
	assert("fail to open simple delta input file!" && seqf.is_open());

	int i = 0;
	while (!seqf.eof()) {
		getline(seqf, line);
		if (empty_line(line))
			continue;
		std::vector<int> v;
		if(!SequenceLineParser<std::vector<int> >::parse_sequence(v, line, sep_char_))
			assert("bad simple delta input sequence!" && 0);
		assert(v.size() == 2);
		SimpleDeltaSequence::ValuePair *p = new SimpleDeltaSequence::ValuePair(v[0], v[1]);
		sequence_[i] = p;
		i++;
	}
	seqf.close();
}

void
SimpleDeltaSequence::add_number(int v, int bound, int k)
{
	SimpleDeltaSequence::ValuePair *p = new SimpleDeltaSequence::ValuePair(v, bound);
	seq_map_[k] = p;
}

int
SimpleDeltaSequence::get_number_by_pos(int /*pos*/)
{
	assert(0);
	return 0;
}

int
SimpleDeltaSequence::get_number(int bound)
{

	SimpleDeltaSequence::ValuePair *p = sequence_[current_pos_];
	assert("SimpleDeltaSequence: get_number p is NULL!" && p);
	int b = p->get_bound();
	assert("SimpleDeltaSequence: bound doesn't match!" && bound == b);
	seq_map_[current_pos_] = p;
	++current_pos_;
	return p->get_value();
}

void
SimpleDeltaSequence::clear()
{
	std::map<int, SimpleDeltaSequence::ValuePair*>::iterator i;
	for (i = seq_map_.begin(); i != seq_map_.end(); ++i) {
		assert((*i).second);
		delete ((*i).second);
	}
	seq_map_.clear();
}

void SimpleDeltaSequence::output_one(ostream &out,
					const SimpleDeltaSequence::ValuePair *p)
{
	assert(p);
	int value = p->get_value();
	int bound = p->get_bound();
	out << value << sep_char_ << bound;
	out << std::endl;
}

void
SimpleDeltaSequence::get_sequence(ostream &out)
{
	assert(!seq_map_.empty());

	std::map<int, SimpleDeltaSequence::ValuePair*>::iterator i;
	for (i = seq_map_.begin(); i != seq_map_.end(); ++i) {
		output_one(out, (*i).second);
	}
}

unsigned INT64
SimpleDeltaSequence::sequence_length()
{
	return sequence_.size();
}

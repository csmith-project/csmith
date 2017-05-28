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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "DefaultRndNumGenerator.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>

#include "Filter.h"
#include "SequenceFactory.h"
#include "Sequence.h"
#include "CGOptions.h"
#include "DeltaMonitor.h"

DefaultRndNumGenerator *DefaultRndNumGenerator::impl_ = 0;

/*
 *
 */
DefaultRndNumGenerator::DefaultRndNumGenerator(const unsigned long, Sequence *concrete_seq)
	: rand_depth_(0),
	  trace_string_(""),
	  seq_(concrete_seq)
{
	//Nothing to do
}

/*
 *
 */
DefaultRndNumGenerator::~DefaultRndNumGenerator()
{
	SequenceFactory::destroy_sequences();
}

/*
 * Create singleton instance.
 */
DefaultRndNumGenerator*
DefaultRndNumGenerator::make_rndnum_generator(const unsigned long seed)
{
	if (impl_)
		return impl_;

	Sequence *seq = SequenceFactory::make_sequence();

	impl_ = new DefaultRndNumGenerator(seed, seq);
	assert(impl_);
	//impl_->seedrand(seed);

	return impl_;
}

/*
 * Return the sequence of random choices
 */
void
DefaultRndNumGenerator::get_sequence(std::string &sequence)
{
	std::ostringstream ss;
	seq_->get_sequence(ss);
	sequence = ss.str();
}

std::string
DefaultRndNumGenerator::get_prefixed_name(const std::string &name)
{
	return name;
}

void
DefaultRndNumGenerator::add_number(int v, int bound, int k)
{
	if (DeltaMonitor::is_running())
		seq_->add_number(v, bound, k);
}

/*
 * Return a random number in the range 0..(n-1).
 */
unsigned int
DefaultRndNumGenerator::rnd_upto(const unsigned int n, const Filter *f, const std::string *where)
{
	static int g = 0;
	int h = g;
	if (h == 440)
		BREAK_NOP;   // for debugging
	unsigned int v = genrand() % n;
	unsigned INT64 local_depth = rand_depth_;
	rand_depth_++;
	//ofstream out("rnd.log", ios_base::app);
	//out << g++ << ": " << v << "(" << n << ")" << endl;

	if (f) {
		while (f->filter(v)) {
			// We could add numbers into sequence inside the previous filter.
			// If the previous filter failed, we need to roll back the rand_depth_ here.
			// This will also overwrite the value added in the map.
			rand_depth_ = local_depth+1;
			v = genrand() % n;
			/*out << g++ << ": " << v << "(" << n << ")" << endl;*/
		}
	}
	//out.close();
	if (where) {
	std::ostringstream ss;
		ss << *where << "->";
	trace_string_ += ss.str();
	}
	add_number(v, n, local_depth);
	return v;
}

/*
 * Return `true' p% of the time.
 */
bool
DefaultRndNumGenerator::rnd_flipcoin(const unsigned int p, const Filter *f, const std::string *)
{
	assert(p <= 100);
	unsigned INT64 local_depth = rand_depth_;
	rand_depth_++;
	if (f) {
		if (f->filter(0)) {
			add_number(1, 2, local_depth);
			return true;
		}
		else if (f->filter(1)) {
			add_number(0, 2, local_depth);
			return false;
		}
	}

	bool rv = (genrand() % 100) < p;
	if (rv) {
		add_number(1, 2, local_depth);
	}
	else {
		add_number(0, 2, local_depth);
	}
	return rv;
}

std::string &
DefaultRndNumGenerator::trace_depth()
{
	return trace_string_;
}

unsigned long
DefaultRndNumGenerator::genrand(void)
{
	return AbsRndNumGenerator::genrand();
}

std::string
DefaultRndNumGenerator::RandomHexDigits( int num )
{
	if (!CGOptions::is_random())
		return AbsRndNumGenerator::RandomHexDigits(num);

	std::string str;
	const char* hex1 = AbsRndNumGenerator::get_hex1();
	while (num--) {
		int x = genrand() % 16;
		str += hex1[x];
		seq_->add_number(x, 16, rand_depth_);
		rand_depth_++;
	}
	return str;
}

std::string
DefaultRndNumGenerator::RandomDigits( int num )
{
	if (!CGOptions::is_random())
		return AbsRndNumGenerator::RandomDigits(num);

	std::string str;
	const char* dec1 = AbsRndNumGenerator::get_dec1();
	while (num--) {
		int x = genrand() % 10;
		str += dec1[x];
		seq_->add_number(x, 10, rand_depth_);
		rand_depth_++;
	}
	return str;
}


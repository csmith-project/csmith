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

#include "LinearSequence.h"

#include <cassert>
#include <sstream>
#include <iostream>

#include "CGOptions.h"

using namespace std;

const char LinearSequence::default_sep_char = '_';
/*
	seq_map_ = map <int , int>
	 _______________
	|_int___|_int___|
	|	|	|
	|	|	|
	|_______|_______|
 */
LinearSequence::LinearSequence(const char sep_char)
	: sep_char_(sep_char)
{
	// Nothing to do
}

LinearSequence::~LinearSequence()
{
	// Nothing to do
}

/*
 *
 */
void
LinearSequence::init_sequence()
{

}
//adds value in map
// at key = k
// and value = v
void
LinearSequence::add_number(int v, int /*bound*/, int k)
{
	seq_map_[k] = v;
}

int
LinearSequence::get_number(int /*bound*/)
{
	return -1;
}
//get the value at pos
//key = pos
//return  = value at key
int
LinearSequence::get_number_by_pos(int pos)
{
	int rv = seq_map_[pos];
	assert(rv >= 0);
	return rv;
}

void
LinearSequence::clear()
{
	seq_map_.clear();
}
/*
	appends '_' for n-1 values of the map (seq_char_) and 
	at end appends the last value
	ex. step 1 --> 1_2_3_
	    step 2 --> 1_2_3_4 where 1,2,3,4 are the values in map
 */
void
LinearSequence::get_sequence(ostream &ss)
{
	assert(!seq_map_.empty());

	size_t i = 0;
	for (i = 0; i < seq_map_.size() - 1; ++i) {
		ss << seq_map_[i] << sep_char_;
	}
	ss << seq_map_[i];
}
//retrurns the number of elements in map seq_map_
unsigned INT64
LinearSequence::sequence_length()
{
	return seq_map_.size();
}

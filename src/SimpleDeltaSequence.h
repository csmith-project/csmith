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

#ifndef SIMPLE_DELTA_SEQUENCE_H
#define SIMPLE_DELTA_SEQUENCE_H

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include "Common.h"
#include "Sequence.h"

class SimpleDeltaSequence : public Sequence {
public:
	static SimpleDeltaSequence *CreateInstance(const char sep_char);

	virtual ~SimpleDeltaSequence();

	virtual void init_sequence();

	virtual unsigned INT64 sequence_length();

	virtual void add_number(int v, int bound, int k);

	virtual int get_number(int bound);

	virtual int get_number_by_pos(int pos);

	virtual void clear();

	virtual void get_sequence(std::ostream &);

	virtual char get_sep_char() const { return sep_char_; }

	static const char default_sep_char;

private:
	class ValuePair;

	explicit SimpleDeltaSequence(const char sep_char);

	bool empty_line(const std::string &line);

	void output_one(std::ostream &out, const ValuePair *p);

	std::map<int, ValuePair*> seq_map_;

	std::map<int, ValuePair*> sequence_;

	static SimpleDeltaSequence *impl_;

	const char sep_char_;

	unsigned INT64 current_pos_;
};

#endif // SIMPLE_DELTA_SEQUENCE_H

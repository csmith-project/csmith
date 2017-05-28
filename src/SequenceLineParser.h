// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2015 The University of Utah
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

#ifndef SEQUENCE_LINE_PARSER_H
#define SEQUENCE_LINE_PARSER_H

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

template <class Container>
class SequenceLineParser {
public:
	SequenceLineParser();

	~SequenceLineParser();

	static bool parse_sequence(Container &q, const std::string &seq, const char sep_char = ',');

private:
	static int str2int(const std::string &s);
};

template <class Container>
SequenceLineParser<Container>::SequenceLineParser()
{

}

template <class Container>
SequenceLineParser<Container>::~SequenceLineParser()
{

}

template <class Container> bool
SequenceLineParser<Container>::parse_sequence(Container &q, const std::string &seq, const char sep_char)
{
	size_t pos = 0;
	size_t start_pos = 0;

	//while (pos < static_cast<int>(seq.size())) {
	while(true) {
		while (seq[pos] == ' ')
			pos++;

		start_pos = pos;
		pos = seq.find_first_of(sep_char, pos);
		std::string s = seq.substr(start_pos, (pos - start_pos));
		int i = SequenceLineParser::str2int(s);
		if (i < 0)
			return false;
		q.push_back(i);
		if (pos == string::npos)
			break;
		pos++;
	}
	return true;
}

template <class Container> int
SequenceLineParser<Container>::str2int(const std::string &s)
{
	stringstream ss(s);
	int i = -1;
	ss >> i;

	return i;
}

#endif // SEQUENCE_PARSER_H

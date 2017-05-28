// -*- mode: C++ -*-
//
// Copyright (c) 2008, 2009, 2010, 2011, 2015, 2017 The University of Utah
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

#include "PartialExpander.h"
#include <cassert>
#include <iostream>

using namespace std;

std::map<eStatementType, bool> PartialExpander::expands_;

std::map<eStatementType, bool> PartialExpander::expands_backup_;

void
print_map(const map<eStatementType, bool> &m)
{
	std::map<eStatementType, bool>::const_iterator i;
	for (i = m.begin(); i != m.end(); ++i) {
		cout << "t = " << (*i).first << " , v = " << (*i).second << std::endl;
	}
}

void
PartialExpander::init_map(map<eStatementType, bool> &m, bool value)
{
	m[eAssign] = value;
	m[eBlock] = value;
	m[eFor] = value;
	m[eIfElse] = value;
	m[eInvoke] = value;
	m[eReturn] = value;
	m[MAX_STATEMENT_TYPE] = value;
}

bool
PartialExpander::init_partial_expander(const std::string &options)
{
	PartialExpander::init_map(expands_, false);
	if (!PartialExpander::parse_options(options, ','))
		return false;

	PartialExpander::expands_[MAX_STATEMENT_TYPE] = true;
	PartialExpander::copy_expands(PartialExpander::expands_backup_, PartialExpander::expands_);
	return true;
}

bool
PartialExpander::parse_options(const std::string &options, char sep_char)
{
	size_t pos = 0;
	size_t start_pos = 0;

	while(true) {
		// Shouldn't happen
		assert(options[pos] != ' ');

		start_pos = pos;
		pos = options.find_first_of(sep_char, pos);
		std::string s = options.substr(start_pos, (pos - start_pos));
		if (!PartialExpander::set_expand(s))
			return false;
		if (pos == string::npos)
			break;
		pos++;
	}
	return true;
}

void
PartialExpander::set_stmt_expand(eStatementType t, bool value)
{
	expands_[t] = value;
}

bool
PartialExpander::set_expand(const std::string s)
{

	if (!s.compare("assignment")) {
		PartialExpander::set_stmt_expand(eAssign, true);
	}
	else if (!s.compare("block")) {
		PartialExpander::set_stmt_expand(eBlock, true);
	}
	else if (!s.compare("for")) {
		PartialExpander::set_stmt_expand(eFor, true);
	}
	else if (!s.compare("if-else")) {
		PartialExpander::set_stmt_expand(eIfElse, true);
	}
	else if (!s.compare("invoke")) {
		PartialExpander::set_stmt_expand(eInvoke, true);
	}
	else if (!s.compare("return")) {
		PartialExpander::set_stmt_expand(eReturn, true);
	}
	else if (!s.compare("all")) {
		PartialExpander::init_map(expands_, true);
	}
	else {
		return false;
	}
	return true;
}

void
PartialExpander::copy_expands(std::map<eStatementType, bool> &dest, const map<eStatementType, bool> &src)
{
	std::map<eStatementType, bool>::const_iterator i;
	for (i = src.begin(); i != src.end(); ++i) {
		eStatementType t = (*i).first;
		dest[t] = (*i).second;
	}
}

void
PartialExpander::restore_init_values()
{
	PartialExpander::copy_expands(PartialExpander::expands_, PartialExpander::expands_backup_);
}

bool
PartialExpander::direct_expand_check(eStatementType t)
{
	//assert(expands_.find(t) != expands_.end());
	return expands_[t];
}

bool
PartialExpander::expand_check(eStatementType t)
{
	// If expand is false, then all values are valid
	if (!expands_[MAX_STATEMENT_TYPE])
		return true;

	assert(expands_.find(t) != expands_.end());

	bool rv = false;
	rv = expands_[t];

	if (t == eAssign) {
		rv = rv || expands_[eInvoke];
	}

	if (rv) {
		set_stmt_expand(MAX_STATEMENT_TYPE, false);
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////
PartialExpander::PartialExpander()
{

}

PartialExpander::~PartialExpander()
{

}



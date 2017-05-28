// -*- mode: C++ -*-
//
// Copyright (c) 2008, 2009, 2010, 2011, 2015 The University of Utah
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

#ifndef PARTIAL_EXPANDER_H
#define PARTIAL_EXPANDER_H

#include <string>
#include <map>
#include "Statement.h"

class PartialExpander {
public:
	static bool init_partial_expander(const std::string &options);

	static void restore_init_values();

	static bool expand_check(eStatementType t);

	static bool direct_expand_check(eStatementType t);

private:
	PartialExpander();

	~PartialExpander();

	static void init_map(map<eStatementType, bool> &m, bool value);

	static void set_stmt_expand(eStatementType t, bool value);

	static void copy_expands(std::map<eStatementType, bool> &dest, const map<eStatementType, bool> &src);

	static bool set_expand(const std::string s);

	static bool parse_options(const std::string &options, char sep_char);

	static std::map<eStatementType, bool> expands_;

	static std::map<eStatementType, bool> expands_backup_;
};

#endif

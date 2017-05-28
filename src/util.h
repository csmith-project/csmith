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

#ifndef UTIL_H
#define UTIL_H

///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <ostream>

typedef std::vector<int> intvec;

std::string gensym(const char *basename);
std::string gensym(const std::string &basename);
void reset_gensym();
std::vector<intvec> permute(intvec in);
int expand_within_ranges(std::vector<unsigned int>, std::vector<intvec>& out);
void outputln(std::ostream &out);
void really_outputln(std::ostream &out);
void output_comment_line(std::ostream &out, const std::string &comment);
void output_tab(std::ostream &out, int indent);
void output_print_str(std::ostream& out, std::string str, std::string str_value, int indent);
void output_open_encloser(const char* symbol, std::ostream &out, int& indent);
void output_close_encloser(const char* symbol, std::ostream &out, int& indent, bool no_newline = false);

///////////////////////////////////////////////////////////////////////////////

#endif // UTIL_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

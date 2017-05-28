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

#ifndef DELTA_MONITOR_H
#define DELTA_MONITOR_H

#include <string>
#include <ostream>

class SequenceFactory;
class Sequence;

enum DELTA_TYPE {
	dSimpleDelta,
};

#define MAX_DELTA_TYPE ((DELTA_TYPE) (dSimpleDelta+1))

class DeltaMonitor {
public:
	static Sequence *GetSequence();

	static char GetSepChar();

	static void CreateRndNumInstance(const unsigned long seed);

	static bool is_running() { return is_running_; }

	static bool is_delta() { return is_delta_; }

	static bool no_delta_reduction() { return no_delta_reduction_; }

	static bool init(std::string &msg, const std::string &monitor_type, const std::string &o_file);

	static bool init_for_running(std::string &msg, const std::string &monitor_type,
				const std::string &o_file, const std::string &i_file, bool no_delta);

	static bool set_delta_type(std::string &msg, const std::string &monitor_type);

	static void Output(std::ostream &out);

	static const std::string &get_input();

	static const std::string &get_output();

private:
	DeltaMonitor();

	~DeltaMonitor();

	static void OutputStatistics(std::ostream &out);

	static DELTA_TYPE delta_type_;

	static std::string output_file_;

	static std::string input_file_;

	static bool is_running_;

	static bool is_delta_;

	static bool no_delta_reduction_;

	static Sequence *seq_;
};

#endif

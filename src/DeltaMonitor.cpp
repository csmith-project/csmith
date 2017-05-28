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

#ifdef WIN32
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif

#include "DeltaMonitor.h"
#include <fstream>
#include <cassert>
#include "SequenceFactory.h"
#include "SimpleDeltaSequence.h"
#include "SimpleDeltaRndNumGenerator.h"
#include "random.h"
#include "RandomNumber.h"

using namespace std;

DELTA_TYPE DeltaMonitor::delta_type_ = MAX_DELTA_TYPE;

std::string DeltaMonitor::output_file_ = "";

std::string DeltaMonitor::input_file_ = "";

bool DeltaMonitor::is_running_ = false;

bool DeltaMonitor::is_delta_ = false;

bool DeltaMonitor::no_delta_reduction_ = false;

Sequence *DeltaMonitor::seq_ = NULL;

DeltaMonitor::DeltaMonitor()
{

}

DeltaMonitor::~DeltaMonitor()
{

}

Sequence *
DeltaMonitor::GetSequence()
{
	assert(delta_type_ != MAX_DELTA_TYPE);

	switch (DeltaMonitor::delta_type_) {
	case dSimpleDelta:
		DeltaMonitor::seq_ = SimpleDeltaSequence::CreateInstance(SimpleDeltaSequence::default_sep_char);
		break;
	default:
		assert("DeltaMonitor GetSequence error" && 0);
		break;
	}
	assert(DeltaMonitor::seq_);
	return DeltaMonitor::seq_;
}

char
DeltaMonitor::GetSepChar()
{
	return SimpleDeltaSequence::default_sep_char;
}

void
DeltaMonitor::CreateRndNumInstance(const unsigned long seed)
{
	assert(!DeltaMonitor::input_file_.empty());
	switch (DeltaMonitor::delta_type_) {
	case dSimpleDelta:
		RandomNumber::CreateInstance(rSimpleDeltaRndNumGenerator, seed);
		break;
	default:
		assert("DeltaMonitor CreateRndNumInstance error" && 0);
		break;
	}
}

bool
DeltaMonitor::set_delta_type(std::string &msg, const std::string &monitor_type)
{
	if (!monitor_type.compare("simple")) {
		DeltaMonitor::delta_type_ = dSimpleDelta;
	}
	else {
		msg = "not supported monitor type!";
		return false;
	}
	DeltaMonitor::is_running_ = true;
	return true;
}

bool
DeltaMonitor::init(std::string &msg, const std::string &monitor_type, const std::string &o_file)
{
	assert(!monitor_type.empty());

	if (o_file.empty()) {
		msg = "please specify the file for delta output by --delta-output [file]";
		return false;
	}
	DeltaMonitor::output_file_ = o_file;

	if (!DeltaMonitor::set_delta_type(msg, monitor_type)) {
		return false;
	}

	return true;
}

bool
DeltaMonitor::init_for_running(std::string &msg, const std::string &monitor_type,
		const std::string &o_file, const std::string &i_file, bool no_delta)
{
	assert(!monitor_type.empty());

	if (i_file.empty()) {
		msg = "please specify the file for delta input by --delta-input [file]";
		return false;
	}
	if (o_file.empty()) {
		DeltaMonitor::output_file_ = i_file;
	}
	else {
		DeltaMonitor::output_file_ = o_file;
	}
	DeltaMonitor::input_file_ = i_file;

	if (!DeltaMonitor::set_delta_type(msg, monitor_type)) {
		return false;
	}

	DeltaMonitor::is_delta_ = true;
	DeltaMonitor::no_delta_reduction_ = no_delta;
	return true;
}

void
DeltaMonitor::OutputStatistics(ostream &out)
{
	switch (DeltaMonitor::delta_type_) {
	case dSimpleDelta:
		SimpleDeltaRndNumGenerator::OutputStatistics(out);
		break;
	default:
		assert(0);
		break;
	}
}

void
DeltaMonitor::Output(ostream &out)
{
	if (!DeltaMonitor::is_running_)
		return;

	if (is_delta_)
		DeltaMonitor::OutputStatistics(out);

	assert(!output_file_.empty());
	std::string s;

	get_sequence(s);
	ofstream ofile(output_file_.c_str());
	ofile << s;
}

const std::string &
DeltaMonitor::get_input()
{
	return DeltaMonitor::input_file_;
}

const std::string &
DeltaMonitor::get_output()
{
	return DeltaMonitor::output_file_;
}


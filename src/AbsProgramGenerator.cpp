// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2017 The University of Utah
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

#include "AbsProgramGenerator.h"
#include <iostream>
#include <cassert>
#include <string>
#include "CGOptions.h"
#include "DefaultProgramGenerator.h"
#include "DFSProgramGenerator.h"
#include "Probabilities.h"

using namespace std;

AbsProgramGenerator *AbsProgramGenerator::current_generator_ = NULL;

OutputMgr *
AbsProgramGenerator::GetOutputMgr()
{
	assert(current_generator_);
	return current_generator_->getOutputMgr();
}

AbsProgramGenerator *
AbsProgramGenerator::CreateInstance(int argc, char *argv[], unsigned long seed)
{
	if (CGOptions::dfs_exhaustive()) {
		AbsProgramGenerator::current_generator_ = new DFSProgramGenerator(argc, argv, seed);
	}
	else
	{
		AbsProgramGenerator::current_generator_ = new DefaultProgramGenerator(argc, argv, seed);
	}

	AbsProgramGenerator::current_generator_->initialize();

	if (!(CGOptions::dump_default_probabilities().empty())) {
		CGOptions::random_random(false);
		Probabilities *prob = Probabilities::GetInstance();
		prob->dump_default_probabilities(CGOptions::dump_default_probabilities());
		exit(0);
	}
	else if (!(CGOptions::dump_random_probabilities().empty())) {
		CGOptions::random_random(true);
		Probabilities *prob = Probabilities::GetInstance();
		prob->dump_actual_probabilities(CGOptions::dump_random_probabilities(), seed);
		exit(0);
	}

	Probabilities *prob = Probabilities::GetInstance();
	string msg;
	if (!(CGOptions::probability_configuration().empty())) {
		if (!prob->parse_configuration(msg, CGOptions::probability_configuration())) {
			cout << "parsing configuration file error:" << msg << endl;
			return NULL;
		}
	}

	return AbsProgramGenerator::current_generator_;
}

AbsProgramGenerator *
AbsProgramGenerator::GetInstance()
{
	assert(AbsProgramGenerator::current_generator_);
	return AbsProgramGenerator::current_generator_;
}

AbsProgramGenerator::AbsProgramGenerator()
{

}

AbsProgramGenerator::~AbsProgramGenerator()
{

}

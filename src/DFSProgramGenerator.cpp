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

#include "DFSProgramGenerator.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include "RandomNumber.h"
#include "AbsRndNumGenerator.h"
#include "DFSRndNumGenerator.h"
#include "DFSOutputMgr.h"
#include "Finalization.h"
#include "Error.h"
#include "Function.h"
#include "VariableSelector.h"
#include "util.h"
#include "PartialExpander.h"

using namespace std;

DFSProgramGenerator::DFSProgramGenerator(int argc, char *argv[], unsigned long seed)
	: argc_(argc),
	  argv_(argv),
	  seed_(seed),
	  good_count_(0),
	  output_mgr_(NULL)
{

}

DFSProgramGenerator::~DFSProgramGenerator()
{
	Finalization::doFinalization();
	delete output_mgr_;
}

void
DFSProgramGenerator::initialize()
{
	RandomNumber::CreateInstance(rDFSRndNumGenerator, seed_);
	output_mgr_ = DFSOutputMgr::CreateInstance();
	assert(output_mgr_);
}

std::string
DFSProgramGenerator::get_count_prefix(const std::string &name)
{
	std::ostringstream ss;
	ss << "p_" << good_count_ << "_" << name;
	return ss.str();
}

void
DFSProgramGenerator::goGenerator()
{
	DFSRndNumGenerator *impl =
		dynamic_cast<DFSRndNumGenerator*>(RandomNumber::GetRndNumGenerator());
	//unsigned long long count = 0;
	GenerateAllTypes();
	output_mgr_->OutputStructUnions(cout);
	while(!impl->get_all_done()) {
		Error::set_error(SUCCESS);
		GenerateFunctions();
		if (Error::get_error() == SUCCESS) {
			//count++;
			//if (count >= 47376)
				//cout << "here" << std::endl;
			output_mgr_->OutputHeader(argc_, argv_, seed_);
			output_mgr_->Output();
			OutputMgr::really_outputln(cout);
			good_count_++;
		}
		impl->reset_state();
		Function::doFinalization();
		VariableSelector::doFinalization();
		reset_gensym();
		PartialExpander::restore_init_values();
		//cout << "count = " << count << std::endl;
	}
}


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

#include "DefaultProgramGenerator.h"
#include <cassert>
#include <sstream>
#include "RandomNumber.h"
#include "AbsRndNumGenerator.h"
#include "DefaultOutputMgr.h"
#include "ReducerOutputMgr.h"
#include "Finalization.h"
#include "Function.h"
#include "Type.h"
#include "DeltaMonitor.h"
#include "CGOptions.h"
#include "SafeOpFlags.h"
#include "ExtensionMgr.h"

DefaultProgramGenerator::DefaultProgramGenerator(int argc, char *argv[], unsigned long seed)
	: argc_(argc),
	  argv_(argv),
	  seed_(seed),
	  output_mgr_(NULL)
{

}

DefaultProgramGenerator::~DefaultProgramGenerator()
{
	Finalization::doFinalization();
	delete output_mgr_;
}

void
DefaultProgramGenerator::initialize()
{
	if (DeltaMonitor::is_delta()) {
		DeltaMonitor::CreateRndNumInstance(seed_);
	}
	else {
		RandomNumber::CreateInstance(rDefaultRndNumGenerator, seed_);
	}
	if (CGOptions::get_reducer()) {
		output_mgr_ = new ReducerOutputMgr();
	} else {
		output_mgr_ = DefaultOutputMgr::CreateInstance();
	}
	assert(output_mgr_);

	ExtensionMgr::CreateExtension();
}

std::string
DefaultProgramGenerator::get_count_prefix(const std::string &)
{
	assert(0);
	return "";
}

void
DefaultProgramGenerator::goGenerator()
{
	output_mgr_->OutputHeader(argc_, argv_, seed_);

	GenerateAllTypes();
	GenerateFunctions();
	output_mgr_->Output();
	if (CGOptions::identify_wrappers()) {
		ofstream ofile;
		ofile.open("wrapper.h");
		ofile << "#define N_WRAP " << SafeOpFlags::wrapper_names.size() << std::endl;
		ofile.close();
	}
}


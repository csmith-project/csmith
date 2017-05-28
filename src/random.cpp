// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2015, 2017 The University of Utah
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

// random.cpp --- utilities for random choice
// 		  Now it's just a wrapper to RandomNumber interface.

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cassert>
#include <cstdlib>

#include "random.h"
#include "RandomNumber.h"
#include "Filter.h"
#include "CGOptions.h"
#include "AbsProgramGenerator.h"

std::string get_prefixed_name(const std::string &name)
{
	if (!CGOptions::prefix_name())
		return name;
	if (!CGOptions::sequence_name_prefix()) {
		AbsProgramGenerator* instance = AbsProgramGenerator::GetInstance();
		assert(instance);
		return instance->get_count_prefix(name);
	}

	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->get_prefixed_name(name);
}

std::string RandomHexDigits( int num )
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->RandomHexDigits(num);
}

std::string RandomDigits( int num )
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->RandomDigits(num);
}

unsigned int
rnd_upto(const unsigned int n, const Filter *f, const std::string* where)
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->rnd_upto(n, f, where);
}

bool
rnd_flipcoin(const unsigned int p, const Filter *f, const std::string* where)
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->rnd_flipcoin(p, f, where);
}

std::string PureRandomHexDigits( int num )
{
	if (!CGOptions::is_random()) {
		RNDNUM_GENERATOR old;
	    	old = RandomNumber::SwitchRndNumGenerator(rDefaultRndNumGenerator);
		std::string rv = RandomHexDigits(num);
	    	RandomNumber::SwitchRndNumGenerator(old);
		return rv;
	}
	else {
		return RandomHexDigits(num);
	}
}

std::string PureRandomDigits( int num )
{

	if (!CGOptions::is_random()) {
		RNDNUM_GENERATOR old;
	    	old = RandomNumber::SwitchRndNumGenerator(rDefaultRndNumGenerator);
		std::string rv = RandomDigits(num);
	    	RandomNumber::SwitchRndNumGenerator(old);
		return rv;
	}
	else {
		return RandomDigits(num);
	}
}

unsigned int
pure_rnd_upto(const unsigned int n, const Filter *f, const std::string* where)
{
	if (n==0) return 0;		// not a random choice, but we still need to handle it though
	if (!CGOptions::is_random()) {
		RNDNUM_GENERATOR old;
	    	old = RandomNumber::SwitchRndNumGenerator(rDefaultRndNumGenerator);
		unsigned int rv = rnd_upto(n, f, where);
	    	RandomNumber::SwitchRndNumGenerator(old);
		return rv;
	}
	else {
		return rnd_upto(n, f, where);
	}
}

bool
pure_rnd_flipcoin(const unsigned int n, const Filter *f, const std::string* where)
{
	if (!CGOptions::is_random()) {
		RNDNUM_GENERATOR old;
	    	old = RandomNumber::SwitchRndNumGenerator(rDefaultRndNumGenerator);
		bool rv = rnd_flipcoin(n, f, where);
	    	RandomNumber::SwitchRndNumGenerator(old);
		return rv;
	}
	else {
		return rnd_flipcoin(n, f, where);
	}
}

std::string &
trace_depth()
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->trace_depth();
}

void
get_sequence(std::string &sequence)
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	rnd->get_sequence(sequence);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

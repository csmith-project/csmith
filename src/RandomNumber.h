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

#ifndef RANDOM_NUMBER_H
#define RANDOM_NUMBER_H

///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <map>
#include "CommonMacros.h"
#include "AbsRndNumGenerator.h"

class Filter;

/*
 * Common interface of all random number generators.
 * Works like a bridge to those generators.
 */
class RandomNumber
{
public:
	// Make it singleton for now.
	static void CreateInstance(RNDNUM_GENERATOR rImpl, const unsigned long seed);

	static RandomNumber *GetInstance();

	static AbsRndNumGenerator *GetRndNumGenerator(void);

	// Return the previous impl
	static RNDNUM_GENERATOR SwitchRndNumGenerator(RNDNUM_GENERATOR rImpl);

	static void doFinalization();

	std::string get_prefixed_name(const std::string &name);

	std::string &trace_depth();

	void get_sequence(std::string &sequence);

	// Probably it's not a good idea to define those functions with default arguments.
	// It would have potential problem to be misused.
	// I defined them in this way only for compatible to the previous code.
	// Use it carefully.
	virtual unsigned int rnd_upto(const unsigned int n, const Filter *f = NULL, const std::string *where = NULL);

	virtual bool rnd_flipcoin(const unsigned int p, const Filter *f = NULL, const std::string *where = NULL);

	virtual ~RandomNumber(void);

	virtual std::string RandomHexDigits(int num);

	virtual std::string RandomDigits(int num);

protected:
	void make_all_rndnum_generators(const unsigned long seed);

	AbsRndNumGenerator *curr_generator_;

	static RandomNumber *instance_;

	std::map<RNDNUM_GENERATOR, AbsRndNumGenerator*> generators_;

private:
	const unsigned long seed_;

	explicit RandomNumber(const unsigned long seed);

	explicit RandomNumber(AbsRndNumGenerator *rndnum_generator);

	// Don't implement them
	DISALLOW_COPY_AND_ASSIGN(RandomNumber);
};

#endif //RANDOM_NUMBER_H

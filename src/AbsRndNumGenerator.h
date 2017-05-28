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

#ifndef ABS_RNDNUM_GENERATOR
#define ABS_RNDNUM_GENERATOR

#include <string>
#include "CommonMacros.h"

class Filter;

enum RNDNUM_GENERATOR {
	rDefaultRndNumGenerator = 0,
	rDFSRndNumGenerator,
	rSimpleDeltaRndNumGenerator,
};

#define MAX_RNDNUM_GENERATOR (rSimpleDeltaRndNumGenerator+1)

// I could make AbsRndNumGenerator not pure, but want to force each subclass implement
// it's own member functions, in case of forgetting something.
class AbsRndNumGenerator
{
public:
	static AbsRndNumGenerator *make_rndnum_generator(RNDNUM_GENERATOR impl, const unsigned long seed);

	static void seedrand(const unsigned long seed);

	static const char* get_hex1();

	static const char* get_dec1();

	static unsigned int count (void) {
		return MAX_RNDNUM_GENERATOR;
	}

	virtual std::string get_prefixed_name(const std::string &name) = 0;

	virtual std::string& trace_depth(void) = 0;

	virtual void get_sequence(std::string &sequence) = 0;

	virtual unsigned int rnd_upto(const unsigned int n, const Filter *f = NULL, const std::string *where = NULL) = 0;

	virtual bool rnd_flipcoin(const unsigned int p, const Filter *f = NULL, const std::string *where = NULL) = 0;

	virtual std::string RandomHexDigits( int num ) = 0;

	virtual std::string RandomDigits( int num ) = 0;

	// Although it's not a good idea to return the kind of different implementation,
	// it's useful for error_handler. Basically we don't want to make the code
	// depend on the kind, use polymorphism instead.
	virtual enum RNDNUM_GENERATOR kind() = 0;

	virtual ~AbsRndNumGenerator(void);

protected:
	virtual unsigned long genrand(void) = 0;

	AbsRndNumGenerator();

private:
	// ------------------------------------------------------------------------------------------
	// "hex" and "dec" are reserved keywords in MSVC, we have to rename them
	static const char *hex1;

	static const char *dec1;

	// Don't implement them
	DISALLOW_COPY_AND_ASSIGN(AbsRndNumGenerator);
};

#endif //ABS_RNDNUM_GENERATOR


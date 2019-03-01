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
/*
   Use :returns a random number formed from hexdigits(only 0 to 9 and A to F)
	internally calls RandomNumber class object

   Parameters:
        num = size of the random number user wants

   Returns:
        a string , ex . "19AC830" for num = 7

 *
 */
std::string RandomHexDigits( int num )
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->RandomHexDigits(num);
}
/*
   Use : returns a random number formed from digit(only 0 to 9) of size num
	  internally calls the RandomNumber class object
   Parameters:
        num = size of the random number user wants

   Returns:
        a string , ex . "1924830" for num = 7

 *
 */
std::string RandomDigits( int num )
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->RandomDigits(num);
}
/*
   Use : returns a random number in the range 0..(n-1)
        internally calls the RandomNumber class object rnd -> rnd_upto()
   Parameters:

   Returns:

 *
 */
unsigned int
rnd_upto(const unsigned int n, const Filter *f, const std::string* where)
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->rnd_upto(n, f, where);
}
/*
   Use : It returns boolean value T or F and it implicitly calls rnd_flipcoin of RandomNumber class
   Parameters:

   Returns: Boolean value T or F

 *
 */
bool
rnd_flipcoin(const unsigned int p, const Filter *f, const std::string* where)
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->rnd_flipcoin(p, f, where);
}

/*
   Use : Calls RandomHexDigits() if command line set is not random.
         If cmd line is set, switch generator and calls RandomHexDigits()
*/
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
/*
   Use : Calls RandomDigits() if command line set is not random.
         If cmd line is set, switch generator and calls RandomDigits()
*/
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
/*
   Use : Calls rnd_upto() if command line set is not random.
	 If cmd line is set, switch generator and calls rnd_upto()
 */
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
/*
   Use : If command line set is not random, then it switches to another generator and then calls
         rnd_flipcoin to return boolean value T or F
	 else calls rnd_flipcoin

   Parameters: Integer of size n

   Returns: Either T or F

 */
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
/*
   Use :
   Parameters: None
   Returns: string from rnd -> rnd_upto()

 *
 */
std::string &
trace_depth()
{
	RandomNumber *rnd = RandomNumber::GetInstance();
	return rnd->trace_depth();
}
/*
   Use - It appends '_' for n-1 values of the map (seq_map_) and
        at end appends the last value
        ex. step 1 --> 1_2_3_
            step 2 --> 1_2_3_4 where 1,2,3,4 are the values in map
   and returns pointer pointing to it. (i.e. return string)

   Parameters: Input string - sequence

   Returns: None

   DS used - seq_map <int, int>			(in LinearSequence class)

 *
 */
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

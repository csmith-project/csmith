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

#include "RandomNumber.h"

#include <cassert>
#include <iostream>
#include "AbsRndNumGenerator.h"
#include "Filter.h"

RandomNumber *RandomNumber::instance_ = NULL;
/*
	generators_  map <RNDNUM_GENERATOR, instance of AbsRndNumGenerator class>
	use : current function adds element in generators_
		and initializes each RNDNUM_GENERATOR with NULL
 */
RandomNumber::RandomNumber(const unsigned long seed)
	: seed_(seed)
{
	//returns the count of the total number of random generators
	// currently 3 types of random generator - default, dfs and simple delta
	unsigned int count = AbsRndNumGenerator::count();

	for (unsigned int i = 0; i < count; ++i) {
		RNDNUM_GENERATOR rImpl = static_cast<RNDNUM_GENERATOR>(i);
		generators_[rImpl] = NULL;
	}
}

/*
	Use - It destroys generators_ map < RND_NUM_GENERATOR, instance of abs >
*/
RandomNumber::~RandomNumber()
{
	generators_.clear();
}

/*
	Use - It creates different random generators based on count value and store in the vector generators_

	How it does - It calls make_rndnum_generator of abs class.
		      which further have 3 choice of selecting generators
					1. Default
					2. DFS
					3. Simple Delta
		     After selecting generator, make_rndnum_generator of respective class is called.
			e.g. for default generator,
					default::make_rndnum_generator

	DS used - generators_ = map <RNDNUM_GENERATOR, instance of AbsRndNumGenerator class>
*/
void
RandomNumber::make_all_rndnum_generators(const unsigned long seed)
{
	unsigned int count = AbsRndNumGenerator::count();
	AbsRndNumGenerator *generator;

	for (unsigned int i = 0; i < count; ++i) {
		RNDNUM_GENERATOR rImpl = static_cast<RNDNUM_GENERATOR>(i);
		//it returns one of the random number generators(from the list - RNDNUM_GENERATOR)
		generator = AbsRndNumGenerator::make_rndnum_generator(rImpl, seed);
		generators_[rImpl] = generator;
	}
}

void
RandomNumber::CreateInstance(RNDNUM_GENERATOR rImpl, const unsigned long seed)
{
	if (!instance_) {
		instance_ = new RandomNumber(seed);
		//instance_->make_all_rndnum_generators(seed);
		//instance_->curr_generator_ = instance_->generators_[rImpl];
		AbsRndNumGenerator *generator = AbsRndNumGenerator::make_rndnum_generator(rImpl, seed);
		assert(generator);
		instance_->curr_generator_ = generator;
		instance_->generators_[rImpl] = generator;
	}
	else {
		instance_->curr_generator_ = instance_->generators_[rImpl];
		assert(instance_->curr_generator_);
	}
}

RandomNumber*
RandomNumber::GetInstance(void)
{
	assert(instance_);
	return instance_;
}

AbsRndNumGenerator*
RandomNumber::GetRndNumGenerator(void)
{
	assert(instance_->curr_generator_);
	return instance_->curr_generator_;
}

/*
 * Currently we can't switch to SeqRandom mode from other mode,
 * because we need a way to specify sequence numbers anyway.
 */
RNDNUM_GENERATOR
RandomNumber::SwitchRndNumGenerator(RNDNUM_GENERATOR rImpl)
{
	unsigned int count = AbsRndNumGenerator::count();
	AbsRndNumGenerator *generator = instance_->generators_[rImpl];
	if (generator == NULL) {
		generator = AbsRndNumGenerator::make_rndnum_generator(rImpl, instance_->seed_);
		assert(generator);
		instance_->generators_[rImpl] = generator;
	}

	RNDNUM_GENERATOR old = instance_->curr_generator_->kind();
	assert(static_cast<unsigned int>(old) < count);

	instance_->curr_generator_ = generator;
	return old;
}

/*
	Use - Returns string "name" as it is in case of Default generator.
	      Does some complexation on string and then return in case of DFS generator
*/
std::string
RandomNumber::get_prefixed_name(const std::string &name)
{
	return curr_generator_->get_prefixed_name(name);
}

/*
        Use - It calls curr generator which returns string "" appending "->" to it  from rnd_upto(). Check later, not sure about it
*/

std::string &
RandomNumber::trace_depth()
{
	return curr_generator_->trace_depth();
}

/*
 * It calls curr generator (it may be default, dfs or simple delta)
   Use - generator appends '_' for n-1 values of the map (seq_map_) and
        at end appends the last value
        ex. step 1 --> 1_2_3_
            step 2 --> 1_2_3_4 where 1,2,3,4 are the values in map

        and return "1_2_3_4"

   Data structure used - seq_map <int,int>      (in LinearSequence class)

 */
void
RandomNumber::get_sequence(std::string &sequence)
{
	curr_generator_->get_sequence(sequence);
}

/*
   calls curr generator and return a value from  0..(n-1) with help of genrand() i.e. lrand48() - selects val from [0, 2^31)
        and implicilty assigns value to  trace_string_
        What is significane of *where string?
*/
unsigned int
RandomNumber::rnd_upto(const unsigned int n, const Filter *f, const std::string *where)
{
	return curr_generator_->rnd_upto(n, f, where);
}

bool
RandomNumber::rnd_flipcoin(const unsigned int p, const Filter *f, const std::string *where)
{
	return curr_generator_->rnd_flipcoin(p, f, where);
}
/*
   Use : returns a random number formed from hexdigits(only 0 to 9 and A to F) of size num
   Parameters:
        num = size of the random number user wants

   Returns:
        a string , ex . "19AC830" for num = 7
 *
 */

std::string
RandomNumber::RandomHexDigits(int num)
{
	return curr_generator_->RandomHexDigits(num);
}
/*
   Use : returns a random number formed from digit(only 0 to 9) of size num
   Parameters:
        num = size of the random number user wants

   Returns:
        a string , ex . "1924830" for num = 7
 *
 */

std::string
RandomNumber::RandomDigits(int num)
{
	return curr_generator_->RandomDigits(num);
}
/*
	Deletes the values in the map
	generators_ = map <RNDNUM_GENERATOR, instance of AbsRndNumGenerator class>
        ________________________________________
       | RNDNUM_GENERATOR | AbsRndNumGenerator*  |
       |__________________|______________________|
       |__________________|______________________|
       |__________________|______________________|
       |__________________|______________________|

short : deletes various instances of RandomNumber generation
	and @end deletes the generators_ data structure (a map)
 */
void
RandomNumber::doFinalization()
{
	unsigned int count = AbsRndNumGenerator::count();
	AbsRndNumGenerator *generator;

	for (unsigned int i = 0; i < count; ++i) {
		RNDNUM_GENERATOR rImpl = static_cast<RNDNUM_GENERATOR>(i);
		generator = instance_->generators_[rImpl];
		if (generator != NULL) {
			delete generator;
		}
	}
	delete instance_;
}


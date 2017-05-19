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

RandomNumber::RandomNumber(const unsigned long seed)
	: seed_(seed)
{
	unsigned int count = AbsRndNumGenerator::count();

	for (unsigned int i = 0; i < count; ++i) {
		RNDNUM_GENERATOR rImpl = static_cast<RNDNUM_GENERATOR>(i);
		generators_[rImpl] = NULL;
	}
}

RandomNumber::~RandomNumber()
{
	generators_.clear();
}

void
RandomNumber::make_all_rndnum_generators(const unsigned long seed)
{
	unsigned int count = AbsRndNumGenerator::count();
	AbsRndNumGenerator *generator;

	for (unsigned int i = 0; i < count; ++i) {
		RNDNUM_GENERATOR rImpl = static_cast<RNDNUM_GENERATOR>(i);
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

std::string
RandomNumber::get_prefixed_name(const std::string &name)
{
	return curr_generator_->get_prefixed_name(name);
}

std::string &
RandomNumber::trace_depth()
{
	return curr_generator_->trace_depth();
}

void
RandomNumber::get_sequence(std::string &sequence)
{
	curr_generator_->get_sequence(sequence);
}

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

std::string
RandomNumber::RandomHexDigits(int num)
{
	return curr_generator_->RandomHexDigits(num);
}

std::string
RandomNumber::RandomDigits(int num)
{
	return curr_generator_->RandomDigits(num);
}

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


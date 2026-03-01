// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011 The University of Utah
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

#ifndef DEFAULT_RNDNUM_GENERATOR_H
#define DEFAULT_RNDNUM_GENERATOR_H

#include "AbsRndNumGenerator.h"
#include "Common.h"
#include <string>
#include <vector>

class Sequence;
class Filter;

// Singleton class for the implementation of default based random generator
class DefaultRndNumGenerator : public AbsRndNumGenerator {
public:
  static DefaultRndNumGenerator *
  make_rndnum_generator(const unsigned long seed);

  virtual std::string get_prefixed_name(const std::string &name) override;

  virtual std::string &trace_depth() override;

  virtual void get_sequence(std::string &sequence) override;

  virtual unsigned int rnd_upto(const unsigned int n, const Filter *f = nullptr,
                                const std::string *where = nullptr) override;

  virtual bool rnd_flipcoin(const unsigned int p, const Filter *f = nullptr,
                            const std::string *where = nullptr) override;

  virtual std::string RandomHexDigits(int num) override;

  virtual std::string RandomDigits(int num) override;

  virtual enum RNDNUM_GENERATOR kind() { return rDefaultRndNumGenerator; }

  void set_rand_depth(unsigned INT64 depth) { rand_depth_ = depth; }

  virtual ~DefaultRndNumGenerator() override;

private:
  DefaultRndNumGenerator(const unsigned long seed, Sequence *concrete_seq);

  void add_number(int v, int bound, int k);

  static DefaultRndNumGenerator *impl_;

  unsigned INT64 rand_depth_;

  std::string trace_string_;

  Sequence *seq_;

  virtual unsigned long genrand(void) override;

  // void seedrand(unsigned long seed);

  // Don't implement them
  DefaultRndNumGenerator(const DefaultRndNumGenerator &) = delete;
  DefaultRndNumGenerator &operator=(const DefaultRndNumGenerator &) = delete;
};

#endif // DEFAULT_RNDNUM_GENERATOR_H

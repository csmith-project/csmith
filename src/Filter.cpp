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

#include "Filter.h"

#include <cassert>
#include "CGOptions.h"
#include "CGContext.h"

using namespace std;

Filter::Filter()
{
	kinds_.set();
}

Filter::~Filter(void)
{
	kinds_.reset();
}

/*
 *
 */
void Filter::enable(FilterKind kind)
{
	assert(kind < MAX_FILTER_KIND_SIZE);
	kinds_[kind] = true;
}

/*
 *
 */
void Filter::disable(FilterKind kind)
{
	assert(kind < MAX_FILTER_KIND_SIZE);
	kinds_[kind] = false;
}

/*
 *
 */
FilterKind
Filter::current_kind(void) const
{
	if (CGOptions::random_based())
		return fDefault;
	else if (CGOptions::dfs_exhaustive())
		return fDFS;
	return MAX_FILTER_KIND_SIZE;
}

/*
 *
 */
bool
Filter::valid_filter() const
{
	FilterKind k = current_kind();

	if (!kinds_.test(k))
		return false;
	return true;
}


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

#include <cassert>
#include "MspFilters.h"
#include "SafeOpFlags.h"

MspBinaryFilter::MspBinaryFilter()
{

}

MspBinaryFilter::~MspBinaryFilter()
{

}

bool
MspBinaryFilter::filter(int v) const
{
	assert(v >= 0);
	eBinaryOps op = static_cast<eBinaryOps>(v);
	if ((op == eDiv || op == eMod))
		return true;
	else
		return false;
}

/////////////////////////////////////////////////////////

MspSafeOpSizeFilter::MspSafeOpSizeFilter(eBinaryOps op)
	: bin_op_(op)
{

}

MspSafeOpSizeFilter::~MspSafeOpSizeFilter()
{

}

bool
MspSafeOpSizeFilter::filter(int v) const
{
	assert(v >= 0);
	SafeOpSize op_size = static_cast<SafeOpSize>(v);
	switch(bin_op_) {
	case eMul: //fall-through
	case eRShift: //fall-through
	case eLShift:
		if ((op_size == sInt32 || op_size == sInt64))
			return true;
		else
			return false;
		break;
	default:
		return false;
	}
}


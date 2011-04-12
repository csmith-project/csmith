
// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011 The University of Utah
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

#ifndef ERROR_H
#define ERROR_H

#include "CommonMacros.h"

#define SUCCESS 0
#define ERROR -1
#define EXCEED_MAX_DEPTH_ERROR -2
#define FILTER_ERROR -3
#define BACKTRACKING_ERROR -4
#define COMPATIBLE_CHECK_ERROR -5
#define INVALID_SIMPLE_DELTA_SEQUENCE -6

#define ERROR_RETURN() \
	if (Error::get_error() != SUCCESS) \
		return;\

#define ERROR_GUARD(rv) \
	if (Error::get_error() != SUCCESS) \
		return rv; \

#define ERROR_GUARD_AND_DEL1(rv, p) \
	if (Error::get_error() != SUCCESS) {\
		delete p; \
		return rv; \
	} \

#define ERROR_GUARD_AND_DEL2(rv, p1, p2) \
	if (Error::get_error() != SUCCESS) {\
		delete p1; \
		delete p2; \
		return rv; \
	} \

#define ERROR_GUARD_AND_DEL3(rv, p1, p2, p3) \
	if (Error::get_error() != SUCCESS) {\
		delete p1; \
		delete p2; \
		delete p3; \
		return rv; \
	} \

#define ERROR_GUARD_AND_DEL4(rv, p1, p2, p3, p4) \
	if (Error::get_error() != SUCCESS) {\
		delete p1; \
		delete p2; \
		delete p3; \
		delete p4; \
		return rv; \
	} \


#define PRT_ERROR(msg) \
	cout << "error: " << msg << ", errorno: " << Error::get_error() << std::endl; \

class Error {
public:
	static int get_error() { return Error::r_error_; }
	static void set_error(int error) { Error::r_error_ = error; }
private:
	Error();
	~Error();
	static int r_error_;

	DISALLOW_COPY_AND_ASSIGN(Error);
};

#endif // ERROR_H

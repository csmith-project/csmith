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

//
// This file was derived from a random program generator written by Bryan
// Turner.  The attributions in that file was:
//
// Random Program Generator
// Bryan Turner (bryan.turner@pobox.com)
// July, 2005
//

// ---------------------------------------
// Platform-Specific code to get a unique seed value (usually from the tick counter, etc)
//
#include <sys/types.h>
#include <sys/timeb.h>

#include "platform.h"

#if (TARGET_CPU_powerpc == 1 || TARGET_CPU_powerpc64 == 1)
/*For PPC, got from:
http://lists.ozlabs.org/pipermail/linuxppc-dev/1999-October/003889.html
*/
static unsigned long long read_time(void) {
	unsigned long long retval;
	unsigned long junk;
	__asm__ __volatile__ ("\n\
1:	mftbu %1\n\
	mftb %L0\n\
	mftbu %0\n\
	cmpw %0,%1\n\
	bne 1b"
	: "=r" (retval), "=r" (junk));
	return retval;
}
#else
#ifdef WIN32
static unsigned __int64 read_time(void) {
        unsigned l, h;
        _asm {rdtsc    
        mov l, eax  
        mov h, edx 
        }
        return (h << 32) + l ;
}
#else
static long long read_time(void) {
        long long l;
        asm volatile(   "rdtsc\n\t"
                : "=A" (l)
        );
        return l;
}
#endif
#endif

unsigned long platform_gen_seed()
{
	return (long) read_time();
}

//////////// platform specific mkdir /////////////////
#ifndef WIN32
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#else
#include <direct.h>
#include <errno.h>
#endif

bool create_dir(const char *dir)
{
#ifndef WIN32
	if (mkdir(dir, 0770) == -1) {
#else
	if (mkdir(dir) == -1) {
#endif
		return (errno == EEXIST) ? true : false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

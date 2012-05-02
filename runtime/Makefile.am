## -*- mode: Makefile-Automake -*-
##
## Copyright (c) 2010, 2011, 2012 The University of Utah
## All rights reserved.
##
## This file is part of `csmith', a random generator of C programs.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
##   * Redistributions of source code must retain the above copyright notice,
##     this list of conditions and the following disclaimer.
##
##   * Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in the
##     documentation and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.

###############################################################################

lib_LIBRARIES = libcsmith.a
libcsmith_a_SOURCES = \
	volatile_runtime.c

# XXX --- This does not work; the library is still installed to $(libdir).
# libcsmith_a_libdir = \
# 	$(libdir)/$(PACKAGE)-$(VERSION)

# Build a shared library, too, in case someone wants this.
lib_LTLIBRARIES = libcsmith.la
libcsmith_la_SOURCES = \
	$(libcsmith_a_SOURCES)

GIT_HASH := $(shell "$(top_srcdir)/git-hash.sh" "$(top_srcdir)" || echo error)
GIT_FLAG = -DGIT_VERSION=\"$(GIT_HASH)\"

## ENE: The idea of stuffing the target CPU name (a string) into a CPP symbol
## is kind of awful.  What we should do instead is use the name to modify the
## include path, e.g., -I"$(target_cpu)".  But that refactoring is for a later
## day...

libcsmith_a_CPPFLAGS = \
	-DTARGET_CPU_$(target_cpu)=1 \
	$(GIT_FLAG)
libcsmith_la_CPPFLAGS = \
	$(libcsmith_a_CPPFLAGS)

libcsmith_a_includedir = \
	$(includedir)/$(PACKAGE)-$(VERSION)
nobase_libcsmith_a_include_HEADERS = \
	csmith.h \
	csmith_minimal.h \
	custom_limits.h \
	custom_stdint_x86.h \
	platform_avr.h \
	platform_generic.h \
	platform_msp430.h \
	random_inc.h \
	safe_abbrev.h \
	stdint_avr.h \
	stdint_ia32.h \
	stdint_ia64.h \
	stdint_msp430.h \
	windows/stdint.h \
	volatile_runtime.c \
	volatile_runtime.h
noinst_HEADERS = \
	safe_math.m4 \
	safe_math_macros.m4 \
	safe_math_macros_notmp.m4
nodist_libcsmith_a_include_HEADERS = \
	safe_math.h \
	safe_math_macros.h \
	safe_math_macros_notmp.h

## XXX should we put `volatile_runtime.c' in _DATA?

BUILT_SOURCES = \
	safe_math.h \
	safe_math_macros.h \
	safe_math_macros_notmp.h
CLEANFILES = \
	$(BUILT_SOURCES)

safe_math.h: safe_math.m4
	$(M4) < $< > $@

safe_math_macros.h: safe_math_macros.m4
	$(M4) < $< > $@

safe_math_macros_notmp.h: safe_math_macros_notmp.m4
	$(M4) < $< > $@

# We distribute, but do not build, the following Windows support files.
EXTRA_DIST = \
	windows/_rand48.c \
	windows/lrand48.c \
	windows/rand48.h \
	windows/srand48.c

###############################################################################

## End of file.

/* -*- mode: C -*-
 *
 * Copyright (c) 2008, 2017 The University of Utah
 * All rights reserved.
 *
 * This file is part of `csmith', a random generator of C programs.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/*****************************************************************************/

#define WRAP_VOLATILES 1

#include "volatile_runtime.h"

/*
 *
 */
#define DEFINE_VOLATILE_FUNCTIONS(type)				\
	type											\
	volatile_##type##_ptr_read(volatile type *p)	\
	{ return *p; }									\
													\
	volatile type *									\
	volatile_##type##_ptr_id(volatile type *p)		\
	{ return p; }

DEFINE_VOLATILE_FUNCTIONS(int8_t)
DEFINE_VOLATILE_FUNCTIONS(int16_t)
DEFINE_VOLATILE_FUNCTIONS(int32_t)
DEFINE_VOLATILE_FUNCTIONS(uint8_t)
DEFINE_VOLATILE_FUNCTIONS(uint16_t)
DEFINE_VOLATILE_FUNCTIONS(uint32_t)

/*****************************************************************************/

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */

/* End of file. */

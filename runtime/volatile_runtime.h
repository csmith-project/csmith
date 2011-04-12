/* -*- mode: C -*-
 *
 * Copyright (c) 2008 The University of Utah
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

#ifndef VOLATILE_RUNTIME_H
#define VOLATILE_RUNTIME_H

/*****************************************************************************/

#ifndef WRAP_VOLATILES
#  define WRAP_VOLATILES 0
#endif

#if WRAP_VOLATILES
/*
 * Wrap accesses to volatile variables with function calls.
 */
#  include <stdint.h>

#  define DECLARE_VOLATILE_WRAPPERS(type)						\
	type volatile_##type##_ptr_read(volatile type *p);			\
	volatile type *volatile_##type##_ptr_id(volatile type *p);

DECLARE_VOLATILE_WRAPPERS(int8_t)
DECLARE_VOLATILE_WRAPPERS(int16_t)
DECLARE_VOLATILE_WRAPPERS(int32_t)
DECLARE_VOLATILE_WRAPPERS(int64_t)
DECLARE_VOLATILE_WRAPPERS(uint8_t)
DECLARE_VOLATILE_WRAPPERS(uint16_t)
DECLARE_VOLATILE_WRAPPERS(uint32_t)
DECLARE_VOLATILE_WRAPPERS(uint64_t)

#  undef DECLARE_VOLATILE_WRAPPERS

#  define VOL_LVAL(var, type)	(*(volatile_##type##_ptr_id(&(var))))
#  define VOL_RVAL(var, type)	(volatile_##type##_ptr_read(&(var)))

#else
/*
 * Access volatile variables directly.
 */
#  define VOL_LVAL(var, type) (var)
#  define VOL_RVAL(var, type) (var)

#endif /* WRAP_VOLATILES */

/*****************************************************************************/

#endif /* VOLATILE_RUNTIME_H */

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */

/* End of file. */

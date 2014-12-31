/* -*- mode: C -*-
 *
 * Copyright (c) 2014 The University of Utah
 */
/* Copyright (C) 1997,1998,1999,2000,2001,2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
   02110-1301 USA.  */

/*
 *	ISO C99: 7.18 Integer types <stdint.h>
 */

/*
 * Our custom limits.h based on the system header file.
 */

#ifndef _CUSTOM_LIMITS_H
#define _CUSTOM_LIMITS_H 1

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
# define INT8_MIN		(-128)
# define INT16_MIN		(-32767-1)
# define INT32_MIN		(-2147483647-1)
#ifndef NO_LONGLONG
# define INT64_MIN		(-(9223372036854775807LL)-1)
#endif
/* Maximum of signed integral types.  */
# define INT8_MAX		(127)
# define INT16_MAX		(32767)
# define INT32_MAX		(2147483647)
#ifndef NO_LONGLONG
# define INT64_MAX		(9223372036854775807LL)
#endif

/* Maximum of unsigned integral types.  */
# define UINT8_MAX		(255)
# define UINT16_MAX		(65535)
# define UINT32_MAX		(4294967295U)
#ifndef NO_LONGLONG
# define UINT64_MAX		(18446744073709551615ULL)
#endif

#  define CHAR_BIT	8

/* Limits of FP types.  */

#define FLT_MAX 3.40282347e+38F
#define DBL_MAX 1.7976931348623158e+308

#endif

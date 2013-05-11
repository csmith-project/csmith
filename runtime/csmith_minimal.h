/* -*- mode: C -*-
 *
 * Copyright (c) 2007-2011, 2013 The University of Utah
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

#ifdef NO_PRINTF
int putchar (int);
#else
extern int printf (const char *, ...);
#endif

// FIXME-- need more versions, and a way to figure out which is needed
#include "custom_stdint_x86.h"

#include "custom_limits.h"

#define STATIC static

#define UNDEFINED(__val) (__val)

#define LOG_INDEX

#define LOG_EXEC

#define FUNC_NAME(x) (safe_##x)

// FIXME
#define assert(x)

#include "safe_math.h"

static inline void platform_main_begin(void)
{
}

static inline void crc32_gentab (void)
{
}

#define _CSMITH_BITFIELD(x) ((x>32)?(x%32):x)

int crc32_context;

#ifdef TCC
int strcmp (const char *s1, const char *s2)
{
  for(; *s1 == *s2; ++s1, ++s2)
    if(*s1 == 0)
      return 0;
  return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}
#else
extern int strcmp (const char *, const char *);
#endif

static inline void 
transparent_crc (uint32_t val, char* vname, int flag)
{
#ifndef NO_PRINTF
  if (flag) printf ("%s %d\n", vname, val);
#endif
  crc32_context += val;
}

#ifdef NO_PRINTF
void my_puts (char *p)
{
  int i = 0;
  while (p[i]) {
    putchar (p[i]);
    i++;
  }
}

void put_hex (int x)
{
  switch (x) {
  case 0: putchar ('0'); break;
  case 1: putchar ('1'); break;
  case 2: putchar ('2'); break;
  case 3: putchar ('3'); break;
  case 4: putchar ('4'); break;
  case 5: putchar ('5'); break;
  case 6: putchar ('6'); break;
  case 7: putchar ('7'); break;
  case 8: putchar ('8'); break;
  case 9: putchar ('9'); break;
  case 10: putchar ('a'); break;
  case 11: putchar ('b'); break;
  case 12: putchar ('c'); break;
  case 13: putchar ('d'); break;
  case 14: putchar ('e'); break;
  case 15: putchar ('f'); break;
  }
}
#endif

static inline void
platform_main_end (int x, int flag)
{
#ifndef NOT_PRINT_CHECKSUM
  if (!flag) {
#ifdef NO_PRINTF
    int i;
    my_puts ("checksum = ");
    for (i=0; i<8; i++) {
      put_hex (x & 0xf);
      x >>= 4;
    }
#else
    printf ("checksum = %x\n", x);
#endif
  }
#endif
}

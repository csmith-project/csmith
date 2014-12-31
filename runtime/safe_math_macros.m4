dnl -*- mode: m4 -*-
dnl
dnl Copyright (c) 2008, 2009, 2013 The University of Utah
dnl All rights reserved.
dnl
dnl This file is part of `csmith', a random generator of C programs.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions are met:
dnl
dnl   * Redistributions of source code must retain the above copyright notice,
dnl     this list of conditions and the following disclaimer.
dnl
dnl   * Redistributions in binary form must reproduce the above copyright
dnl     notice, this list of conditions and the following disclaimer in the
dnl     documentation and/or other materials provided with the distribution.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
dnl LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
dnl CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
dnl SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
dnl INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
dnl CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
dnl ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.
dnl
dnl For more information, see:
dnl https://www.securecoding.cert.org/confluence/display/cplusplus/04.+Integers+(INT)

#ifndef SAFE_MATH_H
#define SAFE_MATH_H

define(`safe_signed_math',`

#define safe_unary_minus_func_$1_s(_si) \
  ({ $1 si = (_si) ; \
   ((($1)(si))==($2))? \
    (($1)(si)): \
    (-(($1)(si))) \
  ;})

#define safe_add_func_$1_s_s(_si1,_si2) \
		({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
                 ((((($1)(si1))>(($1)0)) && ((($1)(si2))>(($1)0)) && ((($1)(si1)) > (($3)-(($1)(si2))))) \
		  || (((($1)(si1))<(($1)0)) && ((($1)(si2))<(($1)0)) && ((($1)(si1)) < (($2)-(($1)(si2)))))) ? \
		 (($1)(si1)) :						\
		 ((($1)(si1)) + (($1)(si2)))				\
		;}) 

#define safe_sub_func_$1_s_s(_si1,_si2) \
		({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
                ((((($1)(si1))^(($1)(si2))) \
		& ((((($1)(si1)) ^ (((($1)(si1))^(($1)(si2))) \
		& ((($1)1) << (sizeof($1)*CHAR_BIT-1))))-(($1)(si2)))^(($1)(si2)))) < (($1)0)) \
		? (($1)(si1)) \
		: ((($1)(si1)) - (($1)(si2))) \
		;})

#define safe_mul_func_$1_s_s(_si1,_si2) \
  ({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
  ((((($1)(si1)) > (($1)0)) && ((($1)(si2)) > (($1)0)) && ((($1)(si1)) > (($3) / (($1)(si2))))) || \
  (((($1)(si1)) > (($1)0)) && ((($1)(si2)) <= (($1)0)) && ((($1)(si2)) < (($2) / (($1)(si1))))) || \
  (((($1)(si1)) <= (($1)0)) && ((($1)(si2)) > (($1)0)) && ((($1)(si1)) < (($2) / (($1)(si2))))) || \
  (((($1)(si1)) <= (($1)0)) && ((($1)(si2)) <= (($1)0)) && ((($1)(si1)) != (($1)0)) && ((($1)(si2)) < (($3) / (($1)(si1)))))) \
  ? (($1)(si1)) \
  : (($1)(si1)) * (($1)(si2));})

#define safe_mod_func_$1_s_s(_si1,_si2) \
  ({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
  (((($1)(si2)) == (($1)0)) || (((($1)(si1)) == ($2)) && ((($1)(si2)) == (($1)-1)))) \
  ? (($1)(si1)) \
  : ((($1)(si1)) % (($1)(si2)));})

#define safe_div_func_$1_s_s(_si1,_si2) \
  ({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
  (((($1)(si2)) == (($1)0)) || (((($1)(si1)) == ($2)) && ((($1)(si2)) == (($1)-1)))) \
  ? (($1)(si1)) \
  : ((($1)(si1)) / (($1)(si2)));})

#define safe_lshift_func_$1_s_s(_left,_right) \
  ({ $1 left = (_left); int right = (_right) ; \
   (((($1)(left)) < (($1)0)) \
  || (((int)(right)) < (($1)0)) \
  || (((int)(right)) >= sizeof($1)*CHAR_BIT) \
  || ((($1)(left)) > (($3) >> ((int)(right))))) \
  ? (($1)(left)) \
  : ((($1)(left)) << ((int)(right)));})

#define safe_lshift_func_$1_s_u(_left,_right) \
  ({ $1 left = (_left); unsigned int right = (_right) ; \
   (((($1)(left)) < (($1)0)) \
  || (((unsigned int)(right)) >= sizeof($1)*CHAR_BIT) \
  || ((($1)(left)) > (($3) >> ((unsigned int)(right))))) \
  ? (($1)(left)) \
  : ((($1)(left)) << ((unsigned int)(right)));})

#define safe_rshift_func_$1_s_s(_left,_right) \
	({ $1 left = (_left); int right = (_right) ; \
        (((($1)(left)) < (($1)0)) \
			 || (((int)(right)) < (($1)0)) \
			 || (((int)(right)) >= sizeof($1)*CHAR_BIT)) \
			? (($1)(left)) \
			: ((($1)(left)) >> ((int)(right)));})

#define safe_rshift_func_$1_s_u(_left,_right) \
  ({ $1 left = (_left); unsigned int right = (_right) ; \
   (((($1)(left)) < (($1)0)) \
			 || (((unsigned int)(right)) >= sizeof($1)*CHAR_BIT)) \
			? (($1)(left)) \
			: ((($1)(left)) >> ((unsigned int)(right)));})
')

safe_signed_math(int8_t,INT8_MIN,INT8_MAX)
safe_signed_math(int16_t,INT16_MIN,INT16_MAX)
safe_signed_math(int32_t,INT32_MIN,INT32_MAX)
safe_signed_math(int64_t,INT64_MIN,INT64_MAX)

define(`promote',`ifelse($1,uint64_t,unsigned long long,unsigned int)')

define(`safe_unsigned_math',`

#define safe_unary_minus_func_$1_u(_ui) \
  ({ $1 ui = (_ui); -(($1)(ui));})

#define safe_add_func_$1_u_u(_ui1,_ui2) \
  ({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; \
  (($1)(ui1)) + (($1)(ui2));})

#define safe_sub_func_$1_u_u(_ui1,_ui2) \
  ({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; (($1)(ui1)) - (($1)(ui2));})

#define safe_mul_func_$1_u_u(_ui1,_ui2) \
  ({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; ($1)(((promote($1))(ui1)) * ((promote($1))(ui2)));})

#define safe_mod_func_$1_u_u(_ui1,_ui2) \
	({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; \
         ((($1)(ui2)) == (($1)0)) \
			? (($1)(ui1)) \
			: ((($1)(ui1)) % (($1)(ui2)));})

#define safe_div_func_$1_u_u(_ui1,_ui2) \
	        ({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; \
                 ((($1)(ui2)) == (($1)0)) \
			? (($1)(ui1)) \
			: ((($1)(ui1)) / (($1)(ui2)));})

#define safe_lshift_func_$1_u_s(_left,_right) \
	({ $1 left = (_left); int right = (_right) ; \
          ((((int)(right)) < (($1)0)) \
			 || (((int)(right)) >= sizeof($1)*CHAR_BIT) \
			 || ((($1)(left)) > (($2) >> ((int)(right))))) \
			? (($1)(left)) \
			: ((($1)(left)) << ((int)(right)));})

#define safe_lshift_func_$1_u_u(_left,_right) \
	 ({ $1 left = (_left); unsigned int right = (_right) ; \
           ((((unsigned int)(right)) >= sizeof($1)*CHAR_BIT) \
			 || ((($1)(left)) > (($2) >> ((unsigned int)(right))))) \
			? (($1)(left)) \
			: ((($1)(left)) << ((unsigned int)(right)));})

#define safe_rshift_func_$1_u_s(_left,_right) \
	({ $1 left = (_left); int right = (_right) ; \
          ((((int)(right)) < (($1)0)) \
			 || (((int)(right)) >= sizeof($1)*CHAR_BIT)) \
			? (($1)(left)) \
			: ((($1)(left)) >> ((int)(right)));})

#define safe_rshift_func_$1_u_u(_left,_right) \
	({ $1 left = (_left); unsigned int right = (_right) ; \
                 (((unsigned int)(right)) >= sizeof($1)*CHAR_BIT) \
			 ? (($1)(left)) \
			 : ((($1)(left)) >> ((unsigned int)(right)));})

')

safe_unsigned_math(uint8_t,UINT8_MAX)
safe_unsigned_math(uint16_t,UINT16_MAX)
safe_unsigned_math(uint32_t,UINT32_MAX)
safe_unsigned_math(uint64_t,UINT64_MAX)

#endif

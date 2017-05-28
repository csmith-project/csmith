dnl -*- mode: m4 -*-
dnl
dnl Copyright (c) 2008, 2009, 2013, 2014, 2016 The University of Utah
dnl All rights reserved.
dnl
dnl This file is part of `csmith', a random generator of C programs.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions are met:
dnl
dnl * Redistributions of source code must retain the above copyright notice,
dnl this list of conditions and the following disclaimer.
dnl
dnl * Redistributions in binary form must reproduce the above copyright
dnl notice, this list of conditions and the following disclaimer in the
dnl documentation and/or other materials provided with the distribution.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
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
dnl
dnl But note that some of the functions on that page are wrong!

#ifndef SAFE_MATH_H
#define SAFE_MATH_H

define(`promote1',`ifelse($1,int64_t,long long,int)')
define(`promote2',`ifelse($1,uint64_t,unsigned long long,unsigned int)')
define(`maxshift',`ifelse($1,int64_t,64,32)')

define(`safe_signed_math',`

STATIC $1
FUNC_NAME(unary_minus_func_$1_s)($1 si LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
#if ($3>=INT_MAX)
    (si==$2) ?
    (UNDEFINED(si)) :
#endif
#endif
    -si;
}

STATIC $1
FUNC_NAME(add_func_$1_s_s)($1 si1, $1 si2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
#if ($3>=INT_MAX)
    (((si1>0) && (si2>0) && (si1 > ($3-si2))) || ((si1<0) && (si2<0) && (si1 < ($2-si2)))) ?
    (UNDEFINED(si1)) :
#endif
#endif
    (si1 + si2);
}

STATIC $1
FUNC_NAME(sub_func_$1_s_s)($1 si1, $1 si2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
#if ($3>=INT_MAX)
    (((si1^si2) & (((si1 ^ ((si1^si2) & (~$3)))-si2)^si2)) < 0) ? 
    (UNDEFINED(si1)) : 
#endif
#endif
    (si1 - si2);
}

STATIC $1
FUNC_NAME(mul_func_$1_s_s)($1 si1, $1 si2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
#if ($3>=INT_MAX)
    (((si1 > 0) && (si2 > 0) && (si1 > ($3 / si2))) || ((si1 > 0) && (si2 <= 0) && (si2 < ($2 / si1))) || ((si1 <= 0) && (si2 > 0) && (si1 < ($2 / si2))) || ((si1 <= 0) && (si2 <= 0) && (si1 != 0) && (si2 < ($3 / si1)))) ? 
    (UNDEFINED(si1)) : 
#endif
#endif
    si1 * si2;
}

STATIC $1
FUNC_NAME(mod_func_$1_s_s)($1 si1, $1 si2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((si2 == 0) || ((si1 == $2) && (si2 == (-1)))) ? 
    (UNDEFINED(si1)) : 
#endif
    (si1 % si2);
}

STATIC $1
FUNC_NAME(div_func_$1_s_s)($1 si1, $1 si2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((si2 == 0) || ((si1 == $2) && (si2 == (-1)))) ? 
    (UNDEFINED(si1)) : 
#endif
    (si1 / si2);
}

STATIC $1
FUNC_NAME(lshift_func_$1_s_s)($1 left, int right LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((left < 0) || (((int)right) < 0) || (((int)right) >= maxshift) || (left > ($3 >> ((int)right)))) ? 
    (UNDEFINED(left)) : 
#endif
    (left << ((int)right));
}

STATIC $1
FUNC_NAME(lshift_func_$1_s_u)($1 left, unsigned int right LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((left < 0) || (((unsigned int)right) >= maxshift) || (left > ($3 >> ((unsigned int)right)))) ? 
    (UNDEFINED(left)) : 
#endif
    (left << ((unsigned int)right));
}

STATIC $1
FUNC_NAME(rshift_func_$1_s_s)($1 left, int right LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((left < 0) || (((int)right) < 0) || (((int)right) >= maxshift))? 
    (UNDEFINED(left)) : 
#endif
    (left >> ((int)right));
}

STATIC $1
FUNC_NAME(rshift_func_$1_s_u)($1 left, unsigned int right LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((left < 0) || (((unsigned int)right) >= maxshift)) ? 
    (UNDEFINED(left)) : 
#endif
    (left >> ((unsigned int)right));
}
')

safe_signed_math(int8_t,INT8_MIN,INT8_MAX)
safe_signed_math(int16_t,INT16_MIN,INT16_MAX)
safe_signed_math(int32_t,INT32_MIN,INT32_MAX)
#ifndef NO_LONGLONG
safe_signed_math(int64_t,INT64_MIN,INT64_MAX)
#endif

define(`safe_unsigned_math',`

STATIC $1
FUNC_NAME(unary_minus_func_$1_u)($1 ui LOG_INDEX)
{
  LOG_EXEC
  return -ui;
}

STATIC $1
FUNC_NAME(add_func_$1_u_u)($1 ui1, $1 ui2 LOG_INDEX)
{
  LOG_EXEC
  return ui1 + ui2;
}

STATIC $1
FUNC_NAME(sub_func_$1_u_u)($1 ui1, $1 ui2 LOG_INDEX)
{
  LOG_EXEC
  return ui1 - ui2;
}

STATIC $1
FUNC_NAME(mul_func_$1_u_u)($1 ui1, $1 ui2 LOG_INDEX)
{
  LOG_EXEC
  return ((promote2($1))ui1) * ((promote2($1))ui2);
}

STATIC $1
FUNC_NAME(mod_func_$1_u_u)($1 ui1, $1 ui2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    (ui2 == 0) ? 
    (UNDEFINED(ui1)) : 
#endif
    (ui1 % ui2);
}

STATIC $1
FUNC_NAME(div_func_$1_u_u)($1 ui1, $1 ui2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    (ui2 == 0) ? 
    (UNDEFINED(ui1)) : 
#endif
    (ui1 / ui2);
}

STATIC $1
FUNC_NAME(lshift_func_$1_u_s)($1 left, int right LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((((int)right) < 0) || (((int)right) >= maxshift) || (left > ($2 >> ((int)right)))) ? 
    (UNDEFINED(left)) : 
#endif
    (left << ((int)right));
}

STATIC $1
FUNC_NAME(lshift_func_$1_u_u)($1 left, unsigned int right LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((((unsigned int)right) >= maxshift) || (left > ($2 >> ((unsigned int)right)))) ? 
    (UNDEFINED(left)) : 
#endif
    (left << ((unsigned int)right));
}

STATIC $1
FUNC_NAME(rshift_func_$1_u_s)($1 left, int right LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    ((((int)right) < 0) || (((int)right) >= maxshift)) ? 
    (UNDEFINED(left)) : 
#endif
    (left >> ((int)right));
}

STATIC $1
FUNC_NAME(rshift_func_$1_u_u)($1 left, unsigned int right LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE
    (((unsigned int)right) >= maxshift) ? 
    (UNDEFINED(left)) : 
#endif
    (left >> ((unsigned int)right));
}
')

safe_unsigned_math(uint8_t,UINT8_MAX)
safe_unsigned_math(uint16_t,UINT16_MAX)
safe_unsigned_math(uint32_t,UINT32_MAX)
#ifndef NO_LONGLONG
safe_unsigned_math(uint64_t,UINT64_MAX)
#endif

dnl safe floating point computation, based on Pascal's suggestion
dnl Non-C99 compliant compilers (e.g. MS VC) may not have hexadecimal floats notation.
dnl For those use ldexp and ldexpf calls instead.

define(`safe_float_math',`

STATIC $1
FUNC_NAME(add_func_$1_f_f)($1 sf1, $1 sf2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE_FLOAT
    (fabs$2((0.5$2 * sf1) + (0.5$2 * sf2)) > (0.5$2 * $3)) ? 
    UNDEFINED(sf1) :
#endif
    (sf1 + sf2);
}

STATIC $1
FUNC_NAME(sub_func_$1_f_f)($1 sf1, $1 sf2 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE_FLOAT
    (fabs$2((0.5$2 * sf1) - (0.5$2 * sf2)) > (0.5$2 * $3)) ? 
    UNDEFINED(sf1) :
#endif
    (sf1 - sf2);
}

STATIC $1
FUNC_NAME(mul_func_$1_f_f)($1 sf1, $1 sf2 LOG_INDEX)
{
  LOG_EXEC
  return
#ifndef UNSAFE_FLOAT
#ifdef __STDC__
    (fabs$2((0x1.0p-100$2 * sf1) * ($4 * sf2)) > (0x1.0p-100$2 * ($4 * $3))) ?
#else
    (fabs$2((ldexp$2(1.0, -100) * sf1) * ($4 * sf2)) > (ldexp$2(1.0, -100) * ($4 * $3))) ?
#endif
    UNDEFINED(sf1) :
#endif
    (sf1 * sf2);
}

STATIC $1
FUNC_NAME(div_func_$1_f_f)($1 sf1, $1 sf2 LOG_INDEX)
{
  LOG_EXEC
  return
#ifndef UNSAFE_FLOAT
#ifdef __STDC__
    ((fabs$2(sf2) < 1.0$2) && (((sf2 == 0.0$2) || (fabs$2(($5 * sf1) / (0x1.0p100$2 * sf2))) > (0x1.0p-100$2 * ($5 * $3))))) ?
#else
    ((fabs$2(sf2) < 1.0$2) && (((sf2 == 0.0$2) || (fabs$2(($5 * sf1) / (ldexp$2(1.0, 100) * sf2))) > (ldexp$2(1.0, -100) * ($5 * $3))))) ?
#endif
    UNDEFINED(sf1) :
#endif
    (sf1 / sf2);
}

')

#ifdef __STDC__
safe_float_math(float,f,FLT_MAX,0x1.0p-28f,0x1.0p-49f)
safe_float_math(double,,DBL_MAX,0x1.0p-924,0x1.0p-974)
#else
safe_float_math(float,f,FLT_MAX,ldexpf(1.0, -28),ldexpf(1.0, -49))
safe_float_math(double,,DBL_MAX,ldexp(1.0, -924),ldexp(1.0, -974))
#endif

define(`safe_float_conversion',`
STATIC $2
FUNC_NAME(convert_func_$1_to_$2)($1 sf1 LOG_INDEX)
{
  LOG_EXEC
  return 
#ifndef UNSAFE_FLOAT
    ((sf1 <= $3) || (sf1 >= $4)) ?
    UNDEFINED($4) :
#endif
    (($2)(sf1));
}
')

safe_float_conversion(float, int32_t, INT32_MIN, INT32_MAX)


#endif

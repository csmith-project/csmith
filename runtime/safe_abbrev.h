/* -*- mode: C -*-
 *
 * Copyright (c) 2009, 2010, 2011 The University of Utah
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

#define  n8s(_x_)     safe_unary_minus_macro_int8_t_s(_x_)
#define  a8s(_x_,_y_) safe_add_macro_int8_t_s_s(_x_,_y_)
#define  s8s(_x_,_y_) safe_sub_macro_int8_t_s_s(_x_,_y_)
#define  m8s(_x_,_y_) safe_mul_macro_int8_t_s_s(_x_,_y_)
#define  o8s(_x_,_y_) safe_mod_macro_int8_t_s_s(_x_,_y_)
#define  d8s(_x_,_y_) safe_div_macro_int8_t_s_s(_x_,_y_)
#define l8ss(_x_,_y_) safe_lshift_macro_int8_t_s_s(_x_,_y_)
#define l8su(_x_,_y_) safe_lshift_macro_int8_t_s_u(_x_,_y_)
#define r8ss(_x_,_y_) safe_rshift_macro_int8_t_s_s(_x_,_y_)
#define r8su(_x_,_y_) safe_rshift_macro_int8_t_s_u(_x_,_y_)

#define  n8u(_x_)     safe_unary_minus_macro_uint8_t_u(_x_)
#define  a8u(_x_,_y_) safe_add_macro_uint8_t_u_u(_x_,_y_)
#define  s8u(_x_,_y_) safe_sub_macro_uint8_t_u_u(_x_,_y_)
#define  m8u(_x_,_y_) safe_mul_macro_uint8_t_u_u(_x_,_y_)
#define  o8u(_x_,_y_) safe_mod_macro_uint8_t_u_u(_x_,_y_)
#define  d8u(_x_,_y_) safe_div_macro_uint8_t_u_u(_x_,_y_)
#define l8us(_x_,_y_) safe_lshift_macro_uint8_t_u_s(_x_,_y_)
#define l8uu(_x_,_y_) safe_lshift_macro_uint8_t_u_u(_x_,_y_)
#define r8us(_x_,_y_) safe_rshift_macro_uint8_t_u_s(_x_,_y_)
#define r8uu(_x_,_y_) safe_rshift_macro_uint8_t_u_u(_x_,_y_)

#define  n1s(_x_)     safe_unary_minus_macro_int16_t_s(_x_)
#define  a1s(_x_,_y_) safe_add_macro_int16_t_s_s(_x_,_y_)
#define  s1s(_x_,_y_) safe_sub_macro_int16_t_s_s(_x_,_y_)
#define  m1s(_x_,_y_) safe_mul_macro_int16_t_s_s(_x_,_y_)
#define  o1s(_x_,_y_) safe_mod_macro_int16_t_s_s(_x_,_y_)
#define  d1s(_x_,_y_) safe_div_macro_int16_t_s_s(_x_,_y_)
#define l1ss(_x_,_y_) safe_lshift_macro_int16_t_s_s(_x_,_y_)
#define l1su(_x_,_y_) safe_lshift_macro_int16_t_s_u(_x_,_y_)
#define r1ss(_x_,_y_) safe_rshift_macro_int16_t_s_s(_x_,_y_)
#define r1su(_x_,_y_) safe_rshift_macro_int16_t_s_u(_x_,_y_)
     
#define  n1u(_x_)     safe_unary_minus_macro_uint16_t_u(_x_)
#define  a1u(_x_,_y_) safe_add_macro_uint16_t_u_u(_x_,_y_)
#define  s1u(_x_,_y_) safe_sub_macro_uint16_t_u_u(_x_,_y_)
#define  m1u(_x_,_y_) safe_mul_macro_uint16_t_u_u(_x_,_y_)
#define  o1u(_x_,_y_) safe_mod_macro_uint16_t_u_u(_x_,_y_)
#define  d1u(_x_,_y_) safe_div_macro_uint16_t_u_u(_x_,_y_)
#define l1us(_x_,_y_) safe_lshift_macro_uint16_t_u_s(_x_,_y_)
#define l1uu(_x_,_y_) safe_lshift_macro_uint16_t_u_u(_x_,_y_)
#define r1us(_x_,_y_) safe_rshift_macro_uint16_t_u_s(_x_,_y_)
#define r1uu(_x_,_y_) safe_rshift_macro_uint16_t_u_u(_x_,_y_)

#define  n3s(_x_)     safe_unary_minus_macro_int32_t_s(_x_)
#define  a3s(_x_,_y_) safe_add_macro_int32_t_s_s(_x_,_y_)
#define  s3s(_x_,_y_) safe_sub_macro_int32_t_s_s(_x_,_y_)
#define  m3s(_x_,_y_) safe_mul_macro_int32_t_s_s(_x_,_y_)
#define  o3s(_x_,_y_) safe_mod_macro_int32_t_s_s(_x_,_y_)
#define  d3s(_x_,_y_) safe_div_macro_int32_t_s_s(_x_,_y_)
#define l3ss(_x_,_y_) safe_lshift_macro_int32_t_s_s(_x_,_y_)
#define l3su(_x_,_y_) safe_lshift_macro_int32_t_s_u(_x_,_y_)
#define r3ss(_x_,_y_) safe_rshift_macro_int32_t_s_s(_x_,_y_)
#define r3su(_x_,_y_) safe_rshift_macro_int32_t_s_u(_x_,_y_)

#define  n3u(_x_)     safe_unary_minus_macro_uint32_t_u(_x_)
#define  a3u(_x_,_y_) safe_add_macro_uint32_t_u_u(_x_,_y_)
#define  s3u(_x_,_y_) safe_sub_macro_uint32_t_u_u(_x_,_y_)
#define  m3u(_x_,_y_) safe_mul_macro_uint32_t_u_u(_x_,_y_)
#define  o3u(_x_,_y_) safe_mod_macro_uint32_t_u_u(_x_,_y_)
#define  d3u(_x_,_y_) safe_div_macro_uint32_t_u_u(_x_,_y_)
#define l3us(_x_,_y_) safe_lshift_macro_uint32_t_u_s(_x_,_y_)
#define l3uu(_x_,_y_) safe_lshift_macro_uint32_t_u_u(_x_,_y_)
#define r3us(_x_,_y_) safe_rshift_macro_uint32_t_u_s(_x_,_y_)
#define r3uu(_x_,_y_) safe_rshift_macro_uint32_t_u_u(_x_,_y_)

#define  n6s(_x_)     safe_unary_minus_macro_int64_t_s(_x_)
#define  a6s(_x_,_y_) safe_add_macro_int64_t_s_s(_x_,_y_)
#define  s6s(_x_,_y_) safe_sub_macro_int64_t_s_s(_x_,_y_)
#define  m6s(_x_,_y_) safe_mul_macro_int64_t_s_s(_x_,_y_)
#define  o6s(_x_,_y_) safe_mod_macro_int64_t_s_s(_x_,_y_)
#define  d6s(_x_,_y_) safe_div_macro_int64_t_s_s(_x_,_y_)
#define l6ss(_x_,_y_) safe_lshift_macro_int64_t_s_s(_x_,_y_)
#define l6su(_x_,_y_) safe_lshift_macro_int64_t_s_u(_x_,_y_)
#define r6ss(_x_,_y_) safe_rshift_macro_int64_t_s_s(_x_,_y_)
#define r6su(_x_,_y_) safe_rshift_macro_int64_t_s_u(_x_,_y_)
     
#define  n6u(_x_)     safe_unary_minus_macro_uint64_t_u(_x_)
#define  a6u(_x_,_y_) safe_add_macro_uint64_t_u_u(_x_,_y_)
#define  s6u(_x_,_y_) safe_sub_macro_uint64_t_u_u(_x_,_y_)
#define  m6u(_x_,_y_) safe_mul_macro_uint64_t_u_u(_x_,_y_)
#define  o6u(_x_,_y_) safe_mod_macro_uint64_t_u_u(_x_,_y_)
#define  d6u(_x_,_y_) safe_div_macro_uint64_t_u_u(_x_,_y_)
#define l6us(_x_,_y_) safe_lshift_macro_uint64_t_u_s(_x_,_y_)
#define l6uu(_x_,_y_) safe_lshift_macro_uint64_t_u_u(_x_,_y_)
#define r6us(_x_,_y_) safe_rshift_macro_uint64_t_u_s(_x_,_y_)
#define r6uu(_x_,_y_) safe_rshift_macro_uint64_t_u_u(_x_,_y_)

// Extension to CSmith to test floating point optimizations
// Jacek Burys, Imperial College London 2015

#ifndef FLOAT_TEST_H
#define FLOAT_TEST_H

#ifdef FLOAT_TEST_ENABLED
#define __FLOAT float_interval_t
////////////////////////////////////

#include <stdint.h>

typedef struct float_interval {
	float lower;
	float upper;
} float_interval_t;


float_interval_t add_float_interval(float_interval_t in1, float_interval_t in2);
float_interval_t sub_float_interval(float_interval_t in1, float_interval_t in2);
float_interval_t mul_float_interval(float_interval_t in1, float_interval_t in2);
float_interval_t div_float_interval(float_interval_t in1, float_interval_t in2);

// casts to float_interval
float_interval_t char_to_float_interval(int8_t x);
float_interval_t short_to_float_interval(int16_t x);
float_interval_t int_to_float_interval(int32_t x);
float_interval_t long_to_float_interval(int32_t x);
float_interval_t long_long_to_float_interval(int64_t x);

float_interval_t uchar_to_float_interval(uint8_t x);
float_interval_t ushort_to_float_interval(uint16_t x);
float_interval_t uint_to_float_interval(uint32_t x);
float_interval_t ulong_to_float_interval(uint32_t x);
float_interval_t ulong_long_to_float_interval(uint64_t x);

//casts from float_interval
int8_t float_interval_to_char(float_interval_t in);
int16_t float_interval_to_short(float_interval_t in);
int32_t float_interval_to_int(float_interval_t in);
int32_t float_interval_to_long(float_interval_t in);
int64_t float_interval_to_long_long(float_interval_t in);

uint8_t float_interval_to_uchar(float_interval_t in);
uint16_t float_interval_to_ushort(float_interval_t in);
uint32_t float_interval_to_uint(float_interval_t in);
uint32_t float_interval_to_ulong(float_interval_t in);
uint64_t float_interval_to_ulong_long(float_interval_t in);




////////////////////////////////////

#else
#define __FLOAT float
#endif

#endif

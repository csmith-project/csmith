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

#define FLOAT_TEST_CONSTANT(c) (float_interval_t){c, c}

#define OUTPUT_FLOAT_INTERVAL_MACRO(x) printf(#x);\
printf(" = [%.8a, %.8a]\n", x.lower, x.upper);

#define CHAR_TO_FLOAT_INTERVAL(x) char_to_float_interval(x)
#define INT_TO_FLOAT_INTERVAL(x) int_to_float_interval(x)
#define SHORT_TO_FLOAT_INTERVAL(x) short_to_float_interval(x)
#define LONG_TO_FLOAT_INTERVAL(x) long_to_float_interval(x)
#define LONG_LONG_TO_FLOAT_INTERVAL(x) long_long_to_float_interval(x)
#define UCHAR_TO_FLOAT_INTERVAL(x) uchar_to_float_interval(x)
#define UINT_TO_FLOAT_INTERVAL(x) uint_to_float_interval(x)
#define USHORT_TO_FLOAT_INTERVAL(x) ushort_to_float_interval(x)
#define ULONG_TO_FLOAT_INTERVAL(x) ulong_to_float_interval(x)
#define ULONG_LONG_TO_FLOAT_INTERVAL(x) ulong_long_to_float_interval(x)


#define	FLOAT_INTERVAL_TO_CHAR(x) float_interval_to_char(x)
#define	FLOAT_INTERVAL_TO_INT(x) float_interval_to_int(x)
#define	FLOAT_INTERVAL_TO_SHORT(x) float_interval_to_short(x)
#define	FLOAT_INTERVAL_TO_LONG(x) float_interval_to_long(x)
#define	FLOAT_INTERVAL_TO_LONG_LONG(x) float_interval_to_long_long(x)
#define	FLOAT_INTERVAL_TO_UCHAR(x) float_interval_to_uchar(x)
#define	FLOAT_INTERVAL_TO_UINT(x) float_interval_to_uint(x)
#define	FLOAT_INTERVAL_TO_USHORT(x) float_interval_to_ushort(x)
#define	FLOAT_INTERVAL_TO_ULONG(x) float_interval_to_ulong(x)
#define	FLOAT_INTERVAL_TO_ULONG_LONG(x) float_interval_to_ulong_long(x)



#define MINUS_FLOAT_INTERVAL(x) minus_float_interval(x)
#define PLUS_FLOAT_INTERVAL(x) plus_float_interval(x)
#define NOT_FLOAT_INTERVAL(x) not_float_interval(x)
#define BITNOT_FLOAT_INTERVAL(x) not_float_interval(x)


#define FLOAT_AND_MACRO(x, y) float_test_and(x, y)
#define FLOAT_OR_MACRO(x, y) float_test_or(x, y)
#define FLOAT_CMPEQ_MACRO(x, y) float_test_cmpeq(x, y)
#define FLOAT_CMPNE_MACRO(x, y) float_test_cmpne(x, y)
#define FLOAT_CMPGT_MACRO(x, y) float_test_cmpgt(x, y)
#define FLOAT_CMPLT_MACRO(x, y) float_test_cmplt(x, y)
#define FLOAT_CMPLE_MACRO(x, y) float_test_cmple(x, y)
#define FLOAT_CMPGE_MACRO(x, y) float_test_cmpge(x, y)

		// Bitwise Ops
#define FLOAT_BITAND_MACRO(x, y) float_test_bitand(x, y)
#define FLOAT_BITOR_MACRO(x, y) float_test_bitor(x, y)
#define FLOAT_BITXOR_MACRO(x, y) float_test_bitxor(x, y)
#define FLOAT_LSHIFT_MACRO(x, y) float_test_lshift(x, y)
#define FLOAT_RSHIFT_MACRO(x, y) float_test_rshift(x, y)


//binary operators
float_interval_t add_float_interval(float_interval_t in1, float_interval_t in2);
float_interval_t sub_float_interval(float_interval_t in1, float_interval_t in2);
float_interval_t mul_float_interval(float_interval_t in1, float_interval_t in2);
float_interval_t div_float_interval(float_interval_t in1, float_interval_t in2);

//unary operators
float_interval_t plus_float_interval(float_interval_t in);
float_interval_t minus_float_interval(float_interval_t in);
int not_float_interval(float_interval_t in);
float_interval_t bitnot_float_interval(float_interval_t in);


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

//comparison operators
int float_test_and(float_interval_t in1, float_interval_t in2);
int float_test_or(float_interval_t in1, float_interval_t in2);
int float_test_cmpeq(float_interval_t in1, float_interval_t in2);
int float_test_cmpne(float_interval_t in1, float_interval_t in2);
int float_test_cmpgt(float_interval_t in1, float_interval_t in2);
int float_test_cmplt(float_interval_t in1, float_interval_t in2);
int float_test_cmple(float_interval_t in1, float_interval_t in2);
int float_test_cmpge(float_interval_t in1, float_interval_t in2);

		// Bitwise Ops
int float_test_bitand(float_interval_t in1, float_interval_t in2);
int float_test_bitor(float_interval_t in1, float_interval_t in2);
int float_test_bitxor(float_interval_t in1, float_interval_t in2);
int float_test_lshift(float_interval_t in1, float_interval_t in2);
int float_test_rshift(float_interval_t in1, float_interval_t in2);


////////////////////////////////////

#else

///////// NORMAL MODE

#define __FLOAT float
#define FLOAT_TEST_CONSTANT(c) c

#define OUTPUT_FLOAT_INTERVAL_MACRO(x) printf(#x);\
printf(" = %.8a\n", x);

#define CHAR_TO_FLOAT_INTERVAL(x) x
#define INT_TO_FLOAT_INTERVAL(x) x
#define SHORT_TO_FLOAT_INTERVAL(x) x
#define LONG_TO_FLOAT_INTERVAL(x) x
#define LONG_LONG_TO_FLOAT_INTERVAL(x) x
#define UCHAR_TO_FLOAT_INTERVAL(x) x
#define UINT_TO_FLOAT_INTERVAL(x) x
#define USHORT_TO_FLOAT_INTERVAL(x) x
#define ULONG_TO_FLOAT_INTERVAL(x) x
#define ULONG_LONG_TO_FLOAT_INTERVAL(x) x

#define	FLOAT_INTERVAL_TO_CHAR(x) x
#define	FLOAT_INTERVAL_TO_INT(x) x
#define	FLOAT_INTERVAL_TO_SHORT(x) x
#define	FLOAT_INTERVAL_TO_LONG(x) x
#define	FLOAT_INTERVAL_TO_LONG_LONG(x) x
#define	FLOAT_INTERVAL_TO_UCHAR(x) x
#define	FLOAT_INTERVAL_TO_UINT(x) x
#define	FLOAT_INTERVAL_TO_USHORT(x) x
#define	FLOAT_INTERVAL_TO_ULONG(x) x
#define	FLOAT_INTERVAL_TO_ULONG_LONG(x) x

#define MINUS_FLOAT_INTERVAL(x) -x
#define PLUS_FLOAT_INTERVAL(x) +x
#define NOT_FLOAT_INTERVAL(x) !x
#define BITNOT_FLOAT_INTERVAL(x) ~x

#define FLOAT_AND_MACRO(x, y) x && y
#define FLOAT_OR_MACRO(x, y) x || y
#define FLOAT_CMPEQ_MACRO(x, y) x == y
#define FLOAT_CMPNE_MACRO(x, y) x != y
#define FLOAT_CMPGT_MACRO(x, y) x > y
#define FLOAT_CMPLT_MACRO(x, y) x < y
#define FLOAT_CMPLE_MACRO(x, y) x <= y
#define FLOAT_CMPGE_MACRO(x, y) x >= y

#define FLOAT_BITAND_MACRO(x, y) x & y
#define FLOAT_BITOR_MACRO(x, y) x | y
#define FLOAT_BITXOR_MACRO(x, y) x ^ y
#define FLOAT_LSHIFT_MACRO(x, y) x << y
#define FLOAT_RSHIFT_MACRO(x, y) x >> y

/////////

#endif

#endif

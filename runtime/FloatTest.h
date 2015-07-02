// Extension to CSmith to test floating point optimizations
// Jacek Burys, Imperial College London 2015

#ifndef FLOAT_TEST_H
#define FLOAT_TEST_H

#ifdef FLOAT_TEST_ENABLED
#define __FLOAT float_interval_t
////////////////////////////////////

typedef struct float_interval {
	float lower;
	float upper;
} float_interval_t;



float_interval_t float_test_add_func(float_interval_t in1, float_interval_t in2);
float_interval_t float_test_sub_func(float_interval_t in1, float_interval_t in2);
float_interval_t float_test_mul_func(float_interval_t in1, float_interval_t in2);
float_interval_t float_test_div_func(float_interval_t in1, float_interval_t in2);

float_interval_t float_test_interval_to_uint(float_interval_t val);


////////////////////////////////////

#else
#define __FLOAT float
#endif

#endif

// Extension to CSmith to test floating point optimizations
// Jacek Burys, Imperial College London 2015

#include "FloatTest.h"

#ifdef FLOAT_TEST_ENABLED

///////////////////////////////////

float_interval_t float_test_add_func(float_interval_t in1, float_interval_t in2) {
	float_interval_t res;
	res.upper = 0;
	res.lower = 0;
	return res;
}

float_interval_t float_test_sub_func(float_interval_t in1, float_interval_t in2) {
	float_interval_t res;
	res.upper = 0;
	res.lower = 0;
	return res;
}

float_interval_t float_test_mul_func(float_interval_t in1, float_interval_t in2) {
	float_interval_t res;
	res.upper = 0;
	res.lower = 0;
	return res;
}

float_interval_t float_test_div_func(float_interval_t in1, float_interval_t in2) {
	float_interval_t res;
	res.upper = 0;
	res.lower = 0;
	return res;
}

uint32_t float_test_interval_to_uint(float_interval_t val) {
	return (uint32_t) val;
}


///////////////////////////////////

#endif

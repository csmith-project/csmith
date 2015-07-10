// Extension to CSmith to test floating point optimizations
// Jacek Burys, Imperial College London 2015


#ifdef FLOAT_TEST_ENABLED
#include "FloatTest.h"

///////////////////////////////////

float_interval_t add_float_interval(float_interval_t in1, float_interval_t in2){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t sub_float_interval(float_interval_t in1, float_interval_t in2){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t mul_float_interval(float_interval_t in1, float_interval_t in2){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t div_float_interval(float_interval_t in1, float_interval_t in2){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}


//////////////

float_interval_t plus_float_interval(float_interval_t in){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t minus_float_interval(float_interval_t in){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}


int not_float_interval(float_interval_t in){
	  return 0;
}


float_interval_t bitnot_float_interval(float_interval_t in){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}



///////////

float_interval_t char_to_float_interval(int8_t x){
  float_interval_t result;
  result.lower = 0.0f;
  result.upper = 0.0f;
  return result;
}

float_interval_t short_to_float_interval(int16_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;

}

float_interval_t int_to_float_interval(int32_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t long_to_float_interval(int32_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t long_long_to_float_interval(int64_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

// unsigned

float_interval_t uchar_to_float_interval(uint8_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t ushort_to_float_interval(uint16_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t uint_to_float_interval(uint32_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t ulong_to_float_interval(uint32_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

float_interval_t ulong_long_to_float_interval(uint64_t x){
	  float_interval_t result;
	  result.lower = 0.0f;
	  result.upper = 0.0f;
	  return result;
}

// comparison operators

int float_test_and(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_or(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_cmpeq(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_cmpne(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_cmpgt(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_cmplt(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_cmple(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_cmpge(float_interval_t in1, float_interval_t in2) {return 0;}

		// Bitwise Ops
int float_test_bitand(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_bitor(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_bitxor(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_lshift(float_interval_t in1, float_interval_t in2) {return 0;}
int float_test_rshift(float_interval_t in1, float_interval_t in2) {return 0;}


///////////////////////////////////

#endif

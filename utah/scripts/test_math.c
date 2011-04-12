#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

static long __undefined;

#undef SAFE_MATH_H
#define UNDEFINED(x) (__undefined=1,(x))
#define FUNC_NAME(x) checked_##x
#include "safe_math.h"
#undef UNDEFINED
#undef FUNC_NAME

#undef SAFE_MATH_H
#define UNSAFE
#define FUNC_NAME(x) unsafe_##x
#include "safe_math.h"
#undef UNSAFE
#undef FUNC_NAME

#undef SAFE_MATH_H
#define UNDEFINED(x) (x)
#define FUNC_NAME(x) safe_##x
#include "safe_math.h"
#undef UNDEFINED
#undef FUNC_NAME

#define checkit(OP,TYP,SUF)			\
      __undefined = 0; \
      z1 = checked_##OP##_func_##TYP##_##SUF (x, y); \
      if (__undefined) {	 \
	OP##_undef_cnt++;  \
      } else { \
	z2 = safe_##OP##_func_##TYP##_##SUF (x, y); \
	assert (z1==z2); \
	z3 = unsafe_##OP##_func_##TYP##_##SUF (x, y); \
	assert (z1==z3); \
      }

#define CHECK_FUNC(TYPE,MIN,MAX,SUF)		\
void check_##TYPE (void) \
{ \
  long long add_undef_cnt = 0; \
  long long sub_undef_cnt = 0; \
  long long mul_undef_cnt = 0; \
  long long mod_undef_cnt = 0; \
  long long div_undef_cnt = 0; \
  long long  x,y; \
  for (x=MIN; x<MAX; x++) { \
    for (y=MIN; y<MAX; y++) { \
      long long z1, z2, z3; \
      checkit(add,TYPE,SUF);   \
      checkit(sub,TYPE,SUF);      \
      checkit(mul,TYPE,SUF);	  \
      checkit(mod,TYPE,SUF);	  \
      checkit(div,TYPE,SUF);	  \
    } \
  } \
  printf ("%d-bit add undefined at %lld locations\n", 8*sizeof(TYPE), add_undef_cnt); \
  printf ("%d-bit sub undefined at %lld locations\n", 8*sizeof(TYPE), sub_undef_cnt); \
  printf ("%d-bit mul undefined at %lld locations\n", 8*sizeof(TYPE), mul_undef_cnt); \
  printf ("%d-bit mod undefined at %lld locations\n", 8*sizeof(TYPE), mod_undef_cnt); \
  printf ("%d-bit div undefined at %lld locations\n", 8*sizeof(TYPE), div_undef_cnt); \
}

CHECK_FUNC(int8_t,INT8_MIN,INT8_MAX,s_s)
CHECK_FUNC(int16_t,INT16_MIN,INT16_MAX,s_s)
CHECK_FUNC(uint8_t,0,UINT8_MAX,u_u)
CHECK_FUNC(uint16_t,0,UINT16_MAX,u_u)

int main (void)
{
  check_int8_t();
  check_uint8_t();
  check_int16_t();
  check_uint16_t();

  return 0;
}

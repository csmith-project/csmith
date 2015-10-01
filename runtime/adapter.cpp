// Adapter for Boost Interval Arithmetic Library
// Jacek Burys, Imperial Colleg London, UROP 2015

#include "float_interval.h"
#include <boost/numeric/interval.hpp>
#include <assert.h>
#include <stdio.h>

using namespace boost::numeric;
using namespace interval_lib;
typedef interval<float> I;

#ifdef TRACKING
extern float_interval_t* tracked;
static float_interval_t previous;

bool initialized = false;

void 
check()
{
    if (tracked == NULL) return;
    if (!initialized) {
        previous = *tracked;
        initialized = true;
    }
    if (tracked->lower == previous.lower && tracked->upper == previous.upper) {
        return;
    }
    printf("Change detected: %a, %a\n", previous.lower, previous.upper);
    previous = *tracked;
}

#endif

#ifdef WIDE
void 
check_wide(float_interval_t in)
{
    if (in.lower < in.upper) {
        printf("Wide interval detected: [%a, %a]\n", in.lower, in.upper);
    }
}
#endif

I 
struct_to_interval(float_interval_t in)
{
    I result(in.lower, in.upper);
    return result;
}

float_interval_t 
interval_to_struct(I in)
{
    float_interval_t result = {in.lower(), in.upper()};
    return result;
}

float_interval_t 
add_float_interval(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return interval_to_struct(struct_to_interval(in1) + struct_to_interval(in2));
}

float_interval_t 
sub_float_interval(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return interval_to_struct(struct_to_interval(in1) - struct_to_interval(in2));
}

float_interval_t 
mul_float_interval(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return interval_to_struct(struct_to_interval(in1) * struct_to_interval(in2));
}

float_interval_t 
div_float_interval(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return interval_to_struct(struct_to_interval(in1) / struct_to_interval(in2));
}

//

float_interval_t 
plus_float_interval(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    return interval_to_struct(+(struct_to_interval(in)));
}

float_interval_t 
minus_float_interval(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    return interval_to_struct(-(struct_to_interval(in)));
}


bool 
equivalent_to_true(float_interval_t in)
{
    return !zero_in(struct_to_interval(in));
}

bool 
equivalent_to_false(float_interval_t in)
{
    return in.lower == 0 && in.upper == 0;
}

bool 
float_interval_to_bool(float_interval_t in)
{
    if (equivalent_to_true(in)) return 1;
    if (equivalent_to_false(in)) return 0;
    assert(false && "Failed to cast interval to bool");
    return 0;
}

bool 
is_singleton(float_interval_t in)
{
    return in.lower == in.upper;
}

int 
not_float_interval(float_interval_t in)
{
    return !float_interval_to_bool(in);
}


float_interval_t 
bitnot_float_interval(float_interval_t in)
{
    assert(false && "Unsupported operator: ~");
    float_interval_t result = {0,0};
    return result;
}

///////////


//this will evaluate both LHS and RHS, it shouldn't be like this
/*
int float_test_and(float_interval_t in1, float_interval_t in2){
    return float_interval_to_bool(in1) && float_interval_to_bool(in2);
}

int float_test_or(float_interval_t in1, float_interval_t in2){
    return float_interval_to_bool(in1) || float_interval_to_bool(in2);
}
*/

int 
float_test_cmpeq(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return struct_to_interval(in1) == struct_to_interval(in2);
}

int 
float_test_cmpne(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return struct_to_interval(in1) != struct_to_interval(in2);
}

int 
float_test_cmpgt(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return struct_to_interval(in1) > struct_to_interval(in2);
}

int 
float_test_cmplt(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return struct_to_interval(in1) < struct_to_interval(in2);
}

int 
float_test_cmple(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return struct_to_interval(in1) <= struct_to_interval(in2);
}

int 
float_test_cmpge(float_interval_t in1, float_interval_t in2)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in1);
    check_wide(in2);
#endif
    return struct_to_interval(in1) >= struct_to_interval(in2);
}


		// Bitwise Ops
int 
float_test_bitand(float_interval_t in1, float_interval_t in2)
{
    assert(false && "Unsupported operator &");
    return 0;
}

int 
float_test_bitor(float_interval_t in1, float_interval_t in2)
{
    assert(false && "Unsupported operator |");
    return 0;
}

int 
float_test_bitxor(float_interval_t in1, float_interval_t in2)
{
    assert(false && "Unsupported operator ^");
    return 0;
}

int 
float_test_lshift(float_interval_t in1, float_interval_t in2)
{
    assert(false && "Unsupported operator <<");
    return 0;
}

int 
float_test_rshift(float_interval_t in1, float_interval_t in2)
{
    assert(false && "Unsupported operator >>");
    return 0;
}


float_interval_t 
char_to_float_interval(int8_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
short_to_float_interval(int16_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
int_to_float_interval(int32_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
long_to_float_interval(int32_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
long_long_to_float_interval(int64_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
uchar_to_float_interval(uint8_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
ushort_to_float_interval(uint16_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
uint_to_float_interval(uint32_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
ulong_to_float_interval(uint32_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

float_interval_t 
ulong_long_to_float_interval(uint64_t x)
{
#ifdef TRACKING
    check();
#endif
    return (float_interval_t){x,x};
}

// casts from interval

int8_t 
float_interval_to_char(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to char");
    return 0;
}

int16_t 
float_interval_to_short(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to short");
    return 0;
}

int32_t 
float_interval_to_int(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to int");
    return 0;
}

int32_t 
float_interval_to_long(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to long");
    return 0;
}

int64_t 
float_interval_to_long_long(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to long long");
    return 0;
}

uint8_t 
float_interval_to_uchar(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to unsigned char");
    return 0;
}

uint16_t 
float_interval_to_ushort(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to unsigned short");
    return 0;
}

uint32_t 
float_interval_to_uint(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to unsigned int");
    return 0;
}

uint32_t 
float_interval_to_ulong(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to unsigned long");
    return 0;
}

uint64_t 
float_interval_to_ulong_long(float_interval_t in)
{
#ifdef TRACKING
    check();
#endif
#ifdef WIDE
    check_wide(in);
#endif
    if (is_singleton(in)) return in.lower;
    assert(false && "Failed to convert interval to unsigned long long");
    return 0;
}


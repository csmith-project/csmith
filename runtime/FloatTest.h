// Extension to CSmith to test floating point optimizations
// Jacek Burys, Imperial College London 2015

#ifndef FLOAT_TEST_H
#define FLOAT_TEST_H

#ifdef FLOAT_TEST_ENABLED

////////////////////////////////////

#include <boots/numeric/interval.hpp>
#define __FLOAT interval<float>


////////////////////////////////////

#else
#define __FLOAT float
#endif

#endif

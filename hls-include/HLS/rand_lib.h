/*  Copyright 1992-2021 Intel Corporation.                                 */
/*                                                                         */
/*  This software and the related documents are Intel copyrighted          */
/*  materials, and your use of them is governed by the express license     */
/*  under which they were provided to you ("License"). Unless the License  */
/*  provides otherwise, you may not use, modify, copy, publish,            */
/*  distribute, disclose or transmit this software or the related          */
/*  documents without Intel's prior written permission.                    */
/*                                                                         */
/*  This software and the related documents are provided as is, with no    */
/*  express or implied warranties, other than those that are expressly     */
/*  stated in the License.                                                 */

///////////////////////////////////////////////////////////////////////////////
//
// Random Number Generator Library
//
// Author: Paul White
///////////////////////////////////////////////////////////////////////////////
// Overview
//
// Use this library to generate pseudorandom numbers from within your HLS
// components. You can generate random integers and floats that follow a 
// uniform distribution, or random floats that follow a Gaussian
// distribution. Floats are selected from the range [0, 1). Integers are 
// selected from the range [-2³¹, 2³¹-1]. 
//
// This component generates and returns a uniform random integer:
//
//   component int foo_int_uniform() {
//     static RNG_Uniform<int> m_rng;
//     return m_rng.rand();
//   }
//
// This component generates and returns a uniform random float:
//
//   component float foo_float_uniform() {
//     static RNG_Uniform<float> m_rng;
//     return m_rng.rand();
//   }
// 
// The Gaussian random number generator can produce random floats using either
// the Central Limit Theorem (CLT) method or the Box-Muller method. The method
// is specified using an `ihc::GaussianMethod` template paramter. If you do not
// specify an `ihc::GaussianMethod` parameter, then the default method is CLT
// (`ihc::GAUSSIAN_CLT`). Random numbers are generated with a mean of 0 and
// a variance of 1. Currently, this class of random number generator can
// produce only floats.
//
// This component uses the CLT method:
//
// 	 component float foo_float_gaussian_clt() {
//     static RNG_Gaussian<float, ihc::GAUSSIAN_CLT> m_rng(SEED_VAL);
//     return m_rng.rand();
//   }
//
// This component uses the Box-Muller method:
//
//   component float foo_float_gaussian_box_muller() {
//     static RNG_Gaussian<float, ihc::GAUSSIAN_BOX_MULLER> m_rng(SEED_VAL);
//     return m_rng.rand();
//   }
//
///////////////////////////////////////////////////////////////////////////////
// 
// Important Notice
// The use of these pseudo random number generator (PRNG) algorithms are not
// recommended for cryptographic purposes. The PRNGs included in this library
// are not Cryptographically Secure Pseudo-Random Number Generators (CSPRNG) and
// should not be used for cryptography. CSPRNG algorithms are designed so that
// no polynomial time algorithm (PTA) can compute or predict the next bit in the
// pseudo-random sequence, nor is there a PTA that can predict past values of
// the CSPRNG; these algorithms do not achieve this purpose. Additionally, these
// algorithms have not been reviewed nor are they recommended for use as a PRNG
// component of a CSPRNG, even if the input values are from a non-deterministic
// entropy source with an appropriate entropy extractor.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _IHC_RAND_LIB
#define _IHC_RAND_LIB
#include "HLS/math.h"

#define RNG_CLT_DELTA_PARAM     0.5f // Delta parameter used in CLT method
#define RNG_CLT_N_PARAM         33   // N parameter used in CLT method
#define RNG_DEFAULT_SEED_VALUE  1    // seed value to use if user does not specify one in a constructor
static const float prob_vec[33] =
{
	0.2786516742639180758323021361857622199454670598784089533768877188e-19f,
	0.9769274243862327463306370984469089366473086219894329693285839933e-17f,
	0.1047975224033549871734315745550826281128160448558429209905040686e-14f,
	0.1036530592944121931953839624560807109783368048362354895983233072e-12f,
	0.7376468431626166978657893006657302368891927897935610501259457605e-11f,
	0.4082513728588890096242760117784197029753726621043440449872200703e-9f,
	0.1699716724434418208130909887579061506041591595377235464274115530e-7f,
	0.5369925353596465814649929632171846607696122226393481047132358051e-6f,
	0.1273269144280605728244338279854928915243986761708290557744797040e-4f,
	0.0002258175372976356648611610831048831680010912378808846993489405f,
	0.0029652481244236893731625186915085794407555752662725960236423052f,
	0.0285377579837071715348687805700245020101052664101913429536074225f,
	0.1981589313766277837715907890841506367626851724357698815668905995f,
	0.9717621420627986061524017041067552295208252102230837212836539187f,
	0.7417268575098959224256330307431806600151762290367323947132141044f,
	1.0f,
	0.9044268482711402465936209101023957266107341028042274045266220441f,
	0.9469944047790697134567371885847534802396262614536561884470048097f,
	0.4699489922927155763039623991085833144411765358639483599187887193f,
	0.9717621420627986061524017041067552295208252102230837212836539187f,
	0.1981589313766277837715907890841506367626851724357698815668905995f,
	0.0285377579837071715348687805700245020101052664101913429536074225f,
	0.0029652481244236893731625186915085794407555752662725960236423052f,
	0.0002258175372976356648611610831048831680010912378808846993489405f,
	0.1273269144280605728244338279854928915243986761708290557744797040e-4f,
	0.5369925353596465814649929632171846607696122226393481047132358051e-6f,
	0.1699716724434418208130909887579061506041591595377235464274115530e-7f,
	0.4082513728588890096242760117784197029753726621043440449872200703e-9f,
	0.7376468431626166978657893006657302368891927897935610501259457605e-11f,
	0.1036530592944121931953839624560807109783368048362354895983233072e-12f,
	0.1047975224033549871734315745550826281128160448558429209905040686e-14f,
	0.9769274243862327463306370984469089366473086219894329693285839933e-17f,
	0.2786516742639180758323021361857622199454670598784089533768877188e-19f
};
static const int alias_vec[33] = { 16, 16, 17, 17, 16, 15, 15, 16, 18, 17, 18,
		17, 16, 15, 17, -1, 15, 15, 14, 15, 18, 15, 14, 16, 15, 14, 17, 16, 17,
		15, 16, 15, 16 };
static const char LogTable256[256] = {
		-1,
		0,
		1, 1,
		2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3,
	    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

///////////////////////////////////////////////////////////////////////////
// Enumerations for the floating point RNG:
//   Distribution type:
//     - Uniform
//     - Gaussian
//   The method additionally specifies the algorithm used to generate the
//   gaussian distribution and two options are available:
//     - CLT
//     - Box Muller
////////////////////////////////////////////////////////////////////////////

namespace ihc {

enum GaussianMethod {
	GAUSSIAN_CLT, GAUSSIAN_BOX_MULLER
};
}
///////////////////////////////////////////////////////////////////////////////
// Helper Function:
//   These functions are utilized by the different variants of the random
//   number generator.
///////////////////////////////////////////////////////////////////////////////
unsigned int randi(unsigned int &a, unsigned int &b, unsigned int &c);
unsigned int msb(unsigned int v);
unsigned int lod(unsigned int u0_int);
float fx_to_fp(unsigned int e_int, unsigned int m_int);


///////////////////////////////////////////////////////////////////////////////
// Default RNG_Uniform Class
// This will not be invoked, the RNG should only run for one of the template
// specializations (as defined below).
///////////////////////////////////////////////////////////////////////////////

template<typename T>
class RNG_Uniform
{
public:
	RNG_Uniform() = 0;
	RNG_Uniform(const int seed) = 0;
};



///////////////////////////////////////////////////////////////////////////////
// Integer RNG_Uniform Specialization
// This class has two public methods, the constructor and the `rand()` function.
//
// For use inside a component, instantiate as a static member object and call
// `rand()` whenever needed. Each instance of the `rand()` function call will
// generate its own piece of hardware with a common seed and state variables.
///////////////////////////////////////////////////////////////////////////////
template<>
class RNG_Uniform<int> {
public:
	RNG_Uniform() {
		s1 = RNG_DEFAULT_SEED_VALUE + 1;
		s2 = RNG_DEFAULT_SEED_VALUE + 2;
		s3 = RNG_DEFAULT_SEED_VALUE + 3;
	}
	
	RNG_Uniform(const int seed) {
		s1 = seed + 1;
		s2 = seed + 2;
		s3 = seed + 3;
	}
	int rand();

private:
	// Internal state variables
	unsigned int s1, s2, s3;

private:
	// Private friend functions
	friend unsigned int randi(unsigned int &a, unsigned int &b,
			unsigned int &c);
};

///////////////////////////////////////////////////////////////////////////////
// Integer RNG_Uniform `rand()` method:
//   This method will generate a pseudo random 32-bit integer
///////////////////////////////////////////////////////////////////////////////
int RNG_Uniform<int>::rand() {
	return randi(s1, s2, s3);
}

///////////////////////////////////////////////////////////////////////////////
// Single-Precision Floating point RNG_Uniform Specialization
// This class has two public methods, the constructor and the `rand()` function.
//
// For use inside a component, instantiate as a static member object and call
// `rand()` whenever needed. Each instance of the `rand()` function call will
// generate its own piece of hardware with a common seed and state variables.
///////////////////////////////////////////////////////////////////////////////
template<>
class RNG_Uniform<float> {
public:
	RNG_Uniform() {
		e0_s1 = RNG_DEFAULT_SEED_VALUE + 1;
		e0_s2 = RNG_DEFAULT_SEED_VALUE + 2;
		e0_s3 = RNG_DEFAULT_SEED_VALUE + 3;
		m0_s1 = RNG_DEFAULT_SEED_VALUE + 4;
		m0_s2 = RNG_DEFAULT_SEED_VALUE + 5;
		m0_s3 = RNG_DEFAULT_SEED_VALUE + 6;
	}
	RNG_Uniform(const int seed) {
		e0_s1 = seed + 1;
		e0_s2 = seed + 2;
		e0_s3 = seed + 3;
		m0_s1 = seed + 4;
		m0_s2 = seed + 5;
		m0_s3 = seed + 6;
	}
	float rand();

private:

	// Floating point state
	unsigned int e0_s1, e0_s2, e0_s3;
	unsigned int m0_s1, m0_s2, m0_s3;

	// Private functions
	void RNG_uniform_float_init();
	float RND_uniform();

	// Friend functions
	friend unsigned int randi(unsigned int &a, unsigned int &b,
			unsigned int &c);
	friend float fx_to_fp(unsigned int e_int, unsigned int m_int);
};

///////////////////////////////////////////////////////////////////////////////
// Single-precision Floating-Point RNG_Uniform `rand()` method:
//   This method will generate a pseudo random 32-bit floating-point number
///////////////////////////////////////////////////////////////////////////////
float RNG_Uniform<float>::rand() {
	unsigned int e0_int = randi(e0_s1, e0_s2, e0_s3);
	unsigned int m0_int = randi(m0_s1, m0_s2, m0_s3);

	float rand_out = fx_to_fp(e0_int, m0_int);

	return rand_out;
}

///////////////////////////////////////////////////////////////////////////////
// Default RNG_Gaussian Class
// This will not be invoked, the RNG should only run for one of the template
// specializations (as defined below).
///////////////////////////////////////////////////////////////////////////////
template<typename T, ihc::GaussianMethod m_method = ihc::GAUSSIAN_CLT>
class RNG_Gaussian
{
public:
	RNG_Gaussian() = 0;
	RNG_Gaussian(const int seed) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Single Precision Floating Point RNG Gaussian Specialization
// This class has two public methods, the constructor and the `rand()`
// function.
//
// For use inside a component, instantiate as a static member object and call
// `rand()` whenever needed. Each instance of the `rand()` function call will
// generate its own piece of hardware with a common seed and state variables.
//
// Note that this requires a lot more state tom_dist,  be carried arounm_dist, d than the
// integer specialization, and has additional helper functions. This class
// should be easily extensible to support alternate distributions and methods
// of generation.
////////////////////////////////////////////////////////////////////////////////
template<>
class RNG_Gaussian<float, ihc::GAUSSIAN_CLT> {
public:
	RNG_Gaussian() { 
		e0_s1    = RNG_DEFAULT_SEED_VALUE + 1;
		e0_s2    = RNG_DEFAULT_SEED_VALUE + 2;
		e0_s3    = RNG_DEFAULT_SEED_VALUE + 3;
		m0_s1    = RNG_DEFAULT_SEED_VALUE + 4;
		m0_s2    = RNG_DEFAULT_SEED_VALUE + 5;
		m0_s3    = RNG_DEFAULT_SEED_VALUE + 6;
		e1_s1    = RNG_DEFAULT_SEED_VALUE + 7;
		e1_s2    = RNG_DEFAULT_SEED_VALUE + 8;
		e1_s3    = RNG_DEFAULT_SEED_VALUE + 9;
		m1_s1    = RNG_DEFAULT_SEED_VALUE + 10;
		m1_s2    = RNG_DEFAULT_SEED_VALUE + 11;
		m1_s3    = RNG_DEFAULT_SEED_VALUE + 12;
		u0_s1_c0 = RNG_DEFAULT_SEED_VALUE + 123424;
		u0_s2_c0 = RNG_DEFAULT_SEED_VALUE + 146775;
		u0_s3_c0 = RNG_DEFAULT_SEED_VALUE + 94345 ;
		u1_s1_c0 = RNG_DEFAULT_SEED_VALUE + 146565;
		u1_s2_c0 = RNG_DEFAULT_SEED_VALUE + 764678;
		u1_s3_c0 = RNG_DEFAULT_SEED_VALUE + 16774 ;
		u0_s1_c1 = RNG_DEFAULT_SEED_VALUE + 134675;
		u0_s2_c1 = RNG_DEFAULT_SEED_VALUE + 745455;
		u0_s3_c1 = RNG_DEFAULT_SEED_VALUE + 456713;
		u1_s1_c1 = RNG_DEFAULT_SEED_VALUE + 356495;
		u1_s2_c1 = RNG_DEFAULT_SEED_VALUE + 546477;
		u1_s3_c1 = RNG_DEFAULT_SEED_VALUE + 546789;
		u0_s1_c2 = RNG_DEFAULT_SEED_VALUE + 465373;
		u0_s2_c2 = RNG_DEFAULT_SEED_VALUE + 564316;
		u0_s3_c2 = RNG_DEFAULT_SEED_VALUE + 65734 ;
		u1_s1_c2 = RNG_DEFAULT_SEED_VALUE + 446457;
		u1_s2_c2 = RNG_DEFAULT_SEED_VALUE + 974616;
		u1_s3_c2 = RNG_DEFAULT_SEED_VALUE + 46789 ;
		u0_s1_c3 = RNG_DEFAULT_SEED_VALUE + 475564;
		u0_s2_c3 = RNG_DEFAULT_SEED_VALUE + 75643 ;
		u0_s3_c3 = RNG_DEFAULT_SEED_VALUE + 456789;
		u1_s1_c3 = RNG_DEFAULT_SEED_VALUE + 896465;
		u1_s2_c3 = RNG_DEFAULT_SEED_VALUE + 94976 ;
		u1_s3_c3 = RNG_DEFAULT_SEED_VALUE + 794564;
		u0_s1_c4 = RNG_DEFAULT_SEED_VALUE + 687974;
		u0_s2_c4 = RNG_DEFAULT_SEED_VALUE + 354679;
		u0_s3_c4 = RNG_DEFAULT_SEED_VALUE + 789643;
		u1_s1_c4 = RNG_DEFAULT_SEED_VALUE + 499746;
		u1_s2_c4 = RNG_DEFAULT_SEED_VALUE + 453167;
		u1_s3_c4 = RNG_DEFAULT_SEED_VALUE + 649797;
		u0_s1_c5 = RNG_DEFAULT_SEED_VALUE + 231346;
		u0_s2_c5 = RNG_DEFAULT_SEED_VALUE + 46575 ;
		u0_s3_c5 = RNG_DEFAULT_SEED_VALUE + 879463;
		u1_s1_c5 = RNG_DEFAULT_SEED_VALUE + 456797;
		u1_s2_c5 = RNG_DEFAULT_SEED_VALUE + 67456 ;
		u1_s3_c5 = RNG_DEFAULT_SEED_VALUE + 543456;
	}
	RNG_Gaussian(int seed) {
		e0_s1    = seed + 1;
		e0_s2    = seed + 2;
		e0_s3    = seed + 3;
		m0_s1    = seed + 4;
		m0_s2    = seed + 5;
		m0_s3    = seed + 6;
		e1_s1    = seed + 7;
		e1_s2    = seed + 8;
		e1_s3    = seed + 9;
		m1_s1    = seed + 10;
		m1_s2    = seed + 11;
		m1_s3    = seed + 12;
		u0_s1_c0 = seed + 123424;
		u0_s2_c0 = seed + 146775;
		u0_s3_c0 = seed + 94345 ;
		u1_s1_c0 = seed + 146565;
		u1_s2_c0 = seed + 764678;
		u1_s3_c0 = seed + 16774 ;
		u0_s1_c1 = seed + 134675;
		u0_s2_c1 = seed + 745455;
		u0_s3_c1 = seed + 456713;
		u1_s1_c1 = seed + 356495;
		u1_s2_c1 = seed + 546477;
		u1_s3_c1 = seed + 546789;
		u0_s1_c2 = seed + 465373;
		u0_s2_c2 = seed + 564316;
		u0_s3_c2 = seed + 65734 ;
		u1_s1_c2 = seed + 446457;
		u1_s2_c2 = seed + 974616;
		u1_s3_c2 = seed + 46789 ;
		u0_s1_c3 = seed + 475564;
		u0_s2_c3 = seed + 75643 ;
		u0_s3_c3 = seed + 456789;
		u1_s1_c3 = seed + 896465;
		u1_s2_c3 = seed + 94976 ;
		u1_s3_c3 = seed + 794564;
		u0_s1_c4 = seed + 687974;
		u0_s2_c4 = seed + 354679;
		u0_s3_c4 = seed + 789643;
		u1_s1_c4 = seed + 499746;
		u1_s2_c4 = seed + 453167;
		u1_s3_c4 = seed + 649797;
		u0_s1_c5 = seed + 231346;
		u0_s2_c5 = seed + 46575 ;
		u0_s3_c5 = seed + 879463;
		u1_s1_c5 = seed + 456797;
		u1_s2_c5 = seed + 67456 ;
		u1_s3_c5 = seed + 543456;
	}
	float rand()
	{
		return RND_CLT_get();
	}

private:
	// Floating point state
	unsigned int e0_s1, e0_s2, e0_s3;
	unsigned int e1_s1, e1_s2, e1_s3;
	unsigned int m0_s1, m0_s2, m0_s3;
	unsigned int m1_s1, m1_s2, m1_s3;

	// CLT Extra States
	unsigned int u0_s1_c0, u0_s2_c0, u0_s3_c0;
	unsigned int u1_s1_c0, u1_s2_c0, u1_s3_c0;
	unsigned int u0_s1_c1, u0_s2_c1, u0_s3_c1;
	unsigned int u1_s1_c1, u1_s2_c1, u1_s3_c1;
	unsigned int u0_s1_c2, u0_s2_c2, u0_s3_c2;
	unsigned int u1_s1_c2, u1_s2_c2, u1_s3_c2;
	unsigned int u0_s1_c3, u0_s2_c3, u0_s3_c3;
	unsigned int u1_s1_c3, u1_s2_c3, u1_s3_c3;
	unsigned int u0_s1_c4, u0_s2_c4, u0_s3_c4;
	unsigned int u1_s1_c4, u1_s2_c4, u1_s3_c4;
	unsigned int u0_s1_c5, u0_s2_c5, u0_s3_c5;
	unsigned int u1_s1_c5, u1_s2_c5, u1_s3_c5;

	// Private functions
	float RND_CLT_get();

	// Friend functions
	friend unsigned int randi(unsigned int &a, unsigned int &b,
			unsigned int &c);
	friend unsigned int msb(unsigned int v);
	friend unsigned int lod(unsigned int u0_int);
	friend float fx_to_fp(unsigned int e_int, unsigned int m_int);
};

template<>
class RNG_Gaussian<float, ihc::GAUSSIAN_BOX_MULLER> {
public:
	RNG_Gaussian() {
		u0_f0_s1 = RNG_DEFAULT_SEED_VALUE + 1;
		u0_f0_s2 = RNG_DEFAULT_SEED_VALUE + 2;
		u0_f0_s3 = RNG_DEFAULT_SEED_VALUE + 3;
		u1_f0_s1 = RNG_DEFAULT_SEED_VALUE + 4;
		u1_f0_s2 = RNG_DEFAULT_SEED_VALUE + 5;
		u1_f0_s3 = RNG_DEFAULT_SEED_VALUE + 6;
		u0_f1_s1 = RNG_DEFAULT_SEED_VALUE + 7;
		u0_f1_s2 = RNG_DEFAULT_SEED_VALUE + 8;
		u0_f1_s3 = RNG_DEFAULT_SEED_VALUE + 9;
		u1_f1_s1 = RNG_DEFAULT_SEED_VALUE + 10;
		u1_f1_s2 = RNG_DEFAULT_SEED_VALUE + 11;
		u1_f1_s3 = RNG_DEFAULT_SEED_VALUE + 12;	
	}
	RNG_Gaussian(int seed) { 
		u0_f0_s1 = seed + 1;
		u0_f0_s2 = seed + 2;
		u0_f0_s3 = seed + 3;
		u1_f0_s1 = seed + 4;
		u1_f0_s2 = seed + 5;
		u1_f0_s3 = seed + 6;
		u0_f1_s1 = seed + 7;
		u0_f1_s2 = seed + 8;
		u0_f1_s3 = seed + 9;
		u1_f1_s1 = seed + 10;
		u1_f1_s2 = seed + 11;
		u1_f1_s3 = seed + 12;	
	}
	float rand()
	{
		return RND_box_muller();
	}

private:

	// Box Muller Extra States
	unsigned int u0_f0_s1, u0_f0_s2, u0_f0_s3;
	unsigned int u1_f0_s1, u1_f0_s2, u1_f0_s3;
	unsigned int u0_f1_s1, u0_f1_s2, u0_f1_s3;
	unsigned int u1_f1_s1, u1_f1_s2, u1_f1_s3;

	// Private functions
	float RND_box_muller();
	float box_muller_helper(float u0, float u1);

	// Friend functions
	friend unsigned int randi(unsigned int &a, unsigned int &b,
			unsigned int &c);
	friend unsigned int msb(unsigned int v);
	friend unsigned int lod(unsigned int u0_int);
	friend float fx_to_fp(unsigned int e_int, unsigned int m_int);
};


///////////////////////////////////////////////////////////////////////////////
// `rand()` method helper for floating point Gaussian CLT RNG
///////////////////////////////////////////////////////////////////////////////
float RNG_Gaussian<float, ihc::GAUSSIAN_CLT>::RND_CLT_get() {
	// random number y
	unsigned int e0_int = randi(e0_s1, e0_s2, e0_s3);
	unsigned int m0_int = randi(m0_s1, m0_s2, m0_s3);
	float y = fx_to_fp(e0_int, m0_int);

	// random number i
	unsigned int e1_int = randi(e1_s1, e1_s2, e1_s3);
	unsigned int m1_int = randi(m1_s1, m1_s2, m1_s3);
	unsigned int i = floorf(fx_to_fp(e1_int, m1_int) * RNG_CLT_N_PARAM);

	// select component from the alias table
	int comp_id;
	if (y < prob_vec[i]) {
		comp_id = i;
	} else {
		comp_id = alias_vec[i];
	}

	// Component 1
	unsigned int u0_int_c0 = randi(u0_s1_c0, u0_s2_c0, u0_s3_c0);
	unsigned int u1_int_c0 = randi(u1_s1_c0, u1_s2_c0, u1_s3_c0);
	float rand_out = fx_to_fp(u0_int_c0, u1_int_c0);

	// Component 2
	unsigned int u0_int_c1 = randi(u0_s1_c1, u0_s2_c1, u0_s3_c1);
	unsigned int u1_int_c1 = randi(u1_s1_c1, u1_s2_c1, u1_s3_c1);
	rand_out -= fx_to_fp(u0_int_c1, u1_int_c1);

	// Component 3
	unsigned int u0_int_c2 = randi(u0_s1_c2, u0_s2_c2, u0_s3_c2);
	unsigned int u1_int_c2 = randi(u1_s1_c2, u1_s2_c2, u1_s3_c2);
	rand_out += fx_to_fp(u0_int_c2, u1_int_c2);

	// Component 4
	unsigned int u0_int_c3 = randi(u0_s1_c3, u0_s2_c3, u0_s3_c3);
	unsigned int u1_int_c3 = randi(u1_s1_c3, u1_s2_c3, u1_s3_c3);
	rand_out -= fx_to_fp(u0_int_c3, u1_int_c3);

	// Component 5
	unsigned int u0_int_c4 = randi(u0_s1_c4, u0_s2_c4, u0_s3_c4);
	unsigned int u1_int_c4 = randi(u1_s1_c4, u1_s2_c4, u1_s3_c4);
	rand_out += fx_to_fp(u0_int_c4, u1_int_c4);

	// Component 6
	unsigned int u0_int_c5 = randi(u0_s1_c5, u0_s2_c5, u0_s3_c5);
	unsigned int u1_int_c5 = randi(u1_s1_c5, u1_s2_c5, u1_s3_c5);
	rand_out -= fx_to_fp(u0_int_c5, u1_int_c5);

	rand_out += (comp_id - (RNG_CLT_N_PARAM - 1) / 2) * RNG_CLT_DELTA_PARAM;

	return rand_out;
}

///////////////////////////////////////////////////////////////////////////////
// `rand()` method helper for Gaussian Box Muller floating point RNG
///////////////////////////////////////////////////////////////////////////////
float RNG_Gaussian<float, ihc::GAUSSIAN_BOX_MULLER>::RND_box_muller() {
	unsigned int u0_f0_int = randi(u0_f0_s1, u0_f0_s2, u0_f0_s3);
	unsigned int u1_f0_int = randi(u1_f0_s1, u1_f0_s2, u1_f0_s3);
	float f0_fp = fx_to_fp(u0_f0_int, u1_f0_int);

	unsigned int u0_f1_int = randi(u0_f1_s1, u0_f1_s2, u0_f1_s3);
	unsigned int u1_f1_int = randi(u1_f1_s1, u1_f1_s2, u1_f1_s3);
	float f1_fp = fx_to_fp(u0_f1_int, u1_f1_int);

	float rand_out = box_muller_helper(f0_fp, f1_fp);

	return rand_out;
}

///////////////////////////////////////////////////////////////////////////////
// Box-Muller algorithm
///////////////////////////////////////////////////////////////////////////////
float RNG_Gaussian<float, ihc::GAUSSIAN_BOX_MULLER>::box_muller_helper(float u0, float u1) {

	float e = -2.0f * logf(u0);
	float f = sqrtf(e);
	float g0 = sinf(
			2.0f * u1 * 3.141592653589793238462643383279502884197169399375105f); // NOTE: M_PI is in math.h but it is a double, not a float.
	float x0 = f * g0;

	return x0;
}

/////////////////////////
// FRIEND FUNCTIONS
/////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Random integer generator
///////////////////////////////////////////////////////////////////////////////
unsigned int randi(unsigned int &a, unsigned int &b, unsigned int &c) {
	unsigned int t;
	// Generates 32-bit integers.
	t = (((a << 13) ^ a) >> 19);
	a = (((a & 4294967294) << 12) ^ t);
	t = (((b << 2) ^ b) >> 25);
	b = (((b & 4294967288) << 4) ^ t);
	t = (((c << 3) ^ c) >> 11);
	c = (((c & 4294967280) << 17) ^ t);
	return (a ^ b ^ c);
}

///////////////////////////////////////////////////////////////////////////////
// Find the Most Significant Bit
///////////////////////////////////////////////////////////////////////////////
unsigned int msb(unsigned int v) {
	unsigned int r;     // r will be lg(v)
	unsigned int t, tt; // temporaries

	if ((tt = v >> 16)) {
		r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt]; // intentional assignment
	} else {
		r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];         // intentional assignment
	}
	return (v == 0) ? 0 : (r + 1);
}

///////////////////////////////////////////////////////////////////////////////
// Leading One Detector
///////////////////////////////////////////////////////////////////////////////
unsigned int lod(unsigned int u0_int) {
	unsigned int temp_mask = 1;
	unsigned int lo_pos = 0;
	lo_pos = msb(u0_int);

	return lo_pos;
}

///////////////////////////////////////////////////////////////////////////////
// Combines two fixed point random numbers into a floating point number
///////////////////////////////////////////////////////////////////////////////
float fx_to_fp(unsigned int e_int, unsigned int m_int) {
	float fp_rand;
	unsigned int eu = lod(e_int) + 94;             // find the leading one's position, use as the exponent
	unsigned int temp = (eu << 23) ^ (m_int >> 9); // combine the sign bit '0', the exponent, and the fraction
	fp_rand = *(float *) (&temp);
	return fp_rand;
}

#endif // _IHC_RAND_LIB

// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2014, 2015, 2016, 2017 The University of Utah
// All rights reserved.
//
// This file is part of `csmith', a random generator of C programs.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "Constant.h"
#include <cassert>
#include <ostream>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <climits>

#include "CGContext.h"
#include "Type.h"
#include "Variable.h"
#include "Error.h"
#include "CGOptions.h"
#include "StringUtils.h"
#include "random.h"

using namespace std;

static string GenerateRandomConstant(const Type* type);

///////////////////////////////////////////////////////////////////////////////

/*
 * TODO: make well-known constants
 */

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
Constant::Constant(const Type *t, const string &v)
	: Expression(eConstant),
	  type(t),
	  value(v)
{
}

/*
 *
 */
Constant::Constant(const Constant &c)
	: Expression(eConstant),
	  type(c.type),
	  value(c.value)
{
}

/*
 *
 */
Constant::~Constant(void)
{
	// Nothing else to do.
}

///////////////////////////////////////////////////////////////////////////////

Expression *
Constant::clone() const
{
	return new Constant(*this);
}

// --------------------------------------------------------------
static string
GenerateRandomCharConstant(void)
{
	string ch;
	if (CGOptions::ccomp() || !CGOptions::longlong())
		ch = string("0x") + RandomHexDigits(2);
	else
		ch = string("0x") + RandomHexDigits(2) + "L";
	return ch;
}

// --------------------------------------------------------------
static string
GenerateRandomIntConstant(void)
{
	string val;
	// Int constant - Max 8 Hex digits on 32-bit platforms
	if (CGOptions::ccomp() || !CGOptions::longlong())
		val = "0x" + RandomHexDigits( 8 );
	else
		val = "0x" + RandomHexDigits( 8 ) + "L";

	return val;
}

// --------------------------------------------------------------
static string
GenerateRandomShortConstant(void)
{
	string val;
	// Short constant - Max 4 Hex digits on 32-bit platforms
	if (CGOptions::ccomp() || !CGOptions::longlong())
		val = "0x" + RandomHexDigits( 4 );
	else
		val = "0x" + RandomHexDigits( 4 ) + "L";

	return val;
}

// --------------------------------------------------------------
static string
GenerateRandomLongConstant(void)
{
	string val;
	// Long constant - Max 8 Hex digits on 32-bit platforms
	if (!CGOptions::longlong())
		val = "0x" + RandomHexDigits( 8 );
	else
		val = "0x" + RandomHexDigits( 8 ) + "L";
	return val;
}

// --------------------------------------------------------------
static string
GenerateRandomLongLongConstant(void)
{
	// Long constant - Max 8 Hex digits on 32-bit platforms
	string val = "0x" + RandomHexDigits( 16 ) + "LL";
	return val;
}

// --------------------------------------------------------------
#if 0
static string
GenerateRandomFloatConstant(void)
{
	string val = RandomDigits(5) + "." + RandomDigits(5);
	return val;
}
#endif // 0

/*
 * Generate hexadecimal floating point constants [0xF.FFFFFFp-99, 0xF.FFFFFFp+99]
 */
static string
GenerateRandomFloatHexConstant(void)
{
	int exp = pure_rnd_upto(100);
	ostringstream oss;
	oss << "0x" << RandomHexDigits(1) << "." << RandomHexDigits(6) << "p";

	if (pure_rnd_flipcoin(50)) {
		oss << "+";
	}
	else {
		oss << "-";
	}
	oss << exp;
	return oss.str();
}

/*
 * Generate small hexadecimal floating point constants
 */
static string
GenerateSmallRandomFloatHexConstant(int num)
{
	ostringstream oss;
	if (num >= 0) {
		oss << "0x";
	}
	else {
		oss << "-0x";
		num = -num;
	}
	oss << num << "." << RandomHexDigits(1) << "p";

	if (pure_rnd_flipcoin(50)) {
		oss << "+1";
	}
	else {
		oss << "-1";
	}
	return oss.str();
}

static string
GenerateRandomConstantInRange(const Type* type, int bound)
{
	assert(type->eType == eSimple);

	ostringstream oss;
	if (type->simple_type == eInt) {
		int b = static_cast<int>(pow(2, static_cast<double>(bound) / 2));
		int num = pure_rnd_upto(b);
		ERROR_GUARD("");
		bool flag = pure_rnd_flipcoin(50);
		ERROR_GUARD("");
		if (flag)
			oss << num;
		else
			oss << "-" << num;
	}
	else if (type->simple_type == eUInt) {
		int b = static_cast<int>(pow(2, static_cast<double>(bound) / 2));
		if (b < 0)
			b = INT_MAX;
		int num = pure_rnd_upto(b);
		ERROR_GUARD("");
		oss << num;
	}
	else {
		assert(0);
	}
	return CGOptions::mark_mutable_const() ? "(" + oss.str() + ")" : oss.str();
}

// --------------------------------------------------------------
  /* generate a constant struct in the form of
   * "{2, 4, {2, 4}, 6.0}"
   *************************************************************/
static string
GenerateRandomStructConstant(const Type* type)
{
	string value = "{";
	size_t i;
	assert(type->eType == eStruct);
	assert(type->fields.size() == type->bitfields_length_.size());

	for (i = 0; i < type->fields.size(); i++) {
		bool is_bitfield = type->is_bitfield(i);
		if (is_bitfield) {
			int bound = type->bitfields_length_[i];
			if (bound == 0)
				continue;
			string v = GenerateRandomConstantInRange(type->fields[i], bound);
			ERROR_GUARD("");
			if (i > 0) {
				value += ",";
			}
			value += v;
		}
		else {
        		string v = GenerateRandomConstant(type->fields[i]);
			ERROR_GUARD("");
        		if (i > 0) {
            			value += ",";
			}
        		value += v;
		}
	}
	value += "}";
	return value;
}

// --------------------------------------------------------------
  /* generate an union initializer: unlike struct, initializing
     the first field is enough
   *************************************************************/
static string
GenerateRandomUnionConstant(const Type* type)
{
	string value = "{";
	assert(type->eType == eUnion && type->fields.size() == type->bitfields_length_.size());
	value += GenerateRandomConstant(type->fields[0]);
	value += "}";
	return value;
}

static string
GenerateRandomConstant(const Type* type)
{
	string v;
	if (type == 0) {
		v = "0";
	}
	else if (type->eType == eStruct) {
		v = GenerateRandomStructConstant(type);
		ERROR_GUARD("");
	}
	else if (type->eType == eUnion) {
		v = GenerateRandomUnionConstant(type);
		ERROR_GUARD("");
	}
	// the only possible constant for a pointer is "0"
	else if (type->eType == ePointer) {
		v = "0";
	}
	else if (type->eType == eSimple) {
		eSimpleType st = type->simple_type;
		assert(st != eVoid);
		//assert((eType >= 0) && (eType <= MAX_SIMPLE_TYPES));
		if (pure_rnd_flipcoin(50)) {
			ERROR_GUARD("");
			int num = 0;
			if (pure_rnd_flipcoin(50)) {
				ERROR_GUARD("");
				num = pure_rnd_upto(3)-1;
			} else {
				ERROR_GUARD("");
				num = pure_rnd_upto(20)-10;
			}
			// don't use negative number for unsigned type, as this causes
			//trouble for some static analyzers
			ostringstream oss;
			switch (st) {
			case eUChar:
				oss << (unsigned int)(unsigned char)num;
				break;
			case eUShort:
				oss << (unsigned short)num;
				break;
			case eUInt:
				oss << (unsigned int)num;
				break;
			case eULong:
			case eULongLong:
				if (!CGOptions::longlong()) {
					oss << (unsigned int)num;
				} else {
					oss << ((type->simple_type == eULong) ? (unsigned long)num : (unsigned INT64)num);
				}
				break;
			case eFloat:
				oss << GenerateSmallRandomFloatHexConstant(num);
				break;
			default:
				oss << num;
				break;
			}
			if (type->simple_type == eFloat) {
				v = oss.str();
			}
			else {
				if (CGOptions::ccomp() || !CGOptions::longlong())
					v = oss.str() + (type->is_signed() ? "" : "U");
				else
					v = oss.str() + (type->is_signed() ? "L" : "UL");
			}
		} else {
		    switch (st) {
			case eVoid:      v = "/* void */";				break;
			case eChar:      v = GenerateRandomCharConstant();		break;
			case eInt:       v = GenerateRandomIntConstant();		break;
			case eShort:     v = GenerateRandomShortConstant();		break;
			case eLong:      v = GenerateRandomLongConstant();		break;
			case eLongLong:  v = GenerateRandomLongLongConstant();		break;
			case eUChar:     v = GenerateRandomCharConstant();		break;
			case eUInt:      v = GenerateRandomIntConstant();		break;
			case eUShort:    v = GenerateRandomShortConstant();		break;
			case eULong:     v = GenerateRandomLongConstant();		break;
			case eULongLong: v = GenerateRandomLongLongConstant();		break;
			case eFloat:     v = GenerateRandomFloatHexConstant();		break;
			// case eDouble:    v = GenerateRandomFloatConstant();		break;
			default:
				assert(0 && "Unsupported type!");
		    }
		}
	} else {
		assert(0);  // no support for types other than integers and structs for now
    	}
	return (type->eType == eSimple && CGOptions::mark_mutable_const()) ? "(" + v + ")" : v;
}

// --------------------------------------------------------------
/*
 * Sometimes we need to generate constants outside of any current function,
 * e.g., initializers for global variables
 */
Constant *
Constant::make_random(const Type* type)
{
	string v = GenerateRandomConstant(type);
	ERROR_GUARD(NULL);
	return new Constant(type, v);
}

Constant *
Constant::make_random_upto(unsigned int limit)
{
	ostringstream oss;
	oss << rnd_upto(limit);
	ERROR_GUARD(NULL);
	return new Constant(&Type::get_simple_type(eUInt), oss.str());
}

Constant*
Constant::make_random_nonzero(const Type* type)
{
	string v = GenerateRandomConstant(type);
	ERROR_GUARD(NULL);
	while (StringUtils::str2int(v) == 0) {
		v = GenerateRandomConstant(type);
	}
	return new Constant(type, v);
}

/*
 * Return a `Constant' representing the integer value `v'.  Note that the
 * return value may not be "fresh."
 */
Constant *
Constant::make_int(int v)
{
	// Commented out code for the cache:
	// It's hard for releasint the memory for those cached Constants.
#if 0
	static const int cache_size = 256;
	static bool cache_inited = false;
	static Constant *cache_constants[cache_size];
#endif

	const Type &int_type = Type::get_simple_type(eInt);
	ERROR_GUARD(NULL);

#if 0
	// Initialize our cache of small-number constants.
	if (!cache_inited) {
		cache_inited = true;

		ostringstream oss;
		for (int i = 0; i < cache_size; ++i) {
			oss.str("");
			oss << i;
			cache_constants[i] = new Constant(&int_type, oss.str());
		}
	}

	// Get small numbers out of the cache.
	if ((v >= 0) && (v < cache_size)) {
		return cache_constants[v];
	}
#endif

	// Create fresh constants for values outside of our cache limit.
	ostringstream oss;
	oss << v;
	string s = CGOptions::mark_mutable_const() ? ("(" + oss.str() + ")") : oss.str();
	return new Constant(&int_type, s);
}

bool
Constant::compatible(const Variable *v) const
{
	assert(v);
	if (CGOptions::expand_struct())
		return true;
		//return !v->is_field_var();
	return false;
}

bool
Constant::compatible(const Expression *exp) const
{
	assert(exp);
	return false;
}

bool
Constant::less_than(int num) const
{
	return StringUtils::str2int(value) < num;
}

bool
Constant::not_equals(int num) const
{
	return StringUtils::str2int(value) != num;
}

bool
Constant::equals(int num) const
{
	return StringUtils::str2int(value) == num;
}

string
Constant::get_field(size_t fid) const
{
	vector<string> fields;
	StringUtils::split_string(value, fields, "{},");
	if (fid < fields.size()) {
		return fields[fid];
	}
	return "";
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
const Type &
Constant::get_type(void) const
{
	return *(type);
}

/*
 *
 */
void
Constant::Output(std::ostream &out) const
{
	//enclose negative numbers in parenthesis to avoid syntax errors such as "--8"
	if (!value.empty() && value[0] == '-') {
        output_cast(out);
        out << "(" << value << ")";
	} else if (type->eType == ePointer && equals(0)){
        // don't output cast for NULL:
		if (CGOptions::lang_cpp()) {
			if (CGOptions::cpp11())
				out << "nullptr";
			else
				out << "NULL";
		} else {
			out << "(void*)" << value;
		}
	} else {
        output_cast(out);
        out << value;
	}
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

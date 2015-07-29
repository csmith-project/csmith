// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2014 The University of Utah
// All rights reserved.
//
// Copyright (c) 2015-2016 Huawei Technologies Co., Ltd
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

//
// This file was derived from a random program generator written by Bryan
// Turner.  The attributions in that file was:
//
// Random Program Generator
// Bryan Turner (bryan.turner@pobox.com)
// July, 2005
//

#include "Type.h"
#include "PointerType.h"
#include "AggregateType.h"
#include <sstream>
#include <assert.h>
#include <math.h>
#include "Common.h"
#include "CGOptions.h"
#include "random.h"
#include "Filter.h"
#include "util.h"
#include "Bookkeeper.h"
#include "Probabilities.h"
#include "Enumerator.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
const Type *Type::simple_types[MAX_SIMPLE_TYPES];

Type *Type::void_type = NULL;

// ---------------------------------------------------------------------
// List of all types used in the program
vector<Type *> Type::AllTypes;

///////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------
/* constructor for simple types
 ********************************************************/
Type::Type(eSimpleType simple_type) :
	eType(eSimple),
	simple_type(simple_type),
	used(false)
{
	// Nothing else to do.
}

// --------------------------------------------------------------
Type::Type(void) :
	simple_type(MAX_SIMPLE_TYPES),
	used(false)
{
	// Nothing to do.
}

// --------------------------------------------------------------
Type::~Type(void)
{
	// Nothing to do.
}

// ---------------------------------------------------------------------
const Type &
Type::get_simple_type(eSimpleType st)
{
	static bool inited = false;

	if (!inited) {
		for (int i = 0; i < MAX_SIMPLE_TYPES; ++i) {
			Type::simple_types[i] = 0;
		}
		inited = true;
	}

	if (Type::simple_types[st] == 0) {
		// find if type is in the allTypes already (most likely only "eVoid" is not there)
		for (size_t i=0; i<AllTypes.size(); i++) {
			Type* tt = AllTypes[i];
			if (tt->eType == eSimple && tt->simple_type == st) {
				Type::simple_types[st] = tt;
			}
		}
		if (Type::simple_types[st] == 0) {
			Type *t = new Type(st);
			Type::simple_types[st] = t;
	    		AllTypes.push_back(t);
		}
	}
	return *Type::simple_types[st];
}

const Type *
Type::get_type_from_string(const string &type_string)
{
	if (type_string == "Void") {
		return Type::void_type;
	}
	else if (type_string == "Char") {
		return &Type::get_simple_type(eChar);
	}
	else if (type_string == "UChar") {
		return &Type::get_simple_type(eUChar);
	}
	else if (type_string == "Short") {
		return &Type::get_simple_type(eShort);
	}
	else if (type_string == "UShort") {
		return &Type::get_simple_type(eUShort);
	}
	else if (type_string == "Int") {
		return &Type::get_simple_type(eInt);
	}
	else if (type_string == "UInt") {
		return &Type::get_simple_type(eUInt);
	}
	else if (type_string == "Long") {
		return &Type::get_simple_type(eLong);
	}
	else if (type_string == "ULong") {
		return &Type::get_simple_type(eULong);
	}
	else if (type_string == "Longlong") {
		return &Type::get_simple_type(eLongLong);
	}
	else if (type_string == "ULonglong") {
		return &Type::get_simple_type(eULongLong);
	}
	else if (type_string == "Float") {
		return &Type::get_simple_type(eFloat);
	}

	assert(0 && "Unsupported type string!");
	return NULL;
}

// ---------------------------------------------------------------------
/* return the most commonly used type - integer
 *************************************************************/
const Type *
get_int_type()
{
    return &Type::get_simple_type(eInt);
}

Type*
Type::find_type(const Type* t)
{
    for (size_t i=0; i<AllTypes.size(); i++) {
        if (AllTypes[i] == t) {
            return AllTypes[i];
        }
    }
    return 0;
}

bool
Type::is_const_struct_union() const
{
	return false;
}

bool
Type::is_volatile_struct_union() const
{
	return false;
}

bool
Type::is_bitfield(size_t index) const
{
    return false;
}

bool
Type::has_int_field() const
{
	if (is_int()) return true;
	return false;
}

bool
Type::signed_overflow_possible() const
{
	return eType == eSimple && is_signed() && ((int)SizeInBytes()) >= CGOptions::int_size();
}

const Type*
Type::random_type_from_type(const Type* type, bool no_volatile, bool strict_simple_type)
{
	const Type* t = type;
	if (type == 0) {
		t = no_volatile ? choose_random_nonvoid_nonvolatile() : choose_random_nonvoid();
	}
	if (type->eType == eSimple && !strict_simple_type) {
		t = choose_random_simple();
	}
	if (t->eType == eSimple) {
		assert(t->simple_type != eVoid);
	}
	return t;
}

// ---------------------------------------------------------------------
static bool
MoreTypesProbability(void)
{
	// Always have at least 10 types in the program.
	if (Type::AllTypes.size() < 10)
		return true;
	// by default 50% probability for each additional struct or union type.
	return rnd_flipcoin(MoreStructUnionTypeProb);
}

// ---------------------------------------------------------------------
eSimpleType
Type::choose_random_nonvoid_simple(void)
{
	eSimpleType simple_type;

	simple_type = (eSimpleType) rnd_upto(MAX_SIMPLE_TYPES, SIMPLE_TYPES_PROB_FILTER);

	return simple_type;
}

// ---------------------------------------------------------------------
void
Type::GenerateSimpleTypes(void)
{
    unsigned int st;
    for (st=eChar; st<MAX_SIMPLE_TYPES; st++)
    {
		AllTypes.push_back(new Type((enum eSimpleType)st));
    }
    Type::void_type = new Type((enum eSimpleType)eVoid);
}

// ---------------------------------------------------------------------
void
GenerateAllTypes(void)
{
	Type::GenerateSimpleTypes();
    if (CGOptions::use_struct()) {
        while (MoreTypesProbability()) {
		    Type *ty = AggregateType::make_random_struct_type();
		    Type::AllTypes.push_back(ty);
	    }
    }
	if (CGOptions::use_union()) {
        while (MoreTypesProbability()) {
		    Type *ty = AggregateType::make_random_union_type();
		    Type::AllTypes.push_back(ty);
	    }
    }
}

// ---------------------------------------------------------------------
const Type *
Type::choose_random()
{
	VectorFilter f;
	unsigned int type_index = 0;
	Type *type = 0;
	for( type_index = 0; type_index < AllTypes.size(); type_index++) {
		type = AllTypes[type_index];
		assert(type);
		if (type->eType == eSimple) {
			Filter *filter = SIMPLE_TYPES_PROB_FILTER;
			if(filter->filter(type->simple_type))
				f.add(type_index);
		}
		else if ((type->eType == eStruct) && (!CGOptions::return_structs())) {
			f.add(type_index);
		}
	}
	
	type_index = rnd_upto(AllTypes.size(), &f);
	
	Type *typ = AllTypes[type_index];
	
	if (!typ->used) {
		Bookkeeper::record_type_with_bitfields(typ);
		typ->used = true;
	}
	assert(typ);
	return typ;
}

const Type *
Type::choose_random_nonvoid(void)
{
	VectorFilter f;
	unsigned int type_index = 0;
	Type *type = 0;
	
	for( type_index = 0; type_index < AllTypes.size(); type_index++) {	
		type = AllTypes[type_index];
		
		if (type->simple_type == eVoid)
			f.add(type_index);
		
		if (type->eType == eSimple) {
			Filter *filter = SIMPLE_TYPES_PROB_FILTER;
			if(filter->filter(type->simple_type))
				f.add(type_index);
		}
	}
	
	type_index = rnd_upto(AllTypes.size(), &f);

	Type *typ = AllTypes[type_index];
	
	if (!typ->used) {
		Bookkeeper::record_type_with_bitfields(typ);
		typ->used = true;
	}
	assert(typ);
	return typ;
}

const Type *
Type::choose_random_nonvoid_nonvolatile(void)
{
	VectorFilter f;
	unsigned int type_index = 0;
	Type *type = 0;
	
	for( type_index = 0; type_index < AllTypes.size(); type_index++) {	
		type = AllTypes[type_index];
		
		if (type->simple_type == eVoid)
			f.add(type_index);

		if (type->is_aggregate() && type->is_volatile_struct_union())
			f.add(type_index);

		if ((type->eType == eStruct) && (!CGOptions::arg_structs())) {
			f.add(type_index);
		}

		if ((type->eType == eUnion) && (!CGOptions::arg_unions())) {
			f.add(type_index);
		}

		if (type->eType == eSimple) {
			Filter *filter = SIMPLE_TYPES_PROB_FILTER;
			if(filter->filter(type->simple_type))
				f.add(type_index);
		}
	}
	
	type_index = rnd_upto(AllTypes.size(), &f);

	Type *typ = AllTypes[type_index];
	
	if (!typ->used) {
		Bookkeeper::record_type_with_bitfields(typ);
		typ->used = true;
	}
	assert(typ);
	return typ;
}

// ---------------------------------------------------------------------
const Type *
Type::choose_random_simple(void)
{
    eSimpleType ty = choose_random_nonvoid_simple();
    assert(ty != eVoid);
    return &get_simple_type(ty);
}

bool
Type::is_unamed_padding(size_t index) const
{
	return false;
}

bool
Type::has_bitfields() const
{
	return false;
}

// conservatively assume padding is present in all unpacked structures
// or whenever there is bitfields
bool
Type::has_padding(void) const
{
	return false;
}

bool
Type::is_full_bitfields_struct() const
{
	return false;
}

bool
Type::is_signed(void) const
{
	switch (eType) {
	default:
		return false;

	case eSimple:
		switch (simple_type) {
		case eUChar:
		case eUInt:
		case eUShort:
		case eULong:
		case eULongLong:
			return false;
			break;
		default:
			break;
		}
		break;
	}
	return true;
}

const Type*
Type::to_unsigned(void) const
{
	if (eType == eSimple) {
		switch (simple_type) {
			case eUChar:
			case eUInt:
			case eUShort:
			case eULong:
			case eULongLong:
				return this;
			case eChar: return &get_simple_type(eUChar);
			case eInt: return &get_simple_type(eUInt);
			case eShort: return &get_simple_type(eUShort);
			case eLong: return &get_simple_type(eULong);
			case eLongLong: return &get_simple_type(eULongLong);
			default:
				break;
		}
	}
	return NULL;
}

bool
Type::is_promotable(const Type* t) const
{
    if (eType == eSimple && t->eType == eSimple) {
		eSimpleType t2 = t->simple_type;
		switch (simple_type) {
			case eChar:
			case eUChar: return (t2 != eVoid);
			case eShort:
			case eUShort: return (t2 != eVoid && t2 != eChar && t2 != eUChar);
			case eInt:
			case eUInt: return (t2 != eVoid && t2 != eChar && t2 != eUChar && t2 != eShort && t2 != eUShort);
			case eLong:
			case eULong: return (t2 == eLong || t2 == eULong || t2 == eLongLong || t2 == eULongLong);
			case eLongLong:
			case eULongLong: return (t2 == eLongLong || t2 == eULongLong);
			case eFloat: return (t2 != eVoid);
			default: break;
		}
	}
	return false;
}

// ---------------------------------------------------------------------
/* generally integer types can be converted to any other interger types
 * void / struct / union / array types are not. Pointers depend on the
 * type they point to. (unsigned int*) is convertable to (int*), etc
 *************************************************************/
bool
Type::is_convertable(const Type* t) const
{
    if (this == t)
        return true;
    if (eType == eSimple && t->eType == eSimple) {
		// forbiden conversion from float to int
		if (t->is_float() && !is_float())
			return false;
        if ((simple_type != eVoid && t->simple_type != eVoid) ||
            simple_type == t->simple_type)
            return true;
    }
    return false;
}

// eLong & eInt, eULong & eUInt are equivalent
bool
Type::is_equivalent(const Type* t) const
{
	if(this == t)
		return true;

	if(eType == eSimple) {
		return (is_signed() == t->is_signed()) && (SizeInBytes() == t->SizeInBytes());
	}

	return false;
}

bool
Type::needs_cast(const Type* t) const
{
	return (eType == ePointer) && !get_base_type()->is_equivalent(t->get_base_type());
}

bool
Type::match(const Type* t, enum eMatchType mt) const
{
	switch (mt) {
	case eExact: return (this == t);
	case eConvert: return is_convertable(t);
	case eDereference: return is_dereferenced_from(t);
	case eDerefExact: return (t == this || is_dereferenced_from(t));
	case eFlexible: return is_derivable(t);
	default: break;
	}
	return false;
}

// ---------------------------------------------------------------------
/* return true if this type can be derived from the given type
 * by dereferencing
 *************************************************************/
bool
Type::is_dereferenced_from(const Type* t) const
{
    if (t->eType == ePointer) {
        const Type* pt = (dynamic_cast<const PointerType *>(t))->ptr_type;
        while (pt) {
            if (pt == this) {
                return true;
            }
			pt = (pt->eType == ePointer) ? (dynamic_cast<const PointerType *>(pt))->ptr_type : NULL;
        }
    }
    return false;
}

// ---------------------------------------------------------------------
/* return true if this type can be derived from the given type
 * by taking address(one level) or dereferencing (multi-level)
 * question: allow interger conversion? i.e. char* p = &(int)i?
 *************************************************************/
bool
Type::is_derivable(const Type* t) const
{
    if (this == t) {
        return true;
    }
	return is_convertable(t) || is_dereferenced_from(t);
}

unsigned long
Type::SizeInBytes(void) const
{
	switch (simple_type) {
		case eVoid:		return 0;
		case eInt:		return 4;
		case eShort:	return 2;
		case eChar:		return 1;
		case eLong:		return 4;
		case eLongLong:	return 8;
		case eUChar:	return 1;
		case eUInt:		return 4;
		case eUShort:	return 2;
		case eULong:	return 4;
		case eULongLong:return 8;
		case eFloat:	return 4;
//		case eDouble:	return 8;
	}
	return 0;
}

// --------------------------------------------------------------
 /* Select a left hand type for assignments
  ************************************************************/
const Type *
Type::SelectLType(bool no_volatile, eAssignOps op)
{
	const Type* type = NULL;
	// occasionally we want to play with pointers
	// We haven't implemented pointer arith,
	// so choose pointer types iff we create simple assignment
	// (see Statement::make_random)
	if (op == eSimpleAssign && rnd_flipcoin(PointerAsLTypeProb)) {
		type = PointerType::make_random_pointer_type();
	}

	// choose a struct type as LHS type
	if (!type) {
		vector<AggregateType *> ok_struct_types;
		AggregateType::get_all_ok_struct_union_types(ok_struct_types, true, no_volatile, false, true);
		if ((ok_struct_types.size() > 0) && (op == eSimpleAssign) && rnd_flipcoin(StructAsLTypeProb)) {
			type = AggregateType::choose_random_struct_union_type(ok_struct_types);
		}
	}

	// choose float as LHS type
	if (!type) {
		if (StatementAssign::AssignOpWorksForFloat(op) && rnd_flipcoin(FloatAsLTypeProb)) {
			type = &Type::get_simple_type(eFloat);
		}
	}

	// default is any integer type
	if (!type) {
		type = get_int_type();
	}
	return type;
}

void
Type::get_int_subfield_names(string prefix, vector<string>& names, const vector<int>& excluded_fields) const
{
	if (eType == eSimple) {
		names.push_back(prefix);
	}
}

bool
Type::contain_pointer_field(void) const
{
	if (eType == ePointer) return true;
	return false;
}

// ---------------------------------------------------------------------
void
Type::Output(std::ostream &out) const
{
	if (this->simple_type == eVoid) {
		out << "void";
	} else if (this->simple_type == eFloat) {
	    out << "float";
	} else {
		out << (is_signed() ? "int" : "uint");
		out << (SizeInBytes() * 8);
		out << "_t";
	}
}

void
Type::get_type_sizeof_string(std::string &s) const
{
	ostringstream ss;
	ss << "sizeof(";
	Output(ss);
	ss << ")";
	s = ss.str();
}

/*
 * return the printf directive string for the type. for example, int -> "%d"
 */
std::string
Type::printf_directive(void) const
{
	string ret;
	if (SizeInBytes() >= 8) {
		ret = is_signed() ? "%lld" : "%llu";
	} else {
		ret = is_signed() ? "%d" : "%u";
	}
	return ret;
}

/*
 *
 */
void
Type::doFinalization(void)
{
	vector<Type *>::iterator j;
	for(j = AllTypes.begin(); j != AllTypes.end(); ++j)
		delete (*j);
	AllTypes.clear();
	PointerType::doFinalization();
}


///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

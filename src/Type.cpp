// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011 The University of Utah
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
#include <sstream>
#include <assert.h>
#include "Common.h"
#include "CGOptions.h"
#include "random.h"
#include "Filter.h"
#include "Error.h"
#include "util.h"
#include "Bookkeeper.h"
#include "Probabilities.h"
#include "DepthSpec.h"
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
static vector<Type *> AllTypes;
static vector<Type *> derived_types;

//////////////////////////////////////////////////////////////////////
class NonVoidTypeFilter : public Filter
{
public:
	NonVoidTypeFilter();

	virtual ~NonVoidTypeFilter();

	virtual bool filter(int v) const;

	Type *get_type();

private:
	mutable Type *typ_;
	
};

NonVoidTypeFilter::NonVoidTypeFilter()
	: typ_(NULL)
{

}

NonVoidTypeFilter::~NonVoidTypeFilter()
{

}

bool
NonVoidTypeFilter::filter(int v) const
{
	assert(static_cast<unsigned int>(v) < AllTypes.size());
	Type *type = AllTypes[v];
	if (type->eType == eSimple && type->simple_type == eVoid)
		return true;

	if (!type->used) {
		Bookkeeper::record_bitfields_structs(type);
		type->used = true;
	}

	typ_ = type;
	if (type->eType == eSimple) {
		Filter *filter = SIMPLE_TYPES_PROB_FILTER;
		return filter->filter(typ_->simple_type);
	}
	
	return false;
}

Type *
NonVoidTypeFilter::get_type()
{
	assert(typ_);
	return typ_;
}

class NonVoidNonVolatileTypeFilter : public Filter
{
public:
	NonVoidNonVolatileTypeFilter();

	virtual ~NonVoidNonVolatileTypeFilter();

	virtual bool filter(int v) const;

	Type *get_type();

private:
	mutable Type *typ_;
	
};

NonVoidNonVolatileTypeFilter::NonVoidNonVolatileTypeFilter()
	: typ_(NULL)
{

}

NonVoidNonVolatileTypeFilter::~NonVoidNonVolatileTypeFilter()
{

}

bool
NonVoidNonVolatileTypeFilter::filter(int v) const
{
	assert(static_cast<unsigned int>(v) < AllTypes.size());
	Type *type = AllTypes[v];
	if (type->eType == eSimple && type->simple_type == eVoid)
		return true;

	if (type->eType == eStruct && !Type::is_nonvolatile_struct(type))
		return true;

	if ((type->eType == eStruct) && (!CGOptions::arg_structs())) {
		return true;
	}

	if (!type->used) {
		Bookkeeper::record_bitfields_structs(type);
		type->used = true;
	}

	typ_ = type;
	if (type->eType == eSimple) {
		Filter *filter = SIMPLE_TYPES_PROB_FILTER;
		return filter->filter(typ_->simple_type);
	}
	
	return false;
}

Type *
NonVoidNonVolatileTypeFilter::get_type()
{
	assert(typ_);
	return typ_;
}

class ChooseRandomTypeFilter : public Filter
{
public:
	ChooseRandomTypeFilter();

	virtual ~ChooseRandomTypeFilter();

	virtual bool filter(int v) const;

	Type *get_type();

private:
	mutable Type *typ_;
	
};

ChooseRandomTypeFilter::ChooseRandomTypeFilter()
{

}

ChooseRandomTypeFilter::~ChooseRandomTypeFilter()
{

}

bool
ChooseRandomTypeFilter::filter(int v) const
{
	assert((v >= 0) && (static_cast<unsigned int>(v) < AllTypes.size()));
	typ_ = AllTypes[v];
	assert(typ_);
	if (typ_->eType == eSimple) {
		Filter *filter = SIMPLE_TYPES_PROB_FILTER;
		return filter->filter(typ_->simple_type);
	}
	else if ((typ_->eType == eStruct) && (!CGOptions::return_structs())) {
		return true;
	}
	return false;
}

Type *
ChooseRandomTypeFilter::get_type()
{
	assert(typ_);
	return typ_;
}

///////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------
/* constructor for simple types
 ********************************************************/
Type::Type(eSimpleType simple_type) :
	eType(eSimple),
	ptr_type(0),
	simple_type(simple_type),
	used(false),
	printed(false),
        packed_(false)
{
	// Nothing else to do.
}

// --------------------------------------------------------------
 /* copy constructor
  *******************************************************/
#if 0
Type::Type(const Type &t) :
	eType(t.eType),
	ptr_type(t.ptr_type),
	simple_type(t.simple_type),
	dimensions(t.dimensions),
	fields(t.fields),
	used(t.used),
	printed(t.printed)
{
	// Nothing else to do.
} 
#endif

// --------------------------------------------------------------
 /* constructor for struct or union types
  *******************************************************/
Type::Type(vector<const Type*>& struct_fields, bool isStruct, bool packed, 
			vector<CVQualifiers> &qfers, vector<int> &fields_length) :
    ptr_type(0),
    fields(struct_fields),
    used(false),
    printed(false),
    packed_(packed),
    qfers_(qfers),
    bitfields_length_(fields_length)
{
    static unsigned int sequence = 0;
	if (isStruct) 
        eType = eStruct;
    else
        eType = eUnion;
    sid =  sequence++;
}

// --------------------------------------------------------------
 /* constructor for pointers
  *******************************************************/
Type::Type(const Type* t) :
	eType(ePointer),
    ptr_type(t),
    used(false),
    printed(false),
    packed_(false)
{
	// Nothing else to do.
}

// --------------------------------------------------------------
Type::~Type(void)
{
	// Nothing to do.
}

// --------------------------------------------------------------
#if 0
Type &
Type::operator=(const Type& t)
{
	if (this == &t) {
		return *this;
	}

	eType = t.eType;
	simple_type = t.simple_type;
    dimensions = t.dimensions;
    fields = t.fields;
	
	return *this;
}
#endif

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

// ---------------------------------------------------------------------
/* find the pointer type to the given type in existing types,
 * return 0 if not found
 *************************************************************/
Type*
Type::find_pointer_type(const Type* t, bool add)
{ 
    for (size_t i=0; i<derived_types.size(); i++) {
        if (derived_types[i]->ptr_type == t) {
            return derived_types[i];
        }
    }
	if (add) {
		Type* ptr_type = new Type(t);
		derived_types.push_back(ptr_type);
		return ptr_type;
	}
    return 0;
}

/*
 * flag == 0, test const;
 * flag == 1, test volatile;
 */
bool
Type::is_nonconst_or_nonvolatile_struct(const Type *ty, const int flag)
{
	if (ty->eType != eStruct)
		return false;
	assert(ty->fields.size() == ty->qfers_.size());

	for (size_t i = 0; i < ty->fields.size(); ++i) {
		const Type *field = ty->fields[i];
		if (field->eType == eStruct) {
			bool flag2 = Type::is_nonconst_or_nonvolatile_struct(field, flag);
			if (!flag2)
				return false;
		}

		CVQualifiers cf = ty->qfers_[i];
		if (flag == 0) {
			if (cf.is_const())
				return false;
		}
		else if (flag == 1) {
			if (cf.is_volatile())
				return false;
		}
		else {
			assert(0);
		}
	}
	return true;
}

bool
Type::is_nonconst_struct(const Type *ty)
{
	return Type::is_nonconst_or_nonvolatile_struct(ty, 0);
}

bool
Type::is_nonvolatile_struct(const Type *ty)
{
	return Type::is_nonconst_or_nonvolatile_struct(ty, 1);
}

bool
Type::is_const_struct() const
{
	return eType==eStruct && !Type::is_nonconst_or_nonvolatile_struct(this, 0);
}

bool
Type::is_volatile_struct() const
{
	return eType==eStruct && !Type::is_nonconst_or_nonvolatile_struct(this, 1);
}

bool
Type::struct_has_int_field() const
{
	assert(eType == eStruct);
	vector<const Type*>::const_iterator i;
	for (i = fields.begin(); i != fields.end(); ++i) {
		if (((*i)->eType == eSimple) && ((*i)->simple_type != eVoid))
			return true;
		else if ((*i)->eType == eStruct) {
			bool rv = false;
			vector<const Type*>::const_iterator j;
			for (j = fields.begin(); j != fields.begin(); ++j) {
				rv = (*j)->struct_has_int_field();
				if (rv)
					return true;
			}
		}
	}
	return false;
}

void
Type::get_all_ok_struct_types(vector<Type *> &ok_types, bool no_const, bool no_volatile, bool need_int_field)
{
	vector<Type *>::iterator i;

	for(i = AllTypes.begin(); i != AllTypes.end(); ++i) {
		Type* t = (*i);
		if ((t->eType != eStruct) ||
			(no_const && t->is_const_struct()) ||
			(no_volatile && t->is_volatile_struct()) ||
			(need_int_field && (!t->struct_has_int_field()))) {
			continue;
		}
		ok_types.push_back(t);
	}
}

const Type*
Type::choose_random_struct_type(vector<Type *> &ok_types)
{
	size_t sz = ok_types.size();
	assert(sz > 0);

	int index = rnd_upto(ok_types.size());
	ERROR_GUARD(0);
	assert(index >= 0);
	Type *rv_type = ok_types[index];
	if (!rv_type->used) {
		Bookkeeper::record_bitfields_structs(rv_type);
		rv_type->used = true;
	}
	return rv_type;
}

const Type* 
Type::choose_random_pointer_type(void) 
{
	unsigned int index = rnd_upto(derived_types.size());
	ERROR_GUARD(NULL);
	return derived_types[index];
}

bool 
Type::has_pointer_type(void) 
{
	return derived_types.size() > 0;
}

const Type*
Type::choose_random_struct_from_type(const Type* type, bool no_volatile)
{
	if (!type)
		return NULL;

	const Type* t = type;
	
	vector<Type *> ok_struct_types;
	get_all_ok_struct_types(ok_struct_types, no_volatile, false, true);

	if (ok_struct_types.size() > 0) {
		DEPTH_GUARD_BY_DEPTH_RETURN(1, NULL);

		t = Type::choose_random_struct_type(ok_struct_types);
		ERROR_GUARD(NULL);
	}
	return t;
}

const Type* 
Type::random_type_from_type(const Type* type, bool no_volatile, bool strict_simple_type)
{
	const Type* t = type;
	DEPTH_GUARD_BY_TYPE_RETURN(dtRandomTypeFromType, NULL);
	if (type == 0) {
		t = no_volatile ? choose_random_nonvoid_nonvolatile() : choose_random_nonvoid(); 
		ERROR_GUARD(NULL);
	}
	if (type->eType == eSimple && !strict_simple_type) {
		t = choose_random_simple();
		ERROR_GUARD(NULL);
	} 
	if (t->eType == eSimple) {
		assert(t->simple_type != eVoid);
	}
	return t;
}

// ---------------------------------------------------------------------
static bool
AllTypesProbability(void)
{
	// Always have at least 10 types in the program.
	if (AllTypes.size() < 10)
		return true;
	// 50% probability for each additional type.
	return rnd_flipcoin(50);
}

// ---------------------------------------------------------------------
eSimpleType
Type::choose_random_nonvoid_simple(void)
{
	eSimpleType simple_type;
#if 0
	vector<unsigned int> vs;
	vs.push_back(eVoid);
	
	if (!CGOptions::allow_int64()) {
		vs.push_back(eLongLong);
		vs.push_back(eULongLong);
	}

	VectorFilter filter(vs);
#endif

	simple_type = (eSimpleType) rnd_upto(MAX_SIMPLE_TYPES, SIMPLE_TYPES_PROB_FILTER);

	return simple_type;
}

void
Type::make_one_bitfield(size_t index, bool &last_is_zero,
				vector<const Type*> &random_fields,
				vector<CVQualifiers> &qualifiers,
				vector<int> &fields_length)
{
	int max_length = CGOptions::bitfields_length();

	bool sign = rnd_flipcoin(BitFieldsSignedProb);
	ERROR_RETURN();
	const Type *type = sign ? &Type::get_simple_type(eInt) : &Type::get_simple_type(eUInt);
       	random_fields.push_back(type);
	CVQualifiers qual = CVQualifiers::random_qualifiers(type, FieldConstProb, FieldVolatileProb);
	ERROR_RETURN();
	qualifiers.push_back(qual);
	int length = rnd_upto(CGOptions::bitfields_length());
	//while (i == 0 && length == 0)
		//length = rnd_upto(CGOptions::bitfields_length());
	ERROR_RETURN();
	if (index==0 || last_is_zero) {
		if (max_length <= 2)
			length = 1;
		else
			length = rnd_upto(max_length - 1) + 1;
	}
	else {
		length = rnd_upto(max_length);
	}
	ERROR_RETURN();
	last_is_zero = (length == 0) ? true : false;
	fields_length.push_back(length);
}

// ---------------------------------------------------------------------
void
Type::make_full_bitfields_struct_fields(size_t field_cnt, vector<const Type*> &random_fields, 
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length)
{
	bool last_is_zero = false;

	for (size_t i=0; i<field_cnt; i++) {
		bool is_non_bitfield = rnd_flipcoin(ScalarFieldInFullBitFieldsProb);
		if (is_non_bitfield) {
			make_one_struct_field(i, random_fields, qualifiers, fields_length);
			last_is_zero = false;
		}
		else {
			make_one_bitfield(i, last_is_zero, random_fields, qualifiers, fields_length);
		}
	}
}

void
Type::make_one_struct_field(size_t, vector<const Type*> &random_fields, 
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length)
{
	ChooseRandomTypeFilter f;
	unsigned int i = rnd_upto(AllTypes.size(), &f);
	ERROR_RETURN();
        const Type* type = AllTypes[i];
        random_fields.push_back(type);
	CVQualifiers qual = CVQualifiers::random_qualifiers(type, FieldConstProb, FieldVolatileProb);
	ERROR_RETURN();
	qualifiers.push_back(qual);
	fields_length.push_back(-1);
}

void
Type::make_normal_struct_fields(size_t field_cnt, vector<const Type*> &random_fields, 
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length)
{
    bool last_is_zero = false;

    for (size_t i=0; i<field_cnt; i++)
    {
	bool is_bitfield = CGOptions::bitfields() && rnd_flipcoin(BitFieldInNormalStructProb);
	if (is_bitfield) {
		make_one_bitfield(i, last_is_zero, random_fields, qualifiers, fields_length);
	}
	else {
		make_one_struct_field(i, random_fields, qualifiers, fields_length);
		last_is_zero = false;
	}
    }
}

void
Type::make_all_full_bitfields_struct_types()
{
/*
	bool sign = false;
	if (Probabilities::zero_probability(BitFieldsSignedProb)) 
*/
}

#define ZERO_BITFIELD 0
#define RANDOM_BITFIELD 1
//#define MAX_BITFIELD 2
#define ENUM_BITFIELD_SIZE 2

void
Type::init_is_bitfield_enumerator(Enumerator<string> &enumerator, int bitfield_prob)
{
	int field_cnt = CGOptions::max_struct_fields();
	for (int i = 0; i < field_cnt; ++i) {
		std::ostringstream ss;
		ss << "bitfield" << i;

		if (CGOptions::bitfields()) {
			enumerator.add_bool_elem(ss.str(), bitfield_prob);
		}
		else {
			enumerator.add_bool_elem(ss.str(), 0);
		}
	}
}

void
Type::init_fields_enumerator(Enumerator<string> &enumerator, 
				Enumerator<string> &bitfield_enumerator, 
				int type_bound, int qual_bound, 
				int bitfield_qual_bound)
{
	int field_cnt = CGOptions::max_struct_fields();
	for (int i = 0; i < field_cnt; ++i) {
		std::ostringstream ss;
		ss << "bitfield" << i;
		bool is_bitfield = bitfield_enumerator.get_elem(ss.str());
		if (is_bitfield) {
			std::ostringstream ss1, ss2, ss3;
			ss1 << "bitfield_sign" << i;
			ss2 << "bitfield_qualifier" << i;
			ss3 << "bitfield_length" << i;
			enumerator.add_bool_elem_of_bool(ss1.str(), false);
			enumerator.add_elem(ss2.str(), bitfield_qual_bound);
			enumerator.add_elem(ss3.str(), ENUM_BITFIELD_SIZE);
		}
		else {
			std::ostringstream ss1, ss2;
			ss1 << "field" << i;
			ss2 << "qualifier" << i;
			enumerator.add_elem(ss1.str(), type_bound);
			enumerator.add_elem(ss2.str(), qual_bound);
		}
	}
	enumerator.add_bool_elem_of_bool("packed", CGOptions::packed_struct());
}

int
Type::get_bitfield_length(int length_flag)
{
	int max_length = CGOptions::bitfields_length();
	assert(max_length > 0);
	int length;
	switch (length_flag) {
	case ZERO_BITFIELD:
		length = 0;
		break;
#if 0
	case MAX_BITFIELD:
		length = max_length;
		break;
#endif
	case RANDOM_BITFIELD:
		length = pure_rnd_upto(max_length);
		break;
	default:
		assert(0);
	}
	return length;
}

bool
Type::make_one_bitfield_by_enum(Enumerator<string> &enumerator,
				vector<CVQualifiers> &all_bitfield_quals,
				vector<const Type*> &random_fields,
				vector<CVQualifiers> &qualifiers,
				vector<int> &fields_length,
				int index, bool &last_is_zero)
{

	std::ostringstream ss1, ss2, ss3;
	ss1 << "bitfield_sign" << index;
	ss2 << "bitfield_qualifier" << index;
	ss3 << "bitfield_length" << index;

	bool sign = enumerator.get_elem(ss1.str());
	// we cannot allow too many structs,
	// so randomly choose the sign of fields.
	if (pure_rnd_flipcoin(50))
		sign = true;
	const Type *type = sign ? &Type::get_simple_type(eInt) : &Type::get_simple_type(eUInt);
       	random_fields.push_back(type);
	int qual_index = enumerator.get_elem(ss2.str());
	assert((qual_index >= 0) && ((static_cast<unsigned int>(qual_index)) < all_bitfield_quals.size()));
	CVQualifiers qual = all_bitfield_quals[qual_index];
	qualifiers.push_back(qual);

	int length_flag = enumerator.get_elem(ss3.str());
	
	int length = get_bitfield_length(length_flag);
	if ((index==0 || last_is_zero) && (length == 0)) {
		return false;
	}
	last_is_zero = (length == 0) ? true : false;
	fields_length.push_back(length);
	return true;
}

bool 
Type::make_one_normal_field_by_enum(Enumerator<string> &enumerator, vector<const Type*> &all_types,
				vector<CVQualifiers> &all_quals, vector<const Type*> &fields,
				vector<CVQualifiers> &quals, vector<int> &fields_length, int i)
{
	int types_size = all_types.size();
	int quals_size = all_quals.size();
	Filter *filter = SIMPLE_TYPES_PROB_FILTER;

	std::ostringstream ss1, ss2;
	ss1 << "field" << i;
	int typ_index = enumerator.get_elem(ss1.str());
	assert(typ_index >= 0 && typ_index < types_size);
	Type *typ = const_cast<Type*>(all_types[typ_index]);
	if (typ->eType == eSimple) {
		assert(typ->simple_type != eVoid);
		if (filter->filter(typ->simple_type))
			return false;
	}

	assert(typ != NULL);
	fields.push_back(typ);

	ss2 << "qualifier" << i;
	int qual_index = enumerator.get_elem(ss2.str());
	assert(qual_index >= 0 && qual_index < quals_size);
	CVQualifiers qual = all_quals[qual_index];
	quals.push_back(qual);

	fields_length.push_back(-1);
	return true;
}

void
Type::make_all_struct_types_(Enumerator<string> &bitfields_enumerator, vector<const Type*> &accum_types,
				vector<const Type*> &all_types, vector<CVQualifiers> &all_quals, 
				vector<CVQualifiers> &all_bitfield_quals)
{
	Enumerator<string> fields_enumerator;
	init_fields_enumerator(fields_enumerator, bitfields_enumerator, all_types.size(), 
			all_quals.size(), all_bitfield_quals.size());
	
	Enumerator<string> *i;
	for (i = fields_enumerator.begin(); i != fields_enumerator.end(); i = i->next()) {
		make_all_struct_types_with_bitfields(*i, bitfields_enumerator, accum_types, all_types, all_quals, all_bitfield_quals);
	}
}

void
Type::make_all_struct_types_with_bitfields(Enumerator<string> &enumerator, 
				Enumerator<string> &bitfields_enumerator, vector<const Type*> &accum_types,
				vector<const Type*> &all_types, vector<CVQualifiers> &all_quals, 
				vector<CVQualifiers> &all_bitfield_quals)
{
	vector<const Type*> fields;
	vector<CVQualifiers> quals;
	vector<int> fields_length;
	int field_cnt = CGOptions::max_struct_fields();
    	bool last_is_zero = false;

	int bitfields_cnt = 0;
	int normal_fields_cnt = 0;
	for (int i = 0; i < field_cnt; ++i) {
		std::ostringstream ss;
		ss << "bitfield" << i;
		bool is_bitfield = bitfields_enumerator.get_elem(ss.str());
		bool rv = false;
		if (is_bitfield) {
			rv = make_one_bitfield_by_enum(enumerator, all_bitfield_quals, fields, quals, fields_length, i, last_is_zero);
			bitfields_cnt++;
		}
		else {
			rv = make_one_normal_field_by_enum(enumerator, all_types, all_quals, fields, quals, fields_length, i);
			last_is_zero = rv ? false : last_is_zero;
			normal_fields_cnt++;
		}
		if (!rv)
			return;
	}
	if ((ExhaustiveBitFieldsProb > 0) && (ExhaustiveBitFieldsProb < 100) && 
		((bitfields_cnt == field_cnt) || (normal_fields_cnt == field_cnt)))
		return;

	bool packed = enumerator.get_elem("packed");
	Type* new_type = new Type(fields, true, packed, quals, fields_length);
	new_type->used = true;
	accum_types.push_back(new_type);
}

/*
 * level control's the nested level of struct
 */
void
Type::copy_all_fields_types(vector<const Type*> &dest_types, vector<const Type*> &src_types)
{
	vector<const Type*>::const_iterator i;
	for (i = src_types.begin(); i != src_types.end(); ++i)
		dest_types.push_back(*i);
}

void
Type::reset_accum_types(vector<const Type*> &accum_types)
{
	accum_types.clear();
	vector<Type*>::const_iterator i;
	for (i = AllTypes.begin(); i != AllTypes.end(); ++i)
		accum_types.push_back(*i);
}

void
Type::delete_useless_structs(vector<const Type*> &all_types, vector<const Type*> &accum_types)
{
	assert(all_types.size() <= accum_types.size());
	for (size_t i = 0; i < all_types.size(); ++i) {
		const Type *t = all_types[i];
		if (t->eType == eStruct) {
			const Type *t1 = accum_types[i];
			delete t1;
			accum_types[i] = t;
		}
	}
}

void
Type::make_all_struct_types(int level, vector<const Type*> &accum_types)
{

	if (level > 0) {
		make_all_struct_types(level - 1, accum_types);
	}
	vector<const Type*> all_types;
	copy_all_fields_types(all_types, accum_types);
	reset_accum_types(accum_types);

	vector<CVQualifiers> all_quals;
	CVQualifiers::get_all_qualifiers(all_quals, RegularConstProb, RegularVolatileProb);

	vector<CVQualifiers> all_bitfield_quals;
	CVQualifiers::get_all_qualifiers(all_bitfield_quals, FieldConstProb, FieldVolatileProb);

	Enumerator<string> fields_enumerator;
	init_is_bitfield_enumerator(fields_enumerator, ExhaustiveBitFieldsProb);

	Enumerator<string> *i;
	for (i = fields_enumerator.begin(); i != fields_enumerator.end(); i = i->next()) {
		make_all_struct_types_(*i, accum_types, all_types, all_quals, all_bitfield_quals);
	}
	delete_useless_structs(all_types, accum_types);
}

void
Type::make_all_struct_union_types(void)
{
	int level = CGOptions::max_nested_struct_level();
	if (CGOptions::dfs_exhaustive()) {
		vector<const Type*> accum_types;
		reset_accum_types(accum_types);
		make_all_struct_types(level, accum_types);
		assert(accum_types.size() >= AllTypes.size());
		for (size_t i = AllTypes.size(); i < accum_types.size(); ++i)
			AllTypes.push_back(const_cast<Type*>(accum_types[i]));
	}
}

Type*
Type::make_random_struct_union_type(void)
{ 
    size_t field_cnt = 0;
    size_t max_cnt = CGOptions::max_struct_fields();
    if (CGOptions::fixed_struct_fields()) 
        field_cnt = max_cnt;
    else
        field_cnt = rnd_upto(max_cnt) + 1;
    ERROR_GUARD(NULL);
    vector<const Type*> random_fields;
    vector<CVQualifiers> qualifiers;
    vector<int> fields_length;
    bool is_bitfields = CGOptions::bitfields() && rnd_flipcoin(BitFieldsCreationProb);
    ERROR_GUARD(NULL);
    //if (CGOptions::bitfields())
    if (is_bitfields)
        make_full_bitfields_struct_fields(field_cnt, random_fields, qualifiers, fields_length);
    else
        make_normal_struct_fields(field_cnt, random_fields, qualifiers, fields_length);

    ERROR_GUARD(NULL);
    // for now, no union type
    bool packed = false;
    if (CGOptions::packed_struct()) {
        packed = rnd_flipcoin(50);
        ERROR_GUARD(NULL);
    }

    Type* new_type = new Type(random_fields, true, packed, qualifiers, fields_length);
    return new_type;
}

// ---------------------------------------------------------------------
Type*
Type::make_random_pointer_type(void)
{ 
    //Type* new_type = 0;
    //Type* ptr_type = 0;
    // occasionally choose pointer to pointers
    if (rnd_flipcoin(20)) {
		ERROR_GUARD(NULL);
        if (derived_types.size() > 0) {
			unsigned int rnd_num = rnd_upto(derived_types.size());
			ERROR_GUARD(NULL);
			const Type* t = derived_types[rnd_num];
			if (t->get_indirect_level() < CGOptions::max_indirect_level()) {
				return find_pointer_type(t, true);
			}
		}
	}
    
    // choose a pointer to basic/aggregate types 
	const Type* t = choose_random();
	ERROR_GUARD(NULL);
	// consolidate all integer pointer types into "int*", hopefully this increase
	// chance of pointer assignments and dereferences
	if (t->eType == eSimple) {
		t = get_int_type();
		ERROR_GUARD(NULL);
	}
	return find_pointer_type(t, true);
}

// ---------------------------------------------------------------------
void
Type::GenerateSimpleTypes(void)
{ 
    unsigned int st;
    for (st=eChar; st<MAX_SIMPLE_TYPES; st++)
    { 
#if 0
        if ((st==eLongLong || st==eULongLong) && !CGOptions::allow_int64())
		continue;
#endif
	AllTypes.push_back(new Type((enum eSimpleType)st));
    }
    Type::void_type = new Type((enum eSimpleType)eVoid);
}

// ---------------------------------------------------------------------
void
GenerateAllTypes(void)
{
	// In the exhaustive mode, we want to generate all type first. 
	// We don't support struct for now
	if (CGOptions::dfs_exhaustive()) {
        	Type::GenerateSimpleTypes();
		if (CGOptions::use_struct() && CGOptions::expand_struct())
			Type::make_all_struct_union_types();
		return;
	}

    if (!CGOptions::use_struct())
    {
       	while (AllTypesProbability()) {
	    AllTypes.push_back(new Type(Type::choose_random_nonvoid_simple()));
    	}
    	Type::void_type = new Type((enum eSimpleType)eVoid);
    }
    else
    {
        Type::GenerateSimpleTypes();
	    while (AllTypesProbability()) {
		    ERROR_RETURN();
		    Type *ty = Type::make_random_struct_union_type();
		    ERROR_RETURN();
		    AllTypes.push_back(ty);
	    }
    }
}

// ---------------------------------------------------------------------
const Type *
Type::choose_random()
{
	ChooseRandomTypeFilter f;
	rnd_upto(AllTypes.size(), &f);
	ERROR_GUARD(NULL);
	Type *rv_type = f.get_type();
	if (!rv_type->used) {
		Bookkeeper::record_bitfields_structs(rv_type);
		rv_type->used = true;
	}
	return rv_type;
}

const Type *
Type::choose_random_nonvoid(void)
{
	DEPTH_GUARD_BY_DEPTH_RETURN(1, NULL);
	NonVoidTypeFilter f;
	rnd_upto(AllTypes.size(), &f);

	ERROR_GUARD(NULL);
	Type *typ = f.get_type();
	assert(typ);
	return typ;
}

const Type *
Type::choose_random_nonvoid_nonvolatile(void)
{
	DEPTH_GUARD_BY_DEPTH_RETURN(1, NULL);
	NonVoidNonVolatileTypeFilter f;
	rnd_upto(AllTypes.size(), &f);

	ERROR_GUARD(NULL);
	Type *typ = f.get_type();
	assert(typ);
	return typ;
}

// ---------------------------------------------------------------------
const Type *
Type::choose_random_simple(void)
{
    DEPTH_GUARD_BY_TYPE_RETURN(dtTypeChooseSimple, NULL);
    eSimpleType ty = choose_random_nonvoid_simple();
    ERROR_GUARD(NULL);
    assert(ty != eVoid);
    return &get_simple_type(ty);
}

// ---------------------------------------------------------------------
int 
Type::get_indirect_level() const
{
    int level = 0;
    const Type* pt = ptr_type; 
    while (pt != 0) {
        level++;
        pt = pt->ptr_type;
    }
    return level;
}

// ---------------------------------------------------------------------
int 
Type::get_struct_depth() const
{
    int depth = 0; 
    if (eType == eStruct) {
		depth++;
		for (size_t i=0; i<fields.size(); i++) {
			depth += fields[i]->get_struct_depth();
		}
	}
	return depth;
}

bool
Type::is_unamed_padding(size_t index) const
{
	size_t sz = bitfields_length_.size();
	
	if (sz == 0)
		return false;

	assert(index < sz);

	return (bitfields_length_[index] == 0);
}

bool
Type::is_bitfield(size_t index) const
{
	assert(index < bitfields_length_.size());
	return (bitfields_length_[index] >= 0);
}

bool
Type::has_bitfields() const
{
	size_t i;
	for (i = 0; i < bitfields_length_.size(); ++i) {
		if (bitfields_length_[i] >= 0)
			return true;
	}
	return false;
}

bool
Type::is_full_bitfields_struct() const
{
	//return (bitfields_length_.size() > 0);
	size_t i;
	for (i = 0; i < bitfields_length_.size(); ++i) {
		if (bitfields_length_[i] < 0)
			return false;
	}
	return true;
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

const Type* 
Type::get_base_type(void) const
{
	const Type* tmp = this;
	while (tmp->ptr_type != 0) {
		tmp = tmp->ptr_type;
	}
	return tmp;
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
        if ((simple_type != eVoid && t->simple_type != eVoid) ||
            simple_type == t->simple_type) 
            return true;
    }
    else if (eType == ePointer && t->eType == ePointer) {
		if (ptr_type == t->ptr_type) {
			return true;
		}
		if (ptr_type->eType == eSimple && t->ptr_type->eType == eSimple) {
			return ptr_type->SizeInBytes() == t->ptr_type->SizeInBytes();
		}
        //return ptr_type->is_convertable(t->ptr_type);
		//return t->ptr_type->is_promotable(ptr_type);
    }
    return false;
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
        const Type* pt = t->ptr_type;
        while (pt) {
            if (pt == this) {
                return true;
            }
            pt = pt->ptr_type;
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
	return is_convertable(t) || is_dereferenced_from(t) || (ptr_type==t);
}

unsigned long
Type::SizeInBytes(void) const
{
    size_t i;
	switch (eType) {
	default: break;
	case eSimple:
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
//		case eFloat:	return 4;
//		case eDouble:	return 8;
		}
		break;
	case eUnion: { 
        unsigned int max_size = 0;
        for (i=0; i<fields.size(); i++) { 
            if (fields[i]->SizeInBytes() > max_size) {
                max_size = fields[i]->SizeInBytes();
            }
        }
        return max_size;
    }
    case eStruct: { 
        unsigned int total_size = 0;
        for (i=0; i<fields.size(); i++) { 
            total_size += fields[i]->SizeInBytes();
        }
        return total_size;
    }
	case ePointer: return CGOptions::x86_64() ? 8 : 4; 
	}
	return 0;
}

// --------------------------------------------------------------
 /* Select a left hand type
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
		ERROR_GUARD(NULL);
		type = Type::make_random_pointer_type(); 
	}

	ERROR_GUARD(NULL);
	// allow struct as LValue
	vector<Type *> ok_struct_types;
	get_all_ok_struct_types(ok_struct_types, true, no_volatile, false);

	if ((ok_struct_types.size() > 0) && !type && (op == eSimpleAssign) && rnd_flipcoin(StructAsLTypeProb)) {
		ERROR_GUARD(NULL);
		type = Type::choose_random_struct_type(ok_struct_types);
	}

	ERROR_GUARD(NULL);
	// default is any integer type
	if (!type)
		type = get_int_type();
	ERROR_GUARD(NULL);
 
	return type;
}

void 
Type::get_int_subfield_names(string prefix, vector<string>& names) const
{
	if (eType == eSimple) {
		names.push_back(prefix);
	}
	else if (eType == eStruct) {
		size_t i;
		size_t j = 0;
		for (i=0; i<fields.size(); i++) {
			// skip 0 length bitfields
			if (is_unamed_padding(i)) {
				continue;
			}
			ostringstream oss;
			oss << prefix << ".f" << j++;
			fields[i]->get_int_subfield_names(oss.str(), names);
		}
	}
}

// ---------------------------------------------------------------------
void
Type::Output(std::ostream &out) const
{
	switch (eType) {
	case eSimple:
		if (this->simple_type == eVoid) {
			out << "void";
		} else {
			out << (is_signed() ? "int" : "uint");
			out << (SizeInBytes() * 8);
			out << "_t";
		} 
		break;
    case ePointer:   ptr_type->Output( out ); out << "*"; break;
    case eUnion:     out << "union U" << sid; break;
    case eStruct:    out << "struct S" << sid; break;
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

// ---------------------------------------------------------------------
/* print struct definition (fields etc)
 *************************************************************/
void OutputStruct(Type* type, std::ostream &out)
{
    size_t i;
    // sanity check
    assert (type->eType==eStruct || type->eType==eUnion);

    if (!type->printed) { 
        // output dependent structs, if any
        for (i=0; i<type->fields.size(); i++) { 
            if (type->fields[i]->eType==eStruct || type->fields[i]->eType==eUnion) {
                OutputStruct((Type*)type->fields[i], out);
            }
        } 
        // output myself
        if (type->packed_) {
            out << "#pragma pack(push)";
            really_outputln(out);
            out << "#pragma pack(1)";
            really_outputln(out);
        }
        type->Output(out);
        out << " {";
		really_outputln(out);

		assert(type->fields.size() == type->qfers_.size());
		//if (is_full_bitfields)
		//assert(type->fields.size() == type->bitfields_length_.size());
		size_t j = 0;
        for (i=0; i<type->fields.size(); i++) {
            out << "   ";
			const Type *field = type->fields[i];
			bool is_bitfield = type->is_bitfield(i);
            if (is_bitfield) {
				assert(field->eType == eSimple);
				type->qfers_[i].OutputFirstQuals(out);
				if (field->simple_type == eInt)
					out << "signed";
				else if (field->simple_type == eUInt)
					out << "unsigned";
				else
					assert(0);
				int length = type->bitfields_length_[i];
				assert(length >= 0);
				if (length == 0)
					out << " : ";
				else
					out << " f" << j++ << " : ";
				out << "_CSMITH_BITFIELD(" << length << ");";
			}
			else {
				type->qfers_[i].output_qualified_type(field, out);
				out << " f" << j++ << ";";
			}
			really_outputln(out);
        }
        out << "};";
		really_outputln(out);
        if (type->packed_) {
			out << "#pragma pack(pop)";
			really_outputln(out);
        }
        type->printed = true;
		really_outputln(out);
    }
}

// ---------------------------------------------------------------------
/* print all struct definitions (fields etc)
 *************************************************************/
void
OutputStructDeclarations(std::ostream &out)
{
    size_t i;
    output_comment_line(out, "--- Struct/Union Declarations ---");
    for (i=0; i<AllTypes.size(); i++)
    {
        Type* t = AllTypes[i];
        if (t->used && (t->eType == eStruct || t->eType == eUnion)) {
            OutputStruct(AllTypes[i], out);
        }
    }
}

/*
 * return the printf directive string for the type. for example, int -> "%d"
 */
std::string 
Type::printf_directive(void) const
{
	string ret;
	size_t i;
	switch (eType) {
	case eSimple:
		if (SizeInBytes() >= 8) {
			ret = is_signed() ? "%lld" : "%llu";
		} else {
			ret = is_signed() ? "%d" : "%u";
		}
		break;
	case ePointer:   
		ret = "0x%0x"; 
		break;
	case eUnion:   
	case eStruct:  
		ret = "{";
		for (i=0; i<fields.size(); i++) {
			if (i > 0) ret += ", ";
			ret += fields[i]->printf_directive();
		}
		ret += "}"; 
		break;
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

	for(j = derived_types.begin(); j != derived_types.end(); ++j)
		delete (*j);
	derived_types.clear();
}


///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

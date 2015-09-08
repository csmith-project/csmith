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

#ifndef TYPE_H
#define TYPE_H

///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <ostream>
#include <vector>
#include "CommonMacros.h"
#include "StatementAssign.h"
#include "CVQualifiers.h"
#include "Filter.h"

using namespace std;

#define SIZE_UNKNOWN 0xFFFF

/*
 *
 */
enum eTypeDesc
{
	eSimple,
	ePointer,
	eUnion,
    eStruct,
};
#define MAX_TYPE_DESC ((eTypeDesc) (eStruct+1))

/*
 *
 */
enum eSimpleType
{
	eVoid,
	eChar,
	eInt,
	eShort,
	eLong,
	eLongLong,
	eUChar,
	eUInt,
	eUShort,
	eULong,
	eFloat,
	// eDouble,
	eULongLong,
};
#define MAX_SIMPLE_TYPES ((eSimpleType) (eULongLong+1))

enum eMatchType
{
	eExact,
	eConvert,
	eDereference,
	eDerefExact,
	eFlexible,
};

//////////////////////////////////////////////////////////////////////

/*
 *
 */
class Type
{
public:
	// Pseudo-factory method.  This is `choose_random()' rather than
	// `make_random()' because the returned object is not fresh.
	static const Type *choose_random(Filter * additional_filter = NULL);

	static const Type *choose_random_nonvoid(Filter * additional_filter = NULL);

	// choose a random integer type
	static const Type *choose_random_simple(void);
	static const Type *choose_random_simple(Filter * filter);

	virtual bool has_int_field() const;

	bool signed_overflow_possible() const;

	static const Type * choose_random_nonvoid_nonvolatile(Filter * additional_filter = NULL);

	static const Type* random_type_from_type(const Type* type, bool no_volatile=false, bool strict_simple_type=false);

	static const Type *get_type_from_string(const string &type_string);

	// generate all simple types except void and long long(if it is not allowed)
	static void GenerateSimpleTypes(void);

	void get_type_sizeof_string(std::string &s) const;

	// For choosing a random, non-void `eSimpleType'.
	static eSimpleType choose_random_nonvoid_simple(void);

	// select a type for LHS
	static const Type *SelectLType(bool no_volatile, eAssignOps op);

	explicit Type(eSimpleType simple_type);
	explicit Type(std::string name, int id, int size);
	Type(void);
	~Type(void);

	static const Type &get_simple_type(eSimpleType simple_type);

	static void doFinalization(void);

	virtual const Type* get_base_type(void) const { return this; }
	virtual int get_indirect_level(void) const { return 0; }
	virtual int get_struct_depth(void) const { return 0; }
	virtual void get_int_subfield_names(string prefix, vector<string>& names, const vector<int>& excluded_fields) const;
	bool is_signed(void) const;
	bool is_long_long(void) const {
		return ((eType == eSimple) && (simple_type == eLongLong || simple_type == eULongLong));
	}
	const Type* to_unsigned(void) const;
	virtual bool is_pointer_to_char(void) const { return false; }
	bool is_signed_char() const {
		return ((eType == eSimple) && (simple_type == eChar));
	}
	bool is_float() const {
		return ((eType == eSimple) && (simple_type == eFloat));
	}
	bool is_promotable(const Type* t) const;
	virtual bool is_convertable(const Type* t) const;
	virtual bool is_derivable(const Type* t) const;
	bool is_dereferenced_from(const Type* t) const;
	bool is_equivalent(const Type* t) const;
	bool needs_cast(const Type* t) const;
	virtual bool is_unamed_padding(size_t index) const;
	virtual bool is_full_bitfields_struct() const;
	virtual bool is_bitfield(size_t index) const;
	virtual bool has_bitfields() const;
	virtual bool has_padding(void) const;
	virtual bool contain_pointer_field(void) const;
	virtual bool is_const_struct_union() const;
	virtual bool is_volatile_struct_union() const;
	bool is_int(void) const { return eType == eSimple && simple_type != eVoid && simple_type != eFloat && simple_type < MAX_SIMPLE_TYPES;}
    bool is_void(void) const { return eType == eSimple && simple_type == eVoid;}
	bool is_aggregate(void) const { return eType == eStruct || eType == eUnion;}
	bool match(const Type* t, enum eMatchType mt) const;
	virtual unsigned long SizeInBytes(void) const;
	virtual void Output(std::ostream &) const;
	virtual std::string printf_directive(void) const;
	static Type* find_type(const Type* t);
	static int find_type(std::string name, int id = -1, int size = -1);
	std::string get_type_name() const { return type_name; } 


// private:
	eTypeDesc eType;
	eSimpleType simple_type;

	int type_index;
	bool used;                          // whether any variable declared with this type

	static Type *void_type;

	static vector<Type *> AllTypes;

private:
	DISALLOW_COPY_AND_ASSIGN(Type);

	static map<int, const Type *> simple_types;

	std::string type_name;
	
	int type_size;
	
	// Package init.
	friend void GenerateAllTypes(void);
};

void GenerateAllTypes(void);
const Type * get_int_type(void);

///////////////////////////////////////////////////////////////////////////////

#endif // TYPE_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

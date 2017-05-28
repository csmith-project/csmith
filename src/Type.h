// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2014, 2015, 2016 The University of Utah
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

template <class Name> class Enumerator;
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

/*
 *
 */
class Type
{
public:
	// Pseudo-factory method.  This is `choose_random()' rather than
	// `make_random()' because the returned object is not fresh.
	static const Type *choose_random();

	static const Type *choose_random_nonvoid();

	// choose a random integer type
	static const Type *choose_random_simple(void);

	// choose a random pointer type
	static const Type* choose_random_pointer_type(void);

	static const Type *choose_random_struct_from_type(const Type* type, bool no_volatile);

	static void get_all_ok_struct_union_types(vector<Type *> &ok_types, bool no_const, bool no_volatile, bool need_int_field, bool bStruct);

	static bool if_struct_will_have_assign_ops();
	static bool if_union_will_have_assign_ops();

	bool has_int_field() const;

	bool signed_overflow_possible() const;

	static const Type* choose_random_struct_union_type(vector<Type *> &ok_types);

	static const Type * choose_random_nonvoid_nonvolatile(void);

	static bool has_pointer_type(void);

	static const Type* random_type_from_type(const Type* type, bool no_volatile=false, bool strict_simple_type=false);

	static void copy_all_fields_types(vector<const Type*> &dest_types, vector<const Type*> &src_types);

	static void reset_accum_types(vector<const Type*> &accum_types);

	static void delete_useless_structs(vector<const Type*> &all_types, vector<const Type*> &accum_types);

	static void init_is_bitfield_enumerator(Enumerator<string> &enumerator, int bitfield_prob);

	static void init_fields_enumerator(Enumerator<std::string> &enumerator,
				Enumerator<string> &bitfield_enumerator,
				int type_bound, int qual_bound, int bitfield_qual_bound);

	static bool make_one_normal_field_by_enum(Enumerator<string> &enumerator, vector<const Type*> &all_types,
				vector<CVQualifiers> &all_quals, vector<const Type*> &fields,
				vector<CVQualifiers> &quals, vector<int> &fields_length, int i);

	static bool make_one_bitfield_by_enum(Enumerator<string> &enumerator,
				vector<CVQualifiers> &all_bitfield_quals,
				vector<const Type*> &random_fields,
				vector<CVQualifiers> &qualifiers,
				vector<int> &fields_length,
				int index, bool &last_is_zero);

	static int get_bitfield_length(int length_flag);

	static void make_all_struct_types_(Enumerator<string> &bitfields_enumerator, vector<const Type*> &accum_types,
				vector<const Type*> &all_types, vector<CVQualifiers> &all_quals,
				vector<CVQualifiers> &all_bitfield_quals);

	static void make_all_struct_types_with_bitfields(Enumerator<string> &enumerator,
					Enumerator<string> &bitfields_enumerator,
					vector<const Type*> &accum_types, vector<const Type*> &all_types,
					vector<CVQualifiers> &all_quals, vector<CVQualifiers> &all_bitfield_quals);

	static void make_all_struct_types(int level, vector<const Type*> &accum_types);

	static void make_all_struct_union_types();

	// make a random struct or union type
	static Type* make_random_struct_type(void);
	static Type* make_random_union_type(void);

	static void make_one_bitfield(vector<const Type*> &random_fields,
				vector<CVQualifiers> &qualifiers,
				vector<int> &fields_length);

	static void make_one_struct_field(vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length, 
					bool structHasAssignOps);
	static void make_one_union_field(vector<const Type*> &fields, vector<CVQualifiers> &qfers, vector<int> &lens);

	static void make_full_bitfields_struct_fields(size_t field_cnt, vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length, 
					bool structHasAssignOps);

	static void make_normal_struct_fields(size_t field_cnt, vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length, 
					bool structHasAssignOps);

	// make a random pointer type
	static Type* make_random_pointer_type(void);

	static const Type *get_type_from_string(const string &type_string);

	// generate all simple types except void and long long(if it is not allowed)
	static void GenerateSimpleTypes(void);

	void get_type_sizeof_string(std::string &s) const;

	// For choosing a random, non-void `eSimpleType'.
	static eSimpleType choose_random_nonvoid_simple(void);

	// select a type for LHS
	static const Type *SelectLType(bool no_volatile, eAssignOps op);

	static bool has_aggregate_field(const vector<const Type *> &fields);

	static bool has_longlong_field(const vector<const Type *> &fields);

	explicit Type(eSimpleType simple_type);
	Type(vector<const Type*>& fields, bool isStruct, bool packed,
			vector<CVQualifiers> &qfers, vector<int> &fields_length, bool hasAssignOps, bool hasImplicitNontrivialAssignOps);
	Type(vector<unsigned int>& array_dims, eSimpleType st);
	explicit Type(const Type* t);
	~Type(void);

	static const Type &get_simple_type(eSimpleType simple_type);

	static void doFinalization(void);

	const Type* get_base_type(void) const;
	int get_indirect_level(void) const;
	int get_struct_depth(void) const;
	void get_int_subfield_names(string prefix, vector<string>& names,
		vector<const Type *>& types, const vector<int>& excluded_fields) const;
	bool is_signed(void) const;
	bool is_long_long(void) const {
		return ((eType == eSimple) && (simple_type == eLongLong || simple_type == eULongLong));
	}
	const Type* to_unsigned(void) const;
	bool is_pointer_to_char(void) const { return ptr_type && ptr_type->eType == eSimple && (ptr_type->simple_type==eChar || ptr_type->simple_type==eUChar);}
	bool is_signed_char() const {
		return ((eType == eSimple) && (simple_type == eChar));
	}
	bool is_float() const {
		return ((eType == eSimple) && (simple_type == eFloat));
	}
	bool is_promotable(const Type* t) const;
	bool is_convertable(const Type* t) const;
	bool is_derivable(const Type* t) const;
	bool is_dereferenced_from(const Type* t) const;
	bool is_equivalent(const Type* t) const;
	bool needs_cast(const Type* t) const;
	bool is_unamed_padding(size_t index) const;
	bool is_full_bitfields_struct() const;
	bool is_bitfield(size_t index) const ;
	bool has_bitfields() const;
	bool has_padding(void) const;
	bool contain_pointer_field(void) const;
	bool is_const_struct_union() const;
	bool is_volatile_struct_union() const;
	bool has_assign_ops() const  { return has_assign_ops_; }
    bool has_implicit_nontrivial_assign_ops() const {
        return has_implicit_nontrivial_assign_ops_;
    }
	bool is_int(void) const { return eType == eSimple && simple_type != eVoid;}
	bool is_aggregate(void) const { return eType == eStruct || eType == eUnion;}
	bool match(const Type* t, enum eMatchType mt) const;
	unsigned long SizeInBytes(void) const;
	void Output(std::ostream &) const;
	std::string printf_directive(void) const;
	static Type* find_pointer_type(const Type* t, bool add);
	static Type* find_type(const Type* t);

// private:
	eTypeDesc eType;
	const Type *ptr_type;
	eSimpleType simple_type;
	vector<unsigned int> dimensions;    // for array types
	vector<const Type*> fields;         // for struct/union types
	unsigned int sid;                   // sequence id, for struct/union types

	bool used;                          // whether any variable declared with this type
	bool printed;                       // whether this struct/union has been printed in the random program
	const bool packed_;					// whether this struct/union should be packed
	bool has_assign_ops_;				// assign ops are needed if we have:
										// struct S0 foo; volatile struct S0 bar; ... foo=bar;
    bool has_implicit_nontrivial_assign_ops_;   // if a struct has a struct with assign ops as a field,
                                        // than the former struct also has assign ops;
                                        // also true if struct itself has assign ops
	vector<CVQualifiers> qfers_;		// conresponds to each element of fields
					// It's a tradeoff between the current implementation and the
					// need of struct's level type qualifiers.
	vector<int> bitfields_length_;		// -1 means it's a regular field

	static Type *void_type;
private:
	DISALLOW_COPY_AND_ASSIGN(Type);

	static const Type *simple_types[MAX_SIMPLE_TYPES];

	// Package init.
	friend void GenerateAllTypes(void);
};

void GenerateAllTypes(void);
const Type * get_int_type(void);
void OutputStructUnionDeclarations(std::ostream &);
void OutputStructAssignOps(Type* type, std::ostream &out, bool vol);
void OutputStructUnion(Type* type, std::ostream &out);

///////////////////////////////////////////////////////////////////////////////

#endif // TYPE_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

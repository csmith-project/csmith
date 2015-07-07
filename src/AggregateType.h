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

#ifndef AGGREGATETYPE_H
#define AGGREGATETYPE_H

///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <ostream>
#include <vector>
#include "CommonMacros.h"
#include "StatementAssign.h"
#include "CVQualifiers.h"
#include "Type.h"
#include "PointerType.h"

using namespace std;

/*
 *
 */
class AggregateType : public Type
{
public:
	
	static void get_all_ok_struct_union_types(vector<AggregateType *> &ok_types, bool no_const, bool no_volatile, bool need_int_field, bool bStruct);
	
	static const Type *choose_random_struct_from_type(const Type* type, bool no_volatile);

	static const AggregateType* choose_random_struct_union_type(vector<AggregateType *> &ok_types);
	
	// make a random struct or union type
	static AggregateType* make_random_struct_type(void);
	
	static AggregateType* make_random_union_type(void);

	AggregateType(vector<const Type*>& struct_fields, bool isStruct, bool packed,
			vector<CVQualifiers> &qfers, vector<int> &fields_length);
	~AggregateType() { /* Nothing else to do. */ }

	bool has_int_field() const ;
	int get_struct_depth(void) const ;
	bool is_unamed_padding(size_t index) const ;
	bool is_full_bitfields_struct() const ;
	bool is_bitfield(size_t index) const ;
	bool has_bitfields() const ;
	bool has_padding(void) const ;
	bool contain_pointer_field(void) const ;
	bool is_const_struct_union() const ;
	bool is_volatile_struct_union() const ;
	void get_int_subfield_names(string prefix, vector<string>& names, const vector<int>& excluded_fields) const ;
	
	unsigned long SizeInBytes(void) const ;
	void Output(std::ostream &) const ;
	std::string printf_directive(void) const ;
	
	vector<const Type*> fields;         // for struct/union types
	unsigned int sid;                   // sequence id, for struct/union types

	bool printed;                       // whether this struct/union has been printed in the random program
	const bool packed_;					// whether this struct/union should be packed
	vector<CVQualifiers> qfers_;		// conresponds to each element of fields
					// It's a tradeoff between the current implementation and the
					// need of struct's level type qualifiers.
	vector<int> bitfields_length_;		// -1 means it's a regular field


private:


	static void make_one_bitfield(vector<const Type*> &random_fields,
				vector<CVQualifiers> &qualifiers,
				vector<int> &fields_length);

	static void make_one_struct_field(vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length);

	static void make_one_union_field(vector<const Type*> &fields, vector<CVQualifiers> &qfers, vector<int> &lens);

	static void make_full_bitfields_struct_fields(size_t field_cnt, vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length);

	static void make_normal_struct_fields(size_t field_cnt, vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length);

	static bool has_aggregate_field(const vector<const Type *> &fields);

	static bool has_longlong_field(const vector<const Type *> &fields);

};

void OutputStructUnionDeclarations(std::ostream &);

#endif // AGGREGATETYPE_H
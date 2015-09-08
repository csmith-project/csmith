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

using namespace std;

static void OutputStructUnion(Type* type, std::ostream &out);
///////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------
 /* constructor for struct or union types
  *******************************************************/
AggregateType::AggregateType(vector<const Type*>& struct_fields, bool isStruct, bool packed,
			vector<CVQualifiers> &qfers, vector<int> &fields_length) :
    fields(struct_fields),
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

void
AggregateType::get_all_ok_struct_union_types(vector<AggregateType *> &ok_types, bool no_const, bool no_volatile, bool need_int_field, bool bStruct)
{
	vector<Type *>::iterator i;
	for(i = AllTypes.begin(); i != AllTypes.end(); ++i) {
		Type* t = (*i);
		if (bStruct && t->eType != eStruct) continue;
		if (!bStruct && t->eType != eUnion) continue;
		if ((no_const && t->is_const_struct_union()) ||
			(no_volatile && t->is_volatile_struct_union()) ||
			(need_int_field && (!t->has_int_field()))) {
			continue;
		}
		ok_types.push_back(dynamic_cast<AggregateType *>(t));
	}
}

const AggregateType*
AggregateType::choose_random_struct_union_type(vector<AggregateType *> &ok_types)
{
	size_t sz = ok_types.size();
	assert(sz > 0);

	int index = rnd_upto(ok_types.size());
	assert(index >= 0);
	AggregateType *rv_type = ok_types[index];
	if (!rv_type->used) {
		Bookkeeper::record_type_with_bitfields(rv_type);
		rv_type->used = true;
	}
	return rv_type;
}

const Type*
AggregateType::choose_random_struct_from_type(const Type* type, bool no_volatile)
{
	if (!type)
		return NULL;

	const Type* t = type;
	vector<AggregateType *> ok_struct_types;
	get_all_ok_struct_union_types(ok_struct_types, no_volatile, false, true, true);

	if (ok_struct_types.size() > 0) {
		t = AggregateType::choose_random_struct_union_type(ok_struct_types);
	}
	return t;
}

void
AggregateType::make_one_bitfield(vector<const Type*> &random_fields, vector<CVQualifiers> &qualifiers,
			vector<int> &fields_length)
{
	int max_length = CGOptions::int_size() * 8;
	bool sign = rnd_flipcoin(BitFieldsSignedProb);

	const Type *type = sign ? &Type::get_simple_type(eInt) : &Type::get_simple_type(eUInt);
	random_fields.push_back(type);
	CVQualifiers qual = CVQualifiers::random_qualifiers(type, FieldConstProb, FieldVolatileProb);
	
	qualifiers.push_back(qual);
	int length = rnd_upto(max_length);

	bool no_zero_len = fields_length.empty() || (fields_length.back() == 0);
	// force length to be non-zero is required
	if (length == 0 && no_zero_len) {
		if (max_length <= 2) length = 1;
		else length = rnd_upto(max_length - 1) + 1;
	}
	fields_length.push_back(length);
}

// ---------------------------------------------------------------------
void
AggregateType::make_full_bitfields_struct_fields(size_t field_cnt, vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length)
{
	for (size_t i=0; i<field_cnt; i++) {
		bool is_non_bitfield = rnd_flipcoin(ScalarFieldInFullBitFieldsProb);
		if (is_non_bitfield) {
			make_one_struct_field(random_fields, qualifiers, fields_length);
		}
		else {
			make_one_bitfield(random_fields, qualifiers, fields_length);
		}
	}
}

void
AggregateType::make_one_struct_field(vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length)
{
	VectorFilter f;
	unsigned int type_index = 0;
	Type *typ = 0;
	for( type_index = 0; type_index < AllTypes.size(); type_index++) {
		typ = AllTypes[type_index];
		assert(typ);
		if (typ->eType == eSimple) {
			Filter *filter = SIMPLE_TYPES_PROB_FILTER;
			if(filter->filter(typ->simple_type))
				f.add(type_index);
		}
		else if ((typ->eType == eStruct) && (!CGOptions::return_structs())) {
			f.add(type_index);
		}
		if (typ->get_struct_depth() >= CGOptions::max_nested_struct_level()) {
			f.add(type_index);
		}
	}
	type_index = rnd_upto(AllTypes.size(), &f);
	const Type* type = AllTypes[type_index];
	random_fields.push_back(type);
	CVQualifiers qual = CVQualifiers::random_qualifiers(type, FieldConstProb, FieldVolatileProb);
	qualifiers.push_back(qual);
	fields_length.push_back(-1);
}

void
AggregateType::make_one_union_field(vector<const Type*> &fields, vector<CVQualifiers> &qfers, vector<int> &lens)
{
	bool is_bitfield = CGOptions::bitfields() && !CGOptions::ccomp() && rnd_flipcoin(BitFieldInNormalStructProb);
	if (is_bitfield) {
		make_one_bitfield(fields, qfers, lens);
	}
	else {
		size_t i;
		vector<Type*> ok_types;
		// filter out struct types containing bit-fields. Their layout is implementation
		// defined, we don't want to mess with them in unions for now
		for (i=0; i<AllTypes.size(); i++) {
			if (!AllTypes[i]->has_bitfields()) {
				ok_types.push_back(AllTypes[i]);
			}
		}

		// find of struct types
		vector<Type*> struct_types;
		for (size_t i=0; i<ok_types.size(); i++) {
			if (ok_types[i]->eType == eStruct) {
				struct_types.push_back(ok_types[i]);
			}
		}
		const Type* type = NULL;
		do {
			// 10% chance to be struct field
			if (!struct_types.empty() && pure_rnd_flipcoin(10)) {
				type = struct_types[pure_rnd_upto(struct_types.size())];
				assert(type->eType == eStruct);
			}
			// 10% chance to be char* if pointer is allowed
			else if (CGOptions::pointers() && CGOptions::int8() && pure_rnd_flipcoin(10)) {
				type = PointerType::find_pointer_type(&get_simple_type(eChar), true);
			}
			else {
				unsigned int i = pure_rnd_upto(ok_types.size());
				const Type* t = ok_types[i];
				// no union in union?
				if (t->eType == eUnion ||
					(t->eType == eSimple && SIMPLE_TYPES_PROB_FILTER->filter(t->simple_type))) {
					continue;
				}
				type = t;
			}
		} while (type == NULL);

		fields.push_back(type);
		CVQualifiers qual = CVQualifiers::random_qualifiers(type, FieldConstProb, FieldVolatileProb);
		qfers.push_back(qual);
		lens.push_back(-1);
	}
}

void
AggregateType::make_normal_struct_fields(size_t field_cnt, vector<const Type*> &random_fields,
					vector<CVQualifiers> &qualifiers,
					vector<int> &fields_length)
{
	for (size_t i=0; i<field_cnt; i++)
	{
		bool is_bitfield = CGOptions::bitfields() && rnd_flipcoin(BitFieldInNormalStructProb);
		if (is_bitfield) {
			make_one_bitfield(random_fields, qualifiers, fields_length);
		}
		else {
			make_one_struct_field(random_fields, qualifiers, fields_length);
		}
	}
}

bool
AggregateType::has_aggregate_field(const vector<const Type *> &fields)
{
	for (vector<const Type *>::const_iterator iter = fields.begin(),
       iter_end = fields.end(); iter != iter_end; ++iter) {
    if ((*iter)->is_aggregate())
        return true;
	}
	return false;
}

AggregateType *
AggregateType::make_random_struct_type(void)
{
    size_t field_cnt = 0;
    size_t max_cnt = CGOptions::max_struct_fields();
    if (CGOptions::fixed_struct_fields())
        field_cnt = max_cnt;
    else
        field_cnt = rnd_upto(max_cnt) + 1;
    vector<const Type*> random_fields;
    vector<CVQualifiers> qualifiers;
    vector<int> fields_length;
    bool is_bitfields = CGOptions::bitfields() && rnd_flipcoin(BitFieldsCreationProb);
    
    //if (CGOptions::bitfields())
    if (is_bitfields)
        make_full_bitfields_struct_fields(field_cnt, random_fields, qualifiers, fields_length);
    else
        make_normal_struct_fields(field_cnt, random_fields, qualifiers, fields_length);

    // for now, no union type
    bool packed = false;
    if (CGOptions::packed_struct()) {
	if (CGOptions::ccomp() && (has_aggregate_field(random_fields) || has_longlong_field(random_fields))) {
		// Nothing to do
	}
	else {
            packed = rnd_flipcoin(50);
        }
    }

    AggregateType* new_type = new AggregateType(random_fields, true, packed, qualifiers, fields_length);
    return new_type;
}

AggregateType *
AggregateType::make_random_union_type(void)
{
    size_t max_cnt = CGOptions::max_union_fields();
    size_t field_cnt = rnd_upto(max_cnt) + 1;

    vector<const Type*> fields;
    vector<CVQualifiers> qfers;
    vector<int> lens;

	for (size_t i=0; i<field_cnt; i++) {
		make_one_union_field(fields, qfers, lens);
		assert(!fields.back()->has_bitfields());
	}
    AggregateType* new_type = new AggregateType(fields, false, false, qfers, lens);
    return new_type;
}

bool
AggregateType::has_longlong_field(const vector<const Type *> &fields)
{
	for (vector<const Type *>::const_iterator iter = fields.begin(),
       iter_end = fields.end(); iter != iter_end; ++iter) {
    if ((*iter)->is_long_long())
        return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
bool
AggregateType::is_const_struct_union() const
{
	assert(fields.size() == qfers_.size());

	for (size_t i = 0; i < fields.size(); ++i) {
		const Type *field = fields[i];
		if (field->is_const_struct_union()) {
			return true;
		}
		const CVQualifiers& cf = qfers_[i];
		if (cf.is_const()) return true;
	}
	return false;
}

bool
AggregateType::is_volatile_struct_union() const
{
	assert(fields.size() == qfers_.size());

	for (size_t i = 0; i < fields.size(); ++i) {
		const Type *field = fields[i];
		if (field->is_volatile_struct_union()) {
			return true;
		}
		const CVQualifiers& cf = qfers_[i];
		if (cf.is_volatile())
			return true;
	}
	return false;
}
// ---------------------------------------------------------------------
int
AggregateType::get_struct_depth() const
{
    int depth = 0;
    if (eType == eStruct) {
		depth++;
		int max_depth = 0;
		for (size_t i=0; i<fields.size(); i++) {
			int field_depth = fields[i]->get_struct_depth();
			if(field_depth > max_depth) {
				max_depth = field_depth;
			}
		}
		depth += max_depth;
	}
	return depth;
}

bool
AggregateType::is_unamed_padding(size_t index) const
{
	size_t sz = bitfields_length_.size();
	if (sz == 0)
		return false;

	assert(index < sz);
	return (bitfields_length_[index] == 0);
}

bool
AggregateType::is_bitfield(size_t index) const
{
	assert(index < bitfields_length_.size());
	return (bitfields_length_[index] >= 0);
}

bool
AggregateType::has_bitfields() const
{
	for (size_t i=0; i<fields.size(); i++) {
		if (bitfields_length_[i] >= 0) {
			return true;
		}
		if (fields[i]->eType == eStruct && fields[i]->has_bitfields()) {
			return true;
		}
	}
	return false;
}

// conservatively assume padding is present in all unpacked structures
// or whenever there is bitfields
bool
AggregateType::has_padding(void) const
{
	if (eType == eStruct && !packed_) return true;
	for (size_t i=0; i<fields.size(); i++) {
		if (is_bitfield(i) || fields[i]->has_padding()) {
			return true;
		}
	}
	return false;
}

bool
AggregateType::is_full_bitfields_struct() const
{
	if (eType != eStruct) return false;
	size_t i;
	for (i = 0; i < bitfields_length_.size(); ++i) {
		if (bitfields_length_[i] < 0)
			return false;
	}
	return true;
}

bool
AggregateType::has_int_field() const
{
	for (size_t i=0; i<fields.size(); ++i) {
		const Type* t = fields[i];
		if (t->has_int_field()) return true;
	}
	return false;
}

unsigned long
AggregateType::SizeInBytes(void) const
{
    size_t i;
    switch (eType) {
    	case eUnion: {
    	    unsigned int max_size = 0;
    	    for (i=0; i<fields.size(); i++) {
    			unsigned int sz = 0;
    			if (is_bitfield(i)) {
    				assert(i < bitfields_length_.size());
    				sz = (int)(ceil(bitfields_length_[i] / 8.0) * 8);
    			} else {
    				sz = fields[i]->SizeInBytes();
    			}
    			if (sz == SIZE_UNKNOWN) return sz;
    	        if (sz > max_size) {
    	            max_size = sz;
    	        }
    	    }
    	    return max_size;
    	}
    	case eStruct: {
    		if (!this->packed_) return SIZE_UNKNOWN;
    		// give up if there are bitfields, too much compiler-dependence and machine-dependence
    		if (this->has_bitfields()) return SIZE_UNKNOWN;
    	    unsigned int total_size = 0;
    	    for (i=0; i<fields.size(); i++) {
    			unsigned int sz = fields[i]->SizeInBytes();
    			if (sz == SIZE_UNKNOWN) return sz;
    	        total_size += sz;
    	    }
    	    return total_size;
        }
        default:
            return 0;
    }
}



void
AggregateType::get_int_subfield_names(string prefix, vector<string>& names, const vector<int>& excluded_fields) const
{
	size_t i;
	size_t j = 0;
	for (i=0; i<fields.size(); i++) {
		if (is_unamed_padding(i)) continue; // skip 0 length bitfields
		// skip excluded fields
		if (std::find(excluded_fields.begin(), excluded_fields.end(), j) != excluded_fields.end()) {
			j++;
			continue;
		}
		ostringstream oss;
		oss << prefix << ".f" << j++;
		vector<int> empty;
		fields[i]->get_int_subfield_names(oss.str(), names, empty);
	}
}

bool
AggregateType::contain_pointer_field(void) const
{
	if (eType == eStruct || eType == eUnion) {
		for (size_t i=0; i<fields.size(); i++) {
			if (fields[i]->contain_pointer_field()) {
				return true;
			}
		}
	}
	return false;
}

// ---------------------------------------------------------------------
void
AggregateType::Output(std::ostream &out) const
{
	switch (eType) {
	case eUnion:     out << "union U" << sid; break;
	case eStruct:    out << "struct S" << sid; break;
	}
}

// ---------------------------------------------------------------------
/* print struct definition (fields etc)
 *************************************************************/
static void 
OutputStructUnion(Type* aggregate_type, std::ostream &out)
{
    size_t i;
    // sanity check
    assert (aggregate_type->is_aggregate());
	AggregateType * type = dynamic_cast<AggregateType *>(aggregate_type);
    if (!type->printed) {
        // output dependent structs, if any
        for (i=0; i<type->fields.size(); i++) {
			if (type->fields[i]->is_aggregate()) {
                OutputStructUnion((Type*)type->fields[i], out);
            }
        }
        // output myself
        if (type->packed_) {
            if (!CGOptions::ccomp()) {
                out << "#pragma pack(push)";
                really_outputln(out);
            }
            out << "#pragma pack(1)";
            really_outputln(out);
        }
        type->Output(out);
        out << " {";
		really_outputln(out);

		assert(type->fields.size() == type->qfers_.size());
		unsigned int j = 0;
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
				out << length << ";";
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
		if (CGOptions::ccomp()) {
			out << "#pragma pack()";
		}
		else {
			out << "#pragma pack(pop)";
		}
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
OutputStructUnionDeclarations(std::ostream &out)
{
    size_t i;
    output_comment_line(out, "--- Struct/Union Declarations ---");
    for (i=0; i<Type::AllTypes.size(); i++)
    {
        Type* t = Type::AllTypes[i];
        if (t->used && (t->eType == eStruct || t->eType == eUnion)) {
            OutputStructUnion(Type::AllTypes[i], out);
        }
    }
}
/*
 * return the printf directive string for the type. for example, int -> "%d"
 */
std::string
AggregateType::printf_directive(void) const
{
	string ret;
	size_t i;
	switch (eType) {
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

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

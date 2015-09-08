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

#include "PointerType.h"
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
#include "TypeConfig.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------
// List of all types used in the program
vector<PointerType *> PointerType::derived_types;

///////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------
 /* constructor for pointers
  *******************************************************/
PointerType::PointerType(const Type* t) :
    ptr_type(t)
{
	eType = ePointer;
}

// ---------------------------------------------------------------------
/* find the pointer type to the given type in existing types,
 * return 0 if not found
 *************************************************************/
PointerType *
PointerType::find_pointer_type(const Type* t, bool add)
{
    for (size_t i=0; i<derived_types.size(); i++) {
        if (derived_types[i]->ptr_type == t) {
            return derived_types[i];
        }
    }
	if (add) {
		PointerType* ptr_type = new PointerType(t);
		derived_types.push_back(ptr_type);
		return ptr_type;
	}
    return 0;
}

// ---------------------------------------------------------------------
PointerType*
PointerType::find_pointer_type(const Type * t, int indirect)
{ 
	assert(indirect >= 1 && (t->eType != ePointer));
	Type * rvtype = const_cast<Type*> (t);
	while(indirect--)
	{
		rvtype = find_pointer_type(rvtype,true);
	}
	return dynamic_cast<PointerType*> (rvtype);
}

// ---------------------------------------------------------------------
PointerType*
PointerType::make_random_pointer_type(void)
{
    //Type* new_type = 0;
    //Type* ptr_type = 0;
    // occasionally choose pointer to pointers
    if (rnd_flipcoin(20)) {
        if (derived_types.size() > 0) {
			unsigned int rnd_num = rnd_upto(derived_types.size());
			const PointerType* t = derived_types[rnd_num];
			if (t->get_indirect_level() < CGOptions::max_indirect_level()) {
				return find_pointer_type(t, true);
			}
		}
	}

    // choose a pointer to basic/aggregate types
	const Type* t = choose_random();
	// consolidate all integer pointer types into "int*", hopefully this increase
	// chance of pointer assignments and dereferences
	if (t->eType == eSimple) {
		t = get_int_type();
	}
	return find_pointer_type(t, true);
}

const PointerType *
PointerType::choose_random_pointer_type(void)
{
	unsigned int index = rnd_upto(derived_types.size());
	return derived_types[index];
}

bool
PointerType::has_pointer_type(void)
{
	return derived_types.size() > 0;
}

/*
 *
 */
void
PointerType::doFinalization(void)
{
	vector<PointerType *>::iterator j;
	for(j = derived_types.begin(); j != derived_types.end(); ++j)
		delete (*j);
	derived_types.clear();
}

// ---------------------------------------------------------------------
int
PointerType::get_indirect_level() const
{
    int level = 0;
    const Type* pt = ptr_type;
    while (pt != 0) {
        level++;
        pt = (pt->eType == ePointer) ? (dynamic_cast<const PointerType *>(pt))->ptr_type : NULL;
    }
    return level;
}

const Type*
PointerType::get_base_type(void) const
{
	const Type* tmp = this;
	while (tmp->eType == ePointer) {
		tmp = (dynamic_cast<const PointerType *>(tmp))->ptr_type;
	}
	return tmp;
}

// ---------------------------------------------------------------------
/* generally integer types can be converted to any other interger types
 * void / struct / union / array types are not. Pointers depend on the
 * type they point to. (unsigned int*) is convertable to (int*), etc
 *************************************************************/
bool
PointerType::is_convertable(const Type* t) const
{
    if (this == t)
        return true;
        if (t->eType == ePointer) {
            if (ptr_type == (dynamic_cast<const PointerType *>(t))->ptr_type) {
                return true;
            }
        if (ptr_type->eType == eSimple && (dynamic_cast<const PointerType *>(t))->ptr_type->eType == eSimple) {
            return !(TypeConfig::check_exclude_by_convert((dynamic_cast<const PointerType *>(t))->ptr_type, ptr_type));
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
PointerType::is_derivable(const Type* t) const
{
    if (this == t) {
        return true;
    }
	return is_convertable(t) || is_dereferenced_from(t) || (ptr_type==t);
}

// ---------------------------------------------------------------------
void
PointerType::Output(std::ostream &out) const
{
	ptr_type->Output( out ); out << "*"; 
}

/*
 * return the printf directive string for the type. for example, int -> "%d"
 */
std::string
PointerType::printf_directive(void) const
{
	return "0x%0x";
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

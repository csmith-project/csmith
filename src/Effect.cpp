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

#include <cassert>

#include "Effect.h"
#include "Variable.h"
#include "Block.h"
#include "Type.h"
#include "util.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

/*
 * The empty effect.
 */
const Effect Effect::empty_effect;

///////////////////////////////////////////////////////////////////////////////

/*
 * Stuff that so badly wants to be rewritten using decent STL containers and
 * algorithms.
 */
static bool
non_empty_intersection(const vector<const Variable *> &va,
					   const vector<const Variable *> &vb)
{
	vector<const Variable *>::size_type va_len = va.size();
	vector<const Variable *>::size_type vb_len = vb.size();
	vector<const Variable *>::size_type i;

	for (i = 0; i < va_len; ++i) {
		vector<const Variable *>::size_type j;
		for (j = 0; j < vb_len; ++j) {
			if (va[i]->match(vb[j]) || vb[j]->match(va[i])) {
				return true;
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * 
 */
Effect::Effect(void) :
	read_vars(0),
	write_vars(0),
	pure(true),
	side_effect_free(true)
{
	// Nothing else to do.
}

/*
 * 
 */
Effect::Effect(const Effect &e) :
	read_vars(e.read_vars),
	write_vars(e.write_vars),
	pure(e.pure),
	side_effect_free(e.side_effect_free)
{
	// Nothing else to do.
}

/*
 * 
 */
Effect::~Effect(void)
{
	// Nothing else to do.
}

/*
 * 
 */
Effect &
Effect::operator=(const Effect &e)
{
	if (this == &e) {
		return *this;
	}

	read_vars = e.read_vars;
	write_vars = e.write_vars;
	pure = e.pure;
	side_effect_free = e.side_effect_free;

	return *this;
}

/*
 *
 */
void
Effect::read_var(const Variable *v)
{
	if (!is_read(v)) {
		read_vars.push_back(v);
        //if (v->type->eType == eStruct) {
        //    for (size_t i=0; i<v->field_vars.size(); i++) {
        //        read_var(v->field_vars[i]);
        //    }
        //    //read_vars.insert(read_vars.end(), v->field_vars.begin(), v->field_vars.end());
        //}
	}
	pure &= (v->is_const() && !v->is_volatile());
	side_effect_free &= !v->is_volatile();
}

/*
 *
 */
void
Effect::write_var(const Variable *v)
{
	if (!is_written(v)) {
		write_vars.push_back(v);
        //if (v->type->eType == eStruct) {
        //    for (size_t i=0; i<v->field_vars.size(); i++) {
        //        write_var(v->field_vars[i]);
        //    }
        //    //write_vars.insert(write_vars.end(), v->field_vars.begin(), v->field_vars.end());
        //}
	}
	// pure = pure;
	// TODO: not quite correct below ---
	// --- but I think we want "side_effect_free" to mean "volatile_free".
	side_effect_free &= !v->is_volatile();
	// side_effect_free = false;
}

/*
 * 
 */
void
Effect::add_effect(const Effect &e)
{
	if (this == &e) {
		return;
	}

	// Stuff that so badly wants to be rewritten using decent STL containers
	// and algorithms... compute the union effect.

	vector<const Variable *>::size_type len;
	vector<const Variable *>::size_type i;

	len = e.read_vars.size();
	for (i = 0; i < len; ++i) {
		// this->read_var(e.read_vars[i]);
		if (!is_read(e.read_vars[i])) {
			this->read_vars.push_back(e.read_vars[i]);
		}
	}
	len = e.write_vars.size();
	for (i = 0; i < len; ++i) {
		// this->write_var(e.write_vars[i]);
		if (!is_written(e.write_vars[i])) {
			this->write_vars.push_back(e.write_vars[i]);
		}
	}

	pure &= e.pure;
	side_effect_free &= e.side_effect_free;
}

/*
 * 
 */
void
Effect::add_external_effect(const Effect &e)
{
	if (this == &e) {
		return;
	}
	
	vector<Variable *>::size_type len;
	vector<Variable *>::size_type i;

	len = e.read_vars.size();
	for (i = 0; i < len; ++i) {
		if (e.read_vars[i]->is_global()) {
			this->read_var(e.read_vars[i]);
		}
	}

	len = e.write_vars.size();
	for (i = 0; i < len; ++i) {
		if (e.write_vars[i]->is_global()) {
			this->write_var(e.write_vars[i]);
			//Make sure the "purity" is set correctly
			this->pure = false;
		}
	}
	side_effect_free &= e.side_effect_free;
}

/*
 * with call chains, we want to track write/read to stack
 * variables of caller(s) 
 */
void
Effect::add_external_effect(const Effect &e, std::vector<const Block*> call_chain)
{
	if (this == &e) {
		return;
	}
	
	vector<Variable *>::size_type len;
	vector<Variable *>::size_type i;

	len = e.read_vars.size();
	for (i = 0; i < len; ++i) {
		const Variable* var = e.read_vars[i];
		if (var->is_global()) {
			read_var(var);
		}
		else {
			vector<Variable *>::size_type j;
			for (j=0; j<call_chain.size(); j++) {
				const Block* b = call_chain[j];
				if (b->is_var_on_stack(var)) {
					break;
				}
			}
			if (j < call_chain.size()) {
				read_var(var);
			}
		}
	}

	len = e.write_vars.size();
	for (i = 0; i < len; ++i) {
		const Variable* var = e.write_vars[i];
		if (var->is_global()) {
			write_var(var);
			//Make sure the "purity" is set correctly
			pure = false;
		}
		else {
			vector<Variable *>::size_type j;
			for (j=0; j<call_chain.size(); j++) {
				const Block* b = call_chain[j];
				if (b->is_var_on_stack(var)) {
					break;
				}
			}
			if (j < call_chain.size()) {
				write_var(var);
				pure = false;
			}
		}
	}
	side_effect_free &= e.side_effect_free;
}

/*
 * TODO: tried doing this with `find', but I couldn't get all the `const's
 * right.  Gave up; wrote it by hand!
 */
bool
Effect::is_read(const Variable *v) const
{
	vector<Variable *>::size_type len = read_vars.size();
	vector<Variable *>::size_type i;

	for (i = 0; i < len; ++i) {
		if (read_vars[i] == v) {
			return true;
		}
	}
	// if we read a struct, presumingly all the fields are read too
	if (v->isFieldVarOf_) {
		return is_read(v->isFieldVarOf_);
	}
	return false;
}

/*
 * 
 */
bool
Effect::is_read(string vname) const
{
	vector<Variable *>::size_type len = read_vars.size();
	vector<Variable *>::size_type i;

	for (i = 0; i < len; ++i) {
		const string& rname = read_vars[i]->name;
		size_t pos;
		if (rname == vname) {
			return true;
		}
		// match name of a struct field with struct
		if (vname.find(".") != string::npos) {
			string tmp = vname; 
			do {
				pos = tmp.find_last_of("."); 
				tmp = tmp.substr(0, pos);
				if (tmp == rname) {
					return true;
				}
			} while (pos != string::npos);
		}
		// match name of a struct with struct field
		if (rname.find(".") != string::npos) {
			string tmp = rname;
			do {
				pos = tmp.find_last_of("."); 
				tmp = tmp.substr(0, pos);
				if (tmp == vname) {
					return true;
				}
			} while (pos != string::npos);
		}
	}
	return false;
}

/*
 * 
 */
bool
Effect::is_written(const Variable *v) const
{
	vector<Variable *>::size_type len = write_vars.size();
	vector<Variable *>::size_type i;

	for (i = 0; i < len; ++i) {
		if (write_vars[i] == v) {
			return true;
		}
	}
	// if we write a struct, presumingly all the fields are written too
	if (v->isFieldVarOf_) {
		return is_written(v->isFieldVarOf_);
	}
	return false;
}

/*
 * 
 */
bool
Effect::is_written(string vname) const
{
	vector<Variable *>::size_type len = write_vars.size();
	vector<Variable *>::size_type i;

	for (i = 0; i < len; ++i) {
		const string& wname = write_vars[i]->name;
		size_t pos;
		if (wname == vname) {
			return true;
		}
		// match name of a struct field with struct
		if (vname.find(".") != string::npos) {
			string tmp = vname; 
			do {
				pos = tmp.find_last_of("."); 
				tmp = tmp.substr(0, pos);
				if (tmp == wname) {
					return true;
				}
			} while (pos != string::npos);
		}
		// match name of a struct with struct field
		if (wname.find(".") != string::npos) {
			string tmp = wname;
			do {
				pos = tmp.find_last_of("."); 
				tmp = tmp.substr(0, pos);
				if (tmp == vname) {
					return true;
				}
			} while (pos != string::npos);
		}
	}
	return false;
}

/*
 * whether any field of a struct is been read
 */
bool
Effect::field_is_read(const Variable *v) const
{ 
	if (v->type->eType == eStruct) {  
		size_t j;
		for (j=0; j<v->field_vars.size(); j++) {
			const Variable* const field_var = v->field_vars[j];
			if (is_read(field_var) || field_is_read(field_var)) {
				return true; 
			}
		}
	}
	return false;
}

/*
 * whether any field of a struct is been written
 */
bool
Effect::field_is_written(const Variable *v) const
{ 
	if (v->type->eType == eStruct) {  
		size_t j;
		for (j=0; j<v->field_vars.size(); j++) {
			Variable* field_var = v->field_vars[j];
			if (is_written(field_var) || field_is_written(field_var)) {
				return true; 
			}
		}
	}
	return false;
}

/*
 * consolidate the read/write set 
 */
void
Effect::consolidate(void) 
{
	size_t i;
	size_t len = read_vars.size();
	for (i=0; i<len; i++) {
		const Variable* tmp = read_vars[i];
		if (tmp->is_field_var() && is_read(tmp->isFieldVarOf_)) {
			read_vars.erase(read_vars.begin() + i);
			i--;
			len--;
		}
	}
	len = write_vars.size();
	for (i=0; i<len; i++) {
		const Variable* tmp = write_vars[i];
		if (tmp->is_field_var() && is_written(tmp->isFieldVarOf_)) {
			write_vars.erase(write_vars.begin() + i);
			i--;
			len--;
		}
	}
}

/*
 * 
 */
bool
Effect::has_race_with(const Effect &e) const
{
	return (non_empty_intersection(this->read_vars,  e.write_vars)
			|| non_empty_intersection(this->write_vars, e.read_vars)
			|| non_empty_intersection(this->write_vars, e.write_vars));
}

/*
 * 
 */
bool
Effect::is_empty(void) const
{
	return read_vars.size()==0 && write_vars.size()==0;
}

/*
 *
 */
void
Effect::clear(void) 
{
	read_vars.clear();
	write_vars.clear();
	pure = side_effect_free = true;
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
void
Effect::Output(std::ostream &out) const
{
	vector<Variable *>::size_type len;
	vector<Variable *>::size_type i;

	std::ostringstream ss;

	ss << std::endl;
	ss << " * reads :";
	len = read_vars.size();
	for (i = 0; i < len; ++i) {
		ss << " ";
		read_vars[i]->OutputForComment(ss);
		
	}
	ss << endl;

	ss << " * writes:";
	len = write_vars.size();
	for (i = 0; i < len; ++i) {
		ss << " ";
		write_vars[i]->OutputForComment(ss);
	}
	ss << endl;

	output_comment_line(out, ss.str());
}

/*
 *
 */
void
Effect::update_purity(void)
{
	if (has_global_effect())
		pure = false;
}

/*
 * Return true if read_vars or write_vars has global variables.
 */
bool
Effect::has_global_effect(void) const
{
	vector<Variable *>::size_type len;
	vector<Variable *>::size_type i;

	len = read_vars.size();
	for (i = 0; i < len; ++i) {
		if (read_vars[i]->is_global()) {
			return true;
		}
	}

	len = write_vars.size();
	for (i = 0; i < len; ++i) {
		if (write_vars[i]->is_global()) {
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2015, 2017 The University of Utah
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

#include <cassert>

#include "Effect.h"
#include "Variable.h"
#include "ExpressionVariable.h"
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
non_empty_intersection(const VariableSet &va,
					   const VariableSet &vb)
{
	for (auto i = va.begin(); i != va.end(); i++) {
		for (auto j = vb.begin(); j != vb.end(); j++) {
			// It's an intersection if a variable in va is a field of another
			// variable in vb, or vice versa.
			// TODO: optimize, possibly with addition of the containing
			// struct/unions to the sets
			if ((*i)->match(*j) || (*j)->match(*i)) {
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
		read_vars.insert(v);
	}
	pure &= (v->is_const() && !v->is_volatile() && !v->is_access_once());
	side_effect_free &= (!v->is_volatile() && !v->is_access_once());
}

void
Effect::access_deref_volatile(const Variable *v, int deref_level)
{
	assert(v && "NULL Variable!");
	if (!CGOptions::strict_volatile_rule())
		return;
	while (deref_level > 0) {
		if (v->is_volatile_after_deref(deref_level)) {
			side_effect_free = false;
			return;
		}
		deref_level--;
	}
}

void
Effect::write_var(const Variable *v)
{
	if (!is_written(v)) {
		write_vars.insert(v);
	}
	// pure = pure;
	// TODO: not quite correct below ---
	// --- but I think we want "side_effect_free" to mean "volatile_free".
	side_effect_free &= (!v->is_volatile() && !v->is_access_once());
	// side_effect_free = false;
}

void
Effect::write_var_set(const VariableSet& vars)
{
	for (auto i = vars.begin(); i != vars.end(); i++) {
		write_var(*i);
	}
}

/*
 * compute the union effect.
 */
void
Effect::add_effect(const Effect &e, bool include_lhs_effects)
{
	if (this == &e) {
		return;
	}

	for (auto i = e.read_vars.begin(); i != e.read_vars.end(); ++i) {
		if (!is_read(*i)) {
			read_vars.insert(*i);
		}
	}

	for (auto i = e.write_vars.begin(); i != e.write_vars.end(); ++i) {
		if (!is_written(*i)) {
			write_vars.insert(*i);
		}
	}

	if (include_lhs_effects) {
		add_variables_to_set(lhs_write_vars, e.get_lhs_write_vars());
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

	for (auto i = e.read_vars.begin(); i != e.read_vars.end(); ++i) {
		const Variable* v = *i;
		if (v->is_global()) {
			this->read_var(v);
		}
	}

	for (auto i = e.write_vars.begin(); i != e.write_vars.end(); ++i) {
		const Variable* v = *i;
		if (v->is_global()) {
			this->write_var(v);
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

	for (auto i = e.read_vars.begin(); i != e.read_vars.end(); ++i) {
		const Variable* var = *i;
		if (var->is_global()) {
			read_var(var);
		}
		else {
			size_t j;
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

	for (auto i = e.write_vars.begin(); i != e.write_vars.end(); ++i) {
		const Variable* var = *i;
		if (var->is_global()) {
			write_var(var);
			//Make sure the "purity" is set correctly
			pure = false;
		}
		else {
			size_t j;
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
 * Check whether a variable is read in this effect.
 */
bool
Effect::is_read(const Variable *v) const
{
	if (read_vars.find(v) != read_vars.end()) {
		return true;
	}

	// if we read a struct, presumingly all the fields are read too
	// however we can not say the same thing for unions: reading a particular
	// unions field can cause unspecified behaviors, while reading the whole
	// union won't
	if (v->field_var_of && v->field_var_of->type->eType == eStruct) {
		return is_read(v->field_var_of);
	}

	return false;
}

/*
 * Check whether a variable is written in this effect.
 */
bool
Effect::is_written(const Variable *v) const
{
	if (write_vars.find(v) != write_vars.end()) {
		return true;
	}
	// if we write a struct/union, presumingly all the fields are written too
	if (v->field_var_of) {
		return is_written(v->field_var_of);
	}
	return false;
}

/*
 * whether any field of a struct/union is been read
 */
bool
Effect::field_is_read(const Variable *v) const
{
	size_t j;
	if (v->is_aggregate()) {
		for (j=0; j<v->field_vars.size(); j++) {
			Variable* field_var = v->field_vars[j];
			if (is_read(field_var) || field_is_read(field_var)) {
				return true;
			}
		}
	}
	return false;
}

/*
 * whether any field of a struct/union is been written
 */
bool
Effect::field_is_written(const Variable *v) const
{
	size_t j;
	if (v->is_aggregate()) {
		for (j=0; j<v->field_vars.size(); j++) {
			Variable* field_var = v->field_vars[j];
			if (is_written(field_var) || field_is_written(field_var)) {
				return true;
			}
		}
	}
	return false;
}

bool
Effect::sibling_union_field_is_read(const Variable *v) const
{
	const Variable* you = v->get_collective()->get_container_union();
	if (you) {
		for (auto i = read_vars.begin(); i != read_vars.end(); i++) {
			const Variable* me = (*i)->get_collective()->get_container_union();
			if (you == me) {
				return true;
			}
		}
	}
	return false;
}

bool
Effect::sibling_union_field_is_written(const Variable *v) const
{
	const Variable* you = v->get_collective()->get_container_union();
	if (you) {
		for (auto i = write_vars.begin(); i != write_vars.end(); i++) {
			const Variable* me = (*i)->get_collective()->get_container_union();
			if (you == me) {
				return true;
			}
		}
	}
	return false;
}

bool
Effect::is_read_partially(const Variable* v) const
{
	return is_read(v) || field_is_read(v) || sibling_union_field_is_read(v);
}

bool
Effect::is_written_partially(const Variable* v) const
{
	return is_written(v) || field_is_written(v) || sibling_union_field_is_written(v);
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
	std::ostringstream ss;

	ss << std::endl;
	ss << " * reads :";
	for (auto i = read_vars.begin(); i != read_vars.end(); ++i) {
		ss << " ";
		(*i)->OutputForComment(ss);
	}
	ss << endl;

	ss << " * writes:";
	for (auto i = write_vars.begin(); i != write_vars.end(); ++i) {
		ss << " ";
		(*i)->OutputForComment(ss);
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
	for (auto i = read_vars.begin(); i != read_vars.end(); ++i) {
		if ((*i)->is_global()) {
			return true;
		}
	}

	for (auto i = write_vars.begin(); i != write_vars.end(); ++i) {
		if ((*i)->is_global()) {
			return true;
		}
	}
	return false;
}

/*
 * whether any field of an union is been read
 */
bool
Effect::union_field_is_read(void) const
{
	for (auto i = read_vars.begin(); i != read_vars.end(); i++) {
		if ((*i)->is_inside_union_field()) {
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

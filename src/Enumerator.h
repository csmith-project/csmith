// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2015 The University of Utah
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

#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include <map>
#include <cassert>

using namespace std;

template <class Name>
class Enumerator {
public:
	Enumerator();

	~Enumerator();

	void add_elem(Name name, int bound);

	void add_bool_elem(Name name, int value);

	void add_bool_elem_of_bool(Name name, bool value);

	int get_elem(Name name);

	Enumerator *next();

	Enumerator *end() { return NULL; }

	Enumerator *begin();

	bool is_changed(Name name);

private:

	class EnumObject {
	public:
		EnumObject(int bound, bool is_bool, bool bool_value)
			: bound_(bound),
		  	  current_value_(0),
			  is_bool_(is_bool),
			  bool_value_(bool_value),
			  changed_(false)
		{
			assert(bound_ > 0);
		}

		~EnumObject() { }

		int bound() { return bound_; }

		bool bool_value() { return bool_value_; }

		int get_current_value() { return current_value_; }

		bool is_bool() { return is_bool_; }

		bool next()
		{
			current_value_++;
			if (current_value_ < bound_) {
				changed_ = true;
				return true;
			}
			else {
				current_value_--;
				return false;
			}
		}

		bool good_value()
		{
			return (current_value_ < bound_);
		}

		void clear_value() { current_value_ = 0; }

		void reset_changed() { changed_ = 0; }

		bool is_changed() { return changed_; }

	private:
		const int bound_;

		int current_value_;

		const bool is_bool_;

		const bool bool_value_;

		bool changed_;
	};

	void reset_all_changed();

	void reset_pos();

	void reset_after_backward_pos();

	bool roll_back_current_pos();

	std::map<Name, EnumObject*> objs_;

	typename std::map<Name, EnumObject*>::iterator forward_pos_;

	typename std::map<Name, EnumObject*>::iterator backward_pos_;

};

/////////////////////////////////////////////////////////////////////////////////////

template <class Name>
Enumerator<Name>::Enumerator()
{
	forward_pos_ = objs_.end();
	backward_pos_ = objs_.end();
}

template <class Name>
Enumerator<Name>::~Enumerator()
{
	typename map<Name, EnumObject*>::iterator i;
	for (i = objs_.begin(); i != objs_.end(); ++i) {
		if ((*i).second != NULL)
			delete (*i).second;
	}
	objs_.clear();
}

template <class Name>
void
Enumerator<Name>::reset_pos()
{
	forward_pos_ = objs_.end();
	--forward_pos_;
	backward_pos_ = forward_pos_;
	--backward_pos_;
}

template <class Name>
Enumerator<Name> *
Enumerator<Name>::begin()
{
	reset_pos();
	EnumObject *obj = (*forward_pos_).second;
	assert(obj);
	if (forward_pos_ == objs_.begin() && !obj->good_value())
		return NULL;
	return this;
}

template <class Name>
void
Enumerator<Name>::add_elem(Name name, int bound)
{
	assert(objs_.find(name) == objs_.end());

	objs_[name] = new EnumObject(bound, false, false);
}

template <class Name>
void
Enumerator<Name>::add_bool_elem_of_bool(Name name, bool value)
{
	assert(objs_.find(name) == objs_.end());

	int bound = value ? 2 : 1;

	objs_[name] = new EnumObject(bound, true, false);
}

template<class Name>
void
Enumerator<Name>::add_bool_elem(Name name, int value)
{
	int bound = 0;
	bool bool_value = false;
	if (value == 0) {
		bound = 1;
		bool_value = false;
	}
	else if (value == 100) {
		bound = 1;
		bool_value = true;
	}
	else {
		bound = 2;
	}

	assert(objs_.find(name) == objs_.end());

	objs_[name] = new EnumObject(bound, true, bool_value);
}

template <class Name>
int
Enumerator<Name>::get_elem(Name name)
{
	assert(objs_.find(name) != objs_.end());

	EnumObject *obj = objs_[name];
	assert(obj);
	int rv = obj->get_current_value();
	assert(rv >= 0);
	if (obj->is_bool()) {
		if (obj->bound() == 1)
			return obj->bool_value();
		else
			return (rv != 0);
	}
	else {
		return rv;
	}
}

template <class Name>
void
Enumerator<Name>::reset_after_backward_pos()
{
	typename map<Name, EnumObject*>::iterator i = backward_pos_;
	++i;
	while(i != objs_.end()) {
		EnumObject *obj = (*i).second;
		assert(obj);
		obj->clear_value();
		++i;
	}
	forward_pos_ = backward_pos_;
	++forward_pos_;
}

template <class Name>
bool
Enumerator<Name>::roll_back_current_pos()
{
	if (backward_pos_ == objs_.begin()) {
		EnumObject *obj = (*backward_pos_).second;
		assert(obj);
		bool rv = obj->next();
		if (!rv)
			return false;
		reset_after_backward_pos();
		reset_pos();
		return true;
	}
	else {
		EnumObject *obj = (*backward_pos_).second;
		assert(obj);
		if (obj->next()) {
			reset_after_backward_pos();
			reset_pos();
			//forward_pos_= backward_pos_;
			//++forward_pos_;
			return true;
		}
		else {
			--backward_pos_;
			return roll_back_current_pos();
		}
	}
}

template <class Name>
bool
Enumerator<Name>::is_changed(Name name)
{
	assert(objs_.find(name) != objs_.end());

	EnumObject *obj = objs_[name];
	assert(obj);
	return obj->is_changed();
}

template <class Name>
void
Enumerator<Name>::reset_all_changed()
{
	typename map<Name, EnumObject*>::iterator i;
	for (i = objs_.begin(); i != objs_.end(); ++i) {
		EnumObject *obj = (*i).second;
		assert(obj);
		obj->reset_changed();
	}
}

template <class Name>
Enumerator<Name> *
Enumerator<Name>::next()
{
	assert(forward_pos_ != objs_.end());

	reset_all_changed();
	EnumObject *obj = (*forward_pos_).second;
	assert(obj);
	if (obj->next()) {
		return this;
	}
	else {
		++forward_pos_;
		if (forward_pos_ == objs_.end()) {
			if(roll_back_current_pos())
				return this;
			else
				return NULL;
		}
		else {
			reset_pos();
			return next();
		}
	}

	return NULL;
}
#endif // ENUMERATOR_H

// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011 The University of Utah
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

#ifndef PROBABILITY_TABLE_H
#define PROBABILITY_TABLE_H

#include <vector>
#include <algorithm>
#include <functional>
#include "Probabilities.h"
#include "VectorFilter.h"
#include "CGOptions.h"
#include "random.h"

using namespace std;

template <class Key, class Value>
class Table {
public:
	Table(Key k, Value v);

	~Table();

	Key get_key() { return key_; }

	Value get_value() { return value_; }

private:
	Key key_;

	Value value_;
};

template <class Key, class Value>
class ProbabilityTable {
	typedef Table<Key, Value> Tbl;

public:
	ProbabilityTable();

	~ProbabilityTable();

	void initialize(ProbName pname);

	void add_elem(Key k, Value v);

	void sorted_insert(Tbl *t);

	Value get_value(Key k);

	Filter *get_filter() { return vf_; }

	bool filter(Key k);
private:
	Key curr_max_key_;

	std::vector<Tbl *> table_;

	VectorFilter *vf_;

	std::vector<Key> vs_;
};

template <class Key, class Value>
Table<Key, Value>::Table(Key k, Value v)
	: key_(k),
	  value_(v)
{

}

template <class Key, class Value>
Table<Key, Value>::~Table()
{

}

template <class Key, class Value>
ProbabilityTable<Key, Value>::ProbabilityTable()
	: curr_max_key_(0),
          vf_(NULL)
{
	table_.clear();
}

template <class Key, class Value>
ProbabilityTable<Key, Value>::~ProbabilityTable()
{
	typename vector<Tbl *>::iterator i;
	for (i = table_.begin(); i != table_.end(); ++i) 
		delete (*i);
	table_.clear();
}

template <class Key, class Value>
void ProbabilityTable<Key, Value>::initialize(ProbName pname)
{
	Probabilities *impl_ = Probabilities::GetInstance();
	impl_->set_prob_table(this, pname);
}

template <class Key, class Value>
bool my_less(Table<Key, Value> *t, Key k2)
{
	Key k1 = t->get_key();
	return (k1 < k2);
}

template <class Key, class Value>
bool my_greater(Table<Key, Value> *t, Key k2)
{
	Key k1 = t->get_key();
	return (k1 > k2);
}

template <class Key, class Value>
void
ProbabilityTable<Key, Value>::sorted_insert(Tbl *t)
{
	assert(t);
	
	Key k = t->get_key();

	if (table_.empty()) {
		table_.push_back(t);
		curr_max_key_ = k;
		return;
	}
	
	typename vector<Tbl *>::iterator i;
	for (i=table_.begin(); i!=table_.end(); i++) {
		if (my_greater<Key, Value>(*i, k)) {
			break;
		}
	}
	//i = find_if(table_.begin(), table_.end(), std::bind2nd(std::ptr_fun(my_greater<Key, Value>), k));

	if (i != table_.end()) {
		table_.insert(i, t);	
	}
	else {
		table_.push_back(t);
		curr_max_key_ = k;
	}
}

template <class Key, class Value>
void
ProbabilityTable<Key, Value>::add_elem(Key k, Value v)
{
	Tbl *t = new Tbl(k, v);
	sorted_insert(t);
}

template <class Key, class Value>
Value
ProbabilityTable<Key, Value>::get_value(Key k)
{
	assert(k < curr_max_key_);

	typename vector<Tbl *>::iterator i;
	i = find_if(table_.begin(), table_.end(), std::bind2nd(std::ptr_fun(my_greater<Key, Value>), k));

	assert(i != table_.end());
	return (*i)->get_value();
}

template <class Key, class Value>
bool
ProbabilityTable<Key, Value>::filter(Key k)
{
	assert(k < curr_max_key_);
	if (vf_ == NULL) {
		typename vector<Tbl *>::iterator i;
		for (i = table_.begin(); i != table_.end(); ++i) {
			Key tmp_k = (*i)->get_key();
			assert(tmp_k > 0);
			vs_.push_back(tmp_k-1);
		}
		vf_ = new VectorFilter(vs_, NOT_FILTER_OUT);
	}

	vf_->disable(fDefault);
	return vf_->filter(k);
}

#endif

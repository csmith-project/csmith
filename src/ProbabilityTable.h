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

#ifndef PROBABILITY_TABLE_H
#define PROBABILITY_TABLE_H

#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>
#include "Probabilities.h"
#include "VectorFilter.h"
#include "CGOptions.h"
#include "random.h"

using namespace std;
/*
	this class is the leaf node of construction of all tables used in this class
	typeof(key) = class
	typeof(value) = class
	 _______________
	|	|	|
	|   key	|value	|
	+_______|_______+  <------Each Individual entry is provided by this class
				the key and value can be of any class type,hence template used
			operations:
					1.get key
					2.get value
					3. constructor - for insertion key,val

*/
template <class Key, class Value>
class TableEntry {
public:
	TableEntry(Key k, Value v);

	Key get_key() { return key_; }

	Value get_value() { return value_; }

private:
	Key key_;
	Value value_;
};
/*
	      Table_    is a vector of  TableEntry class
			 _______________
			|	|	|
			+__key__|value__+
			|	|	|<--|
			+---------------+   |
			|	|	|<-----TableEntry
			+---------------+   |
curr_max_key_--------->	|	|	|<---
			+---------------+
points to last entry
which is a maximum key anyways

*/
template <class Key, class Value>
class ProbabilityTable {
	typedef TableEntry<Key, Value> Entry;

public:
	ProbabilityTable();

	~ProbabilityTable();

	void initialize(ProbName pname);

	void add_elem(Key k, Value v);

	void sorted_insert(Entry *t);

	Value get_value(Key k);

private:
	//POINTS TO LAST ENTRY OF table_
	Key curr_max_key_;
	std::vector<Entry *> table_;
};

template <class Key, class Value>
TableEntry<Key, Value>::TableEntry(Key k, Value v)
	: key_(k),
	  value_(v)
{
}

template <class Key, class Value>
ProbabilityTable<Key, Value>::ProbabilityTable()
	: curr_max_key_(0)
{
	table_.clear();
}
/*
	clears this:
	Table_
	 _______________
	|	|	|
	+__key__|value__+
	|	|	|<--|
	+---------------+   |
	|	|	|<-----TableEntry
	+---------------+   |
	|	|	|<---
	+---------------+
*/

template <class Key, class Value>
ProbabilityTable<Key, Value>::~ProbabilityTable()
{
	typename vector<Entry *>::iterator i;
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
//DEAD CODE? ISN'T USED ANYWHERE!!
//IF FIRST KEY IS LESS THAN SECOND -> RETURN true
template <class Key, class Value>
bool my_less(TableEntry<Key, Value> *t, Key k2)
{
	Key k1 = t->get_key();
	return (k1 < k2);
}
/*
	if (key(t)> K2)
		RETURN 1
	ELSE
		RETURN 0
*/
template <class Key, class Value>
bool my_greater(TableEntry<Key, Value> *t, Key k2)
{
	Key k1 = t->get_key();
	return (k1 > k2);
}
/*
short: inserts entry in @sorted location of key
Long:
     t(the TableEntry to be inserted)
     |			Table_ = sorted in ascending order of key
     V			 _______________
+------------+		|	|	|
| key |	value|		+__key__|value__+
+------------+		|	|	|<--|
			+---------------+   |
			|	|	|<-----TableEntry
			+---------------+   |
curr_max_key_--------->	|	|	|<---
			+---------------+

't' is inserted into Table_ based on appropriate location

*/
template <class Key, class Value>
void
ProbabilityTable<Key, Value>::sorted_insert(Entry *t)
{
	assert(t);

	Key k = t->get_key();

	if (table_.empty()) {
		table_.push_back(t);
		curr_max_key_ = k;
		return;
	}

	typename vector<Entry *>::iterator i;
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
/*
use: adds into Table_ new entry
	creates a TableEntry first
+------------+
| key |	value|	and inserts into Table_
+------------+
*/
template <class Key, class Value>
void
ProbabilityTable<Key, Value>::add_elem(Key k, Value v)
{
	Entry *t = new Entry(k, v);
	sorted_insert(t);
}
/*
	locates the key and returns the value[@key]
*/
template <class Key, class Value>
Value
ProbabilityTable<Key, Value>::get_value(Key k)
{
	assert(k < curr_max_key_);

	typename vector<Entry *>::iterator i;
	i = find_if(table_.begin(), table_.end(), std::bind2nd(std::ptr_fun(my_greater<Key, Value>), k));

	assert(i != table_.end());
	return (*i)->get_value();
}

class DistributionTable {
public:
	DistributionTable() { max_prob_ = 0;}
	~DistributionTable() {};

	void add_entry(int key, int prob);
	int get_max(void) const { return max_prob_;}
	int key_to_prob(int key) const;
	int rnd_num_to_key(int rnd) const;
private:
	int max_prob_;
	vector<int> keys_;
	vector<int> probs_;
};

#endif

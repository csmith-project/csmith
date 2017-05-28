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

#ifndef FACT_H
#define FACT_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <vector>
using namespace std;

enum eFactCategory {
    ePointTo=1,
	eUnionWrite=2,
    /* todo
	eIntRange=8,
    eEquality=16,
    eAlias=32,
	*/
};

class Statement;
class StatementAssign;
class StatementReturn;
class Variable;
class Expression;
class Lhs;
class Function;
class ExpressionVariable;

///////////////////////////////////////////////////////////////////////////////

class Fact
{
public:
	Fact(eFactCategory e);

	virtual ~Fact(void);

	virtual Fact* clone(void) const = 0;

	virtual int join(const Fact& /*fact*/) {return 0; };

	virtual int join_visits(const Fact& fact) { return join(fact);}

	virtual bool imply(const Fact& /*fact*/) const = 0;

	// lattice functions
	virtual bool is_top(void) const = 0;
	virtual bool is_bottom(void) const = 0;
	virtual void set_top(void) = 0;
	virtual void set_bottom(void) = 0;

	virtual bool is_assertable(const Statement* s) const = 0;

	virtual bool is_related(const Fact& fact) const { return eCat == fact.eCat && get_var() == fact.get_var();}

	virtual bool equal(const Fact& fact) const { return this == &fact; };

	virtual void Output(std::ostream &out) const = 0;

	virtual void OutputAssertion(std::ostream &out, const Statement* s = NULL) const;

	virtual const Variable* get_var(void) const { return 0;};

	virtual std::vector<const Fact*> abstract_fact_for_assign(const std::vector<const Fact*>& /*facts*/, const Lhs* /*lhs*/, const Expression* /*rhs*/) = 0;

	virtual vector<const Fact*> abstract_fact_for_return(const std::vector<const Fact*>& facts, const ExpressionVariable* expr, const Function* func);

	vector<const Fact*> abstract_fact_for_var_init(const Variable* v);

	static void doFinalization();

	enum eFactCategory eCat;

protected:
	// keep track all created facts. used for releasing memory in doFinalization
	static std::vector<Fact*> facts_;
};

///////////////////////////////////////////////////////////////////////////////
typedef std::vector<const Fact*> FactVec;
typedef FactVec* FactVecP;

/******************* Fact Manipulating Functions **********************/
/* find a fact from facts env */
int find_fact(const FactVec& facts, const Fact* fact);

/* find a specific type of fact (same variable most likely) from facts env */
const Fact* find_related_fact(const FactVec& facts, const Fact* new_fact);
const Fact* find_related_fact(const vector<Fact*>& facts, const Fact* new_fact);

/* merge a fact into env */
bool merge_fact(FactVec& facts, const Fact* new_fact);

/* merge two facts env */
bool merge_facts(FactVec& facts, const FactVec& new_facts);

/* check if two facts env are identical */
bool same_facts(const FactVec& facts1, const FactVec& facts2);

/* check if one facts env is a subset of the other */
bool subset_facts(const FactVec& facts1, const FactVec& facts2);

/* renew a fact in env (append is absent) */
bool renew_fact(FactVec& facts, const Fact* new_fact);

/* renew facts in new_facts into existing facts env */
bool renew_facts(FactVec& facts, const FactVec& new_facts);

/* clone an env */
vector<Fact*> copy_facts(const FactVec& facts_in);

/* combine facts in two env, discard facts not exist in one of them */
void combine_facts(vector<Fact*>& facts1, const FactVec& facts2);

/* add a new variable fact to env */
void add_new_var_fact(const Variable* v, FactVec& facts);

/* print facts env */
void print_facts(const FactVec& facts);

/* print fact(s) in env regarding a given variable */
void print_var_fact(const FactVec& facts, const char* vname);

#endif // FACT_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

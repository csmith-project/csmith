// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2015, 2017 The University of Utah
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
#include "Fact.h"
#include "Variable.h"
#include "Lhs.h"
#include "Function.h"
#include "ArrayVariable.h"
#include "StatementAssign.h"
#include "StatementReturn.h"

using namespace std;
std::vector<Fact*> Fact::facts_;

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
Fact::Fact(eFactCategory e) :
    eCat(e)
{
	// Nothing else to do.
}

/*
 *
 */
Fact::~Fact(void)
{
	// Nothing else to do.
}

/*
 * output an assertion about the fact to check the correctness of compiler
 * or generation time analysis
 */
void
Fact::OutputAssertion(std::ostream &out, const Statement* s) const
{
	if (!is_top()) {
		if (s && !is_assertable(s)) {
			out << "//";
		}
        out << "assert (";
        Output(out);
        out << ");" << endl;
    }
}

std::vector<const Fact*>
Fact::abstract_fact_for_return(const std::vector<const Fact*>& facts, const ExpressionVariable* expr, const Function* func)
{
	Lhs lhs(*func->rv);
	return abstract_fact_for_assign(facts, &lhs, expr);
}

vector<const Fact*>
Fact::abstract_fact_for_var_init(const Variable* v)
{
	vector<const Fact*> empty;
	// only consider points-to facts and union-write-field facts for now
	if (v->type == NULL || (v->type->eType != ePointer && v->type->eType != eUnion)) return empty;

	Lhs lhs(*v);
	vector<const Fact*> facts = abstract_fact_for_assign(empty, &lhs, v->init);
	if (v->isArray) {
		const ArrayVariable* av = dynamic_cast<const ArrayVariable*>(v);
		assert(av);
		for (size_t i=0; i<av->get_more_init_values().size(); i++) {
			const Expression* init = av->get_more_init_values()[i];
			vector<const Fact*> more_facts = abstract_fact_for_assign(empty, &lhs, init);
			merge_facts(facts, more_facts);
		}
	}
	return facts;
}

void
Fact::doFinalization()
{
	std::vector<Fact*>::iterator i;
	for( i = facts_.begin(); i != facts_.end(); ++i) {
		delete (*i);
	}
	facts_.clear();
}

// fact manipulating functions
int
find_fact(const FactVec& facts, const Fact* fact)
{
    size_t i;
    for (i=0; i<facts.size(); i++) {
        if (fact->equal(*facts[i])) {
            return i;
        }
    }
    return -1;
}

const Fact*
find_related_fact(const FactVec& facts, const Fact* new_fact)
{
    size_t i;
    for (i=0; i<facts.size(); i++) {
        if (new_fact->is_related(*facts[i])) {
            return facts[i];
        }
    }
    return 0;
}

const Fact*
find_related_fact(const vector<Fact*>& facts, const Fact* new_fact)
{
    size_t i;
    for (i=0; i<facts.size(); i++) {
        if (new_fact->is_related(*facts[i])) {
            return facts[i];
        }
    }
    return 0;
}

// TODO: we really need to free the memory properly while maintain the memory in compact
// way, i.e., don't allocate a Fact object unless it's absolutely necessary
bool
merge_fact(FactVec& facts, const Fact* new_fact)
{
    bool changed = false;
    size_t i;
    for (i=0; i<facts.size(); i++) {
        const Fact* f = facts[i];
        if (f->is_related(*new_fact)) {
            if (!f->imply(*new_fact)) {
				Fact* copy_fact = new_fact->clone();
                copy_fact->join(*f);
                facts[i] = copy_fact;
                changed = true;
				// TODO: release the memory for f???
            }
            else {
                //delete new_fact;   // new fact is useless, unsafe to do so???
            }
            break;
        }
    }
    if (i == facts.size()) {// if not found
        facts.push_back(new_fact);
        changed = true;
    }
    return changed;
}

bool
renew_fact(FactVec& facts, const Fact* new_fact)
{
    size_t i;
    for (i=0; i<facts.size(); i++) {
        if (new_fact->is_related(*facts[i])) {
            if (new_fact->equal(*facts[i])) {
                return false;
            }
            facts[i] = new_fact;
            break;
        }
    }
    // if not found, append the new fact
    if (i==facts.size()) {
        facts.push_back(new_fact);
    }
    return true;
}

bool
merge_facts(FactVec& facts, const FactVec& new_facts)
{
    size_t i;
    bool changed = false;
    for (i=0; i<new_facts.size(); i++) {
		//new_facts[i]->Output(cout);
        if (merge_fact(facts, new_facts[i])) {
            changed = true;
        }
    }
    return changed;
}

bool
renew_facts(FactVec& facts, const FactVec& new_facts)
{
    size_t i;
    bool changed = false;
    for (i=0; i<new_facts.size(); i++) {
        if (renew_fact(facts, new_facts[i])) {
            changed = true;
        }
    }
    return changed;
}

vector<Fact*>
copy_facts(const FactVec& facts_in)
{
    size_t i;
	vector<Fact*> facts_out;
    for (i=0; i<facts_in.size(); i++) {
		Fact* f = facts_in[i]->clone();
        facts_out.push_back(f);
    }
	return facts_out;
}

/*************************************************************
 * combine facts obtained from two visits to the same piece of code
 *************************************************************/
void
combine_facts(vector<Fact*>& facts1, const FactVec& facts2)
{
    size_t i, j;
    for (i=0; i<facts2.size(); i++) {
		const Fact* new_fact = facts2[i];
		for (j=0; j<facts1.size(); j++) {
			Fact* old_fact = facts1[j];
			if (old_fact->is_related(*new_fact)) {
				old_fact->join_visits(*new_fact);
				break;
			}
		}
	}
}

bool
same_facts(const FactVec& facts1, const FactVec& facts2)
{
	if (facts1.size() == facts2.size()) {
		size_t i;
		for (i=0; i<facts1.size(); i++) {
			if (find_fact(facts2, facts1[i]) == -1) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool
subset_facts(const FactVec& facts1, const FactVec& facts2)
{
	if (facts1.size() == facts2.size()) {
		size_t i;
		for (i=0; i<facts1.size(); i++) {
			const Fact* f1 = facts1[i];
			const Fact* f2 = find_related_fact(facts2, f1);
			if (f2 == 0 || !f2->imply(*f1)) {
				return false;
			}
		}
		return true;
	}
	return false;
}

void
print_facts(const FactVec& facts)
{
	for (size_t i=0; i<facts.size(); i++) {
		const Fact* f = facts[i];
		f->OutputAssertion(cout);
	}
}

void
print_var_fact(const FactVec& facts, const char* vname)
{
	for (size_t i=0; i<facts.size(); i++) {
		const Fact* f = facts[i];
		if (f->get_var()->name == vname) {
			f->OutputAssertion(cout);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

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

#ifndef LHS_H
#define LHS_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>

#include "Expression.h"

class CGContext;
class Variable;
class Constant;

/*
 *
 */
class Lhs : public Expression
{
public:
	// Factory method.
	static Lhs *make_random(CGContext &cg_context, const Type* t, const CVQualifiers* qfer, bool for_compound_assign, bool no_signed_overflow=false);

	explicit Lhs(const Variable &v);

	Lhs(const Variable &v, const Type* t, bool compound_assign);

	virtual ~Lhs(void);

	virtual Expression *clone() const;

	virtual CVQualifiers get_qualifiers(void) const;

	virtual void get_eval_to_subexps(vector<const Expression*>& subs) const {subs.push_back(this);}

	int get_indirect_level(void) const;

	void get_lvars(const vector<const Fact*>& facts, vector<const Variable*>& vars) const;

	bool is_volatile() const;

	const Variable* get_var(void) const {return &var;};

	bool compatible(const Variable *v) const;

	bool compatible(const Expression *c) const;

	bool visit_indices(vector<const Fact*>& inputs, CGContext& cg_context) const;
	//
	virtual std::vector<const ExpressionVariable*> get_dereferenced_ptrs(void) const;
	virtual void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const;
	virtual unsigned int get_complexity(void) const { return 1;}

	virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;

	virtual const Type &get_type(void) const;

	virtual void Output(std::ostream &) const;

private:
	explicit Lhs(const Lhs &lhs);

	bool ptr_modified_in_rhs(vector<const Fact*>& inputs, CGContext& cg_context) const;

	const Variable &var;

	const Type* type;

	const bool for_compound_assign;

	// unimplementable
	Lhs &operator=(const Lhs &ev);
};

///////////////////////////////////////////////////////////////////////////////

#endif // ELHS_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

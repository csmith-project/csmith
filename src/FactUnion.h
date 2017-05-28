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

#ifndef FACTUNION_H
#define FACTUNION_H

///////////////////////////////////////////////////////////////////////////////
#include <ostream>
#include <vector>
#include "Fact.h"
#include "Variable.h"

class Function;
class Statement;
class Block;
class StatementAssign;
class StatementReturn;

using namespace std;

///////////////////////////////////////////////////////////////////////////////

class FactUnion : public Fact
{
public:
	static FactUnion *make_fact(const Variable* v, int fid = 0);
	static vector<const Fact*> make_facts(const vector<const Variable*>& vars, int fid);
	static void doFinalization();
	static bool is_nonreadable_field(const Variable *v, const std::vector<const Fact*>& facts);

	virtual ~FactUnion(void) {};

	virtual const Variable* get_var(void) const { return var;};
	void set_var(const Variable* v) { var = v;}
	const Type* get_last_written_type(void) const;
	int   get_last_written_fid(void) const { return last_written_fid; };
	static bool is_field_readable(const Variable* v, int fid, const vector<const Fact*>& facts);

	// lattice functions
	virtual bool is_top(void) const { return last_written_fid == TOP;}
	virtual bool is_bottom(void) const { return last_written_fid == BOTTOM;}
	virtual void set_top(void) { last_written_fid = TOP;}
	virtual void set_bottom(void) { last_written_fid = BOTTOM;}
	virtual bool imply(const Fact& fact) const;
	virtual bool equal(const Fact& fact) const;
	virtual int join(const Fact& fact);

	// transfer functions
	vector<const Fact*> rhs_to_lhs_transfer(const vector<const Fact*>& facts, const vector<const Variable*>& lvars, const Expression* rhs);
	virtual vector<const Fact*> abstract_fact_for_assign(const std::vector<const Fact*>& facts, const Lhs* lhs, const Expression* /*rhs*/);
	//virtual vector<const Fact*> abstract_fact_for_return(const std::vector<const Fact*>& facts, const ExpressionVariable* rv, const Function* func);
	virtual Fact* join_var_facts(const vector<const Fact*>& facts, const vector<const Variable*>& vars) const;
	virtual Fact* clone(void) const;

	// output functions
	virtual bool is_assertable(const Statement* /*s*/) const { return false;}
	virtual void Output(std::ostream &out) const;
	virtual void OutputAssertion(std::ostream &/*out*/, const Statement* /*s*/) const {};

	// constants to indicate lattice top/bottom
	static const int TOP;
	static const int BOTTOM;
private:
	FactUnion(const Variable* v);
	FactUnion(const Variable* v, int fid);

	const Variable* var;

	// last written field id
	int  last_written_fid;
};

///////////////////////////////////////////////////////////////////////////////

#endif // FactUnion_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

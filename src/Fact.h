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

#ifndef FACT_H
#define FACT_H

///////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <vector>

enum eFactCategory {
    eNullPtr=1, 
    eDeadPtr=2,
    ePointTo=4,
    //ePointToSize=4, 
    eIntRange=8, 
    eEquality=16, 
    eAlias=32,
};

class StatementAssign;
class StatementReturn;
class Block;
class CGContext;
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

	virtual bool conflict_with(const Fact& /*fact*/) const { return false;}; 

	virtual bool is_related(const Fact& fact) const = 0;

	virtual bool is_relevant(const Fact& ) const {return false;}

	virtual bool equal(const Fact& fact) const { return this == &fact; };

	virtual void Output(std::ostream &out) const = 0;

	virtual void OutputAssertion(std::ostream &out) const = 0; 

	virtual const Variable* get_var(void) const { return 0;};

	virtual std::vector<const Fact*> abstract_fact_for_assign(const std::vector<const Fact*>& /*facts*/, const Lhs* /*lhs*/, const Expression* /*rhs*/) = 0;

	virtual Fact* abstract_fact_for_return(const std::vector<const Fact*>& /*facts*/, const ExpressionVariable* /*var*/, const Function* /*func*/) {return 0;};

	enum eFactCategory eCat;

	bool no_more_update;
};

///////////////////////////////////////////////////////////////////////////////

#endif // FACT_H

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

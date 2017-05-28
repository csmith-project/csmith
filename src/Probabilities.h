// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2014, 2015 The University of Utah
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

#ifndef PROBABILITIES_H
#define PROBABILITIES_H

#include <string>
#include <map>
#include <vector>
#include <ostream>
#include "Filter.h"

template <class Key, class Value>
class ProbabilityTable;

using namespace std;
enum ProbName {
	pMoreStructUnionProb,
	pBitFieldsCreationProb,
	pBitFieldsSignedProb,
	pBitFieldInNormalStructProb,
	pScalarFieldInFullBitFieldsProb,
	pExhaustiveBitFieldsProb,
	pSafeOpsSignedProb,
	pSelectDerefPointerProb,
	pRegularVolatileProb,
	pRegularConstProb,
	pStricterConstProb,
	pLooserConstProb,
	pFieldVolatileProb,
	pFieldConstProb,
	pStdUnaryFuncProb,
	pShiftByNonConstantProb,
	pPointerAsLTypeProb,
	pStructAsLTypeProb,
	pUnionAsLTypeProb,
	pFloatAsLTypeProb,
	pNewArrayVariableProb,
	pAccessOnceVariableProb,
	pInlineFunctionProb,
	pBuiltinFunctionProb,

	// group for statement
	pStatementProb,
	pAssignProb,
	pBlockProb,
	pForProb,
	pIfElseProb,
	pInvokeProb,
	pReturnProb,
	pContinueProb,
	pBreakProb,
	pGotoProb,
	pArrayOpProb,

	// group for assignment ops
	pAssignOpsProb,
	pSimpleAssignProb,
	pMulAssignProb,
	pDivAssignProb,
	pRemAssignProb,
	pAddAssignProb,
	pSubAssignProb,
	pLShiftAssignProb,
	pRShiftAssignProb,
	pBitAndAssignProb,
	pBitXorAssignProb,
	pBitOrAssignProb,
	pPreIncrProb,
	pPreDecrProb,
	pPostIncrProb,
	pPostDecrProb,

	// for unary ops
	pUnaryOpsProb,
	pPlusProb,
	pMinusProb,
	pNotProb,
	pBitNotProb,

	// for binary ops
	pBinaryOpsProb,
	pAddProb,
	pSubProb,
	pMulProb,
	pDivProb,
	pModProb,
	pCmpGtProb,
	pCmpLtProb,
	pCmpGeProb,
	pCmpLeProb,
	pCmpEqProb,
	pCmpNeProb,
	pAndProb,
	pOrProb,
	pBitXorProb,
	pBitAndProb,
	pBitOrProb,
	pRShiftProb,
	pLShiftProb,

	// group for simple types
	pSimpleTypesProb,
	pVoidProb,
	pCharProb,
	pIntProb,
	pShortProb,
	pLongProb,
	pLongLongProb,
	pUCharProb,
	pUIntProb,
	pUShortProb,
	pULongProb,
	pULongLongProb,
	pFloatProb,

	// for safe math ops
	pSafeOpsSizeProb,
	pInt8Prob,
	pInt16Prob,
	pInt32Prob,
	pInt64Prob,

};

#define MAX_PROB_NAME ((ProbName)(pStatementProb+1))

#define MoreStructUnionTypeProb \
	Probabilities::get_prob(pMoreStructUnionProb)

#define BitFieldsCreationProb \
	Probabilities::get_prob(pBitFieldsCreationProb)

#define BitFieldInNormalStructProb \
	Probabilities::get_prob(pBitFieldInNormalStructProb)

#define ScalarFieldInFullBitFieldsProb \
	Probabilities::get_prob(pScalarFieldInFullBitFieldsProb)

#define ExhaustiveBitFieldsProb \
	Probabilities::get_prob(pExhaustiveBitFieldsProb)

#define BitFieldsSignedProb \
	Probabilities::get_prob(pBitFieldsSignedProb)

#define SafeOpsSignedProb \
	Probabilities::get_prob(pSafeOpsSignedProb)

#define SelectDerefPointerProb \
	Probabilities::get_prob(pSelectDerefPointerProb)

#define RegularVolatileProb \
	Probabilities::get_prob(pRegularVolatileProb)

#define RegularConstProb \
	Probabilities::get_prob(pRegularConstProb)

#define StricterConstProb \
	Probabilities::get_prob(pStricterConstProb)

#define LooserConstProb \
	Probabilities::get_prob(pLooserConstProb)

#define FieldVolatileProb \
	Probabilities::get_prob(pFieldVolatileProb)

#define FieldConstProb \
	Probabilities::get_prob(pFieldConstProb)

#define StdUnaryFuncProb \
	Probabilities::get_prob(pStdUnaryFuncProb)

#define ShiftByNonConstantProb \
	Probabilities::get_prob(pShiftByNonConstantProb)

#define PointerAsLTypeProb \
	Probabilities::get_prob(pPointerAsLTypeProb)

#define StructAsLTypeProb \
	Probabilities::get_prob(pStructAsLTypeProb)

#define UnionAsLTypeProb \
	Probabilities::get_prob(pUnionAsLTypeProb)

#define FloatAsLTypeProb \
	Probabilities::get_prob(pFloatAsLTypeProb)

#define NewArrayVariableProb \
	Probabilities::get_prob(pNewArrayVariableProb)

#define AccessOnceVariableProb \
	Probabilities::get_prob(pAccessOnceVariableProb)

#define InlineFunctionProb \
	Probabilities::get_prob(pInlineFunctionProb)

#define BuiltinFunctionProb \
	Probabilities::get_prob(pBuiltinFunctionProb)

//////////////////////////////////////////////////
#define UNARY_OPS_PROB_FILTER \
	Probabilities::get_prob_filter(pUnaryOpsProb)

#define BINARY_OPS_PROB_FILTER \
	Probabilities::get_prob_filter(pBinaryOpsProb)

#define SIMPLE_TYPES_PROB_FILTER \
	Probabilities::get_prob_filter(pSimpleTypesProb)

#define SAFE_OPS_SIZE_PROB_FILTER \
	Probabilities::get_prob_filter(pSafeOpsSizeProb)

class ProbabilityFilter : public Filter
{
public:
	ProbabilityFilter(ProbName pname);

	virtual ~ProbabilityFilter(void);

	virtual bool filter(int v) const;
private:
	const ProbName pname_;
};

class ProbElem {
public:
	ProbElem() {};

	virtual ~ProbElem() = 0;

	virtual int get_prob(ProbName pname) = 0;

	virtual void set_prob(ProbName pname, int val) = 0;

	virtual void dump_default(std::ostream &out) = 0;

	virtual void dump_val(std::ostream &out) = 0;

	virtual void set_prob_table(ProbabilityTable<unsigned int, ProbName> * const table) = 0;

	virtual bool is_equal() = 0;
};

class SingleProbElem : public ProbElem {
public:
	SingleProbElem(const std::string &sname, ProbName pname, int default_val, int val);

	virtual ~SingleProbElem();

	int get_prob_direct();

	virtual int get_prob(ProbName pname);

	virtual void set_prob(ProbName pname, int val);

	void set_prob(int val) { val_ = val; };

	void get_all_values(std::vector<int> &values);

	virtual void dump_default(std::ostream &out);

	virtual void dump_val(std::ostream &out);

	virtual void set_prob_table(ProbabilityTable<unsigned int, ProbName> * const table);

	virtual bool is_equal() { return false; }

	static const char single_elem_sep_char;

private:
	const std::string sname_;

	// the name used by csmith internally
	const ProbName pname_;

	// default probability
	const int default_val_;

	// actual probability
	int val_;
};

class Probabilities;

class GroupProbElem : public ProbElem {
	friend class ProbabilityFilter;
public:
	GroupProbElem(bool is_equal, const std::string &sname);

	virtual ~GroupProbElem();

	virtual int get_prob(ProbName pname);

	virtual void set_prob(ProbName pname, int val);

	virtual void set_prob_table(ProbabilityTable<unsigned int, ProbName> * const table);

	virtual void dump_default(std::ostream &out);

	virtual void dump_val(std::ostream &out);

	virtual bool is_equal() { return is_equal_; }

	void initialize(Probabilities *impl, const std::map<ProbName, int> pairs);

	static const char group_open_delim;

	static const char group_close_delim;

	static const char equal_open_delim;

	static const char equal_close_delim;

	static const char group_sep_char;

private:
	int get_random_single_prob(int orig_val, std::vector<unsigned int> &invalid_vals);

	bool elem_exist(ProbName pname);

	void get_all_values(std::vector<SingleProbElem*> &values);

	// represents equal probabilities
	const bool is_equal_;

	const std::string sname_;

	// the name used by csmith internally
	// const ProbName pname_;

	std::map<ProbName, SingleProbElem*> probs_;
};

class Probabilities {
	friend class ProbabilityFilter;
public:
	static Probabilities *GetInstance();

	static void DestroyInstance();

	static unsigned int pname_to_type(ProbName pname);

	static int get_random_single_prob(int orig_val);

	void set_prob_table(ProbabilityTable<unsigned int, ProbName> * const table, ProbName pname);

	// Only get single prob value
	// Play with prob table if you need any group probs
	static unsigned int get_prob(ProbName pname);

	static Filter *get_prob_filter(ProbName pname);

	static void register_extra_filter(ProbName pname, Filter *filter);

	static void unregister_extra_filter(ProbName pname, Filter *filter);

	ProbName get_pname(const std::string &sname);

	std::string get_sname(ProbName pname);

	bool parse_configuration(std::string &error_msg, const std::string &fname);

	void dump_default_probabilities(const std::string &fname);

	void dump_actual_probabilities(const std::string &fname, unsigned long seed);

	Filter *get_binary_ops_prob_filter();
private:

	void set_group_prob(bool is_equal, ProbName pname, const std::map<ProbName, int> &m);

	bool setup_group_probabilities(bool is_equal, const std::vector<string> &elems);

	bool parse_group_probabilities(bool is_equal, std::string &error_msg, const std::string &line);

	int parse_single_elem(bool is_equal, ProbElem *elem, const std::string &line);

	bool parse_single_probability(std::string &err_msg, const std::string &line);

	bool parse_line(std::string &error_msg, std::string &line);

	void set_single_name_maps();

	void set_single_name(const char *sname, ProbName pname);

	void set_single_name(const char *sname, ProbName pname, unsigned int type);

	void set_prob_filter(ProbName pname);

	void set_extra_filters(ProbName pname);

	bool check_extra_filter(ProbName pname, int v);

	void initialize_single_probs();

	void clear_filter(std::map<ProbName, Filter*> &filter);

	void initialize_group_probs();

	void set_default_statement_prob();

	void set_default_simple_types_prob();

	void set_default_binary_ops_prob();

	void set_default_unary_ops_prob();

	void set_default_safe_ops_size_prob();

	void initialize();

	static Probabilities *instance_;

	static const char comment_line_prefix;

	std::map<std::string, ProbName> sname_to_pname_;

	std::map<ProbName, std::string> pname_to_sname_;

	std::map<ProbName, unsigned int> pname_to_type_;

	std::map<ProbName, ProbElem *> probabilities_;

	std::map<ProbName, Filter*> prob_filters_;

	std::map<ProbName, Filter*> extra_filters_;
	//const static ProbabilityFilter *binary_ops_prob_filter_;

	Probabilities();

	~Probabilities();
};

#endif //PROBABILITIES_H

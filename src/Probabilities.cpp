// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 The University of Utah
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

#include "Probabilities.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <algorithm>
#include "StringUtils.h"
#include "ProbabilityTable.h"
#include "Statement.h"
#include "StatementAssign.h"
#include "FunctionInvocation.h"
#include "Type.h"
#include "SafeOpFlags.h"
#include "CGOptions.h"
#include "MspFilters.h"
#include "VectorFilter.h"
#include "random.h"

////////////////////////////////////////////////////////////////////////////////////
#define VAL_ASSERT(val) assert(((val) <= 100) && ((val) >= -1))

using namespace std;

ProbabilityFilter::ProbabilityFilter(ProbName pname)
	: pname_(pname)
{

}

ProbabilityFilter::~ProbabilityFilter()
{

}

// only used it for equivalent group
bool
ProbabilityFilter::filter(int v) const
{
	assert(v >= 0);
	Probabilities *prob = Probabilities::GetInstance();
	assert(prob);
	if (prob->check_extra_filter(pname_, v))
		return true;

	GroupProbElem *elem = dynamic_cast<GroupProbElem*>(prob->probabilities_[pname_]);
	assert(elem);
	assert(elem->is_equal());

	map<ProbName, SingleProbElem *>::iterator i;
	bool rv = false;
	for (i = elem->probs_.begin(); i != elem->probs_.end(); ++i) {
		ProbName pname = (*i).first;
		unsigned val = Probabilities::pname_to_type(pname);
		if ((static_cast<unsigned int>(v)) == val) {
			int prob = (*i).second->get_prob_direct();
			rv = (prob == 0);
			break;
		}
	}
	return rv;
}

/////////////////////////////////////////////////////////////////
ProbElem::~ProbElem()
{

}

/////////////////////////////////////////////////////////////////
//
const char SingleProbElem::single_elem_sep_char = '=';

SingleProbElem::SingleProbElem(const std::string &sname, ProbName pname, int default_val, int val)
	: sname_(sname),
	  pname_(pname),
	  default_val_(default_val),
	  val_(val)
{

}

SingleProbElem::~SingleProbElem()
{

}

int
SingleProbElem::get_prob_direct()
{
	return val_;
}

int
SingleProbElem::get_prob(ProbName pname)
{
	// assert(val_ > 0);
	assert(pname == pname_);
	return val_;
}

void
SingleProbElem::set_prob(ProbName pname, int val)
{
	assert(pname == pname_);
	VAL_ASSERT(val);
	if (pname == pVoidProb) {
		assert(!val && "Invalid probability - the probability value of void_prob must be 0!");
	}
	if (val >= 0)
		val_ = val;
}

void
SingleProbElem::set_prob_table(ProbabilityTable<unsigned int, ProbName> * const)
{
	assert(0);
}

void
SingleProbElem::dump_default(std::ostream &out)
{
	out << sname_ << single_elem_sep_char << default_val_;
}

void
SingleProbElem::dump_val(std::ostream &out)
{
	out << sname_ << single_elem_sep_char << val_;
}

/////////////////////////////////////////////////////////////////

const char GroupProbElem::group_open_delim = '[';
const char GroupProbElem::group_close_delim = ']';
const char GroupProbElem::group_sep_char = ',';

const char GroupProbElem::equal_open_delim = '(';
const char GroupProbElem::equal_close_delim = ')';

bool
single_elem_less(SingleProbElem *elem1, SingleProbElem *elem2)
{
	assert(elem1);
	assert(elem2);
	int val1 = elem1->get_prob_direct();
	int val2 = elem2->get_prob_direct();
	return (val1 < val2);
}

GroupProbElem::GroupProbElem(bool is_equal, const std::string &sname)
	: is_equal_(is_equal),
	  sname_(sname)
	  // pname_(pname)
{

}

GroupProbElem::~GroupProbElem()
{
	std::map<ProbName, SingleProbElem*>::iterator i;
	for (i = probs_.begin(); i != probs_.end(); ++i) {
		SingleProbElem *elem = (*i).second;
		assert(elem);
		delete elem;
	}
	probs_.clear();
}

int
GroupProbElem::get_random_single_prob(int orig_val, std::vector<unsigned int> &invalid_vals)
{
	// we won't change val if it's disallowed before
	if (orig_val == 0) {
		return orig_val;
	}
	if (is_equal_) {
		bool p = rnd_flipcoin(50);
		return (p ? 1 : 0);
	}
	else {
		VectorFilter f(invalid_vals);
		return rnd_upto(101, &f);
	}
}

void
GroupProbElem::initialize(Probabilities *impl, const std::map<ProbName, int> pairs)
{
	assert(impl);
	std::map<ProbName, int>::const_iterator i;
	std::vector<unsigned int> invalid_vals;
	invalid_vals.push_back(100);
	invalid_vals.push_back(0);
	std::vector<SingleProbElem*> valid_probs;

	for (i = pairs.begin(); i != pairs.end(); ++i) {
		bool valid = false;
		int default_val = (*i).second;
		int val = default_val;
		// make sure val is within the correct range
		assert(val >= 0 && val <= 100);
		if (CGOptions::random_random()) {
			val = get_random_single_prob(val, invalid_vals);
			assert(val != 100);
			if(val != 0) {
				invalid_vals.push_back(val);
				valid = true;
			}
		}
		ProbName pname = (*i).first;
		std::string sname = impl->get_sname(pname);
		SingleProbElem *elem = new SingleProbElem(sname, pname, default_val, val);
		probs_[pname] = elem;
		if (valid)
			valid_probs.push_back(elem);
	}

	int size = valid_probs.size();
	if (CGOptions::random_random()) {
		// we can't allow all 0 vals for equal prob group
		// if we got one, just use the default settings.
		if (is_equal_ && size == 0) {
			for (i = pairs.begin(); i != pairs.end(); ++i) {
				int val = (*i).second;
				assert(val >= 0 && val <= 100);
				ProbName pname = (*i).first;
				std::string sname = impl->get_sname(pname);
				assert(probs_[pname]);
				probs_[pname]->set_prob(val);
			}
		}
		else if (!is_equal_) {
			assert(size > 0);
			int rnd = rnd_upto(size);
			valid_probs[rnd]->set_prob(100);
		}
	}
}

bool
GroupProbElem::elem_exist(ProbName pname)
{
	std::map<ProbName, SingleProbElem*>::iterator i = probs_.find(pname);
	return (!(i == probs_.end()));
}

void
GroupProbElem::set_prob(ProbName pname, int val)
{
	assert(elem_exist(pname));
	VAL_ASSERT(val);
	SingleProbElem *elem = probs_[pname];
	assert(elem);
	elem->set_prob(pname, val);
}

int
GroupProbElem::get_prob(ProbName pname)
{
	assert(elem_exist(pname));
	SingleProbElem *elem = probs_[pname];
	assert(elem);
	return elem->get_prob(pname);
}

void
GroupProbElem::set_prob_table(ProbabilityTable<unsigned int, ProbName> * const table)
{
	map<ProbName, SingleProbElem*>::iterator i;
	for (i = probs_.begin(); i != probs_.end(); ++i) {
		SingleProbElem *elem = (*i).second;
		assert(elem);
		ProbName pname = (*i).first;
		int val = elem->get_prob(pname);
		// we only care about val > 0 for a probability table
		// other value means that we don't put it into the table
		if (val > 0)
			table->add_elem(static_cast<unsigned int>(val), pname);
	}
}

void
GroupProbElem::get_all_values(vector<SingleProbElem*> &values)
{
	map<ProbName, SingleProbElem*>::iterator i;
	for (i = probs_.begin(); i != probs_.end(); ++i)
		values.push_back((*i).second);
}

void
GroupProbElem::dump_default(std::ostream &out)
{
	char open_delim = is_equal_ ? equal_open_delim : group_open_delim;
	char close_delim = is_equal_ ? equal_close_delim : group_close_delim;

	out << open_delim << sname_ << group_sep_char;

	vector<SingleProbElem*> values;
	get_all_values(values);
	assert(values.size() >= 1);
#if 0
	sort(values.begin(), values.end(), single_elem_less);
#endif
	size_t count = 0;
	for (count = 0; count < values.size() - 1; count++) {
		values[count]->dump_default(out);
		out << group_sep_char;
	}
	assert(count < values.size());
	values[count]->dump_default(out);
	out << close_delim;
}

void
GroupProbElem::dump_val(std::ostream &out)
{
	char open_delim = is_equal_ ? equal_open_delim : group_open_delim;
	char close_delim = is_equal_ ? equal_close_delim : group_close_delim;

	out << open_delim << sname_ << group_sep_char;
	map<ProbName, SingleProbElem*>::iterator i = probs_.begin();
	for (size_t count = 0; count < probs_.size() - 1; count++) {
		((*i).second)->dump_val(out);
		out << group_sep_char;
		++i;
	}
	assert(i != probs_.end());
	((*i).second)->dump_val(out);
	out << close_delim;
}

/////////////////////////////////////////////////////////////////

Probabilities* Probabilities::instance_ = NULL;

Probabilities *
Probabilities::GetInstance()
{
	if (Probabilities::instance_)
		return Probabilities::instance_;

	Probabilities::instance_ = new Probabilities();
	assert(Probabilities::instance_);
	Probabilities::instance_->initialize();
	return Probabilities::instance_;
}

void
Probabilities::DestroyInstance()
{
	if (Probabilities::instance_) {
		delete Probabilities::instance_;
		Probabilities::instance_ = NULL;
	}
}

void
Probabilities::set_single_name(const char *sname, ProbName pname)
{
	string s = sname;

	sname_to_pname_[s] = pname;
	pname_to_sname_[pname] = s;
}

void
Probabilities::set_single_name(const char *sname, ProbName pname, unsigned int type)
{
	pname_to_type_[pname] = type;
	set_single_name(sname, pname);
}

#define SET_SINGLE_NAME(s, type, value) \
	set_single_name(s, p##type##Prob, e##type); \
	m[p##type##Prob] = value;

#define SET_SINGLE_NAME1(str, type, value) \
	set_single_name(str, p##type##Prob, s##type); \
	m[p##type##Prob] = value;

void
Probabilities::set_single_name_maps()
{
	// for single probs
	// for generating more struct or union types
	set_single_name("more_struct_union_type_prob", pMoreStructUnionProb);

	set_single_name("bitfields_creation_prob", pBitFieldsCreationProb);

	// for single bitfield in a normal struct
	set_single_name("bitfield_in_normal_struct_prob", pBitFieldInNormalStructProb);

	// for single field in a full-bitfields struct
	set_single_name("scalar_field_in_full_bitfields_struct_prob", pScalarFieldInFullBitFieldsProb);

	// for single field in exhaustive mode
	set_single_name("exhaustive_bitfield_prob", pExhaustiveBitFieldsProb);

	// for signed flag of struct/union fields.
	set_single_name("bitfields_signed_prob", pBitFieldsSignedProb);

	// for signed flag of safe ops
	set_single_name("safe_ops_signed_prob", pSafeOpsSignedProb);

	// for selecting deref pointer
	set_single_name("select_deref_pointer_prob", pSelectDerefPointerProb);

	// for regular volatile
	set_single_name("regular_volatile_prob", pRegularVolatileProb);

	// for regular const
	set_single_name("regular_const_prob", pRegularConstProb);

	// for stricter const
	set_single_name("stricter_const_prob", pStricterConstProb);

	// for looser const
	set_single_name("looser_const_prob", pLooserConstProb);

	// for field
	set_single_name("field_volatile_prob", pFieldVolatileProb);

	// for field
	set_single_name("field_const_prob", pFieldConstProb);

	// for std func
	set_single_name("std_unary_func_prob", pStdUnaryFuncProb);

	// for shift by non constant
	set_single_name("shift_by_non_constant_prob", pShiftByNonConstantProb);

	// for choosing pointer as LType
	set_single_name("pointer_as_ltype_prob", pPointerAsLTypeProb);

	// for choosing struct as LType
	set_single_name("struct_as_ltype_prob", pStructAsLTypeProb);

	// for choosing union as LType
	set_single_name("union_as_ltype_prob", pUnionAsLTypeProb);

	// for choosing float as LType
	set_single_name("float_as_ltype_prob", pFloatAsLTypeProb);

	// for creating new array var
	set_single_name("new_array_var_prob", pNewArrayVariableProb);

	// for wrapping all accesses to a var by ACCESS_ONCE macro
	set_single_name("access_once_var_prob", pAccessOnceVariableProb);

	// for marking each function as inline
	set_single_name("inline_function_prob", pInlineFunctionProb);

	// for choosing a builtin function
	set_single_name("builtin_function_prob", pBuiltinFunctionProb);

        //////////////////////////////////////////////////////////////////
	// group for statement
	set_single_name("statement_prob", pStatementProb);

	// group for assignment ops
	set_single_name("assign_ops_prob", pAssignOpsProb);

	// group for unary ops which equal probability
	set_single_name("assign_unary_ops_prob", pUnaryOpsProb);

	// group for binary ops which equal probability
	set_single_name("assign_binary_ops_prob", pBinaryOpsProb);

	// group for simple types which equal probability
	set_single_name("simple_types_prob", pSimpleTypesProb);

	// group for simple types which equal probability
	set_single_name("safe_ops_size_prob", pSafeOpsSizeProb);
}

void
Probabilities::set_prob_table(ProbabilityTable<unsigned int, ProbName> *const table, ProbName pname)
{
	ProbElem *elem = probabilities_[pname];
	elem->set_prob_table(table);
}

unsigned int
Probabilities::pname_to_type(ProbName pname)
{
	assert(Probabilities::instance_);

	return instance_->pname_to_type_[pname];
}

int
Probabilities::get_random_single_prob(int orig_val)
{
	// orig_val == 0 means that we disallow it explicitly before,
	// so don't change it.
	if (orig_val == 0)
		return orig_val;
	else
		return rnd_upto(101);
}

void
Probabilities::initialize_single_probs()
{
	std::map<ProbName, int> m;
	m[pMoreStructUnionProb] = 50;
	m[pBitFieldsCreationProb] = 50;
	m[pBitFieldInNormalStructProb] = 10;
	m[pScalarFieldInFullBitFieldsProb] = 10;
	m[pExhaustiveBitFieldsProb] = 10;
	m[pBitFieldsSignedProb] = 50;
	m[pSafeOpsSignedProb] = 50;

	if (CGOptions::volatiles())
		m[pRegularVolatileProb] = 50;
	else
		m[pRegularVolatileProb] = 0;

	if (CGOptions::consts())
		m[pRegularConstProb] = 10;
	else
		m[pRegularConstProb] = 0;

	if (CGOptions::consts())
		m[pStricterConstProb] = 50;
	else
		m[pStricterConstProb] = 0;

	if (CGOptions::consts())
		m[pLooserConstProb] = 50;
	else
		m[pLooserConstProb] = 0;

	if (CGOptions::volatiles() &&
	    CGOptions::vol_struct_union_fields() &&
	    CGOptions::global_variables())
		m[pFieldVolatileProb] = 30;
	else
		m[pFieldVolatileProb] = 0;

	if (CGOptions::consts() && CGOptions::const_struct_union_fields())
		m[pFieldConstProb] = 20;
	else
		m[pFieldConstProb] = 0;

	m[pStdUnaryFuncProb] = 5;
	m[pShiftByNonConstantProb] = 50;
	m[pStructAsLTypeProb] = 30;
	m[pUnionAsLTypeProb] = 25;
	if (CGOptions::enable_float())
		m[pFloatAsLTypeProb] = 40;
	else
		m[pFloatAsLTypeProb] = 0;
	if (CGOptions::arrays())
		m[pNewArrayVariableProb] = 20;
	else
		m[pNewArrayVariableProb] = 0;
	if (CGOptions::pointers()) {
		m[pPointerAsLTypeProb] = 50;
		m[pSelectDerefPointerProb] = 80;
	}
	else {
		m[pPointerAsLTypeProb] = 0;
		m[pSelectDerefPointerProb] = 0;
	}

	m[pAccessOnceVariableProb] = 20;
	m[pInlineFunctionProb] = CGOptions::inline_function_prob();
	m[pBuiltinFunctionProb] = CGOptions::builtin_function_prob();

	std::map<ProbName, int>::iterator i;
	for (i = m.begin(); i != m.end(); ++i) {
		int default_val = (*i).second;
		int val = default_val;
		assert(val >= 0 && val <= 100);
		if (CGOptions::random_random())
			val = Probabilities::get_random_single_prob(val);
		// For single prob, we don't allow -1 from initialization
		ProbName pname = (*i).first;
		std::string sname = get_sname(pname);
		SingleProbElem *elem = new SingleProbElem(sname, pname, default_val, val);
		probabilities_[pname] = elem;
	}
}

void
Probabilities::initialize_group_probs()
{
	set_default_statement_prob();
	set_default_unary_ops_prob();
	set_default_binary_ops_prob();
	set_default_simple_types_prob();
	set_default_safe_ops_size_prob();

	// setup random distribution of assignment operators (=, +=, /=...)
	StatementAssign::InitProbabilityTable();

	// setup random distribution of expression term types (const, variable, function ...)
	Expression::InitProbabilityTables();
}

void
Probabilities::set_group_prob(bool is_equal, ProbName pname, const std::map<ProbName, int> &m)
{
	string sname = get_sname(pname);
	GroupProbElem *g_elem = new GroupProbElem(is_equal, sname);
	g_elem->initialize(this, m);
	probabilities_[pname] = g_elem;
}

void
Probabilities::set_default_safe_ops_size_prob()
{
	std::map<ProbName, int> m;

	// each op has equivalent probability

	if (CGOptions::int8() && CGOptions::uint8()) {
		SET_SINGLE_NAME1("safe_ops_size_int8", Int8, 1);
	}
	else {
		SET_SINGLE_NAME1("safe_ops_size_int8", Int8, 0);
	}

	SET_SINGLE_NAME1("safe_ops_size_int16", Int16, 1);
	SET_SINGLE_NAME1("safe_ops_size_int32", Int32, 1);
	if (CGOptions::allow_int64()) {
		SET_SINGLE_NAME1("safe_ops_size_int64", Int64, 1);
	}
	else {
		SET_SINGLE_NAME1("safe_ops_size_int64", Int64, 0);
	}

	set_group_prob(true, pSafeOpsSizeProb, m);
	set_prob_filter(pSafeOpsSizeProb);
}
void
Probabilities::set_default_simple_types_prob()
{
	std::map<ProbName, int> m;

	// each op has equivalent probability

	// We only use void for function's parameter, so
	// disallow choosing void type from other places
	SET_SINGLE_NAME("void_prob", Void, 0);
	if (CGOptions::int8()) {
		SET_SINGLE_NAME("char_prob", Char, 1);
	}
	else {
		SET_SINGLE_NAME("char_prob", Char, 0);
	}

	SET_SINGLE_NAME("int_prob", Int, 1);
	SET_SINGLE_NAME("short_prob", Short, 1);

	if (CGOptions::ccomp()) {
		SET_SINGLE_NAME("long_prob", Long, 0);
		SET_SINGLE_NAME("ulong_prob", ULong, 0);
	}
	else {
		SET_SINGLE_NAME("long_prob", Long, 1);
		SET_SINGLE_NAME("ulong_prob", ULong, 1);
	}

	if (CGOptions::uint8()) {
		SET_SINGLE_NAME("uchar_prob", UChar, 1);
	}
	else {
		SET_SINGLE_NAME("uchar_prob", UChar, 0);
	}

	SET_SINGLE_NAME("uint_prob", UInt, 1);
	SET_SINGLE_NAME("ushort_prob", UShort, 1);

	if (CGOptions::allow_int64()) {
		SET_SINGLE_NAME("long_long_prob", LongLong, 1);
		SET_SINGLE_NAME("ulong_long_prob", ULongLong, 1);
	}
	else {
		SET_SINGLE_NAME("long_long_prob", LongLong, 0);
		SET_SINGLE_NAME("ulong_long_prob", ULongLong, 0);
	}

	if (CGOptions::enable_float()) {
		SET_SINGLE_NAME("float_prob", Float, 1);
	}
	else {
		SET_SINGLE_NAME("float_prob", Float, 0);
	}

	set_group_prob(true, pSimpleTypesProb, m);
	set_prob_filter(pSimpleTypesProb);
}

void Probabilities::set_default_unary_ops_prob()
{
	std::map<ProbName, int> m;

	// each op has equivalent probability
	if (CGOptions::unary_plus_operator()) {
		SET_SINGLE_NAME("unary_plus_prob", Plus, 1);
	}
	else {
		SET_SINGLE_NAME("unary_plus_prob", Plus, 0);
	}

	SET_SINGLE_NAME("unary_minus_prob", Minus, 1);
	SET_SINGLE_NAME("unary_not_prob", Not, 1);
	SET_SINGLE_NAME("unary_bit_not_prob", BitNot, 1);

	set_group_prob(true, pUnaryOpsProb, m);
	set_prob_filter(pUnaryOpsProb);
}

void
Probabilities::set_default_binary_ops_prob()
{
	std::map<ProbName, int> m;

	// each op has equivalent probability
	SET_SINGLE_NAME("binary_add_prob", Add, 1);
	SET_SINGLE_NAME("binary_sub_prob", Sub, 1);

	if (CGOptions::muls()) {
		SET_SINGLE_NAME("binary_mul_prob", Mul, 1);
	}
	else {
		SET_SINGLE_NAME("binary_mul_prob", Mul, 0);
	}

	if (CGOptions::divs()) {
		SET_SINGLE_NAME("binary_div_prob", Div, 1);
	}
	else {
		SET_SINGLE_NAME("binary_div_prob", Div, 0);
	}

	SET_SINGLE_NAME("binary_mod_prob", Mod, 1);
	SET_SINGLE_NAME("binary_gt_prob", CmpGt, 1);
	SET_SINGLE_NAME("binary_lt_prob", CmpLt, 1);
	SET_SINGLE_NAME("binary_ge_prob", CmpGe, 1);
	SET_SINGLE_NAME("binary_le_prob", CmpLe, 1);
	SET_SINGLE_NAME("binary_eq_prob", CmpEq, 1);
	SET_SINGLE_NAME("binary_ne_prob", CmpNe, 1);
	SET_SINGLE_NAME("binary_and_prob", And, 1);
	SET_SINGLE_NAME("binary_or_prob", Or, 1);
	SET_SINGLE_NAME("binary_bit_xor_prob", BitXor, 1);
	SET_SINGLE_NAME("binary_bit_and_prob", BitAnd, 1);
	SET_SINGLE_NAME("binary_bit_or_prob", BitOr, 1);
	SET_SINGLE_NAME("binary_bit_rshift_prob", RShift, 1);
	SET_SINGLE_NAME("binary_bit_lshift_prob", LShift, 1);

	set_group_prob(true, pBinaryOpsProb, m);
	set_prob_filter(pBinaryOpsProb);
}

void
Probabilities::set_default_statement_prob()
{
	std::map<ProbName, int> m;

	// never generate stand-alone blocks
	SET_SINGLE_NAME("statement_block_prob", Block, 0);
	SET_SINGLE_NAME("statement_ifelse_prob", IfElse, 15);
	SET_SINGLE_NAME("statement_for_prob", For, 30);
	SET_SINGLE_NAME("statement_return_prob", Return, 35);
	SET_SINGLE_NAME("statement_continue_prob", Continue, 40);
	SET_SINGLE_NAME("statement_break_prob", Break, 45);
	if (CGOptions::jumps() && CGOptions::arrays()) {
		SET_SINGLE_NAME("statement_goto_prob", Goto, 50);
		SET_SINGLE_NAME("statement_arrayop_prob", ArrayOp, 60);
	}
	else if (CGOptions::jumps() && !CGOptions::arrays()) {
		SET_SINGLE_NAME("statement_arrayop_prob", ArrayOp, 0);
		SET_SINGLE_NAME("statement_goto_prob", Goto, 50);
	}
	else if (!CGOptions::jumps() && CGOptions::arrays()) {
		SET_SINGLE_NAME("statement_goto_prob", Goto, 0);
		SET_SINGLE_NAME("statement_arrayop_prob", ArrayOp, 55);
	}
	else {
		SET_SINGLE_NAME("statement_goto_prob", Goto, 0);
		SET_SINGLE_NAME("statement_arrayop_prob", ArrayOp, 0);
	}
	// use the remaining probabilities for assignments
	SET_SINGLE_NAME("statement_assign_prob", Assign, 100);

	set_group_prob(false, pStatementProb, m);
}

Filter*
Probabilities::get_prob_filter(ProbName pname)
{
	Probabilities *impl = Probabilities::GetInstance();
	assert(impl);
	Filter *filter = impl->prob_filters_[pname];
	if (!filter)
		filter = impl->extra_filters_[pname];
	assert(filter);
	return filter;
}

void
Probabilities::set_prob_filter(ProbName pname)
{
	prob_filters_[pname] = new ProbabilityFilter(pname);
	set_extra_filters(pname);
}

void
Probabilities::register_extra_filter(ProbName pname, Filter *filter)
{
	assert(filter);
	Probabilities *impl = Probabilities::GetInstance();
	assert(impl);
	impl->extra_filters_[pname] = filter;
}

void
Probabilities::unregister_extra_filter(ProbName pname, Filter *filter)
{
	assert(filter);
	Probabilities *impl = Probabilities::GetInstance();
	assert(impl);
	assert(impl->extra_filters_[pname] == filter);
	impl->extra_filters_[pname] = NULL;
}

void
Probabilities::set_extra_filters(ProbName pname)
{
	if (CGOptions::msp()) {
		switch(pname) {
		case pBinaryOpsProb:
			extra_filters_[pname] = new MspBinaryFilter();
			break;
		default:
			break;
		}
	}
}

bool
Probabilities::check_extra_filter(ProbName pname, int v)
{
	assert(v >= 0);
	std::map<ProbName, Filter*>::iterator i = extra_filters_.find(pname);
	if (i != extra_filters_.end() && ((*i).second != NULL))
		return (*i).second->filter(v);
	else
		return false;
}

// set up default probabilities
void
Probabilities::initialize()
{
	set_single_name_maps();
	initialize_single_probs();
	initialize_group_probs();
}

unsigned int
Probabilities::get_prob(ProbName pname)
{
	Probabilities *impl = Probabilities::GetInstance();
	ProbElem *elem = impl->probabilities_[pname];
	int val = elem->get_prob(pname);

	// This assert rules out all invalid accesses to group probs
	// and invalid single prob
	assert(val >= 0 && val <= 100);
	return static_cast<unsigned int>(val);
}

ProbName
Probabilities::get_pname(const string &sname)
{
	std::map<std::string, ProbName>::iterator i = sname_to_pname_.find(sname);
	if (i == sname_to_pname_.end())
		assert("invalid string in the configuration file:" && sname.c_str() && 0);
	return (*i).second;
}

std::string
Probabilities::get_sname(ProbName pname)
{
	std::map<ProbName, std::string>::iterator i = pname_to_sname_.find(pname);
	if (i == pname_to_sname_.end())
		assert("invalid string in the configuration file" && 0);
	return (*i).second;
}

bool
Probabilities::parse_configuration(std::string &error_msg, const string &fname)
{
	ifstream conf(fname.c_str());
	if (!conf.is_open()) {
		error_msg = "fail to open probabilities configuration file!";
		return false;
	}

	std::string line;
	while(!conf.eof()) {
		getline(conf, line);
		if (StringUtils::empty_line(line))
			continue;
		if (!parse_line(error_msg, line))
			return false;
	}
	conf.close();
	//dump_actual_probabilities();
	return true;
}

bool
Probabilities::setup_group_probabilities(bool is_equal, const vector<string> &elems)
{
	assert(elems.size() > 1);
	ProbName pname = get_pname(elems[0]);
	ProbElem *elem = probabilities_[pname];
	assert(elem);
	// Used for sanity check - make sure no two probabilities are the same
	set<int> vals;
	bool all_zero = true;
	bool valid_max_value = false;
	for (size_t i = 1; i < elems.size(); i++) {
		int val = parse_single_elem(is_equal, elem, elems[i]);
		if (is_equal) {
			valid_max_value = true;
			assert(val == 0 || val == 1);
			if (val == 1)
				all_zero = false;
		}
		else {
			all_zero = false;
			if (val == 100)
				valid_max_value = true;
			assert(val >= 0 && val <= 100);
			if ((val > 0) && (vals.find(val) != vals.end()))
				assert("duplicated values in a group probability" && 0);
			else
				vals.insert(val);
		}
	}
	// assert(vals.size() == (elems.size() - 1));
	assert(!all_zero && "Invalid probabilities: all probabilities are zero!");
	assert(valid_max_value && "Invalid group probabilities: one probability value must be 100!");
	return true;
}

bool
Probabilities::parse_group_probabilities(bool is_equal, std::string &error_msg, const std::string &line)
{
	string s;
	if (is_equal)
		s = StringUtils::get_substring(line, GroupProbElem::equal_open_delim, GroupProbElem::equal_close_delim);
	else
		s = StringUtils::get_substring(line, GroupProbElem::group_open_delim, GroupProbElem::group_close_delim);

	if (s.empty()) {
		error_msg = "empty group probabilities!";
		return false;
	}
	std::vector<string> elems;
	StringUtils::split_string(s, elems, GroupProbElem::group_sep_char);
	if (elems.size() <= 1) {
		error_msg = "invalid group probabilities format!";
		return false;
	}
	return setup_group_probabilities(is_equal, elems);
}

int
Probabilities::parse_single_elem(bool is_equal, ProbElem *elem, const std::string &line)
{
	assert(elem);
	std::vector<std::string> pairs;
	StringUtils::split_string(line, pairs, SingleProbElem::single_elem_sep_char);
	assert(pairs.size() == 2);
	int val = StringUtils::str2int(pairs[1]);
	if (is_equal)
		assert(val == 0 || val == 1);
	else
		assert(val >= 0 && val <= 100);
	ProbName pname = get_pname(pairs[0]);
	elem->set_prob(pname, val);
	return val;
}

bool
Probabilities::parse_single_probability(std::string &, const std::string &line)
{
	std::vector<string> pairs;
	StringUtils::split_string(line, pairs, SingleProbElem::single_elem_sep_char);
	assert(pairs.size() == 2);

	int val = StringUtils::str2int(pairs[1]);
	VAL_ASSERT(val);
	ProbName pname = get_pname(pairs[0]);
	ProbElem *elem = probabilities_[pname];
	assert(elem);
	elem->set_prob(pname, val);
	return true;
}

const char Probabilities::comment_line_prefix = '#';

bool
Probabilities::parse_line(std::string &error_msg, string &line)
{
	char c = StringUtils::first_nonspace_char(line);
	bool rv = false;
	bool is_equal = false;
	if (c == '\0') {
		assert("parse empty line, cannot happen!\n" && 0);
	}
	else if (c == Probabilities::comment_line_prefix) {
		return true;
	}
	else if (c == GroupProbElem::group_open_delim) {
		is_equal = false;
		rv = parse_group_probabilities(is_equal, error_msg, line);
	}
	else if (c == GroupProbElem::equal_open_delim) {
		is_equal = true;
		rv = parse_group_probabilities(is_equal, error_msg, line);
	}
	else {
		rv = parse_single_probability(error_msg, line);
	}
	return rv;
}

void
Probabilities::dump_default_probabilities(const string &fname)
{
	assert(!fname.empty());
	ofstream out(fname.c_str());

	std::map<ProbName, ProbElem*>::iterator i; 
	for (i = probabilities_.begin(); i != probabilities_.end(); ++i) {
		(*i).second->dump_default(out);
		out << std::endl << std::endl;
	}
}

void
Probabilities::dump_actual_probabilities(const string &fname, unsigned long seed)
{
	assert(!fname.empty());
	ofstream out(fname.c_str());
	out << "# Seed: " << seed << std::endl << std::endl;

	std::map<ProbName, ProbElem*>::iterator i;
	for (i = probabilities_.begin(); i != probabilities_.end(); ++i) {
		(*i).second->dump_val(out);
		out << std::endl << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////
Probabilities::Probabilities()
{

}

void
Probabilities::clear_filter(std::map<ProbName, Filter*> &filters)
{
	std::map<ProbName, Filter*>::iterator j;
	for (j = filters.begin(); j!= filters.end(); ++j) {
		Filter *f = (*j).second;
		if (f)
			delete f;
	}
	filters.clear();
}

Probabilities::~Probabilities()
{
	std::map<ProbName, ProbElem*>::iterator i;
	for (i = probabilities_.begin(); i != probabilities_.end(); ++i) {
		ProbElem *elem = (*i).second;
		assert(elem);
		delete elem;
	}
	probabilities_.clear();
	clear_filter(prob_filters_);
	clear_filter(extra_filters_);
}

void DistributionTable::add_entry(int key, int prob)
{
	keys_.push_back(key);
	probs_.push_back(prob);
	max_prob_ += prob;
}

int DistributionTable::key_to_prob(int key) const
{
	for (size_t i=0; i<keys_.size(); i++) {
		if (keys_[i] == key) {
			return probs_[i];
		}
	}
	// 0 probablility for keys not found
	return 0;
}

int DistributionTable::rnd_num_to_key(int rnd) const
{
	assert(rnd < max_prob_ && rnd >= 0);
	assert(keys_.size() == probs_.size());
	for (size_t i=0; i<probs_.size(); i++) {
		if (rnd < probs_[i]) {
			return keys_[i];
		}
		rnd -= probs_[i];
	}
	assert(0);
	return -1;
}


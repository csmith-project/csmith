// -*- mode: C++ -*-
//
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 The University of Utah
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

///////////////////////////////////////////////////////////////////////////////

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "CGOptions.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <map>
#include "Fact.h"
#include "DefaultOutputMgr.h"
#include "Bookkeeper.h"
#include "CompatibleChecker.h"
#include "PartialExpander.h"
#include "DeltaMonitor.h"
#include "Probabilities.h"
#include "OutputMgr.h"
#include "StringUtils.h"

using namespace std;
Reducer* CGOptions::reducer_ = NULL;
vector<int> CGOptions::safe_math_wrapper_ids_;
map<string, bool> CGOptions::enabled_builtin_kinds_;
int CGOptions::int_size_ = 0;
int CGOptions::pointer_size_ = 0;

/*
 *
 */
#define DEFINE_GETTER_SETTER(type, init, f)	\
	type CGOptions::f##_ = (init); \
	type \
	CGOptions::f(void) { return f##_; }	\
	type \
	CGOptions::f(type p) { f##_ = p; return p; }

/*
 *
 */
#define DEFINE_GETTER_SETTER_BOOL(f) \
	DEFINE_GETTER_SETTER(bool, false, f)

#define DEFINE_GETTER_SETTER_INT(f) \
	DEFINE_GETTER_SETTER(int, 0, f)

#define DEFINE_GETTER_SETTER_STRING_REF(f) \
	DEFINE_GETTER_SETTER(std::string, "", f)

///////////////////////////////////////////////////////////////////////////////

DEFINE_GETTER_SETTER_BOOL(compute_hash)
DEFINE_GETTER_SETTER_BOOL(depth_protect)
DEFINE_GETTER_SETTER_INT (max_split_files)
DEFINE_GETTER_SETTER_STRING_REF(split_files_dir)
DEFINE_GETTER_SETTER_STRING_REF(output_file)
DEFINE_GETTER_SETTER_INT (max_funcs)
DEFINE_GETTER_SETTER_INT (max_params)
DEFINE_GETTER_SETTER_INT (max_block_size)
DEFINE_GETTER_SETTER_INT (max_blk_depth)
DEFINE_GETTER_SETTER_INT (max_expr_depth)
DEFINE_GETTER_SETTER_BOOL(wrap_volatiles)
DEFINE_GETTER_SETTER_BOOL(allow_const_volatile)
DEFINE_GETTER_SETTER_BOOL(avoid_signed_overflow)
DEFINE_GETTER_SETTER_INT (max_struct_fields)
DEFINE_GETTER_SETTER_INT (max_union_fields)
DEFINE_GETTER_SETTER_INT (max_nested_struct_level)
DEFINE_GETTER_SETTER_STRING_REF(struct_output)
DEFINE_GETTER_SETTER_BOOL (fixed_struct_fields)
DEFINE_GETTER_SETTER_BOOL (expand_struct)
DEFINE_GETTER_SETTER_BOOL (use_struct)
DEFINE_GETTER_SETTER_BOOL (use_union)
DEFINE_GETTER_SETTER_INT (max_indirect_level)
DEFINE_GETTER_SETTER_INT (max_array_dimensions)
DEFINE_GETTER_SETTER_INT (max_array_length_per_dimension)
DEFINE_GETTER_SETTER_INT (max_array_length)
DEFINE_GETTER_SETTER_INT (interested_facts)
DEFINE_GETTER_SETTER_BOOL(paranoid)
DEFINE_GETTER_SETTER_BOOL(quiet)
DEFINE_GETTER_SETTER_BOOL(concise)
DEFINE_GETTER_SETTER_BOOL(nomain)
DEFINE_GETTER_SETTER_BOOL(random_based)
DEFINE_GETTER_SETTER_BOOL(dfs_exhaustive)
DEFINE_GETTER_SETTER_STRING_REF(dfs_debug_sequence)
DEFINE_GETTER_SETTER_INT (max_exhaustive_depth)
DEFINE_GETTER_SETTER_BOOL(compact_output)
DEFINE_GETTER_SETTER_BOOL(msp)
DEFINE_GETTER_SETTER_INT(func1_max_params)
DEFINE_GETTER_SETTER_BOOL(splat)
DEFINE_GETTER_SETTER_BOOL(klee)
DEFINE_GETTER_SETTER_BOOL(crest)
DEFINE_GETTER_SETTER_BOOL(ccomp)
DEFINE_GETTER_SETTER_BOOL(coverage_test)
DEFINE_GETTER_SETTER_INT(coverage_test_size)
DEFINE_GETTER_SETTER_BOOL(packed_struct)
DEFINE_GETTER_SETTER_BOOL(bitfields)
DEFINE_GETTER_SETTER_BOOL(prefix_name)
DEFINE_GETTER_SETTER_BOOL(sequence_name_prefix)
DEFINE_GETTER_SETTER_BOOL(compatible_check)
DEFINE_GETTER_SETTER_STRING_REF(partial_expand)
DEFINE_GETTER_SETTER_STRING_REF(delta_monitor)
DEFINE_GETTER_SETTER_STRING_REF(delta_output)
DEFINE_GETTER_SETTER_STRING_REF(go_delta)
DEFINE_GETTER_SETTER_STRING_REF(delta_input)
DEFINE_GETTER_SETTER_BOOL(no_delta_reduction)
DEFINE_GETTER_SETTER_BOOL(math64)
DEFINE_GETTER_SETTER_BOOL(inline_function)
DEFINE_GETTER_SETTER_BOOL(math_notmp)
DEFINE_GETTER_SETTER_BOOL(longlong)
DEFINE_GETTER_SETTER_BOOL(int8)
DEFINE_GETTER_SETTER_BOOL(uint8)
DEFINE_GETTER_SETTER_BOOL(enable_float)
DEFINE_GETTER_SETTER_BOOL(strict_float)
DEFINE_GETTER_SETTER_BOOL(pointers)
DEFINE_GETTER_SETTER_BOOL(arrays)
DEFINE_GETTER_SETTER_BOOL(strict_const_arrays)
DEFINE_GETTER_SETTER_BOOL(jumps)
DEFINE_GETTER_SETTER_BOOL(return_structs)
DEFINE_GETTER_SETTER_BOOL(arg_structs)
DEFINE_GETTER_SETTER_BOOL(return_unions)
DEFINE_GETTER_SETTER_BOOL(arg_unions)
DEFINE_GETTER_SETTER_BOOL(volatiles)
DEFINE_GETTER_SETTER_BOOL(volatile_pointers)
DEFINE_GETTER_SETTER_BOOL(const_pointers)
DEFINE_GETTER_SETTER_BOOL(global_variables)
DEFINE_GETTER_SETTER_BOOL(access_once)
DEFINE_GETTER_SETTER_BOOL(strict_volatile_rule)
DEFINE_GETTER_SETTER_BOOL(addr_taken_of_locals)
DEFINE_GETTER_SETTER_BOOL(fresh_array_ctrl_var_names)
DEFINE_GETTER_SETTER_BOOL(consts)
DEFINE_GETTER_SETTER_BOOL(builtins)
DEFINE_GETTER_SETTER_BOOL(dangling_global_ptrs)
DEFINE_GETTER_SETTER_BOOL(divs)
DEFINE_GETTER_SETTER_BOOL(muls)
DEFINE_GETTER_SETTER_BOOL(accept_argc)
DEFINE_GETTER_SETTER_BOOL(random_random)
DEFINE_GETTER_SETTER_INT(stop_by_stmt)
DEFINE_GETTER_SETTER_BOOL(step_hash_by_stmt)
DEFINE_GETTER_SETTER_BOOL(compound_assignment)
DEFINE_GETTER_SETTER_STRING_REF(dump_default_probabilities)
DEFINE_GETTER_SETTER_STRING_REF(dump_random_probabilities)
DEFINE_GETTER_SETTER_STRING_REF(probability_configuration)
DEFINE_GETTER_SETTER_BOOL(const_as_condition)
DEFINE_GETTER_SETTER_BOOL(match_exact_qualifiers)
DEFINE_GETTER_SETTER_BOOL(blind_check_global)
DEFINE_GETTER_SETTER_BOOL(no_return_dead_ptr)
DEFINE_GETTER_SETTER_BOOL(hash_value_printf)
DEFINE_GETTER_SETTER_BOOL(signed_char_index)
DEFINE_GETTER_SETTER_INT (max_array_num_in_loop)
DEFINE_GETTER_SETTER_BOOL(identify_wrappers)
DEFINE_GETTER_SETTER_BOOL(mark_mutable_const)
DEFINE_GETTER_SETTER_BOOL(force_globals_static)
DEFINE_GETTER_SETTER_BOOL(force_non_uniform_array_init)
DEFINE_GETTER_SETTER_INT(inline_function_prob)
DEFINE_GETTER_SETTER_INT(builtin_function_prob)
DEFINE_GETTER_SETTER_INT(null_pointer_dereference_prob)
DEFINE_GETTER_SETTER_INT(dead_pointer_dereference_prob)
DEFINE_GETTER_SETTER_BOOL(union_read_type_sensitive);
DEFINE_GETTER_SETTER_BOOL(pre_incr_operator);
DEFINE_GETTER_SETTER_BOOL(pre_decr_operator);
DEFINE_GETTER_SETTER_BOOL(post_incr_operator);
DEFINE_GETTER_SETTER_BOOL(post_decr_operator);
DEFINE_GETTER_SETTER_BOOL(unary_plus_operator);
DEFINE_GETTER_SETTER_BOOL(use_embedded_assigns);
DEFINE_GETTER_SETTER_BOOL(use_comma_exprs);
DEFINE_GETTER_SETTER_BOOL(take_union_field_addr);
DEFINE_GETTER_SETTER_BOOL(vol_struct_union_fields);
DEFINE_GETTER_SETTER_BOOL(const_struct_union_fields);
DEFINE_GETTER_SETTER_BOOL(lang_cpp);
DEFINE_GETTER_SETTER_BOOL(cpp11);
DEFINE_GETTER_SETTER_BOOL(fast_execution);

void
CGOptions::set_default_builtin_kinds()
{
	enabled_builtin_kinds_["generic"] = true;
	enabled_builtin_kinds_["x86"] = true;
}

void
CGOptions::set_default_settings(void)
{
	set_platform_specific_options();
	compute_hash(true);
	max_funcs(CGOPTIONS_DEFAULT_MAX_SPLIT_FILES);
	max_funcs(CGOPTIONS_DEFAULT_MAX_FUNCS);
	max_params(CGOPTIONS_DEFAULT_MAX_PARAMS);
	max_block_size(CGOPTIONS_DEFAULT_MAX_BLOCK_SIZE);
	max_blk_depth(CGOPTIONS_DEFAULT_MAX_BLOCK_DEPTH);
	max_expr_depth(CGOPTIONS_DEFAULT_MAX_EXPR_DEPTH);
	max_struct_fields(CGOPTIONS_DEFAULT_MAX_STRUCT_FIELDS);
	max_union_fields(CGOPTIONS_DEFAULT_MAX_UNION_FIELDS);
	max_nested_struct_level(CGOPTIONS_DEFAULT_MAX_NESTED_STRUCT_LEVEL);
	fixed_struct_fields(false);
	expand_struct(false);
	max_array_dimensions(CGOPTIONS_DEFAULT_MAX_ARRAY_DIMENSIONS);
	max_array_length_per_dimension(CGOPTIONS_DEFAULT_MAX_ARRAY_LENGTH_PER_DIMENSION);
	max_array_length(CGOPTIONS_DEFAULT_MAX_ARRAY_LENGTH);
	max_exhaustive_depth(CGOPTIONS_DEFAULT_MAX_EXHAUSTIVE_DEPTH);
	max_indirect_level(CGOPTIONS_DEFAULT_MAX_INDIRECT_LEVEL);
	output_file(CGOPTIONS_DEFAULT_OUTPUT_FILE);
	interested_facts(ePointTo | eUnionWrite);
	allow_const_volatile(true);
	avoid_signed_overflow(true);
	CGOptions::paranoid(false);
	CGOptions::quiet(false);
	CGOptions::concise(false);
	CGOptions::nomain(false);
	random_based(true);
	use_struct(true);
	use_union(true);
	compact_output(false);
	msp(false);
	func1_max_params(CGOPTIONS_DEFAULT_FUNC1_MAX_PARAMS);
	splat(false);
	klee(false);
	crest(false);
	ccomp(false);
	coverage_test(false);
	coverage_test_size(CGOPTIONS_DEFAULT_COVERAGE_TEST_SIZE);
	packed_struct(true);
	bitfields(true);
	prefix_name(false);
	sequence_name_prefix(false);
	compatible_check(false);
	compound_assignment(true);
	math64(true);
	inline_function(false);
	math_notmp(false);
	longlong(true);
	int8(true);
	uint8(true);
	enable_float(false);
	strict_float(false);
	pointers(true);
	arrays(true);
	strict_const_arrays(false);
	jumps(true);
	return_structs(true);
	arg_structs(true);
	return_unions(true);
	arg_unions(true);
	volatiles(true);
	volatile_pointers(true);
	const_pointers(true);
	global_variables(true);
	consts(true);
	dangling_global_ptrs(true);
	divs(true);
	muls(true);
	accept_argc(true);
	stop_by_stmt(-1);
	step_hash_by_stmt(false);
	const_as_condition(false);
	match_exact_qualifiers(false);
	blind_check_global(false);
	no_return_dead_ptr(true);
	hash_value_printf(true);
	signed_char_index(true);
	identify_wrappers(false);
	mark_mutable_const(false);
	force_globals_static(true);
	force_non_uniform_array_init(true);
	max_array_num_in_loop(CGOPTIONS_DEFAULT_MAX_ARRAY_NUM_IN_LOOP);
	inline_function_prob(50);
	builtin_function_prob(50);
	null_pointer_dereference_prob(0);
	dead_pointer_dereference_prob(0);
	union_read_type_sensitive(true);
	pre_incr_operator(true);
	pre_decr_operator(true);
	post_incr_operator(true);
	post_decr_operator(true);
	unary_plus_operator(true);
	use_embedded_assigns(true);
	use_comma_exprs(true);
	take_union_field_addr(true);
	vol_struct_union_fields(true);
	const_struct_union_fields(true);
	addr_taken_of_locals(true);
	lang_cpp(false);
	cpp11(false);
  fast_execution(false);

	set_default_builtin_kinds();
}

// Add options necessary for cpp 
void
CGOptions::fix_options_for_cpp(void)
{
	match_exact_qualifiers(true);
    vol_struct_union_fields(false);		// makes implementation of volatile structs easier
    const_struct_union_fields(false);	// restriction of current implementation; TODO
}

/*
   looking for the platform info file in the working directory
   and load platform specific information. If not found, use
   info from the platform that Csmith is running, and output them
   to the file
*/
void
CGOptions::set_platform_specific_options(void)
{
	const char* int_str = "integer size = ";
	const char* ptr_str = "pointer size = ";
	ifstream conf(PLATFORM_CONFIG_FILE);
	if (conf.fail()) {
		ofstream conf(PLATFORM_CONFIG_FILE);
		conf << int_str << sizeof(int) << endl;
		conf << ptr_str << sizeof(int*) << endl;
		int_size(sizeof(int));
		pointer_size(sizeof(int*));
		conf.close();
	}
	else {
		string line;
		while(!conf.eof()) {
			getline(conf, line);
			if (line.substr(0, strlen(int_str)) == int_str) {
				string s = line.substr(strlen(int_str));
				StringUtils::chop(s);
				int_size(StringUtils::str2int(s));
			}
			if (line.substr(0, strlen(ptr_str)) == ptr_str) {
				string s = line.substr(strlen(ptr_str));
				StringUtils::chop(s);
				pointer_size(StringUtils::str2int(s));
			}
		}
		if (!int_size_) {
			cout << "please specify integer size in " << PLATFORM_CONFIG_FILE << endl;
			exit(-1);
		}
		if (!pointer_size_) {
			cout << "please specify pointer size in " << PLATFORM_CONFIG_FILE << endl;
			exit(-1);
		}
		conf.close();
	}
}

int
CGOptions::int_size(void)
{
	Bookkeeper::rely_on_int_size = true;
	return int_size_;
}

int
CGOptions::pointer_size(void)
{
	return pointer_size_;
}

bool
CGOptions::is_random(void)
{
	return random_based_;
}

std::string CGOptions::conflict_msg_ = "";

const std::string&
CGOptions::conflict_msg(void)
{
	return conflict_msg_;
}

bool CGOptions::has_random_based_conflict(void)
{
	bool rv = false;
	if (!CGOptions::random_based()) {
		if (!CGOptions::split_files_dir().empty()) {
			conflict_msg_ = "split_files_dir ";
			rv = true;
		}
		else if (CGOptions::max_split_files() > 0) {
			conflict_msg_ = "max_split_files ";
			rv = true;
		}
		if (rv) {
			conflict_msg_ += "can only be applied to random mode";
			return rv;
		}
	}
	else {
		if (CGOptions::sequence_name_prefix()) {
			conflict_msg_ = "--sequence-name-prefix option can only be \
					used with --dfs-exhaustive";
			rv = true;
		}
	}
	return rv;
}

bool CGOptions::resolve_exhaustive_options()
{
	if (!CGOptions::dfs_exhaustive())
		return false;

	bool rv = false;

	if (CGOptions::compatible_check())
		CompatibleChecker::enable_compatible_check();
	if (!CGOptions::partial_expand().empty()) {
		if (!PartialExpander::init_partial_expander(CGOptions::partial_expand())) {
			conflict_msg_ = "wrong partial-expand options!";
			return true;
		}
	}
	if (CGOptions::max_exhaustive_depth() <= 0) {
		conflict_msg_ = "max-exhaustive-depth must be at least 0";
		return true;
	}

	if (!CGOptions::use_struct() && CGOptions::expand_struct()) {
		conflict_msg_ = "expand-struct cannot be used with --no-struct";
		return true;
	}

	if (CGOptions::has_extension_support()) {
		conflict_msg_ = "exhaustive mode doesn't support splat|klee|crest|coverage-test extension";
		return true;
	}
	// For effeciency reason, we fix the size of struct fields
	CGOptions::fixed_struct_fields(true);
	return rv;
}

bool
CGOptions::has_delta_conflict()
{
	if (!CGOptions::delta_monitor().empty() &&
		!CGOptions::go_delta().empty()) {
		conflict_msg_ = "you cannot specify --delta-monitor and \
			--go-delta monitor at the same time. --go-delta \
			will invoke the corresponding delta monitor implicitly.";
		return true;
	}
	return false;
}

bool
CGOptions::has_extension_conflict()
{
	int count = 0;

	if (CGOptions::splat())
		count++;
	if (CGOptions::klee())
		count++;
	if (CGOptions::crest())
		count++;
	if (CGOptions::coverage_test())
		count++;

	if (count > 1) {
		conflict_msg_ = "You could only specify --splat or --klee or --crest or --coverage-test";
		return true;
	}
	return false;
}

bool
CGOptions::has_extension_support()
{
	return (CGOptions::splat() || CGOptions::klee()
		|| CGOptions::crest() || CGOptions::coverage_test());
}

bool
CGOptions::allow_int64()
{
	return ((!CGOptions::has_extension_support()) &&
			CGOptions::math64() &&
			CGOptions::longlong());
}

bool
CGOptions::has_conflict(void)
{
	if (CGOptions::has_random_based_conflict())
		return true;

	if (resolve_exhaustive_options())
		return true;

	if (CGOptions::has_extension_conflict())
		return true;

	if (CGOptions::has_delta_conflict())
		return true;

	if ((CGOptions::inline_function_prob() < 0) ||
	    (CGOptions::inline_function_prob() > 100)) {
		conflict_msg_ = "inline-function-prob value must between [0,100]";
		return true;
	}

	if ((CGOptions::builtin_function_prob() < 0) ||
	    (CGOptions::builtin_function_prob() > 100)) {
		conflict_msg_ = "builtin-function-prob value must between [0,100]";
		return true;
	}


	if (CGOptions::max_funcs() < 1) {
		conflict_msg_ = "max-funcs must be at least 1";
		return true;
	}
	if (CGOptions::max_blk_depth() < 1) {
		conflict_msg_ = "max-stmt-depth must be at least 1";
		return true;
	}

	if (CGOptions::func1_max_params() > CGOptions::max_params()) {
		conflict_msg_ = "func1_max_params() cannot be larger than max_params()";
		return true;
	}
	if (CGOptions::split_files_dir().empty()) {
		CGOptions::split_files_dir(CGOPTIONS_DEFAULT_SPLIT_FILES_DIR);
	}
	if (CGOptions::max_split_files() > 0) {
		if (!DefaultOutputMgr::create_output_dir(CGOptions::split_files_dir())) {
			conflict_msg_ = "cannot create dir for split files!";
			return true;
		}
	}

	if (!CGOptions::delta_monitor().empty()) {
		string msg;
		if (!DeltaMonitor::init(msg, CGOptions::delta_monitor(), CGOptions::delta_output())) {
			conflict_msg_ = msg;
			return true;
		}
	}

	if (!CGOptions::go_delta().empty()) {
		string msg;
		if (!DeltaMonitor::init_for_running(msg, CGOptions::go_delta(), CGOptions::delta_output(),
				CGOptions::delta_input(), CGOptions::no_delta_reduction())) {
			conflict_msg_ = msg;
			return true;
		}
	}

	if (!CGOptions::lang_cpp() && CGOptions::cpp11()) {
		conflict_msg_ = "--cpp11 option makes sense only with --lang-cpp option enabled.";
		return true;
	}

#if 0
	Probabilities *prob = Probabilities::GetInstance();
	if (!(CGOptions::dump_default_probabilities().empty()))
		prob->dump_default_probabilities(CGOptions::dump_default_probabilities());

	string msg;
	if (!(CGOptions::probability_configuration().empty())) {
		if (!prob->parse_configuration(msg, CGOptions::probability_configuration())) {
			conflict_msg_ = msg;
			return true;
		}
	}
#endif

	return false;
}

void
CGOptions::monitored_funcs(std::string fnames)
{
	parse_string_options(fnames, OutputMgr::monitored_funcs_);
}

void
CGOptions::parse_string_options(string vname, vector<std::string> &v)
{
	if (vname.empty()) return;
	// parse the string if there are multiple variable names in format "v1,v2,..."
	size_t pos1 = 0;
	size_t pos2 = 0;
	do {
		pos2 = vname.find_first_of(',', pos1);
		string var_name = (pos2 != string::npos) ? vname.substr(pos1, pos2 - pos1) : vname.substr(pos1);
		v.push_back(var_name);
		pos1 = pos2 + 1;
	} while (pos2 != string::npos);
}

bool
CGOptions::x86_64()
{
#if defined __x86_64__
	return true;
#else
	return false;
#endif
}

std::string CGOptions::vol_tests_mach_ = "";

std::string
CGOptions::vol_tests_mach(void)
{
	return CGOptions::vol_tests_mach_;
}

void
CGOptions::safe_math_wrapper(string ids)
{
	StringUtils::split_int_string(ids, safe_math_wrapper_ids_, ",");
}

bool
CGOptions::safe_math_wrapper(int id)
{
	// if no safe math wrapper ids specified, assume all needs wrapper
	if (safe_math_wrapper_ids_.empty()) return true;
	return std::find(safe_math_wrapper_ids_.begin(), safe_math_wrapper_ids_.end(), id) != safe_math_wrapper_ids_.end();
}

void
CGOptions::disable_builtin_kinds(const string &kinds)
{
	vector<string> vs;
	StringUtils::split_string(kinds, vs, ",");
	for (vector<string>::const_iterator i = vs.begin(), e = vs.end(); i != e; ++i) {
		enabled_builtin_kinds_[*i] = false;
	}
}

void
CGOptions::enable_builtin_kinds(const string &kinds)
{
	vector<string> vs;
	StringUtils::split_string(kinds, vs, ",");
	for (vector<string>::const_iterator i = vs.begin(), e = vs.end(); i != e; ++i) {
		enabled_builtin_kinds_[*i] = true;
	}
}

bool CGOptions::enabled_builtin_kind(const string &kind)
{
	map<string, bool>::iterator i = enabled_builtin_kinds_.find(kind);
	if (i == enabled_builtin_kinds_.end())
		return false;
	return i->second;
}

bool
CGOptions::enabled_builtin(const string &ks)
{
	vector<string> vs;
	StringUtils::split_string(ks, vs, "|");
	for (vector<string>::iterator iter_begin = vs.begin(), iter_end = vs.end(); iter_begin != iter_end; ++iter_begin) {
		if (enabled_builtin_kind(*iter_begin))
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

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

//
// This file was derived from a random program generator written by Bryan
// Turner.  The attributions in that file was:
//
// Random Program Generator
// Bryan Turner (bryan.turner@pobox.com)
// July, 2005
//
// Contributions and bug fixes by:
// Jan-2007 : Mat Hostetter - Explicit "return 0" for main()
//

/*
Random C/C++ Program Generator
------------------------------
1) Create a set of random types to be used throughout the program
2) Create the main func.
3) Generate a random block of statements with maximum control & expression nesting depths.
4) If new functions were defined in #3, then loop back to fill in its body.
5) When a maximum number of functions is reached, stop generating new functions and finish off the bodies of the remaining funcs.
6) Output generated program.

GOALS:
- Locate basic errors in compiler front-ends (crashes, etc).
- Ferret out correctness errors in optimization passes.
- Support the design of tools to search for improved optimization paths (partial exection, etc).

TODO:
- Protect back links with a global DEPTH, don't call if DEPTH is too high (avoid infinite recursion)
- Main function generates hash of global state as output of program - use to locate optimization errors.
	- Compile in Debug mode vs. Optimized mode, compare hash value at program termination.
- Improve hash function; use stronger hashing (ARCFOUR) and perform hashing at random points in the graph.
	- Output only after successful program termination.

FUTURE:
- Complex types
- Type-correct expressions
- Some substitutions allowed
	- int, char, short, long, float, double - all interchangeable to some degree (use appropriate casts)
	- array <--> pointer
	- pointer promotion (ie: passing the pointer to a local var, or droping the pointer to pass by value)
- Memory Allocation & Manipulation?
	- Choose random functions to perform allocations
	- Choose random children/ancestors to perform deallocations
	- Work from leaves to root
	- If node uses pointer or array, it is potential heap store allocated.
*/

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef WIN32
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif

#include <ostream>
#include <fstream>
#include <cstring>
#include <cstdio>

#include "Common.h"

#include "CGOptions.h"
#include "AbsProgramGenerator.h"

#include "git_version.h"
#include "platform.h"
#include "random.h"

using namespace std;

//#define PACKAGE_STRING "csmith 1.1.1"
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// Globals

// Program seed - allow user to regenerate the same program on different
// platforms.
static unsigned long g_Seed = 0;

// ----------------------------------------------------------------------------
static void
print_version(void)
{
	cout << PACKAGE_STRING << endl;
	cout << "Git version: " << git_version << endl;
	// XXX print copyright, contact info, etc.?
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
bool parse_string_arg(const char *arg, string &s)
{
	s.assign(arg);
	return ((!s.empty()) &&
		(s.compare(0, 2, "--")));
}

static bool
parse_int_arg(char *arg, unsigned long *ret)
{
	int res;
	res = sscanf (arg, "%lu", ret);

	if (res == 0) {
		cout << "expected integer at arg position " << endl;
		return false;
	}
	return true;
}

static void print_help()
{
	cout << "Command line options: " << endl << endl;
	// most common options
	cout << "  --help or -h: print this information." << endl << endl;
	cout << "  -hh: describe extra options probably useful only for Csmith developers." << endl << endl;
	cout << "  --version or -v: print the version of Csmith." << endl << endl;
	cout << "  --seed <seed> or -s <seed>: use <seed> instead of a random seed generated by Csmith." << endl << endl;
	cout << "  --output <filename> or -o <filename>: specify the output file name." << endl << endl;

	// enabling/disabling options
	cout << "  --argc | --no-argc: genereate main function with/without argv and argc being passed (enabled by default)." << endl << endl;
	cout << "  --arrays | --no-arrays: enable | disable arrays (enabled by default)." << endl << endl;
	cout << "  --bitfields | --no-bitfields: enable | disable full-bitfields structs (enabled by default)." << endl << endl;
	cout << "  --checksum | --no-checksum: enable | disable checksum calculation (enabled by default)." << endl << endl;
	cout << "  --comma-operators | --no-comma-operators: enable | disable comma operators (enabled by default)." << endl << endl;
	cout << "  --compound-assignment | --no-compound-assignment: enable | disable compound assignments (enabled by default)." << endl << endl;
	cout << "  --concise: generated programs with minimal comments (disabled by default)." << endl << endl;
	cout << "  --consts | --no-consts: enable | disable const qualifier (enabled by default)." << endl << endl;
	cout << "  --divs | --no-divs: enable | disable divisions (enabled by default)." << endl << endl;
	cout << "  --embedded-assigns | --no-embedded-assigns: enable | disable embedded assignments as sub-expressions (enabled by default)." << endl << endl;
	cout << "  --pre-incr-operator | --no-pre-incr-operator: enable | disable pre ++ operator (enabled by default)." << endl << endl;
	cout << "  --pre-decr-operator | --no-pre-decr-operator: enable | disable pre -- operator (enabled by default)." << endl << endl;
	cout << "  --post-incr-operator | --no-post-incr-operator: enable | disable post ++ operator (enabled by default)." << endl << endl;
	cout << "  --post-decr-operator | --no-post-decr-operator: enable | disable post -- operator (enabled by default)." << endl << endl;
	cout << "  --unary-plus-operator | --no-unary-plus-operator: enable | disable + operator (enabled by default)." << endl << endl;
	cout << "  --jumps | --no-jumps: enable | disable jumps (enabled by default)." << endl << endl;
	cout << "  --longlong| --no-longlong: enable | disable long long (enabled by default)." << endl << endl;
	cout << "  --int8 | --no-int8: enable | disable int8_t (enabled by default)." << endl << endl;
	cout << "  --uint8 | --no-uint8: enable | disable uint8_t (enabled by default)." << endl << endl;
	cout << "  --float | --no-float: enable | disable float (disabled by default)." << endl << endl;
	cout << "  --main | --nomain: enable | disable to generate main function (enabled by default)." << endl <<  endl;
	cout << "  --math64 | --no-math64: enable | disable 64-bit math ops (enabled by default)." << endl << endl;
	cout << "  --inline-function | --no-inline-function: enable | disable inline attributes on generated functions." << endl << endl;
	cout << "  --inline-function-prob <num>: set the probability of each function being marked as inline (default is 50)." << endl << endl;

	// numbered controls
	cout << "  --max-array-dim <num>: limit array dimensions to <num>. (default 3)" << endl << endl;
	cout << "  --max-array-len-per-dim <num>: limit array length per dimension to <num> (default 10)." << endl << endl;
	cout << "  --max-block-depth <num>: limit depth of nested blocks to <num> (default 5)." << endl << endl;
	cout << "  --max-block-size <size>: limit the number of non-return statements in a block to <size> (default 4)." << endl << endl;
	cout << "  --max-expr-complexity <num>: limit expression complexities to <num> (default 10)." << endl << endl;
	cout << "  --max-funcs <num>: limit the number of functions (besides main) to <num>  (default 10)." << endl << endl;
	cout << "  --max-pointer-depth <depth>: limit the indirect depth of pointers to <depth> (default 2)." << endl << endl;
	cout << "  --max-struct-fields <num>: limit the number of struct fields to <num> (default 10). " << endl << endl;
	cout << "  --max-union-fields <num>: limit the number of union fields to <num> (default 5). " << endl << endl;

	cout << "  --muls | --no-muls: enable | disable multiplications (enabled by default)." << endl << endl;
	cout << "  --safe-math | --no-safe-math: Emit safe math wrapper functions (enabled by default)." << endl << endl;
	cout << "  --packed-struct | --no-packed-struct: enable | disable packed structs by adding #pragma pack(1) before struct definition (enabled by default)." << endl << endl;
	cout << "  --paranoid | --no-paranoid: enable | disable pointer-related assertions (disabled by default)." << endl << endl;
	cout << "  --pointers | --no-pointers: enable | disable pointers (enabled by default)." << endl << endl;
	cout << "  --quiet: generate programs with less comments (disabled by default)." << endl << endl;
	cout << "  --structs | --no-structs: enable | disable to generate structs (enable by default)." << endl << endl;
	cout << "  --unions | --no-unions: enable | disable to generate unions (enable by default)." << endl << endl;
	cout << "  --volatiles | --no-volatiles: enable | disable volatiles (enabled by default)." << endl << endl;
	cout << "  --volatile-pointers | --no-volatile-pointers: enable | disable volatile pointers (enabled by default)." << endl << endl;
	cout << "  --const-pointers | --no-const-pointers: enable | disable const pointers (enabled by default)." << endl << endl;
	cout << "  --global-variables | --no-global-variables: enable | disable global variables (enabled by default)." << endl << endl;

	cout << "  --builtins | --no-builtins: enable | disable to generate builtin functions (disabled by default)." << endl << endl;
	cout << "  --enable-builtin-kinds k1,k2 | --disable-builtin-kinds k1,k2: enable | disable certain kinds of builtin functions." << endl << endl;
	cout << "  --builtin-function-prob <num>: set the probability of choosing a builtin function (default is 20)." << endl << endl;

        // language options
	cout << "  --lang-cpp : generate C++ code (C by default)." << endl << endl;
	cout << "  --cpp11 : generate C++11 code (C++03 by default). Works if lang-cpp is enabled." << endl << endl;

}

static void print_advanced_help()
{
	cout << "'Advanced' command line options that are probably only useful for Csmith's" << endl;
	cout << "original developers:" << endl << endl;
	// file split options
	cout << "  --max-split-files <num>: evenly split a generated program into <num> different files(default 0)." << endl << endl;
	cout << "  --split-files-dir <dir>: generate split-files into <dir> (default ./output)." << endl << endl;

	// dfs-exhaustive mode options
	cout << "  --dfs-exhaustive: enable depth first exhaustive random generation (disabled by default)." << endl << endl;
	cout << "  --expand-struct: enable the expansion of struct in the exhaustive mode. ";
	cout << "Only works in the exhaustive mode and cannot used with --no-structs." << endl << endl;

	cout << "  --compact-output: print generated programs in compact way. ";
	cout << "Only works in the exhaustive mode." << endl << endl;

	cout << "  --max-nested-struct-level <num>: limit maximum nested level of structs to <num>(default 0). ";
	cout << "Only works in the exhaustive mode." << endl << endl;

	cout << "  --struct-output <file>: dump structs declarations to <file>. ";
	cout << "Only works in the exhaustive mode." << endl << endl;

	cout << "  --prefix-name: prefix names of global functions and variables with increasing numbers. ";
	cout << "Only works in the exhaustive mode." << endl << endl;

	cout << "  --sequence-name-prefix: prefix names of global functions and variables with sequence numbers.";
	cout << "Only works in the exhaustive mode." << endl << endl;

	cout << "  --compatible-check: disallow trivial code such as i = i in random programs. ";
	cout << "Only works in the exhaustive mode." << endl << endl;

	// target platforms
	cout << "  --msp: enable certain msp related features " << endl << endl;
	cout << "  --ccomp: generate compcert-compatible code" << endl << endl;

	// symblic excutions
	cout << "  --splat: enable splat extension" << endl << endl;
	cout << "  --klee: enable klee extension" << endl << endl;
	cout << "  --crest: enable crest extension" << endl << endl;

	// coverage test options
	cout << "  --coverage-test: enable coverage-test extension" << endl << endl;
	cout << "  --coverage-test-size <num>: specify size (default 500) of the array generated to test coverage. ";
	cout << "Can only be used with --coverage-test." << endl << endl;

	cout << "  --func1_max_params <num>: specify the number of symbolic variables passed to func_1 (default 3). ";
	cout << "Only used when --splat | --crest | --klee | --coverage-test is enabled." << endl << endl;

	// struct/union related options
	cout << "  --fixed-struct-fields: fix the size of struct fields to max-struct-fields (default 10)." << endl << endl;
	cout << "  --return-structs | --no-return-structs: enable | disable return structs from a function (enabled by default)." << endl << endl;
	cout << "  --arg-structs | --no-arg-structs: enable | disable structs being used as args (enabled by default)." << endl << endl;
	cout << "  --return-unions | --no-return-unions: enable | disable return unions from a function (enabled by default)." << endl << endl;
	cout << "  --arg-unions | --no-arg-unions: enable | disable unions being used as args (enabled by default)." << endl << endl;
	cout << "  --take-union-field-addr | --take-no-union-field-addr: allow | disallow addresses of union fields to be taken (allowed by default)." << endl << endl;
	cout << "  --vol-struct-union-fields | --no-vol-struct-union-fields: enable | disable volatile struct/union fields (enabled by default)" << endl << endl;
	cout << "  --const-struct-union-fields | --no-const-struct-union-fields: enable | disable const struct/union fields (enabled by default)" << endl << endl;

	// delta related options
	cout << "  --delta-monitor [simple]: specify the type of delta monitor. Only [simple] type is supported now." << endl << endl;
	cout << "  --delta-input [file]: specify the file for delta input." << endl << endl;
	cout << "  --delta-output [file]: specify the file for delta output (default to <delta-input>)." << endl << endl;
	cout << "  --go-delta [simple]: run delta reduction on <delta-input>." << endl << endl;
	cout << "  --no-delta-reduction: output the same program as <delta-input>. ";
	cout << "Only works with --go-delta option." << endl << endl;

	// probability options
	cout << "  --dump-default-probabilities <file>: dump the default probability settings into <file>" << endl << endl;
	cout << "  --dump-random-probabilities <file>: dump the randomized probabilities into <file>" << endl << endl;
	cout << "  --probability-configuration <file>: use probability configuration <file>" << endl << endl;
	cout << "  --random-random: enable random probabilities." << endl << endl;

	// volatile checking options
	cout << "  --enable-access-once: enable testing access once macro." << endl << endl;
	cout << "  --strict-volatile-rule: make sure only one volatile access between any pair of sequence points. " << endl << endl;

	cout << "  --addr-taken-of-locals: enable addr-taken of local vars. [default]" << endl << endl;
	cout << "  --no-addr-taken-of-locals: disable addr-taken of local vars. " << endl << endl;

	cout << "  --fresh-array-ctrl-var-names: create fresh names [i1,i2,i3...] for array control vars rather than use unique names such as i, j, k." << endl << endl;

	// other options
	cout << "  --math-notmp: make csmith generate code for safe_math_macros_notmp." << endl << endl;

	cout << "  --strict-const-arrays: restrict array elements to constants." << endl << endl;

	cout << "  --partial-expand <assignment[,for[,block[,if-else[,invoke[,return]]]]]: ";
	cout <<"partial-expand controls which what kind of statements should be generated first. ";
	cout <<"For example, it could make Csmith start to generate if-else without go over assignment or for." << endl << endl;

	cout << "  --dangling-global-pointers | --no-dangling-global-pointers: enable | disable to reset all dangling global pointers to null at the end of main. (enabled by default)" << endl << endl;

	cout << "  --check-global: print the values of all integer global variables." << endl << endl;

	cout << "  --monitor-funcs <name1,name2...>: dump the checksums after each statement in the monitored functions." << endl << endl;

	cout << "  --step-hash-by-stmt: dump the checksum after each statement. It is applied to all functions unless --monitor-funcs is specified." << endl << endl;

	cout << "  --stop-by-stmt <num>: try to stop generating statements after the statement with id <num>." << endl << endl;

	cout << "  --const-as-condition: enable const to be conditions of if-statements. " << endl << endl;

	cout << "  --match-exact-qualifiers: match exact const/volatile qualifiers for LHS and RHS of assignments." << endl << endl;

	cout << "  --reduce <file>: reduce random program under the direction of the configuration file." << endl << endl;

	cout << "  --return-dead-pointer | --no-return-dead-pointer: allow | disallow functions from returning dangling pointers (disallowed by default)." << endl << endl;

	cout <<	"  --identify-wrappers: assign ids to used safe math wrappers." << endl << endl;

	cout << "  --safe-math-wrappers <id1,id2...>: specifiy ids of wrapper functions that are necessary to avoid undefined behaviors, use 0 to specify none." << endl << endl;

	cout << "  --mark-mutable-const: mark constants that can be mutated with parentheses (disabled by default)." << endl << endl;

	cout << "  --force-non-uniform-arrays | --no-force-non-uniform-arrays: force integer arrays to be initialized with multiple values (enabled by default)." << endl << endl;

	cout << "  --null-ptr-deref-prob <N>: allow null pointers to be dereferenced with probability N% (0 by default)." << endl << endl;

	cout << "  --dangling-ptr-deref-prob <N>: allow dangling pointers to be dereferenced with probability N% (0 by default)." << endl << endl;

	cout << "  --union-read-type-sensitive | --no-union-read-type-sensitive: allow | disallow reading an union field when there is no risk of "
		 << "reading padding bits (enabled by default)." << endl << endl;

	cout << "  --max-struct-nested-level: controls the max depth of nested structs (default is 3)." << endl << endl;
	cout << "  --no-hash-value-printf: do not emit printf on the index of an array" << endl << endl;
	cout << "  --no-signed-char-index: do not allow a var of type char to be used as array index" << endl << endl;
	cout << "  --strict-float: do not allow assignments between floats and integers" << endl << endl;
}

void arg_check(int argc, int i)
{
	if (i >= argc) {
		cout << "expect arg at pos " << i << std::endl;
		exit(-1);
	}
}

// ----------------------------------------------------------------------------
int
main(int argc, char **argv)
{
	g_Seed = platform_gen_seed();

	CGOptions::set_default_settings();

	for (int i=1; i<argc; i++) {

		if (strcmp (argv[i], "--help") == 0 ||
			strcmp (argv[i], "-h") == 0) {
			print_help();
			return 0;
		}

		if (strcmp (argv[i], "-hh") == 0) {
			print_advanced_help();
			return 0;
		}

		if (strcmp (argv[i], "--version") == 0 ||
			strcmp (argv[i], "-v") == 0) {
			print_version();
			return 0;
		}

		if (strcmp (argv[i], "--seed") == 0 ||
			strcmp (argv[i], "-s") == 0) {
			i++;
			arg_check(argc, i);

			if (!parse_int_arg(argv[i], &g_Seed))
				exit(-1);
			continue;
		}

		if (strcmp (argv[i], "--max-block-size") == 0) {
			unsigned long size = 0;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &size))
				exit(-1);
			CGOptions::max_block_size(size);
			continue;
		}

		if (strcmp (argv[i], "--max-funcs") == 0) {
			unsigned long size = 0;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &size))
				exit(-1);
			CGOptions::max_funcs(size);
			continue;
		}

		if (strcmp (argv[i], "--func1_max_params") == 0) {
			unsigned long num = 0;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &num))
				exit(-1);
			CGOptions::func1_max_params(num);
			continue;
		}

		if (strcmp (argv[i], "--splat") == 0) {
			CGOptions::splat(true);
			continue;
		}

		if (strcmp (argv[i], "--klee") == 0) {
			CGOptions::klee(true);
			continue;
		}

		if (strcmp (argv[i], "--crest") == 0) {
			CGOptions::crest(true);
			continue;
		}

		if (strcmp (argv[i], "--ccomp") == 0) {
			CGOptions::ccomp(true);
			continue;
		}

		if (strcmp (argv[i], "--coverage-test") == 0) {
			CGOptions::coverage_test(true);
			continue;
		}

		if (strcmp (argv[i], "--coverage-test-size") == 0) {
			unsigned long size = 0;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &size))
				exit(-1);
			CGOptions::coverage_test_size(size);
			continue;
		}

		if (strcmp (argv[i], "--max-split-files") == 0) {
			unsigned long size = 0;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &size))
				exit(-1);
			CGOptions::max_split_files(size);
			continue;
		}

		if (strcmp (argv[i], "--split-files-dir") == 0) {
			string dir;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], dir)) {
				cout << "please specify <dir>" << std::endl;
				exit(-1);
			}
			CGOptions::split_files_dir(dir);
			continue;
		}

		if (strcmp (argv[i], "--dfs-exhaustive") == 0) {
			CGOptions::dfs_exhaustive(true);
			CGOptions::random_based(false);
			continue;
		}

		if (strcmp (argv[i], "--compact-output") == 0) {
			CGOptions::compact_output(true);
			continue;
		}

		if (strcmp (argv[i], "--msp") == 0) {
			CGOptions::msp(true);
			continue;
		}

		if (strcmp (argv[i], "--packed-struct") == 0) {
			CGOptions::packed_struct(true);
			continue;
		}

		if (strcmp (argv[i], "--no-packed-struct") == 0) {
			CGOptions::packed_struct(false);
			continue;
		}

		if (strcmp (argv[i], "--bitfields") == 0) {
			CGOptions::bitfields(true);
			continue;
		}

		if (strcmp (argv[i], "--no-bitfields") == 0) {
			CGOptions::bitfields(false);
			continue;
		}

		if (strcmp (argv[i], "--prefix-name") == 0) {
			CGOptions::prefix_name(true);
			continue;
		}

		if (strcmp (argv[i], "--sequence-name-prefix") == 0) {
			CGOptions::sequence_name_prefix(true);
			continue;
		}

		if (strcmp (argv[i], "--compatible-check") == 0) {
			CGOptions::compatible_check(true);
			continue;
		}

		if (strcmp (argv[i], "--partial-expand") == 0) {
			string s;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], s)) {
				cout << "--partial-expand needs options" << std::endl;
				exit(-1);
			}
			CGOptions::partial_expand(s);
			continue;
		}

		if (strcmp (argv[i], "--paranoid") == 0) {
			CGOptions::paranoid(true);
			continue;
		}
		if (strcmp (argv[i], "--no-paranoid") == 0) {
			CGOptions::paranoid(false);
			continue;
		}

		if (strcmp (argv[i], "--quiet") == 0) {
			CGOptions::quiet(true);
			continue;
		}

		if (strcmp (argv[i], "--main") == 0) {
			CGOptions::nomain(false);
			continue;
		}

		if (strcmp (argv[i], "--nomain") == 0) {
			CGOptions::nomain(true);
			continue;
		}

		if (strcmp (argv[i], "--compound-assignment") == 0) {
			CGOptions::compound_assignment(true);
			continue;
		}

		if (strcmp (argv[i], "--no-compound-assignment") == 0) {
			CGOptions::compound_assignment(false);
			continue;
		}

		if (strcmp (argv[i], "--structs") == 0) {
			CGOptions::use_struct(true);
			continue;
		}

		if (strcmp (argv[i], "--no-structs") == 0) {
			CGOptions::use_struct(false);
			continue;
		}

		if (strcmp (argv[i], "--unions") == 0) {
			CGOptions::use_union(true);
			continue;
		}

		if (strcmp (argv[i], "--no-unions") == 0) {
			CGOptions::use_union(false);
			continue;
		}

		if (strcmp (argv[i], "--argc") == 0) {
			CGOptions::accept_argc(true);
			continue;
		}

		if (strcmp (argv[i], "--no-argc") == 0) {
			CGOptions::accept_argc(false);
			continue;
		}

		if (strcmp (argv[i], "--expand-struct") == 0) {
			CGOptions::expand_struct(true);
			continue;
		}

		if (strcmp (argv[i], "--fixed-struct-fields") == 0) {
			CGOptions::fixed_struct_fields(true);
			continue;
		}

		if (strcmp (argv[i], "--max-struct-fields") ==0 ) {
			unsigned long ret;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &ret))
				exit(-1);
			CGOptions::max_struct_fields(ret);
			continue;
		}

		if (strcmp (argv[i], "--max-union-fields") ==0 ) {
			unsigned long ret;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &ret))
				exit(-1);
			CGOptions::max_union_fields(ret);
			continue;
		}

		if (strcmp (argv[i], "--max-nested-struct-level") ==0 ) {
			unsigned long ret;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &ret))
				exit(-1);
			CGOptions::max_nested_struct_level(ret);
			continue;
		}

		if (strcmp (argv[i], "--struct-output") == 0) {
			string s;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], s))
				exit(-1);
			CGOptions::struct_output(s);
			continue;
		}

		if (strcmp (argv[i], "--dfs-debug-sequence") == 0) {
			string s;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], s))
				exit(-1);
			CGOptions::dfs_debug_sequence(s);
			continue;
		}

		if (strcmp (argv[i], "--max-exhaustive-depth") ==0 ) {
			unsigned long ret;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &ret))
				exit(-1);
			CGOptions::max_exhaustive_depth(ret);
			continue;
		}

		if (strcmp (argv[i], "--max-pointer-depth") ==0 ) {
			unsigned long ret;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &ret))
				exit(-1);
			CGOptions::max_indirect_level(ret);
			continue;
		}

		if (strcmp (argv[i], "--output") == 0 ||
			strcmp (argv[i], "-o") == 0) {
			string o_file;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], o_file))
				exit(-1);
			CGOptions::output_file(o_file);
			continue;
		}

		if (strcmp (argv[i], "--delta-monitor") == 0) {
			string monitor;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], monitor)) {
				cout<< "please specify one delta monitor!" << std::endl;
				exit(-1);
			}
			CGOptions::delta_monitor(monitor);
			continue;
		}

		if (strcmp (argv[i], "--delta-output") == 0) {
			string o_file;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], o_file)) {
				cout<< "please specify delta output file!" << std::endl;
				exit(-1);
			}
			CGOptions::delta_output(o_file);
			continue;
		}

		if (strcmp (argv[i], "--go-delta") == 0) {
			string monitor;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], monitor)) {
				cout<< "please specify one delta type!" << std::endl;
				exit(-1);
			}
			CGOptions::go_delta(monitor);
			continue;
		}

		if (strcmp (argv[i], "--no-delta-reduction") == 0) {
			CGOptions::no_delta_reduction(true);
			continue;
		}

		if (strcmp (argv[i], "--math-notmp") == 0) {
			CGOptions::math_notmp(true);
			continue;
		}

		if (strcmp (argv[i], "--math64") == 0) {
			CGOptions::math64(true);
			continue;
		}

		if (strcmp (argv[i], "--no-math64") == 0) {
			CGOptions::math64(false);
			continue;
		}

		if (strcmp (argv[i], "--inline-function") == 0) {
			CGOptions::inline_function(true);
			continue;
		}

		if (strcmp (argv[i], "--no-inline-function") == 0) {
			CGOptions::inline_function(false);
			continue;
		}

		if (strcmp (argv[i], "--longlong") == 0) {
			CGOptions::longlong(true);
			continue;
		}

		if (strcmp (argv[i], "--no-longlong") == 0) {
			CGOptions::longlong(false);
			continue;
		}

		if (strcmp (argv[i], "--int8") == 0) {
			CGOptions::int8(true);
			continue;
		}

		if (strcmp (argv[i], "--no-int8") == 0) {
			CGOptions::int8(false);
			continue;
		}

		if (strcmp (argv[i], "--uint8") == 0) {
			CGOptions::uint8(true);
			continue;
		}

		if (strcmp (argv[i], "--no-uint8") == 0) {
			CGOptions::uint8(false);
			continue;
		}

		if (strcmp (argv[i], "--float") == 0) {
			CGOptions::enable_float(true);
			continue;
		}

		if (strcmp (argv[i], "--no-float") == 0) {
			CGOptions::enable_float(false);
			continue;
		}

		if (strcmp (argv[i], "--strict-float") == 0) {
			CGOptions::strict_float(true);
			continue;
		}

		if (strcmp (argv[i], "--pointers") == 0) {
			CGOptions::pointers(true);
			continue;
		}

		if (strcmp (argv[i], "--no-pointers") == 0) {
			CGOptions::pointers(false);
			continue;
		}

		if (strcmp (argv[i], "--max-array-dim") ==0 ) {
			unsigned long dim;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &dim))
				exit(-1);
			CGOptions::max_array_dimensions(dim);
			continue;
		}

		if (strcmp (argv[i], "--max-array-len-per-dim") ==0 ) {
			unsigned long length;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &length))
				exit(-1);
			CGOptions::max_array_length_per_dimension(length);
			continue;
		}

		if (strcmp (argv[i], "--arrays") == 0) {
			CGOptions::arrays(true);
			continue;
		}

		if (strcmp (argv[i], "--no-arrays") == 0) {
			CGOptions::arrays(false);
			continue;
		}

		if (strcmp (argv[i], "--strict-const-arrays") == 0) {
			CGOptions::strict_const_arrays(true);
			continue;
		}

		if (strcmp (argv[i], "--jumps") == 0) {
			CGOptions::jumps(true);
			continue;
		}

		if (strcmp (argv[i], "--no-jumps") == 0) {
			CGOptions::jumps(false);
			continue;
		}

		if (strcmp (argv[i], "--return-structs") == 0) {
			CGOptions::return_structs(true);
			continue;
		}

		if (strcmp (argv[i], "--no-return-structs") == 0) {
			CGOptions::return_structs(false);
			continue;
		}

		if (strcmp (argv[i], "--arg-structs") == 0) {
			CGOptions::arg_structs(true);
			continue;
		}

		if (strcmp (argv[i], "--no-arg-structs") == 0) {
			CGOptions::arg_structs(false);
			continue;
		}

		if (strcmp (argv[i], "--return-unions") == 0) {
			CGOptions::return_unions(true);
			continue;
		}

		if (strcmp (argv[i], "--no-return-unions") == 0) {
			CGOptions::return_unions(false);
			continue;
		}

		if (strcmp (argv[i], "--arg-unions") == 0) {
			CGOptions::arg_unions(true);
			continue;
		}

		if (strcmp (argv[i], "--no-arg-unions") == 0) {
			CGOptions::arg_unions(false);
			continue;
		}

		if (strcmp (argv[i], "--volatiles") == 0) {
			CGOptions::volatiles(true);
			continue;
		}

		if (strcmp (argv[i], "--no-volatiles") == 0) {
			CGOptions::volatiles(false);
			continue;
		}

		if (strcmp (argv[i], "--volatile-pointers") == 0) {
			CGOptions::volatile_pointers(true);
			continue;
		}

		if (strcmp (argv[i], "--no-volatile-pointers") == 0) {
			CGOptions::volatile_pointers(false);
			continue;
		}

		if (strcmp (argv[i], "--const-pointers") == 0) {
			CGOptions::const_pointers(true);
			continue;
		}

		if (strcmp (argv[i], "--no-const-pointers") == 0) {
			CGOptions::const_pointers(false);
			continue;
		}

		if (strcmp (argv[i], "--global-variabless") == 0) {
			CGOptions::global_variables(true);
			continue;
		}

		if (strcmp (argv[i], "--no-global-variables") == 0) {
			CGOptions::global_variables(false);
			continue;
		}

		if (strcmp (argv[i], "--enable-access-once") == 0) {
			CGOptions::access_once(true);
			continue;
		}

		if (strcmp (argv[i], "--strict-volatile-rule") == 0) {
			CGOptions::strict_volatile_rule(true);
			continue;
		}

		if (strcmp (argv[i], "--addr-taken-of-locals") == 0) {
			CGOptions::addr_taken_of_locals(true);
			continue;
		}

		if (strcmp (argv[i], "--no-addr-taken-of-locals") == 0) {
			CGOptions::addr_taken_of_locals(false);
			continue;
		}

		if (strcmp (argv[i], "--fresh-array-ctrl-var-names") == 0) {
			CGOptions::fresh_array_ctrl_var_names(true);
			continue;
		}

		if (strcmp (argv[i], "--consts") == 0) {
			CGOptions::consts(true);
			continue;
		}

		if (strcmp (argv[i], "--no-consts") == 0) {
			CGOptions::consts(false);
			continue;
		}

		if (strcmp (argv[i], "--dangling-global-pointers") == 0) {
			CGOptions::dangling_global_ptrs(true);
			continue;
		}

		if (strcmp (argv[i], "--no-dangling-global-pointers") == 0) {
			CGOptions::dangling_global_ptrs(false);
			continue;
		}

		if (strcmp (argv[i], "--divs") == 0) {
			CGOptions::divs(true);
			continue;
		}

		if (strcmp (argv[i], "--no-divs") == 0) {
			CGOptions::divs(false);
			continue;
		}

		if (strcmp (argv[i], "--muls") == 0) {
			CGOptions::muls(true);
			continue;
		}

		if (strcmp (argv[i], "--no-muls") == 0) {
			CGOptions::muls(false);
			continue;
		}

		if (strcmp (argv[i], "--checksum") == 0) {
			CGOptions::compute_hash(true);
			continue;
		}

		if (strcmp (argv[i], "--no-checksum") == 0) {
			CGOptions::compute_hash(false);
			continue;
		}

		if (strcmp (argv[i], "--builtins") == 0) {
			CGOptions::builtins(true);
			continue;
		}

		if (strcmp (argv[i], "--no-builtins") == 0) {
			CGOptions::builtins(false);
			continue;
		}

		if (strcmp (argv[i], "--random-random") == 0) {
			CGOptions::random_random(true);
			continue;
		}

		if (strcmp (argv[i], "--check-global") == 0) {
			CGOptions::blind_check_global(true);
			continue;
		}

		if (strcmp (argv[i], "--step-hash-by-stmt") == 0) {
			CGOptions::step_hash_by_stmt(true);
			continue;
		}

		if (strcmp (argv[i], "--stop-by-stmt") ==0 ) {
			unsigned long num;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &num))
				exit(-1);
			CGOptions::stop_by_stmt(num);
			continue;
		}

		if (strcmp (argv[i], "--monitor-funcs") == 0) {
			string vname;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], vname)) {
				cout<< "please specify name(s) of the func(s) you want to monitor" << std::endl;
				exit(-1);
			}
			CGOptions::monitored_funcs(vname);
			continue;
		}

		if (strcmp (argv[i], "--delta-input") == 0) {
			string filename;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], filename)) {
				cout<< "please specify delta output file!" << std::endl;
				exit(-1);
			}
			CGOptions::delta_input(filename);
			continue;
		}

		if (strcmp (argv[i], "--dump-default-probabilities") == 0) {
			string filename;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], filename)) {
				cout<< "please pass probability configuration output file!" << std::endl;
				exit(-1);
			}
			CGOptions::dump_default_probabilities(filename);
			continue;
		}

		if (strcmp (argv[i], "--dump-random-probabilities") == 0) {
			string filename;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], filename)) {
				cout<< "please pass probability configuration output file!" << std::endl;
				exit(-1);
			}
			CGOptions::dump_random_probabilities(filename);
			continue;
		}

		if (strcmp (argv[i], "--probability-configuration") == 0) {
			string filename;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], filename)) {
				cout<< "please probability configuration file!" << std::endl;
				exit(-1);
			}
			CGOptions::probability_configuration(filename);
			continue;
		}

		if (strcmp (argv[i], "--const-as-condition") == 0) {
			CGOptions::const_as_condition(true);
			continue;
		}

		if (strcmp (argv[i], "--match-exact-qualifiers") == 0) {
			CGOptions::match_exact_qualifiers(true);
			continue;
		}

		if (strcmp (argv[i], "--no-return-dead-pointer") == 0) {
			CGOptions::no_return_dead_ptr(true);
			continue;
		}

		if (strcmp (argv[i], "--return-dead-pointer") == 0) {
			CGOptions::no_return_dead_ptr(false);
			continue;
		}

		if (strcmp (argv[i], "--concise") == 0) {
			//CGOptions::quiet(true);
			//CGOptions::paranoid(false);
			CGOptions::concise(true);
			continue;
		}

		if (strcmp (argv[i], "--identify-wrappers") == 0) {
			CGOptions::identify_wrappers(true);
			continue;
		}

		if (strcmp (argv[i], "--safe-math-wrappers") == 0) {
			string ids;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], ids)) {
				cout<< "please specify safe math wrappers in the form of id1,id2..." << std::endl;
				exit(-1);
			}
			CGOptions::safe_math_wrapper(ids);
			continue;
		}

		if (strcmp (argv[i], "--mark-mutable-const") == 0) {
			CGOptions::mark_mutable_const(true);
			continue;
		}

		if (strcmp (argv[i], "--force-globals-static") == 0) {
			CGOptions::force_globals_static(true);
			continue;
		}

		if (strcmp (argv[i], "--force-non-uniform-arrays") == 0) {
			CGOptions::force_non_uniform_array_init(true);
			continue;
		}

		if (strcmp (argv[i], "--no-force-non-uniform-arrays") == 0) {
			CGOptions::force_non_uniform_array_init(false);
			continue;
		}

		if (strcmp (argv[i], "--inline-function-prob") == 0 ) {
			unsigned long prob;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &prob))
				exit(-1);
			CGOptions::inline_function_prob(prob);
			continue;
		}

		if (strcmp (argv[i], "--builtin-function-prob") == 0 ) {
			unsigned long prob;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &prob))
				exit(-1);
			CGOptions::builtin_function_prob(prob);
			continue;
		}

		if (strcmp (argv[i], "--enable-builtin-kinds") == 0) {
			string kinds;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], kinds)) {
				cout<< "please specify enabled builtin kinds in the form of k1,k2..." << std::endl;
				exit(-1);
			}
			CGOptions::enable_builtin_kinds(kinds);
			continue;
		}

		if (strcmp (argv[i], "--disable-builtin-kinds") == 0) {
			string kinds;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], kinds)) {
				cout<< "please specify disabled builtin kinds in the form of k1,k2..." << std::endl;
				exit(-1);
			}
			CGOptions::disable_builtin_kinds(kinds);
			continue;
		}

		if (strcmp (argv[i], "--null-ptr-deref-prob") == 0 ) {
			unsigned long prob;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &prob))
				exit(-1);
			CGOptions::null_pointer_dereference_prob(prob);
			continue;
		}

		if (strcmp (argv[i], "--dangling-ptr-deref-prob") == 0 ) {
			unsigned long prob;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &prob))
				exit(-1);
			CGOptions::dead_pointer_dereference_prob(prob);
			continue;
		}

		if (strcmp (argv[i], "--max-expr-complexity") == 0 ) {
			unsigned long comp;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &comp))
				exit(-1);
			CGOptions::max_expr_depth(comp);
			continue;
		}

		if (strcmp (argv[i], "--max-block-depth") == 0 ) {
			unsigned long depth;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &depth))
				exit(-1);
			CGOptions::max_blk_depth(depth);
			continue;
		}

		if (strcmp (argv[i], "--max-struct-nested-level") == 0 ) {
			unsigned long depth;
			i++;
			arg_check(argc, i);
			if (!parse_int_arg(argv[i], &depth))
				exit(-1);
			CGOptions::max_nested_struct_level(depth);
			continue;
		}

		if (strcmp (argv[i], "--union-read-type-sensitive") == 0) {
			CGOptions::union_read_type_sensitive(true);
			continue;
		}

		if (strcmp (argv[i], "--no-union-read-type-sensitive") == 0) {
			CGOptions::union_read_type_sensitive(false);
			continue;
		}

		if (strcmp (argv[i], "--pre-incr-operator") == 0) {
			CGOptions::pre_incr_operator(true);
			continue;
		}

		if (strcmp (argv[i], "--no-pre-incr-operator") == 0) {
			CGOptions::pre_incr_operator(false);
			continue;
		}

		if (strcmp (argv[i], "--pre-decr-operator") == 0) {
			CGOptions::pre_decr_operator(true);
			continue;
		}

		if (strcmp (argv[i], "--no-pre-decr-operator") == 0) {
			CGOptions::pre_decr_operator(false);
			continue;
		}

		if (strcmp (argv[i], "--post-incr-operator") == 0) {
			CGOptions::post_incr_operator(true);
			continue;
		}

		if (strcmp (argv[i], "--no-post-incr-operator") == 0) {
			CGOptions::post_incr_operator(false);
			continue;
		}

		if (strcmp (argv[i], "--post-decr-operator") == 0) {
			CGOptions::post_decr_operator(true);
			continue;
		}

		if (strcmp (argv[i], "--no-post-decr-operator") == 0) {
			CGOptions::post_decr_operator(false);
			continue;
		}

		if (strcmp (argv[i], "--unary-plus-operator") == 0) {
			CGOptions::unary_plus_operator(true);
			continue;
		}

		if (strcmp (argv[i], "--no-unary-plus-operator") == 0) {
			CGOptions::unary_plus_operator(false);
			continue;
		}

		if (strcmp (argv[i], "--embedded-assigns") == 0) {
			CGOptions::use_embedded_assigns(true);
			continue;
		}

        if (strcmp (argv[i], "--no-safe-math") == 0){
            CGOptions::avoid_signed_overflow(false);
            continue;
        }

        if (strcmp (argv[i], "--safe-math") == 0){
            CGOptions::avoid_signed_overflow(true);
            continue;
        }

		if (strcmp (argv[i], "--no-embedded-assigns") == 0) {
			CGOptions::use_embedded_assigns(false);
			continue;
		}

		if (strcmp (argv[i], "--comma-operators") == 0) {
			CGOptions::use_comma_exprs(true);
			continue;
		}

		if (strcmp (argv[i], "--no-comma-operators") == 0) {
			CGOptions::use_comma_exprs(false);
			continue;
		}

		if (strcmp (argv[i], "--take-no-union-field-addr") == 0) {
			CGOptions::take_union_field_addr(false);
			continue;
		}

		if (strcmp (argv[i], "--take-union-field-addr") == 0) {
			CGOptions::take_union_field_addr(true);
			continue;
		}

		if (strcmp (argv[i], "--vol-struct-union-fields") == 0) {
			CGOptions::vol_struct_union_fields(true);
			continue;
		}

		if (strcmp (argv[i], "--no-vol-struct-union-fields") == 0) {
			CGOptions::vol_struct_union_fields(false);
			continue;
		}

		if (strcmp(argv[i], "--const-struct-union-fields") == 0) {
			CGOptions::const_struct_union_fields(true);
			continue;
		}

		if (strcmp(argv[i], "--no-const-struct-union-fields") == 0) {
			CGOptions::const_struct_union_fields(false);
			continue;
		}

		if (strcmp (argv[i], "--no-hash-value-printf") == 0) {
			CGOptions::hash_value_printf(false);
			continue;
		}

		if (strcmp (argv[i], "--no-signed-char-index") == 0) {
			CGOptions::signed_char_index(false);
			continue;
		}

		if (strcmp (argv[i], "--lang-cpp") == 0) {
			CGOptions::lang_cpp(true);
			continue;
		}

		if (strcmp(argv[i], "--cpp11") == 0) {
			CGOptions::cpp11(true);
			continue;
		}

		if (strcmp (argv[i], "--reduce") == 0) {
			string filename;
			i++;
			arg_check(argc, i);
			if (!parse_string_arg(argv[i], filename)) {
				cout<< "please specify reduction directive file!" << std::endl;
				exit(-1);
			}
			ifstream conf(filename.c_str());
			if (conf.fail()) {
				cout<< "can't read reduction directive file " << filename << "!" << std::endl;
				exit(-1);
			}
			CGOptions::init_reducer(filename);
			continue;
		}

    if (strcmp(argv[i], "--fast-execution") == 0) {
      CGOptions::lang_cpp(true);
      // jumps can easily cause infinite loops. Just disable them
      CGOptions::jumps(false);
      // large arrays are also reported to cause slow execution
      CGOptions::max_array_length_per_dimension(5);
      continue;
    }
		// OMIT help

		// OMIT compute-hash

		// OMIT depth-protect

		// OMIT wrap-volatiles

		// OMIT allow-const-volatile

		// OMIT allow-int64

		// OMIT avoid-signed-overflow

		// FIXME-- we should parse all options and then this should be
		// an error

		cout << "invalid option " << argv[i] << " at: "
			 << i
			 << endl;
		exit(-1);
	}

	if (CGOptions::lang_cpp()) {
		CGOptions::fix_options_for_cpp();
	}

	if (CGOptions::has_conflict()) {
		cout << "error: options conflict - " << CGOptions::conflict_msg() << std::endl;
		exit(-1);
	}

	AbsProgramGenerator *generator = AbsProgramGenerator::CreateInstance(argc, argv, g_Seed);
	if (!generator) {
		cout << "error: can't create generator!" << std::endl;
		exit(-1);
	}
	generator->goGenerator();
	delete generator;

//	file.close();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

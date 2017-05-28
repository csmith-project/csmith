// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2015, 2017 The University of Utah
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

#include "ReducerOutputMgr.h"

#include <cassert>
#include <sstream>
#include "Common.h"
#include "CGOptions.h"
#include "platform.h"
#include "Bookkeeper.h"
#include "Function.h"
#include "FunctionInvocation.h"
#include "FunctionInvocationUser.h"
#include "ExpressionFuncall.h"
#include "ExpressionVariable.h"
#include "CGContext.h"
#include "ArrayVariable.h"
#include "VariableSelector.h"
#include "Type.h"
#include "random.h"
#include "DeltaMonitor.h"
#include "Error.h"
#include "Reducer.h"
#include "Block.h"
#include "Statement.h"
#include "StatementIf.h"
#include "StatementFor.h"
#include "StatementArrayOp.h"
#include "StatementGoto.h"
#include "StringUtils.h"
#include "FactMgr.h"

using namespace std;

#define BINARY_REDUCTION_LIMIT 18

struct binary_reduced_stm {
	string cmd;
	string stm;
	int len;
};

std::ostream &
ReducerOutputMgr::get_main_out()
{
	if (ofile_) return *ofile_;
	if (!CGOptions::output_file().empty()) {
		ofile_ = new ofstream(CGOptions::output_file().c_str());
		return *ofile_;
	}
	else {
		return std::cout;
	}
}

ReducerOutputMgr::ReducerOutputMgr()
: ofile_(NULL)
{
	reducer = CGOptions::get_reducer();
	assert(reducer);
}

ReducerOutputMgr::~ReducerOutputMgr()
{
	if (ofile_) {
		ofile_->close();
		delete ofile_;
	}
}

void
ReducerOutputMgr::OutputHeader(int argc, char *argv[], unsigned long /*seed*/)
{
	// output shortened header: csmith options + random_inc.h include
	ostream& out = get_main_out();
	out << "// Options:  ";
	if (argc <= 1) {
		out << " (none)";
	} else {
		for (int i = 1; i < argc; ++i) {
			out << " " << argv[i];
		}
	}
	out << endl;
	out << "#include \"csmith.h\"" << endl << endl;
}

void
ReducerOutputMgr::output_var(const Variable* v, std::ostream &out, int indent)
{
	if (!reducer->is_var_init_reduced(v)) {
		v->OutputDef(out, indent);
	}
	else {
		string init = reducer->map_reduced_var_inits[v];
		output_tab(out, indent);
		v->OutputDecl(out);
		out << " = " << init << ";" << endl;
	}
}

void
ReducerOutputMgr::output_vars(const vector<Variable*> &vars, std::ostream &out, int indent)
{
	size_t i;
	int dimen = 0;
	vector<const ArrayVariable*> avs;
	// print used vars, and find the max dimension of all array variables
	for (i=0; i<vars.size(); i++) {
		const Variable* v = vars[i];
		if (!reducer->is_var_used(v)) continue;
		output_var((const ArrayVariable*)v, out, indent);
		if (v->isArray) {
			const ArrayVariable* av = (const ArrayVariable*)(v);
			if (!av->no_loop_initializer()) {
				if (av->get_dimension() > (size_t)dimen) {
					dimen = av->get_dimension();
				}
				avs.push_back(av);
			}
		}
	}
	// output array initializers
	if (dimen > 0) {
		vector <const Variable*> &ctrl_vars = Variable::get_new_ctrl_vars();
		OutputArrayCtrlVars(ctrl_vars, out, dimen, indent);
		for (i=0; i<avs.size(); i++) {
			const ArrayVariable* av = avs[i];
			av->output_init(out, av->init, ctrl_vars, indent);
		}
	}
}

int
ReducerOutputMgr::output_block(const Block* blk, std::ostream& out, int indent, bool no_bracelet)
{
	size_t i;
	if (reducer->is_blk_deleted(blk)) {
		output_tab(out, indent + 1);
		out << ";" << endl;
		return 0;
	}
	// see Reducer::config_diff_active_blks
	if (reducer->replaced_stms.find(blk) != reducer->replaced_stms.end() && reducer->replaced_stms[blk] == NULL) {
		output_tab(out, indent + 1);
		out << reducer->map_pre_stm_assigns[blk] << endl;
		return 0;
	}

	if (!no_bracelet) {
		output_open_encloser("{", out, indent);
	}
	// add support for "--math-notmp"
	if (CGOptions::math_notmp()) {
		blk->OutputTmpVariableList(out, indent);
	}
	output_vars(blk->local_vars, out, indent);

	// dump global state for top level block if necessary
	if (blk->parent == NULL) {
		output_global_state_for_func(blk->func, out, indent);
	}
	// dump "entering block ..." information
	output_block_entry_msg(blk, out, indent);
	if (reducer->dump_stms_in_blocks == blk->func) {
		string fname = blk->func->name;
		if (fname == "func_1") fname = "main";
		string s = "// " + fname + " block " + StringUtils::int2str(blk->stm_id) + " [";
		for (i=0; i<blk->stms.size(); i++) {
			if (i > 0) {
				s += ":";
			}
			s += StringUtils::int2str(blk->stms[i]->stm_id);
		}
		s += "]";
		output_tab(out, indent);
		out << s << endl;
	}

	FactMgr* fm = get_fact_mgr_for_func(blk->func);
	for (i=0; i<blk->stms.size(); i++) {
		const Statement* stm = blk->stms[i];
		output_stm(stm, out, fm, indent);
	}

	if (!no_bracelet) {
		output_close_encloser("}", out, indent, true);
	}
	outputln(out);
	return 0;
}

int
ReducerOutputMgr::output_func_header(const Function* f, std::ostream& out)
{
	// output function header
	out << "static ";
	f->rv->qfer.output_qualified_type(f->return_type, out);
	out << " " << f->name << "(";
	size_t i;
	bool first = true;
	for (i=0; i<f->param.size(); i++) {
		const Variable* var = f->param[i];
		if (!reducer->is_param_dropped(f, i)) {
			if (!first) {
				out << ", ";
			}
			var->output_qualified_type(out);
			out << " " << var->name;
			first = false;
		}
	}
	out << ")";
	return 0;
}

void
ReducerOutputMgr::output_crc_lines(std::ostream& out)
{
	// declare loop variables if they are used in crc lines
	for (char c = 'i'; c <= 'z'; c++) {
		string pattern = string("; ") + c + "++)";
		if (reducer->crc_lines.find(pattern) != string::npos) {
			output_tab(out, 1);
			out << "int " << c << " = 0;" << endl;
		}
	}
	if (reducer->crc_lines.find("print_hash_value") != string::npos) {
		output_tab(out, 1);
		out << "int print_hash_value = 0;"  << endl;
	}
	// print the real CRC lines
	output_tab(out, 1);
	out << reducer->crc_lines << endl;
}

int
ReducerOutputMgr::output_main_func(std::ostream& out)
{
	size_t i;
	const Function* f = reducer->main;
	if (f->param.size() == 0 && (reducer->main_str == "" || reducer->main_str.find("func_")==0)) {
		out << "int  main(void)" << endl;
		if (reducer->is_blk_deleted(f->body)) {
			out << "{" << endl;
			if (!reducer->crc_lines.empty()) {
				output_crc_lines(out);
			}
			output_tab(out, 1);
			out << "return 0;" << endl;
			out << "}" << endl;
		}
		else {
			output_block(f->body, out, 0);
		}
	}
	else {
		output_func(f, out);
		out << endl << "int  main(void) {" << endl;
		output_tab(out, 1);
		// break up function call and parameters, and skip parameters that are reduced
		vector<string> strs;
		StringUtils::split_string(reducer->main_str, strs, "();");
		assert(strs.size() == 2 || strs.size() == 1);
		string func_name = strs[0];
		out << func_name << "(";
		if (strs.size() == 2) {
			const Function* f = find_function_by_name(func_name);
			assert(f);
			string params = strs[1];
			strs.clear();
			StringUtils::split_string(params, strs, ",");
			bool first = true;
			for (i=0; i<strs.size(); i++) {
				if (!reducer->is_param_dropped(f, i)) {
					if (!first) {
						out << ", ";
					}
					out << strs[i];
					first = false;
				}
			}
		}
		out << ");" << endl;
		output_tab(out, 1);
		out << "return 0;" << endl << "}";
		outputln(out);
	}
	return 0;
}
int
ReducerOutputMgr::output_func(const Function* f, std::ostream& out)
{
	output_func_header(f, out);
	out << endl;
	output_block(f->body, out, 0);
	return 0;
}

void
ReducerOutputMgr::output_pre_stm_assigns(const Statement* stm, std::ostream &out, int indent)
{
	if (reducer->map_pre_stm_assigns.find(stm) != reducer->map_pre_stm_assigns.end()) {
		string assigns = reducer->map_pre_stm_assigns[stm];
		output_tab(out, indent);
		out << assigns;
		outputln(out);
	}
}

void
ReducerOutputMgr::output_block_entry_msg(const Block* blk, std::ostream &out, int indent)
{
	if (reducer->dump_block_entry) {
		const Statement* s = blk->find_container_stm();
		if (s && s->eType == eFor) {
			output_tab(out, indent);
			out << "static int cnt = 0;" << endl;
			output_tab(out, indent++);
			out << "if (cnt++ < 1000) {" << endl;
		}
		string msg = "entering " + blk->func->name + "@" + StringUtils::int2str(blk->stm_id) + "\\n";
		output_print_str(out, msg, "", indent);
		outputln(out);
		if (s && s->eType == eFor) {
			output_tab(out, --indent);
			out << "}" << endl;
		}
	}
}

void
ReducerOutputMgr::output_pre_stm_values(const Statement* stm, std::ostream &out, FactMgr* fm, int indent)
{
	if (find_stm_in_set(reducer->dump_value_before, stm) != -1) {
		assert(stm->parent);
		string blkid = StringUtils::int2str(stm->parent->stm_id);
		string id = StringUtils::int2str(stm->stm_id);
		out << "/* replacing " << blkid << " " << id << " before"  << endl;
		output_write_var_values("values before " + id + "...\\n", stm, out, fm, indent, true);
		out << "*/" << endl;
	}
}

void
ReducerOutputMgr::output_post_stm_values(const Statement* stm, std::ostream &out, FactMgr* fm, int indent)
{
	// print value of variables that may have been written by the statement
	if (find_stm_in_set(reducer->dump_value_after, stm) != -1) {
		assert(stm->parent);
		string blkid = StringUtils::int2str(stm->parent->stm_id);
		string id = StringUtils::int2str(stm->stm_id);
		out << "/* replacing " << blkid << " " << id << " after" << endl;
		output_write_var_values("values after " + id + "...\\n", stm, out, fm, indent, true);
		output_memory_addrs(stm, out, indent);
		out << "*/" << endl;
	}
}

/*
 * compute the real "meaningful" length of statement or expression
 * anything not affecting the program complexity is ignored
 */
int real_length(string exp)
{
	int len = 0;
	size_t i;
	for (i=1; i<exp.length(); i++) {
		if (exp[i-1] == '0' && exp[i] == 'x') {
			len -= 2;
		}
		if (exp[i] == 'L') {
			len -= 1;
		}
		if (exp[i-1] == '-' && exp[i] >= '0' && exp[i] <= '9') {
			len -= 1;
		}
		if (((exp[i-1] >= '0' && exp[i-1] <= '9') || (exp[i-1] >= 'A' && exp[i-1] <= 'F')) &&
			((exp[i] >= '0' && exp[i] <= '9') || (exp[i] >= 'A' && exp[i] <= 'F'))) {
			len -= 1;
		}
		len++;
	}
	return len;
}

/*
 * insert an output for a  binary reduced statement into the ordered array
 */
void insert_alt_stm_cmd(vector<struct binary_reduced_stm>& stms, const string& cmd, const string& stm)
{
	int len = real_length(stm);
	struct binary_reduced_stm alt = {cmd, stm, len};
	for (size_t i=0; i<stms.size(); i++) {
		if (stm == stms[i].stm) return;
		if (len < stms[i].len) {
			stms.insert(stms.begin() + i, alt);
			return;
		}
	}
	stms.push_back(alt);
}

/*
 * limit the number of binary operations by removing some simple ones
 */
void
ReducerOutputMgr::limit_binarys(vector<const FunctionInvocationBinary*>& binarys, vector<int>& ids)
{
	int s;
	while (binarys.size() > BINARY_REDUCTION_LIMIT) {
		for (s = binarys.size() - 1; s>=0; s--) {
			const FunctionInvocationBinary* fib = binarys[s];
			vector<const FunctionInvocationBinary*> dummy1;
			vector<int> dummy2;
			if (reducer->find_binary_operations(fib->param_value[0], dummy1, dummy2, true) == 0 &&
				reducer->find_binary_operations(fib->param_value[1], dummy1, dummy2, true) == 0) {
				binarys.erase(binarys.begin() + s);
				ids.erase(ids.begin() + s);
				break;
			}
		}
	}
	// if still more than limit, delete the last few binary operations
	if (binarys.size() > BINARY_REDUCTION_LIMIT) {
		int extra = binarys.size() - BINARY_REDUCTION_LIMIT;
		for (s=0; s<extra; s++) {
			binarys.pop_back();
			ids.pop_back();
		}
	}
}

/*******************************************************************************
 * output statement with "all" possible binary reductions.
 * note this probably takes a long time to compute, therefore we need to limit
 * the number of binary operations we could reduce (default 18)
 *******************************************************************************/
void
ReducerOutputMgr::output_alt_exprs(const Statement* stm, std::ostream &out, int indent)
{
	size_t i, j, k, len;
	if (!reducer->reduce_binaries || stm->get_direct_invocation() == NULL) return;
	vector<const FunctionInvocationBinary*> binarys;
	vector<int> ids;
	reducer->find_binary_operations(stm, binarys, ids, true);
	assert(ids.size() == binarys.size());

	// reduce runtime by limiting the number of binary operations we try to reduce
	limit_binarys(binarys, ids);
	vector<intvec> combinations;
	vector<intvec> left_trees, right_trees;

	reducer->build_left_right_binary_trees(binarys, left_trees, right_trees);
	assert(binarys.size() == left_trees.size() && binarys.size() == right_trees.size());

	// special case: output "if (1)" for if conditions
	if (stm->eType == eIfElse) {
		output_tab(out, indent);
		out << "// [" << StringUtils::int2str(stm->stm_id) << ":0:-1] " << "if (1)" << endl;
	}
	// output the selections at single points
	for (i=0; i<binarys.size(); i++) {
		const FunctionInvocation* fi = binarys[i];
		for (j=0; j<2; j++) {
			output_tab(out, indent);
			out << "// [" << StringUtils::int2str(stm->stm_id);
			const Expression* op = fi->param_value[j];
			reducer->map_reduced_invocations[fi] = op;
			out << ":" + StringUtils::int2str(ids[i]) << ":" << StringUtils::int2str(j+1) << "] ";
			stm->eType == eIfElse ? ((const StatementIf*)stm)->output_condition(out, 0, 0) : stm->Output(out, 0, 0);
			reducer->map_reduced_invocations.erase(fi);
		}
	}
	if (binarys.size()) {
		output_tab(out, indent);
		out << "// end of single choices" << endl;
	}
	intvec orig;
	for (i=0; i<binarys.size(); i++) {
		orig.push_back(0);
	}
	combinations.push_back(orig);
	for (i=0; i<binarys.size(); i++) {
		len = combinations.size();
		for (j=0; j<len; j++) {
			const intvec& curr = combinations[j];
			if (curr[i] == 0) {
				// make choice, select left, fork a new combination
				intvec combination = curr;
				combination[i] = 1;
				intvec& skipped = right_trees[i];
				for (k=0; k<skipped.size(); k++) {
					combination[skipped[k]] = 3;
				}
				combinations.push_back(combination);
				//combinations.push_back(combination);
				// make choice, select left, fork a new combination
				combination = combinations[j];
				combination[i] = 2;
				skipped = left_trees[i];
				for (k=0; k<skipped.size(); k++) {
					combination[skipped[k]] = 3;
				}
				combinations.push_back(combination);
			}
		}
	}

	vector<struct binary_reduced_stm> alt_stms;
	for (i=1; i<combinations.size(); i++) {
		intvec& tmp = combinations[i];
		assert(tmp.size() == binarys.size());
		string cmd = "// [" + StringUtils::int2str(stm->stm_id);
		vector<const FunctionInvocation*> reduced_invokes;
		for (j=0; j<binarys.size(); j++) {
			const FunctionInvocation* fi = binarys[j];
			if (tmp[j] == 1 || tmp[j] == 2) {
				int choice = tmp[j];
				const Expression* op = binarys[j]->param_value[choice - 1];
				reducer->map_reduced_invocations[fi] = op;
				cmd += ":" + StringUtils::int2str(ids[j]) + ":" + StringUtils::int2str(choice);
				reduced_invokes.push_back(fi);
			}
		}
		cmd += "]";
		ostringstream oss;
		stm->eType == eIfElse ? ((const StatementIf*)stm)->output_condition(oss, 0, 0) : stm->Output(oss, 0, 0);
		insert_alt_stm_cmd(alt_stms, cmd, oss.str());
		// impose an artificial upper limit on number of alternatives so reducer can finish quicker
		if (alt_stms.size() >= 10000) {
			break;
		}

		for (j=0; j<reduced_invokes.size(); j++) {
			reducer->map_reduced_invocations.erase(reduced_invokes[j]);
		}
	}
	for (i=0; i<alt_stms.size(); i++) {
		output_tab(out, indent);
		out << alt_stms[i].cmd << " " << alt_stms[i].stm;
	}
}

void
ReducerOutputMgr::output_if_stm(const StatementIf* si, std::ostream &out, int indent)
{
	if (reducer->is_blk_deleted(si->get_true_branch()) && reducer->is_blk_deleted(si->get_false_branch())) {
		assert(0); // should have been taken care in get_used_vars_and_funcs_and_labels
	}
	else if (reducer->is_blk_deleted(si->get_false_branch())) {
		if (reducer->replaced_stms.find(si) != reducer->replaced_stms.end() && reducer->replaced_stms[si] == si->get_true_branch()) {
			output_block(si->get_true_branch(), out, indent, true);
		} else {
			if (reducer->output_if_ids) {
				out << "/* if reduction candidate: " << StringUtils::int2str(si->stm_id) << " */" << endl;
			}
			si->output_condition(out, 0, indent);
			output_block(si->get_true_branch(), out, indent);
		}
	}
	else if (reducer->is_blk_deleted(si->get_true_branch())) {
		if (reducer->replaced_stms.find(si) != reducer->replaced_stms.end() && reducer->replaced_stms[si] == si->get_false_branch()) {
			output_block(si->get_false_branch(), out, indent, true);
		} else {
			if (reducer->output_if_ids) {
				out << "/* if reduction candidate: " << StringUtils::int2str(si->stm_id) << " */" << endl;
			}
			// special case: if condition is replaced, don't flip it by outputting "!"
			if (reducer->is_exp_replaced(si->get_test())) {
				si->output_condition(out, 0, indent);
			}
			else {
				output_tab(out, indent);
				out << "if (!(";
				si->get_test()->Output(out);
				out << "))" << endl;
			}
			output_block(si->get_false_branch(), out, indent);
		}
	}
	else {
		si->output_condition(out, 0, indent);
		output_block(si->get_true_branch(), out, indent);
		output_tab(out, indent);
		out << "else" << endl;
		output_block(si->get_false_branch(), out, indent);
	}
 }

void
ReducerOutputMgr::output_reduced_stm(const Statement* stm, std::ostream &out, int indent)
{
	vector<const Block*> blks;
	stm->get_blocks(blks);
	if (blks.empty()) {
		// insert printing value for focus variable
		if (stm->eType == eReturn) {
			// output value(s) of monitor variable(s) before return
			if (reducer->dump_monitored_var && stm->func->feffect.is_written_partially(reducer->monitored_var)) {
				output_tab(out, indent);
				string vname = reducer->monitored_var->name;
				out <<"printf(\"   " << vname << " = %d\", " << vname <<");";
				outputln(out);
				output_tab(out, indent);
				out << "printf(\" before leaving " << stm->func->name << ":%d\\n\", call_id);";
				outputln(out);
			}
			// output value of key variable for the main function before return
			else if (stm->func == reducer->main) {
				if (reducer->monitored_var) {
					ostringstream oss;
					const Variable* key = reducer->monitored_var;
					oss << "checksum " << key->get_actual_name() << " = ";
					key->output_runtime_value(out, oss.str(), "\\n", indent, 0);
					outputln(out);
				}
			}
			if (stm->func == GetFirstFunction()) {
				// output crc lines if they are required
				if (!reducer->crc_lines.empty()) {
					output_crc_lines(out);
				}
				output_tab(out, indent);
				out << "return 0;" << endl;
				return;
			}
		}
		else if (stm->eType == eGoto && reducer->dump_block_entry) {
			const StatementGoto* sg = (const StatementGoto*)stm;
			output_tab(out, indent);
			out << "if (";
			sg->test.Output(out);
			out << ")" << endl;
			output_open_encloser("{", out, indent);
			output_block_entry_msg(sg->dest->parent, out, indent);
			output_tab(out, indent);
			out << "goto " << sg->label << ";" << endl;
			output_close_encloser("}", out, indent);
			return;
		}
		stm->Output(out, 0, indent);
		return;
	}

	switch (stm->eType) {
		case eIfElse:
			output_if_stm((const StatementIf*)stm, out, indent);
			break;
		case eFor: {
			const StatementFor* sf = (const StatementFor*)stm;
			if (reducer->is_blk_deleted(sf->get_body())) {
				bool keep_body = false;
				const Variable* cv = sf->get_init()->get_lhs()->get_var();
				if (cv == reducer->monitored_var) {
					vector<string> labels, req_labels;
					if (sf->get_body()->find_contained_labels(labels)) {
						size_t i;
						for (i=0; i<labels.size(); i++) {
							if (reducer->is_label_used(labels[i])) {
								req_labels.push_back(labels[i]);
							}
						}
						if (req_labels.size() > 0) {
							sf->output_header(out, indent);
							for (i=0; i<req_labels.size()-1; i++) {
								out << req_labels[i] << ":" << endl;
							}
							out << req_labels[req_labels.size() -1] << ": ;" << endl;
							keep_body = true;
						}
					}
				}
				if (!keep_body) {
					sf->get_init()->Output(out, 0, indent);
				}
			}
			else {
				sf->output_header(out, indent);
				output_block(sf->get_body(), out, indent);
			}
			break;
		}
		case eArrayOp: {
			const StatementArrayOp* sa = (const StatementArrayOp*)stm;
			if (reducer->is_blk_deleted(sa->body)) {
				break;
			}
			sa->output_header(out, indent);
			output_block(sa->body, out, indent);
			break;
		}
		default: break;
	}
}
void
ReducerOutputMgr::output_stm(const Statement* stm, std::ostream &out, FactMgr* fm, int indent)
{
	string str_out;
	// print pre-statement assignments
	output_pre_stm_assigns(stm, out, indent);
	output_pre_stm_values(stm, out, fm, indent);

	if (reducer->replaced_stms.find(stm) != reducer->replaced_stms.end()) {
		const Statement* alt_stm = reducer->replaced_stms[stm];
		vector<string> labels;
		// special case for goto target: if the jump source is still around, we have to keep the label somehow
		if (reducer->find_missing_labels(stm, alt_stm, labels)) {
			for (size_t i=0; i<labels.size(); i++) {
				out << labels[i] << ": ;" << endl;
			}
		}
		if (alt_stm && alt_stm->eType == eBlock) {
			output_block((const Block*)alt_stm, out, indent, true);
		} else if (alt_stm) {
			output_stm(reducer->replaced_stms[stm], out, fm, indent);
		}
		return;
	}

	if (stm->func == reducer->rewrite_calls_inside) {
		rewrite_func_calls(stm, out, indent);
	}
	// print lable for jump detination
	string label = reducer->find_jump_label(stm);
	label.empty() ? (out << "") : (out << label << ":" << endl);

	output_alt_exprs(stm, out, indent);

	output_reduced_stm(stm, out, indent);

	output_post_stm_values(stm, out, fm, indent);
}

void
ReducerOutputMgr::rewrite_func_call(const Statement* stm, const FunctionInvocation* invoke, string tmp_id, std::ostream& out, int indent)
{
	// output pre-values
	FactMgr* fm = get_fact_mgr_for_func(stm->func);
	output_write_var_values("begin of values before rewrite...\\n", stm, out, fm, indent);
	// output "<type> t1 = <call>"
	const Type* type = &(invoke->get_type());;
	Expression* tmp_call = new ExpressionFuncall(*(invoke->clone()));
	CVQualifiers qfer = CVQualifiers::random_qualifiers(type, 0, 0);
	Variable* tmp_var = Variable::CreateVariable(tmp_id, type, tmp_call, &qfer);
	tmp_var->OutputDef(out, indent);

	// output the value of the tmp variable
	ostringstream oss;
	oss << "<" << tmp_id << " = ";
	tmp_var->output_runtime_value(out, oss.str(), ">\\n", indent);
	outputln(out);

	// output post values
	output_write_var_values("begin of values after rewrite...\\n", stm, out, fm, indent);
	if (reducer->is_ptr_written_in_stm(stm)) {
		output_memory_addrs(stm, out, indent);
	}
	ExpressionVariable* tmp_ev = new ExpressionVariable(*tmp_var);
	reducer->map_reduced_invocations[invoke] = tmp_ev;
	// output stm with call replaced with tmp_id
	(stm->eType == eIfElse) ? ((const StatementIf*)stm)->output_condition(out, 0, indent) : stm->Output(out, 0, indent);

	// clean up
	reducer->map_reduced_invocations.erase(invoke);
	delete tmp_ev;
	delete tmp_var;
}

int
ReducerOutputMgr::rewrite_func_calls(const Statement* stm, std::ostream &out, int indent)
{
	vector<const FunctionInvocationUser*> calls;
	const FunctionInvocation* fi = stm->get_direct_invocation();
	if (fi) {
		vector<string> ids;
		string init_name = "t" + StringUtils::int2str(stm->stm_id);
		reducer->find_called_funcs(fi, init_name, calls, ids);
		if (!calls.empty()) {
			size_t i;
			for (i=0; i<calls.size(); i++) {
				// exclude calls that return struct/union for now because struct const is not accepted as parameter value
				if (calls[i]->get_type().is_aggregate()) {
					continue;
				}
				output_tab(out, indent);
				out << "/* " << "replacing " << ids[i] << endl;
				rewrite_func_call(stm, calls[i], ids[i], out, indent);
				output_tab(out, indent);
				out << "*/" << endl;
			}
		}
	}
	return calls.size();
}

/*
 * dumping global states at the function entry point for path shortcutting purpose
 */
void
ReducerOutputMgr::output_global_state_for_func(const Function* f, std::ostream &out, int indent)
{
	if (!reducer->dump_monitored_var) return;
	size_t i;
	if (reducer->monitored_func && f == reducer->main) {
		output_global_values("values before main\\n", out, indent);
	}
	// output statement to increment call counter for delta purpose
	if (f->feffect.is_written(reducer->monitored_var)) {
		output_tab(out, indent);
		out <<"int call_id = ++global_call_id;";
		outputln(out);

		// output global state if we are interested in this particular function call
		if (f == reducer->monitored_func) {
			assert(reducer->monitored_call_id != "");
			output_tab(out, indent);
			out << "if (call_id == " << reducer->monitored_call_id << ")" << endl;
			output_open_encloser("{", out, indent);

			// dump addresses of global variables
			output_memory_addrs(f->body, out, indent);

			// dump global state at the beginning of this call
			output_global_values("values after main and before " + f->name + "\\n", out, indent);

			// output parameter values for this call
			output_print_str(out, "\\n", "", indent);
			output_print_str(out, f->name + "(", "", indent);
			bool first = true;
			for (i=0; i<f->param.size(); i++) {
				const Variable* v = f->param[i];
				if (reducer->is_var_used(v)) {
					if (!first) {
						output_print_str(out, ", ", "", 0);
					}
					v->output_runtime_value(out, "", "", 0);
					first = false;
				}
			}
			output_print_str(out, ");\\n", "", indent);
			output_close_encloser("}", out, indent);
			outputln(out);
		}
	}
}

void
ReducerOutputMgr::output_write_var_values(string title, const Statement* stm, std::ostream &out, FactMgr* fm, int indent, bool cover_block_writes)
{
	output_print_str(out, title, "", indent);
	outputln(out);
	size_t i;
	const Statement* s = stm;
	if (cover_block_writes) {
		s = stm->parent;
		assert(s);
	}
	const vector<const Variable *>& write_vars = fm->map_stm_effect[s].get_write_vars();
	for (i=0; i<write_vars.size(); i++) {
		const Variable* wvar = write_vars[i];
		if (wvar->is_visible(stm->parent) && reducer->is_var_used(wvar)) {
			// output printf statements to record the type of the variable
			ostringstream oss;
			//wvar->OutputDecl(oss);
			string dimen = StringUtils::int2str(wvar->get_dimension());
			oss << "<" << wvar->get_actual_name() << "(" << dimen << ")" << " = ";
			wvar->output_runtime_value(out, oss.str(), ">\\n", indent);
			outputln(out);
		}
	}
	output_print_str(out, "end of values\\n", "", indent);
	outputln(out);
}

void
ReducerOutputMgr::output_memory_addrs(const Statement* stm, std::ostream& out, int indent)
{
	size_t i;
	vector<Variable*> all_vars = VariableSelector::find_all_visible_vars(stm->parent);
	//combine_variable_sets(beffect.get_read_vars(), beffect.get_write_vars(), all_vars);
	//remove_field_vars(all_vars);
	output_print_str(out, "begin memory dump \\n", "", indent);
	outputln(out);
	for (i=0; i<all_vars.size(); i++) {
		const Variable* v = all_vars[i];
		if (reducer->is_var_used(v)) {
			v->output_addressable_name(out, indent);
		}
	}
	output_print_str(out, "end memory dump \\n", "", indent);
	outputln(out);
}

void
ReducerOutputMgr::output_global_values(string header, std::ostream& out, int indent)
{
	size_t i;
	// dump values of global variables
	// "begin global vars entering main \\n"
	output_print_str(out, header, "", indent);
	outputln(out);
	for (i=0; i<reducer->used_vars.size(); i++) {
		const Variable* v = reducer->used_vars[i];
		if (v->is_global()) {
			ostringstream oss;
			string dimen = StringUtils::int2str(v->get_dimension());
			oss << "<" << v->get_actual_name() << "(" << dimen << ")" << " = ";
			v->output_runtime_value(out, oss.str(), ">\\n", indent);
			outputln(out);
		}
	}
	output_print_str(out, "end of values\\n", "", indent);
	outputln(out);
}

void
ReducerOutputMgr::OutputStructUnions(ostream& out)
{
	size_t i;
	for (i=0; i<reducer->used_vars.size(); i++) {
		const Type* t = reducer->used_vars[i]->type;
		if (t->is_aggregate()) {
			Type* type = Type::find_type(t);
			assert(type);
			OutputStructUnion(type, out);
		}
	}
}

void
ReducerOutputMgr::output_artificial_globals(ostream& out)
{
	for (size_t i=0; i<reducer->artificial_globals.size(); i++) {
		out << reducer->artificial_globals[i] << ";" << endl;
	}
}

void
ReducerOutputMgr::output_tail(ostream& out)
{
	size_t i;
	if (reducer->dump_block_entry || reducer->dump_all_block_info) {
		out << "// all blocks: ";
		// print from backward so the leaf blocks will be printed last
		for (i=reducer->all_blks.size(); i>0; i--) {
			out << StringUtils::int2str(reducer->all_blks[i-1]) << ", ";
		}
		out << endl;
	}
	if (reducer->drop_params && reducer->dump_dropped_params) {
		out << "// all dropped parameters: ";
		for (i=0; i<reducer->dropped_params.size(); i++) {
			out << reducer->dropped_params[i]->name << ", ";
		}
		out << endl;
	}
}

void
ReducerOutputMgr::Output()
{
	// configure reducer
	reducer->configure();

	// find all the functions and variables that are used after reductions
	const Function* main = reducer->main;
	reducer->get_used_vars_and_funcs_and_labels(main->body, reducer->used_vars, reducer->used_funcs, reducer->used_labels);
	reducer->expand_used_vars();

	std::ostream &out = get_main_out();
	OutputStructUnions(out);
	output_vars(*VariableSelector::GetGlobalVariables(), out, 0);
	output_artificial_globals(out);
	size_t i;
	for (i=0; i<reducer->used_funcs.size(); i++) {
		output_func_header(reducer->used_funcs[i], out);
		out << ";" << endl;
	}
	for (int j=reducer->used_funcs.size(); j>0; j--) {
		const Function* f = reducer->used_funcs[j-1];
		outputln(out);
		output_func(f, out);
		outputln(out);
	}
	outputln(out);
	output_main_func(out);
	output_tail(out);
}


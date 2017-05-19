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

#ifdef WIN32
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif

#include "Reducer.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cassert>
#include "StringUtils.h"
#include "Function.h"
#include "FunctionInvocationUser.h"
#include "FunctionInvocationBinary.h"
#include "Block.h"
#include "Statement.h"
#include "StatementExpr.h"
#include "StatementIf.h"
#include "StatementFor.h"
#include "StatementGoto.h"
#include "StatementArrayOp.h"
#include "StatementReturn.h"
#include "Lhs.h"
#include "Variable.h"
#include "VariableSelector.h"
#include "ArrayVariable.h"
#include "Expression.h"
#include "ExpressionVariable.h"
#include "ExpressionFuncall.h"
#include "ExpressionAssign.h"
#include "ExpressionComma.h"
#include "Constant.h"
#include "CVQualifiers.h"
#include "FactMgr.h"

Reducer::Reducer(string fname)
: dump_block_entry(false),
  dump_all_block_info(false),
  dump_monitored_var(false),
  dump_dropped_params(false),
  drop_params(false),
  rewrite_calls_inside(NULL),
  reduce_binaries(false),
  output_if_ids(false),
  monitored_var(NULL),
  monitored_func(NULL),
  monitored_call_id(""),
  dump_stms_in_blocks(NULL),
  configured(false),
  fname_(fname)
{
	// nothing else to do
}

Reducer::~Reducer(void)
{
}

void
Reducer::configure(void)
{
	ifstream conf(fname_.c_str());
	std::string line;
	// default: use the first function as mian
	main = GetFirstFunction();
	while(!conf.eof()) {
		getline(conf, line);
		if (StringUtils::empty_line(line))
			continue;
		if (dump_dropped_params) {
			// if "dropped parameters" is not the last setting, means we are done with parameter dropping
			dump_dropped_params = false;
		}
		// find the variable that caused diff. checksums
		if (line.find("focus variable") == 0) {
			getline(conf, line);
			StringUtils::chop(line);
			// make sure the focus var is marked as used var
			const Variable* key = VariableSelector::find_var_by_name(line);
			if (key == NULL) {
				// it's possible an array variable that is not specifically itemized in the
				// program that is the focus var. we manually itemize it here
				vector<string> strs;
				StringUtils::split_string(line, strs, "[]");
				const Variable* ary = VariableSelector::find_var_by_name(strs[0]);
				assert(ary && ary->isArray);
				const ArrayVariable* av = (const ArrayVariable*)ary;
				vector<int> indices;
				for (size_t k=0; k<av->get_dimension(); k++) {
					assert(k + 1 < strs.size());
					indices.push_back(StringUtils::str2int(strs[k+1]));
				}
				av->itemize(indices);
			}
			key = VariableSelector::find_var_by_name(line);
			assert(key);
			monitored_var = key;
			used_vars.push_back(key->get_named_var());
		}
		// find the line that crashed a compiler
		else if (line.find("crc lines") == 0) {
			getline(conf, line);
			StringUtils::chop(line);
			// find the global variables in CRC statements, and mark them as used
			vector<string> strs;
			StringUtils::split_string(line, strs, "(),[.");
			for (size_t i=0; i<strs.size(); i++) {
				if (strs[i].find("g_") == 0) {
					const Variable* v = VariableSelector::find_var_by_name(strs[i]);
					assert(v);
					add_variable_to_set(used_vars, v);
				}
			}
			crc_lines = line;
		}
		else if (line.find("keep variable") == 0) {
			getline(conf, line);
			StringUtils::chop(line);
			// make sure the focus var is marked as used var
			vector<string> vnames;
			StringUtils::split_string(line, vnames, ", ");
			for (size_t i=0; i<vnames.size(); i++) {
				const Variable* v = VariableSelector::find_var_by_name(vnames[i]);
				assert(v);
				used_vars.push_back(v);
			}
		}
		else if (line.find("drop parameters") == 0) {
			getline(conf, line);
			StringUtils::chop(line);
			if (line == "all") {
				drop_params = true;
				dump_dropped_params = true;
			}
		}
		// find the active blocks
		else if (line.find("active blocks") == 0) {
			getline(conf, line);
			config_active_blks(line);
		}
		else if (line.find("call chain shortcut") == 0) {
			getline(conf, line);
			config_call_chain_shortcut(line);
		}
		else if (line.find("replace statement") == 0) {
			getline(conf, line);
			config_stm_reduction(line);
		}
		else if (line.find("replace calls") == 0) {
			getline(conf, line);
			config_expr_reduction(line);
		}
		else if (line.find("replace binary") == 0) {
			getline(conf, line);
			config_binary_reduction(line);
		}
		else if (line.find("replace if") == 0) {
			getline(conf, line);
			config_if_reduction(line);
		}
		else if (line.find("replace array initializer") == 0) {
			getline(conf, line);
			config_var_init_reduction(line);
		}
	}
	conf.close();
	configured = true;
}

const Statement*
Reducer::find_stm_by_id(int stm_id)
{
	size_t j;
	map<const Block*, int>::const_iterator iter;
	for(iter = map_active_blks.begin(); iter != map_active_blks.end(); ++iter) {
		const Block* b = iter->first;
		for (j=0; j<b->stms.size(); j++) {
			if (b->stms[j]->stm_id == stm_id) {
				return b->stms[j];
			}
		}
	}
	return NULL;
}

bool
Reducer::is_ptr_written_in_stm(const Statement* stm)
{
	size_t i;
	FactMgr* fm = get_fact_mgr_for_func(stm->func);
	const vector<const Variable *>& write_vars = fm->map_stm_effect[stm].get_write_vars();
	for (i=0; i<write_vars.size(); i++) {
		const Variable* wvar = write_vars[i];
		if (wvar->is_pointer()) {
			const Block* blk = (stm->eType == eBlock) ? (const Block*)stm : stm->parent;
			if (wvar->is_visible(blk) && is_var_used(wvar)) {
				return true;
			}
		}
	}
	return false;
}

bool
Reducer::is_exp_replaced(const Expression* e)
{
	if (e->term_type == eVariable || e->term_type == eLhs) {
		return map_reduced_vars.find(e) != map_reduced_vars.end();
	}
	if (e->term_type == eFunction) {
		const FunctionInvocation* fi = e->get_invoke();
		assert(fi);
		return map_reduced_invocations.find(fi) != map_reduced_invocations.end();
	}
	return false;
}

bool
Reducer::is_var_init_reduced(const Variable* v)
{
	return map_reduced_var_inits.find(v) != map_reduced_var_inits.end();
}

bool
Reducer::is_label_used(const string l)
{
	if (l == "") return false;
	return std::find(used_labels.begin(), used_labels.end(), l) != used_labels.end();
}

string
Reducer::find_jump_label(const Statement* stm)
{
	string label = stm->find_jump_label();
	return (is_label_used(label)) ? label : "";
}

int
Reducer::find_required_labels(const Statement* stm, vector<string>& labels)
{
	string label = find_jump_label(stm);
	if (label != "") {
		labels.push_back(label);
	}

	vector<const Block*> blks;
	size_t i, j;
	stm->get_blocks(blks);
	for (i=0; i<blks.size(); i++) {
		if (!is_blk_deleted(blks[i])) {
			for (j=0; j<blks[i]->stms.size(); j++) {
				const Statement* s = blks[i]->stms[j];
				find_required_labels(s, labels);
			}
		}
	}
	return labels.size();
}

int
Reducer::find_missing_labels(const Statement* stm, const Statement* alt_stm, vector<string>& labels)
{
	labels.clear();
	vector<string> labels1, labels2;
	stm->find_contained_labels(labels1);
	if (alt_stm) {
		alt_stm->find_contained_labels(labels2);
	}
	size_t i;
	for (i=0; i<labels1.size(); i++) {
		string l = labels1[i];
		if (is_label_used(l)) {
			if (std::find(labels2.begin(), labels2.end(), l) == labels2.end()) {
				labels.push_back(l);
			}
		}
	}
	return labels.size();
}

void
Reducer::output_block_skeleton(const Block* blk, vector<const Block*>& work_blks, vector<const Function*>& work_funcs, std::ostream& out)
{
	size_t i, j;
	vector<const Block*> blks;
	vector<const FunctionInvocationUser*> funcalls;
	vector<const Function*> uniq_funcs;
	out << "****** block: " << blk->stm_id << " ******" << endl;
	for (i=0; i<blk->stms.size(); i++) {
		const Statement* stm = blk->stms[i];
		funcalls.clear();
		uniq_funcs.clear();
		stm->get_called_funcs(funcalls);
		stm->get_blocks(blks);
		out << stm->stm_id;
		if (funcalls.size() > 0) {
			// find the unique functions that was called
			for (j=0; j<funcalls.size(); j++) {
				const Function* f = funcalls[j]->get_func();
				if (find_function_in_set(uniq_funcs, f) == -1) {
					uniq_funcs.push_back(f);
				}
			}
			// print the unique functions, add them to work list if necessary
			out << "(";
			for (j=0; j<uniq_funcs.size(); j++) {
				if (j > 0) {
					out << ", ";
				}
				const Function* f = uniq_funcs[j];
				if (find_function_in_set(work_funcs, f) == -1) {
					work_funcs.push_back(f);
				}
				out << f->name;
			}
			out << ")";
		}
		if (blks.size() > 0) {
			out << "{";
			for (j=0; j<blks.size(); j++) {
				if (j > 0) {
					out << ", ";
				}
				work_blks.push_back(blks[j]);
				out << blks[j]->stm_id;
			}
			out << "}";
		}
		out << endl;
	}
}

bool
Reducer::is_blk_deleted(const Block* b) const
{
	// if active_blks is not initialized, we assume all functions are to be kept
	if (map_active_blks.size() == 0) return false;
	return map_active_blks.find(b) == map_active_blks.end();
}

bool
Reducer::is_stm_deleted(const Statement* stm) const
{
	const Statement* s = stm;
	while (s && replaced_stms.find(s) == replaced_stms.end() && !is_blk_deleted(s->parent)) {
		s = s->find_container_stm();
	}
	// s = NULL means the statement is not reduced
	return !(s == NULL);
}

bool
Reducer::is_param_dropped(const Function* f, int i)
{
	// assume all parameters are not dropped during configuration
	if (!drop_params || !configured) return false;
	return !is_var_used(f->param[i]);
}

void
Reducer::replace_stm(const Statement* stm, const Statement* new_stm, string pre_stm)
{
	replaced_stms[stm] = new_stm;
	if (!pre_stm.empty()) {
		map_pre_stm_assigns[stm] = pre_stm;
		map_str_effects[pre_stm] = find_used_vars(pre_stm);
	}
}

void
Reducer::delete_stms_after(const Statement* stm, bool include_parent_blks)
{
	const Block* parent = stm->parent;
	size_t i, j;
	bool begin_delete = false;
	const Statement* return_stm = stm->func->body->get_last_stm();
	assert(return_stm);
	for (i=0; i<parent->stms.size(); i++) {
		const Statement* s = parent->stms[i];
		vector<string> labels;

		if (begin_delete) {
			if (find_required_labels(s, labels)==0 && s != return_stm) {
				// don't delete statements that contains add-back blocks (those has 0 count in map_active_blks)
				vector<const Block*> blks;
				s->get_blocks(blks);
				bool must_dump = false;
				for (j=0; j<blks.size(); j++) {
					if (!is_blk_deleted(blks[j]) && map_active_blks[blks[j]] == 0) {
						must_dump = true;
						break;
					}
				}
				if (!must_dump) {
					replace_stm(s, NULL, "");
				}
			}
		}
		if (s == stm) {
			begin_delete = true;
		}
	}
	if (include_parent_blks) {
		const Statement* s = stm->find_container_stm();
		if (s) {
			delete_stms_after(s, include_parent_blks);
		}
	}
}

// find local variables that have to be lifted to global, most likely because
// they are used via pointer in a callee function at the bottom of a call chain
int
Reducer::find_local_vars_to_lift(vector<const Variable*>& vars)
{
	size_t i;
	vars.clear();
	if (monitored_func) {
		for (i=0; i<used_vars.size(); i++) {
			const Variable* v = used_vars[i];
			if (monitored_func->feffect.is_read(v) || monitored_func->feffect.is_written(v)) {
				// if there is a variable that is read by the function externally,
				// but is neither a global or a parameter, this must be a local variable
				// of one of the functions in the call chain

				// Note this can not handle parameters not belong to monitored function,
				// so taking address of parameters and passing into a function call is forbidden
				// see ExpressionVariable::make_random
				if (!v->is_global() && !v->is_argument()) {
					vars.push_back(v);
					break;
				}
			}
		}
	}
	return vars.size();
}

void
Reducer::expand_used_vars(void)
{
	size_t i, j;
	for (i=0; i<used_vars.size(); i++) {
		const Variable* v = used_vars[i];
		vector<const Expression*> init_values;
		if (v->isArray) {
			const ArrayVariable* av = dynamic_cast<const ArrayVariable*>(v);
			init_values = av->get_init_values();
		}
		init_values.push_back(v->init);

		for (j=0; j<init_values.size(); j++) {
			const Expression* init = init_values[j];
			if (init && init->term_type == eVariable) {
				const ExpressionVariable* ev = (const ExpressionVariable*)(init);
				if (ev->get_indirect_level() < 0) {
					string dummy;
					const Variable* addr_var = ev->get_var()->get_array(dummy);
					// if not an array ...
					if (addr_var == 0) {
						addr_var = ev->get_var();
					}
					// if addr_var is a field, get the container var
					addr_var = addr_var->get_named_var();
					if (!is_var_used(addr_var)) {
						used_vars.push_back(addr_var);
					}
				}
			}
		}
	}
}

bool
Reducer::is_replaced_var(const ExpressionVariable* ev, string& str_out)
{
	if (map_reduced_vars.find(ev) != map_reduced_vars.end()) {
		str_out = map_reduced_vars[ev];
		return true;
	}
	return false;
}

const Expression*
Reducer::get_replaced_invocation(const FunctionInvocation* fi)
{
	if (map_reduced_invocations.find(fi) != map_reduced_invocations.end()) {
		return map_reduced_invocations[fi];
	}
	return NULL;
}

void
Reducer::get_used_vars_and_funcs_and_labels(const FunctionInvocation* fi, vector<const Variable*>& vars, vector<const Function*>& funcs, vector<string>& labels)
{
	const Expression* replace = get_replaced_invocation(fi);
	bool must_visit = std::find(must_use_var_invocations.begin(), must_use_var_invocations.end(), fi) != must_use_var_invocations.end();
	if (replace != NULL && !must_visit) {
		get_used_vars_and_funcs_and_labels(replace, vars, funcs, labels);
		return;
	}
	size_t i;
	if (fi->invoke_type == eFuncCall) {
		const FunctionInvocationUser* call = (const FunctionInvocationUser*)(fi);
		if (find_function_in_set(funcs, call->get_func()) == -1) {
			// special case: if the body of function is never entered, means this
			// function is never invoked, mostly due to a && or || shortcut, we
			// replace the function call with a constant 0. In fact it's not evaluated
			// anyway, so which constant we use doesn't matter
			if (is_blk_deleted(call->get_func()->body)) {
				if (call->get_type().is_aggregate()) {
					// it's not a real constant, but we don't want to go through the trouble
					// of creating an ExpressionVariable
					string vname = add_artificial_globals(&call->get_type());
					map_reduced_invocations[fi] = new Constant(&call->get_type(), vname);
				}
				else {
					map_reduced_invocations[fi] = new Constant(&fi->get_type(), "0");
				}
				return;
			}
			// recursively find used vars and functions
			const Function* f = call->get_func();

			get_used_vars_and_funcs_and_labels(f->body, vars, funcs, labels);
			funcs.push_back(f);
			// remember the parameters we dropped
			for (i=0; i<f->param.size(); i++) {
				if (is_param_dropped(f, i)) {
					dropped_params.push_back(f->param[i]);
				}
			}
		}
		// only visit the parameters that are not dropped
		for (i=0; i<fi->param_value.size(); i++) {
			if (!is_param_dropped(call->get_func(), i)) {
				get_used_vars_and_funcs_and_labels(fi->param_value[i], vars, funcs, labels);
			}
		}
	}
	else {
		for (i=0; i<fi->param_value.size(); i++) {
			get_used_vars_and_funcs_and_labels(fi->param_value[i], vars, funcs, labels);
		}
	}
}

void
Reducer::get_used_vars_and_funcs_and_labels(const Expression* e, vector<const Variable*>& vars, vector<const Function*>& funcs, vector<string>& labels)
{
	string tmp;
	switch (e->term_type) {
	case eLhs: {
		const Lhs* lhs = (const Lhs*)e;
		add_variable_to_set(vars, lhs->get_var()->get_named_var());
		break;
	}
	case eConstant: {
		const Constant* cst = (const Constant*)e;
		const Variable* v = find_addressed_var(cst->get_value());
		if (v) {
			add_variable_to_set(vars, v);
		}
		break;
	}
	// check if the variable is replaced by a constant
	case eVariable: {
		const ExpressionVariable* ev = (const ExpressionVariable*)e;
		if (!is_replaced_var(ev, tmp) && !ev->get_var()->is_tmp_var()) {
			add_variable_to_set(vars, ev->get_var()->get_named_var());
		}
		break;
	}
	// check if invocation is replaced by a variable or constant
	case eFunction: {
		const ExpressionFuncall* funcall = (const ExpressionFuncall*)e;
		get_used_vars_and_funcs_and_labels(funcall->get_invoke(), vars, funcs, labels);
		break;
	}
	case eAssignment: {
		const ExpressionAssign* ea = (const ExpressionAssign*)e;
		get_used_vars_and_funcs_and_labels(ea->get_lhs(), vars, funcs, labels);
		get_used_vars_and_funcs_and_labels(ea->get_rhs(), vars, funcs, labels);
		break;
	}
	case eCommaExpr: {
		const ExpressionComma* ec = (const ExpressionComma*)e;
		get_used_vars_and_funcs_and_labels(ec->get_lhs(), vars, funcs, labels);
		get_used_vars_and_funcs_and_labels(ec->get_rhs(), vars, funcs, labels);
		break;
	}
	default: break;
	}
}

void
Reducer::find_called_funcs(const FunctionInvocation* fi, string id, vector<const FunctionInvocationUser*>& funcs, vector<string>& ids)
{
	// analyze reduced expression instead if there is one
	if (map_reduced_invocations.find(fi) != map_reduced_invocations.end()) {
		const Expression* e = map_reduced_invocations[fi];
		const FunctionInvocation* invoke = e->get_invoke();
		if (invoke) {
			find_called_funcs(invoke, id, funcs, ids);
		}
		return;
	}

	const FunctionInvocationUser* func_call = NULL;
	if (fi->invoke_type == eFuncCall) {
		func_call = (const FunctionInvocationUser*)fi;
		funcs.push_back(func_call);
		ids.push_back(id);
	}
	// find calls in parameters, skip dropped parameters
	for (size_t i=0; i<fi->param_value.size(); i++) {
		const Expression* value = fi->param_value[i];
		if (func_call==NULL || !is_param_dropped(func_call->get_func(), i)) {
			const FunctionInvocation* fi = value->get_invoke();
			if (fi) {
				find_called_funcs(fi, id + "_" + StringUtils::int2str(i), funcs, ids);
			}
		}
	}
}

int
Reducer::reduce_const_binary_op(const FunctionInvocationBinary* fib)
{
	const Expression* op1 = fib->param_value[0];
	const Expression* op2 = fib->param_value[1];
	if (op1->get_invoke() != NULL && map_reduced_invocations.find(op1->get_invoke()) != map_reduced_invocations.end()) {
		op1 = map_reduced_invocations[op1->get_invoke()];
	}
	if (op2->get_invoke() != NULL && map_reduced_invocations.find(op2->get_invoke()) != map_reduced_invocations.end()) {
		op2 = map_reduced_invocations[op2->get_invoke()];
	}
	if (op1->term_type == eConstant || map_reduced_vars.find(op1) != map_reduced_vars.end()) {
		string str1 = (op1->term_type == eConstant) ? ((const Constant*)op1)->get_value() : map_reduced_vars[op1];
		if (op2->term_type == eConstant || map_reduced_vars.find(op2) != map_reduced_vars.end()) {
			string str2 = (op2->term_type == eConstant) ? ((const Constant*)op2)->get_value() : map_reduced_vars[op2];
			INT64 result = 0;
			INT64 v1 = StringUtils::str2longlong(str1);
			INT64 v2 = StringUtils::str2longlong(str2);
			switch (fib->get_operation()) {
				case eAdd: result = v1 + v2; break;
				case eSub: result = v1 - v2; break;
				case eMul: result = v1 * v2; break;
				case eDiv: result = v2 ? v1 / v2 : v1; break;
				case eMod: result = v2 ? v1 % v2 : v1; break;
				case eCmpGt: result = v1 > v2; break;
				case eCmpLt: result = v1 < v2; break;
				case eCmpGe: result = v1 >= v2; break;
				case eCmpLe: result = v1 <= v2; break;
				case eCmpEq: result = v1 == v2; break;
				case eCmpNe: result = v1 != v2; break;
				case eAnd: result = v1 && v2; break;
				case eOr:  result = v1 || v2; break;
				case eBitXor: result = v1 ^ v2; break;
				case eBitAnd: result = v1 & v2; break;
				case eBitOr:  result = v1 | v2; break;
				case eRShift: result = (v2 > 0) ? v1 >> v2 : v1; break;
				case eLShift: result = (v2 > 0) ? v1 << v2 : v1; break;
			}
			Constant* cst = new Constant(&fib->get_type(), StringUtils::longlong2str(result));
			map_reduced_invocations[fib] = cst;
			return 1;
		}
	}
	return 0;
}

void
Reducer::reduce_const_binary_ops(vector<const FunctionInvocationBinary*>& ops)
{
	size_t i;
	size_t len = ops.size();
	for (i=0; i<len; i++) {
		const FunctionInvocationBinary* fi = ops[i];
		if (reduce_const_binary_op(fi)) {
			ops.erase(ops.begin() + i);
			i--;
			len--;
		}
	}
}

int
Reducer::find_binary_operations(const Statement* stm, vector<const FunctionInvocationBinary*>& ops, vector<int>& ids, bool no_ptr_cmp)
{
	int cnt = 0;
	vector<const Expression*> exprs;
	stm->get_exprs(exprs);
	for (size_t i=0; i<exprs.size(); i++) {
		cnt += find_binary_operations(exprs[i], ops, ids, no_ptr_cmp);
	}
	return cnt;
}

int
Reducer::find_binary_operations(const Expression* exp, vector<const FunctionInvocationBinary*>& ops, vector<int>& ids, bool no_ptr_cmp)
{
	const FunctionInvocation* fi = exp->get_invoke();
	if (fi) {
		// analyze reduced expression instead if there is one
		if (map_reduced_invocations.find(fi) != map_reduced_invocations.end()) {
			return find_binary_operations(map_reduced_invocations[fi], ops, ids, no_ptr_cmp);
		}

		if (fi->invoke_type == eBinaryPrim) {
			const FunctionInvocationBinary* fib = (const FunctionInvocationBinary*)fi;
			if (!reduce_const_binary_op(fib)) {
				if (!no_ptr_cmp || !fib->ptr_cmp) {
					ops.push_back(fib);
					ids.push_back(exp->expr_id);
				}
			}
		}
		const FunctionInvocationUser* func_call = NULL;
		if (fi->invoke_type == eFuncCall) {
			func_call = (const FunctionInvocationUser*)fi;
		}
		// find binary operations in parameters, skip dropped parameters
		for (size_t i=0; i<fi->param_value.size(); i++) {
			const Expression* param = fi->param_value[i];
			if (func_call==NULL || !is_param_dropped(func_call->get_func(), i)) {
				find_binary_operations(param, ops, ids, no_ptr_cmp);
			}
		}
	}
	return ops.size();
}

const FunctionInvocation*
Reducer::find_invoke_by_eid(const Statement* s, int id) const
{
	size_t i;
	vector<const Expression*> exprs;
	s->get_exprs(exprs);
	for (i=0; i<exprs.size(); i++) {
		const FunctionInvocation* fi = find_invoke_by_eid(exprs[i], id);
		if (fi) {
			return fi;
		}
	}
	return NULL;
}

const FunctionInvocation*
Reducer::find_invoke_by_eid(const Expression* e, int id) const
{
	const FunctionInvocation* fi = e->get_invoke();
	if (e->expr_id == id) {
		return fi;
	}
	if (fi) {
		for (size_t i=0; i<fi->param_value.size(); i++) {
			const Expression* param = fi->param_value[i];
			const FunctionInvocation* tmp = find_invoke_by_eid(param, id);
			if (tmp) {
				return tmp;
			}
		}
	}
	return NULL;
}

void
Reducer::build_left_right_binary_trees(vector<const FunctionInvocationBinary*>& ops, vector<intvec>& left_trees, vector<intvec>& right_trees)
{
	size_t i, j, k;
	vector<const FunctionInvocationBinary*> sub_ops;
	vector<int> dummy, covered;
	left_trees.clear();
	right_trees.clear();
	for (i=0; i<ops.size(); i++) {
		const FunctionInvocationBinary* op = ops[i];
		sub_ops.clear();
		covered.clear();
		find_binary_operations(op->param_value[0], sub_ops, dummy, true);
		for (j=0; j<sub_ops.size(); j++) {
			int index = -1;
			for (k=0; k<ops.size(); k++) {
				if (ops[k] == sub_ops[j]) {
					index = k;
					covered.push_back(index);
					break;
				}
			}
		}
		left_trees.push_back(covered);

		sub_ops.clear();
		covered.clear();
		find_binary_operations(op->param_value[1], sub_ops, dummy, true);
		for (j=0; j<sub_ops.size(); j++) {
			int index = -1;
			for (k=0; k<ops.size(); k++) {
				if (ops[k] == sub_ops[j]) {
					index = k;
					covered.push_back(index);
					break;
				}
			}
		}
		right_trees.push_back(covered);
	}
}

/*
 * find all the variables and functions used in this statement
 */
void
Reducer::get_used_vars_and_funcs_and_labels(const Statement* stm, vector<const Variable*>& vars, vector<const Function*>& funcs, vector<string>& labels)
{
	if (stm == NULL) return;
	// for if statements with both branches deleted
	if (stm->eType == eIfElse) {
		const StatementIf* si = (const StatementIf*)stm;
		if (is_blk_deleted(si->get_true_branch()) && is_blk_deleted(si->get_false_branch())) {
			replace_stm(si, NULL, "");
			delete_stms_after(si, false);
		}
	}
	// for goto statement whose target is deleted
	else if (stm->eType == eGoto) {
		const StatementGoto* sg = (const StatementGoto*)stm;
		if (is_blk_deleted(sg->dest->parent)) {
			bool keep = false;
			// special case: if goto jump into a for-loop that use the monitored variable as
			// induction variable, we keep the jump, for seed 355297830
			const Statement* s = sg->dest->find_container_stm();
			if (s && s->eType == eFor) {
				const StatementFor* sf = (const StatementFor*)s;
				if (sf->get_init()->get_lhs()->get_var() == this->monitored_var) {
					keep = true;
					if (std::find(labels.begin(), labels.end(), sg->label) == labels.end()) {
						labels.push_back(sg->label);
					}
				}
			}
			if (!keep) {
				replace_stm(sg, NULL, "");
			}
		} else {
			if (std::find(labels.begin(), labels.end(), sg->label) == labels.end()) {
				labels.push_back(sg->label);
			}
		}
	}
	// include used variables from pre-statement assignments
	if (map_pre_stm_assigns.find(stm) != map_pre_stm_assigns.end()) {
		const string& assigns = map_pre_stm_assigns[stm];
		add_variables_to_set(vars, map_str_effects[assigns]);
	}
	// used replacement for replaced statements
	bool must_visit = std::find(must_use_var_stms.begin(), must_use_var_stms.end(), stm) != must_use_var_stms.end();
	if (!must_visit && replaced_stms.find(stm) != replaced_stms.end()) {
		get_used_vars_and_funcs_and_labels(replaced_stms[stm], vars, funcs, labels);
		return;
	}

	string label = stm->find_jump_label();
	if (!label.empty()) {
		labels.push_back(label);
	}
	size_t i, j;
	//stm->Output(cout);
	vector<const Expression*> exps;
	stm->get_exprs(exps);
	for (i=0; i<exps.size(); i++) {
		get_used_vars_and_funcs_and_labels(exps[i], vars, funcs, labels);
	}
	if (stm->eType == eArrayOp) {
		const StatementArrayOp* sa = (const StatementArrayOp*)stm;
		for (i=0; i<sa->ctrl_vars.size(); i++) {
			add_variable_to_set(vars, sa->ctrl_vars[i]->get_named_var());
		}
		// JYTODO: detect the case when initializer is not invoked
		if (sa->init_value) {
			add_variable_to_set(vars, sa->array_var->get_named_var());
		}
	}
	vector<const Block*> blks;
	stm->get_blocks(blks);
	for (i=0; i<blks.size(); i++) {
		if (is_blk_deleted(blks[i])) {
			continue;
		}
		for (j=0; j<blks[i]->stms.size(); j++) {
			const Statement* s = blks[i]->stms[j];
			get_used_vars_and_funcs_and_labels(s, vars, funcs, labels);
		}
		if (std::find(all_blks.begin(), all_blks.end(), blks[i]->stm_id) == all_blks.end()) {
			all_blks.push_back(blks[i]->stm_id);
		}
	}
}

int
Reducer::configure_diff_active_blks(string line, int first_bid)
{
	vector<int> ids;
	StringUtils::split_int_string(line, ids, ",()");
	size_t i;
	assert(!ids.empty());
	const Block* one_branch = find_block_by_id(first_bid);
	assert(one_branch);
	const Statement* stm = one_branch->find_container_stm();
	if (stm && stm->eType == eIfElse) {
		for (i=0; i<ids.size(); i+=2) {
			int id = ids[i];
			if (1) { //id <= first_bid) {
				int cnt = ids[i+1];
				const Block* blk = find_block_by_id(id);
				assert(blk);
				map_active_blks[blk] = cnt;
			}
		}
		const StatementIf* si = (const StatementIf*)stm;
		const Block* other_branch = (one_branch == si->get_false_branch()) ? si->get_true_branch() : si->get_false_branch();
		map_active_blks[other_branch] = map_active_blks[one_branch];
		assert(!used_vars.empty());

		// replace both branches with direct conflicting assignments
		string vname = monitored_var->name;
		string assign1 = vname + " = 0;";
		string assign2 = vname + " = 1;";
		replace_stm(one_branch, NULL, assign1);
		replace_stm(other_branch, NULL, assign2);

		// delete all statement following this if-else statement
		delete_stms_after(si, true);
		return 1;
	}
	else return 0;
}

void
Reducer::config_active_blks(string cmd)
{
	size_t i;
	vector<string> tmp_strs;
	if (cmd == "poll") {
		dump_block_entry = true;
	} else if (cmd == "blind poll") {
		dump_all_block_info = true;
	}
	else {
		tmp_strs.clear();
		bool take_diff_branch = false;
		if (cmd.find("||") != string::npos) {
			StringUtils::split_string(cmd, tmp_strs, "|");
			assert(tmp_strs.size() == 2);
			cmd = tmp_strs[0];
			int blkid = StringUtils::str2int(tmp_strs[1]);
			take_diff_branch = configure_diff_active_blks(cmd, blkid);
		}
		if (!take_diff_branch) {
			StringUtils::split_string(cmd, tmp_strs, ",()");
			for (i=0; i<tmp_strs.size(); i+=2) {
				const Block* blk = find_block_by_id(StringUtils::str2int(tmp_strs[i]));
				assert(i+1 < tmp_strs.size());
				int cnt = StringUtils::str2int(tmp_strs[i+1]);
				assert(blk);
				map_active_blks[blk] = cnt;
				// for rollback blocks, find the parant blocks and put them on active block list as well
				if (cnt == 0) {
					for (const Block* b = blk->parent; b != NULL; b = b->parent) {
						if (map_active_blks.find(b) != map_active_blks.end()) {
							break;
						}
						map_active_blks[b] = 0;
					}
				}
			}
		}
	}
}

void
Reducer::config_call_chain_shortcut(string cmd)
{
	if (cmd == "poll") {
		dump_monitored_var = true;
		add_artificial_globals(get_int_type(), "global_call_id");
	}
	else {
		vector<string> strs;
		StringUtils::split_string(cmd, strs, ":");
		assert(strs.size() == 2 || strs.size() == 3);
		if (strs.size() == 2) {
			dump_monitored_var = true;
			add_artificial_globals(get_int_type(), "global_call_id");
			monitored_func = find_function_by_name(strs[0]);
			assert(monitored_func);
			monitored_call_id = strs[1];
		}
		else {
			main = find_function_by_name(strs[0]);
			assert(main);
			main_str = strs[2];
		}
	}
}

void
Reducer::config_stm_reduction(string cmd)
{
	size_t i, j;
	if (cmd.find(":") == string::npos) {
		size_t pos = cmd.find(" blind");
		if (pos != string::npos) {
			cmd = cmd.substr(0, pos);
			if (cmd == "main") cmd = "func_1";
			const Function* f = find_function_by_name(cmd);
			assert(f);
			dump_stms_in_blocks = f;
		}
		else {
			// the reducer treats func_1 as main
			if (cmd == "main") cmd = "func_1";
			const Function* f = find_function_by_name(cmd);
			assert(f);
			for (i=0; i<f->blocks.size(); i++) {
				const Block* b = f->blocks[i];
				if (!is_blk_deleted(b) && map_active_blks[b] == 1) {
					for (j=0; j<b->stms.size(); j++) {
						const Statement* s = b->stms[j];
						if (!s->must_jump()) {
							dump_value_before.push_back(s);
							dump_value_after.push_back(s);
						}
					}
				}
			}
		}
	}
	else {
		vector<string> strs;
		StringUtils::split_string(cmd, strs, ":");
		assert(strs.size() == 2 || strs.size() == 3);
		string assigns = strs.size()==3 ? strs[2] : "";
		reduce_stms_with_assigns(StringUtils::str2int(strs[0]), StringUtils::str2int(strs[1]), assigns);
	}
}

void
Reducer::config_expr_reduction(string cmd)
{
	if (cmd.find(":") == string::npos) {
		// the reducer treats func_1 as main
		if (cmd == "main") cmd = "func_1";
		const Function* f = find_function_by_name(cmd);
		assert(f);
		rewrite_calls_inside = f;
	}
	else {
		vector<string> strs;
		StringUtils::split_string(cmd, strs, ":");
		assert(strs.size() == 2 || strs.size() == 3);
		string assigns = strs.size()==3 ? strs[2] : "";
		reduce_call_with_assigns(strs[0], strs[1], assigns);
	}
}

void
Reducer::config_binary_reduction(string cmd)
{
	if (cmd.find(":") == string::npos) {
		// the reducer treats func_1 as main
		reduce_binaries = (cmd == "all");
	}
	else {
		size_t i;
		// for rollback, ignore this reduction if ends with a '-', keep vars as used_vars if ends with '+'
		char last = cmd[cmd.length()-1];
		if (last == '-') return;
		if (last == '+') {
			cmd = cmd.substr(0, cmd.length()-1);
		}
		vector<int> ints;
		StringUtils::split_int_string(cmd, ints, "[:]");
		assert(ints.size() >= 2);
		const Statement* stm = find_stm_by_id(ints[0]);
		assert(stm);
		ints.erase(ints.begin());
		// must be pairs of "<exp_id>:<choice>" where choice is 1 (select left) or 2 (select right)
		assert(ints.size() % 2 == 0);

		for (i = 0; i < ints.size(); i += 2) {
			// special case: reduce if (...) to if (1)
			if (ints[i] == 0) {
				assert(ints[i+1] == -1);
				assert(stm->eType == eIfElse);
				const FunctionInvocation* fi = stm->get_direct_invocation();
				assert(fi);
				map_reduced_invocations[fi] = new Constant(&fi->get_type(), "1");
				break;
			}
			const FunctionInvocation* fi = find_invoke_by_eid(stm, ints[i]);
			assert(fi && fi->invoke_type == eBinaryPrim);
			int choice = ints[i+1] - 1;
			assert(choice == 0 || choice == 1);
			const Expression* op = fi->param_value[choice];
			if (last == '+') {
				must_use_var_invocations.push_back(fi);
			}
			map_reduced_invocations[fi] = op;
		}
	}
}

void
Reducer::config_if_reduction(string cmd)
{
	if (cmd.find(",") == string::npos) {
		// the reducer treats func_1 as main
		output_if_ids = (cmd == "poll");
	}
	else {
		size_t i;
		vector<string> strs;
		StringUtils::split_string(cmd, strs, ",");
		for (i=0; i<strs.size(); i++) {
			string s = strs[i];
			bool rollback = false;
			if (!s.empty() && s[s.length()-1] == '+') {
				s = s.substr(0, s.length() -1);
				rollback = true;
			}
			const Statement* stm = find_stm_by_id(StringUtils::str2int(s));
			assert(stm && stm->eType == eIfElse);
			const StatementIf* si = (const StatementIf*)stm;
			if (is_blk_deleted(si->get_false_branch())) {
				replace_stm(si, si->get_true_branch(), "");
			}
			else if (is_blk_deleted(si->get_true_branch())) {
				replace_stm(si, si->get_false_branch(), "");
			}
			// at least one branch should be deleted
			else assert(0);

			if (rollback) {
				must_use_var_stms.push_back(stm);
			}
		}
	}
}

void
Reducer::config_var_init_reduction(string cmd)
{
	vector<string> strs;
	StringUtils::split_string(cmd, strs, ":");
	assert(strs.size() == 2);

	const Variable* v = VariableSelector::find_var_by_name(strs[0]);
	assert(v);
	map_reduced_var_inits[v] = strs[1];
}

string
Reducer::add_artificial_globals(const Type* t, string name)
{
	static int cnt = 0;
	ostringstream oss;
	t->Output(oss);
	if (name == "") {
		name = "tmp_" + StringUtils::int2str(cnt++);
	}
	artificial_globals.push_back(oss.str() + " " + name);
	return name;
}

const Variable*
Reducer::find_addressed_var(string addr)
{
	if (addr.find("&") == 0) {
		string addr_var = addr.substr(1);
		size_t dotbracket = StringUtils::find_any_char(addr_var, 0, "[.");
		if (dotbracket != string::npos) {
			addr_var = addr_var.substr(0, dotbracket);
		}
		const Variable* addr_v = VariableSelector::find_var_by_name(addr_var);
		assert(addr_v);
		return addr_v;
	}
	return NULL;
}

vector<const Variable*>
Reducer::find_used_vars(string assigns)
{
	size_t i;
	vector<const Variable*> vars;
	vector<string> vnames, values;
	StringUtils::breakup_assigns(assigns, vnames, values);

	// parse the assignment string
	for (i=0; i<vnames.size(); i++) {
		string vname = vnames[i];
		size_t bracket = vname.find("[");
		if (bracket != string::npos) {
			vname = vname.substr(0, bracket);
		}
		const Variable* v = VariableSelector::find_var_by_name(vname);
		assert(v);
		vars.push_back(v);

		// if value is the address of a variable, mark it as used
		string value = values[i];
		v = find_addressed_var(value);
		if (v) {
			vars.push_back(v);
		}
	}
	return vars;
}

bool
Reducer::is_var_used(const Variable* v)
{
	return is_variable_in_set(used_vars, v);
}

/*
 * replace a call in a statement with it's value + optional assignments to compensate for the function call effect
 * for example: a = func_1() + c can be replaced with
 *              global_ptr = &global_v1;
 *              a = 1234 + c;
 * if func_1 returns 1234 and global_ptr is initialized to &global_v1 in func_1
 *
 * special case: if value == delete, the whole statement will be deleted
 */
int
Reducer::reduce_call_with_assigns(const string& tmp_name, const string& value, const string& extra_assigns)
{
	// find statement id and invoke id
	size_t dash = tmp_name.find('_');
	int stm_id = StringUtils::str2int(tmp_name.substr(1, dash-1));

	const Statement* stm = find_stm_by_id(stm_id);
	assert(stm);
	// special case
	if (value == "delete") {
		// when we delete the condition for a if...else..., keep one branch alive
		if (stm->eType == eIfElse) {
			const Block* if_true = ((const StatementIf*)stm)->get_true_branch();
			const Block* if_false = ((const StatementIf*)stm)->get_false_branch();
			const Block* b = is_blk_deleted(if_true) ? if_false : if_true;
			assert(b);
			replace_stm(stm, b, "");
		} else {
			replace_stm(stm, NULL, "");
		}
		return 0;
	}

	// find invocation based on id
	vector<const FunctionInvocationUser*> calls;
	vector<string> ids;
	const FunctionInvocation* invoke = NULL;
	const FunctionInvocation* fi = stm->get_direct_invocation();
	assert(fi);
	find_called_funcs(fi, tmp_name.substr(0, dash), calls, ids);
	size_t i;
	for (i=0; i<ids.size(); i++) {
		if (ids[i] == tmp_name) {
			invoke = calls[i];
			break;
		}
	}
	assert(invoke);

	const Type* type = &(invoke->get_type());
	Constant* cst = new Constant(type, value);
	// record this reduction
	map_reduced_invocations[invoke] = cst; //tmp_ev;
	if (!extra_assigns.empty()) {
		map_pre_stm_assigns[stm] = extra_assigns;
		map_str_effects[extra_assigns] = find_used_vars(extra_assigns);
	}
	return 0;
}

int
Reducer::reduce_stms_with_assigns(int id1, int id2, const string& assigns)
{
	const Statement* stm = find_stm_by_id(id1);
	assert(stm);
	const Block* blk = stm->parent;
	assert(blk);

	size_t i;
	int begin = -1;
	int end = -1;
	for (i=0; i<blk->stms.size(); i++) {
		const Statement* s = blk->stms[i];
		if (s == stm) {
			begin = i;
			end = i;
		}
		if (s->stm_id == id2) {
			assert(begin != -1);
			end = i;
			break;
		}
	}

	if (begin != -1 && end != -1) {
		for (int i=begin+1; i<=end; i++) {
			replace_stm(blk->stms[i], NULL, "");
		}
		replace_stm(blk->stms[begin], NULL, assigns);
	}
	return 0;
}

int
Reducer::output_expr(const Expression* e, std::ostream &out)
{
	string tmp;
	if (e->term_type == eVariable) {
		const ExpressionVariable* ev = (const ExpressionVariable*)e;
		if (is_replaced_var(ev, tmp)) {
			out << tmp;
			return 1;
		}
	}
	// check if invocation is replaced by a variable or constant
	else if (e->term_type == eFunction) {
		const ExpressionFuncall* funcall = (const ExpressionFuncall*)e;
		const FunctionInvocation* invoke = funcall->get_invoke();
		const Expression* exp = get_replaced_invocation(invoke);
		if (exp) {
			exp->Output(out);
			return 1;
		} else {
			if (invoke->invoke_type == eFuncCall) {
				const FunctionInvocationUser* call = (const FunctionInvocationUser*)(invoke);
				const Function* func = call->get_func();
				out << func->name << "(";
				size_t i;
				bool first = true;
				for (i=0; i<func->param.size(); i++) {
					if (!is_param_dropped(func, i)) {
						if (!first) {
							out << ", ";
						}
						call->param_value[i]->Output(out);
						first = false;
					}
				}
				out << ")";
				return 1;
			}
		}
	}
	else if (e->term_type == eAssignment) {
		const ExpressionAssign* ea = dynamic_cast<const ExpressionAssign*>(e);
		ostringstream oss;
		// if RHS can be simplified, print the simplified version
		if (output_expr(ea->get_rhs(), oss)) {
			out << "(";
			ea->get_lhs()->Output(out);
			out << " ";
			ea->get_stm_assign()->output_op(out);
			out << " ";
			out << oss.str();
			out << ")";
			return 1;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.

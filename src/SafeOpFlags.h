// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2017 The University of Utah
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

#ifndef SAFEOPFLAGS_H
#define SAFEOPFLAGS_H

#include <ostream>
#include "FunctionInvocation.h"
#include "Type.h"

enum SafeOpKind {
	sOpUnary,
	sOpBinary,
	sOpAssign,
};

#define MAX_SAFE_OP_KIND ((SafeOpKind) (sOpAssign+1))

enum SafeOpSize {
	sInt8,
	sInt16,
	sInt32,
	sInt64,
	sFloat,
};
#define MAX_SAFE_OP_SIZE ((SafeOpSize) (sFloat+1))

class SafeOpFlags {
public:
	static SafeOpFlags *make_random_binary(const Type *rv_type, const Type *op1_type, const Type *op2_type,
					SafeOpKind op_kind, eBinaryOps op);

	static SafeOpFlags *make_random_unary(const Type *rv_type, const Type *op1_type, eUnaryOps op);

	static SafeOpFlags *make_dummy_flags();

	static eSimpleType flags_to_type(bool sign, enum SafeOpSize size);

	const Type* get_lhs_type(void);
	const Type* get_rhs_type(void);

	SafeOpFlags *clone() const;

	void OutputSize(std::ostream &out) const;

	void OutputFuncOrMacro(std::ostream &out) const;

	void OutputOp1(std::ostream &out) const;

	void OutputOp2(std::ostream &out) const;

	bool get_op1_sign() { return op1_; }

	bool get_op2_sign() { return op2_; }

	enum SafeOpSize get_op_size() const { return op_size_; }

	std::string to_string(enum eBinaryOps op) const;
	std::string to_string(enum eUnaryOps  op) const;
	static int to_id(std::string fname);

	~SafeOpFlags();

	static std::vector<std::string> wrapper_names;
private:
	bool op1_;
	bool op2_;
	bool is_func_;
	SafeOpSize op_size_;

	void OutputSign(std::ostream &out, bool sgnd) const;

	bool static return_float_type(const Type *rv_type, const Type *op1_type, const Type *op2_type,
					eBinaryOps op);

	bool static return_float_type(const Type *rv_type, const Type *op1_type,
					eUnaryOps uop);

	std::string safe_float_func_string(enum eBinaryOps op) const;

	SafeOpFlags();

	SafeOpFlags(const SafeOpFlags &flags);

	SafeOpFlags(bool op1, bool op2, bool is_func, SafeOpSize size);

	SafeOpFlags &operator=(const SafeOpFlags &flags); //unimplemented;
};

#endif //SAFEOPFLAGS_H


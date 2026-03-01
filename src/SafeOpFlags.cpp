// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2014, 2015, 2017 The
// University of Utah All rights reserved.
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

#include <config.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#include "CGOptions.h"
#include "DepthSpec.h"
#include "Error.h"
#include "Probabilities.h"
#include "SafeOpFlags.h"
#include "random.h"

using namespace std;

vector<string> SafeOpFlags::wrapper_names;

SafeOpFlags::SafeOpFlags() {
  // Nothing to do
}

SafeOpFlags::SafeOpFlags(bool o1, bool o2, bool is_func, SafeOpSize osize)
    : op1_(o1), op2_(o2), is_func_(is_func), op_size_(osize) {
  // Nothing to do
}

SafeOpFlags::SafeOpFlags(const SafeOpFlags &flags)
    : op1_(flags.op1_), op2_(flags.op2_), is_func_(flags.is_func_),
      op_size_(flags.op_size_) {}

SafeOpFlags *SafeOpFlags::make_dummy_flags() {
  return new SafeOpFlags(false, false, false, SafeOpSize::sInt8);
}

eSimpleType SafeOpFlags::flags_to_type(bool sign, SafeOpSize size) {
  if (sign) {
    switch (size) {
    case SafeOpSize::sInt8:
      return eSimpleType::eChar;
    case SafeOpSize::sInt16:
      return eSimpleType::eShort;
    case SafeOpSize::sInt32:
      return eSimpleType::eInt;
    case SafeOpSize::sInt64:
      return eSimpleType::eLongLong;
    case SafeOpSize::sFloat:
      return eSimpleType::eFloat;
    default:
      assert(0);
      break;
    }
  } else {
    switch (size) {
    case SafeOpSize::sInt8:
      return eSimpleType::eUChar;
    case SafeOpSize::sInt16:
      return eSimpleType::eUShort;
    case SafeOpSize::sInt32:
      return eSimpleType::eUInt;
    case SafeOpSize::sInt64:
      return eSimpleType::eULongLong;
    default:
      assert(0);
      break;
    }
  }
  assert(0);
  return eSimpleType::eInt;
}

const Type *SafeOpFlags::get_lhs_type(void) {
  eSimpleType st = flags_to_type(op1_, op_size_);
  const Type &t = Type::get_simple_type(st);
  return &t;
}

const Type *SafeOpFlags::get_rhs_type(void) {
  eSimpleType st = flags_to_type(op2_, op_size_);
  const Type &t = Type::get_simple_type(st);
  return &t;
}

bool SafeOpFlags::return_float_type(const Type *rv_type, const Type *op1_type,
                                    const Type *op2_type, eBinaryOps bop) {
  if (!CGOptions::enable_float())
    return false;
  if (rv_type && rv_type->is_float())
    return true;
  if ((op1_type && op1_type->is_float()) || (op2_type && op2_type->is_float()))
    return true;
  if (!FunctionInvocation::BinaryOpWorksForFloat(bop))
    return false;
  return false;
}

bool SafeOpFlags::return_float_type(const Type *rv_type, const Type *op1_type,
                                    eUnaryOps uop) {
  if (!CGOptions::enable_float())
    return false;
  if (rv_type && rv_type->is_float())
    return true;
  if (op1_type && op1_type->is_float())
    return true;
  if (!FunctionInvocation::UnaryOpWorksForFloat(uop))
    return false;
  return false;
}

SafeOpFlags *SafeOpFlags::make_random_unary(const Type *rv_type,
                                            const Type *op1_type,
                                            eUnaryOps uop) {
  SafeOpFlags *flags = new SafeOpFlags();
  assert("new SafeOpFlags fail!");
  bool rv_is_float = return_float_type(rv_type, op1_type, uop);

  // floating point is always signed
  if (rv_is_float) {
    assert(FunctionInvocation::UnaryOpWorksForFloat(uop) && "Invalid unary op");
    flags->op1_ = true;
  } else {
    flags->op1_ = rnd_flipcoin(SafeOpsSignedProb());
  }
  flags->op2_ = flags->op1_;

  // ISSUE: in the old code, is_func is always true
  // Probably need to be fixed later.
  flags->is_func_ = true;
  if (rv_is_float) {
    assert(CGOptions::enable_float());
    flags->op_size_ = SafeOpSize::sFloat;
  } else {
    flags->op_size_ = static_cast<SafeOpSize>(
        rnd_upto(static_cast<unsigned int>(MAX_SAFE_OP_SIZE) - 1,
                 SAFE_OPS_SIZE_PROB_FILTER()));
  }
  return flags;
}

SafeOpFlags *SafeOpFlags::make_random_binary(const Type *rv_type,
                                             const Type *op1_type,
                                             const Type *op2_type,
                                             SafeOpKind op_kind,
                                             eBinaryOps bop) {
  DEPTH_GUARD_BY_TYPE_RETURN_WITH_FLAG(dtSafeOpFlags,
                                       static_cast<int>(op_kind), nullptr);
  SafeOpFlags *flags = new SafeOpFlags();
  assert("new SafeOpFlags fail!");
  bool rv_is_float = return_float_type(rv_type, op1_type, op2_type, bop);

  // floating point is always signed
  if (rv_is_float) {
    if (op_kind == SafeOpKind::sOpBinary) {
      assert(FunctionInvocation::BinaryOpWorksForFloat(bop) &&
             "Invalid binary op");
    }
    flags->op1_ = true;
  } else {
    flags->op1_ = rnd_flipcoin(SafeOpsSignedProb());
  }
  ERROR_GUARD_AND_DEL1(nullptr, flags);

  if (op_kind == SafeOpKind::sOpBinary) {
    if (rv_is_float)
      flags->op2_ = true;
    else
      flags->op2_ = rnd_flipcoin(SafeOpsSignedProb());
    ERROR_GUARD_AND_DEL1(nullptr, flags);
  } else {
    flags->op2_ = flags->op1_;
  }

  // ISSUE: in the old code, is_func is always true
  // Probably need to be fixed later.
  flags->is_func_ = true;

  if (rv_is_float) {
    assert(CGOptions::enable_float());
    flags->op_size_ = SafeOpSize::sFloat;
  } else {
    flags->op_size_ = static_cast<SafeOpSize>(
        rnd_upto(static_cast<unsigned int>(MAX_SAFE_OP_SIZE) - 1,
                 SAFE_OPS_SIZE_PROB_FILTER()));
  }
  return flags;
}

SafeOpFlags *SafeOpFlags::clone() const { return new SafeOpFlags(*this); }

void SafeOpFlags::OutputSize(std::ostream &out) const {
  if (!op1_)
    out << "u";

  switch (op_size_) {
  case SafeOpSize::sInt8:
    out << "int8_t";
    break;
  case SafeOpSize::sInt16:
    out << "int16_t";
    break;
  case SafeOpSize::sInt32:
    out << "int32_t";
    break;
  case SafeOpSize::sInt64:
    out << "int64_t";
    break;
  case SafeOpSize::sFloat:
    out << "float";
    break;
  default:
    assert(!"invalid size!");
    break;
  }
}

void SafeOpFlags::OutputFuncOrMacro(std::ostream &out) const {
  is_func_ ? (out << "func_") : (out << "macro_");
}

void SafeOpFlags::OutputSign(std::ostream &out, bool is_signed) const {
  is_signed ? (out << "_s") : (out << "_u");
}

void SafeOpFlags::OutputOp1(std::ostream &out) const { OutputSign(out, op1_); }

void SafeOpFlags::OutputOp2(std::ostream &out) const { OutputSign(out, op2_); }

SafeOpFlags::~SafeOpFlags() {
  // Nothing to do
}

std::string SafeOpFlags::safe_float_func_string(eBinaryOps op) const {
  string s;
  switch (op) {
  case eBinaryOps::eAdd:
    s = "safe_add_";
    break;
  case eBinaryOps::eSub:
    s = "safe_sub_";
    break;
  case eBinaryOps::eMul:
    s = "safe_mul_";
    break;
  case eBinaryOps::eDiv:
    s = "safe_div_";
    break;
  default:
    assert(0);
    break;
  }
  s += "func_float_f_f";
  return s;
}

/* find the safe math function/macro name */
std::string SafeOpFlags::to_string(eBinaryOps op) const {
  if (op_size_ == SafeOpSize::sFloat)
    return safe_float_func_string(op);
  string s;
  switch (op) {
  case eBinaryOps::eAdd:
    s = "safe_add_";
    break;
  case eBinaryOps::eSub:
    s = "safe_sub_";
    break;
  case eBinaryOps::eMul:
    s = "safe_mul_";
    break;
  case eBinaryOps::eMod:
    s = "safe_mod_";
    break;
  case eBinaryOps::eDiv:
    s = "safe_div_";
    break;
  case eBinaryOps::eLShift:
    s = "safe_lshift_";
    break;
  case eBinaryOps::eRShift:
    s = "safe_rshift_";
    break;
  default:
    break;
  }
  ostringstream oss;
  OutputFuncOrMacro(oss);
  OutputSize(oss);
  OutputOp1(oss);
  (op == eBinaryOps::eLShift || op == eBinaryOps::eRShift) ? OutputOp2(oss) : OutputOp1(oss);
  s += oss.str();
  return s;
}

/* find the safe math function/macro name */
std::string SafeOpFlags::to_string(eUnaryOps op) const {
  assert((op_size_ != SafeOpSize::sFloat) && "No safe unary function on floating point!");
  string s;
  switch (op) {
  case eUnaryOps::eMinus:
    s = "safe_unary_minus_";
    break;
  default:
    break;
  }
  ostringstream oss;
  OutputFuncOrMacro(oss);
  OutputSize(oss);
  OutputOp1(oss);
  s += oss.str();
  return s;
}

/* assign id to safe math function */
int SafeOpFlags::to_id(const std::string &fname) {
  for (size_t i = 0; i < wrapper_names.size(); i++) {
    if (wrapper_names[i] == fname) {
      return i + 1;
    }
  }
  wrapper_names.push_back(fname);
  return wrapper_names.size();
}

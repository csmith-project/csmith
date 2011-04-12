// -*- mode: C++ -*-
//
// Copyright (c) 2008, 2009, 2010, 2011 The University of Utah
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

#ifndef EXTENSION_MGR_H
#define EXTENSION_MGR_H

#include <ostream>

class AbsExtension;
class Function;
class FunctionInvocation;
class CGContext;

class ExtensionMgr {
public:
	static void CreateExtension();

	static void DestroyExtension();

	static AbsExtension *GetExtension();

	static void GenerateFirstParameterList(Function &curFunc);

	static void GenerateValues();

	static FunctionInvocation *MakeFuncInvocation(Function *curFunc, CGContext &cg_context);

	static void OutputTail(std::ostream &out);

	static void OutputHeader(std::ostream &out);

	static void OutputInit(std::ostream &out);

	static void OutputFirstFunInvocation(std::ostream &out, FunctionInvocation *invoke);

private:
	static AbsExtension *extension_;

};

#endif // EXTENSION_MGR_H

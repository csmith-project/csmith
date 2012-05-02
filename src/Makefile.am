## -*- mode: Makefile-Automake -*-
##
## Copyright (c) 2008-2012 The University of Utah
## All rights reserved.
##
## This file is part of `csmith', a random generator of C programs.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
##   * Redistributions of source code must retain the above copyright notice,
##     this list of conditions and the following disclaimer.
##
##   * Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in the
##     documentation and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.

###############################################################################

bin_PROGRAMS = csmith

# XXX --- yuck!
csmith_SOURCES = \
	AbsExtension.cpp \
	AbsExtension.h \
	AbsProgramGenerator.cpp \
	AbsProgramGenerator.h \
	AbsRndNumGenerator.cpp \
	AbsRndNumGenerator.h \
	ArrayVariable.cpp \
	ArrayVariable.h \
	Block.cpp \
	Block.h \
	Bookkeeper.cpp \
	Bookkeeper.h \
	CFGEdge.cpp \
	CFGEdge.h \
	CGContext.cpp \
	CGContext.h \
	CGOptions.cpp \
	CGOptions.h \
	CVQualifiers.cpp \
	CVQualifiers.h \
	Common.h \
	CommonMacros.h \
	CompatibleChecker.cpp \
	CompatibleChecker.h \
	Constant.cpp \
	Constant.h \
	CoverageTestExtension.cpp \
	CoverageTestExtension.h \
	CrestExtension.cpp \
	CrestExtension.h \
	DFSOutputMgr.cpp \
	DFSOutputMgr.h \
	DFSProgramGenerator.cpp \
	DFSProgramGenerator.h \
	DFSRndNumGenerator.cpp \
	DFSRndNumGenerator.h \
	DefaultOutputMgr.cpp \
	DefaultOutputMgr.h \
	DefaultProgramGenerator.cpp \
	DefaultProgramGenerator.h \
	DefaultRndNumGenerator.cpp \
	DefaultRndNumGenerator.h \
	DeltaMonitor.cpp \
	DeltaMonitor.h \
	DepthSpec.cpp \
	DepthSpec.h \
	Effect.cpp \
	Effect.h \
	Enumerator.h \
	Error.cpp \
	Error.h \
	Expression.cpp \
	Expression.h \
	ExpressionAssign.cpp \
	ExpressionAssign.h \
	ExpressionComma.cpp \
	ExpressionComma.h \
	ExpressionFuncall.cpp \
	ExpressionFuncall.h \
	ExpressionVariable.cpp \
	ExpressionVariable.h \
	ExtensionMgr.cpp \
	ExtensionMgr.h \
	ExtensionValue.cpp \
	ExtensionValue.h \
	Fact.cpp \
	Fact.h \
	FactMgr.cpp \
	FactMgr.h \
	FactPointTo.cpp \
	FactPointTo.h \
	FactUnion.cpp \
	FactUnion.h \
	Filter.cpp \
	Filter.h \
	Finalization.cpp \
	Finalization.h \
	Function.cpp \
	Function.h \
	FunctionInvocation.cpp \
	FunctionInvocation.h \
	FunctionInvocationBinary.cpp \
	FunctionInvocationBinary.h \
	FunctionInvocationUnary.cpp \
	FunctionInvocationUnary.h \
	FunctionInvocationUser.cpp \
	FunctionInvocationUser.h \
	KleeExtension.cpp \
	KleeExtension.h \
	Lhs.cpp \
	Lhs.h \
	LinearSequence.cpp \
	LinearSequence.h \
	MspFilters.cpp \
	MspFilters.h \
	OutputMgr.cpp \
	OutputMgr.h \
	PartialExpander.cpp \
	PartialExpander.h \
	Probabilities.cpp \
	Probabilities.h \
	ProbabilityTable.h \
	RandomNumber.cpp \
	RandomNumber.h \
	RandomProgramGenerator.cpp \
	Reducer.cpp \
	Reducer.h \
	ReducerOutputMgr.cpp \
	ReducerOutputMgr.h \
	SafeOpFlags.cpp \
	SafeOpFlags.h \
	Sequence.cpp \
	Sequence.h \
	SequenceFactory.cpp \
	SequenceFactory.h \
	SequenceLineParser.h \
	SimpleDeltaRndNumGenerator.cpp \
	SimpleDeltaRndNumGenerator.h \
	SimpleDeltaSequence.cpp \
	SimpleDeltaSequence.h \
	SplatExtension.cpp \
	SplatExtension.h \
	Statement.cpp \
	Statement.h \
	StatementArrayOp.cpp \
	StatementArrayOp.h \
	StatementAssign.cpp \
	StatementAssign.h \
	StatementBreak.cpp \
	StatementBreak.h \
	StatementContinue.cpp \
	StatementContinue.h \
	StatementExpr.cpp \
	StatementExpr.h \
	StatementFor.cpp \
	StatementFor.h \
	StatementGoto.cpp \
	StatementGoto.h \
	StatementIf.cpp \
	StatementIf.h \
	StatementReturn.cpp \
	StatementReturn.h \
	StringUtils.cpp \
	StringUtils.h \
	Type.cpp \
	Type.h \
	Variable.cpp \
	Variable.h \
	VariableSelector.cpp \
	VariableSelector.h \
	VectorFilter.cpp \
	VectorFilter.h \
	platform.cpp \
	platform.h \
	random.cpp \
	random.h \
	util.cpp \
	util.h

# Include VC project files in the distribution.
EXTRA_DIST = \
	csmith.vcproj \
	csmith.vcxproj

GIT_HASH := $(shell "$(top_srcdir)/git-hash.sh" "$(top_srcdir)" || echo error)
GIT_FLAG = -DGIT_VERSION=\"$(GIT_HASH)\"

## ENE: The idea of stuffing the target CPU name (a string) into a CPP symbol
## is kind of awful.  What we should do instead is use the name to modify the
## include path, e.g., -I"$(target_cpu)".  But that refactoring is for a later
## day...

csmith_CPPFLAGS = \
	-DTARGET_CPU_$(target_cpu)=1 \
	$(GIT_FLAG) \
	$(BOOST_CPPFLAGS)

csmith_LDFLAGS = $(BOOST_LDFLAGS)
csmith_LDADD   = $(BOOST_PROGRAM_OPTIONS_LIB)

###############################################################################

## End of file.

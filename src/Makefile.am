## -*- mode: Makefile-Automake -*-
##
## Copyright (c) 2008-2017 The University of Utah
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

# For source files in `rand48/'.
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = csmith

GIT_HASH := $(shell "$(top_srcdir)/git-hash.sh" "$(top_srcdir)" || echo error)
DUMMY1 := $(shell echo "$(GIT_HASH)" > git_version.txt.tmp)
DUMMY2 := $(shell cmp -s git_version.txt.tmp git_version.txt || \
		  cp git_version.txt.tmp git_version.txt)
DUMMY3 := $(shell rm -f git_version.txt.tmp)

git_version_edit = sed \
	-e 's|@GIT_HASH[@]|$(GIT_HASH)|g'

git_version.cpp: git_version.cpp.in git_version.txt
	rm -f $@ $@.tmp
	srcdir=''; \
	  test -f ./$@.in || srcdir=$(srcdir)/; \
	  $(git_version_edit) $${srcdir}$@.in > $@.tmp
	mv $@.tmp $@

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
	git_version.h \
	platform.cpp \
	platform.h \
	random.cpp \
	random.h \
	util.cpp \
	util.h

nodist_csmith_SOURCES = \
	git_version.cpp

# Windows lacks `lrand48', so we provide an implementation.
if HAVE_LRAND48
else
csmith_SOURCES += \
	rand48/_rand48.c \
	rand48/lrand48.c \
	rand48/rand48.h \
	rand48/srand48.c
endif

EXTRA_DIST = \
	CMakeLists.txt \
	git_version.cpp.in

# csmith_CPPFLAGS =
# csmith_LDFLAGS  =
# csmith_LDADD    =

CLEANFILES = \
	git_version.txt \
	git_version.txt.tmp \
	git_version.cpp \
	git_version.cpp.tmp

###############################################################################

## End of file.

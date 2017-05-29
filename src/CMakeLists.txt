## -*- mode: CMake -*-
##
## Copyright (c) 2017 The University of Utah
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

cmake_minimum_required(VERSION 2.8.12)
project(csmith_src)

set(BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")

###############################################################################

# Generate file "git_version.cpp".
#
configure_file("git_version.cpp.in" "git_version.cpp" @ONLY)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}) # needed for gen'ed .cpp files

set(csmith_SOURCES
  ${CMAKE_BINARY_DIR}/config.h
  AbsExtension.cpp
  AbsExtension.h
  AbsProgramGenerator.cpp
  AbsProgramGenerator.h
  AbsRndNumGenerator.cpp
  AbsRndNumGenerator.h
  ArrayVariable.cpp
  ArrayVariable.h
  Block.cpp
  Block.h
  Bookkeeper.cpp
  Bookkeeper.h
  CFGEdge.cpp
  CFGEdge.h
  CGContext.cpp
  CGContext.h
  CGOptions.cpp
  CGOptions.h
  CVQualifiers.cpp
  CVQualifiers.h
  Common.h
  CommonMacros.h
  CompatibleChecker.cpp
  CompatibleChecker.h
  Constant.cpp
  Constant.h
  CoverageTestExtension.cpp
  CoverageTestExtension.h
  CrestExtension.cpp
  CrestExtension.h
  DFSOutputMgr.cpp
  DFSOutputMgr.h
  DFSProgramGenerator.cpp
  DFSProgramGenerator.h
  DFSRndNumGenerator.cpp
  DFSRndNumGenerator.h
  DefaultOutputMgr.cpp
  DefaultOutputMgr.h
  DefaultProgramGenerator.cpp
  DefaultProgramGenerator.h
  DefaultRndNumGenerator.cpp
  DefaultRndNumGenerator.h
  DeltaMonitor.cpp
  DeltaMonitor.h
  DepthSpec.cpp
  DepthSpec.h
  Effect.cpp
  Effect.h
  Enumerator.h
  Error.cpp
  Error.h
  Expression.cpp
  Expression.h
  ExpressionAssign.cpp
  ExpressionAssign.h
  ExpressionComma.cpp
  ExpressionComma.h
  ExpressionFuncall.cpp
  ExpressionFuncall.h
  ExpressionVariable.cpp
  ExpressionVariable.h
  ExtensionMgr.cpp
  ExtensionMgr.h
  ExtensionValue.cpp
  ExtensionValue.h
  Fact.cpp
  Fact.h
  FactMgr.cpp
  FactMgr.h
  FactPointTo.cpp
  FactPointTo.h
  FactUnion.cpp
  FactUnion.h
  Filter.cpp
  Filter.h
  Finalization.cpp
  Finalization.h
  Function.cpp
  Function.h
  FunctionInvocation.cpp
  FunctionInvocation.h
  FunctionInvocationBinary.cpp
  FunctionInvocationBinary.h
  FunctionInvocationUnary.cpp
  FunctionInvocationUnary.h
  FunctionInvocationUser.cpp
  FunctionInvocationUser.h
  KleeExtension.cpp
  KleeExtension.h
  Lhs.cpp
  Lhs.h
  LinearSequence.cpp
  LinearSequence.h
  MspFilters.cpp
  MspFilters.h
  OutputMgr.cpp
  OutputMgr.h
  PartialExpander.cpp
  PartialExpander.h
  Probabilities.cpp
  Probabilities.h
  ProbabilityTable.h
  RandomNumber.cpp
  RandomNumber.h
  RandomProgramGenerator.cpp
  Reducer.cpp
  Reducer.h
  ReducerOutputMgr.cpp
  ReducerOutputMgr.h
  SafeOpFlags.cpp
  SafeOpFlags.h
  Sequence.cpp
  Sequence.h
  SequenceFactory.cpp
  SequenceFactory.h
  SequenceLineParser.h
  SimpleDeltaRndNumGenerator.cpp
  SimpleDeltaRndNumGenerator.h
  SimpleDeltaSequence.cpp
  SimpleDeltaSequence.h
  SplatExtension.cpp
  SplatExtension.h
  Statement.cpp
  Statement.h
  StatementArrayOp.cpp
  StatementArrayOp.h
  StatementAssign.cpp
  StatementAssign.h
  StatementBreak.cpp
  StatementBreak.h
  StatementContinue.cpp
  StatementContinue.h
  StatementExpr.cpp
  StatementExpr.h
  StatementFor.cpp
  StatementFor.h
  StatementGoto.cpp
  StatementGoto.h
  StatementIf.cpp
  StatementIf.h
  StatementReturn.cpp
  StatementReturn.h
  StringUtils.cpp
  StringUtils.h
  Type.cpp
  Type.h
  Variable.cpp
  Variable.h
  VariableSelector.cpp
  VariableSelector.h
  VectorFilter.cpp
  VectorFilter.h
  git_version.h
  platform.cpp
  platform.h
  random.cpp
  random.h
  util.cpp
  util.h
  ${PROJECT_BINARY_DIR}/git_version.cpp
  )
if(NOT HAVE_LRAND48)
  include_directories(${PROJECT_SOURCE_DIR}/rand48)
  list(APPEND csmith_SOURCES
    rand48/_rand48.c
    rand48/lrand48.c
    rand48/rand48.h
    rand48/srand48.c
    )
endif()

add_executable(csmith
  ${csmith_SOURCES}
  )

if(BSD_LIBRARY)
  target_link_libraries(csmith "${BSD_LIBRARY}")
endif()

install(PROGRAMS
  "${PROJECT_BINARY_DIR}/csmith"
  DESTINATION "${BIN_DIR}"
  )

###############################################################################

## End of file.

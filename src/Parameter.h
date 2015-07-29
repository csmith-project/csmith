// -*- mode: C++ -*-
//
// Copyright (c) 2015-2016 Xuejun Yang
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

#ifndef PARAMETER_H
#define PARAMETER_H

///////////////////////////////////////////////////////////////////////////////
// Class description:
//   This class manages the generation, configuration, outputting of function
//   parameters. It inherits most of the functionality from its base class
//   Variable except for parameter-specific attributes such as in/out.
//////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <sstream>
#include <string>
using namespace std;

#include "Variable.h" 
#include "ParameterBuiltin.h"

enum eParamInOutType {
	eParamNone,
	eParamIn, 
	eParamOut,
	eParamInOut,
    eParamRef
};

class ExtensionValue;

class Parameter : public Variable
{
public:
    static void GenerateParametersFromValues(Function &curFunc, std::vector<ExtensionValue *> &values);
    static void GenerateParametersFromString(Function &currFunc, const string &params_string); 
    static void GenerateParameterFromXmlConfig(Function &currFunc, std::vector<ParameterBuiltin*> params_configs);
    static void GenerateParameterList(Function &curFunc);
    
	// TODO: configure the inout string based on external file (for languages other than C/C++)
    static void ConfigureInOutTypeNames(string noneStr="", string inputStr="in", string outputStr="out", string bothStr="inout", string refStr="&");

    static enum eParamInOutType string_to_inout_type(string s);
    static string inout_type_to_string(enum eParamInOutType inout);

	virtual ~Parameter(void);   
    virtual bool is_param(void) const {return true;}
    eParamInOutType get_param_inout_type(void) const { return _inout; }  
	virtual void Output(std::ostream &) const;    

    bool is_output_param(void) const { return _inout == eParamOut || _inout == eParamInOut || _inout == eParamRef;}

    int imm_bottom;
    int imm_top;
    bool is_imm;

private:
	Parameter(const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer, enum eParamInOutType inout);
    Parameter(const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer, int bottom, int top, enum eParamInOutType inout);
     
    static void GenerateParameter(Function &curFunc);
	static Parameter* GenerateParameter(const Type *type, const CVQualifiers *qfer, enum eParamInOutType inout=eParamNone);
    static Parameter* GenerateParameter(const Type *type, const CVQualifiers *qfer, int bottom, int top, enum eParamInOutType inout=eParamNone);// for the parameter which has [bottom, top] range.

    eParamInOutType _inout;
    static vector<string> _inOutTypeNames;
};

///////////////////////////////////////////////////////////////////////////////

#endif // PARAMETER_H

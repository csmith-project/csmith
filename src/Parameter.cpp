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

#ifdef WIN32
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif
#include "Parameter.h"  
#include "Function.h"
#include "ExtensionValue.h"
#include "VariableSelector.h" 

using namespace std;     
vector<string> Parameter::_inOutTypeNames;

Parameter::Parameter(const std::string &name, const Type *type, const Expression* init, const CVQualifiers* qfer, enum eParamInOutType inoutType)
    : Variable(name, type, init, qfer),
    _inout(inoutType)
{
}
  
Parameter::~Parameter(void)
{
} 

// -----------------------------------------------------------------
// Configure/initialize strings for in/out modes
// -----------------------------------------------------------------
void 
Parameter::ConfigureInOutTypeNames(string noneStr, string inputStr, string outputStr, string bothStr, string refStr)
{
    _inOutTypeNames.push_back(noneStr);
	_inOutTypeNames.push_back(inputStr);
	_inOutTypeNames.push_back(outputStr);
	_inOutTypeNames.push_back(bothStr);
    _inOutTypeNames.push_back(refStr);
}

// -----------------------------------------------------------------
// Convert a string, e.g. "inout", to enum eParamInOutType
// -----------------------------------------------------------------
enum eParamInOutType 
Parameter::string_to_inout_type(string s)
{
    if (_inOutTypeNames.size() == 0)
        ConfigureInOutTypeNames();

    for (size_t i=0; i<_inOutTypeNames.size(); i++) {
        if (_inOutTypeNames[i] == s)
            return (enum eParamInOutType)i;
    }

    // default is none
    return eParamNone;
}
 
// -----------------------------------------------------------------
// Convert eParamInOutType to string
// -----------------------------------------------------------------
string 
Parameter::inout_type_to_string(enum eParamInOutType inoutType)
{
    if (_inOutTypeNames.size() == 0)
        ConfigureInOutTypeNames();

    assert((size_t)inoutType < _inOutTypeNames.size());
    return _inOutTypeNames[inoutType];
}

// -----------------------------------------------------------------
// generate a random parameter name
// -----------------------------------------------------------------
static string
RandomParamName(void)
{
	return gensym("p_");
}

// -----------------------------------------------------------------
// Generate parameters out of type/qualifier of existing values 
// TODO: use ExpressionVariable instead of ExtensionValue
// -----------------------------------------------------------------
void
Parameter::GenerateParametersFromValues(Function &curFunc, std::vector<ExtensionValue *> &values)
{
	vector<ExtensionValue *>::iterator i;
	for (i = values.begin(); i != values.end(); ++i) {
		assert(*i);
		CVQualifiers qfer = (*i)->get_qfer();
		Parameter * p = GenerateParameter((*i)->get_type(), &qfer);
		assert(p);
		curFunc.params.push_back(p);
	}
}


/*
 * generate parameter with given type, qualifier, and inout mode
 */
Parameter *
Parameter::GenerateParameter(const Type *type, const CVQualifiers *qfer, enum eParamInOutType inoutType)
{
    // TODO: support default values for parameters
    Parameter* param = new Parameter(RandomParamName(), type, NULL, qfer, inoutType);
	return param;
}

// --------------------------------------------------------------
// choose a random type for parameter
// --------------------------------------------------------------
void
Parameter::GenerateParameter(Function &curFunc)
{
	// Add this type to our parameter list.
	const Type* t = 0;
	bool rnd = rnd_flipcoin(40);
	if (Type::has_pointer_type() && rnd) {
		t= Type::choose_random_pointer_type();
	}
	else {
		t = Type::choose_random_nonvoid_nonvolatile();
	}
	if (t->eType == eSimple)
		assert(t->simple_type != eVoid);

	CVQualifiers qfer = CVQualifiers::random_qualifiers(t);
	Parameter *param = GenerateParameter(t, &qfer);
	curFunc.params.push_back(param);
}

// -----------------------------------------------------------------
// Determine how many parameters are to be generated
// -----------------------------------------------------------------
static unsigned int
ParamListProbability()
{
	return rnd_upto(CGOptions::max_params());
}

// -----------------------------------------------------------------
// Create parameters out of strings in the format of: 
//   "UInt, UChar" or 
//   "UInt inout, ULong in"
//   ...
// -----------------------------------------------------------------
void
Parameter::GenerateParametersFromString(Function &currFunc, const string &params_string)
{
	vector<string> vs;
	StringUtils::split_string(params_string, vs, ",");

	for (size_t i = 0; i < vs.size(); i++) {
        vector<string> tokens;
        StringUtils::split_string(vs[i], tokens, " ");
        assert(tokens.size() >= 1);

		const Type *ty = Type::get_type_from_string(tokens[0]);
		assert(!ty->is_void() && "Invalid parameter type!");
		
        // dummy qualifier
        CVQualifiers qfer;
		qfer.add_qualifiers(false, false);

        // get inout type if specified (default none)
        string inoutStr = tokens.size() > 1 ? tokens[1] : "";
        enum eParamInOutType inout = string_to_inout_type(inoutStr);

		Parameter *p = GenerateParameter(ty, &qfer, inout);
		assert(p);
		currFunc.params.push_back(p);
	}
}

// ----------------------------------------------------------------
// Create random parameters for a function
// ----------------------------------------------------------------
void
Parameter::GenerateParameterList(Function &curFunc)
{
	unsigned int max = ParamListProbability();

	for (unsigned int i =0; i <= max; i++) {
		GenerateParameter(curFunc);
	}
}

// --------------------------------------------------------------
void
Parameter::Output(std::ostream &out) const
{ 
	if (_inout != eParamNone) {
		out << inout_type_to_string(_inout) << " ";
	}
    Variable::Output(out);
} 
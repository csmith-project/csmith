// -*- mode: C++ -*-
//
// Copyright (c) 2015-2016 Huawei Technologies Co., Ltd
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
//

#ifndef TYPE_CONFIG_H
#define TYPE_CONFIG_H

#include <vector>
#include <string>
#include <iostream>
#include "Filter.h"
#include "Type.h"
#include "SafeOpFlags.h"
#include "FunctionInvocation.h"
#include "FunctionInvocationBinary.h"
#include "FunctionInvocationUnary.h"
#include "StatementAssign.h"
#include "tinyxml.h"

/*
* The restrict of type using
*/
enum eTypeRequestType
{
asGlobal,
asLocal,
asParam,
asReturn,
asStructMember,
asUnionMember,
asPointer,
asArray,
asUnaryExprRv,
asBinaryExprRv,
asAssignExprRv,
asGlobalInit

};
#define MAX_TYPE_REQUEST ((eTypeRequestType)(asGlobalInit+1))

/*
* This class packages the restrict of type using, include:
* 1 some restrict of type using, such as "Is the type able to be used as global variable or struct member "
* 2 the restrict of type convert;
* 3 some new types don't support some unary ops;
* 4 some new types don't support some binary ops
*/
class TypeConfig
{
public:
    static TypeConfig * create_instance( std::string file_path);
    static TypeConfig * get_instance();

    Filter* get_filter_for_request(eTypeRequestType request);
    
    Filter* get_filter_for_convert(const Type* type);

    Filter* get_filter_for_assignop(int op);

    void get_filter_for_assignop(const Type * type, VectorFilter * filter);

    bool check_exclude_by_request(const Type * type, eTypeRequestType request);
    
    bool check_exclude_by_convert(const Type * src_type, const Type * dest_type);

    bool check_exclude_by_unaryop(const Type * type, int op = MAX_UNARY_OP);
    
    bool check_exclude_by_binaryop(const Type * type, int op = MAX_BINARY_OP);

    bool check_additional_by_unaryop(const Type * type, int op, std::vector<int>& rhs );
    
    bool check_additional_by_binaryop(const Type * type, int op, std::vector< std::pair<int, int> >& rhs);

private:
    TypeConfig() { /* do nothing */ }
    ~TypeConfig() { /* do nothing */ }

    void initialize( std::string file_name );
    void parse_config( std::string file_name );
    void parse_config( TiXmlNode* node);

    void set_extend_types(TiXmlNode* node);
    void set_type_request_map(TiXmlNode* node);
    void set_type_convert_map(TiXmlNode* node);
    
    void set_unary_operators_map(TiXmlNode* node);
    void set_binary_operators_map(TiXmlNode* node);
    void set_assign_operators_map(TiXmlNode* node);
    
    int get_op_from_string(std::string op_string, SafeOpKind op_kind);

    std::map<int, vector<int> > types_excluded_by_request; /* map<request, type> */
    std::map<int, vector<int> > types_excluded_by_convert; /* map<dest_type, src_type> */

    std::map<int, vector<int> > types_excluded_by_unaryop; /* map<op, return_type> */
    std::map<int, vector<int> > types_excluded_by_binaryop; /* map<op, return_type> */
    std::map<int, vector<int> > types_excluded_by_assignop; /* map<op, return_type> */

    std::map<pair<int, int>, vector<int> > types_additional_by_unaryop;
    std::map<pair<int, int>, vector<pair<int, int> > > types_additional_by_binaryop;
    std::map<pair<int, int>, vector<int> > types_additional_by_assignop;

    static TypeConfig * type_config_;

};

#endif //TYPE_CONFIG_H

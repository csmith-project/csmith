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

#include "TypeConfig.h"

using namespace std;

TypeConfig * TypeConfig::type_config_ = NULL;

TypeConfig *
TypeConfig::create_instance( std::string file_path )
{
    std::string file_name = file_path + "/Syntax.xml";
    TypeConfig::type_config_ = new TypeConfig();
    TypeConfig::type_config_->initialize(file_name);
    return TypeConfig::type_config_;
}

TypeConfig *
TypeConfig::get_instance()
{
    assert(TypeConfig::type_config_);
    return TypeConfig::type_config_;
}

void
TypeConfig::initialize( std::string file_name )
{
    parse_config(file_name);
}

void
TypeConfig::parse_config( std::string file_name )
{
    TiXmlDocument doc(file_name.c_str());
    bool load_okay = doc.LoadFile();
    if (load_okay)
    {
        parse_config( &doc );
    }
    else
    {
        cout << "Failed to load file " << file_name << "\n";
    }
}

void
TypeConfig::parse_config( TiXmlNode* node)
{
    assert(node);
    std::string tag = "";
    int type = node->Type();
    static int config_flags = 0;

    switch ( type ) {
        case TiXmlNode::TINYXML_ELEMENT:
        {
            tag = node->Value();

            if((tag == "types") || (tag == "request") || (tag == "convert") || (tag == "unaryop") || (tag == "binaryop") || (tag == "assignop") )
                config_flags++;
            else
            {
                switch (config_flags)
                {
                    case 1:
                        set_extend_types(node);
                        break;
                    case 2:
                        set_type_request_map(node);
                        break;
                    case 3:
                        set_type_convert_map(node);
                        break;
                    case 4:
                        set_unary_operators_map(node);
                        break;
                    case 5:
                        set_binary_operators_map(node);
                        break;
                    case 6:
                        set_assign_operators_map(node);
                        break;
                    default:
                        break;
                }
            }
        }
        case TiXmlNode::TINYXML_DOCUMENT:
            break;
        case TiXmlNode::TINYXML_COMMENT:
            break;
        case TiXmlNode::TINYXML_UNKNOWN:
            break;
        case TiXmlNode::TINYXML_TEXT:
            break;
        case TiXmlNode::TINYXML_DECLARATION:
            break;
        default:
            break;
    }
    TiXmlNode * child = node->FirstChild();
    for (; child != 0; child = child->NextSibling())
    {
        parse_config( child );
    }
}

void
TypeConfig::set_extend_types(TiXmlNode* node)
{
    assert(node);

    TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

    std::string tag = node->Value();

    if(tag == "type")
    {
        std::string type_name = "";
        int type_id = -1;
        int type_size = -1;

        while(attr)
        {
            tag = attr->Name();
            if(tag == "name")
                type_name = attr->Value();

            else if(tag == "id")
                type_id = attr->IntValue();

            else if(tag == "size")
                type_size = attr->IntValue();

            attr = attr->Next();
        }
        Type::find_type(type_name, type_id, type_size);
    }
}

void
TypeConfig::set_type_request_map(TiXmlNode* node)
{
    assert(node);

    TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

    std::string tag = node->Value();
    std::string attr_name, attr_value;

    if(tag == "type")
    {
        int simple_type = -1;

        while(attr)
        {
            attr_name = attr->Name();
            attr_value = attr->Value();
            if(attr_name == "name")
                simple_type = Type::find_type(attr_value);

            else if(attr_name == "asGlobal" && attr_value == "false")
                (types_excluded_by_request[asGlobal]).push_back(simple_type);

            else if(attr_name == "asLocal" && attr_value == "false")
                (types_excluded_by_request[asLocal]).push_back(simple_type);

            else if(attr_name == "asParam" && attr_value == "false")
                (types_excluded_by_request[asParam]).push_back(simple_type);

            else if(attr_name == "asReturn" && attr_value == "false")
                (types_excluded_by_request[asReturn]).push_back(simple_type);

            else if(attr_name == "asStructMember" && attr_value == "false")
                (types_excluded_by_request[asStructMember]).push_back(simple_type);

            else if(attr_name == "asUnionMember" && attr_value == "false")
                (types_excluded_by_request[asUnionMember]).push_back(simple_type);

            else if(attr_name == "asPointer" && attr_value == "false")
                (types_excluded_by_request[asPointer]).push_back(simple_type);

            else if(attr_name == "asArray" && attr_value == "false")
                (types_excluded_by_request[asArray]).push_back(simple_type);

            else if(attr_name == "asUnaryExprRv" && attr_value == "false")
                (types_excluded_by_request[asUnaryExprRv]).push_back(simple_type);

            else if(attr_name == "asBinaryExprRv" && attr_value == "false")
                (types_excluded_by_request[asBinaryExprRv]).push_back(simple_type);

            else if(attr_name == "asAssignExprRv" && attr_value == "false")
                (types_excluded_by_request[asAssignExprRv]).push_back(simple_type);

            else if(attr_name == "asGlobalInit" && attr_value == "false")
                (types_excluded_by_request[asGlobalInit]).push_back(simple_type);
            
            attr = attr->Next();
        }
    }
}

void
TypeConfig::set_type_convert_map(TiXmlNode* node)
{
    assert(node);

    int src_type = -1;
    int dest_type = -1;
    bool is_symmetrical = false;
    std::string tag = node->Value();
    std::string attr_name, attr_value;

    TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

    if(tag == "cast")
    {
        while(attr)
        {
            attr_name = attr->Name();
            attr_value = attr->Value();
            if(attr_name == "src")
                src_type = Type::find_type(attr_value);
            if(attr_name == "dest")
                dest_type = Type::find_type(attr_value);
            if(attr_name == "symmetrical")
                is_symmetrical = (attr_value == "true") ? true : false;
            attr = attr->Next();
        }

        if(is_symmetrical)
            (types_excluded_by_convert[src_type]).push_back(dest_type);
        (types_excluded_by_convert[dest_type]).push_back(src_type);
    }
}

int 
TypeConfig::get_op_from_string(std::string op_string, SafeOpKind op_kind)
{
    int op_index = -1;
    std::string op_str = "";
    switch (op_kind)
    {
        case sOpUnary:
            for(op_index = 0; op_index < MAX_UNARY_OP; op_index++)
            {
                op_str = FunctionInvocationUnary::get_op((eUnaryOps)op_index);
                if( op_str == op_string )
                    return op_index;
            }
            break;
        case sOpBinary:
            for(op_index = 0; op_index < MAX_BINARY_OP; op_index++)
            {
                op_str = FunctionInvocationBinary::get_op((eBinaryOps)op_index);
                if( op_str == op_string )
                    return op_index;
            }
            break;
        case sOpAssign:
            for(op_index = 0; op_index < MAX_ASSIGN_OP; op_index++)
            {
                op_str = StatementAssign::get_op((eAssignOps)op_index);
                if( op_str == op_string )
                    return op_index;
            }
            break;
    }

    return -1;
}

void 
TypeConfig::set_unary_operators_map(TiXmlNode* node)
{
    assert(node);

    int op = -1;
    int lhs_type = -1;
    int rhs_type = -1;
    bool additional = false;
    pair<int, int> op_type;

    TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

    std::string tag = node->Value();
    std::string attr_name, attr_value;

    if(tag == "op")
    {
        while(attr)
        {
            attr_name = attr->Name();
            attr_value = attr->Value();
            if(attr_name == "name")
                op = get_op_from_string(attr_value, sOpUnary);

            else if(attr_name == "rhs")
                rhs_type = Type::find_type(attr_value);

            else if(attr_name == "lhs" )
                lhs_type = Type::find_type(attr_value);

            else if(attr_name == "additional")
                additional = true;                

            attr = attr->Next();
        }
        assert((lhs_type != -1) && (op != -1) && "imcomplete operator settings");
        if(additional)
        {
            assert((rhs_type != -1) && "imcomplete operator settings");
            op_type = std::make_pair(op, lhs_type);            
            types_additional_by_unaryop[op_type].push_back(rhs_type);
        }
        else
             types_excluded_by_unaryop[op].push_back(lhs_type);                   
    }
}

void 
TypeConfig::set_binary_operators_map(TiXmlNode* node)
{
    assert(node);

    int op = -1;
    int lhs_type = -1;
    int rhs1_type = -1;
    int rhs2_type = -1;
    bool additional = false;
    pair<int, int> op_type;
    pair<int, int> rhs_types;

    TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

    std::string tag = node->Value();
    std::string attr_name, attr_value;

    if(tag == "op")
    {
        while(attr)
        {
            attr_name = attr->Name();
            attr_value = attr->Value();
            if(attr_name == "name")
                op = get_op_from_string(attr_value, sOpBinary);

            else if(attr_name == "rhs1")
                rhs1_type = Type::find_type(attr_value);

            else if(attr_name == "rhs2")
                rhs2_type = Type::find_type(attr_value);

            else if(attr_name == "lhs" )
                lhs_type = Type::find_type(attr_value);

            else if(attr_name == "additional")
                additional = true;                

            attr = attr->Next();
        }
        assert((lhs_type != -1) && (op != -1) && "imcomplete operator settings");
        if(additional)
        {
            assert((rhs1_type != -1) && (rhs2_type != -1) && "imcomplete operator settings");
            op_type = std::make_pair(op, lhs_type);            
            rhs_types = std::make_pair(rhs1_type, rhs2_type);
            types_additional_by_binaryop[op_type].push_back(rhs_types);
        }
        else
             types_excluded_by_binaryop[op].push_back(lhs_type);                   
    }
}

void 
TypeConfig::set_assign_operators_map(TiXmlNode* node)
{
    assert(node);

    int op = -1;
    int lhs_type = -1;
    int rhs_type = -1;
    bool additional = false;
    pair<int, int> op_type;

    TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

    std::string tag = node->Value();
    std::string attr_name, attr_value;

    if(tag == "op")
    {
        while(attr)
        {
            attr_name = attr->Name();
            attr_value = attr->Value();
            if(attr_name == "name")
                op = get_op_from_string(attr_value, sOpAssign);

            else if(attr_name == "rhs")
                rhs_type = Type::find_type(attr_value);

            else if(attr_name == "lhs" )
                lhs_type = Type::find_type(attr_value);

            else if(attr_name == "additional")
                additional = true;                

            attr = attr->Next();
        }
        assert((lhs_type != -1) && (op != -1) && "imcomplete operator settings");
        if(additional)
        {
            assert((rhs_type != -1) && "imcomplete operator settings");
            op_type = std::make_pair(op, lhs_type);            
            types_additional_by_assignop[op_type].push_back(rhs_type);
        }
        else
             types_excluded_by_assignop[op].push_back(lhs_type);                   
    }
}

Filter* 
TypeConfig::get_filter_for_request(eTypeRequestType request)
{
    VectorFilter * filter = new VectorFilter();
    vector<int> exclude_types = types_excluded_by_request[static_cast<int>(request)];

    vector<int>::iterator iter;
    for(iter = exclude_types.begin(); iter != exclude_types.end(); ++iter)
    {
        filter->add(Type::get_simple_type((eSimpleType)*iter).type_index);
    }

    return filter;
}

bool 
TypeConfig::check_exclude_by_request(const Type * type, eTypeRequestType request)
{
    if(type->eType == eSimple)
    {
        int type_index = type->simple_type;
        vector<int> types = types_excluded_by_request[request];
        vector<int>::iterator iter = types.begin();
        for(;iter != types.end(); ++iter)
        {
            if(*iter == type_index)
                return true;
        }
    }
    return false;
}

Filter* 
TypeConfig::get_filter_for_convert(const Type* dest_type)
{
    assert(dest_type->eType == eSimple);

    VectorFilter * filter = new VectorFilter();
    vector<int> exclude_types = types_excluded_by_convert[static_cast<int>(dest_type->simple_type)];

    vector<int>::iterator iter;
    for(iter = exclude_types.begin(); iter != exclude_types.end(); ++iter)
    {
        filter->add(Type::get_simple_type((eSimpleType)*iter).type_index);
    }

    return filter;
}

bool 
TypeConfig::check_exclude_by_convert(const Type * src_type, const Type * dest_type)
{
    if((src_type->eType == eSimple) && (dest_type->eType == eSimple))
    {
        int type_src_index = src_type->simple_type;
        int type_dest_index = dest_type->simple_type;
        vector<int> types = types_excluded_by_convert[type_dest_index];
        vector<int>::iterator iter = types.begin();
        for(;iter != types.end(); ++iter)
        {
            if(*iter == type_src_index)
                return true;
        }
    }
    return false;
}

Filter* 
TypeConfig::get_filter_for_assignop(int op)
{
    pair<int, int> op_type;
    VectorFilter * filter = new VectorFilter();
    vector<int> exclude_types = types_excluded_by_assignop[op];

    vector<int>::iterator iter;
    for(iter = exclude_types.begin(); iter != exclude_types.end(); ++iter)
    {
        op_type = std::make_pair(op, *iter);
        if ( types_additional_by_assignop.count(op_type) > 0 )
            continue;
        filter->add(Type::get_simple_type((eSimpleType)*iter).type_index);
    }

	for(iter = types_excluded_by_request[asAssignExprRv].begin(); iter != types_excluded_by_request[asAssignExprRv].end(); ++iter)
    {
        filter->add(Type::get_simple_type((eSimpleType)*iter).type_index);
    }

    return filter;
}

void 
TypeConfig::get_filter_for_assignop(const Type * type, VectorFilter * filter)
{
    pair<int, int> op_type;
    int op_id = 0;
    int max_op = MAX_ASSIGN_OP;

    for(; op_id < max_op; ++op_id)
    {
        op_type = std::make_pair(op_id, type->simple_type);
        if ( types_additional_by_assignop.count(op_type) > 0 )
            continue;
        
        vector<int> exclude_types = types_excluded_by_assignop[op_id];

        if(std::find(exclude_types.begin(), exclude_types.end(), type->simple_type) != exclude_types.end())
            filter->add(op_id);
    }
}

bool
TypeConfig::check_exclude_by_unaryop(const Type * type, int op)
{
    pair<int, int> op_type = std::make_pair(op, type->simple_type);

    if ( types_additional_by_unaryop.count(op_type) > 0 )
            return false;
        
    vector<int> exclude_types = types_excluded_by_unaryop[op];

    if( std::find(exclude_types.begin(), exclude_types.end(), type->simple_type) == exclude_types.end())
        return false;

    return true;
}

bool
TypeConfig::check_exclude_by_binaryop(const Type * type, int op)
{
    pair<int, int> op_type = std::make_pair(op, type->simple_type);

    if ( types_additional_by_binaryop.count(op_type) > 0 )
            return false;
        
    vector<int> exclude_types = types_excluded_by_binaryop[op];

    if( std::find(exclude_types.begin(), exclude_types.end(), type->simple_type) == exclude_types.end())
        return false;

    return true;
}

bool 
TypeConfig::check_additional_by_unaryop(const Type * type, int op, std::vector<int>& rhs )
{
    pair<int, int> op_type = std::make_pair(op, type->simple_type);
    if ( types_additional_by_unaryop.count(op_type) > 0 )
    {
        rhs = types_additional_by_unaryop[op_type];
        return true;
    }
    return false;    
}

bool 
TypeConfig::check_additional_by_binaryop(const Type * type, int op, std::vector< std::pair<int, int> >& rhs )
{
    pair<int, int> op_type = std::make_pair(op, type->simple_type);
    if ( types_additional_by_binaryop.count(op_type) > 0 )
    {
        rhs = types_additional_by_binaryop[op_type];
        return true;
    }
    return false;   
    
}
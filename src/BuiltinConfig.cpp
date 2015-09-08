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

#include "BuiltinConfig.h"
#include "ParameterBuiltin.h"
#include "Function.h"
#include "FunctionBuiltin.h"
#include "Type.h"
#include "PointerType.h"

#include "StringUtils.h"
#include "tinyxml.h"


enum ESTAGE {
    STAGE_RETURN_TYPE,
    STAGE_INTRIN_NAME,
    STAGE_PARAM_CONST,
    STAGE_PARAM_PTR,
    STAGE_PARAM_TYPE,
    STAGE_PARAM_BASE,
    STAGE_PARAM_OUT,
    STAGE_PARAM_NOALIGN,
    STAGE_PARAM_BIAS,
    STAGE_PARAM_TOP,
    STAGE_PARAM_BOTTOM,
    STAGE_PARAM_ALIGNSIZE,
    STAGE_PARAM_DIVISOR,
    STAGE_INVALID
};

const unsigned int NUM_INDENTS_PER_SPACE=2;
const char* tag_builtin = "intrinsic";
const char* tag_return = "return";
const char* tag_type = "type";
const char* tag_name = "name";
const char* tag_param = "param";
const char* attri_id = "id";
const char* tag_out = "out";
const char* tag_pointer = "pointer";
const char* tag_base = "base";
const char* tag_noalign = "noalign";
const char* tag_bias = "bias";
const char* tag_top = "top";
const char* tag_bottom = "bottom";
const char* tag_alignsize = "alignsize";
const char* tag_divisor = "divisor";
const char* tag_const = "const";

static FunctionBuiltin* builtin = 0;
static ParameterBuiltin* param = 0;

std::vector<FunctionBuiltin*> BuiltinConfig::builtin_config_list;

BuiltinConfig::~BuiltinConfig(void)
{
}

void
dump_intrin_info(TiXmlNode* pParent, unsigned int indent = 0) {
    if ( !pParent ) return;

    TiXmlNode* pChild;
    TiXmlText* pText;
    static bool sFlagReturn = false;
    static bool sFlagParam = false;
    static ESTAGE sStage = STAGE_INVALID;
    int t = pParent->Type();

    switch ( t ) {
        case TiXmlNode::TINYXML_DOCUMENT:
            //printf( "Document" );
            break;

        case TiXmlNode::TINYXML_ELEMENT:
            //printf( "Element [%s]", pParent->Value() );
			if ( strcmp(pParent->Value(), tag_builtin) == 0 ) {
				// 构建一个新的struIntrinsic实例
				builtin = new FunctionBuiltin();
				// 置sFlagReturn和sFlagParam为false
				sFlagReturn = false;
				sFlagParam = false;
				
				builtin->used = false;
				builtin->out = false;
				builtin->params_cnt = 0;
				BuiltinConfig::builtin_config_list.push_back(builtin);
				break;
			}
			if ( strcmp(pParent->Value(), tag_return) == 0 ) {
				sFlagReturn = true;
				break;
			}
			if ( strcmp(pParent->Value(), tag_param) == 0 ) {
				sFlagParam = true;
				param = new ParameterBuiltin();
				
				// 初始化缺省值
				param->is_imm = false;
				param->out = false;
				param->base = false;
				param->no_align = false;
				param->bias = false;
				param->is_divisor = false;
				param->ptr_level = 0;
				param->const_level = -1;
				param->has_range = false;
				param->bottom = 0;
				param->top = 0;
				param->align_size = 0;
				
				if (builtin) {
					builtin->params.push_back(param);
					builtin->params_cnt++;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_name) == 0 ) {
				// assert(intrin);
				sStage = STAGE_INTRIN_NAME;
				break;
			}
			if ( strcmp(pParent->Value(), tag_type) == 0 ) {
				if (sFlagReturn) {
					// 当前走到了返回类型标签
					sStage = STAGE_RETURN_TYPE;
					sFlagReturn = false;
				} else if (sFlagParam) {
					sStage = STAGE_PARAM_TYPE;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_pointer) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_PTR;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_base) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_BASE;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_bias) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_BIAS;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_out) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_OUT;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_noalign) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_NOALIGN;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_top) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_TOP;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_bottom) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_BOTTOM;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_alignsize) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_ALIGNSIZE;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_divisor) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_DIVISOR;
				}
				break;
			}
			if ( strcmp(pParent->Value(), tag_const) == 0 ) {
				if (sFlagParam) {
					sStage = STAGE_PARAM_CONST;
				}
				break;
			}
            
            break;

        case TiXmlNode::TINYXML_COMMENT:
            //printf( "Comment: [%s]", pParent->Value());
            break;

        case TiXmlNode::TINYXML_UNKNOWN:
            //printf( "Unknown" );
            break;

        case TiXmlNode::TINYXML_TEXT:
            pText = pParent->ToText();
            //printf( "Text: [%s]", pText->Value() );
			switch (sStage) {
				case STAGE_RETURN_TYPE: {
					// assert(intrin);
					builtin->return_type_name = pText->Value();
					break;
				}
				case STAGE_INTRIN_NAME: {
					//assert(intrin);
					builtin->name = pText->Value();
					break;
				}
				case STAGE_PARAM_CONST: {
					//assert(param);
					param->const_level = StringUtils::str2int(pText->Value());
					break;
				}
				case STAGE_PARAM_PTR: {
					//assert(param);
					param->ptr_level = StringUtils::str2int(pText->Value());
					break;
				}
				case STAGE_PARAM_TYPE: {
					//assert(param);
					param->type_name = pText->Value();
					if (param->type_name == "immediate") {
						param->is_imm = true;
                        param->type_name = "Int";
					}
					break;
				}
				case STAGE_PARAM_BASE: {
					//assert(param);
					param->base = ( strcmp( pText->Value(), "true" ) == 0 ) ? true : false;
					break;
				}
				case STAGE_PARAM_OUT: {
					//assert(param);
					param->out = ( strcmp( pText->Value(), "true" ) == 0 ) ? true : false;
					if (param->out) {
						builtin->out = true;
                    }
					break;
				}
				case STAGE_PARAM_NOALIGN: {
					//assert(param);
					param->no_align = ( strcmp( pText->Value(), "true" ) == 0 ) ? true : false;
					break;
				}
				case STAGE_PARAM_BIAS: {
					//assert(param);
					param->bias = ( strcmp( pText->Value(), "true" ) == 0 ) ? true : false;
					break;
				}
				case STAGE_PARAM_TOP: {
					//assert(param);
					param->top = StringUtils::str2int(pText->Value());
					param->has_range = true;
					break;
				}
				case STAGE_PARAM_BOTTOM: {
					//assert(param);
					param->bottom = StringUtils::str2int(pText->Value());
					break;
				}
				case STAGE_PARAM_ALIGNSIZE: {
					//assert(param);
					param->align_size = StringUtils::str2int(pText->Value());
					break;
				}
				case STAGE_PARAM_DIVISOR: {
					//assert(param);
					param->is_divisor = ( strcmp( pText->Value(), "true" ) == 0 ) ? true : false;
					break;
				}
				default: {
					break;
				}
			}
			sStage = STAGE_INVALID;
            break;

        case TiXmlNode::TINYXML_DECLARATION:
            // printf( "Declaration" );
            break;
        default:
            break;
    }
    
    for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
    {
        dump_intrin_info( pChild, indent+1 );
    }
}

void BuiltinConfig::build_builtin_list_from_xml(std::string file_path) {
	// read XML configuration file
	std::string file_name = file_path + "/Builtin.xml";
	TiXmlDocument builtinDoc(file_name.c_str());
    bool loadOkay = builtinDoc.LoadFile();
    if (loadOkay) {
        dump_intrin_info(&builtinDoc);
    } else {
        cout << "please make sure builtin config file has been specified." << std::endl;
        return; // this failure should not stop the whole progress.
    }

    // config all Types CVQualifiers
	unsigned int i = 0;
    for (; i < BuiltinConfig::builtin_config_list.size(); i++) {
        Function::make_builtin_function(BuiltinConfig::builtin_config_list[i]);
    }
}


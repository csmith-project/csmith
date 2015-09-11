#ifndef FLOAT_TEST_UTILS
#define FLOAT_TEST_UTILS

#include "Common.h"
#include <cassert>

//float_test
static void
output_cast_to_interval_macro(std::ostream &out, const Type& type){

	string s = "";
	switch (type.simple_type) {
	case eChar:
		s = "CHAR_TO_FLOAT_INTERVAL";
		break;
	case eInt:
		s = "INT_TO_FLOAT_INTERVAL";
		break;
	case eShort:
		s = "SHORT_TO_FLOAT_INTERVAL";
		break;
	case eLong:
		s = "LONG_TO_FLOAT_INTERVAL";
		break;
	case eLongLong:
		s = "LONG_LONG_TO_FLOAT_INTERVAL";
		break;
	case eUChar:
		s = "UCHAR_TO_FLOAT_INTERVAL";
		break;
	case eUInt:
		s = "UINT_TO_FLOAT_INTERVAL";
		break;
	case eUShort:
		s = "USHORT_TO_FLOAT_INTERVAL";
		break;
	case eULong:
		s = "ULONG_TO_FLOAT_INTERVAL";
		break;
	case eULongLong:
		s = "ULONG_LONG_TO_FLOAT_INTERVAL";
		break;
	case eVoid:
	case eFloat:
	default:
		if(type.eType != eSimple){
			cerr << "NOT SIMPLE" << endl;
		}else{
			cerr << "GOT SIMPLE" << endl;
			if (type.simple_type == eFloat){
				cerr << "FLOAT" << endl;
			}
		}
		assert(false && "Unexpected type : output_cast_to_interval_macro");
		break;
	}
	s += "(";
	out << s;
}

//float_test
static void
output_func_float_macro(eBinaryOps eFunc, std::ostream &out)
{
	switch (eFunc) {
		// Logical Ops
	case eAnd:		out << "FLOAT_AND_MACRO(";		break;
	case eOr:		out << "FLOAT_OR_MACRO(";		break;
	case eCmpEq:	out << "FLOAT_CMPEQ_MACRO(";	break;
	case eCmpNe:	out << "FLOAT_CMPNE_MACRO(";	break;
	case eCmpGt:	out << "FLOAT_CMPGT_MACRO(";	break;
	case eCmpLt:	out << "FLOAT_CMPLT_MACRO(";	break;
	case eCmpLe:	out << "FLOAT_CMPLE_MACRO(";	break;
	case eCmpGe:	out << "FLOAT_CMPGE_MACRO(";	break;

		// Bitwise Ops
	case eBitAnd:	out << "FLOAT_BITAND_MACRO(";	break;
	case eBitOr:	out << "FLOAT_BITOR_MACRO(";	break;
	case eBitXor:	out << "FLOAT_BITXOR_MACRO(";	break;
	case eLShift:	out << "FLOAT_LSHIFT_MACRO(";   break;
	case eRShift:	out << "FLOAT_RSHIFT_MACRO(";   break;

	case eAdd:
	case eSub:
	case eMul:
	case eDiv:
	case eMod:
	default:
		assert(false && "Trying to output macro for invalid func");
		break;
	}
}


static void
output_func_float_macro(eUnaryOps eFunc, std::ostream &out)
{
	switch (eFunc) {

	case eMinus:	out << "MINUS_FLOAT_INTERVAL(";		break;
	case ePlus:		out << "PLUS_FLOAT_INTERVAL(";		break;
	case eNot:		out << "NOT_FLOAT_INTERVAL(";		break;
	case eBitNot:	out << "BITNOT_FLOAT_INTERVAL(";	break;

	default:
		assert(false && "Trying to output macro for invalid func");
		break;
	}
}

static void
output_cast_from_interval_macro(std::ostream &out, const Type& type){
	//float_test
	// this is quite ugly, but basically we don't want to output cast from float to float
	if (type.simple_type == eFloat){
		assert(false && "Cast from float to float should not be required");
	}

	string s = "";
	switch (type.simple_type) {
	case eChar:
		s = "FLOAT_INTERVAL_TO_CHAR";
		break;
	case eInt:
		s = "FLOAT_INTERVAL_TO_INT";
		break;
	case eShort:
		s = "FLOAT_INTERVAL_TO_SHORT";
		break;
	case eLong:
		s = "FLOAT_INTERVAL_TO_LONG";
		break;
	case eLongLong:
		s = "FLOAT_INTERVAL_TO_LONG_LONG";
		break;
	case eUChar:
		s = "FLOAT_INTERVAL_TO_UCHAR";
		break;
	case eUInt:
		s = "FLOAT_INTERVAL_TO_UINT";
		break;
	case eUShort:
		s = "FLOAT_INTERVAL_TO_USHORT";
		break;
	case eULong:
		s = "FLOAT_INTERVAL_TO_ULONG";
		break;
	case eULongLong:
		s = "FLOAT_INTERVAL_TO_ULONG_LONG";
		break;
	case eVoid:
	default:
		assert(0 && "Unexpected type in cast from float");
		break;
	}
	s += "(";
	out << s;
}




#endif

/*  Copyright 1992-2021 Intel Corporation.                                 */
/*                                                                         */
/*  This software and the related documents are Intel copyrighted          */
/*  materials, and your use of them is governed by the express license     */
/*  under which they were provided to you ("License"). Unless the License  */
/*  provides otherwise, you may not use, modify, copy, publish,            */
/*  distribute, disclose or transmit this software or the related          */
/*  documents without Intel's prior written permission.                    */
/*                                                                         */
/*  This software and the related documents are provided as is, with no    */
/*  express or implied warranties, other than those that are expressly     */
/*  stated in the License.                                                 */

#ifndef __OCL_TYPES_H__
#define __OCL_TYPES_H__

// Short hand names for unsigned integer types
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
#ifdef  _MSC_VER
typedef unsigned long long ulong;
#else
typedef unsigned long ulong;
#endif
// Note long long is not defined in OpenCL 1.2


//Arbitrary Percision Integer up to 64 bits
#include "../kernel_headers/ihc_apint.h"

#define _AV(_T,_n,_a) typedef _T _T##_n __attribute__(( ext_vector_type(_n),aligned(_a) ));
#define _AVP2(_T,_n) _AV(_T,_n,sizeof(_T)*_n)
#if __OPENCL_C_VERSION__ >= 110
#define _IHC_VDECLARES(_T) _AVP2(_T,2) _AVP2(_T,4) _AVP2(_T,8) _AVP2(_T,16) _AV(_T,3,sizeof(_T)*4)
#else
#define _IHC_VDECLARES(_T) _AVP2(_T,2) _AVP2(_T,4) _AVP2(_T,8) _AVP2(_T,16)
#endif
_IHC_VDECLARES(char)
_IHC_VDECLARES(uchar)
_IHC_VDECLARES(short)
_IHC_VDECLARES(ushort)
_IHC_VDECLARES(int)
_IHC_VDECLARES(uint)
_IHC_VDECLARES(long)
_IHC_VDECLARES(ulong)
_IHC_VDECLARES(float)
_IHC_VDECLARES(double)
#undef _AV
#undef _AVP2
#undef _IHC_VDECLARES

//Address Space Qualifiers
#define OCL_ADDRSP_CONSTANT const __attribute__((address_space(2)))
#define OCL_ADDRSP_GLOBAL __attribute__((address_space(1)))
#define OCL_ADDRSP_PRIVATE __attribute__((address_space(0)))
#define OCL_ADDRSP_LOCAL __attribute__((address_space(3)))
#define OCL_ADDRSP_GENERIC __attribute__((address_space(4)))

#endif // __OCL_TYPES_H__

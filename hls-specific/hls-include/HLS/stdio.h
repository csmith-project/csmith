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

#ifndef __HLS_STDIO_H__
#define __HLS_STDIO_H__

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifdef HLS_SYNTHESIS
// Suppress if used in component
# define printf(...) 

#endif //HLS_SYNTHESIS
#endif //__HLS_STDIO_H__

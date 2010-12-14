//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Header:  oal_args.h
//
//  This header file defines OAL boot arguments module interface. The module
//  is internal and it doesn't export any function or variable to kernel.
//  It is used for passing boot arguments from boot loader to HAL/kernel on
//  devices using boot loader.
//
#ifndef __OAL_ARGS_H
#define __OAL_ARGS_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  OAL_ARGS_xxx
//
//  This constant are used to identify/verify argument structure and its
//  version in memory.
//
#define OAL_ARGS_SIGNATURE      'SGRA'
#define OAL_ARGS_VERSION        1

//------------------------------------------------------------------------------
//
//  Type:  OAL_ARG_HEADER
//
//  This type define arguments header. It should be used at start of argument
//  structure to identify it and verify version.
//
typedef struct {
    UINT32  signature;
    UINT16  oalVersion;
    UINT16  bspVersion;
} OAL_ARGS_HEADER;

//------------------------------------------------------------------------------
//
//  Define:  OAL_ARG_QUERY_xxx
//
//  This constant are used to identify argument items in structure. Values 
//  smaller than 64 are reserved for OAL library. The platform implementation
//  can use values beginning from BSP_ARGS_QUERY.
//
#define OAL_ARGS_QUERY_DEVID        1
#define OAL_ARGS_QUERY_KITL         2
#define OAL_ARGS_QUERY_UUID         3
#define OAL_ARGS_QUERY_RTC          4


#define BSP_ARGS_QUERY              64

#define BSP_ARGS_QUERY_DBGSERIAL    BSP_ARGS_QUERY      // Query debug serial port.
#define BSP_ARGS_QUERY_HIVECLEAN    BSP_ARGS_QUERY+1    // Query hive clean flag.
#define BSP_ARGS_QUERY_CLEANBOOT    BSP_ARGS_QUERY+2    // Query clean boot flag.
#define BSP_ARGS_QUERY_FORMATPART   BSP_ARGS_QUERY+3    // Query format partition flag.
#define BSP_ARGS_QUERY_SIGNEDSTATE  BSP_ARGS_QUERY+4    // Query image signed state.

#define S3C2450_BASE_REG_PA_NOUART 0x00000000

//------------------------------------------------------------------------------
//
//  Function:  OALArgsQuery
//
//  This function is called by other OAL modules to obtain value from argument
//  structure. It should return NULL when given argument type wasn't found.
//  Function should also solve issues related to different argument structure
//  versions.
//
VOID* OALArgsQuery(UINT32 type);

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

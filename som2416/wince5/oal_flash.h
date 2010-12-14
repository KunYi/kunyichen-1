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
//  Header:  oal_flash.h
//
#ifndef __OAL_FLASH_H
#define __OAL_FLASH_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

typedef struct {
    UINT32 width;
    UINT32 parallel;
    UINT32 set;
    UINT32 size;
    UINT32 burst;
    UINT32 regions;
    UINT32 aBlockSize[8];
    UINT32 aBlocks[8];
} OAL_FLASH_INFO;

BOOL OALFlashInfo(VOID *pBase, OAL_FLASH_INFO *pInfo);
BOOL OALFlashErase(VOID *pBase, VOID *pStart, UINT32 size);
BOOL OALFlashWrite(VOID *pBase, VOID *pStart, UINT32 size, VOID *pBuffer);
VOID OALFlashDump(VOID *pBase);

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

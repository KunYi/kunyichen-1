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
#ifndef __OAL_BLMENU_H
#define __OAL_BLMENU_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

typedef struct OAL_MENU_ITEM {
    WCHAR key;
    LPCWSTR text;
    VOID (*pfnAction)(struct OAL_MENU_ITEM *);
    VOID *pParam1;
    VOID *pParam2;
    VOID *pParam3;
} OAL_BLMENU_ITEM;

//------------------------------------------------------------------------------

VOID OALBLMenuActivate(UINT32 delay, OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuHeader(LPCWSTR format, ...);

VOID OALBLMenuShow(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuEnable(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuSetDeviceId(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuSetMacAddress(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuSetIpAddress(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuSetIpMask(OAL_BLMENU_ITEM *pMenu);
VOID OALBLMenuSelectDevice(OAL_BLMENU_ITEM *pMenu);

UINT32 OALBLMenuReadLine(LPWSTR szBuffer, size_t size);
WCHAR  OALBLMenuReadKey(BOOL wait);

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

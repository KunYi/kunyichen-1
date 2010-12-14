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
//  Header: oal_power.h
//
#ifndef __OAL_POWER_H
#define __OAL_POWER_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

extern volatile UINT32 g_oalWakeSource;

//------------------------------------------------------------------------------

VOID OEMPowerOff();
BOOL OALPowerWakeSource(UINT32 sysIntr);

BOOL OALIoCtlHalPresuspend(UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*);
BOOL OALIoCtlHalEnableWake(UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*);
BOOL OALIoCtlHalDisableWake(UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*);
BOOL OALIoCtlHalGetWakeSource(UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*);

//------------------------------------------------------------------------------

VOID OALCPUPowerOff();
VOID OALCPUPowerOn();

VOID BSPPowerOff();
VOID BSPPowerOn();

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

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
//  Header: oal_rtc.h
//
#ifndef __OAL_RTC_H
#define __OAL_RTC_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
// File:        oal_rtc.h
//
// This header file defines functions, constant and macros used in RTC module.
//
// The module exports for Windows CE OS kernel:
//      * OEMIoControl/IOCTL_HAL_INIT_RTC
//      * OEMGetRealTime
//      * OEMSetRealTime
//      * OEMSetAlarmTime
//
//  The module export for OAL libraries:
//      -- N/A --
//
//  The module OAL dependencies:
//      * There should be static interrupt mapping for SYSINTR_RTC_ALARM set
//        for IRQ used for RTC alarm.
//
//------------------------------------------------------------------------------

BOOL OALIoCtlHalInitRTC(UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*);

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

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
//  Header: oal_profiler.h
//
//  This header define OAL profiler module. Module code implements HAL functions
//  required for kernel profiling. There are not any internal module function
//  or variables at this time.
//
//  Interaction with kernel:
//      * ProfilerHit (called by interrupt handler)
//      * OEMProfileTimerEnable
//      * OEMProfileTimerDisable
//
//  Interaction with other OAL modules:
//      * g_oalTimer
//      * OALTimerReduceSysTick (timer)
//      * OALTimerExtendSysTick (timer)
//      * OALTimerCountsSinceSysTick (timer)
//
#ifndef __OAL_PROFILER_H
#define __OAL_PROFILER_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Functions:  GetEPC, ProfilerHit
//
//  Following functions are exported by profiler kernel or by profiler module.
//
UINT32 GetEPC();
VOID ProfilerHit(UINT32 epc);

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif


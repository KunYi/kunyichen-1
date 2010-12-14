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
//  File: interrupt.c
//
//  This file implement Samsung S3C2450 SoC specific OALIoCtlxxxx functions.
//
#include <windows.h>
#include <s3c2450.h>
#include <oal.h>

 //[david.modify] 2008-11-14 10:20
 #include <dbgmsg_david.h>

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReboot
//
//
BOOL OALIoCtlHalReboot(UINT32 code, VOID *pInpBuffer, 
                       UINT32 inpSize, VOID *pOutBuffer, 
                       UINT32 outSize, UINT32 *pOutSize)
{
    //
    // If the board design supports software-controllable hardware reset logic, it should be
    // used.  Because this routine is specific to the S3C2450 CPU, it only uses the watchdog
    // timer to assert reset.  One downside to this approach is that nRSTOUT isn't asserted
    // so any board-level logic isn't reset via this method.  This routine can be overidden in
    // the specific platform code to control board-level reset logic, should it exist.
    //
    volatile S3C2450_CLKPWR_REG *pCLKPWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);

 //[david.modify] 2008-11-14 10:21
 // lcpµØÍ¼¸´Î»
	DPSTR_R1("reboot");
	 return ;
	
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));

    // Into reset
	pCLKPWR->SWRST = 0x533C2450;

    // Wait for watchdog reset...
    //
    while(TRUE);

    // Should never get to this point...
    //
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalReboot\r\n"));

    return(TRUE);

}

//------------------------------------------------------------------------------

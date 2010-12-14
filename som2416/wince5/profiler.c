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
//  File:  profiler.c
//
//  This file contains implementation of profiler module suitable for the
//  Samsung S3C2450 CPU/SoC with count/compare timer.  The routines
//  use match register 2 (M2) for the profiling timer.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <intr.h>
#include <s3c2450.h>

//------------------------------------------------------------------------------
// Local Variables 
static S3C2450_PWM_REG *g_pPWMRegs = NULL;

static struct
{
    BOOL enabled;                               // is profiler active?
    UINT32 countsPerHit;                        // counts per profiler interrupt
} g_profiler;

//------------------------------------------------------------------------------
// External Variables 

extern PFN_PROFILER_ISR g_pProfilerISR;

//------------------------------------------------------------------------------
// Local Functions 
UINT32 OALProfileIntrHandler(UINT32 ra);
    

//------------------------------------------------------------------------------
//
//  Function:  ConfigureNextProfilerCount
//
//  Updates the profiler count (prepares the timer for the next timer event).
//
static void ConfigureNextProfilerCount(DWORD dwCountInterval)
{
    DWORD dwTimerTemp;  
	
    if (!g_pProfilerISR) return;

	   // Change number of timer ticks in the period.
	   //
	   OUTREG32(&g_pPWMRegs->TCNTB2, dwCountInterval);

	   dwTimerTemp = INREG32(&g_pPWMRegs->TCON) & (~(0xf << 12));
	   OUTREG32(&g_pPWMRegs->TCON, dwTimerTemp | (2 << 12));	// Update TCVNTB2 and stop.
	   OUTREG32(&g_pPWMRegs->TCON, dwTimerTemp | (1 << 12));	// One-shot mode and start.

}


//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerEnable
//
//  This function is called by kernel to start kernel profiling timer.
//
VOID OEMProfileTimerEnable(DWORD interval)
{
    BOOL enabled;
    UINT32 irq;
    
    OALMSG(TRUE, (L"+OEMProfileTimerEnable(%d)\r\n", interval));
    
    // We can't enable timer second time
    if (g_profiler.enabled) return;
   
    // Obtain a pointer to the PWM registers.
    if (!g_pPWMRegs)
    {
        g_pPWMRegs = (S3C2450_PWM_REG *) OALPAtoVA(S3C2450_BASE_REG_PA_PWM, FALSE);
    }

    // How many hi-res ticks per profiler hit
    g_profiler.countsPerHit = (g_oalTimer.countsPerMSec * interval)/1000;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Configure profiling ISR callback function.
    g_pProfilerISR = OALProfileIntrHandler;

    // Update the compare register for the next profile hit.
    ConfigureNextProfilerCount(g_profiler.countsPerHit);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);
    irq = IRQ_TIMER2;
    OALIntrDoneIrqs(1, &irq);

    // Set flag
    g_profiler.enabled = TRUE;

    OALMSG(TRUE, (L"-OEMProfileTimerEnable\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerDisable
//
//  This function is called by kernel to stop kernel profiling timer.
//

VOID OEMProfileTimerDisable() 
{
    BOOL enabled;
    UINT32 irq;

    OALMSG(TRUE, (L"+OEMProfileTimerDisable()\r\n"));

    // No disable without enable
    if (!g_profiler.enabled) goto cleanUp;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Disable the profile timer interrupt
    irq = IRQ_TIMER2;
    OALIntrDisableIrqs(1, &irq);

    // Deconfigure profiling ISR callback function.
    g_pProfilerISR = NULL;

    // Reset flag
    g_profiler.enabled = FALSE;

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

cleanUp:
    OALMSG(TRUE, (L"-OEMProfileTimerDisable\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALProfileIntrHandler
//
//  This is timer interrupt handler which replace default handler in time when
//  kernel profiling is active. It calls original interrupt handler in
//  appropriate times.
//
UINT32 OALProfileIntrHandler(UINT32 ra)
{
    UINT32 irq;

    // Update the compare register for the next profile hit.
    ConfigureNextProfilerCount(g_profiler.countsPerHit);

    // First call profiler
    ProfilerHit(ra);

    // Enable interrupts
    irq = IRQ_TIMER2;
    OALIntrDoneIrqs(1, &irq);

    return(SYSINTR_NOP); 
}

//------------------------------------------------------------------------------


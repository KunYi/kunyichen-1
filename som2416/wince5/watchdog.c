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
//  File:  watchdog.c
//
//  Samsung SMDK2450 watchdog timer support code.
//
#include <windows.h>
#include <ceddk.h>
#include <oal.h>
#include <s3c2450.h>

//
// kernel exports
//
// woo 1021
extern void (* pfnOEMRefreshWatchDog) (void);   // function pointer to refresh watchdog
extern DWORD   dwOEMWatchDogPeriod;             // watchdog period
extern DWORD   dwNKWatchDogThreadPriority;      // watchdog thread priority, default is 100, set by kernel. OEM can adjust as desired

//
// Watchdog registers
//
#define WTCON_OFFSET            0               // address of WTCON is base of WATCHDOG + 0
#define WTCNT_OFFSET            0x08            // address of WTCNT is base of WATCHDOG + 8

// WTCON - control register, bit specifications
#define WTCON_PRESCALE(x)       ((x) << 8)      // bit 15:8, prescale value, 0 <= (x) <= 27
#define WTCON_ENABLE            0x20            // bit 5, enable watchdog timer
#define WTCON_CLOCK_SELECT(x)   ((x) << 3)      // bit 4:3, clock select
                                                //    0b00: 1/16
                                                //    0b01: 1/32
                                                //    0b10: 1/64
                                                //    0b11: 1/128
#define WTCON_RESET             0x01            // bit 0, reset if timer expired

// WTCNT - watchdog counter register
#define WTCNT_MAX_VALUE         0xffff          // max value for WTCNT = 0xffff



//
// The default setup value: prescale: 27+1, enable watchdog, enable reset, clock select 1/128.
//      With the default setup, the watchdog timer will reset if not refreshed within ~4.6 seconds.
//
#define WTCON_DEFAULT_SETUP_VALUE       (WTCON_PRESCALE(27) | WTCON_ENABLE | WTCON_CLOCK_SELECT(0x3) | WTCON_RESET)

#define WD_REFRESH_PERIOD               3000    // tell the OS to refresh watchdog every 3 second. 


//
// function to refresh watchdog timer
//
void RefreshWatchdogTimer (void)
{
    static DWORD dwWDBase = 0;      // VA for Watchdog base


    if (!dwWDBase) {
        // called the 1st time, setup the watchdog timer
        dwWDBase = (DWORD) OALPAtoVA (S3C2450_BASE_REG_PA_WATCHDOG, FALSE);

        if (!dwWDBase) {
            OALMSG (OAL_ERROR, (L"Address of Watch Dog Base Not Defined, WatchDog not enabled!\r\n"));
        } else {
            WRITE_REGISTER_USHORT (dwWDBase + WTCNT_OFFSET, WTCNT_MAX_VALUE);
            WRITE_REGISTER_USHORT (dwWDBase + WTCON_OFFSET, WTCON_DEFAULT_SETUP_VALUE);
        }

    } else {
        // subsequent refresh calls, just reset the counter register to max value
        WRITE_REGISTER_USHORT (dwWDBase + WTCNT_OFFSET, WTCNT_MAX_VALUE);
    }

}

//------------------------------------------------------------------------------
//
//  Function:  SMDKInitWatchDogTimer
//
//  This is the function to enable hardware watchdog timer support by kernel.
//
void SMDKInitWatchDogTimer (void)
{
    OALMSG(OAL_FUNC, (L"+SMDKInitWatchDogTimer\r\n"));


    pfnOEMRefreshWatchDog = RefreshWatchdogTimer;
    dwOEMWatchDogPeriod   = WD_REFRESH_PERIOD;


    OALMSG(OAL_FUNC, (L"-SMDKInitWatchDogTimer\r\n"));
}

//------------------------------------------------------------------------------

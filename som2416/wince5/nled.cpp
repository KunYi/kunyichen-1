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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

Abstract:  

Notes: 
--*/
#include <windows.h>
#include <nled.h>
#include <led_drvr.h>
#include <types.h>
#include "BSP.h"
 //[david.modify] 2008-05-21 17:00
#include <dbgmsg_david.h>
#include <xllp_gpio_david.h>


BOOL InitializeAddresses(VOID);

typedef struct {
    unsigned __int8 OnOffBlink;     // 0=off, 1=on, 2=blink
    unsigned __int8 OnTime;         // in units of 100ms
    unsigned __int8 OffTime;        // in units of 100ms
} NotificationLED;

HANDLE gLEDThread;
HANDLE gLEDEvent;
DWORD gLEDTimeout;

#define  NUM_LEDS           2

//[david.modify] 2008-05-21 16:51
//#define NLED_SUPPORT    FALSE
#define NLED_SUPPORT    TRUE
 //[david. end] 2008-05-21 16:51
stGPIOInfo g_stGPIOInfo[]={
	{ GLED,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE}
};

	

#define DBGNLED 1  //[david.modify] 2008-05-21 16:52
// Pointer to device control registers
volatile S3C2450_IOPORT_REG *v_pIOPregs;

NLED_SETTINGS_INFO g_ShadowSettingsInfo[NUM_LEDS];

CRITICAL_SECTION g_Lock; // Protects g_ShadowSettingsInfo

const struct NLED_SUPPORTS_INFO g_LEDSupportsInfo[NUM_LEDS] = {
    // 0 - LED
    { 
        0,    // LedNum
        250000,    // lCycleAdjust
        FALSE,   // fAdjustTotalCycleTime
        FALSE,    // fAdjustOnTime
        FALSE,    // fAdjustOffTime
        FALSE,    // fMetaCycleOn
        FALSE    // fMetaCycleOff
    },
    // 1 - Vibrate
    { 
        1,    // LedNum
        -1,    // lCycleAdjust (-1 indicates it is the Vibrate device)
        FALSE,    // fAdjustTotalCycleTime
        FALSE,    // fAdjustOnTime
        FALSE,    // fAdjustOffTime
        FALSE,    // fMetaCycleOn
        FALSE    // fMetaCycleOff
    }
};


BOOL InitializeAddresses(VOID)
{
	BOOL	RetValue = TRUE;
	
	/* IO Register Allocation */
	v_pIOPregs = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (v_pIOPregs == NULL) 
	{
		ERRORMSG(1,(TEXT("For IOPregs : VirtualAlloc failed!\r\n")));
		RetValue = FALSE;
	}
	else 
	{
		if (!VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
		{
			ERRORMSG(1,(TEXT("For IOPregs: VirtualCopy failed!\r\n")));
			RetValue = FALSE;
		}
	}
	if (!RetValue) 
	{
		if (v_pIOPregs) 
		{
			VirtualFree((PVOID) v_pIOPregs, 0, MEM_RELEASE);
		}

		v_pIOPregs = NULL;
		return RetValue;
	}
	InitializeCriticalSection(&g_Lock);
	return(RetValue);
}

void NLED_Thread(void)
{
	bool LED_Blink=0;

	gLEDTimeout = INFINITE;
	gLEDEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	
 	for (;;)
 	{ 	
	 	WaitForSingleObject(gLEDEvent, gLEDTimeout);

   		if (gLEDTimeout == 250)
   		{
   			LED_Blink ^= 1;
//[david.modify] 2008-05-21 17:00
//====================================
          	if (LED_Blink == TRUE) {
//   				v_pIOPregs->GPCDAT  &= ~(0x1 << 7);
			g_stGPIOInfo[0].u32Stat=0;
			DPNOK(g_stGPIOInfo[0].u32Stat);
			SetGPIOInfo(&g_stGPIOInfo[0], (void*)v_pIOPregs);
          		}
  			else{
//        		v_pIOPregs->GPCDAT  |=  (0x1 << 7);
			g_stGPIOInfo[0].u32Stat=1;
			DPNOK(g_stGPIOInfo[0].u32Stat);
			SetGPIOInfo(&g_stGPIOInfo[0], (void*)v_pIOPregs);
  			}
//====================================			
        }
 	}
}

// The NLED MDD calls this routine to initialize the underlying NLED hardware.
// This routine should return TRUE if successful.  If there's a problem
// it should return FALSE and call SetLastError() to pass back the reason
// for the failure.
BOOL WINAPI
NLedDriverInitialize(
                    VOID
                    )
{
    RETAILMSG(1, (TEXT("NLedDriverInitialize: invoked\r\n")));
    if (!InitializeAddresses())
    	return (FALSE);
#if (NLED_SUPPORT)
//[david.modify] 2008-05-21 17:00
//====================================
#if 0
    v_pIOPregs->GPCCON  &= ~(0x3 << 14);    // GPC7 Outpur // 08.04.04 KYS
    v_pIOPregs->GPCCON  |=  (0x1 << 14);
    v_pIOPregs->GPCDAT  |=  (0x1 << 7);
    v_pIOPregs->GPCUDP   |=  (0x1 << 7);
#endif
	
	SetGPIOInfo(&g_stGPIOInfo[0], (void*)v_pIOPregs);
	//g_stGPIOInfo[0].u32PinNo = GLED;  //unused code,f.w.lin
	GetGPIOInfo(&g_stGPIOInfo[0], (void*)v_pIOPregs);
	EPRINT(L"GLED=%d: [%d %d %d] \r\n",
		 g_stGPIOInfo[0].u32PinNo, 
		g_stGPIOInfo[0].u32Stat, g_stGPIOInfo[0].u32AltFunc, g_stGPIOInfo[0].u32PullUpdown);			
//====================================   
    gLEDThread= CreateThread(0, 0, (LPTHREAD_START_ROUTINE) NLED_Thread, 0, 0, 0);
#endif
    return (TRUE);
}

// The NLED MDD calls this routine to deinitialize the underlying NLED
// hardware as the NLED driver is unloaded.  It should return TRUE if 
// successful.  If there's a problem this routine should return FALSE 
// and call SetLastError() to pass back the reason for the failure.
BOOL WINAPI
NLedDriverDeInitialize(
                    VOID
                    )
{
    RETAILMSG(1, (TEXT("NLedDriverDeInitialize: invoked\r\n")));

	if (v_pIOPregs) 
	{
		VirtualFree((PVOID) v_pIOPregs, 0, MEM_RELEASE);
		v_pIOPregs = NULL;
	}

	if(gLEDThread)
	{
		CloseHandle(gLEDThread);
		gLEDThread = NULL;	
	}
	if(gLEDEvent)
	{
		CloseHandle(gLEDEvent);
		gLEDEvent = NULL;	
	}
    DeleteCriticalSection(&g_Lock);
    return (TRUE);
}

void GetLEDInfoFromHardware(const UINT LedNum, NLED_SETTINGS_INFO *pInfo)
{
	RETAILMSG(DBGNLED,(TEXT("++GetLEDInfoFromHardware\r\n")));
    EnterCriticalSection(&g_Lock);
    memcpy(pInfo, &g_ShadowSettingsInfo[LedNum], sizeof(*pInfo));
    LeaveCriticalSection(&g_Lock);
    RETAILMSG(DBGNLED,(TEXT("--GetLEDInfoFromHardware\r\n")));
}

BOOL SetLEDInfoToHardware(const NLED_SETTINGS_INFO *pInfo)
{
    UINT LedNum = pInfo->LedNum;
    LONG OnTime = pInfo->OnTime;      // Convert from units of 1ms to units of 100ms
    LONG OffTime = pInfo->OffTime;    // Convert from units of 1ms to units of 100ms
    INT OffOnBlink = pInfo->OffOnBlink;
	RETAILMSG(DBGNLED,(TEXT("++SetLEDInfoToHardware\r\n")));
    RETAILMSG(DBGNLED,(TEXT("LednNum=%d, pInfo->OnTime=%d, pInfo->OffTime=%d, pInfo->OffOnBlink=%d\r\n"),pInfo->LedNum, pInfo->OnTime, pInfo->OffTime, pInfo->OffOnBlink));
    // Validate the pInfo fields
    if (LedNum >= NUM_LEDS
        || OnTime < 0 
        || OffTime < 0
        || OffOnBlink < 0
        || OffOnBlink > 2
        || (pInfo->MetaCycleOn != 0 && OnTime == 0) // allow MetaCycleOn=1 only if OnTime != 0
        || pInfo->MetaCycleOff != 0) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    // The order matters:  the write to OnOffBlink should come at the end, as it
    // is what triggers the emulator to update how it displays notifications.
    EnterCriticalSection(&g_Lock);
    memcpy(&g_ShadowSettingsInfo[LedNum], pInfo, sizeof(*pInfo));
    g_ShadowSettingsInfo[LedNum].TotalCycleTime = pInfo->OnTime + pInfo->OffTime;
    
    if(LedNum == 0)
    {
    	RETAILMSG(DBGNLED,(TEXT("Chanage LED 0 Configuration\r\n"))); 
    	if(OffOnBlink == 0)  // Led Off
    	{
    		gLEDTimeout = INFINITE;
//		    v_pIOPregs->GPCDAT  |=  (0x1 << 7);
			g_stGPIOInfo[0].u32Stat=1;
			DPNOK(g_stGPIOInfo[0].u32Stat);
			SetGPIOInfo(&g_stGPIOInfo[0], (void*)v_pIOPregs);
		}
		else if(OffOnBlink == 1) // Led On
		{
			gLEDTimeout = INFINITE;
//     	    v_pIOPregs->GPCDAT  &= ~(0x1 << 7);
			g_stGPIOInfo[0].u32Stat=0;
			DPNOK(g_stGPIOInfo[0].u32Stat);
			SetGPIOInfo(&g_stGPIOInfo[0], (void*)v_pIOPregs);
     	}
     	else if(OffOnBlink == 2) // Led Blink
     	{
     		gLEDTimeout = 250;
    		SetEvent(gLEDEvent);
    	}
    	else 
    	{
    		RETAILMSG(DBGNLED,(TEXT("LED0 Invalid Parameter\r\n")));
			SetLastError(ERROR_INVALID_PARAMETER);
			return (FALSE);
    	}
    }
    else if (LedNum == 1)
    {
		RETAILMSG(DBGNLED,(TEXT("Chanage LED 1(Vibrator) Configuration\r\n")));
		if(OffOnBlink == 0) //Vibrator off
		{
			RETAILMSG(DBGNLED,(TEXT("Vibrator Off\r\n")));
		}   
		else if(OffOnBlink == 1) //Vibrator On  
		{
			RETAILMSG(DBGNLED,(TEXT("Vibrator On\r\n")));	
		}
		else
		{
			RETAILMSG(DBGNLED,(TEXT("Vibrator Invalid Parameter\r\n")));
			SetLastError(ERROR_INVALID_PARAMETER);
			return (FALSE);			
		}
    }
    else
    {
    		SetLastError(ERROR_INVALID_PARAMETER);
			return (FALSE);
    }
    LeaveCriticalSection(&g_Lock);
	RETAILMSG(DBGNLED,(TEXT("--SetLEDInfoToHardware\r\n")));
    return TRUE;
}

// This routine retrieves information about the NLED device(s) that
// this driver supports.  The nInfoId parameter indicates what specific
// information is being queried and pOutput is a buffer to be filled in.
// The size of pOutput depends on the type of data being requested.  This
// routine returns TRUE if successful, or FALSE if there's a problem -- in
// which case it also calls SetLastError() to pass back more complete
// error information.  The NLED MDD invokes this routine when an application
// calls NLedGetDeviceInfo().
BOOL
WINAPI
NLedDriverGetDeviceInfo(
                       INT     nInfoId,
                       PVOID   pOutput // note: this is an IN/OUT parameter
                       )
{
    BOOL fOk = TRUE;
    SETFNAME(_T("NLedDriverGetDeviceInfo"));
	RETAILMSG(DBGNLED,(TEXT("++NLedDriverGetDeviceInfo\r\n")));
    if ( nInfoId == NLED_COUNT_INFO_ID ) {
        struct NLED_COUNT_INFO  *p = (struct NLED_COUNT_INFO*)pOutput;
		RETAILMSG(DBGNLED,(TEXT("NLedDriverGetDeviceInfo:: NLED_COUNT_INFO_ID\r\n")));
        __try {
            p -> cLeds = NUM_LEDS;
        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(ERROR_INVALID_PARAMETER);
            fOk = FALSE;
        }
    } else if ( nInfoId == NLED_SUPPORTS_INFO_ID ) {
        __try {
            RETAILMSG(DBGNLED,(TEXT("NLedDriverGetDeviceInfo:: NLED_SUPPORTS_INFO_ID\r\n")));
            UINT LedNum = ((NLED_SUPPORTS_INFO*)pOutput)->LedNum;
             if (LedNum < NUM_LEDS) {
                memcpy(pOutput, &g_LEDSupportsInfo[LedNum], sizeof(NLED_SUPPORTS_INFO));
            } else {
                fOk = FALSE;
                SetLastError(ERROR_INVALID_PARAMETER);
            }
        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(ERROR_INVALID_PARAMETER);
            fOk = FALSE;
        }
    } else if ( nInfoId == NLED_SETTINGS_INFO_ID ) {
        NLED_SETTINGS_INFO Info;
        UINT LedNum;
		RETAILMSG(DBGNLED,(TEXT("++NLedDriverGetDeviceInfo:: NLED_SETTINGS_INFO_ID\r\n")));
        __try {
            LedNum = ((NLED_SETTINGS_INFO*)pOutput)->LedNum;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(ERROR_INVALID_PARAMETER);
            fOk = FALSE;
        }
		if (fOk) {
            if (LedNum < NUM_LEDS) {
                GetLEDInfoFromHardware(LedNum, &Info);
            } else {
                fOk = FALSE;
                SetLastError(ERROR_INVALID_PARAMETER);
            }
        }

        if (fOk) {
            Info.LedNum = LedNum;
            __try {
                memcpy(pOutput, &Info, sizeof(Info));
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                SetLastError(ERROR_INVALID_PARAMETER);
                fOk = FALSE;
            }
        }
    	RETAILMSG(DBGNLED,(TEXT("--NLedDriverGetDeviceInfo:: NLED_SETTINGS_INFO_ID\r\n")));
    } else {
        fOk = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
    }
	RETAILMSG(DBGNLED,(TEXT("--NLedDriverGetDeviceInfo\r\n")));
    DEBUGMSG(ZONE_PDD || (!fOk && ZONE_WARN), 
        (_T("%s: returning %d\r\n"), TEXT(__FUNCTION__), fOk));
    return (fOk);
}


// This routine changes the configuration of an LED.  The nInfoId parameter
// indicates what kind of configuration information is being changed.  
// Currently only the NLED_SETTINGS_INFO_ID value is supported.  The pInput
// parameter points to a buffer containing the data to be updated.  The size
// of the buffer depends on the value of nInfoId.  This routine returns TRUE
// if successful or FALSE if there's a problem -- in which case it also calls
// SetLastError().  The NLED MDD invokes this routine when an application 
// calls NLedSetDevice().
BOOL
WINAPI
NLedDriverSetDevice(
                   INT     nInfoId,
                   PVOID   pInput
                   )
{
    BOOL fOk = TRUE;
	RETAILMSG(DBGNLED,(TEXT("++NLedDriverSetDevice\r\n")));
    if ( nInfoId == NLED_SETTINGS_INFO_ID ) {
        struct NLED_SETTINGS_INFO Info;
        __try {
            memcpy(&Info, pInput, sizeof(Info));
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(ERROR_INVALID_PARAMETER);
            fOk = FALSE;
        }
        if (fOk) {
        	RETAILMSG(DBGNLED,(TEXT("LedNum=%d, OnTime=%d, OffTime=%d, OffOnBlink=%d\r\n"),Info.LedNum, Info.OnTime, Info.OffTime, Info.OffOnBlink));
        	            fOk = SetLEDInfoToHardware(&Info);
        }
    } else {
        fOk = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    RETAILMSG(DBGNLED,(TEXT("--NLedDriverSetDevice\r\n")));
    return (fOk);
}


// This routine is invoked by the driver MDD when the system suspends or
// resumes.  The power_down flag indicates whether the system is powering 
// up or powering down.
VOID WINAPI
NLedDriverPowerDown(
                   BOOL power_down
                   )
{
    return;
}


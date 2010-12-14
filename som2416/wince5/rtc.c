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
//  Module: rtc.c
//
//  Real-time clock (RTC) routines for the Samsung S3C2450 processor
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <s3c2450.h>

//------------------------------------------------------------------------------
// Defines 

#define FROM_BCD(n)     ((((n) >> 4) * 10) + ((n) & 0xf))
#define TO_BCD(n)       ((((n) / 10) << 4) | ((n) % 10))
#define RTC_YEAR_DATUM  1980

#define RTC_FUNC 0



//------------------------------------------------------------------------------
//
//  External:  g_oalRtcResetTime
//
//  RTC init time after a RTC reset has occured. It should
//  be defined in platform code.

static SYSTEMTIME g_oalRtcResetTime = {2008, 6, 0, 1, 12, 0, 0, 0};

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot.
//  Input buffer contains SYSTEMTIME structure with default time value.
//  If hardware has persistent real time clock it will ignore this value
//  (or all call).
//
BOOL OALIoCtlHalInitRTC(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize
) {
    BOOL rc = FALSE;
    BOOL bResetRTC = FALSE;
    SYSTEMTIME SysTime;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

	memset(&SysTime,0,sizeof(SYSTEMTIME));    
    
    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalInitRTC: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

	OEMGetRealTime(&SysTime);

    /* RTC Time validity check */
    bResetRTC = (SysTime.wYear  < RTC_YEAR_DATUM    || (SysTime.wYear - RTC_YEAR_DATUM) > 99)   ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wMonth > 12                || SysTime.wMonth < 1)                      ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wDay   > 31                || SysTime.wDay < 1)                        ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wDayOfWeek > 6             || SysTime.wDayOfWeek < 0)                  ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wHour  > 23                || SysTime.wHour < 0)                       ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wMinute > 59               || SysTime.wMinute < 0)                     ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wSecond > 59               || SysTime.wSecond < 0)                     ? TRUE : bResetRTC;

    if(bResetRTC)
    {
        OALMSG(OAL_RTC&&OAL_ERROR,(L"Invalid RTC Time (%d.%d.%d, %d:%d:%d, (%d th day of week)\r\n", \
            SysTime.wYear, SysTime.wMonth,  SysTime.wDay,    \
            SysTime.wHour, SysTime.wMinute, SysTime.wSecond,    \
            SysTime.wDayOfWeek
            ));
    }

    // Set time
    if(bResetRTC)
    {
    	rc = OEMSetRealTime(&g_oalRtcResetTime);
    }
       
    
cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}



//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  Reads the current RTC value and returns a system time.
//
BOOL OEMGetRealTime(SYSTEMTIME *pTime)
{
    BOOL rc = FALSE;
    S3C2450_RTC_REG *pRTCReg;
    UINT32 data;
    UINT16 seconds;


    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OEMGetRealTime(pTime = 0x%x)\r\n", pTime));

    if (pTime == NULL) goto cleanUp;
    
    // Get uncached virtual address
    pRTCReg = OALPAtoVA(S3C2450_BASE_REG_PA_RTC, FALSE);
    do {
        data = INPORT32(&pRTCReg->BCDSEC) & 0x7f;
        seconds = FROM_BCD(data);
        data = INPORT32(&pRTCReg->BCDYEAR);
        pTime->wYear = FROM_BCD(data) + RTC_YEAR_DATUM;		
        data = INPORT32(&pRTCReg->BCDMON) & 0x1f;	
        pTime->wMonth = FROM_BCD(data);
        data = INPORT32(&pRTCReg->BCDDATE) & 0x3f;
        pTime->wDay = FROM_BCD(data);
        pTime->wDayOfWeek = (WORD)INPORT32(&pRTCReg->BCDDAY) - 1;
        data = INPORT32(&pRTCReg->BCDHOUR) & 0x3f;
        pTime->wHour = FROM_BCD(data);
        data = INPORT32(&pRTCReg->BCDMIN) & 0x7f;
        pTime->wMinute = FROM_BCD(data);
        data = INPORT32(&pRTCReg->BCDSEC) & 0x7f;
        pTime->wSecond = FROM_BCD(data);
        pTime->wMilliseconds = 0;
    } while (pTime->wSecond != seconds);

    // Done
    rc = TRUE;

cleanUp:
     RETAILMSG(RTC_FUNC, (TEXT("-OEMGetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n"), 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));
    //OALMSG(OAL_FUNC, (L"-OEMGetRealTime(rc = %d)\r\n", rc));
	OALMSG(OAL_FUNC, (L"."));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  Updates the RTC with the specified system time.
//


BOOL OEMSetRealTime(LPSYSTEMTIME pTime) 
{
    BOOL rc = FALSE;
    S3C2450_RTC_REG *pRTCReg;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));
    
    RETAILMSG(1, (TEXT("+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n"), 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));
    if (pTime == NULL) goto cleanUp;
    
    // The RTC will only support a BCD year value of 0 - 99.  The year datum is
    // 1980, so any dates greater than 2079 will fail unless the datum is
    // adjusted.
    if (pTime->wYear < RTC_YEAR_DATUM || (pTime->wYear - RTC_YEAR_DATUM) > 99) {
        OALMSG(OAL_ERROR, (L"ERROR: OEMSetRealTime: "
            L"RTC cannot support a year greater than %d or less than %d "
            L"(value %d)\r\n", (RTC_YEAR_DATUM + 99), RTC_YEAR_DATUM, 
            pTime->wYear
        ));
        goto cleanUp;
    }

    // Get uncached virtual address
    pRTCReg = OALPAtoVA(S3C2450_BASE_REG_PA_RTC, FALSE);

    // Enable RTC control.
    SETREG32(&pRTCReg->RTCCON, 1);

    OUTPORT32(&pRTCReg->BCDSEC,  TO_BCD(pTime->wSecond));
    OUTPORT32(&pRTCReg->BCDMIN,  TO_BCD(pTime->wMinute));
    OUTPORT32(&pRTCReg->BCDHOUR, TO_BCD(pTime->wHour));
    OUTPORT32(&pRTCReg->BCDDAY, pTime->wDayOfWeek + 1);
    OUTPORT32(&pRTCReg->BCDDATE,  TO_BCD(pTime->wDay));
    OUTPORT32(&pRTCReg->BCDMON,  TO_BCD(pTime->wMonth));
    OUTPORT32(&pRTCReg->BCDYEAR, TO_BCD(pTime->wYear - RTC_YEAR_DATUM));

    // Disable RTC control.
    CLRREG32(&pRTCReg->RTCCON, 1);
    
    // Done
    rc = TRUE;

	RETAILMSG(RTC_FUNC,(TEXT("-OEMSetRealTime(rc=%d)\r\n"),rc));

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetRealTime(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  Set the RTC alarm time.
//
BOOL OEMSetAlarmTime(SYSTEMTIME *pTime)
{
    BOOL rc = FALSE;
    S3C2450_RTC_REG *pRTCReg;
    S3C2450_INTR_REG *pIntrReg;
    UINT32 irq;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wMonth, pTime->wDay, pTime->wYear, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));
    
    RETAILMSG(RTC_FUNC,(TEXT("+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n"), 
        pTime->wMonth, pTime->wDay, pTime->wYear, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    if (pTime == NULL) goto cleanUp;
    
    // Get uncached virtual address
    pRTCReg = OALPAtoVA(S3C2450_BASE_REG_PA_RTC, FALSE);
    pIntrReg = OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);
    
    // Enable RTC control
    SETREG32(&pRTCReg->RTCCON, 1);

    OUTPORT32(&pRTCReg->ALMSEC,  TO_BCD(pTime->wSecond));
    OUTPORT32(&pRTCReg->ALMMIN,  TO_BCD(pTime->wMinute));
    OUTPORT32(&pRTCReg->ALMHOUR, TO_BCD(pTime->wHour));
    OUTPORT32(&pRTCReg->ALMDATE, TO_BCD(pTime->wDay));
    OUTPORT32(&pRTCReg->ALMMON,  TO_BCD(pTime->wMonth));
    OUTPORT32(&pRTCReg->ALMYEAR, TO_BCD(pTime->wYear - RTC_YEAR_DATUM));
   
    // Enable the RTC alarm interrupt
    OUTPORT32(&pRTCReg->RTCALM, 0x7F);
 
    // Disable RTC control.
    CLRREG32(&pRTCReg->RTCCON, 1);

    // Enable/clear RTC interrupt
    irq = IRQ_RTC;
    OALIntrDoneIrqs(1, &irq);

    // Done
    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------

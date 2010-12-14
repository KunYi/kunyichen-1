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

    Drv.c   Power Controller Driver

Abstract:

   Streams interface driver (MDD)

Functions:

Notes:

--*/

#include <windows.h>
#include <winreg.h>
#include <winioctl.h>
#include <ceddk.h>
#include <devload.h>

#include <types.h>
#include <excpt.h>
#include <tchar.h>
#include <cardserv.h>
#include <cardapi.h>
#include <tuple.h>
#include <diskio.h>
#include <nkintr.h>
#include <windev.h>
#include <winbase.h>

#include "pwr.h"

#ifndef SHIP_BUILD
DBGPARAM dpCurSettings = {
    TEXT("POWER"), {
    TEXT("Error"),     TEXT("Warn"),    TEXT("Init"),  TEXT("Open"),
    TEXT("Read"),      TEXT("Write"),   TEXT("IOCTL"), TEXT("IST"),
    TEXT("Registers"), TEXT("RegDump"), TEXT("ADC"),   TEXT("Battery"),
    TEXT("Events"),    TEXT("Power"),      TEXT("14"),    TEXT("Trace"),
    },
    0x0003 // ZONE_WRN|1
};
#endif  // DEBUG

BOOL
PWR_PowerUp(
   PVOID Context
   );

BOOL
PWR_PowerDown(
   PVOID Context
   );


BOOL
DllEntry(
    HINSTANCE   hinstDll,             /*@parm Instance pointer. */
    DWORD   dwReason,                 /*@parm Reason routine is called. */
    LPVOID  lpReserved                /*@parm system parameter. */
    )
{
    if ( dwReason == DLL_PROCESS_ATTACH ) {
        DEBUGREGISTER(hinstDll);
        RETAILMSG (1, (TEXT("PWR: Process Attach\r\n")));
    }

    if ( dwReason == DLL_PROCESS_DETACH ) {
        RETAILMSG (1, (TEXT("PWR: Process Detach\r\n")));
    }

    return(TRUE);
}


BOOL
PWR_Deinit(
   PPWR_CONTEXT pPWR
   )
{
    RETAILMSG(1, (TEXT(">PWR_Deinit\r\n")));

    if (!pPWR)
        return FALSE;
        
    HW_Deinit(pPWR);

    LocalFree(pPWR);

    RETAILMSG(1, (TEXT("<PWR_Deinit\r\n")));

    return TRUE;
}


/*++

Called by Device Manager to initialize the streams interface in response to ActivateDevice.
We passed ActivateDevice a pointer to our device context, but must read it out of the registry as "ClientInfo".

Returns context used in XXX_Open, XXX_PowerDown, XXX_PowerUp, and XXX_Deinit

--*/
PPWR_CONTEXT
PWR_Init(
   PVOID Context
   )
{
	LPTSTR ActivePath = (LPTSTR)Context; // HKLM\Drivers\Active\xx
	PPWR_CONTEXT pPWR;
	BOOL bRc = FALSE;

	RETAILMSG(1, (TEXT(">PWR_Init(%p)\r\n"), ActivePath));

	// Allocate for our main data structure and one of it's fields.
	pPWR = (PPWR_CONTEXT)LocalAlloc( LPTR, sizeof(PWR_CONTEXT) );
	if ( !pPWR )
		return( NULL );

	pPWR->Sig = PCF_SIG;

	// init h/w
	if ( ERROR_SUCCESS != HW_Init(pPWR) )
		goto ALLOCFAILED;

	pPWR->Dx = D0;

	RETAILMSG(1, (TEXT("<PWR_Init:0x%x\r\n"), pPWR ));

	return (pPWR);

ALLOCFAILED:
	PWR_Deinit(pPWR);

	return NULL;
}


PPWR_CONTEXT
PWR_Open(
   PPWR_CONTEXT pPWR,       // context returned by PWR_Init.
   DWORD        AccessCode, // @parm access code
   DWORD        ShareMode   // @parm share mode
   )
{
    UNREFERENCED_PARAMETER(ShareMode);
    UNREFERENCED_PARAMETER(AccessCode);

    RETAILMSG(1,(TEXT(">PWR_Open(0x%x, 0x%x, 0x%x)\r\n"),pPWR, AccessCode, ShareMode));

    pPWR->OpenCount++;

    HW_Open(pPWR);

    RETAILMSG(1,(TEXT("<PWR_Open:%u\r\n"), pPWR->OpenCount ));

    return pPWR;
}


BOOL
PWR_Close(
   PPWR_CONTEXT pPWR
   )
{
   RETAILMSG(1,(TEXT("PWR_Close(0x%x)\r\n"),pPWR));

    if ( pPWR->OpenCount ) {
        
        pPWR->OpenCount--;

        HW_Close(pPWR);
    }

    return TRUE;
}


ULONG
PWR_Write(
   PPWR_CONTEXT pPWR,
   PUCHAR pBuffer,
   ULONG  BufferLength
   )
{
    return 0;
}


ULONG
PWR_Read(
   PPWR_CONTEXT pPWR,
   PUCHAR pBuffer,
   ULONG  BufferLength
   )
{
   return 0;
}


BOOL
PWR_IOControl(
    PPWR_CONTEXT pPWR,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
   )
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL bRc = TRUE;

    RETAILMSG(1,(TEXT(">PWR_IOControl(0x%x, 0x%x, %d, 0x%x)\r\n"),
        dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut ));

    if ( !pPWR->OpenCount ) {
        RETAILMSG (1,(TEXT(" COM_IOControl - device was closed\r\n")));
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    switch (dwCode) {
        //
        // Power Management
        //
        case IOCTL_POWER_CAPABILITIES:
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(POWER_CAPABILITIES)) ) {
                bRc = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            
            bRc = HW_PowerCapabilities(pPWR, (PPOWER_CAPABILITIES)pBufOut);
            if ( bRc ) {
                *pdwActualOut = sizeof(POWER_CAPABILITIES);
            }
            break;

        case IOCTL_POWER_SET: 
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
                bRc = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            bRc = HW_PowerSet(pPWR, (PCEDEVICE_POWER_STATE)pBufOut);
            if ( bRc ) {
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            }
            break;

        case IOCTL_POWER_GET: 
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
                bRc = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            bRc = HW_PowerGet(pPWR, (PCEDEVICE_POWER_STATE)pBufOut);
            if ( bRc ) {
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            }
            break;

        default:
            // passthruogh
            bRc  = HW_IOControl(pPWR, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
            break;            
    }

    if ( !bRc ) {
        RETAILMSG (1, (TEXT("PWR_IOControl ERROR: %u\r\n"), dwErr));
        SetLastError(dwErr);
    }

    RETAILMSG(1,(TEXT("<PWR_IOControl:%d\r\n"), bRc));

    return bRc;
}


ULONG
PWR_Seek(
   PVOID Context,
   LONG  Position,
   DWORD Type
   )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Position);
    UNREFERENCED_PARAMETER(Type);
    RETAILMSG( 1, (TEXT("PWR_Seek\r\n")));
    return (ULONG)-1;
}


BOOL
PWR_PowerUp(
   PVOID Context
   )
{
    return HW_PowerUp(Context);
}


BOOL
PWR_PowerDown(
   PVOID Context
   )
{
    return HW_PowerDown(Context);
}

// EOF

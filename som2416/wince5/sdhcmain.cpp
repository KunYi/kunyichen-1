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

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <s3c2450.h>
#include "SDCardDDK.h"
#include <SDHCD.h>
#include <bsp.h>
#include "SDIOControllerBase.h"



    // initialize debug zones
SD_DEBUG_INSTANTIATE_ZONES(
     TEXT("SDHC_SC2450"), // module name
     ZONE_ENABLE_INIT | ZONE_ENABLE_ERROR | ZONE_ENABLE_WARN, 
     TEXT("Interrupts"),
     TEXT("Send Handler "), 
     TEXT("Responses"), 
     TEXT("Receive Data"),                   
     TEXT("Clock Control"), 
     TEXT("Transmit Data"), 
     TEXT(""), 
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT(""));


#define SDHCD_SLOTS 1
#define SDHCD_REGISTRY_BASE_PATH TEXT("Drivers\\SDCARD\\HostController\\SDHCD")

BOOL g_fRegisteredWithBusDriver;

///////////////////////////////////////////////////////////////////////////////
//  SDHDCardDllEntry - the main dll entry point
//  Input:  hInstance - the instance that is attaching
//          Reason - the reason for attaching
//          pReserved - 
//  Output: 
//  Return: TRUE
//  Notes:  this is only used to initialize the zones
///////////////////////////////////////////////////////////////////////////////
BOOL DllEntry(HINSTANCE hInstance, ULONG Reason, LPVOID pReserved)
{
    BOOL fRet = TRUE;

    if(Reason == DLL_PROCESS_ATTACH) 
	{
        DEBUGREGISTER(hInstance);
        DisableThreadLibraryCalls( (HMODULE) hInstance );

        if( !SDInitializeCardLib() )
        {
            fRet = FALSE;
        }
        else if( !SD_API_SUCCESS( SDHCDInitializeHCLib() ) ) 
        {
            SDDeinitializeCardLib();
            fRet = FALSE;
        }

        g_fRegisteredWithBusDriver = FALSE;
    }

    if(Reason == DLL_PROCESS_DETACH) 
	{	
        SDHCDDeinitializeHCLib();
        SDDeinitializeCardLib();
    }

    return(TRUE);
}



///////////////////////////////////////////////////////////////////////////////
//  SDH_Init - the init entry point 
//  Input:  dwContext - the context for this init
//  Output: 
//  Return: returns instance context
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
extern "C"
DWORD SDH_Init(DWORD dwContext)
{
    DWORD                   dwRet = 0;           // return value    
    PSDCARD_HC_CONTEXT      pHostContext = NULL; // new HC context
    SD_API_STATUS           status;              // SD status
    LPCTSTR pszActiveKey = (LPCTSTR) dwContext;
    CSDIOControllerBase* pController = NULL;

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC +Init\r\n")));

    ASSERT( g_fRegisteredWithBusDriver == FALSE );

    // Allocate the context
	status = SDHCDAllocateContext(SDHCD_SLOTS/*, sizeof(SDHCD_HARDWARE_CONTEXT)*/, &pHostContext);

    if(!SD_API_SUCCESS(status)) 
	{
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD: Failed to allocate context : 0x%08X \r\n"), status));
        goto EXIT;
    }

    // Create the SD Host Controller object
    pController = CreateSDIOController( pHostContext );
    if( pController == NULL ) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC Failed to allocate SD Host Controller object\r\n")));
        goto EXIT;
    }

    // Set our extension
    pHostContext->pHCSpecificContext = pController;

    // Read SD Host Controller Info from registry.
    if (!pController->InterpretCapabilities((LPCTSTR)dwContext)) {
        goto EXIT;
    }

    // now register the host controller 
    status = SDHCDRegisterHostController(pHostContext);

    if(!SD_API_SUCCESS(status)) 
	{
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD: Failed to register host controller: %0x08X \r\n"),status));
        goto EXIT;
    }
    g_fRegisteredWithBusDriver = TRUE;

    // return the controller context
    dwRet = (DWORD)pController;
    
EXIT:
    if( dwRet == 0 )
    {
        if( pHostContext ) 
        {
            // deregister the host controller
            if (g_fRegisteredWithBusDriver) 
            {
                SDHCDDeregisterHostController(pHostContext);
                g_fRegisteredWithBusDriver = FALSE;
            }

            // delete the SD Host Controller object
            if( pController )
            {
                delete pController;
            }

            // cleanup the host context
            SDHCDDeleteContext(pHostContext);
        }
    }

    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SDHC -Init\r\n")));
    
    return dwRet;
}


///////////////////////////////////////////////////////////////////////////////
//  SDH_PreDeinit - the predeinit entry point 
//  Input:  hDeviceContext - the context returned from SDH_Init
//  Output: 
//  Return: always returns TRUE
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
extern "C"
BOOL SDH_PreDeinit(DWORD hDeviceContext)
{
    CSDIOControllerBase *pController = (CSDIOControllerBase*)hDeviceContext;

    if( g_fRegisteredWithBusDriver )
    {
        PSDCARD_HC_CONTEXT pHostContext = pController->GetHostContext();

        pController->PreDeinit();

        // deregister the host controller
        SDHCDDeregisterHostController(pHostContext);

        g_fRegisteredWithBusDriver = FALSE;
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_Deinit - the deinit entry point 
//  Input:  hDeviceContext - the context returned from SDH_Init
//  Output: 
//  Return: always returns TRUE
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
extern "C"
BOOL SDH_Deinit(DWORD hDeviceContext)
{
    CSDIOControllerBase *pController = (CSDIOControllerBase*)hDeviceContext;
    PSDCARD_HC_CONTEXT pHostContext = pController->GetHostContext();

    if( g_fRegisteredWithBusDriver )
    {
        // deregister the host controller
        SDHCDDeregisterHostController(pHostContext);

        g_fRegisteredWithBusDriver = FALSE;
    }

    // delete the SD Host Controller object
    delete pController;

    // Cleanup the host context
    SDHCDDeleteContext(pHostContext);

    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
//  SDH_IOControl - the I/O control entry point
//  Input:  hOpenContext - the context returned from SDH_Open
//          dwCode - the ioctl code
//          pBufIn - the input buffer from the user
//          dwLenIn - the length of the input buffer
//          pBufOut - the output buffer from the user
//          dwLenOut - the length of the output buffer
//          pdwActualOut - the size of the transfer
//  Output: 
//  Return: FALSE
//  Notes:  Not used
///////////////////////////////////////////////////////////////////////////////
extern "C"
BOOL SDH_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
                   PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("SDHCD: +-SDH_IOControl \r\n")));
    return FALSE;;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_Open - the open entry point for the bus driver
//  Input:  hDeviceContext - the device context from SDH_Init
//          AccessCode - the desired access
//          ShareMode - the desired share mode
//  Output: 
//  Return: 0
//  Notes:  not used
///////////////////////////////////////////////////////////////////////////////
extern "C"
DWORD SDH_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("SDHCD: +-SDH_Open\r\n")));
    CSDIOControllerBase *pController = (CSDIOControllerBase*)hDeviceContext;
    return (DWORD) pController;
}



///////////////////////////////////////////////////////////////////////////////
//  SDH_Close - the close entry point 
//  Input:  hOpenContext - the context returned from SDH_Open
//  Output: 
//  Return: TRUE
//  Notes:  not used
///////////////////////////////////////////////////////////////////////////////
extern "C"
BOOL SDH_Close(DWORD hOpenContext)
{
	DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("SDHCD: +-SDH_Close\r\n")));
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_PowerDown - the power down entry point
//  Input:  hDeviceContext - the device context from SDH_Init
//  Output: 
//  Notes:  preforms no actions
///////////////////////////////////////////////////////////////////////////////
extern "C"
void SDH_PowerDown(DWORD hDeviceContext)
{
	DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("SDHCD: +-SDH_PowerDown\r\n")));
    CSDIOControllerBase *pController = (CSDIOControllerBase*)hDeviceContext;
    if( pController )
    {
        pController->OnPowerDown();
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
//  SDH_PowerUp - the power up entry point 
//  Input:  hDeviceContext - the device context from SDH_Init
//  Output: 
//  Notes:  preforms no actions
///////////////////////////////////////////////////////////////////////////////
extern "C"
void SDH_PowerUp(DWORD  hDeviceContext)
{
	DEBUGMSG(SDCARD_ZONE_FUNC, (TEXT("SDHCD: +-SDH_PowerUp\r\n")));
    CSDIOControllerBase *pController = (CSDIOControllerBase*)hDeviceContext;
    if( pController )
    {
        pController->OnPowerUp();
    }
    return;
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

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
#include <creg.hxx>
#include <ddkreg.h>
#include "SDCardDDK.h"
#include <SDHCD.h>
#include <bsp.h>
#include "SDIOController.h"
#include <S3C2450REF_GPIO.h>

///////////////////////////////////////////////////////////////////////////////
//  CSDIOController::CSDIOController - CSDIOController constructor
//  Input:  pHCContext - hardware context
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
CSDIOController::CSDIOController( PSDCARD_HC_CONTEXT pHCContext )
: CSDIOControllerBase( pHCContext )
{
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOController::~CSDIOController - CSDIOController destructor
//  Input:  
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
CSDIOController::~CSDIOController()
{
}

#define CARD_DETECT_GPIO_TEXT TEXT("CardDetectGPIO")
#define CARD_DETECT_MASK_TEXT TEXT("CardDetectMask")
#define CARD_DETECT_FLAG_TEXT TEXT("CardDetectFlag")
#define CARD_DETECT_CONTROL_MASK_TEXT TEXT("CardDetectControlMask")
#define CARD_DETECT_CONTROL_FLAG_TEXT TEXT("CardDetectControlFlag")
#define CARD_DETECT_PULLUP_MASK_TEXT TEXT("CardDetectPullupMask")
#define CARD_DETECT_PULLUP_FLAG_TEXT TEXT("CardDetectPullupFlag")

#define CARD_READWRITE_GPIO_TEXT TEXT("CardReadWriteGPIO")
#define CARD_READWRITE_MASK_TEXT TEXT("CardReadWriteMask")
#define CARD_READWRITE_FLAG_TEXT TEXT("CardReadWriteFlag")
#define CARD_READWRITE_CONTROL_MASK_TEXT TEXT("CardReadWriteControlMask")
#define CARD_READWRITE_CONTROL_FLAG_TEXT TEXT("CardReadWriteControlFlag")
#define CARD_READWRITE_PULLUP_MASK_TEXT TEXT("CardReadWritePullupMask")
#define CARD_READWRITE_PULLUP_FLAG_TEXT TEXT("CardReadWritePullupFlag")

BOOL CSDIOController::CustomSetup( LPCTSTR pszRegistryPath )
{
    BOOL fRetVal = TRUE;
    CReg regDevice; // encapsulated device key
    HKEY hKeyDevice = OpenDeviceKey(pszRegistryPath);
    if ( (hKeyDevice == NULL) || !regDevice.Open(hKeyDevice, NULL) ) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("CSDIOControllerBase::InterpretCapabilities: Failed to open device key\r\n")));
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }

    // read the card detect GPIO settings
    LPCTSTR pszCardDetectGPIO = regDevice.ValueSZ( CARD_DETECT_GPIO_TEXT );
    if( pszCardDetectGPIO && pszCardDetectGPIO[0] >= TEXT('a') && pszCardDetectGPIO[0] <= TEXT('h') )
    {
        m_chCardDetectGPIO = (char)pszCardDetectGPIO[0] - ('a'-'A');
    }
    else if( pszCardDetectGPIO && pszCardDetectGPIO[0] >= TEXT('A') && pszCardDetectGPIO[0] <= TEXT('H') )
    {
        m_chCardDetectGPIO = (char)pszCardDetectGPIO[0];
    }
    else
    {
        // invalid SDIO SYSINTR value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("invalid SDIO SYSINTR value!\r\n")));
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }
    m_dwCardDetectMask = regDevice.ValueDW( CARD_DETECT_MASK_TEXT );
    m_dwCardDetectFlag = regDevice.ValueDW( CARD_DETECT_FLAG_TEXT );
    m_dwCardDetectControlMask = regDevice.ValueDW( CARD_DETECT_CONTROL_MASK_TEXT );
    m_dwCardDetectControlFlag = regDevice.ValueDW( CARD_DETECT_CONTROL_FLAG_TEXT );
    m_dwCardDetectPullupMask = regDevice.ValueDW( CARD_DETECT_PULLUP_MASK_TEXT );
    m_dwCardDetectPullupFlag = regDevice.ValueDW( CARD_DETECT_PULLUP_FLAG_TEXT );

    // read the card read/write GPIO settings
    LPCTSTR pszCardReadWriteGPIO = regDevice.ValueSZ( CARD_READWRITE_GPIO_TEXT );
    if( pszCardReadWriteGPIO && pszCardReadWriteGPIO[0] >= TEXT('a') && pszCardReadWriteGPIO[0] <= TEXT('h') )
    {
        m_chCardReadWriteGPIO = (char)pszCardReadWriteGPIO[0] - ('a'-'A');
    }
    else if( pszCardReadWriteGPIO || pszCardReadWriteGPIO[0] >= TEXT('A') && pszCardReadWriteGPIO[0] <= TEXT('H') )
    {
        m_chCardReadWriteGPIO = (char)pszCardReadWriteGPIO[0];
    }
    else
    {
        // invalid SDIO SYSINTR value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("invalid SDIO SYSINTR value!\r\n")));
        if (hKeyDevice) RegCloseKey(hKeyDevice);
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }
    m_dwCardReadWriteMask = regDevice.ValueDW( CARD_READWRITE_MASK_TEXT );
    m_dwCardReadWriteFlag = regDevice.ValueDW( CARD_READWRITE_FLAG_TEXT );
    m_dwCardReadWriteControlMask = regDevice.ValueDW( CARD_READWRITE_CONTROL_MASK_TEXT );
    m_dwCardReadWriteControlFlag = regDevice.ValueDW( CARD_READWRITE_CONTROL_FLAG_TEXT );
    m_dwCardReadWritePullupMask = regDevice.ValueDW( CARD_READWRITE_PULLUP_MASK_TEXT );
    m_dwCardReadWritePullupFlag = regDevice.ValueDW( CARD_READWRITE_PULLUP_FLAG_TEXT );

FUNCTION_EXIT:
    if (hKeyDevice) RegCloseKey(hKeyDevice);
    return fRetVal;
}

BOOL CSDIOController::InitializeHardware( BOOL bOnPowerUp )
{
    // Configure GPIO pin to detect WRITE-PROTECT status
    switch( m_chCardReadWriteGPIO )
    {
    case 'A':
//        vm_pIOPreg->GPACON  = ( vm_pIOPreg->GPACON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
        break;
    case 'B':
        vm_pIOPreg->GPBCON  = ( vm_pIOPreg->GPBCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPBUDP   = ( vm_pIOPreg->GPBUDP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'C':
        vm_pIOPreg->GPCCON  = ( vm_pIOPreg->GPCCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPCUDP   = ( vm_pIOPreg->GPCUDP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'D':
        vm_pIOPreg->GPDCON  = ( vm_pIOPreg->GPDCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPDUDP   = ( vm_pIOPreg->GPDUDP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'E':
        vm_pIOPreg->GPECON  = ( vm_pIOPreg->GPECON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPEUDP   = ( vm_pIOPreg->GPEUDP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'F':
        vm_pIOPreg->GPFCON  = ( vm_pIOPreg->GPFCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPFUDP   = ( vm_pIOPreg->GPFUDP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'G':
        vm_pIOPreg->GPGCON  = ( vm_pIOPreg->GPGCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPGUDP   = ( vm_pIOPreg->GPGUDP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'H':
        vm_pIOPreg->GPHCON  = ( vm_pIOPreg->GPHCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPHUDP   = ( vm_pIOPreg->GPHUDP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    default:
        ASSERT(0); // invalid GPIO! We should never get here!
        return FALSE;
    }

    // Configure GPIO pin to detect CARD PRESENT status
    switch( m_chCardDetectGPIO )
    {
    case 'A':
        break;
    case 'B':
	    vm_pIOPreg->GPBUDP   = ( vm_pIOPreg->GPBUDP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'C':
	    vm_pIOPreg->GPCUDP   = ( vm_pIOPreg->GPCUDP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'D':
	    vm_pIOPreg->GPDUDP   = ( vm_pIOPreg->GPDUDP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'E':
	    vm_pIOPreg->GPEUDP   = ( vm_pIOPreg->GPEUDP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'F':
	    vm_pIOPreg->GPFUDP   = ( vm_pIOPreg->GPFUDP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'G':
	    vm_pIOPreg->GPGUDP   = ( vm_pIOPreg->GPGUDP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'H':
	    vm_pIOPreg->GPHUDP   = ( vm_pIOPreg->GPHUDP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    default:
        ASSERT(0); // invalid GPIO! We should never get here!
        return FALSE;
    }
#if 1
	// updated for supporting 2450 EVT1 by JJG 061106
#ifdef EVT1
	vm_pIOPreg->EXTINT0 = (READEXTINT0(vm_pIOPreg->EXTINT0) & ~(0xF<<4)) | (0xF<<4);
#else
	vm_pIOPreg->EXTINT0 = (vm_pIOPreg->EXTINT0 & ~(0xF<<4)) | (0xF<<4);
#endif
	vm_pIOPreg->GPFCON &= ~(3<<2);
	vm_pIOPreg->GPFCON  |= (2<<2);
#endif	
    return TRUE;
}

void CSDIOController::DeinitializeHardware( BOOL bOnPowerDown )
{
    // nothing to do
}

BOOL CSDIOController::IsCardWriteProtected()
{
    switch( m_chCardReadWriteGPIO )
    {
    case 'A':
//        return ( ( vm_pIOPreg->GPADAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'B':
        return ( ( vm_pIOPreg->GPBDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'C':
        return ( ( vm_pIOPreg->GPCDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'D':
        return ( ( vm_pIOPreg->GPDDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'E':
        return ( ( vm_pIOPreg->GPEDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'F':
        return ( ( vm_pIOPreg->GPFDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'G':
        return ( ( vm_pIOPreg->GPGDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'H':
        return ( ( vm_pIOPreg->GPHDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    default:
        ASSERT(0); // invalid GPIO!  We should never get here
        return TRUE;
    }
}

BOOL CSDIOController::IsCardPresent()
{
    UINT32 dTemp = 0;
	UINT32 dRetVal = 0;
    switch( m_chCardDetectGPIO )
    {
    case 'A':
//	dTemp = vm_pIOPreg->GPACON;
//	vm_pIOPreg->GPACON  = ( vm_pIOPreg->GPACON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
//       ( ( vm_pIOPreg->GPADAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? dRetVal = TRUE  : dRetVal = FALSE;
//	vm_pIOPreg->GPACON = dTemp;
	return dRetVal;
    case 'B':
	dTemp = vm_pIOPreg->GPBCON;
	vm_pIOPreg->GPBCON  = ( vm_pIOPreg->GPBCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
       ( ( vm_pIOPreg->GPBDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? dRetVal = TRUE  : dRetVal = FALSE;
	vm_pIOPreg->GPBCON = dTemp;
	return dRetVal;
    case 'C':
	dTemp = vm_pIOPreg->GPCCON;
	vm_pIOPreg->GPCCON  = ( vm_pIOPreg->GPCCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
       ( ( vm_pIOPreg->GPCDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? dRetVal = TRUE  : dRetVal = FALSE;
	vm_pIOPreg->GPCCON = dTemp;
	return dRetVal;
    case 'D':
	dTemp = vm_pIOPreg->GPDCON;
	vm_pIOPreg->GPDCON  = ( vm_pIOPreg->GPDCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
       ( ( vm_pIOPreg->GPDDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? dRetVal = TRUE  : dRetVal = FALSE;
	vm_pIOPreg->GPDCON = dTemp;
	return dRetVal;
    case 'E':
	dTemp = vm_pIOPreg->GPECON;
	vm_pIOPreg->GPECON  = ( vm_pIOPreg->GPECON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
       ( ( vm_pIOPreg->GPEDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? dRetVal = TRUE  : dRetVal = FALSE;
	vm_pIOPreg->GPECON = dTemp;
	return dRetVal;
    case 'F':
	dTemp = vm_pIOPreg->GPFCON;
	vm_pIOPreg->GPFCON  = ( vm_pIOPreg->GPFCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
       ( ( vm_pIOPreg->GPFDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? dRetVal = TRUE  : dRetVal = FALSE;
	vm_pIOPreg->GPFCON = dTemp;
	return dRetVal;
    case 'G':
	dTemp = vm_pIOPreg->GPGCON;
	vm_pIOPreg->GPGCON  = ( vm_pIOPreg->GPGCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
	( ( vm_pIOPreg->GPGDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? dRetVal = TRUE  : dRetVal = FALSE;
	vm_pIOPreg->GPGCON = dTemp;
	
	return dRetVal;
    case 'H':
	dTemp = vm_pIOPreg->GPHCON;
	vm_pIOPreg->GPHCON  = ( vm_pIOPreg->GPHCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
       ( ( vm_pIOPreg->GPHDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? dRetVal = TRUE  : dRetVal = FALSE;
	vm_pIOPreg->GPHCON = dTemp;
	return dRetVal;

    default:
        ASSERT(0); // invalid GPIO!  We should never get here
        return FALSE;
    }
}

CSDIOControllerBase* CreateSDIOController( PSDCARD_HC_CONTEXT pHCContext )
{
    return new CSDIOController( pHCContext );
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

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
    kitlusbser.c
    
Abstract:

    Platform specific code for USB serial KITL services.
        
Functions:


Notes: 

--*/

#include <windows.h>
#include <bsp.h>
#include <kitl_cfg.h>

static DWORD KitlIoPortBase;
volatile S3C2450_USBD_REG *g_pUSBDReg;

S3CUSB_INFO  g_Info; // record keeping
USBSERKITL_INFO USBSerInfo;

extern void S3C2450_USB_INIT(void);
extern void S3C2450_USB_EP0(void);

///////////////////////////////////////////////////////////////////////////////
// Hard coded Wait
//////////////////////////////////////////////////////////////////////////////
void WaitMS (DWORD dwMS)
{
	volatile DWORD dCount,dwIndex;

	for (dwIndex=0;dwIndex<dwMS;dwIndex++) {
		for (dCount=0;dCount<100000L;dCount++);
	}
}
/* S3C2450USBSER_Init
 *
 *  Called by PQOAL KITL framework to initialize the serial port
 *
 *  Return Value:
 */
BOOL S3C2450USBSER_Init (KITL_SERIAL_INFO *pSerInfo)
{

#if 0
    volatile S3C2450_IOPORT_REG *g_pIOPortReg;

    g_pIOPortReg = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	
	memset(&g_Info, 0, sizeof(g_Info));
	memset(&USBSerInfo, 0, sizeof(USBSERKITL_INFO));

    KITLOutputDebugString ("Wait for connecting\n");

	KitlIoPortBase = (DWORD)pSerInfo->pAddress;

	if (!KitlIoPortBase)
	{
		return FALSE;
	}
	else
	{
		g_pUSBDReg = (volatile S3C2450_USBD_REG *)OALPAtoVA(KitlIoPortBase, FALSE);

//		//Shoule re-plugin the usb cable
//		while(1)
//		{
//			if(g_pUSBDReg->PMR.usb_re )
//				break;
//		}
		
		g_pIOPortReg->GPGDAT &= ~0x1<<12;
#ifdef EVT1		
		g_pIOPortReg->GPGUDP  |= 0x1<<24;
#else
		g_pIOPortReg->GPGUDP  |= 0x2<<24;
#endif		
	
		g_pIOPortReg->GPGCON  = (g_pIOPortReg->GPGCON & 0x3<<24) | 0x1<<24;
		g_pUSBDReg->UIR.reset_int = 1;

		WaitMS(100);
		
		g_pIOPortReg->GPGDAT |= 0x1<<12;
		
		S3C2450_USB_INIT();

		while(USBSerInfo.dwState != KITLUSBSER_STATE_CONFIGURED)
		{
			if (g_pUSBDReg->UIR.reset_int)
				S3C2450_USB_INIT();
			
			g_pUSBDReg->INDEX.index = 0;
			S3C2450_USB_EP0();
		}
		
	}

    pSerInfo->bestSize = 64;

    //KITLOutputDebugString ("-S3C2450USBSER_Kitl_Init\n");
#endif
    return TRUE;
}


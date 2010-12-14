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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------
#include <windows.h>
#include <bsp.h>
#include <kitl_cfg.h>

#define KITL_DBON 1

//#define KITL_USBRNDIS

OAL_KITL_DEVICE g_kitlDevice;

const OAL_KITL_SERIAL_DRIVER *GetKitlSerialDriver (void);
//const OAL_KITL_SERIAL_DRIVER *GetKitlUSBSerialDriver (void);
static OAL_KITL_ETH_DRIVER g_kitlEthCS8900A = OAL_ETHDRV_CS8900A;


volatile S3C2450_IOPORT_REG *g_pIOPortReg;

BOOL USBSerKitl_POLL = FALSE;

#ifdef KITL_USBRNDIS
static OAL_KITL_ETH_DRIVER g_kitlUSBRndis = OAL_ETHDRV_RNDIS;
#endif

#ifdef KITL_ETHERNET
BOOL InitKitlEtherArgs (OAL_KITL_ARGS *pKitlArgs)
{
	RETAILMSG(1,(TEXT("InitKitlEtherArgs\n")));
   	// init flags
   	pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED | OAL_KITL_FLAGS_VMINI | OAL_KITL_FLAGS_POLL;
#ifdef CS8900A_KITL_POLLMODE
   	pKitlArgs->flags |= OAL_KITL_FLAGS_POLL;
#endif //CS8900A_KITL_POLLMODE
#ifdef CS8900A_KITL_DHCP
    pKitlArgs->flags |= OAL_KITL_FLAGS_DHCP;
#endif //CS8900A_KITL_DHCP

    pKitlArgs->devLoc.IfcType    	= Internal;
   	pKitlArgs->devLoc.BusNumber    	= 0;
   	pKitlArgs->devLoc.LogicalLoc    = BSP_BASE_REG_PA_CS8900A_IOBASE;  // base address
   	pKitlArgs->devLoc.Pin           = 0;
    
   	OALKitlStringToMAC(CS8900A_MAC,pKitlArgs->mac);

#ifndef CS8900A_KITL_DHCP
   	pKitlArgs->ipAddress            = OALKitlStringToIP(CS8900A_IP_ADDRESS);
   	pKitlArgs->ipMask            	= OALKitlStringToIP(CS8900A_IP_MASK);
   	pKitlArgs->ipRoute            	= OALKitlStringToIP(CS8900A_IP_ROUTER);
#endif CS8900A_KITL_DHCP

	g_kitlDevice.ifcType			= Internal;
   	g_kitlDevice.type               = OAL_KITL_TYPE_ETH;
   	g_kitlDevice.pDriver            = (void *)&g_kitlEthCS8900A;



	//setting EINT13 as IRQ_LAN
	if (!(pKitlArgs->flags & OAL_KITL_FLAGS_POLL))
	{
		/*
    	g_pIOPortReg = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
		g_pIOPortReg->GPGCON = (g_pIOPortReg->GPGCON & ~(0x3<<10)) | (0x2<<10);
		g_pIOPortReg->GPGUDP |= 0x1<<10;
		g_pIOPortReg->EXTINT1 = (g_pIOPortReg->EXTINT1 & ~(0xf<<20)) | (0x1<<20);
		*/
	}
    return TRUE;
}
#endif

#ifdef KITL_SERIAL
BOOL InitKitlSerialArgs (OAL_KITL_ARGS *pKitlArgs)
{
	DWORD dwIoBase = UART_Kitl;

    // init flags
    pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED | OAL_KITL_FLAGS_POLL;

    pKitlArgs->devLoc.LogicalLoc    = dwIoBase;
    pKitlArgs->devLoc.Pin           = OAL_INTR_IRQ_UNDEFINED;
    pKitlArgs->baudRate             = CBR_115200;
    pKitlArgs->dataBits             = DATABITS_8;
    pKitlArgs->parity               = PARITY_NONE;
    pKitlArgs->stopBits             = STOPBITS_10;

    g_kitlDevice.type               = OAL_KITL_TYPE_SERIAL;
    g_kitlDevice.pDriver            = (VOID*) GetKitlSerialDriver ();
    
    return TRUE;
}
#endif

#ifdef KITL_USBSERIAL
BOOL InitKitlUSBSerialArgs (OAL_KITL_ARGS *pKitlArgs)
{

	DWORD dwIoBase = S3C2450_BASE_REG_PA_USBD;

    // init flags
    pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED|OAL_KITL_FLAGS_POLL;
#ifdef USBSER_KITL_POLL
    pKitlArgs->flags |= OAL_KITL_FLAGS_POLL;
    USBSerKitl_POLL = TRUE;
#endif

    pKitlArgs->devLoc.LogicalLoc    = dwIoBase;
    pKitlArgs->devLoc.Pin           = IRQ_USBD;
 
    g_kitlDevice.type               = OAL_KITL_TYPE_SERIAL;

    g_kitlDevice.pDriver            = (VOID*) GetKitlUSBSerialDriver ();
    
    return TRUE;
}

//-----------------------------------------------------------------------------------------------------
#endif

void KITL_PortInit(void)
{
	volatile S3C2450_SSMC_REG *pSSMCReg;	
		//configure ssmc for cs8900a
   	pSSMCReg = (S3C2450_SSMC_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_SSMC, FALSE);
	pSSMCReg->SMBIDCYR1 = 0;
	pSSMCReg->SMBWSTWRR1 = 14;
	pSSMCReg->SMBWSTOENR1 = 2;
	pSSMCReg->SMBWSTWENR1 = 2;
	pSSMCReg->SMBWSTRDR1 = 14;  //14clk
	
	
	pSSMCReg->SMBCR1 |= ((1<<2)|(1<<0));
	pSSMCReg->SMBCR1 &= ~((3<<20)|(3<<20)|(3<<12));
	pSSMCReg->SMBCR1 &= ~(3<<4);
	pSSMCReg->SMBCR1 |= (1<<4) | (1<<7) | (1<<15);  
}
//-----------------------------------------------------------------------------------------------------

#ifdef KITL_USBRNDIS
BOOL InitKitlUSBRndisArgs (OAL_KITL_ARGS *pKitlArgs)
{

	KITLOutputDebugString ("+++InitKitlUSBRndisArgs Start\n");

	
	pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED|OAL_KITL_FLAGS_VMINI|OAL_KITL_FLAGS_POLL;
   
	pKitlArgs->devLoc.IfcType = Internal;
	pKitlArgs->devLoc.Pin     = 0;
	pKitlArgs->devLoc.LogicalLoc =  (DWORD)(S3C2450_BASE_REG_PA_USBD);
       pKitlArgs->devLoc.Pin           = IRQ_USBD;

   	OALKitlStringToMAC(USBRNDIS_MAC,pKitlArgs->mac);

   	pKitlArgs->ipAddress            = OALKitlStringToIP(USBRNDIS_IP_ADDRESS);
   	pKitlArgs->ipMask            	= OALKitlStringToIP(USBRNDIS_IP_MASK);
   	pKitlArgs->ipRoute            	= OALKitlStringToIP(USBRNDIS_IP_ROUTER);

	
	pKitlArgs->devLoc.PhysicalLoc =  (PVOID)pKitlArgs->devLoc.LogicalLoc;

  	g_kitlDevice.name = L"2450USBRNDIS";
   	g_kitlDevice.ifcType = pKitlArgs->devLoc.IfcType;
   	g_kitlDevice.resource = 0;
   	g_kitlDevice.type = OAL_KITL_TYPE_ETH;
   	g_kitlDevice.pDriver = (void *)&g_kitlUSBRndis;
    	
    return TRUE;
}
#endif

BOOL OALKitlStart()
{
    OAL_KITL_ARGS   kitlArgs, *pArgs;
    BOOL            fRet = FALSE;
    UCHAR			*szDeviceId,buffer[OAL_KITL_ID_SIZE]="\0";

    memset (&kitlArgs, 0, sizeof (kitlArgs));

    pArgs = (OAL_KITL_ARGS *)OALArgsQuery(OAL_ARGS_QUERY_KITL);
    szDeviceId = (CHAR*)OALArgsQuery(OAL_ARGS_QUERY_DEVID);

    // common parts
    kitlArgs.devLoc.IfcType = g_kitlDevice.ifcType
                            = InterfaceTypeUndefined;
    g_kitlDevice.name       = g_oalIoCtlPlatformType;

    KITLOutputDebugString ("OALKitlStart : ");

    
#ifdef KITL_SERIAL
    KITLOutputDebugString ("SERIAL\n");
	fRet = InitKitlSerialArgs (&kitlArgs);
	strcpy(buffer,"2450SerKitl");
	szDeviceId = buffer;
    g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;
#endif //KITL_SERIAL

#ifdef KITL_USBSERIAL
    KITLOutputDebugString ("USB SERIAL\n");
	fRet = InitKitlUSBSerialArgs (&kitlArgs);
	strcpy(buffer,"2450USBSerKitl");
	szDeviceId = buffer;
    g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;
#endif //KITL_USBSERIAL

#ifdef KITL_USBRNDIS
    KITLOutputDebugString ("KITL USBRNDIS\n");

    fRet = InitKitlUSBRndisArgs (&kitlArgs);

	OALKitlCreateName(BSP_DEVICE_PREFIX, kitlArgs.mac, buffer);

	szDeviceId = buffer;
	g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;

    strcpy(buffer,"2450USBRndKitl");
    szDeviceId = buffer;
    g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;
#endif //KITL_USBRNDIS


#ifdef KITL_ETHERNET
    KITLOutputDebugString ("ETHERNET\n");
	KITL_PortInit();

//	if (pArgs->devLoc.LogicalLoc == 0)
	if (pArgs == NULL)
	{	
		KITLOutputDebugString ("pArgs = NULL\n");
		fRet = InitKitlEtherArgs (&kitlArgs);
		OALKitlCreateName(BSP_DEVICE_PREFIX, kitlArgs.mac, buffer);
		szDeviceId = buffer;
		g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;
	}
	else
	{
    	KITLOutputDebugString ("Kitl args bring from argument setting of RAM\n");
		g_kitlDevice.ifcType			= Internal;
			g_kitlDevice.id					= BSP_BASE_REG_PA_CS8900A_IOBASE;  // base address
			g_kitlDevice.type			   = OAL_KITL_TYPE_ETH;
			g_kitlDevice.pDriver			= (void *)&g_kitlEthCS8900A;
		
		memcpy(&kitlArgs, pArgs, sizeof (kitlArgs));
		kitlArgs.flags |= OAL_KITL_FLAGS_POLL;
		OALKitlCreateName(BSP_DEVICE_PREFIX, kitlArgs.mac, buffer);
		szDeviceId = buffer;
	
		fRet = TRUE;
	}
	 RETAILMSG(KITL_DBON, (
		L"DeviceId................. %hs\r\n", szDeviceId
	));
	RETAILMSG(KITL_DBON, (
		L"kitlArgs.flags............. 0x%x\r\n", kitlArgs.flags
	));
	RETAILMSG(KITL_DBON, (
		L"kitlArgs.devLoc.IfcType.... %d\r\n",   kitlArgs.devLoc.IfcType
	));
	RETAILMSG(KITL_DBON, (
		L"kitlArgs.devLoc.LogicalLoc. 0x%x\r\n", kitlArgs.devLoc.LogicalLoc
	));
	RETAILMSG(KITL_DBON, (
		L"kitlArgs.devLoc.PhysicalLoc 0x%x\r\n", kitlArgs.devLoc.PhysicalLoc
	));
	RETAILMSG(KITL_DBON, (
		L"kitlArgs.devLoc.Pin........ %d\r\n",   kitlArgs.devLoc.Pin
	));
	RETAILMSG(KITL_DBON, (
		L"kitlArgs.ip4address........ %s\r\n",   OALKitlIPtoString(kitlArgs.ipAddress)
	));
#endif //KITL_ETHERNET

	if (fRet == FALSE)
	{
	    KITLOutputDebugString ("NONE\n");
		return FALSE;
	}
	
	if ((kitlArgs.flags & OAL_KITL_FLAGS_ENABLED) == 0)
	RETAILMSG(1, (TEXT("KITL is disabled.\r\n")));
	else
	RETAILMSG(1, (TEXT("KITL is enabeld.\r\n")));
	KITLOutputDebugString ("Call OALKitlInit : ");
	OALKitlInit (szDeviceId, &kitlArgs, &g_kitlDevice);
	return fRet;
}



//------------------------------------------------------------------------------
//
//  Function:  OALGetTickCount
//
//  This function is called by some KITL libraries to obtain relative time
//  since device boot. It is mostly used to implement timeout in network
//  protocol.
//

UINT32 OALGetTickCount()
{
    static ULONG count = 0;

    count++;
    return count/100;
}

// Define a dummy SetKMode function to satisfy the NAND FMD.
//
DWORD SetKMode (DWORD fMode)
{
    return(1);
}

DWORD OEMEthGetSecs(void)
{
    SYSTEMTIME sTime;

    OEMGetRealTime(&sTime);
    return((60UL * (60UL * (24UL * (31UL * sTime.wMonth + sTime.wDay) + sTime.wHour) + sTime.wMinute)) + sTime.wSecond);
}

//------------------------------------------------------------------------------


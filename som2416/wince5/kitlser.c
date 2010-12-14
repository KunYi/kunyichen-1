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
    kitlser.c
    
Abstract:

    Platform specific code for serial KITL services.
        
Functions:


Notes: 

--*/

#include <windows.h>
#include <bsp.h>
#include <kitl_cfg.h>

static DWORD KitlIoPortBase;
volatile S3C2450_UART_REG *g_pUARTReg;

/* S3C2450UART_Init
 *
 *  Called by PQOAL KITL framework to initialize the serial port
 *
 *  Return Value:
 */
BOOL S3C2450UART_Init (KITL_SERIAL_INFO *pSerInfo)
{
    volatile S3C2450_IOPORT_REG *g_pIOPortReg;

    g_pIOPortReg = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	
	
    //KITLOutputDebugString ("+S3C2450UART_Kitl_Initx\n");
    //KITLOutputDebugString ("   pAddress = 0x%x\n", pSerInfo->pAddress);
    //KITLOutputDebugString ("   BaudRate = 0x%x\n", pSerInfo->baudRate);
    //KITLOutputDebugString ("   DataBits = 0x%x\n", pSerInfo->dataBits);
    //KITLOutputDebugString ("   StopBits = 0x%x\n", pSerInfo->stopBits);
    //KITLOutputDebugString ("   Parity   = 0x%x\n", pSerInfo->parity);

	KitlIoPortBase = (DWORD)pSerInfo->pAddress;

	if (!KitlIoPortBase)
	{
		return 0;
	}
	else if (KitlIoPortBase == S3C2450_BASE_REG_PA_UART0)
	{
    	// GPH0 and GPH1 are UART0 TXD0 and RXD0, respectively.
    	//
    	g_pIOPortReg->GPHCON = (g_pIOPortReg->GPHCON & ~((3 << 0)|(3 << 2))) | ((2 << 0)|(2 << 2));

    	// Disable pull-up on TXD0 and RXD0.
  	  	g_pIOPortReg->GPHUDP |= (1 << 0)|(1 << 2);
    }
    else if (KitlIoPortBase == S3C2450_BASE_REG_PA_UART1)
    {
    	// GPH2 and GPH3 are UART1 TXD1 and RXD1, respectively.
    	//
    	g_pIOPortReg->GPHCON = (g_pIOPortReg->GPHCON & ~((3 << 4)|(3 << 6))) | ((2 << 4)|(2 << 6));

    	// Disable pull-up on TXD1 and RXD1.
    	//
    	g_pIOPortReg->GPHUDP |= (1 << 4)|(1 << 6);
    }

    g_pUARTReg = (S3C2450_UART_REG *)OALPAtoVA(KitlIoPortBase,FALSE);

    // Configure the UART.
    //
    g_pUARTReg->UFCON = BSP_UART_UFCON;
    g_pUARTReg->UMCON = BSP_UART_UMCON;
    g_pUARTReg->ULCON = BSP_UART_ULCON;
    g_pUARTReg->UCON  = BSP_UART_UCON;
    g_pUARTReg->UBRDIV= BSP_UART_UBRDIV;

    pSerInfo->bestSize = 1;       // read it one by one

    //KITLOutputDebugString ("-S3C2450UART_Kitl_Init\n");

    return 1;
}

/* S3C2450UART_WriteData
 *
 *  Block until the byte is sent
 *
 *  Return Value: TRUE on success, FALSE otherwise
 */
UINT16 S3C2450UART_WriteData (UINT8 *pch, UINT16 length)
{
    if (!KitlIoPortBase) 
    {
        length = 0;
	}
	else
	{
        DEBUGCHK (length == 1);
    	// Wait for transmit buffer to be empty
    	while ((g_pUARTReg->UTRSTAT & 0x02) == 0);

	    // Send character
    	g_pUARTReg->UTXH = *pch;
	}
    return length;
}


/* S3C2450UART_ReadData
 *
 *  Called from PQOAL KITL to read a byte from serial port
 *
 *  Return Value: TRUE on success, FALSE otherwise
 */
UINT16 S3C2450UART_ReadData (UINT8 *pch, UINT16 length)
{
    UCHAR uStatus;
    UINT16 count = 0;

    if (KitlIoPortBase) 
    {
	    uStatus = g_pUARTReg->UTRSTAT;
    	if ((uStatus & 0x01) != 0) 
    	{
       		*pch = (UINT8) g_pUARTReg->URXH;
			count = 1;
    	}

	}
    return count;
}


/* S3C2450UART_EnableInt
 *
 *  Enable Recv data interrupt
 *
 *  Return Value:
 */
VOID S3C2450UART_EnableInt (void)
{
    // polling, no interrupt
}

/* S3C2450UART_DisableInt
 *
 *  Disable Recv data interrupt
 *
 *  Return Value:
 */
VOID S3C2450UART_DisableInt (void)
{
    // polling, no interrupt
}

void S3C2450UART_PowerOff(void)
{
	return;
}

void S3C2450UART_PowerOn(void)
{
    volatile S3C2450_IOPORT_REG *g_pIOPortReg;
    g_pIOPortReg = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    
	OEMInitDebugSerial();
	KITLOutputDebugString ("+S3C2450UART_PowerOn\n");
	
	
	if (KitlIoPortBase == S3C2450_BASE_REG_PA_UART0)
	{
    	// GPH0 and GPH1 are UART0 TXD0 and RXD0, respectively.
    	//
    	g_pIOPortReg->GPHCON = (g_pIOPortReg->GPHCON & ~((3 << 0)|(3 << 2))) | ((2 << 0)|(2 << 2));

    	// Disable pull-up on TXD0 and RXD0.
    	//
    	g_pIOPortReg->GPHUDP |= (1 << 0)|(1 << 2);
    }
    else if (KitlIoPortBase == S3C2450_BASE_REG_PA_UART1)
    {
    	// GPH2 and GPH3 are UART1 TXD1 and RXD1, respectively.
    	//
    	g_pIOPortReg->GPHCON = (g_pIOPortReg->GPHCON & ~((3 << 4)|(3 << 6))) | ((2 << 4)|(2 << 6));

    	// Disable pull-up on TXD1 and RXD1.
    	//
    	g_pIOPortReg->GPHUDP |= (1 << 4)|(1 << 6);
    }

    // Configure the UART.
    //
    g_pUARTReg->UFCON = BSP_UART_UFCON;
    g_pUARTReg->UMCON = BSP_UART_UMCON;
    g_pUARTReg->ULCON = BSP_UART_ULCON;
    g_pUARTReg->UCON  = BSP_UART_UCON;
    g_pUARTReg->UBRDIV= BSP_UART_UBRDIV;

}


// serial driver
OAL_KITL_SERIAL_DRIVER DrvSerial = {
    S3C2450UART_Init,
    NULL,
    S3C2450UART_WriteData,
    NULL,
    S3C2450UART_ReadData,
    S3C2450UART_EnableInt,
    S3C2450UART_DisableInt,
    S3C2450UART_PowerOff,
    S3C2450UART_PowerOn,
    NULL,
};

const OAL_KITL_SERIAL_DRIVER *GetKitlSerialDriver (void)
{
    return &DrvSerial;
}

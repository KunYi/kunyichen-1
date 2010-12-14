/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2003  Microsoft Corporation

Module Name:	DMA.H

Abstract:		Definitions for the DMA controller found on the Samsung 2450 CPU.
  
Notes:			

Environment:	
-----------------------------------------------------------------------------*/


#include <windows.h>

//===================== Register Configuration Constants ======================

//----- Register definitions for DISRCn control register -----
//
// bits[30-0] = start address of source data to transfer



//----- Register definitions for DIDSTn control register -----
//
// bits[30-0] = start address of destination for the transfer



// bits[26-24] = select DMA source for the respective channel:
//------------------------------------------------------------
#define XDREQ0_DMA0							0x00000000
#define UART0_DMA0							0x01000000
#define MMC_DMA0							0x02000000
#define TIMER_DMA0							0x03000000
#define USB_EP1_DMA0						0x04000000
#define I2SSDO_DMA0							0x05000000
#define AC97PCMIN_DMA0						0x06000000	// AC97 PCMIN


#define XDREQ1_DMA1							0x00000000
#define UART1_DMA1							0x01000000
#define I2SSDI_DMA1							0x02000000
#define SPI_DMA1							0x03000000
#define USB_EP2_DMA1						0x04000000
#define AC97PCMOUT_DMA1						0x05000000	// AC97 PCMOUT
#define MMC_DMA1							0x06000000

#define I2SSDO_DMA2							0x00000000
#define I2SSDI_DMA2							0x01000000
#define MMC_DMA2							0x02000000
#define TIMER_DMA2							0x03000000
#define USB_EP3_DMA2						0x04000000
#define AC97PCMIN_DMA2						0x05000000	// AC97 PCMIN
#define AC97MICIN_DMA2						0x06000000	// AC97 MICIN

#define UART2_DMA3							0x00000000
#define MMC_DMA3							0x01000000
#define SPI_DMA3							0x02000000
#define TIMER_DMA3							0x03000000
#define USB_EP4_DMA3						0x04000000
#define AC97MICIN_DMA3						0x05000000	// AC97 MICIN
#define AC97PCMOUT_DMA3						0x06000000	// AC97 PCMOUT
//------------------------------------------------------------


//
// bits[19-0] = used to specify the number of transfer operations in a DMA request



//
// bits[19-0] = the current transfer count


//----- Register definitions for DCSRCn configuration register -----
//
// bits[30-0] = current source address for DMA channel n


//----- Register definitions for DCDSTn configuration register -----
//
// bits[30-0] = current destination address for DMA channel n




//=============================================================================


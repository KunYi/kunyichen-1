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
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

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

//------------------------------------------------------------
//----- Register definitions for DISRCCn control register -----
//
#define SOURCE_PERIPHERAL_BUS				(1<<1)
#define FIXED_SOURCE_ADDRESS				(1<<0)

//------------------------------------------------------------
//----- Register definitions for DIDSTn control register -----
//
// bits[30-0] = start address of destination for the transfer

//------------------------------------------------------------
//----- Register definitions for DIDSTCn control register -----
//
#define DESTINATION_PERIPHERAL_BUS		SOURCE_PERIPHERAL_BUS
#define FIXED_DESTINATION_ADDRESS			FIXED_SOURCE_ADDRESS

//------------------------------------------------------------
//----- Register definitions for DCONn control register -----
//
#define HANDSHAKE_MODE					(1<<31)
#define DREQ_DACK_SYNC						(1<<30)		// 0:APB, 1:AHB bus sync
#define GENERATE_INTERRUPT					(1<<29)
#define SELECT_BURST_TRANSFER				(1<<28)
#define SELECT_WHOLE_SERVICE_MODE		(1<<27)
#define NO_DMA_AUTO_RELOAD				(1<<22)

// bits[21-20] = select transfer word size
#define TRANSFER_BYTE						(0<<20)				// 8  bits
#define TRANSFER_HALF_WORD				(1<<20)				// 16 bits
#define TRANSFER_WORD						(2<<20)				// 32 bits

// bits[19-0] = used to specify the number of transfer operations in a DMA request


//------------------------------------------------------------
//----- Register definitions for DSTATn status register -----
#define DMA_TRANSFER_IN_PROGRESS			(1<<20)
//
// bits[19-0] = the current transfer count


//------------------------------------------------------------
//----- Register definitions for DCSRCn configuration register -----
//
// bits[30-0] = current source address for DMA channel n


//------------------------------------------------------------
//----- Register definitions for DCDSTn configuration register -----
//
// bits[30-0] = current destination address for DMA channel n


//------------------------------------------------------------
//----- Register definitions for DMASKTRIGn configuration register -----
//DMASKTRIGn
#define STOP_DMA_TRANSFER					(1<<2)
#define ENABLE_DMA_CHANNEL				(1<<1)
#define DMA_SW_TRIGGER					(1<<0)


//------------------------------------------------------------
// DMAREQSEL

// bits[5:1] = used to specify which DMA is selected.

#define DMA_TRIGGERED_BY_HARDWARE		(1<<0)

//=============================================================================



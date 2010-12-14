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
/******************************************************************************
 *
 * System On Chip(SOC)
 *
 * Copyright (c) 2002 Software Center, Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information of Samsung 
 * Electronics, Inc("Confidential Information"). You Shall not disclose such 
 * Confidential Information and shall use it only in accordance with the terms 
 * of the license agreement you entered into Samsung.
 *      
 ******************************************************************************
*/
    
#include <windows.h>
#include <halether.h>
#include "cs8900dbg.h"

// Hash creation constants.
//
#define CRC_PRIME               0xFFFFFFFF;
#define CRC_POLYNOMIAL          0x04C11DB6;

#define IOREAD(o)					((USHORT)*((volatile USHORT *)(dwEthernetIOBase + (o))))
#define IOWRITE(o, d)				*((volatile USHORT *)(dwEthernetIOBase + (o))) = (USHORT)(d)

#define MEMREAD(o)					((USHORT)*((volatile USHORT *)(dwEthernetMemBase + (o))))
#define MEMWRITE(o, d)				*((volatile USHORT *)(dwEthernetMemBase + (o))) = (USHORT)(d)

#define MAX_COUNT					0x100000

#define CS8900DBG_PROBE				(1 << 0)

static DWORD dwEthernetIOBase;
static DWORD dwEthernetMemBase;

#define CS8900_MEM_MODE

#ifdef	CS8900_MEM_MODE

#define READ_REG1					ReadReg
#define READ_REG2					MEMREAD

#define WRITE_REG1					WriteReg
#define WRITE_REG2					MEMWRITE

#else

#define READ_REG1					ReadReg
#define READ_REG2					ReadReg

#define WRITE_REG1					WriteReg
#define WRITE_REG2					WriteReg

#endif

static	BOOL bIsPacket;


static USHORT 
ReadReg(USHORT offset)
{
	IOWRITE(IO_PACKET_PAGE_POINTER, offset);
	return IOREAD(IO_PACKET_PAGE_DATA_0);
}

static void 
WriteReg(USHORT offset, USHORT data)
{
	IOWRITE(IO_PACKET_PAGE_POINTER, offset);
	IOWRITE(IO_PACKET_PAGE_DATA_0 , data);
}


/*
    @func   BYTE | CalculateHashIndex | Computes the logical addres filter hash index value.  This used when there are multiple
	                                    destination addresses to be filtered.
    @rdesc  Hash index value.
    @comm    
    @xref   
*/
BYTE CalculateHashIndex( BYTE  *pMulticastAddr )
{
   DWORD CRC;
   BYTE  HashIndex;
   BYTE  AddrByte;
   DWORD HighBit;
   int   Byte;
   int   Bit;

   // Prime the CRC.
   CRC = CRC_PRIME;

   // For each of the six bytes of the multicast address.
   for ( Byte=0; Byte<6; Byte++ )
   {
      AddrByte = *pMulticastAddr++;

      // For each bit of the byte.
      for ( Bit=8; Bit>0; Bit-- )
      {
         HighBit = CRC >> 31;
         CRC <<= 1;

         if ( HighBit ^ (AddrByte & 1) )
         {
            CRC ^= CRC_POLYNOMIAL;
            CRC |= 1;
         }

         AddrByte >>= 1;
      }
   }

   // Take the least significant six bits of the CRC and copy them
   // to the HashIndex in reverse order.
   for( Bit=0,HashIndex=0; Bit<6; Bit++ )
   {
      HashIndex <<= 1;
      HashIndex |= (BYTE)(CRC & 1);
      CRC >>= 1;
   }

   return(HashIndex);
}


static BOOL Probe(void)
{
	BOOL r = FALSE;

	do 
	{
		/* Check the EISA registration number.	*/
		if (READ_REG1(PKTPG_EISA_NUMBER) != CS8900_EISA_NUMBER)
		{	RETAILMSG(1, (TEXT("0x%X\n"),READ_REG1(PKTPG_EISA_NUMBER)));
			RETAILMSG(1, (TEXT("ERROR: Probe: EISA Number Error.\r\n")));
			break;
		}
		/* Check the Product ID.				*/
		if ((READ_REG1(PKTPG_PRDCT_ID_CODE) & CS8900_PRDCT_ID_MASK)
			!= CS8900_PRDCT_ID)
		{
			RETAILMSG(1, (TEXT("ERROR: Probe: Product ID Error.\r\n")));
			break;
		}
	   
		RETAILMSG(1, (TEXT("INFO: Probe: CS8900 is detected.\r\n")));
		r = TRUE;
	} while (0);

	return r;
}

static BOOL 
Reset(void)
{
	BOOL r = FALSE;
	USHORT dummy;
	int i;
											/* Set RESET bit of SelfCTL register.	*/
	do 
	{
		//WRITE_REG1(PKTPG_SELF_CTL, SELF_CTL_RESET | SELF_CTL_LOW_BITS);
		WRITE_REG1(PKTPG_SELF_CTL, SELF_CTL_RESET);

								/* Wait until INITD bit of SelfST register is set.	*/
		for (i = 0; i < MAX_COUNT; i++)
		{
			dummy = READ_REG1(PKTPG_SELF_ST);
			if (dummy & SELF_ST_INITD) break;
		}

		if (i >= MAX_COUNT)
		{
			RETAILMSG(1, (TEXT("ERROR: Reset: Reset failed (SelfST).\r\n")));
			break;
		}

						/* Wait until SIBUSY bit of SelfST register is cleared.		*/
		for (i = 0; i < MAX_COUNT; i++)
		{
			dummy = READ_REG1(PKTPG_SELF_ST);
			if ((dummy & SELF_ST_SIBUSY) == 0) break;
		}

		if (i >= MAX_COUNT)
		{
			RETAILMSG(1, (TEXT("ERROR: Reset: Reset failed (SIBUSY).\r\n")));
			break;
		}
		r = TRUE;

	} while (0);

	return r;
}


void CS8900DBG_EnableInts(void)
{
	USHORT temp;
						/* If INTERRUPT_NUMBER is 0,							*/
						/*	Interrupt request will be generated from INTRQ0 pin */
	WRITE_REG2(PKTPG_INTERRUPT_NUMBER, INTERRUPT_NUMBER);
	temp = READ_REG2(PKTPG_BUS_CTL) | BUS_CTL_ENABLE_IRQ;
	WRITE_REG2(PKTPG_BUS_CTL, temp);
}


void CS8900DBG_DisableInts(void)
{
	USHORT temp;

	temp = READ_REG2(PKTPG_BUS_CTL) & ~BUS_CTL_ENABLE_IRQ;
	WRITE_REG2(PKTPG_BUS_CTL, temp);
}


static BOOL 
Init(USHORT *mac)
{
	USHORT temp;

#ifdef CS8900_MEM_MODE

	WRITE_REG1(PKTPG_MEMORY_BASE_ADDR     , (USHORT)(dwEthernetMemBase & 0xffff));
	WRITE_REG1(PKTPG_MEMORY_BASE_ADDR + 2 , (USHORT)(dwEthernetMemBase >> 16   ));
	WRITE_REG1(PKTPG_BUS_CTL              ,  BUS_CTL_MEMORY_E | BUS_CTL_LOW_BITS);

#endif

	temp = READ_REG2(PKTPG_LINE_CTL) | LINE_CTL_10_BASE_T | LINE_CTL_MOD_BACKOFF;
	WRITE_REG2(PKTPG_LINE_CTL, temp);
						
	WRITE_REG2(PKTPG_RX_CFG, RX_CFG_RX_OK_I_E | RX_CFG_LOW_BITS);

	WRITE_REG2(PKTPG_INDIVISUAL_ADDR + 0, *mac++);
	WRITE_REG2(PKTPG_INDIVISUAL_ADDR + 2, *mac++);
	WRITE_REG2(PKTPG_INDIVISUAL_ADDR + 4, *mac  );

	WRITE_REG2(PKTPG_RX_CTL, (RX_CTL_RX_OK_A | RX_CTL_IND_ADDR_A | RX_CTL_BROADCAST_A | RX_CTL_LOW_BITS));

	WRITE_REG2(PKTPG_TX_CFG, TX_CFG_LOW_BITS);

	temp = READ_REG2(PKTPG_LINE_CTL) | LINE_CTL_RX_ON | LINE_CTL_TX_ON;
	WRITE_REG2(PKTPG_LINE_CTL,temp);

	RETAILMSG(1, (TEXT("INFO: Init: CS8900_Init OK.\r\n")));
	return TRUE;
}

static int
RcvPkt(BYTE *pbData, DWORD dwLength)
{
					/* use int rather than short for the reason of performance	*/
	DWORD   length;
	DWORD   rlen = 0;
	USHORT *bp;
	USHORT	data;

														/* Discard RxStatus		*/
	data   = IOREAD(IO_RX_TX_DATA_0);
												/* Read the frame's length.		*/
	length = IOREAD(IO_RX_TX_DATA_0);

	if (length > dwLength) length = 0;

	bp    = (USHORT *)pbData;

	rlen = length;

	while (rlen)
	{
		data = IOREAD(IO_RX_TX_DATA_0);

		if (rlen == 1)
		{
			*((BYTE *)bp) = (BYTE)data;
			rlen--;
		}
		else
		{
			*bp++ = data;
			rlen -= 2;
		}
	}

	return length;
}


static USHORT 
TransmitPkt(BYTE *pbData, USHORT slen)
{
	USHORT * bp;
	USHORT   data;
	DWORD	 i;

	/* Send Command */
	IOWRITE(IO_TX_CMD, TX_CMD_START_ALL | TX_CMD_LOW_BITS);
	IOWRITE(IO_TX_LENGTH, slen);

	/* Wait			*/
	for (i = 0; i < MAX_COUNT; i++)
	{
		data = READ_REG2(PKTPG_BUS_ST);
		if (data & BUS_ST_RDY_4_TX_NOW) break;
	} 

	if (i >= MAX_COUNT) return 1;

	bp = (USHORT *)pbData;

	while (slen)
	{
		IOWRITE(IO_RX_TX_DATA_0, *bp++);
		slen--;

		if (slen) slen--;
	}

	return 0;
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

BOOL
CS8900DBG_Init(BYTE *iobase, ULONG membase, USHORT MacAddr[3])
{
	BOOL r = FALSE;

	bIsPacket         = FALSE;
	dwEthernetIOBase  = (DWORD)iobase;
	dwEthernetMemBase = membase;
    
	RETAILMSG(1, (TEXT("CS8900: MAC Address: %x:%x:%x:%x:%x:%x\r\n"),
				MacAddr[0] & 0x00FF, MacAddr[0] >> 8,
				MacAddr[1] & 0x00FF, MacAddr[1] >> 8,
				MacAddr[2] & 0x00FF, MacAddr[2] >> 8));
    do
	{
		//if (!Reset())			break;
		if (!Probe())			break;
		if (!Init(MacAddr))		break;

		r = TRUE;
	} while (0);
	return r;
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

DWORD
CS8900DBG_GetPendingInts(void)
{
	USHORT event;
    RX_CTL   RxCTL;
	BOOL   r = FALSE;

	event		= IOREAD(IO_ISQ);			
	RxCTL.All   = READ_REG2(PKTPG_RX_CTL);

	if ( ((event & ISQ_REG_NUM)	== REG_NUM_RX_EVENT ) &&
		 (event & RX_EVENT_RX_OK) &&
		 ((event & RX_EVENT_HASHED) || (event & RX_EVENT_IND_ADDR) || (event & RX_EVENT_BROADCAST))) 
	{
		r = bIsPacket = TRUE;
	}
	else
		bIsPacket = FALSE;

	return(r ? INTR_TYPE_RX : 0);
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
// TODO - the LAF hash process can yield the same index value for different MAC addresses.
// As such, we should do a second level check in this routine.
UINT16
CS8900DBG_GetFrame(BYTE *pbData, UINT16 *pwLength)
{
    UINT16 r = 0;
	
	if (!bIsPacket) 
		CS8900DBG_GetPendingInts();

	if (bIsPacket) 
	{
		bIsPacket = FALSE;
		r = RcvPkt(pbData, *pwLength);

		if (pwLength)
		{
			// Remove the length of the CRC value.
			//
			*pwLength = r;
		}
	}

	return r;
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
UINT16 
CS8900DBG_SendFrame( BYTE *pbData, DWORD dwLength ) 
{
	return 
		TransmitPkt(pbData, (UINT16)dwLength);
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
BOOL
CS8900DBG_ReInit(DWORD iobase, DWORD membase)
{
	bIsPacket = FALSE;

	dwEthernetIOBase  = iobase;
	dwEthernetMemBase = membase;

	return TRUE;
}

/*
    @func   void | CS8900DBG_CurrentPacketFilter | Sets a receive packet h/w filter.
    @rdesc  N/A.
    @comm    
    @xref   
*/
void CS8900DBG_CurrentPacketFilter(DWORD dwFilter)
{
    RX_CTL   RxCTL;
	LINE_CTL LineCTL;
		
	// Clear RxON to disable frame receive.
	//
	LineCTL.All = READ_REG2(PKTPG_LINE_CTL);
	LineCTL.Bits.SerRxON = 0;
	WRITE_REG2(PKTPG_LINE_CTL, LineCTL.All);

	RxCTL.All   = READ_REG2(PKTPG_RX_CTL);

	// Reset all packet filter bits to defaults.
	//
	RxCTL.Bits.MulticastA	= 0;
	RxCTL.Bits.PromiscuousA = 0;
	RxCTL.Bits.BroadcastA	= 1;	// Always accept broadcast messages.
	RxCTL.Bits.IndividualA	= 1;	// Always accept packets with a direct address.
	RxCTL.Bits.IAHashA		= 0;	// We don't support multiple directed addresses.


	// What kind of filtering do we want to apply?
	//
	// NOTE: the filter provided might be 0, but since this EDBG driver is used for KITL, we don't want
	// to stifle the KITL connection, so broadcast and directed packets should always be accepted.
	//
	if (dwFilter & PACKET_TYPE_ALL_MULTICAST)
	{	// Accept *all* multicast packets.
		WRITE_REG2((PKTPG_LOGICAL_ADDR_FILTER + 0), 0xFFFF);
		WRITE_REG2((PKTPG_LOGICAL_ADDR_FILTER + 2), 0xFFFF);
		WRITE_REG2((PKTPG_LOGICAL_ADDR_FILTER + 4), 0xFFFF);
		WRITE_REG2((PKTPG_LOGICAL_ADDR_FILTER + 6), 0xFFFF);

		RxCTL.Bits.MulticastA = 1;
	}

	if (dwFilter & PACKET_TYPE_MULTICAST)
	{	// Accept multicast packets.
		RxCTL.Bits.MulticastA = 1;
	}
 
#if 0	// Promiscuous mode is causing random hangs - it's not strictly needed.
	if (dwFilter & PACKET_TYPE_PROMISCUOUS)
	{	// Accept anything.
		RxCTL.Bits.PromiscuousA = 1;
	}
#endif

    EdbgOutputDebugString("CS8900: Set receive packet filter [Filter=0x%x, RxCTL=0x%x].\r\n", dwFilter, RxCTL.All);

	WRITE_REG2(PKTPG_RX_CTL, RxCTL.All);

	// Set RxON to disable frame receive.
	//
	LineCTL.Bits.SerRxON = 1;
	WRITE_REG2(PKTPG_LINE_CTL, LineCTL.All);

}	// CS8900DBG_CurrentPacketFilter().


/*
    @func   BOOL | CS8900DBG_MulticastList | Sets a multicast address filter list.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm    
    @xref   
*/
BOOL CS8900DBG_MulticastList(PUCHAR pucMulticastAddresses, DWORD dwNumAddresses)
{
	BYTE nCount;
	BYTE nIndex;
	LINE_CTL LineCTL;
	USHORT LAF[4];	// Logical address filter (hash index).

	// Clear RxON to disable frame receive.
	//
	LineCTL.All = READ_REG2(PKTPG_LINE_CTL);
	LineCTL.Bits.SerRxON = 0;
	WRITE_REG2(PKTPG_LINE_CTL, LineCTL.All);

	memset(LAF, 0, (4 * sizeof(USHORT)));

	// Compute the logical address filter value.
	//
	for (nCount = 0 ; nCount < dwNumAddresses ; nCount++)
	{
        EdbgOutputDebugString("CS8900: Multicast[%d of %d]  = %x-%x-%x-%x-%x-%x\r\n",
                             (nCount + 1),
							 dwNumAddresses,
                             pucMulticastAddresses[6*nCount + 0],
                             pucMulticastAddresses[6*nCount + 1],
                             pucMulticastAddresses[6*nCount + 2],
                             pucMulticastAddresses[6*nCount + 3],
                             pucMulticastAddresses[6*nCount + 4],
                             pucMulticastAddresses[6*nCount + 5]);

		nIndex = CalculateHashIndex(&pucMulticastAddresses[6*nCount]);
        LAF[nIndex/16]  |=  1 << (nIndex%16);
	}

	EdbgOutputDebugString("CS8900: Logical Address Filter = %x.%x.%x.%x.\r\n", LAF[3], LAF[2], LAF[1], LAF[0]);

	// Write the logical address filter value.
	//
	WRITE_REG2((PKTPG_LOGICAL_ADDR_FILTER + 0), LAF[0]);
	WRITE_REG2((PKTPG_LOGICAL_ADDR_FILTER + 2), LAF[1]);
	WRITE_REG2((PKTPG_LOGICAL_ADDR_FILTER + 4), LAF[2]);
	WRITE_REG2((PKTPG_LOGICAL_ADDR_FILTER + 6), LAF[3]);

	// Set RxON to disable frame receive.
	//
	LineCTL.Bits.SerRxON = 1;
	WRITE_REG2(PKTPG_LINE_CTL, LineCTL.All);

    return(TRUE);

}	// CS8900_MulticastList().



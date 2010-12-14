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

   
Module Name:	I2S.CPP

Abstract:		I2S Interface Routines for Samsung SC2450 CPU
  
Environment:	Samsung SC2450 CPU and Windows 3.0 (or later)
    
-*/


#include <windows.h>
//#include "s2450.h"

#define SD2450	1

//===================== Register Configuration Constants ======================

#define IIS_INTERNAL_CLOCK_ENABLE		(1<<13)					// Enable CPU clock to IIS controller

//----- Register definitions for IISCON control register (global config register) -----
// IISCON

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
#define TX_FIFO_UNDERRUN_STATUS                   (1<<17)
#define TX_FIFO_UNDERRUN_INT_ENABLE               (1<<16)
#define TX_FIFO_2_EMPTY_STATUS                    (1<<15)
#define TX_FIFO_1_EMPTY_STATUS                    (1<<14)
#define TX_FIFO_2_FULL_SATATUS                    (1<<13)
#define TX_FIFO_1_FULL_SATATUS                    (1<<12)
#define LR_CLOCK_STATUS                           (1<<11)
#define TX_FIFO_0_EMPTY_STATUS                    (1<<10)
#define RX_FIFO_EMPTY_STATUS                      (1<< 9)
#define TX_FIFO_0_FULL_STATUS                     (1<< 8)
#define RX_FIFO_FULL_STATUS                       (1<< 7)
#define IISCON_STATUS_MASK                        (0x5FF << 7)
#endif

#define TRANSMIT_DMA_PAUSE			(1<<6)				// Pauses transmit DMA
#define RECEIVE_DMA_PAUSE				(1<<5)				// Pauses receive DMA
#define TRANSMIT_IDLE_CMD				(1<<4)				// Pauses transmit Channel
#define RECEIVE_IDLE_CMD				(1<<3)				// Pauses receive Channel
#define TRANSMIT_DMA_REQUEST_ENABLE	(1<<2)				// Enables transmit DMA request
#define RECEIVE_DMA_REQUEST_ENABLE	(1<<1)				// Enables receive DMA request
#define IIS_INTERFACE_ENABLE			(1<<0)				// Enables IIS controller

// IISPSR
#define IIS_PRESCALER_ENABLE			(1<<15)				// Enables clock prescaler

//----- Register definitions for IISMOD status register (global status register) -----
// IISMOD

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
#define IIS_CH2_DATA_DISCARD_MASK                 (3<<20)
#define IIS_CH2_DATA_NO_DISCARD                   (0<<20)
#define IIS_CH2_DATA_RIGHT_HALFWORD_DISCARD       (1<<20)
#define IIS_CH2_DATA_LEFT_HALFWORD_DISCARD        (2<<20)

#define IIS_CH1_DATA_DISCARD_MASK                 (3<<18)
#define IIS_CH1_DATA_NO_DISCARD                   (0<<18)
#define IIS_CH1_DATA_RIGHT_HALFWORD_DISCARD       (1<<18)
#define IIS_CH1_DATA_LEFT_HALFWORD_DISCARD        (2<<18)

#define IIS_DATA_CHANNEL_MASK                     (3<<16)
#define IIS_SD2_CHANEEL_ENABLE                    (1<<17)
#define IIS_SD1_CHANEEL_ENABLE                    (1<<16)

#define IIS_BIT_LENGTH_CONTROL_MASK               (3<<13)
#define IIS_BIT_LENGTH_PER_CHANNEL_16BIT          (0<<13)
#define IIS_BIT_LENGTH_PER_CHANNEL_8BIT           (1<<13)
#define IIS_BIT_LENGTH_PER_CHANNEL_24BIT          (2<<13)

#define IIS_CODEC_CLK_SOURCE_MASK                 (1<<12)
#define IIS_CODEC_CLK_SOURCE_INTERNAL             (0<<12)
#define IIS_CODEC_CLK_SOURCE_EXTERNAL             (1<<12)
#endif

#define IIS_MASTER_MODE				(0<<11|0<<10)		// 00: PCLK master, Selects master/slave mode
#define IIS_SLAVE_MODE					(2<<10)				
#define IIS_NOTRANSFER_MODE			0x00000000				// Selects transfer mode
#define IIS_RECEIVE_MODE				(1<<8)
#define IIS_TRANSMIT_MODE				(0<<9|0<<8)			// 00:TX/RX, 01:RX only, 10:TX only, 11:reserved
#define IIS_TRANSMIT_RECEIVE_MODE		(1<<9)
#define ACTIVE_CHANNEL_LEFT			(0<<7)				// Selects active channel
#define ACTIVE_CHANNEL_RIGHT			(1<<7)

#define SERIAL_INTERFACE_IIS_COMPAT	(0<<5)				// Selects serial interface format
#define SERIAL_INTERFACE_MSBL_COMPAT	(1<<5)
#if (BSP_TYPE == BSP_SMDK2443)
#define DATA_8_BITS_PER_CHANNEL		(1<<0)				// Selects # of data bits per channel
#define DATA_16_BITS_PER_CHANNEL		(0<<0)				
#elif (BSP_TYPE == BSP_SMDK2450)
#endif
#define MASTER_CLOCK_FREQ_256fs		(0<<3)				// Selects master clock frequency
#define MASTER_CLOCK_FREQ_384fs		(2<<3)				
#define SERIAL_BIT_CLOCK_FREQ_16fs		(2<<1)				// Selects serial data bit clock frequency
#define SERIAL_BIT_CLOCK_FREQ_32fs		(0<<1)				
#define SERIAL_BIT_CLOCK_FREQ_48fs		(1<<1)	


//----- Register definitions for IISPSR control register (global config register) -----
//		FORMAT:			bits[9:5] - Prescaler Control A
//						bits[4:0] - Prescaler Control B
//
//						Range: 0-31 and the division factor is N+1 (a.k.a. 1-32)
//
//		The I2SLRCLK frequency is determined as follows:
//
//				I2SLRCLK = CODECLK / I2SCDCLK		and		(prescaler+1) = PCLK / CODECLK
//
//		Thus, rearranging the equations a bit we can see that:
//
//				prescaler = (PCLK / CODECLK) - 1 
//		or
//				prescaler = ((PCLK / (IS2LRCLK * IS2CDCLK)) - 1
//		
// Here are some popular values for IS2LRCLK:
//		
#define IS2LRCLK_800					800
#define IS2LRCLK_11025					11025
#define IS2LRCLK_16000					16000
#define IS2LRCLK_22050					22050
#define IS2LRCLK_32000					32000
#define IS2LRCLK_44100					44100
#define IS2LRCLK_48000					48000
#define IS2LRCLK_64000					64000
#define IS2LRCLK_88200					88200
#define IS2LRCLK_96000					96000
	

//----- Register definitions for IISFCON control register (global config register) -----
#define TRANSMIT_FIFO_ACCESS_NORMAL		0x00000000				// Selects the transmit FIFO access mode
#define TRANSMIT_FIFO_ACCESS_DMA			(1<<2)	//0x00008000				
#define RECEIVE_FIFO_ACCESS_NORMAL		0x00000000				// Selects the receive FIFO access mode
#define RECEIVE_FIFO_ACCESS_DMA			(1<<1) //0x00004000				

//----- Register definitions for IISFIFO control register (global config register) -----
//		NOTE: This register is used to access the transmit/receive FIFO
#define MAX_TRANSMIT_FIFO_ENTRIES		24
#define MAX_RECEIVE_FIFO_ENTRIES		24

//=============================================================================

//-------------------------- Public Interface ------------------------------
BOOL I2S_Init();
BOOL I2S_Deinit();
BOOL I2S_WriteData(LPWORD lpBuff, DWORD dwLen);

//-------------------- Private Interface (Helper routines) ------------------
VOID StartI2SClock(VOID);
VOID StopI2SClock(VOID);
int SetI2SClockRate(DWORD ClockRate);


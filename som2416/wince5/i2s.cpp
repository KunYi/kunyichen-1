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
  
Notes:			This code assumes that the CPU acts as the master that is 
				connected to a CODEC chip configured in slave mode.

				Some platforms require the audio CODEC chip to act as the master
				while the CPU's IIS controller is put into slave mode.  For these
				environments, the IIS_MASTER_MODE flag (located in I2S_Init())
				should be replaced with the IIS_SLAVE_MODE flag.

Environment:	Samsung SC2450 CPU and Windows 3.0 (or later)

-2005.11.12 - DonGo.

-*/

#include <windows.h>
#include <s3c2450.h>
#include "i2s.h"
#include <bsp_cfg.h>

//------------------------------ GLOBALS -------------------------------------------
extern volatile	S3C2450_IISBUS_REG *g_pIISregs;								// I2S control registers
extern volatile S3C2450_IOPORT_REG *g_pIOPregs;								// GPIO registers (needed to enable SPI)
extern volatile S3C2450_CLKPWR_REG *g_pCLKPWRreg;							// CLCKPWR (needed to enable SPI clocks)
//----------------------------------------------------------------------------------

// Display debug message.
#define DBG_ON 0 

#ifdef DEBUG
#define ZONE_ERROR  1
#endif


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		I2S_Init()

Description:	Initializes the IIS bus and controller.

Notes:			This routine assumes that the control registers (see 
				the globals section above) have already been initialized.

Returns:		Boolean indicating success.
-------------------------------------------------------------------*/
#define I2S_BIT_CLK_TIMES	32		//which times of I2SLRCK?
#define I2S_CODEC_CLK_TIMES	384		//which times of I2SLRCK?

BOOL I2S_Init()
{
	volatile UINT i = 0;
	volatile UINT count = 0;
	volatile UINT prescaler = 0;

//	float I2S_Codec_CLK;
	
	RETAILMSG(DBG_ON, (TEXT("+I2S_Init\n")));

   	//-------------------------------------------------------------------------------
	//PORT E GROUP
	//Ports  :	 GPE4			 GPE3			 GPE2		   GPE1			GPE0 
	//Signal :	I2S DO		   I2S DI		 CDCLK		I2S CLK		I2S LRCLK
	//Binary :   10,				10,			 10,			  10,			  10,			10	
	//-------------------------------------------------------------------------------
	g_pIOPregs->GPECON = g_pIOPregs->GPECON & ~(0x3ff) | 0x2aa;


	//IIS Pre-scaler Setting...The following value is not valid...
	g_pIISregs->IISPSR &= 0;
	g_pIISregs->IISPSR |= IIS_PRESCALER_ENABLE |(2<<8);  	// PCLK(50Mhz)/(2+1)=16.9Mhz
	// Disable all the activities
	g_pIISregs->IISCON  = (1<<6)+(1<<5)+(1<<4)+(1<<3)+(0<<2)+(0<<1)+(0<<0);
	for(i=0; i<10; i++);

	g_pIISregs->IISCON  = (TRANSMIT_DMA_REQUEST_ENABLE | RECEIVE_DMA_REQUEST_ENABLE);

#if (BSP_TYPE == BSP_SMDK2443)
	g_pIISregs->IISMOD  = (  IIS_MASTER_MODE | IIS_TRANSMIT_RECEIVE_MODE | ACTIVE_CHANNEL_LEFT
						   	| SERIAL_INTERFACE_IIS_COMPAT | MASTER_CLOCK_FREQ_384fs 
						   	| SERIAL_BIT_CLOCK_FREQ_32fs| DATA_16_BITS_PER_CHANNEL);
#elif (BSP_TYPE == BSP_SMDK2450)
	g_pIISregs->IISMOD  = (  IIS_MASTER_MODE | IIS_TRANSMIT_RECEIVE_MODE | ACTIVE_CHANNEL_LEFT
						   	| SERIAL_INTERFACE_IIS_COMPAT | MASTER_CLOCK_FREQ_384fs 
						   	| SERIAL_BIT_CLOCK_FREQ_32fs|IIS_BIT_LENGTH_PER_CHANNEL_16BIT);
#endif

	prescaler = SetI2SClockRate(IS2LRCLK_44100);
	
	//----- 4. For power management purposes, shut the clocks off! -----
	//Configure IIS regs
	g_pIISregs->IISFIC &= ~((1<<15)|(1<<7));	// TXFIFO/RXFIFO flush.
	for(i=0; i<10; i++);
	g_pIISregs->IISFIC |= (1<<15)|(1<<7);	// TXFIFO/RXFIFO flush.
	for(i=0; i<10; i++);
	g_pIISregs->IISFIC &= ~((1<<15)|(1<<7));	// TXFIFO/RXFIFO flush.
	for(i=0; i<10; i++);

	//I2S_Codec_CLK = (float)((float)S3C2450_PCLK/(float)(prescaler+1));
	#if 0
	DWORD modeval = g_pIISregs->IISMOD;
	switch( (modeval>>10)&0x03 )
	{
		case 0:
			RETAILMSG(DBG_ON, (TEXT("I2S Master CLK(PCLK) = %ldHz\n"), S3C2450_PCLK));
			break;
		case 1:
			//if(g_pCLKPWRreg->CLKSRC&(1<<9))
				RETAILMSG(DBG_ON, (TEXT("I2S Master CLK: FOUT_mpll\n")));
			else
				RETAILMSG(DBG_ON, (TEXT("I2S Master CLK: EREFCLK\n")));
			break;
		case 2:
			RETAILMSG(DBG_ON, (TEXT("I2S Master CLK(PCLK) = %ldHz\n"), S3C2450_PCLK));			
			break;
		default:
			RETAILMSG(DBG_ON, (TEXT("I2S Master CLK Default!!!\n")));
			break;
	}
	RETAILMSG(DBG_ON,(TEXT("I2S Codec Clock=%ldHz\n"), (DWORD)(I2S_Codec_CLK)));
	RETAILMSG(DBG_ON,(TEXT("I2S Bit Clock=%ldHz\n"), (DWORD)((I2S_Codec_CLK/I2S_CODEC_CLK_TIMES)*I2S_BIT_CLK_TIMES)));
	RETAILMSG(DBG_ON,(TEXT("I2S Sampling Rate=%ldHz\n"), (DWORD)(I2S_Codec_CLK/I2S_CODEC_CLK_TIMES)));
	#else
	DWORD modeval = g_pIISregs->IISMOD;
	switch( (modeval>>10)&0x03 )
	{
		case 0:
			//RETAILMSG(1, (TEXT("I2S Master CLK(PCLK) = %ldHz\n"), S3C2450_PCLK));
			break;
		case 1:
			/*
			//if(g_pCLKPWRreg->CLKSRC&(1<<9))
				RETAILMSG(1, (TEXT("I2S Master CLK: FOUT_mpll\n")));
			else
				RETAILMSG(1, (TEXT("I2S Master CLK: EREFCLK\n")));
			break;*/
		case 2:
			RETAILMSG(1, (TEXT("I2S Master CLK(PCLK) = %ldHz\n"), S3C2450_PCLK));			
			break;
		default:
			RETAILMSG(1, (TEXT("I2S Master CLK Default!!!\n")));
			break;
	}
	//RETAILMSG(1,(TEXT("I2S Codec Clock=%ldHz\n"), (DWORD)(I2S_Codec_CLK)));
	//RETAILMSG(1,(TEXT("I2S Bit Clock=%ldHz\n"), (DWORD)((I2S_Codec_CLK/I2S_CODEC_CLK_TIMES)*I2S_BIT_CLK_TIMES)));
	//RETAILMSG(1,(TEXT("I2S Sampling Rate=%ldHz\n"), (DWORD)(I2S_Codec_CLK/I2S_CODEC_CLK_TIMES)));
	#endif
	RETAILMSG(DBG_ON, (TEXT("-I2S_Init\n")));

	//g_pIISregs->IISCON |= IIS_INTERFACE_ENABLE;				// Enable I2S clock

	return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		I2S_Deinit()

Description:	Deinitializes the I2S bus and controller.

Notes:			This routine DOES NOT unmap the control registers;
				the caller is responsible for freeing this memory.

Returns:		Boolean indicating success.
-------------------------------------------------------------------*/
BOOL I2S_Deinit()
{
	//----- 1. Stop the I2S clocks -----

	RETAILMSG(DBG_ON, (TEXT("I2S_Deinit()\n")));
	StopI2SClock();

	return TRUE;
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		I2S_WriteData()

Description:	Outputs the specified data onto the I2S bus.

Notes:			This routine expects that the I2S clock is already
				running when it is called.

Returns:		Boolean indicating success.
-------------------------------------------------------------------*/
BOOL I2S_WriteData(LPWORD lpBuff, DWORD dwLen)
{
	
	return FALSE;
}


//------------------------------------ Helper Routines ------------------------------------

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StartI2SClock()

Description:	Enables the I2S clock that drives the audio codec chip.

Returns:		N/A
-------------------------------------------------------------------*/
VOID StartI2SClock(VOID)
{
	g_pIISregs->IISCON |= IIS_INTERFACE_ENABLE;			// Disable the I2S clock
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StopI2SClock()

Description:	Disables the I2S clock that drives the audio codec chip.

Returns:		N/A
-------------------------------------------------------------------*/
VOID StopI2SClock(VOID)
{
	g_pIISregs->IISCON &= ~IIS_INTERFACE_ENABLE;			// Disable the I2S clock
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		SetI2SClockRate()

Description:	Sets the I2S clock that drives the audio codec chip:

Params:			ClockRate	IS2LRCLK_800		800   Khz
							IS2LRCLK_11025		11025 Khz
							IS2LRCLK_16000		16000 Khz
							IS2LRCLK_22050		22050 Khz
							IS2LRCLK_32000		32000 Khz
							IS2LRCLK_44100		44100 Khz
							IS2LRCLK_48000		48000 Khz
							IS2LRCLK_64000		64000 Khz
							IS2LRCLK_88200		88200 Khz
							IS2LRCLK_96000		96000 Khz

Returns:		N/A
-------------------------------------------------------------------*/
int SetI2SClockRate(DWORD ClockRate)
{
	BYTE  prescaler;
	DWORD codeclock, i2scdclk;

	//----- 1. Set the clock rate  -----
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
	//		NOTE: The following formula is actually used in order to avoid floating point arithmetic:
	//
	//				prescaler = ((PCLK + ((IS2LRCLK * IS2CDCLK) - 1)) / (IS2LRCLK * IS2CDCLK)) - 1
	//
	if(g_pIISregs->IISMOD & MASTER_CLOCK_FREQ_384fs)
	{
		i2scdclk = 384;				// Sampling frequency
	}
	else
	{
		i2scdclk = 256;				// Sampling frequency
	}

	codeclock = ClockRate * i2scdclk;

	prescaler = (BYTE)((S3C2450_PCLK + (codeclock/2)) / codeclock + 0.5) - 1;

	RETAILMSG(DBG_ON,(TEXT("I2CCDCLK=%d --> Prescaler:%d\r\n"), i2scdclk, prescaler));
 
	//----- IMPORTANT: Make sure we set both channel prescalers to the same value (see datasheet for details) -----
#if (BSP_TYPE == BSP_SMDK2443)
	g_pIISregs->IISPSR = (g_pIISregs->IISPSR & ~(0x3ff<<0)) | (prescaler<<0) | IIS_PRESCALER_ENABLE;
#elif (BSP_TYPE == BSP_SMDK2450)
	g_pIISregs->IISPSR = (g_pIISregs->IISPSR & ~(0x3f<<8)) | (prescaler<<8) | IIS_PRESCALER_ENABLE;
#endif

		return prescaler;
}





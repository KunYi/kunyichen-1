#pragma once
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

   
Module Name:	HWCTXT.H

Abstract:		Platform dependent code for the mixing audio driver.
  
Environment:	Samsung SC2450 CPU and Windows 3.0 (or later)
	
-*/

//#include <oalintr.h>
//#include <s3c2450.h>
#include <bsp.h>
#include "i2c.h"

#define I2C_MUTEX_NAME		TEXT("I2C_CONTROL")
#define OUTCHANNELS (2)
#define BITSPERSAMPLE (16)
#define SAMPLERATE  (44100)

// Inverse sample rate, in .32 fixed format, with 1 added at bottom to ensure round up.
#define INVSAMPLERATE ((UINT32)(((1i64<<32)/SAMPLERATE)+1))

typedef INT16 HWSAMPLE;
typedef HWSAMPLE *PHWSAMPLE;

// Set USE_MIX_SATURATE to 1 if you want the mixing code to guard against saturation
// This costs a couple of instructions in the inner loop
#define USE_MIX_SATURATE (1)
// The code will use the follwing values as saturation points
#define AUDIO_SAMPLE_MAX	(32767)
#define AUDIO_SAMPLE_MIN	(-32768)

#define AUDIO_DMA_PAGE_SIZE		4096					// Size in bytes			

//----- Used to track DMA controllers status -----
#define DMA_CLEAR			0x00000000
#define DMA_DONEA			0x00000008
#define DMA_STRTA			0x00000010
#define DMA_DONEB			0x00000020
#define DMA_STRTB			0x00000040
#define DMA_BIU				0x00000080					// Determines which buffer is in use: (A=0, B=1)

//----- Used for scheduling DMA transfers -----
#define  OUT_BUFFER_A		0							
#define  OUT_BUFFER_B		1
#define	 IN_BUFFER_A		0
#define  IN_BUFFER_B		1


class HardwareContext
{
public:
	static BOOL CreateHWContext(DWORD Index);
	HardwareContext();
	~HardwareContext();

	DECLARE_RT_CODEC_CLASS

	DWORD RT_AudioMessage(UINT uMsg,DWORD dwParam1,DWORD dwParam2);
	void OSTDelayMilliSecTime(DWORD MilliSec);
	BOOL SafeWriteCodec(BYTE Offset, unsigned short int Data);
    BOOL SafeReadCodec(BYTE Offset, unsigned short int * Data);
    short int MsgWriteCodec(DWORD dwParam1,DWORD dwParam2);
    short int MsgReadCodec(DWORD dwParam1,DWORD dwParam2);

	void Lock()   {EnterCriticalSection(&m_Lock);}
	void Unlock() {LeaveCriticalSection(&m_Lock);}

	DWORD GetNumInputDevices()  {return 1;}
	DWORD GetNumOutputDevices() {return 1;}
    DWORD GetNumMixerDevices()  {return 1;}

	DeviceContext *GetInputDeviceContext(UINT DeviceId)
	{
		return &m_InputDeviceContext;
	}

	DeviceContext *GetOutputDeviceContext(UINT DeviceId)
	{
		return &m_OutputDeviceContext;
	}

	BOOL Init(DWORD Index);										
	BOOL Deinit();

	void PowerUp();
	void PowerDown();

	BOOL StartInputDMA();
	BOOL StartOutputDMA();

	void StopInputDMA();
	void StopOutputDMA();

	void InterruptThread();
    BOOL IOControl ( 	DWORD  dwOpenData,
                        DWORD  dwCode,
                        PBYTE  pBufIn,
                        DWORD  dwLenIn,
                        PBYTE  pBufOut,
                        DWORD  dwLenOut,
                        PDWORD pdwActualOut);

	DWORD 	Open();
	DWORD 	Close();    
	
    CEDEVICE_POWER_STATE GetDx() { return m_Dx;}

    DWORD       GetOutputGain (void);
    MMRESULT    SetOutputGain (DWORD dwVolume);
    DWORD       GetInputGain (void);
    MMRESULT    SetInputGain (DWORD dwVolume);

    BOOL        GetOutputMute (void);
    MMRESULT    SetOutputMute (BOOL fMute);
    BOOL        GetInputMute (void);
    MMRESULT    SetInputMute (BOOL fMute);
 //=======================
VOID AdjustAudioVol(BOOL bUp);
void AdjustAudioVolThread();
int SetAudioVolLevel(int nVolLevel);
int GetAudioVolLevel();

     HANDLE m_hAudioVolChangeEvent;
 //=======================	

	BOOL 	bInputClose;
	BOOL 	bPowerSet;
	BOOL 	bDoingSuspend;
protected:
    DWORD m_dwOutputGain;
    DWORD m_dwInputGain;
    BOOL  m_fInputMute;
    BOOL  m_fOutputMute;

	DWORD m_MicrophoneRouting;
	DWORD m_SpeakerRouting;
	DWORD m_InternalRouting;
	DWORD m_MasterOutputGain;

	BOOL InitInterruptThread();

	BOOL InitInputDMA();
	BOOL InitOutputDMA();
	BOOL Codec_channel();
	BOOL InitCodec();
	
	BOOL MapRegisters();
	BOOL UnmapRegisters();
	BOOL MapDMABuffers();
	BOOL UnmapDMABuffers();

	ULONG TransferInputBuffer(ULONG NumBuf);
	ULONG TransferOutputBuffer(ULONG NumBuf);
	ULONG TransferInputBuffers(DWORD dwDCSR);
	ULONG TransferOutputBuffers(DWORD dwDCSR);

	void 	CodecPowerDown();

#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
	DWORD HW_WriteRegisters(PUCHAR pBuff, UCHAR StartReg, DWORD nRegs);
	void WriteCodecRegister(UCHAR Reg, USHORT Val);
#endif
	DWORD GetInterruptThreadPriority();

	DWORD m_DriverIndex;
	CRITICAL_SECTION m_Lock;

	BOOL m_Initialized;
	BOOL m_InPowerHandler;
	DWORD m_IntrAudio;
	// DonGo Modiefied.
	DWORD m_dwSysintrOutput, m_dwSysintrInput;

	InputDeviceContext m_InputDeviceContext;
	OutputDeviceContext m_OutputDeviceContext;

	PBYTE		m_Input_pbDMA_PAGES[2];
	PBYTE		m_Output_pbDMA_PAGES[2];
	PBYTE		m_pVirtDMABufferAddr;
	
	BOOL m_InputDMARunning;
	BOOL m_OutputDMARunning;
#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
    BOOL m_SavedInputDMARunning;
    BOOL m_SavedOutputDMARunning;
#endif
	ULONG m_OutBytes[2];
	ULONG m_InBytes[2];

	WORD  m_nOutputVolume;					// Current HW Playback Volume 
	WORD  m_nInputVolume;					// Current HW Input (Microphone) Volume 

	HANDLE m_hAudioInterrupt;				// Handle to Audio Interrupt event.
	HANDLE m_hAudioInterruptThread;			// Handle to thread which waits on an audio interrupt event.

	//----------------------- Platform specific members ----------------------------------

	DWORD  m_OutputDMAStatus;					// Output DMA channel's status
	DWORD  m_InputDMAStatus;					// Input DMA channel's status

	BOOL AudioMute(DWORD channel, BOOL bMute);	
	//------------------------------------------------------------------------------------
	CEDEVICE_POWER_STATE    m_Dx;
	
	BOOL 	bClosing;
	WORD 	TransferCount;
	BOOL 	bIdlePwrDown;				

	DWORD 	dTimeoutval1;
	BOOL 	bIsPlaying;
	BOOL 	bIsRecording;	
};

void CallInterruptThread(HardwareContext *pHWContext);


//----------------------------------- Helper Functions and Macros ----------------------------------------

//======== Record =========
#define ioRecordPointerLow						(g_pDMAregs->DIDST1)
#define ioRecordPointerHigh						(g_pDMAregs->DIDST1)

#define RECORD_DMA_BUFFER_PHYS					(g_PhysDMABufferAddr.LowPart + 2 * AUDIO_DMA_PAGE_SIZE)

#define AUDIO_RESET_RECORD_POINTER()			{ioRecordPointerLow  = (RECORD_DMA_BUFFER_PHYS); \
												 ioRecordPointerHigh = (RECORD_DMA_BUFFER_PHYS+ AUDIO_DMA_PAGE_SIZE); }

#define AUDIO_IN_CLEAR_INTERRUPTS()				(g_pDMAregs->DCON1 = g_pDMAregs->DCON1)

#if 0
#define AUDIO_IN_DMA_ENABLE()					{ 	g_pDMAregs->DMASKTRIG1 |= ENABLE_DMA_CHANNEL; \
													g_pDMAregs->DMASKTRIG1 &= ~STOP_DMA_TRANSFER; \
													g_pIISregs->IISCON  &= ~((1<<5)|(1<<3)); \
													g_pIISregs->IISCON  |= RECEIVE_DMA_REQUEST_ENABLE;	 
 													
#define AUDIO_IN_DMA_DISABLE()					{ 	StopI2SClock(); \
													g_pIISregs->IISCON &= ~RECEIVE_DMA_REQUEST_ENABLE;	\
													g_pIISregs->IISCON  |= ((1<<5)|(1<<3)); \
													g_pIISregs->IISCON  &= ~RECEIVE_DMA_REQUEST_ENABLE;	 }
#else
#define AUDIO_IN_DMA_ENABLE()					{ 	g_pDMAregs->DMASKTRIG1 &= ~STOP_DMA_TRANSFER; \
													g_pDMAregs ->DMASKTRIG1 |= ENABLE_DMA_CHANNEL; \
													g_pIISregs->IISCON  &= ~(RECEIVE_DMA_PAUSE|RECEIVE_IDLE_CMD); \
													g_pIISregs->IISCON  |= (RECEIVE_DMA_REQUEST_ENABLE|IIS_INTERFACE_ENABLE); }
 													
#define AUDIO_IN_DMA_DISABLE()					{ 	g_pDMAregs ->DMASKTRIG1 &= ~ENABLE_DMA_CHANNEL; \
													g_pDMAregs	->DMASKTRIG1 |= STOP_DMA_TRANSFER; \
													g_pIISregs->IISCON &= ~RECEIVE_DMA_REQUEST_ENABLE;	\
													g_pIISregs->IISCON  |= (RECEIVE_DMA_PAUSE|RECEIVE_IDLE_CMD); }

#endif
#define SELECT_AUDIO_DMA_INPUT_BUFFER_A()		(g_pDMAregs->DIDST1 = (int)(g_PhysDMABufferAddr.LowPart+2*AUDIO_DMA_PAGE_SIZE) )
#define SELECT_AUDIO_DMA_INPUT_BUFFER_B()		(g_pDMAregs->DIDST1 = (int)(g_PhysDMABufferAddr.LowPart+3*AUDIO_DMA_PAGE_SIZE) )


//======== Playback =========
#define ioPlaybackPointerLow					(g_pDMAregs->DISRC2)
#define ioPlaybackPointerHigh					(g_pDMAregs->DISRC2)

#define AUDIO_RESET_PLAYBACK_POINTER()			{ioPlaybackPointerLow  = (g_PhysDMABufferAddr.LowPart);	\
												 ioPlaybackPointerHigh = (g_PhysDMABufferAddr.LowPart + AUDIO_DMA_PAGE_SIZE); }   

#define AUDIO_OUT_CLEAR_INTERRUPTS()			(g_pDMAregs->DCON2 = g_pDMAregs->DCON2)
#if 0	// DonGo
#define AUDIO_OUT_DMA_ENABLE()					{	 StartI2SClock(); \
													g_pDMAregs->DMASKTRIG2 &= ~STOP_DMA_TRANSFER;  \
													g_pDMAregs->DMASKTRIG2 |= ENABLE_DMA_CHANNEL;  }
													
#define AUDIO_OUT_DMA_DISABLE()					{	StopI2SClock(); \
													g_pDMAregs->DMASKTRIG2 |= STOP_DMA_TRANSFER; \
												  	g_pDMAregs->DMASKTRIG2 &= ~ENABLE_DMA_CHANNEL; }

#else
#define AUDIO_OUT_DMA_ENABLE()					{ g_pIISregs->IISCON  &= ~(TRANSMIT_DMA_PAUSE|TRANSMIT_IDLE_CMD); \
													g_pIISregs->IISCON  |= (TRANSMIT_DMA_REQUEST_ENABLE|IIS_INTERFACE_ENABLE); \
													g_pDMAregs->DMASKTRIG2 &= ~STOP_DMA_TRANSFER;  \
													g_pDMAregs->DMASKTRIG2 |= ENABLE_DMA_CHANNEL;  }
													
#define AUDIO_OUT_DMA_DISABLE()					{ g_pDMAregs->DMASKTRIG2 |= STOP_DMA_TRANSFER; \
												  	g_pDMAregs->DMASKTRIG2 &= ~ENABLE_DMA_CHANNEL; \
													g_pIISregs->IISCON  |= (TRANSMIT_DMA_PAUSE|TRANSMIT_IDLE_CMD); \
													g_pIISregs->IISCON  &= ~(TRANSMIT_DMA_REQUEST_ENABLE); }
													//g_pIISregs->IISCON  &= ~(TRANSMIT_DMA_REQUEST_ENABLE|IIS_INTERFACE_ENABLE); }
#endif
#define SELECT_AUDIO_DMA_OUTPUT_BUFFER_A()		(g_pDMAregs->DISRC2 = (int)(g_PhysDMABufferAddr.LowPart) )
#define SELECT_AUDIO_DMA_OUTPUT_BUFFER_B()		(g_pDMAregs->DISRC2 = (int)(g_PhysDMABufferAddr.LowPart+AUDIO_DMA_PAGE_SIZE) )

#define SET_AUDIO_DMA_OUTPUT_DCON()			g_pDMAregs->DCON2 = (   HANDSHAKE_MODE | GENERATE_INTERRUPT \
												| NO_DMA_AUTO_RELOAD | TRANSFER_WORD | (AUDIO_DMA_PAGE_SIZE / 4 ) );

//------------------------------------------ Externs ----------------------------------------------
extern HardwareContext *g_pHWContext;



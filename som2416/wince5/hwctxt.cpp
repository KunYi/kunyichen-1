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

   
Module Name:	HWCTXT.CPP

Abstract:		Platform dependent code for the mixing audio driver.

Notes:			The following file contains all the hardware specific code
				for the mixing audio driver.  This code's primary responsibilities
				are:

					* Initialize audio hardware (including codec chip)
					* Schedule DMA operations (move data from/to buffers)
					* Handle audio interrupts

				All other tasks (mixing, volume control, etc.) are handled by the "upper"
				layers of this driver.

-2005.11.12 - DonGo.

-*/

#include "wavemain.h"
 //[david.modify] 2008-05-14 09:46
#include <s3c2450.h>
#include "I2S.h"
#include "hwctxt.h"
#include <p2.h>
#include <bsp.h>
#include <S3C2450REF_GPIO.h>
#include "s3c2450_intr.h"
 //[david.modify] 2008-06-05 14:11
#include <dbgmsg_david.h>

#define DMA_FLAG 1
#define DMA_CH_MIC 1
#define DMA_CH_OUT 2
#define DBG_WAV 0

// debug message
#define DBG_ON		0	// Debug message on/off
#define DBG_AUDIO 	0

unsigned int delay_count;
#define DELAY_COUNT (100000)

//----- Macro used to send commands to the audio codec chip over the SPI bus -----
//		NOTE: The command format is 16 bits:		bits[15-9]	= register
//													bits[8-0]	= data command
//
//		Refer to the TILV320AC reference guide for details.
//
//#define SEND_CODEC_COMMAND(reg, dat)	{ SPI_SendWord((reg | dat));  }

int rec_mode=0;
//-------------------------------- Global Variables --------------------------------------
volatile S3C2450_IISBUS_REG 	*g_pIISregs		= NULL;		// I2S control registers
volatile S3C2450_IOPORT_REG 	*g_pIOPregs		= NULL;		// GPIO registers (needed to enable I2S and SPI)

volatile S3C2450_DMA_REG 		*g_pDMAregs	= NULL;		// DMA1 registers audio in (needed for I/O on I2S bus)

volatile S3C2450_CLKPWR_REG 	*g_pCLKPWRreg	= NULL;		// Clock power registers (needed to enable I2S and SPI clocks)
volatile S3C2450_INTR_REG 		*s2450INT 		= NULL;

				
//UTL_FASTCALL	g_tblFastCall;							// Needed for fast driver->driver calling mechanism
HANDLE		  g_hUTLObject		= INVALID_HANDLE_VALUE;

HardwareContext *g_pHWContext		= NULL;
PHYSICAL_ADDRESS g_PhysDMABufferAddr;
UINT32		g_ForCount=1;
int 	TransferCount;

short int WriteI2SCodec(BYTE Offset, UINT16 Data);
short int ReadI2SCodec(BYTE Offset, UINT16 *Data);
HANDLE          hI2C;   // I2C Bus Driver
I2C_FASTCALL    fc;     // I2C Fastcall driver-to-driver entrypoints

extern int g_NeedtoSleep;
//----------------------------------------------------------------------------------------

DBGPARAM dpCurSettings = {
	TEXT("CONSOLE"), {
		TEXT("0"),TEXT("1"),TEXT("2"),TEXT("3"),
		TEXT("4"),TEXT("5"),TEXT("6"),TEXT("7"),
		TEXT("8"),TEXT("9"),TEXT("10"),TEXT("11"),
		TEXT("12"),TEXT("Function"),TEXT("Init"),TEXT("Error")},
	0x8000  // Errors only, by default
}; 

#pragma optimize ("",off)
void DelayMS(unsigned int msDelay)
{
	unsigned int i;
	unsigned int tick;
	tick=msDelay*1200;
	for(i=0;i<tick;i++);	
}
#pragma optimize ("",on)

short int WriteI2SCodec(BYTE Offset, UINT16 Data)
{

  short int retVal = TRUE;
  static UCHAR pBuff[2];

	pBuff[1]=(UCHAR)(Data & 0xff);
	pBuff[0]=(UCHAR)(Data>>8) & 0xff;


		    retVal = (short int)fc.I2CWrite(fc.Context,
                              CODEC_SLAVE_ADDRESS,   	// SlaveAddress
                             (UCHAR) Offset,         // WordAddress
                            (PUCHAR) pBuff,
                              2);      
		
    	if ( retVal ) 
		{	       		 
			return FALSE;
		}
//		RETAILMSG(1, (TEXT("Write Offset=%x,Data=%x \r\n"),Offset,Data));
    return(TRUE);

}

short int ReadI2SCodec(BYTE Offset, UINT16 *Data)
{
  short int retVal = TRUE;
  static UCHAR	pBuff[2];

       // use the driver-to-driver call
  		  retVal = (short int)fc.I2CRead(fc.Context,
                             CODEC_SLAVE_ADDRESS, 	// SlaveAddress
                             (UCHAR)Offset,      // WordAddress
                            (PUCHAR) pBuff,
                             2);  

	    if ( retVal ) 
		{        	
			return FALSE;
		}

	*Data=	pBuff[0]<<8|pBuff[1];

//	RETAILMSG(1, (TEXT("Read Offset=%x,Data=%x \r\n"),Offset,*Data));
	return(TRUE);
}

//Sub-Routines  
//Setting Port related to IIS  
void IIS_Port_Init(void)
{
#if (BSP_TYPE == BSP_SMDK2443)
   	//----------------------------------------------------------
	//PORT G GROUP
	//Ports  :   GPG0       GPG1        GPG2  
	//Signal :  L3MODE     L3DATA      L3CLK
	//Setting:  OUTPUT     OUTPUT      OUTPUT 
	//	        [1:0]      [3:2]       [5:4]
	//Binary :  01          01           01 
	//----------------------------------------------------------    
    	g_pIOPregs->GPGCON = g_pIOPregs->GPGCON & ~((0x3<<4)|(0x3<<2)|(0x3)); 
   	g_pIOPregs->GPGCON |= ((0x1<<4)|(0x1<<2)|(0x1)); // output setting
#ifdef EVT1
	g_pIOPregs->EXTINT1 = READEXTINT1(g_pIOPregs->EXTINT1) | (1<<11)|(1<<7)|(1<<3);
#else
	g_pIOPregs->GPGUDP  = g_pIOPregs->GPGUDP & ~(0x3f<<0) | (2<<4)|(2<<2)|(2<<0);	// 1:Pull-Down disable
#endif
#elif (BSP_TYPE == BSP_SMDK2450)
#endif


   	//-------------------------------------------------------------------------------
	//PORT E GROUP
	//Ports  :   GPE4  			GPE3           GPE2         GPE1           GPE0 
	//Signal :   I2S DO         I2S DI         CDCLK        I2S CLK        I2S LRCLK
	//Binary :   10,            10,            10,          10,            10  
	//-------------------------------------------------------------------------------
	g_pIOPregs->GPECON = g_pIOPregs->GPECON & ~(0x3ff) | 0x2aa;   
#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1	
	g_pIOPregs->GPEUDP = g_pIOPregs->GPEUDP  & ~(0x3ff)  | 0x155; 
#else
	g_pIOPregs->GPEUDP = g_pIOPregs->GPEUDP  & ~(0x3ff)  | 0x2AA; 
#endif	
#elif (BSP_TYPE == BSP_SMDK2450)
	g_pIOPregs->GPEUDP = g_pIOPregs->GPEUDP  & ~(0x3ff);	//Pull-up/down disable 
#endif

}

BOOL HardwareContext::CreateHWContext(DWORD Index)
{
	if (g_pHWContext)
	{
		return(TRUE);
	}

	g_pHWContext = new HardwareContext;
	if (!g_pHWContext)
	{
		return(FALSE);
	}

	return(g_pHWContext->Init(Index));
}

HardwareContext::HardwareContext()
: m_InputDeviceContext(), m_OutputDeviceContext()
{
	InitializeCriticalSection(&m_Lock);
	m_Initialized=FALSE;
}

HardwareContext::~HardwareContext()
{
	DeleteCriticalSection(&m_Lock);
}
 //[david.modify] 2008-06-19 18:00
 //===========================
// #define ATLAS_AUDIO_VOL_MSG			_T("_ATLAS_AUDIO_VOL_MSG")
#include <xllp_gpio_david.h>

DWORD timelast=0;


UINT32 g_u32ArrWinceVol[]={0x0, 0x33333333, 0x66666666,0x99999999,0xcccccccc,0xffffffff};

UINT32 g_u32ArrWinceVol2[]={0x0, 0x33333333, 0x66666666,0x99999999,0xcccccccc,0xffffffff};
//UINT32 g_u32ArrWinceVol2[]={0x0, 0xcccccccc, 0xdddddddd,0xeeeeeeee,0xff00ff00,0xffffffff};
#define WINCE_VOL_LEVEL (sizeof(g_u32ArrWinceVol)/sizeof(UINT32))

UINT32 Gain2Level(UINT32 u32Gain)
{
	UINT32 i=0;
	for(i=0;i<WINCE_VOL_LEVEL;i++){
		if(u32Gain<=g_u32ArrWinceVol[i])
			break;
	}
	return i;
}

UINT32 Level2Gain(UINT32 u32Level)
{
	if(u32Level>=(WINCE_VOL_LEVEL-1))
		u32Level = WINCE_VOL_LEVEL-1;
	return g_u32ArrWinceVol[u32Level];
}


int HardwareContext::SetAudioVolLevel(int nVolLevel)
{
//	m_nCurVolLevel = nVolLevel < m_nMaxVolLevel ? (nVolLevel < 0 ? 0 : nVolLevel) : m_nMaxVolLevel;
	if(nVolLevel<0) nVolLevel = 0;
	if(nVolLevel>=(WINCE_VOL_LEVEL-1)) nVolLevel = WINCE_VOL_LEVEL-1;


	UINT32 u32Gain = Level2Gain(nVolLevel);
	DPNOK(nVolLevel);
	DPNOK(u32Gain);	
	waveOutSetVolume(0, u32Gain);

	SetRegistryDWORD(HKEY_CURRENT_USER, TEXT("ControlPanel\\Volume"), TEXT("Volume"), u32Gain);
	
	int m_nCurVolLevel = nVolLevel;
	return  m_nCurVolLevel;
}

int HardwareContext::GetAudioVolLevel()
{
	DWORD u32Gain=0;
	waveOutGetVolume(0, &u32Gain);
	int nVolLevel =	Gain2Level(u32Gain);
	DPNOK(nVolLevel);
	DPNOK(u32Gain);		
	return nVolLevel;
}

VOID HardwareContext::AdjustAudioVol(BOOL bUp)
{
	DWORD timenow = GetTickCount();
				
	if(timelast==0)
	{
		timelast=timenow;
	}
	else
	{
		UINT32 gap=200;
		if(timenow-timelast<gap)
		{
			return ;
		}
		else
		{
			timelast=timenow;
		}
	}
				
	static UINT nVolWndMsg = 0;

 //[david.modify] 2008-06-20 09:58
 //================================
	SetAudioVolLevel(bUp ? GetAudioVolLevel() + 1 : GetAudioVolLevel() - 1);
 //================================	

//	RETAILMSG(1, (TEXT("AdjustAudioVolThread: GetAC97Volume()=0x%x\r\n"), ac97_regs[WM9712_HEADPHONE_VOLUME/2]));
	if (nVolWndMsg == 0)
	{
		nVolWndMsg = RegisterWindowMessage(ATLAS_AUDIO_VOL_MSG);
	}
		
	if (nVolWndMsg != 0)
	{
		PostMessage(HWND_BROADCAST, nVolWndMsg, 0, 0);
	}
}
	 
 void HardwareContext:: AdjustAudioVolThread()
{
	BOOL bVolUp;

	while(1)
	{
		WaitForSingleObject(m_hAudioVolChangeEvent, INFINITE);
		bVolUp = (BOOL) GetEventData(m_hAudioVolChangeEvent);
		DEBUGMSG(1, (TEXT("AdjustAudioVolThread: WaitForSingleObject - bVolUp=%d!\r\n"), bVolUp));
		AdjustAudioVol(bVolUp);		
	}
}
 
 void CallAdjustAudioVolThread(HardwareContext *pHWContext)
{
    pHWContext->AdjustAudioVolThread();
}
  //===========================


BOOL HardwareContext::Init(DWORD Index)
{
	UINT32 Irq;
	DWORD dwErr = ERROR_SUCCESS, bytes;	
	if (m_Initialized)
	{
			return(FALSE);
	}
	m_IntrAudio = SYSINTR_NOP;	

	//RETAILMSG(1, (TEXT("AUDIO sysintr is %d\r\n"), m_IntrAudio));
	//----- 1. Initialize the state/status variables -----
	m_DriverIndex		= Index;
#if (BSP_TYPE == BSP_SMDK2443)
	m_IntrAudio	        = IRQ_AUDIO;
#elif (BSP_TYPE == BSP_SMDK2450)
	m_IntrAudio	        = IRQ_I2S0;
#endif
	m_InPowerHandler	= FALSE;
	m_InputDMARunning   = FALSE;
	m_OutputDMARunning  = FALSE;
#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
    m_SavedInputDMARunning = FALSE;
    m_SavedOutputDMARunning = FALSE;
#endif
	m_InputDMAStatus	= DMA_CLEAR;
	m_OutputDMAStatus	= DMA_CLEAR;
	
	bIdlePwrDown		= TRUE;			// TRUE = Codec Power Turned off	
		//----- 2. Map the necessary descriptory channel and control registers into the driver's virtual address space -----
	if(!MapRegisters())
	{
		RETAILMSG(DBG_AUDIO, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - Failed to map config registers.\r\n")));
			goto Exit;
	}

	// 3.5 Init the GPIO ports
	IIS_Port_Init();
	
	//----- 3. Map the DMA buffers into driver's virtual address space -----
	if(!MapDMABuffers())
	{
		RETAILMSG(DBG_AUDIO, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - Failed to map DMA buffers.\r\n")));
			goto Exit;
	}

	// Call the OAL to translate the audio IRQ into a SYSINTR value.
	//
	Irq = IRQ_DMA1;  // audio input DMA interrupt.
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(UINT32), &m_dwSysintrInput, sizeof(UINT32), NULL))
	{
		RETAILMSG(TRUE, (TEXT("ERROR: HardwareContext::Init: Failed to obtain sysintr value for input interrupt.\r\n")));
		return FALSE;
	}
	RETAILMSG(1, (TEXT("Audio Input IRQ(DMA1) mapping: [IRQ:%d->sysIRQ:%d].\r\n"), Irq, m_dwSysintrInput));

	Irq = IRQ_DMA2;  // audio output DMA interrupt.
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(UINT32), &m_dwSysintrOutput, sizeof(UINT32), NULL))
	{
		RETAILMSG(TRUE, (TEXT("ERROR: HardwareContext::Init: Failed to obtain sysintr value for output interrupt.\r\n")));
		return FALSE;
	}
	RETAILMSG(1, (TEXT("Audio Output IRQ(DMA2) mapping: [IRQ:%d->sysIRQ:%d].\r\n"), Irq, m_dwSysintrOutput));

	//----- Initialize the i2c interface--------------
    //
    // Init I2C
    //
    hI2C = CreateFile( L"I2C0:",
                       GENERIC_READ|GENERIC_WRITE,
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, 0, 0);
                   
    if ( INVALID_HANDLE_VALUE == hI2C ) {
        dwErr = GetLastError();
        RETAILMSG(1, (TEXT("Error %d opening device '%s' \r\n"), dwErr, L"I2C0:" ));
        goto Exit;
    }

    RETAILMSG(1, (TEXT("CIS::CreateFile(\"I2C0\") \r\n")));


    //
    // Get Fastcall driver-to-driver entrypoints
    //
    if ( !DeviceIoControl(hI2C,
                          IOCTL_I2C_GET_FASTCALL, 
                          NULL, 0, 
                          &fc, sizeof(fc),
                          &bytes, NULL) ) 
    {
        dwErr = GetLastError();
        
		RETAILMSG(1, (TEXT("IOCTL_I2C_GET_FASTCALL ERROR \r\n")));
        goto Exit;
    }     


	ALLOCATE_RT_CODEC_CLASS

	if(!m_RTCodec)
	{
		return FALSE;
	}
	else
	{
		if(!m_RTCodec->Init(this))
		{
			return FALSE;
		}		
	}
	//Initialize the IIS registers
	I2S_Init();

	//----- 4. Configure the Codec -----
	InitCodec();
	//----- 5. Initialize the interrupt thread -----
	if (!InitInterruptThread())
	{
		RETAILMSG(DBG_AUDIO, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - Failed to initialize interrupt thread.\r\n")));
			goto Exit;
	}
//[david.modify] 2008-06-19 17:56
//=========================
	m_hAudioVolChangeEvent = CreateEvent(NULL, FALSE, FALSE, ATLAS_AUDIO_VOL_EVENT);
	if (m_hAudioVolChangeEvent == NULL)
	{
		ERRORMSG(1, (TEXT("Create Audio Vol Change Event failed!!!\r\n")));
		goto Exit;
	}

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CallAdjustAudioVolThread, this, 0, NULL);

//	msWait(0);
	
//=========================

	
//	Codec_channel111();
	m_Initialized=TRUE;
	
Exit:
	return(m_Initialized);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		MapRegisters()

Description:	Maps the config registers used by both the SPI and
				I2S controllers.

Notes:			The SPI and I2S controllers both use the GPIO config
				registers, so these MUST be initialized FIRST.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::MapRegisters()
{
	// IIS registers.
	//
		g_pIISregs = (volatile S3C2450_IISBUS_REG*)VirtualAlloc(0, sizeof(S3C2450_IISBUS_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pIISregs)
	{
		RETAILMSG(1, (TEXT("S3C2450_IISBUS_REG: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
#if (BSP_TYPE == BSP_SMDK2443)
	if (!VirtualCopy((PVOID)g_pIISregs, (PVOID)(S3C2450_BASE_REG_PA_IISBUS>>8), sizeof(S3C2450_IISBUS_REG), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
#elif (BSP_TYPE == BSP_SMDK2450)
	if (!VirtualCopy((PVOID)g_pIISregs, (PVOID)(S3C2450_BASE_REG_PA_IISBUS0>>8), sizeof(S3C2450_IISBUS_REG), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
#endif
	{
		RETAILMSG(1, (TEXT("S3C2450_IISBUS_REG: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	g_pIOPregs = (volatile S3C2450_IOPORT_REG*)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pIOPregs)
	{
		RETAILMSG(1, (TEXT("S3C2450_IOPORT_REG: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)g_pIOPregs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT>>8), sizeof(S3C2450_IOPORT_REG), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
	{
		RETAILMSG(1, (TEXT("S3C2450_IOPORT_REG: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	g_pDMAregs = (volatile S3C2450_DMA_REG*)VirtualAlloc(0, sizeof(S3C2450_DMA_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pDMAregs)
	{
		RETAILMSG(1, (TEXT("S3C2450_DMA_REG1: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)g_pDMAregs, (PVOID)(S3C2450_BASE_REG_PA_DMA>>8), sizeof(S3C2450_DMA_REG), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
	{
		RETAILMSG(1, (TEXT("S3C2450_DMA_REG1: VirtualCopy failed!\r\n")));
		return(FALSE);
	}


	s2450INT = (volatile S3C2450_INTR_REG*)VirtualAlloc(0, sizeof(S3C2450_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!s2450INT)
	{
		RETAILMSG(1, (TEXT("S3C2450_INTR_REG: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)s2450INT, (PVOID)(S3C2450_BASE_REG_PA_INTR>>8), sizeof(S3C2450_INTR_REG), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
	{
		RETAILMSG(1, (TEXT("S3C2450_INTR_REG: VirtualCopy failed!\r\n")));
		return(FALSE);
	}

	g_pCLKPWRreg = (volatile S3C2450_CLKPWR_REG *)VirtualAlloc(0, sizeof(S3C2450_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!g_pCLKPWRreg)
	{
		RETAILMSG(1, (TEXT("S3C2450_CLKPWR_REG: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)g_pCLKPWRreg, (PVOID)(S3C2450_BASE_REG_PA_CLOCK_POWER>>8), sizeof(S3C2450_CLKPWR_REG), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
	{
		RETAILMSG(1, (TEXT("S3C2450_CLKPWR_REG: VirtualCopy failed!\r\n")));
		return(FALSE);
	}
	

   
	//PowerUp();

	return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		Deinit()

Description:	Deinitializest the hardware: disables DMA channel(s), 
				clears any pending interrupts, powers down the audio
				codec chip, etc.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::Deinit()
{
	//----- 1. Disable the input/output channels -----
	//	AUDIO_IN_DMA_DISABLE();
	AUDIO_OUT_DMA_DISABLE();

	//----- 2. Disable/clear DMA input/output interrupts -----
	AUDIO_IN_CLEAR_INTERRUPTS();
	AUDIO_OUT_CLEAR_INTERRUPTS();

	//----- 3. Turn the audio hardware off -----
	AudioMute(DMA_CH_OUT | DMA_CH_MIC, TRUE);

	//----- 4. Unmap the control registers and DMA buffers -----
	UnmapRegisters();
	UnmapDMABuffers();

	return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		UnmapRegisters()

Description:	Unmaps the config registers used by both the SPI and
				I2S controllers.

Notes:			The SPI and I2S controllers both use the GPIO config
				registers, so these MUST be deinitialized LAST.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::UnmapRegisters()
{
	//----- 1. Free the fast driver-->driver calling mechanism object -----
	if(g_hUTLObject) 
	{
		CloseHandle(g_hUTLObject);
	}
	
	if ( g_pIISregs )
		VirtualFree((PVOID)g_pIISregs, 0, MEM_RELEASE);
	if ( g_pDMAregs )	
		VirtualFree((PVOID)g_pDMAregs, 0, MEM_RELEASE);
	if ( g_pIOPregs )			
		VirtualFree((PVOID)g_pIOPregs, 0, MEM_RELEASE);
	if ( g_pCLKPWRreg )		
		VirtualFree((PVOID)g_pCLKPWRreg, 0, MEM_RELEASE);

	if ( s2450INT )		
		VirtualFree((PVOID)s2450INT, 0, MEM_RELEASE);

	return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		MapDMABuffers()

Description:	Maps the DMA buffers used for audio input/output
				on the I2S bus.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::MapDMABuffers()
{
    PBYTE pVirtDMABufferAddr = NULL;
    DMA_ADAPTER_OBJECT Adapter;


    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
    //
    pVirtDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (AUDIO_DMA_PAGE_SIZE * 4), &g_PhysDMABufferAddr, FALSE);
    if (pVirtDMABufferAddr == NULL)
    {
        RETAILMSG(TRUE, (TEXT("WAVEDEV.DLL:HardwareContext::MapDMABuffers() - Failed to allocate DMA buffer.\r\n")));
        return(FALSE);
    }

    // Setup the DMA page pointers.
    // NOTE: Currently, input and output each have two DMA pages: these pages are used in a round-robin
    // fashion so that the OS can read/write one buffer while the audio codec chip read/writes the other buffer.
    //
    m_Output_pbDMA_PAGES[0] = pVirtDMABufferAddr;
    m_Output_pbDMA_PAGES[1] = pVirtDMABufferAddr + AUDIO_DMA_PAGE_SIZE;
    m_Input_pbDMA_PAGES[0]  = pVirtDMABufferAddr + (2 * AUDIO_DMA_PAGE_SIZE);
    m_Input_pbDMA_PAGES[1]  = pVirtDMABufferAddr + (3 * AUDIO_DMA_PAGE_SIZE);
    m_pVirtDMABufferAddr = pVirtDMABufferAddr;

    return(TRUE);
	
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		UnmapDMABuffers()

Description:	Unmaps the DMA buffers used for audio input/output
				on the I2S bus.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::UnmapDMABuffers()
{
	if(m_pVirtDMABufferAddr)
	{
		VirtualFree((PVOID)m_pVirtDMABufferAddr, 0, MEM_RELEASE);
	}

	return TRUE;
}

BOOL HardwareContext::Codec_channel()
{
	if( m_InputDMARunning & m_OutputDMARunning )
	{
			m_RTCodec->ChangeCodecPowerStatus(POWER_STATE_D1);
	}	   
	else if( m_InputDMARunning )
	{
			m_RTCodec->ChangeCodecPowerStatus(POWER_STATE_D2_PLAYBACK);	
			m_RTCodec->ChangeCodecPowerStatus(POWER_STATE_D1_RECORD);		
	}
	else if( m_OutputDMARunning )
	{
		//	m_RTCodec->ChangeCodecPowerStatus(POWER_STATE_D2_RECORD);
			m_RTCodec->ChangeCodecPowerStatus(POWER_STATE_D1_PLAYBACK);
	}
	else
	{
//			m_RTCodec->ChangeCodecPowerStatus(POWER_STATE_D2);	
	}

	return(TRUE);
}


MMRESULT HardwareContext::SetOutputGain (DWORD dwGain)
{
    m_dwOutputGain = dwGain & 0xffff; // save off so we can return this from GetGain - but only MONO
    // convert 16-bit gain to 5-bit attenuation
    UCHAR ucGain;
    if (m_dwOutputGain == 0) {
        ucGain = 0x3F; // mute: set maximum attenuation
    }
    else {
        ucGain = (UCHAR) ((0xffff - m_dwOutputGain) >> 11); // codec supports 64dB attenuation, we'll only use 32
    }
    ASSERT((ucGain & 0xC0) == 0); // bits 6,7 clear indicate DATA0 in Volume mode.

    return MMSYSERR_NOERROR;
}

MMRESULT HardwareContext::SetOutputMute (BOOL fMute)
{
    m_fOutputMute = fMute;

    return MMSYSERR_NOERROR;
}

BOOL HardwareContext::GetOutputMute (void)
{
    return m_fOutputMute;
}

DWORD HardwareContext::GetOutputGain (void)
{
    return m_dwOutputGain;
}

BOOL HardwareContext::GetInputMute (void)
{
    return m_fInputMute;
}

MMRESULT HardwareContext::SetInputMute (BOOL fMute)
{
    m_fInputMute = fMute;
    return m_InputDeviceContext.SetGain(fMute ? 0: m_dwInputGain);
}

DWORD HardwareContext::GetInputGain (void)
{
    return m_dwInputGain;
}

MMRESULT HardwareContext::SetInputGain (DWORD dwGain)
{
    m_dwInputGain = dwGain;
    if (! m_fInputMute) {
        m_InputDeviceContext.SetGain(dwGain);
    }
    return MMSYSERR_NOERROR;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		InitCodec()

Description:	Initializes the audio codec chip.

Notes:			The audio codec chip is intialized for output mode
				but powered down.  To conserve battery life, the chip
				is only powered up when the user starts playing a 
				file.

				Specifically, the powerup/powerdown logic is done 
				in the AudioMute() function.  If either of the 
				audio channels are unmuted, then the chip is powered
				up; otherwise the chip is powered own.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitCodec()
{

	if(m_RTCodec)
		m_RTCodec->InitRTCodecReg();

	return(TRUE);
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		InitOutputDMA()

Description:	Initializes the DMA channel for output.

Notes:			DMA Channel 2 is used for transmitting output sound
				data from system memory to the I2S controller.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitOutputDMA()
{

	RETAILMSG(DBG_AUDIO,(TEXT("+++InitOutputDMA\n")));

	//----- 1. Initialize the DMA channel for output mode and use the first output DMA buffer -----
    g_pDMAregs->DISRC2	= (int)(g_PhysDMABufferAddr.LowPart); 
	g_pDMAregs->DISRCC2 &= ~(SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);				// Source is system bus, increment addr

	//----- 2. Initialize the DMA channel to send data over the I2S bus -----
	g_pDMAregs->DIDST2	= IISFIFTX_PHYS; 
	g_pDMAregs->DIDSTC2 = (DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);	// Dest is  periperal bus, fixed addr

	//----- 3. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
	//		   single tx, single service, I2SSDO, I2S request, no auto-reload, half-word, tx count
	g_pDMAregs->DCON2 = (   HANDSHAKE_MODE | GENERATE_INTERRUPT
#if	DMA_FLAG
//					| TRANSFER_HALF_WORD | (AUDIO_DMA_PAGE_SIZE / 2 ) );				
					| TRANSFER_WORD | (AUDIO_DMA_PAGE_SIZE / 4 ) );
#else						   
					| NO_DMA_AUTO_RELOAD | TRANSFER_HALF_WORD | (AUDIO_DMA_PAGE_SIZE / 2) );	
//					| NO_DMA_AUTO_RELOAD | TRANSFER_WORD | (AUDIO_DMA_PAGE_SIZE / 4 ) );
#endif						   


	g_pDMAregs->DMAREQSEL2 = (DMAREQSEL_I2SSDO) |(1<<0);	// HW_SEL
	
	//----- 4. Reset the playback pointers -----
	AUDIO_RESET_PLAYBACK_POINTER();
	
	RETAILMSG(DBG_AUDIO,(TEXT("---InitOutputDMA\n")));

	return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StartOutputDMA()

Description:	Starts outputting the sound data to the audio codec
				chip via DMA.

Notes:			Currently, both playback and record share the same
				DMA channel.  Consequently, we can only start this
				operation if the input channel isn't using DMA.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::StartOutputDMA()
{
	volatile int i=0;
	static int tmp=0;
	
	g_ForCount=1;
	RETAILMSG(DBG_ON, (TEXT("StartOutputDMA\n")));
	
	//RETAILMSG(1, (TEXT("#####Start OUTPUT DMA.\r\n")));
#if (BSP_TYPE == BSP_SMDK2443)
	if(!m_OutputDMARunning)
#elif (BSP_TYPE == BSP_SMDK2450)
	if(!m_OutputDMARunning && (m_Dx == D0) )
#endif
	{
		//RETAILMSG(1, (TEXT("### it's the first time\r\n")));
		//----- 1. Initialize our buffer counters -----
		m_OutputDMARunning=TRUE;
		Codec_channel();	
		//--------------------------- for short length audio file hsjang 060518 ------------------------------
		if (g_NeedtoSleep)
		{
			Sleep(15);
		}
		// -------------------------------------------------------------------------------------------
		m_OutBytes[OUT_BUFFER_A]=m_OutBytes[OUT_BUFFER_B]=0;
		//----- 2. Prime the output buffer with sound data -----
		m_OutputDMAStatus = (DMA_DONEA | DMA_DONEB) & ~DMA_BIU;			
		ULONG OutputTransferred = TransferOutputBuffers(m_OutputDMAStatus);
		RETAILMSG(DBG_ON,(TEXT("OutputTransferred=%d\n"), OutputTransferred));
		// Turn ON output channel
		//----- 3. If we did transfer any data to the DMA buffers, go ahead and enable DMA -----

		if(OutputTransferred)
	   {
		   //RETAILMSG(1, (TEXT("### it's the first time ########\r\n")));
			//----- 4. Configure the DMA channel for playback -----
			if(!InitOutputDMA())
			{
				RETAILMSG(DBG_AUDIO, (TEXT("HardwareContext::StartOutputDMA() - Unable to initialize output DMA channel!\r\n")));
				goto START_ERROR;
			}

			////////////////////////////////////////////////////////////////////////////////
			// To correct left/right channel on ouput stream,
			// You should reset IISCON[0] bit.
			Lock();
			
			/*
			g_pIISregs->IISCON  &= ~IIS_INTERFACE_ENABLE;	// interface disable
			
			g_pIISregs->IISCON  |= TRANSMIT_DMA_REQUEST_ENABLE;
			g_pIISregs->IISCON  &= ~TRANSMIT_IDLE_CMD;	// Not Idle.(channel no stop)
			*/
			//g_pIISregs->IISMOD  &= ~(1<<9|1<<8); //IIS_TRANSMIT_MODE;	// Transmit only mode

			//----- 5. Make sure the audio isn't muted -----
			AudioMute(DMA_CH_OUT, FALSE);		

			//----- 6. Start the DMA controller -----
			AUDIO_RESET_PLAYBACK_POINTER();

			SELECT_AUDIO_DMA_OUTPUT_BUFFER_A();

			//Codec_channel();			// Turn ON output channel

			//RETAILMSG(1,(TEXT("+")));
			//RETAILMSG(1,(TEXT("-")));
			
			AUDIO_OUT_DMA_ENABLE();

			// wait for DMA to start.
			delay_count = 0;
	        	while((g_pDMAregs->DSTAT2&0xfffff)==0) // wait until DSTAT2 becomes zero
			{
	        	      if( delay_count++ > DELAY_COUNT )	break; // if delay count is over DELAY_COUNT defined for TIMEOUT
	        	}
			if(delay_count>DELAY_COUNT) RETAILMSG(1, (TEXT("TimeOut!!!\n")));
			
			//g_pIISregs->IISCON  |= IIS_INTERFACE_ENABLE;

			

			Unlock();
			////////////////////////////////////////////////////////////////////////////////

			// change the buffer pointer
			SELECT_AUDIO_DMA_OUTPUT_BUFFER_B();
			// Set DMA for B Buffer

		

		}
		else	// We didn't transfer any data, so DMA wasn't enabled
		{
		RETAILMSG(DBG_AUDIO,(TEXT("<<<<m_OutputDMARunning=FALSE; >>>>>\n")));
			m_OutputDMARunning=FALSE;
		}
	
		
	}

	RETAILMSG(DBG_AUDIO,(TEXT("---StartOutputDMA\n")));


	return TRUE;

START_ERROR:
	return FALSE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StopOutputDMA()

Description:	Stops any DMA activity on the output channel.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
void HardwareContext::StopOutputDMA()
{
	//RETAILMSG(DBG_ON, (TEXT("+StopOutputDMA\r\n")));
	//----- 1. If the output DMA is running, stop it -----
	if (m_OutputDMARunning)
	{
		m_OutputDMAStatus = DMA_CLEAR;				
		//StopI2SClock();
		AUDIO_OUT_DMA_DISABLE();
		AUDIO_OUT_CLEAR_INTERRUPTS();
		/*
		g_pIISregs->IISCON &= ~TRANSMIT_DMA_REQUEST_ENABLE;	// Disable TX DMA
		g_pIISregs->IISCON |= TRANSMIT_IDLE_CMD;		// TXCHPAUSE
		*/
		//g_pIISregs->IISMOD  &= ~IIS_TRANSMIT_MODE;		// reserved
#if (BSP_TYPE == BSP_SMDK2443)
		AudioMute(DMA_CH_OUT, TRUE);		
#elif (BSP_TYPE == BSP_SMDK2450)
#endif
	}

	m_OutputDMARunning = FALSE;
#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
		AudioMute(DMA_CH_OUT, TRUE);
#endif
	Codec_channel();
	g_ForCount=1;
	//RETAILMSG(DBG_ON, (TEXT("-StopOutputDMA\r\n")));

}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		InitInputDMA()

Description:	Initializes the DMA channel for input.

Notes:			***** NOT IMPLEMENTED *****

				The following routine is not implemented due to a
				hardware bug in the revision of the Samsung SC2450
				CPU this driver was developed on.  See the header
				at the top of this file for details.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitInputDMA()
{

	RETAILMSG(DBG_AUDIO,(TEXT("+++InitInputDMA\r\n")));
	//============================ Configure DMA Channel 1 ===========================
	//------ On platforms with the revision of the Samsung SC2450 CPU with the IIS SLAVE bug fix, this -----
	//		 code can be used to configure DMA channel 1 for input.

	//----- 1. Initialize the DMA channel for input mode and use the first input DMA buffer -----
	//g_pDMAregs->DISRC1	= (unsigned int)IISFIFRX_PHYS;	// DonGo modified.
	g_pDMAregs->DISRC1	= (unsigned int)0x55000014;	// DonGo modified.
	g_pDMAregs->DISRCC1 = (SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);				// Source is periperal bus, fixed addr

	
	//----- 2. Initialize the DMA channel to receive data over the I2S bus -----
    g_pDMAregs->DIDST1	= (int)(g_PhysDMABufferAddr.LowPart); 
	g_pDMAregs->DIDSTC1 &= ~(DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);	// Destination is system bus, increment addr

	//----- 3. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
	//		   single tx, single service, I2SSDI, I2S request, no auto-reload, half-word, tx count
	g_pDMAregs->DCON1	= (  HANDSHAKE_MODE | GENERATE_INTERRUPT 
#if DMA_FLAG	
//						   | TRANSFER_HALF_WORD | (AUDIO_DMA_PAGE_SIZE / 2) );
						   | TRANSFER_WORD | (AUDIO_DMA_PAGE_SIZE / 4) );
#else						   
						   | NO_DMA_AUTO_RELOAD | TRANSFER_HALF_WORD | (AUDIO_DMA_PAGE_SIZE / 2) );
//						   | NO_DMA_AUTO_RELOAD | TRANSFER_WORD | (AUDIO_DMA_PAGE_SIZE / 4) );
#endif						   

	g_pDMAregs->DMAREQSEL1 = (DMAREQSEL_I2SSDI) |(1) ;

	
	RETAILMSG(DBG_AUDIO,(TEXT("---InitInputDMA\r\n")));
	return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StartInputDMA()

Description:	Starts inputting the recorded sound data from the 
				audio codec chip via DMA.

Notes:			***** NOT IMPLEMENTED *****

				The following routine is not implemented due to a
				hardware bug in the revision of the Samsung SC2450
				CPU this driver was developed on.  See the header
				at the top of this file for details.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::StartInputDMA()
{
	
	//------ On platforms with the revsion of the Samsung SC2450 CPU with the IIS SLAVE bug fix, this -----
	//		 code can be used to configure DMA channel 1 for input.

	//RETAILMSG(1,(TEXT("+++StartInputDMA\n")));
	
	if(!m_InputDMARunning)
		{
		//RETAILMSG(1,(TEXT("+++InputDMArunning first\r\n")));
		//----- 1. Initialize our buffer counters -----
		m_InputDMARunning=TRUE;

		Codec_channel();		// Turn On Input channel

		m_InBytes[IN_BUFFER_A]=m_InBytes[IN_BUFFER_B]=0;

		//----- 2. Prime the output buffer with sound data -----
		m_InputDMAStatus = (DMA_DONEA | DMA_DONEB) & ~DMA_BIU;	

		//----- 3. Configure the DMA channel for record -----
		if(!InitInputDMA())
		{
			RETAILMSG(1, (TEXT("HardwareContext::StartInputDMA() - Unable to initialize input DMA channel!\r\n")));
			goto START_ERROR;
		}
		/*
		g_pIISregs->IISCON  &= ~(RECEIVE_DMA_PAUSE|RECEIVE_IDLE_CMD);	 
		g_pIISregs->IISCON  |= RECEIVE_DMA_REQUEST_ENABLE;	 
		*/
		//g_pIISregs->IISMOD  |= (IIS_RECEIVE_MODE|MASTER_CLOCK_FREQ_384fs);
	
		//----- 4. Make sure the audio isn't muted -----
		AudioMute(DMA_CH_MIC, FALSE);					

		//----- 5. Start the input DMA -----
		AUDIO_RESET_RECORD_POINTER();
		SELECT_AUDIO_DMA_INPUT_BUFFER_A();

		//Codec_channel();		// Turn On Input channel
		
		//g_pDMAregs->DMASKTRIG1 = ENABLE_DMA_CHANNEL;	
		AUDIO_IN_DMA_ENABLE();
		
		/*
	#if DMA_FLAG
		// wait for DMA to start.
	RETAILMSG(DBG_AUDIO, (TEXT("Going into the DMA start loop")));
		//while((g_pDMAregs->DSTAT1&0xfffff)!=0);
	RETAILMSG(DBG_AUDIO, (TEXT("Out of the DMA start loop")));
		// change the buffer pointer
			SELECT_AUDIO_DMA_INPUT_BUFFER_B();
	#endif
	*/
	}

	// wait for DMA to start.
	delay_count = 0;
	while((g_pDMAregs->DSTAT1&0xfffff)==0)
	{
	      	if( delay_count++ > DELAY_COUNT )	break;
	}
	if(delay_count>DELAY_COUNT) RETAILMSG(1, (TEXT("TimeOut!!!\n")));


	SELECT_AUDIO_DMA_INPUT_BUFFER_B();

		
	//RETAILMSG(1,(TEXT("---StartInputDMA\n")));
	
	return(TRUE);

START_ERROR:
	return(FALSE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		StopInputDMA()

Description:	Stops any DMA activity on the input channel.

Notes:			***** IMPLEMENTED *****

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
void HardwareContext::StopInputDMA()
{

	//------ On platforms with the revsion of the Samsung SC2450 CPU with the IIS SLAVE bug fix, this -----
	//		 code can be used to configure DMA channel 1 for input.

	//RETAILMSG(1,(TEXT("+++StopInputDMA\n")));
	//----- 1. If the input DMA is running, stop it -----
	if (m_InputDMARunning)
	{
		m_InputDMAStatus = DMA_CLEAR;				

		//StopI2SClock();

		/*
		g_pIISregs->IISCON &= ~RECEIVE_DMA_REQUEST_ENABLE;
		g_pIISregs->IISCON |= RECEIVE_IDLE_CMD;		// RXCHPAUSE
		
		g_pDMAregs->DMASKTRIG1 |= STOP_DMA_TRANSFER;
		g_pDMAregs->DMASKTRIG1 &= ~ENABLE_DMA_CHANNEL;
		*/
		AUDIO_IN_DMA_DISABLE();
		//g_pIISregs->IISMOD  |= IIS_TRANSMIT_MODE;		// reserved		
		AUDIO_IN_CLEAR_INTERRUPTS();

#if (BSP_TYPE == BSP_SMDK2443)
		AudioMute(DMA_CH_MIC, TRUE);		
#elif (BSP_TYPE == BSP_SMDK2450)
#endif	
	}

	m_InputDMARunning = FALSE;
#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
		AudioMute(DMA_CH_MIC, TRUE);	
#endif
	Codec_channel();
	//RETAILMSG(1,(TEXT("---StopInputDMA\n")));
}


DWORD HardwareContext::GetInterruptThreadPriority()
{
	HKEY hDevKey;
	DWORD dwValType;
	DWORD dwValLen;
	DWORD dwPrio = 249; // Default priority

	hDevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);
	if (hDevKey)
	{
		dwValLen = sizeof(DWORD);
		RegQueryValueEx(
			hDevKey,
			TEXT("Priority256"),
			NULL,
			&dwValType,
			(PUCHAR)&dwPrio,
			&dwValLen);
		RegCloseKey(hDevKey);
	}

	return dwPrio;
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		InitInterruptThread()

Description:	Initializes the IST for handling DMA interrupts.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::InitInterruptThread()
{

	m_hAudioInterrupt = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!m_hAudioInterrupt)
	{
		return(FALSE);
	}

	if (! InterruptInitialize(m_dwSysintrOutput, m_hAudioInterrupt, NULL, 0)) {
		RETAILMSG(1, (TEXT("Unable to initialize output interrupt\n")));
		return FALSE;
	}
	if (! InterruptInitialize(m_dwSysintrInput, m_hAudioInterrupt, NULL, 0)) {
		RETAILMSG(1, (TEXT("Unable to initialize input interrupt\n")));
		return FALSE;
	}
	
	m_hAudioInterruptThread  = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
											0,
											(LPTHREAD_START_ROUTINE)CallInterruptThread,
											this,
											0,
											NULL);
	if (!m_hAudioInterruptThread)
	{
		RETAILMSG(1, (TEXT("\ninterrupt thread open error.\n\n")));		
		return(FALSE);
	}

	// Bump up the priority since the interrupt must be serviced immediately.
	CeSetThreadPriority(m_hAudioInterruptThread, GetInterruptThreadPriority());

	return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		PowerUp()

Description:	Powers up the audio codec chip.

Notes:			Currently, this function is unimplemented because
				the audio codec chip is ONLY powered up when the 
				user wishes to play or record.  The AudioMute() function
				handles the powerup sequence.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
void HardwareContext::PowerUp()
{
	I2S_Init();
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		PowerDown()

Description:	Powers down the audio codec chip.

Notes:			Even if the input/output channels are muted, this
				function powers down the audio codec chip in order
				to conserve battery power.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
void HardwareContext::PowerDown()
{
	RETAILMSG(DBG_ON, (TEXT("HardwareContext::PowerDown\r\n")));
	StopOutputDMA();
	AudioMute((DMA_CH_OUT | DMA_CH_MIC), TRUE);
}


//############################################ Helper Functions #############################################

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		TransferOutputBuffer()

Description:	Retrieves the next "mixed" audio buffer of data to
				DMA into the output channel.

Returns:		Number of bytes needing to be transferred.
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferOutputBuffer(ULONG NumBuf)
{
	ULONG BytesTransferred = 0;
	PBYTE pBufferStart = m_Output_pbDMA_PAGES[NumBuf];
	PBYTE pBufferEnd = pBufferStart + AUDIO_DMA_PAGE_SIZE;
	PBYTE pBufferLast;

	__try
	{
		pBufferLast = m_OutputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd,NULL);
		BytesTransferred = m_OutBytes[NumBuf] = pBufferLast-pBufferStart;


		// Enable if you need to clear the rest of the DMA buffer
		StreamContext::ClearBuffer(pBufferLast,pBufferEnd);
		if(NumBuf == OUT_BUFFER_A)			// Output Buffer A
		{
			m_OutputDMAStatus &= ~DMA_DONEA;
			m_OutputDMAStatus |= DMA_STRTA;
		}
		else									// Output Buffer B
		{
			m_OutputDMAStatus &= ~DMA_DONEB;
			m_OutputDMAStatus |= DMA_STRTB;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		RETAILMSG(DBG_AUDIO, (TEXT("WAVDEV2.DLL:TransferOutputBuffer() - EXCEPTION: %d"), GetExceptionCode()));
	}
	
	RETAILMSG(DBG_AUDIO, (TEXT("pBufferTransferred/NumBuf=%d(%d).\n"), BytesTransferred,NumBuf));

	return BytesTransferred;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		TransferOutputBuffers()

Description:	Determines which output buffer (A or B) needs to 
				be filled with sound data.  The correct buffer is
				then populated with data and ready to DMA to the 
				output channel.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferOutputBuffers(DWORD dwDCSR)
{
	ULONG BytesTransferred = 0;

	ULONG BytesTotal;
	DWORD Bits = dwDCSR & (DMA_DONEA|DMA_DONEB|DMA_BIU);

	switch (Bits)
	{
	case 0:
	case DMA_BIU:
		// No done bits set- must not be my interrupt
		return 0;
	case DMA_DONEA|DMA_DONEB|DMA_BIU:
		// Load B, then A
		BytesTransferred = TransferOutputBuffer(OUT_BUFFER_B);
		// fall through
	case DMA_DONEA: // This should never happen!
	case DMA_DONEA|DMA_BIU:
		BytesTransferred += TransferOutputBuffer(OUT_BUFFER_A);		// charlie, A => B
		break;
	case DMA_DONEA|DMA_DONEB:
		// Load A, then B
		BytesTransferred = TransferOutputBuffer(OUT_BUFFER_A);
#if DMA_FLAG		
		// charlie
		BytesTransferred += TransferOutputBuffer(OUT_BUFFER_B);
		break;		// charlie
#endif		
		// fall through
	case DMA_DONEB|DMA_BIU: // This should never happen!
	case DMA_DONEB:
		// Load B
		BytesTransferred += TransferOutputBuffer(OUT_BUFFER_B);		// charlie, B => A
		break;
	}
	// If it was our interrupt, but we weren't able to transfer any bytes
	// (e.g. no full buffers ready to be emptied)
	// and all the output DMA buffers are now empty, then stop the output DMA
	BytesTotal = m_OutBytes[OUT_BUFFER_A]+m_OutBytes[OUT_BUFFER_B];

	if (BytesTotal==0)
	{
		//RETAILMSG(1,(TEXT("+++ NO BYTES transferred by DMA\r\n")));
		StopOutputDMA();
	}
#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
	else
	{
		StartOutputDMA();		// for DMA resume when wake up
	}
#endif

	RETAILMSG(DBG_AUDIO,(TEXT("--TransferOutputBuffers(%d+%d)\n"), m_OutBytes[OUT_BUFFER_A],m_OutBytes[OUT_BUFFER_B]));

	return BytesTransferred;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		TransferInputBuffer()

Description:	Retrieves the chunk of recorded sound data and inputs
				it into an audio buffer for potential "mixing".

Returns:		Number of bytes needing to be transferred.
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferInputBuffer(ULONG NumBuf)
{
	ULONG BytesTransferred = 0;

	PBYTE pBufferStart = m_Input_pbDMA_PAGES[NumBuf];
	PBYTE pBufferEnd = pBufferStart + AUDIO_DMA_PAGE_SIZE;
	PBYTE pBufferLast;

	__try
	{
		pBufferLast = m_InputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd,NULL);
		BytesTransferred = m_InBytes[NumBuf] = pBufferLast-pBufferStart;

		if(NumBuf == IN_BUFFER_A)			// Input Buffer A
		{
			m_InputDMAStatus &= ~DMA_DONEA;
			m_InputDMAStatus |= DMA_STRTA;
		}
		else								// Input Buffer B
		{
			m_InputDMAStatus &= ~DMA_DONEB;
			m_InputDMAStatus |= DMA_STRTB;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		RETAILMSG(DBG_AUDIO, (TEXT("WAVDEV2.DLL:TransferInputBuffer() - EXCEPTION: %d"), GetExceptionCode()));
	}

	return BytesTransferred;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		TransferInputBuffers()

Description:	Determines which input buffer (A or B) needs to 
				be filled with recorded sound data.  The correct 
				buffer is then populated with recorded sound data
				from the input channel.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
ULONG HardwareContext::TransferInputBuffers(DWORD dwDCSR)
{
	ULONG BytesTransferred=0;
	DWORD Bits = dwDCSR & (DMA_DONEA|DMA_DONEB|DMA_BIU);

	switch (Bits)
	{
	case 0:
	case DMA_BIU:
		// No done bits set- must not be my interrupt
		return 0;
	case DMA_DONEA|DMA_DONEB|DMA_BIU:
		// Load B, then A
		BytesTransferred = TransferInputBuffer(IN_BUFFER_B);
		// fall through
	case DMA_DONEA: // This should never happen!
	case DMA_DONEA|DMA_BIU:
		// Load A
		BytesTransferred += TransferInputBuffer(IN_BUFFER_A);
		break;
	case DMA_DONEA|DMA_DONEB:
		// Load A, then B
		BytesTransferred = TransferInputBuffer(IN_BUFFER_A);
#if DMA_FLAG		
		BytesTransferred += TransferInputBuffer(IN_BUFFER_B);
		break;
#endif		
		// fall through
	case DMA_DONEB|DMA_BIU: // This should never happen!
	case DMA_DONEB:
		// Load B
		BytesTransferred += TransferInputBuffer(IN_BUFFER_B);
		break;
	}

	// If it was our interrupt, but we weren't able to transfer any bytes
	// (e.g. no empty buffers ready to be filled)
	// Then stop the input DMA
	if (BytesTransferred==0)
	{
		//RETAILMSG(1,(TEXT("+++ NO BYTES transferred by DMA\r\n")));
		StopInputDMA();
	}
#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
	else
	{
		StartInputDMA();		// for DMA resume when wake up
	}
#endif
	return BytesTransferred;
}


void HardwareContext::InterruptThread()
{
	ULONG InputTransferred, OutputTransferred;
	BOOL dmaInterruptSource = 0;

	// Fast way to access embedded pointers in wave headers in other processes.
	SetProcPermissions((DWORD)-1);
	
RETAILMSG(DBG_AUDIO,(TEXT("InterruptThread... wait event\n")));
	while(TRUE)
	{
		RETAILMSG(DBG_AUDIO,(TEXT("Wait event\n")));
#if (BSP_TYPE == BSP_SMDK2443)
		RETAILMSG(DBG_AUDIO,(TEXT(" INTMSK[0x%x]\n"),s2450INT->INTMSK));
#elif (BSP_TYPE == BSP_SMDK2450)
		RETAILMSG(DBG_AUDIO,(TEXT(" INTMSK1[0x%x]\n"),s2450INT->INTMSK1));
#endif
		
		WaitForSingleObject(m_hAudioInterrupt, INFINITE);
#if (BSP_TYPE == BSP_SMDK2443)
		RETAILMSG(DBG_AUDIO,(TEXT("INTMSK: 0x%x\n"),s2450INT->INTMSK));
#elif (BSP_TYPE == BSP_SMDK2450)
		RETAILMSG(DBG_AUDIO,(TEXT("INTMSK1: 0x%x\n"),s2450INT->INTMSK1));
#endif
		g_ForCount++;
		dmaInterruptSource = 0;
		//----- 1. Grab the lock -----
		Lock();

		__try
		{
			RETAILMSG(DBG_AUDIO,(TEXT("try\n")));

			//----- 3. Determine the interrupt source (input DMA operation or output DMA operation?) -----
			//----- NOTE:	Often, platforms use two separate DMA channels for input/output operations but
			//				have the OAL return SYSINTR_AUDIO as the interrupt source.  If this is the case,
			//				then the interrupt source (input or output DMA channel) must be determined in
			//				this step.
			// charlie, determine the interrupt source

			/*
			RETAILMSG(1,(TEXT("INTMSK: 0x%x\n"),s2450INT->INTMSK));
			RETAILMSG(1,(TEXT("INTPND: 0x%x\n"),s2450INT->INTPND));
			RETAILMSG(1,(TEXT("SUBSRCPND: 0x%x\n"),s2450INT->SUBSRCPND));
			RETAILMSG(1,(TEXT("INTSUBMSK: 0x%x\n"),s2450INT->INTSUBMSK));*/
			if( s2450INT->INTSUBMSK & (1<< IRQ_SUB_DMA1))
			{
				delay_count = 0;
				dmaInterruptSource |= DMA_CH_MIC;								// Input DMA is supported...
				// For determine the interrupt source
				//----- 2. Acknowledge the DMA interrupt -----
				//RETAILMSG(1,(TEXT("MIC-------------------\n")));
				
				
				InterruptDone(m_dwSysintrInput);
			}
			
			if( s2450INT->INTSUBMSK & (1<< IRQ_SUB_DMA2)){
				dmaInterruptSource |= DMA_CH_OUT;								// Output DMA is supported...
				// For determine the interrupt source
				//----- 2. Acknowledge the DMA interrupt -----
				InterruptDone(m_dwSysintrOutput);
			}

#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
			if (m_Dx==D0)
			{
#endif
			//----- 4. Handle any interrupts on the input source -----
			//		   NOTE: The InterruptDone() call below automatically clears the interrupt.
			if((dmaInterruptSource == DMA_CH_MIC))
			{
				//----- Determine which buffer just completed the DMA transfer -----
				if(m_InputDMAStatus & DMA_BIU)
				{
					m_InputDMAStatus &= ~DMA_STRTB;							// Buffer B just completed...
					m_InputDMAStatus |= DMA_DONEB;

					m_InputDMAStatus &= ~DMA_BIU;							// Buffer A is in use
#if DMA_FLAG					
					SELECT_AUDIO_DMA_INPUT_BUFFER_B();
#else					
					SELECT_AUDIO_DMA_INPUT_BUFFER_A();
#endif					
					//RETAILMSG(1,(TEXT("try2-MIC_B(%x)\n"), m_InputDMAStatus));
				}else
				{
					m_InputDMAStatus &= ~DMA_STRTA;							// Buffer A just completed...
					m_InputDMAStatus |= DMA_DONEA;

					m_InputDMAStatus |= DMA_BIU;							// Buffer B is in use
#if DMA_FLAG
					SELECT_AUDIO_DMA_INPUT_BUFFER_A();
#else										
					SELECT_AUDIO_DMA_INPUT_BUFFER_B();
#endif					
					//RETAILMSG(1,(TEXT("try2_A(%x)\n"), m_InputDMAStatus));
				}

#if !DMA_FLAG		
				//----- 5. Schedule the next DMA transfer -----
				AUDIO_IN_DMA_ENABLE();
#endif				
				
				//----- 6. Retrieve the next chunk of recorded data from the non-playing buffer -----
				InputTransferred = TransferInputBuffers(m_InputDMAStatus);			
			}
			else
			{
				//----- 7. Handle any interrupts on the output source -----
				//		   NOTE: The InterruptDone() call below automatically clears the interrupt.

				//----- Determine which buffer just completed the DMA transfer -----
				if(m_OutputDMAStatus & DMA_BIU)
				{
					m_OutputDMAStatus &= ~DMA_STRTB;						// Buffer A just completed...
					m_OutputDMAStatus |= DMA_DONEB;

					m_OutputDMAStatus &= ~DMA_BIU;							// Buffer B is in use

					// Wait for start DMA.
					delay_count = 0;
					while((g_pDMAregs->DSTAT2&0xfffff)==0){
	        				if( delay_count++ > DELAY_COUNT )	break;
					}

#if DMA_FLAG		
					SELECT_AUDIO_DMA_OUTPUT_BUFFER_B();		// charlie. B => A
#else				
					SELECT_AUDIO_DMA_OUTPUT_BUFFER_A();		// charlie. B => A	
#endif					
				RETAILMSG(DBG_AUDIO,(_T("try-DMA_BIU-B(%x)\n"), m_OutputDMAStatus));

				}else
				{
					m_OutputDMAStatus &= ~DMA_STRTA;						// Buffer B just completed...
					m_OutputDMAStatus |= DMA_DONEA;

					m_OutputDMAStatus |= DMA_BIU;							// Buffer A is in use


					// Wait for start DMA.
					delay_count = 0;
					while((g_pDMAregs->DSTAT2&0xfffff)==0){
	        				if( delay_count++ > DELAY_COUNT )	break;
					}
					
#if DMA_FLAG					
					SELECT_AUDIO_DMA_OUTPUT_BUFFER_A();		// charlie. B => A
#else				
					SELECT_AUDIO_DMA_OUTPUT_BUFFER_B();		// charlie. B => A	
#endif					
				}

//				SET_AUDIO_DMA_OUTPUT_DCON();
//				AUDIO_OUT_DMA_ENABLE();

#if !DMA_FLAG
				//----- 8. Schedule the next DMA transfer -----
				AUDIO_OUT_DMA_ENABLE();
#endif
				RETAILMSG(DBG_AUDIO,(_T("try-!DMA_BIU-A(%x)\n"), m_OutputDMAStatus));

				//----- 9. Fill the non-playing buffer with the next chunk of audio data to play -----
				OutputTransferred = TransferOutputBuffers(m_OutputDMAStatus);
			}
#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
			}
#endif

		}
		__except(EXCEPTION_EXECUTE_HANDLER) 
		{
			RETAILMSG(DBG_AUDIO, (TEXT("WAVDEV2.DLL:InterruptThread() - EXCEPTION: %d"), GetExceptionCode()));
		}

		//----- 10. Unlock ----- 
		Unlock();
	}  
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		AudioMute()

Description:	Mutes/unmutes the specified audio channel.

Notes:			If both audio channels are MUTED, then the chip 
				is powered down to conserve battery life.  
				Alternatively, if either audio channel is unMUTED, 
				the chip is powered up.

Returns:		Boolean indicating success
-------------------------------------------------------------------*/
BOOL HardwareContext::AudioMute(DWORD channel, BOOL bMute)
{
	#if 0
	static DWORD dwActiveChannel = 0;
	{
		if(bMute)
		{
			_WrL3Addr(0x14);
			_WrL3Data(0xa4,0);
		}
		else
		{
			_WrL3Addr(0x14);
			_WrL3Data(0xa0,0);
		}
	}		
	#else

	if((channel & DMA_CH_OUT ))
	{	
		if(m_RTCodec)
		{	
		//	m_RTCodec->AudioOutEnable(RT_WAVOUT_PATH_HP,bMute);
		//	m_RTCodec->AudioOutEnable(RT_WAVOUT_PATH_SPK,bMute);
		}
	}		
	#endif
	return(TRUE);
	
}


void CallInterruptThread(HardwareContext *pHWContext)
{
	pHWContext->InterruptThread();
}

DWORD HardwareContext::Open(void)
{
	DWORD mmErr = MMSYSERR_NOERROR;

	// Don't allow play when not on, if there is a power constraint upon us.
	if ( D0 != m_Dx )
	{
		// Tell the Power Manager we need to power up.
		// If there is a power constraint then fail.
		RETAILMSG(DBG_WAV, (TEXT("[A_HW] Open : DevicePowerNotify - D0\r\n")));

		DWORD dwErr = DevicePowerNotify(_T("WAV1:"), D0, POWER_NAME);
		if ( ERROR_SUCCESS !=  dwErr )
		{
			RETAILMSG(DBG_WAV, (TEXT("WAVEDEV::Open:DevicePowerNotify ERROR1: %u\r\n"), dwErr ));
			dwErr = DevicePowerNotify(_T("WAV1:"), D0, POWER_NAME);
			mmErr = MMSYSERR_ERROR;
		}
	}

	return mmErr;
}

DWORD HardwareContext::Close(void)
{
	DWORD mmErr = MMSYSERR_NOERROR;
	DWORD dwErr;

	if (bInputClose==TRUE) bIsRecording = FALSE;
	else bIsPlaying = FALSE;
	// we are done so inform Power Manager to power us down
	if(bIsPlaying==FALSE && bIsRecording==FALSE)
	{
		bClosing = TRUE;
    		if ( D4 != m_Dx )
		{
    			RETAILMSG(DBG_WAV, (TEXT("[A_HW] Close : DevicePowerNotify - D4\r\n")));
    			dwErr = DevicePowerNotify(_T("WAV1:"), (_CEDEVICE_POWER_STATE)D4, POWER_NAME);	

			if ( ERROR_SUCCESS !=  dwErr )
			{
				RETAILMSG(DBG_WAV, (TEXT("WAVEDEV::Close:DevicePowerNofify ERROR1: %u\r\n"), dwErr));
				dwErr = DevicePowerNotify(_T("WAV1:"), (_CEDEVICE_POWER_STATE)D4, POWER_NAME);	
				mmErr = MMSYSERR_ERROR;
			}
		}
	}

	if(bInputClose==TRUE && m_InputDMARunning==TRUE)	
	{
		RETAILMSG(1, (TEXT("[A_HW] Close : StopInputDMA Exception!\r\n")));
		StopInputDMA();

	}
	bInputClose=FALSE;

	return mmErr;
}

void HardwareContext::CodecPowerDown()
{
	if(!bIdlePwrDown)
	{
		RETAILMSG(DBG_WAV,(TEXT("[A_HW] CodecPowerDown : Done\r\n")));	
	}
	else
		RETAILMSG(DBG_WAV,(TEXT("[A_HW] CodecPowerDown : Already Off\r\n")));	
}

BOOL 
HardwareContext::IOControl(DWORD  dwOpenData, 
    DWORD  dwCode,
    PBYTE  pBufIn,
    DWORD  dwLenIn,
    PBYTE  pBufOut,
    DWORD  dwLenOut,
    PDWORD pdwActualOut)
{
    DWORD dwErr = ERROR_SUCCESS;    
    BOOL  bRc = TRUE;
	WCHAR wPState[20];
	DWORD dFlag;

    
    switch (dwCode) {
        //
        // Power Management
        //
    case IOCTL_POWER_QUERY:
            if(pBufOut != NULL && dwLenIn == sizeof(CEDEVICE_POWER_STATE))
            {
                // return a good status on any valid query, since we are always ready to
                // change power states.
                bRc = FALSE;
                __try
                {
                    CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE) pBufOut;
                    if(VALID_DX(NewDx))
                    {
                        // this is a valid Dx state so return a good status
                        bRc = TRUE;
                    }
                    RETAILMSG(1, (L"%s: IOCTL_POWER_QUERY %u %s\r\n", TEXT("AUDIO"), 
						NewDx, bRc == TRUE ? L"succeeded" : L"failed"));
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    RETAILMSG(1, (L"%s: exception in ioctl2\r\n"));
                    bRc=FALSE;
                }
            }
            break;
        case IOCTL_POWER_CAPABILITIES: 
        {
            PPOWER_CAPABILITIES ppc;
            
            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(POWER_CAPABILITIES)) ) {
                bRc = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            
            ppc = (PPOWER_CAPABILITIES)pBufOut;
            
            memset(ppc, 0, sizeof(POWER_CAPABILITIES));

            // support D0, D4
            ppc->DeviceDx = DX_MASK(D0)|DX_MASK(D4);

			DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_CAPABILITIES = 0x%x\r\n"), ppc->DeviceDx));
			RETAILMSG(DBG_WAV,(TEXT("WAVDEV::IOCTL_POWER_CAPABILITIES\r\n")));
			
            
            *pdwActualOut = sizeof(POWER_CAPABILITIES);
            
        } break;
        
     	case IOCTL_POWER_SET:
			
			CEDEVICE_POWER_STATE NewDx;

			if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) )
			{
				bRc = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
				break;
			}

			NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;

			RETAILMSG(1, (TEXT("[A_HW] IOControl(IOCTL_POWER_SET) : D%u => D%u \r\n"), m_Dx, NewDx));

			if ( VALID_DX(NewDx) )
			{
				// grab the CS since the normal Xxx_PowerXxx can not.
#if (BSP_TYPE == BSP_SMDK2443)
				Lock();
#elif (BSP_TYPE == BSP_SMDK2450)
#endif
				switch ( NewDx )
				{
					case D0:
						if (m_Dx != D0)
						{
						    	m_Dx = D0;

						    	// Codec Power On, if Off
							if (bIdlePwrDown == TRUE)				// When Codec is off,
							{
								RETAILMSG(1, (TEXT("[A_HW] IOControl : Turning Codec On\r\n")));


								/*g_pCLKPWRreg->CLKSRC = g_pCLKPWRreg->CLKSRC & ~(0x3<<14); //clock from Divided EPLL
								g_pIISregs->IISMOD = g_pIISregs->IISMOD & ~(0x3<<10) | (0x01<<10);	//clock from EPLL*/
								g_pIISregs->IISMOD = g_pIISregs->IISMOD & ~(0x3<<10);	// PCLK
								//g_pCLKPWRreg->CLKCON |= IIS_INTERNAL_CLOCK_ENABLE;		

#if (BSP_TYPE == BSP_SMDK2443)
#elif (BSP_TYPE == BSP_SMDK2450)
                            				PowerUp();
								Lock();
#if 0 // TODO: Resume Input DMA after wake up?
								if(m_SavedInputDMARunning)
								{
									m_SavedInputDMARunning = FALSE;
									SetInterruptEvent(m_dwSysintrInput);
									//StartInputDMA();
								}
#endif
								if(m_SavedOutputDMARunning)
								{
									m_SavedOutputDMARunning = FALSE;
									SetInterruptEvent(m_dwSysintrOutput);
									//StartOutputDMA();
								}

								Unlock();
#endif


								InitCodec();
#if (BSP_TYPE == BSP_SMDK2443)
								bIdlePwrDown = FALSE;				// Then, Codec is on
#elif (BSP_TYPE == BSP_SMDK2450)
								bIdlePwrDown = TRUE;				// Then, Codec is on
#endif
							}

						    	dwErr = GetSystemPowerState(wPState, 20, &dFlag);
						    	RETAILMSG(1, (TEXT("[A_HW] IOControl : System Power State - %s \r\n"),wPState));

							//bPowerSet = TRUE;		// for logo test (unattended mode)
						}
						break;

					default:
						if (m_Dx != (_CEDEVICE_POWER_STATE)D4)
						{
#if (BSP_TYPE == BSP_SMDK2443)
							m_Dx = (_CEDEVICE_POWER_STATE)D4;
#elif (BSP_TYPE == BSP_SMDK2450)
							// Save last DMA state before Power Down
							m_SavedInputDMARunning = m_InputDMARunning;
							m_SavedOutputDMARunning = m_OutputDMARunning;

							m_Dx = (_CEDEVICE_POWER_STATE)D4;

							StopOutputDMA();
							StopInputDMA();

							Unlock();

						       PowerDown();
#endif


							dwErr = GetSystemPowerState(wPState, 20, &dFlag);
						    	RETAILMSG(1, (TEXT("[A_HW] IOControl : System Power State - %s \r\n"),wPState));
							AudioMute((DMA_CH_OUT | DMA_CH_MIC), TRUE);
							
							if(m_OutputDMARunning && !bClosing && (_tcscmp(wPState, _T("unattended")) != 0))
							{
								bDoingSuspend = TRUE;
								RETAILMSG(1, (TEXT("[A_HW] IOControl : going into Suspend state\r\n")));
							}

							bClosing = FALSE;
							//bPowerSet = FALSE;		// for logo test (unattended mode)
 //[david.modify] 2008-06-05 14:07
 //========================
DPSTR("+POWER_STATE_D4");
if(m_RTCodec) {							
	DPNOK(m_RTCodec);
	m_RTCodec->ChangeCodecPowerStatus(POWER_STATE_D4);
}
DPSTR("-POWER_STATE_D4");
 //========================							
						}
						break;
				}

				// return our state
				*(PCEDEVICE_POWER_STATE)pBufOut = m_Dx;

#if (BSP_TYPE == BSP_SMDK2443)
				Unlock();
#elif (BSP_TYPE == BSP_SMDK2450)
#endif

				*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
			}
			else
			{
				bRc = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
			}

			break;

		case IOCTL_POWER_GET:
			if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) )
			{
				bRc = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
				break;
			}

			*(PCEDEVICE_POWER_STATE)pBufOut = m_Dx;

			RETAILMSG(1, (TEXT("WAVEDEV: IOCTL_POWER_GET: D%u \r\n"), m_Dx));

			*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
			break;
            
        default:
            bRc = FALSE;
            dwErr = ERROR_INVALID_FUNCTION;
            DEBUGMSG (ZONE_FUNCTION, (TEXT(" Unsupported ioctl 0x%X\r\n"), dwCode));
            break;            
    }
    
    if ( !bRc ) {
        SetLastError(dwErr);
    }
    
    return(bRc);
}

BOOL HardwareContext::SafeWriteCodec(BYTE Offset, unsigned short int Data)
{
	return (BOOL)WriteI2SCodec(Offset,Data);
}

BOOL HardwareContext::SafeReadCodec(BYTE Offset, unsigned short int * Data)
{	
	return (BOOL)ReadI2SCodec(Offset,Data);
}

short int HardwareContext::MsgWriteCodec(DWORD dwParam1,DWORD dwParam2)
{
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "write %x %x \r\n" ),dwParam1,dwParam2)); 

	WriteI2SCodec((BYTE)dwParam1,(unsigned short int)dwParam2);
    return (MMSYSERR_NOERROR);
}

short int HardwareContext::MsgReadCodec(DWORD dwParam1,DWORD dwParam2)
{
    unsigned short int MyData=0; 

    DEBUGMSG(ZONE_VERBOSE, (TEXT( "read %x %x \r\n" ),dwParam1,MyData ));

	ReadI2SCodec((BYTE)dwParam1,&MyData);

    if (dwParam2 != (unsigned short int) NULL)
            * (unsigned short int *) dwParam2 =  MyData;

    return (MMSYSERR_NOERROR);
}

DWORD HardwareContext::RT_AudioMessage(UINT uMsg,DWORD dwParam1,DWORD dwParam2)
{	
	DWORD bRetVal=MMSYSERR_ERROR;	

	if(m_RTCodec)
	{
		bRetVal=m_RTCodec->ProcessAudioMessage(uMsg,dwParam1,dwParam2);		
	}
	
	return 	bRetVal;
}


void HardwareContext::OSTDelayMilliSecTime(DWORD MilliSec)
{
	DelayMS(MilliSec);	
}

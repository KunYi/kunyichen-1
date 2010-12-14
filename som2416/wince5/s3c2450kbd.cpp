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

#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>

#undef ZONE_INIT

#include <keybddbg.h>
#include <keybddr.h>
#include <keybdpdd.h>
#include <keybdist.h>
#include "s3c2450kbd.hpp"
#include <s3c2450.h>
 //[david.modify] 2008-05-16 11:58
 #include <args.h>
 #include <bsp_cfg.h>
 #include <image_cfg.h>
 #include <dbgmsg_david.h>
 #include <xllp_gpio_david.h>

// Pointer to device control registers
volatile S3C2450_IOPORT_REG *v_pIOPregs;
volatile S3C2450_SPI_REG *v_pSPIregs;

//[david.modify] 2008-05-16 12:08
volatile BSP_ARGS *g_pBspArgs;
//[david. end] 2008-05-16 12:08


DWORD g_dwSysIntr_Keybd = SYSINTR_UNDEFINED;

// Scan code consts
static const UINT8 scE0Extended	= 0xe0;
static const UINT8 scE1Extended	= 0xe1;
static const UINT8 scKeyUpMask	= 0x80;

UINT32
ScanCodeToVKeyEx(
        UINT32                  ScanCode,
        KEY_STATE_FLAGS KeyStateFlags,
        UINT32                  VKeyBuf[16],
        UINT32                  ScanCodeBuf[16],
        KEY_STATE_FLAGS KeyStateFlagsBuf[16]
        );

//	There is really only one physical keyboard supported by the system.
Ps2Keybd *v_pp2k;

extern void ReadRegDWORD( LPCWSTR szKeyName, LPCWSTR szValueName, LPDWORD pdwValue );


 //[david.modify] 2008-06-12 16:54
 //==========================
 #include <hotkey.h>
//static BOOL HotKeyHandler (DWORD dwHotKey, DWORD dwScanCode, BOOL bKeyUp);
//static BOOL HotKeyHandlerApp (DWORD dwHotKey, DWORD dwScanCode, BOOL bKeyUp);
static HANDLE g_hAudioVolChangeEvent = NULL;
//static HANDLE g_hAppEvent = NULL;


//#define dim(arr)    (sizeof (arr) / sizeof (arr[0]))
#if 0 //unused code, f.w.lin
typedef struct
{
    PFN_HOTKEY_HDLR pfHdlr;
    HANDLE          hEvent;
    LPCTSTR         szName;
} HOTKEY;

static HOTKEY g_HotKeys [MAX_HOTKEY_NUM];
void HotKeyInit()
{
	memset(g_HotKeys, 0, sizeof(g_HotKeys));
}

HANDLE HotKeyRegister (DWORD dwHotKey, PFN_HOTKEY_HDLR pfHdlr, LPTSTR szName)
{
    if (dwHotKey < MAX_HOTKEY_NUM)
    {
      	g_HotKeys[dwHotKey].szName = szName;    

        if (NULL == g_HotKeys [dwHotKey].hEvent)
        {
            	g_HotKeys [dwHotKey].hEvent = CreateEvent (
                    NULL, FALSE, FALSE, g_HotKeys[dwHotKey].szName);

		if (NULL==g_HotKeys [dwHotKey].hEvent)
		{
			DEBUGMSG(1, (TEXT("HotKeyRegister: create event fail, error code %d\r\n"), GetLastError()));
		}
        }

        if (NULL != g_HotKeys [dwHotKey].hEvent)
        {
            g_HotKeys [dwHotKey].pfHdlr = pfHdlr;
        }
    }

    return g_HotKeys [dwHotKey].hEvent;
}

// HotKeyProcess() function to call the hotkey handler registered in the g_HotKeys[] in sequence,
// if One handler returns TURE, means the key event is completely handled and not necessary to deliver to other handlers
// otherwise return FALSE means the handler not treat the key or it still necessary to be treated in other handler.
// Retrun value:
// FALSE: the key have not finished handling, continue to be transfer.
// TURE: the key have been finished handling.

BOOL HotKeyProcess (DWORD dwScanCode, BOOL bKeyUp)
{
    BOOL    bRet = FALSE;
    DWORD   i;

    for (i = 0; i < MAX_HOTKEY_NUM; i++)
    {
        if (g_HotKeys [i].pfHdlr != NULL)
        {
            if ((*g_HotKeys [i].pfHdlr)(i, dwScanCode, bKeyUp))
            {
                bRet = TRUE;
                break;
            }
        }
    }

    return bRet;
}
#endif

#if 0
// from bsp_cfg.h
// KEYBD 有三个键,对应如下;
//[david.modify] 2008-05-16 10:19
//GPG2	KEY6(up)		音量加EINT10 - down
//GPG1	KEY4 (OK)	确认键EINT9
//GPG0	KEY3 (down)	音量减EINT8 - up
//代表SCANCODE, 在ScanCodeToVKeyTable中映射成VKEY
#define VK_RETURN_SCANCODE 0x5a
#define VK_DOWN_SCANCODE 0x6a
#define VK_UP_SCANCODE 0x6c

#define EINT8_KEY VK_UP_SCANCODE //8			
#define EINT9_KEY VK_RETURN_SCANCODE //9
#define EINT10_KEY VK_DOWN_SCANCODE  //10

#endif

//#define SCAN_CODE_VOLUP     0x00
//#define SCAN_CODE_VOLDOWN   0x01
#define SCAN_CODE_VOLUP     EINT8_KEY
#define SCAN_CODE_VOLDOWN   EINT10_KEY

#define SCAN_CODE_APP2      0x02

#define SCAN_CODE_UP       0x08
#define SCAN_CODE_LEFT       0x09
#define SCAN_CODE_RIGHT     0x05
#define SCAN_CODE_DOWN      0x03
#define SCAN_CODE_APP1    0x04
#define SCAN_CODE_APP3    0x06
#define SCAN_CODE_CANCEL    0x07
#define SCAN_CODE_RETURN   0x0A
#define SCAN_CODE_NONE      0xFF

#define KEYPAD_NUM          3
#if 0 //unused code, f.w.lin
static BOOL HotKeyHandler (DWORD dwHotKey, DWORD dwScanCode, BOOL bKeyUp)
{
	BOOL bRet = FALSE;

	if (dwHotKey == HOTKEY_VOL_CHANGE)
	{
		bRet = TRUE;
		switch (dwScanCode)
		{
		case SCAN_CODE_VOLUP:
			if (bKeyUp)
			{
				SetEventData (g_hAudioVolChangeEvent, 1);
				SetEvent (g_hAudioVolChangeEvent);
			}
			break;
		case SCAN_CODE_VOLDOWN:
			if (bKeyUp)
			{
				SetEventData (g_hAudioVolChangeEvent, 0);
				SetEvent (g_hAudioVolChangeEvent);
			}
			break;
		default:
			bRet = FALSE;
			break;
		}
	}

    return bRet;
}

static BOOL HotKeyHandlerApp (DWORD dwHotKey, DWORD dwScanCode, BOOL bKeyUp)
{
	BOOL bRet = FALSE;

	if (dwHotKey == HOTKEY_ATLAS_APP)
	{
		bRet = TRUE;
		switch (dwScanCode)
		{
		case SCAN_CODE_APP1:
			if (bKeyUp)
			{	
				SetEventData (g_hAppEvent, 1);
				SetEvent (g_hAppEvent);			
			}
			break;
		case SCAN_CODE_APP2:
			if (bKeyUp)
			{	
				SetEventData (g_hAppEvent, 2);
				SetEvent (g_hAppEvent);
			}
			break;
		case SCAN_CODE_APP3:
			if (bKeyUp)
			{		
				SetEventData (g_hAppEvent, 3);
				SetEvent (g_hAppEvent);			
			}
			break;
		default:
			bRet = FALSE;
			break;
		}
	}

    return bRet;
}
#endif
//==========================

















void WINAPI KeybdPdd_PowerHandler(BOOL bOff)
{
	if (!bOff) { 
	   v_pp2k->KeybdPowerOn();
	}
	else {
	   v_pp2k->KeybdPowerOff();
	}
	return;
}

#define ONEBIT    0x1


int	putcToKBCTL(UCHAR c)
{
	UINT	i;

	v_pSPIregs->SPPIN &= ~(1<<1);
  	v_pIOPregs->GPLDAT &= ~(ONEBIT << 14);       //Set _SS signal to low (Slave Select)

	while((v_pSPIregs->SPSTA & 1)==0);	// wait while busy

	v_pSPIregs->SPTDAT = c;	                // write left justified data

	while((v_pSPIregs->SPSTA & 1)==0);	// wait while busy
   	
   	v_pIOPregs->GPLDAT |= (ONEBIT << 14);        //Set _SS signal to high (Slave Select)
   	v_pSPIregs->SPPIN |= (1<<1);

	i = v_pSPIregs->SPRDATB;

	return(i);
}


void getsFromKBCTL(UINT8 *m, int cnt) {
	int	i, j;
	volatile tmp = 1;

	for(j = 0; j < 3; j++)
		tmp += tmp;
	for(j = 0; j < 250 * 30; j++)
		tmp += tmp;

	for(i = 0; i < cnt; i++) {
		m[i] = putcToKBCTL(0xFF);

		for(j = 0; j < 400; j++)
			tmp+= tmp;
	}
}

void putsToKBCTL(UINT8 *m,  int cnt)
{
	int	i, j, x;
	volatile tmp = 1;
	
	for(j = 0; j < 3; j++)
		x = j;
	for(j = 0; j < 3; j++)
		tmp += tmp;
	for(j = 0; j < 250 * 30; j++)
		tmp += tmp;

	for(i = 0; i < cnt; i++) {

		j = putcToKBCTL(m[i]);

		for(j = 0; j < 400; j++)
			tmp+= tmp;
		for(j = 0; j < 400; j++)
			x = j;
    }
}

char lrc(UINT8 *buffer, int count)
{
    char lrc;
    int n;

    lrc = buffer[0] ^ buffer[1];

    for (n = 2; n < count; n++)
    {
        lrc ^= buffer[n];
    }

    if (lrc & 0x80)
        lrc ^= 0xC0;

    return lrc;
}

int USAR_WriteRegister(int reg, int data)
{
    UINT8 cmd_buffer[4];

    cmd_buffer[0] = 0x1b; //USAR_PH_WR;
    cmd_buffer[1] = (unsigned char)reg;
    cmd_buffer[2] = (unsigned char)data;

    cmd_buffer[3] = lrc((UINT8 *)cmd_buffer,3);
    putsToKBCTL((UINT8 *)cmd_buffer,4);

    return TRUE;
}


BOOL
KeybdDriverInitializeAddresses(
	void
	)
{
	bool RetValue = TRUE;
	DWORD dwIOBase;
	DWORD dwSSPBase;

	DEBUGMSG(1,(TEXT("++KeybdDriverInitializeAddresses\r\n")));

	ReadRegDWORD(TEXT("HARDWARE\\DEVICEMAP\\KEYBD"), _T("IOBase"), &dwIOBase );
	if(dwIOBase == 0) {
		DEBUGMSG(1, (TEXT("Can't fount registry entry : HARDWARE\\DEVICEMAP\\KEYBD\\IOBase\r\n")));
		goto error_return;
	}
	DEBUGMSG(1, (TEXT("HARDWARE\\DEVICEMAP\\KEYBD\\IOBase:%x\r\n"), dwIOBase));

	ReadRegDWORD(TEXT("HARDWARE\\DEVICEMAP\\KEYBD"), _T("SSPBase"), &dwSSPBase );
	if(dwSSPBase == 0) {
		DEBUGMSG(1, (TEXT("Can't fount registry entry : HARDWARE\\DEVICEMAP\\KEYBD\\SSPBase\r\n")));
		goto error_return;
	}
	DEBUGMSG(1, (TEXT("HARDWARE\\DEVICEMAP\\KEYBD\\SSPBase:%x\r\n"), dwSSPBase));

	v_pIOPregs = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if(v_pIOPregs == NULL) {
		DEBUGMSG(1,(TEXT("[KBD] v_pIOPregs : VirtualAlloc failed!\r\n")));
		goto error_return;
	}
	else {
		if(!VirtualCopy((PVOID)v_pIOPregs, (PVOID)(dwIOBase), sizeof(S3C2450_IOPORT_REG), PAGE_READWRITE|PAGE_NOCACHE )) 
		{
			DEBUGMSG(1,(TEXT("[KBD] v_pIOPregs : VirtualCopy failed!\r\n")));
			goto error_return;
		}
	}
	DEBUGMSG(1, (TEXT("[KBD] v_pIOPregs mapped at %x\r\n"), v_pIOPregs));
	
	v_pSPIregs = (volatile S3C2450_SPI_REG *)VirtualAlloc(0, sizeof(S3C2450_SPI_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (v_pSPIregs == NULL) {
		DEBUGMSG(1, (TEXT("[KBD] v_pSPIregs : VirtualAlloc failed!\r\n")));
		goto error_return;
	}
	else {
		if (!VirtualCopy((PVOID)v_pSPIregs, (PVOID)(dwSSPBase), sizeof(S3C2450_SPI_REG), PAGE_READWRITE | PAGE_NOCACHE)) 
		{
		    	DEBUGMSG(1, (TEXT("[KBD] v_pSPIregs : VirtualCopy failed!\r\n")));
				goto error_return;
		}
	}

 //[david.modify] 2008-05-16 12:07
 //==================================
 	if (g_pBspArgs == NULL)
	{
		PHYSICAL_ADDRESS BspArgsPA = { IMAGE_SHARE_ARGS_PA_START, 0 };
		g_pBspArgs = (BSP_ARGS *) MmMapIoSpace(BspArgsPA, sizeof(BSP_ARGS), FALSE);
	}

 //[david.modify] 2008-06-12 16:53
 //添加音量控制; 按音量键, 产生g_hAudioVolChangeEvent事件;
 // AUDIO驱动收到g_hAudioVolChangeEvent事件,产生MSG
 //=========================
//	g_hAudioVolChangeEvent = HotKeyRegister (HOTKEY_VOL_CHANGE, HotKeyHandler, ATLAS_AUDIO_VOL_EVENT);
	g_hAudioVolChangeEvent = CreateEvent(NULL, FALSE, FALSE, ATLAS_AUDIO_VOL_EVENT);
	if (g_hAudioVolChangeEvent == NULL)
	{
		RetValue=FALSE;
	}
	DPNOK(g_hAudioVolChangeEvent);

	
	
 //==================================

	DEBUGMSG(1, (TEXT("[KBD] v_pSPIregs mapped at %x\r\n"), v_pSPIregs));
	
	DEBUGMSG(1,(TEXT("--KeybdDriverInitializeAddresses\r\n")));
	
	return TRUE;

error_return:
	if ( v_pIOPregs )
		VirtualFree((PVOID)v_pIOPregs, 0, MEM_RELEASE);
	if ( v_pSPIregs )
		VirtualFree((PVOID)v_pSPIregs, 0, MEM_RELEASE);
	v_pIOPregs = 0;
	v_pSPIregs = 0;

	DEBUGMSG(1,(TEXT("--KeybdDriverInitializeAddresses[FAILED!!!]\r\n")));

	return FALSE;
}

static UINT KeybdPdd_GetEventEx2(UINT uiPddId, UINT32 rguiScanCode[16], BOOL rgfKeyUp[16])
{
    SETFNAME(_T("KeybdPdd_GetEventEx2"));

    UINT32   scInProgress = 0;
    static UINT32   scPrevious;

    BOOL            fKeyUp;
    UINT8           ui8ScanCode;
    UINT            cEvents = 0;
    stGPIOInfo stGPIOInfo; //[david.modify] 2008-05-17 12:03

    DEBUGCHK(rguiScanCode != NULL);
    DEBUGCHK(rgfKeyUp != NULL);

 //[david.modify] 2008-05-16 12:02
 //=====================================
	//getsFromKBCTL(&ui8ScanCode, 1);
	ui8ScanCode = g_pBspArgs->uninit_misc.dwButtonType;
	RETAILMSG(1,(TEXT("KEY:0x%X\n"),ui8ScanCode));
	DPNOK(ui8ScanCode);	
 //=====================================	

#if 0
    DEBUGMSG(ZONE_SCANCODES, 
        (_T("%s: scan code 0x%08x, code in progress 0x%08x, previous 0x%08x\r\n"),
        pszFname, ui8ScanCode, scInProgress, scPrevious));
#endif


//[david.modify] 2008-05-17 10:40
//==========================
//按键有上升沿和下降沿两次中断
#if 0
#define VK_RETURN_SCANCODE 0x5a
#define VK_DOWN_SCANCODE 0x6a
#define VK_UP_SCANCODE 0x6c

#define EINT8_KEY VK_DOWN_SCANCODE //8			
#define EINT9_KEY VK_RETURN_SCANCODE //9
#define EINT10_KEY VK_UP_SCANCODE //10

// KEYBD 有三个键,对应如下;
//[david.modify] 2008-05-16 10:19
//GPG2	KEY6(up)		音量加EINT10 - down
//GPG1	KEY4 (OK)	确认键EINT9
//GPG0	KEY3 (down)	音量减EINT8 - up
//代表SCANCODE, 在ScanCodeToVKeyTable中映射成VKEY
#define VK_RETURN_SCANCODE 0x5a
#define VK_DOWN_SCANCODE 0x6a
#define VK_UP_SCANCODE 0x6c

#define EINT8_KEY VK_UP_SCANCODE //8			
#define EINT9_KEY VK_RETURN_SCANCODE //9
#define EINT10_KEY VK_DOWN_SCANCODE  //10
#endif

#define SCAN_CODE_VOLUP EINT8_KEY
#define SCAN_CODE_VOLDOWN EINT10_KEY
	DPNOK(ui8ScanCode);
	if(ui8ScanCode==SCAN_CODE_VOLUP) {
		stGPIOInfo.u32PinNo = GPG0;
		GetGPIOInfo(&stGPIOInfo, (void*)v_pIOPregs);	
		DPNOK(stGPIOInfo.u32Stat);
		if(stGPIOInfo.u32Stat) {	//high
			ui8ScanCode|=scKeyUpMask;

				SetEventData (g_hAudioVolChangeEvent, 1);
				SetEvent (g_hAudioVolChangeEvent);			
			
		}else{					//low

		}
	}

	else if(ui8ScanCode==EINT9_KEY) {
		stGPIOInfo.u32PinNo = GPG1;
		GetGPIOInfo(&stGPIOInfo, (void*)v_pIOPregs);	
		DPNOK(stGPIOInfo.u32Stat);		
		if(stGPIOInfo.u32Stat) {	//high
			ui8ScanCode|=scKeyUpMask;
		}else{					//low

		}
	}

	else if(ui8ScanCode==SCAN_CODE_VOLDOWN) {
		stGPIOInfo.u32PinNo = GPG2;
		DPNOK(stGPIOInfo.u32Stat);		
		GetGPIOInfo(&stGPIOInfo, (void*)v_pIOPregs);	
		if(stGPIOInfo.u32Stat) {	//high
			ui8ScanCode|=scKeyUpMask;

				SetEventData (g_hAudioVolChangeEvent, 0);
				SetEvent (g_hAudioVolChangeEvent);
				
		}else{					//low

		}
	}
	

 
//==========================		

    scInProgress = ui8ScanCode;
    if (scInProgress == scPrevious) {
        //	mdd handles auto-repeat so ignore auto-repeats from keybd
    } else {
        // Not a repeated key.  This is the real thing.
        scPrevious = scInProgress;
        
        if (ui8ScanCode & scKeyUpMask) {
            fKeyUp = TRUE;
            scInProgress &= ~scKeyUpMask;
        } else {
            fKeyUp = FALSE;
        }
        
        rguiScanCode[cEvents] = scInProgress;
		rgfKeyUp[cEvents] = fKeyUp;
        ++cEvents;
    }

    return cEvents;
}


void WINAPI KeybdPdd_ToggleKeyNotification(KEY_STATE_FLAGS	KeyStateFlags)
{
	unsigned int	fLights;

	DEBUGMSG(1, (TEXT("KeybdPdd_ToggleKeyNotification\r\n")));
	fLights = 0;
	if (KeyStateFlags & KeyShiftCapitalFlag) {
		fLights |= 0x04;
		}
	if (KeyStateFlags & KeyShiftNumLockFlag) {
		fLights |= 0x2;
		}
	/*
	Keyboard lights is disabled once driver is installed because the
	PS2 controller sends back a response which goes to the IST and corrupts
	the interface.  When we figure out how to disable the PS2 response we
	can re-enable the lights routine below
	*/

	return;
}


BOOL Ps2Keybd::IsrThreadProc()
{
	DWORD dwPriority;
    //DWORD dwIrq_Keybd = 0;

	// look for our priority in the registry -- this routine sets it to zero if
	// it can't find it.
	ReadRegDWORD( TEXT("HARDWARE\\DEVICEMAP\\KEYBD"), _T("Priority256"), &dwPriority );
	if(dwPriority == 0) {
		dwPriority = 240;	// default value is 145
	}

    DEBUGMSG(1, (TEXT("IsrThreadProc:\r\n")));
    m_hevInterrupt = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (m_hevInterrupt == NULL) {
        DEBUGMSG(1, (TEXT("IsrThreadProc: InterruptInitialize\r\n")));
		goto leave;
	}

	//ReadRegDWORD( TEXT("HARDWARE\\DEVICEMAP\\KEYBD"), _T("Irq"), &dwIrq_Keybd );

 //[david.modify] 2008-05-16 09:50
 //======================================
/*
[HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\KEYBD]
    "DriverName"="kbdmouse.dll"
    "Irq"=dword:2
    "IOBase"=dword:B2100000
    "SSPBase"=dword:B1D00000
*/
//===============================================
	//DPNOK(dwIrq_Keybd);
    // Call the OAL to translate the IRQ into a SysIntr value.
    // EINT1
#if 0
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq_Keybd, sizeof(DWORD), &g_dwSysIntr_Keybd, sizeof(DWORD), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to obtain sysintr value for keyboard interrupt.\r\n")));
        g_dwSysIntr_Keybd = SYSINTR_UNDEFINED;
        goto leave;
    }
#endif

 //[david.modify] 2008-05-16 11:58
 // 强制指定KEYPAD中断INTR
	g_dwSysIntr_Keybd = SYSINTR_KEYPAD;
	if (!InterruptInitialize(g_dwSysIntr_Keybd,m_hevInterrupt,NULL,0)) {
		DEBUGMSG(1, (TEXT("IsrThreadProc: KeybdInterruptEnable\r\n")));
		goto leave;
	}

	// update the IST priority
	CeSetThreadPriority(GetCurrentThread(), (int)dwPriority);

	extern UINT v_uiPddId;
	extern PFN_KEYBD_EVENT v_pfnKeybdEvent;

	KEYBD_IST keybdIst;
	keybdIst.hevInterrupt = m_hevInterrupt;
	keybdIst.dwSysIntr_Keybd = g_dwSysIntr_Keybd;
	keybdIst.uiPddId = v_uiPddId;
	keybdIst.pfnGetKeybdEvent = KeybdPdd_GetEventEx2;
	keybdIst.pfnKeybdEvent = v_pfnKeybdEvent;
		
	KeybdIstLoop(&keybdIst);

leave:
    return 0;
}

DWORD Ps2KeybdIsrThread(Ps2Keybd *pp2k)
{
	DEBUGMSG(1,(TEXT("Ps2KeybdIsrThread:\r\n")));
	pp2k->IsrThreadProc();
	return 0;
}

BOOL Ps2Keybd::IsrThreadStart()
{
	HANDLE	hthrd;

	DEBUGMSG(1,(TEXT("IsrThreadStart:\r\n")));
	hthrd = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Ps2KeybdIsrThread,this,0,NULL);
	//	Since we don't need the handle, close it now.
	CloseHandle(hthrd);
	return TRUE;
}

BOOL Ps2Keybd::Initialize()
{
	DEBUGMSG(1,(TEXT("Ps2Keybd::Initialize\r\n")));

	DEBUGMSG(1,(TEXT("Ps2Keybd::Initialize Done\r\n")));
	return TRUE;
}


BOOL Ps2Keybd::KeybdPowerOn()
{
//    UINT8 msg[5];
//    int t;
    char dummy = (char)0xff;

	DEBUGMSG(1,(TEXT("++Ps2Keybd::KeybdPowerOn\r\n")));

#if 0
	// Setup KBDINT as output
    v_pIOPregs->GPFCON &= ~(0x3 << 4); 	// Clear GPF2
    v_pIOPregs->GPFCON |= (0x2 << 4);  	// Set GPF2 to EINT2 for Keyboard interrupt
    v_pIOPregs->GPFUDP &= ~(0x3 << 4);
    v_pIOPregs->GPFUDP |= (0x1 << 4);

    v_pIOPregs->EXTINT0 = READEXTINT0(v_pIOPregs->EXTINT0) & ~(0x7 << 8);    // Clear EINT2
    v_pIOPregs->EXTINT0 = READEXTINT0(v_pIOPregs->EXTINT0) | (0xA << 8);     // fallig edge triggered for EINT2

	// setup SPI interface
	// GPL12 : SPIMISO (KBDSPIMISO)
	// GPL11 : SPIMOSI (KBDSPIMOSI)
	// GPL10 : SPICLK  (KBDSPICLK)
    v_pIOPregs->GPLCON &= ~((0x3 << 24) | (0x3 << 22) | (0x3 << 20));   // Clear GPG12,11,10
    v_pIOPregs->GPLCON |= ((0x2 << 24) | (0x2 << 22) | (0x2 << 20));    
    v_pIOPregs->GPLUDP &= ~((0x3 << 24) | (0x3 << 22) | (0x3 << 20));
    v_pIOPregs->GPLUDP |= ((0x1 << 24) | (0x1 << 22) | (0x1 << 20));       
     
	// setup _SS signal(nSS_KBD)
    v_pIOPregs->GPLCON &= ~(0x3 << 28);         // Clear GPL14
    v_pIOPregs->GPLCON |= (1 << 28);        // Set Port GPL14 to output for nSS signal
    v_pIOPregs->GPLUDP &= ~((0x3 << 28));
    v_pIOPregs->GPLUDP |= ((0x1 << 28));    
    
	// setup _PWR_OK signal (KEYBOARD)
    v_pIOPregs->GPFCON &= ~(0x3 << 6);         // Clear GPF3 
    v_pIOPregs->GPFCON |= (1 << 6);       // Set Port GPF3 to output for _PWR_OK signal
    v_pIOPregs->GPFUDP &= ~((0x3 << 6));
    v_pIOPregs->GPFUDP |= ((0x1 << 6));    

    v_pIOPregs->GPFDAT &=~(1 << 3);        // set _PWR_OK to 0
    
	// Setup IO port for SPI interface & Keyboard

	// Setup SPI registers
    // Interrupt mode, prescaler enable, master mode, active high clock, format B, normal mode
    v_pSPIregs->SPCON &=~(0x3f<<0);  
    v_pSPIregs->SPCON |= (1<<5)|(1<<4)|(1<<3)|(0x0<<2)|(1<<1);
    v_pSPIregs->SPPIN |= (1<<0);//Feedback Clock Disable, Master Out Keep
    
	// Developer MUST change the value of prescaler properly whenever value of PCLK is changed.
    v_pSPIregs->SPPRE = 255;// 99.121K = 203M/4/2/(255+1) PCLK=50.75Mhz FCLK=203Mhz SPICLK=99.121Khz
        
    
         
    for(t=0;t<20000; t++); // delay
	    msg[0] = (char)0x1b; msg[1] = (char)0xa0; msg[2] = (char)0x7b; msg[3] = (char)0; // Initialize USAR
    	for(t=0; t < 10; t++) {
    	dummy = putcToKBCTL(0xff);
    }
    
    for(t=0; t<10; t++) { // wait for a while
        putsToKBCTL(msg,3);
        for(t=0;t<20000; t++);
    }
    t = 100;
    while(t--) {
        volatile unsigned int read_atn = 0;
	 v_pIOPregs->GPFCON &= ~(3<<4);
	 read_atn = v_pIOPregs->GPFDAT & 0x04;
	 v_pIOPregs->GPFCON |= (2<< 4);

	 if(read_atn == 0) { // Read _ATN
            break;
        }
    }	//check _ATN
    if(t != 0) {
        getsFromKBCTL(msg,3);
    }    
    t=100000;
    while(t--); // delay
	msg[0] = (char)0x1b; msg[1] = (char)0xa1; msg[2] = (char)0x7a; msg[3] = (char)0; //Initialization complete
	putsToKBCTL(msg,3);
#endif 
	DEBUGMSG(1,(TEXT("--Ps2Keybd::KeybdPowerOn\r\n")));
	return(TRUE);
}

BOOL Ps2Keybd::KeybdPowerOff()
{
	DEBUGMSG(1,(TEXT("++Ps2Keybd::KeybdPowerOff\r\n")));

	DEBUGMSG(1,(TEXT("--Ps2Keybd::KeybdPowerOff\r\n")));
	return(TRUE);
}

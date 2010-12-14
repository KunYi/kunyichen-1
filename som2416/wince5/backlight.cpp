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
/*

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

*/

#include <windows.h>
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
/*

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

*/

#include <windows.h>
#include <pm.h>

#include "S3C2450_ioport.h"	
//#include "S3C2450_lcd.h"	
#include "S3C2450_base_regs.h"
#include "S3C2450_pwm.h"
#include <ceddk.h>
#include "Args.h"
#include "Image_cfg.h"

 //[david.modify] 2008-05-19 15:16
#include <xllp_gpio_david.h>
#include <dbgmsg_david.h>
#include <sdk_api_david.h>

 //[david.modify] 2008-07-14 10:54


#define BL_DBG 1/*For Backlight Debugging*/
#define BL_DBG1 1

CEDEVICE_POWER_STATE bklPS;

volatile S3C2450_IOPORT_REG *v_pGPIOReg;
volatile S3C2450_PWM_REG * v_pPWMregs;

static int int_BL =0; 

extern volatile BSP_ARGS *g_pBspArgs = NULL;
extern BOOL WINAPI BL_Init();


//BL003 begin
#if 0 //unused code, f.w.lin
const TCHAR szevtBacklightChange[] = TEXT("BacklightSettingPanelUpdateEvent");
const TCHAR szevtPowerChanged[] = TEXT("PowerChangedEvent");
const TCHAR szevtUserInput[] = TEXT("UserInputEvent");
const TCHAR	szevtBrightChange[] = TEXT("BrightChangeEvent");
#endif
const TCHAR szregRootKey[] = TEXT("ControlPanel\\Backlight");
const TCHAR szregBatteryTimeout[] = TEXT("BatteryTimeout");
const TCHAR szregACTimeout[] = TEXT("ACTimeout");
#if 0 //unused code, f.w.lin
const TCHAR szregBatteryAuto[] = TEXT("BacklightOnTap");
const TCHAR szregACAuto[] = TEXT("ACBacklightOnTap");
const TCHAR	szregBatteryLevel[] = TEXT("BacklightLuminanceLevel");

//Nicho suggest that both the backlight level are the same whether is charging or not.
//Nicho begin 2006-12-09
//const TCHAR	szregACLevel[] = TEXT("ACBacklightLuminanceLevel");
const TCHAR	szregACLevel[] = TEXT("BacklightLuminanceLevel");
//Nicho end
const TCHAR	szregMaxLevel[] = TEXT("BacklightMaxLuminanceLevel");
const TCHAR	szregPowerStatus[] = TEXT("BacklightPowerStatus");
const TCHAR	szregBatteryEnable[] = _T("BatteryEnable");
const TCHAR	szregACPowerEnable[] = _T("ACPowerEnable");
//BL003 end
#endif


 //[david.modify] 2008-07-14 11:42
#define LIGHTNESS_ITEM _T("Lightness")
 //[david.modify] 2008-08-20 09:04
 // base on ATLASIII
 //=============================================================
#define HW_PWM_FREQ 100000    //100khz HW recommedned setting
#define BKL_TOTAL_LEVELS_DEFAULT 10//0x5//10
#define BACKLIGHT_DEFAULT_LEVEL  8//0x3//5
#define BKL_REG_KEY    TEXT("ControlPanel\\BackLight")
#define BKL_REG_CURR_LEVEL    TEXT("BacklightCurrentLevel")
#define BKL_REG_TOTAL_LEVELS    TEXT("BacklightTotalLevels")



DWORD gBklLevelCurrent2UpPrecentTable[11] = {    1,        //off    //0
											     15,       //Level-1: 8mA  
			                                                             20,     //Level-2: 15mA   
			                                                             40,
			                                                             60,      //Level-3: 31mA 
			                                                             70,
		                                                                    80,      //Level-4:56mA  
		                                                                    90,
			                                                             110,      //Level-5:98mA   
			                                                             130,
			                                                             140,
			                                                             };
DWORD  g_dwBklTotalLevels;
DWORD g_dwCurrLevel=BACKLIGHT_DEFAULT_LEVEL;//BKL_MAX_LEVEL;
 //[david.modify] 2008-09-09 10:20

#define BKLIGHT_DRIVER_REGISTRY TEXT("\\Drivers\\BuiltIn\\BKL")
#define DEF_u32LcdPwrOnDelay 300
typedef struct
{
	UINT32 u32Debug;	// 调试使用
	UINT32 u32LcdPwrOnDelay;	// 调试使用
}stBklightParam;


stBklightParam g_stBklightParam;


void SetPWMValue1(int LightLevel, BOOL bSave2Reg);
void SetCurRegistry_pwm();
DWORD  BspBacklightSetLevel(DWORD nLevel, BOOL bSave2Reg);
DWORD BspBacklightDefaultLevel();
BOOL BacklightInitialize();
void BL_On(BOOL bOn);





//因为AUDIO驱动影响到了背光
//发现拿掉AUDIO驱动或者将AUDIO驱动的睡眠拿掉就OK
HANDLE g_tmpThread;  //[david.modify] 2008-08-19 14:30
HANDLE g_BackLightD0Event;
static void Backlight_IST();

DWORD 
BKL_Init(DWORD dwContext)
{
	bklPS = D0;
	/*Perform all one-time initialization of the backlight*/
	if (!BacklightInitialize())
	{
		RETAILMSG(1, (TEXT("[BL_BKL_Init] BKL_Init:couldn't initialize backlight hardware \r\n")));
		return 0;
	}
	RETAILMSG(1,(TEXT("[BL_BKL_Init] Backlight Driver is initialized. \r\n")));

 //[david.modify] 2008-08-19 02:10
 //创建一个线程，用来解决第1次发送给AP事件，AP收不着的问题
 //======================
	g_tmpThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) Backlight_IST, NULL, 0, NULL);

 #define Backlight_IST_PRI 101
	CeSetThreadPriority(g_tmpThread, Backlight_IST_PRI);
	g_BackLightD0Event = CreateEvent(NULL, FALSE, FALSE, NULL);

 //======================
	
  	return TRUE;
}


//[david.modify] 2008-05-30 16:05
#define TCON_TIMER_SHIFT 12

 //[david.modify] 2008-07-11 15:01
 //背光新板
 //============================================
	stGPIOInfo g_stGPIOInfo={
		BACKLIGHT_PWM,  1, ALT_FUNC_02, PULL_UPDOWN_DISABLE
	};

 //[david.modify] 2008-08-18 22:34
 // lcd电源控制
	stGPIOInfo g_stGPIOInfo_LcdPwr={
		LCD_PWREN,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE
	};
 //============================================

int PWM_Init( int LightLevel ) //add the parm,f.w.lin
{


 //[david.modify] 2008-06-04 16:01
 // 背光原来使用GPB3，但与TOUCH冲突
 //改成GPB2,但前期装机的机器硬件没改，还是用GPB3
 //==================================
 #if 1
//	SetGPIOInfo(&stGPIOInfo_old,  v_p2450IOP);
    if( LightLevel )
    {
        g_stGPIOInfo.u32Stat = 1;
    }
    else
	{
        g_stGPIOInfo.u32Stat = 0;
	}
	SetGPIOInfo(&g_stGPIOInfo,  (void*)v_pGPIOReg);
 	GetGPIOInfo(&g_stGPIOInfo, (void*)v_pGPIOReg);
	EPRINT(L"BACKLIGHT_PWM=%d: [%d %d %d] \r\n",
		 g_stGPIOInfo.u32PinNo, 
		g_stGPIOInfo.u32Stat, g_stGPIOInfo.u32AltFunc, g_stGPIOInfo.u32PullUpdown);	

 #endif
 //===================================
	
	

	/* Backlight PWM Setting*/
	//v_pPWMregs->TCFG0  |= 24 << 8;	/*Timer 0,1 prescaler value*/
	v_pPWMregs->TCFG1  &= ~(0xf << 8);/* Timer3's Divider Value*/
	v_pPWMregs->TCFG1  |=  (3   << 8);/* 1/16	*/
	DPNOK(v_pPWMregs->TCFG1);
	v_pPWMregs->TCNTB2=150;/*Origin Value*/
	v_pPWMregs->TCMPB2= MAX_PWM_VALUE;

	v_pPWMregs->TCON &= ~(0xf<<TCON_TIMER_SHIFT);
	v_pPWMregs->TCON |=  (0x2<<TCON_TIMER_SHIFT);
	v_pPWMregs->TCON |=  (0x9<<TCON_TIMER_SHIFT);
	v_pPWMregs->TCON |=  (0x9<<TCON_TIMER_SHIFT);
	v_pPWMregs->TCON &=  ~(0x2<<TCON_TIMER_SHIFT);	/*1.69KHz*/

	return 1;

}




int GetPWMValue1()
{
//	volatile S3C2450_PWM_REG * v_pPWMregs=NULL;	
	if(NULL==v_pPWMregs) {
//		v_pPWMregs = (S3C2450_PWM_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_PWM, FALSE);
		DPN(v_pPWMregs);
		return -1;
	}
	DPNOK(v_pPWMregs->TCMPB2);
	return v_pPWMregs->TCMPB2 ;
}
	
	
void SetPWMValue1(int LightLevel, BOOL bSave2Reg)
{
//	volatile S3C2450_PWM_REG * v_pPWMregs=NULL;	
	if(NULL==v_pPWMregs) {
//		v_pPWMregs = (S3C2450_PWM_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_PWM, FALSE);
		DPN(v_pPWMregs);
		return ;
	}
//    if( LightLevel ) //f.w.lin
    PWM_Init(LightLevel);  //move to here, the LightLevel will be changed.

 //[david.modify] 2008-08-18 20:58
 // 0- 表示关闭PWM
	if(0!=LightLevel) {
		if((LightLevel<MIN_PWM_VALUE)||(LightLevel>MAX_PWM_VALUE)){
			DPN(LightLevel);
			return;
		}
	}else{
		LightLevel = 1;
	}


 //==================================
 #if 0
//强制设制背光IO为功能脚
	SetGPIOInfo(&g_stGPIOInfo,  (void*)v_pGPIOReg);
 	GetGPIOInfo(&g_stGPIOInfo, (void*)v_pGPIOReg);
	EPRINT(L"BACKLIGHT_PWM=%d: [%d %d %d] \r\n",
		 g_stGPIOInfo.u32PinNo, 
		g_stGPIOInfo.u32Stat, g_stGPIOInfo.u32AltFunc, g_stGPIOInfo.u32PullUpdown);	
	DPNOK(LightLevel);
#else
	//PWM_Init(LightLevel);  //move up, the LightLevel will be changed.
 #endif
 //===================================

	v_pPWMregs->TCMPB2 = LightLevel;
	/*Clear manual_update bit*/
	v_pPWMregs->TCON &=  ~(0x2<<TCON_TIMER_SHIFT);
	/*Off the inverter*/
	v_pPWMregs->TCON &=  ~(0x4<<TCON_TIMER_SHIFT);
	/*Set the AutoReload*/
	v_pPWMregs->TCON  |= (0x8 <<TCON_TIMER_SHIFT);
	/*Set the start bit*/
	v_pPWMregs->TCON |= (0x1 <<TCON_TIMER_SHIFT);

 //[david.modify] 2008-07-14 11:45
 	if(bSave2Reg) {
		SetRegistryDWORD(HKEY_CURRENT_USER, TEXT("ControlPanel\\Backlight"), LIGHTNESS_ITEM, LightLevel);
 	}
	
}


void SetCurRegistry_pwm()
{
	//DWORD u32Light = OS_PWM_VALUE1; //unused ? f.w.lin
	DWORD u32Level = BACKLIGHT_DEFAULT_LEVEL;	
	DWORD u32TotalLevels = BKL_TOTAL_LEVELS_DEFAULT;	
	//if (GetRegistryDWORD(HKEY_CURRENT_USER, TEXT("ControlPanel\\Backlight"), LIGHTNESS_ITEM, &u32Light))
	//{    	
	//}		
	//DPNOK(u32Light);
	if (GetRegistryDWORD(HKEY_CURRENT_USER, TEXT("ControlPanel\\Backlight"), BKL_REG_CURR_LEVEL, &u32Level))
	{    	
	}		
	DPNOK(u32Level);	
	if (GetRegistryDWORD(HKEY_CURRENT_USER, TEXT("ControlPanel\\Backlight"), BKL_REG_TOTAL_LEVELS, &u32TotalLevels))
	{    	
	}		
	DPNOK(u32TotalLevels);	

 //[david.modify] 2008-09-09 10:24
	 g_stBklightParam.u32LcdPwrOnDelay=DEF_u32LcdPwrOnDelay;
	if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, BKLIGHT_DRIVER_REGISTRY, TEXT("u32LcdPwrOnDelay"), (DWORD*)&g_stBklightParam.u32LcdPwrOnDelay))
	{    	
	}		
	DPNDEC(g_stBklightParam.u32LcdPwrOnDelay);	
	
	g_dwCurrLevel = u32Level;
	g_dwBklTotalLevels=u32TotalLevels;
//	SetPWMValue1(u32Light, TRUE);
	BspBacklightSetLevel(g_dwCurrLevel, TRUE);
}

 //[david.modify] 2008-08-20 09:18
int MapLevel2PwmValue(int nLevel)
{
	return gBklLevelCurrent2UpPrecentTable[nLevel];
}


DWORD  BspBacklightSetLevel(DWORD nLevel, BOOL bSave2Reg)
{
	int nPwmValue=0;


	if(BKL_OFF==nLevel) {
		SetPWMValue1(0, bSave2Reg);
	}else if(nLevel>BKL_TOTAL_LEVELS_DEFAULT){
		return -1;
	}else {
		
		nPwmValue = MapLevel2PwmValue(nLevel);
		SetPWMValue1(nPwmValue, bSave2Reg);		
	 //[david.modify] 2008-07-14 11:45
	 	if(bSave2Reg) {
			SetRegistryDWORD(HKEY_CURRENT_USER, TEXT("ControlPanel\\Backlight"), BKL_REG_CURR_LEVEL, nLevel);
	 	}
	}

	return nLevel;
}

DWORD BspBacklightDefaultLevel()
{
	BspBacklightSetLevel(BACKLIGHT_DEFAULT_LEVEL, TRUE);
	return 1;
}



BOOL 
BacklightInitialize()
{
	//DWORD  dwType = REG_DWORD;
	//!DWORD  nACBrightness,nBrightness;
	//!HKEY	hKey;

	BOOL    bRet = TRUE;
    #if 0 //unused code, f.w.lin
	stGPIOInfo stGPIOInfo={
		BACKLIGHT_PWM,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE
	};
    #endif
	RETAILMSG(BL_DBG,(TEXT("[BL_BKL_Initialize] \r\n")));

	/*START Memory Maped by yeob 06.02.12*/
	v_pGPIOReg = NULL;
	v_pPWMregs= NULL;

	if (v_pPWMregs == NULL)
		{
		PHYSICAL_ADDRESS PWMregPA = { S3C2450_BASE_REG_PA_PWM, 0 };
		v_pPWMregs = (S3C2450_PWM_REG* )MmMapIoSpace(PWMregPA, sizeof(S3C2450_PWM_REG),FALSE);
		}

	if (v_pGPIOReg == NULL)
		{
		PHYSICAL_ADDRESS ioPhysicalBase = { S3C2450_BASE_REG_PA_IOPORT, 0 };
		v_pGPIOReg = (S3C2450_IOPORT_REG* )MmMapIoSpace(ioPhysicalBase, sizeof(S3C2450_IOPORT_REG),FALSE);
		}
	if (g_pBspArgs == NULL)
		{
		PHYSICAL_ADDRESS BspArgsPA = { IMAGE_SHARE_ARGS_PA_START, 0 };
		g_pBspArgs = (BSP_ARGS *) MmMapIoSpace(BspArgsPA, sizeof(BSP_ARGS), FALSE);
		}

 //[david.modify] 2008-05-22 12:08
 // 因为在BOOTLOADER中已经初始化过了
#if 1
	PWM_Init(1);
#endif	

	SetCurRegistry_pwm();

	BL_On(TRUE);

	return bRet;
}

void BL_On(BOOL bOn)
{

	RETAILMSG(1,(TEXT("[BL::BL_On]..ENTER  \r\n")));

	if(bOn) 
	{
		if(bklPS != D0)
        {
			RETAILMSG(1,(TEXT("[BL::BL_On]..not D0..Backlight turn ON \r\n")));
			
			if(g_pBspArgs->pwr.BatteryPIOWakeup2)
			{
				RETAILMSG(BL_DBG,(_T("[BL_BL_On] not D0..BatteryPIOWakeup2")));
			} 
			else  
			{
                RETAILMSG(BL_DBG,(_T("+++++SystemIdleFix 55555 ====== \r\n")));

                //[david.modify] 2008-08-18 23:01
                // 解唤醒时背光突然一闪
                // 屏从睡眠中醒来，重开之后，需要一个时间稳定
                //否则会产生一闪白屏的问题
                //===========================
              #if 1
            	//开LCD电源LCD_PWREN
            	g_stGPIOInfo_LcdPwr.u32Stat = 1;
            	SetGPIOInfo(&g_stGPIOInfo_LcdPwr, (void*)v_pGPIOReg);
            	DPNDEC(g_stBklightParam.u32LcdPwrOnDelay);
            	Sleep(g_stBklightParam.u32LcdPwrOnDelay);
              #endif
                //===========================?
				DPSTR("SetCurRegistry_pwm");
				SetCurRegistry_pwm();
              #if 0
            	Sleep(10);
                #define VK_UNSIGNED_0xE5  0xE5
                //加如下两行去解有时唤醒时，背光不亮，要点下TOUCH或按下键才OK
                //=============
            	keybd_event((BYTE)VK_UNSIGNED_0xE5, 0, KEYEVENTF_SILENT, 0);
            	keybd_event ((BYTE)VK_UNSIGNED_0xE5, 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);
                //=============				
              #endif

				/*End ExtEscape Function Support Test #7015 by Yeob0420*/
				g_pBspArgs->misc.BackLight=1;
   				bklPS = D0;
			}
		} /*if(bklPS != D0)*/
	}
	else /*not if(bOn), D0-> D4*/
	{ 
		if(bklPS != D4)
		{
			RETAILMSG(BL_DBG,(_T("+++++SystemIdleFix 7777777777 ====== \r\n")));
            //[david.modify] 2008-08-18 09:52
			SetPWMValue1(0, FALSE);
          //[david.modify] 2008-08-18 11:36
          //之所以加延时，为了应用程序HIDE图片需要
    #if ( LCD_MODULE_TYPE	== BSP_LCD_INNOLUX_43 ) //no delay,f.w.lin

    #else
			Sleep(100);
    #endif

			g_pBspArgs->misc.BackLight=0;
   			bklPS = D4;
   		}
	}
}


BOOL 
BKL_Deinit(DWORD dwContext)
{
    if (v_pPWMregs)
    {
        MmUnmapIoSpace((void *)v_pPWMregs, sizeof(S3C2450_PWM_REG));
        v_pPWMregs = NULL;
    }

    if (g_pBspArgs)
    {
        MmUnmapIoSpace((void *)g_pBspArgs, sizeof(BSP_ARGS));
        g_pBspArgs = NULL;
    }

    if (v_pGPIOReg)
    {
        MmUnmapIoSpace((void *)v_pGPIOReg, sizeof(S3C2450_IOPORT_REG));
        v_pGPIOReg = NULL;
    }

 //[david.modify] 2008-08-19 14:50
 //================================
	if(g_tmpThread)
	 	CloseHandle(g_tmpThread);
	if(g_BackLightD0Event)
		CloseHandle(g_BackLightD0Event);
 //================================
    return TRUE;
}


DWORD 
BKL_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
	return dwData;
}


BOOL 
BKL_Close(DWORD Handle)
{
    return TRUE;
}


void 
BKL_PowerDown(void)
{
	RETAILMSG(1,(TEXT("[BL::BKL_PowerDown]..Enter SleepBacklight! \r\n"))); 
//	SleepBacklight(); /*Yeob 0216*/
 //[david.modify] 2008-08-18 10:57
//	BL_On(FALSE);
	RETAILMSG(BL_DBG,(_T("[BL::BKL_PowerDown]..Complete Sleep Backlight! \r\n")));
}


void
BKL_PowerUp(void)
{
	RETAILMSG(1,(TEXT("[BL::BKL_PowerUp]..Enter ResumeBacklight! \r\n"))); 
//	ResumeBacklight(v_pPWMregs,g_pBspArgs);
	RETAILMSG(BL_DBG,(TEXT("[BL::BKL_PowerUp]Comlete Resume Backlight!! \r\n"))); 
}

 //[david.modify] 2008-07-14 11:21
BOOL 
BKL_IOControl(
			  DWORD Handle, 
			  DWORD dwCode, 
			  PBYTE pBufIn, 
			  DWORD dwLenIn,
			  PBYTE pBufOut, 
			  DWORD dwLenOut, 
			  PDWORD pdwActualOut
			  )
{
	BOOL RetVal = TRUE;
       BOOL bRet=FALSE;
	DWORD dwErr = ERROR_SUCCESS;    
	UINT32 u32Pulse=0;	
	UINT32 uLevel=0;	


	switch (dwCode) 
	{
		/*Power Management*/
		case IOCTL_POWER_CAPABILITIES: 
			{
				PPOWER_CAPABILITIES ppc;
				if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(POWER_CAPABILITIES)) )
				{
					RetVal = FALSE;
					dwErr = ERROR_INVALID_PARAMETER;
					break;
				}
				ppc = (PPOWER_CAPABILITIES)pBufOut;
				memset(ppc, 0, sizeof(POWER_CAPABILITIES));

				/*support D0, D4*/
				ppc->DeviceDx = 0x11;
	   
				/*25 m = 25000 uA*/
				/*TODO: find out a more accurate value*/
				ppc->Power[D0] = 25000;
	            
				*pdwActualOut = sizeof(POWER_CAPABILITIES);
			}
			RETAILMSG(BL_DBG,(TEXT("[BL_BKL_IOControl]IOCTL_POWER_CAPABILITIES \r\n"))); 
			break;

		case IOCTL_POWER_SET: 
			RETAILMSG(1,(TEXT("[BL::BKL_IOControl] Enter IOCTL_POWER_SET \r\n"))); 
			RETAILMSG(BL_DBG,(TEXT("IOCTL_POWER_SET int_BL %d====== \r\n"), int_BL));

			{
				CEDEVICE_POWER_STATE NewDx;

				if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) 
				{
					RetVal = FALSE;
					dwErr = ERROR_INVALID_PARAMETER;
					break;
				}
            
				NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;
            
				RETAILMSG(BL_DBG,(TEXT("[BL_BKL_IOControl] BackLight IOCTL_POWER_SET  PowerSate = D%d \r\n"),NewDx)); 
            
				if ( VALID_DX(NewDx) ) 
				{
					switch ( NewDx ) 
					{
						case D0:
						/*Power changed, we need to notify the monitor thread to resync the timer*/
							RETAILMSG(BL_DBG,(TEXT("[BL::BKL_IOControl] IOCTL_POWER_SET..Turn on backlight!! \r\n"))); 
							BL_On(TRUE);
//[david.modify] 2008-09-09 10:03
//还是需要加此句，开背光
							SetEvent(g_BackLightD0Event);
							
							break;
						default:
							RETAILMSG(BL_DBG,(TEXT("[BL::BKL_IOControl] IOCTL_POWER_SET..Turn off backlight!! \r\n"))); 
							BL_On(FALSE);
							break;
					}
					*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
				}
				else
				{
					RetVal = FALSE;
					dwErr = ERROR_INVALID_PARAMETER;
					RETAILMSG(1,(TEXT("[BL_BKL_IOControl] Fault!! IOCTL_POWER_SET\r\n"))); 
				}            
			}
			RETAILMSG(1,(TEXT("[BL::BKL_IOControl] Exit IOCTL_POWER_SET \r\n")));
			break;

		case IOCTL_POWER_GET: 
			DPSTR("IOCTL_POWER_GET");
			if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) 
			{
				RetVal = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
				break;
			}

			CEDEVICE_POWER_STATE Dx;

			if (bklPS == D4){
				Dx = D4;
			}
			else
			{
				Dx = D0;
			}
			RETAILMSG(BL_DBG,(TEXT("[BL_BKL_IOControl] IOCTL_POWER_GET  PowerSate = D%d \r\n"),Dx));            

			*(PCEDEVICE_POWER_STATE)pBufOut = Dx;
			*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

			break;
//[david.modify] 2008-05-21 16:35
//========================== 
		case OEM_BKL_SET_ON:
			BL_On(TRUE);
			RETAILMSG(BL_DBG, (_T("BKL_IOControl: OEM_BKL_SET_ON:\r\n")));
			break;
		case OEM_BKL_SET_OFF:
			BL_On(FALSE);
			RETAILMSG(1, (_T("BKL_IOControl: OEM_BKL_SET_OFF:\r\n")));
			//RETAILMSG(1, (_T("BKL_IOControl: OEM_BKL_SET_OFF:\r\n")));
			break;
		case OEM_BKL_SET_ALWAYSON:
				{
				HKEY    hKey;	
				DWORD   dwVal;					
			if(RegOpenKeyEx(HKEY_CURRENT_USER, szregRootKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    			{
					// Get battery timeout and set to infinite
        			if(RegQueryValueEx(hKey, szregBatteryTimeout, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        			{
        				dwVal = INFINITE;
        				RegSetValueEx(hKey, szregBatteryTimeout, 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
        			}
        			else
        			{ 
        				RETAILMSG(1,(_T("Get battery timeout fail!!!\r\n")));
						RETAILMSG(1,(_T("Can't set the backlight always on in this condition\r\n")));
        			}
        	
					// Get ac power timeout and set to infinite
        			if(RegQueryValueEx(hKey, szregACTimeout, NULL,NULL, NULL, NULL) == ERROR_SUCCESS)
        			{
        				dwVal = INFINITE;
        				RegSetValueEx(hKey, szregACTimeout, 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
        			}
        			else    
        			{ 
        				RETAILMSG(1,(_T("Get ac power timeout fail!!!\r\n")));
						RETAILMSG(1,(_T("Can't set the backlight always on in this condition\r\n")));
        			}
				}
			RegCloseKey(hKey);
			}
			break;
	
//========================== 			
          case IOCTL_BACKLIGHT_SET:
		  	DPSTR("IOCTL_BACKLIGHT_SET");
            if(pBufIn&& dwLenIn>= sizeof(CEDEVICE_POWER_STATE))
            {
			uLevel = *pBufIn;			
			DPNOK(uLevel);			
 //[david.modify] 2008-08-18 20:24
 // IGO 地图启动时会CALL
 // 启动时会CALL IOCTL_BACKLIGHT_SET=10 , 10表示LEVEL,最大背光
 // 启动时会CALL IOCTL_BACKLIGHT_SET=255,255表示无此LEVEL,使用缺省LEVEL
			if (uLevel == BKL_ON)
                    {
                        BspBacklightSetLevel(g_dwCurrLevel,FALSE);
                    }
                    else   if (uLevel == BKL_OFF)
                    {
                        BspBacklightSetLevel(BKL_OFF, FALSE);
                    }
                    else
                    {
                        g_dwCurrLevel = uLevel;
                        g_dwCurrLevel=BspBacklightSetLevel(g_dwCurrLevel, TRUE);

                     }


            }
            RetVal=TRUE;
			DPNOK(u32Pulse);
            break;
          case IOCTL_BACKLIGHT_GET:
            if(pBufOut&& dwLenOut>= sizeof(CEDEVICE_POWER_STATE))
            {
                *(ULONG *)pBufOut =g_dwCurrLevel;
            }
            bRet=TRUE;
            break;
          case IOCTL_GET_TOTAL_LEVELS:
            if(pBufOut&& dwLenOut>= sizeof(CEDEVICE_POWER_STATE))
            {
                *(ULONG *)pBufOut =g_dwBklTotalLevels;
            }
            bRet=TRUE;
            break;
          case IOCTL_BACKLIGHT_SET_MAX:
            //g_dwCurrLevel = g_dwBklTotalLevels;
            //BspBacklightSetLevel(g_dwCurrLevel);
            bRet=TRUE;
            break;
          case IOCTL_BACKLIGHT_SET_CURRENT:
            BspBacklightDefaultLevel();
            bRet=TRUE;
            break;		

		default:
			RetVal = FALSE;
			break;
	}
	
	return(RetVal);
}


BOOL
WINAPI
DllMain(
    HANDLE  hinstDll,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}




int GetGPIOInfo2(
	stGPIOInfo *pstGPIOInfo,	// [out] GPIO信息
	void* pGPIOVirtBaseAddr	// [in] GPIO虚拟基地址
	)
{
	return GetGPIOInfo(pstGPIOInfo, pGPIOVirtBaseAddr);
}
	
int SetGPIOInfo2(
	stGPIOInfo *pstGPIOInfo,	// [out] GPIO信息
	void* pGPIOVirtBaseAddr	// [in] GPIO虚拟基地址
	)
{
	return SetGPIOInfo(pstGPIOInfo, pGPIOVirtBaseAddr);
}

 //[david.modify] 2008-08-19 14:25
 //======================================
void
Backlight_IST()
{
    DWORD we;
	
    while (1) {
        __try {
            we = WaitForSingleObject(g_BackLightD0Event, INFINITE);

 //[david.modify] 2008-08-19 14:36
 		DPSTR("g_BackLightD0Event");
 		Sleep(1000);
#if 1
#define VK_UNSIGNED_0xE5  0xE5
//加如下两行去解有时唤醒时，背光不亮，要点下TOUCH或按下键才OK
//=============
	keybd_event((BYTE)VK_UNSIGNED_0xE5, 0, KEYEVENTF_SILENT, 0);		
	keybd_event ((BYTE)VK_UNSIGNED_0xE5, 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);					
//=============
#endif
		ResetEvent(g_BackLightD0Event);

        } _except(EXCEPTION_EXECUTE_HANDLER) {
            RETAILMSG(1,(TEXT("!!! PWR_IST EXCEPTION: 0x%X !!!\r\n"), GetExceptionCode() ));
        }
    }

//	return 0;
}



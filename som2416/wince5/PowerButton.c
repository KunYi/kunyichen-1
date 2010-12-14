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

/* ++

    PCF50606 Power Supply Controller & Battery Management

    Notes:
    ======
    o) I2c client
	

-- */

#include <windows.h>
#include <nkintr.h>
#include <windev.h>
#include <winbase.h>

#include "pwr.h"
#include <bsp.h>
//[david.modify] 2008-05-16 09:46
#include "dbgmsg_david.h"
#include <xllp_gpio_david.h>
volatile BSP_ARGS *g_pBspArgs=NULL;

#define PWR_UNUSED_CODE 0  //屏蔽无用代码,等确认

static DWORD PWR_IST(LPVOID Context);

// Make sure the driver builds OK in tinykern configuration
HMODULE g_hMod = NULL;
typedef void (*PFN_GwesPowerOffSystem)(void);
PFN_GwesPowerOffSystem g_pfnGwesPowerOffSystem = NULL;

#define TICKTEST	0

#if TICKTEST
UINT32 g_PwrButtonIrq = IRQ_TICK;	// Determined by SMDK2450 board layout.
#else
UINT32 g_PwrButtonIrq = IRQ_EINT0;	// Determined by SMDK2450 board layout.
#endif

UINT32 g_PwrButtonSysIntr = SYSINTR_UNDEFINED;

UINT32 g_ResetButtonIrq = IRQ_EINT3;	// Determined by SMDK2450 board layout.
UINT32 g_ResetButtonSysIntr = SYSINTR_UNDEFINED;
HANDLE g_ResetThread;
HANDLE g_ResetEvent;
static DWORD RESET_IST();

volatile S3C2450_IOPORT_REG *v_pIOPregs;
volatile S3C2450_RTC_REG *v_pRTCregs;
volatile S3C2450_INTR_REG	*v_pIntrRegs;

#define ENABLE  TRUE
#define DISABLE FALSE

//[david.modify] 2008-05-16 09:42
// s805g pnd项目没有软件复位BUTTON 
//#define SWRST_BTN 1
#define SWRST_BTN 0
//[david. end] 2008-05-16 09:43

 //[david.modify] 2008-09-09 14:32
 //=======================================
#if(LCD_MODULE_TYPE==BSP_LCD_BYD_43INCH_480X272) || (LCD_MODULE_TYPE==BSP_LCD_INNOLUX_43)
#define MENU_KEY_BTN 1
#elif(LCD_MODULE_TYPE==BSP_LCD_YASSY_43INCH_480X272)
#define MENU_KEY_BTN 1
#else
#define MENU_KEY_BTN 0
#endif

#if MENU_KEY_BTN
static DWORD MENU_IST();
UINT32 g_MenuButtonIrq = IRQ_EINT4;	// Determined by SMDK2450 board layout.
UINT32 g_MenuButtonSysIntr = SYSINTR_UNDEFINED;
HANDLE g_MenuThread;
HANDLE g_MenuEvent;
#endif
 //=======================================

#define PWR_DRIVER_REGISTRY TEXT("\\Drivers\\BuiltIn\\PowerButton")
#define SUSPEND_BIT 0x1
#define RESUME_BIT 0x2
#define DEF_u32Debug (SUSPEND_BIT|RESUME_BIT)
#define DEF_u32Lcd 1

//[david.modify] 2008-08-07 17:55
#define DEF_u32BackLightDelay 0
#define DEF_u32PwrbtHoldTime 1500		// 缺省短按进入睡眠
typedef struct
{
	UINT32 u32Debug;	// 调试使用
	UINT32 u32Lcd;
	UINT32 u32LCDTYPE;
	UINT32 u32BackLightDelay;	// 用于控制背光开启DELAY时间
	UINT32 u32GPSPwr;		//得到睡眠前GPS电源状态
	UINT32 u32SDPwr;		//得到睡眠前SD CARD电源状态		
	UINT32 u32GLEDPwr;		//得到睡眠前GLED电源状态	
	UINT32 u32PwrbtHoldTime;	//控制长短按进入睡眠
}stPWRParam;
stPWRParam g_stPWRParam;
//[david.modify] 2008-07-11 11:54
 #define BITMAP_HIDE_ID 100
 #define BITMAP7_ID 7
#if PWR_UNUSED_CODE
HANDLE 	g_hEvent = 	NULL;
HANDLE 	g_hEvent2 = NULL;
#endif

//[david.modify] 2008-08-07 10:35
//========================================================================
#define SAFE_CLOSE_HANDLE(h)    do {            \
    if ( h ) { CloseHandle( h ); (h) = NULL; }  \
} while (0)

#define PM_POWERBTN_SLEEP_WAKEUP_EVENT  _T("SHARE_POWER_NOTIFY_EVENT")
HANDLE g_hPowerNotifyEvent                                    = NULL;
//========================================================================


stGPIOInfo g_stGPIOInfo[]={
	{ PWREN_USB,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
	{ LCD_PWREN,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ GPS_POWER_EN,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ SD_PWREN,  0, GPA_ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
	{ GLED,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		

//
	{ BACKLIGHT_PWM_OLD,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},			
	{ BACKLIGHT_PWM,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
	
	{ LCDRST,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},					
	{ LCDCS,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		

	{ LCDCLK,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ LCDSDA,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
//	{ TMC_ON,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},

	};	
WCHAR  stringGPIO[16]={0};
static WCHAR *strWhichGPIO(UINT32 i)
{
	WCHAR* strBuffer = stringGPIO;
		if(i<=GPA_END) {
			wsprintf(strBuffer, L"GPA%d", (i-GPA0));
		}else if(i<=GPB_END) {
			wsprintf(strBuffer, L"GPB%d", (i-GPB0));
		}else if(i<=GPC_END) {
			wsprintf(strBuffer, L"GPC%d", (i-GPC0));
		}else if(i<=GPD_END) {
			wsprintf(strBuffer, L"GPD%d", (i-GPD0));
		}else if(i<=GPE_END) {
			wsprintf(strBuffer, L"GPE%d", (i-GPE0));
		}else if(i<=GPF_END) {
			wsprintf(strBuffer, L"GPF%d", (i-GPF0));
		}else if(i<=GPG_END) {
			wsprintf(strBuffer, L"GPG%d", (i-GPG0));
		}else if(i<=GPH_END) {
			wsprintf(strBuffer, L"GPH%d", (i-GPH0));
		}else if(i<=GPJ_END) {
			wsprintf(strBuffer, L"GPJ%d", (i-GPJ0));
		}else if(i<=GPK_END) {
			wsprintf(strBuffer, L"GPK%d", (i-GPK0));
		}else if(i<=GPL_END) {
			wsprintf(strBuffer, L"GPL%d", (i-GPL0));
		}else if(i<=GPM_END) {
			wsprintf(strBuffer, L"GPM%d", (i-GPM0));
		}
		return strBuffer;
}


void PrintAllGPIOStat()
{
	stGPIOInfo stGPIOInfo;
//	volatile XLLP_GPIO_T     *v_pGPIOReg = NULL;	
 	int i=0;
       if(v_pIOPregs==NULL) return 0;
	EPRINT(L"GPXX(NO)= [u32Stat-u32AltFunc(0-in, 01-out, 2,3-func)-u32PullUpdown(2,3-disable; 0,1-enable)] \r\n");		
	EPRINT(L"=============================================== \r\n");	
	for(i=0;i<S3C24XX_GPIOS;i++){
		stGPIOInfo.u32PinNo = 	i;	
		GetGPIOInfo(&stGPIOInfo, v_pIOPregs);
		EPRINT(L"%s (%d)= [%d %d %d] \r\n",
			strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
			stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		
		
	}
	EPRINT(L"\r\n=============================================== \r\n");			
	return 1;
}


 #include <spi_david.h>

//[david.modify] 2008-05-26 17:01
//==============================
int OEM_Suspend()
{
	DWORD dwResult=0;
	DPNOK(0);
       if(v_pIOPregs==NULL) {
	   	DPN(v_pIOPregs);
	   	return 0;
       }

	//关USB电源PWREN_USB	   
	g_stGPIOInfo[0].u32Stat = 0;
	SetGPIOInfo(&g_stGPIOInfo[0], v_pIOPregs);

	if(g_stPWRParam.u32Lcd) {
/*		
		DPSTR("+LCD_Sharp_Enter2Sleep");
		LCD_Sharp_Enter2Sleep(v_pIOPregs);
		DPSTR("-LCD_Sharp_Enter2Sleep");	
*/		

		BspLcdPowerDownPanel(v_pIOPregs, g_pBspArgs->u32LCDType);
		//关LCD电源LCD_PWREN
		g_stGPIOInfo[1].u32Stat = 0;
		SetGPIOInfo(&g_stGPIOInfo[1], v_pIOPregs);		
	}


	
#if 0	
	// 关背光
	g_stGPIOInfo[5].u32Stat = 0;	
	SetGPIOInfo(&g_stGPIOInfo[5], v_pIOPregs);
	g_stGPIOInfo[6].u32Stat = 0;	
	SetGPIOInfo(&g_stGPIOInfo[6], v_pIOPregs);
#endif	

 //[david.modify] 2008-07-11 12:11
 //通知应用程序将SHOW的图片隐藏起来
 //=========================
 #if 0
	g_hEvent = 	OpenEvent(EVENT_ALL_ACCESS ,FALSE , EVENT_OVL);
	DPNOK(g_hEvent);	
	if(g_hEvent != INVALID_HANDLE_VALUE)
	{
		DPNOK(BITMAP_HIDE_ID);
		SetEventData(g_hEvent , BITMAP_HIDE_ID);	//
		SetEvent (g_hEvent);
		CloseHandle(g_hEvent);

		DPSTR("g_hEvent2");
		g_hEvent2 = 	OpenEvent(EVENT_ALL_ACCESS ,FALSE , EVENT_OVL_END);
		DPNOK(g_hEvent2);		
		dwResult = WaitForSingleObject(g_hEvent2, 10000);	//等待10s	
		DPNOK(g_hEvent2);		
		DPNOK(dwResult);
		CloseHandle(g_hEvent2);			
		
	}
	DPNOK(g_hEvent);
#endif

  //=========================
	
	
	
	
	//关GPS电源GPS_POWER_EN
#if 0
// 因为已经在串口驱动中改了
	GetGPIOInfo(&g_stGPIOInfo[2], v_pIOPregs);
	g_stPWRParam.u32GPSPwr = g_stGPIOInfo[2].u32Stat ;
	g_stGPIOInfo[2].u32Stat = 0;	
	SetGPIOInfo(&g_stGPIOInfo[2], v_pIOPregs);
#endif	

	//关SD卡电源SD_PWREN
	GetGPIOInfo(&g_stGPIOInfo[3], v_pIOPregs);
	g_stPWRParam.u32SDPwr = g_stGPIOInfo[3].u32Stat ;
	g_stGPIOInfo[3].u32Stat = 0;	
	SetGPIOInfo(&g_stGPIOInfo[3], v_pIOPregs);	

	//关GLED指示灯
	GetGPIOInfo(&g_stGPIOInfo[4], v_pIOPregs);
	g_stPWRParam.u32GLEDPwr = g_stGPIOInfo[4].u32Stat ;	
	g_stGPIOInfo[4].u32Stat = 1;	
	SetGPIOInfo(&g_stGPIOInfo[4], v_pIOPregs);




	g_stGPIOInfo[7].u32Stat = 0;	
	SetGPIOInfo(&g_stGPIOInfo[7], v_pIOPregs);

	g_stGPIOInfo[8].u32Stat = 0;	
	SetGPIOInfo(&g_stGPIOInfo[8], v_pIOPregs);

	g_stGPIOInfo[9].u32Stat = 0;	
	SetGPIOInfo(&g_stGPIOInfo[9], v_pIOPregs);

	g_stGPIOInfo[10].u32Stat = 0;	
	SetGPIOInfo(&g_stGPIOInfo[10], v_pIOPregs);	
	
	
	


	DPNOK(g_stPWRParam.u32GPSPwr);
	DPNOK(g_stPWRParam.u32SDPwr);
	DPNOK(g_stPWRParam.u32GLEDPwr);	
	
	PrintAllGPIOStat();

	return TRUE;
}

int OEM_Resume()
{
	DPNOK(0);
       if(v_pIOPregs==NULL) {
	   	DPN(v_pIOPregs);	   	
	   	return 0;	
       }

	    //[david.modify] 2008-06-26 14:36
    	DPNOK(g_stPWRParam.u32GPSPwr);
	DPNOK(g_stPWRParam.u32SDPwr);
	DPNOK(g_stPWRParam.u32GLEDPwr);	
//===================================	    
	//开USB电源PWREN_USB	   
//	g_stGPIOInfo[0].u32Stat = 1;
//	SetGPIOInfo(&g_stGPIOInfo[0], v_pIOPregs);



//[david.modify] 2008-09-02 14:12
// 因为在kernel\oal\init.c中唤醒后有调用BspLcdPowerUpPanel
//所以在此处拿掉这部分代码
#if 0	
	if(g_stPWRParam.u32Lcd) 
//	if(0)
	{
			//开LCD电源LCD_PWREN
			g_stGPIOInfo[1].u32Stat = 1;
			SetGPIOInfo(&g_stGPIOInfo[1], v_pIOPregs);	
	
			DPSTR("+LCD_Sharp_Enter2Sleep");
//[david.modify] 2008-07-04 18:08
//LCD_Sharp_ExitFromSleep是造成屏闪的问题的原因
//==============================
//			LCD_Sharp_ExitFromSleep(v_pIOPregs);
//			InitLDI_LTV350_sharp2(v_pIOPregs);
	DPNOK(g_pBspArgs->u32LCDType);
	BspLcdPowerUpPanel(v_pIOPregs, g_pBspArgs->u32LCDType);		
//==============================
			DPSTR("-LCD_Sharp_Enter2Sleep");	
	}
#endif	
	
	//开GPS电源GPS_POWER_EN
#if 0
// 因为已经在串口驱动中改了	
	g_stGPIOInfo[2].u32Stat = g_stPWRParam.u32GPSPwr;	
	SetGPIOInfo(&g_stGPIOInfo[2], v_pIOPregs);
#endif	
	
	//开SD卡电源SD_PWREN
	g_stGPIOInfo[3].u32Stat = g_stPWRParam.u32SDPwr;	
	SetGPIOInfo(&g_stGPIOInfo[3], v_pIOPregs);	

	//开GLED指示灯
	g_stGPIOInfo[4].u32Stat = g_stPWRParam.u32GLEDPwr;	
	SetGPIOInfo(&g_stGPIOInfo[4], v_pIOPregs);	

// SPI 控制
//=================================

//背光
//=============
	// 等待500 ms再开背光， 解决一闪殘画面问题
#if 0	
	Sleep(5000);
	g_stGPIOInfo[5].u32Stat = 1;	
	SetGPIOInfo(&g_stGPIOInfo[5], v_pIOPregs);

	g_stGPIOInfo[6].u32Stat = 1;	
	SetGPIOInfo(&g_stGPIOInfo[6], v_pIOPregs);
#endif	
//=============	

#if 0
	g_stGPIOInfo[7].u32Stat = 1;	
	SetGPIOInfo(&g_stGPIOInfo[7], v_pIOPregs);

	g_stGPIOInfo[8].u32Stat = 1;	
	SetGPIOInfo(&g_stGPIOInfo[8], v_pIOPregs);
#endif	

//=================================	


	
	PrintAllGPIOStat();	
	return TRUE;
}









//==============================
 
DWORD
HW_InitRegisters(
    PPWR_CONTEXT pPWR
    )
{
    DWORD retry = 0;

#if TICKTEST
	v_pRTCregs = (volatile S3C2450_RTC_REG *)VirtualAlloc(0, sizeof(S3C2450_RTC_REG), MEM_RESERVE, PAGE_NOACCESS);
	VirtualCopy((PVOID)v_pRTCregs, (PVOID)(S3C2450_BASE_REG_PA_RTC >> 8), sizeof(S3C2450_RTC_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE );	
	v_pIOPregs = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE );	

	
	v_pRTCregs->TICNT0 = (1<<7);
//	v_pRTCregs->TICNT2 = (3<<0);
	v_pRTCregs->TICNT2 = 0xa;
	v_pRTCregs->RTCCON = (5<<5)|(1<<0);

	v_pIOPregs->GPFUDP&= ~(0x3 << 0);		// Pull up down disable
	v_pIOPregs->GPFUDP|=  (0x0 << 0);

	v_pIOPregs->GPFUDP&= ~(0x3 << 6);		
	v_pIOPregs->GPFUDP|=  (0x2 << 6);// Pull up enable
	
#else

	v_pIOPregs = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE );	
	
   v_pIntrRegs = (volatile S3C2450_INTR_REG *)VirtualAlloc(0, sizeof(S3C2450_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
   VirtualCopy((PVOID)v_pIntrRegs, (PVOID)(S3C2450_BASE_REG_PA_INTR >> 8), sizeof(S3C2450_INTR_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE );	
   
	v_pIOPregs->EXTINT0 = (v_pIOPregs->EXTINT0 & ~(0xf<<0)) | (0x0 << 3)|(0x2 << 0);		/* Configure EINT0 as Falling Edge Mode				*/	
	
	v_pIOPregs->GPFCON  &= ~(0x3 << 0);		/* Set EINT0(GPF0) as EINT0							*/
	v_pIOPregs->GPFCON  |=  (0x2 << 0);
	
	v_pIOPregs->GPFUDP&= ~(0x3 << 0);		// Pull up down disable
	v_pIOPregs->GPFUDP|=  (0x0 << 0);

 //[david.modify] 2008-07-30 14:20
 // USB_DET 为GPF2 （EINT2），用来检测USB是否插入，唤醒SYSTEM
 //===================================================
 	v_pIOPregs->EXTINT0 = (v_pIOPregs->EXTINT0 & ~(0xf<<8)) | (0x0 << 11)|(0x2 << 8);		/* Configure EINT2 as Falling Edge Mode				*/	
	
	v_pIOPregs->GPFCON  &= ~(0x3 << 4);		/* Set EINT2(GPF2) as EINT2							*/
	v_pIOPregs->GPFCON  |=  (0x2 << 4);
	
	v_pIOPregs->GPFUDP&= ~(0x3 << 4);		// Pull up down disable
	v_pIOPregs->GPFUDP|=  (0x0 << 4);
	DPNOK(v_pIOPregs->EXTINT0);
 //==================================================

#if MENU_KEY_BTN
	v_pIOPregs->EXTINT0 = (v_pIOPregs->EXTINT0& ~(0xf<<16)) |(0x0 <<19)| (0x2 << 16); 	
	
	v_pIOPregs->GPFCON  &= ~(0x3 << 8);		/* Set EINT4(GPF4) as EINT4							*/
	v_pIOPregs->GPFCON  |=  (0x2 << 8);

	v_pIOPregs->GPFUDP&= ~(0x3 << 8);		
	v_pIOPregs->GPFUDP|=  (0x2 << 8);// Pull up enable

	DPNOK(v_pIOPregs->GPFCON);
	DPSTR("MENU_KEY_BTN\r\n");
#endif	



#if SWRST_BTN
	v_pIOPregs->EXTINT0 = (v_pIOPregs->EXTINT0& ~(0xf<<12)) |(0x0 <<15)| (0x2 << 12); 	
	
	v_pIOPregs->GPFCON  &= ~(0x3 << 6);		/* Set EINT3(GPF3) as EINT3							*/
	v_pIOPregs->GPFCON  |=  (0x2 << 6);

	v_pIOPregs->GPFUDP&= ~(0x3 << 6);		
	v_pIOPregs->GPFUDP|=  (0x2 << 6);// Pull up enable
#endif  // SWRST_BTN
#endif  // TICKTEST

    return 0;
}


/* ++

    The reset value of the PCF50606 INTxM registers is 0: Interrupt enabled.
    Once we enable the interrupt they start firing unless we mask them _before_
    enabeling the interrupt(s).
    
-- */
DWORD
HW_Init(
    PPWR_CONTEXT pPWR
    )
{
    DWORD dwErr = ERROR_SUCCESS;

    if ( !pPWR )
        return ERROR_INVALID_PARAMETER;

 //[david.modify] 2008-05-27 15:31
 //==========================

 //[david.modify] 2008-05-17 10:46

    g_stPWRParam.u32Debug=DEF_u32Debug;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, PWR_DRIVER_REGISTRY, TEXT("u32Debug"), &g_stPWRParam.u32Debug))
    {    	
    }	
    DPNOK(g_stPWRParam.u32Debug);	

    g_stPWRParam.u32Lcd=DEF_u32Lcd;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, PWR_DRIVER_REGISTRY, TEXT("u32Lcd"), &g_stPWRParam.u32Lcd))
    {    	
    }	
    DPNOK(g_stPWRParam.u32Lcd);		

    g_stPWRParam.u32BackLightDelay=DEF_u32BackLightDelay;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, PWR_DRIVER_REGISTRY, TEXT("u32BackLightDelay"), &g_stPWRParam.u32BackLightDelay))
    {    	
    }	
    DPNOK(g_stPWRParam.u32BackLightDelay);		


    g_stPWRParam.u32PwrbtHoldTime=DEF_u32PwrbtHoldTime;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, PWR_DRIVER_REGISTRY, TEXT("u32PwrbtHoldTime"), &g_stPWRParam.u32PwrbtHoldTime))
    {    	
    }	
    DPNOK(g_stPWRParam.u32PwrbtHoldTime);	

 //============================


 //[david.modify] 2008-08-30 12:07
 //=============================
	if (g_pBspArgs == NULL)
		{
		PHYSICAL_ADDRESS BspArgsPA = { IMAGE_SHARE_ARGS_PA_START, 0 };
		g_pBspArgs = (BSP_ARGS *) MmMapIoSpace(BspArgsPA, sizeof(BSP_ARGS), FALSE);
		}
 //=============================	

    // Init H/W
    pPWR->State = INITIALIZE;
    
    RETAILMSG(1, (TEXT("HW_Init : HW_InitRegisters \r\n")));
	dwErr = HW_InitRegisters(pPWR);
    if ( dwErr ) {
	    RETAILMSG(1, (TEXT("HW_Init : HW_InitRegisters  Error (0x%x) \r\n"), dwErr));
        goto _error_exit;
    }

    RETAILMSG(1, (TEXT("HW_Init : CreateEvent \r\n")));
	pPWR->ISTEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Request a SYSINTR value from the OAL.
    //
    RETAILMSG(1, (TEXT("HW_Init : IOCTL_HAL_REQUEST_SYSINTR \r\n")));
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &g_PwrButtonIrq, sizeof(UINT32), &g_PwrButtonSysIntr, sizeof(UINT32), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: PwrButton: Failed to request sysintr value for power button interrupt.\r\n")));
        return(0);
    }
    RETAILMSG(1,(TEXT("INFO: PwrButton: Mapped Irq 0x%x to SysIntr 0x%x.\r\n"), g_PwrButtonIrq, g_PwrButtonSysIntr));

	if (!(InterruptInitialize(g_PwrButtonSysIntr, pPWR->ISTEvent, 0, 0))) 
	{
		RETAILMSG(1, (TEXT("ERROR: PwrButton: Interrupt initialize failed.\r\n")));
	}

    RETAILMSG(1, (TEXT("HW_Init : CreateThread \r\n"), dwErr));
    if ( (pPWR->IST = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) PWR_IST, pPWR, 0, NULL)) == NULL) {
        dwErr = GetLastError();
        RETAILMSG(1,(TEXT("PCF50606 ERROR: Unable to create IST: %u\r\n"), dwErr));
        goto _error_exit;
    }
    
    RETAILMSG(1, (TEXT("HW_Init : CeSetThreadPriority \r\n"), dwErr));
    // TODO: registry override
    if ( !CeSetThreadPriority(pPWR->IST, POWER_THREAD_PRIORITY)) {
        dwErr = GetLastError();
        RETAILMSG(1, (TEXT("PCF50606 ERROR: CeSetThreadPriority ERROR:%d \r\n"), dwErr));
        goto _error_exit;
    }

    pPWR->State = RUN;

    RETAILMSG(1, (TEXT("HW_Init : Done \r\n"), dwErr));    
#if SWRST_BTN
	g_ResetEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &g_ResetButtonIrq, sizeof(UINT32), &g_ResetButtonSysIntr, sizeof(UINT32), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: ResetButton: Failed to request sysintr value for reset button interrupt.\r\n")));
        return(0);
    }  
    RETAILMSG(1,(TEXT("INFO: ResetButton: Mapped Irq 0x%x to SysIntr 0x%x.\r\n"), g_ResetButtonIrq, g_ResetButtonSysIntr));
	if (!(InterruptInitialize(g_ResetButtonSysIntr, g_ResetEvent, 0, 0))) 
	{
		RETAILMSG(1, (TEXT("ERROR: ResetButton: Interrupt initialize failed.\r\n")));
	}
    if ( (g_ResetThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) RESET_IST, 0, 0, NULL)) == NULL) {
        dwErr = GetLastError();
        RETAILMSG(1,(TEXT("PCF50606 ERROR: Unable to create IST: %u\r\n"), dwErr));
        goto _error_exit;
    }	
    if ( !CeSetThreadPriority(g_ResetThread, POWER_THREAD_PRIORITY)) {
        dwErr = GetLastError();
        RETAILMSG(1, (TEXT("PCF50606 ERROR: CeSetThreadPriority ERROR:%d \r\n"), dwErr));
        goto _error_exit;
    }    
#endif    


#if MENU_KEY_BTN
	g_MenuEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &g_MenuButtonIrq, sizeof(UINT32), &g_MenuButtonSysIntr, sizeof(UINT32), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: ResetButton: Failed to request sysintr value for reset button interrupt.\r\n")));
        return(0);
    }  
    RETAILMSG(1,(TEXT("INFO: ResetButton: Mapped Irq 0x%x to SysIntr 0x%x.\r\n"), g_MenuButtonIrq, g_MenuButtonSysIntr));
	DPNOK(g_MenuButtonIrq);	
	DPNOK(g_MenuButtonSysIntr);		
	if (!(InterruptInitialize(g_MenuButtonSysIntr, g_MenuEvent, 0, 0))) 
	{
		RETAILMSG(1, (TEXT("ERROR: ResetButton: Interrupt initialize failed.\r\n")));
	}
    if ( (g_MenuThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) MENU_IST, 0, 0, NULL)) == NULL) {
        dwErr = GetLastError();
        RETAILMSG(1,(TEXT("PCF50606 ERROR: Unable to create IST: %u\r\n"), dwErr));
        goto _error_exit;
    }	
    if ( !CeSetThreadPriority(g_MenuThread, POWER_THREAD_PRIORITY)) {
        dwErr = GetLastError();
        RETAILMSG(1, (TEXT("PCF50606 ERROR: CeSetThreadPriority ERROR:%d \r\n"), dwErr));
        goto _error_exit;
    }    
#endif    



 //[david.modify] 2008-07-11 15:06
#if PWR_UNUSED_CODE 
	g_hEvent = 	OpenEvent(EVENT_ALL_ACCESS ,FALSE , EVENT_OVL);
 	DPNOK(g_hEvent);  	
	g_hEvent2 =OpenEvent(EVENT_ALL_ACCESS ,FALSE , EVENT_OVL_END);
 	DPNOK(g_hEvent2);	    
#endif

_error_exit:
    return dwErr;
}


DWORD
HW_Deinit(
    PPWR_CONTEXT pPWR
    )
{
    if ( !pPWR )
        return ERROR_INVALID_PARAMETER;
    
    RETAILMSG(1,(TEXT("+PWR_Deinit \r\n")));

    pPWR->State = UNINITIALIZED;

    if (pPWR->hADC && CloseHandle(pPWR->hADC))
        pPWR->hADC = NULL;

    if (pPWR->hTSCPRES && CloseHandle(pPWR->hTSCPRES))
        pPWR->hTSCPRES = NULL;

    if (pPWR->hI2C && pPWR->hI2C != INVALID_HANDLE_VALUE && CloseHandle(pPWR->hI2C))
        pPWR->hI2C = INVALID_HANDLE_VALUE;
    
    InterruptDisable(g_PwrButtonSysIntr);

    if (pPWR->ISTEvent && CloseHandle(pPWR->ISTEvent))
        pPWR->ISTEvent = NULL;
    
    if (pPWR->IST && CloseHandle(pPWR->IST))
        pPWR->IST = NULL;  

    //  Free the coredll instance if we have allocated one
    if(g_hMod) {
        FreeLibrary(g_hMod);
        g_hMod = NULL;
    }

 //[david.modify] 2008-07-11 15:07
#if PWR_UNUSED_CODE 
 	if(g_hEvent)		CloseHandle(g_hEvent);
 	if(g_hEvent2)		CloseHandle(g_hEvent2);	
#endif

	VirtualFree((PVOID) v_pIOPregs, 0, MEM_RELEASE);
	// EINT0, GPF0 Port Init Done
	VirtualFree((PVOID) v_pRTCregs, 0, MEM_RELEASE);	

    RETAILMSG(1,(TEXT("-PWR_Deinit \r\n")));

 //[david.modify] 2008-08-07 10:40
	SAFE_CLOSE_HANDLE( g_hPowerNotifyEvent );

 //[david.modify] 2008-08-30 12:08
    if (g_pBspArgs)
    {
        MmUnmapIoSpace((void *)g_pBspArgs, sizeof(BSP_ARGS));
        g_pBspArgs = NULL;
    }

    return ERROR_SUCCESS;
}


DWORD 
HW_Open(
    PPWR_CONTEXT pPWR
    )
{
    RETAILMSG(1, (TEXT("PCF: HW_Open \r\n")));
    return ERROR_SUCCESS;
}


DWORD 
HW_Close(
    PPWR_CONTEXT pPWR
    )
{
    RETAILMSG(1, (TEXT("PCF: HW_Close \r\n")));
    return ERROR_SUCCESS;
}


BOOL
HW_PowerUp(
    PPWR_CONTEXT pPWR
   )
{
    RETAILMSG(1, (TEXT("PCF: HW_PowerUp \r\n")));
    pPWR->State = RESUME;
//    pPWR->State = RUN;
//    SetInterruptEvent(g_PwrButtonSysIntr);
    DPNOK(g_stPWRParam.u32Debug);	
    if(g_stPWRParam.u32Debug&RESUME_BIT){
	    OEM_Resume();
    }
    return TRUE;
}


BOOL
HW_PowerDown(
    PPWR_CONTEXT pPWR
   )
{
    RETAILMSG(1, (TEXT("PCF: HW_PowerDown \r\n")));
    pPWR->State = SUSPEND;

    DPNOK(g_stPWRParam.u32Debug);	
    if(g_stPWRParam.u32Debug&SUSPEND_BIT){	
    OEM_Suspend();
    }
    return TRUE;
}


BOOL
HW_PowerCapabilities(
    PPWR_CONTEXT pPWR,
    PPOWER_CAPABILITIES ppc
    )
{
    return TRUE;
}


BOOL
HW_PowerSet(
    PPWR_CONTEXT pPWR,
    PCEDEVICE_POWER_STATE pDx   // IN, OUT
   )
{   
    CEDEVICE_POWER_STATE NewDx = *pDx;

    if ( VALID_DX(NewDx) ) 
    {
        // We only support D0, so do nothing.
        // Just return current state.
        pPWR->Dx = *pDx = D0;
        RETAILMSG(1, (TEXT("PCF: IOCTL_POWER_SET: D%u => D%u \r\n"), NewDx, pPWR->Dx));
        return TRUE;
    }

    return FALSE;
}


BOOL
HW_PowerGet(
    PPWR_CONTEXT pPWR,
    PCEDEVICE_POWER_STATE pDx
   )
{   
    // return our Current Dx value
    *pDx = pPWR->Dx;
    RETAILMSG(1, (TEXT("PCF: IOCTL_POWER_GET: D%u \r\n"), pPWR->Dx));

    return TRUE;
}


 //[david.modify] 2008-07-08 17:30
 //=======================

 
int PwrButtonState(UINT32 dwMustHoldTime)
{
	DWORD dwStartTick;
 	stGPIOInfo stGPIOInfo1;
	int nCnt=0;
	
	stGPIOInfo1.u32PinNo = PWR_KEY;
	dwStartTick = GetTickCount();	
	GetGPIOInfo(&stGPIOInfo1, v_pIOPregs);
	while (GetTickCount() -  dwStartTick < dwMustHoldTime)
	{
		Sleep(50);	
		nCnt++;
		DPNOK(nCnt);
		
		GetGPIOInfo(&stGPIOInfo1, v_pIOPregs);	
		if(0==stGPIOInfo1.u32Stat)		// 0-低
		{
			DPSTR("SHORT PRESS");			
			continue;
		}
		else{
			return FALSE;
		}
		
	}
	DPSTR("LONG PRESS");
	return TRUE;

}

 //[david.modify] 2008-08-07 14:24
 //
//#define WAKEUP_EVENT_ID  12
static int SendPwrEvent(UINT32 u32ID)
{

	HANDLE hEvent=NULL;
	
	DPNOK(u32ID);
	hEvent = 	CreateEvent(NULL , FALSE , FALSE  , PM_POWER_NOTIFY_EVENT2); 
	DPNOK(hEvent);
	if(hEvent != INVALID_HANDLE_VALUE)
	{	 
		SetEventData (hEvent, u32ID); 	//唤醒消息
		SetEvent (hEvent);
				
		CloseHandle(hEvent);		
		hEvent = NULL;
	}

	return 1;
}
 //=============================

int Send_ShortkeyEvent()
{
	HANDLE SleepShortKeyEvent;	    
	SleepShortKeyEvent=CreateEvent(NULL, FALSE, FALSE, _T("_ATLAS_APP_EVENT"));
	DPNOK(SleepShortKeyEvent);
	if(SleepShortKeyEvent != INVALID_HANDLE_VALUE)
	{	 
		DPSTR("SleepShortKeyEvent");
		SetEventData (SleepShortKeyEvent, 4);
		SetEvent (SleepShortKeyEvent);	
		
		CloseHandle(SleepShortKeyEvent);		
		SleepShortKeyEvent = NULL;
	}
	return 1;
}

  //[david.modify] 2008-08-18 11:03
  #include <sdk_api_david.h>
  int OEM_IOCTL_Backlight_ONOFF(int nONOff)
  {
  	int nRet;
  	HANDLE hBacklight;

	DPNOK(nONOff);

	if((hBacklight = CreateFile(L"BKL1:", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE)
	{
		DPN(-1);
		return -1;
	}else{	
		DPNOK(hBacklight);
		if(1==nONOff){
		nRet = DeviceIoControl(hBacklight , OEM_BKL_SET_ON, NULL , NULL , NULL, 0, NULL, NULL);
		}else{
		nRet = DeviceIoControl(hBacklight , OEM_BKL_SET_OFF , NULL , NULL , NULL, 0, NULL, NULL);
		}
		CloseHandle(hBacklight);			
	}
	DPNOK(hBacklight);
	return nRet;
  }


static DWORD 
PWR_IST(
    PPWR_CONTEXT pPWR
    )
{
    DWORD we;
	#if PWR_UNUSED_CODE  //unused code, f.w.lin
	WCHAR  state[1024] = {0};
	LPWSTR pState = &state[0];
	DWORD dwBufChars = (sizeof(state) / sizeof(state[0]));
	DWORD  dwStateFlags = 0;
//	DWORD dwErr;
    DWORD dwTimeout = 1000;
	DWORD dwResult=0;
	#endif
	int nRet =0;
	HANDLE SleepShortKeyEvent;	
    
    if (!pPWR)
        return ERROR_INVALID_PARAMETER;

	SleepShortKeyEvent=CreateEvent(NULL, FALSE, FALSE, _T("_ATLAS_APP_EVENT"));
	g_hPowerNotifyEvent = CreateEvent(NULL,FALSE,FALSE,PM_POWERBTN_SLEEP_WAKEUP_EVENT);
	
    while (1) {
        __try {
            we = WaitForSingleObject(pPWR->ISTEvent, INFINITE);
			RETAILMSG(1,(TEXT("\r\nPWR_IST: pPWR->State = 0x%x \r\n"), pPWR->State));

			InterruptDone(g_PwrButtonSysIntr);

            
			if ((pPWR->State == RUN))
			{
				DPNOK(g_stPWRParam.u32PwrbtHoldTime);
				nRet = PwrButtonState(g_stPWRParam.u32PwrbtHoldTime);
				if(TRUE==nRet) {
 //[david.modify] 2008-06-24 15:26
				DPSTR("POWER_STATE_SUSPEND");

#if 1
    		    RETAILMSG(1, (TEXT("Set Sleep Event!\r\n")));
                SetEventData(g_hPowerNotifyEvent, 1);
                SetEvent(g_hPowerNotifyEvent);
#endif

 
#if 1
	DPSTR("+BITMPA_SLEEP_ID");
	SendOvlEvent(BITMPA_SLEEP_ID);
	DPSTR("-BITMPA_SLEEP_ID");
	Sleep(2000);

	// 关背光

	DPSTR("+SetDevicePower");
//      SetDevicePower(L"BKL1:", POWER_NAME, D4);
	OEM_IOCTL_Backlight_ONOFF(0);
	DPSTR("-SetDevicePower");
//	Sleep(100);

	//RETAILMSG(1, (TEXT("\r\n close LCD_PWREN\r\n")));
	//关LCD电源LCD_PWREN
#if ( LCD_MODULE_TYPE	== BSP_LCD_INNOLUX_43 )
	//解决群创屏睡眠有阴影问题最简单的办法f.w.lin
	g_stGPIOInfo[1].u32Stat = 0;
	SetGPIOInfo(&g_stGPIOInfo[1], v_pIOPregs);
#endif

	DPSTR("+BITMAP_HIDE_ID");	
	SendOvlEvent(BITMAP_HIDE_ID);
	DPSTR("-BITMAP_HIDE_ID");
	Sleep(150);
#else	// from battery
	DPSTR("+BITMAP_LOWBAT_ID");
	SendOvlEvent(BITMAP_LOWBAT_ID);
	DPSTR("-BITMAP_LOWBAT_ID");
	Sleep(2000);

	// 关背光
	OEM_IOCTL_Backlight_ONOFF(0);		
	
	DPSTR("+BITMAP_HIDE_ID");	
	SendOvlEvent(BITMAP_HIDE_ID);
	DPSTR("-BITMAP_HIDE_ID");

	Sleep(100);	
	

#endif

 #if 0
 //[david.modify] 2008-07-11 11:52
 // 向绘图应用程序发送一个事件，要求绘图
 //===========================================
	g_hEvent = 	OpenEvent(EVENT_ALL_ACCESS ,FALSE , EVENT_OVL);
	DPNOK(g_hEvent);
	if(g_hEvent != INVALID_HANDLE_VALUE)
	{
		SetEventData(g_hEvent , BITMAP7_ID);	//
		SetEvent (g_hEvent);
		CloseHandle(g_hEvent);			
//		Sleep(3000);
 //[david.modify] 2008-07-11 12:16
 //========================
 		DPSTR("g_hEvent2");	
		g_hEvent2 = 	OpenEvent(EVENT_ALL_ACCESS ,FALSE , EVENT_OVL_END);
		DPNOK(g_hEvent2);

		dwResult = WaitForSingleObject(g_hEvent2, 10000);	//等待10s
		DPNOK(g_hEvent2);
		DPNOK(dwResult);
		CloseHandle(g_hEvent2);					
 //==========================		

	}
	DPNOK(g_hEvent);

#endif 	

 //===========================================

 
					SetSystemPowerState( NULL, POWER_STATE_SUSPEND, POWER_FORCE );					
				}else{
					DPSTR("SKIP POWER_STATE_SUSPEND ");					
					//SetSystemPowerState( NULL, POWER_STATE_SUSPEND, POWER_FORCE );					

// myshell 会去收如下MSG					
//=============================
#if 1
			if (NULL!=SleepShortKeyEvent)
			{
#if 0			
				DPSTR("SleepShortKeyEvent");
				SetEventData (SleepShortKeyEvent, 4);
				SetEvent (SleepShortKeyEvent);	
#endif

// 4.3inch的屏有两个BUTTON，一个PWR_KEY, 一个MENU_KEY
// 3.5INCH的只有一个PWR_KEY, 复用MENU_KEY功能
#if(LCD_MODULE_TYPE==BSP_LCD_BYD_43INCH_480X272) || \
   (LCD_MODULE_TYPE==BSP_LCD_INNOLUX_43)

#elif(LCD_MODULE_TYPE==BSP_LCD_YASSY_43INCH_480X272)

#else
				Send_ShortkeyEvent();
#endif

			}					
#endif			
//=============================
				}
#if TICKTEST
/*

				v_pRTCregs->TICNT0 = (1<<7);
				v_pRTCregs->TICNT2 = 0xa;
				v_pRTCregs->RTCCON = (5<<5)|(1<<0);*/
#endif

			}
			else///APR if ( pPWR->State == 0x2 )
			{
#if TICKTEST
/*
				v_pRTCregs->TICNT0 = (1<<7);
				v_pRTCregs->TICNT2 = 0xa;
				v_pRTCregs->RTCCON = (5<<5)|(1<<0);*/
#endif				
				SetSystemPowerState( NULL, POWER_STATE_ON, POWER_FORCE );
				pPWR->State = RUN;
 //[david.modify] 2008-06-24 15:26
				DPSTR("POWER_STATE_ON");			

//[david.modify] 2008-08-07 14:28 加上如下这句是为了告诉AP
// 让AP收到此消息不发出SD卡插拔声音
//=========================================
//				SendPwrEvent(WAKEUP_EVENT_ID);
//=========================================

#if 1
	DPNOK(g_stPWRParam.u32BackLightDelay);
	Sleep(g_stPWRParam.u32BackLightDelay);
#endif	

#if 0		// 背光开启将在BACKLIGHT驱动里完成	
	g_stGPIOInfo[5].u32Stat = 1;	
	SetGPIOInfo(&g_stGPIOInfo[5], v_pIOPregs);

	g_stGPIOInfo[6].u32Stat = 1;	
	SetGPIOInfo(&g_stGPIOInfo[6], v_pIOPregs);
#endif	

#if 0
#define VK_UNSIGNED_0xE5  0xE5
//加如下两行去解有时唤醒时，背光不亮，要点下TOUCH或按下键才OK
//=============
	keybd_event((BYTE)VK_UNSIGNED_0xE5, 0, KEYEVENTF_SILENT, 0);		
	keybd_event ((BYTE)VK_UNSIGNED_0xE5, 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);					
//=============
#endif


			}
        } _except(EXCEPTION_EXECUTE_HANDLER) {
            RETAILMSG(1,(TEXT("!!! PWR_IST EXCEPTION: 0x%X !!!\r\n"), GetExceptionCode() ));
        }
	}
}


static DWORD 
RESET_IST()
{
    while (1) {
        __try {
            WaitForSingleObject(g_ResetEvent, INFINITE);
			RETAILMSG(1,(TEXT("\r\nReset Button pressed \r\n")));
			InterruptDone(g_ResetButtonSysIntr);
			KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);

        } _except(EXCEPTION_EXECUTE_HANDLER) {
            RETAILMSG(1,(TEXT("!!! RESET_IST EXCEPTION: 0x%X !!!\r\n"), GetExceptionCode() ));
        }
	}
	
	return 0;
}


#if MENU_KEY_BTN

static DWORD 
MENU_IST()
{
    while (1) {
        __try {
            WaitForSingleObject(g_MenuEvent, INFINITE);
			RETAILMSG(1,(TEXT("\r\nMENU Button pressed \r\n")));
			InterruptDone(g_MenuButtonSysIntr);
			DPSTR("Send_ShortkeyEvent+");
			Send_ShortkeyEvent();
			DPSTR("Send_ShortkeyEvent-");			

        } _except(EXCEPTION_EXECUTE_HANDLER) {
            RETAILMSG(1,(TEXT("!!! MENU_IST EXCEPTION: 0x%X !!!\r\n"), GetExceptionCode() ));
        }
	}
	
	return 0;
}
#endif

/* ++

 Get/Set the PCF RTC.
 
 One neat PCF feature is the stand alone RTC.
 You could power the ARM core to full off and maintin the RTC & ALARM on the PCF.
 This is not a required feature for PPC2002 release, but OEMs are 
 free to add this as desired. You should sync the ARM & PCF RTC
 and update as appropriate. If you choose to implement this it would
 be good to power as much of the PCF off as possible to maintin it's RTC
 for much longer time.

-- */
BOOL 
PWR_GetRealTime(
    PPWR_CONTEXT pPWR,
    LPSYSTEMTIME lpst
    )
{
    return TRUE;
}


BOOL
PWR_SetRealTime(
    PPWR_CONTEXT pPWR,
    LPSYSTEMTIME lpst
    ) 
{
    return TRUE;
}


BOOL
HW_IOControl(
    PPWR_CONTEXT pPWR,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
   )
{
	return FALSE;
}



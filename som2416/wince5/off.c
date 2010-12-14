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
// NOTE: stubs are being used - this isn't done

#include <windows.h>
#include <bsp.h>
#include <s3c2450.h>
#if (BSP_TYPE == BSP_SMDK2443)
#include <S3C2450REF_GPIO.h> //this is renamed from #include <S3C2443REF_GPIO.h>, Same File
#elif (BSP_TYPE == BSP_SMDK2450)
#endif

#ifdef DVS_EN
#include "bsp_cfg.h"
	extern volatile int CurrentState;
	extern void DVS_ON(void);
	extern void DVS_OFF(void);
	extern void ChangeVoltage(int, int *);
	extern	void HCLK_RECOVERYUP();
	extern	void Max1718_Init(int);
#endif
 //[david.modify] 2008-07-04 11:37
#include <xllp_gpio_david.h> 
#include <dbgmsg_david.h>
 volatile BSP_ARGS *g_pBspArgs;
volatile S3C2450_INTR_REG *g_pIntrRegs;
volatile S3C2450_IOPORT_REG *g_pIOPort;
volatile S3C2450_RTC_REG *g_pRTCRegs;
volatile S3C2450_NAND_REG *g_pNandRegs;
volatile S3C2450_CLKPWR_REG *g_pCLKPWRRegs;
volatile S3C2450_PWM_REG *g_pPWMRegs;
//volatile S3C2450_MEMCTRL_REG *g_pMEMCTRLReg;
volatile S3C2450_LCD_REG *g_pLCDReg;
volatile S3C2450_ADC_REG *g_pADCReg;

 //[david.modify] 2008-07-04 11:48


#define GPG_ALT_FUNC_EINT 2
#define GPG_ALT_FUNC_OUT 1
stGPIOInfo g_stGPIOInfo[]={
	{ PWR_KEY,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},
	{ KEY6,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},	
	{ KEY3,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},	
	{ KEY4,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},		
	{ GLED,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
	};
DWORD g_dwLastWakeUpSource;
volatile BOOL g_bAcAdapterInBeforeSleep = FALSE;
extern void ConfigSleepGPIO(void);
//------------------------------------------------------------------------------
//
// Function:     OEMPowerOff
//
// Description:  Called when the system is to transition to it's lowest
//               power mode (off)
//
 //[david.modify] 2008-06-19 15:57
 //================
#ifndef LCD_DELAY_1MS
#define LCD_DELAY_1MS	180000
#endif
 static void delayLoop(int count) 
{ 
	volatile int j; 
	for(j = 0; j < count; j++)  ; 
}
 //================

 //[david.modify] 2008-07-04 11:18
 //==========================


INT IsChargerIn()
{
	int i=0;
	int nState = 0;
	stGPIOInfo stGPIOInfo;
	
	if(NULL==g_pIOPort){
//		v_p2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);	
		DPN(0);	
		return 0;
	}


	g_pIOPort->GPFCON =0x55555566;   	// ½«USBÉèÖÃ³ÉÖÐ¶ÏGPF2

	g_pIOPort->GPFDAT = 0x0;
	g_pIOPort->GPFUDP = 0x0;		// disable pull-up/down		




	// get	
	EPRINT(L"==USB IO==\r\n");
	stGPIOInfo.u32PinNo = USB_DET;
//	Sleep(1);
	delayLoop(1*LCD_DELAY_1MS);
	GetGPIOInfo(&stGPIOInfo, g_pIOPort);
	EPRINT(L"USB_DET=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	if(stGPIOInfo.u32Stat) {	//¸ßµçÆ½
		nState = 0;
	}else{
		nState = 1;			// µÍµç,±íÊ¾¼ì²âµ½ÓÐUSB »òadapter
	}
	
#if 0			
	stGPIOInfo.u32PinNo = CH_nFULL;
	GetGPIOInfo(&stGPIOInfo, g_pIOPort);
	EPRINT(L"CH_nFULL=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = CH_CON;
	GetGPIOInfo(&stGPIOInfo, g_pIOPort);
	EPRINT(L"CH_nFULL=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = PWREN_USB;
	GetGPIOInfo(&stGPIOInfo, g_pIOPort);
	EPRINT(L"PWREN_USB=%d: [%d %d %d] \r\n",
		 stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
#endif
	DPNOK(nState);
	if(nState) 
		DPSTR("CHARGER IN!");
	else 
		DPSTR("CHARGER OUT!");
	
	return nState;
}


 
void ConfigSleepGPIO_david(void)
{
	DPNOK(0);

//[david.modify] 2008-06-04 18:26


// GPA 
//==========================================
	DPNOK(g_pIOPort->GPACON);
	DPNOK(g_pIOPort->GPADAT);
	
// s805g  ÓÐÓÃµ½GPA
#if 0
#define GPS_BOOT GPA13	//BOOT£¬1£ºÉÕÂ¼Èí¼þ£»0£ºÕý³£¹¤×÷	
#define GPS_RESET GPA15	//GPS ¸´Î»ÐÅºÅ£¬µÍÓÐÐ§£¬ÏÈµÍ200ms£¬ÔÙÀ­¸ß
#define SD_PWREN GPA14	//SD¿¨µçÔ´Ê¹ÄÜ£¬1£º¿ªµçÔ´£»0£º¹ØµçÔ´
#define nFCE2 GPA12		//???±£Áô¸øË«DIE FLASH, Æ¬Ñ¡
#endif

	g_pIOPort->GPACON = 0xFFFFFFFF;
	g_pIOPort->GPADAT= 0x1FFFF;	
//¹Ø±ÕSD¿¨µçÔ´
	g_pIOPort->GPACON&=~( (1<<13)|(1<<14));
	g_pIOPort->GPADAT&=~( (1<<13)|(1<<14));



	DPNOK(g_pIOPort->GPACON);
	DPNOK(g_pIOPort->GPADAT);

// GPB 	
//==========================================
	DPNOK(g_pIOPort->GPBCON);
	DPNOK(g_pIOPort->GPBDAT);
	DPNOK(g_pIOPort->GPBUDP);	

#if 0
	#define PWREN_USB GPB4		//USBµçÔ´¹©µç0-¹Øµç
	#define CPUCORE_CON GPB1	// 0-1.0V , 1-±íÊ¾1.3v (Õý³£Ó¦¸ÃVCORE=1.2V) 
	#define LCDRST GPB9 			// ¸´Î»
	#define LCDCS GPB5 			//SPIÆ¬Ñ¡	LCDÍ¸¹ýSPIËÍ²ÎÊý³õÊ¼»¯LCD
	#define LCDSDA GPB10 		//SPI DATA	
//[david.modify] 2008-06-04 11:39
// ±³¹âIO´ÓGPB3-->GPB2,ÒòÎªTIMER3±»TOUCHÕ¼ÓÃ 
// 28khz pwm frequency
//#define BACKLIGHT_PWM GPB3
#define BACKLIGHT_PWM GPB2
#define BACKLIGHT_PWM_OLD GPB3
	#define GLED GPB0			//¹¤×÷Ö¸Ê¾µÆ£¬0£ºµÆÁÁ£»1£ºµÆÃð¡£	
#endif
	g_pIOPort->GPBCON = 0x155555;   	// B   0101 0101 0101 0101 0101
	g_pIOPort->GPBDAT = 0x3;
	g_pIOPort->GPBUDP = 0x0;		// disable pull-up/down

	DPNOK(g_pIOPort->GPBCON);
	DPNOK(g_pIOPort->GPBDAT);
	DPNOK(g_pIOPort->GPBUDP);	
	
// GPC 
//==========================================
	DPNOK(g_pIOPort->GPCCON);
	DPNOK(g_pIOPort->GPCDAT);
	DPNOK(g_pIOPort->GPCUDP);	

#if 0
#define LCDHSYNC GPC4	//ÐÐÊ±ÖÓ
#define LCDVSYNC GPC2 	//Ö¡Ê±ÖÓ
#define LCDPCLK GPC1 	// PIX Ê±ÖÓ
#define LCDDE GPC3		// CHIP ENABLE£	
#define LCD_B0 GPC11 
#define LCD_B1 GPC12 
#define LCD_B2 GPC13 
#define LCD_B3 GPC14 
#define LCD_B4 GPC15 
#define ULC_RST GPC0		//¸´Î»Íâ½ÓÉè±¸
#endif
	g_pIOPort->GPCCON = 0x55555555;   	
	g_pIOPort->GPCDAT = 0x0;
	g_pIOPort->GPCUDP = 0x0;		// disable pull-up/down

	DPNOK(g_pIOPort->GPCCON);
	DPNOK(g_pIOPort->GPCDAT);
	DPNOK(g_pIOPort->GPCUDP);

// GPD 
//==========================================
	DPNOK(g_pIOPort->GPDCON);
	DPNOK(g_pIOPort->GPDDAT);
	DPNOK(g_pIOPort->GPDUDP);
#if 0
#define LCD_G0 GPD2 
#define LCD_G1 GPD3 
#define LCD_G2 GPD4 
#define LCD_G3 GPD5 
#define LCD_G4 GPD6 
#define LCD_G5 GPD7 
#define LCD_R0 GPD11 
#define LCD_R1 GPD12 
#define LCD_R2 GPD13 
#define LCD_R3 GPD14 
#define LCD_R4 GPD15

#endif
	g_pIOPort->GPDCON = 0x55555555;   	
	g_pIOPort->GPDDAT = 0x0;
	g_pIOPort->GPDUDP = 0x0;		// disable pull-up/down	

	DPNOK(g_pIOPort->GPDCON);
	DPNOK(g_pIOPort->GPDDAT);
	DPNOK(g_pIOPort->GPDUDP);
	
// GPE 
//==========================================
	DPNOK(g_pIOPort->GPECON);
	DPNOK(g_pIOPort->GPEDAT);
	DPNOK(g_pIOPort->GPEUDP);
#if 0
#define I2SLRCK GPE0 	// Êý×ÖÒôÆµÍ¬²½ÐÅºÅ
#define I2SSCLK GPE1	//Êý×ÖÒôÆµ´®ÐÐCLK
#define I2SSDI GPE3	// ´®ÐÐADCÊý¾ÝÊäÈë
#define I2SSDO GPE4	//´®ÐÐDACÊý¾ÝÊä³ö
#define I2CSCL GPE14	// I2C CLK
#define I2CSDA GPE15	// I2C DATA

#define SD0_DATA0 GPE7
#define SD0_DATA1 GPE8
#define SD0_DATA2 GPE9
#define SD0_DATA3 GPE10
#define SDCLK GPE5
#define SDCMD GPE6

#define LCDCLK GPE13	// ÒòÎªJTAGÓÃµ½GPB6ÁË

	g_pIOPort->GPECON = 0x55555555;   	
	g_pIOPort->GPEDAT = 0x0;
	g_pIOPort->GPEUDP = 0x0;		// disable pull-up/down		
#endif
	DPNOK(g_pIOPort->GPECON);
	DPNOK(g_pIOPort->GPEDAT);
	DPNOK(g_pIOPort->GPEUDP);	
	
// GPF
//==========================================
	DPNOK(g_pIOPort->GPFCON);
	DPNOK(g_pIOPort->GPFDAT);
	DPNOK(g_pIOPort->GPFUDP);
#if 0
#define PWR_KEY GPF0		//µçÔ´¼ü
#define SD0_NCD GPF1 	//SD¿¨¼ì²â£¬0£ºÓÐ¿¨£»1£ºÃ»¿¨
#define USB_DET GPF2		//USB¼ì²â in
#endif
//	g_pIOPort->GPFCON = 0x55555556;   	
	g_pIOPort->GPFCON =0x55555566;   	// ½«USBÉèÖÃ³ÉÖÐ¶ÏGPF2

	g_pIOPort->GPFDAT = 0x0;
	g_pIOPort->GPFUDP = 0x0;		// disable pull-up/down		

	DPNOK(g_pIOPort->GPFCON);
	DPNOK(g_pIOPort->GPFDAT);
	DPNOK(g_pIOPort->GPFUDP);


 //[david.modify] 2008-07-30 14:20
 // USB_DET ÎªGPF2 £¨EINT2£©£¬ÓÃÀ´¼ì²âUSBÊÇ·ñ²åÈë£¬»½ÐÑSYSTEM
 //===================================================
// 	g_pIOPort->EXTINT0 = (g_pIOPort->EXTINT0 & ~(0xf<<8)) | (0x0 << 11)|(0x2 << 8);		/* Configure EINT2 as Falling Edge Mode				*/	
 //[david.modify] 2008-08-19 01:08 ÒªÇó±£Ö¤Ë«Ïò»½ÐÑÏµÍ³
 	g_pIOPort->EXTINT0 = (g_pIOPort->EXTINT0 & ~(0xf<<8)) | (0x0 << 11)|(0x6 << 8);		/* Configure EINT2 as both Edge Mode				*/	
	
	g_pIOPort->GPFCON  &= ~(0x3 << 4);		/* Set EINT2(GPF2) as EINT2							*/
	g_pIOPort->GPFCON  |=  (0x2 << 4);
	
	g_pIOPort->GPFUDP&= ~(0x3 << 4);		// Pull up down disable
	g_pIOPort->GPFUDP|=  (0x0 << 4);
	DPNOK(g_pIOPort->EXTINT0);
 //==================================================

	
// GPG
//==========================================
	DPNOK(g_pIOPort->GPGCON);
	DPNOK(g_pIOPort->GPGDAT);
	DPNOK(g_pIOPort->GPGUDP);
#if 1

#if 0
#define CH_nFULL GPG7	//³äµçÄ£Ê½¿ØÖÆ in
#define CH_CON GPG6		// ³äµçÄ£Ê½¿ØÖÆ OUT, Ë¯ÃßL
#define LCD_PWREN GPG4	//LCDÆÁµçÔ´Ê¹ÄÜ
#define KEY6 GPG2		//ÒôÁ¿+		//ent10
#define KEY3	GPG0		//ÒôÁ¿-		// ent8
#define KEY4 GPG1		//È·ÈÏ¼ü		//ent9
#define GPS_POWER_EN GPG5
#define TMC_ANT_DET GPG3
#endif

// ¼ÓÏÂÃæÕâ¸ö´úÂë,»áÔì³ÉÒôÁ¿OSDµ¯³ö,¹ÖÊÂ;¶øÇÒµçÁ÷´ó
//============================================
	g_pIOPort->GPGCON = 0x5555556a;   	//Èý¸öKEYÄÜ»½ÐÑÏµÍ³
//	g_pIOPort->GPGCON = 0x55555555;   	//ÉèÖÃKEYÎªIO¿Ú
	g_pIOPort->GPGDAT = 0x0;
	g_pIOPort->GPGUDP = 0x0;		// disable pull-up/down		


//[david.modify] 2008-08-01 11:32
// ½ûÖ¹°´¼ü»½ÐÑÏµÍ³
//========================================
	//EINT8 , EINT9, EINT10 , FILTER DISABLE
 	g_pIOPort->EXTINT1 = (g_pIOPort->EXTINT1 & ~(0xf<<0)) | (0x1 << 3)|(0x2 << 8);		
 	g_pIOPort->EXTINT1 = (g_pIOPort->EXTINT1 & ~(0xf<<4)) | (0x1 << 7)|(0x2 << 8);		
 	g_pIOPort->EXTINT1 = (g_pIOPort->EXTINT1 & ~(0xf<<8)) | (0x1 << 11)|(0x2 << 8);			

	// EINT8 - INPUT
	g_pIOPort->GPGCON  &= ~(0x3 << 0);		
	g_pIOPort->GPGCON  |=  (0x0 << 0);	// 0-INPUT

	// EINT9 - INPUT	
	g_pIOPort->GPGCON  &= ~(0x3 << 2);		
	g_pIOPort->GPGCON  |=  (0x0 << 2);	// 0-INPUT

	// EINT10 - INPUT	
	g_pIOPort->GPGCON  &= ~(0x3 << 4);		
	g_pIOPort->GPGCON  |=  (0x0 << 4);	// 0-INPUT
	
	DPNOK(g_pIOPort->EXTINT1);

	// ¹Ø±Õ°´¼ü²úÉúÖÐ¶Ï,»½ÐÑÏµÍ³
	g_pIOPort->EINTMASK|= (1<<8)|(1<<9)|(1<<10);

//========================================




	
//============================================	
#endif

	DPNOK(g_pIOPort->GPGCON);
	DPNOK(g_pIOPort->GPGDAT);
	DPNOK(g_pIOPort->GPGUDP);
	
// GPH
//==========================================
	DPNOK(g_pIOPort->GPHCON);
	DPNOK(g_pIOPort->GPHDAT);
	DPNOK(g_pIOPort->GPHUDP);
#if 0
#define GPS_RXD GPH5
#define GPS_TXD GPH4
#define RTS0 GPH9
#define CTS0 GPH8
#define RXD0 GPH0
#define TXD0 GPH1
#define TMC_ON GPH12	//ÄÚÖÃTMCµçÔ´Ê¹ÄÜ£¬1£º¿ªµçÔ´£»0£º¹ØµçÔ´¡£ÔÝÃ»ÓÃ
#endif
#if 1
// GPHÉæ¼°ºÜ¶à´®¿Ú,ÔÝ²»Éè
	g_pIOPort->GPHCON = 0x55555555;   	
	g_pIOPort->GPHDAT = 0x0;
	g_pIOPort->GPHUDP = 0x0;		// disable pull-up/down	
#endif	

	DPNOK(g_pIOPort->GPHCON);
	DPNOK(g_pIOPort->GPHDAT);
	DPNOK(g_pIOPort->GPHUDP);
// GPK
//==========================================
	DPNOK(g_pIOPort->GPKCON);
	DPNOK(g_pIOPort->GPKDAT);
	DPNOK(g_pIOPort->GPKUDP);
#if 0

#endif
#if 0
// GPKÎªÊý¾ÝÏß,Ò²²»Éè
	g_pIOPort->GPKCON = 0x55555555;   	
	g_pIOPort->GPKDAT = 0x0;
	g_pIOPort->GPKUDP = 0x0;		// disable pull-up/down	
#endif	

	DPNOK(g_pIOPort->GPKCON);
	DPNOK(g_pIOPort->GPKDAT);
	DPNOK(g_pIOPort->GPKUDP);	

// GPL
//==========================================
	DPNOK(g_pIOPort->GPLCON);
	DPNOK(g_pIOPort->GPLDAT);
	DPNOK(g_pIOPort->GPLUDP);
#if 1


	g_pIOPort->GPLCON = 0x55555555;   	
	g_pIOPort->GPLDAT = 0x0;
	g_pIOPort->GPLUDP = 0x0;		// disable pull-up/down		
#endif
	DPNOK(g_pIOPort->GPLCON);
	DPNOK(g_pIOPort->GPLDAT);
	DPNOK(g_pIOPort->GPLUDP);
	
// GPM
//==========================================
	DPNOK(g_pIOPort->GPMCON);
	DPNOK(g_pIOPort->GPMDAT);
	DPNOK(g_pIOPort->GPMUDP);
#if 0

#endif
#if 1
// GPMÔÝ²»Éè
	g_pIOPort->GPMCON = 0x55555555;   	
	g_pIOPort->GPMDAT = 0x0;
	g_pIOPort->GPMUDP = 0x0;		// disable pull-up/down			
#endif	

	DPNOK(g_pIOPort->GPMCON);
	DPNOK(g_pIOPort->GPMDAT);
	DPNOK(g_pIOPort->GPMUDP);

 //[david.modify] 2008-08-04 12:33
 // ÓÃÓÚ¼ì²âË¯ÂÇÇ°CHARGERÊÇ²»ÊÇÒÑ¾­²åÈëµÄ
 //=================================
#if 0 
     if (IsChargerIn())
    {
        g_bAcAdapterInBeforeSleep = TRUE;
    }
    else
    {
        g_bAcAdapterInBeforeSleep = FALSE;
    }
#endif
//======================================	

	

}


void ConfigMiscReg_david(void)
{
	// Setting interrupt mask can not protect waking up from sleep by RTC or EINT
	// so that disabling wake-up from them should be done by their own control registers.
//	g_pIntrRegs->INTMSK1 = 0xFFFFFFFF;

	 //[david.modify] 2008-08-01 10:28
	 // ½«EINT0£¬EINT2Ê¹ÄÜ
	 // EINT0 - PWR BUTTON
	 // EINT2 - USB INSERT
	 // ¿ÉÊÇÔÚOALCPUPowerOff (startup.s)ÖÐÓÐÉèÕâ¸öREGSITER
	g_pIntrRegs->INTMSK1 = 0xFFFFFFFa; 


	// select wake-up sources by setting as input or EINT
	// disable button wake-up
/*
//KEYPAD
//==============
//°´¼üµÄÄ¬ÈÏ×´Ì¬Îª¸ß£¬µ±¼ü°´ÏÂÊ±ÎªµÍ¡£
#define PWR_KEY GPF0	//µçÔ´¼ü
#define KEY6 GPG2		//ÒôÁ¿+		//ent10
#define KEY3	GPG0		//ÒôÁ¿-		// ent8
#define KEY4 GPG1		//È·ÈÏ¼ü		//ent9
*/
	 //[david.modify] 2008-07-04 11:59
	DPNOK(g_pIOPort->EINTMASK);	 
	 g_pBspArgs->uninit_misc.dwWakeUpLockBtn = 1;
	if( g_pBspArgs->uninit_misc.dwWakeUpLockBtn )
	{
#if 0		
		// ¹Ø±Õ°´¼ü²úÉúÖÐ¶Ï,»½ÐÑÏµÍ³
		g_pIOPort->EINTMASK|= (1<<8)|(1<<9)|(1<<10);
	
		g_stGPIOInfo[1].u32AltFunc = ALT_FUNC_OUT;
		g_stGPIOInfo[2].u32AltFunc = ALT_FUNC_OUT;
		g_stGPIOInfo[3].u32AltFunc = ALT_FUNC_OUT;		
		SetGPIOInfo(&g_stGPIOInfo[1],g_pIOPort);
		SetGPIOInfo(&g_stGPIOInfo[2],g_pIOPort);
		SetGPIOInfo(&g_stGPIOInfo[3],g_pIOPort);		
#endif		
	}
	// enable button wake-up
	else
	{	
//		g_pIOPort->EINTMASK&= ~((1<<8)|(1<<9)|(1<<10));
	}

	DPNOK(g_pIOPort->EINTMASK);

 //[david.modify] 2008-07-01 14:16
 	DPNOK(g_pRTCRegs->RTCCON);
	DPNOK(g_pADCReg->ADCCON);
	DPNOK(g_pIOPort->MISCCR);	

	g_pRTCRegs->RTCCON = 0x0;   // R/W disable, 1/32768, Normal(merge), No reset
	g_pADCReg->ADCCON |= (1<<2);		// ADC StanbyMode
    	g_pIOPort->MISCCR |= (1<<12); //USB port0 = suspend

	DPNOK(g_pRTCRegs->RTCCON);
	DPNOK(g_pADCReg->ADCCON);
	DPNOK(g_pIOPort->MISCCR);		
	

}

 //[david.modify] 2008-07-08 14:58
 // È¥¼ì²âPWRBUTTONÊÇ·ñ³¤°´³¬¹ýÁËÒ»¶¨Ê±¼ä
 // 0--°´ÏÂ1-ËÉ¿ª
void WaitMS (DWORD dwMS)
{
	volatile DWORD dCount,dwIndex;

	for (dwIndex=0;dwIndex<dwMS;dwIndex++) {
		for (dCount=0;dCount<100000L;dCount++);
	}
}


 
int OALBspWakeupCheckPwrBtn(int nTime)
{
	UINT32 u32cnt = 0;

	while(1) {
		// µÃµ½GPIO×´Ì¬		
		GetGPIOInfo(&g_stGPIOInfo[0],g_pIOPort);
//		WaitMS(1);	//µÈ1MS
		if(1==g_stGPIOInfo[0].u32Stat)	//Èç¹û¸ß£¬ÔòÍË³ö
		{
			return FALSE;
		}else
		{
			u32cnt++;
			 //[david.modify] 2008-07-08 16:24
			 //
			if(u32cnt>nTime)
				break;
		}
	}

	return TRUE;
 }

BOOL OALBspWakeupCheckACAdapter()
{
    if (IsChargerIn() && !g_bAcAdapterInBeforeSleep )
        return TRUE;
    else
        return FALSE;
}


BOOL OALBspWakeupCheck()
{
    BOOL bRet = FALSE;

   if ( OALBspWakeupCheckACAdapter() )
    {
        g_dwLastWakeUpSource = SYSINTR_UNDEFINED;
        bRet = TRUE;
        goto EXIT;
    }

//    DPNOK(g_pBspArgs->u32WakeupHoldTime);	
    if ( OALBspWakeupCheckPwrBtn(g_pBspArgs->u32WakeupHoldTime) )
    {
//        g_dwLastWakeUpSource = g_oalSysInfo.pwrbtnInfo.dwSysIntr;
        bRet = TRUE;
        goto EXIT;
    }



	
    
EXIT:

    return bRet;
}




//===============================
 //[david.modify] 2008-07-04 16:55
extern void InitDisplay2(void);
void OEMPowerOff()
{
#if (BSP_TYPE == BSP_SMDK2443)
	static UINT32 saveArea[85];
#elif (BSP_TYPE == BSP_SMDK2450)
	static UINT32 saveArea[90];
#endif
	volatile S3C2450_INTR_REG *pIntr = (S3C2450_INTR_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);
	volatile S3C2450_IOPORT_REG *pIOPort = (S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2450_LCD_REG *pLCD = (S3C2450_LCD_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
	volatile S3C2450_CLKPWR_REG *pCLKPWR = (S3C2450_CLKPWR_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);
	// First do platform specific actions
	//BSPPowerOff();
	RETAILMSG(1,(TEXT("\r\n --OEMPowerOff--\r\n")));

	g_pLCDReg = (volatile S3C2450_LCD_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
//	g_pMEMCTRLReg = (volatile S3C2450_MEMCTRL_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_MEMCTRL, FALSE);
	g_pPWMRegs = (volatile S3C2450_PWM_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_PWM, FALSE);
	g_pCLKPWRRegs = (volatile S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);
	g_pNandRegs = (volatile S3C2450_NAND_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_NAND, FALSE);
	g_pRTCRegs = (volatile S3C2450_RTC_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_RTC, FALSE);
	g_pIOPort = (volatile S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	g_pIntrRegs = (volatile S3C2450_INTR_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);
	g_pBspArgs = (volatile BSP_ARGS *)OALPAtoVA(IMAGE_SHARE_ARGS_PA_START, FALSE);
	g_pADCReg = (S3C2450_ADC_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_ADC, FALSE);


	
#ifdef DVS_EN
	RETAILMSG(1,(TEXT("DVS OFF\r\n")));		
	{
		int voltage_set[2] = HIGH_V_SET;
		ChangeVoltage(ARM_INT_VDD, voltage_set);
	}
	HCLK_RECOVERYUP();
	DVS_OFF();
	g_oalIoCtlClockSpeed = S3C2450_FCLK;
	CurrentState = Active;	

#endif //DVS_EN  

	// Then save system registers


#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1	
	pIOPort->GPACDL = 0xFFFF;
	pIOPort->GPACDH = 0x1FFFF;	
#else
	saveArea[0]  = INPORT32(&pIOPort->GPACON);
	saveArea[1]  = INPORT32(&pIOPort->GPADAT);
#endif
#elif (BSP_TYPE == BSP_SMDK2450)

#if 0
//[david.modify] 2008-06-04 18:26
// s805g  ÓÐÓÃµ½GPA
//=========================
	pIOPort->GPACON = 0xFFFFFFFF;
	pIOPort->GPADAT= 0x1FFFF;	
//ÔÚ´Ë´¦¹Ø±ÕGPA13 
//GPA14
	pIOPort->GPACON&=~( (1<<13)|(1<<14));
	pIOPort->GPADAT&=~( (1<<13)|(1<<14));
	
//=========================	
#endif
#endif

	saveArea[2]  = INPORT32(&pIOPort->GPBCON);
	saveArea[3]  = INPORT32(&pIOPort->GPBDAT);
	saveArea[4]  = INPORT32(&pIOPort->GPBUDP);
	saveArea[5]  = INPORT32(&pIOPort->GPCCON);
	saveArea[6]  = INPORT32(&pIOPort->GPCDAT);
	saveArea[7]  = INPORT32(&pIOPort->GPCUDP);
	saveArea[8]  = INPORT32(&pIOPort->GPDCON);
	saveArea[9]  = INPORT32(&pIOPort->GPDDAT);
	saveArea[10] = INPORT32(&pIOPort->GPDUDP);
	saveArea[11] = INPORT32(&pIOPort->GPECON);
	saveArea[12] = INPORT32(&pIOPort->GPEDAT);
	saveArea[13] = INPORT32(&pIOPort->GPEUDP);
	saveArea[14] = INPORT32(&pIOPort->GPFCON);
	saveArea[15] = INPORT32(&pIOPort->GPFDAT);
	saveArea[16] = INPORT32(&pIOPort->GPFUDP);
	saveArea[17] = INPORT32(&pIOPort->GPGCON);
	saveArea[18] = INPORT32(&pIOPort->GPGDAT);
	saveArea[19] = INPORT32(&pIOPort->GPGUDP);
	saveArea[20] = INPORT32(&pIOPort->GPHCON);
	saveArea[21] = INPORT32(&pIOPort->GPHDAT);
	saveArea[22] = INPORT32(&pIOPort->GPHUDP);
	saveArea[23] = INPORT32(&pIOPort->GPJCON);
	saveArea[24] = INPORT32(&pIOPort->GPJDAT);
	saveArea[25] = INPORT32(&pIOPort->GPJUDP);

#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1
	saveArea[26] = INPORT32(&pIOPort->GPKCON);
	saveArea[27] = INPORT32(&pIOPort->GPKDAT);
	saveArea[28] = INPORT32(&pIOPort->DATAPDEN);
#else
	saveArea[26] = INPORT32(&pIOPort->GPKCON);
	saveArea[27] = INPORT32(&pIOPort->GPKDAT);
	saveArea[28] = INPORT32(&pIOPort->GPKUDP);
#endif
#elif (BSP_TYPE == BSP_SMDK2450)
	saveArea[26] = INPORT32(&pIOPort->GPKCON);
	saveArea[27] = INPORT32(&pIOPort->GPKDAT);
	saveArea[28] = INPORT32(&pIOPort->GPKUDP);
#endif	

	saveArea[29] = INPORT32(&pIOPort->GPLCON);
	saveArea[30] = INPORT32(&pIOPort->GPLDAT);
	saveArea[31] = INPORT32(&pIOPort->GPLUDP);
	saveArea[32] = INPORT32(&pIOPort->GPMCON);
	saveArea[33] = INPORT32(&pIOPort->GPMDAT);
	saveArea[34] = INPORT32(&pIOPort->GPMUDP);	

	saveArea[35] = INPORT32(&pIOPort->MISCCR);
	saveArea[36] = INPORT32(&pIOPort->DCLKCON);
#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1		
	saveArea[37] = READEXTINT0(pIOPort->EXTINT0);
	saveArea[38] = READEXTINT1(pIOPort->EXTINT1);
	saveArea[39] = READEXTINT2(pIOPort->EXTINT2);
#else
	saveArea[37] = INPORT32(&pIOPort->EXTINT0);
	saveArea[38] = INPORT32(&pIOPort->EXTINT1);
	saveArea[39] = INPORT32(&pIOPort->EXTINT2);
#endif	
#elif (BSP_TYPE == BSP_SMDK2450)
	saveArea[37] = INPORT32(&pIOPort->EXTINT0);
	saveArea[38] = INPORT32(&pIOPort->EXTINT1);
	saveArea[39] = INPORT32(&pIOPort->EXTINT2);
#endif	
	saveArea[40] = INPORT32(&pIOPort->EINTFLT0);
	saveArea[41] = INPORT32(&pIOPort->EINTFLT1);
	saveArea[42] = INPORT32(&pIOPort->EINTFLT2);
	saveArea[43] = INPORT32(&pIOPort->EINTFLT3);
	saveArea[44] = INPORT32(&pIOPort->EINTMASK);

#if (BSP_TYPE == BSP_SMDK2443)
	saveArea[45] = INPORT32(&pIntr->INTMOD);
	saveArea[46] = INPORT32(&pIntr->INTMSK);
#elif (BSP_TYPE == BSP_SMDK2450)
	saveArea[45] = INPORT32(&pIntr->INTMOD1);
	saveArea[46] = INPORT32(&pIntr->INTMSK1);
#endif

	saveArea[47] = INPORT32(&pIntr->INTSUBMSK);

	saveArea[48] = INPORT32(&pLCD->VIDCON0);
	saveArea[49] = INPORT32(&pLCD->VIDCON1);
	saveArea[50] = INPORT32(&pLCD->VIDTCON0);
	saveArea[51] = INPORT32(&pLCD->VIDTCON1);
	saveArea[52] = INPORT32(&pLCD->VIDTCON2);
	saveArea[53] = INPORT32(&pLCD->WINCON0);
	saveArea[54] = INPORT32(&pLCD->WINCON1);
	saveArea[55] = INPORT32(&pLCD->VIDOSD0A);
	saveArea[56] = INPORT32(&pLCD->VIDOSD0B);
	saveArea[57] = INPORT32(&pLCD->VIDOSD0C);
	saveArea[58] = INPORT32(&pLCD->VIDOSD1A);
	saveArea[59] = INPORT32(&pLCD->VIDOSD1B);
	saveArea[60] = INPORT32(&pLCD->VIDOSD1C);
	saveArea[61] = INPORT32(&pLCD->VIDW00ADD0B0);
	saveArea[62] = INPORT32(&pLCD->VIDW00ADD0B1);
	saveArea[63] = INPORT32(&pLCD->VIDW01ADD0);
	saveArea[64] = INPORT32(&pLCD->VIDW00ADD1B0);
	saveArea[65] = INPORT32(&pLCD->VIDW00ADD1B1);
	saveArea[66] = INPORT32(&pLCD->VIDW01ADD1);
	saveArea[67] = INPORT32(&pLCD->VIDW00ADD2B0);
	saveArea[68] = INPORT32(&pLCD->VIDW00ADD2B1);
	saveArea[69] = INPORT32(&pLCD->VIDW01ADD2);
	saveArea[70] = INPORT32(&pLCD->VIDINTCON);	
	saveArea[71] = INPORT32(&pLCD->W1KEYCON0);	
	saveArea[72] = INPORT32(&pLCD->W1KEYCON1);	
	saveArea[73] = INPORT32(&pLCD->W2KEYCON0);	
	saveArea[74] = INPORT32(&pLCD->W2KEYCON1);	
	saveArea[75] = INPORT32(&pLCD->W3KEYCON0);	
	saveArea[76] = INPORT32(&pLCD->W3KEYCON1);	
	saveArea[77] = INPORT32(&pLCD->W4KEYCON0);	
	saveArea[78] = INPORT32(&pLCD->W4KEYCON1);	
	saveArea[79] = INPORT32(&pLCD->WIN0MAP);	
	saveArea[80] = INPORT32(&pLCD->WIN1MAP);	
	saveArea[81] = INPORT32(&pLCD->WPALCON);		
	saveArea[82] = INPORT32(&pLCD->SYSIFCON0);	
	saveArea[83] = INPORT32(&pLCD->SYSIFCON1);	
	saveArea[84] = INPORT32(&pLCD->DITHMODE);		

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
	saveArea[85] = INPORT32(&pIntr->INTMOD2);
	saveArea[86] = INPORT32(&pIntr->INTMSK2);
#endif
	pLCD->VIDCON0   = 0;
	pLCD->VIDCON1   = 0;
	pLCD->VIDTCON0   = 0;
	pLCD->VIDTCON1   = 0;
	pLCD->VIDTCON2   = 0;
	pLCD->WINCON0 = 0;
	pLCD->WINCON1 = 0;
	pLCD->VIDOSD0A = 0;
	pLCD->VIDOSD0B    = 0;
	pLCD->VIDOSD0C      = 0;
	pLCD->VIDW00ADD0B0      = 0;
	pLCD->VIDW00ADD1B0      = 0;
	pLCD->VIDW00ADD2B0      = 0;	
//	ConfigStopGPIO();

#define NEW_BSP_MODIFY 1
#if NEW_BSP_MODIFY
    //EVT3 For IROM Boot, GPC is retention in sleep mode
    pIOPort->GPCCON    &= ~(0x3f<<10);
    pIOPort->GPCUDP &= ~(0x3f<<10);
#endif	
    
	// Switch off power for KITL device
	//OALKitlPowerOff();  // woo 1021

	 //[david.modify] 2008-07-04 11:19
	 //====================================
		DPNOK(g_pBspArgs->u32WakeupHoldTime);
		ConfigSleepGPIO_david();
	//[david.modify] 2008-07-23 18:49
	// ½«´ËÐÐÄÃµô£¬²âÊÔ´ËÐÐÊÇ·ñÓ°Ïìµ½Ë¯ÃßºóTOUCH²»ºÃÊ¹µÄÎÊÌâ
	  	ConfigMiscReg_david();			

		// clear interrupt pending register
		g_pIOPort->EINTPEND = g_pIOPort->EINTPEND;
		g_pIntrRegs->SUBSRCPND = g_pIntrRegs->SUBSRCPND;
		g_pIntrRegs->SRCPND1 = g_pIntrRegs->SRCPND1;
		g_pIntrRegs->INTPND1 = g_pIntrRegs->INTPND1;
 //====================================		

 //[david.modify] 2008-07-08 15:07
 // ¸Ä³ÉÖ§³Ö³¤°´»½ÐÑºÍ¶Ì°´»½ÐÑ
	if(WAKEUP_MODE1_SHORTPRESS==g_pBspArgs->u32WakeUpMode){
		// Go to power off mode
		OALCPUPowerOff();
	}
	else/* if(WAKEUP_MODE2_LONGPRESS==g_pBspArgs->u32WakeUpMode)*/
	{
		do {			

			g_stGPIOInfo[4].u32Stat = 1;
			SetGPIOInfo(&g_stGPIOInfo[4], g_pIOPort);

 //[david.modify] 2008-08-04 12:33
 // ÓÃÓÚ¼ì²âË¯ÂÇÇ°CHARGERÊÇ²»ÊÇÒÑ¾­²åÈëµÄ
 //=================================
     if (IsChargerIn())
    {
     //[david.modify] 2008-08-19 00:31
        g_bAcAdapterInBeforeSleep = TRUE;
	  //[david.modify] 2008-08-19 00:31
//        g_bAcAdapterInBeforeSleep = FALSE;
    }
    else
    {
        g_bAcAdapterInBeforeSleep = FALSE;
    }
//======================================	

			
			// Go to power off mode
			OALCPUPowerOff();
		}while(!OALBspWakeupCheck());		
	}
 

	// Switch on power for KITL device
	// OALKitlPowerOn();
	/* Recover Process, Load CPU Regs       */
#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1
#else
	OUTPORT32(&pIOPort->GPACON,   saveArea[0]);
	OUTPORT32(&pIOPort->GPADAT,   saveArea[1]);
#endif
#elif (BSP_TYPE == BSP_SMDK2450)

#endif	
	//OUTPORT32(&pIOPort->GPACON,   saveArea[0]);
	//OUTPORT32(&pIOPort->GPADAT,   saveArea[1]);
	OUTPORT32(&pIOPort->GPBCON,   saveArea[2]);
	OUTPORT32(&pIOPort->GPBDAT,   saveArea[3]);
	OUTPORT32(&pIOPort->GPBUDP,    saveArea[4]);
	OUTPORT32(&pIOPort->GPCCON,   saveArea[5]);
	OUTPORT32(&pIOPort->GPCDAT,   saveArea[6]);
	OUTPORT32(&pIOPort->GPCUDP,    saveArea[7]);
	OUTPORT32(&pIOPort->GPDCON,   saveArea[8]);
	OUTPORT32(&pIOPort->GPDDAT,   saveArea[9]);
	OUTPORT32(&pIOPort->GPDUDP,    saveArea[10]);
	OUTPORT32(&pIOPort->GPECON,   saveArea[11]);
	OUTPORT32(&pIOPort->GPEDAT,   saveArea[12]);
	OUTPORT32(&pIOPort->GPEUDP,    saveArea[13]);
	OUTPORT32(&pIOPort->GPFCON,   saveArea[14]);
	OUTPORT32(&pIOPort->GPFDAT,   saveArea[15]);
	OUTPORT32(&pIOPort->GPFUDP,    saveArea[16]);
	OUTPORT32(&pIOPort->GPGCON,   saveArea[17]);
	OUTPORT32(&pIOPort->GPGDAT,   saveArea[18]);
	OUTPORT32(&pIOPort->GPGUDP,    saveArea[19]);
	OUTPORT32(&pIOPort->GPHCON,   saveArea[20]);
	OUTPORT32(&pIOPort->GPHDAT,   saveArea[21]);
	OUTPORT32(&pIOPort->GPHUDP,    saveArea[22]);
	OUTPORT32(&pIOPort->GPJCON,   saveArea[23]);
	OUTPORT32(&pIOPort->GPJDAT,   saveArea[24]);
	OUTPORT32(&pIOPort->GPJUDP,    saveArea[25]);
#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1
	OUTPORT32(&pIOPort->GPKCON,   saveArea[26]);
	OUTPORT32(&pIOPort->GPKDAT,   saveArea[27]);
	OUTPORT32(&pIOPort->DATAPDEN,    saveArea[28]);
#else
	OUTPORT32(&pIOPort->GPKCON,   saveArea[26]);
	OUTPORT32(&pIOPort->GPKDAT,   saveArea[27]);
	OUTPORT32(&pIOPort->GPKUDP,    saveArea[28]);
#endif
#elif (BSP_TYPE == BSP_SMDK2450)
	OUTPORT32(&pIOPort->GPKCON,   saveArea[26]);
	OUTPORT32(&pIOPort->GPKDAT,   saveArea[27]);
	OUTPORT32(&pIOPort->GPKUDP,    saveArea[28]);
#endif	
	OUTPORT32(&pIOPort->GPLCON,   saveArea[29]);
	OUTPORT32(&pIOPort->GPLDAT,   saveArea[30]);
	OUTPORT32(&pIOPort->GPLUDP,    saveArea[31]);
	OUTPORT32(&pIOPort->GPMCON,   saveArea[32]);
	OUTPORT32(&pIOPort->GPMDAT,   saveArea[33]);
	OUTPORT32(&pIOPort->GPMUDP,    saveArea[34]);	
								
	OUTPORT32(&pIOPort->MISCCR,   saveArea[35]);
	OUTPORT32(&pIOPort->DCLKCON,   saveArea[36]);
	
	OUTPORT32(&pIOPort->EXTINT0,  saveArea[37]);
	OUTPORT32(&pIOPort->EXTINT1,  saveArea[38]);
	OUTPORT32(&pIOPort->EXTINT2,  saveArea[39]);
	
	OUTPORT32(&pIOPort->EINTFLT0, saveArea[40]);
	OUTPORT32(&pIOPort->EINTFLT1, saveArea[41]);
	OUTPORT32(&pIOPort->EINTFLT2, saveArea[42]);
	OUTPORT32(&pIOPort->EINTFLT3, saveArea[43]);
	OUTPORT32(&pIOPort->EINTMASK, saveArea[44]);

#if NEW_BSP_MODIFY
	// retention release 
	pCLKPWR->RSTCON |= (1<<16);	
#endif

#if (BSP_TYPE == BSP_SMDK2443)
	OUTPORT32(&pIntr->INTMOD,     saveArea[45]);
	OUTPORT32(&pIntr->INTMSK,     saveArea[46]); 
#elif (BSP_TYPE == BSP_SMDK2450)
	OUTPORT32(&pIntr->INTMOD1,     saveArea[45]);
	OUTPORT32(&pIntr->INTMSK1,     saveArea[46]); 
#endif
	OUTPORT32(&pIntr->INTSUBMSK,  saveArea[47]);

//	InitDisplay2();	

	pLCD->VIDCON0    		=  saveArea[48]; 
	pLCD->VIDCON1 			=  saveArea[49];
	pLCD->VIDTCON0      		=  saveArea[50]; 
	pLCD->VIDTCON1  		=  saveArea[51]; 
	pLCD->VIDTCON2   		=  saveArea[52]; 
	pLCD->WINCON0  		=  saveArea[53];
	pLCD->WINCON1    		=  saveArea[54]; 
	pLCD->VIDOSD0A 		=  saveArea[55]; 
	pLCD->VIDOSD0B 		=  saveArea[56]; 
	pLCD->VIDOSD0C 		=  saveArea[57]; 
	pLCD->VIDOSD1A  		=  saveArea[58]; 
	pLCD->VIDOSD1B   		=  saveArea[59]; 
	pLCD->VIDOSD1C			=  saveArea[60]; 
	pLCD->VIDW00ADD0B0   	=  saveArea[61]; 
	pLCD->VIDW00ADD0B1  	=  saveArea[62];
	pLCD->VIDW01ADD0		=  saveArea[63];
	pLCD->VIDW00ADD1B0	=  saveArea[64];
	pLCD->VIDW00ADD1B1	=  saveArea[65];
	pLCD->VIDW01ADD1		=  saveArea[66];
	pLCD->VIDW00ADD2B0	=  saveArea[67];
	pLCD->VIDW00ADD2B1	=  saveArea[68];
	pLCD->VIDW01ADD2		=  saveArea[69];
	pLCD->VIDINTCON		=  saveArea[70]; 
	pLCD->W1KEYCON0		=  saveArea[71];
	pLCD->W1KEYCON1		=  saveArea[72];
	pLCD->W2KEYCON0		=  saveArea[73];
	pLCD->W2KEYCON1		=  saveArea[74];
	pLCD->W3KEYCON0		=  saveArea[75];
	pLCD->W3KEYCON1		=  saveArea[76];
	pLCD->W4KEYCON0		=  saveArea[77];
	pLCD->W4KEYCON1		=  saveArea[78];
	pLCD->WIN0MAP			=  saveArea[79];
	pLCD->WIN1MAP			=  saveArea[80];
	pLCD->WPALCON			=  saveArea[81];
	pLCD->SYSIFCON0		=  saveArea[82];
	pLCD->SYSIFCON1		=  saveArea[83];
	pLCD->DITHMODE			=  saveArea[84];

	 //[david.modify] 2008-06-19 15:21
//===========================	
#if 0

	delayLoop(100*LCD_DELAY_1MS);
#endif
//===========================

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
	OUTPORT32(&pIntr->INTMOD2,     saveArea[85]);
	OUTPORT32(&pIntr->INTMSK2,     saveArea[86]); 
#endif



	pCLKPWR->RSTCON 	|= pCLKPWR->RSTCON;		// This is for control GPIO pads.	


#if (BSP_TYPE == BSP_SMDK2443)
	/* Interrupt Clear                      */
	//OUTPORT32(&pIOPort->EINTPEND, INPORT32(&pIOPort->EINTPEND));
	//OUTPORT32(&pIntr->SUBSRCPND, INPORT32(&pIntr->SUBSRCPND));

#elif (BSP_TYPE == BSP_SMDK2450)
	/* Interrupt Clear                      */
	OUTPORT32(&pIOPort->EINTPEND, INPORT32(&pIOPort->EINTPEND));
	OUTPORT32(&pIntr->SUBSRCPND, INPORT32(&pIntr->SUBSRCPND));
		/* Interrupt Clear                      */
	//OUTPORT32(&pIntr->SRCPND, INPORT32(&pIntr->SRCPND));
	//OUTPORT32(&pIntr->INTPND, INPORT32(&pIntr->INTPND));

#endif	

	//    pLCD->LCDSRCPND = pLCD->LCDSRCPND;
	//    pLCD->LCDINTPND = pLCD->LCDINTPND;

	// Do platform dependent power on actions
	
	BSPPowerOn();
	
}





//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalPresuspend
//
// Description:  
//

BOOL OALIoCtlHalPresuspend(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}

#if 0
//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalEnableWake
//
// Description:  
//

BOOL OALIoCtlHalEnableWake(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalDisableWake
//
// Description:  
//

BOOL OALIoCtlHalDisableWake(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function:     OALIoCtlHalDisableWake
//
// Description:  
//

BOOL OALIoCtlHalGetWakeSource(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    return TRUE;
}
#endif //0.

//------------------------------------------------------------------------------


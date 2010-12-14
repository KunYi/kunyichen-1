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
//------------------------------------------------------------------------------
//
//  File:  init.c
//
//  Samsung SMDK2450 board initialization code.
//
#include <bsp.h>

//#include "bitmap.c" 
#include "display.h"
#include "DisplaySample_320_240.h"
 //[david.modify] 2008-05-12 09:58
 #include "dbgmsg_david.h"
 #include <xllp_gpio_david.h>
 #include <s3c2450_adc.h> 
UINT32 g_u32Temp=0;
UINT8 g_u8Temp=0;
volatile S3C2450_IOPORT_REG *v_p2450IOP=NULL;
volatile S3C2450_PWM_REG * v_pPWMregs=NULL;	


void 	OALInitUSBHost();
void InitDisplay(void);
extern DWORD CEProcessorType;
void ConfigureGPIO(void);

UINT32 g_oalIoCtlClockSpeed;

#ifdef DVS_EN
//-------------------------------------------
//@{ Variables for DVS
static int CurrVoltage[2];			// for arm and int
extern volatile int CurrentState;
//@}

//---------------------------------------------------------------------------
//{@ Function for DVS
//
BOOL ChangeVoltage(int which, int *voltage_table);	
int GetCurrentVoltage(int which);
//@}

// ----------------------------------------------------------------------------
#endif
void Max1718_Set(int pwr, int voltage);  // add 060624
void Max1718_Init(int);  // add 060624


//------------------------------------------------------------------------------
//
//  Global:  g_oalRtcResetTime
//
//  RTC init time after a RTC reset has occured.
//
 //[david.modify] 2008-06-02 09:44
//SYSTEMTIME g_oalRtcResetTime = {2005, 5, 5, 27, 12, 0, 0, 0};
SYSTEMTIME g_oalRtcResetTime = {2008, 6, 0, 1, 12, 0, 0, 0};
//------------------------------------------------------------------------------
//
//  Function:  OEMInit
//
//  This is Windows CE OAL initialization function. It is called from kernel
//  after basic initialization is made.
//
volatile BSP_ARGS *g_pBspArgs;
void OEMInit()
{
	UINT32 u32Temp=0;
//	volatile S3C2450_CLKPWR_REG *s2450PWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);
//	volatile S3C2450_SDRAM_REG *s2450DRAM = (S3C2450_SDRAM_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_SDRAM, FALSE);
//[david.modify] 2008-05-16 12:08
	g_pBspArgs = (BSP_ARGS *)OALPAtoVA(IMAGE_SHARE_ARGS_PA_START, FALSE);;


 //[david.modify] 2008-05-12 09:59
	DPNOK(OAL_FUNC);

 //[david.modify] 2008-05-16 16:46
 //=========================
 	DPNOK(g_pBspArgs->u32OALLogSetZones);
//	u32Temp = g_pBspArgs->u32OALLogSetZones;
	OALLogSetZones(g_pBspArgs->u32OALLogSetZones); 
	//OALLogSetZones(0xFFFFFFFF);
 //=========================
    OALMSG(OAL_FUNC, (L"+OEMInit\r\n"));
    
	g_oalIoCtlClockSpeed = S3C2450_FCLK;
    CEProcessorType = PROCESSOR_STRONGARM;
	


    // Set memory size for DrWatson kernel support
    dwNKDrWatsonSize = 128 * 1024;

    //NKForceCleanBoot();	// clean registry and set new, later must be erased.	- 06.06.30 JJG

    // Initilize cache globals
    OALCacheGlobalsInit();

    OALLogSerial(
        L"DCache: %d sets, %d ways, %d line size, %d size\r\n", 
        g_oalCacheInfo.L1DSetsPerWay, g_oalCacheInfo.L1DNumWays,
        g_oalCacheInfo.L1DLineSize, g_oalCacheInfo.L1DSize
    );
    OALLogSerial(
        L"ICache: %d sets, %d ways, %d line size, %d size\r\n", 
        g_oalCacheInfo.L1ISetsPerWay, g_oalCacheInfo.L1INumWays,
        g_oalCacheInfo.L1ILineSize, g_oalCacheInfo.L1ISize
    );
    
    RETAILMSG(1,(TEXT("FCLK:%d, HCLK:%d, PCLK:%d\n"), S3C2450_FCLK, S3C2450_HCLK, S3C2450_PCLK));
    
    ConfigureGPIO();
    
    // Initialize interrupts
    if (!OALIntrInit()) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OEMInit: failed to initialize interrupts\r\n"
        ));
    }

	RETAILMSG(1, (L"uninit_misc.dwResetCause %x, dwEbootFlag %x\r\n", 
		g_pBspArgs->uninit_misc.dwResetCause, g_pBspArgs->uninit_misc.dwEootFlag));
	
	switch(g_pBspArgs->uninit_misc.dwResetCause)
	{
		case HARD_RESET_FLAG:
			NKForceCleanBoot();
			RETAILMSG(1, (L"g_pBspArgs->ResetCause=HARD_RESET_FLAG\r\n"));
			// Init RTC
			break;
		case FACTORY_RESET_FLAG:
			NKForceCleanBoot();
			RETAILMSG(1, (L"g_pBspArgs->ResetCause=FACTORY_RESET_FLAG\r\n"));
			// Init RTC
			break;
		case PARTIAL_FACTORY_RESET_FLAG:
			NKForceCleanBoot();
			RETAILMSG(1, (L"g_pBspArgs->ResetCause=PARTIAL_FACTORY_RESET_FLAG\r\n"));
			// Init RTC
			break;
		case WDOG_RESET_FLAG:
			RETAILMSG(1, (L"g_pBspArgs->ResetCause=WDOG_RESET_FLAG\r\n"));
			if (g_pBspArgs->uninit_misc.dwEootFlag == FACTORY_LAUNCH)
			{
				RETAILMSG(1, (L"Eboot Mode is FACTORY MODE \r\n"));
				NKForceCleanBoot();
				break;
			}
	}

	if ((g_pBspArgs->uninit_misc.dwResetCause == FACTORY_RESET_FLAG) || (g_pBspArgs->uninit_misc.dwResetCause == PARTIAL_FACTORY_RESET_FLAG))
	{
		OALMSG (OAL_FUNC, (L"[bgkim] Factory Reset...........Set Hive Flag"));
		g_pBspArgs->uninit_misc.bCleanSystemHive = TRUE;
		g_pBspArgs->uninit_misc.bCleanUserHive = TRUE;
	}


	

    // Initialize system clock
    // OALTimerInit(1, (25-1), 0);	// 25 = S3C2450_PCLK/250/16/1000 
    OALTimerInit(RESCHED_PERIOD, (OEM_COUNT_1MS ), 0);

   
	if(0==g_pBspArgs->u32DebugDavid) {
		DPSTR_R1("InitDisplay");
		InitDisplay();
	}else{
		DPSTR_R1("NO InitDisplay");
//		InitDisplay();

	}

    // Initialize the KITL connection if required
#if (BSP_TYPE == BSP_SMDK2443)
	    OALKitlStart();
#elif (BSP_TYPE == BSP_SMDK2450)
        OALKitlStart();
#endif


//	user_test();


#ifdef DVS_EN
	Max1718_Init(TRUE);			
	CurrentState = Active;	
	{
		int voltage_set[2] = HIGH_V_SET;
		CurrVoltage[0] = voltage_set[0];
		CurrVoltage[1] = voltage_set[1];
	}
#endif
/*
	RETAILMSG(1,(TEXT("MPLLCON=0x%08X\n"),s2450PWR->MPLLCON));
	RETAILMSG(1,(TEXT("HCLKCON=0x%08X\n"),s2450PWR->HCLKCON));
	RETAILMSG(1,(TEXT("PCLKCON=0x%08X\n"),s2450PWR->PCLKCON));
	RETAILMSG(1,(TEXT("SCLKCON=0x%08X\n"),s2450PWR->SCLKCON));
	
	RETAILMSG(1,(TEXT("BANKCFG=0x%08X\n"),s2450DRAM->BANKCFG));
	RETAILMSG(1,(TEXT("BANKCON1=0x%08X\n"),s2450DRAM->BANKCON1));
	RETAILMSG(1,(TEXT("BANKCON2=0x%08X\n"),s2450DRAM->BANKCON2));	
	RETAILMSG(1,(TEXT("BANKCON3=0x%08X\n"),s2450DRAM->BANKCON3));	
	RETAILMSG(1,(TEXT("REFRESH=0x%08X\n"),s2450DRAM->REFRESH));	
*/
    OALMSG(OAL_FUNC, (L"-OEMInit\r\n"));
    RETAILMSG(1,(TEXT("-OEMInit\\n")));
 //[david.modify] 2008-05-12 09:58
	DPNOK(OAL_FUNC);	
}

//------------------------------------------------------------------------------

void delayLoop(int count) 
{ 
	volatile int j; 
	for(j = 0; j < count; j++)  ; 
}


void Write_LDI_LTV350(int address, int data)
{
	volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2450_LCD_REG    *s2450LCD = (S3C2450_LCD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);	
 	UINT8	dev_id_code=0x1D;
        int     j;
	unsigned char DELAY=50;
		

	
	LCD_DEN_Hi; 		//	EN = High					CS high
	LCD_DCLK_Hi;							//	SCL High
	LCD_DSERI_Hi;							//	Data Low

	delayLoop(DELAY);

	LCD_DEN_Lo; 		//	EN = Low				CS Low
	delayLoop(DELAY);
	
	for (j = 5; j >= 0; j--)
	{	
		LCD_DCLK_Lo;							//	SCL Low

		if ((dev_id_code >> j) & 0x0001)	// DATA HIGH or LOW
		{
			LCD_DSERI_Hi;		
		}
		else
		{
			LCD_DSERI_Lo;
		}

		delayLoop(DELAY);

		LCD_DCLK_Hi;			// CLOCK = High
		delayLoop(DELAY);

	}
	
	// RS = "0" : index data
	LCD_DCLK_Lo;			// CLOCK = Low
	LCD_DSERI_Lo;
	delayLoop(DELAY);
	LCD_DCLK_Hi;			// CLOCK = High
	delayLoop(DELAY);

	// Write
	LCD_DCLK_Lo;			// CLOCK = Low
	LCD_DSERI_Lo;
	delayLoop(DELAY);
	LCD_DCLK_Hi;			// CLOCK = High
	delayLoop(DELAY);

	for (j = 15; j >= 0; j--)
	{
		LCD_DCLK_Lo;							//	SCL Low

		if ((address >> j) & 0x0001)	// DATA HIGH or LOW
		{
			LCD_DSERI_Hi;		
		}
		else
		{
			LCD_DSERI_Lo;
		}

		delayLoop(DELAY);

		LCD_DCLK_Hi;			// CLOCK = High
		delayLoop(DELAY);

	}
	LCD_DSERI_Hi;
	delayLoop(DELAY);
	
	LCD_DEN_Hi; 				// EN = High
	delayLoop(DELAY*10);

	LCD_DEN_Lo; 		//	EN = Low				CS Low
	delayLoop(DELAY);
	
	for (j = 5; j >= 0; j--)
	{	
		LCD_DCLK_Lo;							//	SCL Low

		if ((dev_id_code >> j) & 0x0001)	// DATA HIGH or LOW
		{
			LCD_DSERI_Hi;		
		}
		else
		{
			LCD_DSERI_Lo;
		}

		delayLoop(DELAY);

		LCD_DCLK_Hi;			// CLOCK = High
		delayLoop(DELAY);

	}
	
	// RS = "1" instruction data
	LCD_DCLK_Lo;			// CLOCK = Low
	LCD_DSERI_Hi;
	delayLoop(DELAY);
	LCD_DCLK_Hi;			// CLOCK = High
	delayLoop(DELAY);

	// Write
	LCD_DCLK_Lo;			// CLOCK = Low
	LCD_DSERI_Lo;
	delayLoop(DELAY);
	LCD_DCLK_Hi;			// CLOCK = High
	delayLoop(DELAY);

	for (j = 15; j >= 0; j--)
	{
		LCD_DCLK_Lo;							//	SCL Low

		if ((data >> j) & 0x0001)	// DATA HIGH or LOW
		{
			LCD_DSERI_Hi;		
		}
		else
		{
			LCD_DSERI_Lo;
		}

		delayLoop(DELAY);

		LCD_DCLK_Hi;			// CLOCK = High
		delayLoop(DELAY);

	}
	
	LCD_DEN_Hi; 				// EN = High
	delayLoop(DELAY);

}


#if (BSP_TYPE == BSP_SMDK2450)
void InitLDI_LTE480(void)
{
    	volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	delayLoop(LCD_DELAY_1MS*10);
//[david.modify] 2008-05-28 10:50
//	s2450IOP->GPBDAT |= (1<<1);	// PCI High	
}
#elif (BSP_TYPE == BSP_SMDK2443)
#endif
#define LTV350 (TRUE)

void InitLDI_LTV350(void)
{	
    	volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    	volatile S3C2450_LCD_REG    *s2450LCD = (S3C2450_LCD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);	
		volatile S3C2450_CLKPWR_REG *s2450PWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);    	
		
	// enable EPLL CLOCK for LCD
	s2450PWR->SCLKCON |= (1<<10);

	// LCD module reset
	s2450IOP->GPBDAT |= (1<<(LCD_nRESET));
	s2450IOP->GPBDAT &= ~(1<<(LCD_nRESET)); // goes to LOW

	// delay about 5ms
	delayLoop(LCD_DELAY_1MS*10);	
	s2450IOP->GPBDAT |= (1<<(LCD_nRESET));  // goes to HIGH

		
	//SET_CONFIG_PORT( spi1)
	s2450IOP->GPLCON &= ~(((3<<(LCD_DEN_BIT*2))) | ((3<<(LCD_DCLK_BIT*2))) | ((3<<(LCD_DSERI_BIT*2))));	
	s2450IOP->GPLCON |= (((1<<(LCD_DEN_BIT*2))) | ((1<<(LCD_DCLK_BIT*2))) | ((1<<(LCD_DSERI_BIT*2))));
	// pull-up pull-down disable
	s2450IOP->GPLUDP &= ~(((3<<(LCD_DEN_BIT*2))) | ((3<<(LCD_DCLK_BIT*2))) | ((3<<(LCD_DSERI_BIT*2))));	

	s2450IOP->GPLUDP |= (((1<<(LCD_DEN_BIT*2))) | ((1<<(LCD_DCLK_BIT*2))) | ((1<<(LCD_DSERI_BIT*2))));

	// delay about 5ms
	delayLoop(LCD_DELAY_1MS*10);	
		
	LCD_DEN_Hi;
	LCD_DCLK_Hi;
	LCD_DSERI_Hi;	

	///////////////////////////////////////////////////////////////////
	// Init_Lcd_Function
	//////////////////////////////////////////////////////////////////
#if LTV350
	Write_LDI_LTV350(0x01,0x001d);	
	Write_LDI_LTV350(0x02,0x0000);    
   	Write_LDI_LTV350(0x03,0x0000);    
   	Write_LDI_LTV350(0x04,0x0000);
   	Write_LDI_LTV350(0x05,0x50a3);
   	Write_LDI_LTV350(0x06,0x0000);
   	Write_LDI_LTV350(0x07,0x0000);
   	Write_LDI_LTV350(0x08,0x0000);
   	Write_LDI_LTV350(0x09,0x0000);
   	Write_LDI_LTV350(0x0a,0x0000);
   	Write_LDI_LTV350(0x10,0x0000);
   	Write_LDI_LTV350(0x11,0x0000);
   	Write_LDI_LTV350(0x12,0x0000);
   	Write_LDI_LTV350(0x13,0x0000);
   	Write_LDI_LTV350(0x14,0x0000);
   	Write_LDI_LTV350(0x15,0x0000);
   	Write_LDI_LTV350(0x16,0x0000);
   	Write_LDI_LTV350(0x17,0x0000);
   	Write_LDI_LTV350(0x18,0x0000);
   	Write_LDI_LTV350(0x19,0x0000);
#else
	Write_LDI_LTV350(0x09,0x0000);
#endif
	///////////////////////////////////////////////////////////////////
	// Power On Reset Display off State
	//////////////////////////////////////////////////////////////////
	//Write_LDI_LTV350(0x09,0x0000);

	// delay about 10ms
	delayLoop(LCD_DELAY_1MS*10);
	
	///////////////////////////////////////////////////////////////////
	// Power Setting Function 1
	//////////////////////////////////////////////////////////////////
#if LTV350
	Write_LDI_LTV350(0x09,0x4055);
	Write_LDI_LTV350(0x0a,0x0000);
#else
	Write_LDI_LTV350(0x09,0x4055);
	Write_LDI_LTV350(0x0a,0x2000);
#endif

	// delay about 10ms
	delayLoop(LCD_DELAY_1MS*10);
	
	/////////////////////////////////////////////////////////////////////
	// Power Setting 2
	/////////////////////////////////////////////////////////////////////
#if LTV350
	Write_LDI_LTV350(0x0a,0x2000);
#else
	Write_LDI_LTV350(0x09,0x4055);
#endif

	// delay about 50ms
	delayLoop(LCD_DELAY_1MS*50);
	
	///////////////////////////////////////////////////////////////////
	// Instruction Setting
	///////////////////////////////////////////////////////////////////
#if LTV350
	Write_LDI_LTV350(0x01,0x409d);
	Write_LDI_LTV350(0x02,0x0204);
	Write_LDI_LTV350(0x03,0x2100);
	Write_LDI_LTV350(0x04,0x1000);
	Write_LDI_LTV350(0x05,0x5003);
	Write_LDI_LTV350(0x06,0x0009);	//vbp
	Write_LDI_LTV350(0x07,0x000f);	//hbp
	Write_LDI_LTV350(0x08,0x0800);
	Write_LDI_LTV350(0x10,0x0000);
	Write_LDI_LTV350(0x11,0x0000);
	Write_LDI_LTV350(0x12,0x000f);
	Write_LDI_LTV350(0x13,0x1f00);
	Write_LDI_LTV350(0x14,0x0000);
	Write_LDI_LTV350(0x15,0x0000);
	Write_LDI_LTV350(0x16,0x0000);
	Write_LDI_LTV350(0x17,0x0000);
	Write_LDI_LTV350(0x18,0x0000);
	Write_LDI_LTV350(0x19,0x0000);
#else
	Write_LDI_LTV350(0x01,0x409d);
	Write_LDI_LTV350(0x02,0x0204);
	Write_LDI_LTV350(0x03,0x0100);
	Write_LDI_LTV350(0x04,0x3000);
	Write_LDI_LTV350(0x05,0x4003);
	Write_LDI_LTV350(0x06,0x0009);	//vbp
	Write_LDI_LTV350(0x07,0x000f);	//hbp
	Write_LDI_LTV350(0x08,0x0c00);
	Write_LDI_LTV350(0x10,0x0103);
	Write_LDI_LTV350(0x11,0x0301);
	Write_LDI_LTV350(0x12,0x1f0f);
	Write_LDI_LTV350(0x13,0x1f0f);
	Write_LDI_LTV350(0x14,0x0707);
	Write_LDI_LTV350(0x15,0x0307);
	Write_LDI_LTV350(0x16,0x0707);
	Write_LDI_LTV350(0x17,0x0000);
	Write_LDI_LTV350(0x18,0x0004);
	Write_LDI_LTV350(0x19,0x0000);
#endif
	
	
	// delay about 2 frames
	delayLoop(LCD_DELAY_1MS*50);
	

	///////////////////////////////////////////////////////////////////
	// Display On Sequence
	///////////////////////////////////////////////////////////////////
#if LTV350
	Write_LDI_LTV350(0x09,0x4a55);
	Write_LDI_LTV350(0x0a,0x2000);	
#else
	Write_LDI_LTV350(0x09,0x4a55);
	Write_LDI_LTV350(0x05,0x5003);	

#endif
 
}



void Delay(void)
{
    volatile int i;

    for(i=0 ; i < 1000 ; i++)
    {
    }
}
#if (BSP_TYPE == BSP_SMDK2443)
static void InitDisplay(void)
{
	UINT8 pagewidth_in_byte=0,offsize_in_byte=0;	
	volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2450_LCD_REG    *s2450LCD = (S3C2450_LCD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
	volatile S3C2450_INTR_REG	*s2450INTR = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);
	UINT8 clkval=0;
	UINT16 hsync_cnt,vclk_cnt;
	UINT16 lcd_horizon_value,lcd_line_value;
	UINT8 lcd_vbpd,lcd_vfpd,lcd_vspw,lcd_hbpd,lcd_hfpd,lcd_hspw;
	UINT8 lcd_frame_rate;	
	UINT16 * pFB = NULL;
	    RETAILMSG(1,(TEXT("InitDisplay : SMDK2443\n")));
	s2450IOP->MISCCR |= (1<<28);  // select LCD controller for TFT lcd controller
	s2450IOP->GPCUDP     = 0xFFFFFFFF;
	s2450IOP->GPCCON    = 0xAAAAAAAA;
	s2450IOP->GPDUDP     = 0xFFFFFFFF;
	s2450IOP->GPDCON    = 0xAAAAAAAA; 
	#if 1 // tempory
	s2450IOP->GPBCON = (s2450IOP->GPBCON & ~(3<<8)) | (1<<8); // Backlight Pwm control
	s2450IOP->GPBDAT  |= (1<<8);
	#else // control the backlight using pwm timer output
	s2450IOP->GPBCON = (s2450IOP->GPBCON & ~(3<<8)) | (2<<8); // set Timer out
	#endif
	s2450IOP->GPBCON = (s2450IOP->GPBCON & ~(3<<2)) |(1<<2);
	s2450IOP->GPBDAT |= (1<<1);	
	switch(LCD_MODULE_TYPE)
	{
		case LCD_MODULE_LTV350:
			lcd_horizon_value = LTV350_HOZVAL;
			lcd_line_value = LTV350_LINEVAL;
			lcd_vbpd = LTV350_VBPD;
			lcd_vfpd = LTV350_VFPD;
			lcd_vspw = LTV350_VSPW;
			lcd_hbpd = LTV350_HBPD;
			lcd_hfpd = LTV350_HFPD;
			lcd_hspw = LTV350_HSPW;
			lcd_frame_rate = LTV350_FRAME_RATE;
			InitLDI_LTV350();
			break;
		default:
			break;
	}	
	pagewidth_in_byte = lcd_horizon_value/8*16;		
	offsize_in_byte = 0;
	s2450LCD->WINCON0 &= ~0x01;
	s2450LCD->WINCON1 &= ~0x01;
	s2450LCD->VIDCON0 &= (~3); // ENVID Off using Per Frame method
	s2450LCD->VIDCON0 = VIDCON0_S_RGB_IF|VIDCON0_S_RGB_PAR|VIDCON0_S_VCLK_GATING_OFF|VIDCON0_S_CLKDIR_DIVIDED|VIDCON0_S_CLKSEL_HCLK;
	hsync_cnt = (lcd_vbpd+lcd_vfpd+lcd_vspw+lcd_line_value);
	vclk_cnt = (lcd_hbpd+lcd_hfpd+lcd_hspw+lcd_horizon_value);
	clkval = (UINT8)(((float)S3C2450_SCLK/(float)(hsync_cnt*vclk_cnt*lcd_frame_rate))+0.5)-1;	
	s2450LCD->VIDCON0 |= (clkval <<VIDCON0_CLKVAL_F_SHIFT)|VIDCON0_S_CLKSEL_UPLL;
	s2450LCD->VIDCON1 = VIDCON1_S_HSYNC_INVERTED|VIDCON1_S_VSYNC_INVERTED;
	s2450LCD->VIDTCON0=((lcd_vbpd-1)<<VIDTCON0_BPD_S)|((lcd_vfpd-1)<<VIDTCON0_FPD_S)|(lcd_vspw-1);
	s2450LCD->VIDTCON1=((lcd_hbpd-1)<<VIDTCON0_BPD_S)|((lcd_hfpd-1)<<VIDTCON0_FPD_S)|(lcd_hspw-1);
	s2450LCD->VIDTCON2 = ((lcd_line_value-1)<<VIDTCON2_LINEVAL_S)|(lcd_horizon_value-1);
	s2450LCD->WINCON0 = (0<<WINCON_SWAP_S)|(WINCONx_16WORD_BURST<<WINCON_BURSTLEN_S)|(WINCONx_16BPP_565<<WINCON_BPP_S); // 4word burst, 16bpp, 
	s2450LCD->VIDOSD0A = (0<<VIDOSDxAB_HORIZON_X_S)|(0);
	s2450LCD->VIDOSD0B = ((lcd_horizon_value-1)<<VIDOSDxAB_HORIZON_X_S)|(lcd_line_value-1);	
	s2450LCD->VIDW00ADD0B0 = (UINT32)IMAGE_FRAMEBUFFER_DMA_BASE;		
	s2450LCD->VIDW00ADD1B0 = (UINT32)IMAGE_FRAMEBUFFER_DMA_BASE + (LCD_XSIZE_TFT*LCD_YSIZE_TFT*2);
	s2450LCD->VIDW00ADD2B0 = (offsize_in_byte<<VIDWxADD2_OFFSET_SIZE_S)|(LCD_XSIZE_TFT*2);
	s2450LCD->WINCON0 |= (1<<WINCON_SWAP_S);	
	s2450LCD->WINCON0 |= 0x1;
	s2450LCD->VIDCON0 |= 0x3;
	s2450INTR->INTSUBMSK |= (0xf << IRQ_SUB_LCD1);	// MASK all LCD Sub Interrupt
}
#elif (BSP_TYPE == BSP_SMDK2450)

 //[david.modify] 2008-05-13 15:09
#include <eboot_os_share.h>
 

int Init_IO_LCD(void* pGPIOVirtBaseAddr)
{
	int i=0;
	stGPIOInfo stGPIOInfo[]={
	{ LCDDE,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
//	{ BACKLIGHT_PWM,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ LCD_PWREN,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	

	{ LCDRST,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ LCDCS,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ LCDCLK,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ LCDSDA,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
	
	};

	for(i=0;i<sizeof(stGPIOInfo)/sizeof(stGPIOInfo[0]);i++) {
		SetGPIOInfo(&stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}

	//PRINT RESULT
	//Print_IO_LCD(pGPIOVirtBaseAddr);
	return TRUE;
}


//[david.modify] 2008-07-11 19:24

#ifndef LCD_DELAY_1MS
#define LCD_DELAY_1MS	180000
#endif

 //[david.modify] 2008-11-10 10:31
 int ShowEbootLogo(UINT32 u32Width, UINT32 u32Height)
{
	unsigned short *pFB;
	int nRet=0;
	int i;
	UINT32 u32Temp[2]={0};

	DPNOK(u32Width);
	DPNOK(u32Height);

	u32Temp[0]=g_pBspArgs->u32BootLogoAddr;
	u32Temp[1]=256*1024;

	DPNOK(u32Temp[0]);	
	DPNOK(u32Temp[1]);		
	nRet = Show565Bmp(IMAGE_FRAMEBUFFER_UA_BASE,  u32Temp[0], u32Temp[1], u32Width, u32Height );
	DPNOK(nRet);
	if(nRet!=TRUE){
		pFB = (unsigned short *)IMAGE_FRAMEBUFFER_UA_BASE;
		for (i=0; i<u32Width*(u32Height/3); i++){
			*pFB++ = 0xF800;		// red
		}	
		for (i=0; i<u32Width*(u32Height/3); i++){
			*pFB++ = 0x07E0;		// Green
		}
		
		for (i=0; i<u32Width*(u32Height/3); i++){
			*pFB++ = 0x001F;		// Blue
		}

	}

	
	return nRet;
}




 //[david.modify] 2008-06-19 16:03
void InitDisplay(void)
{

 //[david.modify] 2008-05-16 15:02
 // BOOTLOADER, Ͳٳʼһ
 #if 1
	UINT8 pagewidth_in_byte=0,offsize_in_byte=0;	
	volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2450_LCD_REG    *s2450LCD = (S3C2450_LCD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
	volatile S3C2450_INTR_REG	*s2450INTR = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);
	UINT8 clkval=0;
	UINT16 hsync_cnt,vclk_cnt;
	UINT16 lcd_horizon_value,lcd_line_value;
	UINT8 lcd_vbpd,lcd_vfpd,lcd_vspw,lcd_hbpd,lcd_hfpd,lcd_hspw;
	UINT8 lcd_frame_rate;	
	int i;
	unsigned short *pFB;
	UINT32 u32PCLK;
	stGPIOInfo g_TmpstGPIOInfo;
	int nRet=0;


	Backlight_OnOff(0, s2450IOP);  	//˴رձ⣬Ա֤ʱ
	LCDPwr_OnOff(1, s2450IOP);
	delayLoop(10*LCD_DELAY_1MS);
	DPSTR_R1(">> InitDisplay ");	
	DPNDEC3(g_pBspArgs->u32LCDType);
	BspLcdPowerUpPanel(s2450IOP, g_pBspArgs->u32LCDType);		
	
	s2450IOP->MISCCR |= (1<<28);  // select LCD controller for TFT lcd controller
	s2450IOP->GPCUDP     = 0xFFFFFFFF;
	s2450IOP->GPCCON    = 0xAAAAAAAA;
	s2450IOP->GPDUDP     = 0xFFFFFFFF;
	s2450IOP->GPDCON    = 0xAAAAAAAA; 
	s2450IOP->GPLCON = s2450IOP->GPLCON & ~(0x3ff<<20) | (0x1<< 28) | (0x1<< 26) | (0x1<< 24) | (0x1<< 22) | (0x1<< 20);
	s2450IOP->GPLDAT |= (0x1f<<10);


//ʱõЩ룬һTOUCHͺڵ
#if 0	
	s2450IOP->GPBUDP &= ~0x3;
	s2450IOP->GPBCON = (s2450IOP->GPBCON & ~((3<<6)|(3<<2)|(3<<0))) | ((1<<6)|(1<<2)|(1<<0)); // Backlight Pwm control
	s2450IOP->GPBDAT  |=((1<<0) |(1<<3));
//	s2450IOP->GPBDAT &= ~(1<<1);
#endif	

//[david.modify] 2008-08-30 10:56
//=============================================
// ȱʡLCDͺ
//	g_pBspArgs->u32LCDType = LCD_MODULE_TYPE;
#if 0
	DPSTR("1-BSP_LCD_BYD_4281L 2-BSP_LCD_YASSY_YF35F03CIB  3--BSP_LCD_SHARP_LQ035Q1 ");
	g_u32Temp = ReadSerialByte();
	switch(g_u32Temp){
	case '1': g_u32Temp=BSP_LCD_BYD_4281L;break;
	case '2': g_u32Temp=BSP_LCD_YASSY_YF35F03CIB ;break;
	case '3': g_u32Temp=BSP_LCD_SHARP_LQ035Q1 ;break;	
	default:	g_u32Temp=0;break;
	}	
	pBSPArgs->u32LCDType = g_u32Temp;
	DPNDEC(pBSPArgs->u32LCDType);
#endif		
	switch(g_pBspArgs->u32LCDType){
	case BSP_LCD_BYD_4281L:		
//		#define BYD_HPSYNC_VAL				0x17f
		#define BYD_HWSYNC_VAL			0x1f
//		#define BYD_VPSYNC_VAL				0x105
		#define BYD_VWSYNC_VAL				0x11  //0x811
		#define BYD_SCN_HSTART_VAL			68
		#define BYD_SCN_VSTART_VAL			18
		#define BYD_SCN_HEND_VAL			20
		#define BYD_SCN_VEND_VAL			4
		lcd_horizon_value = 320;
		lcd_line_value = 240;
		

 //[david.modify] 2008-10-09 14:46
 //===========================
 #if 0
		lcd_hbpd = 68;
		lcd_hfpd = 20;
		lcd_hspw = 0x1f;
		
		lcd_vbpd = 18;
		lcd_vfpd = 4;
		lcd_vspw = 0x11;
		lcd_frame_rate = 60;
#else
		lcd_hbpd = 38;
		lcd_hfpd = 20;
		lcd_hspw = 60;
//		lcd_hspw = 5;
		
		lcd_vbpd = 15;
		lcd_vfpd = 4;
		lcd_vspw = 3;
//		lcd_vspw = 1;		
		lcd_frame_rate = 60;
#endif
//#define FREQ_6500KHZ_VAL ( S3C2450_HCLK/(6500*1000) )
#define FREQ_6500KHZ_VAL ( S3C2450_HCLK/(8850*1000) )
 //[david.modify] 2008-05-15 10:22
 // S3C2450_HCLK=133 --> 133/0x16 = 133/22 = 6.05mhz
		u32PCLK =  FREQ_6500KHZ_VAL;
		break;
 //===========================	
	case BSP_LCD_YASSY_YF35F03CIB :

//			#define HPSYNC_VAL				0x197
			#define HWSYNC_VAL			33
//			#define VPSYNC_VAL				0x105
			#define VWSYNC_VAL				0x14 //0x814
			#define SCN_HSTART_VAL			33
			#define SCN_VSTART_VAL			18
			#define SCN_HEND_VAL			37
			#define SCN_VEND_VAL			17		

		lcd_horizon_value = 320;
		lcd_line_value = 240;
		
#if 1		
		lcd_hbpd = 36;
		lcd_hfpd = 50;
		lcd_hspw = 1;
		
		lcd_vbpd = 18;
		lcd_vfpd = 4;
		lcd_vspw = 0x1;		
		lcd_frame_rate = 60;
#endif
		u32PCLK =  FREQ_6500KHZ_VAL;
		break;
	case BSP_LCD_SHARP_LQ035Q1 :
	
		lcd_horizon_value = 320;
		lcd_line_value = 240;
		lcd_vbpd = 1;
		lcd_vfpd = 1;
		lcd_vspw = 1;
		lcd_hbpd = 8;
		lcd_hfpd = 8;
		lcd_hspw = 2;
		lcd_frame_rate = 60;
		u32PCLK =  FREQ_6500KHZ_VAL;
		break;
	case BSP_LCD_BYD_43INCH_480X272:
	case BSP_LCD_INNOLUX_43:
		g_TmpstGPIOInfo.u32PullUpdown = PULL_UPDOWN_DISABLE;
		g_TmpstGPIOInfo.u32PinNo = LCD_ICPOWER_IO_GPC0;
		g_TmpstGPIOInfo.u32AltFunc  = ALT_FUNC_OUT;
		g_TmpstGPIOInfo.u32Stat= 1;
		DPSTR("0-LCD_ICPOWER_IO_GPC0=0 1-LCD_ICPOWER_IO_GPC0=1");
//		g_u32Temp = ReadSerialByte();	
		g_u32Temp='1'	;
		if('0'==g_u32Temp) {
			g_TmpstGPIOInfo.u32Stat= 0;
		}else{
			g_TmpstGPIOInfo.u32Stat= 1;
		}
		DPNOK(g_TmpstGPIOInfo.u32Stat);
		SetGPIOInfo(&g_TmpstGPIOInfo, s2450IOP);


		g_TmpstGPIOInfo.u32PullUpdown = PULL_UPDOWN_DISABLE;	
		g_TmpstGPIOInfo.u32PinNo = LCD_BIAS_VOL_IO_GPE12;
		g_TmpstGPIOInfo.u32AltFunc  = ALT_FUNC_OUT;
		g_TmpstGPIOInfo.u32Stat= 1;
		DPSTR("0-LCD_BIAS_VOL_IO_GPE12=0 1-LCD_BIAS_VOL_IO_GPE12=1");
//		g_u32Temp = ReadSerialByte();	
		g_u32Temp='1'	;		
		if('0'==g_u32Temp) {
			g_TmpstGPIOInfo.u32Stat= 0;
		}else{
			g_TmpstGPIOInfo.u32Stat= 1;
		}
		DPNOK(g_TmpstGPIOInfo.u32Stat);			
		SetGPIOInfo(&g_TmpstGPIOInfo, s2450IOP);			
		
		
		lcd_horizon_value = 480;
		lcd_line_value = 272;
#if 0
		lcd_vbpd = 1;
		lcd_vfpd = 1;
		lcd_vspw = 1;
		lcd_hbpd = 8;
		lcd_hfpd = 8;
		lcd_hspw = 2;
		lcd_frame_rate = 60;		
#else
		lcd_hbpd = 68;
		lcd_hfpd = 20;
		lcd_hspw = 0x1f;
		
		lcd_vbpd = 18;
		lcd_vfpd = 4;
		lcd_vspw = 0x11;		
		lcd_frame_rate = 60;		
#endif
#define FREQ_9000KHZ_VAL ( S3C2450_HCLK/(9000*1000) )
		u32PCLK =  FREQ_9000KHZ_VAL;		
		break;

	case BSP_LCD_YASSY_43INCH_480X272 :
		g_TmpstGPIOInfo.u32PullUpdown = PULL_UPDOWN_DISABLE;
		g_TmpstGPIOInfo.u32PinNo = LCD_ICPOWER_IO_GPC0;
		g_TmpstGPIOInfo.u32AltFunc  = ALT_FUNC_OUT;
		g_TmpstGPIOInfo.u32Stat= 1;
		DPSTR("0-LCD_ICPOWER_IO_GPC0=0 1-LCD_ICPOWER_IO_GPC0=1");
//		g_u32Temp = ReadSerialByte();	
		g_u32Temp='1'	;
		if('0'==g_u32Temp) {
			g_TmpstGPIOInfo.u32Stat= 0;
		}else{
			g_TmpstGPIOInfo.u32Stat= 1;
		}
		DPNOK(g_TmpstGPIOInfo.u32Stat);
		SetGPIOInfo(&g_TmpstGPIOInfo, s2450IOP);


		g_TmpstGPIOInfo.u32PullUpdown = PULL_UPDOWN_DISABLE;	
		g_TmpstGPIOInfo.u32PinNo = LCD_BIAS_VOL_IO_GPE12;
		g_TmpstGPIOInfo.u32AltFunc  = ALT_FUNC_OUT;
		g_TmpstGPIOInfo.u32Stat= 1;
		DPSTR("0-LCD_BIAS_VOL_IO_GPE12=0 1-LCD_BIAS_VOL_IO_GPE12=1");
//		g_u32Temp = ReadSerialByte();	
		g_u32Temp='1'	;		
		if('0'==g_u32Temp) {
			g_TmpstGPIOInfo.u32Stat= 0;
		}else{
			g_TmpstGPIOInfo.u32Stat= 1;
		}
		DPNOK(g_TmpstGPIOInfo.u32Stat);			
		SetGPIOInfo(&g_TmpstGPIOInfo, s2450IOP);			
		
		
		lcd_horizon_value = 480;
		lcd_line_value = 272;
#if 0
		lcd_vbpd = 1;
		lcd_vfpd = 1;
		lcd_vspw = 1;
		lcd_hbpd = 8;
		lcd_hfpd = 8;
		lcd_hspw = 2;
		lcd_frame_rate = 60;		
#else
		lcd_hbpd = 68;
		lcd_hfpd = 20;
		lcd_hspw = 0x1f;
		
		lcd_vbpd = 18;
		lcd_vfpd = 4;
		lcd_vspw = 0x11;		
		lcd_frame_rate = 60;		
#endif
#define FREQ_9000KHZ_VAL ( S3C2450_HCLK/(9000*1000) )
		u32PCLK =  FREQ_9000KHZ_VAL;		
		break;		
	case BSP_LCD_INNOLUX_35:
		//Ⱥf.w.lin
		#define FREQ_10200KHZ_VAL ( S3C2450_HCLK/(10200*1000) )	//10.2M
	    //RETAILMSG(1,(TEXT("+0+OAL BSP_LCD_INNOLUX_35\n")));
		s2450IOP->GPECON = s2450IOP->GPECON & ~(0x3<< 24);
		s2450IOP->GPECON = s2450IOP->GPECON | (0x1<< 24);
		s2450IOP->GPEDAT |= (0x1<<12);

		lcd_horizon_value = 320;
		lcd_line_value = 240;

		lcd_hbpd = 47;//40;
		lcd_hfpd = 4;
		lcd_hspw = 1;

		lcd_vbpd = 20;//13;
		lcd_vfpd = 214;
		lcd_vspw = 1;
		lcd_frame_rate = 60;
		u32PCLK =  FREQ_10200KHZ_VAL;
		break;
	default:
		break;	}

//=============================================
#if 1
//	     memcpy((void *)IMAGE_FRAMEBUFFER_UA_BASE, prayer16bpp, sizeof(prayer16bpp));
//	     memcpy((void *)IMAGE_FRAMEBUFFER_UA_BASE, prayer16bpp, LCD_ARRAY_SIZE_TFT_16BIT);
		DPSTR("ShowEbootLogo+");
		nRet = ShowEbootLogo(lcd_horizon_value, lcd_line_value);
		DPSTR("ShowEbootLogo-");		
		DPNOK(nRet);		
#else
		pFB = (unsigned short *)IMAGE_FRAMEBUFFER_UA_BASE;
		for (i=0; i<lcd_horizon_value*(lcd_line_value/3); i++){
			*pFB++ = 0xF800;		// red
		}	
		for (i=0; i<lcd_horizon_value*(lcd_line_value/3); i++){
			*pFB++ = 0x07E0;		// Green
		}
		
		for (i=0; i<lcd_horizon_value*(lcd_line_value/3); i++){
			*pFB++ = 0x001F;		// Blue
		}
#endif	
//=============================================
	pagewidth_in_byte = lcd_horizon_value/8*16;		
	offsize_in_byte = 0;
	s2450LCD->WINCON0 &= ~0x01;
	s2450LCD->WINCON1 &= ~0x01;
	s2450LCD->VIDCON0 &= (~3); // ENVID Off using Per Frame method
	s2450LCD->VIDCON0 = VIDCON0_S_RGB_IF|VIDCON0_S_RGB_PAR|VIDCON0_S_VCLK_GATING_OFF|VIDCON0_S_CLKDIR_DIVIDED|VIDCON0_S_CLKSEL_HCLK;
	hsync_cnt = (lcd_vbpd+lcd_vfpd+lcd_vspw+lcd_line_value);
	vclk_cnt = (lcd_hbpd+lcd_hfpd+lcd_hspw+lcd_horizon_value);

	clkval = (UINT8)(((float)S3C2450_HCLK/(float)(hsync_cnt*vclk_cnt*lcd_frame_rate*2))+0.5)-1;	
 //[david.modify] 2008-05-15 10:17
 	DPNOK(S3C2450_HCLK);
 	DPNOK(clkval);
// 	clkval = VIDCON0_CLKVAL_VAR;
 	clkval =  	u32PCLK;
	DPNOK(clkval);
  //[david. end] 2008-05-15 10:17
	s2450LCD->VIDCON0 |= (clkval <<VIDCON0_CLKVAL_F_SHIFT);
	 //[david.modify] 2008-05-13 15:07
	DPNOK(g_pBspArgs->u32LCDType);
	if(BSP_LCD_BYD_43INCH_480X272 ==g_pBspArgs->u32LCDType || BSP_LCD_INNOLUX_43 ==g_pBspArgs->u32LCDType ){	//½
		s2450LCD->VIDCON1 = VIDCON1_S_HSYNC_INVERTED|VIDCON1_S_VSYNC_INVERTED;
	}
	else if(BSP_LCD_YASSY_43INCH_480X272 ==g_pBspArgs->u32LCDType){	//½
		s2450LCD->VIDCON1 = VIDCON1_S_HSYNC_INVERTED|VIDCON1_S_VSYNC_INVERTED;
	}	
	else{											//
		s2450LCD->VIDCON1 = VIDCON1_S_HSYNC_INVERTED|VIDCON1_S_VSYNC_INVERTED|VIDCON1_S_VCLK_RISE_EDGE_FETCH;
	}


	s2450LCD->VIDTCON0=((lcd_vbpd-1)<<VIDTCON0_BPD_S)|((lcd_vfpd-1)<<VIDTCON0_FPD_S)|(lcd_vspw-1);
	s2450LCD->VIDTCON1=((lcd_hbpd-1)<<VIDTCON0_BPD_S)|((lcd_hfpd-1)<<VIDTCON0_FPD_S)|(lcd_hspw-1);
	s2450LCD->VIDTCON2 = ((lcd_line_value-1)<<VIDTCON2_LINEVAL_S)|(lcd_horizon_value-1);
#if DISPLAY_24BIT_MODE	
	s2450LCD->WINCON0 = (0<<WINCON_SWAP_S)|(WINCONx_16WORD_BURST<<WINCON_BURSTLEN_S)|(WINCONx_24BPP_888<<WINCON_BPP_S); // 4word burst, 16bpp, 
#else
	s2450LCD->WINCON0 = (0<<WINCON_SWAP_S)|(WINCONx_16WORD_BURST<<WINCON_BURSTLEN_S)|(WINCONx_16BPP_565<<WINCON_BPP_S); // 4word burst, 16bpp, 
#endif
	s2450LCD->VIDOSD0A = (0<<VIDOSDxAB_HORIZON_X_S)|(0);
	s2450LCD->VIDOSD0B = ((lcd_horizon_value-1)<<VIDOSDxAB_HORIZON_X_S)|(lcd_line_value-1);	
	s2450LCD->VIDW00ADD0B0 = (UINT32)IMAGE_FRAMEBUFFER_DMA_BASE;		

#if DISPLAY_24BIT_MODE	
	s2450LCD->VIDW00ADD1B0 = (UINT32)IMAGE_FRAMEBUFFER_DMA_BASE + (lcd_horizon_value*lcd_line_value*4);	
#else
	s2450LCD->VIDW00ADD1B0 = (UINT32)IMAGE_FRAMEBUFFER_DMA_BASE + (lcd_horizon_value*lcd_horizon_value*2);
#endif
	s2450LCD->VIDW00ADD2B0 = (offsize_in_byte<<VIDWxADD2_OFFSET_SIZE_S)|(lcd_horizon_value*2);
#if DISPLAY_24BIT_MODE	
	s2450LCD->WINCON0 &=~ (1<<WINCON_SWAP_S);	
#else	
	s2450LCD->WINCON0 |= (1<<WINCON_SWAP_S);	
#endif
	s2450LCD->WINCON0 |= 0x1;
	s2450LCD->VIDCON0 |= 0x3;
	s2450INTR->INTSUBMSK |= (0xf << IRQ_SUB_LCD1);	// MASK all LCD Sub Interrupt
#endif

 //[david.modify] 2008-06-19 15:21
//===========================	 

//	delayLoop(100*LCD_DELAY_1MS);
// һЩȴʱ
//	delayLoop(100*LCD_DELAY_1MS);
//===========================
	delayLoop(100*LCD_DELAY_1MS);
	
//	LCDPwr_OnOff(1, s2450IOP);
	Backlight_OnOff(1, s2450IOP);

//[david.modify] 2008-05-30 16:10
// ʾOK֮,֮ǰҵ
//================================
//	PWM_Init();
	SetPWMValue1(OS_PWM_VALUE1);
	g_u32Temp = GetPWMValue1();
	DPNOK(g_u32Temp);	
//================================

	
}

 //[david.modify] 2008-06-19 16:03
void InitDisplay2(void)
{

 //[david.modify] 2008-05-16 15:02
 // BOOTLOADER, Ͳٳʼһ
	UINT8 pagewidth_in_byte=0,offsize_in_byte=0;	
	volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2450_LCD_REG    *s2450LCD = (S3C2450_LCD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
	volatile S3C2450_INTR_REG	*s2450INTR = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);
	UINT8 clkval=0;
	UINT16 hsync_cnt,vclk_cnt;
	UINT16 lcd_horizon_value,lcd_line_value;
	UINT8 lcd_vbpd,lcd_vfpd,lcd_vspw,lcd_hbpd,lcd_hfpd,lcd_hspw;
	UINT8 lcd_frame_rate;	
	int i;
	unsigned short *pFB;

 //[david.modify] 2008-08-16 10:48
//	Backlight_OnOff(0, s2450IOP);  

#if 0	
	Init_IO_LCD(s2450IOP);				
	InitLDI_LTV350_sharp2(s2450IOP);
#endif	

 //[david.modify] 2008-09-01 10:35
 //һҪ֤LCD POWERȿ͸SPIЧPanel
// ģʽѺspi
// 1. ڴ˴ 2. power button ȥ
// note: power button
#if 1
	Backlight_OnOff(0, s2450IOP);  	//˴رձ⣬Ա֤ʱ
	LCDPwr_OnOff(1, s2450IOP);
	delayLoop(10*LCD_DELAY_1MS);
	BspLcdPowerUpPanel(s2450IOP, g_pBspArgs->u32LCDType);			
#else
	
#endif

	
//	LCDPwr_OnOff(0, s2450IOP);
//	Backlight_OnOff(0, s2450IOP);  

//	delayLoop(100*LCD_DELAY_1MS);
// һЩȴʱ
	delayLoop(1*LCD_DELAY_1MS);
//===========================


}
 
#endif


void ConfigureGPIO()
{
    volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    volatile S3C2450_CLKPWR_REG *s2450CLKPWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);

}
#ifdef DVS_EN
#define UNDERSHOOT_WORKAROUND
/// @func ChangeVoltage
/// 
BOOL ChangeVoltage(int which, int *voltage_table)
{
	volatile int i;
	if((which & ARM_INT_VDD) == 0)
	{
		RETAILMSG(1,(TEXT("not defined voltage source to change. Voltage is not changed\n")));
		return FALSE;
	}
	else if(which == ARM_INT_VDD)
	{
#ifdef UNDERSHOOT_WORKAROUND
	{	
	 	volatile int i;
		for(i=0;i<VOLTAGEDELAY*2;i++);
	}
#endif			
		Max1718_Set(ARM_VDD, voltage_table[0]);
		Max1718_Set(INT_VDD, voltage_table[1]);
		if(	CurrVoltage[0] < voltage_table[0])		///< the voltage is change to higher
		{
			/// Regulator Delay
			for(i=0; i < VOLTAGEDELAY; i++);
		}
		CurrVoltage[0] = voltage_table[0];	///< ARM Voltage
		CurrVoltage[1] = voltage_table[1];	///< Int Voltage

	}
	else
	{
#ifdef UNDERSHOOT_WORKAROUND
	{	
	 	volatile int i;
		for(i=0;i<VOLTAGEDELAY*2;i++);
	}
#endif			

		Max1718_Set(which, voltage_table[which-1]);
		if(	CurrVoltage[which-1] < voltage_table[which-1])		///< the voltage is change to higher
		{
			/// Regulator Delay
			for(i=0; i < VOLTAGEDELAY; i++);
		}
		CurrVoltage[which-1] = voltage_table[which-1];
		
	}

	return TRUE;
}

/// Input ARM_VDD : 1
///       INT_VDD : 2
/// Returns
///		Current Voltage
int GetCurrentVoltage(int which)
{
	if ((which == ARM_VDD )	|| (which == INT_VDD))
	{
		return CurrVoltage[which-1];
	}
	else
	{
		return 0;
	}
}
#endif


//------------------------------------------------
//	MAX1718 ARM core, internal regulator setting		added 06.07.03 JJG
//------------------------------------------------


#if (BSP_TYPE == BSP_SMDK2443)
void Max1718_Init(int LatchOut_OnOff)  // add 060624
{
    volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	//////////////////////////////////////////////
	// GPF3 GPF4 GPF5 GPF6 GPF7
	//  D4   D3   D2   D1   D0
	
	//	0	 0	  0 	0	 0		// 1.75V
	//	0	 0	  0 	0	 1		// 1.70V
	//	0	 0	  0 	1	 0		// 1.65V
	//	0	 0	  0 	1	 1		// 1.60V
	//	0	 0	  1 	0	 0		// 1.55V
	//	0	 0	  1 	0	 1		// 1.50V
	//	0	 0	  1 	1	 0		// 1.45V, Max. voltage of S3C2450 VDDarm
	//	0	 0	  1 	1	 1		// 1.40V
	//	0	 1	  0 	0	 0		// 1.35V
	//	0	 1	  0 	0	 1		// 1.30V
	//	0	 1	  0 	1	 0		// 1.25V
	//	0	 1	  0 	1	 1		// 1.20V
	//	0	 1	  1 	0	 0		// 1.15V
	//	0	 1	  1 	0	 1		// 1.10V
	//	0	 1	  1 	1	 0		// 1.05V
	//	0	 1	  1 	1	 1		// 1.00V
	//	1	 0	  0 	0	 1		// 0.95V
	//	1	 0	  0 	1	 1		// 0.90V
	//	1	 0	  1 	0	 1		// 0.85V
	//	1	 0	  1 	1	 1		// 0.80V
	if(LatchOut_OnOff == TRUE)
	{
		RETAILMSG(1,(TEXT("I GPFDAT: %x, GPFCON: %x, EXTINT0: %x, GPGDAT: %x, GPGCON: %x, EXTINT1: %x, GPBDAT: %x, GPBCON: %x, GPBUDP: %x\n"),
			 s2450IOP->GPFDAT, s2450IOP->GPFCON, s2450IOP->EXTINT0,
			 s2450IOP->GPGDAT, s2450IOP->GPGCON, s2450IOP->EXTINT1,
			 s2450IOP->GPBDAT, s2450IOP->GPBCON, s2450IOP->GPBUDP));	
       
    s2450IOP->GPFCON = s2450IOP->GPFCON & ~(0x3ff<<6)|(0x155<<6); // GPF3 ~ 7(Data5 ~ 1): OUTPUT
		// Always pull down, and force driving by GPFDAT to high
    
    s2450IOP->GPGCON = s2450IOP->GPGCON & ~(0xf<<12)|(0x5<<12); // GPG6(nARM_REG_LE), GPG7(CORE_REG_OE): OUTPUT
    s2450IOP->GPGUDP = s2450IOP->GPGUDP & ~(0xf<<12)|(0xa<<12); // pull down disable
    
    s2450IOP->GPBCON = s2450IOP->GPBCON & ~(0x3<<4)|(0x1<<4); // GPB2(nINT_REG_LE): OUTPUT
    s2450IOP->GPBUDP = s2450IOP->GPBUDP & ~(0x3<<4)|(0x2<<4); // pull down disable
    
   
    s2450IOP->GPGDAT|=(1<<7);   //Output enable
 
    s2450IOP->GPGDAT&=~(1<<6);   //Arm Latch enable
    s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(0<<5)|(0<<6)|(0<<7);	//1.35V
		s2450IOP->GPGDAT|=(1<<6);   //Arm Latch disable     

    s2450IOP->GPBDAT&=~(1<<2);   //Int Latch enable
		s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(0<<5)|(1<<6)|(1<<7);	//1.2V
		s2450IOP->GPBDAT|=(1<<2);   //Int Latch diaable

		RETAILMSG(1,(TEXT("E GPFDAT: %x, GPFCON: %x, EXTINT0: %x, GPGDAT: %x, GPGCON: %x, EXTINT1: %x, GPBDAT: %x, GPBCON: %x, GPBUDP: %x\n"),
			 s2450IOP->GPFDAT, s2450IOP->GPFCON, s2450IOP->EXTINT0,
			 s2450IOP->GPGDAT, s2450IOP->GPGCON, s2450IOP->EXTINT1,
			 s2450IOP->GPBDAT, s2450IOP->GPBCON, s2450IOP->GPBUDP));
	}
	else
	{
		s2450IOP->GPGDAT&=~(1<<7);		//Output Disable
	}	

}
#elif (BSP_TYPE == BSP_SMDK2450)
void Max1718_Init(int LatchOut_OnOff)
{
    volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	//////////////////////////////////////////////
	// GPB2 GPC7 GPC6     GPC5     GPC0
	//  D4   D3   D2       D1       D0
	//	0	 0	  0 	   0	    0		// 1.75V
	//	0	 0	  0 	   0	    1		// 1.70V
	//	0	 0	  0 	   1	    0		// 1.65V
	//	0	 0	  0 	   1	    1		// 1.60V
	//	0	 0	  1 	   0	    0		// 1.55V
	//	0	 0	  1 	   0	    1		// 1.50V
	//	0	 0	  1 	   1	    0		// 1.45V, Max. voltage of S3C2450 VDDarm
	//	0	 0	  1 	   1	    1		// 1.40V
	//	0	 1	  0 	   0	    0		// 1.35V
	//	0	 1	  0 	   0	    1		// 1.30V
	//	0	 1	  0 	   1	    0		// 1.25V
	//	0	 1	  0 	   1	    1		// 1.20V
	//	0	 1	  1 	   0	    0		// 1.15V
	//	0	 1	  1 	   0	    1		// 1.10V
	//	0	 1	  1 	   1	    0		// 1.05V
	//	0	 1	  1 	   1	    1		// 1.00V
	//	1	 0	  0 	   0	    1		// 0.95V
	//	1	 0	  0 	   1	    1		// 0.90V
	//	1	 0	  1 	   0	    1		// 0.85V
	//	1	 0	  1 	   1	    1		// 0.80V

	if(LatchOut_OnOff == TRUE)
	{
		s2450IOP->GPCCON = (s2450IOP->GPCCON & ~((0x3f<<10)|(0x3<<0)))|(0x15<<10)|(0x1<<0);  // GPC0, GPC5, GPC6, GPC7: OUTPUT
		s2450IOP->GPBCON = (s2450IOP->GPBCON & ~(0x3<<4)) | (0x1<<4); // GPB2: OUTPUT

		s2450IOP->GPFCON = s2450IOP->GPFCON & ~(0x3f<<10)|(0x15<<10); // GPF5(nARM_REG_LE), GPF6(CORE_REG_OE), GPF7(nINT_REG_LE): OUTPUT
		s2450IOP->GPFUDP = s2450IOP->GPFUDP & ~(0x3f<<10); // pull up-down disable
		
		s2450IOP->GPFDAT |= (1<<6);   //Output enable

		s2450IOP->GPFDAT&=~(1<<5);   //Arm Latch enable
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(0<<6)|(0<<5)|(0<<0);//D3, D2, D1, D0 ---- 1.35V
		s2450IOP->GPFDAT|=(1<<5);   //Arm Latch disable     

		s2450IOP->GPFDAT &= ~(1<<7);   //Int Latch enable
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(0<<6)|(1<<5)|(1<<0);//D3, D2, D1, D0 ---- 1.2V
		s2450IOP->GPFDAT|=(1<<7);   //Int Latch disable
	}
	else
	{
		s2450IOP->GPFDAT&=~(1<<6);		//Output Disable
	}
}
#endif

//------------------------------------------------
//	MAX1718 ARM core, internal regulator setting
//------------------------------------------------
#if (BSP_TYPE == BSP_SMDK2443)
void Max1718_Set(int pwrsrc, int voltage)  // add 060624
{
    volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);

     int vtg;
	//////////////////////////////////////////////
	// GPF3 GPF4 GPF5 GPF6 GPF7
	//  D4   D3   D2   D1   D0
	
	//	0	 0	  0 	0	 0		// 1.75V
	//	0	 0	  0 	0	 1		// 1.70V
	//	0	 0	  0 	1	 0		// 1.65V
	//	0	 0	  0 	1	 1		// 1.60V
	//	0	 0	  1 	0	 0		// 1.55V
	//	0	 0	  1 	0	 1		// 1.50V
	//	0	 0	  1 	1	 0		// 1.45V, Max. voltage of S3C2450 VDDarm
	//	0	 0	  1 	1	 1		// 1.40V
	//	0	 1	  0 	0	 0		// 1.35V
	//	0	 1	  0 	0	 1		// 1.30V
	//	0	 1	  0 	1	 0		// 1.25V
	//	0	 1	  0 	1	 1		// 1.20V
	//	0	 1	  1 	0	 0		// 1.15V
	//	0	 1	  1 	0	 1		// 1.10V
	//	0	 1	  1 	1	 0		// 1.05V
	//	0	 1	  1 	1	 1		// 1.00V
	//	1	 0	  0 	0	 1		// 0.95V
	//	1	 0	  0 	1	 1		// 0.90V
	//	1	 0	  1 	0	 1		// 0.85V
	//	1	 0	  1 	1	 1		// 0.80V
		UINT32 backup_gpfdat;
		backup_gpfdat = s2450IOP->GPFDAT; 
    vtg=voltage;
  /*  
    s2450IOP->GPFCON = s2450IOP->GPFCON & ~(0x3ff<<6)|(0x155<<6); // GPF3 ~ 7(Data5 ~ 1): OUTPUT
    s2450IOP->GPFUDP = s2450IOP->GPFUDP & ~(0x3ff<<6)|(0x2aa<<6); // pull down disable
    
    s2450IOP->GPGCON = s2450IOP->GPGCON & ~(0xf<<12)|(0x5<<12); // GPG6(nARM_REG_LE), GPG7(CORE_REG_OE): OUTPUT
    s2450IOP->GPGUDP = s2450IOP->GPGUDP & ~(0xf<<12)|(0xa<<12); // pull down disable
    
    s2450IOP->GPBCON = s2450IOP->GPBCON & ~(0x3<<4)|(0x1<<4); // GPB2(nINT_REG_LE): OUTPUT
    s2450IOP->GPBUDP = s2450IOP->GPBUDP & ~(0x3<<4)|(0x2<<4); // pull down disable
*/
// RETAILMSG(1,(TEXT("A GPFDAT: %x, GPFCON: %x, GPFUDP: %x, GPGDAT: %x, GPGCON: %x, GPGUDP: %x, GPBDAT: %x, GPBCON: %x, GPBUDP: %x\n"),
//			 s2450IOP->GPFDAT, s2450IOP->GPFCON, s2450IOP->GPFUDP,
//			 s2450IOP->GPGDAT, s2450IOP->GPGCON, s2450IOP->GPGUDP,
//			 s2450IOP->GPBDAT, s2450IOP->GPBCON, s2450IOP->GPBUDP));

    switch (vtg)
	{
	case V1750mV:
	      s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(0<<4)|(0<<5)|(0<<6)|(0<<7);	//D4~0
	      break;

	case V1700mV:
	      s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(0<<4)|(0<<5)|(0<<6)|(1<<7);	//D4~0
	      break;

	case V1650mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(0<<4)|(0<<5)|(1<<6)|(0<<7);	//D4~0
		 break;
       
	case V1600mV:
		s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(0<<4)|(0<<5)|(1<<6)|(1<<7);	//D4~0
		break;

	case V1550mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(0<<4)|(1<<5)|(0<<6)|(0<<7);	//D4~0
		 break;

	case V1500mV:
		s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(0<<4)|(1<<5)|(0<<6)|(1<<7);	//D4~0
		 break;

	case V1450mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(0<<4)|(1<<5)|(1<<6)|(0<<7);	//D4~0
		 break;

	case V1400mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(0<<4)|(1<<5)|(1<<6)|(1<<7);	//D4~0
		 break;

	case V1350mV:
	      s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(0<<5)|(0<<6)|(0<<7);	//D4~0
	      break;

	case V1300mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(0<<5)|(0<<6)|(1<<7);	//D4~0
		 break;
       
	case V1250mV:
		s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(0<<5)|(1<<6)|(0<<7);	//D4~0
		break;

	case V1200mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(0<<5)|(1<<6)|(1<<7);	//D4~0
		 break;

	case V1150mV:
		s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(1<<5)|(0<<6)|(0<<7);	//D4~0
		 break;

	case V1100mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(1<<5)|(0<<6)|(1<<7);	//D4~0
		 break;

	case V1050mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(1<<5)|(1<<6)|(0<<7);	//D4~0
		 break;

	case V1000mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7);	//D4~0
		break;

	case V950mV:
		s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(1<<3)|(0<<4)|(0<<5)|(0<<6)|(1<<7);	//D4~0
		 break;
	case V925mV:
		s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(1<<3)|(0<<4)|(0<<5)|(1<<6)|(0<<7);	//D4~0
		 break;

	case V900mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(1<<3)|(0<<4)|(0<<5)|(1<<6)|(1<<7);	//D4~0
		 break;

	case V850mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(1<<3)|(0<<4)|(1<<5)|(0<<6)|(1<<7);	//D4~0
		 break;
	case V825mV:
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(1<<3)|(0<<4)|(1<<5)|(1<<6)|(0<<7);	//D4~0
		 break;

    case V800mV:
		s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(1<<3)|(0<<4)|(1<<5)|(1<<6)|(1<<7);	//D4~0
		 break;

    default:	// 1.35V
		 s2450IOP->GPFDAT=(s2450IOP->GPFDAT&~(0x1f<<3))|(0<<3)|(1<<4)|(0<<5)|(0<<6)|(0<<7);	//D4~0
		 break;	 		
	}
	
	
	if(pwrsrc == ARM_VDD)	s2450IOP->GPGDAT&=~(1<<6);   //Arm Latch enable
	else  	s2450IOP->GPBDAT&=~(1<<2);   //Int Latch enable
	
	s2450IOP->GPGDAT|=(1<<7);   //Output enable
	
	if(pwrsrc == ARM_VDD)	s2450IOP->GPGDAT|=(1<<6);   //Arm Latch disable
	else  	s2450IOP->GPBDAT|=(1<<2);   //Int Latch diaable

	s2450IOP->GPFDAT = backup_gpfdat; 	

// RETAILMSG(1,(TEXT("B GPFDAT: %x, GPFCON: %x, GPFUDP: %x, GPGDAT: %x, GPGCON: %x, GPGUDP: %x, GPBDAT: %x, GPBCON: %x, GPBUDP: %x\n"),
//			 s2450IOP->GPFDAT, s2450IOP->GPFCON, s2450IOP->GPFUDP,
//			 s2450IOP->GPGDAT, s2450IOP->GPGCON, s2450IOP->GPGUDP,
//			 s2450IOP->GPBDAT, s2450IOP->GPBCON, s2450IOP->GPBUDP));
}
#elif (BSP_TYPE == BSP_SMDK2450)
void Max1718_Set(int pwr, int voltage)  // add 060624
{
	volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);

	int vtg;
	// GPB2 GPC7 GPC6     GPC5     GPC0
	//  D4   D3   D2       D1       D0
	//	0	 0	  0 	   0	    0		// 1.75V
	//	0	 0	  0 	   0	    1		// 1.70V
	//	0	 0	  0 	   1	    0		// 1.65V
	//	0	 0	  0 	   1	    1		// 1.60V
	//	0	 0	  1 	   0	    0		// 1.55V
	//	0	 0	  1 	   0	    1		// 1.50V
	//	0	 0	  1 	   1	    0		// 1.45V, Max. voltage of S3C2450 VDDarm
	//	0	 0	  1 	   1	    1		// 1.40V
	//	0	 1	  0 	   0	    0		// 1.35V
	//	0	 1	  0 	   0	    1		// 1.30V
	//	0	 1	  0 	   1	    0		// 1.25V
	//	0	 1	  0 	   1	    1		// 1.20V
	//	0	 1	  1 	   0	    0		// 1.15V
	//	0	 1	  1 	   0	    1		// 1.10V
	//	0	 1	  1 	   1	    0		// 1.05V
	//	0	 1	  1 	   1	    1		// 1.00V
	//	1	 0	  0 	   0	    1		// 0.95V
	//	1	 0	  0 	   1	    1		// 0.90V
	//	1	 0	  1 	   0	    1		// 0.85V
	//	1	 0	  1 	   1	    1		// 0.80V
	UINT32 backup_gpbdat;
	UINT32 backup_gpcdat;
	backup_gpbdat = s2450IOP->GPBDAT;
	backup_gpcdat = s2450IOP->GPCDAT;
	vtg=voltage;

	s2450IOP->GPCCON = (s2450IOP->GPCCON & ~((0x3f<<10)|(0x3<<0)))|(0x15<<10)|(0x1<<0);  // GPC0, GPC5, GPC6, GPC7: OUTPUT
	s2450IOP->GPBCON = (s2450IOP->GPBCON & ~(0x3<<4)) | (0x1<<4); // GPB2: OUTPUT

	s2450IOP->GPFCON = s2450IOP->GPFCON & ~(0x3f<<10)|(0x15<<10); // GPF5(nARM_REG_LE), GPF6(CORE_REG_OE), GPF7(nINT_REG_LE): OUTPUT
	s2450IOP->GPFUDP = s2450IOP->GPFUDP & ~(0x3f<<10); // pull up-down disable

    switch (vtg)
	{
	case V1750mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(0<<6)|(0<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V1700mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(0<<6)|(0<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V1650mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(0<<6)|(1<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V1600mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(0<<6)|(1<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V1550mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(1<<6)|(0<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V1500mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(1<<6)|(0<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V1450mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(1<<6)|(1<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V1400mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(1<<6)|(1<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V1350mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(0<<6)|(0<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V1300mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(0<<6)|(0<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V1250mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(0<<6)|(1<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V1200mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(0<<6)|(1<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V1150mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(1<<6)|(0<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V1100mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(1<<6)|(0<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V1050mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(1<<6)|(1<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V1000mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(1<<6)|(1<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V950mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(1<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(0<<6)|(0<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V925mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(1<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(0<<6)|(1<<5)|(0<<0);//D3, D2, D1, D0
		break;

	case V900mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(1<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(0<<6)|(1<<5)|(1<<0);//D3, D2, D1, D0
		break;

	case V850mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(1<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(1<<6)|(0<<5)|(1<<0);//D3, D2, D1, D0
		break;

    case V825mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(1<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(1<<6)|(1<<5)|(0<<0);//D3, D2, D1, D0
		break;

    case V800mV:
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(1<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(0<<7)|(1<<6)|(1<<5)|(1<<0);//D3, D2, D1, D0
		break;

    default:	// 1.35V
		s2450IOP->GPBDAT=(s2450IOP->GPBDAT&~(0x1<<2))|(0<<2);//D4
		s2450IOP->GPCDAT=(s2450IOP->GPCDAT&~((0x7<<5)|(0x1<<0)))|(1<<7)|(0<<6)|(0<<5)|(0<<0);//D3, D2, D1, D0
		break;
	}

	if(pwr)	s2450IOP->GPFDAT&=~(1<<5);   //Arm Latch enable
	else  	s2450IOP->GPFDAT &= ~(1<<7);   //Int Latch enable

	s2450IOP->GPFDAT |= (1<<6);   //Output enable

	if(pwr)	s2450IOP->GPFDAT|=(1<<5);   //Arm Latch disable     
	else  	s2450IOP->GPFDAT|=(1<<7);   //Int Latch disable

	s2450IOP->GPBDAT = backup_gpbdat;
	s2450IOP->GPCDAT = backup_gpcdat;
}
#endif

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
Copyright (c) 2005. Samsung Electronics, co. ltd  All rights reserved.

Module Name:  

Abstract:

    Platform dependent TOUCH initialization functions

rev:
	2004.4.27	: S3C2440 port (Hoyjoon Kim)
	2005.05.23  : Magneto porting revision (junkim)
	2006.06.29  :  S3C2450 port(JJG)

Notes: 
--*/


#include <windows.h>
#include <types.h>
#include <memory.h>
#include <nkintr.h>
#include <tchddsi.h>
#include <nkintr.h>

#include <s3c2450.h>
#include <bsp_cfg.h>
#include <dbgmsg_david.h>

#define PUBLIC		
#define PRIVATE							static

#define TOUCH_12BIT_SUPPORT 1

#define TSP_SAMPLE_RATE_LOW			50  
#define TSP_SAMPLE_RATE_HIGH			100
#define TSP_TIMER_DIVIDER			D2		// 
#define TSP_TICK_1S				(S3C2450_PCLK / (PRESCALER+1) / TSP_TIMER_DIVIDER /1000)
#define TSP_SAMPLETICK_LOW			(TSP_TICK_1S*1000/ TSP_SAMPLE_RATE_LOW - 1)	
#define TSP_SAMPLETICK_HIGH			(TSP_TICK_1S*1000/ TSP_SAMPLE_RATE_HIGH - 1)	// 1020 = PCLK/PRESCALE(=250)/DIV3(=2)/1000(1SEC)*(1000/100)


#define ADCPRS							49 // 200
#define TS_DBGON						0

/*
#define TSP_MINX						74 //105  //85
#define TSP_MINY						98 //70 //105

#define TSP_MAXX						932 //910 //965
#define TSP_MAXY						920 // 940 //980
*/

#define TSP_CHANGE						15
#define TSP_INVALIDLIMIT				40


//#define TSP_LCDX						(LCD_XSIZE_TFT * 4)
//#define TSP_LCDY						(LCD_YSIZE_TFT * 4)

#define	TSP_SAMPLE_NUM					4 


#ifndef OAL_INTR_FORCE_STATIC
#define OAL_INTR_FORCE_STATIC       (1 << 2)
#endif

DWORD gIntrTouch        = SYSINTR_NOP;
DWORD gIntrTouchChanged = SYSINTR_NOP;

extern "C" const int MIN_CAL_COUNT   = 1;

PRIVATE INT TSP_CurRate = 1;

static BOOL Touch_Pen_Filtering(INT *px, INT *py); 

PRIVATE volatile S3C2450_IOPORT_REG * v_pIOPregs;
PRIVATE volatile S3C2450_ADC_REG * v_pADCregs;
PRIVATE volatile S3C2450_INTR_REG * v_pINTregs;
PRIVATE volatile S3C2450_PWM_REG * v_pPWMregs;

PRIVATE BOOL	 bTSP_DownFlag;

VOID	TSP_VirtualFree(VOID);
BOOL	TSP_VirtualAlloc(VOID);

PRIVATE PVOID
TSP_RegAlloc(PVOID addr, INT sz)
{
	PVOID reg;

	reg = (PVOID)VirtualAlloc(0, sz, MEM_RESERVE, PAGE_NOACCESS);

	if (reg)
	{
		if (!VirtualCopy(reg, (PVOID)((UINT32)addr >> 8), sz, PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) 
		{
			VirtualFree(reg, 0, MEM_RELEASE);
			reg = NULL;
		}
	}

	return reg;
}


PRIVATE BOOL
TSP_VirtualAlloc(VOID)
{
	BOOL r = FALSE;

    RETAILMSG(0,(TEXT("::: TSP_VirtualAlloc()\r\n")));

	do
	{
		v_pIOPregs = (volatile S3C2450_IOPORT_REG *)TSP_RegAlloc((PVOID)S3C2450_BASE_REG_PA_IOPORT, sizeof(S3C2450_IOPORT_REG));
		if (v_pIOPregs == NULL) 
		{
			ERRORMSG(1,(TEXT("For IOPreg: VirtualAlloc failed!\r\n")));
			break;
		}
	
		v_pADCregs = (volatile S3C2450_ADC_REG *)TSP_RegAlloc((PVOID)S3C2450_BASE_REG_PA_ADC, sizeof(S3C2450_ADC_REG));
		if (v_pADCregs == NULL) 
		{
	    	ERRORMSG(1,(TEXT("For ADCreg: VirtualAlloc failed!\r\n")));
		    break;
		}

		v_pINTregs = (volatile S3C2450_INTR_REG *)TSP_RegAlloc((PVOID)S3C2450_BASE_REG_PA_INTR, sizeof(S3C2450_INTR_REG));
		if (v_pADCregs == NULL) 
		{
	    	ERRORMSG(1,(TEXT("For INTregs: VirtualAlloc failed!\r\n")));
		    break;
		}

		v_pPWMregs = (volatile S3C2450_PWM_REG *)TSP_RegAlloc((PVOID)S3C2450_BASE_REG_PA_PWM, sizeof(S3C2450_PWM_REG));
		if (v_pPWMregs == NULL) 
		{
	    	ERRORMSG(1,(TEXT("For PWMregs: VirtualAlloc failed!\r\n")));
		    break;
		}
		
		r = TRUE;
	} while (0);

	if (!r)
	{
		TSP_VirtualFree();

		RETAILMSG(0,(TEXT("::: TSP_VirtualAlloc() - Fail\r\n")));
	}
	else
	{
		RETAILMSG(0,(TEXT("::: TSP_VirtualAlloc() - Success\r\n")));
	}


	return r;
}

PRIVATE void
TSP_VirtualFree(VOID)
{
    RETAILMSG(0,(TEXT("::: TSP_VirtualFree()\r\n")));

	if (v_pIOPregs)
    {
        VirtualFree((PVOID)v_pIOPregs, 0, MEM_RELEASE);
        v_pIOPregs = NULL;
    }
    if (v_pADCregs)
    {   
        VirtualFree((PVOID)v_pADCregs, 0, MEM_RELEASE);
        v_pADCregs = NULL;
    } 
    if (v_pINTregs)
    {   
        VirtualFree((PVOID)v_pINTregs, 0, MEM_RELEASE);
        v_pINTregs = NULL;
    } 
    if (v_pPWMregs)
    {   
        VirtualFree((PVOID)v_pPWMregs, 0, MEM_RELEASE);
        v_pPWMregs = NULL;
    } 
}

PRIVATE VOID
TSP_SampleStart(VOID)
{
	DWORD tmp;

	if (v_pINTregs->INTMSK1 & (1<<IRQ_TIMER3)) InterruptDone(gIntrTouchChanged); 

	tmp = v_pPWMregs->TCON & (~(0xf << 16));

	v_pPWMregs->TCON = tmp | (2 << 16);		/* update TCVNTB3, stop					*/
	v_pPWMregs->TCON = tmp | (9 << 16);		/* interval mode,  start				*/
//	v_pPWMregs->TCON = tmp | (1 << 16);		/* one-shot mode,  start				*/
}

PRIVATE VOID
TSP_SampleStop(VOID)
{
	v_pPWMregs->TCON &= ~(1 << 16);			/* Timer3, stop							*/
}


PRIVATE VOID
TSP_PowerOn(VOID)
{
    RETAILMSG(TS_DBGON,(TEXT("::: TSP_PowerOn()\r\n")));


	RETAILMSG(TS_DBGON,(TEXT("Touch::ADCPRS=%d\r\n"),ADCPRS));


	 //[david.modify] 2008-08-12 18:11
//∏ƒ±‰¥À÷µª·”∞œÏCPUµƒ’º”√¬ 
//==========================
// 	v_pADCregs->ADCDLY=50000;                 //Normal conversion mode delay about (1/3.6864M)*50000=13.56ms
//waiting for samsung 
 //[david.modify] 2008-09-04 13:44
 
	v_pADCregs->ADCDLY=2000;                 //Normal conversion mode delay about (1/3.6864M)*50000=13.56ms
//	v_pADCregs->ADCDLY=500;                 //Normal conversion mode delay about (1/3.6864M)*50000=13.56ms
	
//DPNOK3(v_pADCregs->ADCDLY);	
#ifdef TOUCH_12BIT_SUPPORT	
	v_pADCregs->ADCCON = v_pADCregs->ADCCON & ~(0xffff) | (1<<14) | (ADCPRS<<6) | (1 <<3) | (0<<2);
#else
	v_pADCregs->ADCCON = v_pADCregs->ADCCON & ~(0xffff) | (1<<14) | (ADCPRS<<6) | (0<<2);
#endif
	v_pADCregs->ADCTSC = 0xd3;  //Wfait,XP_PU,XP_Dis,XM_Dis,YP_Dis,YM_En

    v_pINTregs->INTSUBMSK  &= ~(1<<IRQ_SUB_TC);
    
    v_pPWMregs->TCFG1  &= ~(0xf << 12);		/* Timer3's Divider Value				*/
#if( TSP_TIMER_DIVIDER == D2 )
    v_pPWMregs->TCFG1  |=  (D1_2   << 12);		/* 1/2									*/
#elif ( TSP_TIMER_DIVIDER == D4 )
    v_pPWMregs->TCFG1  |=  (D1_4   << 12);		/* 1/4	*/
#elif ( TSP_TIMER_DIVIDER == D8 )
    v_pPWMregs->TCFG1  |=  (D1_8   << 12);		/* 1/8	*/
#elif ( TSP_TIMER_DIVIDER == D16 )
    v_pPWMregs->TCFG1  |=  (D1_16   << 12);		/* 1/16	*/
#endif
    
  	
  	
    v_pPWMregs->TCNTB3  = TSP_SAMPLETICK_HIGH;	/* Interrupt Frequency					*/
	// v_pPWMregs->TCMPB3  = 0;		/**/
}

PRIVATE VOID
TSP_PowerOff(VOID)
{
    RETAILMSG(TS_DBGON,(TEXT("::: TSP_PowerOff()\r\n")));

    v_pINTregs->INTSUBMSK |= (1<<IRQ_SUB_TC);
}

PRIVATE BOOL
TSP_CalibrationPointGet(TPDC_CALIBRATION_POINT *pTCP)
{
    
	INT32	cDisplayWidth  = pTCP->cDisplayWidth;
	INT32	cDisplayHeight = pTCP->cDisplayHeight;

	int	CalibrationRadiusX = cDisplayWidth  / 20;
	int	CalibrationRadiusY = cDisplayHeight / 20;

	switch (pTCP -> PointNumber)
	{
	case	0:
		pTCP->CalibrationX = cDisplayWidth  / 2;
		pTCP->CalibrationY = cDisplayHeight / 2;
		break;

	case	1:
		pTCP->CalibrationX = CalibrationRadiusX * 2;
		pTCP->CalibrationY = CalibrationRadiusY * 2;
		break;

	case	2:
		pTCP->CalibrationX = CalibrationRadiusX * 2;
		pTCP->CalibrationY = cDisplayHeight - CalibrationRadiusY * 2;
		break;

	case	3:
		pTCP->CalibrationX = cDisplayWidth  - CalibrationRadiusX * 2;
		pTCP->CalibrationY = cDisplayHeight - CalibrationRadiusY * 2;
		break;

	case	4:
		pTCP->CalibrationX = cDisplayWidth - CalibrationRadiusX * 2;
		pTCP->CalibrationY = CalibrationRadiusY * 2;
		break;

	default:
		pTCP->CalibrationX = cDisplayWidth  / 2;
		pTCP->CalibrationY = cDisplayHeight / 2;

		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	
	RETAILMSG(TS_DBGON, (TEXT("::: TSP_CalibrationPointGet()\r\n")));
    RETAILMSG(TS_DBGON, (TEXT("cDisplayWidth        : %4d\r\n"), cDisplayWidth     ));
    RETAILMSG(TS_DBGON, (TEXT("cDisplayHeight       : %4d\r\n"), cDisplayHeight    ));
    RETAILMSG(TS_DBGON, (TEXT("CalibrationRadiusX   : %4d\r\n"), CalibrationRadiusX));
    RETAILMSG(TS_DBGON, (TEXT("CalibrationRadiusY   : %4d\r\n"), CalibrationRadiusY));
    RETAILMSG(TS_DBGON, (TEXT("pTCP -> PointNumber  : %4d\r\n"), pTCP->PointNumber));
    RETAILMSG(TS_DBGON, (TEXT("pTCP -> CalibrationX : %4d\r\n"), pTCP->CalibrationX));
    RETAILMSG(TS_DBGON, (TEXT("pTCP -> CalibrationY : %4d\r\n"), pTCP->CalibrationY));

	return TRUE;
}

#if 0
PRIVATE void
TSP_TransXY(INT *px, INT *py)
{
	INT TmpX, TmpY;
	TmpX = (*px >= TSP_MAXX) ? (TSP_MAXX) : *px;
	TmpY = (*py >= TSP_MAXY) ? (TSP_MAXY) : *py;
	
	TmpX -= TSP_MINX;
	TmpY -= TSP_MINY;
	
	if(TmpX <= 0)
	TmpX=0;
	if(TmpY <=0)
	TmpY=0;
	
	*px = TmpX * TSP_LCDX / (TSP_MAXX - TSP_MINX);
	*py = (/*LCD_YSIZE_TFT - */(TmpY * LCD_YSIZE_TFT) / (TSP_MAXY - TSP_MINY))*4;

	*px = (*px >= TSP_LCDX) ? TSP_LCDX -1 : *px;
	*py = (*py >= TSP_LCDY) ? TSP_LCDY -1 : *py;
	RETAILMSG(TS_DBGON,(TEXT("TSP_TransXY: *px=%d, *py=%d\r\n"),*px,*py));
}
#endif

PRIVATE BOOL
TSP_GetXY(INT *px, INT *py)
{
	INT i,j,k;
	//INT xsum, ysum, temp; //unused code, f.w.lin
	INT temp;
	INT x[TSP_SAMPLE_NUM], y[TSP_SAMPLE_NUM];
	INT dx, dy;
	
	temp = 0 ;
	//xsum = ysum = 0; //unused code, f.w.lin

	for (i = 0; i < TSP_SAMPLE_NUM; i++)
	{
		v_pADCregs->ADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(1<<2)|(0);			
		v_pADCregs->ADCCON |= (1 << 0);				/* Start Auto conversion				*/

		while (v_pADCregs->ADCCON & 0x1);				/* check if Enable_start is low			*/
		while (!(v_pADCregs->ADCCON & (1 << 15)));		/* Check ECFLG							*/

#ifdef TOUCH_12BIT_SUPPORT	
		y[i] = (0xfff & v_pADCregs->ADCDAT1);
		x[i] = (0xfff & v_pADCregs->ADCDAT0);
#else
		y[i] = (0x3ff & v_pADCregs->ADCDAT1);
		x[i] = (0x3ff & v_pADCregs->ADCDAT0);
#endif

	}
	
	for (j = 0; j < TSP_SAMPLE_NUM -1; ++j)
	{
		for (k = j+1; k < TSP_SAMPLE_NUM; ++k)
		{
			if(x[j]>x[k])	
			{
				temp = x[j];
				x[j]=x[k];
				x[k]=temp;
			}
			if(y[j]>y[k])	
			{
				temp = y[j];
				y[j]=y[k];
				y[k]=temp;
			}
		}
	}
	
	*px = (x[1]+x[2])>>1;
	*py = (y[1]+y[2])>>1;

	v_pADCregs->ADCTSC = (1<<8)|(1<<7)|(1<<6)|(0<<5)|(1<< 4)|(0<<3)|(0<<2)|(3);				
	dx = x[2]-x[1];
	dy = y[2]-y[1];
	
	RETAILMSG(TS_DBGON,(TEXT("TSP_GetXY: 1- x= %d, y=%d\r\n"),x[0],y[0]));
	RETAILMSG(TS_DBGON,(TEXT("TSP_GetXY: 2- x= %d, y=%d\r\n"),x[1],y[1]));
	RETAILMSG(TS_DBGON,(TEXT("TSP_GetXY: 3- x= %d, y=%d\r\n"),x[2],y[2]));
	RETAILMSG(TS_DBGON,(TEXT("TSP_GetXY: 4- x= %d, y=%d\r\n"),x[3],y[3]));
	RETAILMSG(TS_DBGON,(TEXT("TSP_GetXY: *px=%d, *py=%d\r\n"),*px,*py));  //junkim


	return ((dx > TSP_INVALIDLIMIT || dy > TSP_INVALIDLIMIT) ? FALSE : TRUE);
}

/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */

PUBLIC BOOL
DdsiTouchPanelGetDeviceCaps(INT	iIndex, LPVOID  lpOutput)
{

	if ( lpOutput == NULL )
	{
		ERRORMSG(1, (__TEXT("TouchPanelGetDeviceCaps: invalid parameter.\r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		DebugBreak();
		return FALSE;
	}
	


	switch	( iIndex )
	{
	case TPDC_SAMPLE_RATE_ID:
		{
			TPDC_SAMPLE_RATE	*pTSR = (TPDC_SAMPLE_RATE*)lpOutput;
			RETAILMSG(TS_DBGON, (TEXT("TouchPanelGetDeviceCaps::TPDC_SAMPLE_RATE_ID\r\n")));
            
			pTSR->SamplesPerSecondLow      = TSP_SAMPLE_RATE_LOW;
			pTSR->SamplesPerSecondHigh     = TSP_SAMPLE_RATE_HIGH;
			pTSR->CurrentSampleRateSetting = TSP_CurRate;
			RETAILMSG(TS_DBGON,(TEXT("SamplesPerSecondLow= %d, SamplesPerSecondHigh=%d, Setting=%d \r\n"),TSP_SAMPLE_RATE_LOW,TSP_SAMPLE_RATE_HIGH,TSP_CurRate));
		}
		break;

	case TPDC_CALIBRATION_POINT_COUNT_ID:
		{
			TPDC_CALIBRATION_POINT_COUNT *pTCPC = (TPDC_CALIBRATION_POINT_COUNT*)lpOutput;
			RETAILMSG(TS_DBGON, (TEXT("TouchPanelGetDeviceCaps::TPDC_CALIBRATION_POINT_COUNT_ID\r\n")));

			pTCPC->flags              = 0;
			pTCPC->cCalibrationPoints = 5;
		}
		break;

	case TPDC_CALIBRATION_POINT_ID:
		//RETAILMSG(TSP_CurRate, (TEXT("TouchPanelGetDeviceCaps::TPDC_CALIBRATION_POINT_ID\r\n")));
		return(TSP_CalibrationPointGet((TPDC_CALIBRATION_POINT*)lpOutput));

	default:
		ERRORMSG(1, (__TEXT("TouchPanelGetDeviceCaps: invalid parameter.\r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		DebugBreak();
		return FALSE;

	}

	return TRUE;
}

/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
PUBLIC BOOL
DdsiTouchPanelSetMode(INT iIndex, LPVOID  lpInput)
{
    BOOL  ReturnCode = FALSE;

	RETAILMSG(TS_DBGON, (TEXT("::: DdsiTouchPanelSetMode()\r\n")));

    switch ( iIndex )
    {
	case TPSM_SAMPLERATE_LOW_ID:
		TSP_CurRate = 0;

		v_pPWMregs->TCNTB3  = TSP_SAMPLETICK_LOW;

		SetLastError( ERROR_SUCCESS );
		ReturnCode = TRUE;
		RETAILMSG(TS_DBGON,(TEXT("TPSM_SAMPLERATE_LOW_ID\r\n")));
		break;
	case TPSM_SAMPLERATE_HIGH_ID:
		TSP_CurRate = 1;

		v_pPWMregs->TCNTB3  = TSP_SAMPLETICK_HIGH;

		SetLastError( ERROR_SUCCESS );
		RETAILMSG(TS_DBGON,(TEXT("TPSM_SAMPLERATE_HIGH_ID\r\n")));
		ReturnCode = TRUE;
		break;

	default:
		SetLastError( ERROR_INVALID_PARAMETER );
		break;
    }


    return ( ReturnCode );
}

/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */

PUBLIC BOOL
DdsiTouchPanelEnable(VOID)
{
    BOOL r;
    UINT32 Irq[3]={-1,OAL_INTR_FORCE_STATIC,0};

	r = TSP_VirtualAlloc();

	if (r) 
	{
		TSP_PowerOn();
	}

    // Obtain sysintr values from the OAL for the touch and touch changed interrupts.
    //
	//RETAILMSG(TS_DBGON, (TEXT("enable touch sysintr.\r\n")));
    Irq[0]=-1;Irq[1]=OAL_INTR_FORCE_STATIC;Irq[2]=IRQ_ADC;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(Irq), &gIntrTouch, sizeof(UINT32), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to request the touch sysintr.\r\n")));
        gIntrTouch = SYSINTR_UNDEFINED;
        return(FALSE);
    }
    Irq[0]=-1;Irq[1]=OAL_INTR_FORCE_STATIC;Irq[2]=IRQ_TIMER3;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(Irq), &gIntrTouchChanged, sizeof(UINT32), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to request the touch changed sysintr.\r\n")));
        gIntrTouchChanged = SYSINTR_UNDEFINED;
        return(FALSE);
    }


    return r;
}



/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */

PUBLIC VOID
DdsiTouchPanelDisable(VOID)
{
    if (v_pADCregs) 
	{
	    TSP_PowerOff();
		TSP_VirtualFree();
	}
}

/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */

LONG
DdsiTouchPanelAttach(VOID)
{
    return (1);
}


LONG
DdsiTouchPanelDetach(VOID)
{
    return (0);
}


/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
PUBLIC VOID
DdsiTouchPanelPowerHandler(BOOL	bOff)
{
	RETAILMSG(TS_DBGON, (TEXT("::: DdsiTouchPanelPowerHandler()\r\n")));
    if (bOff)
	{
        TSP_PowerOff();
    }
    else
	{
        TSP_PowerOn();
    }
}


/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */



PUBLIC VOID
DdsiTouchPanelGetPoint(TOUCH_PANEL_SAMPLE_FLAGS	* pTipStateFlags,
					   INT	* pUncalX,
					   INT	* pUncalY)
{
	static INT PrevX=0;
	static INT PrevY=0;
	INT TmpX = 0;
	INT TmpY = 0;
    
    RETAILMSG(TS_DBGON,(TEXT("DdsiTouchPanelGetPoint:: *pUncalX=%d, *pUncalY=%d\r\n"),*pUncalX,*pUncalY));
	if (v_pINTregs->SUBSRCPND & (1<<IRQ_SUB_TC))		/* SYSINTR_TOUCH Interrupt Case				*/
	{
		*pTipStateFlags = TouchSampleValidFlag;

		if ( (v_pADCregs->ADCDAT0 & (1 << 15)) ||
			 (v_pADCregs->ADCDAT1 & (1 << 15)) )
		{
			bTSP_DownFlag = FALSE;

			DEBUGMSG(ZONE_TIPSTATE, (TEXT("up\r\n")));

			RETAILMSG(TS_DBGON,(TEXT("up\r\n")));
			v_pADCregs->ADCTSC = 0xD3;

			*pUncalX = PrevX;
			*pUncalY = PrevY;
			
			RETAILMSG(TS_DBGON,(TEXT("*pUncalX=%d,*pUncalY=%d\r\n"),*pUncalX,*pUncalY));
			TSP_SampleStop();
		}
		else 
		{
			bTSP_DownFlag = TRUE;
			
			*pTipStateFlags |= TouchSampleIgnore;
			//if (!TSP_GetXY(&x, &y)) 
			//	*pTipStateFlags = TouchSampleIgnore;

			//TSP_TransXY(&x, &y);

			*pUncalX = PrevX;
			*pUncalY = PrevY;

			*pTipStateFlags |= TouchSampleDownFlag;

			//DEBUGMSG(ZONE_TIPSTATE, (TEXT("down %x %x\r\n"), x, y));
			RETAILMSG(TS_DBGON, (TEXT("down %d %d\r\n"), *pUncalX, *pUncalY));

			TSP_SampleStart();
		}

		v_pINTregs->SUBSRCPND  =  (1<<IRQ_SUB_TC);
		v_pINTregs->INTSUBMSK &= ~(1<<IRQ_SUB_TC);

		InterruptDone(gIntrTouch);
	}
	else		/* SYSINTR_TOUCH_CHANGED Interrupt Case		*/
	{
//		TSP_SampleStart();

		if ( (v_pADCregs->ADCDAT0 & (1 << 15)) ||
			 (v_pADCregs->ADCDAT1 & (1 << 15)) )
		{
			bTSP_DownFlag = FALSE;

			DEBUGMSG(ZONE_TIPSTATE, (TEXT("up\r\n")));

			RETAILMSG(TS_DBGON,(TEXT("up\r\n")));
			v_pADCregs->ADCTSC = 0xD3;

			*pTipStateFlags = TouchSampleValidFlag;
			*pUncalX = PrevX;
			*pUncalY = PrevY;
			
			RETAILMSG(TS_DBGON,(TEXT("*pUncalX=%d,*pUncalY=%d\r\n"),*pUncalX,*pUncalY));
			TSP_SampleStop();
		}		
		else if (bTSP_DownFlag)
		{

			if (!TSP_GetXY(&TmpX, &TmpY)) 
				*pTipStateFlags = TouchSampleIgnore;
			else 
			{
				//TSP_TransXY(&TmpX, &TmpY);

				if(Touch_Pen_Filtering(&TmpX, &TmpY))
				{
					RETAILMSG(TS_DBGON,(TEXT("Valid Touch Pen\r\n")));
					*pTipStateFlags = TouchSampleValidFlag | TouchSampleDownFlag;			
					*pTipStateFlags &= ~TouchSampleIgnore; //??? ªπ–Ë“™’‚æ‰ª∞¬Â?f.w.lin
				}
				else	// Invalid touch pen 
				{
		    		RETAILMSG(TS_DBGON, (TEXT("invalid touch pen\r\n")));
					*pTipStateFlags = TouchSampleValidFlag;
					*pTipStateFlags |= TouchSampleIgnore;				
				}		
				*pUncalX = PrevX = TmpX;
				*pUncalY = PrevY = TmpY;					
				RETAILMSG(TS_DBGON,(TEXT("*Final:*pUncalX=%d, *pUncalY=%d\r\n"),*pUncalX,*pUncalY));
			}
		}
		else
		{
			*pTipStateFlags = TouchSampleIgnore;
			RETAILMSG(TS_DBGON,(TEXT("TSP_SampleStop()\r\n")));
			TSP_SampleStop();
		}

		InterruptDone(gIntrTouchChanged);
	}
}

#define FILTER_LIMIT 25

static BOOL Touch_Pen_Filtering(INT *px, INT *py)
{
	BOOL RetVal = TRUE;
	// TRUE  : Valid pen sample
	// FALSE : Invalid pen sample
	static int count = 0;
	static INT x[2], y[2];
	INT TmpX, TmpY;
	INT dx, dy;
	

	count++;

	if (count > 2) 
	{ 
		// apply filtering rule
		count = 2;
		
		// average between x,y[0] and *px,y
		#if 0 //improve it, f.w.lin
		TmpX = (x[0] + *px) / 2;
		TmpY = (y[0] + *py) / 2;
		#else
		TmpX = (x[0] + *px) >> 1;
		TmpY = (y[0] + *py) >> 1;        
        #endif
		// difference between x,y[1] and TmpX,Y
		dx = (x[1] > TmpX) ? (x[1] - TmpX) : (TmpX - x[1]);
		dy = (y[1] > TmpY) ? (y[1] - TmpY) : (TmpY - y[1]);
		
		if ((dx > FILTER_LIMIT) || (dy > FILTER_LIMIT)) {

			// Invalid pen sample

			*px = x[1];
			*py = y[1]; // previous valid sample
			RetVal = FALSE;
			count = 0;
			
		} else {
			// Valid pen sample
			x[0] = x[1]; y[0] = y[1];		
			x[1] = *px; y[1] = *py; // reserve pen samples
			
			RetVal = TRUE;
		}
		
	} else { // till 2 samples, no filtering rule
	//’‚¿Ôø…“‘”≈ªØ,f.w.lin
		x[0] = x[1]; y[0] = y[1];		
		x[1] = *px; y[1] = *py; // reserve pen samples
		
		RetVal = FALSE;	// <- TRUE jylee 2003.03.04 
	}
	
	return RetVal;	
}


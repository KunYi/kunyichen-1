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
//  Module: timer.c           
//
//  Interface to OAL timer services.
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <s3c2450.h>
#include <bsp_cfg.h>

#define IRQFORTIMER		IRQ_TIMER4

extern void MMU_WaitForInterrupt();

#ifdef DVS_EN
extern void DVS_ON(void);
extern void DVS_OFF(void);
extern void ChangeVoltage(int, int*);
extern int GetCurrentVoltage(int);
extern	void HCLK_DOWNTO_PCLK();
extern	void HCLK_RECOVERYUP();

volatile int CurrentState;
DWORD dwCurrentidle;

DWORD dwPrevTotalTick1000 = 0;
DWORD dwPrevTotalTick100 = 0;

DWORD dwPrevIdleTick1000 = 0;
DWORD dwPrevIdleTick100 = 0;

DWORD dwPercentIdle1000 = 0;
DWORD dwPercentIdle100 = 0;
#endif
//------------------------------------------------------------------------------
// Local Variables 

static S3C2450_PWM_REG *g_pPWMRegs = NULL;

#if (BSP_TYPE == BSP_SMDK2443)
extern volatile S3C2450_INTR_REG *g_pIntrRegs; 
#elif (BSP_TYPE == BSP_SMDK2450)
volatile S3C2450_IOPORT_REG *g_pPortRegs;
volatile S3C2450_CLKPWR_REG *g_pClkpwrRegs;
volatile S3C2450_INTR_REG *g_pIntrRegs; 
#endif

UINT32	g_idleMSec;
//------------------------------------------------------------------------------
//
//  Global:  g_oalLastSysIntr
//
//  This global variable is set by fake version of interrupt/timer handler
//  to last SYSINTR value.
//
volatile UINT32 g_oalLastSysIntr; 

//------------------------------------------------------------------------------
//
//  Function: OALTimerInit
//
//  This function is typically called from the OEMInit to initialize
//  Windows CE system timer. The tickMSec parameter determine timer
//  period in milliseconds. On most platform timer period will be
//  1 ms, but it can be usefull to use higher value for some
//  specific (low-power) devices.
//
//  Implementation for s3c2450 is using timer 4 as system timer.
//
BOOL OALTimerInit(
    UINT32 msecPerSysTick, UINT32 countsPerMSec, UINT32 countsMargin
) {
    BOOL rc = FALSE;
    UINT32 countsPerSysTick;
    UINT32 sysIntr, irq;
    UINT32 tcon;
/*#undef	OAL_TIMER
#undef	OAL_FUNC
#define OAL_TIMER	1
#define	OAL_FUNC	1*/
    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit( %d, %d, %d )\r\n", 
        msecPerSysTick, countsPerMSec, countsMargin
    ));

    // Validate Input parameters
    countsPerSysTick = countsPerMSec * msecPerSysTick;
    if (
        msecPerSysTick < 1 || msecPerSysTick > 1000 ||
        countsPerSysTick < 1 || countsPerSysTick > 65535
    ) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: System tick period out of range..."
        ));
        goto cleanUp;
    }

    // Initialize timer state global variable    
    g_oalTimer.msecPerSysTick = msecPerSysTick;
    g_oalTimer.countsPerMSec = countsPerMSec;
    g_oalTimer.countsMargin = countsMargin;
    g_oalTimer.countsPerSysTick = countsPerSysTick;
    g_oalTimer.curCounts = 0;
    g_oalTimer.maxPeriodMSec = 0xFFFF/g_oalTimer.countsPerMSec;

	g_oalTimer.actualMSecPerSysTick = msecPerSysTick;
	g_oalTimer.actualCountsPerSysTick = countsPerSysTick;

    // Set kernel exported globals to initial values
    idleconv = countsPerMSec;
    curridlehigh = 0;
    curridlelow = 0;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter = OALTimerQueryPerformanceCounter;

    // Create SYSINTR for timer
    irq = IRQFORTIMER;
    sysIntr = OALIntrRequestSysIntr(1, &irq, OAL_INTR_FORCE_STATIC);

    // Hardware Setup
#if (BSP_TYPE == BSP_SMDK2443)
    g_pPWMRegs = (S3C2450_PWM_REG*)OALPAtoUA(S3C2450_BASE_REG_PA_PWM);
#elif (BSP_TYPE == BSP_SMDK2450)
    g_pPWMRegs = (volatile S3C2450_PWM_REG*)OALPAtoUA(S3C2450_BASE_REG_PA_PWM);
    g_pClkpwrRegs = (volatile S3C2450_CLKPWR_REG *)OALPAtoUA(S3C2450_BASE_REG_PA_CLOCK_POWER);
    g_pIntrRegs = (volatile S3C2450_INTR_REG *)OALPAtoUA(S3C2450_BASE_REG_PA_INTR);
    g_pPortRegs = (volatile S3C2450_IOPORT_REG *)OALPAtoUA(S3C2450_BASE_REG_PA_IOPORT);
#endif	
    // Set prescaler 1 to 1 
    OUTREG32(&g_pPWMRegs->TCFG0, INREG32(&g_pPWMRegs->TCFG0) & ~0x0000FF00);
	OUTREG32(&g_pPWMRegs->TCFG0, INREG32(&g_pPWMRegs->TCFG0) | (PRESCALER<<8));
    // Select MUX input 1/2
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) & ~(0xF << 16));
#if( SYS_TIMER_DIVIDER == DV2 )
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) | (D1_2 << 16));
#elif ( SYS_TIMER_DIVIDER == DV4 )
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) | (D1_4 << 16));
#elif ( SYS_TIMER_DIVIDER == DV8 )
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) | (D1_8 << 16));
#elif ( SYS_TIMER_DIVIDER == DV16 )
    OUTREG32(&g_pPWMRegs->TCFG1, INREG32(&g_pPWMRegs->TCFG1) | (D1_16 << 16));
#endif
    // Set timer register
    OUTREG32(&g_pPWMRegs->TCNTB4, g_oalTimer.countsPerSysTick);

    // Start timer in auto reload mode
    tcon = INREG32(&g_pPWMRegs->TCON) & ~(0x0F << 20);
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x2 << 20) );
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x5 << 20) );

    // Enable System Tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
        ));
        goto cleanUp;

    }

//    
// Define ENABLE_WATCH_DOG to enable watchdog timer support.
// NOTE: When watchdog is enabled, the device will reset itself if watchdog timer is not refreshed within ~4.5 second.
//       Therefore it should not be enabled when kernel debugger is connected, as the watchdog timer will not be refreshed.
//
#ifdef ENABLE_WATCH_DOG
    {
        extern void SMDKInitWatchDogTimer (void);
        SMDKInitWatchDogTimer ();
    }
#endif

    // Done        
    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", rc));
    return rc;
}

#ifdef DVS_EN
VOID ChangeSystemStateDVS()
{
	unsigned int dwCurrentMSec, dwCurrentIdleSec;
	//unsigned int PercentIdle;
	static unsigned int SlowCount = 0;

	dwCurrentMSec = CurMSec;
	dwCurrentIdleSec = dwCurrentidle;
	
	if ( dwCurrentMSec - dwPrevTotalTick100 > 100 )
	{
		dwPercentIdle100 = ((100*(dwCurrentIdleSec - dwPrevIdleTick100)) / (dwCurrentMSec - dwPrevTotalTick100));
		dwPrevTotalTick100 = dwCurrentMSec;
		dwPrevIdleTick100 = dwCurrentIdleSec;
		if ( dwPercentIdle100 < 20 )
		{
			if ( CurrentState == SlowActive)
			{
				if ( GetCurrentVoltage(ARM_VDD) == 0 ){
					RETAILMSG(1,(TEXT("Cannot get current voltage. State Freezing\n")));
					return;
				}
				if ( GetCurrentVoltage(ARM_VDD) != HIGHVOLTAGE )
				{
					// DVS OFF...
					int voltage_set[2] = HIGH_V_SET;
					ChangeVoltage(ARM_INT_VDD, voltage_set);
//					RETAILMSG(1, (TEXT("H")));					
				}
				DVS_OFF();
				g_oalIoCtlClockSpeed = S3C2450_FCLK;

				CurrentState = Active;
				SlowCount = 0;
//				RETAILMSG(1, (TEXT("-A-")));
			}
			if( CurrentState == LazyActive)		// Change state from SlowActive to Active...
			{
				if ( GetCurrentVoltage(ARM_VDD) == 0 ){
					RETAILMSG(1,(TEXT("Cannot get current voltage. State Freezing\n")));
					return;
				}
				if ( GetCurrentVoltage(ARM_VDD) != HIGHVOLTAGE )
				{
					// DVS OFF...
					int voltage_set[2] = HIGH_V_SET;
					ChangeVoltage(ARM_INT_VDD, voltage_set);
//					RETAILMSG(1, (TEXT("H")));					
				}
				DVS_OFF();
				g_oalIoCtlClockSpeed = S3C2450_FCLK;
				
				HCLK_RECOVERYUP();

				CurrentState = Active;
				SlowCount = 0;

//				RETAILMSG(1, (TEXT("-A-")));
			}
		}
	}
	if ( dwCurrentMSec - dwPrevTotalTick1000 > 1000 )
	{
		dwPercentIdle1000 = ((100*(dwCurrentIdleSec - dwPrevIdleTick1000)) / (dwCurrentMSec - dwPrevTotalTick1000));
		dwPrevTotalTick1000 = dwCurrentMSec;
		dwPrevIdleTick1000 = dwCurrentIdleSec;
//		RETAILMSG(1, (TEXT("%d,%d "), dwPercentIdle100, CurrentState));
		if ( dwPercentIdle1000 > 70 )
		{
			if ( CurrentState == Active)
			{
				int voltage_set[2] = MID_V_SET;
				DVS_ON();				
				ChangeVoltage(ARM_INT_VDD, voltage_set);
				g_oalIoCtlClockSpeed = S3C2450_HCLK;
				CurrentState = SlowActive;
//				RETAILMSG(1, (TEXT("-S-")));
			}
			else if ( CurrentState == SlowActive && ++SlowCount >= 1)
			{
				HCLK_DOWNTO_PCLK();
				g_oalIoCtlClockSpeed = S3C2450_PCLK;
				if ( GetCurrentVoltage(ARM_VDD) == 0 ){
					RETAILMSG(1,(TEXT("Cannot get current voltage. State Freezing\n")));
					return;
				}
				if ( GetCurrentVoltage(ARM_VDD) == MIDVOLTAGE ){
					int voltage_set[2] = LOW_V_SET;
					ChangeVoltage(ARM_INT_VDD, voltage_set);
//					RETAILMSG(1, (TEXT("L")));
				}
				
				CurrentState = LazyActive;
				RETAILMSG(0, (TEXT("-L-")));
			}
		}
	}
}
#endif	// DVS_EN
//------------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implement timer interrupt handler. It is called from common
//  ARM interrupt handler.
//
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr = SYSINTR_NOP;

#if (BSP_TYPE == BSP_SMDK2443)
#if 1// for WFI

        OUTREG32(&g_pIntrRegs->SRCPND, 1 << IRQ_TIMER4);
        OUTREG32(&g_pIntrRegs->INTPND, 1 << IRQ_TIMER4);
#endif	
#elif (BSP_TYPE == BSP_SMDK2450)
    static DWORD HeartBeatCnt, HeartBeatStat;  //LED4 is used for heart beat	
        // Clear the interrupt
        OUTREG32(&g_pIntrRegs->SRCPND1, 1 << IRQFORTIMER);
        OUTREG32(&g_pIntrRegs->INTPND1, 1 << IRQFORTIMER);

#ifndef DVS_EN
 //[david.modify] 2008-09-09 17:30
 // GPF4在4.3INCH项目上用做MENU_KEY
#if 0
		if (++HeartBeatCnt > 100)
		{
			HeartBeatCnt   = 0;
			HeartBeatStat ^= 1;
			g_pPortRegs->GPCCON = (g_pPortRegs->GPCCON & ~(3<<14)) | (1<<14); 
			if (HeartBeatStat) 
			{
				g_pPortRegs->GPCDAT &= ~(1<<7); // LED 4 Off
			}
			else
			{
				g_pPortRegs->GPCDAT |=  (1<<7); // LED 4 On
			}			
		}
#endif

#endif	
#endif	
    // Update high resolution counter
    g_oalTimer.curCounts += g_oalTimer.countsPerSysTick;
                             
    // Update the millisecond counter
    CurMSec += g_oalTimer.msecPerSysTick;


    // Reschedule?
    if ((int)(CurMSec - dwReschedTime) >= 0) sysIntr = SYSINTR_RESCHED;

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        if (--g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }
#endif

#ifdef DVS_EN
	ChangeSystemStateDVS();
#endif
    return sysIntr;
}


//------------------------------------------------------------------------------
//
//  Function: OALTimerCountsSinceSysTick
//
//  This function return count of hi res ticks since system tick.
//
//  Timer 4 counts down, so we should substract actual value from 
//  system tick period.
//

INT32 OALTimerCountsSinceSysTick()
{
	return (g_oalTimer.countsPerSysTick - INREG32(&g_pPWMRegs->TCNTO4));
}

//------------------------------------------------------------------------------
//
//  Function: OALTimerUpdate
//
//  This function is called to change length of actual system timer period.
//  If end of actual period is closer than margin period isn't changed (so
//  original period elapse). Function returns time which already expires
//  in new period length units. If end of new period is closer to actual time
//  than margin period end is shifted by margin (but next period should fix
//  this shift - this is reason why OALTimerRecharge doesn't read back 
//  compare register and it uses saved value instead).
//
UINT32 OALTimerUpdate(UINT32 period, UINT32 margin)
{

#if (BSP_TYPE == BSP_SMDK2443)
	UINT32 tcon, ret;
	
	ret = OALTimerCountsSinceSysTick();

	OUTREG32(&g_pPWMRegs->TCNTB4, period);
    tcon = INREG32(&g_pPWMRegs->TCON) & ~(0x0F << 20);
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x2 << 20) );
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x5 << 20) );

	return (ret);

#elif (BSP_TYPE == BSP_SMDK2450)

#if	0	// Fixed Tick do not Update TImer
	UINT32 tcon, ret;
	
	ret = OALTimerCountsSinceSysTick();

	OUTREG32(&g_pPWMRegs->TCNTB4, period);
    tcon = INREG32(&g_pPWMRegs->TCON) & ~(0x0F << 20);
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x2 << 20) );
    OUTREG32(&g_pPWMRegs->TCON, tcon | (0x5 << 20) );

	return (ret);
#else
	return	0;
#endif

#endif
}
//------------------------------------------------------------------------------
//
//  Function:   OEMIdle
//
//  This Idle function implements a busy idle. It is intend to be used only
//  in development (when CPU doesn't support idle mode it is better to stub
//  OEMIdle function instead use this busy loop). The busy wait is cleared by
//  an interrupt from interrupt handler setting the g_oalLastSysIntr.
//

//
void OEMIdle(DWORD idleParam)
{
   UINT32 baseMSec;
    INT32 usedCounts, idleCounts;
    ULARGE_INTEGER idle;

    // Get current system timer counter
    baseMSec = CurMSec;
    
    // Find how many hi-res ticks was already used
    usedCounts = OALTimerCountsSinceSysTick();
	if (usedCounts == g_oalTimer.countsPerSysTick)
	{
		return;
	}
    // We should wait this time
    idleCounts = g_oalTimer.actualCountsPerSysTick;
    
    // Move SoC/CPU to idle mode
    OALCPUIdle();

    // When there wasn't timer interrupt modify idle time
    if (CurMSec == baseMSec) {
        idleCounts = OALTimerCountsSinceSysTick();
    }

    // Get real idle value. If result is negative we didn't idle at all.
    idleCounts -= usedCounts;
    if (idleCounts < 0) idleCounts = 0;

    // Update idle counters
    idle.LowPart = curridlelow;
    idle.HighPart = curridlehigh;
    idle.QuadPart += idleCounts;
    curridlelow  = idle.LowPart;
    curridlehigh = idle.HighPart;

#ifdef DVS_EN
	dwCurrentidle = (DWORD)(idle.QuadPart/idleconv);
#endif
}


//------------------------------------------------------------------------------
//
//  Function:   OALCPUIdle
//
//  This Idle function implements a busy idle. It is intend to be used only
//  in development (when CPU doesn't support idle mode it is better to stub
//  OEMIdle function instead use this busy loop). The busy wait is cleared by
//  an interrupt from interrupt handler setting the g_oalLastSysIntr.
//
//
VOID OALCPUIdle()
{

#if (BSP_TYPE == BSP_SMDK2443)
    volatile S3C2450_CLKPWR_REG *s2450CLKPWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);
    volatile S3C2450_INTR_REG *s2450INTR = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);
    volatile S3C2450_IOPORT_REG *s2450IOPORT = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);

	UINT32 sysIntr;
  

	s2450IOPORT->GPFDAT |= ( 1<< 5);    // GPF5 output data, turn LED on
	s2450CLKPWR->PWRMODE |=  (1 << 17);
	
	MMU_WaitForInterrupt();


	s2450CLKPWR->PWRMODE &=  ~(1 << 17);
	s2450IOPORT->GPFDAT &= ~(1 << 5);    // GPF5 output data, turn LED off
	if(g_pIntrRegs->SRCPND & (1<< IRQ_TIMER4))
		sysIntr = OALTimerIntrHandler();

#elif (BSP_TYPE == BSP_SMDK2450)
#ifndef DVS_EN

//[david.modify] 2008-09-06 19:01
//GPC0 在开发板上用做LED灯指示
//但在我们4.3INCH PND的项目上用做了LCD_ON信号给LCD供电
// 一定要为高

#if 0
	g_pPortRegs->GPCCON &= ~(3<< 0);
	g_pPortRegs->GPCCON |= ( 1<< 0);    
	g_pPortRegs->GPCDAT |= ( 1<< 0);    
#endif
	
#endif
	g_pClkpwrRegs->PWRCFG |= (0x1<<17);  
	g_pClkpwrRegs->PWRCFG &=~(0x3<<5);

	MMU_WaitForInterrupt();

	g_pClkpwrRegs->PWRCFG &= ~(0x1<<17);  
	
	if((g_pIntrRegs->SRCPND1 & (1<<IRQFORTIMER)) != 0)
	{
		OALTimerIntrHandler();		
	}
#ifndef DVS_EN	

#if 0
	g_pPortRegs->GPCDAT &= ~(1 << 0);   
#endif


#endif
#endif
}


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
#include <nkintr.h>
#include <ceddk.h>
#include <s3c2450.h>
#include "SDHC_s3c2450_ch0.h"

#include <cebuscfg.h>

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
#define _DBG_ 0

#ifdef _SMDK2450_CH0_EXTCD_
// 08.04.04 by KYS
// New Constructor for card detect of HSMMC ch0 on SMDK2450.
CSDHController::CSDHController() : CSDHCBase() { 
	m_htCardDetectThread = NULL; 
	m_hevCardDetectEvent = NULL;  
	m_dwSDDetectSysIntr = SYSINTR_UNDEFINED;   
}
#endif

#endif  // !(BSP_TYPE == BSP_SMDK2443)


BOOL 
CSDHController::Init(
    LPCTSTR pszActiveKey
    )
{
#if (BSP_TYPE == BSP_SMDK2443)
  S3C2450_CLKPWR_REG *pCLKPWR;
  S3C2450_HSMMC_REG *pHSMMC;
  S3C2450_IOPORT_REG *pIOPreg;

  RETAILMSG(0,(TEXT("CSDHController::Init\n")));

  //GPIO Setting
  //----- 1. Map the GPIO registers needed to enable the SDI controller -----
  pIOPreg = (S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pIOPreg == NULL) 
  {
    RETAILMSG (1,(TEXT("GPIO registers not allocated\r\n")));
    return FALSE;
  }
  if (!VirtualCopy((PVOID)pIOPreg, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
    RETAILMSG (1,(TEXT("GPIO registers not mapped\r\n")));

    return FALSE;
  }

  pCLKPWR = (S3C2450_CLKPWR_REG *)VirtualAlloc(0, sizeof(S3C2450_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pCLKPWR == NULL) 
  {
    RETAILMSG (1,(TEXT("Clock & Power Management Special Register not allocated\n\r")));
    return FALSE;
  }
  if (!VirtualCopy((PVOID)pCLKPWR, (PVOID)(S3C2450_BASE_REG_PA_CLOCK_POWER >> 8), sizeof(S3C2450_CLKPWR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
    RETAILMSG (1,(TEXT("Clock & Power Management Special Register not mapped\n\r")));
    return FALSE;
  }

  pHSMMC = (S3C2450_HSMMC_REG *)VirtualAlloc(0, sizeof(S3C2450_HSMMC_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pHSMMC == NULL) 
  {
    RETAILMSG (1,(TEXT("HSMMC Special Register not allocated\n\r")));
    return FALSE;
  }
  if (!VirtualCopy((PVOID)pHSMMC, (PVOID)(S3C2450_BASE_REG_PA_HSMMC >> 8), sizeof(S3C2450_HSMMC_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
    RETAILMSG (1,(TEXT("HSMMC Special Register not mapped\n\r")));
    return FALSE;
  }	

  //	pIOPreg->GPFCON &= ~(0x3<<14);
  //	pIOPreg->GPFCON |= (0x1<<14);	

  pIOPreg->GPLCON &= ~(0xFFFFF);
  pIOPreg->GPLCON |= (0xAAAAA);
  //	pIOPreg->GPLUDP &= ~(0xFFFFF);
  //	pIOPreg->GPLUDP |= (0xAAAAA);

  pIOPreg->GPJCON &= ~(0x3F << 26);
  pIOPreg->GPJCON |= (0x2A << 26);

  //	pIOPreg->GPJUDP &= ~(0x3F << 26);
  //	pIOPreg->GPJUDP |= (0x2A << 26);

  pHSMMC->CONTROL2 = (pHSMMC->CONTROL2 & ~(0xffffffff)) | (0x1<<15)|(0x1<<14)|(0x1<<8)|(0x2/*EPLL*/<<4);
  pHSMMC->CONTROL3 = (0<<31) | (1<<23) | (0<<15) | (1<<7);
  RETAILMSG(0,(TEXT("pHSMMC->CONTROL2 = 0x%X\n"),pHSMMC->CONTROL2));
  pCLKPWR->HCLKCON |= (0x1<<16);
  pCLKPWR->SCLKCON |= (0x1<<13);	
  pCLKPWR->SCLKCON |= (0x1<<12);
  //	pCLKPWR->CLKDIV1 |= (0x3<<6);

  VirtualFree((PVOID) pIOPreg, 0, MEM_RELEASE);
  VirtualFree((PVOID) pCLKPWR, 0, MEM_RELEASE);
  VirtualFree((PVOID) pHSMMC, 0, MEM_RELEASE);

#elif (BSP_TYPE == BSP_SMDK2450)
  RETAILMSG(1, (TEXT("[HSMMC0] Initializing the HSMMC Host Controller\n")));
  // 08.03.13 by KYS
  // HSMMC Ch1 initialization
  if (!InitCh()) return FALSE;
#endif  // !(BSP_TYPE == BSP_SMDK2443)
  return CSDHCBase::Init(pszActiveKey);
}

  void
CSDHController::PowerUp()
{
#if (BSP_TYPE == BSP_SMDK2443)
  S3C2450_CLKPWR_REG *pCLKPWR;
  S3C2450_HSMMC_REG *pHSMMC;
  S3C2450_IOPORT_REG *pIOPreg;
  S3C2450_INTR_REG *pINTreg;

  //GPIO Setting
  //----- 1. Map the GPIO registers needed to enable the SDI controller -----
  pIOPreg = (S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pIOPreg == NULL) 
  {
    RETAILMSG (1,(TEXT("GPIO registers not allocated\r\n")));
    return;
  }
  if (!VirtualCopy((PVOID)pIOPreg, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
    RETAILMSG (1,(TEXT("GPIO registers not mapped\r\n")));

    return;
  }

  pCLKPWR = (S3C2450_CLKPWR_REG *)VirtualAlloc(0, sizeof(S3C2450_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pCLKPWR == NULL) 
  {
    RETAILMSG (1,(TEXT("Clock & Power Management Special Register not allocated\n\r")));
    return;
  }
  if (!VirtualCopy((PVOID)pCLKPWR, (PVOID)(S3C2450_BASE_REG_PA_CLOCK_POWER >> 8), sizeof(S3C2450_CLKPWR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
    RETAILMSG (1,(TEXT("Clock & Power Management Special Register not mapped\n\r")));
    return;
  }

  pHSMMC = (S3C2450_HSMMC_REG *)VirtualAlloc(0, sizeof(S3C2450_HSMMC_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pHSMMC == NULL) 
  {
    RETAILMSG (1,(TEXT("HSMMC Special Register not allocated\n\r")));
    return;
  }
  if (!VirtualCopy((PVOID)pHSMMC, (PVOID)(S3C2450_BASE_REG_PA_HSMMC >> 8), sizeof(S3C2450_HSMMC_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
    RETAILMSG (1,(TEXT("HSMMC Special Register not mapped\n\r")));
    return;
  }	

  pINTreg = (S3C2450_INTR_REG *)VirtualAlloc(0, sizeof(S3C2450_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pINTreg == NULL) 
  {
    RETAILMSG (1,(TEXT("HSMMC Special Register not allocated\n\r")));
    return;
  }
  if (!VirtualCopy((PVOID)pINTreg, (PVOID)(S3C2450_BASE_REG_PA_INTR >> 8), sizeof(S3C2450_INTR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
    RETAILMSG (1,(TEXT("HSMMC Special Register not mapped\n\r")));
    return;
  }    

  //	pIOPreg->GPFCON &= ~(0x3<<14);
  //	pIOPreg->GPFCON |= (0x1<<14);	

  pIOPreg->GPLCON &= ~(0xFFFFF);
  pIOPreg->GPLCON |= (0xAAAAA);
  //	pIOPreg->GPLUDP &= ~(0xFFFFF);
  //	pIOPreg->GPLUDP |= (0x55555);

  pIOPreg->GPJCON &= ~(0x3F << 26);
  pIOPreg->GPJCON |= (0x2A << 26);
  //	pIOPreg->GPJUDP &= ~(0x3F << 26);
  //	pIOPreg->GPJUDP |= (0x15 << 26);

  pHSMMC->CONTROL2 = (pHSMMC->CONTROL2 & ~(0xffffffff)) | (0x1<<15)|(0x1<<14)|(0x1<<8)|(0x2/*EPLL*/<<4);
  pHSMMC->CONTROL3 = (0<<31) | (1<<23) | (0<<15) | (1<<7);

  pCLKPWR->HCLKCON |= (0x1<<16);
  pCLKPWR->SCLKCON |= (0x1<<13);	
  pCLKPWR->SCLKCON |= (0x1<<12);	

  RETAILMSG(0,(TEXT("HSMMC!!!!!!!!!!! pINTreg->INTMSK = 0x%08X\n"),pINTreg->INTMSK));
  RETAILMSG(0,(TEXT("HSMMC!!!!!!!!!!! pHSMMC->NORINTSTSEN = 0x%08X\n"),pHSMMC->NORINTSTSEN));


  VirtualFree((PVOID) pINTreg, 0, MEM_RELEASE);
  VirtualFree((PVOID) pIOPreg, 0, MEM_RELEASE);
  VirtualFree((PVOID) pCLKPWR, 0, MEM_RELEASE);
  VirtualFree((PVOID) pHSMMC, 0, MEM_RELEASE);	

#elif (BSP_TYPE == BSP_SMDK2450)
  RETAILMSG(1, (TEXT("[HSMMC0] Power Up the HSMMC Host Controller\n")));
  // 08.03.13 by KYS
  // HSMMC Ch1 initialization for "WakeUp"
  if (!InitCh()) return;
#endif  // !(BSP_TYPE == BSP_SMDK2443)
  CSDHCBase::PowerUp();
}

extern "C"
PCSDHCBase
CreateHSMMCHCObject(
    )
{
  return new CSDHController;
}


VOID 
CSDHController::DestroyHSMMCHCObject(
    PCSDHCBase pSDHC
    )
{
  DEBUGCHK(pSDHC);
  delete pSDHC;
}

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
BOOL CSDHController::InitClkPwr() {
  volatile S3C2450_CLKPWR_REG *pCLKPWR = NULL;

  pCLKPWR = (volatile S3C2450_CLKPWR_REG *)VirtualAlloc(0, sizeof(S3C2450_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pCLKPWR == NULL) {
    RETAILMSG(_DBG_,(TEXT("[HSMMC0] Clock & Power Management Special Register is *NOT* allocated.\n")));
    return FALSE;
  }
  if (!VirtualCopy((PVOID)pCLKPWR, (PVOID)(S3C2450_BASE_REG_PA_CLOCK_POWER >> 8),
        sizeof(S3C2450_CLKPWR_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE)) {
    RETAILMSG(_DBG_,(TEXT("[HSMMC0] Clock & Power Management Special Register is *NOT* mapped.\n")));
    return FALSE;
  }

  RETAILMSG(_DBG_, (TEXT("[HSMMC0] Setting registers for the EPLL (for SDCLK) : SYSCon.\n")));
  pCLKPWR->CLKSRC  |= (0x0<<16);	// HSMMC0 clock: 0 = EPLL, 1 = USB 48MHz
  pCLKPWR->HCLKCON |= (0x1<<15);	// Enable HCLK into the HSMMC0
  pCLKPWR->SCLKCON |= (0x0<<13);	// Disable HSMMC_EXT clock for HSMMC 0,1 (EXTCLK)
  pCLKPWR->SCLKCON |= (0x1<<6);	// Enable HSMMC_0 clock for (EPLL/USB48M)
  VirtualFree((PVOID) pCLKPWR, 0, MEM_RELEASE);
  return TRUE;
}

BOOL CSDHController::InitGPIO() {
  volatile S3C2450_IOPORT_REG *pIOPreg = NULL;

  pIOPreg = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pIOPreg == NULL) {
    RETAILMSG(_DBG_,(TEXT("[HSMMC0] GPIO registers is *NOT* allocated.\n")));
    return FALSE;
  }
  if (!VirtualCopy((PVOID)pIOPreg, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8),
        sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE)) {
    RETAILMSG(_DBG_,(TEXT("[HSMMC0] GPIO registers is *NOT* mapped.\n")));
    return FALSE;
  }

  RETAILMSG(_DBG_, (TEXT("[HSMMC0] Setting registers for the GPIO.\n")));
  pIOPreg->GPECON &= ~(0xFFF<<10);
  pIOPreg->GPECON |=  (0xAAA<<10);  // SD0_DAT[3:0], SD0_CMD, SD0_CLK
  pIOPreg->GPEUDP &= ~(0xFFF<<10);  // pull-up/down disabled

#ifdef _SMDK2450_CH0_EXTCD_
  // 08.04.04 by KYS
	// Setting for card detect pin of HSMMC ch0 on SMDK2450.
	pIOPreg->GPFCON    = ( pIOPreg->GPFCON & ~(0x3<<2) ) | (0x2<<2);	// SD_CD0 by EINT1(GPF1)
	pIOPreg->GPFUDP		 = ( pIOPreg->GPFUDP & ~(0x3<<2) ) | (0x0<<2);  // pull-up/down disabled

  pIOPreg->EXTINT0   = ( pIOPreg->EXTINT0 & ~(0xF<<4) ) | (0x6<<4); // 4b'0110 : Filter Enable, Both edge triggered
#endif

#ifdef _SMDK2450_CH0_WP_
	  //
	  // Here for the Setting code on WriteProtection!!
	  // Refer to InitGPIO() in HSMMC_ch1\HSMMC_ch1\sdhc_s3c2450_ch1.cpp file
	  //
#endif

  VirtualFree((PVOID) pIOPreg, 0, MEM_RELEASE);
  return TRUE;
}

BOOL CSDHController::InitHSMMC() {
  volatile S3C2450_HSMMC_REG *pHSMMC = NULL;

  pHSMMC = (volatile S3C2450_HSMMC_REG *)VirtualAlloc(0, sizeof(S3C2450_HSMMC_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pHSMMC == NULL) 
  {
    RETAILMSG(_DBG_,(TEXT("[HSMMC0] HSMMC Special Register is *NOT* allocated.\n")));
    return FALSE;
  }
  if (!VirtualCopy((PVOID)pHSMMC, (PVOID)(S3C2450_BASE_REG_PA_HSMMC0 >> 8),
        sizeof(S3C2450_HSMMC_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE)) {
    RETAILMSG(_DBG_,(TEXT("[HSMMC0] HSMMC Special Register is *NOT* mapped.\n")));
    return FALSE;
  }

  RETAILMSG(_DBG_, (TEXT("[HSMMC0] Setting registers for the EPLL : HSMMCCon.\n")));
  pHSMMC->CONTROL2 = (pHSMMC->CONTROL2 & ~(0xffffffff)) |	// clear all
    (0x1<<15) |	// enable Feedback clock for Tx Data/Command Clock
    (0x1<<14) |	// enable Feedback clock for Rx Data/Command Clock
    (0x3<<9)  |	// Debounce Filter Count 0x3=64 iSDCLK
    (0x1<<8)  |	// SDCLK Hold Enable
    (0x2<<4);	// Base Clock Source = EPLL (from SYSCON block)
  pHSMMC->CONTROL3 = (0<<31) | (1<<23) | (0<<15) | (1<<7);	// controlling the Feedback Clock

  VirtualFree((PVOID) pHSMMC, 0, MEM_RELEASE);
  return TRUE;
}

BOOL CSDHController::InitCh() {

  if (!InitClkPwr()) return FALSE;
  if (!InitGPIO()) return FALSE;
  if (!InitHSMMC()) return FALSE;
  return TRUE;
}

#ifdef _SMDK2450_CH0_EXTCD_
// 08.04.04 by KYS
// New function to Card detect thread of HSMMC ch0 on SMDK2450.
DWORD CSDHController::CardDetectThread() {
	BOOL  bSlotStateChanged = FALSE;
	DWORD dwWaitResult  = WAIT_TIMEOUT;
	PCSDHCSlotBase pSlotZero = GetSlot(0);

	CeSetThreadPriority(GetCurrentThread(), 100);

	while(1) {
		// Wait for the next insertion/removal interrupt
		dwWaitResult = WaitForSingleObject(m_hevCardDetectEvent, INFINITE);

		Lock();
		pSlotZero->HandleInterrupt(SDSLOT_INT_CARD_DETECTED);
		Unlock();
		InterruptDone(m_dwSDDetectSysIntr);

		EnableCardDetectInterrupt();
	}

	return TRUE;
}

// 08.04.04 by KYS
// New function to request a SYSINTR for Card detect interrupt of HSMMC ch0 on SMDK2450.
BOOL CSDHController::InitializeHardware() {
	m_dwSDDetectIrq = SD_CD0_IRQ;

	// convert the SDI hardware IRQ into a logical SYSINTR value
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &m_dwSDDetectIrq, sizeof(DWORD), &m_dwSDDetectSysIntr, sizeof(DWORD), NULL)) {
		// invalid SDDetect SYSINTR value!
		RETAILMSG(1, (TEXT("[HSMMC0] invalid SD detect SYSINTR value!\n")));
		m_dwSDDetectSysIntr = SYSINTR_UNDEFINED;
		return FALSE;
	}

	return CSDHCBase::InitializeHardware();
}

// 08.04.04 by KYS
// New Start function for Card detect of HSMMC ch0 on SMDK2450.
SD_API_STATUS CSDHController::Start() {
	SD_API_STATUS status = SD_API_STATUS_INSUFFICIENT_RESOURCES;

	m_fDriverShutdown = FALSE;

	// allocate the interrupt event
	m_hevInterrupt = CreateEvent(NULL, FALSE, FALSE,NULL);

	if (NULL == m_hevInterrupt)	{
		goto EXIT;
	}

	// initialize the interrupt event
	if (!InterruptInitialize (m_dwSysIntr, m_hevInterrupt, NULL, 0)) {
		goto EXIT;
	}

	m_fInterruptInitialized = TRUE;

	// create the interrupt thread for controller interrupts
	m_htIST = CreateThread(NULL, 0, ISTStub, this, 0, NULL);
	if (NULL == m_htIST) {
		goto EXIT;
	}

	// allocate the card detect event
	m_hevCardDetectEvent = CreateEvent(NULL, FALSE, FALSE,NULL);

	if (NULL == m_hevCardDetectEvent) {
		goto EXIT;
	}

	// initialize the interrupt event
	if (!InterruptInitialize (m_dwSDDetectSysIntr, m_hevCardDetectEvent, NULL, 0)) {
		goto EXIT;
	}

	// create the card detect interrupt thread   
	m_htCardDetectThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SD_CardDetectThread, this, 0, NULL);
	if (NULL == m_htCardDetectThread)	{
		goto EXIT;
	}

	for (DWORD dwSlot = 0; dwSlot < m_cSlots; ++dwSlot) {
		PCSDHCSlotBase pSlot = GetSlot(dwSlot);
		status = pSlot->Start();

		if (!SD_API_SUCCESS(status)) {
			goto EXIT;
		}
	}

	// wake up the interrupt thread to check the slot
	::SetInterruptEvent(m_dwSDDetectSysIntr);   

	status = SD_API_STATUS_SUCCESS;

EXIT:
	if (!SD_API_SUCCESS(status)) {
		// Clean up
		Stop();
	}

	return status;
}

// 08.04.04 by KYS
// New function for enabling the Card detect interrupt of HSMMC ch0 on SMDK2450.
BOOL CSDHController::EnableCardDetectInterrupt() {
  volatile S3C2450_INTR_REG *pINTRreg = NULL;
  pINTRreg = (volatile S3C2450_INTR_REG *)VirtualAlloc(0, sizeof(S3C2450_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
  if (pINTRreg == NULL) {
    RETAILMSG(_DBG_,(TEXT("[HSMMC0] INTR registers is *NOT* allocated.\n")));
    return FALSE;
  }
  if (!VirtualCopy((PVOID)pINTRreg, (PVOID)(S3C2450_BASE_REG_PA_INTR >> 8),
        sizeof(S3C2450_INTR_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE)) {
    RETAILMSG(_DBG_,(TEXT("[HSMMC0] INTR registers is *NOT* mapped.\n")));
    return FALSE;
  }
  
	//pINTRreg->SRCPND1 = ( pINTRreg->SRCPND1 |  (0x1<<1));     //clear EINT1SRC pending bit 
	pINTRreg->INTPND1 = ( pINTRreg->INTPND1 |  (0x1<<1));     //clear EINT1 pending bit 
	pINTRreg->INTMSK1 = ( pINTRreg->INTMSK1 & ~(0x1<<1));     //enable EINT19

  VirtualFree((PVOID)pINTRreg, 0, MEM_RELEASE);
	return TRUE;
}
#endif

#endif  // !(BSP_TYPE == BSP_SMDK2443)


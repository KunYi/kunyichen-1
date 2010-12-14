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
//  File:  intr.h
//
//  This file contains SMDK2450 board specific interrupt code. The board uses
//  GPG1 pin as interrupt for CS8900A ethernet chip.
//
#include <bsp.h>
 //[david.modify] 2008-05-16 14:21
#include <bsp_bitsdef_david.h>
#include <dbgmsg_david.h>


 //[david.modify] 2008-05-16 14:21
UINT32 IO_InterruptHandler8_23(UINT32 irq);
UINT32 IO_InterruptHandler4_7(UINT32 irq);
#define KEYPAD_DBG 		1
//=======================================
volatile S3C2450_IOPORT_REG *pOalPortRegs;
volatile S3C2450_INTR_REG *pIntrRegs; 
volatile BSP_ARGS *g_pBspArgs;


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrInit
//
BOOL BSPIntrInit()
{
//    S3C2450_IOPORT_REG *pOalPortRegs;

//    ULONG value;

    OALMSG(OAL_INTR&&OAL_FUNC, (L"+BSPIntrInit\r\n"));
    
    // Then get virtual address for IO port
    pOalPortRegs = OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    pIntrRegs = OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);
    g_pBspArgs = OALPAtoVA(IMAGE_SHARE_ARGS_PA_START, FALSE);	


    
#ifdef 	KITL_ETHERNET
    // Set GPG5 as EINT13 for CS8900A
    value = INREG32(&pOalPortRegs->GPGCON);
    OUTREG32(&pOalPortRegs->GPGCON, (value & ~(3 << 10))|(2 << 10));

    // Disable pullup
    //value = INREG32(&pOalPortRegs->GPGUDP);
    //OUTREG32(&pOalPortRegs->GPGUDP, value | (1 << 10));
	
	// Disable pullup and
    // High level interrupt
    value = INREG32(&pOalPortRegs->EXTINT1);
    OUTREG32(&pOalPortRegs->EXTINT1, (value & ~(0xf << 20))|((0x1<<23) | (0x1 << 20)));
#endif
	
	RETAILMSG(1,(L"+BSPIntrInit INTMSK1 : 0x%08X\r\n", pIntrRegs->INTMSK1));
	RETAILMSG(1,(L"+BSPIntrInit SRCPND1 : 0x%08X\r\n", pIntrRegs->SRCPND1));
	RETAILMSG(1,(L"+BSPIntrInit INTPND1 : 0x%08X\r\n", pIntrRegs->INTPND1));
	RETAILMSG(1,(L"+BSPIntrInit INTMOD1 : 0x%08X\r\n", pIntrRegs->INTMOD1));
	RETAILMSG(1,(L"+BSPIntrInit INTOFFSET1 : 0x%08X\r\n", pIntrRegs->INTOFFSET1));

	RETAILMSG(1,(L"+BSPIntrInit EINTMASK : 0x%08X\r\n", pOalPortRegs->EINTMASK));
	RETAILMSG(1,(L"+BSPIntrInit EXTINT0: 0x%08X\r\n", pOalPortRegs->EXTINT0));
	RETAILMSG(1,(L"+BSPIntrInit EXTINT1: 0x%08X\r\n", pOalPortRegs->EXTINT1));
	RETAILMSG(1,(L"+BSPIntrInit EXTINT2: 0x%08X\r\n", pOalPortRegs->EXTINT2));
	RETAILMSG(1,(L"+BSPIntrInit EINTPEND: 0x%08X\r\n", pOalPortRegs->EINTPEND));

	
    // Add static mapping for Built-In OHCI 
    OALIntrStaticTranslate(SYSINTR_OHCI, IRQ_USBH);
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_RTC);

	 //[david.modify] 2008-05-16 11:02
	 //================================
	     OALIntrStaticTranslate(SYSINTR_KEYPAD, IRQ_KEYPAD);
	 //================================	 

	// OALIntrStaticTranslate(SYSINTR_USBFN, IRQ_USBFN);
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-BSPIntrInit(rc = 1)\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------

BOOL BSPIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
    BOOL rc = FALSE;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+BSPIntrRequestIrq(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
        pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
        pDevLoc->Pin, pCount, pIrqs
    ));
	 //[david.modify] 2008-05-16 17:09
	DPNOK(0);	

    if (pIrqs == NULL || pCount == NULL || *pCount < 1) goto cleanUp;

    switch (pDevLoc->IfcType) {
    case Internal:
        switch ((ULONG)pDevLoc->LogicalLoc) {
        case BSP_BASE_REG_PA_CS8900A_IOBASE:
            pIrqs[0] = IRQ_EINT13;
            *pCount = 1;
            rc = TRUE;
            break;
        }
        break;
    }

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-BSPIntrRequestIrq(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrEnableIrq
//
//  This function is called from OALIntrEnableIrq to enable interrupt on
//  secondary interrupt controller.
//
UINT32 BSPIntrEnableIrq(UINT32 irq)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrEnableIrq(%d)\r\n", irq));
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrEnableIrq(irq = %d)\r\n", irq));
    return irq;
}

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrDisableIrq
//
//  This function is called from OALIntrDisableIrq to disable interrupt on
//  secondary interrupt controller.
//
UINT32 BSPIntrDisableIrq(UINT32 irq)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDisableIrq(%d)\r\n", irq));
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDisableIrq(irq = %d\r\n", irq));
    return irq;
}


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrDoneIrq
//
//  This function is called from OALIntrDoneIrq to finish interrupt on
//  secondary interrupt controller.
//
UINT32 BSPIntrDoneIrq(UINT32 irq)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDoneIrq(%d)\r\n", irq));
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDoneIrq(irq = %d)\r\n", irq));
    return irq;
}


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrActiveIrq
//
//  This function is called from interrupt handler to give BSP chance to 
//  translate IRQ in case of secondary interrupt controller.
//
UINT32 BSPIntrActiveIrq(UINT32 irq)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrActiveIrq(%d)\r\n", irq));
 //[david.modify] 2008-05-16 11:54
  switch(irq)
    {
    case IRQ_EINT0 :
    case IRQ_EINT2 :
    	break;
    	
  	case IRQ_EINT3 :
    	break;
			

    case IRQ_EINT4_7 :
    	irq = IO_InterruptHandler4_7(irq);
    	break;

    case IRQ_EINT8_23 :
	irq = IO_InterruptHandler8_23(irq);
        break;    

    default:
        break;
    }


	
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrActiveIrq(%d)\r\n", irq));
    return irq;
}

//------------------------------------------------------------------------------

 //[david.modify] 2008-05-16 11:07
 //===============================
 
UINT32 IO_InterruptHandler4_7(UINT32 irq)
{
 //[david.modify] 2008-05-16 17:09
	DPNOK(irq);	
    return irq;
}
//====================================
 //[david. end] 2008-05-16 11:07

//[david.modify] 2008-05-16 10:50
//================================= 
// KEYBD 有三个键,对应如下;
//[david.modify] 2008-05-16 10:19
//GPG2	KEY6(up)		音量加EINT10
//GPG1	KEY4 (OK)	确认键EINT9
//GPG0	KEY3 (down)	音量减EINT8

UINT32 IO_InterruptHandler8_23 (UINT32 irq)
{
//	RETAILMSG(1, (L"+-IO_InterruptHandler8_23 EINT 0x%08X ISR\r\n", pOalPortRegs->EINTPEND));
	DPNOK(pOalPortRegs->EINTPEND);
//KEYPAD
	if (pOalPortRegs->EINTPEND & BIT_8)
	{
	        SETREG32(&pOalPortRegs->EINTMASK, BIT_8);        

		RETAILMSG(KEYPAD_DBG, (L"BIT_8 KEY\r\n"));
		g_pBspArgs->uninit_misc.dwButtonType = EINT8_KEY;

		OUTREG32(&pOalPortRegs->EINTPEND, BIT_8);
		CLRREG32(&pOalPortRegs->EINTMASK, BIT_8);

		return IRQ_KEYPAD;
	}
	else if (pOalPortRegs->EINTPEND & BIT_9)
	{
    SETREG32(&pOalPortRegs->EINTMASK, BIT_9);        

		RETAILMSG(KEYPAD_DBG, (L"BIT_9 KEY\r\n"));
		g_pBspArgs->uninit_misc.dwButtonType = EINT9_KEY;

		OUTREG32(&pOalPortRegs->EINTPEND, BIT_9);
		CLRREG32(&pOalPortRegs->EINTMASK, BIT_9);

		return IRQ_KEYPAD;
	}
	else if (pOalPortRegs->EINTPEND & BIT_10)
	{
	        SETREG32(&pOalPortRegs->EINTMASK, BIT_10);        
		RETAILMSG(KEYPAD_DBG, (L"BIT_10 KEY\r\n"));			
		g_pBspArgs->uninit_misc.dwButtonType = EINT10_KEY;
		OUTREG32(&pOalPortRegs->EINTPEND, BIT_10);
		CLRREG32(&pOalPortRegs->EINTMASK, BIT_10);

		return IRQ_KEYPAD;
	}	


	SETREG32(&pOalPortRegs->EINTMASK, INREG32(&pOalPortRegs->EINTPEND));
	OUTREG32(&pOalPortRegs->EINTPEND, INREG32(&pOalPortRegs->EINTPEND));
	CLRREG32(&pOalPortRegs->EINTMASK, INREG32(&pOalPortRegs->EINTPEND));

	return irq;
}




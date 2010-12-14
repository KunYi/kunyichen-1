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
//  File: interrupt.c
//
//  This file implement major part of interrupt module for S3C3210X SoC.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <s3c2450.h>
#include <s3c2450_intr.h>
#include <intr.h>
 

//------------------------------------------------------------------------------
//
//  Globals:  g_pIntrRegs/g_pPortRegs
//
//  The global variables are storing virual address for interrupt and port
//  registers for use in interrupt handling to avoid possible time consumig
//  call to OALPAtoVA function.
//
volatile S3C2450_INTR_REG *g_pIntrRegs; 
static S3C2450_IOPORT_REG *g_pPortRegs;
static S3C2450_CFCARD_REG *g_pATAPIRegs;
static S3C2450_LCD_REG	*g_pLCDRegs;


//  Function pointer to profiling timer ISR routine.
//
PFN_PROFILER_ISR g_pProfilerISR = NULL;
 
//------------------------------------------------------------------------------
//
//  Function:  OALIntrInit
//
//  This function initialize interrupt mapping, hardware and call platform
//  specific initialization.
//
BOOL OALIntrInit()
{
    BOOL rc = FALSE;
    
    OALMSG( OAL_FUNC&&OAL_INTR, (L"+OALInterruptInit\r\n") );

    // Initialize interrupt mapping
    OALIntrMapInit();

    // First get uncached virtual addresses
    g_pIntrRegs = (S3C2450_INTR_REG*)OALPAtoVA(
        S3C2450_BASE_REG_PA_INTR, FALSE
    );
    g_pPortRegs = (S3C2450_IOPORT_REG*)OALPAtoVA(
        S3C2450_BASE_REG_PA_IOPORT, FALSE
    );
    g_pLCDRegs = (S3C2450_LCD_REG *)OALPAtoVA(
    S3C2450_BASE_REG_PA_LCD, FALSE
    );
    
    g_pATAPIRegs = (S3C2450_CFCARD_REG*)OALPAtoVA(
        S3C2450_BASE_REG_PA_CFCARD, FALSE
    );    

    // Mask and clear external interrupts
    OUTREG32(&g_pPortRegs->EINTMASK, 0xFFFFFFFF);
    OUTREG32(&g_pPortRegs->EINTPEND, 0xFFFFFFFF);

    
    // Mask and clear internal sub interrupts
    OUTREG32(&g_pIntrRegs->SUBSRCPND, 0xFFFFFFFF);
    OUTREG32(&g_pIntrRegs->INTSUBMSK, 0xFFFFFFFF);
    
    // Mask and clear internal interrupts
    OUTREG32(&g_pIntrRegs->INTMSK, 0xFFFFFFFF);
    OUTREG32(&g_pIntrRegs->SRCPND, 0xFFFFFFFF);
    

    // S3C2450 developer notice (page 4) warns against writing a 1 to any
    // 0 bit field in the INTPND register.  Instead we'll write the INTPND
    // value itself.
    OUTREG32(&g_pIntrRegs->INTPND, INREG32(&g_pIntrRegs->INTPND));

    // Unmask the system tick timer interrupt
    CLRREG32(&g_pIntrRegs->INTMSK, 1 << IRQ_TIMER4);

#ifdef OAL_BSP_CALLBACKS
    // Give BSP change to initialize subordinate controller
    rc = BSPIntrInit();
#else
    rc = TRUE;
#endif

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALInterruptInit(rc = %d)\r\n", rc));
    return rc;
}



//------------------------------------------------------------------------------
//
//  Function:  OALIntrRequestIrqs
//
//  This function returns IRQs for CPU/SoC devices based on their
//  physical address.
//
BOOL OALIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
    BOOL rc = FALSE;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrRequestIrqs(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
        pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
        pDevLoc->Pin, pCount, pIrqs
    ));

    // This shouldn't happen
    if (*pCount < 1) goto cleanUp;

#ifdef OAL_BSP_CALLBACKS
    rc = BSPIntrRequestIrqs(pDevLoc, pCount, pIrqs);
#endif    

cleanUp:        
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrRequestIrqs(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrEnableIrqs
//
BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    BOOL rc = TRUE;
    UINT32 i, mask, irq;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to enable irq on subordinate interrupt controller
        irq = BSPIntrEnableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
        // Depending on IRQ number use internal or external mask register
        if (irq <= IRQ_ADC) {
            // Use interrupt mask register
            if ((INREG32(&g_pIntrRegs->INTPND) & (1<<irq)) != 0) {
                OUTREG32(&g_pIntrRegs->INTPND, 1<<irq);
            }
            CLRREG32(&g_pIntrRegs->INTMSK, 1 << irq);
        } else if (irq <= IRQ_EINT7) {
            // Use external mask register
            CLRREG32(&g_pIntrRegs->INTMSK, 1 << IRQ_EINT4_7);
            CLRREG32(&g_pPortRegs->EINTMASK, 1 << (irq - IRQ_EINT4 + 4));
        } else if (irq <= IRQ_EINT23) {
            // Use external mask register
            mask = 1 << (irq - IRQ_EINT4 + 4);
            if ((INREG32(&g_pPortRegs->EINTPEND) & mask) != 0) {
                OUTREG32(&g_pPortRegs->EINTPEND, mask);
            }            
            
            CLRREG32(&g_pPortRegs->EINTMASK, mask);
            mask = 1 << IRQ_EINT8_23;
            if ((INREG32(&g_pIntrRegs->INTPND) & mask) != 0) {
                OUTREG32(&g_pIntrRegs->INTPND, mask);
            }
            CLRREG32( &g_pIntrRegs->INTMSK, 1 << IRQ_EINT8_23);
        }  
        else if( irq <= IRQ_AUDIO)
        {
       	} 
        else if (irq <= IRQ_DMA5)	// irq >= IRQ_DMA0 && irq <= IRQ_DMA5
        {
        	//RETAILMSG(1,(TEXT("DMA!! Interrupt Enable!! -- %d\n"),(5 - (IRQ_DMA5 - irq)) ));        	
        	CLRREG32(&g_pIntrRegs->INTMSK, 1 << IRQ_DMA);
         	mask = 1 << (IRQ_SUB_DMA5 - (IRQ_DMA5 - irq));	
         	CLRREG32(&g_pIntrRegs->INTSUBMSK, mask);
     	}   
     	else if(irq == IRQ_LCD_VSYNC)
     	{
     		CLRREG32(&g_pIntrRegs->INTMSK, 1 << IRQ_LCD);
     		CLRREG32(&g_pIntrRegs->INTSUBMSK, 1 << IRQ_SUB_LCD3);
     	}
     	
        else {
            rc = FALSE;
        }
    }        

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrEnableIrqs(rc = %d)\r\n", rc));
    return rc;    
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrDisableIrqs
//
VOID OALIntrDisableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 i, mask, irq;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to disable irq on subordinate interrupt controller
        irq = BSPIntrDisableIrq(pIrqs[i]);
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
#endif
        // Depending on IRQ number use internal or external mask register
        if (irq <= IRQ_ADC) {
            // Use interrupt mask register
            mask = 1 << irq;
            SETREG32(&g_pIntrRegs->INTMSK, mask);
        } else if (irq <= IRQ_EINT23) {
            // Use external mask register
            mask = 1 << (irq - IRQ_EINT4 + 4);
            SETREG32(&g_pPortRegs->EINTMASK, mask);
        }
        else if( irq <= IRQ_AUDIO)
        {
       	} 
        else if (irq <= IRQ_DMA5)	// irq >= IRQ_DMA0 && irq <= IRQ_DMA5
        {
        	//RETAILMSG(1,(TEXT("DMA!! Interrupt Enable!! -- %d\n"),irq));        	
        	//SETREG32(&g_pIntrRegs->INTMSK, 1 << IRQ_DMA);
         	mask = 1 << (IRQ_SUB_DMA5 - (IRQ_DMA5 - irq));	
         	SETREG32(&g_pIntrRegs->INTSUBMSK, mask);
     	}  
     	else if(irq == IRQ_LCD_VSYNC)
     	{
     		SETREG32(&g_pIntrRegs->INTMSK, 1 << IRQ_LCD);
     		SETREG32(&g_pIntrRegs->INTSUBMSK, 1 << IRQ_SUB_LCD3);
     	}     	       
    }

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrDisableIrqs\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrDoneIrqs
//
VOID OALIntrDoneIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 i, mask, irq;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to finish irq on subordinate interrupt controller
        irq = BSPIntrDoneIrq(pIrqs[i]);
#endif    
        // Depending on IRQ number use internal or external mask register
        
        if (irq == IRQ_CFCON) {
            // Use interrupt mask register
            g_pATAPIRegs->ATA_IRQ = 0xff;
       	    g_pATAPIRegs->ATA_IRQ_MASK = 0xffffffe0;           
            mask = 1 << irq;
            OUTREG32(&g_pIntrRegs->SRCPND, mask);
            CLRREG32(&g_pIntrRegs->INTMSK, mask);
        } else if (irq <= IRQ_ADC) {
            // Use interrupt mask register
            mask = 1 << irq;
            OUTREG32(&g_pIntrRegs->SRCPND, mask);
            CLRREG32(&g_pIntrRegs->INTMSK, mask);
        } else if (irq <= IRQ_EINT23) {
            // Use external mask register
            mask = 1 << (irq - IRQ_EINT4 + 4);
            OUTREG32(&g_pPortRegs->EINTPEND, mask);
            CLRREG32(&g_pPortRegs->EINTMASK, mask);
        }
        else if( irq <= IRQ_AUDIO)
        {
       	} 
        else if (irq <= IRQ_DMA5)	// irq >= IRQ_DMA0 && irq <= IRQ_DMA5
        {
        	//RETAILMSG(1,(TEXT("DMA!! Interrupt Done!! -- %d\n"),irq));
         	mask = 1 << (IRQ_SUB_DMA5 - (IRQ_DMA5 - irq));	
         	OUTREG32(&g_pIntrRegs->SUBSRCPND, mask);
         	CLRREG32(&g_pIntrRegs->INTSUBMSK, mask);
     	}   
     	else if(irq == IRQ_LCD_VSYNC)
     	{
  			OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_LCD3) );			// Clear LCD3 sub source pending register bit
  			CLRREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_LCD3) );		// masking LCD3 sub interrupt			
     	}  
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDoneIrqs\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandler
//
ULONG OEMInterruptHandler(ULONG ra)
{
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 irq, irq2, mask;
    static DWORD HeartBeatCnt, HeartBeatStat;  //LED4 is used for heart beat
    static DWORD DMA2CNT=0;
    
    // Get pending interrupt(s)
    irq = INREG32(&g_pIntrRegs->INTOFFSET);

    // System timer interrupt?
    if (irq == IRQ_TIMER4) {

	if (++HeartBeatCnt > 100)
	{
		HeartBeatCnt   = 0;
		HeartBeatStat ^= 1;
		g_pPortRegs->GPFCON = (g_pPortRegs->GPFCON & ~(3<<8)) | (1<<8); // GPF4 Output
		if (HeartBeatStat) 
		{
			g_pPortRegs->GPFDAT &= ~(1<<4); // LED 4 Off
		}
		else
		{
			g_pPortRegs->GPFDAT |=  (1<<4); // LED 4 On
		}
	}

        // Rest is on timer interrupt handler
        sysIntr = OALTimerIntrHandler();
	}    
    // Profiling timer interrupt?
    else if (irq == IRQ_TIMER2)
    {
        // Mask and Clear the interrupt.
        mask = 1 << irq;
        SETREG32(&g_pIntrRegs->INTMSK, mask);
        OUTREG32(&g_pIntrRegs->SRCPND, mask);
        OUTREG32(&g_pIntrRegs->INTPND, mask);
        INREG32(&g_pIntrRegs->INTPND);

        // The rest is up to the profiling interrupt handler (if profiling
        // is enabled).
        //
        if (g_pProfilerISR)
        {
            sysIntr = g_pProfilerISR(ra);
        }
    }
    else 
    {

#ifdef OAL_ILTIMING
        if (g_oalILT.active) {
            g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
            g_oalILT.savedPC = 0;
            g_oalILT.interrupts++;
        }        
#endif
    
        if (irq == IRQ_EINT4_7 || irq == IRQ_EINT8_23) { // 4 or 5

            // Find external interrupt number
            mask = INREG32(&g_pPortRegs->EINTPEND);
            mask &= ~INREG32(&g_pPortRegs->EINTMASK);
            mask = (mask ^ (mask - 1)) >> 5;
            irq2 = IRQ_EINT4;
            while (mask != 0) {
                mask >>= 1;
                irq2++;
            }

            // Mask and clear interrupt
            mask = 1 << (irq2 - IRQ_EINT4 + 4);
            SETREG32(&g_pPortRegs->EINTMASK, mask);
            OUTREG32(&g_pPortRegs->EINTPEND, mask);

            // Clear primary interrupt
            mask = 1 << irq;
            OUTREG32(&g_pIntrRegs->SRCPND, mask);
            OUTREG32(&g_pIntrRegs->INTPND, mask);
            INREG32(&g_pIntrRegs->INTPND);

            // From now we care about this irq
            irq = irq2;

        }  
      else if(irq == IRQ_CAM)
        {
        	if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_CAM_C))
        	{
	        	  SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_CAM_C));
	        	  SETREG32(&g_pIntrRegs->INTMSK, (1<<IRQ_CAM));
	        	  OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_CAM_C));
	        	  OUTREG32(&g_pIntrRegs->SRCPND, (1<<IRQ_CAM));
	        	  OUTREG32(&g_pIntrRegs->INTPND,(1<<IRQ_CAM));
	        	  INREG32(&g_pIntrRegs->INTPND);
	        	  //RETAILMSG(1,(TEXT("IRQ_CAM Codec\r\n")));
        	}
        	
        	else if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_CAM_P))
         	{
	        	  SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_CAM_P));
	        	  SETREG32(&g_pIntrRegs->INTMSK, (1<<IRQ_CAM));
	        	  OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_CAM_P));
	        	  OUTREG32(&g_pIntrRegs->SRCPND, (1<<IRQ_CAM));
	        	  OUTREG32(&g_pIntrRegs->INTPND,(1<<IRQ_CAM));
	        	  INREG32(&g_pIntrRegs->INTPND);
	        	  //RETAILMSG(1,(TEXT("PreView\r\n")));
        	}
        	
        	else
        	{
        		SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_CAM_C)|(1<<IRQ_SUB_CAM_P)); 
        		SETREG32(&g_pIntrRegs->INTMSK, (1<<IRQ_CAM));
        		SETREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_CAM_C)|(1<<IRQ_SUB_CAM_P));       	  
        		OUTREG32(&g_pIntrRegs->SRCPND, (1<<IRQ_CAM));
        		OUTREG32(&g_pIntrRegs->INTPND,(1<<IRQ_CAM));
        		INREG32(&g_pIntrRegs->INTPND);
        	       // RETAILMSG(1,(TEXT("nop\r\n")));        		
        		return SYSINTR_NOP; 
        	}
        }
		else if(irq == IRQ_LCD)
		{    
			sysIntr = SYSINTR_NOP;
      		if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1 << IRQ_SUB_LCD3) )				 
      		{
      			if ( (INREG32(&g_pLCDRegs->VIDCON1) & LCD_VSTATUS) )
      			{      		
      				SETREG32(&g_pIntrRegs->INTSUBMSK, (1 << IRQ_SUB_LCD3) );		// masking LCD3 sub interrupt		
      				//RETAILMSG(1,(TEXT("+")));
      				irq = IRQ_LCD_VSYNC;
      			}
      			OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_LCD3) );			// Clear LCD3 sub source pending register bit
      		}
			OUTREG32(&g_pIntrRegs->SRCPND, (1<<IRQ_LCD) );							// Clear LCD source pending register bit
			OUTREG32(&g_pIntrRegs->INTPND, (1<<IRQ_LCD) );							// Clear LCD interrupt register bit
			INREG32(&g_pIntrRegs->INTPND);											// confirm
		}  
        else if(irq == IRQ_DMA)
        {
        	SETREG32(&g_pIntrRegs->INTMSK, (1<<IRQ_DMA));
        	//RETAILMSG(1,(TEXT("DMA\n")));
        	if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_DMA0))
        	{
        	  	SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_DMA0));
        	  	OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_DMA0));
        	  	irq = IRQ_DMA0;
        	}
        	else if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_DMA1))
        	{
        		//RETAILMSG(1,(TEXT("DMA1\n")));
        		//RETAILMSG(1,(TEXT("g_pIntrRegs->SUBSRCPND=0x%08X\n"),g_pIntrRegs->SUBSRCPND));
        		SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_DMA1));
        		OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_DMA1));
        		
        		irq = IRQ_DMA1;
        	} 
        	else if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_DMA2))
        	{

       			//RETAILMSG(1,(TEXT("DMA2\n")));
   
        		SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_DMA2));
        		OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_DMA2));
        		irq = IRQ_DMA2;
        	} 
        	else if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_DMA3))
        	{
	        	SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_DMA3));
	        	OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_DMA3));
        	    irq = IRQ_DMA3;
        	} 
        	else if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_DMA4))
        	{
				SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_DMA4));
				OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_DMA4));
				irq = IRQ_DMA4;
        	}   
        	else if(INREG32(&g_pIntrRegs->SUBSRCPND) & (1<<IRQ_SUB_DMA5))
        	{
				SETREG32(&g_pIntrRegs->INTSUBMSK, (1<<IRQ_SUB_DMA5));
				OUTREG32(&g_pIntrRegs->SUBSRCPND, (1<<IRQ_SUB_DMA5));
				irq = IRQ_DMA5;
        	}         	      	        	        	       	
        	OUTREG32(&g_pIntrRegs->SRCPND, (1<<IRQ_DMA));
        	OUTREG32(&g_pIntrRegs->INTPND, (1<<IRQ_DMA));
        	INREG32(&g_pIntrRegs->INTPND);
        	CLRREG32(&g_pIntrRegs->INTMSK, (1<<IRQ_DMA));
        }      	

        else if(irq == IRQ_CFCON)
        {
       	    if ( !(g_pATAPIRegs->ATA_IRQ & 0x1))
       	    {
       	    	g_pATAPIRegs->ATA_IRQ = 0xff;
            	OUTREG32(&g_pIntrRegs->SRCPND, 1<<IRQ_CFCON );
            	OUTREG32(&g_pIntrRegs->INTPND, 1<<IRQ_CFCON );   
            	INREG32(&g_pIntrRegs->INTPND);    	    	
        	return SYSINTR_NOP;       	    	
       	    }

       	    g_pATAPIRegs->ATA_IRQ_MASK = 0xffffffff;
       	    g_pATAPIRegs->ATA_IRQ = 0xff;       	    
            SETREG32(&g_pIntrRegs->INTMSK, 1<<IRQ_CFCON );
            OUTREG32(&g_pIntrRegs->SRCPND, 1<<IRQ_CFCON );
            OUTREG32(&g_pIntrRegs->INTPND, 1<<IRQ_CFCON );  
            INREG32(&g_pIntrRegs->INTPND);      		
       	}
        	
        	
	else {
            // Mask and clear interrupt
            mask = 1 << irq;
            SETREG32(&g_pIntrRegs->INTMSK, mask);
            OUTREG32(&g_pIntrRegs->SRCPND, mask);
            OUTREG32(&g_pIntrRegs->INTPND, mask);
            INREG32(&g_pIntrRegs->INTPND);
        }

        // First find if IRQ is claimed by chain
        sysIntr = NKCallIntChain((UCHAR)irq);
        if (sysIntr == SYSINTR_CHAIN || !NKIsSysIntrValid(sysIntr)) {
            // IRQ wasn't claimed, use static mapping
            sysIntr = OALIntrTranslateIrq(irq);
        }
    }

	//g_oalLastSysIntr = sysIntr;
    return sysIntr;
}

//------------------------------------------------------------------------------

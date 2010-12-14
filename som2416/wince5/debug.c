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
//  File:  debug.c            
//
//  This module is provides the interface to the serial port.
//
#include <bsp.h>
#include <nkintr.h>
 
//------------------------------------------------------------------------------
// Defines 
 
//------------------------------------------------------------------------------
// Externs
 
//------------------------------------------------------------------------------
// Global Variables 
 
//------------------------------------------------------------------------------
// Local Variables 
static S3C2450_UART_REG *g_pUARTReg;

//------------------------------------------------------------------------------
// Local Functions 


//------------------------------------------------------------------------------
//
//  Function: OEMInitDebugSerial
//
//  Initializes the debug serial port
//


VOID OEMInitDebugSerial_ORG() 
{
    S3C2450_IOPORT_REG *pIOPortReg;
    UINT32 logMask;
	UINT32 count;


  //  while(1);

#if 0
	volatile int i;
	while(1)
	{
		for(i = 0; i <0x80000;i++);
		OEMWriteDebugLED(0,1);
		//*(UINT32 *)(0xB1E00054) = 0xC0;
		for(i = 0; i <0x80000;i++);
		OEMWriteDebugLED(0,4);
		//*(UINT32 *)(0xB1E00054) = 0x30;
	}
#endif


    // At this moment we must suppress logging.
    //
    logMask = g_oalLogMask;
    g_oalLogMask = 0;

    // Configure port H for UART1.
    //
    pIOPortReg = (S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);

    // GPH2 and GHP3 are UART1 Tx and Rx, respectively.
    //
    CLRREG32(&pIOPortReg->GPHCON, (3 << 4)|(3 << 6));
    SETREG32(&pIOPortReg->GPHCON, (2 << 4)|(2 << 6));

    // Disable pull-up on TXD1 and RXD1.
    //
        SETREG32(&pIOPortReg->GPHUDP, (1 << 4)|(1 << 6));

    // UART1 (TXD1 & RXD1) used for debug serial.
    //
    g_pUARTReg = (S3C2450_UART_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_UART1, FALSE);

    // Configure the UART.
    //
    OUTREG32(&g_pUARTReg->UFCON,  BSP_UART1_UFCON);
    OUTREG32(&g_pUARTReg->UMCON,  BSP_UART1_UMCON);
    OUTREG32(&g_pUARTReg->ULCON,  BSP_UART1_ULCON);
    OUTREG32(&g_pUARTReg->UCON,   BSP_UART1_UCON);
    OUTREG32(&g_pUARTReg->UBRDIV, BSP_UART1_UBRDIV);

	for (count=0;count<0xffff;count++);
    // Restore the logging mask.
    //
    g_oalLogMask = logMask;
}


	
VOID OEMInitDebugSerial_1(UINT32 DbgSerPhysAddr) 
{
    S3C2450_IOPORT_REG *pIOPortReg;
    UINT32 logMask;
	UINT32 count;
	UINT32 u32Temp = 0;
	UINT32 DIV_VAL = 0;
	UINT32 UDIV_SLOT=0;




    // At this moment we must suppress logging.
    //
    logMask = g_oalLogMask;
    g_oalLogMask = 0;



    // Configure port H for UART1.
    //
    pIOPortReg = (S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);

 //[david.modify] 2008-04-28 10:05
	u32Temp= (pIOPortReg->GPHCON & ~0xffff ) | 0xaaaa;
	pIOPortReg->GPHCON = u32Temp;
/*
    // GPH2 and GHP3 are UART1 Tx and Rx, respectively.
    //
    CLRREG32(&pIOPortReg->GPHCON, (3 << 4)|(3 << 6));
    SETREG32(&pIOPortReg->GPHCON, (2 << 4)|(2 << 6));
    // Disable pull-up on TXD1 and RXD1.
    //
#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1    
        SETREG32(&pIOPortReg->GPHUDP, (1 << 4)|(1 << 6));
#else
	SETREG32(&pIOPortReg->GPHUDP, (2 << 4)|(2 << 6));
#endif
#elif (BSP_TYPE == BSP_SMDK2450)
        SETREG32(&pIOPortReg->GPHUDP, (1 << 4)|(1 << 6));
#endif
    
*/    

//[david.modify] 2008-06-02 10:36
// using for release
//=========================================
	switch (DbgSerPhysAddr) {
	case S3C2450_BASE_REG_PA_UART0:
	case S3C2450_BASE_REG_PA_UART1:
	case S3C2450_BASE_REG_PA_UART2:
	case S3C2450_BASE_REG_PA_UART3:		
		g_pUARTReg = (S3C2450_UART_REG *)OALPAtoVA(DbgSerPhysAddr, FALSE);
		break;
	default:
		g_pUARTReg=NULL;
		return ;
	}
//=========================================



    // UART1 (TXD1 & RXD1) used for debug serial.
    //
//    g_pUARTReg = (S3C2450_UART_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_UART0, FALSE);

    // Configure the UART.
    //
		g_pUARTReg->ULCON= 0x3; //Line control register : Normal,No parity,1 stop,8 bits
	     //  11]  [10]       [9]     [8]        [7]         [6]         [5]          [4]              [3:2]               [1:0]
	     // Clock Sel,  Tx Int,  Rx Int, Rx Time Out, Rx err, Loop-back, Send break,  Transmit Mode, Receive Mode
	     //  1   0          0       0    ,     0            1           0               0     ,       01                  01
	     //   PCLK2      Pulse  Pulse    Disable    Generate  Normal      Normal        Interrupt or Polling
/*
		pUartRegs->rUCon = 0x245; 
		pUartRegs->rUbrDiv = ((int)(PCLK/16./baud+0.5)-1);   //Baud rate divisior register 0 
		pUartRegs->rUfCon = 0x0; //UART FIFO control register, FIFO disable
		pUartRegs->rUmCon = 0x0; //UART MODEM control register, AFC disable
		pUartRegs->rUdivSlot= 0x54AA; //UART MODEM control register, AFC disable
*/
		DIV_VAL = BSP_UART0_UBRDIV;

		g_pUARTReg->UCON= 0x845;
		g_pUARTReg->UBRDIV= (int)DIV_VAL;   //Baud rate divisior register 0 
		g_pUARTReg->UFCON= 0x6; //UART FIFO control register, FIFO disable, tx, rx fifo reset
		g_pUARTReg->UMCON= 0x0; //UART MODEM control register, AFC disable

		UDIV_SLOT = (int)( (DIV_VAL-(int)DIV_VAL)*16 );
		switch (UDIV_SLOT) {

		case 0 : g_pUARTReg->UDIVSLOT = 0; break;
		case 1 : g_pUARTReg->UDIVSLOT = 0x0080; break;
		case 2 : g_pUARTReg->UDIVSLOT = 0x0808; break;
		case 3 : g_pUARTReg->UDIVSLOT = 0x0888; break;
		case 4 : g_pUARTReg->UDIVSLOT = 0x2222; break;
		case 5 : g_pUARTReg->UDIVSLOT = 0x4924; break;
		case 6 : g_pUARTReg->UDIVSLOT = 0x4a52; break;
		case 7 : g_pUARTReg->UDIVSLOT = 0x54aa; break;
		case 8 : g_pUARTReg->UDIVSLOT = 0x5555; break;
		case 9 : g_pUARTReg->UDIVSLOT = 0xd555; break;
		case 10 : g_pUARTReg->UDIVSLOT = 0xd5d5; break;
		case 11 : g_pUARTReg->UDIVSLOT = 0xddd5; break;
		case 12 : g_pUARTReg->UDIVSLOT = 0xdddd; break;
		case 13 : g_pUARTReg->UDIVSLOT = 0xdfdd; break;
		case 14 : g_pUARTReg->UDIVSLOT = 0xdfdf; break;
		case 15 : g_pUARTReg->UDIVSLOT = 0xffdf; break;
		}    

	for (count=0;count<0xffff;count++);
    // Restore the logging mask.
    //
    g_oalLogMask = logMask;
}




VOID OEMInitDebugSerial(void)
{
//    UINT32 *pDbgSerPhysAddr;

 //[david.modify] 2007-04-02 21:46
    UINT32 u32Temp[4]={0};
    BSP_ARGS *pBSP_ARGS = (BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START;

    u32Temp[0] = 	pBSP_ARGS->dbgSerPhysAddr;	 
	switch ( u32Temp[0] ) {
	case S3C2450_BASE_REG_PA_UART0:
	case S3C2450_BASE_REG_PA_UART1:
	case S3C2450_BASE_REG_PA_UART2:
	case S3C2450_BASE_REG_PA_UART3:		
	case S3C2450_BASE_REG_PA_NOUART:			
		OEMInitDebugSerial_1(u32Temp[0] );
		break;
	default:
		OEMInitDebugSerial_1(S3C2450_BASE_REG_PA_UART0);
		return ;
	}	

#if 0
    	 //[david.modify] 2007-04-02 21:46
    	 DPNOK(u32Temp[0]);
    	 DPNOK(u32Temp[1]);	
        DPNOK(pDbgSerPhysAddr);		 
	 //[david. end] 2007-04-02 21:49
#endif	 
    		
}




//------------------------------------------------------------------------------
//
//  Function: OEMWriteDebugByte
//
//  Transmits a character out the debug serial port.
//
VOID OEMWriteDebugByte(UINT8 ch) 
{

 //[david.modify] 2008-06-02 10:41
	if(g_pUARTReg==NULL)
		return;

    // Wait for transmit buffer to be empty
    while ((INREG32(&g_pUARTReg->UTRSTAT) & 0x02) == 0);

    // Send character
    OUTREG32(&g_pUARTReg->UTXH, ch);
}


//------------------------------------------------------------------------------
//
//  Function: OEMReadDebugByte
//
//  Reads a byte from the debug serial port. Does not wait for a character. 
//  If a character is not available function returns "OEM_DEBUG_READ_NODATA".
//

int OEMReadDebugByte() 
{
    UINT32 status, ch;

 //[david.modify] 2008-06-02 10:41
	if(g_pUARTReg==NULL)
		return -1;

    status = INREG32(&g_pUARTReg->UTRSTAT);
    if ((status & 0x01) != 0) {
       ch = INREG32(&g_pUARTReg->URXH);
       // if ((status & UART_LINESTAT_RF) != 0) ch = OEM_DEBUG_COM_ERROR;
    } else {
       ch = OEM_DEBUG_READ_NODATA;
    }
    return (int)ch;
}


/*
    @func   void | OEMWriteDebugLED | Writes specified pattern to debug LEDs 1-4.
    @rdesc  None.
    @comm    
    @xref   
*/
#include <xllp_gpio_david.h>
void OEMWriteDebugLED(UINT16 Index, DWORD Pattern)
{
    volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);

    // The S24x0X01 Eval platform supports 4 LEDs..
    //
//    s2450IOP->GPFDAT = (s2450IOP->GPFDAT & 0xf) | ((Pattern & 0xf)<<4);
 //[david.modify] 2008-04-28 10:21 PND 使用GPB0一个LED
 // Pattern=1 关GPB0, 0-开GPB0
//      s2450IOP->GPBDAT = (s2450IOP->GPBDAT & 0x1) | ((Pattern & 0x1)<<0);
//     s2450IOP->GPBDAT &= ~(0x1<<0);
//     s2450IOP->GPBDAT |=((Pattern & 0x1)<<0);	 
//    s2450IOP->GPBDAT = (s2450IOP->GPBDAT & 0x1) | ((Pattern & 0x1)<<0);
		stGPIOInfo stGPIOInfo[]={			
		{ GLED,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE}
    		};
		SetGPIOInfo(&stGPIOInfo[0],  (VOID*)s2450IOP);		
}

//------------------------------------------------------------------------------

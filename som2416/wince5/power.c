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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------
#include <windows.h>
#include <bsp.h>

#define Clear_FrameBuffer 0xA0100000

void InitTimer(void);
void ConfigSleepGPIO(void);
void Write_LDI_LTV350(int address, int data);
void InitLDI_LTV350(void);
void InitLDI_LTE480(void); //For WVGA
 //[david.modify] 2008-07-04 10:14
//解决醒来后屏闪
void InitLDI_LTV350_sharp2(void);


VOID BSPPowerOff()
{
	//RETAILMSG(1,(TEXT("BSPPowerOff\n")));
	volatile S3C2450_IOPORT_REG *pIOPort = (S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2450_ADC_REG *pADCPort = (S3C2450_ADC_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_ADC, FALSE);
 	volatile S3C2450_RTC_REG *pRTCPort = (S3C2450_RTC_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_RTC, FALSE);
 	volatile S3C2450_LCD_REG *pLCDPort = (S3C2450_LCD_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
	volatile S3C2450_CLKPWR_REG *pCLKPWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);


	ULONG *FrameBufferPtr = (ULONG *)Clear_FrameBuffer;
//	ULONG FrameBufferCount;
	//v_pRTCregs->RTCCON = (5<<5)|(1<<0);
	pADCPort->ADCCON|=(1<<2);		// ADC StanbyMode

	// USB2.0 and PHY power
	pIOPort->MISCCR|=(1<<12); //USB port = suspend
	
	pCLKPWR->USB_PHYPWR |= (0xf<<0);
	pCLKPWR->PWRCFG &= ~(1<<4);
	pCLKPWR->USB_CLKCON &= ~(1<<31);		
	pCLKPWR->INFORM0 = 0x2BED;	
	

	//pIOPort->MISCCR|=(1<<2); //Previous state at STOP(?) mode (???)

	//D[31:0] pull-up off. The data bus will not be float by the external bus holder.
	//If the pull-up resitsers are turned on,
	//there will be the leakage current through the pull-up resister
	//pIOPort->MISCCR=pIOPort->MISCCR|(3<<0);

	// In the evaluation board, Even though in sleep mode, the devices are all supplied the power.
//	pIOPort->MSLCON = (0<<11)|(0<<10)|(0<<9)|(0<<8)|(0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0);
//	pIOPort->DSC0 = (1<<31)|(3<<8)|(3<<0);
//	pIOPort->DSC1 = (3<<28)|(3<<26)|(3<24)|(3<<22)|(3<<20)|(3<<18);

    /* LCD Controller Disable               */
	CLRPORT32(&pIOPort->GPGDAT, 1 << 4);

	

}

void ConfigSleepGPIO(void)
{

    volatile S3C2450_IOPORT_REG *pIOPort = (S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    volatile S3C2450_CLKPWR_REG *pCLKPWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);
	
// USB2.0 and PHY power
	pCLKPWR->USB_PHYPWR |= (0xf<<0);
	pCLKPWR->PWRCFG &= ~(1<<4);
	#ifdef		__CHK2443
	pCLKPWR->USB_CLKCON &= ~(1<<31);		
	#else
	#endif
	
	
///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_OP1 IO
	// GPF[7:0], GPG[7:0], GPE[15:14], GPE[4:0]


//GPF


	pIOPort->GPFCON &=~(0xffff<<0);
	pIOPort->GPFCON |= ((0x1<<14)|(0x1<<12)|(0x1<<10)|(0x1<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x2<<0));
#ifdef		__CHK2443	
	//rEXTINT0 = (rdEXTINT0 & ~((1<<31) | (1<<27) | (1<<23) | (1<<19) | (1<<15) | (1<<11) | (1<<7) | (1<<3) ));
	//rEXTINT0 = (rdEXTINT0  | ((1<<31) | (1<<27) | (1<<23) | (1<<19) | (0<<15) | (1<<11) | (1<<7) | (1<<3) )); //rEXTINT0[11] = PD_dis(Because of VBUS_DET)
#else	           
	pIOPort->GPFUDP &=~(0xffff<<0);
	pIOPort->GPFUDP |= ((0x2<<14)|(0x2<<12)|(0x2<<10)|(0x2<<8)|(0x1<<6)|(0x1<<4)|(0x2<<2)|(0x1<<0));
#endif
	pIOPort->GPFDAT &=~ (0xff<<0);
	pIOPort->GPFDAT |= ((0x0<<7)|(0x0<<6)|(0x0<<5)|(0x0<<4)|(0x0<<3)|(0x0<<2)|(0x0<<1)|(0x0<<0));
//#endif	
#if 0
	//GPG
	pIOPort->GPGCON &=~(0xffff<<0);
	pIOPort->GPGCON |= ((0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x2<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	           
	pIOPort->GPGUDP &=~(0xffff<<0);
#ifdef		__CHK2443	
	//rEXTINT1 = (rdEXTINT1 | (0<<31) | (0<<27) | (0<<23) | (0<<19) | (1<<15) | (0<<11) | (0<<7)| (0<<0) );	
#else
	pIOPort->GPGUDP |= ((0x2<<14)|(0x2<<12)|(0x1<<10)|(0x1<<8)|(0x1<<6)|(0x1<<4)|(0x1<<2)|(0x1<<0));
#endif
	
	pIOPort->GPGDAT &=~ (0xff<<0);
	pIOPort->GPGDAT |= ((0x1<<7)|(0x0<<6)|(0x1<<5)|(0x1<<4)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));

#endif			
	//GPA
#ifdef		__CHK2443
	//pIOPort->GPACDH = (rdGPACDH | (1<<16) | (1<<5));	//GPACDH[16] is exclusive use of nRSTOUT and '1' is PRESET
#else
	pIOPort->GPACON &=~(0x1<<21);
	pIOPort->GPACON |= ((0x0<<21));//Set GPA21 as Output(default : nRSTOUT)
	
	pIOPort->GPADAT &=~ (0x7<<21);//Clear GPA21~23 
	pIOPort->GPADAT |= ((0x1<<21));//Set GPA21(DAT) as '1'
#endif

	
	// DP0 / DN0
	pIOPort->MISCCR |= (1<<12); //Set USB port as suspend mode	

	
///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_OP2 IO
	// GPB[10:0], GPH[12:0], GPE[15:14], GPE[4:0]

	
	//GPB
	pIOPort->GPBCON &=~(0x3fffff<<0);
#ifdef		__CHK2443
	pIOPort->GPBCON |= ((0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x3<<0));
#else
	pIOPort->GPBCON |= ((0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x1<<6)|(0x1<<4)|(0x0<<2)|(0x1<<0));
#endif
	
	pIOPort->GPBUDP = (pIOPort->GPBUDP &=~(0x3fffff<<0));
#ifdef		__CHK2443											
	pIOPort->GPBUDP |= ((0x3<<20)|(0x2<<18)|(0x2<<16)|(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x3<<8)|(0x3<<6)|(0x3<<4)|(0x0<<2)|(0x3<<0));
#else
	pIOPort->GPBUDP |= ((0x1<<20)|(0x1<<18)|(0x1<<16)|(0x1<<14)|(0x1<<12)|(0x1<<10)|(0x0<<8)|(0x2<<6)|(0x2<<4)|(0x1<<2)|(0x2<<0));          
#endif
	
	
	//GPE
	pIOPort->GPECON &=~((0xf<<28)|(0x3ff<<0));	
#ifdef		__CHK2443
	pIOPort->GPECON |= ((0x0<<30)|(0x0<<28)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
#else	
	pIOPort->GPECON |= ((0x0<<30)|(0x0<<28)|(0x1<<8)|(0x0<<6)|(0x1<<4)|(0x0<<2)|(0x0<<0));
#endif
	
	pIOPort->GPEUDP &=~((0xf<<28)|(0x3ff<<0));
#ifdef		__CHK2443
	pIOPort->GPEUDP |= ((0x0<<30)|(0x0<<28)|(0x0<<8)|(0x3<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));                
#else
	pIOPort->GPEUDP |= ((0x2<<30)|(0x2<<28)|(0x2<<8)|(0x1<<6)|(0x2<<4)|(0x1<<2)|(0x1<<0));          
#endif
	
	pIOPort->GPEDAT &=~ (0xffff<<0);
	pIOPort->GPEDAT |= ((0x1<<15)|(0x1<<14)|(0x1<<4)|(0x1<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));
#ifdef		__CHK2443
#else
	pIOPort->GPEDAT = 0;
	//printf("pIOPort->GPEcon=%08x, pIOPort->GPEUDP=%08x, pIOPort->GPeDAT=%08x\n", pIOPort->GPECON,pIOPort->GPEUDP,pIOPort->GPEDAT);
#endif
	
	
	//GPG
	pIOPort->GPGCON &=~(0xffff<<16);
	pIOPort->GPGCON |= ((0x0<<30)|(0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16));
	
	pIOPort->GPGUDP &=~(0xffff<<16);
#ifdef		__CHK2443	
	pIOPort->GPGUDP |= ((0x2<<30)|(0x0<<28)|(0x3<<26)|(0x3<<24)|(0x0<<22)|(0x3<<20)|(0x3<<18)|(0x3<<16));
#else
	pIOPort->GPGUDP |= ((0x0<<30)|(0x1<<28)|(0x1<<26)|(0x1<<24)|(0x1<<22)|(0x2<<20)|(0x2<<18)|(0x1<<16));
#endif
	
	pIOPort->GPGDAT &=~ (0xff<<8);
	pIOPort->GPGDAT |= ((0x0<<15)|(0x0<<14)|(0x0<<13)|(0x0<<12)|(0x0<<11)|(0x0<<10)|(0x0<<9)|(0x0<<8));

	
	
	//GPH
	pIOPort->GPHCON &=~(0x3fffffff<<0);
#ifdef		__CHK2443
	pIOPort->GPHCON |= ((0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
#else
	pIOPort->GPHCON |= ((0x1<<28)|(0x0<<26)|(0x0<<24)|(0x1<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
#endif
	
	pIOPort->GPHUDP &=~(0x3fffffff<<0);
#ifdef		__CHK2443	
	pIOPort->GPHUDP |= ((0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x3<<20)|(0x0<<18)|(0x3<<16)|(0x3<<6)|(0x0<<4)|(0x3<<2)|(0x0<<0));
#else
	pIOPort->GPHUDP |= ((0x2<<28)|(0x1<<26)|(0x1<<24)|(0x2<<22)|(0x1<<20)|(0x2<<18)|(0x1<<16)|(0x1<<6)|(0x2<<4)|(0x1<<2)|(0x2<<0));
#endif	
	pIOPort->GPHDAT &=~ (0x7fff<<0);
	pIOPort->GPHDAT |= ((0x0<<14)|(0x1<<13)|(0x0<<12)|(0x1<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));



///////////////////////////////////////////////////////////////////////////////////////////
	// VDD_SD IO
	// GPE[13:5], GPL[14:0], GPJ[15:13]
	
	//GPE
	pIOPort->GPECON &=~(0x3ffff<<10);
	pIOPort->GPECON |= ((0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<14)|(0x0<<12)|(0x0<<10));//Set Input All
	
	pIOPort->GPEUDP &=~(0x3ffff<<10);
	#ifdef		__CHK2443	
	pIOPort->GPEUDP |= ((0x0<<26)|(0x0<<24)|(0x0<<22)|(0x3<<20)|(0x3<<18)|(0x3<<16)|(0x3<<14)|(0x3<<12)|(0x0<<10));
	#else
	pIOPort->GPEUDP |= ((0x1<<26)|(0x1<<24)|(0x1<<22)|(0x2<<20)|(0x2<<18)|(0x2<<16)\
	           |(0x2<<14)|(0x2<<12)|(0x1<<10));
	#endif
	
	pIOPort->GPEDAT &=~ (0x1ff<<5);
	pIOPort->GPEDAT |= ((0x0<<13)|(0x0<<12)|(0x0<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x0<<7)|(0x1<<6)|(0x1<<5));

	
	//GPL
	pIOPort->GPLCON &=~(0x3fffffff<<0);
	pIOPort->GPLCON |= ((0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	pIOPort->GPLUDP &=~(0x3fffffff<<0);
	#ifdef		__CHK2443	
	pIOPort->GPLUDP |= ((0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x3<<16)|(0x3<<14)|(0x3<<12)|(0x3<<10)|(0x3<<8)|(0x3<<6)|(0x3<<4)|(0x3<<2)|(0x3<<0));
	#else
	pIOPort->GPLUDP |= ((0x1<<28)|(0x1<<26)|(0x1<<24)|(0x1<<22)|(0x1<<20)|(0x1<<18)|(0x2<<16)|(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x2<<8)|(0x1<<6)|(0x2<<4)|(0x2<<2)|(0x2<<0));
	#endif	
	pIOPort->GPLDAT &=~ (0x1fff<<0);
	pIOPort->GPLDAT |= ((0x0<<14)|(0x0<<13)|(0x0<<12)|(0x0<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x1<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));
	
	//GPJ
	pIOPort->GPJCON &=~(0x3f<<26);
	pIOPort->GPJCON |= ((0x0<<30)|(0x0<<28)|(0x0<<26));
		
	pIOPort->GPJUDP &=~(0x3f<<26);
	#ifdef		__CHK2443	
	pIOPort->GPJUDP |= ((0x0<<30)|(0x0<<28)|(0x0<<26));
	#else
	pIOPort->GPJUDP |= ((0x2<<30)|(0x2<<28)|(0x2<<26));
	#endif
	
	pIOPort->GPJDAT &=~ (0x7<<13);
	pIOPort->GPJDAT |= ((0x1<<15)|(0x1<<14)|(0x1<<13));
	
	#ifdef		__CHK2443	
	//GPH - This configuration is setting for CFG3(UART) set as [off:off:off:off]
	pIOPort->GPHCON &= ~((0x3<<14)|(0x3<<12)|(0x3<<10)|(0x3<<8));	
	pIOPort->GPHUDP &= ~((0x3<<14)|(0x3<<12)|(0x3<<10)|(0x3<<8));
	#endif
	



///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_LCD IO
	// GPC[15:0], GPD[15:0]
	
	//GPC
	pIOPort->GPCCON &=~(0xffffffff<<0);
	pIOPort->GPCCON |= ((0x0<<30)|(0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)\
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	
	pIOPort->GPCUDP &=~(0xffffffff<<0);
	#ifdef		__CHK2443	
	pIOPort->GPCUDP |= ((0x2<<30)|(0x2<<28)|(0x2<<26)|(0x2<<24)|(0x2<<22)|(0x2<<20)|(0x2<<18)|(0x2<<16)
	           |(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x2<<8)|(0x2<<6)|(0x2<<4)|(0x2<<2)|(0x2<<0));
	#else
	pIOPort->GPCUDP |= ((0x0<<30)|(0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	#endif
	
	pIOPort->GPCDAT &=~ (0x1fff<<0);
	pIOPort->GPCDAT |= ((0x0<<14)|(0x1<<13)|(0x0<<12)|(0x1<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));

	//GPD
	pIOPort->GPDCON &=~(0xffffffff<<0);
	pIOPort->GPDCON |= ((0x0<<30)|(0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)\
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));	           
	
	           
	pIOPort->GPDUDP &=~(0xffffffff<<0);
	#ifdef		__CHK2443
	pIOPort->GPDUDP |= ((0x2<<30)|(0x2<<28)|(0x2<<26)|(0x2<<24)|(0x2<<22)|(0x2<<20)|(0x2<<18)|(0x2<<16)\
	           |(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x2<<8)|(0x2<<6)|(0x2<<4)|(0x2<<2)|(0x2<<0));
	#else
	pIOPort->GPDUDP |= ((0x0<<30)|(0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)\
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	#endif
	
	pIOPort->GPDDAT &=~ (0xffff<<0);
	pIOPort->GPDDAT |= ((0x0<<14)|(0x1<<13)|(0x0<<12)|(0x1<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));


///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_CAM IO
	// GPJ[15:0],
	
	//GPJ
	pIOPort->GPJCON &=~(0x3ffffff<<0);
	pIOPort->GPJCON |= ((0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)\
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	           
	pIOPort->GPJUDP &=~(0x3ffffff<<0);
	#ifdef		__CHK2443
	pIOPort->GPJUDP |= ((0x2<<24)|(0x2<<22)|(0x2<<20)|(0x2<<18)|(0x2<<16)\
	           |(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x2<<8)|(0x2<<6)|(0x2<<4)|(0x2<<2)|(0x2<<0));
	#else
	pIOPort->GPJUDP |= ((0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)\
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	#endif
	
	pIOPort->GPJDAT &=~ (0x1fff<<0);
	pIOPort->GPJDAT |= ((0x0<<12)|(0x1<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)\
	             |(0x0<<7)|(0x1<<6)|(0x1<<5)|(0x1<<4)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));

///////////////////////////////////////////////////////////////////////////////////////////
	// VDD_RMOP
	//rMSLCON = 0x0;
	#ifdef		__CHK2443
	pIOPort->GPMCON &=~((0x3<<2) | (0x3<<0));
	pIOPort->GPMUDP &=~((0x3<<4) |(0x3<<2) | (0x3<<0));
	pIOPort->GPMUDP |= ((0x3<<4) | (0x3<<2) | (0x3<<0));
	#endif

///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_SMOP : sleep wakeup iteration fail or not?
	//rMSLCON = 0x0;
	#ifdef		__CHK2443	
	//rDATAPDEN &=~((0x1<<0)|(0x1<<1)|(0x1<<2)|(0x1<<3)|(0x1<<4)|(0x1<<5)); // reset value = 0x3f; --> 0x30 = 2uA
	//rDATAPDEN = (0x3<<4);
	#else
	pIOPort->GPKUDP &=~((0x1<<0)|(0x1<<1)|(0x1<<2)|(0x1<<3)|(0x1<<4)|(0x1<<5)); // reset value = 0x3f; --> 0x30 = 2uA
	pIOPort->GPKUDP |= (0x3<<4);
	#endif

}
#if 0
void ConfigStopGPIO(void)
{

    volatile S3C2450_IOPORT_REG *pIOPort = (S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	
	
///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_OP1 IO
	// GPF[7:0], GPG[7:0], GPE[15:14], GPE[4:0]

	
//GPF
	pIOPort->GPFCON &=~(0xffff<<0);
	pIOPort->GPFCON |= ((0x1<<14)|(0x1<<12)|(0x1<<10)|(0x1<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x2<<0));


	//pIOPort->EXTINT0 = (READEXTINT0(pIOPort->EXTINT0) & ~((1<<31) | (1<<27) | (1<<23) | (1<<19) | (1<<15) | (1<<11) | (1<<7) | (1<<3) ));
	//pIOPort->EXTINT0 = (READEXTINT0(pIOPort->EXTINT0)  | ((1<<31) | (1<<27) | (1<<23) | (1<<19) | (0<<15) | (1<<11) | (1<<7) | (1<<3) )); //rEXTINT0[11] = PD_dis(Because of VBUS_DET)

	pIOPort->GPFDAT &=~ (0xff<<0);
	pIOPort->GPFDAT |= ((0x0<<7)|(0x0<<6)|(0x0<<5)|(0x0<<4)|(0x0<<3)|(0x0<<2)|(0x0<<1)|(0x0<<0));
	
	//GPG
	pIOPort->GPGCON &=~((0x0<<14)|(0x0<<12)|(0x3<<10)|(0x3<<8)|(0x3<<6)|(0x3<<4)|(0x3<<2)|(0x3<<0));
	pIOPort->GPGCON |= ((0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x2<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	           


//	pIOPort->EXTINT1 = (READEXTINT1(pIOPort->EXTINT1) | (0<<31) | (0<<27) | (0<<23) | (0<<19) | (1<<15) | (0<<11) | (0<<7)| (0<<3) );	
	
	pIOPort->GPGDAT &=~ ((0x0<<7)|(0x0<<6)|(0x1<<5)|(0x1<<4)|(0x1<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));
	pIOPort->GPGDAT |= ((0x1<<7)|(0x1<<6)|(0x1<<5)|(0x1<<4)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));
	
	//pIOPort->GPACDH = (READGPACDH(pIOPort->GPACDL,pIOPort->GPACDH) | (1<<16) | (1<<5));	//GPACDH[16] is exclusive use of nRSTOUT and '1' is PRESET

	
	// DP0 / DN0
	pIOPort->MISCCR |= (1<<12); //Set USB port as suspend mode	

	
///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_OP2 IO
	// GPB[10:0], GPH[12:0], GPE[15:14], GPE[4:0]

	
	//GPB
	pIOPort->GPBCON &= ~((0x3<<20)|(0x3<<18)|(0x3<<16)|(0x3<<14)|(0x3<<12)|(0x3<<10)|(0x3<<8)|(0x3<<6)|(0x0<<4)|(0x3<<2)|(0x3<<0));

	pIOPort->GPBCON |= ((0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x1<<4)|(0x0<<2)|(0x3<<0));

	
	pIOPort->GPBUDP &= ~((0x3<<20)|(0x3<<18)|(0x3<<16)|(0x3<<14)|(0x3<<12)|(0x3<<10)|(0x3<<8)|(0x3<<6)|(0x0<<4)|(0x3<<2)|(0x3<<0));

	pIOPort->GPBUDP |= ((0x3<<20)|(0x2<<18)|(0x2<<16)|(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x3<<8)|(0x3<<6)|(0x3<<4)|(0x0<<2)|(0x3<<0));

	
	
	//GPE
	pIOPort->GPECON &=~((0xf<<28)|(0x3ff<<0));	

	pIOPort->GPECON |= ((0x0<<30)|(0x0<<28)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));

	pIOPort->GPEUDP &=~((0xf<<28)|(0x3ff<<0));

	pIOPort->GPEUDP |= ((0x0<<30)|(0x0<<28)|(0x0<<8)|(0x3<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));                

	
	pIOPort->GPEDAT &=~ (0xffff<<0);
	pIOPort->GPEDAT |= ((0x1<<15)|(0x1<<14)|(0x1<<4)|(0x1<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));

	
	
	//GPG
	pIOPort->GPGCON &=~(0xffff<<16);
	pIOPort->GPGCON |= ((0x0<<30)|(0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16));
	
	pIOPort->GPGUDP &=~(0xffff<<16);

	pIOPort->GPGUDP |= ((0x2<<30)|(0x0<<28)|(0x3<<26)|(0x3<<24)|(0x0<<22)|(0x3<<20)|(0x3<<18)|(0x3<<16));

	pIOPort->GPGDAT &=~ (0xff<<8);
	pIOPort->GPGDAT |= ((0x0<<15)|(0x0<<14)|(0x0<<13)|(0x0<<12)|(0x0<<11)|(0x0<<10)|(0x0<<9)|(0x0<<8));

	
	
	//GPH
	pIOPort->GPHCON &=~(0x3fffffff<<0);

	pIOPort->GPHCON |= ((0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));

	pIOPort->GPHUDP &=~(0x3fffffff<<0);

	pIOPort->GPHUDP |= ((0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x3<<20)|(0x0<<18)|(0x3<<16)|(0x3<<6)|(0x0<<4)|(0x3<<2)|(0x0<<0));

	pIOPort->GPHDAT &=~ (0x7fff<<0);
	pIOPort->GPHDAT |= ((0x0<<14)|(0x1<<13)|(0x0<<12)|(0x1<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));

	

///////////////////////////////////////////////////////////////////////////////////////////
	// VDD_SD IO
	// GPE[13:5], GPL[14:0], GPJ[15:13]
	
	//GPE
	pIOPort->GPECON &=~(0x3ffff<<10);
	pIOPort->GPECON |= ((0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<14)|(0x0<<12)|(0x0<<10));//Set Input All
	
	pIOPort->GPEUDP &=~(0x3ffff<<10);

	pIOPort->GPEUDP |= ((0x0<<26)|(0x0<<24)|(0x0<<22)|(0x3<<20)|(0x3<<18)|(0x3<<16)|(0x3<<14)|(0x3<<12)|(0x0<<10));

	
	pIOPort->GPEDAT &=~ (0x1ff<<5);
	pIOPort->GPEDAT |= ((0x0<<13)|(0x0<<12)|(0x0<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x0<<7)|(0x1<<6)|(0x1<<5));

	
	//GPL
	pIOPort->GPLCON &=~(0x3fffffff<<0);
	pIOPort->GPLCON |= ((0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)|(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	pIOPort->GPLUDP &=~(0x3fffffff<<0);

	pIOPort->GPLUDP |= ((0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x3<<16)|(0x3<<14)|(0x3<<12)|(0x3<<10)|(0x3<<8)|(0x3<<6)|(0x3<<4)|(0x3<<2)|(0x3<<0));

	pIOPort->GPLDAT &=~ (0x1fff<<0);
	pIOPort->GPLDAT |= ((0x0<<14)|(0x0<<13)|(0x0<<12)|(0x0<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x1<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));
	
	//GPJ
	pIOPort->GPJCON &=~(0x3f<<26);
	pIOPort->GPJCON |= ((0x0<<30)|(0x0<<28)|(0x0<<26));
		
	pIOPort->GPJUDP &=~(0x3f<<26);

	pIOPort->GPJUDP |= ((0x0<<30)|(0x0<<28)|(0x0<<26));

	
	pIOPort->GPJDAT &=~ (0x7<<13);
	pIOPort->GPJDAT |= ((0x1<<15)|(0x1<<14)|(0x1<<13));
	




///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_LCD IO
	// GPC[15:0], GPD[15:0]
	
	//GPC
	pIOPort->GPCCON &=~(0xffffffff<<0);
	pIOPort->GPCCON |= ((0x0<<30)|(0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)\
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	
	pIOPort->GPCUDP &=~(0xffffffff<<0);

	pIOPort->GPCUDP |= ((0x2<<30)|(0x2<<28)|(0x2<<26)|(0x2<<24)|(0x2<<22)|(0x2<<20)|(0x2<<18)|(0x2<<16)
	           |(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x2<<8)|(0x2<<6)|(0x2<<4)|(0x2<<2)|(0x2<<0));

	
	pIOPort->GPCDAT &=~ (0x1fff<<0);
	pIOPort->GPCDAT |= ((0x0<<14)|(0x1<<13)|(0x0<<12)|(0x1<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));

	//GPD
	pIOPort->GPDCON &=~(0xffffffff<<0);
	pIOPort->GPDCON |= ((0x0<<30)|(0x0<<28)|(0x0<<26)|(0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)\
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));	           
	
	           
	pIOPort->GPDUDP &=~(0xffffffff<<0);

	pIOPort->GPDUDP |= ((0x2<<30)|(0x2<<28)|(0x2<<26)|(0x2<<24)|(0x2<<22)|(0x2<<20)|(0x2<<18)|(0x2<<16)\
	           |(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x2<<8)|(0x2<<6)|(0x2<<4)|(0x2<<2)|(0x2<<0));

	
	pIOPort->GPDDAT &=~ (0xffff<<0);
	pIOPort->GPDDAT |= ((0x0<<14)|(0x1<<13)|(0x0<<12)|(0x1<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));


///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_CAM IO
	// GPJ[15:0],
	
	//GPJ
	pIOPort->GPJCON &=~(0x3ffffff<<0);
	pIOPort->GPJCON |= ((0x0<<24)|(0x0<<22)|(0x0<<20)|(0x0<<18)|(0x0<<16)\
	           |(0x0<<14)|(0x0<<12)|(0x0<<10)|(0x0<<8)|(0x0<<6)|(0x0<<4)|(0x0<<2)|(0x0<<0));
	           
	pIOPort->GPJUDP &=~(0x3ffffff<<0);

	pIOPort->GPJUDP |= ((0x2<<24)|(0x2<<22)|(0x2<<20)|(0x2<<18)|(0x2<<16)\
	           |(0x2<<14)|(0x2<<12)|(0x2<<10)|(0x2<<8)|(0x2<<6)|(0x2<<4)|(0x2<<2)|(0x2<<0));

	
	pIOPort->GPJDAT &=~ (0x1fff<<0);
	pIOPort->GPJDAT |= ((0x0<<12)|(0x1<<11)|(0x0<<10)|(0x1<<9)|(0x1<<8)\
	             |(0x0<<7)|(0x1<<6)|(0x1<<5)|(0x1<<4)|(0x0<<3)|(0x1<<2)|(0x1<<1)|(0x1<<0));

///////////////////////////////////////////////////////////////////////////////////////////
	// VDD_RMOP
	pIOPort->MSLCON = 0x0;

///////////////////////////////////////////////////////////////////////////////////////////	
	// VDD_SMOP : sleep wakeup iteration fail or not?
	pIOPort->MSLCON = 0x0;

	pIOPort->DATAPDEN &=~((0x1<<0)|(0x1<<1)|(0x1<<2)|(0x1<<3)|(0x1<<4)|(0x1<<5)); // reset value = 0x3f; --> 0x30 = 2uA
	pIOPort->DATAPDEN = (0x3<<4);

}
#endif

#define USE_2CE_NAND

// NFCONF
#define NF_NANDBOOT	(0x1<<31)
#define NF_1BIT_ECC		(0x0<<23)
#define NF_4BIT_ECC		(0x2<<23)
#define NF_8BIT_ECC		(0x1<<23)
#define NF_TACLS(n)		(((n)&0x7)<<12)
#define NF_TWRPH0(n)	(((n)&0x7)<<8)
#define NF_TWRPH1(n)	(((n)&0x7)<<4)

// NFCONT
#define NF_4BIT_ECC_DEC		(0x0<<18)
#define NF_4BIT_ECC_ENC		(0x1<<18)
#define NF_LOCK_TIGHT_EN		(0x1<<17)
#define NF_SOFT_LOCK_EN		(0x1<<16)
#define NF_ECC_ENC_INT_EN		(0x1<<13)
#define NF_ECC_DEC_INT_EN		(0x1<<12)
#define NF_ILLACC_INT_EN		(0x1<<10)
#define NF_RNB_INT_EN			(0x1<<9)
#define NF_RNB_DETECT_RISE		(0x0<<8)
#define NF_RNB_DETECT_FALL		(0x1<<8)
#define NF_MAIN_ECC_UNLOCK	(0x0<<7)
#define NF_MAIN_ECC_LOCK		(0x1<<7)
#define NF_SPARE_ECC_UNLOCK	(0x0<<6)
#define NF_SPARE_ECC_LOCK		(0x1<<6)
#define NF_INIT_MECC			(0x1<<5)
#define NF_INIT_SECC			(0x1<<4)
#define NF_NFCE1				(0x1<<2)
#define NF_NFCE0				(0x1<<1)
#define NF_NFCON_DIS			(0x0)
#define NF_NFCON_EN			(0x1)

// NFSTAT
#define NF_ECC_ENC_DONE	(0x1<<7)
#define NF_ECC_DEC_DONE	(0x1<<6)
#define NF_ILLEGAL_ACCESS	(0x1<<5)
#define NF_RNB_TRANS            (0x1<<4)
#define NF_NFCE1_HI			(0x1<<3)
#define NF_NFCE0_HI			(0x1<<2)
#define NF_RNB_BUSY			(0x0)
#define NF_RNB_READY		(0x1)

// Default NAND Flash timing @HCLK 133MHz (tHCLK = 7.5ns)
#define	 DEFAULT_TACLS		(1)	// 1 HCLK (7.5ns)
//#define	 DEFAULT_TWRPH0	(3)	// 4 HCLK (30ns)
#define	 DEFAULT_TWRPH0	(4)	// 5 HCLK (37.5ns)
#define	 DEFAULT_TWRPH1	(1)	// 2 HCLK (15ns)

// Initialize nand controller for MLC with support 2CE
VOID InitNAND()
{
	volatile S3C2450_NAND_REG * pNANDFConReg = (volatile S3C2450_NAND_REG *)0xB1500000;			// 0x4E000000
	volatile S3C2450_MATRIX_REG * pMatrixConReg = (volatile S3C2450_MATRIX_REG *)0xB1600000;			// 0x4E800000
	volatile S3C2450_DMA_REG *pDMAConReg = (volatile S3C2450_DMA_REG *)0xB0E00000;				// 0x4B000000

	pNANDFConReg->NFCONF = NF_4BIT_ECC | NF_TACLS(DEFAULT_TACLS) | NF_TWRPH0(DEFAULT_TWRPH0) | NF_TWRPH1(DEFAULT_TWRPH1);
	pNANDFConReg->NFCONT = NF_MAIN_ECC_LOCK | NF_SPARE_ECC_LOCK | NF_INIT_MECC | NF_INIT_SECC | NF_NFCE1 | NF_NFCE0 | NF_NFCON_EN;
	pNANDFConReg->NFSTAT = NF_RNB_READY;	// Clear RnB Transition Detect Bit	

	// Initialize EBICON for 2nd nCE pin (nFCE1)
#ifdef	USE_2CE_NAND
	pMatrixConReg->EBICON |= (0x1<<8);	// Bank1_Cfg -> NAND
#endif

}

//[david.modify] 2008-06-19 16:19
extern void InitDisplay();
extern void InitDisplay2();

VOID BSPPowerOn()
{
    volatile S3C2450_IOPORT_REG *pIOPort = (S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    volatile S3C2450_LCD_REG *pLCD = (S3C2450_LCD_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
    volatile S3C2450_CLKPWR_REG *pCLKPWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);
	volatile BSP_ARGS *pBspArgs = (BSP_ARGS*)OALPAtoVA(IMAGE_SHARE_ARGS_PA_START, FALSE);


	
	OEMInitDebugSerial();


	
	InitTimer();			// consume 2us
	//OALTimerInit(RESCHED_PERIOD, (OEM_COUNT_1MS), 0);
	//
#if (BSP_TYPE == BSP_SMDK2443)
	InitLDI_LTV350();
#elif (BSP_TYPE == BSP_SMDK2450)
//	InitLDI_LTE480();

 //[david.modify] 2008-08-15 22:14
 // 加上InitDisplay();会产生唤醒后关闭MEDIA PLAYER挂机问题
///======================

//[david.modify] 2008-06-19 16:05
//[david.modify] 2008-07-04 18:08
//LCD_Sharp_ExitFromSleep是造成屏闪的问题的原因
//	InitDisplay();
	InitDisplay2();	// only初始化屏
//	InitLDI_LTV350_sharp2();

#endif

	InitNAND();

    
	// USB2.0 and PHY power
	pIOPort->MISCCR&=~(0x1<<12); //USB port = suspend
	//pCLKPWR->USB_PHYPWR |= (0xf<<0);
	pCLKPWR->PWRCFG |= (0x1<<4);
	//pIOPort->GSTATUS2 = pIOPort->GSTATUS2;

    //pIOPort->MISCCR &= ~(1<<12); //USB port0 = normal mode
    //pIOPort->MISCCR &= ~(1<<13); //USB port1 = normal mode

 //[david.modify] 2008-06-19 16:17
 //此句打开了LCD POWER
 //==============================
    /* LCD Controller Enable               */
//    SETPORT32(&pIOPort->GPGDAT, 1 << 4);
	 //[david.modify] 2008-06-19 15:21
//===========================	 
}

#ifdef VARTICK
void	InitTimer()
{
	S3C2450_PWM_REG *pPWMRegs = (S3C2450_PWM_REG*)OALPAtoUA(S3C2450_BASE_REG_PA_PWM);
    UINT32 countsPerSysTick;
    UINT32 tcon;
    
    // Validate Input parameters
    countsPerSysTick = RESCHED_PERIOD * OEM_COUNT_1MS;    
    // Set prescaler 1 to 1 
    //OUTREG32(&pPWMRegs->TCFG0, INREG32(&pPWMRegs->TCFG0) & ~0x0000FF00);
    //OUTREG32(&pPWMRegs->TCFG0, INREG32(&pPWMRegs->TCFG0) | (250-1) <<8);		// charlie, Timer4 scale value
    
     // Set prescaler 1 to 1 
    OUTREG32(&pPWMRegs->TCFG0, INREG32(&pPWMRegs->TCFG0) & ~0x0000FF00);
    OUTREG32(&pPWMRegs->TCFG0, INREG32(&pPWMRegs->TCFG0) | PRESCALER <<8);
    
    // Select MUX input 1/2
    //OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) & ~(0xF << 16));
    //OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (2 << 16));	// charlie, Timer4 Division    
    
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) & ~(0xF << 16));
#if( SYS_TIMER_DIVIDER == D2 )
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (D1_2 << 16));
#elif ( SYS_TIMER_DIVIDER == D4 )
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (D1_4 << 16));
#elif ( SYS_TIMER_DIVIDER == D8 )
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (D1_8 << 16));
#elif ( SYS_TIMER_DIVIDER == D16 )
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (D1_16 << 16));
#endif

    // Set timer register
    OUTREG32(&pPWMRegs->TCNTB4, countsPerSysTick);

    // Start timer in auto reload mode
    tcon = INREG32(&pPWMRegs->TCON) & ~(0x0F << 20);
    OUTREG32(&pPWMRegs->TCON, tcon | (0x2 << 20) );
    OUTREG32(&pPWMRegs->TCON, tcon | (0x1 << 20) );	
}
#else 
void	InitTimer()
{
	S3C2450_PWM_REG *pPWMRegs = (S3C2450_PWM_REG*)OALPAtoUA(S3C2450_BASE_REG_PA_PWM);
    UINT32 countsPerSysTick;
    UINT32 tcon;
    
    // Validate Input parameters
    countsPerSysTick = RESCHED_PERIOD * OEM_COUNT_1MS;    
    // Set prescaler 1 to 1 
    //OUTREG32(&pPWMRegs->TCFG0, INREG32(&pPWMRegs->TCFG0) & ~0x0000FF00);
    //OUTREG32(&pPWMRegs->TCFG0, INREG32(&pPWMRegs->TCFG0) | (250-1) <<8);		// charlie, Timer4 scale value
    
     // Set prescaler 1 to 1 
    OUTREG32(&pPWMRegs->TCFG0, INREG32(&pPWMRegs->TCFG0) & ~0x0000FF00);
    OUTREG32(&pPWMRegs->TCFG0, INREG32(&pPWMRegs->TCFG0) | PRESCALER <<8);
    
    // Select MUX input 1/2
    //OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) & ~(0xF << 16));
    //OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (2 << 16));	// charlie, Timer4 Division    
    
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) & ~(0xF << 16));
#if( SYS_TIMER_DIVIDER == D2 )
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (D1_2 << 16));
#elif ( SYS_TIMER_DIVIDER == D4 )
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (D1_4 << 16));
#elif ( SYS_TIMER_DIVIDER == D8 )
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (D1_8 << 16));
#elif ( SYS_TIMER_DIVIDER == D16 )
    OUTREG32(&pPWMRegs->TCFG1, INREG32(&pPWMRegs->TCFG1) | (D1_16 << 16));
#endif

    // Set timer register
    OUTREG32(&pPWMRegs->TCNTB4, countsPerSysTick);

    // Start timer in auto reload mode
    tcon = INREG32(&pPWMRegs->TCON) & ~(0x0F << 20);
    OUTREG32(&pPWMRegs->TCON, tcon | (0x2 << 20) );
    OUTREG32(&pPWMRegs->TCON, tcon | (0x5 << 20) );	
}

#endif

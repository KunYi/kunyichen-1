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

Module Name:  

Abstract:

    Serial PDD for SamSang 2450 Development Board.

Notes: 
--*/
#include <windows.h>
#include <types.h>
#include <ceddk.h>

#include <ddkreg.h>
#include <serhw.h>
#include <Serdbg.h>
#include <bsp.h>
#include <pdds3c2450_ser.h>
#include <s3c2450_base_regs.h>
#include <s3c2450_ioport.h>

#define BSP_GPS_GLONAV4540  100
#define BSP_GPS_GLONAV7560  101
#define BSP_GPS_NX3         200

#ifndef BSP_GPS_TYPE
   #define BSP_GPS_TYPE     BSP_GPS_NX3
#endif

// CPdd2450Serial1 is only use for UART0 which 
// RxD0 & TxD0 uses GPH1 & GPH0 respectively
// RTS0 & CTS0 uses GPH9 & GPH8 respectively
// DTR0 & DSR0 uses EINT0/GPF0 & EINT11/GPG3 respectively
class CPdd2450Serial0 : public CPdd2450Uart {
public:
    CPdd2450Serial0(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj)
        : CPdd2450Uart(lpActivePath, pMdd, pHwObj)
        {
        m_pIOPregs = NULL;
        m_fIsDSRSet = FALSE;
    }
    ~CPdd2450Serial0() {
        if (m_pIOPregs!=NULL)
            MmUnmapIoSpace((PVOID)m_pIOPregs,0);
    }
    virtual BOOL Init() {
        PHYSICAL_ADDRESS    ioPhysicalBase = { S3C2450_BASE_REG_PA_IOPORT, 0};
        ULONG               inIoSpace = 0;
        if (TranslateBusAddr(m_hParent,Internal,0, ioPhysicalBase,&inIoSpace,&ioPhysicalBase)) {
            // Map it if it is Memeory Mapped IO.
            m_pIOPregs = (S3C2450_IOPORT_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C2450_IOPORT_REG),FALSE);
        }
        if (m_pIOPregs) {
            DDKISRINFO ddi;
            if (GetIsrInfo(&ddi)== ERROR_SUCCESS && 
                    KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &ddi.dwIrq, sizeof(UINT32), &ddi.dwSysintr, sizeof(UINT32), NULL))
            {   
 //           	RETAILMSG( TRUE, (TEXT("DEBUG: Serial0 SYSINTR : %d\r\n"), (PBYTE)&ddi.dwSysintr)); 
                RegSetValueEx(DEVLOAD_SYSINTR_VALNAME,REG_DWORD,(PBYTE)&ddi.dwSysintr, sizeof(UINT32));
            }
            else
                return FALSE;
	        m_pIOPregs->GPHCON &= ~(0x3<<0 | 0x3<<2 | 0x3<<16 | 0x3<<18 );	///< Clear Bit
			m_pIOPregs->GPHCON |=  (0x2<<0 | 0x2<<2 | 0x2<<16 | 0x2<<18 ); 	///< Select UART IP
            m_pDTRPort = (volatile ULONG *)&(m_pIOPregs->GPCDAT);
            m_pDSRPort = (volatile ULONG *)&(m_pIOPregs->GPCDAT);
            m_dwDTRPortNum = 6;
            m_dwDSRPortNum = 5;
			m_pIOPregs->GPHUDP &= ~(0x3<<0 | 0x3<<2 | 0x3<<16 | 0x3<<18 );
			// DTR0(GPC6), DSR0(GPC7)

 //[david.modify] 2008-10-22 15:16
// 1. 因为UARTO有用到GPC6，GPC7; 这个会用到UART0的ACTIVESYNC，所以造成4.3INCH项目莫名出现同步FAIL对话框。
//>> 修改方法， 在驱动中拿掉
#if 0
						// If you want to use COM1 port for ActiveSync, use these statements.     
	        m_pIOPregs->GPCCON &= ~(0x3<<12); ///<dtr0
			m_pIOPregs->GPCCON |= (0x1<<12);	///< Output
			m_pIOPregs->GPCUDP &= ~(0x3<<12);	/// pull-up disable 
	        m_pIOPregs->GPCCON &= ~(0x3<<10); ///<dsr0
			m_pIOPregs->GPCCON |= (0x0<<10);	///< Input			
			m_pIOPregs->GPCUDP &= ~(0x3<<10);	/// pull-up disable					       					       
#endif			

            return CPdd2450Uart::Init();
        }
        return FALSE;
    };
    virtual void    SetDefaultConfiguration() {
        CPdd2450Uart::SetDefaultConfiguration();
    }
    virtual BOOL    InitModem(BOOL bInit) {
        SetDTR(bInit);
        return CPdd2450Uart::InitModem(bInit);
    }
    virtual ULONG   GetModemStatus() {
 //[david.modify] 2008-10-22 15:16
// 1. 因为UARTO有用到GPC6，GPC7; 这个会用到UART0的ACTIVESYNC，所以造成4.3INCH项目莫名出现同步FAIL对话框。
//>> 修改方法， 在驱动中拿掉		
#if 0		
        ULONG ulReturn = CPdd2450Uart::GetModemStatus();
        ULONG ulEvent = 0;
        m_HardwareLock.Lock();
        BOOL fIsDSRSet = (((*m_pDSRPort) & (1<<m_dwDSRPortNum))==0);
 //       RETAILMSG(TRUE, (TEXT("DEBUG: DSRPort Register 0x%lx, Value 0x%lx, fIsDSRSet(%d).\r\n"), m_pDSRPort, *m_pDSRPort, fIsDSRSet)); 
        if (fIsDSRSet != m_fIsDSRSet) {
            ulEvent |= EV_DSR | EV_RLSD;
        }
        ulReturn |= (fIsDSRSet?(MS_DSR_ON|MS_RLSD_ON):0);
        m_fIsDSRSet = fIsDSRSet;
        m_HardwareLock.Unlock();
        if (ulEvent!=0)
            EventCallback(ulEvent,ulReturn);
        return ulReturn;
#else
        return (CPdd2450Uart::GetModemStatus() | MS_CTS_ON);
#endif
    }
    virtual void    SetDTR(BOOL bSet) {
//    	RETAILMSG(TRUE, (TEXT("DEBUG: DTRPort Register 0x%x, DTRSet?(%d).\r\n"), m_pDTRPort, bSet)); 
        if (bSet){
            *m_pDTRPort &= ~(1<<m_dwDTRPortNum);
//           	RETAILMSG(TRUE, (TEXT("DEBUG: DTRSet Bit=(%d).\r\n"), (*m_pDTRPort & (1<<m_dwDTRPortNum) ) )); 
        }
        else{
             *m_pDTRPort |= (1<<m_dwDTRPortNum);
        }
    };
private:
    volatile S3C2450_IOPORT_REG * m_pIOPregs; 
    volatile ULONG *    m_pDTRPort;
    DWORD               m_dwDTRPortNum;
    volatile ULONG *    m_pDSRPort;
    DWORD               m_dwDSRPortNum;
    BOOL                m_fIsDSRSet;
};



//[david.modify] 2008-05-29 17:44
#include <dbgmsg_david.h> 
#include <xllp_gpio_david.h>
stGPIOInfo g_stGPIOInfo[]={
	{ GPS_BOOT,  0, GPA_ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
	{ GPS_RESET,  1, GPA_ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
	{ GPS_POWER_EN,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ GPS_RXD,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},		
	{ GPS_TXD,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},			
	};

//volatile S3C2450_IOPORT_REG *v_pIOPregs=NULL;
//==========================

/// CPdd2450Serial_Irda is only use for IrDA(UART2)
/// DeviceArrayIndex is 2
/// IrDA enabling is dependent to board jumper setting.
/// We assume that jumper setting is correct.
/// RxD2 & TxD2 uses GPH5 & GPH4 respectively

class CPdd2450Serial_IrDA : public CPdd2450Uart {
public:
    CPdd2450Serial_IrDA(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj)
        : CPdd2450Uart(lpActivePath, pMdd, pHwObj)
        {
        m_pIOPregs = NULL;
    }
    ~CPdd2450Serial_IrDA() {
        if (m_pIOPregs!=NULL)
            MmUnmapIoSpace((PVOID)m_pIOPregs,0);
    }
    virtual BOOL Init() {
        PHYSICAL_ADDRESS    ioPhysicalBase = { S3C2450_BASE_REG_PA_IOPORT, 0};
        ULONG               inIoSpace = 0;
        if (TranslateBusAddr(m_hParent,Internal,0, ioPhysicalBase,&inIoSpace,&ioPhysicalBase)) {
            // Map it if it is Memeory Mapped IO.
            m_pIOPregs = (S3C2450_IOPORT_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C2450_IOPORT_REG),FALSE);
        }
        if (m_pIOPregs) {
            DDKISRINFO ddi;
            if (GetIsrInfo(&ddi)== ERROR_SUCCESS && 
                    KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &ddi.dwIrq, sizeof(UINT32), &ddi.dwSysintr, sizeof(UINT32), NULL))
            {   
//            	RETAILMSG( TRUE, (TEXT("DEBUG: Serial_IrDA SYSINTR : %d\r\n"), (PBYTE)&ddi.dwSysintr)); 
                RegSetValueEx(DEVLOAD_SYSINTR_VALNAME,REG_DWORD,(PBYTE)&ddi.dwSysintr, sizeof(UINT32));
            }
            else
                return FALSE;

			// TXD2(GPH4), RXD2(GPH5)
	        m_pIOPregs->GPHCON &= ~(0x3<<8 | 0x3<<10); 
	        m_pIOPregs->GPHCON |= (0x2<<8 | 0x2<<10); 

        #if (BSP_GPS_TYPE == BSP_GPS_NX3 )
            m_pDTRPort = (volatile ULONG *)&(m_pIOPregs->GPHDAT);
            m_dwDTRPortNum = 4;
        #endif

    	    m_pIOPregs->GPHUDP &= ~(0x1<<(8) | 0x1<<(10));
            return CPdd2450Uart::Init();
        }
        return FALSE;
    };
    virtual void    SetDefaultConfiguration() {
        CPdd2450Uart::SetDefaultConfiguration();
    }
    virtual ULONG   GetModemStatus() {
        return (CPdd2450Uart::GetModemStatus() | MS_CTS_ON);
    }
    /// change GPIO between RXD2 and Input
    virtual void    Rx_Pause(BOOL bSet) {
    	if(bSet){		///< from RXD2 to Input
    		m_pIOPregs->GPHCON = (m_pIOPregs->GPHCON & ~(0x3<<10)) | 0x0<<10;
    	}
    	else{			///< from Input to RXD2
    		m_pIOPregs->GPHCON = (m_pIOPregs->GPHCON & ~(0x3<<10)) | 0x2<<10;
    	}
    }


 //[david.modify] 2008-07-17 11:58
//static BOOL  bPower_BeforeSleep = FALSE;
BOOL HAL_GetGPSPower_BeforeSleep()
{
	int nRet = 0;
 //[david.modify] 2008-07-17 11:50
 // 得到GPS VCC状态，做为返回值
	GetGPIOInfo(&g_stGPIOInfo[2], (void*)m_pIOPregs);	 		
	nRet = g_stGPIOInfo[2].u32Stat;

	DPNOK(nRet);
//	bPower_BeforeSleep = nRet;
	return (BOOL)nRet;
}



 BOOL HAL_SwitchGPSPower(BOOL bEnable)
{
	int nRet = 0;
	PHYSICAL_ADDRESS ioPhysicalBase = {S3C2450_BASE_REG_PA_IOPORT, 0};
	
	if(!m_pIOPregs) {        	
        	m_pIOPregs = (S3C2450_IOPORT_REG*)MmMapIoSpace(ioPhysicalBase, sizeof(S3C2450_IOPORT_REG),FALSE);
		SetGPIOInfo(&g_stGPIOInfo[0], (void*)m_pIOPregs);	 			
		SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);	 					
   	}





	if( bEnable) {
//==============================================		
			// TXD2(GPH4), RXD2(GPH5)
	        m_pIOPregs->GPHCON &= ~(0x3<<8 | 0x3<<10); 
	        m_pIOPregs->GPHCON |= (0x2<<8 | 0x2<<10); 
#if (BSP_TYPE == BSP_SMDK2443)
    	    m_pIOPregs->GPHUDP |= (0x1<<(8) | 0x1<<(10));
#elif (BSP_TYPE == BSP_SMDK2450)
    	    m_pIOPregs->GPHUDP &= ~(0x1<<(8) | 0x1<<(10));
#endif
		Sleep(20);	
//==============================================
		

 //[david.modify] 2008-07-14 16:33
#if 0 	

		g_stGPIOInfo[2].u32Stat = 1;					// 位高gps电源
		nRet =  SetGPIOInfo(&g_stGPIOInfo[2], (void*)m_pIOPregs);	 
#else		
		DPSTR("RESET-ON");

#if 0
		g_stGPIOInfo[1].u32AltFunc=GPA_ALT_FUNC_OUT; 			
		g_stGPIOInfo[1].u32Stat=1;
		SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);	// gps reset
		Sleep(1);
		
		g_stGPIOInfo[1].u32Stat=0;
		SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);	// gps reset
		Sleep(200);
#endif		


		g_stGPIOInfo[1].u32AltFunc=GPA_ALT_FUNC_OUT; 	
		g_stGPIOInfo[1].u32Stat=0;
		SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);	// gps reset	
		Sleep(20);		

		g_stGPIOInfo[2].u32Stat = 1;					// gps vcc
		g_stGPIOInfo[2].u32AltFunc = ALT_FUNC_OUT;		
		nRet =  SetGPIOInfo(&g_stGPIOInfo[2], (void*)m_pIOPregs);	 							
		Sleep(200+500);	//睡眠500MS

		g_stGPIOInfo[1].u32AltFunc=GPA_ALT_FUNC_OUT; 	
		g_stGPIOInfo[1].u32Stat=1;
		SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);	// gps reset	

 //[david.modify] 2008-07-17 11:50
 // 得到GPS VCC状态，做为返回值
		GetGPIOInfo(&g_stGPIOInfo[2], (void*)m_pIOPregs);	 		
		nRet = g_stGPIOInfo[2].u32Stat;
		
		
		
#endif		
	}
	else{

 //[david.modify] 2008-08-08 16:23
 // 发现TXD，RXD串电入GPS CHIP，造成电关不掉
 //==========================
 		DPSTR("Rx_Pause");
//		Rx_Pause(TRUE);
		 //[david.modify] 2008-08-08 16:50
		 // 将TXD，RXD都改成输入
 		// TXD2(GPH4), RXD2(GPH5)
//    		m_pIOPregs->GPHCON = (m_pIOPregs->GPHCON & ~(0x3<<8)) | 0x0<<8;	
    		//m_pIOPregs->GPHCON = (m_pIOPregs->GPHCON & ~(0x3<<10)) | 0x0<<10;	


#if 1
		m_pIOPregs->GPHDAT=(m_pIOPregs->GPHDAT & ~(0x1<<4)) | 0x0<<4;	
		m_pIOPregs->GPHDAT=(m_pIOPregs->GPHDAT & ~(0x1<<5)) | 0x0<<5;	
    		m_pIOPregs->GPHCON = (m_pIOPregs->GPHCON & ~(0x3<<8)) | 0x1<<8;	
    		m_pIOPregs->GPHCON = (m_pIOPregs->GPHCON & ~(0x3<<10)) | 0x1<<10;	
		m_pIOPregs->GPHDAT=(m_pIOPregs->GPHDAT & ~(0x1<<4)) | 0x0<<4;	
		m_pIOPregs->GPHDAT=(m_pIOPregs->GPHDAT & ~(0x1<<5)) | 0x0<<5;				
		Sleep(20);		
#endif
 		
 //==========================	
		g_stGPIOInfo[1].u32Stat=0;
		g_stGPIOInfo[1].u32AltFunc=GPA_ALT_FUNC_OUT; 	
		SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);	// gps reset
		Sleep(20);		
		
		g_stGPIOInfo[2].u32Stat = 0;						// gps vcc
		g_stGPIOInfo[2].u32AltFunc = ALT_FUNC_OUT;				
		nRet =  SetGPIOInfo(&g_stGPIOInfo[2], (void*)m_pIOPregs);	 	

		Sleep(100);		
		 //[david.modify] 2008-08-08 10:34
		  //[david.modify] 2008-08-08 17:18
		  // 将复位信号再次拉低以保证不串入2.5V
		 //================================
		g_stGPIOInfo[1].u32Stat=0;	//将GPS RESET信号又拉回高
		g_stGPIOInfo[1].u32AltFunc=GPA_ALT_FUNC_OUT; 			
		SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);	// gps reset		
		Sleep(700);
		 //================================
 //[david.modify] 2008-07-17 11:50
 // 得到GPS VCC状态，做为返回值
		GetGPIOInfo(&g_stGPIOInfo[2], (void*)m_pIOPregs);	 		
		nRet = g_stGPIOInfo[2].u32Stat;		
	}
	
	return nRet;
}	
	

virtual BOOL    Open()
{
	BOOL bRet = 0, bRet2=0;

	bRet2 = HAL_SwitchGPSPower(1);
//	DPNOK(bRet2);	
	if(bRet2) {
		DPSTR("==GPS PWR ON==");
		m_bPower_BeforeSleep = TRUE;
//		HAL_SwitchGPSPowerLED(GPS_PWRLED_GPIO_ON);
	}else{
		DPSTR("ERROR: ==GPS NOT PWR ON==");
	}
	bRet = CPdd2450Uart::Open();

	return bRet;

}
virtual BOOL    Close()
{
	BOOL bRet = 0;
	bRet = HAL_SwitchGPSPower(0);
	DPNOK(bRet);	
	if(!bRet) {
		DPSTR("==GPS PWR OFF==");
		m_bPower_BeforeSleep = FALSE;
//		HAL_SwitchGPSPowerLED(GPS_PWRLED_GPIO_OFF);		
	}else{
		DPSTR("ERROR: ==GPS NOT PWR OFF==");
	}
	return CPdd2450Uart::Close();
}	


 //[david.modify] 2008-07-17 11:35
 //=====================================
 // GLONAV的机器需要在SLEEP/WAKEUP时控制时序
 virtual BOOL    PowerOff()
 {
	BOOL bRet = 0;


	// 得到当前GPS电源状态，是开还是关
	m_bPower_BeforeSleep =HAL_GetGPSPower_BeforeSleep();
	DPNOK(m_bPower_BeforeSleep);
	if(m_bPower_BeforeSleep) {			//如果打开了，则要在睡眠时关闭
		bRet = HAL_SwitchGPSPower(0);
		DPNOK(bRet);	
		if(!bRet) {
			DPSTR("==GPS PWR OFF==");
	//		HAL_SwitchGPSPowerLED(GPS_PWRLED_GPIO_OFF);		
		}else{
			DPSTR("ERROR: ==GPS NOT PWR OFF==");
		}	
	}
	return CPdd2450Uart::PowerOff();
 }

 virtual BOOL    PowerOn()
{
	BOOL bRet2=0;
	DPNOK(m_bPower_BeforeSleep);
	if(m_bPower_BeforeSleep) {
		bRet2 = HAL_SwitchGPSPower(1);
	//	DPNOK(bRet2);	
		if(bRet2) {
			DPSTR("==GPS PWR ON==");
	//		HAL_SwitchGPSPowerLED(GPS_PWRLED_GPIO_ON);
		}else{
			DPSTR("ERROR: ==GPS NOT PWR ON==");
		}
	}

	return CPdd2450Uart::PowerOn();
}

  #if (BSP_GPS_TYPE == BSP_GPS_NX3 )

    virtual void    SetDTR(BOOL bSet) {
        RETAILMSG(TRUE, (TEXT("----GPS TRSet? %d\r\n"), bSet));
        if (bSet){
            *m_pDTRPort &= ~(1<<m_dwDTRPortNum);
            g_stGPIOInfo[1].u32AltFunc=GPA_ALT_FUNC_OUT;
            g_stGPIOInfo[1].u32Stat=0;
            SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);   // gps reset
        }
        else{
            *m_pDTRPort |= (1<<m_dwDTRPortNum);
            g_stGPIOInfo[1].u32AltFunc=GPA_ALT_FUNC_OUT;
            g_stGPIOInfo[1].u32Stat=1;
            SetGPIOInfo(&g_stGPIOInfo[1], (void*)m_pIOPregs);   // gps reset
        }
    }

    volatile ULONG *    m_pDTRPort;
    DWORD               m_dwDTRPortNum;
  
  #endif

    volatile S3C2450_IOPORT_REG * m_pIOPregs; 

 //[david.modify] 2008-07-17 12:06
 //  确定睡眠前GPS是开还是关
 	BOOL m_bPower_BeforeSleep;
 
	
};

/// CPdd2450Serial1 is used for COM2(UART1)
/// enabling UART1 is dependent to board's jumper setting.
/// We assume that jumper setting is correct.
/// RTS1 & CTS1 uses GPH11 & GPH10 respectively
/// RxD1 & TxD1 uses GPH3 & GPH2 respectively

class CPdd2450Serial1 : public CPdd2450Uart {
public:
    CPdd2450Serial1(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj)
        : CPdd2450Uart(lpActivePath, pMdd, pHwObj)
        {
        m_pIOPregs = NULL;
    }
    ~CPdd2450Serial1() {
        if (m_pIOPregs!=NULL)
            MmUnmapIoSpace((PVOID)m_pIOPregs,0);
    }
    virtual BOOL Init() {
        PHYSICAL_ADDRESS    ioPhysicalBase = { S3C2450_BASE_REG_PA_IOPORT, 0};
        ULONG               inIoSpace = 0;
        if (TranslateBusAddr(m_hParent,Internal,0, ioPhysicalBase,&inIoSpace,&ioPhysicalBase)) {
            // Map it if it is Memeory Mapped IO.
            m_pIOPregs = (S3C2450_IOPORT_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C2450_IOPORT_REG),FALSE);
        }
        if (m_pIOPregs) {
            DDKISRINFO ddi;
            if (GetIsrInfo(&ddi)== ERROR_SUCCESS && 
                    KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &ddi.dwIrq, sizeof(UINT32), &ddi.dwSysintr, sizeof(UINT32), NULL))
            {   
//            	RETAILMSG( TRUE, (TEXT("DEBUG: Serial1 SYSINTR : %d\r\n"), (PBYTE)&ddi.dwSysintr)); 
                RegSetValueEx(DEVLOAD_SYSINTR_VALNAME,REG_DWORD,(PBYTE)&ddi.dwSysintr, sizeof(UINT32));
            }
            else
                return FALSE;

			// TXD1(GPH2), RXD1(GPH3), RTS1(GPH11), CTS1(GPH10)
	        m_pIOPregs->GPHCON &= ~(0x3<<4 | 0x3<<6 | 0x3<<20 | 0x3<<22); 
	        m_pIOPregs->GPHCON |= (0x2<<4 | 0x2<<6 | 0x2<<20 | 0x2<<22); 
			m_pIOPregs->GPHUDP &= ~( 0x1<<(4) | 0x1<<(6) | 0x1<<(20) | 0x1<<(22) );
            return CPdd2450Uart::Init();
        }
        return FALSE;
    };
    virtual void    SetDefaultConfiguration() {
        CPdd2450Uart::SetDefaultConfiguration();
    }
    virtual ULONG   GetModemStatus() {
        return (CPdd2450Uart::GetModemStatus() | MS_CTS_ON);
    }

    volatile S3C2450_IOPORT_REG * m_pIOPregs; 
};

/// CPdd2450Serial3 is used for COM3(UART3)
/// enabling UART3 is dependent to board's jumper setting.
/// We assume that jumper setting is correct.
/// UART3 has no RTS&CTS signal
/// RxD3 & TxD3 uses GPH7 & GPH6 respectively

class CPdd2450Serial3 : public CPdd2450Uart {
public:
    CPdd2450Serial3(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj)
        : CPdd2450Uart(lpActivePath, pMdd, pHwObj)
        {
        m_pIOPregs = NULL;
    }
    ~CPdd2450Serial3() {
        if (m_pIOPregs!=NULL)
            MmUnmapIoSpace((PVOID)m_pIOPregs,0);
    }
    virtual BOOL Init() {
        PHYSICAL_ADDRESS    ioPhysicalBase = { S3C2450_BASE_REG_PA_IOPORT, 0};
        ULONG               inIoSpace = 0;
        if (TranslateBusAddr(m_hParent,Internal,0, ioPhysicalBase,&inIoSpace,&ioPhysicalBase)) {
            // Map it if it is Memeory Mapped IO.
            m_pIOPregs = (S3C2450_IOPORT_REG *)MmMapIoSpace(ioPhysicalBase, sizeof(S3C2450_IOPORT_REG),FALSE);
        }
        if (m_pIOPregs) {
            DDKISRINFO ddi;
            if (GetIsrInfo(&ddi)== ERROR_SUCCESS && 
                    KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &ddi.dwIrq, sizeof(UINT32), &ddi.dwSysintr, sizeof(UINT32), NULL))
            {   
//            	RETAILMSG( TRUE, (TEXT("DEBUG: Serial3 SYSINTR : %d\r\n"), (PBYTE)&ddi.dwSysintr)); 
                RegSetValueEx(DEVLOAD_SYSINTR_VALNAME,REG_DWORD,(PBYTE)&ddi.dwSysintr, sizeof(UINT32));
            }
            else
                return FALSE;

			// TXD3(GPH6), RXD3(GPH7)
	        m_pIOPregs->GPHCON &= ~(0x3<<12 | 0x3<<14); 
	        m_pIOPregs->GPHCON |= (0x2<<12 | 0x2<<14);
    	    m_pIOPregs->GPHUDP &= ~( 0x1<<(12) | 0x1<<(14) );
            return CPdd2450Uart::Init();
        }
        return FALSE;
    };
    virtual void    SetDefaultConfiguration() {
        CPdd2450Uart::SetDefaultConfiguration();
    }
    virtual ULONG   GetModemStatus() {
        return (CPdd2450Uart::GetModemStatus() | MS_CTS_ON);
    }

    volatile S3C2450_IOPORT_REG * m_pIOPregs; 
};


CSerialPDD * CreateSerialObject(LPTSTR lpActivePath, PVOID pMdd,PHWOBJ pHwObj, DWORD DeviceArrayIndex)
{
    CSerialPDD * pSerialPDD = NULL;
	RETAILMSG( TRUE, (TEXT("DEBUG: CreateSerialObject %d\r\n"), DeviceArrayIndex)); 
    switch (DeviceArrayIndex) {
      case 0:		///< UART0
        pSerialPDD = new CPdd2450Serial0(lpActivePath,pMdd, pHwObj);
        break;
      case 1:		///< UART1
        pSerialPDD = new CPdd2450Serial1(lpActivePath,pMdd, pHwObj);
        break;
	  case 2:		///< UART2(IrDA)
		pSerialPDD = new CPdd2450Serial_IrDA(lpActivePath, pMdd, pHwObj);
		break;
	  case 3:		///< UART3
		pSerialPDD = new CPdd2450Serial3(lpActivePath, pMdd, pHwObj);
		break;
    }
    if (pSerialPDD && !pSerialPDD->Init()) {
        delete pSerialPDD;
        pSerialPDD = NULL;
    }    
    return pSerialPDD;
}
void DeleteSerialObject(CSerialPDD * pSerialPDD)
{
    if (pSerialPDD)
        delete pSerialPDD;
}



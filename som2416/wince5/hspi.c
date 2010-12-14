//-------------------------------------------------------------------------------------------------------------------------
// Copyright (c) Samsung Electronics Co., Ltd.  All rights reserved.
//-------------------------------------------------------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, 
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
// Module Name :    HSPI.C
//
// Abstract     :   HS-SPI Interface Routines for Samsung SC2450 CPU
//
// Environment :    Samsung SC2450 / CE5.0
//
// 2007/03/05
//
//-------------------------------------------------------------------------------------------------------------------------

#include <windows.h>
//#include <types.h>	//
#include <nkintr.h>

//#include <ceddk.h>	//For DMA Buffer Alloc

//#include <memory.h> //070205
//#include <bsp_cfg.h>  //070205 
#include <s3c2450.h>	//070205


//#include <excpt.h>
//#include <tchar.h>
//#include <cardserv.h>
//#include <cardapi.h>
//#include <tuple.h>
//#include <devload.h>
//#include <diskio.h>
//#include <windev.h>

#include "hspi.h"  //070307

#define MASTER_CS_ENABLE		pSPIregs->SLAVE_SELECTION_REG = 0
#define MASTER_CS_DISABLE		pSPIregs->SLAVE_SELECTION_REG = 1

#define TRAIL_CNT(n)				(((n)&0x3FF)<<19)


DWORD HW_Init(PSPI_PUBLIC_CONTEXT pPublicSpi);

DWORD ThreadForTx(PSPI_PUBLIC_CONTEXT pPublicSpi);
DWORD ThreadForRx(PSPI_PUBLIC_CONTEXT pPublicSpi);
DWORD ThreadForSpi(PSPI_PUBLIC_CONTEXT pPublicSpi);
DWORD ThreadForRxDmaDone(PSPI_PUBLIC_CONTEXT pPublicSpi);
DWORD ThreadForTxDmaDone(PSPI_PUBLIC_CONTEXT pPublicSpi);



BOOL
DllEntry(
    HINSTANCE   hinstDll,            
    DWORD   	dwReason,               
    LPVOID  	lpReserved)
{
    if ( dwReason == DLL_PROCESS_ATTACH ) {
        DEBUGMSG (1, (TEXT("SPI: Process Attach\r\n")));
    }

    if ( dwReason == DLL_PROCESS_DETACH ) {
        DEBUGMSG (1, (TEXT("SPI: Process Detach\r\n")));
    }

    return(TRUE);
}

DWORD
HW_Init(
    PSPI_PUBLIC_CONTEXT pPublicSpi
    )
{

	BOOL bResult = TRUE;


	if ( !pPublicSpi ) {
	    return FALSE;
	}
	


	// GPIO Virtual alloc
	pPublicSpi->pGPIOregs = (volatile S3C2450_IOPORT_REG *) VirtualAlloc(0,sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if(pPublicSpi->pGPIOregs == NULL) {
		RETAILMSG(1,(TEXT("For pGPIOregs: VirtualAlloc failed!\r\n")));
		bResult = FALSE;
	}
	else {
		if(!VirtualCopy((PVOID)pPublicSpi->pGPIOregs,(PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8),sizeof(S3C2450_IOPORT_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For pGPIOregs: VirtualCopy failed!\r\n")));
			bResult = FALSE;
		}
	}

	// HS-SPI Virtual alloc
	pPublicSpi->pHSSPIregs = (volatile S3C2450_HSSPI_REG *) VirtualAlloc(0,sizeof(S3C2450_HSSPI_REG), MEM_RESERVE, PAGE_NOACCESS);
	if(pPublicSpi->pHSSPIregs == NULL) {
		RETAILMSG(1,(TEXT("For pHS SPIregs: VirtualAlloc failed!\r\n")));
		bResult = FALSE;
	}
	else {
		if(!VirtualCopy((PVOID)pPublicSpi->pHSSPIregs,(PVOID)(S3C2450_BASE_REG_PA_HSSPI0 >> 8),sizeof(S3C2450_HSSPI_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For pSPIregs: VirtualCopy failed!\r\n")));
			bResult = FALSE;
		}
	}

	// Interrupt Virtual alloc
	pPublicSpi->pINTRregs = (volatile S3C2450_INTR_REG *) VirtualAlloc(0,sizeof(S3C2450_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if(pPublicSpi->pINTRregs == NULL) {
		RETAILMSG(1,(TEXT("For pINTRregs: VirtualAlloc failed!\r\n")));
		bResult = FALSE;
	}
	else {
		if(!VirtualCopy((PVOID)pPublicSpi->pINTRregs,(PVOID)(S3C2450_BASE_REG_PA_INTR >> 8),sizeof(S3C2450_INTR_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For pINTRregs: VirtualCopy failed!\r\n")));
			bResult = FALSE;
		}
	}

	// Clock Virtual alloc
	pPublicSpi->pCLKPWRregs = (volatile S3C2450_CLKPWR_REG *) VirtualAlloc(0,sizeof(S3C2450_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if(pPublicSpi->pCLKPWRregs == NULL) {
		RETAILMSG(1,(TEXT("For pSPIregs: VirtualAlloc failed!\r\n")));
		bResult = FALSE;
	}
	else {
		if(!VirtualCopy((PVOID)pPublicSpi->pCLKPWRregs,(PVOID)(S3C2450_BASE_REG_PA_CLOCK_POWER >> 8),sizeof(S3C2450_CLKPWR_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For pSPIregs: VirtualCopy failed!\r\n")));
			bResult = FALSE;
		}
	}
	// DMA Virtual alloc
	pPublicSpi->pDMAregs = (volatile S3C2450_DMA_REG *) VirtualAlloc(0,sizeof(S3C2450_DMA_REG), MEM_RESERVE, PAGE_NOACCESS);
	if(pPublicSpi->pDMAregs == NULL) {
		RETAILMSG(1,(TEXT("For pSPIregs: VirtualAlloc failed!\r\n")));
		bResult = FALSE;
	}
	else {
		if(!VirtualCopy((PVOID)pPublicSpi->pDMAregs,(PVOID)(S3C2450_BASE_REG_PA_DMA >> 8),sizeof(S3C2450_DMA_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For pSPIregs: VirtualCopy failed!\r\n")));
			bResult = FALSE;
		}
	}

	if (!bResult)
	{
		if(pPublicSpi->pGPIOregs) VirtualFree((PVOID) pPublicSpi->pGPIOregs, 0,   MEM_RELEASE);
		if(pPublicSpi->pHSSPIregs) VirtualFree((PVOID) pPublicSpi->pHSSPIregs, 0,    MEM_RELEASE);		// for HS-SPI
		if(pPublicSpi->pINTRregs) VirtualFree((PVOID) pPublicSpi->pINTRregs, 0,   MEM_RELEASE);
		if(pPublicSpi->pDMAregs) VirtualFree((PVOID) pPublicSpi->pDMAregs, 0,   MEM_RELEASE);

		pPublicSpi->pGPIOregs     = NULL;
		pPublicSpi->pHSSPIregs      = NULL;		// for HS-SPI
		pPublicSpi->pINTRregs     = NULL;
		pPublicSpi->pDMAregs      = NULL;
		
		return FALSE;
	}


	//Set GPIO for MISO, MOSI, SPICLK, SS
	pPublicSpi->pGPIOregs->MISCCR |= (1<<31);	// HS-SPI Select
	pPublicSpi->pGPIOregs->GPEUDP = pPublicSpi->pGPIOregs->GPEUDP & ~(0x3f<<22);
	pPublicSpi->pGPIOregs->GPLUDP = pPublicSpi->pGPIOregs->GPLUDP & ~(0x3ff<<20);

	pPublicSpi->pGPIOregs->GPECON = pPublicSpi->pGPIOregs->GPECON & ~(0x3f<<22) | (1<<27) | (1<<25) | (1<<23); // SPICLK0, SPIMOSI0, SPIMISO0
	pPublicSpi->pGPIOregs->GPLCON = pPublicSpi->pGPIOregs->GPLCON & ~(0x3ff<<20) | (1<<29) | (1<<27) | (1<<25) | (1<<23) | (1<<21); // SS1, SS0, SPIMISO1, SPIMOSI1, SPICLK1


	// Clock On
	pPublicSpi->pCLKPWRregs->SCLKCON |= (1<<14);		// For HS-SPI
	pPublicSpi->pCLKPWRregs->PCLKCON |= (1<<6);			// For HS-SPI

	// Set EPLL

#if 0
	pPublicSpi->pCLKPWRregs->LOCKCON1=0x800; 
	pPublicSpi->pCLKPWRregs->CLKSRC|= (1<<6);  // EPLL Output
	pPublicSpi->pCLKPWRregs->EPLLCON  = (40<<16) | (1<<8) | 1;	//96MHz
	pPublicSpi->pCLKPWRregs->EPLLCON &= ~(1<<24);  // EPLL ON 
	pPublicSpi->pCLKPWRregs->CLKDIV1 = (pPublicSpi->pCLKPWRregs->CLKDIV1 & ~(0x3<<24)) | (0x0<<24);	// SPI Clk Divider
	pPublicSpi->pGPIOregs->MISCCR = pPublicSpi->pGPIOregs->MISCCR & ~(0x7<<8) | (1<<8);	// CLKOUT1 pad :  EPLL Output
#endif




	
	return TRUE;

}



PSPI_PUBLIC_CONTEXT HSP_Init(PVOID Context) 
{
	LPTSTR          				ActivePath = (LPTSTR) Context; // HKLM\Drivers\Active\xx
	PSPI_PUBLIC_CONTEXT		pPublicSpi = NULL;
	BOOL            				bResult = TRUE;
	DWORD           			dwHwIntr=0;

	RETAILMSG(1,(TEXT("++HSP_Init Function\r\n")));
	RETAILMSG(1,(TEXT("Active Path : %s\n"), ActivePath));

	if ( !(pPublicSpi = (PSPI_PUBLIC_CONTEXT)LocalAlloc( LPTR, sizeof(SPI_PUBLIC_CONTEXT) )) )
	{
		RETAILMSG(1,(TEXT("Can't not allocate for SPI Context\n")));
		return NULL;
	}


    if(!HW_Init(pPublicSpi)) 
    {
    	RETAILMSG(1,(TEXT("HW_Init is failed\n")));
    	return NULL;
    } else {
    	RETAILMSG(1,(TEXT("HW_Init is completed\n")));
    }
    
	do 
	{
		InitializeCriticalSection(&(pPublicSpi->CsRxAccess));
		InitializeCriticalSection(&(pPublicSpi->CsTxAccess));

		//Rx Thread
		pPublicSpi->hRxEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		
		pPublicSpi->hRxThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadForRx, (LPVOID)pPublicSpi, 0, (LPDWORD)&pPublicSpi->dwRxThreadId);
		if (pPublicSpi->hRxThread == NULL)
		{
			RETAILMSG(1,(TEXT("SPI Rx Thread creation error!!!\n")));
			bResult = FALSE;
			break;
		}
		
		pPublicSpi->hRxDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);		
		pPublicSpi->hRxIntrDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		
		//Tx Thread
		pPublicSpi->hTxEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		
		pPublicSpi->hTxThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadForTx, (LPVOID)pPublicSpi, 0, (LPDWORD)&pPublicSpi->dwTxThreadId);
		if (pPublicSpi->hTxThread == NULL)
		{
			RETAILMSG(1,(TEXT("SPI Dma Thread creation error!!!\n")));
			bResult = FALSE;
			break;
		}
		
		pPublicSpi->hTxDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);		
		pPublicSpi->hTxIntrDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		
		//Spi ISR
		pPublicSpi->dwSpiSysIntr = SYSINTR_NOP;
		dwHwIntr = IRQ_SPI1;		//HS-SPI

		pPublicSpi->hSpiEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		
		if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwHwIntr, sizeof(DWORD), &pPublicSpi->dwSpiSysIntr, sizeof(DWORD), NULL))
		{
			RETAILMSG(1,(TEXT("Failed to request the SPI sysintr.\n")));
			pPublicSpi->dwSpiSysIntr = SYSINTR_UNDEFINED;
			bResult = FALSE;
			break;
		}

		if (!InterruptInitialize(pPublicSpi->dwSpiSysIntr, pPublicSpi->hSpiEvent, NULL, 0))
		{
			RETAILMSG(1,(TEXT("SPI Interrupt Initialization failed!!!\n")));
			bResult = FALSE;
			break;
		}

		pPublicSpi->hSpiThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadForSpi, (LPVOID)pPublicSpi, 0, (LPDWORD)&pPublicSpi->dwSpiThreadId);
		if (pPublicSpi->hSpiThread == NULL)
		{
			RETAILMSG(1,(TEXT("SPI ISR Thread creation error!!!\n")));
			bResult = FALSE;
			break;
		}
		
		
		//Rx DMA Done ISR
		pPublicSpi->dwRxDmaDoneSysIntr = SYSINTR_NOP;	
		dwHwIntr = IRQ_DMA3;	

		pPublicSpi->hRxDmaDoneDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		pPublicSpi->hRxDmaDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);


		if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwHwIntr, sizeof(DWORD), &pPublicSpi->dwRxDmaDoneSysIntr, sizeof(DWORD), NULL))
		{
			RETAILMSG(1,(TEXT("Failed to request the SPI_DMA sysintr.\n")));
			pPublicSpi->dwRxDmaDoneSysIntr = SYSINTR_UNDEFINED;
			bResult = FALSE;
			break;
		}

		if (!InterruptInitialize(pPublicSpi->dwRxDmaDoneSysIntr, pPublicSpi->hRxDmaDoneEvent, NULL, 0))
		{
			RETAILMSG(1,(TEXT("DMA Interrupt Initialization failed!!!\n")));
			bResult = FALSE;
			break;
		}

		pPublicSpi->hRxDmaDoneThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadForRxDmaDone, (LPVOID)pPublicSpi, 0, (LPDWORD)&pPublicSpi->dwRxDmaDoneThreadId);
		if (pPublicSpi->hRxDmaDoneThread == NULL)
		{
			RETAILMSG(1,(TEXT("SPI Dma Thread creation error!!!\n")));
			bResult = FALSE;
			break;
		}
		
		//Tx DMA Done ISR
		pPublicSpi->dwTxDmaDoneSysIntr = SYSINTR_NOP;
		dwHwIntr = IRQ_DMA4;		
		
		pPublicSpi->hTxDmaDoneDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		pPublicSpi->hTxDmaDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);


		if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwHwIntr, sizeof(DWORD), &pPublicSpi->dwTxDmaDoneSysIntr, sizeof(DWORD), NULL))
		{
			RETAILMSG(1,(TEXT("Failed to request the SPI_DMA sysintr.\n")));
			pPublicSpi->dwTxDmaDoneSysIntr = SYSINTR_UNDEFINED;
			bResult = FALSE;
			break;
		}

		if (!InterruptInitialize(pPublicSpi->dwTxDmaDoneSysIntr, pPublicSpi->hTxDmaDoneEvent, NULL, 0))
		{
			RETAILMSG(1,(TEXT("DMA Interrupt Initialization failed!!!\n")));
			bResult = FALSE;
			break;
		}

		pPublicSpi->hTxDmaDoneThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadForTxDmaDone, (LPVOID)pPublicSpi, 0, (LPDWORD)&pPublicSpi->dwTxDmaDoneThreadId);
		if (pPublicSpi->hTxDmaDoneThread == NULL)
		{
			RETAILMSG(1,(TEXT("SPI Dma Thread creation error!!!\n")));
			bResult = FALSE;
			break;
		}
	} while (0);
	
	
	if(bResult) return pPublicSpi;
	else 		return NULL;
}

DWORD
HSP_Open(
   DWORD 		pContext,   
   DWORD        AccessCode,
   DWORD        ShareMode)
{
	PSPI_PUBLIC_CONTEXT		pSpiPublic  = (PSPI_PUBLIC_CONTEXT) pContext;
	PSPI_PRIVATE_CONTEXT	pSpiPrivate = NULL;
	BOOL            		bResult 	= TRUE;
	

	if ( !(pSpiPrivate = (PSPI_PRIVATE_CONTEXT)LocalAlloc( LPTR, sizeof(SPI_PRIVATE_CONTEXT) )) )
	{
		RETAILMSG(1,(TEXT("Can't not allocate for SPI Context\n")));
		return (DWORD)NULL;
	}
    
	do 
	{
		pSpiPrivate->State				= STATE_INIT;
		
		pSpiPrivate->pSpiPublic			= pSpiPublic;
		

		pSpiPrivate->bUseRxDMA  		= FALSE;
		pSpiPrivate->bUseRxIntr		= FALSE;
		pSpiPrivate->bUseTxDMA		= FALSE;
		pSpiPrivate->bUseTxIntr		= FALSE;
		
	} while(FALSE);
	
	
	if(bResult) return (DWORD) pSpiPrivate;
	else		return (DWORD) NULL;
}


DWORD 
HSP_Read(
	DWORD 	hOpenContext, 
	LPVOID 	pBuffer, 
	DWORD 	Count) 
{
	PSPI_PRIVATE_CONTEXT pSpiPrivate = (PSPI_PRIVATE_CONTEXT)hOpenContext;
	PSPI_PUBLIC_CONTEXT pSpiPublic = pSpiPrivate->pSpiPublic;
//	DWORD dwReadSize;
	
	
	//param check
	if(pSpiPrivate->State != STATE_IDLE) {
		RETAILMSG(1,(TEXT("READ ERROR : STATE IS NOT IDLE\n")));
		return 0;
	}
	RETAILMSG(1,(TEXT("pBuffer : 0x%X, Count : %d\n"), pBuffer, Count));
	

	if(pSpiPrivate->bUseRxDMA)
	{
		PDMA_BUFFER pDmaBuffer = (PDMA_BUFFER) pBuffer;
		pSpiPrivate->pRxBuffer 		= pDmaBuffer->VirtualAddress;
		pSpiPrivate->pRxDMABuffer 	= pDmaBuffer->PhysicalAddress;
	}
	else 
	{
		pSpiPrivate->pRxBuffer = (LPVOID)pBuffer;
	} 
	
	pSpiPrivate->dwRxCount = Count;
	pSpiPublic->pSpiPrivate = pSpiPrivate;
	SetEvent(pSpiPublic->hRxEvent);
	WaitForSingleObject(pSpiPublic->hRxDoneEvent, INFINITE);	
	
	
	pSpiPrivate->State = STATE_IDLE;

	RETAILMSG(1,(TEXT("Return Value : %d\n"),pSpiPrivate->dwRxCount));
	
	
	return pSpiPrivate->dwRxCount ;
}


DWORD 
HSP_Write(
	DWORD 	hOpenContext, 
	LPVOID 	pBuffer, 
	DWORD 	Count) 
{
	PSPI_PRIVATE_CONTEXT pSpiPrivate = (PSPI_PRIVATE_CONTEXT)hOpenContext;
	PSPI_PUBLIC_CONTEXT pSpiPublic = pSpiPrivate->pSpiPublic;
	
	
	//param check
	if(pSpiPrivate->State != STATE_IDLE) {
		RETAILMSG(1,(TEXT("WRITE ERROR : STATE IS NOT IDLE\n")));
		return 0;
	}
	RETAILMSG(1,(TEXT("pBuffer : 0x%X, Count : %d\n"), pBuffer, Count));
	


	
	if(pSpiPrivate->bUseTxDMA)
	{
		PDMA_BUFFER pDmaBuffer = (PDMA_BUFFER) pBuffer;
		pSpiPrivate->pTxBuffer 		= pDmaBuffer->VirtualAddress;
		pSpiPrivate->pTxDMABuffer 	= pDmaBuffer->PhysicalAddress;
	}
	else 
	{
		pSpiPrivate->pTxBuffer = (LPVOID)pBuffer; 
	}
	
	pSpiPrivate->dwTxCount = Count;
	pSpiPublic->pSpiPrivate = pSpiPrivate;
	SetEvent(pSpiPublic->hTxEvent);
	WaitForSingleObject(pSpiPublic->hTxDoneEvent, INFINITE);	
	
	
	pSpiPrivate->State = STATE_IDLE;

	RETAILMSG(1,(TEXT("Return Value : %d\n"),pSpiPrivate->dwTxCount));
	
	
	return pSpiPrivate->dwTxCount;
}




BOOL 
HSP_IOControl(
	DWORD dwInst, 
	DWORD dwIoControlCode, 
	PBYTE lpInBuf, 
	DWORD nInBufSize, 
	PBYTE lpOutBuf, 
	DWORD nOutBufSize, 
	LPDWORD lpBytesRetruned) 
{
	PSPI_PRIVATE_CONTEXT 	pSpiPrivate 	= (PSPI_PRIVATE_CONTEXT)dwInst;
	PSPI_PUBLIC_CONTEXT		pSpiPublic 	= pSpiPrivate->pSpiPublic; 
	volatile S3C2450_HSSPI_REG 	*pSPIregs   	= pSpiPublic->pHSSPIregs;		// for HS-SPI
	volatile S3C2450_HSSPI_REG   	*pRxSPIregs = &pSpiPrivate->RxSPIregs;	// for HS-SPI
	volatile S3C2450_HSSPI_REG   	*pTxSPIregs = &pSpiPrivate->TxSPIregs;	// for HS-SPI

	PSET_CONFIG 			pSetConfig;
	BOOL					bResult = TRUE;
	
	
	switch(dwIoControlCode)
	{
		case SPI_IOCTL_SET_CONFIG:
			if( nInBufSize != sizeof(SET_CONFIG) ) 
			{
				bResult = FALSE;
				break;
			}
			pSetConfig = (PSET_CONFIG) lpInBuf;
			
//===========================================COMMON PART===========================================
			pSpiPrivate->dwTimeOutVal 		= pSetConfig->dwTimeOutVal;
			pSpiPrivate->dwMode 			= pSetConfig->dwMode;
			pSpiPrivate->dwPrescaler    		= pSetConfig->dwPrescaler;			

			pSpiPrivate->bUseRxDMA 		= pSetConfig->bUseRxDMA;
			pSpiPrivate->bUseRxIntr		= pSetConfig->bUseRxIntr;
			
			pSpiPrivate->bUseTxDMA 		= pSetConfig->bUseTxDMA;
			pSpiPrivate->bUseTxIntr		= pSetConfig->bUseTxIntr;


			pRxSPIregs->CH_CFG		= CPOL_RISING|CPHA_FORMAT_A;
			pRxSPIregs->CLK_CFG		= CLKSEL_PCLK|(pSpiPrivate->dwPrescaler);
			//pRxSPIregs->MODE_CFG	= (0x3ff<<19)|(0<<2);
			pRxSPIregs->MODE_CFG	= MODE_DEFAULT;

			pTxSPIregs->CH_CFG		= pRxSPIregs->CH_CFG;	
			pTxSPIregs->CLK_CFG		= pRxSPIregs->CLK_CFG;	
			pTxSPIregs->MODE_CFG	= pRxSPIregs->MODE_CFG;
			
			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) {
				pRxSPIregs->CH_CFG	|= SPI_MASTER;
				pRxSPIregs->CLK_CFG	|= ENCLK_ENABLE;
				
				pTxSPIregs->CH_CFG	|= SPI_MASTER;
				pTxSPIregs->CLK_CFG	|= ENCLK_ENABLE;
			} else if(pSpiPrivate->dwMode == SPI_SLAVE_MODE) {
				pRxSPIregs->CH_CFG	|= SPI_SLAVE;
				pRxSPIregs->CLK_CFG	|= ENCLK_DISABLE;
				
				pTxSPIregs->CH_CFG	|= SPI_SLAVE;
				pTxSPIregs->CLK_CFG	|= ENCLK_DISABLE;
			} else {
				RETAILMSG(1,(TEXT("it's not supported MODE\n")));
				pSpiPrivate->State = STATE_ERROR;
				break;
			}
			
//===========================================RX PART============================================


			if(pSpiPrivate->bUseRxIntr)
			//INTR
			{

			}
			else if(pSpiPrivate->bUseRxDMA)
			//DMA
			{

			}
			else 
			//Polling
			{

			}
										
//===========================================TX PART============================================			
			

			if(pSpiPrivate->bUseTxIntr) 
			// INTR
			{

			}
			else if(pSpiPrivate->bUseTxDMA)
			// DMA 
			{

			}
			else 
			//Polling
			{

			}

			break;
		case SPI_IOCTL_GET_CONFIG:
			break;
		case SPI_IOCTL_CLR_TXBUFF:
			break;
		case SPI_IOCTL_CLR_RXBUFF:
			break;
		case SPI_IOCTL_STOP:
			break;

		case SPI_IOCTL_START:
			if(pSpiPrivate->State == STATE_ERROR) {
				RETAILMSG(1,(TEXT("SPI_IOCTL_START ERROR\n")));
				bResult = FALSE;
				break;
			}
			pSpiPrivate->State = STATE_IDLE;
			RETAILMSG(1,(TEXT("SPI STATE : SPI_IOCTL_START\n")));
			break;
		default:
			break;
	}
	
	return bResult;
}


DWORD ThreadForTx(PSPI_PUBLIC_CONTEXT pSpiPublic)
{
	volatile S3C2450_HSSPI_REG 	*pSPIregs   	= pSpiPublic->pHSSPIregs;	// for HS-SPI
	volatile S3C2450_INTR_REG 	*pINTRregs 	= pSpiPublic->pINTRregs;
	volatile S3C2450_DMA_REG 	*pDMAregs   	= pSpiPublic->pDMAregs;
	PSPI_PRIVATE_CONTEXT 	pSpiPrivate;
	DWORD 	dwTxCount;
	PBYTE 	pTxBuffer;
	DWORD 	dwOldPerm;

	PBYTE 	pTestBuffer;
	DWORD 	dwTestCount;

	
	do
	{
		WaitForSingleObject(pSpiPublic->hTxEvent, INFINITE);
		
		
		pSpiPrivate 	= (PSPI_PRIVATE_CONTEXT) pSpiPublic->pSpiPrivate;
		dwTestCount 	= dwTxCount = pSpiPrivate->dwTxCount;
		dwOldPerm 	= SetProcPermissions((DWORD)-1);
		pTestBuffer 	= pTxBuffer = (PBYTE) MapPtrToProcess(pSpiPrivate->pTxBuffer, (HANDLE) GetCurrentProcessId());

		RETAILMSG(1,(TEXT("pTxBuffer : 0x%X, dwTxCount : %d \r\n"), pTxBuffer, dwTxCount));

		//Reset
		pSPIregs->CH_CFG |= SW_RST;
		RETAILMSG(1,(TEXT("\n HS SPI reset\n")));
		pSPIregs->CH_CFG &= ~SW_RST;	



		if(pSpiPrivate->bUseTxIntr)
		// INT  + TX
		{
			RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : USE INT \r\n")));
			pSpiPrivate->State = STATE_TXINTR;
/*
			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) {
				pSPIregs->CH_CFG 	= 0x0;
				pSPIregs->CLK_CFG  	= pSpiPrivate->TxSPIregs.CLK_CFG;	
				pSPIregs->MODE_CFG	= (TX_TRIG_LEVEL<<5);
			} else {
				pSPIregs->CH_CFG 	= (0x1<<4);
				pSPIregs->CLK_CFG  	= pSpiPrivate->TxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG	= (TX_TRIG_LEVEL<<5);
			}	


			pSPIregs->SP_INT_EN			=	(1<<0);
			pSPIregs->PENDING_CLR_REG	=	(0x1f);
			pSPIregs->CH_CFG			= 	(1<<0);

			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) {
				RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : MASTER MODE \r\n")));
				pSPIregs->SLAVE_SELECTION_REG = 0;
			} 
			else{ 
				RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : SLAVE MODE \r\n")));
			}

			WaitForSingleObject(pSpiPublic->hTxIntrDoneEvent, INFINITE);

			while(((pSPIregs ->SPI_STATUS>>6) & 0x7f));
			while(!((pSPIregs ->SPI_STATUS>>21) & 0x1));
*/
		}
		else if(pSpiPrivate->bUseTxDMA)
		// DMA + TX
		{
			DWORD dwDmaLen			= dwTxCount & 0xFFFFF ;

			RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : USE DMA (TxCount : %d) \r\n"),dwDmaLen));
			
			pSpiPrivate->State = STATE_TXDMA;
			VirtualCopy((PVOID)pSpiPrivate->pTxBuffer, (PVOID)((ULONG) pSpiPrivate->pTxDMABuffer>>8), sizeof(dwTxCount), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL);

			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) 
			{
				pSPIregs->CH_CFG 	= pSpiPrivate->TxSPIregs.CH_CFG;
				pSPIregs->CLK_CFG  	= pSpiPrivate->TxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG  = pSpiPrivate->TxSPIregs.MODE_CFG;
			}else {
				pSPIregs->CH_CFG 	= pSpiPrivate->TxSPIregs.CH_CFG;
				pSPIregs->CLK_CFG  	= pSpiPrivate->TxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG  = pSpiPrivate->TxSPIregs.MODE_CFG;
			}	

		
			if(dwDmaLen > 0)
			{

				pSPIregs->MODE_CFG		|=	TX_DMA_ON|DMA_SINGLE;
				pSPIregs->CH_CFG 		|=	TX_CH_ON;
				
				pDMAregs->DISRC4      	= (UINT)pSpiPrivate->pTxDMABuffer;
				pDMAregs->DISRCC4     	= ~(DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS); 
				pDMAregs->DIDST4      	= (UINT)SPI_TX_DATA_PHY_ADDR;
				pDMAregs->DIDSTC4     	= (SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS); 
//				pDMAregs->DCON4  		= HANDSHAKE_MODE |GENERATE_INTERRUPT |PADDRFIX |NO_DMA_AUTO_RELOAD | dwDmaLen;
				pDMAregs->DCON4  		= HANDSHAKE_MODE |GENERATE_INTERRUPT |NO_DMA_AUTO_RELOAD | dwDmaLen;
				pDMAregs->DMAREQSEL4 	= ( DMAREQSEL_SPI_0TX | DMA_TRIGGERED_BY_HARDWARE );


				if(pSpiPrivate->dwMode == SPI_MASTER_MODE) 
				{
					RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : MASTER MODE \r\n")));
					MASTER_CS_ENABLE;
				} 
				else
				{ 
					RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : SLAVE MODE \r\n")));
				}

				pDMAregs->DMASKTRIG4 	= ENABLE_DMA_CHANNEL; 	
				
				WaitForSingleObject(pSpiPublic->hTxDmaDoneDoneEvent, INFINITE);
				

				pSpiPrivate->dwTxCount -= dwDmaLen;
				pSpiPrivate->pTxBuffer = (((PUINT) pSpiPrivate->pTxBuffer) + dwDmaLen);
			}
			VirtualFree((PVOID)pTxBuffer, 0, MEM_RELEASE);

			while(((pSPIregs ->SPI_STATUS>>6) & 0x7f));
			while(!(pSPIregs ->SPI_STATUS & TX_DONE));

		}
		else		
		// POLLING + TX
		{
			RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : USE Polling (TxCount : %d) \r\n"), dwTxCount));

			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) {
				pSPIregs->CH_CFG 	= pSpiPrivate->TxSPIregs.CH_CFG;
				pSPIregs->CLK_CFG 	= pSpiPrivate->TxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG  = pSpiPrivate->TxSPIregs.MODE_CFG;
			} else{
				pSPIregs->CH_CFG 	= pSpiPrivate->TxSPIregs.CH_CFG;
				pSPIregs->CLK_CFG 	= pSpiPrivate->TxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG  = pSpiPrivate->TxSPIregs.MODE_CFG;
			}
			pSPIregs->CH_CFG 		|=	TX_CH_ON;

			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) 
			{
				RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : MASTER MODE \r\n")));
				MASTER_CS_ENABLE;
			}
			else
			{
				RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : SLAVE MODE \r\n")));
			}
			
			do
			{
				while(((pSPIregs ->SPI_STATUS>>6) & 0x7f)==FIFO_FULL);
				pSPIregs->SPI_TX_DATA = *(PBYTE)pSpiPrivate->pTxBuffer;
			} while(--pSpiPrivate->dwTxCount > 0 && ++(PBYTE)pSpiPrivate->pTxBuffer);

			while(((pSPIregs ->SPI_STATUS>>6) & 0x7f));
			while(!(pSPIregs ->SPI_STATUS & TX_DONE));

		}


		pSpiPrivate->dwTxCount = dwTestCount - pSpiPrivate->dwTxCount;
		
#ifdef TEST_MODE
		do
		{
			RETAILMSG(1,(TEXT("WRITE BYTE : %02X(dwTxCount : %d)\n"), *pTestBuffer, dwTestCount));
		} while( (--dwTestCount > 0) && ++pTestBuffer);
#endif

		RETAILMSG(FALSE,(TEXT("[HSPI DD] TX_CH_OFF \n")));
		pSPIregs->CH_CFG 	&= ~TX_CH_ON;	

		if(pSpiPrivate->dwMode == SPI_MASTER_MODE)	
			MASTER_CS_DISABLE;
		
		UnMapPtr(pTxBuffer);
		SetProcPermissions(dwOldPerm);
		
		SetEvent(pSpiPublic->hTxDoneEvent);
		
	} while(TRUE);
	
	return 0;
}



DWORD ThreadForRx(PSPI_PUBLIC_CONTEXT pSpiPublic)
{
	volatile S3C2450_HSSPI_REG *pSPIregs   = pSpiPublic->pHSSPIregs;	// for HS-SPI
	volatile S3C2450_INTR_REG *pINTRregs = pSpiPublic->pINTRregs;
	volatile S3C2450_DMA_REG *pDMAregs   = pSpiPublic->pDMAregs;
	PSPI_PRIVATE_CONTEXT pSpiPrivate;
	DWORD 	dwRxCount;
	PBYTE 	pRxBuffer;
	DWORD 	dwOldPerm;
	
	PBYTE 	pTestBuffer;
	DWORD 	dwTestCount;
	
	do
	{
		WaitForSingleObject(pSpiPublic->hRxEvent, INFINITE);
		
		
		pSpiPrivate = (PSPI_PRIVATE_CONTEXT) pSpiPublic->pSpiPrivate;
		dwTestCount = dwRxCount = pSpiPrivate->dwRxCount;
		dwOldPerm = SetProcPermissions((DWORD)-1);
		pTestBuffer = pRxBuffer = (PBYTE) MapPtrToProcess(pSpiPrivate->pRxBuffer, (HANDLE) GetCurrentProcessId());

		RETAILMSG(1,(TEXT("pRxBuffer : 0x%X, dwRxCount : %d \r\n"), pRxBuffer, dwRxCount));

		//Reset
		pSPIregs->CH_CFG |= SW_RST;
		RETAILMSG(1,(TEXT("\n HS SPI reset\n")));
		pSPIregs->CH_CFG &= ~SW_RST;
		
	
		if(pSpiPrivate->dwMode == SPI_MASTER_MODE) 
		{
			RETAILMSG(1,(TEXT("[HSPI DD] Thread for RX : MASTER MODE \r\n")));
			MASTER_CS_ENABLE;
		}
		else
			RETAILMSG(1,(TEXT("[HSPI DD] Thread for RX : SLAVE MODE \r\n")));


		if(pSpiPrivate->bUseRxIntr)
		//INT Mode + RX	
		{
			RETAILMSG(1,(TEXT("[HSPI DD] Thread for RX : USE INT \r\n")));
			pSpiPrivate->State = STATE_RXINTR;
/*			
			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) 
			{
				pSPIregs->CH_CFG 	= 0x0;
				pSPIregs->CLK_CFG  	= pSpiPrivate->RxSPIregs.CLK_CFG;	
				//pSPIregs->MODE_CFG	=	(0x8<<11) |(0x8<<5);
				//pSPIregs->MODE_CFG	=	(0x3ff<<19)|(0x8<<11) |(0x8<<5);
				pSPIregs->MODE_CFG	=	(0x3ff<<19)|(RX_TRIG_LEVEL<<11);
				pSPIregs->SP_INT_EN	=	(1<<6);
			}
			else 
			{
				pSPIregs->CH_CFG 	= (0x1<<4);
				pSPIregs->CLK_CFG  	= pSpiPrivate->RxSPIregs.CLK_CFG;			
				//pSPIregs->MODE_CFG	= (0x8<<11) |(0x14<<5);
				pSPIregs->MODE_CFG	= (RX_TRIG_LEVEL<<11);
			}
			// INT
			pSPIregs->PENDING_CLR_REG	|=	0xffffffff;
			pSPIregs->SP_INT_EN			|=	(1<<1);
			pSPIregs->CH_CFG			|= 	(1<<1);	
			RETAILMSG(1,(TEXT("CH_CFG : 0x%08X, pSPIregs->MODE_CFG 0x%08X, SPI_STATUS : 0x%08X\n"),pSPIregs->CH_CFG, pSPIregs->MODE_CFG, pSPIregs->SPI_STATUS));	

			WaitForSingleObject(pSpiPublic->hRxIntrDoneEvent, INFINITE);
*/
		}
		
		else if(pSpiPrivate->bUseRxDMA)
		//DMA Mode + Rx
		{
			DWORD dwDmaLen	= (pSpiPrivate->dwRxCount & 0xFFFFF);

			RETAILMSG(1,(TEXT("[HSPI DD] Thread for RX : USE DMA \r\n")));

			pSpiPrivate->State = STATE_RXDMA;
			VirtualCopy((PVOID)pSpiPrivate->pRxBuffer, (PVOID)((ULONG) pSpiPrivate->pRxDMABuffer>>8), sizeof(dwRxCount), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL);

			//Reset
			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) {
				pSPIregs->CH_CFG 	= pSpiPrivate->RxSPIregs.CH_CFG;
				pSPIregs->CLK_CFG  	= pSpiPrivate->RxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG  = pSpiPrivate->RxSPIregs.MODE_CFG;
				pSPIregs->PACKET_COUNT_REG = PACKET_CNT_EN | (pSpiPrivate->dwRxCount) ;
			} else {
				pSPIregs->CH_CFG 	= pSpiPrivate->RxSPIregs.CH_CFG;
				pSPIregs->CLK_CFG  	= pSpiPrivate->RxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG  = pSpiPrivate->RxSPIregs.MODE_CFG;
			}	
			
			if(dwDmaLen > 0)
			{
				pSPIregs->MODE_CFG		|=	RX_DMA_ON|DMA_SINGLE;
				pSPIregs->CH_CFG 		|=	RX_CH_ON;
				
				pDMAregs->DISRC3      	= (UINT)SPI_RX_DATA_PHY_ADDR;
				pDMAregs->DISRCC3     	= (SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS); 
				pDMAregs->DIDST3      	= (UINT)pSpiPrivate->pRxDMABuffer;
				pDMAregs->DIDSTC3     	= ~(DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS); 
//				pDMAregs->DCON3  		= HANDSHAKE_MODE |GENERATE_INTERRUPT |PADDRFIX |NO_DMA_AUTO_RELOAD | dwDmaLen;
				pDMAregs->DCON3  		= HANDSHAKE_MODE |GENERATE_INTERRUPT |NO_DMA_AUTO_RELOAD | dwDmaLen;
				pDMAregs->DMAREQSEL3 	= ( DMAREQSEL_SPI_0RX | DMA_TRIGGERED_BY_HARDWARE );	
				pDMAregs->DMASKTRIG3 	= ENABLE_DMA_CHANNEL;

				WaitForSingleObject(pSpiPublic->hRxDmaDoneDoneEvent, INFINITE);

				pSpiPrivate->dwRxCount -= dwDmaLen;
				pSpiPrivate->pRxBuffer = (PBYTE) (((PUINT) pSpiPrivate->pRxBuffer) + dwDmaLen);
			}
			VirtualFree((PVOID)pRxBuffer, 0, MEM_RELEASE);
		}
		else
		{
		//POLLING Mode + RX
			RETAILMSG(1,(TEXT("[HSPI DD] Thread for TX : USE Polling (TxCount : %d) \r\n"), dwRxCount));

			if(pSpiPrivate->dwMode == SPI_MASTER_MODE) {
				pSPIregs->CH_CFG 	= pSpiPrivate->RxSPIregs.CH_CFG;
				pSPIregs->CLK_CFG 	= pSpiPrivate->RxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG  = pSpiPrivate->RxSPIregs.MODE_CFG;
				pSPIregs->PACKET_COUNT_REG = PACKET_CNT_EN | (pSpiPrivate->dwRxCount) ;
			} else{ 
				pSPIregs->CH_CFG 	= pSpiPrivate->RxSPIregs.CH_CFG;
				pSPIregs->CLK_CFG 	= pSpiPrivate->RxSPIregs.CLK_CFG;
				pSPIregs->MODE_CFG  = pSpiPrivate->RxSPIregs.MODE_CFG;
			}

			pSPIregs->CH_CFG 	|= RX_CH_ON;	

			do
			{
				while (((pSPIregs ->SPI_STATUS>>13)&0x7f)==FIFO_EMPTY);
				 *(PBYTE)pSpiPrivate->pRxBuffer = pSPIregs->SPI_RX_DATA;
			} while(--pSpiPrivate->dwRxCount > 0 && ++(PBYTE)pSpiPrivate->pRxBuffer);

			//RETAILMSG(1,(TEXT("[Test]  STATUS  0x%X  \r\n"),pSPIregs ->SPI_STATUS));
		}
		
		pSpiPrivate->dwRxCount = dwTestCount - pSpiPrivate->dwRxCount;

#ifdef TEST_MODE
		do
		{
			RETAILMSG(1,(TEXT("READ BYTE : %02X(dwRxCount %d)\n"), *pTestBuffer, dwTestCount));
		} while((--dwTestCount > 0) && ++pTestBuffer);
#endif

		RETAILMSG(FALSE,(TEXT("[HSPI DD] RX_CH_OFF \n")));
		pSPIregs->CH_CFG 	&= ~RX_CH_ON;	

		if(pSpiPrivate->dwMode == SPI_MASTER_MODE)
			MASTER_CS_DISABLE;
		
		UnMapPtr(pRxBuffer);
		SetProcPermissions(dwOldPerm);
		
		SetEvent(pSpiPublic->hRxDoneEvent);
		
	} while(TRUE);
	
	return 0;
}


DWORD ThreadForSpi(PSPI_PUBLIC_CONTEXT pSpiPublic)
{
	volatile S3C2450_HSSPI_REG *pSPIregs  = pSpiPublic->pHSSPIregs;		// for HS-SPI
	PSPI_PRIVATE_CONTEXT pSpiPrivate;
//	DWORD 	dwRxCount;
//	DWORD 	dwTxCount;
	DWORD	dwOldPerm;

	RETAILMSG(1,(TEXT("ThreadForSpi thread is created \r\n")));
	do
	{
		WaitForSingleObject(pSpiPublic->hSpiEvent, INFINITE);

		pSpiPrivate = (PSPI_PRIVATE_CONTEXT) pSpiPublic->pSpiPrivate;		
		dwOldPerm = SetProcPermissions((DWORD)-1);

/*
		if((pSPIregs ->SPI_STATUS>>20) & 0x1) {
			pSPIregs->PENDING_CLR_REG |= (1<<0);
			RETAILMSG(1,(TEXT("Time out event is occurred \r\n")));
			goto END_POINT;
		}


		if(pSpiPrivate->State == STATE_RXINTR)
		{ 

		}

		else if(pSpiPrivate->State == STATE_TXINTR)
		{


			RETAILMSG(1,(TEXT("STATE_TXINTR  %d \r\n"),pSpiPrivate->dwTxCount ));
			
			

#if 1
			dwTxCount = FIFO_SIZE -((pSPIregs ->SPI_STATUS>>6) & 0x7f) ;
			//RETAILMSG(1,(TEXT("dwTxCount = FIFO_SIZE -((pSPIregs ->SPI_STATUS>>6) & 0x7f) ;  %d \r\n"),dwTxCount));
			do
			{
				pSPIregs->SPI_TX_DATA = *(PBYTE)pSpiPrivate->pTxBuffer;
//			} while(--pSpiPrivate->dwTxCount > 0 && ++(PBYTE)pSpiPrivate->pTxBuffer && --dwTxCount > 0);
			} while(--pSpiPrivate->dwTxCount > FIFO_SIZE && ++(PBYTE)pSpiPrivate->pTxBuffer && --dwTxCount > 0);

			if(pSpiPrivate->dwTxCount ==0)
				SetEvent(pSpiPublic->hTxIntrDoneEvent);

#else
			dwTxCount = FIFO_SIZE - TX_TRIG_LEVEL;
			do
			{
				pSPIregs->SPI_TX_DATA = *(PBYTE)pSpiPrivate->pTxBuffer;
			} while(--pSpiPrivate->dwTxCount > 0 && ++(PBYTE)pSpiPrivate->pTxBuffer && --dwTxCount > 0);

			if(pSpiPrivate->dwTxCount ==0)
				SetEvent(pSpiPublic->hTxIntrDoneEvent);
#endif
		}

		else 
		{
			RETAILMSG(1,(TEXT("UNSOLVED OPERATION\n")));
		}

		
END_POINT:
*/		
		SetProcPermissions(dwOldPerm);
		InterruptDone(pSpiPublic->dwSpiSysIntr);
	} while(TRUE);
	return 0;
}


DWORD ThreadForRxDmaDone(PSPI_PUBLIC_CONTEXT pSpiPublic)
{
//	volatile S3C2450_SPI_REG 	*pSPIregs   	= pSpiPublic->pSPIregs;
//	PSPI_PRIVATE_CONTEXT 	pSpiPrivate;
//	DWORD					dwOldPerm;
//	volatile S3C2450_DMA_REG *pDMAregs   = pSpiPublic->pDMAregs;	
	do
	{
		WaitForSingleObject(pSpiPublic->hRxDmaDoneEvent, INFINITE);
		
		SetEvent(pSpiPublic->hRxDmaDoneDoneEvent);
		InterruptDone(pSpiPublic->dwRxDmaDoneSysIntr);
	} while(TRUE);
	return 0;
}

DWORD ThreadForTxDmaDone(PSPI_PUBLIC_CONTEXT pSpiPublic)
{
//	volatile S3C2450_SPI_REG 	*pSPIregs   	= pSpiPublic->pSPIregs;
//	PSPI_PRIVATE_CONTEXT 	pSpiPrivate;
//	DWORD					dwOldPerm;
//	volatile S3C2450_DMA_REG *pDMAregs   = pSpiPublic->pDMAregs;
	do
	{
		WaitForSingleObject(pSpiPublic->hTxDmaDoneEvent, INFINITE);
		
		SetEvent(pSpiPublic->hTxDmaDoneDoneEvent);
		InterruptDone(pSpiPublic->dwTxDmaDoneSysIntr);
	} while(TRUE);
	return 0;
}

void HSP_PowerDown (DWORD dwContext) 
{
	return;
}

void HSP_Deinit (DWORD dwContext) 
{
	return;
}

void HSP_PowerUp (DWORD dwContext) 
{
	return;
}

BOOL HSP_Close (DWORD dwOpen) 
{
	return TRUE;
}

DWORD HSP_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
	return 0;
}



//-------------------------------------------------------------------------------------------------------------------------
// Copyright (c) Samsung Electronics Co., Ltd.  All rights reserved.
//-------------------------------------------------------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, 
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
// Module Name :	HSPI.H
//
// Abstract    :	High SPI Interface Routines for Samsung SC2450 CPU
//
// Environment :	Samsung SC2450
//
// 2007/03/05
//
//-------------------------------------------------------------------------------------------------------------------------

#ifndef _HSPI_H_
#define _HSPI_H_

#include <windows.h>
#include <s3c2450.h>

//#define TEST_MODE

#define SPI_TX_DATA_PHY_ADDR	0x52000018
#define SPI_RX_DATA_PHY_ADDR	0x5200001C

#define FIFO_SIZE 			0x40
#define FIFO_HALF_SIZE		0x20

#define FIFO_FULL			0x40
#define FIFO_EMPTY			0x0

#define	RX_TRIG_LEVEL		0x8
#define	TX_TRIG_LEVEL		0x14


#define HIGH_SLAVE_DISABLE  (0<<6)
#define HIGH_SLAVE_ENABLE   (1<<6)

#define	SW_RST				(1<<5)
#define	SPI_MASTER			(0<<4)
#define	SPI_SLAVE			(1<<4)
#define	CPOL_RISING		(0<<3)
#define	CPOL_FALLING		(1<<3)
#define	CPHA_FORMAT_A		(0<<2)
#define	CPHA_FORMAT_B		(1<<2)
#define	RX_CH_OFF			(0<<1)
#define	RX_CH_ON			(1<<1)
#define	TX_CH_OFF			(0<<0)
#define	TX_CH_ON			(1<<0)

#define	CLKSEL_PCLK		(0<<9)

#define	CLKSEL_USBCLK		(1<<9)
#define	CLKSEL_EPLL			(2<<9)

#define	ENCLK_DISABLE		(0<<8)
#define	ENCLK_ENABLE		(1<<8)

#define CH_SIZE_BYTE        (0<<29)
#define CH_SIZE_HALFWORD    (1<<29)
#define CH_SIZE_WORD        (2<<29)
#define	BUS_SIZE_BYTE		(0<<17)
#define	BUS_SIZE_HALFWORD   (1<<17)
#define BUS_SIZE_WORD       (2<<17)

#define	DMA_SINGLE        	(0<<0)
#define	DMA_4BURST        	(1<<0)
#define	RX_DMA_ON			(1<<2)
#define	TX_DMA_ON			(1<<1)
#define	MODE_DEFAULT		(0)


#define	INT_TRAILING		(1<<6)
#define	INT_RX_OVERRUN	(1<<5)
#define	INT_RX_UNDERRUN	(1<<4)
#define	INT_TX_OVERRUN	(1<<3)
#define	INT_TX_UNDERRUN	(1<<2)
#define	INT_RX_FIFORDY		(1<<1)
#define	INT_TX_FIFORDY		(1<<0)

#define	TX_DONE			(1<<21)
#define	TRAILCNT_ZERO		(1<<20)
#define	RX_OVERRUN			(1<<5)
#define	RX_UNDERRUN		(1<<4)
#define	TX_OVERRUN			(1<<3)
#define	TX_UNDERRUN		(1<<2)
#define	RX_FIFORDY			(1<<1)
#define	TX_FIFORDY			(1<<0)

#define PACKET_CNT_EN		(1<<16)

// SWAP_CFG - Swap Configuration Register 
#define RX_HALFWORDSWAP_DIS (0<<7)
#define RX_HALFWORDSWAP_EN  (1<<7)
#define RX_BYTESWAP_DIS     (0<<6)
#define RX_BYTESWAP_EN      (1<<6)
#define RX_BITSWAP_DIS      (0<<5)
#define RX_BITSWAP_EN       (1<<5)
#define RX_SWAP_OFF         (0<<4)
#define RX_SWAP_ON          (1<<4)
#define TX_HALFWORDSWAP_DIS (0<<3)
#define TX_HALFWORDSWAP_EN  (1<<3)
#define TX_BYTESWAP_DIS     (0<<2)
#define TX_BYTESWAP_EN      (1<<2)
#define TX_BITSWAP_DIS      (0<<1)
#define TX_BITSWAP_EN       (1<<1)
#define TX_SWAP_OFF         (0<<0)
#define TX_SWAP_ON          (1<<0)

// FB_CLK_SEL - Feedback clock selecting register
#define FB_CLK_0NS_DELAY    (0<<0)
#define FB_CLK_3NS_DELAY    (1<<0)
#define FB_CLK_6NS_DELAY    (2<<0)
#define FB_CLK_9NS_DELAY    (3<<0)

#define	PADDRFIX			(1<<24)

typedef enum {			
	STATE_TIMEOUT,
	STATE_READING,
	STATE_RXDMA,
	STATE_RXINTR,
	STATE_WRITING,
	STATE_TXDMA,
	STATE_TXINTR,
	STATE_CONTROLLING,
	STATE_RXBUFFERRING,
	STATE_TXBUFFERRING,
	STATE_IDLE,
	STATE_CANCELLING,
	STATE_INIT,
	STATE_ERROR
} SPI_STATUS;

typedef struct {
	PBYTE pStrMem;
	PBYTE pEndMem;
	PBYTE pCurMem;
	DWORD dwMemSize;
	DWORD dwDataSize;
	DWORD dwUsedSize;
	DWORD dwUnusedSize;
	BOOL  bNeedBuffering;
} SPI_BUFFER;


typedef struct {
	PVOID						pSpiPrivate;
	
	volatile S3C2450_IOPORT_REG 	*pGPIOregs;
	volatile S3C2450_HSSPI_REG   	*pHSSPIregs;	//	For HS-SPI
	volatile S3C2450_INTR_REG  	*pINTRregs;
	volatile S3C2450_CLKPWR_REG 	*pCLKPWRregs;
	volatile S3C2450_DMA_REG   	*pDMAregs;
	
	
	DWORD						dwRxThreadId;
	DWORD						dwRxThreadPrio;
	HANDLE						hRxEvent;
	HANDLE						hRxThread;
	HANDLE						hRxDoneEvent;
	HANDLE						hRxIntrDoneEvent;
	
	DWORD						dwTxThreadId;
	DWORD						dwTxThreadPrio;
	HANDLE						hTxEvent;
	HANDLE						hTxThread;
	HANDLE						hTxDoneEvent;
	HANDLE						hTxIntrDoneEvent;
	
	DWORD						dwSpiThreadId;
	DWORD						dwSpiThreadPrio;
	DWORD						dwSpiSysIntr;
	HANDLE						hSpiEvent;
	HANDLE 						hSpiThread;
	
	DWORD						dwRxDmaDoneThreadId;
	DWORD						dwRxDmaDoneThreadPrio;
	DWORD 						dwRxDmaDoneSysIntr;
	HANDLE 	       				hRxDmaDoneEvent;
	HANDLE 	       				hRxDmaDoneDoneEvent;
	HANDLE                  			hRxDmaDoneThread;
	
	DWORD						dwTxDmaDoneThreadId;
	DWORD 			            		dwTxDmaDoneThreadPrio;
	DWORD 						dwTxDmaDoneSysIntr;
	HANDLE 	       				hTxDmaDoneEvent;
	HANDLE 	       				hTxDmaDoneDoneEvent;
	HANDLE                  			hTxDmaDoneThread;
	
	CRITICAL_SECTION  			CsTxAccess;
	CRITICAL_SECTION  			CsRxAccess;
	
//	DWORD                       			dwSizeOfUsedBuffer;
} SPI_PUBLIC_CONTEXT, *PSPI_PUBLIC_CONTEXT;

typedef struct {
	PSPI_PUBLIC_CONTEXT			pSpiPublic;
	
	DWORD						dwMode;

	BOOL                      			bUseRxDMA;
	BOOL						bUseRxIntr;
	SPI_BUFFER             			RxBuffer;
	LPVOID						pRxBuffer;
	LPVOID						pRxDMABuffer;	
	DWORD						dwRxCount;
	S3C2450_HSSPI_REG   	        	RxSPIregs;	// for HS-SPI		
	
	BOOL                      			bUseTxDMA;
	BOOL						bUseTxIntr;
	SPI_BUFFER            			TxBuffer;
	LPVOID						pTxBuffer;
	LPVOID						pTxDMABuffer;
	DWORD						dwTxCount;
	S3C2450_HSSPI_REG   	        	TxSPIregs;	// for HS-SPI
	
	DWORD						dwTimeOutVal;
	DWORD						dwPrescaler;
	DWORD						dwError;
	
	SPI_STATUS					State;
} SPI_PRIVATE_CONTEXT, *PSPI_PRIVATE_CONTEXT;





#endif


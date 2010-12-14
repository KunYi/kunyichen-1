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
//  Header: s3c2450_hsspi.h
//
//  Defines the High Speed Serial Peripheral Interface (HS-SPI) controller CPU register layout and
//  definitions.
//
#ifndef __S3C2450_HSSPI_H
#define __S3C2450_HSSPI_H

#include <winioctl.h>
#include <ceddk.h>

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2450_HSSPI_REG    
//
//  Defines the HS-SPI register layout. This register bank is located by the 
//  constant CPU_REG_BASE_XX_HSSPI in the configuration file cpu_reg_base_cfg.h.
//

typedef struct  
{
    UINT32      CH_CFG;                         //0x00
    UINT32      CLK_CFG;                        //0x04
    UINT32      MODE_CFG;                       //0x08
    UINT32      SLAVE_SELECTION_REG;            //0x0C
    UINT32      SP_INT_EN;                      //0x10
    UINT32      SPI_STATUS;                     //0x14
    UINT32      SPI_TX_DATA;                    //0x18
    UINT32      SPI_RX_DATA;                    //0x1C
    UINT32      PACKET_COUNT_REG;               //0x20
    UINT32      PENDING_CLR_REG;                //0x24
	UINT32      SWAP_CFG;                       //0x28
    UINT32      FB_CLK_SEL;                     //0x2C

} S3C2450_HSSPI_REG, *PS3C2450_HSSPI_REG; 



// IOCTL Commands
#define SPI_IOCTL_SET_CONFIG			CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SPI_IOCTL_GET_CONFIG			CTL_CODE(FILE_DEVICE_SERIAL_PORT, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SPI_IOCTL_CLR_TXBUFF			CTL_CODE(FILE_DEVICE_SERIAL_PORT, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SPI_IOCTL_CLR_RXBUFF			CTL_CODE(FILE_DEVICE_SERIAL_PORT, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SPI_IOCTL_STOP       			CTL_CODE(FILE_DEVICE_SERIAL_PORT, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SPI_IOCTL_START       			CTL_CODE(FILE_DEVICE_SERIAL_PORT, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define SPI_MASTER_MODE 		1
#define SPI_SLAVE_MODE			0
#if 0
#define SPI_MASTER_MODE 		0
#define SPI_SLAVE_MODE			1
#endif

typedef struct {
	PVOID            	VirtualAddress;
	PVOID			PhysicalAddress;
} DMA_BUFFER, *PDMA_BUFFER;

typedef struct {
	DWORD					dwMode;

	BOOL					bUseFullDuflex;
	
	DWORD					dwRxBurstDataLen;
//	DWORD					dwRxFIFORB;
//	BOOL                      		bUseRxFIFO;
	BOOL                      		bUseRxDMA;
	BOOL					bUseRxIntr;
//	BOOL                      		bIsRxBuffering;
	
	DWORD					dwTxBurstDataLen;
//	DWORD					dwTxFIFORB;
//	BOOL	                    		bUseTxFIFO;
	BOOL                      		bUseTxDMA;
	BOOL					bUseTxIntr;
//	BOOL                      		bIsTxBuffering;
	
	DWORD					dwPrescaler;
	DWORD					dwTimeOutVal;
} SET_CONFIG, *PSET_CONFIG;



#if __cplusplus
    }
#endif

#endif 

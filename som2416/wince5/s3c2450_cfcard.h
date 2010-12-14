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
//  Header: s3c2450_cfcard.h
//
//  Defines the CF card controller CPU register layout and definitions.
//
#ifndef __S3C2450_cfcard_H
#define __S3C2450_cfcard_H

#if __cplusplus
    extern "C" 
    {
#endif

//------------------------------------------------------------------------------
//  Type: S3C2450_CFCARD_REG    
//
//  SSMC control registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_CFCARD in the configuration file cpu_base_reg_cfg.h.
//

typedef struct {
	UINT8  PAD1[0x1800];
	UINT32 MUX_REG;
	UINT8  PAD2[0x100-0x4];
    UINT32 ATA_CONTROL;						// 0x4b801900
    UINT32 ATA_STATUS;						// 0x4b801904
    UINT32 ATA_COMMAND;                 	// 0x4b801908
    UINT32 ATA_SWRST;                 		// 0x4b80190c
    UINT32 ATA_IRQ;			        		// 0x4b801910
    UINT32 ATA_IRQ_MASK;					// 0x4b801914
    UINT32 ATA_CFG;							// 0x4b801918
    UINT32 ATA_RESERVED1;					// 0x4b80191c
    UINT32 ATA_RESERVED2;					// 0x4b801920
    UINT32 ATA_RESERVED3;					// 0x4b801924
    UINT32 ATA_RESERVED4;					// 0x4b801928
    UINT32 ATA_PIO_TIME;					// 0x4b80192c
    UINT32 ATA_UDMA_TIME;					// 0x4b801930
    UINT32 ATA_XFR_NUM;						// 0x4b801934
    UINT32 ATA_XFR_CNT;						// 0x4b801938
    UINT32 ATA_TBUF_START;                  // 0x4b80193c
    UINT32 ATA_TBUF_SIZE;					// 0x4b801940
    UINT32 ATA_SBUF_START;					// 0x4b801944
    UINT32 ATA_SBUF_SIZE;					// 0x4b801948
    UINT32 ATA_CADDR_TBUR;					// 0x4b80194c
    UINT32 ATA_CADDR_SBUF;					// 0x4b801950
    UINT32 ATA_PIO_DTR;						// 0x4b801954
    UINT32 ATA_PIO_FED;						// 0x4b801958
    UINT32 ATA_PIO_SCR;						// 0x4b80195c
    UINT32 ATA_PIO_LLR;						// 0x4b801960 
    UINT32 ATA_PIO_LMR;						// 0x4b801964
    UINT32 ATA_PIO_LHR;						// 0x4b801968
    UINT32 ATA_PIO_DVR;						// 0x4b80196c
    UINT32 ATA_PIO_CSD;						// 0x4b801970
    UINT32 ATA_PIO_DAD;						// 0x4b801974
    UINT32 ATA_PIO_READY;					// 0x4b801978
    UINT32 ATA_PIO_RDATA;					// 0x4b80197c
    UINT32 ATA_RESERVED6;					// 0x4b801980
    UINT32 ATA_RESERVED7;					// 0x4b801984
    UINT32 ATA_RESERVED8;					// 0x4b801988
    UINT32 ATA_RESERVED9;					// 0x4b80198c
    UINT32 BUS_FIFO_STATUS;					// 0x4b801990
    UINT32 ATA_FIFO_STATUS;					// 0x4b801994
} S3C2450_CFCARD_REG, *PS3C2450_CFCARD_REG;

#if __cplusplus
    }
#endif

#endif 

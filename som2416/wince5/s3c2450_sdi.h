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
//  Header: s3c2450_sdi.h
//
//  Defines the Secure Digital IO controller CPU register layout and
//  definitions.
//
#ifndef __S3C2450_SDI_H
#define __S3C2450_SDI_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2450_SDI_REG    
//
//  SDI Control Registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_SDI in the configuration file cpu_base_reg_cfg.h.
//

typedef struct 
{
    UINT32    SDICON;                 // control reg
    UINT32    SDIPRE;                 // baud rate prescaler reg
    UINT32    SDICARG;                // command arg reg
    UINT32    SDICCON;                // command control reg
    UINT32    SDICSTA;                // command status reg
    UINT32    SDIRSP0;                // response reg 0
    UINT32    SDIRSP1;                // response reg 1
    UINT32    SDIRSP2;                // response reg 2
    UINT32    SDIRSP3;                // response reg 3
    UINT32    SDIDTIMER;              // data/busy timer reg
    UINT32    SDIBSIZE;               // block size reg
    UINT32    SDIDCON;                // data control reg
    UINT32    SDIDCNT;                // data remain counter reg
    UINT32    SDIDSTA;                // data status reg
    UINT32    SDIFSTA;                // FIFO status reg
    UINT32    SDIIMSK;                // interrupt mask reg
    UINT32    SDIDAT32;                 // data reg
	UINT16	  SDIDAT16;
	UINT16	  PAD1;
	UINT8	  SDIDAT8;

} S3C2450_SDI_REG, *PS3C2450_SDI_REG;


#if __cplusplus
    }
#endif

#endif 

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
//  Header: s3c2450_ssmc.h
//
//  Defines the SSMC controller CPU register layout and definitions.
//
#ifndef __S3C2450_SSMC_H
#define __S3C2450_SSMC_H

#if __cplusplus
    extern "C" 
    {
#endif

//------------------------------------------------------------------------------
//  Type: S3C2450_SSMC_REG    
//
//  SSMC control registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_SSMC in the configuration file cpu_base_reg_cfg.h.
//

typedef struct {
    UINT32 SMBIDCYR0;             // 0x00
    UINT32 SMBWSTRDR0;             // 0x04
    UINT32 SMBWSTWRR0;             // 0x08
    UINT32 SMBWSTOENR0;             // 0x0C
    UINT32 SMBWSTWENR0;             // 0x10
    UINT32 SMBCR0;                  // 0x14
    UINT32 SMBSR0;                  // 0x18
    UINT32 SMBWSTBRDR0;             // 0x1C

    UINT32 SMBIDCYR1;              // 0x20
    UINT32 SMBWSTRDR1;             // 0x24
    UINT32 SMBWSTWRR1;             // 0x28
    UINT32 SMBWSTOENR1;             // 0x2C
    UINT32 SMBWSTWENR1;             // 0x30
    UINT32 SMBCR1;                  // 0x34
    UINT32 SMBSR1;                  // 0x38
    UINT32 SMBWSTBRDR1;             // 0x3C

    UINT32 SMBIDCYR2;              // 0x40
    UINT32 SMBWSTRDR2;             // 0x44
    UINT32 SMBWSTWRR2;             // 0x48
    UINT32 SMBWSTOENR2;             // 0x4C
    UINT32 SMBWSTWENR2;             // 0x50
    UINT32 SMBCR2;                  // 0x54
    UINT32 SMBSR2;                  // 0x58
    UINT32 SMBWSTBRDR2;             // 0x5C

    UINT32 SMBIDCYR3;              // 0x60
    UINT32 SMBWSTRDR3;             // 0x64
    UINT32 SMBWSTWRR3;             // 0x68
    UINT32 SMBWSTOENR3;             // 0x6C
    UINT32 SMBWSTWENR3;             // 0x70
    UINT32 SMBCR3;                  // 0x74
    UINT32 SMBSR3;                  // 0x78
    UINT32 SMBWSTBRDR3;             // 0x7C

    UINT32 SMBIDCYR4;              // 0x80
    UINT32 SMBWSTRDR4;             // 0x84
    UINT32 SMBWSTWRR4;             // 0x88
    UINT32 SMBWSTOENR4;             // 0x8C
    UINT32 SMBWSTWENR4;             // 0x90
    UINT32 SMBCR4;                  // 0x94
    UINT32 SMBSR4;                  // 0x98
    UINT32 SMBWSTBRDR4;             // 0x9C

    UINT32 SMBIDCYR5;              // 0xA0
    UINT32 SMBWSTRDR5;             // 0xA4
    UINT32 SMBWSTWRR5;             // 0xA8
    UINT32 SMBWSTOENR5;             // 0xAC
    UINT32 SMBWSTWENR5;             // 0xB0
    UINT32 SMBCR5;                  // 0xB4
    UINT32 SMBSR5;                  // 0xB8
    UINT32 SMBWSTBRDR5;             // 0xBC

    UINT32 PAD[80];                 // 0xC0 ~ 0x1FC

    UINT32 SSMCSR;                  // 0x200
    UINT32 SSMCCR;                  // 0x204 
} S3C2450_SSMC_REG, *PS3C2450_SSMC_REG;

#if __cplusplus
    }
#endif

#endif 

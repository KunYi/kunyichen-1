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
//  Header: s3c2450_clkpwr.h
//
//  Defines the clock and power register layout and definitions.
//
#ifndef __S3C2450_CLKPWR_H
#define __S3C2450_CLKPWR_H

#if __cplusplus
    extern "C" 
    {
#endif



//------------------------------------------------------------------------------
//  Type: S3C2450_CLKPWR_REG
//
//  Clock and Power Management registers.
//

typedef struct 
{

    UINT32   LOCKCON0;           // 0x00    // MPLL lock time count register
    UINT32   LOCKCON1;           // 0x04    // EPLL lock time count register
    UINT32   OSCSET;             // 0x08
    UINT32   PAD1;               // 0x0C
    UINT32   MPLLCON;            // 0x10    // MPLL configuration register
    UINT32   PAD2;               // 0x14
    UINT32   EPLLCON;            // 0x18    // EPLL configuration register
    UINT32   EPLLCON_K;               // 0x1C
    UINT32   CLKSRC;             // 0x20
    UINT32   CLKDIV0;            // 0x24
    UINT32   CLKDIV1;            // 0x28
    UINT32   CLKDIV2;               // 0x2C
    UINT32   HCLKCON;            // 0x30
    UINT32   PCLKCON;            // 0x34
    UINT32   SCLKCON;            // 0x38
    UINT32   PAD5;               // 0x3C
    UINT32   PWRMODE;            // 0x40
    UINT32   SWRST;              // 0x44        // Software reset control
    UINT32   PAD6;               // 0x48
    UINT32   PAD7;               // 0x4C
    UINT32   BUSPRI0;            // 0x50
    UINT32   PAD8;               // 0x54
    UINT32   PAD9;            // 0x58
    UINT32   ENDIAN;              // 0x5C
    UINT32   PWRCFG;             // 0x60
    UINT32   RSTCON;             // 0x64
    UINT32   RSTSTAT;            // 0x68
    UINT32   WKUPSTAT;           // 0x6C
    UINT32   INFORM0;            // 0x70
    UINT32   INFORM1;            // 0x74
    UINT32   INFORM2;            // 0x78
    UINT32   INFORM3;            // 0x7C
    UINT32   USB_PHYCTRL;               // 0x80
    UINT32   USB_PHYPWR;               // 0x84
    UINT32   USB_RSTCON;               // 0x88
    UINT32   USB_CLKCON;               // 0x8C

//    UINT32   CLKCON;                 // clock generator control register
//    UINT32   CLKSLOW;                // slow clock control register
//    UINT32   CLKDIVN;                // clock divider control register
//    UINT32	 CAMDIVN;				 // camera clock divider register

} S3C2450_CLKPWR_REG, *PS3C2450_CLKPWR_REG,  S3C2450_SYSCON_REG, *PS3C2450_SYSCON_REG;


#if __cplusplus
    }
#endif

#endif 

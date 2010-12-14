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
//  Header: s3c2450_usbd.h
//
//  Defines the USB device controller CPU register layout and definitions.
//
#ifndef __S3C2450_USBD_H
#define __S3C2450_USBD_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C2450_USBD_REG    
//
//  Defines the USB device control register block. This register bank is
//  located by the constant S3C2450_BASE_REG_PA_USBD in configuration file 
//  s3c2450_base_reg_cfg.h.
//

typedef struct
{                     
    UINT32 IR;                  // 0x00		Index Register 
    UINT32 EIR;                 // 0x04		Endpoint Interrupt Register
    UINT32 EIER;                // 0x08		Endpoing Interrupt Enable Register
    UINT32 FARR;                 // 0x0C		Function Address Register
    UINT32 FNR;                 // 0x10		Frame Number Register
    UINT32 EDR;                 // 0x14		Endpoint Direction Register
    UINT32 TR;                  // 0x18		Test Register
    UINT32 SSR;                 // 0x1C		System Status Register
    UINT32 SCR;                 // 0x20		System Control Register
    UINT32 EP0SR;               // 0x24		EP0 Status Register
    UINT32 EP0CR;               // 0x28		EP0 Control Register
    UINT32 ESR;                 // 0x2C		Endpoints Status Register
    UINT32 ECR;                 // 0x30		Endpoints Control Register
    UINT32 BRCR;                // 0x34		Byte Read Count Register
    UINT32 BWCR;                // 0x38		Byte Write Count Register
    UINT32 MPR;                 // 0x3C		Max Packet Register
    UINT32 DCR;                 // 0x40		DMA control Register
    UINT32 DTCR;                // 0x44		DMA Transfer Counter Register
    UINT32 DFCR;                // 0x48		DMA FIFO Counter Register
    UINT32 DTTCR1;              // 0x4C		DMA Total Transfer Counter1 Register
    UINT32 DTTCR2;              // 0x50		DMA Total Transfer Counter2 Register

    UINT32 PAD1[3];             // 0x54 ~ 0x5C

    UINT32 EP0BR;               // 0x60		EP0 Buffer Register
    UINT32 EP1BR;               // 0x64		EP1 Buffer Register
    UINT32 EP2BR;               // 0x68		EP2 Buffer Register
    UINT32 EP3BR;               // 0x6C		EP3 Buffer Register
    UINT32 EP4BR;               // 0x70		EP4 Buffer Register
    UINT32 EP5BR;               // 0x74		EP5 Buffer Register
    UINT32 EP6BR;               // 0x78		EP6 Buffer Register
    UINT32 EP7BR;               // 0x7C		EP7 Buffer Register
    UINT32 EP8BR;               // 0x80		EP8 Buffer Register

    UINT32 MICR;                // 0x84		Master Interface Control Register
    UINT32 MBAR;               // 0x88		Memory Base Address Register
    UINT32 MCAR;               // 0x8C		Memory Current Address Register
		
    UINT32 PAD2[4];                // 0x90 ~ 0x9C
    
    UINT32 FCON;		// 0x100	Burst FIFO-DMA Control
    UINT32 FSTAT;		// 0x104	Burst FIFIO Status

} S3C2450_USBD_REG, *PS3C2450_USBD_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 

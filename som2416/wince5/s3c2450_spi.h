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
//  Header: s3c2450_spi.h
//
//  Defines the Serial Peripheral Interface (SPI) controller CPU register layout and
//  definitions.
//
#ifndef __S3C2450_SPI_H
#define __S3C2450_SPI_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2450_SPI_REG    
//
//  Defines the SPI register layout. This register bank is located by the 
//  constant CPU_REG_BASE_XX_SPI in the configuration file cpu_reg_base_cfg.h.
//

typedef struct  
{
    UINT32      SPCON;
    UINT32      SPSTA;
    UINT32      SPPIN;
    UINT32      SPPRE;
    UINT32      SPTDAT;
    UINT32      SPRDAT;
    UINT32      SPTXFIFO;
    UINT32      SPRXFIFO;
    UINT32      SPRDATB;
    UINT32      SPFIC;
    UINT32      SPTOV;
    
    /*UINT32    SPCON0;             // chan 0 control reg
    UINT32    SPSTA0;             // chan 0 status reg
    UINT32    SPPIN0;             // chan 0 pin control reg
    UINT32    SPPRE0;             // chan 0 baud rate prescaler reg
    UINT32    SPTDAT0;            // chan 0 tx data reg
    UINT32    SPRDAT0;            // chan 0 rx data reg
    UINT32    PAD[2];
    UINT32    SPCON1;             // chan 1 control reg   - offset 0x20
    UINT32    SPSTA1;             // chan 1 status reg
    UINT32    SPPIN1;             // chan 1 pin control reg
    UINT32    SPPRE1;             // chan 1 baud rate prescaler reg
    UINT32    SPTDAT1;            // chan 1 tx data reg
    UINT32    SPRDAT1;            // chan 1 rx data reg*/

} S3C2450_SPI_REG, *PS3C2450_SPI_REG; 


#if __cplusplus
    }
#endif

#endif 

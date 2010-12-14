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
//  Header: s3c2450_iicbus.h
//
//  Defines the I2C bus controller CPU register layout and definitions.
//
#ifndef __S3C2450_IICBUS_H
#define __S3C2450_IICBUS_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2450_IICBUS_REG
//
//  Defines IIC bus control register layout. This register bank is located by
//  the constant CPU_BASE_REG_XX_IICBUS in the configuration file 
//  cpu_base_reg_cfg.h.
//

typedef struct 
{
    UINT32    IICCON;
    UINT32    IICSTAT;
    UINT32    IICADD;
    UINT32    IICDS;
    UINT32    IICLC;
    
} S3C2450_IICBUS_REG, *PS3C2450_IICBUS_REG;        


#if __cplusplus
    }
#endif

#endif 

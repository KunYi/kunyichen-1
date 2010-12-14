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
//  Header: s3c2450_iisbus.h
//
//  Defines the IIS bus control register layout and associated constants 
//  and types.
//
//------------------------------------------------------------------------------

#ifndef __S3C2450_IISBUS_H
#define __S3C2450_IISBUS_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: name    
//
//  Defines IIS bus control register layout. This register bank is located by
//  the constant CPU_BASE_REG_XX_IISBUS in the configuration file 
//  cpu_base_reg_cfg.h.
//

typedef struct 
{
    UINT32    IISCON;
    UINT32    IISMOD;
    UINT32    IISFIC;
    UINT32    IISPSR;
    UINT32    IISTXD;
    UINT32    IISRXD;
  //  UINT32    IISFCON;
  //  UINT32    IISFIFO;

} S3C2450_IISBUS_REG, *PS3C2450_IISBUS_REG;


#if __cplusplus
    }
#endif

#endif 

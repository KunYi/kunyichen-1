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
//  Header: s3c2450_matrix.h
//
//  Defines the MATRIX CPU register layout and definitions.
//
#ifndef __S3C2450_MATRIX_H
#define __S3C2450_MATRIX_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2450_MATRIX_REG    
//
//  MATRIX register layout. This register bank is located 
//  by the constant CPU_BASE_REG_XX_MATRIX in the configuration file 
//  cpu_base_reg_cfg.h.
//

typedef struct  
{
    UINT32  PRIORITY0;             //0x00         
    UINT32  PRIORITY1;             //0x04        
    UINT32  EBICON;                //0x08
    UINT32  ROMSEL;                //0x0C

} S3C2450_MATRIX_REG, *PS3C2450_MATRIX_REG;    


#if __cplusplus
    }
#endif

#endif 

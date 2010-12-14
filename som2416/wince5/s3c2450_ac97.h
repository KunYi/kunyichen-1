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
//  Header:  s3c2450_ac97.h
//
//  Defines the AC97 controller register layout and associated types 
//  and constants.
//
#ifndef __S3C2450_AC97_H
#define __S3C2450_AC97_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C2450_AC97_REG    
//
//  AC97 controller registers. This register bank is located by the 
//  constant S3C2450_BASE_REG_AC97 in the configuration file 
//  s3c2450_reg_base_cfg.h.
//

typedef struct {
    UINT32 AC_GLBCTRL;               // 0x00 
    UINT32 AC_GLBSTAT;               // 0x04
    UINT32 AC_CODEC_CMD;               // 0x08
    UINT32 AC_CODEC_STAT;               // 0x0C
    UINT32 AC_PCMADDR;               // 0x10
    UINT32 AC_MICADDR;               // 0x14
    UINT32 AC_PCMDATA;               // 0x18
    UINT32 AC_MICDATA;               // 0x1C

} S3C2450_AC97_REG, *PS3C2450_AC97_REG;

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif 

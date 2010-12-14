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
//  Header: s3c2450_pwm.h
//
//  Defines the PWM Timer register layout and associated types and constants.
//
#ifndef __S3C2450_PWM_H
#define __S3C2450_PWM_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C2450_PWM_REG
//
//  Defines the PWM Timer control register layout. This register bank is 
//  located by the constant S3C2450_BASE_REG_XX_PWM in the configuration file
//  s3c2450_base_reg_cfg.h.
//
typedef struct  {
    UINT32 TCFG0;                       //0x00
    UINT32 TCFG1;                       //0x04
    UINT32 TCON;                        //0x08
    UINT32 TCNTB0;                      //0x0C
    UINT32 TCMPB0;                      //0x10
    UINT32 TCNTO0;                      //0x14
    UINT32 TCNTB1;                      //0x18
    UINT32 TCMPB1;                      //0x1C
    UINT32 TCNTO1;                      //0x20
    UINT32 TCNTB2;                      //0x24
    UINT32 TCMPB2;                      //0x28
    UINT32 TCNTO2;                      //0x2C
    UINT32 TCNTB3;                      //0x30
    UINT32 TCMPB3;                      //0x34
    UINT32 TCNTO3;                      //0x38
    UINT32 TCNTB4;                      //0x3C
    UINT32 TCNTO4;                      //0x40

} S3C2450_PWM_REG, *PS3C2450_PWM_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 

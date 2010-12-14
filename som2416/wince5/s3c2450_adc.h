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
//  Header: s3c2450_adc.h
//
//  Defines the analog to digital (ADC) controller CPU register layout and 
//  definitions.
//
#ifndef __S3C2450_ADC_H
#define __S3C2450_ADC_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C2450_ADC_REG    
//
//  Defines the A/D converter's control register block. This register bank is
//  located by the constant S3C2450_BASE_REG_ADC in configuration file 
//  s3c2450_base_reg_cfg.h.
//

typedef struct {
    UINT32 ADCCON;                      // control register
    UINT32 ADCTSC;                      // touch screen control register
    UINT32 ADCDLY;                      // start or interval delay register
    UINT32 ADCDAT0;                     // conversion data register 0
    UINT32 ADCDAT1;                     // conversion data register 1
    UINT32 ADCUPDN;						// touch screen up-down int check
    UINT32 ADCMUX;
} S3C2450_ADC_REG, *PS3C2450_ADC_REG;        

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif 

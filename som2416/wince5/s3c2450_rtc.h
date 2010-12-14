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
//  Header: s3c2450_rtc.h
//
//  Defines the Real Time Clock (RTC) register layout and associated 
//  types and constants.
//
#ifndef __S3C2450_RTC_H__
#define __S3C2450_RTC_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C2450_RTC_REG    
//
//  RTC control registers. This register bank is located by the constant 
//  S3C2450_BASE_REG_XX_RTC in the configuration file s3c2450_base_reg_cfg.h.
//

typedef struct {
    UINT32 PAD1[16];                // pad to - 0x3C
    UINT32 RTCCON;                  // control reg              - 0x40
    UINT32 TICNT0;                   // tick time count reg      - 0x44
    UINT32 TICNT1;                 // pad
    UINT32 TICNT2;                   // tick time count reg      - 0x4C
    UINT32 RTCALM;                  // alarm control reg         - 0x50
    UINT32 ALMSEC;                  // alarm sec data reg        - 0x54
    UINT32 ALMMIN;                  // alarm min data reg        - 0x58
    UINT32 ALMHOUR;                 // alarm hour data reg       - 0x5C
    UINT32 ALMDATE;                 // alarm date (day) data reg - 0x60
    UINT32 ALMMON;                  // alarm month data reg      - 0x64
    UINT32 ALMYEAR;                 // alarm year data reg       - 0x68
    UINT32 PAD2;                  //         - 0x6C
    UINT32 BCDSEC;                  // BCD values...             - 0x70
    UINT32 BCDMIN;                  //                           - 0x74 
    UINT32 BCDHOUR;                 //                           - 0x78
    UINT32 BCDDATE;                 //                           - 0x7C
    UINT32 BCDDAY;                  //                           - 0x80
    UINT32 BCDMON;                  //                           - 0x84
    UINT32 BCDYEAR;                 //                           - 0x88
    UINT32 PAD3;                 // pad
    UINT32 TICKCNT;                 //                           - 0x90
    UINT32 PAD4;                 //                           - 0x94

} S3C2450_RTC_REG, *PS3C2450_RTC_REG;    

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 

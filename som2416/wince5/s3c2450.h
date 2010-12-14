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
//  Header: s3c2450.h
//
//  This header file defines the S3C2450 processor.
//
//  The s3C2450 is a System on Chip (SoC) part consisting of an ARM920T core. 
//  This header file is comprised of component header files that define the 
//  register layout of each component.
//  
//------------------------------------------------------------------------------
#ifndef __S3C2450_H
#define __S3C2450_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

// Base Definitions
#include "s3c2450_base_regs.h"

// SoC Components
#include "s3c2450_ac97.h"
#include "s3c2450_adc.h"
#include "s3c2450_clkpwr.h"
#include "s3c2450_dma.h"
#include "s3c2450_iicbus.h"
#include "s3c2450_iisbus.h"
#include "s3c2450_intr.h"
#include "s3c2450_ioport.h"
#include "s3c2450_lcd.h"
#include "s3c2450_memctrl.h"
#include "s3c2450_nand.h"
#include "s3c2450_pwm.h"
#include "s3c2450_rtc.h"
#include "s3c2450_sdi.h"
#include "s3c2450_spi.h"
#include "s3c2450_hsspi.h"
#include "s3c2450_uart.h"
#include "s3c2450_usbd.h"
#include "s3c2450_wdog.h"
#include "s3c2450_ssmc.h"
#include "s3c2450_cam.h"
#include "s3c2450_hsmmc.h"
#include "s3c2450_matrix.h"
#include "s3c2450_cfcard.h"
#include "s3c2450_sdram.h"

#include "s3c2450_dmatransfer.h"
//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 

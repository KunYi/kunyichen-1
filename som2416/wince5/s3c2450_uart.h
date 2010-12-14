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
//  Header:  s3c2450_uart.h
//
//  Defines the UART controller register layout associated types and constants.
//
#ifndef __S3C2450_UART_H
#define __S3C2450_UART_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C2450_UART_REG    
//
//  UART control registers. This register bank is located by the constant
//  S3C2450_BASE_REG_XX_UARTn in the configuration file
//  s3c2450_base_reg_cfg.h.
//
typedef struct {
    UINT32 ULCON;                   // line control reg
    UINT32 UCON;                    // control reg
    UINT32 UFCON;                   // FIFO control reg
    UINT32 UMCON;                   // modem control reg
    UINT32 UTRSTAT;                 // tx/rx status reg
    UINT32 UERSTAT;                 // rx error status reg
    UINT32 UFSTAT;                  // FIFO status reg
    UINT32 UMSTAT;                  // modem status reg
    UINT32 UTXH;                    // tx buffer reg
    UINT32 URXH;                    // rx buffer reg
    UINT32 UBRDIV;                  // baud rate divisor
    UINT32 UDIVSLOT;                // baud rate divisor
} S3C2450_UART_REG, *PS3C2450_UART_REG;

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif


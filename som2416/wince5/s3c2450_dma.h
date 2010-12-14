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
//  Header: s3c2450_dma.h
//
//  Defines the DMA controller CPU register layout and definitions.
//
#ifndef __S3C2450_DMA_H
#define __S3C2450_DMA_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2450_DMA_REG    
//
//  DMA control registers. This register bank is located by the constant 
//  CPU_BASE_REG_DMA in the configuration file cpu_base_reg_cfg.h.
//

typedef struct 
{
    // DMA 0

    UINT32    DISRC0;         // initial source reg               - 0x0
    UINT32    DISRCC0;        // initial source control reg       - 0x04
    UINT32    DIDST0;         // initial destination reg          - 0x08
    UINT32    DIDSTC0;        // initial destination control reg  - 0x0C
    UINT32    DCON0;          // control reg                      - 0x10
    UINT32    DSTAT0;         // count reg                        - 0x14
    UINT32    DCSRC0;         // current source reg               - 0x18
    UINT32    DCDST0;         // current destination reg          - 0x1C
    UINT32    DMASKTRIG0;     // mask trigger reg                 - 0x20
    UINT32    DMAREQSEL0;     // request selection reg            - 0x24
    UINT32    PAD1[54];        // pad                              - 0x28 - 0xFC

    // DMA 1

    UINT32    DISRC1;         // initial source reg               - 0x100
    UINT32    DISRCC1;        // initial source control reg       - 0x104
    UINT32    DIDST1;         // initial destination reg          - 0x108
    UINT32    DIDSTC1;        // initial destination control reg  - 0x10C
    UINT32    DCON1;          // control reg                      - 0x110
    UINT32    DSTAT1;         // count reg                        - 0x114
    UINT32    DCSRC1;         // current source reg               - 0x118
    UINT32    DCDST1;         // current destination reg          - 0x11C
    UINT32    DMASKTRIG1;     // mask trigger reg                 - 0x120
    UINT32    DMAREQSEL1;     // request selection reg            - 0x124
    UINT32    PAD2[54];        // pad                              - 0x128 - 0x1FC

    // DMA 2

    UINT32    DISRC2;         // initial source reg               - 0x200
    UINT32    DISRCC2;        // initial source control reg       - 0x204
    UINT32    DIDST2;         // initial destination reg          - 0x208
    UINT32    DIDSTC2;        // initial destination control reg  - 0x20C
    UINT32    DCON2;          // control reg                      - 0x210
    UINT32    DSTAT2;         // count reg                        - 0x214
    UINT32    DCSRC2;         // current source reg               - 0x218
    UINT32    DCDST2;         // current destination reg          - 0x21C
    UINT32    DMASKTRIG2;     // mask trigger reg                 - 0x220
    UINT32    DMAREQSEL2;     // request selection reg            - 0x224
    UINT32    PAD3[54];        // pad                             - 0x228 - 0xFC

    // DMA 3

    UINT32    DISRC3;         // initial source reg               - 0x300
    UINT32    DISRCC3;        // initial source control reg       - 0x304
    UINT32    DIDST3;         // initial destination reg          - 0x308
    UINT32    DIDSTC3;        // initial destination control reg  - 0x30C
    UINT32    DCON3;          // control reg                      - 0x310
    UINT32    DSTAT3;         // count reg                        - 0x314
    UINT32    DCSRC3;         // current source reg               - 0x318
    UINT32    DCDST3;         // current destination reg          - 0x31C
    UINT32    DMASKTRIG3;     // mask trigger reg                 - 0x320
    UINT32    DMAREQSEL3;     // request selection reg            - 0x324
    UINT32    PAD4[54];        // pad                             - 0x328 - 0x3FC

    // DMA 4

    UINT32    DISRC4;         // initial source reg               - 0x400
    UINT32    DISRCC4;        // initial source control reg       - 0x404
    UINT32    DIDST4;         // initial destination reg          - 0x408
    UINT32    DIDSTC4;        // initial destination control reg  - 0x40C
    UINT32    DCON4;          // control reg                      - 0x410
    UINT32    DSTAT4;         // count reg                        - 0x414
    UINT32    DCSRC4;         // current source reg               - 0x418
    UINT32    DCDST4;         // current destination reg          - 0x41C
    UINT32    DMASKTRIG4;     // mask trigger reg                 - 0x420
    UINT32    DMAREQSEL4;     // request selection reg            - 0x424
    UINT32    PAD5[54];       // pad                              - 0x428 - 0x4FC

    // DMA 5

    UINT32    DISRC5;         // initial source reg               - 0x500
    UINT32    DISRCC5;        // initial source control reg       - 0x504
    UINT32    DIDST5;         // initial destination reg          - 0x508
    UINT32    DIDSTC5;        // initial destination control reg  - 0x50C
    UINT32    DCON5;          // control reg                      - 0x510
    UINT32    DSTAT5;         // count reg                        - 0x514
    UINT32    DCSRC5;         // current source reg               - 0x518
    UINT32    DCDST5;         // current destination reg          - 0x51C
    UINT32    DMASKTRIG5;     // mask trigger reg                 - 0x520
    UINT32    DMAREQSEL5;     // request selection reg            - 0x524
    UINT32    PAD6[54];        // pad                             - 0x528 - 0x5FC

    // DMA 6

    UINT32    DISRC6;         // initial source reg               - 0x600
    UINT32    DISRCC6;        // initial source control reg       - 0x604
    UINT32    DIDST6;         // initial destination reg          - 0x608
    UINT32    DIDSTC6;        // initial destination control reg  - 0x60C
    UINT32    DCON6;          // control reg                      - 0x610
    UINT32    DSTAT6;         // count reg                        - 0x614
    UINT32    DCSRC6;         // current source reg               - 0x618
    UINT32    DCDST6;         // current destination reg          - 0x61C
    UINT32    DMASKTRIG6;     // mask trigger reg                 - 0x620
    UINT32    DMAREQSEL6;     // request selection reg            - 0x624
    UINT32    PAD7[54];        // pad                             - 0x628 - 0x6FC

    // DMA 7

    UINT32    DISRC7;         // initial source reg               - 0x700
    UINT32    DISRCC7;        // initial source control reg       - 0x704
    UINT32    DIDST7;         // initial destination reg          - 0x708
    UINT32    DIDSTC7;        // initial destination control reg  - 0x70C
    UINT32    DCON7;          // control reg                      - 0x710
    UINT32    DSTAT7;         // count reg                        - 0x714
    UINT32    DCSRC7;         // current source reg               - 0x718
    UINT32    DCDST7;         // current destination reg          - 0x71C
    UINT32    DMASKTRIG7;     // mask trigger reg                 - 0x720
    UINT32    DMAREQSEL7;     // request selection reg            - 0x724
    //UINT32    PAD8[54];        // NO pad                             - 0x728 - 0x7FC

} S3C2450_DMA_REG, *PS3C2450_DMA_REG;


// DMA values for diff devices using the DMA controller for the DMAREQSEL register
// DonGo
//
#define 		DMAREQSEL_SPI_0TX			(0<<1)
#define 		DMAREQSEL_SPI_0RX			(1<<1)
#define 		DMAREQSEL_SPI_1TX			(2<<1)
#define 		DMAREQSEL_SPI_1RX			(3<<1)
#define 		DMAREQSEL_I2SSDO			(4<<1)
#define 		DMAREQSEL_I2SSDI			(5<<1)
#define 		DMAREQSEL_PWM_TIMER		(9<<1)
#define 		DMAREQSEL_SDMMC			(10<<1)
//#define 		DMAREQSEL_USB_EP1		(13<<1)
//#define 		DMAREQSEL_USB_EP2		(14<<1)
//#define 		DMAREQSEL_USB_EP3		(15<<1)
//#define 		DMAREQSEL_USB_EP4		(16<<1)
#define 		DMAREQSEL_nXDREQ0		(17<<1)
#define 		DMAREQSEL_nXDREQ1		(18<<1)
#define 		DMAREQSEL_UART0_0		(19<<1)
#define 		DMAREQSEL_UART0_1		(20<<1)
#define 		DMAREQSEL_UART1_0		(21<<1)
#define 		DMAREQSEL_UART1_1		(22<<1)
#define 		DMAREQSEL_UART2_0		(23<<1)
#define 		DMAREQSEL_UART2_1		(24<<1)
#define 		DMAREQSEL_UART3_0		(25<<1)
#define 		DMAREQSEL_UART3_1		(26<<1)
#define 		DMAREQSEL_PCMOUT			(27<<1)
#define 		DMAREQSEL_PCMIN			(28<<1)
#define 		DMAREQSEL_MICIN			(29<<1)

#if __cplusplus
    }
#endif

#endif 

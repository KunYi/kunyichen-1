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
//  Header:  s3c2450_intr.h
//
//  Defines the interrupt controller register layout and associated interrupt
//  sources and bit masks.
//
#ifndef __S3C2450_INTR_H
#define __S3C2450_INTR_H

#if __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------
//
//  Type: S3C2450_INTR_REG    
//
//  Interrupt control registers. This register bank is located by the constant 
//  S3C2450_BASE_REG_XX_INTR in the configuration file s3c2450_base_reg_cfg.h.
//

typedef struct {

    UINT32 SRCPND1;                     // interrupt request status reg
    UINT32 INTMOD1;                     // interrupt mode reg
    UINT32 INTMSK1;                     // interrupt mask reg
    UINT32 PAD0;                     
    
    UINT32 INTPND1;                     // interrupt pending reg
    UINT32 INTOFFSET1;                  // interrupt offset reg
    UINT32 SUBSRCPND;                  // SUB source pending reg
    UINT32 INTSUBMSK;                  // interrupt SUB mask reg


    UINT32 PAD1[4]; 
	
    UINT32 PRIORITY_MODE1;                   // priority reg
    UINT32 PRIORITY_UPDATE1;                   // priority reg
    UINT32 PAD2[2];


    UINT32 SRCPND2;                     // interrupt request status reg
    UINT32 INTMOD2;                     // interrupt mode reg
    UINT32 INTMSK2;                     // interrupt mask re
    UINT32 PAD3;                     

	
    UINT32 INTPND2;                     // interrupt pending reg
    UINT32 INTOFFSET2;                  // interrupt offset reg
    UINT32 PAD4[2]; 
	
    UINT32 PAD5[4];                    


    UINT32 PRIORITY_MODE2;                   // priority reg
    UINT32 PRIORITY_UPDATE2;                   // priority reg
    UINT32 PAD6[2];
} S3C2450_INTR_REG, *PS3C2450_INTR_REG;


//------------------------------------------------------------------------------
//
//  Define: IRQ_XXX
//
//  Interrupt sources numbers
//

#define IRQ_EINT0           0           // Arbiter 0
#define IRQ_EINT1           1
#define IRQ_EINT2           2
#define IRQ_EINT3           3

#define IRQ_EINT4_7         4           // Arbiter 1
#define IRQ_EINT8_23        5
#define IRQ_CAM             6
#define IRQ_BAT_FLT         7
#define IRQ_TICK            8
#define IRQ_WDT_AC97        9

#define IRQ_TIMER0          10          // Arbiter 2
#define IRQ_TIMER1          11
#define IRQ_TIMER2          12
#define IRQ_TIMER3          13
#define IRQ_TIMER4          14
#define IRQ_UART2           15

#define IRQ_LCD             16          // Arbiter 3
#define IRQ_DMA             17
#define IRQ_UART3           18
#define IRQ_CFCON           19
#define IRQ_SDI_1           20
#define IRQ_SDI_0           21
#define IRQ_SPI1            22          // Arbiter 4
#define IRQ_UART1           23
#define IRQ_NFCON           24
#define IRQ_USBD            25
#define IRQ_USBH            26
#define IRQ_IIC             27

#define IRQ_UART0           28          // Arbiter 5
#define IRQ_SPI0            29
#define IRQ_RTC             30
#define IRQ_ADC             31

#define IRQ_2D           32
#define IRQ_IIC1           33
#define IRQ_GPS           34
#define IRQ_NONE          35
#define IRQ_PCM0           36
#define IRQ_PCM1           37
#define IRQ_I2S0        38
#define IRQ_I2S1          39


#define IRQ_DMA0   		  40
#define IRQ_DMA1   		  41
#define IRQ_DMA2  		  42
#define IRQ_DMA3   		  43
#define IRQ_DMA4   		  44
#define IRQ_DMA5   		  45
#define IRQ_DMA6   		  46
#define IRQ_DMA7   		  47


#define	IRQ_CAM_C	 		48
#define	IRQ_CAM_P	 		49

#define   IRQ_LCD_VSYNC       50

//[david.modify] 2008-05-14 09:53
// 2443 audio driver using
//==========================
#define IRQ_AUDIO		52

//==========================


#define IRQ_EINT4           51
#define IRQ_EINT5           52
#define IRQ_EINT6           53
#define IRQ_EINT7           54


//[david.modify] 2008-05-16 10:50
//================================= 
// KEYBD 有三个键,对应如下;
//[david.modify] 2008-05-16 10:19
//GPG2	KEY6(up)		音量加EINT10
//GPG1	KEY4 (OK)	确认键EINT9
//GPG0	KEY3 (down)	音量减EINT8

#define IRQ_EINT8           55
#define IRQ_EINT9           56
#define IRQ_EINT10         57
#define IRQ_KEYPAD IRQ_EINT10
//================================= 

#define IRQ_EINT11         58
#define IRQ_EINT12         59
#define IRQ_EINT13         60
#define IRQ_EINT14         61
#define IRQ_EINT15         62
#define IRQ_EINT16          63

//SysINT Number limit : 64
//#define IRQ_EINT17          63
//#define IRQ_EINT18          64
//#define IRQ_EINT19          63
//#define IRQ_EINT20          64
//#define IRQ_EINT21          65
//#define IRQ_EINT22          66
//#define IRQ_EINT23          67

#define IRQ_LAST          IRQ_EINT16

// Interrupt sub-register source numbers
//
#define IRQ_SUB_RXD0     0
#define IRQ_SUB_TXD0     1
#define IRQ_SUB_ERR0     2
#define IRQ_SUB_RXD1     3
#define IRQ_SUB_TXD1     4
#define IRQ_SUB_ERR1     5
#define IRQ_SUB_RXD2     6
#define IRQ_SUB_TXD2     7
#define IRQ_SUB_ERR2     8
#define IRQ_SUB_TC       9
#define IRQ_SUB_ADC      10
#define	IRQ_SUB_CAM_C	 11
#define	IRQ_SUB_CAM_P	 12
#define IRQ_SUB_LCD1     14
#define IRQ_SUB_LCD2     15
#define IRQ_SUB_LCD3     16
#define IRQ_SUB_LCD4     17
#define IRQ_SUB_DMA0     18
#define IRQ_SUB_DMA1     19
#define IRQ_SUB_DMA2     20
#define IRQ_SUB_DMA3     21
#define IRQ_SUB_DMA4     22
#define IRQ_SUB_DMA5     23
#define IRQ_SUB_RXD3     24
#define IRQ_SUB_TXD3     25
#define IRQ_SUB_ERR3     26
#define	IRQ_SUB_WDT	 27 	
#define	IRQ_SUB_AC97	 28 
#define	IRQ_SUB_DMA6	 29 
#define	IRQ_SUB_DMA7	 30 

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 

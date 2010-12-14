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
//  Header: s3c2450_lcd.h
//
//  Defines the LCD controller CPU register layout and definitions.
//
#ifndef __S3C2450_LCD_H
#define __S3C2450_LCD_H

#if __cplusplus
    extern "C" 
    {
#endif

//------------------------------------------------------------------------------
//  Type: S3C2450_LCD_REG    
//
//  LCD control registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_LCD in the configuration file cpu_base_reg_cfg.h.
//

typedef struct {
    UINT32 VIDCON0;             // 0x00
    UINT32 VIDCON1;             // 0x04
    UINT32 VIDTCON0;             // 0x08
    UINT32 VIDTCON1;             // 0x0C
    UINT32 VIDTCON2;             // 0x10
    UINT32 WINCON0;           // 0x14
    UINT32 WINCON1;           // 0x18
    UINT32 PAD[3];     

    UINT32 VIDOSD0A;           // 0x28
    UINT32 VIDOSD0B;              // 0x2C
    UINT32 VIDOSD0C;            // 0x30
    UINT32 VIDOSD1A;             // 0x34
    UINT32 VIDOSD1B;             // 0x38
    UINT32 VIDOSD1C;             // 0x3C
    
    UINT32 PAD1[9];              // 0x40 ~ 0x60 // PAD

    UINT32 VIDW00ADD0B0;             // 0x64
    UINT32 VIDW00ADD0B1;             // 0x68
    UINT32 VIDW01ADD0;               // 0x6C

    UINT32 PAD2[3];              // 0x70 ~ 0x78 // PAD

    UINT32 VIDW00ADD1B0;             // 0x7C
    UINT32 VIDW00ADD1B1;             // 0x80
    UINT32 VIDW01ADD1;               // 0x84

    UINT32 PAD3[3];              // 0x88 ~ 0x90 // PAD

    UINT32 VIDW00ADD2B0;             // 0x94
    UINT32 VIDW00ADD2B1;             // 0x98
    UINT32 VIDW01ADD2;               // 0x9C

    UINT32 PAD4[3];              // 0xA0 ~ 0xA8 // PAD
    
    UINT32 VIDINTCON;                // 0xAC
    UINT32 W1KEYCON0;                // 0xB0
    UINT32 W1KEYCON1;                // 0xB4
    UINT32 W2KEYCON0;                // 0xB8
    UINT32 W2KEYCON1;                // 0xBC
    UINT32 W3KEYCON0;                // 0xC0
    UINT32 W3KEYCON1;                // 0xC4
    UINT32 W4KEYCON0;                // 0xC8
    UINT32 W4KEYCON1;                // 0xCC
    UINT32 WIN0MAP;                  // 0xD0
    UINT32 WIN1MAP;                  // 0xD4

    UINT32 PAD5[3];              // 0xD8 ~ 0xE0 // PAD

    UINT32 WPALCON;                  // 0xE4

    UINT32 PAD6[18];              // 0xE8 ~ 0x12C // PAD

    UINT32 SYSIFCON0;                // 0x130
    UINT32 SYSIFCON1;                // 0x134
    UINT32 DITHMODE;                 // 0x138

} S3C2450_LCD_REG, *PS3C2450_LCD_REG;


//------------------------------------------------------------------------------
//  Define: LCD_TYPE_XXX
//
//  Enumerates the types of LCD displays available.
//

#define    LCD_TYPE_STN8BPP            (1)
#define    LCD_TYPE_TFT16BPP           (2)

//------------------------------------------------------------------------------
//  Define: LCD_TYPE
//
//  Defines the active LCD type from above choices.
//

#define    LCD_TYPE                    LCD_TYPE_TFT16BPP

//------------------------------------------------------------------------------
//  Define: LCD_MODE_XXX
//
//  Defines the LCD mode.
//

#define    LCD_MODE_STN_1BIT       (1)
#define    LCD_MODE_STN_2BIT       (2)
#define    LCD_MODE_STN_4BIT       (4)
#define    LCD_MODE_CSTN_8BIT      (108)
#define    LCD_MODE_CSTN_12BIT     (112)
#define    LCD_MODE_TFT_1BIT       (201)
#define    LCD_MODE_TFT_2BIT       (202)
#define    LCD_MODE_TFT_4BIT       (204)
#define    LCD_MODE_TFT_8BIT       (208)
#define    LCD_MODE_TFT_16BIT      (216)

//------------------------------------------------------------------------------
//  Define: LCD_SCR_XXX
//
//  Screen size definitions.
//



#define    LCD_MVAL                (13)
#define    LCD_MVAL_USED           (0)

// STN/CSTN timing parameter for LCBHBT161M(NANYA)

#define    LCD_WLH                 (3)
#define    LCD_WDLY                (3)
#define    LCD_LINEBLANK           ((1)&0xff)

// TFT timing parameter for V16C6448AB(PRIME VIEW) 

#define    LCD_VBPD                ((1)&0xff)
#define    LCD_VFPD                ((2)&0xff)
#define    LCD_VSPW                ((1)&0x3f)
#define    LCD_HBPD                ((6)&0x7f)
#define    LCD_HFPD                ((2)&0xff)
#define    LCD_HSPW                ((4)&0xff)

// TFT Video Main Control 1 Register Status Field (read only)
#define		LCD_LINECNT						(0x3ff <<(16))
#define		LCD_VSTATUS						(0x3 << (13))
#define		LCD_HSTATUS						(0x3 << (11))

//------------------------------------------------------------------------------
//  Define: LCD_CLKVAL_XXX
//
//  Clock values
//

#define     CLKVAL_STN_MONO         (22)    

// 69.14hz @60Mhz,WLH=16clk,WDLY=16clk,LINEBLANK=1*8,VD=4 

#define     CLKVAL_STN_GRAY         (12)    

//124hz @60Mhz,WLH=16clk,WDLY=16clk,LINEBLANK=1*8,VD=4  

#define     CLKVAL_CSTN             (8)     

//135hz @60Mhz,WLH=16clk,WDLY=16clk,LINEBLANK=1*8,VD=8  

#define     CLKVAL_TFT              (6)

// NOTE: 1)SDRAM should have 32-bit bus width. 
//      2)HBPD,HFPD,HSPW should be optimized. 
// 44.6hz @75Mhz
// VSYNC,HSYNC should be inverted
// HBPD=48VCLK,HFPD=16VCLK,HSPW=96VCLK
// VBPD=33HSYNC,VFPD=10HSYNC,VSPW=2HSYNC

#define     M5D(n)                  ((n)&0x1fffff)


#if __cplusplus
    }
#endif

#endif 

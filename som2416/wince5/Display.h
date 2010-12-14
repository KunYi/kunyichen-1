/*********************************************************************
;* Project Name : s3c2450x
;*
;* Copyright 2006 by Samsung Electronics, Inc.
;* All rights reserved.
;*
;* Project Description :
;* This software is only for verifying functions of the s3c2450x
;* Anybody can use this code without our permission.
;**********************************************************************/

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_cfg.h"

#define LCD_DELAY_1MS       (180000)    //180000    //on the basis of 540MHz


#if (LCD_MODULE_TYPE == LCD_MODULE_LTS222) 

//* SPECIFIC FOR LTS222 MODULE
#define LTS222_VBPD		(9)		
#define LTS222_VFPD		(10)
#define LTS222_VSPW		(3)
#define LTS222_HBPD		(1)		
#define LTS222_HFPD		(6)
#define LTS222_HSPW		(3)
#define LTS222_HOZVAL	(240)
#define LTS222_LINEVAL	(320)

#define LTS222_FRAME_RATE   (65)

#elif (LCD_MODULE_TYPE == LCD_MODULE_LTV350)
//* SPECIFIC FOR LTV350 MODULE
#define LTV350_VBPD		(5)
#define LTV350_VFPD		(3)
#define LTV350_VSPW		(4)
#define LTV350_HBPD		(5)
#define LTV350_HFPD		(3)
#define LTV350_HSPW		(10)
#define LTV350_HOZVAL	(320)
#define LTV350_LINEVAL	(240)

#define LTV350_FRAME_RATE   (65)

#define LCD_WIDTH       (320)
#define LCD_HEIGHT      (240)

#elif (LCD_MODULE_TYPE == LCD_MODULE_LTE480WV)
//* SPECIFIC FOR LTE480(WVGA) MODULE FOR PARALLEL RGB INTERFACE
#define LTE480_VBPD		(7)	// 8-1
#define LTE480_VFPD		(5)	// 5-1	
#define LTE480_VSPW		(1)	// 1-1
#define LTE480_HBPD		(13)// 13-1
#define LTE480_HFPD		(8)	// 8-1
#define LTE480_HSPW		(3)	// 3-1
#define LTE480_HOZVAL	(800)// Horizontal pixel 800
#define LTE480_LINEVAL	(480)// Vertical pixel 480

#define LTE480_FRAME_RATE	(30)


#define LCD_WIDTH       (800)
#define LCD_HEIGHT      (480)

 //[david.modify] 2008-05-08 19:07
#elif (LCD_MODULE_TYPE == BSP_LCD_SHARP_LQ035Q1)
//* SPECIFIC FOR sharp MODULE
//* SPECIFIC FOR sharp MODULE
/*
sharp 3.5inch lcd:
=======================
HBP=8, HFP=8 -- 
VBP=2, VFP=2
*/
#if 1
#define SHARP_VBPD		(1)
#define SHARP_VFPD		(1)
#define SHARP_VSPW		(1)
#define SHARP_HBPD		(8)
#define SHARP_HFPD		(8)
#define SHARP_HSPW		(2)
#define 	VIDCON0_CLKVAL_VAR 0x16  //[david.modify] 2008-05-15 10:18
#endif

#define SHARP_HOZVAL	(320)
#define SHARP_LINEVAL	(240)

#define SHARP_FRAME_RATE	60
#define LCD_WIDTH	320
#define LCD_HEIGHT	240

#elif ( LCD_MODULE_TYPE == BSP_LCD_BYD_43INCH_480X272 || \
        LCD_MODULE_TYPE == BSP_LCD_INNOLUX_43 )
#define SHARP_VBPD		(1)
#define SHARP_VFPD		(1)
#define SHARP_VSPW		(1)
#define SHARP_HBPD		(8)
#define SHARP_HFPD		(8)
#define SHARP_HSPW		(2)
 //[david.modify] 2008-05-15 10:22
 // S3C2450_HCLK=133 --> 133/0x16 = 133/22 = 6.05mhz
#define 	VIDCON0_CLKVAL_VAR 0x16  //[david.modify] 2008-05-15 10:18

#define SHARP_HOZVAL	(480)
#define SHARP_LINEVAL	(272)

#define SHARP_FRAME_RATE	60
#define LCD_WIDTH	480
#define LCD_HEIGHT	272

#elif (LCD_MODULE_TYPE == BSP_LCD_YASSY_43INCH_480X272)
#define SHARP_VBPD		(1)
#define SHARP_VFPD		(1)
#define SHARP_VSPW		(1)
#define SHARP_HBPD		(8)
#define SHARP_HFPD		(8)
#define SHARP_HSPW		(2)
 //[david.modify] 2008-05-15 10:22
 // S3C2450_HCLK=133 --> 133/0x16 = 133/22 = 6.05mhz
#define 	VIDCON0_CLKVAL_VAR 0x16  //[david.modify] 2008-05-15 10:18

#define SHARP_HOZVAL	(480)
#define SHARP_LINEVAL	(272)

#define SHARP_FRAME_RATE	60
#define LCD_WIDTH	480
#define LCD_HEIGHT	272

#elif (LCD_MODULE_TYPE == BSP_LCD_INNOLUX_35)
//群创
#define LCD_WIDTH	320
#define LCD_HEIGHT	240

#endif

//* VIDCON0	
#define VIDCON0_S_RGB_IF					(0<<22)
#define VIDCON0_S_RGB_PAR					(0<<13)
#define VIDCON0_S_BGR_PAR					(1<<13)
#define VIDCON0_S_CLKVAL_F_AlWAYS_UPDATE	(0<<12)
#define VIDCON0_S_CLKVAL_F_SOF_UPDATE		(1<<12)
#define VIDCON0_S_VCLK_GATING_ON			(0<<5)
#define VIDCON0_S_VCLK_GATING_OFF			(1<<5)
#define VIDCON0_S_CLKDIR_DIRECT				(0<<4)
#define VIDCON0_S_CLKDIR_DIVIDED			(1<<4)
#define VIDCON0_S_CLKSEL_HCLK				(0<<2)
#define VIDCON0_S_CLKSEL_UPLL				(1<<2)
#define VIDCON0_S_ENVID_OFF					(0<<1)
#define VIDCON0_S_EVVID_ON					(1<<1)
#define VIDCON0_S_ENVID_F_OFF				(0<<0)
#define VIDCON0_S_ENVID_F_ON				(1<<0)
//bit shift
#define VIDCON0_CLKVAL_F_SHIFT				(6)

//* VIDCON1
#define VIDCON1_S_VCLK_FALL_EDGE_FETCH		(0<<7)
#define VIDCON1_S_VCLK_RISE_EDGE_FETCH		(1<<7)
#define VIDCON1_S_HSYNC_INVERTED			(1<<6)
#define VIDCON1_S_VSYNC_INVERTED			(1<<5)
#define VIDCON1_S_VDEN_INVERTED				(1<<4)


//* VIDTCON0,1
//bit shift
#define VIDTCON0_BPD_S				(16)
#define VIDTCON0_FPD_S				(8)
#define VIDTCON0_SPW_S				(0)

//* VIDTCON2
//bit shift
#define VIDTCON2_LINEVAL_S			(11)
#define VIDTCON2_HOZVAL_S			(0)
/*
//* WINCON0
//shift
#define WINCON0_INRGB_S				(13)
*/
//* WINCON1to4
#define WINCONx_BIT_SWAP_ON			(1<<2)	//shift on basis of half-word swap
#define WINCONx_BYTE_SWAP_ON		(1<<1)	//shift on basis of half-word swap
#define WINCONx_HALFW_SWAP_ON		(1<<0)	//shift on basis of half-word swap
#define WINCONx_4WORD_BURST			(2)
#define WINCONx_8WORD_BURST			(1)
#define WINCONx_16WORD_BURST		(0)
#define WINCONx_PLANE_BLENDING		(0)
#define WINCONx_PIXEL_BLENDING		(1)
#define WINCONx_1BPP_PALLET			(0)
#define WINCONx_2BPP_PALLET			(1)
#define WINCONx_4BPP_PALLET			(2)
#define WINCONx_8BPP_PALLET			(3)
#define WINCONx_8BPP_NO_PALLET		(4)
#define WINCONx_16BPP_565			(5)
#define WINCONx_16BPP_A555			(6)
#define WINCONx_16BPP_1555			(7)
#define WINCONx_18BPP_666			(8)
#define WINCONx_18BPP_A665			(9)
#define WINCONx_19BPP_A666			(10)
#define WINCONx_24BPP_888			(11)
#define WINCONx_24BPP_A887			(12)
#define WINCONx_25BPP_A888			(13)
#define WINCONx_ALPHA_MODE_0		(0)
#define WINCONx_ALPHA_MODE_1		(1)


//bit shift
#define WINCON_SWAP_S				(16)
#define WINCON_BURSTLEN_S			(9)
#define WINCON_BLENDING_S			(6)
#define WINCON_BPP_S				(2)
#define WINCON_ALPHA_S				(1)

//* VIDWxADD2
//bit shift
#define VIDWxADD2_OFFSET_SIZE_S		(13)
#define VIDWxADD2_PAGE_WIDTH_S		(0)

//* VIDOSDxA,B,C
//bit shift
#define VIDOSDxAB_HORIZON_X_S		(11)
#define VIDOSDxAB_VERTICAL_Y_S		(0)
#define VIDOSDxC_ALPHA0_S		(12)

/*
#define LCD_WIN_0					0
#define LCD_WIN_1					1
#define LCD_WIN_2					2
#define LCD_WIN_3					3
#define LCD_WIN_4					4
#define LCD_WIN_ALL					5

#define LCD_OFF						0
#define LCD_ON						1


	// set spi for lcd 
	// 1. set jumper like as 1-2 in J15 at SMDK board
	// 2. you can use the SPI1 to control LCD sfr register
	// nSS1 		---> SPI_LCDnSS  	---> GPL 14
	// SPIMOSI1	---> SPI_LCDMOSI      ---> GPL 11
	// SPICLK1    ---> SPI_LCDCLK        ---> GPL 10
*/
#if 0
#define LCD_DEN		(1<<14)
#define LCD_DSERI	(1<<11)
#define LCD_DCLK	(1<<10)

#define LCD_DEN_BIT     (14)
#define LCD_DSERI_BIT   (11)
#define LCD_DCLK_BIT    (10)

#define LCD_nRESET      (1)



//#define LCD_RESET     (0)
//david
//#if 1	//org.
#define LCD_DEN_Lo		(s2450IOP->GPLDAT &= ~LCD_DEN)
#define LCD_DEN_Hi		(s2450IOP->GPLDAT |=	LCD_DEN)
#define LCD_DCLK_Lo		(s2450IOP->GPLDAT &= ~LCD_DCLK)
#define LCD_DCLK_Hi		(s2450IOP->GPLDAT |=	LCD_DCLK)
#define LCD_DSERI_Lo	(s2450IOP->GPLDAT &= ~LCD_DSERI)
#define LCD_DSERI_Hi	(s2450IOP->GPLDAT |=	LCD_DSERI)
#endif

/*
S3C2416	描述	说明	Input/Output	Nomal State	Sleep State
GPB9	LCDRST	复位信号	O	LCD复位信号	
GPB5	LCDCS	SPI片选，低有效	O	用作SPI初始化LCD屏用	
GPB6	LCDCLK	SPI CLK	O	因为GPB6用于JTAG用， 现在用GPE13替换GPB6功能	GPE13
GPB10	LCDSDA	SPI DATA	I/O		
*/
#define LCD_DEN_BIT		5
#define LCD_DSERI_BIT	10
#define LCD_DCLK_BIT	13
#define LCD_DEN		(1<<5)
#define LCD_DSERI	(1<<10)
#define LCD_DCLK	(1<<13)	 //[david.modify] 2008-05-12 18:01
#define LCD_nRESET		9 //	1
#define LCD_DEN_Lo		(s2450IOP->GPBDAT &= ~LCD_DEN)
#define LCD_DEN_Hi		(s2450IOP->GPBDAT |=	LCD_DEN)
//#define LCD_DCLK_Lo		(s2450IOP->GPBDAT &= ~LCD_DCLK)
//#define LCD_DCLK_Hi		(s2450IOP->GPBDAT |=	LCD_DCLK)
 //[david.modify] 2008-05-12 18:00
 // 因为GPB6用于JTAG用， 现在用GPE13替换GPB6功能
#define LCD_DCLK_Lo		(s2450IOP->GPEDAT &= ~LCD_DCLK)
#define LCD_DCLK_Hi		(s2450IOP->GPEDAT |=	LCD_DCLK)
#define LCD_DSERI_Lo	(s2450IOP->GPBDAT &= ~LCD_DSERI)
#define LCD_DSERI_Hi	(s2450IOP->GPBDAT |=	LCD_DSERI)


/*
#else	//specific for mDirac3
#define LCD_DEN_Lo	\
{ \
	extern uint32 var_GPFDAT; \
	var_GPFDAT &= ~LCD_DEN; \
	rGPFDAT = var_GPFDAT; \
}
#define LCD_DEN_Hi	\
{ \
	extern uint32 var_GPFDAT; \
	var_GPFDAT |= LCD_DEN; \
	rGPFDAT = var_GPFDAT; \
}
#define LCD_DCLK_Lo	\
{ \
	extern uint32 var_GPFDAT; \
	var_GPFDAT &= ~LCD_DCLK; \
	rGPFDAT = var_GPFDAT; \
}
#define LCD_DCLK_Hi	\
{ \
	extern uint32 var_GPFDAT; \
	var_GPFDAT |= LCD_DCLK; \
	rGPFDAT = var_GPFDAT; \
}
#define LCD_DSERI_Lo	\
{ \
	extern uint32 var_GPFDAT; \
	var_GPFDAT &= ~LCD_DSERI; \
	rGPFDAT = var_GPFDAT; \
}
#define LCD_DSERI_Hi	\
{ \
	extern uint32 var_GPFDAT; \
	var_GPFDAT |= LCD_DSERI; \
	rGPFDAT = var_GPFDAT; \
}
#endif

#define LCD_RESET_Lo	(0)
#define LCD_RESET_Hi	(1)


*/

#if (LCD_MODULE_TYPE == LCD_MODULE_LTE480WV) 
#define    LCD_SCR_XSIZE           (800)           // virtual screen  
#define    LCD_SCR_YSIZE           (480)

#define    LCD_SCR_XSIZE_TFT       (800)           // virtual screen  
#define    LCD_SCR_YSIZE_TFT       (480)

#elif (LCD_MODULE_TYPE == LCD_MODULE_LTV350)
#define    LCD_SCR_XSIZE           (640)           // virtual screen  
#define    LCD_SCR_YSIZE           (480)

#define    LCD_SCR_XSIZE_TFT       (480)           // virtual screen  
#define    LCD_SCR_YSIZE_TFT       (640)


#elif (LCD_MODULE_TYPE == BSP_LCD_BYD_4281L) //[david.modify] 2008-05-13 15:15
#define    LCD_SCR_XSIZE           (640)           // virtual screen  
#define    LCD_SCR_YSIZE           (480)

#define    LCD_SCR_XSIZE_TFT       (480)           // virtual screen  
#define    LCD_SCR_YSIZE_TFT       (640)
#elif (LCD_MODULE_TYPE == BSP_LCD_YASSY_YF35F03CIB) //[david.modify] 2008-05-13 15:15
#define    LCD_SCR_XSIZE           (640)           // virtual screen  
#define    LCD_SCR_YSIZE           (480)

#define    LCD_SCR_XSIZE_TFT       (480)           // virtual screen  
#define    LCD_SCR_YSIZE_TFT       (640)
#elif (LCD_MODULE_TYPE == BSP_LCD_SHARP_LQ035Q1) //[david.modify] 2008-05-13 15:15
#define    LCD_SCR_XSIZE           (640)           // virtual screen  
#define    LCD_SCR_YSIZE           (480)

#define    LCD_SCR_XSIZE_TFT       (480)           // virtual screen  
#define    LCD_SCR_YSIZE_TFT       (640)
#elif (LCD_MODULE_TYPE == BSP_LCD_BYD_43INCH_480X272)|| \
      (LCD_MODULE_TYPE == BSP_LCD_INNOLUX_43 )
#define    LCD_SCR_XSIZE           (480)           // virtual screen  
#define    LCD_SCR_YSIZE           (272)

#define    LCD_SCR_XSIZE_TFT       (480)           // virtual screen  
#define    LCD_SCR_YSIZE_TFT       (272)
#elif (LCD_MODULE_TYPE == BSP_LCD_YASSY_43INCH_480X272) //[david.modify] 2008-05-13 15:15
#define    LCD_SCR_XSIZE           (480)           // virtual screen  
#define    LCD_SCR_YSIZE           (272)

#define    LCD_SCR_XSIZE_TFT       (480)           // virtual screen  
#define    LCD_SCR_YSIZE_TFT       (272)
#endif

//------------------------------------------------------------------------------
//  Define: LCD_*SIZE_XXX
//
//  Defines physical screen sizes and orientation.
//
#if (LCD_MODULE_TYPE == LCD_MODULE_LTE480WV) 
#define    LCD_XSIZE_TFT           (800)   
#define    LCD_YSIZE_TFT           (480)

#define    LCD_XSIZE_STN           (800)
#define    LCD_YSIZE_STN           (480)

#define    LCD_XSIZE_CSTN          (800)
#define    LCD_YSIZE_CSTN          (480)

#define     LCD_COLOR_DEPTH         (16)    // If you want to use 24bpp LCD then change this to 32
                                            // And config.bib and image_cfg.h must be changed for framebuffer address

#elif (LCD_MODULE_TYPE == LCD_MODULE_LTV350)
#define    LCD_XSIZE_TFT           (320)   
#define    LCD_YSIZE_TFT           (240)

#define    LCD_XSIZE_STN           (320)
#define    LCD_YSIZE_STN           (240)

#define    LCD_XSIZE_CSTN          (320)
#define    LCD_YSIZE_CSTN          (240)

#define     LCD_COLOR_DEPTH         (16)

#elif (LCD_MODULE_TYPE == BSP_LCD_BYD_4281L) //[david.modify] 2008-05-13 15:15
#define    LCD_XSIZE_TFT           (320)   
#define    LCD_YSIZE_TFT           (240)

#define    LCD_XSIZE_STN           (320)
#define    LCD_YSIZE_STN           (240)

#define    LCD_XSIZE_CSTN          (320)
#define    LCD_YSIZE_CSTN          (240)

#define     LCD_COLOR_DEPTH         (16)

#elif (LCD_MODULE_TYPE == BSP_LCD_YASSY_YF35F03CIB) //[david.modify] 2008-05-13 15:15
#define    LCD_XSIZE_TFT           (320)   
#define    LCD_YSIZE_TFT           (240)

#define    LCD_XSIZE_STN           (320)
#define    LCD_YSIZE_STN           (240)

#define    LCD_XSIZE_CSTN          (320)
#define    LCD_YSIZE_CSTN          (240)

#define     LCD_COLOR_DEPTH         (16)
#elif (LCD_MODULE_TYPE == BSP_LCD_SHARP_LQ035Q1) //[david.modify] 2008-05-13 15:15
#define    LCD_XSIZE_TFT           (320)   
#define    LCD_YSIZE_TFT           (240)

#define    LCD_XSIZE_STN           (320)
#define    LCD_YSIZE_STN           (240)

#define    LCD_XSIZE_CSTN          (320)
#define    LCD_YSIZE_CSTN          (240)

#define     LCD_COLOR_DEPTH         (16)
#elif (LCD_MODULE_TYPE == BSP_LCD_BYD_43INCH_480X272) || \
      (LCD_MODULE_TYPE == BSP_LCD_INNOLUX_43 )
#define    LCD_XSIZE_TFT           (480)   
#define    LCD_YSIZE_TFT           (272)

#define    LCD_XSIZE_STN           (480)
#define    LCD_YSIZE_STN           (272)

#define    LCD_XSIZE_CSTN          (480)
#define    LCD_YSIZE_CSTN          (272)

#define     LCD_COLOR_DEPTH         (16)
#elif (LCD_MODULE_TYPE == BSP_LCD_YASSY_43INCH_480X272) //[david.modify] 2008-05-13 15:15
#define    LCD_XSIZE_TFT           (480)   
#define    LCD_YSIZE_TFT           (272)

#define    LCD_XSIZE_STN           (480)
#define    LCD_YSIZE_STN           (272)

#define    LCD_XSIZE_CSTN          (480)
#define    LCD_YSIZE_CSTN          (272)

#define     LCD_COLOR_DEPTH         (16)

#elif (LCD_MODULE_TYPE == BSP_LCD_INNOLUX_35)
#define    LCD_XSIZE_TFT           (320)
#define    LCD_YSIZE_TFT           (240)

#define    LCD_SCR_XSIZE           (320)           // virtual screen  
#define    LCD_SCR_YSIZE           (240)

#define    LCD_SCR_XSIZE_TFT       (320)           // virtual screen  
#define    LCD_SCR_YSIZE_TFT       (240)

#define     LCD_COLOR_DEPTH         (16)
#endif


//------------------------------------------------------------------------------
//  Define: LCD_ARRAY_SIZE_XXX
//
//  Array Sizes based on screen configuration.
//

#define    LCD_ARRAY_SIZE_STN_1BIT     (LCD_SCR_XSIZE/8*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_STN_2BIT     (LCD_SCR_XSIZE/4*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_STN_4BIT     (LCD_SCR_XSIZE/2*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_CSTN_8BIT    (LCD_SCR_XSIZE/1*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_CSTN_12BIT   (LCD_SCR_XSIZE*2*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_TFT_8BIT     (LCD_SCR_XSIZE/1*LCD_SCR_YSIZE)
#define    LCD_ARRAY_SIZE_TFT_16BIT    (LCD_SCR_XSIZE*2*LCD_SCR_YSIZE)

//------------------------------------------------------------------------------
//  Define: LCD_HOZVAL_XXX
//
//  Desc...
//

#define    LCD_HOZVAL_STN          (LCD_XSIZE_STN/4-1)
#define    LCD_HOZVAL_CSTN         (LCD_XSIZE_CSTN*3/8-1)
#define    LCD_HOZVAL_TFT          (LCD_XSIZE_TFT-1)

//------------------------------------------------------------------------------
//  Define: LCD_LINEVAL_XXX
//
//  Desc...
//

#define    LCD_LINEVAL_STN         (LCD_YSIZE_STN-1)
#define    LCD_LINEVAL_CSTN        (LCD_YSIZE_CSTN-1)
#define    LCD_LINEVAL_TFT         (LCD_YSIZE_TFT-1)
#ifdef __cplusplus
}
#endif
#endif //#ifndef __DISPLAY_H__

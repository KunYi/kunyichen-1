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
//  File:  bsp_cfg.h
//
//  This file contains system constant specific for SMDK2450 board.
//
#ifndef __BSP_CFG_H
#define __BSP_CFG_H


//------------------------------------------------------------------------------
//
//  Define:  BSP_DEVICE_PREFIX
//
//  Prefix used to generate device name for bootload/KITL
//

#if (BSP_TYPE == BSP_SMDK2443)
#define BSP_DEVICE_PREFIX       "SMDK2443"        // Device name prefix
#elif (BSP_TYPE == BSP_SMDK2450)
#define BSP_DEVICE_PREFIX       "SMDK2450"        // Device name prefix
#endif
//------------------------------------------------------------------------------
// Board version
//------------------------------------------------------------------------------
// If you use CPU which is EVT0 version, define only EVT0.
// We recommend newer CPU board.
// Now, you define only in smdk2450.bat file. This definition is not need.
#if (BSP_TYPE == BSP_SMDK2443)
#define	EVT1		//#define EVT0
#elif (BSP_TYPE == BSP_SMDK2450)
#endif


//------------------------------------------------------------------------------
// System Tick define
//------------------------------------------------------------------------------
// There are two type system ticks. choose only one type.
// Fixed tick means that tick interrupt is occurred every 1ms.
// Variable tick means that timer interrupt period is changed when power mode is in idle.
#define FIXEDTICK	
//#define VARTICK
// Clock Preset value
#define PRESET_CLOCK
#if (BSP_TYPE == BSP_SMDK2443)
#define ARM_CLOCK 533
#elif (BSP_TYPE == BSP_SMDK2450)
//#define ARM_CLOCK 533
#define ARM_CLOCK 400133
//#define ARM_CLOCK 400
#endif
//------------------------------------------------------------------------------
// Board clock
//------------------------------------------------------------------------------
#define D1_2			0x0
#define D1_4			0x1
#define D1_8			0x2
#define D1_16			0x3
#define D2				2
#define D4				4
#define D8				8
#define D16				16

//#define S3C2450_FCLK           400000000           // 399.65MHz
#if (BSP_TYPE == BSP_SMDK2443)
#ifdef PRESET_CLOCK
#if (ARM_CLOCK==533)
#define S3C2450_FCLK           534000000           // 534.00MHz
#define S3C2450_HCLK           (S3C2450_FCLK/4)   // divisor 4
#define S3C2450_PCLK           (S3C2450_FCLK/8)   // divisor 2
#define S3C2450_SCLK           96000000
#define HCLKDIV					4
#elif (ARM_CLOCK==400)
#define S3C2450_FCLK           400000000           // 534.00MHz
#define S3C2450_HCLK           (S3C2450_FCLK/4)   // divisor 4
#define S3C2450_PCLK           (S3C2450_FCLK/8)   // divisor 2
#define S3C2450_SCLK           96000000
#define HCLKDIV					4
#elif (ARM_CLOCK==400133)
#define S3C2450_FCLK           400000000           // 400.00MHz
#define S3C2450_HCLK           (S3C2450_FCLK/3)   // divisor 3
#define S3C2450_PCLK           (S3C2450_FCLK/6)   // divisor 2
#define S3C2450_SCLK           96000000
#define HCLKDIV			3
#endif
#if (ARM_CLOCK == 36) // for FPGA 36:18:9
#define S3C2450_FCLK           36000000           // 36.00MHz
#define S3C2450_HCLK           (S3C2450_FCLK/2)   // 
#define S3C2450_PCLK           (S3C2450_FCLK/2)   // 
#define S3C2450_SCLK           96000000
#define HCLKDIV					0
#endif
#else //NoPreSet
#define S3C2450_BASE_REG_VA_CLOCK_POWER	0xB1000000
#define PLLVALUE			(((S3C2450_CLKPWR_REG*)(S3C2450_BASE_REG_VA_CLOCK_POWER))->MPLLCON)
#define CLKDIV				(((S3C2450_CLKPWR_REG*)(S3C2450_BASE_REG_VA_CLOCK_POWER))->CLKDIV0)

#define M_DIV ((PLLVALUE >> 16) & 0xff)
#define P_DIV ((PLLVALUE >> 8) & 0x3)
#define S_DIV ((PLLVALUE >> 0) & 0x1)

#define ARMDIVN ((CLKDIV >> 9) & 0xf)

#define ARMDIV				(ARMDIVN == 0 ?	1 : \
								(ARMDIVN == 8 ? 2 : \
									(ARMDIVN == 2 ? 3 : \
										(ARMDIVN == 9 ? 4 : \
											(ARMDIVN == 10 ? 6 : \
												(ARMDIVN == 11 ? 8 : \
													(ARMDIVN == 13 ? 12 : \
														(ARMDIVN == 15 ? 16 : 1) \
													) \
												) \
											) \
										) \
									) \
								) \
							)
									

#define PREDIV ((CLKDIV >> 4) & 0x3)
#define HCLKDIV				((CLKDIV>>0) & 0x3)
#define PCLKDIV				((CLKDIV>>2) & 0x1)
#define HALFHCLK			((CLKDIV>>3) & 0x1)

#define S3C2450_FOUT   		(2 * (M_DIV + 8L) * (12000000L / (P_DIV) / (1<<S_DIV )))
#define S3C2450_FCLK		(S3C2450_FOUT  / ARMDIV )
#define S3C2450_HCLK        (S3C2450_FOUT / (PREDIV+1) / (HCLKDIV+1))   // divisor 4
#define S3C2450_PCLK        (S3C2450_FOUT / (PCLKDIV+1))  // divisor 2


#define EPLLVALUE			(((S3C2450_CLKPWR_REG*)(S3C2450_BASE_REG_VA_CLOCK_POWER))->EPLLCON)
#define EPLL_M_DIV ((EPLLVALUE >> 16) & 0xff)
#define EPLL_P_DIV ((EPLLVALUE >> 8) & 0x3f)
#define EPLL_S_DIV ((EPLLVALUE >> 0) & 0x3)
#define S3C2450_SCLK   		((EPLL_M_DIV + 8L) * (12000000L / (EPLL_P_DIV+2) / (1<<EPLL_S_DIV )))

#endif //PRESET
#elif (BSP_TYPE == BSP_SMDK2450)
#ifdef PRESET_CLOCK
#if (ARM_CLOCK == 400133) 
#define S3C2450_FCLK           400000000           // 400.00MHz
#define S3C2450_HCLK           (S3C2450_FCLK/3)   // divisor 3
#define S3C2450_PCLK           (S3C2450_FCLK/6)   // divisor 2
#define S3C2450_SCLK           96000000
#define HCLKDIV				3
#elif (ARM_CLOCK==533)
#define S3C2450_FCLK           534000000           // 534.00MHz
#define S3C2450_HCLK           (S3C2450_FCLK/4)   // divisor 4
#define S3C2450_PCLK           (S3C2450_FCLK/8)   // divisor 2
#define S3C2450_SCLK           96000000
#define HCLKDIV					4
#elif (ARM_CLOCK==400)
#define S3C2450_FCLK           400000000           // 534.00MHz
#define S3C2450_HCLK           (S3C2450_FCLK/4)   // divisor 4
#define S3C2450_PCLK           (S3C2450_FCLK/8)   // divisor 2
#define S3C2450_SCLK           96000000
#define HCLKDIV					4
#elif (ARM_CLOCK==266)
#define S3C2450_FCLK           266000000           // 266.00MHz
#define S3C2450_HCLK           (S3C2450_FCLK/2)   // divisor 4
#define S3C2450_PCLK           (S3C2450_FCLK/4)   // divisor 2
#define S3C2450_SCLK           96000000
#define HCLKDIV					2
#endif
#else //NoPreSet
#define S3C2450_BASE_REG_VA_CLOCK_POWER	0xB1000000
#define PLLVALUE			(((S3C2450_CLKPWR_REG*)(S3C2450_BASE_REG_VA_CLOCK_POWER))->MPLLCON)
#define CLKDIV				(((S3C2450_CLKPWR_REG*)(S3C2450_BASE_REG_VA_CLOCK_POWER))->CLKDIV0)

#define M_DIV ((PLLVALUE >> 14) & 0x3ff)
#define P_DIV ((PLLVALUE >> 5) & 0x3F)
#define S_DIV ((PLLVALUE >> 0) & 0x7)

#define ARMDIVN 		((CLKDIV >> 9) & 0xf)
#define ARMDIV				(ARMDIVN == 0 ?	1 : \
								(ARMDIVN == 1 ? 2 : \
									(ARMDIVN == 2 ? 3 : \
										(ARMDIVN == 3 ? 4 : \
											(ARMDIVN == 5 ? 6 : \
												(ARMDIVN == 7 ? 8 : \
													(ARMDIVN == 11 ? 12 : \
														(ARMDIVN == 15 ? 16 : 1) \
													) \
												) \
											) \
										) \
									) \
								) \
							)
									

#define PREDIV 				((CLKDIV >> 4) & 0x3)
#define HCLKDIV				((CLKDIV>>0) & 0x3)
#define PCLKDIV				((CLKDIV>>2) & 0x1)
#define HALFHCLK			((CLKDIV>>3) & 0x1)

#define S3C2450_FOUT   		((M_DIV) * (12000000L / (P_DIV) / (1<<S_DIV )))
#define S3C2450_FCLK		(S3C2450_FOUT  / ARMDIV )
#define S3C2450_HCLK        (S3C2450_FOUT / (PREDIV+1) / (HCLKDIV+1))   // divisor 4
#define S3C2450_PCLK        (S3C2450_HCLK / (PCLKDIV+1))  // divisor 2


#define EPLLVALUE			(((S3C2450_CLKPWR_REG*)(S3C2450_BASE_REG_VA_CLOCK_POWER))->EPLLCON)
#define EPLL_M_DIV ((EPLLVALUE >> 16) & 0xff)
#define EPLL_P_DIV ((EPLLVALUE >> 8) & 0x3f)
#define EPLL_S_DIV ((EPLLVALUE >> 0) & 0x3)
#define S3C2450_SCLK   		((EPLL_M_DIV + 8L) * (12000000L / (EPLL_P_DIV+2) / (1<<EPLL_S_DIV )))

#endif //PRESET
#endif	


#define SYS_TIMER_DIVIDER	D2

#ifdef VARTICK
#define PRESCALER			((S3C2450_PCLK/(1000000*SYS_TIMER_DIVIDER)) - 1)
#define OEM_CLOCK_FREQ		(S3C2450_PCLK/(PRESCALER+1)/SYS_TIMER_DIVIDER)
// Timer count for 1 ms
#define OEM_COUNT_1MS       (OEM_CLOCK_FREQ / 1000)	// OEM_CLOCK_FREQ = 1000000, 1000000/1000 => 1000 = 1 msec
#define RESCHED_PERIOD      1		// 10				// Reschedule period in ms
#else	// FIXEDTICK
#if (S3C2450_PCLK==50000000)			// For find optimized Tick count value
#define PRESCALER			9
#else
#define PRESCALER			14
#endif
#define OEM_CLOCK_FREQ		(S3C2450_PCLK/(PRESCALER+1)/SYS_TIMER_DIVIDER)
// Timer count for 1 ms
#define OEM_COUNT_1MS       ((OEM_CLOCK_FREQ / 1000) - 1)	// OEM_CLOCK_FREQ = 1000000, 1000000/1000 => 1000 = 1 msec
#define RESCHED_PERIOD      1		// 10				// Reschedule period in ms
#endif

//------------------------------------------------------------------------------
// Debug UART1
//------------------------------------------------------------------------------
#define BSP_UART1_BAUDRATE	(115200)

#define BSP_UART1_ULCON         0x03                // 8 bits, 1 stop, no parity
#define BSP_UART1_UCON          0x0005              // pool mode, PCLK for UART
#define BSP_UART1_UFCON         0x00                // disable FIFO
#define BSP_UART1_UMCON         0x00                // disable auto flow control
#define BSP_UART1_UBRDIV        (S3C2450_PCLK/(BSP_UART1_BAUDRATE*16) - 1)

 //[david.modify] 2008-04-28 10:10
#define BSP_UART0_BAUDRATE	(115200)

#define BSP_UART0_ULCON         0x03                // 8 bits, 1 stop, no parity
#define BSP_UART0_UCON          0x0005              // pool mode, PCLK for UART
#define BSP_UART0_UFCON         0x00                // disable FIFO
#define BSP_UART0_UMCON         0x00                // disable auto flow control
#define BSP_UART0_UBRDIV        (S3C2450_PCLK/(BSP_UART1_BAUDRATE*16) - 1)
 //[david. end] 2008-04-28 10:10




//------------------------------------------------------------------------------
// Static SYSINTR Mapping for driver.
#define SYSINTR_OHCI            (SYSINTR_FIRMWARE+1)
#define SYSINTR_HSMMC            (SYSINTR_FIRMWARE+2)

 //[david.modify] 2008-05-16 11:04
 //============================
#define SYSINTR_KEYPAD			(SYSINTR_FIRMWARE+4)

// KEYBD 有三个键,对应如下;
//[david.modify] 2008-05-16 10:19
//GPG2	KEY6(up)		音量加EINT10 - down
//GPG1	KEY4 (OK)	确认键EINT9
//GPG0	KEY3 (down)	音量减EINT8 - up
//代表SCANCODE, 在ScanCodeToVKeyTable中映射成VKEY
#define VK_RETURN_SCANCODE 0x5a
#define VK_DOWN_SCANCODE 0x6a
#define VK_UP_SCANCODE 0x6c

#define EINT8_KEY VK_UP_SCANCODE //8			
#define EINT9_KEY VK_RETURN_SCANCODE //9
#define EINT10_KEY VK_DOWN_SCANCODE  //10
 //============================
// -----------------------------------------------------------------------------
// define For DVS, MAX1718 Preset Value
#define V800mV	800
#define V825mV	825
#define V850mV	850
#define V900mV	900
#define V925mV	925
#define V950mV	950
#define V1000mV	1000
#define V1050mV	1050
#define V1100mV	1100
#define V1150mV	1150
#define V1200mV	1200
#define V1250mV	1250
#define V1300mV	1300
#define V1350mV	1350
#define V1400mV	1400
#define V1450mV	1450
#define V1500mV	1500
#define V1550mV	1550
#define V1600mV	1600
#define V1650mV	1650
#define V1700mV	1700
#define V1750mV	1750

#define ARM_VDD		0x01
#define	INT_VDD		0x02
#define ARM_INT_VDD		(ARM_VDD | INT_VDD)


#ifdef DVS_EN					// using EPLL LCDCLOCK, calculate idletime percentage
											// 3 step Voltage level shifting			HIGH<->MID<->LOW
#define UNDERSHOOT_WORKAROUND		// if there is HW undershoot, define.

#define USESWPWSAVING	1
#define MVAL_USED		0
#if (BSP_TYPE == BSP_SMDK2443)
//	ARM voltage corner data, 07/01/03
//  MHZ		min		typ		max
//  67		0.95	1.0		1.05
//	133		1.05	1.1		1.15	
//	400		1.25	1.3		1.35
//	533		1.35	1.4		1.45  
#ifdef PRESET_CLOCK
#if (ARM_CLOCK==533)
#define HIGHVOLTAGE		V1300mV
#define HIGH_V_SET		{V1300mV, V1100mV}
#define MIDVOLTAGE		V925mV		// DVS Method 4,5 use this 
#define MID_V_SET			{V925mV, V950mV}
#define LOWVOLTAGE		V825mV		// DVS Method 5 use this additionally
#define LOW_V_SET			{V825mV, V850mV}
#elif (ARM_CLOCK==400133)
#define HIGHVOLTAGE		V1250mV
#define HIGH_V_SET		{V1250mV, V1250mV}
#define MIDVOLTAGE		V1050mV		// DVS Method 4,5 use this 
#define MID_V_SET			{V1050mV, V1050mV}
#define LOWVOLTAGE		V950mV		// DVS Method 5 use this additionally
#define LOW_V_SET			{V950mV, V950mV}
#elif (ARM_CLOCK==400)
#define HIGHVOLTAGE		V1200mV
#define HIGH_V_SET		{V1200mV, V1200mV}
#define MIDVOLTAGE		V1100mV 
#define MID_V_SET			{V1100mV, V1100mV}
#define LOWVOLTAGE		V1000mV
#define LOW_V_SET			{V1000mV, V1000mV}
#endif
#endif
#elif (BSP_TYPE == BSP_SMDK2450)
#ifdef PRESET_CLOCK
#if (ARM_CLOCK==533)
#define HIGHVOLTAGE		V1300mV
#define HIGH_V_SET		{V1300mV, V1300mV}
#define MIDVOLTAGE		V1100mV
#define MID_V_SET			{V1100mV, V1100mV}
#define LOWVOLTAGE		V1000mV
#define LOW_V_SET			{V1000mV, V1000mV}
#elif (ARM_CLOCK==400133)
#define HIGHVOLTAGE		V1200mV
#define HIGH_V_SET		{V1200mV, V1200mV}
#define MIDVOLTAGE		V1100mV 
#define MID_V_SET			{V1100mV, V1100mV}
#define LOWVOLTAGE		V1000mV
#define LOW_V_SET			{V1000mV, V1000mV}
 //[david.modify] 2008-05-27 12:18
#elif (ARM_CLOCK==400)
#define HIGHVOLTAGE		V1200mV
#define HIGH_V_SET		{V1200mV, V1200mV}
#define MIDVOLTAGE		V1100mV 
#define MID_V_SET			{V1100mV, V1100mV}
#define LOWVOLTAGE		V1000mV
#define LOW_V_SET			{V1000mV, V1000mV}
 //[david. end] 2008-05-27 12:19
#endif
#endif
#endif	//#if (BSP_TYPE == BSP_SMDK2443)

#define VOLTAGEDELAY	16000

#define Eval_Probe		1

#define DVSON		0x1
#define HCLKHALF	0x2
#define ACTIVE		0x4
#define DeepIdle	(DVSON|HCLKHALF)
#define NIdle		(DVSON)
#define LazyActive	(ACTIVE|DVSON|HCLKHALF)
#define SlowActive	(ACTIVE|DVSON)
#define Active		(ACTIVE)
#endif
//------------------------------------------------------------


#define LCD_MODULE_LTS222	1
#define LCD_MODULE_LTV350	2
#define LCD_MODULE_LTE480WV 3

//[david.modify] 2008-05-08 19:05

 #define DISPLAY_24BIT_MODE 0
//[david. end] 2008-05-08 19:05

//保持和smdk2416.bat一样
#define  BSP_LCD_BYD_4281L 100
#define  BSP_LCD_YASSY_YF35F03CIB 200
#define  BSP_LCD_YASSY_43INCH_480X272 201
#define  BSP_LCD_SHARP_LQ035Q1 300
#define  BSP_LCD_BYD_43INCH_480X272 400
#define  BSP_LCD_INNOLUX_35 500
#define  BSP_LCD_INNOLUX_43 501

#if (BSP_TYPE == BSP_SMDK2443)
#define LCD_MODULE_TYPE 	LCD_MODULE_LTV350	
#elif (BSP_TYPE == BSP_SMDK2450)
//#define LCD_MODULE_TYPE 	BSP_LCD_BYD_43INCH_480X272	
// 在SMDK2416\sources.cmn中有定义

#endif
#endif

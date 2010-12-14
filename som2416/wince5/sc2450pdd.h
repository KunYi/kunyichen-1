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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        SC4243PDD.H

Abstract:

       Samsung S3SC2450 USB Function Platform-Dependent Driver header.

--*/

#ifndef _SC2450PDD_H_
#define _SC2450PDD_H_

#include <windows.h>
#include <ceddk.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <devload.h>

#define RegOpenKey(hkey, lpsz, phk) \
        RegOpenKeyEx((hkey), (lpsz), 0, 0, (phk))

#ifndef SHIP_BUILD
#define STR_MODULE _T("SC2450UsbFn!")
#define SETFNAME() LPCTSTR pszFname = STR_MODULE _T(__FUNCTION__) _T(":")
#else
#define SETFNAME()
#endif

// 2450 Register Set
#define IR			0x00              //Index Register
#define EIR			0x04		//Endpoint Interrupt Register
#define EIER			0x08		//Endpoing Interrupt Enable Register
#define FARR               0x0C		//Function Address Register
#define FNR                 0x10		//Frame Number Register
#define EDR                 0x14		//Endpoint Direction Register OUT:0, IN:1
#define TR                   0x18		//Test Register
#define SSR                 0x1C		//System Status Register
#define SCR                 0x20		//System Control Register
#define EP0SR             0x24		//EP0 Status Register
#define EP0CR             0x28		//EP0 Control Register
#define ESR                 0x2C		//Endpoints Status Register
#define ECR                 0x30		//Endpoints Control Register
#define BRCR               0x34		//Byte Read Count Register
#define BWCR              0x38		//Byte Write Count Register
#define MPR                 0x3C		//Max Packet Register
#define DCR                 0x40		//DMA control Register
#define DTCR               0x44		//DMA Transfer Counter Register
#define DFCR               0x48		//DMA FIFO Counter Register
#define DTTCR1           0x4C		//DMA Total Transfer Counter1 Register
#define DTTCR2           0x50		//DMA Total Transfer Counter2 Register

#define EP0BR              0x60		//EP0 Buffer Register
#define EP1BR              0x64		//EP1 Buffer Register
#define EP2BR              0x68		//EP2 Buffer Register
#define EP3BR              0x6C		//EP3 Buffer Register
#define EP4BR              0x70		//EP4 Buffer Register
#define EP5BR              0x74		//EP5 Buffer Register
#define EP6BR              0x78		//EP6 Buffer Register
#define EP7BR              0x7C		//EP7 Buffer Register
#define EP8BR              0x80		//EP8 Buffer Register

#define MICR                0x84		//Master Interface Control Register
#define MBAR               0x88		//Memory Base Address Register
#define MCAR               0x8C		//Memory Current Address Register

#define FCON			 0x100	       //Burst FIFO-DMA Control
#define FSTAT		 0x104          //Burst FIFIO Status


// Register Base Address and Total Size
#define BASE_REGISTER_OFFSET        0x0
#define REGISTER_SET_SIZE           0x200

// System Status Register Bit
#define HFRES				(0x01 << 0)		//Host Forced Reset
#define HFSUSP				(0x01 << 1)		//Host Forced Suspend
#define HFRM					(0x01 << 2)		//Host Forced Resume 
#define SDE					(0x01 << 3)		//Speed Detection End
#define HSP					(0x01 << 4)		//Host Speed
#define DM					(0x01 << 5)		//DM Data Line State
#define DP					(0x01 << 6)		//DP Data Line State
#define TBM					(0x01 << 7)		//Toggle Bit Mismatch
#define VBUSON				(0x01 << 8)		//VBUS On
#define VBUSOFF				(0x01 << 9)		//VBUS Off
#define SSRINTERR			(0xff80)			//System Error Int to be checked (0xffc0 ?)

// System Control Register Bit
#define HRESE				(0x01 << 0)		//Reset Enable
#define HSUSPE				(0x01 << 1)		//Suspend Enable
#define MFRM				(0x01 << 2)		//Resume by MCU
#define IPS					(0x01 << 4)		//Interrupt Polarity Select
#define RRDE					(0x01 << 5)		//Reverse Read DAta Enable
#define SPDEN				(0x01 << 6)		//Speed Detect End Interrupt Enable
#define BIS					(0x01 << 7)		//Bus Interface Select
#define EIE					(0x01 << 8)		//Error Interrupt Enable
#define RWDE				(0x01 << 9)		//Reverse Wirte Data Enable
#define VBUSONEN			(0x01 << 10)		//VBUS On Enable
#define VBUSOFFEN			(0x01 << 11)		//VBUS Off Enable
#define DIEN					(0x01 << 12)		//Dual Interrupt Enable
#define DTZIEN				(0x01 << 14)		//DMA Total Counter Zero Interrupt Enable


// EP0 Status Register Bit
#define EP0RSR					(0x01 << 0)	//EP0 Rx Successfully Received
#define EP0TST					(0x01 << 1)	//EP0 Tx Successfully Received
#define EP0SHT					(0x01 << 4)	//EP0 Stall Handshake Transmitted
#define EP0LWO					(0x01 << 6)	//EP0 Last Word Odd

// EP0 Control Register Bit
#define EP0TZLS					(0x01 << 0)	//EP0 Tx Zero Length Set
#define EP0ESS					(0x01 << 1)	//EP0 Endpoint Stall Set
#define EP0TSS					(0x01 << 2)	//EP0 Tx Toggle Set
#define EP0TTE					(0x01 << 3)	//EP0 Tx Test Enable

// EP Status Register Bit
#define RPS					(0x01 << 0)		//Rx Packet Success
#define TPS					(0x01 << 1)		//Tx Packet Success
#define PSIF					(0x02 << 2)		//Packet Status in FIFO, 0x2 Two Packet in FIFO
#define LWO					(0x01 << 4)		//Last Word Odd
#define FSC					(0x01 << 5)		//Function Stall Condition
#define FFS					(0x01 << 6)		//FIFO Flushed
#define DOM					(0x01 << 7)		//Dual Operation Mode
#define SPT					(0x01 << 8)		//Short Packet Received
#define DTCZ					(0x01 << 9)		//DMA Total Count Zero
#define OSD					(0x01 << 10)		//Out Start DMA Operation
#define FPID					(0x01 << 11)		//First OUT Packet Interrupt Disable in OUT DMA Operation
#define FOVF					(0x01 << 14)		//FIFO Overflow
#define FUDR					(0x01 << 15)		//FIFO Underflow

// EP Control Register Bit
#define TZLS					(0x01 << 0)		//Tx Zero Length Set
#define ESS 					(0x01 << 1)		//Endpoint Stall Set
#define CDP					(0x01 << 2)		//Clear Data PID
#define TTS					(0x01 << 3)		//Tx Toggle Select
#define TTE					(0x01 << 5)		//Tx Toggle Enable
#define FLUSH				(0x01 << 6)		//FIFO Flush
#define DUEN					(0x01 << 7)		//Dual FIFO Mode Enable
#define IME					(0x01 << 8)		//ISO Mode Endpoint
#define TNPMF				(0x01 << 9)		// Transaction Number Per Micro Frame
#define OUTPKTHLD			(0x01 << 11)		//The MCU can control Rx FIFO Status through this bit
#define INPKTHLD			(0x01 << 12)		// The MCU can control Tx FIFO Status through this bit

// EP Interrupt Register Bit
#define EP0I					(0x01 << 0)
#define EP1I					(0x01 << 1)
#define EP2I					(0x01 << 2)
#define EP3I					(0x01 << 3)
#define EP4I					(0x01 << 4)
#define EP5I					(0x01 << 5)
#define EP6I					(0x01 << 6)
#define EP7I					(0x01 << 7)
#define EP8I					(0x01 << 8)

// EP Interrupt Enable Register Bit
#define EP0IE				(0x01 << 0)
#define EP1IE				(0x01 << 1)
#define EP2IE				(0x01 << 2)
#define EP3IE				(0x01 << 3)
#define EP4IE				(0x01 << 4)
#define EP5IE				(0x01 << 5)
#define EP6IE				(0x01 << 6)
#define EP7IE				(0x01 << 7)
#define EP8IE				(0x01 << 8)

#define CLEAR_ALL_EP_INTRS          (EP0I | EP1I | EP2I | EP3I | EP4I | EP5I | EP6I | EP7I | EP8I)

#define  EP_INTERRUPT_DISABLE_ALL   0x0   // Bits to write to EIER - Use CLEAR

// Bit Definitions for USB_INT_REG and USB_INT_EN_REG_OFFSET
#define USB_RESET_INTR              0x4
#define USB_RESUME_INTR             0x2
#define USB_SUSPEND_INTR            0x1

//Test Register

#define TR_TMD (1<<4)
#define TR_TPS (1<<3)
#define TR_TKS (1<<2)
#define TR_TJS (1<<1)
#define TR_TSNS (1<<0)


// DMA control register bit definitions
#define RUN_OB						0x80
#define STATE						0x70
#define DEMAND_MODE					0x8
#define OUT_DMA_RUN					0x4
#define IN_DMA_RUN					0x2
#define DMA_MODE_EN					0x1

//
#define REAL_PHYSICAL_ADDR_EP0_FIFO		(0x520001c0) //Endpoint 0 FIFO
#define REAL_PHYSICAL_ADDR_EP1_FIFO		(0x520001c4) //Endpoint 1 FIFO
#define REAL_PHYSICAL_ADDR_EP2_FIFO		(0x520001c8) //Endpoint 2 FIFO
#define REAL_PHYSICAL_ADDR_EP3_FIFO		(0x520001cc) //Endpoint 3 FIFO
#define REAL_PHYSICAL_ADDR_EP4_FIFO		(0x520001d0) //Endpoint 4 FIFO

#define DMA_BUFFER_BASE					0xAC000000
#define DMA_PHYSICAL_BASE				0x30000000
#define DRIVER_GLOBALS_PHYSICAL_MEMORY_START  (DMA_BUFFER_BASE + 0x10000)

// We poll for device detach at the following rate.
#define S3C2450_USB_POLL_RATE 1000

// For USB DMA
BOOL InitUsbdDriverGlobals(void);  	//:-)
void UsbdDeallocateVm(void);	   	//:-)
BOOL UsbdAllocateVm(void);	   		//:-)
void UsbdInitDma(int epnum, int bufIndex,int bufOffset);	//:-)


#endif //_SC2450PDD_H_



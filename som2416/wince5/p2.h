/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-2000 Microsoft Corporation.  All rights reserved.

Module Name:  

  p2.h

Abstract:  

  this file provides headers used for the p2 HAL routines

  
Functions:

  
Notes:


Revision History:

--*/

#ifndef _P2_H_
#define _P2_H_

#define READ_REGISTER_ULONG(reg) (*(volatile unsigned long * const)(reg))
#define WRITE_REGISTER_ULONG(reg, val) (*(volatile unsigned long * const)(reg)) = (val)
#define READ_REGISTER_USHORT(reg) (*(volatile unsigned short * const)(reg))
#define WRITE_REGISTER_USHORT(reg, val) (*(volatile unsigned short * const)(reg)) = (val)
#define READ_REGISTER_UCHAR(reg) (*(volatile unsigned char * const)(reg))
#define WRITE_REGISTER_UCHAR(reg, val) (*(volatile unsigned char * const)(reg)) = (val)


/* Defines for Reference Platform control registers */

#undef SYSTEM_ASIC_REGS_BASE
#define SYSTEM_ASIC_REGS_BASE	0xB0000000




#define CPU_BASE		(SYSTEM_ASIC_REGS_BASE + 0x00000800)
#define CPU_RR			0x00000008		/* Soft reset register (requires HW mods) */
#define CPU_MR			0x00000004		/* cpu mask register offset */
#define CPU_ISR			0x00000000  		/* cpu interrupt status register offset */

#define CPU_STATUS_BASE		(SYSTEM_ASIC_REGS_BASE + 0x400)


#define CPU_CSR			0x00
#define FLASH_BUSY		0x80
#define FLASH_POWER_DOWN	0x40
#define DISCHARGE_RECHARGE	0x20
#define AUX_POWER_ENABLE	0x10
#define ERROR_LED		0x08
#define BOOT_RESET		0x04
#define DOCKED			0x02
#define POWER_OFF_INTR		0x01

/* 
 * Interrupt bit positions are determined by the slot numbers of 
 * the cards in the system.  This information is in the VHDL file.
 */

#define SYSTEM_INTR		0x00000001  /* PCMCIA or On/Off switch */
#define DEBUG_SER_INTR		0x00000004  /* it is not really used */
#define PRODUCT_SER_INTR	0x00000008
#define IR_INTR			0x00000080
#define ETHER_INTR		0x00000100
#define KEYB_INTR		0x00000040
#define TOUCH_AUDIO_ADC_INTR	0x00000020


/*
 * The "SLOTs" correspond to "Master" fields in the VHDL file
 * They have to be renamed to avoid the confusion.
 */
#define DEBUG_SER_SLOT      0 
#define DISP_SLOT           1	
#define IR_SLOT             3
#define PRODUCT_SER_SLOT    4
#define TOUCH_SOUND_SLOT	   2


/* 
 * Display defines.
 * The address for the display DMA locations on the P2 are the same as the PeRP.
 */

#define DISP_BASE   		(SYSTEM_ASIC_REGS_BASE + 0x1000)
#define DISP_CSR		0x0004		/* offset from display base reg */
#define DISP_XSIZE		0x0008		/* offset from display base reg */
#define DISP_YSIZE		0x000C		/* offset from display base reg */
#define DISP_WR_DMA 		(CPU_BASE + (DISP_SLOT * 2 * 0x10000) + 0x10000)	
#define DISP_DMA_LOW		0x0010
#define DISP_DMA_HIGH		0x0014
#define DISP_CSR		0x0004
#define LCD_BIAS_ON		0x0002
#define LCD_ON			0x0004
#define LCD_DISPLAY_ENABLE	0x0008

// This buff is now offset via a constant
//#define DISP_DMA_BASE		(DMA_BUFFER_BASE + 0x00100000)  // S3c2400x01
//#define DISP_DMA_PHYS		(DMA_PHYSICAL_BASE + 0x00100000)

/* 
 * Serial and IR defines
 */

#define PRODUCT_SER_BASE	(SYSTEM_ASIC_REGS_BASE + 0x4000)		
#define DEBUG_SER_BASE		(SYSTEM_ASIC_REGS_BASE + 0x2000)		
#define IR_BASE		        (SYSTEM_ASIC_REGS_BASE + 0x8000)		

#define SER_CSR_A		0x0000		/* offset from serial base reg */
#define SER_CSR_B		0x0004		/* offset from serial base reg */

#define DEBUG_SER_RX_DMA_LOW    (CPU_BASE + \
				 (DEBUG_SER_SLOT * 2 * 0x10000 + 0x0010))
#define DEBUG_SER_RX_DMA_HIGH   (CPU_BASE + \
				 (DEBUG_SER_SLOT * 2 * 0x10000 + 0x0014))
#define DEBUG_SER_TX_DMA_LOW    (CPU_BASE + \
				 (DEBUG_SER_SLOT * 2 * 0x10000 + 0x10010))
#define DEBUG_SER_TX_DMA_HIGH   (CPU_BASE + \
				 (DEBUG_SER_SLOT * 2 * 0x10000 + 0x10014))

#define PRODUCT_SER_RX_DMA_LOW    (CPU_BASE + \
				 (PRODUCT_SER_SLOT * 2 * 0x10000 + 0x0010))
#define PRODUCT_SER_RX_DMA_HIGH   (CPU_BASE + \
				 (PRODUCT_SER_SLOT * 2 * 0x10000 + 0x0014))
#define PRODUCT_SER_TX_DMA_LOW    (CPU_BASE + \
				 (PRODUCT_SER_SLOT * 2 * 0x10000 + 0x10010))
#define PRODUCT_SER_TX_DMA_HIGH   (CPU_BASE + \
				 (PRODUCT_SER_SLOT * 2 * 0x10000 + 0x10014))


#define IR_RX_DMA_LOW    (CPU_BASE + \
				 (IR_SLOT * 2 * 0x10000 + 0x0010))
#define IR_RX_DMA_HIGH   (CPU_BASE + \
				 (IR_SLOT * 2 * 0x10000 + 0x0014))
#define IR_TX_DMA_LOW    (CPU_BASE + \
				 (IR_SLOT * 2 * 0x10000 + 0x10010))
#define IR_TX_DMA_HIGH   (CPU_BASE + \
				 (IR_SLOT * 2 * 0x10000 + 0x10014))

// serialCsrA's fields:
#define SERA_RX_CHARACTER_INTR	 0x8000
#define SERA_RX_INTR 	    	 0x4000
#define SERA_RX_END_INTR     	 0x2000
#define SERA_RX_FRAME_ERROR_INTR 0x1000
#define SERA_RX_CHANGED_INTR 	 0x0400
#define SERA_RX_OVERRUN_INTR     0x0200
#define SERA_RI                  0x0080
#define SERA_DSR                 0x0040
#define SERA_TX_FULL             0x0020
#define SERA_TX_INTR		 0x0010
#define SERA_TX_END_INTR	 0x0008
#define SERA_CTS                 0x0004
#define SERA_CD                  0x0002
#define SERA_SERIAL_ON           0x0001


#define SERA_INTR_MASK  (SERA_RX_CHARACTER_INTR | SERA_RX_INTR | \
			 SERA_RX_END_INTR | SERA_RX_FRAME_ERROR_INTR | \
			 SERA_RX_CHANGED_INTR | SERA_RX_OVERRUN_INTR | \
			 SERA_TX_INTR | SERA_TX_END_INTR)

// serialCsrB's fields:
#define SERB_SERIAL_POWER 	  0x8000
#define SERB_RX_EN    	          0x4000
#define SERB_TX_EN		  0x2000
#define SERB_TX_STOP_AT_PAGE      0x1000
#define SERB_CTS_FLOW	          0x0800
#define SERB_NOT_SERIAL_RESET     0x0400
#define SERB_RTS                  0x0200
#define SERB_DTR                  0x0100
#define SERB_BREAK                0x0080
#define SERB_STOP_BITS            0x0040
#define SERB_BIT_FORMAT_MASK      0x0030
#define SERB_BAUD_RATE_MASK       0x000f

// The following values are stored in SERB_BAUD_RATE_MASK
#define SER_BAUD_9600   11 
#define SER_BAUD_14400   7 
#define SER_BAUD_19200   5 
#define SER_BAUD_38400   2 
#define SER_BAUD_57600   1
#define SER_BAUD_115200  0

// The following values are stored in SERB_BIT_FORMAT_MASK. 
#define SER_BIT_FORMAT_6 0x20
#define SER_BIT_FORMAT_7 0x10
#define SER_BIT_FORMAT_8 0x00
  
/*
 * Keyboard defines 
 */

#define KB_BASE			(SYSTEM_ASIC_REGS_BASE + 0xc000)		
#define KB_ISR			0x0004		/* offset from keyboard base reg */
#define KB_CSR			0x0000		/* offset from keyboard base reg */
#define KB_INTR_MASK     	0x0001
#define KB_RDRF			0x0001
#define KB_PARITY		0x0100			
#define KB_DATA_RD		0x0200
#define KB_CLK_RD		0x0400
#define KB_DATA_DR		0x0800
#define KB_CLK_DR		0x1000
#define KB_DATA_DR_EN		0x2000
#define KB_CLK_DR_EN		0x4000
#define KB_CLK_EN		0x8000

/* 
 * Touch and Sound defines
 */

// Only define _ONE_ of the following.  If changing Touch/Audio hardware, you
// also _MUST_ make a change in ...\platform\p2\kernel\hal\sh3\fwp2.src
#define TOUCH_AUDIO_CRYSTAL 1
//#define TOUCH_AUDIO_UCB1100 1

#define TOUCH_SOUND_BASE		(SYSTEM_ASIC_REGS_BASE + 0xA000)	
#define IO_ADCCNTR			0x0000
#define IO_ADCSTR			0x0004
#define UCBCNTR				0x0008
#define UCBSTR				0x000C
#define UCBREGISTER			0x0010
#define IO_SOUNDCNTR		0x0014
#define IO_SOUNDSTR			0x0018
#define INTR_MASK			0x001C

/*
// Make sure this matches entry in config.bib
// These buffs are now offset via a constant
#define TOUCHPANEL_PENSAMPLES_BASE     (DMA_BUFFER_BASE + 0x00020000) 
#define TOUCHPANEL_PENSAMPLES_PHYS     (DMA_PHYSICAL_BASE + 0x00020000) 
#define AUDIO_DMA_BUFFER_BASE			  (DMA_BUFFER_BASE + 0x00002000)
#define AUDIO_DMA_BUFFER_PHYS			  (DMA_PHYSICAL_BASE + 0x00002000)

#define AUDIO_DMA_REG_BASE		(CPU_BASE + \
					(TOUCH_SOUND_SLOT * 2 * 0x10000))
#define IO_RECORD_PTR_LOW		(AUDIO_DMA_REG_BASE + 0x0010)
#define IO_RECORD_PTR_HIGH		(AUDIO_DMA_REG_BASE + 0x0014)
#define IO_PLAYBACK_PTR_LOW		(AUDIO_DMA_REG_BASE + 0x10010)
#define IO_PLAYBACK_PTR_HIGH		(AUDIO_DMA_REG_BASE + 0x10014)
*/

// 
// The following defs were only defined in the kernel\hal\shx\fwp2.src file for the SH3
// need to sync the tchaud.h and reg.h files in the
// touch screen and audio driver dirs.
//
#define soundIntrMask			0x0002		// mask for all 4 sound interrupts
#define playbackIntr			0x2000
#define playbackEndIntr			0x1000
#define recordIntr				0x8000
#define recordEndIntr			0x4000
#define soundIntr				playbackIntr | playbackEndIntr | recordIntr | recordEndIntr

// Touch panel sample area
//
// Note: It's important that buffer A and B are contiguous in memory and
//       16 bytes apart.

#define tchBufA				0x0000		// Sample buffer A
#define tchBufB				0x0010		// Sample buffer B
#define tchHalPointer		0x0020		// HAL's sample pointer
#define tchDevDrvPointer	0x0024		// Device driver's sample pointer
#define tchSemaphore		0x0028		// Touch/Audio sync semaphore
#define tchStatus			0x002c		// Status used for passing info from HAL to touch driver
#define tchCoordCount		0x0030 		// Coordinate count - used in touch HAL code
#define tchTimerState		0x0034		// Used for the timer handler state machine.

// Touch panel bit masks
#define ucbIntr				0x0008
#define ucbIntrMask			0x0008
#define penTimingIntr		0x0004
#define penTimingIntrMask	0x0004
#define penIntr				0x0010
#define penIntrMask			0x0010
#define regIntr				0x0001
#define regIntrMask			0x0001
#define penState			0x1000
#define penTimingEn			0x0400

// Touch panel status register value
#define TOUCH_PEN_DOWN		 0
#define TOUCH_PEN_UP		 1
#define TOUCH_PEN_SAMPLE 	 2

// Timer state machine values
#define TCH_DO_BIAS		 	0
#define TCH_DO_SAMPLE		1

#ifdef TOUCH_AUDIO_UCB1100
#define TOUCH_SAMPLE_VALID		 0x03ff		// Bits which are valid in a touch panel sample
#define TOUCH_MAX_COORD			 12			// Max coord count = max coords * 2 = 6 * 2 = 12
#endif //TOUCH_AUDIO_UCB1100

#ifdef  TOUCH_AUDIO_CRYSTAL
#define TOUCH_SAMPLE_VALID		 0x0fff		// Bits which are valid in a touch panel sample
#define TOUCH_MAX_COORD		 	16			// Max coord count = max coords * 2 = 8 * 2 = 16
#endif //TOUCH_AUDIO_CRYSTAL

#define TOUCH_CHG_BUF_MASK		 0x10		// Mask XOR'd with a buf address to get other buf address
#define TOUCH_X_REQ			 0x4400		// Request an X coordinate
									// This enables doSample, sets adcSel to 00 = get an X coord
									// and leaves the timer enabled
#define TOUCH_Y_REQ			 0x4c00		// Request a Y coordinate
									// This enables doSample, sets adcSel to 01 = get a Y coord
									// and leaves the timer enabled



/*
 * PCMCIA defines
 */

/*
 * PCMCIA Control register
 */
#define PCMCIA_ENABLE			0x40
#define PCMCIA_RESET			0x20
#define PCMCIA_INTR_MASK		0x10
#define PCMCIA_STATE_INTR_MASK	0x08
#define PCMCIA_MEM_WINDOW		0x07

/*
 * PCMCIA Interrupt register
 */
#define PCMCIA_WP			0x10
#define PCMCIA_CD2			0x08
#define PCMCIA_CD1			0x04
#define PCMCIA_INTR			0x02
#define PCMCIA_STATE_INTR		0x01

/*
 * PCMCIA Status register
 */
#define PCMCIA0_BVD1			0x8000
#define PCMCIA0_BVD2			0x4000
#define PCMCIA1_BVD1			0x2000
#define PCMCIA1_BVD2			0x1000
#define PCMCIA0_VSW1			0x0800
#define PCMCIA1_VSW1			0x0400
#define PCMCIA0_VSW2			0x0200
#define PCMCIA1_VSW2			0x0100

/*
 * PCMCIA Register addresses
 */
#define PCMCIA_REG0			    0x10	
#define PCMCIA_INTR_REG0		0x14	
#define PCMCIA_REG1			    0x18	
#define PCMCIA_INTR_REG1		0x1C	

#define PCMCIA_STATUS_REG		0x00	// Offset from PCMCIA_STATUS_BASE		
#define PCMCIA_CTL_REG			0x00    // Offset from PCMCIA_STATUS_BASE	





/*
 *  Bus State Controller Defines
 */

#define BCN_BCR1		0xFFFFFF60
#define BCN_BCR2		0xFFFFFF62

#define BCN_BCR1_DRAM_A2N3D	0x0010	// Area 2 normal, Area 3 DRAM
#define BCN_BCR1_A5PCM		0x0002	// Area 5 is PCMCIA access		
#define BCN_BCR1_A6PCM		0x0001	// Area 6 is PCMCIA access		

#define BCN_BCR2_A6SZ_16 	0x2000 
#define BCN_BCR2_A5SZ_8 	0x0400
#define BCN_BCR2_A4SZ_32	0x0300 
#define BCN_BCR2_A3SZ_32 	0x00C0
#define BCN_BCR2_A2SZ_32  	0x0030
#define BCN_BCR2_A2SZ_16  	0x0020  // Reserved Area on SH3, 16-bit bus
#define BCN_BCR2_A1SZ_32	0x000C

/*
 * Defines for SMC ethernet board. 
 */
#define ETHERNET_BASE  (SYSTEM_ASIC_REGS_BASE + 0x3000)

// The low bit of this word is stored in the FPGA. If set to a 1, it will assert
// the RESET pin of the 91C94.
#define SMC_HARD_RESET_REG (ETHERNET_BASE + 32)



#endif // _P2_H_ 

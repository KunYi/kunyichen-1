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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------

//
// ac97.h 
// 
// definitions for the AC97 Controller and AC97 codec - Sigmatel's STAC9766
//

#ifndef _AC97_H_
#define _AC97_H_

// For audio controller
// 
#define AC97_INTERNAL_CLOCK_ENABLE		(1<<20)					// Enable CPU clock to AC97 controller

#define AC97_PCMDATA_FIFO_LENGTH	(16)

// AC97 Global Control register
//
#define 	AC97_CODEC_READY_ENABLE			(1<<22)
#define 	AC97_PCM_OUT_UNDERRUN_ENABLE	(1<<21)
#define 	AC97_PCM_IN_OVERRUN_ENABLE		(1<<20)
#define 	AC97_MIC_IN_OVERRUN_ENABLE		(1<<19)
#define 	AC97_PCM_IN_THRESHOLD_ENABLE	(1<<18)
#define 	AC97_PCM_OUT_THRESHOLD_ENABLE	(1<<17)
#define 	AC97_MIC_IN_THRESHOLD_ENABLE	(1<<16)

#define 	AC97_PCM_OUT_DMA_MODE			(2<<12)
#define 	AC97_PCM_OUT_PIO_MODE			(1<<12)
#define 	AC97_PCM_IN_DMA_MODE			(2<<10)
#define 	AC97_PCM_IN_PIO_MODE			(1<<10)
#define 	AC97_MIC_IN_DMA_MODE			(1<<9)
#define 	AC97_MIC_IN_PIO_MODE			(1<<8)

#define 	AC97_ACLINK_ENABLE			(1<<3)
#define 	AC97_ACLINK_ON				(1<<2)

#define 	AC97_WARM_RESET				(1<<1)
#define 	AC97_COLD_RESET				(1<<0)

// AC97 Codec Command and Status Register
//
#define		AC97_READ_COMMAND			(1<<23)
#define		AC97_CMD_ADDR_SHIFT			(16)
#define		AC97_CMD_DATA_SHIFT			(0)
#define		AC97_STAT_DATA_READ_MASK			(0xFFFF << 0)
#define		AC97_STAT_ADDR_READ_MASK			(0x7F << 16)

// Defines for audio controller end here


// Defines for the STAC9766 registers
//
#define 	AC97_RESET				0x0000      	// Reset - default:6990h
#define 	AC97_MASTER_VOL_STEREO	0x0002      	// LINE_OUT Master Volume - 8000h
#define 	AC97_HEADPHONE_VOL		0x0004      	// HP_OUT Master Volume - 8000h
#define 	AC97_MASTER_VOL_MONO	0x0006      	// Mono Volume - 8000h
// Register 8 not supported 
#define		AC97_PCBEEP_VOL			0x000a      	// PC BEEP - 0000h
#define 	AC97_PHONE_VOL			0x000c      	// Phone Volume - 8008h
#define 	AC97_MIC_VOL			0x000e      	// MIC Input (mono) - 8008h
#define 	AC97_LINEIN_VOL			0x0010      	// Line Input (stereo) - 8808h
#define 	AC97_CD_VOL				0x0012      	// CD Input (stereo) - 8808h
#define 	AC97_VIDEO_VOL			0x0014      	// Video Input - 8808h
#define 	AC97_AUX_VOL			0x0016      	// Aux Input (stereo) - 8808h
#define 	AC97_PCMOUT_VOL			0x0018      	// PCM Out Output (stereo) - 8808h
#define 	AC97_RECORD_SELECT		0x001a      	// Record Select - 0000h
#define 	AC97_RECORD_GAIN		0x001c		// Record Gain - 8000h
// 1e unsupported
#define 	AC97_GENERAL_PURPOSE	0x0020		// General Purpose - 0000h
#define 	AC97_3D_CONTROL			0x0022		// 3D Control - 0000h
#define 	AC97_INTR_PAGE			0x0024		// Audio Interrupt and Paging
#define 	AC97_POWER_CONTROL		0x0026		// Powerdown Control/Stat - 000Fh

// registers 0x0028 - 0x0058 are reserved
//
#define		AC97_EXT_AUDIO_ID		0x0028		// Extended Audio ID - 0205h
#define 	AC97_EXT_AUDIO_CONTROL	0x002A		// Extended Audio Control - 0400h
#define 	AC97_PCM_DAC_RATE		0x002C		// PCM DAC Rate - BB80h
// 30 unsupported
#define 	AC97_PCM_ADC_RATE		0x0032		// PCM ADC RATE - BB80h
// 34, 36, 38 unsupported
#define 	AC97_SPDIF_CONTROL		0x003A		// SPDIF Control - 2A00h


// registers 0x005a - 0x007a are vendor reserved
//
#define  	AC97_DIG_AUDIO_CONTROL	0x006A		// Digital Audio Control - 0000h
#define  	AC97_REV_CODE			0x006C		// Revision Code - 0000h
#define  	AC97_ANALOG_SPEC		0x006E		// Analog Special - 0000h
#define  	AC97_72H_ENABLE			0x0070		// 72h Enable - 0000h
#define  	AC97_ANALOG_CURR_ADJUST	0x0072		// Analog Current Adjust - 0000h
#define  	AC97_GPIO_CURR_ACCESS	0x0074		// GPIO Current Access - 0000h
#define  	AC97_78H_ENABLE			0x0076		// 78h Enable - 0000h
#define  	AC97_HPF_BYPASS			0x0078		// High Pass Filter Bypass
#define  	AC97_VENDOR_ID1			0x007C		// Vendor ID1 - 8384h
#define  	AC97_VENDOR_ID2			0x007E		// Vendor ID2 - 7666h

// AC97 codec vendor - Sigmatel
//
#define 	AC97_VENDOR_SIGMATEL	0x83847666	// SigmaTel

// Power Control register 3Ch, 3Eh define

// volume control bit defines 

#define 	AC97_MUTE					0x8000		// Mute - Line Out, HP, MONO
#define 	AC97_MAX_ATT_LEFT		0x1f00		// Left Channel - Line Out, HP, Line In, CD, Video, AUX, PCM Out
#define 	AC97_MAX_ATT_RIGHT		0x001f		// Right Channel - Line Out, HP, MONO, Phone, MIC, Line In, CD, Video, AUX, PCM Out

#define	AC97_VOL_LEFT				0x2020		// Optional bits D13, D5 set (Line, HP)- Attenuation = 46.5 dB 
#define 	AC97_VOL_RIGHT			0x2020		// Optional bits D13, D5 set (Line HP, Mono)- Attenuation = 46.5 dB

#define 	AC97_VOL_PCBEEP			0x001E		// Max Attenuation - PC BEEP - Pin grounded

#define 	AC97_MICBOOST           		0x0040		// Boost Enable - Also check 6E register- TBD

// Record Select defines - 1Ah
//
#define 	AC97_RECMUX_MIC         	 	0x0000		// MIC Select
#define 	AC97_RECMUX_CD         		0x0101		// CD In -  Pins grounded
#define 	AC97_RECMUX_VIDEO        	0x0202      // Video in - Pins grounded																																							
#define 	AC97_RECMUX_AUX         		0x0303		// AUX In - PinS grounded
#define 	AC97_RECMUX_LINE        		0x0404		// Line In
#define 	AC97_RECMUX_STEREO_MIX  	0x0505		// Stereo Mix
#define 	AC97_RECMUX_MONO_MIX    	0x0606		// Mono Mix
#define 	AC97_RECMUX_PHONE       	0x0707		// Phone - Pin grounded

// Record Gain - 1Ch
//
#define 	AC97_RECORD_GAIN_VAL		0x0F0F		// Record Gain L & R


// general purpose register bit defines - 20h
// 
#define		AC97_GP_LPBK           0x0080		// Loopback mode
#define		AC97_GP_MS              0x0100		// Mic Select 0=Mic1, 1=Mic2
#define		AC97_GP_MIX            	0x0200		// Mono output select 0=Mix, 1=Mic
#define		AC97_GP_3D              0x2000		// 3D Enhancement 1=on
#define		AC97_GP_POP            0x8000		// DAC Bypasses filter and connects to Line Out


// 3D Control defines - 22h
//
#define		AC97_SS3D_SEP_OFF		0x0000		// 3D Separation OFF
#define		AC97_SS3D_SEP_LOW		0x0004		// LOW
#define		AC97_SS3D_SEP_MED		0x0008		// MEDIUM
#define		AC97_SS3D_SEP_HIGH		0x000C		// HIGH

// powerdown control and status bit defines - 26h
//
#define		AC97_PWR_EAPD			0x8000		// External Amp Power Down
#define		AC97_PWR_REF			0x0008		// Vref nominal
#define		AC97_PWR_ANL			0x0004		// Analog section ready
#define		AC97_PWR_DAC			0x0002		// DAC section ready
#define		AC97_PWR_ADC			0x0001		// ADC section ready

// control
#define		AC97_PWR_PR0            0x0100      // ADC and Mux powerdown
#define		AC97_PWR_PR1            0x0200      // DAC powerdown
#define		AC97_PWR_PR2            0x0400      // Analog Mixer Powerdown (Vref on)
#define		AC97_PWR_PR3            0x0800      // Output Mixer Powerdown (Vref off)
#define		AC97_PWR_PR4            0x1000      // AC-link Powerdown
#define		AC97_PWR_PR5            0x2000      // Internal Clk Disable
#define		AC97_PWR_PR6            0x4000      // Aux Out Powerdown


// useful power states
#define		AC97_PWR_D0             0x0000      // everything on
#define		AC97_PWR_D1             AC97_PWR_PR0|AC97_PWR_PR1|AC97_PWR_PR4
#define		AC97_PWR_D2             AC97_PWR_PR0|AC97_PWR_PR1|AC97_PWR_PR2|AC97_PWR_PR3|AC97_PWR_PR4
#define		AC97_PWR_D3             AC97_PWR_PR0|AC97_PWR_PR1|AC97_PWR_PR2|AC97_PWR_PR3|AC97_PWR_PR4
#define		AC97_PWR_ANLOFF         AC97_PWR_PR2|AC97_PWR_PR3  //analog section off

//Extended Audio ID Defines - 28h 
//The CID0 and CID1 are HIGH and so the DSA bits will reset to 10 which corresponds to left slot 6 and right slot 9


// Extended Audio Control/Status defines - 2Ah
//
#define		AC97_ENABLE_VRA			0x0001		// variable Rate Sampling Enabled
#define		AC97_ENABLE_SPDIF		0x0004		// SPDIF Enabled

// PCM DAC rate Registers defines - 2Ch and 32h
//
#define		AC97_PCM_DAC_RATE_8KHZ	0x1F40		// 8 kHz Sample Rate
#define		AC97_PCM_DAC_RATE_11KHZ	0x2B11		// 11.025 kHz
#define		AC97_PCM_DAC_RATE_16KHZ	0x3E80		// 16 kHz
#define		AC97_PCM_DAC_RATE_22KHZ	0x5622		// 22.05 kHz
#define		AC97_PCM_DAC_RATE_32KHZ	0x7D00		// 32 kHz
#define		AC97_PCM_DAC_RATE_44KHZ	0xAC44		// 44.1 kHz
#define		AC97_PCM_DAC_RATE_48KHZ	0xBB80		// 48 kHz


// SPDIF Control defines - 3Ah Pin 48 is HIGH and so the register 3Ah will read 0000h
//

// Revision Code defines - 6Ch
//
#define		AC97_REVISION_CODE		0x0000		// Revision Code


// Analog Special defines - 6Eh
//
#define		AC97_ALL_MIX			0x1000		// Controls the record source when stereo option is selected
// #define		AC97_   - ADC slots TBD
#define		AC97_MIC_BOOST_20DB		0x0000		// Selected with bit D6 in 0Eh
#define		AC97_MIC_BOOST_30DB		0x0004		// Selected with bit D6 in 0Eh

//
#define		AC97_READLOCKED_REG		0xABBA		// Value to be written for reading/writing locked registers

// Analog Current Adjust defines - 72h and 74h
//
#define		AC97_CURR_NORMAL		0x0000		// Normal Current
#define		AC97_CURR_80_NOMINAL	0x0002		// 80% of than Nominal Current
#define		AC97_CURR_120_NOMINAL	0x0004		// 120% of than Nominal Current
#define		AC97_CURR_140_NOMINAL	0x0006		// 140% of than Nominal Current
#define		AC97_ANTIPOP			0x0080		// Anti-pop 

// GPIO Access register defines - 74h - pins not being used.
//

// High Pass Filter Bypass defines - 76h and 78h
//
#define		AC97_HIPASS_DISABLE		0x0001		// High Pass filtering Disabled

//Vendor ID1 and ID2 defines - 7Ch an 7Eh
//
#define 	AC97_VENDOR_ID1_VAL		0x8384		// Sigmatel
#define 	AC97_VENDOR_ID2_VAL		0x7666		// SigmaTel

#endif // _AC97_H_

#ifndef __AC97_H
#define __AC97_H

#if __cplusplus
    extern "C" 
    {
#endif

//------------------------------------------------------------------------------
//  Type: S3C2450_AC97_REG    
//
//  AC97 control registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_AC97 in the configuration file cpu_base_reg_cfg.h.
//


#define AC_PCMDATA_PHYS					(0x5B000018)	// For PCM In and Out - Stereo
#define AC_MICDATA_PHYS					(0x5B00001C)	// For MIC In - Mono


#if __cplusplus
    }
#endif

#endif 
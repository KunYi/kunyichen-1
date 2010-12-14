#ifndef __RTCODECCOMM_H__
#define __RTCODECCOMM_H__

#include "wavemain.h"
//Index of Codec Register definition

#define RT_RESET						0X00			//RESET CODEC TO DEFAULT
#define RT_SPEAKER_OUT_VOL				0X02			//SPEAKER OUT VOLUME
#define RT_HP_OUT_VOL					0X04			//HEADPHONE OUTPUT VOLUME
#define RT_PHONEIN_MONO_OUT_VOL			0X08			//PHONE INPUT/MONO OUTPUT VOLUME
#define RT_LINE_IN_VOL					0X0A			//LINE IN VOLUME
#define RT_STEREO_DAC_VOL				0X0C			//STEREO DAC VOLUME
#define RT_MIC_VOL						0X0E			//MICROPHONE VOLUME
#define RT_MIC_ROUTING_CTRL				0X10			//MIC ROUTING CONTROL
#define RT_ADC_RECORD_GAIN				0X12			//ADC RECORD GAIN
#define RT_ADC_RECORD_MIXER				0X14			//ADC RECORD MIXER CONTROL
#define RT_VOICE_DAC_OUT_VOL			0X18			//VOICE DAC OUTPUT VOLUME
#define RT_OUTPUT_MIXER_CTRL			0X1C			//OUTPUT MIXER CONTROL
#define RT_MICROPHONE_CTRL				0X22			//MICROPHONE CONTROL
#define RT_POWERDOWN_CTRL_STAT			0X26			//POWER DOWN CONTROL/STATUS
#define RT_TONE_CTRL					0X2A			//TONE CONTROL
#define RT_STEREO_DAC_RATE				0X2C			//AC97 STEREO DAC RATE, DPE RATE
#define RT_STEREO_ADC_RATE				0X32			//AC97 STEREO ADC RATE, DPE RATE
#define	RT_MAIN_SDP_CTRL				0X34			//MAIN SERIAL DATA PORT CONTROL(STEREO I2S)
#define RT_EXTEND_SDP_CTRL				0X36			//EXTEND SERIAL DATA PORT CONTROL(VOICE I2S/PCM)
#define	RT_AC97_CTRL_REG				0X38			//AC97 CONTROL REGISTER
#define	RT_PWR_MANAG_ADD1				0X3A			//POWER MANAGMENT ADDITION 1
#define RT_PWR_MANAG_ADD2				0X3C			//POWER MANAGMENT ADDITION 2
#define RT_PWR_MANAG_ADD3				0X3E			//POWER MANAGMENT ADDITION 3
#define RT_GEN_CTRL_REG1				0X40			//GENERAL PURPOSE CONTROL REGISTER 1
#define	RT_GEN_CTRL_REG2				0X42			//GENERAL PURPOSE CONTROL REGISTER 2
#define RT_PLL_CTRL						0X44			//PLL CONTROL
#define RT_GPIO_PIN_CONFIG				0X4C			//GPIO PIN CONFIGURATION
#define RT_GPIO_PIN_POLARITY			0X4E			//GPIO PIN POLARITY/TYPE	
#define RT_GPIO_PIN_STICKY				0X50			//GPIO PIN STICKY	
#define RT_GPIO_PIN_WAKEUP				0X52			//GPIO PIN WAKE UP
#define RT_GPIO_PIN_STATUS				0X54			//GPIO PIN STATUS
#define RT_GPIO_PIN_SHARING				0X56			//GPIO PIN SHARING
#define	RT_OVER_TEMP_CURR_STATUS		0X58			//OVER TEMPERATURE AND CURRENT STATUS
#define RT_GPIO_OUT_CTRL				0X5C			//GPIO OUTPUT PIN CONTRL
#define RT_MISC_CTRL					0X5E			//MISC CONTROL
#define	RT_STEREO_DAC_CLK_CTRL1			0X60			//STEREO DAC CLOCK CONTROL 1
#define RT_STEREO_DAC_CLK_CTRL2			0X62			//STEREO DAC CLOCK CONTROL 2
#define RT_VOICE_DAC_PCMCLK_CTRL1		0X64			//VOICE/PCM DAC CLOCK CONTROL 1
#define RT_VOICE_DAC_PCMCLK_CTRL2		0X66			//VOICE/PCM DAC CLOCK CONTROL 2
#define RT_PSEDUEO_SPATIAL_CTRL			0X68			//PSEDUEO STEREO /SPATIAL EFFECT BLOCK CONTROL
#define RT_EQ_ANALOG_CTRL_INDEX			0X6A			//EQ &ANALOG CONTROL INDEX PORT
#define RT_EQ_ANALOG_CTRL_DATA			0X6C			//EQ &ANALOG CONTROL DATA PORT 
#define RT_EQ_STATUS					0X6E			//EQ STATUS
#define RT_TP_CTRL_BYTE1				0X74			//TOUCH PANEL CONTROL BYTE 1
#define RT_TP_CTRL_BYTE2				0X76			//TOUCH PANEL CONTROL BYTE 2
#define RT_TP_INDICATION				0X78			//TOUCH PANEL INDICATION
#define RT_VENDOR_ID1	  		    	0x7C			//VENDOR ID1
#define RT_VENDOR_ID2	  		    	0x7E			//VENDOR ID2

//global definition
#define RT_L_MUTE						(0x1<<15)
#define RT_L_ZC							(0x1<<14)
#define RT_R_MUTE						(0x1<<7)
#define RT_R_ZC							(0x1<<6)
#define RT_M_HP_MIXER					(0x1<<15)		//Mute source to HP Mixer
#define RT_M_SPK_MIXER					(0x1<<14)		//Mute source to Speaker Mixer
#define RT_M_MONO_MIXER					(0x1<<13)		//Mute source to Mono Mixer

//Phone Input/MONO Output Volume(0x08)
#define M_MONO_OUT_VOL					(0x1<<7)
#define M_PHONEIN_TO_HP_MIXER			(0x1<<15)


//Mic Routing Control(0x10)
#define MIC2_DIFF_INPUT_CTRL			(0x1<<4)		//MIC2 different input control
#define M_MIC2_TO_MONO_MIXER			(0x1<<5)		//Mute MIC2 to MONO mixer
#define M_MIC2_TO_SPK_MIXER				(0x1<<6)		//Mute MiC2 to SPK mixer
#define M_MIC2_TO_HP_MIXER				(0x1<<7)		//Mute MIC2 to HP mixer
#define MIC1_DIFF_INPUT_CTRL			(0x1<<12)		//MIC1 different input control
#define M_MIC1_TO_MONO_MIXER			(0x1<<13)		//Mute MIC1 to MONO mixer
#define M_MIC1_TO_SPK_MIXER				(0x1<<14)		//Mute MiC1 to SPK mixer
#define M_MIC1_TO_HP_MIXER				(0x1<<15)		//Mute MIC1 to HP mixer


//ADC Record Gain(0x12)
#define ADC_R_ZC_DET					(0x1<<5)
#define ADC_L_ZC_DET					(0x1<<6)


//ADC Input Mixer Control(0x14)



//Output Mixer Control(0x1C)
#define MONO_INPUT_SEL_VMID				(0x0<<6)
#define MONO_INPUT_SEL_HP				(0x1<<6)
#define MONO_INPUT_SEL_SPK				(0x2<<6)
#define MONO_INPUT_SEL_MONO				(0x3<<6)
#define MONO_INPUT_SEL_MASK				(0x3<<6)
#define HPL_INPUT_SEL_HPL				(0x1<<9)
#define	SPKL_INPUT_SEL_VMID				(0x0<<14)
#define	SPKL_INPUT_SEL_HPL				(0x1<<14)
#define	SPKL_INPUT_SEL_SPK				(0x2<<14)
#define	SPKL_INPUT_SEL_MONO				(0x3<<14)
#define	SPKL_INPUT_SEL_MASK				(0x3<<14)


//Voice DAC Output Volume(0x18)
#define M_V_DAC_TO_HP_MIXER				(0x1<<15)
#define M_V_DAC_TO_SPK_MIXER			(0x1<<14)
#define M_V_DAC_TO_MONO_MIXER			(0x1<<13)


//Advance register for Auto Volume Control Register0(0x20)
#define	AVC_CH_SEL_MASK				(0x1)
#define AVC_CH_SEL_L_CH				(0x0)
#define AVC_CH_SEL_R_CH				(0x1)
#define ENABLE_AVC_GAIN_CTRL		(0x1<<15)


//Micphone Control define(0x22)
#define MIC1		1
#define MIC2		2
#define MIC_BIAS_90_PRECNET_AVDD	1
#define	MIC_BIAS_75_PRECNET_AVDD	2

#define MIC1_BOOST_CONTROL_MASK		(0x3<<10)
#define MIC1_BOOST_CONTROL_BYPASS	(0x0<<10)
#define MIC1_BOOST_CONTROL_20DB		(0x1<<10)
#define MIC1_BOOST_CONTROL_30DB		(0x2<<10)
#define MIC1_BOOST_CONTROL_40DB		(0x3<<10)

#define MIC2_BOOST_CONTROL_MASK		(0x3<<8)
#define MIC2_BOOST_CONTROL_BYPASS	(0x0<<8)
#define MIC2_BOOST_CONTROL_20DB		(0x1<<8)
#define MIC2_BOOST_CONTROL_30DB		(0x2<<8)
#define MIC2_BOOST_CONTROL_40DB		(0x3<<8)

#define MIC1_BIAS_VOLT_CTRL_MASK	(0x1<<5)
#define MIC1_BIAS_VOLT_CTRL_90P		(0x0<<5)
#define MIC1_BIAS_VOLT_CTRL_75P		(0x1<<5)

#define MIC2_BIAS_VOLT_CTRL_MASK	(0x1<<4)
#define MIC2_BIAS_VOLT_CTRL_90P		(0x0<<4)
#define MIC2_BIAS_VOLT_CTRL_75P		(0x1<<4)

//PowerDown control of register(0x26)

//power management bits
#define RT_PWR_ADC					(0x1)		//read only
#define RT_PWR_DAC					(0x1<<1)	//read only
#define RT_PWR_ANL					(0x1<<2)	//read only	
#define RT_PWR_REF					(0x1<<3)	//read only
#define RT_PWR_PR0					(0x1<<8) 	//write this bit to power down the adc
#define RT_PWR_PR1					(0x1<<9) 	//write this bit to power down the dac
#define RT_PWR_PR2					(0x1<<10)	//write this bit to power down the mixer(vref/vrefout still on)
#define RT_PWR_PR3					(0x1<<11)	//write this bit to power down the mixer(vref/vrefout out off)
#define RT_PWR_PR4					(0x1<<12)	//write this bit to power down the AC-link
#define RT_PWR_PR5					(0x1<<13)	//write this bit to power down the internal clock(without PLL)
#define RT_PWR_PR6					(0x1<<14)	//write this bit to power down the Headphone Out and MonoOut
#define RT_PWR_PR7					(0x1<<15)	//write this bit to power down the Speaker Amplifier


//Power managment addition 1 (0x3A),0:Disable,1:Enable
#define PWR_DAC_REF					(0x1)
#define PWR_MAIN_BIAS				(0x1<<1)
#define PWR_MIC_BIAS2				(0x1<<2)	
#define PWR_MIC_BIAS1				(0x1<<3)	
#define	PWR_MIC_BIAS2_DET			(0x1<<4)
#define	PWR_MIC_BIAS1_DET			(0x1<<5)
#define PWR_MAIN_I2S				(0x1<<11)	
#define PWR_PRE_MEASURE				(0x1<<12)	
#define PWR_ZC_DET_PD				(0x1<<13)
#define PWR_HI_R_LOAD_HP			(0x1<<14)			
#define PWR_HI_R_LOAD_MONO			(0x1<<15)


//Power managment addition 2(0x3C),0:Disable,1:Enable
#define PWR_R_ADC_REC_MIXER			(0x1)
#define PWR_L_ADC_REC_MIXER			(0x1<<1)
#define PWR_MONO_MIXER				(0x1<<2)
#define PWR_SPK_MIXER				(0x1<<3)
#define PWR_R_HP_MIXER				(0x1<<4)
#define PWR_L_HP_MIXER				(0x1<<5)
#define PWR_R_ADC_CLK_GAIN			(0x1<<6)
#define PWR_L_ADC_CLK_GAIN			(0x1<<7)
#define PWR_R_DAC_CLK				(0x1<<8)
#define PWR_L_DAC_CLK				(0x1<<9)
#define PWR_VOICE_CLOCK				(0x1<<10)
#define PWR_TP_ADC					(0x1<<11)
#define PWR_PLL						(0x1<<12)
#define PWR_MIXER_VREF				(0x1<<13)
#define PWR_CLASS_AB				(0x1<<14)
#define EN_THREMAL_SHUTDOWN			(0x1<<15)


//Power managment addition 3(0x3E),0:Disable,1:Enable
#define PWR_MIC2_BOOST				(0x1)
#define PWR_MIC1_BOOST				(0x1<<1)
#define PWR_MIC2_VOL_CTRL			(0x1<<2)
#define PWR_MIC1_VOL_CTRL			(0x1<<3)
#define PWR_PHONE_VOL				(0x1<<4)
#define PWR_PHONE_MIXER				(0x1<<5)
#define PWR_LINE_IN_R				(0x1<<6)
#define PWR_LINE_IN_L				(0x1<<7)
#define PWR_SPK_R_OUT				(0x1<<8)
#define PWR_SPK_L_OUT				(0x1<<9)
#define PWR_HP_R_OUT				(0x1<<10)
#define PWR_HP_L_OUT				(0x1<<11)
#define PWR_SPK_RN_OUT				(0x1<<12)
#define PWR_SPK_LN_OUT				(0x1<<13)
#define PWR_MONO_VOL				(0x1<<14)



//Main Serial Data Port Control(0x34)			

//PCM Data Format Selection
#define MAIN_I2S_DF_MASK			(0x3)			//main i2s Data Format mask
#define MAIN_I2S_DF_I2S				(0x0)			//I2S FORMAT 
#define MAIN_I2S_DF_RIGHT			(0x1)			//RIGHT JUSTIFIED format
#define	MAIN_I2S_DF_LEFT			(0x2)			//LEFT JUSTIFIED  format
#define MAIN_I2S_DF_PCM				(0x3)			//PCM format

//Data Length Slection
#define MAIN_I2S_DL_MASK			(0x3<<2)		//main i2s Data Length mask	
#define MAIN_I2S_DL_16				(0x0<<2)		//16 bits
#define MAIN_I2S_DL_20				(0x1<<2)		//20 bits
#define	MAIN_I2S_DL_24				(0x2<<2)		//24 bits
#define MAIN_I2S_DL_32				(0x3<<2)		//32 bits

#define MAIN_I2S_PCM_MODE			(0x1<<6)		//PCM    	0:mode A				,1:mode B 
												 	//Non PCM	0:Normal SADLRCK/SDALRCK,1:Invert SADLRCK/SDALRCK 
#define MAIN_I2S_LINEAR_EN			(0x1<<7)		//0:Disable 1:Enable

//I2S DA SIGMA delta clock divider
#define MAIN_I2S_CLK_DIV_MASK		(0x7<<8)
#define MAIN_I2S_CLK_DIV_2			(0x0<<8)		
#define MAIN_I2S_CLK_DIV_4			(0x1<<8)
#define MAIN_I2S_CLK_DIV_8			(0x2<<8)
#define MAIN_I2S_CLK_DIV_16			(0x3<<8)
#define MAIN_I2S_CLK_DIV_32			(0x4<<8)
#define MAIN_I2S_CLK_DIV_64			(0x5<<8)

#define MAIN_I2S_DA_CLK_SOUR		(0x1<<11)		//0:from DA Filter,1:from DA Sigma Delta Clock Divider
#define MAIN_I2S_BCLK_POLARITY		(0x1<<12)		//0:Normal 1:Invert
#define MAIN_I2S_SADLRCK_CTRL		(0x1<<14)		//0:Disable,ADC and DAC use the same fs,1:Enable
#define MAIN_I2S_MODE_SEL			(0x1<<15)		//0:Master mode 1:Slave mode


//Extend Serial Data Port Control(0x36)
#define EXT_I2S_DF_MASK				(0x3)			//Extend i2s Data Format mask
#define EXT_I2S_DF_I2S				(0x0)			//I2S FORMAT 
#define EXT_I2S_DF_RIGHT			(0x1)			//RIGHT JUSTIFIED format
#define	EXT_I2S_DF_LEFT				(0x2)			//LEFT JUSTIFIED  format
#define EXT_I2S_DF_PCM				(0x3)			//PCM format

//Data Length Slection
#define EXT_I2S_DL_MASK				(0x3<<2)		//Extend i2s Data Length mask	
#define EXT_I2S_DL_16				(0x0<<2)		//16 bits
#define EXT_I2S_DL_20				(0x1<<2)		//20 bits
#define	EXT_I2S_DL_24				(0x2<<2)		//24 bits
#define EXT_I2S_DL_32				(0x3<<2)		//32 bits
#define EXT_I2S_PCM_MODE			(0x1<<6)		//PCM    	0:mode A				,1:mode B 
												 	//Non PCM	0:Normal SADLRCK/SDALRCK,1:Invert SADLRCK/SDALRCK 
#define EXT_I2S_BCLK_POLARITY		(0x1<<7)		//0:Normal 1:Invert
#define EXT_I2S_VOICE_ADC_EN		(0x1<<8)		//ADC_L=Stereo, ADC_R=Voice
#define EXT_I2S_MODE_SEL			(0x1<<14)		//0:Master	,1:Slave
#define EXT_I2S_FUNC_ENABLE			(0x1<<15)		//Enable PCM interface on GPIO 1,3,4,5  0:GPIO function,1:Voice PCM interface


//General Purpose Control Register 1(0x40)
#define GP_CLK_FROM_PLL					(0x1<<15)	//Clock source from PLL output

#define GP_HP_AMP_CTRL_MASK				(0x3<<8)
#define GP_HP_AMP_CTRL_RATIO_100		(0x0<<8)		//1.00 Vdd
#define GP_HP_AMP_CTRL_RATIO_125		(0x1<<8)		//1.25 Vdd
#define GP_HP_AMP_CTRL_RATIO_150		(0x2<<8)		//1.50 Vdd

#define GP_SPK_D_AMP_CTRL_MASK			(0x3<<6)
#define GP_SPK_D_AMP_CTRL_RATIO_175		(0x0<<6)		//1.75 Vdd
#define GP_SPK_D_AMP_CTRL_RATIO_150		(0x1<<6)		//1.50 Vdd	
#define GP_SPK_D_AMP_CTRL_RATIO_125		(0x2<<6)		//1.25 Vdd
#define GP_SPK_D_AMP_CTRL_RATIO_100		(0x3<<6)		//1.00 Vdd

#define GP_SPK_AB_AMP_CTRL_MASK			(0x3<<3)
#define GP_SPK_AB_AMP_CTRL_RATIO_225	(0x0<<3)		//2.25 Vdd
#define GP_SPK_AB_AMP_CTRL_RATIO_200	(0x1<<3)		//2.00 Vdd
#define GP_SPK_AB_AMP_CTRL_RATIO_175	(0x2<<3)		//1.75 Vdd
#define GP_SPK_AB_AMP_CTRL_RATIO_150	(0x3<<3)		//1.50 Vdd
#define GP_SPK_AB_AMP_CTRL_RATIO_125	(0x4<<3)		//1.25 Vdd	
#define GP_SPK_AB_AMP_CTRL_RATIO_100	(0x5<<3)		//1.00 Vdd

//PLL Control(0x44)
#define PLL_M_CODE_MASK				0xF				//PLL M code mask
#define PLL_M_CODE_0				0x0				//Div 2
#define PLL_M_CODE_1				0x1				//Div 3
#define PLL_M_CODE_2				0x2				//Div 4
#define PLL_M_CODE_3				0x3				//Div 5	
#define PLL_M_CODE_4				0x4				//Div 6
#define PLL_M_CODE_5				0x5				//Div 7
#define PLL_M_CODE_6				0x6				//Div 8
#define PLL_M_CODE_7				0x7				//Div 9
#define PLL_M_CODE_8				0x8				//Div 10
#define PLL_M_CODE_9				0x9				//Div 11
#define PLL_M_CODE_A				0xA				//Div 12
#define PLL_M_CODE_B				0xB				//Div 13
#define PLL_M_CODE_C				0xC				//Div 14
#define PLL_M_CODE_D				0xD				//Div 15
#define PLL_M_CODE_E				0xE				//Div 16
#define PLL_M_CODE_F				0xF				//Div 17

#define PLL_K_CODE_MASK				(0x7<<4)		//PLL K code mask
#define PLL_K_CODE_0				(0x0<<4)		//Div 2
#define PLL_K_CODE_1				(0x1<<4)		//Div 3
#define PLL_K_CODE_2				(0x2<<4)		//Div 4
#define PLL_K_CODE_3				(0x3<<4)		//Div 5	
#define PLL_K_CODE_4				(0x4<<4)		//Div 6
#define PLL_K_CODE_5				(0x5<<4)		//Div 7
#define PLL_K_CODE_6				(0x6<<4)		//Div 8
#define PLL_K_CODE_7				(0x7<<4)		//Div 9

#define PLL_BYPASS_N				(0x1<<7)		//bypass PLL N code

#define PLL_N_CODE_MASK				(0xFF<<8)		//PLL N code mask
#define PLL_N_CODE_0				(0x0<<8)		//Div 2
#define PLL_N_CODE_1				(0x1<<8)		//Div 3
#define PLL_N_CODE_2				(0x2<<8)		//Div 4
#define PLL_N_CODE_3				(0x3<<8)		//Div 5	
#define PLL_N_CODE_4				(0x4<<8)		//Div 6
#define PLL_N_CODE_5				(0x5<<8)		//Div 7
#define PLL_N_CODE_6				(0x6<<8)		//Div 8
#define PLL_N_CODE_7				(0x7<<8)		//Div 9
//.....
#define PLL_N_CODE_FF				(0xFF<<8)		//Div 257

//PLL Control(0x44)

#define PLL_CTRL_M_VAL(m)		((m)&0xf)
#define PLL_CTRL_K_VAL(k)		(((k)&0x7)<<4)
#define PLL_CTRL_N_VAL(n)		(((n)&0xff)<<8)


//GPIO Pin Configuration(0x4C)
#define GPIO_1						(0x1<<1)
#define	GPIO_2						(0x1<<2)
#define	GPIO_3						(0x1<<3)
#define GPIO_4						(0x1<<4)
#define GPIO_5						(0x1<<5)


////INTERRUPT CONTROL(0x5E)
#define DISABLE_FAST_VREG			(0x1<<15)

#define AVC_TARTGET_SEL_MASK		(0x3<<12)
#define	AVC_TARTGET_SEL_NONE		(0x0<<12)
#define	AVC_TARTGET_SEL_R 			(0x1<<12)
#define	AVC_TARTGET_SEL_L			(0x2<<12)
#define	AVC_TARTGET_SEL_BOTH		(0x3<<12)

//Stereo DAC Clock Control 1(0x60)
#define STEREO_DA_WCLK_DIV_MASK		(1)
#define STEREO_DA_WCLK_DIV_32		(0)
#define STEREO_DA_WCLK_DIV_64		(1)

#define STEREO_AD_WCLK_DIV2_MASK	(0x7<<1)
#define STEREO_AD_WCLK_DIV2_2		(0x0<<1)
#define STEREO_AD_WCLK_DIV2_4		(0x1<<1)
#define STEREO_AD_WCLK_DIV2_8		(0x2<<1)
#define STEREO_AD_WCLK_DIV2_16		(0x3<<1)
#define STEREO_AD_WCLK_DIV2_32		(0x4<<1)

#define STEREO_AD_WCLK_DIV1_MASK	(0xF<<4)
#define STEREO_AD_WCLK_DIV1_1		(0x0<<4)
#define STEREO_AD_WCLK_DIV1_2		(0x1<<4)
#define STEREO_AD_WCLK_DIV1_3		(0x2<<4)
#define STEREO_AD_WCLK_DIV1_4		(0x3<<4)
#define STEREO_AD_WCLK_DIV1_5		(0x4<<4)
#define STEREO_AD_WCLK_DIV1_6		(0x5<<4)
#define STEREO_AD_WCLK_DIV1_7		(0x6<<4)
#define STEREO_AD_WCLK_DIV1_8		(0x7<<4)
#define STEREO_AD_WCLK_DIV1_9		(0x8<<4)
#define STEREO_AD_WCLK_DIV1_10		(0x9<<4)
#define STEREO_AD_WCLK_DIV1_11		(0xA<<4)
#define STEREO_AD_WCLK_DIV1_12		(0xB<<4)
#define STEREO_AD_WCLK_DIV1_13		(0xC<<4)
#define STEREO_AD_WCLK_DIV1_14		(0xD<<4)
#define STEREO_AD_WCLK_DIV1_15		(0xE<<4)
#define STEREO_AD_WCLK_DIV1_16		(0xF<<4)

#define STEREO_SCLK_DIV2_MASK		(0x7<<8)
#define STEREO_SCLK_DIV2_2			(0x0<<8)
#define STEREO_SCLK_DIV2_4			(0x1<<8)
#define STEREO_SCLK_DIV2_8			(0x2<<8)
#define STEREO_SCLK_DIV2_16			(0x3<<8)
#define STEREO_SCLK_DIV2_32			(0x4<<8)

#define STEREO_SCLK_DIV1_MASK		(0xF<<12)
#define STEREO_SCLK_DIV1_1			(0x0<<12)
#define STEREO_SCLK_DIV1_2			(0x1<<12)
#define STEREO_SCLK_DIV1_3			(0x2<<12)
#define STEREO_SCLK_DIV1_4			(0x3<<12)
#define STEREO_SCLK_DIV1_5			(0x4<<12)
#define STEREO_SCLK_DIV1_6			(0x5<<12)
#define STEREO_SCLK_DIV1_7			(0x6<<12)
#define STEREO_SCLK_DIV1_8			(0x7<<12)
#define STEREO_SCLK_DIV1_9			(0x8<<12)
#define STEREO_SCLK_DIV1_10			(0x9<<12)
#define STEREO_SCLK_DIV1_11			(0xA<<12)
#define STEREO_SCLK_DIV1_12			(0xB<<12)
#define STEREO_SCLK_DIV1_13			(0xC<<12)
#define STEREO_SCLK_DIV1_14			(0xD<<12)
#define STEREO_SCLK_DIV1_15			(0xE<<12)
#define STEREO_SCLK_DIV1_16			(0xF<<12)


//Stereo DAC Clock Control 2(0x62)
#define STEREO_AD_64OSR_SEL_MASK	(0x1)
#define STEREO_AD_64OSR_SEL_128X	(0x0)
#define STEREO_AD_64OSR_SEL_64X		(0x1)

#define STEREO_AD_FILTER_DIV2_MASK	(0x7<<1)
#define STEREO_AD_FILTER_DIV2_2		(0x0<<1)
#define STEREO_AD_FILTER_DIV2_4		(0x1<<1)
#define STEREO_AD_FILTER_DIV2_8		(0x2<<1)
#define STEREO_AD_FILTER_DIV2_16	(0x3<<1)
#define STEREO_AD_FILTER_DIV2_32	(0x4<<1)

#define STEREO_AD_FILTER_DIV1_MASK	(0xF<<4)
#define STEREO_AD_FILTER_DIV1_1		(0x0<<4)
#define STEREO_AD_FILTER_DIV1_2		(0x1<<4)
#define STEREO_AD_FILTER_DIV1_3		(0x2<<4)
#define STEREO_AD_FILTER_DIV1_4		(0x3<<4)
#define STEREO_AD_FILTER_DIV1_5		(0x4<<4)
#define STEREO_AD_FILTER_DIV1_6		(0x5<<4)
#define STEREO_AD_FILTER_DIV1_7		(0x6<<4)
#define STEREO_AD_FILTER_DIV1_8		(0x7<<4)
#define STEREO_AD_FILTER_DIV1_9		(0x8<<4)
#define STEREO_AD_FILTER_DIV1_10	(0x9<<4)
#define STEREO_AD_FILTER_DIV1_11	(0xA<<4)
#define STEREO_AD_FILTER_DIV1_12	(0xB<<4)
#define STEREO_AD_FILTER_DIV1_13	(0xC<<4)
#define STEREO_AD_FILTER_DIV1_14	(0xD<<4)
#define STEREO_AD_FILTER_DIV1_15	(0xE<<4)
#define STEREO_AD_FILTER_DIV1_16	(0xF<<4)

#define STEREO_DA_64OSR_SEL_MASK	(1<<8)	
#define STEREO_DA_64OSR_SEL_128X	(0<<8)
#define STEREO_DA_64OSR_SEL_64X		(1<<8)

#define STEREO_DA_FILTER_DIV2_MASK	(0x7<<9)
#define STEREO_DA_FILTER_DIV2_2		(0x0<<9)
#define STEREO_DA_FILTER_DIV2_4		(0x1<<9)
#define STEREO_DA_FILTER_DIV2_8		(0x2<<9)
#define STEREO_DA_FILTER_DIV2_16	(0x3<<9)
#define STEREO_DA_FILTER_DIV2_32	(0x4<<9)

#define STEREO_DA_FILTER_DIV1_MASK	(0xF<<12)
#define STEREO_DA_FILTER_DIV1_1		(0x0<<12)
#define STEREO_DA_FILTER_DIV1_2		(0x1<<12)
#define STEREO_DA_FILTER_DIV1_3		(0x2<<12)
#define STEREO_DA_FILTER_DIV1_4		(0x3<<12)
#define STEREO_DA_FILTER_DIV1_5		(0x4<<12)
#define STEREO_DA_FILTER_DIV1_6		(0x5<<12)
#define STEREO_DA_FILTER_DIV1_7		(0x6<<12)
#define STEREO_DA_FILTER_DIV1_8		(0x7<<12)
#define STEREO_DA_FILTER_DIV1_9		(0x8<<12)
#define STEREO_DA_FILTER_DIV1_10	(0x9<<12)
#define STEREO_DA_FILTER_DIV1_11	(0xA<<12)
#define STEREO_DA_FILTER_DIV1_12	(0xB<<12)
#define STEREO_DA_FILTER_DIV1_13	(0xC<<12)
#define STEREO_DA_FILTER_DIV1_14	(0xD<<12)
#define STEREO_DA_FILTER_DIV1_15	(0xE<<12)
#define STEREO_DA_FILTER_DIV1_16	(0xF<<12)


//Voice DAC PCM Clock Control 1(0x64)
#define VOICE_SCLK_DIV2_MASK		(0x7)
#define VOICE_SCLK_DIV2_2			(0x0)
#define VOICE_SCLK_DIV2_4			(0x1)
#define VOICE_SCLK_DIV2_8			(0x2)
#define VOICE_SCLK_DIV2_16			(0x3)
#define VOICE_SCLK_DIV2_32			(0x4)

#define VOICE_SCLK_DIV1_MASK		(0xF<<4)
#define VOICE_SCLK_DIV1_1			(0x0<<4)
#define VOICE_SCLK_DIV1_2			(0x1<<4)
#define VOICE_SCLK_DIV1_3			(0x2<<4)
#define VOICE_SCLK_DIV1_4			(0x3<<4)
#define VOICE_SCLK_DIV1_5			(0x4<<4)
#define VOICE_SCLK_DIV1_6			(0x5<<4)
#define VOICE_SCLK_DIV1_7			(0x6<<4)
#define VOICE_SCLK_DIV1_8			(0x7<<4)
#define VOICE_SCLK_DIV1_9			(0x8<<4)
#define VOICE_SCLK_DIV1_10			(0x9<<4)
#define VOICE_SCLK_DIV1_11			(0xA<<4)
#define VOICE_SCLK_DIV1_12			(0xB<<4)
#define VOICE_SCLK_DIV1_13			(0xC<<4)
#define VOICE_SCLK_DIV1_14			(0xD<<4)
#define VOICE_SCLK_DIV1_15			(0xE<<4)
#define VOICE_SCLK_DIV1_16			(0xF<<4)

#define VOICE_EXTCLK_OUT_DIV_MASK   (0x7<<8)
#define VOICE_EXTCLK_OUT_DIV_1		(0x0<<8)
#define VOICE_EXTCLK_OUT_DIV_2   	(0x1<<8)
#define VOICE_EXTCLK_OUT_DIV_3   	(0x2<<8)
#define VOICE_EXTCLK_OUT_DIV_4   	(0x3<<8)
#define VOICE_EXTCLK_OUT_DIV_6   	(0x4<<8)
#define VOICE_EXTCLK_OUT_DIV_8   	(0x5<<8)
#define VOICE_EXTCLK_OUT_DIV_12   	(0x6<<8)
#define VOICE_EXTCLK_OUT_DIV_16   	(0x7<<8)

#define VOICE_WCLK_DIV_MASK			(0x1<<13)
#define VOICE_WCLK_DIV_32			(0x0<<13)
#define VOICE_WCLK_DIV_64			(0x1<<13)

#define VOICE_SYSCLK_SEL_MASK		(0x1<<14)
#define VOICE_SYSCLK_SEL_MCLK		(0x0<<14)
#define VOICE_SYSCLK_SEL_EXTCLK		(0x1<<14)

#define VOICE_MCLK_SEL_MASK			(0x1<<15)
#define VOICE_MCLK_SEL_MCLK_INPUT	(0x0<<15)
#define VOICE_MCLK_SEL_PLL_OUTPUT	(0x1<<15)

//Voice DAC PCM Clock Control 2(0x66)

#define VOICE_CLK_FILTER_DIV1_MASK	(0x7)
#define VOICE_CLK_FILTER_DIV1_2		(0x0)
#define VOICE_CLK_FILTER_DIV1_4		(0x1)
#define VOICE_CLK_FILTER_DIV1_8		(0x2)
#define VOICE_CLK_FILTER_DIV1_16	(0x3)
#define VOICE_CLK_FILTER_DIV1_32	(0x4)

#define VOICE_CLK_FILTER_DIV2_MASK	(0xF<<4)
#define VOICE_CLK_FILTER_DIV2_1		(0x0<<4)
#define VOICE_CLK_FILTER_DIV2_2		(0x1<<4)
#define VOICE_CLK_FILTER_DIV2_3		(0x2<<4)
#define VOICE_CLK_FILTER_DIV2_4		(0x3<<4)
#define VOICE_CLK_FILTER_DIV2_5		(0x4<<4)
#define VOICE_CLK_FILTER_DIV2_6		(0x5<<4)
#define VOICE_CLK_FILTER_DIV2_7		(0x6<<4)
#define VOICE_CLK_FILTER_DIV2_8		(0x7<<4)
#define VOICE_CLK_FILTER_DIV2_9		(0x8<<4)
#define VOICE_CLK_FILTER_DIV2_10	(0x9<<4)
#define VOICE_CLK_FILTER_DIV2_11	(0xA<<4)
#define VOICE_CLK_FILTER_DIV2_12	(0xB<<4)
#define VOICE_CLK_FILTER_DIV2_13	(0xC<<4)
#define VOICE_CLK_FILTER_DIV2_14	(0xD<<4)
#define VOICE_CLK_FILTER_DIV2_15	(0xE<<4)
#define VOICE_CLK_FILTER_DIV2_16	(0xF<<4)

#define VOICE_AD_DA_SIG_CLK_DIV_MASK (0x7<<8)
#define VOICE_AD_DA_SIG_CLK_DIV_2 	(0x0<<8)
#define VOICE_AD_DA_SIG_CLK_DIV_4 	(0x1<<8)
#define VOICE_AD_DA_SIG_CLK_DIV_8 	(0x2<<8)
#define VOICE_AD_DA_SIG_CLK_DIV_16 	(0x3<<8)
#define VOICE_AD_DA_SIG_CLK_DIV_32 	(0x4<<8)
#define VOICE_AD_DA_SIG_CLK_DIV_64  (0x5<<8)

#define VOICE_AD_DA_SIG_CLK_F_DA_MASK (0x1<<12)
#define VOICE_AD_DA_SIG_CLK_F_DA_F	(0x0<<12)
#define VOICE_AD_DA_SIG_CLK_F_DA_D	(0x1<<12)

#define VOICE_AD_DA_FILTER_SEL_MASK  (0x1<<13)
#define VOICE_AD_DA_FILTER_SEL_128X	(0x0<<13)
#define VOICE_AD_DA_FILTER_SEL_64X	(0x1<<13)

#define VOICE_FILTER_CLK_F_MASK		(0x1<<14)
#define VOICE_FILTER_CLK_F_MCLK		(0x0<<14)
#define VOICE_FILTER_CLK_F_VBCLK	(0X1<<14)

#define VOICE_CLK_FILTER_SLAVE_DIV_MASK (0x1<<15)
#define VOICE_CLK_FILTER_SLAVE_DIV_1 	(0x0<<15)
#define VOICE_CLK_FILTER_SLAVE_DIV_2	(0x1<<15)	


//Psedueo Stereo & Spatial Effect Block Control(0x68)
#define SPATIAL_GAIN_MASK			(0x3<<6)
#define SPATIAL_GAIN_1_0			(0x0<<6)
#define SPATIAL_GAIN_1_5			(0x1<<6)
#define SPATIAL_GAIN_2_0			(0x2<<6)

#define SPATIAL_RATIO_MASK			(0x3<<4)
#define SPATIAL_RATIO_0_0			(0x0<<4)
#define SPATIAL_RATIO_0_66			(0x1<<4)
#define SPATIAL_RATIO_1_0			(0x2<<4)

#define APF_MASK					(0x3)
#define APF_FOR_48K					(0x3)
#define APF_FOR_44_1K				(0x2)
#define APF_FOR_32K					(0x1)

#define STEREO_EXPENSION_EN			(0x1<<12)		
#define PSEUDO_STEREO_EN			(0x1<<13)
#define ALL_PASS_FILTER_EN			(0x1<<14)
#define SPATIAL_CTRL_EN				(0x1<<15)


typedef enum 
{
 BOOST_BYPASS,
 BOOST_20DB,
 BOOST_30DB,
 BOOST_40DB		
}MIC_BOOST_TYPE;

//Phone Call type define
typedef enum
{
 PHONE_CALL_NORAMAL,
// PHONE_CALL_HANDSFREE,
 PHONE_CALL_HANDSET,
 PHONE_CALL_BT_HEADSET,		
}PHONE_CALL_TYPE;


typedef struct 
{ 
	BYTE 	CodecIndex;
	unsigned short int wCodecValue;

}CodecRegister;	

typedef struct
{
	BOOL bEnable3D;	
	BYTE b3D_Gain;
	BYTE b3D_Ratio;

} S_3D_SPATIAL;

typedef enum 
{
	HW_EQ_LP0_A1=0,
	HW_EQ_LP0_H0,
	HW_EQ_BP1_A1,
	HW_EQ_BP1_A2,
	HW_EQ_BP1_H0,
	HW_EQ_BP2_A1,
	HW_EQ_BP2_A2,
	HW_EQ_BP2_H0,
	HW_EQ_BP3_A1,
	HW_EQ_BP3_A2,
	HW_EQ_BP3_H0,
	HW_EQ_BP4_A1,
	HW_EQ_HP4_H0,
	HW_EQ_CONTROL=0x10,
	HW_EQ_INPUT_VOL,
	HW_EQ_OUTPUT_VOL,
	AVC_CTRL_REG0=0x20,				
	AVC_CTRL_REG1,
	AVC_CTRL_REG2,
	AVC_CTRL_REG3,
	AVC_CTRL_REG4,
	AVC_CTRL_REG5,
	CLASS_AB_REG=0x44,
	CLASS_D_REG=0x46,
	AD_DA_MIXER_IR5=0x54,

}EXT_CODEC_INDEX;


typedef enum 
{
	POWER_STATE_D0=0,
	POWER_STATE_D1,
	POWER_STATE_D1_PLAYBACK,
	POWER_STATE_D1_RECORD,
	POWER_STATE_D2,
	POWER_STATE_D2_PLAYBACK,
	POWER_STATE_D2_RECORD,
	POWER_STATE_D3,
	POWER_STATE_D4

}POWER_STATE;


typedef enum
{
	CLUB=0,
	DANCE,
	LIVE,	
	POP,
	ROCK,
}HW_EQ_PRESET_TYPE;

typedef struct  _HW_EQ_PRESET
{
	HW_EQ_PRESET_TYPE 	HwEqType;
	unsigned short int 	EqValue[14];
	unsigned short int  HwEQCtrl;

}HW_EQ_PRESET;


typedef enum
{
	RT_WAVOUT_PATH_ALL=0,
	RT_WAVOUT_PATH_HP,
	RT_WAVOUT_PATH_SPK,
	RT_WAVOUT_PATH_MONO,
	RT_WAVOUT_PATH_DAC,
}WAVOUT_PATH;

//advance mixer 0x10
#define ENABLE_HW_EQ_BLOCK		(0x1<<15)
#define ENABLE_HW_EQ_HPF		(0x1<<4)
#define ENABLE_HW_EQ_BP3		(0x1<<3)
#define ENABLE_HW_EQ_BP2		(0x1<<2)
#define ENABLE_HW_EQ_BP1		(0x1<<1)
#define ENABLE_HW_EQ_LPF		(0x1<<0)

typedef enum
{
	R_MONO_MIXER=(0),
	R_RECORD_MIXER=(0x1<<1),
	R_HP_MIXER=(0x1<<2),
	R_PHONE=(0x1<<3),
	R_LINE_IN=(0x1<<4),
	R_MIC2=(0x1<<5),
	R_MIC1=(0x1<<6),
	L_MONO_MIXER=(0x1<<8),
	L_RECORD_MIXER=(0x1<<9),
	L_HP_MIXER=(0x1<<10),
	L_PHONE=(0x1<<11),
	L_LINE_IN=(0x1<<12),
	L_MIC2=(0x1<<13),
	L_MIC1=(0x1<<14),		

}ADC_INPUT_MIXER_CTRL;

typedef enum
{
	MASTER_MODE_A=0,
	MASTER_MODE_B,
	SLAVE_MODE_A,
	SLAVE_MODE_B,

}MODE_SEL;

typedef enum 
{
	DEPOP_MODE1_HP

}DEPOP_MODE;

#define ALL_FIELD				0xffff   

#define  MHZ11_980 11980000
#define  KHZ08_000 8000
#define  KHZ11_025 11025
#define  KHZ12_000 12000
#define  KHZ16_000 16000
#define  KHZ22_050 22050
#define  KHZ24_000 24000
#define  KHZ32_000 32000
#define  KHZ44_100 44100
#define  KHZ48_000 48000


//Class definition

class RT_CodecComm
{	
public:
	HardwareContext *m_pHWContext;
	RT_CodecComm();
	~RT_CodecComm();
	BOOL ShadowReadCodec(BYTE Offset, unsigned short int *Data);
	BOOL ShadowWriteCodec(BYTE Offset, unsigned short int Data);
	BOOL WriteCodecRegMask(BYTE Offset, unsigned short int Data,unsigned short int Mask);
	BOOL ChangeCodecPowerStatus(POWER_STATE power_state);
	BOOL WriteCodecAdvance(EXT_CODEC_INDEX Ext_Index,unsigned short int Data);
	BOOL ReadCodecAdvance(EXT_CODEC_INDEX Index,unsigned short int *Data);
	BOOL WriteCodecAdvanceMask(EXT_CODEC_INDEX Ext_Index,unsigned short int Ext_Data,unsigned short int Ext_Data_Mask);
	BOOL Init(HardwareContext *HwCxt);
	BOOL InitRTCodecReg(void);
	BOOL ConfigPcmVoicePath(BOOL bEnableVoicePath,MODE_SEL mode);
	BOOL ConfigMicBias(BYTE Mic,BYTE MicBiasCtrl);
	BOOL ConfigVmidOutput(BYTE Vmid_CaseType);
	BOOL ConfigMicBoost(BYTE Mic,MIC_BOOST_TYPE BoostType);
	BOOL Enable_Main_Spatial(BOOL Enable_Main_Spatial);	
	BOOL Enable_3D_Spatial(S_3D_SPATIAL s3D_Spatial);
	BOOL Enable_Pseudo_Stereo(BOOL Enable_Pseudo_Stereo);
	BOOL Enable_All_Pass_Filter(BOOL Enable_APF);
	BOOL EnableAVC(BOOL Enable_AVC);
	BOOL EnablePLLPath(BOOL bEnablePLL,unsigned short int K,unsigned short int M,unsigned short int N);
	BOOL Enable_ADC_Input_Source(ADC_INPUT_MIXER_CTRL ADC_Input_Sour,BOOL Enable);
	BOOL EnableHwEq(HW_EQ_PRESET_TYPE Hw_Eq_Type,BOOL HwEqEnable);
	BOOL DepopForHP(DEPOP_MODE Depop_mode);
	BOOL AudioOutEnable(WAVOUT_PATH WavOutPath,BOOL Mute);
	void SaveCodecRegToShadow();
	void RestoreRegToCodec();
	void SaveCodecExtRegToShadow();
	void ReStoreExtRegToCodec();
	DWORD ProcessAudioMessage(UINT uMsg,DWORD dwParam1,DWORD dwParam2);
	void DelayMSTime(unsigned int MilliSec);

private:	
	unsigned short int m_WaveInSampleRate;
	unsigned short int m_WaveOutSampleRate;	
	friend class HardwareContext; 
};


#endif //__RTCODECCOMM_H__
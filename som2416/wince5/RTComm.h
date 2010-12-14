#ifndef __RTCOMM_H__
#define __RTCOMM_H__

#include "wavemain.h"

#define RT_CODEC_REG_00		0x00				//CODEC REGISTER 0x00
#define RT_CODEC_REG_02		0x02				//CODEC REGISTER 0x02
#define RT_CODEC_REG_04		0x04				//CODEC REGISTER 0x04
#define RT_CODEC_REG_06		0x06				//CODEC REGISTER 0x06
#define RT_CODEC_REG_08		0x08				//CODEC REGISTER 0x08
#define RT_CODEC_REG_0A		0x0A				//CODEC REGISTER 0x0A
#define RT_CODEC_REG_0C		0x0C				//CODEC REGISTER 0x0C
#define RT_CODEC_REG_0E		0x0E				//CODEC REGISTER 0x0E
#define RT_CODEC_REG_10		0x10				//CODEC REGISTER 0x10
#define RT_CODEC_REG_12		0x12				//CODEC REGISTER 0x12
#define RT_CODEC_REG_14		0x14				//CODEC REGISTER 0x14
#define RT_CODEC_REG_16		0x16				//CODEC REGISTER 0x16
#define RT_CODEC_REG_18		0x18				//CODEC REGISTER 0x18
#define RT_CODEC_REG_1A		0x1A				//CODEC REGISTER 0x1A	
#define RT_CODEC_REG_1C		0x1C				//CODEC REGISTER 0x1C
#define RT_CODEC_REG_1E		0x1E				//CODEC REGISTER 0x1E
#define RT_CODEC_REG_20		0x20				//CODEC REGISTER 0x20	
#define RT_CODEC_REG_22		0x22				//CODEC REGISTER 0x22
#define RT_CODEC_REG_24		0x24				//CODEC REGISTER 0x24
#define RT_CODEC_REG_26		0x26				//CODEC REGISTER 0x26
#define RT_CODEC_REG_28		0x28				//CODEC REGISTER 0x28
#define RT_CODEC_REG_2A		0x2A				//CODEC REGISTER 0x2A
#define RT_CODEC_REG_2C		0x2C				//CODEC REGISTER 0x2C
#define RT_CODEC_REG_2E		0x2E				//CODEC REGISTER 0x2E
#define RT_CODEC_REG_30		0x30				//CODEC REGISTER 0x30
#define RT_CODEC_REG_32		0x32				//CODEC REGISTER 0x32
#define RT_CODEC_REG_34		0x34				//CODEC REGISTER 0x34
#define RT_CODEC_REG_36		0x36				//CODEC REGISTER 0x36
#define RT_CODEC_REG_38		0x38				//CODEC REGISTER 0x38
#define RT_CODEC_REG_3A		0x3A				//CODEC REGISTER 0x3A
#define RT_CODEC_REG_3C		0x3C				//CODEC REGISTER 0x3C
#define RT_CODEC_REG_3E		0x3E				//CODEC REGISTER 0x3E
#define RT_CODEC_REG_40		0x40				//CODEC REGISTER 0x40	
#define RT_CODEC_REG_42		0x42				//CODEC REGISTER 0x42
#define RT_CODEC_REG_44		0x44				//CODEC REGISTER 0x44
#define RT_CODEC_REG_46		0x46				//CODEC REGISTER 0x46
#define RT_CODEC_REG_48		0x48				//CODEC REGISTER 0x48
#define RT_CODEC_REG_4A		0x4A				//CODEC REGISTER 0x4A
#define RT_CODEC_REG_4C		0x4C				//CODEC REGISTER 0x4C
#define RT_CODEC_REG_4E		0x4E				//CODEC REGISTER 0x4E
#define RT_CODEC_REG_50		0x50				//CODEC REGISTER 0x50				
#define RT_CODEC_REG_52		0x52				//CODEC REGISTER 0x52
#define RT_CODEC_REG_54		0x54				//CODEC REGISTER 0x54
#define RT_CODEC_REG_56		0x56				//CODEC REGISTER 0x56
#define RT_CODEC_REG_58		0x58				//CODEC REGISTER 0x58
#define RT_CODEC_REG_5A		0x5A				//CODEC REGISTER 0x5A
#define RT_CODEC_REG_5C		0x5C				//CODEC REGISTER 0x5C
#define RT_CODEC_REG_5E		0x5E				//CODEC REGISTER 0x5E
#define RT_CODEC_REG_62		0x62				//CODEC REGISTER 0x62
#define RT_CODEC_REG_64		0x64				//CODEC REGISTER 0x64
#define RT_CODEC_REG_66		0x66				//CODEC REGISTER 0x66
#define RT_CODEC_REG_68		0x68				//CODEC REGISTER 0x68
#define RT_CODEC_REG_6A		0x6A				//CODEC REGISTER 0x6A
#define RT_CODEC_REG_6C		0x6C				//CODEC REGISTER 0x6C
#define RT_CODEC_REG_6E		0x6E				//CODEC REGISTER 0x6E
#define RT_CODEC_REG_70		0x70				//CODEC REGISTER 0x70
#define RT_CODEC_REG_72		0x72				//CODEC REGISTER 0x72
#define RT_CODEC_REG_74		0x74				//CODEC REGISTER 0x74
#define RT_CODEC_REG_76		0x76				//CODEC REGISTER 0x76
#define RT_CODEC_REG_78		0x78				//CODEC REGISTER 0x78
#define RT_CODEC_REG_7A		0x7A				//CODEC REGISTER 0x7A
#define RT_CODEC_REG_7C		0x7C				//CODEC REGISTER 0x7C
#define RT_CODEC_REG_7E		0x7E				//CODEC REGISTER 0x7E


#define RT_BIT0				(0x1<<0)
#define RT_BIT1				(0x1<<1)
#define RT_BIT2				(0x1<<2)
#define RT_BIT3				(0x1<<3)
#define RT_BIT4				(0x1<<4)
#define RT_BIT5				(0x1<<5)
#define RT_BIT6				(0x1<<6)
#define RT_BIT7				(0x1<<7)
#define RT_BIT8				(0x1<<8)
#define RT_BIT9				(0x1<<9)
#define RT_BIT10			(0x1<<10)
#define RT_BIT11			(0x1<<11)
#define RT_BIT12			(0x1<<12)
#define RT_BIT13			(0x1<<13)
#define RT_BIT14			(0x1<<14)
#define RT_BIT15			(0x1<<15)


#define ALL_FIELD  0xffff   

#define  MHZ11_980 11980000
#define  KHZ08_000 8000									//SampleRate is  8K
#define  KHZ11_025 11025								//SampleRate is 11k
#define  KHZ12_000 12000								//SampleRate is 12k
#define  KHZ16_000 16000								//SampleRate is 16k
#define  KHZ22_050 22050								//SampleRate is 22k
#define  KHZ24_000 24000								//SampleRate is 24k
#define  KHZ32_000 32000								//SampleRate is 32k
#define  KHZ44_100 44100								//SampleRate is 44.1k
#define  KHZ48_000 48000								//SampleRate is 48k
#define  KHZ96_000 96000								//SampleRate is 96k
#define  KHZ192_000 192000								//SampleRate is 192k


//WaveOut channel for realtek codec
enum 
{
	RT_WAVOUT_SPK  				=(0x1<<0),
	RT_WAVOUT_SPK_R				=(0x1<<1),
	RT_WAVOUT_SPK_L				=(0x1<<2),
	RT_WAVOUT_HP				=(0x1<<3),
	RT_WAVOUT_HP_R				=(0x1<<4),
	RT_WAVOUT_HP_L				=(0x1<<5),	
	RT_WAVOUT_MONO				=(0x1<<6),
	RT_WAVOUT_AUXOUT			=(0x1<<7),
	RT_WAVOUT_AUXOUT_R			=(0x1<<8),
	RT_WAVOUT_AUXOUT_L			=(0x1<<9),
	RT_WAVOUT_LINEOUT			=(0x1<<10),
	RT_WAVOUT_LINEOUT_R			=(0x1<<11),
	RT_WAVOUT_LINEOUT_L			=(0x1<<12),	
	RT_WAVOUT_DAC				=(0x1<<13),		
	RT_WAVOUT_ALL_ON			=(0x1<<14),
};

//WaveIn channel for realtek codec
enum
{
	RT_WAVIN_R_MONO_MIXER		=(0x1<<0),
	RT_WAVIN_R_SPK_MIXER		=(0x1<<1),
	RT_WAVIN_R_HP_MIXER			=(0x1<<2),
	RT_WAVIN_R_PHONE			=(0x1<<3),
	RT_WAVIN_R_AUXIN			=(0x1<<3),	
	RT_WAVIN_R_LINE_IN			=(0x1<<4),
	RT_WAVIN_R_MIC2				=(0x1<<5),
	RT_WAVIN_R_MIC1				=(0x1<<6),

	RT_WAVIN_L_MONO_MIXER		=(0x1<<8),
	RT_WAVIN_L_SPK_MIXER		=(0x1<<9),
	RT_WAVIN_L_HP_MIXER			=(0x1<<10),
	RT_WAVIN_L_PHONE			=(0x1<<11),
	RT_WAVIN_L_AUXIN			=(0x1<<11),
	RT_WAVIN_L_LINE_IN			=(0x1<<12),
	RT_WAVIN_L_MIC2				=(0x1<<13),
	RT_WAVIN_L_MIC1				=(0x1<<14),
};

//power status 
enum 
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
};

//EQ preset 
enum
{
	CLUB=0,
	DANCE,
	LIVE,	
	POP,
	ROCK,
};

enum 
{
 	BOOST_BYPASS=0,
 	BOOST_20DB,
 	BOOST_30DB,
 	BOOST_40DB		
};


enum 
{
	DEPOP_MODE1_FOR_HP=0,
	DEPOP_MODE2_FOR_HP,
	DEPOP_MODE3_FOR_HP
};

enum
{
	PCM_MASTER_MODE_A=0,
	PCM_MASTER_MODE_B,
	PCM_SLAVE_MODE_A,
	PCM_SLAVE_MODE_B,
};

typedef struct 
{ 
	BYTE 	CodecIndex;
	unsigned short int wCodecValue;

}CodecRegister;	

typedef struct  _HW_EQ_PRESET
{
	unsigned short int 	HwEqType;
	unsigned short int 	EqValue[14];
	unsigned short int  HwEQCtrl;

}HW_EQ_PRESET;

enum 
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
	DIG_INTER_REG=0x39,
	CLASS_AB_REG=0x44,
	CLASS_D_TEM_SEN=0x4A,
	AD_DA_MIXER_IR=0x54,

};


//realtek codec common function
class RT_CodecComm
{
public:
		HardwareContext *m_pHWContext;		
		RT_CodecComm(void);			
		~RT_CodecComm(void);
		BOOL RTReadCodecReg(BYTE Offset, unsigned short int *Data);
		BOOL RTWriteCodecReg(BYTE Offset, unsigned short int Data);
		BOOL RTWriteCodecRegMask(BYTE Offset, unsigned short int Data,unsigned short int Mask);
		BOOL RTReadCodecIndex(BYTE Ext_Index,unsigned short int *Data);
		BOOL RTWriteCodecIndex(BYTE Ext_Index,unsigned short int Data);		
		BOOL RTWriteCodecIndexMask(BYTE Ext_Index,unsigned short int Ext_Data,unsigned short int Ext_Data_Mask);		
		void DelayMSTime(unsigned int MilliSec);
		void WarmResetController();

		virtual void RT_Codec_Init()=0;
		virtual void RT_Codec_DeInit()=0;
		virtual void RT_PlayBack_Start(USHORT Channel_On,USHORT Channel_Off)=0;
		virtual	void RT_PlayBack_End(USHORT Channel_On,USHORT Channel_Off)=0;
		virtual void RT_Record_Start(USHORT Channel_On,USHORT Channel_Off)=0;
		virtual void RT_Record_End(USHORT Channel_On,USHORT Channel_Off)=0;
		virtual	void RT_Suspend(USHORT Channel_On,USHORT Channel_Off)=0;
		virtual void RT_Resume(USHORT Channel_On,USHORT Channel_Off)=0;
		virtual void ChangeCodecPowerStatus(BYTE power_state)=0;
		virtual DWORD RT_ProcessAudioMessage(UINT uMsg,DWORD dwParam1,DWORD dwParam2)=0;

		unsigned short int m_WaveInSampleRate;
		unsigned short int m_WaveOutSampleRate;	
		unsigned short int m_Channel_Out_Sel;		
		unsigned short int m_Channel_In_Sel;		
private:	

		friend class HardwareContext; 
};

#endif //__RTCOMM_H__
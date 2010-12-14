
//-----------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include "RTCodecComm.h"

//*************************************************************************************************************
//initize codec register

CodecRegister Set_Codec_Reg_Init[]=
{
	{RT_SPEAKER_OUT_VOL		,0x0000},//default Speark to 0DB
	{RT_HP_OUT_VOL			,0x0808},//default HP to -12DB
	{RT_ADC_RECORD_MIXER	,0x3F3F},//default Record is MicIn
	{RT_STEREO_DAC_VOL		,0x0808},//default stereo DAC volume
	{RT_MICROPHONE_CTRL		,0x0400},//set boost to +20DB
	{RT_POWERDOWN_CTRL_STAT	,0x0000},//power on all PR bit
	{RT_EQ_ANALOG_CTRL_INDEX,0x0054},//AD_DA_Mixer_internal Register5
	{RT_EQ_ANALOG_CTRL_DATA	,0xE184},//To reduce power consumption for DAC reference

#if USE_CLASS_AB_SPEAKER	

	{RT_OUTPUT_MIXER_CTRL	,0x4b40},//default output mixer control,CLASS AB

#else

	{RT_OUTPUT_MIXER_CTRL	,0x6b40},//default output mixer control,CLASS D	
	
#endif	
	{RT_MAIN_SDP_CTRL		,0x8000},	//set I2S codec to slave mode

#if 0//if you need use AVC function,please fill below initize value of AVC,below value only be refer	

	{RT_EQ_ANALOG_CTRL_INDEX,0x0020},//Auto Volume Control Register 0 
	{RT_EQ_ANALOG_CTRL_DATA	,0x0058},//2^11 samples for monitor window control
	{RT_EQ_ANALOG_CTRL_INDEX,0x0021},//Auto Volume Control Register 1 
	{RT_EQ_ANALOG_CTRL_DATA	,0x0400},//Maximum PCM absolute level
	{RT_EQ_ANALOG_CTRL_INDEX,0x0022},//Auto Volume Control Register 2 
	{RT_EQ_ANALOG_CTRL_DATA	,0x0350},//Mimimum PCM absolute level
	{RT_EQ_ANALOG_CTRL_INDEX,0x0023},//Auto Volume Control Register 3 
	{RT_EQ_ANALOG_CTRL_DATA	,0x0001},//Non-active PCM absolute level
	{RT_EQ_ANALOG_CTRL_INDEX,0x0024},//Auto Volume Control Register 4 
	{RT_EQ_ANALOG_CTRL_DATA	,0x03FF},//control the sensitivity to increase Gain
	{RT_EQ_ANALOG_CTRL_INDEX,0x0025},//Auto Volume Control Register 5 
	{RT_EQ_ANALOG_CTRL_DATA	,0x0400},//control the sensitivity to decrease Gain

#endif	
};

#define SET_CODEC_REG_INIT_NUM	(sizeof(Set_Codec_Reg_Init)/sizeof(CodecRegister))

//*************************************************************************************************************
//Hardware EQ 

HW_EQ_PRESET *HwEq_Preset;

HW_EQ_PRESET HwEq_Preset_48k[]={
//		0x0	0x1	0x2  0x3   0x4	0x5	0x6	0x7	0x8	0x9	0xa	0xb	0xc	0x10	
	{CLUB,{0x1F2C,0x13D1,0xC1CB,0x1E5D,0x0C73,0xCD47,0x188D,0x0C73,0xC3B5,0x1CD0,0x0C73,0x1DF8,0x2FB2},0x800E},
	{DANCE,{0x1F2C,0x13D1,0xC070,0x1F95,0x0BCE,0xCA43,0x1A29,0xFA73,0xDEDF,0x0ED8,0xF8B1,0x1DF8,0x2FB2},0x800F},
	{LIVE,{0x1E74,0xFA92,0xC249,0x1DF8,0x2298,0xC882,0x1C10,0x0D73,0xDA40,0x1561,0x0556,0x1DF8,0x2FB2},0x800F},
	{POP,{0x1EB5,0xFCB5,0xC1D3,0x1E5D,0x1FC4,0xD9D7,0x15F6,0xFB53,0xFFFF,0x06D3,0xF804,0x1DF8,0x2FB2},0x800F},
	{ROCK,{0x1EB5,0xFCB5,0xC070,0x1F95,0x0556,0xC3D8,0x1C56,0xF6E7,0x0C5D,0x0FC7,0x4030,0x1DF8,0x2FB2},0x800F},
};

HW_EQ_PRESET HwEq_Preset_44k[]={
//			0x0		0x1		0x2	  0x3	0x4		0x5		0x6		0x7	0x8		0x9		0xa	0xb		0xc		0x10			
	{CLUB ,{0x1DCC,0x13D1,0xC289,0x1E39,0x0C73,0xE1A2,0x17F8,0x0C73,0xC5E5,0x1C8B,0x0C73,0x1180,0x2FB2},0x800E},
	{DANCE,{0x1DCC,0x13D1,0xC08E,0x1F8C,0x0BCE,0xCB7E,0x19B2,0xFA73,0x0655,0x0DBB,0xF8B1,0x1180,0x2FB2},0x800F},
	{LIVE ,{0x1CB9,0xFA92,0xC36D,0x1DCC,0x2298,0xD8CA,0x1BBC,0x0D73,0x0748,0x1496,0x0556,0x1180,0x2FB2},0x800F},
	{POP  ,{0x1D40,0xFCB5,0xC2AE,0x1E39,0x1FC4,0x075E,0x1537,0xFB53,0x23D6,0x056C,0xF804,0x1180,0x2FB2},0x800F},
	{ROCK ,{0x1D40,0xFCB5,0xC08E,0x1F8C,0x0556,0xC4D7,0x1C08,0xF6E7,0x2CAB,0x0EA5,0x4030,0x1180,0x2FB2},0x800F},
};


//*************************************************************************************************************
//store/restore register in the suspend/resume mode
BYTE RestoreRegIndex[]={
						RT_SPEAKER_OUT_VOL,			//0x02
						RT_HP_OUT_VOL,				//0x04
						RT_PHONEIN_MONO_OUT_VOL,	//0x08
						RT_LINE_IN_VOL,				//0x0A
						RT_STEREO_DAC_VOL,			//0x0C
						RT_MIC_VOL,					//0x0E
						RT_MIC_ROUTING_CTRL,		//0x10
						RT_ADC_RECORD_GAIN,			//0x12
						RT_ADC_RECORD_MIXER,		//0x14
						RT_VOICE_DAC_OUT_VOL,		//0x18
						RT_OUTPUT_MIXER_CTRL,		//0x1C
						RT_MICROPHONE_CTRL,			//0x22
						RT_POWERDOWN_CTRL_STAT,		//0x26			
						RT_MAIN_SDP_CTRL,			//0x34
						RT_MISC_CTRL,				//0X5E
					   };

#define RESTORE_REG_INDEX_NUM	(sizeof(RestoreRegIndex)/sizeof(BYTE))

unsigned short int 	CodecShadowRegisters[RESTORE_REG_INDEX_NUM];


EXT_CODEC_INDEX RestoreExtRegIndex[]={

						HW_EQ_LP0_A1,				//0x00
						HW_EQ_LP0_H0,				//0x01
						HW_EQ_BP1_A1,				//0x02
						HW_EQ_BP1_A2,				//0x03
						HW_EQ_BP1_H0,				//0x04
						HW_EQ_BP2_A1,				//0x05
						HW_EQ_BP2_A2,				//0x06
						HW_EQ_BP2_H0,				//0x07
						HW_EQ_BP3_A1,				//0x08
						HW_EQ_BP3_A2,				//0x09
						HW_EQ_BP3_H0,				//0x0A
						HW_EQ_BP4_A1,				//0x0B
						HW_EQ_HP4_H0,				//0x0C
						HW_EQ_CONTROL,				//0x10
#if USE_CLASS_AB_SPEAKER
						CLASS_AB_REG,				//0x44	
#else
						CLASS_D_REG,				//0x46
#endif
						AD_DA_MIXER_IR5,			//0x54
						};

#define RESTORE_EXTREG_INDEX_NUM	(sizeof(RestoreExtRegIndex)/sizeof(EXT_CODEC_INDEX)) 

unsigned short int 	ExtCodecShadowRegisters[RESTORE_EXTREG_INDEX_NUM];

//------------------------------------------------------------------------
// function implement
//------------------------------------------------------------------------

RT_CodecComm::RT_CodecComm(void)
{
	
}

RT_CodecComm::~RT_CodecComm(void)
{
	
}

//*****************************************************************************	
//
//function:init Class member of codec
//
//*****************************************************************************
BOOL RT_CodecComm::Init(HardwareContext *HwCxt)
{
	m_pHWContext=HwCxt;

	if(!m_pHWContext)
	{
		return FALSE;
	}
	m_WaveOutSampleRate=SAMPLERATE;
	m_WaveInSampleRate=SAMPLERATE;
		
	return TRUE;
}
//*****************************************************************************	
//
//function:implement delay function
//
//*****************************************************************************
void RT_CodecComm::DelayMSTime(unsigned int MilliSec)
{
	if(m_pHWContext)
	{
		m_pHWContext->OSTDelayMilliSecTime(MilliSec);	
	}	
}

//*****************************************************************************
//
//function:Initialize Register of Codec function
//
//*****************************************************************************
BOOL RT_CodecComm::InitRTCodecReg(void)
{
	int i;
	BOOL bRetVal=FALSE;

		ShadowWriteCodec(RT_POWERDOWN_CTRL_STAT,0x0);	//power on all PR bit
	
		WriteCodecRegMask(RT_PWR_MANAG_ADD2,PWR_MIXER_VREF,PWR_MIXER_VREF);	//power on Vref for All analog circuit
		
		WriteCodecRegMask(RT_PWR_MANAG_ADD1,PWR_MAIN_BIAS | PWR_MAIN_I2S,PWR_MAIN_BIAS | PWR_MAIN_I2S);	//power on main bias and I2S interface

		WriteCodecRegMask(RT_PWR_MANAG_ADD3,PWR_HP_R_OUT|PWR_HP_L_OUT,PWR_HP_R_OUT|PWR_HP_L_OUT);	//power on HP volume AMP				

	for(i=0;i<SET_CODEC_REG_INIT_NUM;i++)
	{
		bRetVal=ShadowWriteCodec(Set_Codec_Reg_Init[i].CodecIndex,Set_Codec_Reg_Init[i].wCodecValue); 

		if(!bRetVal)
			break;
	}	
 	
	return bRetVal;
}
//*****************************************************************************
//
//function:Save codec register to shadow
//
//*****************************************************************************
void RT_CodecComm::SaveCodecRegToShadow()
{
	int i;

	for(i=0;i<RESTORE_REG_INDEX_NUM;i++)
	{		
		if(!ShadowReadCodec(RestoreRegIndex[i],&CodecShadowRegisters[i]))
		{
			return;
		}			
	}	
}
//*****************************************************************************
//
//function:restore shadow register to codec
//
//*****************************************************************************
void RT_CodecComm::RestoreRegToCodec()
{
	int i;

	for(i=0;i<RESTORE_REG_INDEX_NUM;i++)
	{
		if(!ShadowWriteCodec(RestoreRegIndex[i],CodecShadowRegisters[i]))
			{
				return;
			}
	}	

}
//*****************************************************************************
//
//function:Save codec extend register to shadow
//
//*****************************************************************************
void RT_CodecComm::SaveCodecExtRegToShadow()
{
	int i;

	for(i=0;i<RESTORE_EXTREG_INDEX_NUM;i++)
	{
		if(!ReadCodecAdvance(RestoreExtRegIndex[i],&ExtCodecShadowRegisters[i]))
		{
			return;
		}
	}	

}
//*****************************************************************************
//
//function:restore shadow extend register to codec
//
//*****************************************************************************
void RT_CodecComm::ReStoreExtRegToCodec()
{
	int i;

	for(i=0;i<RESTORE_EXTREG_INDEX_NUM;i++)
	{
		if(!WriteCodecAdvance(RestoreExtRegIndex[i],ExtCodecShadowRegisters[i]))
		{
			return;
		}
	}	
}

//*****************************************************************************
//
//function:restore shadow read register to codec
//
//*****************************************************************************
BOOL RT_CodecComm::ShadowReadCodec(BYTE Offset, unsigned short int *Data)
{
	BOOL b_retVal;
	
	if(!m_pHWContext)
	{
		return FALSE;
	}
	
	b_retVal=m_pHWContext->SafeReadCodec(Offset,Data);	
	
	return b_retVal;
}
//*****************************************************************************
//
//function:restore shadow write register to codec
//
//*****************************************************************************
BOOL RT_CodecComm::ShadowWriteCodec(BYTE Offset, unsigned short int Data)
{
	BOOL b_retVal;
	
	if(!m_pHWContext)
	{
		return FALSE;	
	}	
	
	b_retVal=m_pHWContext->SafeWriteCodec(Offset,Data);	

	return b_retVal;
}

//*****************************************************************************
//
//function:write codec register with mask
//
//*****************************************************************************
BOOL RT_CodecComm::WriteCodecRegMask(BYTE Offset, unsigned short int Data,unsigned short int Mask)
{
	BOOL RetVal=FALSE;
	unsigned short int CodecData;

	if(!Mask)
		return RetVal; 

	if(Mask!=ALL_FIELD)
	 {
		if(ShadowReadCodec(Offset,&CodecData))
		{
			CodecData&=~Mask;
			CodecData|=(Data&Mask);
			RetVal=ShadowWriteCodec(Offset,CodecData);
		}
	 }		
	else
	{
		RetVal=ShadowWriteCodec(Offset,Data);
	}

	return RetVal;
}
//*****************************************************************************
//
//function:Write advance register of codec
//
//*****************************************************************************
BOOL RT_CodecComm::WriteCodecAdvance(EXT_CODEC_INDEX Ext_Index,unsigned short int Ext_Data)
{
	BOOL RetVal=FALSE;
			
		RetVal=ShadowWriteCodec(RT_EQ_ANALOG_CTRL_INDEX,Ext_Index);	// write codec advance index

		if(RetVal)
		{
			RetVal=ShadowWriteCodec(RT_EQ_ANALOG_CTRL_DATA,Ext_Data);//write codec advance data
		}		
			
		return  RetVal;	
			
}
//*****************************************************************************
//
//function:Read advance register of codec
//
//*****************************************************************************
BOOL RT_CodecComm::ReadCodecAdvance(EXT_CODEC_INDEX Ext_Index,unsigned short int *Data)
{
	BOOL RetVal=FALSE;		

	RetVal=ShadowWriteCodec(RT_EQ_ANALOG_CTRL_INDEX,Ext_Index);	// write codec advance index		

	if(RetVal)
	 {
		RetVal=ShadowReadCodec(RT_EQ_ANALOG_CTRL_DATA,Data);	// read codec advance data			
	 }

	return RetVal;
}
//*****************************************************************************
//
//function:Write advance register of codec compare with Mask bit
//
//*****************************************************************************

BOOL RT_CodecComm::WriteCodecAdvanceMask(EXT_CODEC_INDEX Ext_Index,unsigned short int Ext_Data,unsigned short int Ext_Data_Mask)
{
	
	unsigned short int CodecAdvData;
	BOOL RetVal=FALSE;

	if(!Ext_Data_Mask)
		return RetVal; 

	if(Ext_Data_Mask!=ALL_FIELD)
	 {
		if(ReadCodecAdvance(Ext_Index,&CodecAdvData))
		{
			CodecAdvData&=~Ext_Data_Mask;
			CodecAdvData|=(Ext_Data&Ext_Data_Mask);			
			RetVal=WriteCodecAdvance(Ext_Index,CodecAdvData);
		}
	 }		
	else
	{
		RetVal=WriteCodecAdvance(Ext_Index,Ext_Data);
	}

	return RetVal;

}
//*****************************************************************************
//
//Function:Enable/Disable Main Spatial function
//
//*****************************************************************************
BOOL RT_CodecComm::Enable_Main_Spatial(BOOL Enable_Main_Spatial)
{
	BOOL bRetVal=FALSE;
	
	if(Enable_Main_Spatial)
	{
		//Enable Main Spatial 
		bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,SPATIAL_CTRL_EN,SPATIAL_CTRL_EN);		
	}
	else
	{
		//Disable Main Spatial 
		bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,0,SPATIAL_CTRL_EN);	
	}

	return bRetVal;
}
//*****************************************************************************
//
//Function:Enable/Disable 3D spatial control function(use this function,enable Main Spatial function first)
//
//*****************************************************************************
BOOL RT_CodecComm::Enable_3D_Spatial(S_3D_SPATIAL s3D_Spatial)
{
	BOOL bRetVal=FALSE;
	
	if(s3D_Spatial.bEnable3D)
	{
		Enable_Main_Spatial(TRUE);

		bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,(STEREO_EXPENSION_EN  | (s3D_Spatial.b3D_Gain<<6) | (s3D_Spatial.b3D_Ratio<<4) ) 
														 ,SPATIAL_GAIN_MASK | SPATIAL_RATIO_MASK | STEREO_EXPENSION_EN );
	}
	else
	{
		bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,0,STEREO_EXPENSION_EN);
	}

	return bRetVal;
}

//*****************************************************************************
//
//function:Enable/Disable Pseudio Stereo function(use this function,enable Main Spatial function first)
//
//*****************************************************************************
BOOL RT_CodecComm::Enable_Pseudo_Stereo(BOOL Enable_Pseudo_Stereo)
{
	BOOL bRetVal=FALSE;

	if(Enable_Pseudo_Stereo)
	{
		Enable_Main_Spatial(TRUE);
		Enable_All_Pass_Filter(TRUE);
		//Enable Pseudio stereo 
		bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,PSEUDO_STEREO_EN,PSEUDO_STEREO_EN);
	}
	else
	{
		Enable_All_Pass_Filter(FALSE);	
		//Disable Pseudio stereo
		bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,0,PSEUDO_STEREO_EN);
		
	}

	return bRetVal;
}

//*****************************************************************************
//
//function:Enable/Disable All Pass Filter function(use this function,enable Main Spatial function first)
//
//*****************************************************************************
BOOL RT_CodecComm::Enable_All_Pass_Filter(BOOL Enable_APF)
{
	BOOL bRetVal=FALSE;
	
	if(Enable_APF)
	{
		Enable_Main_Spatial(TRUE);

		//set parameter a1 support 48K	
		if(KHZ48_000==m_WaveOutSampleRate)
		{				
			bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,APF_FOR_48K,APF_MASK);
		}
		else if(KHZ44_100==m_WaveOutSampleRate)//set parameter a1 support 44.1K	
		{
			bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,APF_FOR_44_1K,APF_MASK);
		}
		else
		{
			//set parameter a1 support 32k and lower	
			bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,APF_FOR_32K,APF_MASK);
		}	

		//Enable All Pass Filter 
		bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,ALL_PASS_FILTER_EN,ALL_PASS_FILTER_EN);
	}
	else
	{
		//Disable All Pass Filter
		bRetVal=WriteCodecRegMask(RT_PSEDUEO_SPATIAL_CTRL,0,ALL_PASS_FILTER_EN);
	}

	return bRetVal;
}
//*****************************************************************************
//
//function:Enable/Disable ADC input source control
//
//*****************************************************************************
BOOL RT_CodecComm::Enable_ADC_Input_Source(ADC_INPUT_MIXER_CTRL ADC_Input_Sour,BOOL Enable)
{
	BOOL bRetVal=FALSE;
	
	if(Enable)
	{
		//Enable ADC source 
		bRetVal=WriteCodecRegMask(RT_ADC_RECORD_MIXER,0,ADC_Input_Sour);
	}
	else
	{
		//Disable ADC source		
		bRetVal=WriteCodecRegMask(RT_ADC_RECORD_MIXER,ADC_Input_Sour,ADC_Input_Sour);
	}

	return bRetVal;
}

//*****************************************************************************
//
//function:Enable/Disable Auto Volume Control function
//
//*****************************************************************************
BOOL RT_CodecComm::EnableAVC(BOOL Enable_AVC)
{
	BOOL bRetVal=FALSE;
	
	if(Enable_AVC)
	{			
		//enable AVC target select,if use voice interface,please use one channel 
		WriteCodecRegMask(RT_MISC_CTRL,AVC_TARTGET_SEL_BOTH,AVC_TARTGET_SEL_MASK); 

		//Enable AVC function
		bRetVal=WriteCodecAdvanceMask(AVC_CTRL_REG0,ENABLE_AVC_GAIN_CTRL,ENABLE_AVC_GAIN_CTRL);
		
	}
	else
	{
		//Disable AVC function
		bRetVal=WriteCodecAdvanceMask(AVC_CTRL_REG0,0,ENABLE_AVC_GAIN_CTRL);
	}

	return bRetVal;
}
//*****************************************************************************
//
//function:Config Vmid Control function
//
//			 AVDD  		HPVDD 	SPKVDD	SPK AB		SPK D		HP		
//case 1	 3.3V		3.3V	3.3V	1.00 Vdd	1.00 Vdd	1.00 Vdd	
//case 2	 3.3V		3.3V	4.2V	1.25 Vdd	1.25 Vdd	1.00 Vdd
//case 3	 2.5V		3.3V	3.3V	1.25 Vdd	1.25 Vdd	1.25 Vdd
//case 4	 2.5V		3.3V	4.2V	1.75 Vdd	1.75 Vdd	1.25 Vdd
//
//*****************************************************************************
BOOL RT_CodecComm::ConfigVmidOutput(BYTE Vmid_CaseType)
{

	BOOL bRetVal=FALSE;



	switch(Vmid_CaseType)
	{
		case 1:
			bRetVal=WriteCodecRegMask(
										RT_GEN_CTRL_REG1,
										(GP_HP_AMP_CTRL_RATIO_100	 |
										 GP_SPK_D_AMP_CTRL_RATIO_100 |
										 GP_SPK_AB_AMP_CTRL_RATIO_100)	
										,
										(GP_HP_AMP_CTRL_MASK		|
										GP_SPK_D_AMP_CTRL_MASK		|
										GP_SPK_AB_AMP_CTRL_MASK)
									 );
			break;

		case 2:
			bRetVal=WriteCodecRegMask(
										RT_GEN_CTRL_REG1,
										(GP_HP_AMP_CTRL_RATIO_100	 |
										 GP_SPK_D_AMP_CTRL_RATIO_125 |
										 GP_SPK_AB_AMP_CTRL_RATIO_125)	
										,
										(GP_HP_AMP_CTRL_MASK		|
										GP_SPK_D_AMP_CTRL_MASK		|
										GP_SPK_AB_AMP_CTRL_MASK)
									 );
			break;

		case 3:
			bRetVal=WriteCodecRegMask(
										RT_GEN_CTRL_REG1,
										(GP_HP_AMP_CTRL_RATIO_125	 |
										 GP_SPK_D_AMP_CTRL_RATIO_125 |
										 GP_SPK_AB_AMP_CTRL_RATIO_125)	
										,
										(GP_HP_AMP_CTRL_MASK		|
										GP_SPK_D_AMP_CTRL_MASK		|
										GP_SPK_AB_AMP_CTRL_MASK)
									 );
			break;

		case 4:
			bRetVal=WriteCodecRegMask(
										RT_GEN_CTRL_REG1,
										(GP_HP_AMP_CTRL_RATIO_125	 |
										 GP_SPK_D_AMP_CTRL_RATIO_175 |
										 GP_SPK_AB_AMP_CTRL_RATIO_175)	
										,
										(GP_HP_AMP_CTRL_MASK		|
										GP_SPK_D_AMP_CTRL_MASK		|
										GP_SPK_AB_AMP_CTRL_MASK)
									 );
			break;

		default:
			return  FALSE;
	}
	return bRetVal;
}

//*****************************************************************************
//
//function:Config Microphone BIAS function
//
//*****************************************************************************
BOOL RT_CodecComm::ConfigMicBias(BYTE Mic,BYTE MicBiasCtrl)
{
	BOOL bRetVal=FALSE;

	if(Mic==MIC1)
	{
		if(MicBiasCtrl==MIC_BIAS_90_PRECNET_AVDD)
		  {	
			bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC1_BIAS_VOLT_CTRL_90P,MIC1_BIAS_VOLT_CTRL_MASK);
		  }
		 else if(MicBiasCtrl==MIC_BIAS_75_PRECNET_AVDD)
		  {	
			bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC1_BIAS_VOLT_CTRL_75P,MIC1_BIAS_VOLT_CTRL_MASK);
		  }			
	}
	else if(Mic==MIC2)
	{
		if(MicBiasCtrl==MIC_BIAS_90_PRECNET_AVDD)
		  {	
			bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC2_BIAS_VOLT_CTRL_90P,MIC2_BIAS_VOLT_CTRL_MASK);
		  }
		 else if(MicBiasCtrl==MIC_BIAS_75_PRECNET_AVDD)
		  {	
			bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC2_BIAS_VOLT_CTRL_75P,MIC2_BIAS_VOLT_CTRL_MASK);
		  }		
	}
	
	return 	bRetVal;
}

//*****************************************************************************
//function:Enable the Voice PCM interface Path
//*****************************************************************************
BOOL RT_CodecComm::ConfigPcmVoicePath(BOOL bEnableVoicePath,MODE_SEL mode)
{
	BOOL bRetVal=FALSE;

	if(bEnableVoicePath)
	 {
			//Power on Voice PCM I2S Digital interface
			WriteCodecRegMask(RT_PWR_MANAG_ADD1,PWR_MAIN_I2S,PWR_MAIN_I2S);
			//Power on Voice DAC/ADC 
			WriteCodecRegMask(RT_PWR_MANAG_ADD2,PWR_VOICE_CLOCK,PWR_VOICE_CLOCK);
			//routing voice to HPMixer
			WriteCodecRegMask(RT_VOICE_DAC_OUT_VOL,0,M_V_DAC_TO_HP_MIXER);
					


		switch(mode)		
		{
			case MASTER_MODE_A:	//8kHz sampling rate,16 bits PCM and master mode,PCM mode is A,MCLK=24.576MHz.
								//CSR PSKEY_PCM_CONFIG32 (HEX) = 0x08000006,PSKEY_FORMAT=0x0060,PCM_CLK=256K				

				//Enable GPIO 1,3,4,5 to voice interface
				//Set I2S to Master mode
				//Voice ADC Enable
				//Set voice i2s VBCLK Polarity to Invert
				//Set PCM mode to Mode A
				//Set Data length to 16 bit
				//set Data Fomrat to PCM format
				//the register 0x36 value's should is 0x8183
				bRetVal=ShadowWriteCodec(RT_EXTEND_SDP_CTRL,EXT_I2S_FUNC_ENABLE|EXT_I2S_VOICE_ADC_EN|EXT_I2S_BCLK_POLARITY|EXT_I2S_DL_16|EXT_I2S_DF_PCM);
				if(!bRetVal)
					goto exit;	

				//Set Voice MCLK from main MCLK
				//set voice SCLK select divide 12 and 8 
				//the register 0x64 value's should is 0x00B2
				bRetVal=ShadowWriteCodec(RT_VOICE_DAC_PCMCLK_CTRL1,VOICE_MCLK_SEL_MCLK_INPUT|VOICE_SCLK_DIV1_12|VOICE_SCLK_DIV2_8);
				if(!bRetVal)
					goto exit;


				//set Voice CLK filter Div 3 and 8
				//the register 0x66 value's should is 0x0022
				bRetVal=ShadowWriteCodec(RT_VOICE_DAC_PCMCLK_CTRL2,VOICE_CLK_FILTER_DIV2_3|VOICE_CLK_FILTER_DIV1_8);
				if(!bRetVal)
					goto exit;

		
				break;

			case MASTER_MODE_B:	//8kHz sampling rate,16 bits PCM and master mode,PCM mode is B,PLL out=24.5625MHz.
								//CSR PSKEY_PCM_CONFIG32 (HEX) = 0x08000002,PSKEY_FORMAT=0x0060,PCM_CLK=256K				

				//Enable GPIO 1,3,4,5 to voice interface
				//Set I2S to Master mode
				//Voice ADC Enable
				//Set voice i2s VBCLK Polarity to Invert
				//Set PCM mode to Mode B
				//Set Data length to 16 bit
				//set Data Fomrat to PCM format
				//the register 0x36 value's should is 0x81C3
				bRetVal=ShadowWriteCodec(RT_EXTEND_SDP_CTRL,EXT_I2S_FUNC_ENABLE|EXT_I2S_VOICE_ADC_EN|EXT_I2S_BCLK_POLARITY|EXT_I2S_PCM_MODE|EXT_I2S_DL_16|EXT_I2S_DF_PCM);
				if(!bRetVal)
					goto exit;	

				//Set Voice MCLK from PLL input
				//set voice SCLK select divide 12 and 8 
				//the register 0x64 value's should is 0x80B2
				bRetVal=ShadowWriteCodec(RT_VOICE_DAC_PCMCLK_CTRL1,VOICE_MCLK_SEL_PLL_OUTPUT|VOICE_SCLK_DIV1_12|VOICE_SCLK_DIV2_8);
				if(!bRetVal)
					goto exit;


				//set Voice CLK filter Div 3 and 8
				//the register 0x66 value's should is 0x0022
				bRetVal=ShadowWriteCodec(RT_VOICE_DAC_PCMCLK_CTRL2,VOICE_CLK_FILTER_DIV2_3|VOICE_CLK_FILTER_DIV1_8);
				if(!bRetVal)
					goto exit;

		
				break;

			case SLAVE_MODE_A:	//8kHz sampling rate,16 bits PCM and slave mode,PLL out=24.5625MHz.
								//CSR PSKEY_PCM_CONFIG32 (HEX) = 0x08400000,PSKEY_FORMAT=0x0060,PCM_CLK=512K				

				//Enable GPIO 1,3,4,5 to voice interface
				//Set I2S to slave mode
				//Voice ADC Enable
				//Set voice i2s VBCLK Polarity to Invert
				//Set PCM mode to Mode B
				//Set Data length to 16 bit
				//set Data Fomrat to PCM format
				//the register 0x36 value's should is 0xC1C3
				bRetVal=ShadowWriteCodec(RT_EXTEND_SDP_CTRL,EXT_I2S_FUNC_ENABLE|EXT_I2S_MODE_SEL|EXT_I2S_VOICE_ADC_EN|EXT_I2S_BCLK_POLARITY|EXT_I2S_PCM_MODE|EXT_I2S_DL_16|EXT_I2S_DF_PCM);
				if(!bRetVal)
					goto exit;	

				//Select Voice filter Clock source from VBCLK
				//Voice DA/AD filter select 64x
				//the register 0x66 value's should is 0x6000
				bRetVal=ShadowWriteCodec(RT_VOICE_DAC_PCMCLK_CTRL2,VOICE_FILTER_CLK_F_VBCLK | VOICE_AD_DA_FILTER_SEL_64X);
				if(!bRetVal)
					goto exit;
		
				break;

			case SLAVE_MODE_B:	//8kHz sampling rate,16 bits PCM and slave mode,PCM mode A
								//CSR PSKEY_PCM_CONFIG32 (HEX) = 0x08400014,PSKEY_FORMAT=0x0060,PCM_CLK=512K				

				//Enable GPIO 1,3,4,5 to voice interface
				//Set I2S to slave mode
				//Voice ADC Enable
				//Set voice i2s VBCLK Polarity to Invert
				//Set PCM mode to Mode A
				//Set Data length to 16 bit
				//set Data Fomrat to PCM format
				//the register 0x36 value's should is 0x81C3
				bRetVal=ShadowWriteCodec(RT_EXTEND_SDP_CTRL,EXT_I2S_FUNC_ENABLE|EXT_I2S_MODE_SEL|EXT_I2S_VOICE_ADC_EN|EXT_I2S_BCLK_POLARITY|EXT_I2S_DL_16|EXT_I2S_DF_PCM);
				if(!bRetVal)
					goto exit;	

				//Select Voice filter Clock source from VBCLK
				//Voice DA/AD filter select 64x
				//the register 0x66 value's should is 0x6000
				bRetVal=ShadowWriteCodec(RT_VOICE_DAC_PCMCLK_CTRL2,VOICE_FILTER_CLK_F_VBCLK | VOICE_AD_DA_FILTER_SEL_64X);
				if(!bRetVal)
					goto exit;
		
				break;

			default:
				//do nothing		
				break;

			}
		}
 		else
		{
			//Power down Voice PCM I2S Digital interface
			WriteCodecRegMask(RT_PWR_MANAG_ADD1,0,PWR_MAIN_I2S);
			//Power down Voice DAC/ADC 
			WriteCodecRegMask(RT_PWR_MANAG_ADD2,0,PWR_VOICE_CLOCK);
			//Disable Voice PCM interface	
			WriteCodecRegMask(RT_EXTEND_SDP_CTRL,0,EXT_I2S_FUNC_ENABLE);
		}
	
exit:	
	return bRetVal;
}
//*****************************************************************************
//function:Enable the PLL function
//*****************************************************************************
BOOL RT_CodecComm::EnablePLLPath(BOOL bEnablePLL,unsigned short int K,unsigned short int M,unsigned short int N)
{	
	unsigned short int usRegVal;
	BOOL bRetVal=FALSE;

	if(bEnablePLL)
	  {	

		bRetVal=ShadowWriteCodec(RT_POWERDOWN_CTRL_STAT,0x0);	//power on all PR bit
	
		bRetVal=WriteCodecRegMask(RT_PWR_MANAG_ADD2,PWR_MIXER_VREF,PWR_MIXER_VREF);	//power on Vref for All analog circuit
		
		bRetVal=WriteCodecRegMask(RT_PWR_MANAG_ADD1,PWR_MAIN_BIAS,PWR_MAIN_BIAS);	//power on main bias	
	
		usRegVal=PLL_CTRL_M_VAL(M) | PLL_CTRL_K_VAL(K) |PLL_CTRL_N_VAL(N);

		bRetVal=ShadowWriteCodec(RT_PLL_CTRL,usRegVal);

		//codec clock source from PLL output		
		bRetVal=WriteCodecRegMask(RT_GEN_CTRL_REG1,GP_CLK_FROM_PLL,GP_CLK_FROM_PLL);
		
		//Enable PLL Power
		bRetVal=WriteCodecRegMask(RT_PWR_MANAG_ADD2,PWR_PLL,PWR_PLL);	
	  }
	else
	  {
		//codec clock source from MCLK output		
		bRetVal=WriteCodecRegMask(RT_GEN_CTRL_REG1,0,GP_CLK_FROM_PLL);		

		//Disable PLL Power
		bRetVal=WriteCodecRegMask(RT_PWR_MANAG_ADD2,0,PWR_PLL);	
	  }	

	return bRetVal;
}

//*****************************************************************************
//
//function:Config Microphone Boost function
//
//*****************************************************************************
BOOL RT_CodecComm::ConfigMicBoost(BYTE Mic,MIC_BOOST_TYPE BoostType)
{
	BOOL bRetVal=FALSE;

	if(Mic==MIC1)
	{
		switch(BoostType)
		{
			//Bypass mic1 boost
			case BOOST_BYPASS:

				bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC1_BOOST_CONTROL_BYPASS,MIC1_BOOST_CONTROL_MASK);

			break;
			//Set mic1 boost to 20DB	
			case BOOST_20DB:

				bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC1_BOOST_CONTROL_20DB,MIC1_BOOST_CONTROL_MASK);

			break;
			//Set mic1 boost to 30DB
			case BOOST_30DB:

				bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC1_BOOST_CONTROL_30DB,MIC1_BOOST_CONTROL_MASK);

			break;
			//Set mic1 boost to 40DB
			case BOOST_40DB:

				bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC1_BOOST_CONTROL_40DB,MIC1_BOOST_CONTROL_MASK);	

			break;

			default:
				bRetVal=FALSE;
				
		}
	}
	else if(Mic==MIC2)
	{
		switch(BoostType)
		{
			//Bypass mic2 boost
			case BOOST_BYPASS:

				bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC2_BOOST_CONTROL_BYPASS,MIC2_BOOST_CONTROL_MASK);

			break;
			//Set mic2 boost to 20DB
			case BOOST_20DB:

				bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC2_BOOST_CONTROL_20DB,MIC2_BOOST_CONTROL_MASK);

			break;
			//Set mic2 boost to 30DB
			case BOOST_30DB:

				bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC2_BOOST_CONTROL_30DB,MIC2_BOOST_CONTROL_MASK);

			break;
			//Set mic2 boost to 40DB
			case BOOST_40DB:

				bRetVal=WriteCodecRegMask(RT_MICROPHONE_CTRL,MIC2_BOOST_CONTROL_40DB,MIC2_BOOST_CONTROL_MASK);

			break;

		  default:
			bRetVal=FALSE;
		}	
	}

	return 	bRetVal;
}

//*****************************************************************************
//
//function:Depop of HP Out
//
//*****************************************************************************
BOOL RT_CodecComm::DepopForHP(DEPOP_MODE Depop_mode)
{
	BOOL bRetVal=FALSE;	
	ShadowWriteCodec(RT_POWERDOWN_CTRL_STAT, 0);//power on all PR bit
	bRetVal=WriteCodecRegMask(RT_PWR_MANAG_ADD1,PWR_MAIN_BIAS,PWR_MAIN_BIAS);//enable main BIAS
	bRetVal=WriteCodecRegMask(RT_PWR_MANAG_ADD2,PWR_MIXER_VREF,PWR_MIXER_VREF);//enable Vref
	bRetVal=WriteCodecRegMask(RT_PWR_MANAG_ADD3,PWR_HP_L_OUT | PWR_HP_R_OUT,PWR_HP_L_OUT | PWR_HP_R_OUT);//enable Amp of HP_R and HP_L 

	DelayMSTime(2000);//delay 2 seconds

	bRetVal=WriteCodecRegMask(RT_PWR_MANAG_ADD1,PWR_HI_R_LOAD_HP,PWR_HI_R_LOAD_HP);	//De-pop Disable

	return bRetVal;

}
//*****************************************************************************
//
//function AudioOutEnable:Mute/Unmute audio out channel
//							WavOutPath:output channel
//							Mute :Mute/Unmute output channel											
//
//*****************************************************************************

BOOL RT_CodecComm::AudioOutEnable(WAVOUT_PATH WavOutPath,BOOL Mute)
{
	BOOL RetVal=FALSE;	

	if(Mute)
	{
		switch(WavOutPath)
		{
			case RT_WAVOUT_PATH_ALL:

				RetVal=WriteCodecRegMask(RT_SPEAKER_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel
				RetVal=WriteCodecRegMask(RT_HP_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
				RetVal=WriteCodecRegMask(RT_PHONEIN_MONO_OUT_VOL,RT_R_MUTE,RT_R_MUTE);				//Mute Mono channel
				RetVal=WriteCodecRegMask(RT_STEREO_DAC_VOL,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER
															  ,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//Mute DAC to HP,Speaker,Mono Mixer
		
			break;
		
			case RT_WAVOUT_PATH_HP:

				RetVal=WriteCodecRegMask(RT_HP_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
					
			break;

			case RT_WAVOUT_PATH_SPK:
				
				RetVal=WriteCodecRegMask(RT_SPEAKER_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel			

			break;
			

			case RT_WAVOUT_PATH_MONO:

				RetVal=WriteCodecRegMask(RT_PHONEIN_MONO_OUT_VOL,RT_R_MUTE,RT_R_MUTE);	//Mute MonoOut channel		

			break;

			case RT_WAVOUT_PATH_DAC:

				RetVal=WriteCodecRegMask(RT_STEREO_DAC_VOL,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER
															  ,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//Mute DAC to HP,Speaker,Mono Mixer				
			break;

			default:

				return FALSE;

		}
	}
	else
	{
		switch(WavOutPath)
		{

			case RT_WAVOUT_PATH_ALL:

				RetVal=WriteCodecRegMask(RT_SPEAKER_OUT_VOL		,0,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel
				RetVal=WriteCodecRegMask(RT_HP_OUT_VOL 		,0,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
				RetVal=WriteCodecRegMask(RT_PHONEIN_MONO_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);	//Mute Mono channel
				RetVal=WriteCodecRegMask(RT_STEREO_DAC_VOL	,0,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//Mute DAC to HP,Speaker,Mono Mixer
		
			break;
		
			case RT_WAVOUT_PATH_HP:

				RetVal=WriteCodecRegMask(RT_HP_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);	//unMute headphone right/left channel
					
			break;

			case RT_WAVOUT_PATH_SPK:
				
				RetVal=WriteCodecRegMask(RT_SPEAKER_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);	//unMute Speaker right/left channel			

			break;			

			case RT_WAVOUT_PATH_MONO:

				RetVal=WriteCodecRegMask(RT_PHONEIN_MONO_OUT_VOL,0,RT_R_MUTE);	//unMute MonoOut channel		

			break;

			case RT_WAVOUT_PATH_DAC:

				RetVal=WriteCodecRegMask(RT_STEREO_DAC_VOL,0,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//unMute DAC to HP,Speaker,Mono Mixer

			default:
				return FALSE;
		}

	}
	
	return RetVal;
}

//*****************************************************************************
//
//function:hardware EQ configuration
//
//*****************************************************************************
BOOL RT_CodecComm::EnableHwEq(HW_EQ_PRESET_TYPE Hw_Eq_Type,BOOL HwEqEnable)
{
	BOOL bRetVal=TRUE;
	int HwEqIndex;
	
	if(m_WaveOutSampleRate==KHZ48_000)
	{
		HwEq_Preset=HwEq_Preset_48k;
	}
	else 
	{
		HwEq_Preset=HwEq_Preset_44k;
	}
	

	if(HwEqEnable)
	{

		//Disable HW EQ setting
		bRetVal=WriteCodecAdvanceMask(HW_EQ_CONTROL,0,ENABLE_HW_EQ_BLOCK | ENABLE_HW_EQ_HPF | ENABLE_HW_EQ_BP3 | ENABLE_HW_EQ_BP2 | ENABLE_HW_EQ_BP1 | ENABLE_HW_EQ_LPF);

	
		//setting HW EQ Coefficient
		for(HwEqIndex=HW_EQ_LP0_A1;HwEqIndex<=HW_EQ_HP4_H0;HwEqIndex++)
		{			

			if(!WriteCodecAdvance((EXT_CODEC_INDEX)HwEqIndex,HwEq_Preset[Hw_Eq_Type].EqValue[HwEqIndex]))
			{
				bRetVal=FALSE;

				return bRetVal;
			}	
		}

		//enable Hardware EQ setting
		if(!WriteCodecAdvance(HW_EQ_CONTROL,HwEq_Preset[Hw_Eq_Type].HwEQCtrl))
			{
				bRetVal=FALSE;

				return bRetVal;
			}

		return bRetVal;	
	}
	else
	{
		//Disable HW EQ setting
		bRetVal=WriteCodecAdvanceMask(HW_EQ_CONTROL,0,ENABLE_HW_EQ_BLOCK | ENABLE_HW_EQ_HPF | ENABLE_HW_EQ_BP3 | ENABLE_HW_EQ_BP2 | ENABLE_HW_EQ_BP1 | ENABLE_HW_EQ_LPF);
			
		return bRetVal;
	}
}

//*****************************************************************************
//
//function:Change audio codec power status
//
//*****************************************************************************

BOOL RT_CodecComm::ChangeCodecPowerStatus(POWER_STATE power_state)
{
	unsigned short int PowerDownState=0;

	switch(power_state)
	{
		case POWER_STATE_D0:			//FULL ON-----power on all power

			ShadowWriteCodec(RT_POWERDOWN_CTRL_STAT,PowerDownState);
			ShadowWriteCodec(RT_PWR_MANAG_ADD1,~PowerDownState);
			ShadowWriteCodec(RT_PWR_MANAG_ADD2,~PowerDownState);
			ShadowWriteCodec(RT_PWR_MANAG_ADD3,~PowerDownState);

		break;	

		case POWER_STATE_D1:		//LOW ON-----																																												   
											
			WriteCodecRegMask(RT_PWR_MANAG_ADD2,PWR_R_ADC_REC_MIXER | PWR_L_ADC_REC_MIXER | PWR_R_HP_MIXER | PWR_L_HP_MIXER | PWR_R_ADC_CLK_GAIN | PWR_L_ADC_CLK_GAIN | 
												PWR_R_DAC_CLK | PWR_L_DAC_CLK | PWR_MIXER_VREF | PWR_CLASS_AB
											   ,PWR_R_ADC_REC_MIXER | PWR_L_ADC_REC_MIXER | PWR_R_HP_MIXER | PWR_L_HP_MIXER | PWR_R_ADC_CLK_GAIN | PWR_L_ADC_CLK_GAIN | 
												PWR_R_DAC_CLK | PWR_L_DAC_CLK | PWR_MIXER_VREF | PWR_CLASS_AB);

			WriteCodecRegMask(RT_PWR_MANAG_ADD3,PWR_MIC1_BOOST | PWR_MIC1_VOL_CTRL | PWR_SPK_R_OUT | PWR_SPK_L_OUT |PWR_HP_R_OUT | PWR_HP_L_OUT |
											    PWR_SPK_RN_OUT | PWR_SPK_LN_OUT
											   ,PWR_MIC1_BOOST | PWR_MIC1_VOL_CTRL | PWR_SPK_R_OUT | PWR_SPK_L_OUT |PWR_HP_R_OUT | PWR_HP_L_OUT	|
											    PWR_SPK_RN_OUT | PWR_SPK_LN_OUT);								

			WriteCodecRegMask(RT_PWR_MANAG_ADD1,PWR_DAC_REF | PWR_MAIN_BIAS | PWR_MIC_BIAS1  | PWR_HI_R_LOAD_HP	| PWR_MAIN_I2S	
											   ,PWR_DAC_REF | PWR_MAIN_BIAS | PWR_MIC_BIAS1  | PWR_HI_R_LOAD_HP | PWR_MAIN_I2S);
		break;

		case POWER_STATE_D1_PLAYBACK:	//Low on of Playback
											
			WriteCodecRegMask(RT_PWR_MANAG_ADD2,PWR_R_HP_MIXER | PWR_L_HP_MIXER | PWR_R_DAC_CLK | PWR_L_DAC_CLK  | PWR_CLASS_AB
											   ,PWR_R_HP_MIXER | PWR_L_HP_MIXER | PWR_R_DAC_CLK | PWR_L_DAC_CLK  | PWR_CLASS_AB);

			WriteCodecRegMask(RT_PWR_MANAG_ADD3,PWR_SPK_R_OUT | PWR_SPK_L_OUT | PWR_HP_R_OUT | PWR_HP_L_OUT	|
											    PWR_SPK_RN_OUT | PWR_SPK_LN_OUT
											   ,PWR_SPK_R_OUT | PWR_SPK_L_OUT | PWR_HP_R_OUT | PWR_HP_L_OUT	|
											    PWR_SPK_RN_OUT | PWR_SPK_LN_OUT);	

			WriteCodecRegMask(RT_PWR_MANAG_ADD1,PWR_DAC_REF | PWR_HI_R_LOAD_HP | PWR_MAIN_I2S,PWR_DAC_REF | PWR_HI_R_LOAD_HP | PWR_MAIN_I2S);

		break;

		case POWER_STATE_D1_RECORD:	//Low on of Record
											//
			WriteCodecRegMask(RT_PWR_MANAG_ADD2,PWR_R_ADC_REC_MIXER | PWR_L_ADC_REC_MIXER | PWR_R_ADC_CLK_GAIN | PWR_L_ADC_CLK_GAIN												
											   ,PWR_R_ADC_REC_MIXER | PWR_L_ADC_REC_MIXER | PWR_R_ADC_CLK_GAIN | PWR_L_ADC_CLK_GAIN);

			WriteCodecRegMask(RT_PWR_MANAG_ADD3,PWR_MIC1_BOOST | PWR_MIC1_VOL_CTRL,PWR_MIC1_BOOST | PWR_MIC1_VOL_CTRL);	

			WriteCodecRegMask(RT_PWR_MANAG_ADD1,PWR_MIC_BIAS1 | PWR_MAIN_I2S,PWR_MIC_BIAS1 | PWR_MAIN_I2S);
		
		break;

		case POWER_STATE_D2:		//STANDBY----
											//																								
			WriteCodecRegMask(RT_PWR_MANAG_ADD1,0,PWR_DAC_REF | PWR_MIC_BIAS1 | PWR_HI_R_LOAD_HP);
											//
			WriteCodecRegMask(RT_PWR_MANAG_ADD2,0,PWR_R_ADC_REC_MIXER | PWR_L_ADC_REC_MIXER | PWR_R_HP_MIXER | PWR_L_HP_MIXER | PWR_R_ADC_CLK_GAIN | PWR_L_ADC_CLK_GAIN | 
												PWR_R_DAC_CLK | PWR_L_DAC_CLK  | PWR_CLASS_AB);

			WriteCodecRegMask(RT_PWR_MANAG_ADD3,0,PWR_MIC1_BOOST | PWR_MIC1_VOL_CTRL | PWR_SPK_R_OUT | PWR_SPK_L_OUT |PWR_HP_R_OUT | PWR_HP_L_OUT |
											    PWR_SPK_RN_OUT | PWR_SPK_LN_OUT);	
				
		break;

		case POWER_STATE_D2_PLAYBACK:	//STANDBY of playback

			WriteCodecRegMask(RT_PWR_MANAG_ADD1,0,PWR_DAC_REF | PWR_HI_R_LOAD_HP);
											//
			WriteCodecRegMask(RT_PWR_MANAG_ADD2,0,PWR_R_HP_MIXER | PWR_L_HP_MIXER | PWR_R_DAC_CLK | PWR_L_DAC_CLK  | PWR_CLASS_AB);

			WriteCodecRegMask(RT_PWR_MANAG_ADD3,0,PWR_SPK_R_OUT | PWR_SPK_L_OUT | PWR_HP_R_OUT | PWR_HP_L_OUT |
											    PWR_SPK_RN_OUT | PWR_SPK_LN_OUT);

		break;

		case POWER_STATE_D2_RECORD:		//STANDBY of record

			WriteCodecRegMask(RT_PWR_MANAG_ADD1,0,PWR_MIC_BIAS1);
											//
			WriteCodecRegMask(RT_PWR_MANAG_ADD2,0,PWR_R_ADC_REC_MIXER | PWR_L_ADC_REC_MIXER | PWR_R_ADC_CLK_GAIN | PWR_L_ADC_CLK_GAIN);

			WriteCodecRegMask(RT_PWR_MANAG_ADD3,0,PWR_MIC1_BOOST | PWR_MIC1_VOL_CTRL);	

		break;		

		case POWER_STATE_D3:		//SLEEP
		case POWER_STATE_D4:		//OFF----power off all power,include PR0,PR1,PR3,PR4,PR5,PR6,EAPD,and addition power managment
			ShadowWriteCodec(RT_PWR_MANAG_ADD1,PowerDownState);
			ShadowWriteCodec(RT_PWR_MANAG_ADD2,PowerDownState);
			ShadowWriteCodec(RT_PWR_MANAG_ADD3,PowerDownState);

			PowerDownState=RT_PWR_PR0 | RT_PWR_PR1 | RT_PWR_PR2 | RT_PWR_PR3 /*| RT_PWR_PR4*/ | RT_PWR_PR5 | RT_PWR_PR6 | RT_PWR_PR7; 		
			ShadowWriteCodec(RT_POWERDOWN_CTRL_STAT,PowerDownState);		
				
		break;	

		default:

		break;
	}

	return TRUE;	
}

//*****************************************************************************
//
//function:Process private message of codec
//
//*****************************************************************************

DWORD RT_CodecComm::ProcessAudioMessage(UINT uMsg,DWORD dwParam1,DWORD dwParam2)
{
	
	S_3D_SPATIAL s3d;

   switch (uMsg)
    {
		
		//To enable/Disable and config HW EQ
        case WPDM_PRIVATE_CONFIG_HW_EQ:
		
			if(!EnableHwEq((HW_EQ_PRESET_TYPE)dwParam1,(BYTE)dwParam2))
				return(MMSYSERR_ERROR);

            return (MMSYSERR_NOERROR);
            break;

		//To enable 3D Spatial function
	   case WPDM_PRIVATE_ENABLE_3D:
			
			s3d.bEnable3D=TRUE;
			s3d.b3D_Gain=(BYTE)dwParam1;
			s3d.b3D_Ratio=(BYTE)dwParam2;
			

			if(!Enable_3D_Spatial(s3d))
				return(MMSYSERR_ERROR);

            return (MMSYSERR_NOERROR);		

			break;
		
		//To disable 3D Spatial	function
	   case WPDM_PRIVATE_DISABLE_3D:
			
			s3d.bEnable3D=FALSE;
			s3d.b3D_Gain=(BYTE)dwParam1;
			s3d.b3D_Ratio=(BYTE)dwParam2;				

			if(!Enable_3D_Spatial(s3d))
				return(MMSYSERR_ERROR);

            return (MMSYSERR_NOERROR);		

			break;

		//To enable/disable pseudo stereo function	
	   case WPDM_PRIVATE_CONFIG_PSEUDO:
			
			if(!Enable_Pseudo_Stereo((BYTE)dwParam1))
				return(MMSYSERR_ERROR);

            return (MMSYSERR_NOERROR);

			break;
		
		//To enable/disable Auto volume control	function
	   case WPDM_PRIVATE_CONFIG_AVC:		
		
			if(!EnableAVC((BYTE)dwParam1))
				return(MMSYSERR_ERROR);

            return (MMSYSERR_NOERROR);

			break;
	}
	
	return(MMSYSERR_NOTSUPPORTED);
}
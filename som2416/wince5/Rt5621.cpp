
//-----------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include "Rt5621.h"


//*************************************************************************************************************
//initize codec register
#define DAVID_ADD_CODE 1

CodecRegister Set_Codec_Reg_Init[]=
{
	{RT5621_SPK_OUT_VOL			,0x0000},//default speaker to 0DB
	{RT5621_HP_OUT_VOL			,0x0808},//default HP to -12DB
	{RT5621_ADC_REC_MIXER		,0x3F3F},//default Record is MicIn
	{RT5621_STEREO_DAC_VOL		,0x0808},//default stereo DAC volume
	{RT5621_MICROPHONE_CTRL		,0x0400},//set boost to +20DB
	{RT5621_AUDIO_INTERFACE		,0x8000},//set I2S codec to slave mode

#if USE_CLASS_AB_SPEAKER	

	{RT5621_OUTPUT_MIXER_CTRL	,0x4740},//default output mixer control,CLASS AB

#else

	{RT5621_OUTPUT_MIXER_CTRL	,0xA740},//default output mixer control,CLASS D	
	{RT5621_HID_CTRL_INDEX	,0x46},	
	{RT5621_HID_CTRL_DATA	,0xFFFF},//Power on all bit of  Class D
	
#endif


#if 0//if you need use AVC function,please fill below initize value of AVC,below value only be refer

	{RT5621_HID_CTRL_INDEX	,0x0021},//Auto Volume Control Register 1 
	{RT5621_HID_CTRL_DATA	,0x0500},//Maximum PCM absolute level
	{RT5621_HID_CTRL_INDEX	,0x0022},//Auto Volume Control Register 2 
	{RT5621_HID_CTRL_DATA	,0x0400},//Mimimum PCM absolute level
	{RT5621_HID_CTRL_INDEX	,0x0023},//Auto Volume Control Register 3 
	{RT5621_HID_CTRL_DATA	,0x0050},//Non-active PCM absolute level
	{RT5621_HID_CTRL_INDEX	,0x0024},//Auto Volume Control Register 4 
	{RT5621_HID_CTRL_DATA	,0x01FF},//control the sensitivity to increase Gain
	{RT5621_HID_CTRL_INDEX	,0x0025},//Auto Volume Control Register 5 
	{RT5621_HID_CTRL_DATA	,0x0200},//control the sensitivity to decrease Gain

#endif	

 //[david.modify] 2008-05-30 10:31
 //=========================
#ifdef DAVID_ADD_CODE
	{RT5621_JACK_DET_CTRL, 0x4810},
#endif
 //=========================
};

#define SET_CODEC_REG_INIT_NUM	(sizeof(Set_Codec_Reg_Init)/sizeof(CodecRegister))

//*************************************************************************************************************
//Hardware EQ 

HW_EQ_PRESET *HwEq_Preset;

HW_EQ_PRESET HwEq_Preset_48k[]={
//		0x0	0x1	0x2  0x3   0x4	0x5	0x6	0x7	0x8	0x9	0xa	0xb	0xc 0x62			
	{CLUB,{0x1F2C,0x13D1,0xC1CB,0x1E5D,0x0C73,0xCD47,0x188D,0x0C73,0xC3B5,0x1CD0,0x0C73,0x1DF8,0x2FB2},0x800E},
	{DANCE,{0x1F2C,0x13D1,0xC070,0x1F95,0x0BCE,0xCA43,0x1A29,0xFA73,0xDEDF,0x0ED8,0xF8B1,0x1DF8,0x2FB2},0x800F},
	{LIVE,{0x1E74,0xFA92,0xC249,0x1DF8,0x2298,0xC882,0x1C10,0x0D73,0xDA40,0x1561,0x0556,0x1DF8,0x2FB2},0x800F},
	{POP,{0x1EB5,0xFCB5,0xC1D3,0x1E5D,0x1FC4,0xD9D7,0x15F6,0xFB53,0xFFFF,0x06D3,0xF804,0x1DF8,0x2FB2},0x800F},
	{ROCK,{0x1EB5,0xFCB5,0xC070,0x1F95,0x0556,0xC3D8,0x1C56,0xF6E7,0x0C5D,0x0FC7,0x4030,0x1DF8,0x2FB2},0x800F},
};

HW_EQ_PRESET HwEq_Preset_44k[]={
//			0x0		0x1		0x2	  0x3	0x4		0x5		0x6		0x7	0x8		0x9		0xa	0xb		0xc		0x62			
	{CLUB ,{0x1DCC,0x13D1,0xC289,0x1E39,0x0C73,0xE1A2,0x17F8,0x0C73,0xC5E5,0x1C8B,0x0C73,0x1180,0x2FB2},0x800E},
	{DANCE,{0x1DCC,0x13D1,0xC08E,0x1F8C,0x0BCE,0xCB7E,0x19B2,0xFA73,0x0655,0x0DBB,0xF8B1,0x1180,0x2FB2},0x800F},
	{LIVE ,{0x1CB9,0xFA92,0xC36D,0x1DCC,0x2298,0xD8CA,0x1BBC,0x0D73,0x0748,0x1496,0x0556,0x1180,0x2FB2},0x800F},
	{POP  ,{0x1D40,0xFCB5,0xC2AE,0x1E39,0x1FC4,0x075E,0x1537,0xFB53,0x23D6,0x056C,0xF804,0x1180,0x2FB2},0x800F},
	{ROCK ,{0x1D40,0xFCB5,0xC08E,0x1F8C,0x0556,0xC4D7,0x1C08,0xF6E7,0x2CAB,0x0EA5,0x4030,0x1180,0x2FB2},0x800F},
};


//*************************************************************************************************************
//store/restore register in the suspend/resume mode
BYTE RestoreRegIndex[]={
						RT5621_SPK_OUT_VOL,			//0x02
						RT5621_HP_OUT_VOL,			//0x04
						RT5621_MONO_AUX_OUT_VOL,	//0x06
						RT5621_AUXIN_VOL,			//0x08
						RT5621_LINE_IN_VOL,			//0x0A
						RT5621_STEREO_DAC_VOL,		//0x0C
						RT5621_MIC_VOL,				//0x0E
						RT5621_MIC_ROUTING_CTRL,	//0x10
						RT5621_ADC_REC_GAIN,		//0x12
						RT5621_ADC_REC_MIXER,		//0x14
						RT5621_OUTPUT_MIXER_CTRL,	//0x1C
						RT5621_MICROPHONE_CTRL,		//0x22			
						RT5621_AUDIO_INTERFACE,		//0x34
						RT5621_MISC_CTRL,			//0X5E
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
						AD_DA_MIXER_IR5,			//0x54
						};

#define RESTORE_EXTREG_INDEX_NUM	(sizeof(RestoreExtRegIndex)/sizeof(EXT_CODEC_INDEX)) 

unsigned short int 	ExtCodecShadowRegisters[RESTORE_EXTREG_INDEX_NUM];

//------------------------------------------------------------------------
// function implement
//------------------------------------------------------------------------

RT5621_Codec::RT5621_Codec(void)
{
	
}

RT5621_Codec::~RT5621_Codec(void)
{
	
}

BOOL RT5621_Codec::Init(HardwareContext *HwCxt)
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

void RT5621_Codec::DelayMSTime(unsigned int MilliSec)
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
BOOL RT5621_Codec::InitRTCodecReg(void)
{
	int i;
	BOOL bRetVal=FALSE;

	WriteCodecRegMask(RT5621_PWR_MANAG_ADD3,PWR_MAIN_BIAS |PWR_HP_R_OUT_VOL |PWR_HP_L_OUT_VOL	//enable main bias and HP L/R AMP
										   ,PWR_MAIN_BIAS |PWR_HP_R_OUT_VOL |PWR_HP_L_OUT_VOL);	
	WriteCodecRegMask(RT5621_PWR_MANAG_ADD2,PWR_VREF,PWR_VREF);				//enable Vref
		
			
	for(i=0;i<SET_CODEC_REG_INIT_NUM;i++)
	{
		bRetVal=ShadowWriteCodec( Set_Codec_Reg_Init[i].CodecIndex,Set_Codec_Reg_Init[i].wCodecValue); 

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
void RT5621_Codec::SaveCodecRegToShadow()
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
void RT5621_Codec::RestoreRegToCodec()
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
void RT5621_Codec::SaveCodecExtRegToShadow()
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
void RT5621_Codec::ReStoreExtRegToCodec()
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
//function:shadow read register from codec or shadow memory
//
//*****************************************************************************
BOOL RT5621_Codec::ShadowReadCodec(BYTE Offset, unsigned short int *Data)
{
	BOOL b_retVal;
	
	if(!m_pHWContext)
	{
		return FALSE;
	}
	
	b_retVal=m_pHWContext->SafeReadCodec(Offset,Data);
	
	//if you use shadow method,please add code to this	

	return b_retVal;
}
//*****************************************************************************
//
//function:restore shadow write register to codec or shadow memory
//
//*****************************************************************************
BOOL RT5621_Codec::ShadowWriteCodec(BYTE Offset, unsigned short int Data)
{
	BOOL b_retVal;
	
	if(!m_pHWContext)
	{
		return FALSE;	
	}	

	//if you use shadow method,please add code to this and mark below line 	

	b_retVal=m_pHWContext->SafeWriteCodec(Offset,Data);

	return b_retVal;
}

//*****************************************************************************
//
//function:write codec register with mask
//
//*****************************************************************************
BOOL RT5621_Codec::WriteCodecRegMask(BYTE Offset, unsigned short int Data,unsigned short int Mask)
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
BOOL RT5621_Codec::WriteCodecAdvance(EXT_CODEC_INDEX Ext_Index,unsigned short int Ext_Data)
{
	BOOL RetVal=FALSE;
			
		RetVal=ShadowWriteCodec(RT5621_HID_CTRL_INDEX,Ext_Index);	// write codec advance index

		if(RetVal)
		{
			RetVal=ShadowWriteCodec(RT5621_HID_CTRL_DATA,Ext_Data);//write codec advance data
		}		
			
		return  RetVal;	
			
}
//*****************************************************************************
//
//function:Read advance register of codec
//
//*****************************************************************************
BOOL RT5621_Codec::ReadCodecAdvance(EXT_CODEC_INDEX Ext_Index,unsigned short int *Data)
{
	BOOL RetVal=FALSE;		

	RetVal=ShadowWriteCodec(RT5621_HID_CTRL_INDEX,Ext_Index);	// write codec advance index		

	if(RetVal)
	 {
		RetVal=ShadowReadCodec(RT5621_HID_CTRL_DATA,Data);	// read codec advance data			
	 }

	return RetVal;
}
//*****************************************************************************
//
//function:Write advance register of codec compare with Mask bit
//
//*****************************************************************************

BOOL RT5621_Codec::WriteCodecAdvanceMask(EXT_CODEC_INDEX Ext_Index,unsigned short int Ext_Data,unsigned short int Ext_Data_Mask)
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
BOOL RT5621_Codec::Enable_Main_Spatial(BOOL Enable_Main_Spatial)
{
	BOOL bRetVal=FALSE;
	
	if(Enable_Main_Spatial)
	{
		//Enable Main Spatial 
		bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,SPATIAL_CTRL_EN,SPATIAL_CTRL_EN);		
	}
	else
	{
		//Disable Main Spatial 
		bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,0,SPATIAL_CTRL_EN);	
	}

	return bRetVal;
}
//*****************************************************************************
//
//Function:Enable/Disable and setting 3D spatial control function(use this function,enable Main Spatial function first)
//
//*****************************************************************************

BOOL RT5621_Codec::Set_3D_Func(SET_3D_PARA_MODE S_3d_para,BYTE para)
{
	BOOL bRetVal=FALSE;
	
	switch(S_3d_para)
	{
		case SET_3D_L_GAIN:

			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,(para<<9),GAIN_3D_PARA_L_MASK);//set 3D left gain

		break;

		case SET_3D_R_GAIN:

			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,(para<<6),GAIN_3D_PARA_R_MASK);//set 3D right gain

		break;

		case SET_3D_L_RATIO:	

			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,(para<<4),RATIO_3D_L_MASK);	//set 3D left ratio

		break;

		case SET_3D_R_RATIO:

			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,(para<<2),RATIO_3D_R_MASK);	//set 3D right ratio

		break;		

		case SET_3D_ENABLE:

			Enable_Main_Spatial(TRUE);
			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,STEREO_EXPENSION_EN,STEREO_EXPENSION_EN);	//enable 3D spatial

		break;

		case SET_3D_DISABLE:

			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,0,STEREO_EXPENSION_EN);				//disable 3D spatial

		break;

		default:
			return FALSE;
	}

	return bRetVal;
}
//*****************************************************************************
//
//function:Enable/Disable Pseudio Stereo function(use this function,enable Main Spatial function first)
//
//*****************************************************************************
BOOL RT5621_Codec::Enable_Pseudo_Stereo(BOOL Enable_Pseudo_Stereo)
{
	BOOL bRetVal=FALSE;

	if(Enable_Pseudo_Stereo)
	{
		Enable_Main_Spatial(TRUE);
		Enable_All_Pass_Filter(TRUE);
		//Enable Pseudio stereo 
		bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,PSEUDO_STEREO_EN,PSEUDO_STEREO_EN);
	}
	else
	{
		Enable_All_Pass_Filter(FALSE);	
		//Disable Pseudio stereo
		bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,0,PSEUDO_STEREO_EN);
		
	}

	return bRetVal;
}

//*****************************************************************************
//
//function:Enable/Disable All Pass Filter function(use this function,enable Main Spatial function first)
//
//*****************************************************************************
BOOL RT5621_Codec::Enable_All_Pass_Filter(BOOL Enable_APF)
{
	BOOL bRetVal=FALSE;
	
	if(Enable_APF)
	{
		Enable_Main_Spatial(TRUE);

		//set parameter a1 support 48K	
		if(KHZ48_000==m_WaveOutSampleRate)
		{				
			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,APF_FOR_48K,APF_MASK);
		}
		else if(KHZ44_100==m_WaveOutSampleRate)//set parameter a1 support 44.1K	
		{
			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,APF_FOR_44_1K,APF_MASK);
		}
		else
		{
			//set parameter a1 support 32k and lower	
			bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,APF_FOR_32K,APF_MASK);
		}	

		//Enable All Pass Filter 
		bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,ALL_PASS_FILTER_EN,ALL_PASS_FILTER_EN);
	}
	else
	{
		//Disable All Pass Filter
		bRetVal=WriteCodecRegMask(RT5621_PSEDUEO_SPATIAL_CTRL,0,ALL_PASS_FILTER_EN);
	}

	return bRetVal;
}
//*****************************************************************************
//
//function:Enable/Disable ADC input source control
//
//*****************************************************************************
BOOL RT5621_Codec::Enable_ADC_Input_Source(unsigned short int ADC_Input_Sour,BOOL Enable)
{
	BOOL bRetVal=FALSE;
	
	if(Enable)
	{
		//Enable ADC source 
		bRetVal=WriteCodecRegMask(RT5621_ADC_REC_MIXER,0,ADC_Input_Sour);
	}
	else
	{
		//Disable ADC source		
		bRetVal=WriteCodecRegMask(RT5621_ADC_REC_MIXER,ADC_Input_Sour,ADC_Input_Sour);
	}

	return bRetVal;
}

//*****************************************************************************
//
//function:Enable/Disable Auto Volume Control function
//
//*****************************************************************************
BOOL RT5621_Codec::EnableAVC(BOOL Enable_AVC)
{
	BOOL bRetVal=FALSE;
	
	if(Enable_AVC)
	{			
		//enable AVC target select
		WriteCodecRegMask(RT5621_AVC_CTRL,AVC_TARTGET_SEL_L,AVC_TARTGET_SEL_MASK); 

		//Enable AVC function
		bRetVal=WriteCodecRegMask(RT5621_AVC_CTRL,AVC_ENABLE,AVC_ENABLE);
		
	}
	else
	{
		//Disable AVC function
		bRetVal=WriteCodecRegMask(RT5621_AVC_CTRL,0,AVC_ENABLE);
	}

	return bRetVal;
}
//*****************************************************************************
//
//function:Config SPEAKER AB&D Vmid ratio Control function
//
//*****************************************************************************
BOOL RT5621_Codec::ConfigVmidOutput(BYTE SPK_Type,unsigned short int VMID_RATIO)
{

	BOOL bRetVal=FALSE;

	switch(SPK_Type)
	{
		case SPK_CLASS_AB:
			bRetVal=WriteCodecRegMask(
										RT5621_ADD_CTRL_REG,
										VMID_RATIO,
										SPK_AB_AMP_CTRL_MASK
									 );
			break;

		case SPK_CLASS_D:
			bRetVal=WriteCodecRegMask(
										RT5621_ADD_CTRL_REG,
										VMID_RATIO,
										SPK_D_AMP_CTRL_MASK
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
BOOL RT5621_Codec::ConfigMicBias(BYTE Mic,BYTE MicBiasCtrl)
{
	BOOL bRetVal=FALSE;

	if(Mic==MIC1)
	{
		if(MicBiasCtrl==MIC_BIAS_90_PRECNET_AVDD)
		  {	
			bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MICBIAS_VOLT_CTRL_90P,MICBIAS_VOLT_CTRL_MASK);
		  }
		 else if(MicBiasCtrl==MIC_BIAS_75_PRECNET_AVDD)
		  {	
			bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MICBIAS_VOLT_CTRL_75P,MICBIAS_VOLT_CTRL_MASK);
		  }			
	}

	
	return 	bRetVal;
}

//*****************************************************************************
//function:Enable the PLL function
//*****************************************************************************
BOOL RT5621_Codec::EnablePLLPath(BOOL bEnablePLL,unsigned short int K,unsigned short int M,unsigned short int N)
{	
	unsigned short int usRegVal;
	BOOL bRetVal=FALSE;

	if(bEnablePLL)
	  {		
		bRetVal=WriteCodecRegMask(RT5621_PWR_MANAG_ADD2,PWR_VREF,PWR_VREF);	//power on Vref for All analog circuit
		
		bRetVal=WriteCodecRegMask(RT5621_PWR_MANAG_ADD3,PWR_MAIN_BIAS,PWR_MAIN_BIAS);	//power on main bias	
	
		usRegVal=PLL_CTRL_M_VAL(M) | PLL_CTRL_K_VAL(K) |PLL_CTRL_N_VAL(N);

		bRetVal=ShadowWriteCodec(RT5621_PLL_CTRL,usRegVal);

		//codec clock source from PLL output		
		bRetVal=WriteCodecRegMask(RT5621_GLOBAL_CLK_CTRL_REG,SYSCLK_SOUR_SEL_PLL,SYSCLK_SOUR_SEL_MASK);
		
		//Disable PLL Power		
		bRetVal=WriteCodecRegMask(RT5621_PWR_MANAG_ADD2,0,PWR_PLL);	
		//Enable PLL Power
		bRetVal=WriteCodecRegMask(RT5621_PWR_MANAG_ADD2,PWR_PLL,PWR_PLL);	

	  }
	else
	  {
		//codec clock source from MCLK output		
		bRetVal=WriteCodecRegMask(RT5621_GLOBAL_CLK_CTRL_REG,SYSCLK_SOUR_SEL_MCLK,SYSCLK_SOUR_SEL_MASK);		
		//Disable PLL Power
		bRetVal=WriteCodecRegMask(RT5621_PWR_MANAG_ADD2,0,PWR_PLL);	
	  }	

	return bRetVal;
}
//*****************************************************************************
//
//function:Config Microphone Boost function
//
//*****************************************************************************
BOOL RT5621_Codec::ConfigMicBoost(BYTE Mic,MIC_BOOST_TYPE BoostType)
{
	BOOL bRetVal=FALSE;

	if(Mic==MIC1)
	{
		switch(BoostType)
		{
			//Bypass mic1 boost
			case BOOST_BYPASS:

				bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MIC1_BOOST_CTRL_BYPASS,MIC1_BOOST_CTRL_MASK);

			break;
			//Set mic1 boost to 20DB	
			case BOOST_20DB:

				bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MIC1_BOOST_CTRL_20DB,MIC1_BOOST_CTRL_MASK);

			break;
			//Set mic1 boost to 30DB
			case BOOST_30DB:

				bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MIC1_BOOST_CTRL_30DB,MIC1_BOOST_CTRL_MASK);

			break;
			//Set mic1 boost to 40DB
			case BOOST_40DB:

				bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MIC1_BOOST_CTRL_40DB,MIC1_BOOST_CTRL_MASK);	

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

				bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MIC2_BOOST_CTRL_BYPASS,MIC2_BOOST_CTRL_MASK);

			break;
			//Set mic2 boost to 20DB
			case BOOST_20DB:

				bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MIC2_BOOST_CTRL_20DB,MIC2_BOOST_CTRL_MASK);

			break;
			//Set mic2 boost to 30DB
			case BOOST_30DB:

				bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MIC2_BOOST_CTRL_30DB,MIC2_BOOST_CTRL_MASK);

			break;
			//Set mic2 boost to 40DB
			case BOOST_40DB:

				bRetVal=WriteCodecRegMask(RT5621_MICROPHONE_CTRL,MIC2_BOOST_CTRL_40DB,MIC2_BOOST_CTRL_MASK);

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
void RT5621_Codec::DepopForHP(DEPOP_MODE Depop_mode)
{

	 if(DEPOP_MODE1_HP)
	{		
		WriteCodecRegMask(RT5621_PWR_MANAG_ADD3,PWR_MAIN_BIAS,PWR_MAIN_BIAS);	//enable Main Bias
		WriteCodecRegMask(RT5621_MISC_CTRL,HP_DEPOP_MODE1_EN,HP_DEPOP_MODE1_EN); //Enable Depop Mode 1
		WriteCodecRegMask(RT5621_PWR_MANAG_ADD3,PWR_HP_L_OUT_VOL|PWR_HP_R_OUT_VOL,PWR_HP_L_OUT_VOL|PWR_HP_R_OUT_VOL); //Enable HP volume AMP

		WriteCodecRegMask(RT5621_PWR_MANAG_ADD2,PWR_VREF ,PWR_VREF ); //Enable Vref
		DelayMSTime(1500);//delay 1.5S
		
		WriteCodecRegMask(RT5621_PWR_MANAG_ADD1,PWR_HP_AMP|PWR_HP_OUT,PWR_HP_AMP|PWR_HP_OUT); //Enable HP output buffer and HP AMP
		WriteCodecRegMask(RT5621_MISC_CTRL,0,HP_DEPOP_MODE1_EN); //Disable Depop Mode 1	
	}
	else if(DEPOP_MODE2_HP)
	{
		
		WriteCodecRegMask(RT5621_PWR_MANAG_ADD3,PWR_MAIN_BIAS,PWR_MAIN_BIAS);	//enable Main Bias
		WriteCodecRegMask(RT5621_MISC_CTRL,HP_DEPOP_MODE2_EN,HP_DEPOP_MODE2_EN); //Enable Depop Mode 2
		WriteCodecRegMask(RT5621_PWR_MANAG_ADD2,PWR_VREF,PWR_VREF); //Enable Vref
		WriteCodecRegMask(RT5621_PWR_MANAG_ADD3,PWR_HP_L_OUT_VOL|PWR_HP_R_OUT_VOL,PWR_HP_L_OUT_VOL|PWR_HP_R_OUT_VOL); //Enable HP volume AMP
		DelayMSTime(800);//delay 800 ms	
				
		WriteCodecRegMask(RT5621_PWR_MANAG_ADD1,PWR_HP_AMP|PWR_HP_OUT,PWR_HP_AMP|PWR_HP_OUT); //Enable HP output buffer and HP AMP		
		WriteCodecRegMask(RT5621_MISC_CTRL,0,HP_DEPOP_MODE2_EN); //Disable Depop Mode 2	
		
	}

}
//*****************************************************************************
//
//function AudioOutEnable:Mute/Unmute audio out channel
//							WavOutPath:output channel
//							Mute :Mute/Unmute output channel											
//
//*****************************************************************************
BOOL RT5621_Codec::AudioOutEnable(WAVOUT_PATH WavOutPath,BOOL Mute)
{
	BOOL RetVal=FALSE;	

	if(Mute)
	{
		switch(WavOutPath)
		{
			case RT_WAVOUT_PATH_ALL:

				RetVal=WriteCodecRegMask(RT5621_SPK_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel
				RetVal=WriteCodecRegMask(RT5621_HP_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
				RetVal=WriteCodecRegMask(RT5621_MONO_AUX_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute Aux/Mono right/left channel
				RetVal=WriteCodecRegMask(RT5621_STEREO_DAC_VOL,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER
															  ,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//Mute DAC to HP,Speaker,Mono Mixer
		
			break;
		
			case RT_WAVOUT_PATH_HP:

				RetVal=WriteCodecRegMask(RT5621_HP_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
					
			break;

			case RT_WAVOUT_PATH_SPK:
				
				RetVal=WriteCodecRegMask(RT5621_SPK_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel			

			break;
			
			case RT_WAVOUT_PATH_AUX:

				RetVal=WriteCodecRegMask(RT5621_MONO_AUX_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute AuxOut right/left channel

			break;

			case RT_WAVOUT_PATH_MONO:

				RetVal=WriteCodecRegMask(RT5621_MONO_AUX_OUT_VOL,RT_L_MUTE,RT_L_MUTE);	//Mute MonoOut channel		

			break;

			case RT_WAVOUT_PATH_DAC:

				RetVal=WriteCodecRegMask(RT5621_STEREO_DAC_VOL,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER
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

				RetVal=WriteCodecRegMask(RT5621_SPK_OUT_VOL		,0,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel
				RetVal=WriteCodecRegMask(RT5621_HP_OUT_VOL 		,0,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
				RetVal=WriteCodecRegMask(RT5621_MONO_AUX_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);	//Mute Aux/Mono right/left channel
				RetVal=WriteCodecRegMask(RT5621_STEREO_DAC_VOL	,0,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//Mute DAC to HP,Speaker,Mono Mixer
		
			break;
		
			case RT_WAVOUT_PATH_HP:

				RetVal=WriteCodecRegMask(RT5621_HP_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);	//unMute headphone right/left channel
					
			break;

			case RT_WAVOUT_PATH_SPK:
				
				RetVal=WriteCodecRegMask(RT5621_SPK_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);	//unMute Speaker right/left channel			

			break;
			
			case RT_WAVOUT_PATH_AUX:

				RetVal=WriteCodecRegMask(RT5621_MONO_AUX_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);//unMute AuxOut right/left channel

			break;

			case RT_WAVOUT_PATH_MONO:

				RetVal=WriteCodecRegMask(RT5621_MONO_AUX_OUT_VOL,0,RT_L_MUTE);	//unMute MonoOut channel		

			break;

			case RT_WAVOUT_PATH_DAC:

				RetVal=WriteCodecRegMask(RT5621_STEREO_DAC_VOL,0,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//unMute DAC to HP,Speaker,Mono Mixer

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
BOOL RT5621_Codec::EnableHwEq(HW_EQ_PRESET_TYPE Hw_Eq_Type,BOOL HwEqEnable)
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
		//enable Hardware EQ setting
		if(!ShadowWriteCodec(RT5621_EQ_CTRL,HwEq_Preset[Hw_Eq_Type].HwEQCtrl))
			{
				bRetVal=FALSE;

				return bRetVal;
			}


		//setting HW EQ Coefficient
		for(HwEqIndex=HW_EQ_LP0_A1;HwEqIndex<=HW_EQ_HP4_H0;HwEqIndex++)
		{			

			if(!WriteCodecAdvance((EXT_CODEC_INDEX)HwEqIndex,HwEq_Preset[Hw_Eq_Type].EqValue[HwEqIndex]))
			{
				bRetVal=FALSE;

				return bRetVal;
			}	
		}

		
		//Enable EQ Change mode
		if(!ShadowWriteCodec(RT5621_EQ_MODE_ENABLE,EQ_HPF_CHANGE_EN|EQ_BP3_CHANGE_EN|EQ_BP2_CHANGE_EN|EQ_BP1_CHANGE_EN|EQ_LPF_CHANGE_EN))
			{
				bRetVal=FALSE;

				return bRetVal;
			}
		
		//Disable EQ Change mode
		if(!ShadowWriteCodec(RT5621_EQ_MODE_ENABLE,0))
			{
				bRetVal=FALSE;

				return bRetVal;
			}

		return bRetVal;	
	}
	else
	{
		//Disable HW EQ setting
		bRetVal=WriteCodecRegMask(RT5621_EQ_CTRL,0,EN_HW_EQ_BLK | EN_HW_EQ_HPF | EN_HW_EQ_BP3 | EN_HW_EQ_BP2 | EN_HW_EQ_BP1 | EN_HW_EQ_LPF);
			
		return bRetVal;
	}
}

//*****************************************************************************
//
//function:Change audio codec power status
//
//*****************************************************************************

BOOL RT5621_Codec::ChangeCodecPowerStatus(POWER_STATE power_state)
{
	unsigned short int PowerDownState=0;

	switch(power_state)
	{
		case POWER_STATE_D0:			//FULL ON-----power on all power
			
			ShadowWriteCodec(RT5621_PWR_MANAG_ADD1,~PowerDownState);
			ShadowWriteCodec(RT5621_PWR_MANAG_ADD2,~PowerDownState);
			ShadowWriteCodec(RT5621_PWR_MANAG_ADD3,~PowerDownState);

		break;	

		case POWER_STATE_D1:		//LOW ON-----


			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,PWR_VREF |PWR_DAC_REF_CIR |PWR_L_DAC_CLK |PWR_R_DAC_CLK |PWR_L_HP_MIXER |PWR_R_HP_MIXER|
													 PWR_L_ADC_CLK_GAIN |PWR_R_ADC_CLK_GAIN |PWR_L_ADC_REC_MIXER |PWR_R_ADC_REC_MIXER
													,PWR_VREF |PWR_DAC_REF_CIR |PWR_L_DAC_CLK |PWR_R_DAC_CLK |PWR_L_HP_MIXER |PWR_R_HP_MIXER|
													 PWR_L_ADC_CLK_GAIN |PWR_R_ADC_CLK_GAIN |PWR_L_ADC_REC_MIXER |PWR_R_ADC_REC_MIXER);

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD3 ,PWR_MAIN_BIAS|PWR_HP_R_OUT_VOL|PWR_HP_L_OUT_VOL|PWR_SPK_R_OUT|PWR_SPK_RN_OUT|
													 PWR_MIC1_FUN_CTRL|PWR_MIC1_BOOST_MIXER
													,PWR_MAIN_BIAS|PWR_HP_R_OUT_VOL|PWR_HP_L_OUT_VOL|PWR_SPK_R_OUT|PWR_SPK_RN_OUT|
													 PWR_MIC1_FUN_CTRL|PWR_MIC1_BOOST_MIXER);			


			WriteCodecRegMask(RT5621_PWR_MANAG_ADD1 ,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_OUT|PWR_MIC1_BIAS_EN
													,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_OUT|PWR_MIC1_BIAS_EN);											   

#if	USE_CLASS_AB_SPEAKER		//Class AB

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,PWR_CLASS_AB,PWR_CLASS_AB);				
#else							//Class D
			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,PWR_CLASS_D,PWR_CLASS_D);		
#endif
									

						
		break;

		case POWER_STATE_D1_PLAYBACK:	//Low on of Playback


			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,PWR_VREF|PWR_DAC_REF_CIR|PWR_L_DAC_CLK|PWR_R_DAC_CLK|PWR_L_HP_MIXER|PWR_R_HP_MIXER
											  		,PWR_VREF|PWR_DAC_REF_CIR|PWR_L_DAC_CLK|PWR_R_DAC_CLK|PWR_L_HP_MIXER|PWR_R_HP_MIXER);

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD3 ,PWR_MAIN_BIAS|PWR_HP_R_OUT_VOL|PWR_HP_L_OUT_VOL|PWR_SPK_R_OUT|PWR_SPK_RN_OUT 
											   		,PWR_MAIN_BIAS|PWR_HP_R_OUT_VOL|PWR_HP_L_OUT_VOL|PWR_SPK_R_OUT|PWR_SPK_RN_OUT);		

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD1 ,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_OUT
											   		,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_OUT);											   
#if	USE_CLASS_AB_SPEAKER		//Class AB

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,PWR_CLASS_AB,PWR_CLASS_AB);				
#else							//Class D
			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,PWR_CLASS_D,PWR_CLASS_D);		
#endif
									

		break;

		case POWER_STATE_D1_RECORD:	//Low on of Record

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD1 ,PWR_MAIN_I2S_EN|PWR_MIC1_BIAS_EN
											   		,PWR_MAIN_I2S_EN|PWR_MIC1_BIAS_EN);											   
											
			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,PWR_VREF|PWR_L_ADC_CLK_GAIN|PWR_R_ADC_CLK_GAIN|PWR_L_ADC_REC_MIXER|PWR_R_ADC_REC_MIXER
											   		,PWR_VREF|PWR_L_ADC_CLK_GAIN|PWR_R_ADC_CLK_GAIN|PWR_L_ADC_REC_MIXER|PWR_R_ADC_REC_MIXER);

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD3 ,PWR_MAIN_BIAS|PWR_MIC1_FUN_CTRL|PWR_MIC1_BOOST_MIXER
											   		,PWR_MAIN_BIAS|PWR_MIC1_FUN_CTRL|PWR_MIC1_BOOST_MIXER);		
	
		break;

		case POWER_STATE_D2:		//STANDBY----

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD1 ,0,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_OUT|PWR_MIC1_BIAS_EN);											   

#if	USE_CLASS_AB_SPEAKER		//Class AB

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,0,PWR_CLASS_AB);				
#else							//Class D
			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,0,PWR_CLASS_D);		
#endif
														
			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,0,PWR_DAC_REF_CIR |PWR_L_DAC_CLK |PWR_R_DAC_CLK |PWR_L_HP_MIXER |PWR_R_HP_MIXER|
													   PWR_L_ADC_CLK_GAIN |PWR_R_ADC_CLK_GAIN |PWR_L_ADC_REC_MIXER |PWR_R_ADC_REC_MIXER);

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD3 ,0 ,PWR_HP_R_OUT_VOL|PWR_HP_L_OUT_VOL|PWR_SPK_R_OUT|PWR_SPK_RN_OUT|
														PWR_MIC1_FUN_CTRL|PWR_MIC1_BOOST_MIXER);
		
		break;

		case POWER_STATE_D2_PLAYBACK:	//STANDBY of playback

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD1 ,0,PWR_HP_AMP|PWR_HP_OUT);											   

#if	USE_CLASS_AB_SPEAKER	//Class AB 

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,0,PWR_CLASS_AB);				
#else						//Class D
			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,0,PWR_CLASS_D);		
#endif
									
			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,0,PWR_DAC_REF_CIR|PWR_L_DAC_CLK|PWR_R_DAC_CLK|PWR_L_HP_MIXER|PWR_R_HP_MIXER);

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD3 ,0,PWR_HP_R_OUT_VOL|PWR_HP_L_OUT_VOL|PWR_SPK_R_OUT|PWR_SPK_RN_OUT);

		break;

		case POWER_STATE_D2_RECORD:		//STANDBY of record

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD1 ,0,PWR_MIC1_BIAS_EN);											   
											
			WriteCodecRegMask(RT5621_PWR_MANAG_ADD2 ,0,PWR_L_ADC_CLK_GAIN|PWR_R_ADC_CLK_GAIN|PWR_L_ADC_REC_MIXER|PWR_R_ADC_REC_MIXER);

			WriteCodecRegMask(RT5621_PWR_MANAG_ADD3 ,0,PWR_MIC1_FUN_CTRL|PWR_MIC1_BOOST_MIXER);		

		break;		

		case POWER_STATE_D3:		//SLEEP
		case POWER_STATE_D4:		//OFF----power off all power
			ShadowWriteCodec(RT5621_PWR_MANAG_ADD1,0);
			ShadowWriteCodec(RT5621_PWR_MANAG_ADD2,0);
			ShadowWriteCodec(RT5621_PWR_MANAG_ADD3,0);		
				
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

DWORD RT5621_Codec::ProcessAudioMessage(UINT uMsg,DWORD dwParam1,DWORD dwParam2)
{
	
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
		//To disable 3D Spatial	function
	    case WPDM_PRIVATE_DISABLE_3D:		
		//To set 3D gain function
	    case WPDM_PRIVATE_SET_3D_PARAMETER:
				
			 if(!Set_3D_Func(SET_3D_PARA_MODE(dwParam1),(BYTE)dwParam2))
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

//-----------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include "RT5627.h"


//*************************************************************************************************************
//initize codec register

CodecRegister Set_Codec_Reg_Init[]=
{
	{RT5627_SPK_OUT_VOL			,0x8080},//default speaker to 0DB
	{RT5627_HP_OUT_VOL			,0x8888},//default HP to -12DB
	 //[david.modify] 2008-09-11 16:19
	 //===========================
	  //[david.modify] 2008-09-13 09:40
//目前参数设置为0X0C=1818，0X40=0700
//   （DAVID：我们实际参数请设为：0X0C=1010，0X40=0900，将功率调的失真一点，将偏压调为1.25倍，因是电池供电）

	 // 改成1616降低声音，减少失真
//	{RT5627_STEREO_DAC_VOL		,0x0808},//default stereo DAC set 0DB
//	{RT5627_STEREO_DAC_VOL		,0x1616},//default stereo DAC set 0DB
	{RT5627_STEREO_DAC_VOL		,0x1010},//default stereo DAC set 0DB
	 //===========================	
	{RT5627_AUDIO_DATA_CTRL		,0x8000},//set I2S codec to slave mode
	{RT5627_OUTPUT_MIXER_CTRL	,0x8b04},//default output mixer control
//	{RT5627_GEN_CTRL			,0x0b00},//set Class D Vmid ratio is 1VDD and DAC have high pass filter
	{RT5627_GEN_CTRL			,0x0900},//set Class D Vmid ratio is 1.25VDD and DAC have high pass filter

	{RT5627_DAC_CLK_CTRL	,0x2004},//set DAC filter to 384fs		//flove091008
	//当用AK7742时,不再设大声音
	//{RT5627_LINE_IN_VOL,         0xc0e0},//set LINE input volume,0xc0c0 f.w.lin

 //[david.modify] 2008-05-30 10:31
 //增加耳机检测配置
 //=========================
#if 1
	{RT5627_JACK_DET_CTRL, 0x4B00},
	{RT5627_GPIO_OUTPUT_PIN_CTRL, 0x0008},	
#endif
 //=========================	
};

#define SET_CODEC_REG_INIT_NUM	(sizeof(Set_Codec_Reg_Init)/sizeof(CodecRegister))

//*************************************************************************************************************
//store/restore register in the suspend/resume mode
BYTE RestoreRegIndex[]={
						RT5627_SPK_OUT_VOL,			//0x02
						RT5627_HP_OUT_VOL,			//0x04
						RT5627_AUX_OUT_VOL,			//0x06
						RT5627_AUXIN_VOL,			//0x08
						RT5627_LINE_IN_VOL,			//0x0A
						RT5627_STEREO_DAC_VOL,		//0x0C
						RT5627_OUTPUT_MIXER_CTRL,	//0x1C
						RT5627_AUDIO_DATA_CTRL,		//0x34
						RT5627_MISC2_CTRL,			//0X5E
					   };

#define RESTORE_REG_INDEX_NUM	(sizeof(RestoreRegIndex)/sizeof(BYTE))

unsigned short int 	CodecShadowRegisters[RESTORE_REG_INDEX_NUM];


BYTE RestoreExtRegIndex[]={

						DIG_INTER_REG

						};

#define RESTORE_EXTREG_INDEX_NUM	(sizeof(RestoreExtRegIndex)/sizeof(BYTE)) 

unsigned short int 	ExtCodecShadowRegisters[RESTORE_EXTREG_INDEX_NUM];

//------------------------------------------------------------------------
// function implement
//------------------------------------------------------------------------
RT5627::RT5627(HardwareContext *HwCxt)
{
	m_pHWContext=HwCxt;

	m_WaveOutSampleRate=SAMPLERATE;
	m_WaveInSampleRate=SAMPLERATE;
    
   #ifdef BSP_USBFN_UART0_NOHF
    BTMode=FALSE;
   #endif
}
//*************************************************************************************************
//
//function:Initize register of codec
//
//*************************************************************************************************
void RT5627::RT_Codec_Init()
{
	int i;
	BOOL bRetVal=FALSE;

	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3,PWR_MAIN_BIAS,PWR_MAIN_BIAS);		//power on main bias	
	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2,PWR_VREF,PWR_VREF);				//power on Vref for All analog circuit

	DepopForHP();	

//flove090408	EnableDepopHPMuUmute();	//flove073008

	//initize customize setting 		
	for(i=0;i<SET_CODEC_REG_INIT_NUM;i++)
	{
		bRetVal=RTWriteCodecReg( Set_Codec_Reg_Init[i].CodecIndex,Set_Codec_Reg_Init[i].wCodecValue); 

		if(!bRetVal)
			break;
	}	
 	
//flove082208_S
//flove 090508 RTWriteCodecReg(RT5627_MISC2_CTRL,0);
//flove082208_E	
	return;
}
//*************************************************************************************************
//
//function:Set Codec register at unloading of driver
//
//*************************************************************************************************
void RT5627::RT_Codec_DeInit()
{

	ChangeCodecPowerStatus(POWER_STATE_D4);

}
//*************************************************************************************************
//
//function:Set Codec register at the beginning of playback
//
//*************************************************************************************************		 
void RT5627::RT_PlayBack_Start(USHORT Channel_On,USHORT Channel_Off)
{
    //RETAILMSG(1,(TEXT("RT_PlayBack_Start;Channel_On:%x,Channel_Off:%x \r\n"), Channel_On, Channel_Off));

RTWriteCodecRegMask(RT5627_MISC2_CTRL,0,HP_DEPOP_MODE2_EN);		//disable depop2 for HP	//flove090508

EnableDepopHPMuUmute();//flove082208

//1.Power on playback DAC and output power path
	
	ChangeCodecPowerStatus(POWER_STATE_D1_PLAYBACK);

//2.Unmute playback DAC and output path
	if(Channel_On)
	{
		if(Channel_On&RT_WAVOUT_HP)	
			AudioOutEnable(RT_WAVOUT_HP,FALSE);
		
		if(Channel_On&RT_WAVOUT_AUXOUT)	
			AudioOutEnable(RT_WAVOUT_MONO,FALSE);			
			
		if(Channel_On&RT_WAVOUT_SPK)	
			AudioOutEnable(RT_WAVOUT_SPK,FALSE);		
		
	}

	if(Channel_Off)
	{
		if(Channel_Off&RT_WAVOUT_HP)	
			AudioOutEnable(RT_WAVOUT_HP,TRUE);
		
		if(Channel_Off&RT_WAVOUT_AUXOUT)	
			AudioOutEnable(RT_WAVOUT_MONO,TRUE);			
			
		if(Channel_Off&RT_WAVOUT_SPK)	
			AudioOutEnable(RT_WAVOUT_SPK,TRUE);						
	}

	//if open EnableDepopHPMuUmute() this function
	//pin 18(Cdepop) need connect capacitance 1u and need delay 30 ms sec to avoid HP pop noise
	Sleep(30);
RTWriteCodecReg(RT5627_MISC2_CTRL,0);//flove082208
}
//*************************************************************************************************
//
//function:Set Codec register at the ending of playback
//
//*************************************************************************************************
void RT5627::RT_PlayBack_End(USHORT Channel_On,USHORT Channel_Off)
{
    //RETAILMSG(1,(TEXT("RT_PlayBack_End,Channel_On:%x,Channel_Off:%x,bt:%d \r\n"), Channel_On, Channel_Off,BTMode ));

  #ifdef BSP_USBFN_UART0_NOHF
    //if in BT mode,don't power down and mute output
    if(BTMode)    return;
  #endif

EnableDepopHPMuUmute();//flove082208
//1.Mute plackback DAC and output path	
	if(Channel_On)
	{
		if(Channel_On&RT_WAVOUT_HP)	
			AudioOutEnable(RT_WAVOUT_HP,FALSE);
		
		if(Channel_On&RT_WAVOUT_AUXOUT)	
			AudioOutEnable(RT_WAVOUT_MONO,FALSE);			
			
		if(Channel_On&RT_WAVOUT_SPK)	
			AudioOutEnable(RT_WAVOUT_SPK,FALSE);		
		
	}

	if(Channel_Off)
	{
		if(Channel_Off&RT_WAVOUT_HP)	
			AudioOutEnable(RT_WAVOUT_HP,TRUE);
		
		if(Channel_Off&RT_WAVOUT_AUXOUT)	
			AudioOutEnable(RT_WAVOUT_MONO,TRUE);			
			
		if(Channel_Off&RT_WAVOUT_SPK)	
			AudioOutEnable(RT_WAVOUT_SPK,TRUE);						
	}

//2.Power off playback DAC and output path

	ChangeCodecPowerStatus(POWER_STATE_D2_PLAYBACK);

RTWriteCodecReg(RT5627_MISC2_CTRL,0);//flove082208

}
//*************************************************************************************************
//
//function:Set Codec register at the beginning of record
//
//*************************************************************************************************
void RT5627::RT_Record_Start(USHORT Channel_On,USHORT Channel_Off)
{
	//ALC5627 don't support ADC function	
}
//*************************************************************************************************
//
//function:Set Codec register at the ending of record
//
//*************************************************************************************************
void RT5627::RT_Record_End(USHORT Channel_On,USHORT Channel_Off)
{
//ALC5627 don't support ADC function
}
//*************************************************************************************************
//
//function:Set Codec register at the suspending of audio driver
//
//*************************************************************************************************
void RT5627::RT_Suspend(USHORT Channel_On,USHORT Channel_Off)
{

	ChangeCodecPowerStatus(POWER_STATE_D3);

}
//*************************************************************************************************
//
//function:Set Codec register at the resuming of audio driver
//
//*************************************************************************************************
void RT5627::RT_Resume(USHORT Channel_On,USHORT Channel_Off)
{

	RT_Codec_Init();

}
//*****************************************************************************
//
//function:Save codec register to shadow
//
//*****************************************************************************
void RT5627::SaveCodecRegToShadow()
{

	int i;

	for(i=0;i<RESTORE_REG_INDEX_NUM;i++)
	{		
		if(!RTReadCodecReg(RestoreRegIndex[i],&CodecShadowRegisters[i]))
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
void RT5627::RestoreRegToCodec()
{

	int i;

	for(i=0;i<RESTORE_REG_INDEX_NUM;i++)
	{
		if(!RTWriteCodecReg(RestoreRegIndex[i],CodecShadowRegisters[i]))
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
void RT5627::SaveCodecExtRegToShadow()
{

	int i;

	for(i=0;i<RESTORE_EXTREG_INDEX_NUM;i++)
	{
		if(!RTReadCodecIndex(RestoreExtRegIndex[i],&ExtCodecShadowRegisters[i]))
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
void RT5627::ReStoreExtRegToCodec()
{
	int i;

	for(i=0;i<RESTORE_EXTREG_INDEX_NUM;i++)
	{
		if(!RTWriteCodecIndex(RestoreExtRegIndex[i],ExtCodecShadowRegisters[i]))
		{
			return;
		}
	}	
}

//*****************************************************************************
//
//function:Enable/Disable Auto Volume Control function
//
//*****************************************************************************
BOOL RT5627::EnableAVC(BOOL Enable_AVC)
{
	BOOL bRetVal=FALSE;
	
	if(Enable_AVC)
	{			
		//enable AVC target select Left channel
		RTWriteCodecRegMask(RT5627_AVC_CTRL,AVC_TARTGET_SEL_L,AVC_TARTGET_SEL_MASK); 

		//Enable AVC function
		bRetVal=RTWriteCodecRegMask(RT5627_AVC_CTRL,AVC_ENABLE,AVC_ENABLE);
		
	}
	else
	{
		//Disable AVC function
		bRetVal=RTWriteCodecRegMask(RT5627_AVC_CTRL,0,AVC_ENABLE);
	}

	return bRetVal;
}

//*****************************************************************************
//function:Enable the PLL function
//*****************************************************************************
BOOL RT5627::EnablePLLPath(BOOL bEnablePLL,unsigned short int K,unsigned short int M,unsigned short int N)
{	
	unsigned short int usRegVal;
	BOOL bRetVal=FALSE;

	if(bEnablePLL)
	  {		
		bRetVal=RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2,PWR_VREF,PWR_VREF);	//power on Vref for All analog circuit
		
		bRetVal=RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3,PWR_MAIN_BIAS,PWR_MAIN_BIAS);	//power on main bias	
	
		usRegVal=PLL_CTRL_M_VAL(M) | PLL_CTRL_K_VAL(K) |PLL_CTRL_N_VAL(N);

		bRetVal=RTWriteCodecReg(RT5627_PLL_CTRL,usRegVal);

		//codec clock source from PLL output		
		bRetVal=RTWriteCodecRegMask(RT5627_GLOBAL_CLK_CTRL,SYSCLK_SOUR_SEL_PLL,SYSCLK_SOUR_SEL_MASK);
		
		//Enable PLL Power
		bRetVal=RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2,PWR_PLL,PWR_PLL);	

	  }
	else
	  {
		//codec clock source from MCLK output		
		bRetVal=RTWriteCodecRegMask(RT5627_GLOBAL_CLK_CTRL,SYSCLK_SOUR_SEL_MCLK,SYSCLK_SOUR_SEL_MASK);		
		//Disable PLL Power
		bRetVal=RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2,0,PWR_PLL);	
	  }	

	return bRetVal;
}

//*****************************************************************************
//
//function:Depop of HP Out(suppress pop noise for HP)
//
//*****************************************************************************
BOOL RT5627::DepopForHP()
{
			
//flove090408	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3,PWR_MAIN_BIAS,PWR_MAIN_BIAS);	//power on main Bias
//flove090408	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2,PWR_VREF,PWR_VREF);		//power on vref 
	
	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1,PWR_SOFTGEN_EN,PWR_SOFTGEN_EN); //power on softgen
	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3,PWR_HP_R_OUT_VOL_AMP|PWR_HP_L_OUT_VOL_AMP,PWR_HP_R_OUT_VOL_AMP|PWR_HP_L_OUT_VOL_AMP);//power HP L/R volume
	RTWriteCodecRegMask(RT5627_MISC2_CTRL,HP_DEPOP_MODE2_EN,HP_DEPOP_MODE2_EN);		//enable depop2 for HP
//flove090508	DelayMSTime(500);
//flove090508	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1,PWR_HP_AMP|PWR_HP_ENH_AMP,PWR_HP_AMP|PWR_HP_ENH_AMP);	//enable HP output
//flove090508	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1,0,PWR_SOFTGEN_EN); //power on softgen
//flove090508	RTWriteCodecRegMask(RT5627_MISC2_CTRL,0,HP_DEPOP_MODE2_EN);		//disable depop2 for HP
	return TRUE;
}
//*****************************************************************************
//
//function:Depop Mute/Ummute of HP Out
//
//*****************************************************************************
BOOL RT5627::EnableDepopHPMuUmute()
{

	BOOL bRetVal=FALSE;

	RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1,PWR_SOFTGEN_EN,PWR_SOFTGEN_EN); //power on softgen
	//enable mute/unmute depop for HP
	bRetVal=RTWriteCodecRegMask(RT5627_MISC2_CTRL,M_UM_DEPOP_EN|HP_R_M_UM_DEPOP_EN|HP_L_M_UM_DEPOP_EN
												 ,M_UM_DEPOP_EN|HP_R_M_UM_DEPOP_EN|HP_L_M_UM_DEPOP_EN);		

	return bRetVal;
}
//*****************************************************************************
//
//function AudioOutEnable:Mute/Unmute audio out channel
//							WavOutPath:output channel
//							Mute :Mute/Unmute output channel											
//
//*****************************************************************************
BOOL RT5627::AudioOutEnable(unsigned short int WavOutPath,BOOL Mute)
{
	BOOL RetVal=FALSE;	

	if(Mute)
	{
		switch(WavOutPath)
		{
			case RT_WAVOUT_ALL_ON:

				RetVal=RTWriteCodecRegMask(RT5627_SPK_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel
				RetVal=RTWriteCodecRegMask(RT5627_HP_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
				RetVal=RTWriteCodecRegMask(RT5627_AUX_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute AuxOut right/left channel
				RetVal=RTWriteCodecRegMask(RT5627_STEREO_DAC_VOL,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER
															  ,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//Mute DAC to HP,Speaker,Mono Mixer
		
			break;
		
			case RT_WAVOUT_HP:

				RetVal=RTWriteCodecRegMask(RT5627_HP_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
			
			break;

			case RT_WAVOUT_SPK:
				
				RetVal=RTWriteCodecRegMask(RT5627_SPK_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel			

			break;
			
			case RT_WAVOUT_AUXOUT:

				RetVal=RTWriteCodecRegMask(RT5627_AUX_OUT_VOL,RT_L_MUTE|RT_R_MUTE,RT_L_MUTE|RT_R_MUTE);	//Mute AuxOut right/left channel

			break;

			case RT_WAVOUT_DAC:

				RetVal=RTWriteCodecRegMask(RT5627_STEREO_DAC_VOL,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER
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

			case RT_WAVOUT_ALL_ON:

				RetVal=RTWriteCodecRegMask(RT5627_SPK_OUT_VOL		,0,RT_L_MUTE|RT_R_MUTE);	//Mute Speaker right/left channel
				RetVal=RTWriteCodecRegMask(RT5627_HP_OUT_VOL 		,0,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
				RetVal=RTWriteCodecRegMask(RT5627_AUX_OUT_VOL		,0,RT_L_MUTE|RT_R_MUTE);//unMute AuxOut right/left channel
				RetVal=RTWriteCodecRegMask(RT5627_STEREO_DAC_VOL	,0,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//Mute DAC to HP,Speaker,Mono Mixer
		
			break;
		
			case RT_WAVOUT_HP:

				RetVal=RTWriteCodecRegMask(RT5627_HP_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);	//Mute headphone right/left channel
						
			break;

			case RT_WAVOUT_SPK:
				
				RetVal=RTWriteCodecRegMask(RT5627_SPK_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);	//unMute Speaker right/left channel			

			break;
			
			case RT_WAVOUT_AUXOUT:

				RetVal=RTWriteCodecRegMask(RT5627_AUX_OUT_VOL,0,RT_L_MUTE|RT_R_MUTE);//unMute AuxOut right/left channel

			break;

			case RT_WAVOUT_DAC:

				RetVal=RTWriteCodecRegMask(RT5627_STEREO_DAC_VOL,0,RT_M_HP_MIXER|RT_M_SPK_MIXER|RT_M_MONO_MIXER);	//unMute DAC to HP,Speaker,Mono Mixer

			default:
				return FALSE;
		}

	}
	
	return RetVal;
}

//*****************************************************************************
//
//function:Change audio codec power status
//
//*****************************************************************************

void RT5627::ChangeCodecPowerStatus(BYTE power_state)
{
	unsigned short int PowerDownState=0;

	switch(power_state)
	{
		case POWER_STATE_D0:			//FULL ON-----power on all power
			
			RTWriteCodecReg(RT5627_PWR_MANAG_ADD1,~PowerDownState);
			RTWriteCodecReg(RT5627_PWR_MANAG_ADD2,~PowerDownState);
			RTWriteCodecReg(RT5627_PWR_MANAG_ADD3,~PowerDownState);

		break;	

		case POWER_STATE_D1:		//LOW ON-----

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2 ,PWR_VREF |PWR_DAC_REF_CIR |PWR_L_DAC_CLK |PWR_R_DAC_CLK|PWR_L_DAC_L_D_S|PWR_R_DAC_R_D_S |PWR_L_HP_MIXER |PWR_R_HP_MIXER|PWR_SPK_MIXER|PWR_CLASS_D
													  ,PWR_VREF |PWR_DAC_REF_CIR |PWR_L_DAC_CLK |PWR_R_DAC_CLK|PWR_L_DAC_L_D_S|PWR_R_DAC_R_D_S |PWR_L_HP_MIXER |PWR_R_HP_MIXER|PWR_SPK_MIXER|PWR_CLASS_D);

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3 ,PWR_MAIN_BIAS|PWR_HP_R_OUT_VOL_AMP|PWR_HP_L_OUT_VOL_AMP|PWR_SPK_OUT
													  ,PWR_MAIN_BIAS|PWR_HP_R_OUT_VOL_AMP|PWR_HP_L_OUT_VOL_AMP|PWR_SPK_OUT);


			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1 ,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_ENH_AMP
													  ,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_ENH_AMP);											   						
						
		break;

		case POWER_STATE_D1_PLAYBACK:	//Low on of Playback

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2 ,PWR_VREF|PWR_DAC_REF_CIR|PWR_L_DAC_CLK|PWR_R_DAC_CLK|PWR_L_DAC_L_D_S|PWR_R_DAC_R_D_S|PWR_L_HP_MIXER|PWR_R_HP_MIXER|PWR_SPK_MIXER|PWR_CLASS_D
											  		  ,PWR_VREF|PWR_DAC_REF_CIR|PWR_L_DAC_CLK|PWR_R_DAC_CLK|PWR_L_DAC_L_D_S|PWR_R_DAC_R_D_S|PWR_L_HP_MIXER|PWR_R_HP_MIXER|PWR_SPK_MIXER|PWR_CLASS_D);

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3 ,PWR_MAIN_BIAS|PWR_HP_R_OUT_VOL_AMP|PWR_HP_L_OUT_VOL_AMP|PWR_SPK_OUT 
											   		,PWR_MAIN_BIAS|PWR_HP_R_OUT_VOL_AMP|PWR_HP_L_OUT_VOL_AMP|PWR_SPK_OUT);		

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1 ,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_ENH_AMP
			  								   		,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_ENH_AMP);

		break;

		case POWER_STATE_D1_RECORD:	//Low on of Record	
			//ALC5627 don't support record function
		break;

		case POWER_STATE_D2:		//STANDBY----

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1 ,0,PWR_MAIN_I2S_EN|PWR_HP_AMP|PWR_HP_ENH_AMP);											   

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3 ,0 ,PWR_HP_R_OUT_VOL_AMP|PWR_HP_L_OUT_VOL_AMP|PWR_SPK_OUT);
														
			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2 ,0,PWR_DAC_REF_CIR |PWR_L_DAC_CLK |PWR_R_DAC_CLK |PWR_L_DAC_L_D_S|PWR_R_DAC_R_D_S|PWR_L_HP_MIXER |PWR_R_HP_MIXER|PWR_SPK_MIXER|PWR_CLASS_D
													   );

		
		break;

		case POWER_STATE_D2_PLAYBACK:	//STANDBY of playback

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3 ,0,/*PWR_HP_R_OUT_VOL_AMP|PWR_HP_L_OUT_VOL_AMP|*/PWR_SPK_OUT);

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1 ,0,PWR_HP_AMP|PWR_HP_ENH_AMP);											   

			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD2 ,0,PWR_DAC_REF_CIR|PWR_L_DAC_CLK|PWR_R_DAC_CLK|PWR_L_DAC_L_D_S|PWR_R_DAC_R_D_S|PWR_L_HP_MIXER|PWR_R_HP_MIXER|PWR_SPK_MIXER|PWR_CLASS_D );



		break;

		case POWER_STATE_D2_RECORD:		//STANDBY of record
			//ALC5627 don't support record function
		break;		

		case POWER_STATE_D3:		//SLEEP
		case POWER_STATE_D4:		//OFF----power off all power
			RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD1 ,0,PWR_HP_AMP|PWR_HP_ENH_AMP);		
			RTWriteCodecReg(RT5627_PWR_MANAG_ADD3,0);
			RTWriteCodecReg(RT5627_PWR_MANAG_ADD1,0);
			RTWriteCodecReg(RT5627_PWR_MANAG_ADD2,0);
		
				
		break;	

		default:

		break;
	}

}

//*****************************************************************************
//
//function:Process private message of codec
//
//*****************************************************************************

DWORD RT5627::RT_ProcessAudioMessage(UINT uMsg,DWORD dwParam1,DWORD dwParam2)
{
	
   switch (uMsg)
    {			
		//To enable/disable Auto volume control	function
	   case WPDM_PRIVATE_CONFIG_AVC:		
		
			if(!EnableAVC((BYTE)dwParam1))
				return(MMSYSERR_ERROR);

            return (MMSYSERR_NOERROR);

			break;

      #ifdef BSP_USBFN_UART0_NOHF
        case  WODM_SET_AUDIOPATH:
     
            SetBTAudioPath((BOOL)dwParam1);
            return (MMSYSERR_NOERROR);

            break;
      #endif
	}
	
	return(MMSYSERR_NOTSUPPORTED);
}


#ifdef BSP_USBFN_UART0_NOHF
//flove101408_E
//*****************************************************************************
//
//function SetBTAudioPath:Enable/disable BT path from LineIn input
//							TRUE:Enable Line In path and power and disable DAC path
//							FALSE:Disable Line In path and power and Enable DAC path								
//
//*****************************************************************************
void RT5627::SetBTAudioPath(BOOL EnableBT)
{
    RETAILMSG(1,(TEXT("--set BTaudiopath, %d\r\n"), EnableBT ));

    BTMode = EnableBT;
	if(EnableBT)
	{
		//BTMode=TRUE;

		RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3,PWR_LINEIN_L_VOL|PWR_LINEIN_R_VOL,PWR_LINEIN_L_VOL|PWR_LINEIN_R_VOL);
		//RTWriteCodecRegMask(RT5627_LINE_IN_VOL,0,(0x1<<15)|(0x1<<14)|(0x1<<7));	//unmute LineIn to HP mixer and speaker mixer
		RTWriteCodecRegMask(RT5627_LINE_IN_VOL,0,(0x1<<15)|(0x1<<14)|(0x1<<7)|(0x1<<6));	//unmute LineIn to HP mixer and speaker mixer,f.w.lin
		RTWriteCodecRegMask(RT5627_STEREO_DAC_VOL,(0x1<<15)|(0x1<<14)|(0x1<<7)|(0x1<<6),(0x1<<15)|(0x1<<14)|(0x1<<7)|(0x1<<6));
		//enable output power and unmute output
		RT_PlayBack_Start(RT_WAVOUT_HP|RT_WAVOUT_SPK,NULL);

    	//RTWriteCodecRegMask(RT5627_SPK_OUT_VOL,0x1F1F,0x1F1F);	//Speaker right/left channel max db(1F), f.w.lin
	}
	else
	{
		//BTMode=FALSE;
		//mute LineIn to HP mixer and speaker mixer
		//RTWriteCodecRegMask(RT5627_LINE_IN_VOL,(0x1<<15)|(0x1<<14)|(0x1<<7),(0x1<<15)|(0x1<<14)|(0x1<<7));
		RTWriteCodecRegMask(RT5627_LINE_IN_VOL,(0x1<<15)|(0x1<<14)|(0x1<<7)|(0x1<<6),(0x1<<15)|(0x1<<14)|(0x1<<7)|(0x1<<6)); //f.w.lin
		RTWriteCodecRegMask(RT5627_PWR_MANAG_ADD3,0,PWR_LINEIN_L_VOL|PWR_LINEIN_R_VOL);
		RTWriteCodecRegMask(RT5627_STEREO_DAC_VOL,0,(0x1<<15)|(0x1<<14)|(0x1<<7)|(0x1<<6));
	}
}
#endif



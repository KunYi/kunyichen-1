
//-----------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include "RTComm.h"

//------------------------------------------------------------------------
//RT Common function implement
//------------------------------------------------------------------------

RT_CodecComm::RT_CodecComm(void)
{
	
}

RT_CodecComm::~RT_CodecComm(void)
{
	
}

//*****************************************************************************	
//
//function:implement WarmReset function,in ac97 interface,if use PLL function,we 
//		   need warmreset function
//
//*****************************************************************************
void RT_CodecComm::WarmResetController()
{
#if !USE_I2S_INTERFACE

	if(m_pHWContext)
	{
		m_pHWContext->WarmResetAC97Control();
	}

#endif
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
//function Read codec register function
//
//*****************************************************************************
BOOL RT_CodecComm::RTReadCodecReg(BYTE Offset, unsigned short int *Data)
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
//function:Write codec register function 
//
//*****************************************************************************
BOOL RT_CodecComm::RTWriteCodecReg(BYTE Offset, unsigned short int Data)
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
//function:Write codec register compare with Mask bit
//
//*****************************************************************************
BOOL RT_CodecComm::RTWriteCodecRegMask(BYTE Offset, unsigned short int Data,unsigned short int Mask)
{
	BOOL RetVal=FALSE;
	unsigned short int CodecData;

	if(!Mask)
		return RetVal; 

	if(Mask!=ALL_FIELD)
	 {
		if(RTReadCodecReg(Offset,&CodecData))
		{
			CodecData&=~Mask;
			CodecData|=(Data&Mask);
			RetVal=RTWriteCodecReg(Offset,CodecData);
		}
	 }		
	else
	{
		RetVal=RTWriteCodecReg(Offset,Data);
	}

	return RetVal;
}
//*****************************************************************************
//
//function:Write index register of codec
//
//*****************************************************************************
BOOL RT_CodecComm::RTWriteCodecIndex(BYTE Ext_Index,unsigned short int Ext_Data)
{
	BOOL RetVal=FALSE;
			
		RetVal=RTWriteCodecReg(RT_CODEC_REG_6A,Ext_Index);	// write codec  index

		if(RetVal)
		{
			RetVal=RTWriteCodecReg(RT_CODEC_REG_6C,Ext_Data);//write codec index data
		}		
			
		return  RetVal;	
			
}
//*****************************************************************************
//
//function:Read index register of codec
//
//*****************************************************************************
BOOL RT_CodecComm::RTReadCodecIndex(BYTE Ext_Index,unsigned short int *Data)
{
	BOOL RetVal=FALSE;		

	RetVal=RTWriteCodecReg(RT_CODEC_REG_6A,Ext_Index);	// write codec index		

	if(RetVal)
	 {
		RetVal=RTReadCodecReg(RT_CODEC_REG_6C,Data);	// read codec index data			
	 }

	return RetVal;
}
//*****************************************************************************
//
//function:Write index register of codec compare with Mask bit
//
//*****************************************************************************

BOOL RT_CodecComm::RTWriteCodecIndexMask(BYTE Ext_Index,unsigned short int Ext_Data,unsigned short int Ext_Data_Mask)
{
	
	unsigned short int CodecAdvData;
	BOOL RetVal=FALSE;

	if(!Ext_Data_Mask)
		return RetVal; 

	if(Ext_Data_Mask!=ALL_FIELD)
	 {
		if(RTReadCodecIndex(Ext_Index,&CodecAdvData))
		{
			CodecAdvData&=~Ext_Data_Mask;
			CodecAdvData|=(Ext_Data&Ext_Data_Mask);			
			RetVal=RTWriteCodecIndex(Ext_Index,CodecAdvData);
		}
	 }		
	else
	{
		RetVal=RTWriteCodecIndex(Ext_Index,Ext_Data);
	}

	return RetVal;

}

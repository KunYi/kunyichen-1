#ifndef __RTCODEC_H__
#define __RTCODEC_H__

#include "RTConfig.h"

#define WPDM_PRIVATE WM_USER+10
#define WPDM_PRIVATE_WRITE_CODEC		WPDM_PRIVATE+0 // do a write to the Codec register
#define WPDM_PRIVATE_READ_CODEC		WPDM_PRIVATE+1 // do a read to the Codec register
#define WPDM_PRIVATE_DIAG_MSG		WPDM_PRIVATE+2 // TBD 
#define WPDM_PRIVATE_SETUP_PHONE_PATH		WPDM_PRIVATE+3 // enable the phone path from the telephone modem/hardware
//#define WPDM_PRIVATE_DISABLE_PHONE_PATH		WPDM_PRIVATE+4 // disable the phone path from the telephone modem/hardware

#define WPDM_PRIVATE_CONFIG_HW_EQ			WPDM_PRIVATE+10	//To enable/disable HW EQ
#define WPDM_PRIVATE_ENABLE_3D				WPDM_PRIVATE+11	//To enable 3D
#define WPDM_PRIVATE_DISABLE_3D				WPDM_PRIVATE+12	//To disable 3D
#define WPDM_PRIVATE_CONFIG_PSEUDO			WPDM_PRIVATE+13	//To enable/disable Pseudo stereo
#define WPDM_PRIVATE_CONFIG_AVC				WPDM_PRIVATE+14	//To enable/disable Auto Volume Control
#define WPDM_PRIVATE_SET_3D_PARAMETER		WPDM_PRIVATE+15	//To setting 3D Parameter



#if RT_ALC5621		//support ALC5621,ALC5622,ALC5623
#define CODEC_SLAVE_ADDRESS	0x34	//i2c slave address
#include "..\Rt5621\Rt5621.h"	
class RT5621_Codec;
#define DECLARE_RT_CODEC_CLASS 	RT5621_Codec *m_RTCodec;
#define ALLOCATE_RT_CODEC_CLASS	m_RTCodec = new RT5621_Codec;

#else			    //support ALC5610,ALC5620
#define CODEC_SLAVE_ADDRESS	0x30	//i2c slave address(A1 pull low,the Device ID=0x30,A1 pull high the device ID=0x32)
#include "..\Rt56X0\RTCodecComm.h"
class RT_CodecComm;
#define DECLARE_RT_CODEC_CLASS 	RT_CodecComm *m_RTCodec;
#define ALLOCATE_RT_CODEC_CLASS	m_RTCodec = new RT_CodecComm;

#endif


#endif //__RTCODEC_H__
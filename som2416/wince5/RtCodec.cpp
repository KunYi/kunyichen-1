#include "wavemain.h"

#if RT_ALC5621

#include "..\Rt5621\Rt5621.cpp"		//support codec ALC5621,ALC5622,ALC5623

#else

#include "..\Rt56x0\RTCodecComm.cpp"	//support codec ALC5620

#endif
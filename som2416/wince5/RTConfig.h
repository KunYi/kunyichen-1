#ifndef __RTCONFIG_H__
#define __RTCONFIG_H__

//ALC5620/ALC5621/ALC5622/ALC5623 is use I2C+I2S interface

#define RT_ALC5621	TRUE
//[david. end] 2008-05-14 16:23
#if !RT_ALC5621

#define RT_ALC5620	TRUE

#endif


//default the speaker is use CLASS_AB
//#define USE_CLASS_AB_SPEAKER		TRUE
 //[david.modify] 2008-05-30 09:45
#define USE_CLASS_AB_SPEAKER		FALSE

#endif //__RTCONFIG_H__
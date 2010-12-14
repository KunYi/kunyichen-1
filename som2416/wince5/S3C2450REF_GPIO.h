#ifndef __S3C2450REF_GPIO_H__
#define __S3C2450REF_GPIO_H__

#include<BASE_TYPE.h>

#define SHOW_ALL_DMSG FALSE
#define HIDE_ALL_DMSG FALSE

#define IOCTL_RESERV 0xFFFF

#define IOCTL_GPACDL 0x0
#define IOCTL_GPACDH 0x4
//#define RESERV 0x8
//#define RESERV 0xC

#define IOCTL_GPBCON 0x10
#define IOCTL_GPBDAT 0x14
#define IOCTL_GPBUDP 0x18
//#define RESERV 0x1C

#define IOCTL_GPCCON 0x20
#define IOCTL_GPCDAT 0x24
#define IOCTL_GPCUDP 0x28
//#define RESERV 0x2C

#define IOCTL_GPDCON 0x30
#define IOCTL_GPDDAT 0x34
#define IOCTL_GPDUPD 0x38
//#define RESERV 0x3C

#define IOCTL_GPECON 0x40
#define IOCTL_GPEDAT 0x44
#define IOCTL_GPEUDP 0x48
//#define RESERV 0x4C

#define IOCTL_GPFCON 0x50
#define IOCTL_GPFDAT 0x54
//#define RESERV 0x58
//#define RESERV 0x5C

#define IOCTL_GPGCON 0x60
#define IOCTL_GPGDAT 0x64
#define IOCTL_GPGUDP 0x68
//#define RESERV 0x6C

#define IOCTL_GPHCON 0x70
#define IOCTL_GPHDAT 0x74
#define IOCTL_GPHUDP 0x78
//#define RESERV 0x7C

#define IOCTL_MISCCR  0x80
#define IOCTL_DCLKCON 0x84
#define IOCTL_EXTINT0 0x88
#define IOCTL_EXTINT1 0x8C

#define IOCTL_EXTINT2  0x90
#define IOCTL_EINTFLT0 0x94
#define IOCTL_EINTFLT1 0x98
#define IOCTL_EINTFLT2 0x9C

#define IOCTL_EINTFLT3 0xA0
#define IOCTL_EINTMASK 0xA4
#define IOCTL_EINTPEND 0xA8
#define IOCTL_GSTATUS0 0xAC

#define IOCTL_GSTATUS1 0xB0

#define IOCTL_DSC0     0xC0
#define IOCTL_DSC1 	 0xC4
#define IOCTL_DSC2	 0xC8
#define IOCTL_DSLCON   0xCC

#define IOCTL_GPJCON 0xD0
#define IOCTL_GPJDAT 0xD4
#define IOCTL_GPJUDP 0xD8
//#define RESERV 0xDC

#define IOCTL_DATAPDEN 0xe8

#define IOCTL_GPLCON 0xF0
#define IOCTL_GPLDAT 0xF4
#define IOCTL_GPLUDP 0xF8
//#define RESERV 0xFC

#define IOCTL_GPMCON 0x100
#define IOCTL_GPMDAT 0x104
#define IOCTL_GPMUDP 0x108
//#define RESERV 0x10C

#ifdef __cplusplus
extern "C" {
#endif
extern U32 READGPACDL(U32 rGPACDL, U32 rGPACDH);
extern U32 READGPACDH(U32 rGPACDL, U32 rGPACDH);
extern U32 READEXTINT0(U32 rEXTINT0);
extern U32 READEXTINT1(U32 rEXTINT1);
extern U32 READEXTINT2(U32 rEXTINT2);
extern U32 READEINTFLT2(U32 rEINTFLT2);
extern U32 READEINTFLT3(U32 rEINTFLT3);
extern BOOLEAN GPIOIoControl(GPIO_FUNC func, U32* baseAddr, U32 offset, U32* value1,  U32 value2, LPCTSTR funcName, BOOLEAN debug);

///////////////////////////
#ifdef __cplusplus
}
#endif

#endif /*__2450GPIO_REF_H__*/
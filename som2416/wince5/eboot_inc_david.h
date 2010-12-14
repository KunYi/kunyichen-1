#ifndef EBOOT_INC_DAVID_H
#define EBOOT_INC_DAVID_H 1

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif // !FALSE


//@rem 将bsp编译用做烧录器使用
//#define BSP_AS_BURNING 1



//[david.modify] 2008-05-08 16:42
//=====================================
 #if 0
; Area used to cache nk.bin while programming flash
	FLSCACHE 80200000  01800000  RESERVED
	DISPLAY  80100000  00080000  RESERVED
#endif
#define FLSCACHE_VIRTADDR 0x80200000
#define FLSCACHE_PHYADDR 0x30200000
#define DISPLAY_VIRTADDR 0x80100000

//=====================================


#define ULONG_MAX   0xffffffffUL    /* maximum unsigned long value */

 //[david.modify] 2008-05-07 18:26
 #ifndef UINT32
/* The left side of these typedefs are machine and compiler dependent */
typedef signed      char        INT8;
typedef unsigned    char        UINT8;
typedef signed      short       INT16;
typedef unsigned    short       UINT16;
typedef signed      int         INT32;
typedef signed      int         INT;
typedef unsigned    int         UINT32;
#endif

 //[david.modify] 2008-05-07 18:11
 #include <windows.h>
 #include "utils_david.h"

#define EBOOT_DEBUG 1
#ifdef EBOOT_DEBUG

#define MYLOGO "DAVID"

#define DPN(x) \
    EdbgOutputDebugString("[%s : %s : %d]=%x\n", MYLOGO,  __FUNCTION__,__LINE__,(x))

#define DPNOK(x) \
    EdbgOutputDebugString("[%s : %s : %d]=%x\n", MYLOGO, __FUNCTION__,__LINE__,(x))   

#define DPNOK3(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   


#define DPNHEX(x) \
    EdbgOutputDebugString("[%s : %s : %d]=%x\n", MYLOGO,  __FUNCTION__,__LINE__,(x))   

#define DPNDEC(x) \
    EdbgOutputDebugString("[%s : %s : %d]=%d\n", MYLOGO, __FUNCTION__,__LINE__,(x))   


#define DPSTR(x)   \
   EdbgOutputDebugString("[%s : %s : %d]%s", MYLOGO, __FUNCTION__, __LINE__, (x))   
   

#define DPCHAR(x)     \
   EdbgOutputDebugString("%c",(x))   

 //[david.modify] 2007-05-23 14:12

// david  2006-09-23 12:21
#define EPRINT EdbgOutputDebugString


#else
#define DPN(x) 
//    EPRINT("[%s]: line %d error=%x\n",__FUNCTION__,__LINE__,(x))
#define DPNOK(x)  
#define DPSTR(x) 
#define EPRINT 
#define DPCHAR
#define EPRINT  

#define DPNOK3(x) 



#endif



#include "xllp_gpio_david.h"
#include "showprogress_david.h"

//[david.modify] 2008-11-12 10:02
// sd卡更新BIT
//================================= 
#define UPDATE_BOOTLOADEROK_OSFAIL 3
//#define SD_UPDATE_MASK 0xFFFFFFFF
#define SD_UPDATE_BLOCK0 (0x1<<0)
#define SD_UPDATE_EBOOT (0x1<<1)
#define SD_UPDATE_LOGO (0x1<<2)
#define SD_UPDATE_OS (0x1<<3)
#define SD_UPDATE_FORTMAT_A (0x1<<4)
#define SD_UPDATE_FORTMAT_B (0x1<<5)
#define SD_UPDATE_FORTMAT_C (0x1<<6)
#define SD_UPDATE_ALL 0xFFFF






#endif


/*-----------------------------------------------------------------------------

Module Name:  
	DbgMsg.h


	
Notes:	

-----------------------------------------------------------------------------*/

#ifndef __DBGMSG_DAVID_H__
#define __DBGMSG_DAVID_H__



/*

//leo add
#define LIUPWRMSG(cond,printf_exp)   \
   ((cond)?(NKDbgPrintfW printf_exp),1:0)
#define GGG 1
#define ZONE_YXPOWER 1
*/
// david added.  2006-11-06 16:0
//#define EBOOT_DEBUG 1

 //[david.modify] 2007-04-03 18:42
//#define AC97_IRQ_GPIO95 1


#include "windows.h"

#define MYLOGO TEXT("DAVID")


//#define DEBUG_IN_EMULATOR 1
//#define DEBUG_DAVID 1
#ifdef DEBUG_DAVID
#define DEBUG 1	 //[david.modify] 2007-08-07 15:18


#define YXMSG(cond,printf_exp)   \
   ((cond)?(NKDbgPrintfW printf_exp),1:0)

 //[david.modify] 2007-05-25 12:25
#undef  RETAILMSG
#define RETAILMSG(cond,printf_exp)   \
   ((cond)?(NKDbgPrintfW printf_exp),1:0)

 //[david.modify] 2007-05-25 12:25
#undef  DEBUGMSG
#define DEBUGMSG(cond,printf_exp)   \
   ((1)?(NKDbgPrintfW printf_exp),1:0)





#define DPN(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"), MYLOGO, TEXT(__FUNCTION__),__LINE__,(x))

#define DPNOK(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   

#define DPNHEX(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   

#define DPNDEC(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%d\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   


#define DPNOK2(cond, x)   \
   ((void)((cond)?   (NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))),1:0))
   

  
#define DPSTR(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%s\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,TEXT(x))   




#define DPSTR_2(cond, x) \
   ((void)((cond)?   (DPSTR(x)),1:0))


#define DPSTR2(CharArr) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%a\r\n"),MYLOGO, TEXT(__FUNCTION__),__LINE__, TString(PrintableString((char*)CharArr, strlen((char*)CharArr))) )   

#define DPSTR2_2(cond, x) \
   ((void)((cond)?   (DPSTR2(x)),1:0))
    

#define DPSTR3(CharArr) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%s (%d)"),MYLOGO, TEXT(__FUNCTION__),__LINE__, TString(PrintableString((char*)CharArr,  strlen((char*)CharArr))),strlen((char*)CharArr))   

#define DPSTR3_2(cond, x) \
   ((void)((cond)?   (DPSTR3(x)),1:0))

//[david.modify] 2007-07-11 20:27
#define EPRINT NKDbgPrintfW


#define DPNOK3(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   

#define DPNHEX3(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   

#define DPNDEC3(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%d\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   

     
    
/*
#define DPN(x) \
    NKDbgPrintfW(TEXT("[%s][%s]: line %d error=%x\r\n"), MYLOGO, TEXT(__FUNCTION__),__LINE__,(x))

#define DPNOK(x) \
    NKDbgPrintfW(TEXT("[%s][%s]: line %d OK=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   


  
#define DPSTR(x) \
    NKDbgPrintfW(TEXT("[%s]: line %d OK=%s\r\n"),TEXT(__FUNCTION__),__LINE__,TEXT(x))   

#define DPSTR2(CharArr) \
    NKDbgPrintfW(TEXT("[%s]: line %d OK=%a\r\n"),TEXT(__FUNCTION__),__LINE__, TString(PrintableString((char*)CharArr, strlen(CharArr))) )   

#define DPSTR3(CharArr) \
    NKDbgPrintfW(TEXT("[%s][%s]: line %d OK=%s (%d)"),MYLOGO, TEXT(__FUNCTION__),__LINE__, TString(PrintableString((char*)CharArr,  strlen(CharArr))),strlen(CharArr))   
*/    
    

//    	NKDbgPrintfW(TEXT("[%s]: line %d OK=%s\r\n"),TEXT(__FUNCTION__),__LINE__,TString(PrintableString(szString, strlen(szString)))) ;  			 		 
//    	NKDbgPrintfW(TEXT("[%s]: line %d OK=%s\r\n"),TEXT(__FUNCTION__),__LINE__,szString) ;  
    

#define DPSTR_R1(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%s\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,TEXT(x))   

#else

//[david.modify] 2008-01-10 16:35
#define YXMSG(cond,printf_exp)

#define DPN(x) \
//    NKDbgPrintfW(TEXT("[%s]: line %d error=%x\r\n"),TEXT(__FUNCTION__),__LINE__,(x))

#define DPNOK(x)  

#define DPNHEX(x) 
#define DPNDEC(x) 

#define DPNOK3(x) \
//    NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   

#define DPNHEX3(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   

#define DPNDEC3(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%d\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   






#define DPNOK2(cond, x)    

#define DPSTR(x) \
//    NKDbgPrintfW(TEXT("[%s : %s : %d]=%s\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,TEXT(x))   

#define DPSTR_2(cond, x) 
#define DPSTR2(CharArr) 
#define DPSTR2_2(cond, x)     
#define DPSTR3(CharArr) 
#define DPSTR3_2(cond, x) 
//[david.modify] 2007-07-11 20:27
#define EPRINT 

#define DPSTR_R1(x) \
    NKDbgPrintfW(TEXT("[%s : %s : %d]=%s\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,TEXT(x))   


#endif


 //[david.modify] 2008-05-27 15:23
 // 用于调试的实用注册表函数
 #include <reg_utils_david.h>

#endif



//====================================================================
// File Name : option.h
// Function  : S3C2450
// Program   : Shin, On Pil (SOP)
// Date      : March 4, 2002
// Version   : 0.0
// History
//   0.0 : Programming start (February 20,2002) -> SOP
//====================================================================

#ifndef __OPTION_H__
#define __OPTION_H__

//#define FCLK		(533000000)	// 533MHz
//#define FCLK		(296352000)
//#define FCLK		(399651840)
#define FCLK		(400000000)

#define HCLK		(FCLK/3)		// divisor 4	// 133 MHz
#define PCLK		(FCLK/6)		// divisor 8	// 66 MHz
//#define HCLK		(FCLK/3)        
//#define PCLK		(FCLK/6)



// BUSWIDTH : 16,32
#define BUSWIDTH    (32)

//64MB
// 0x30000000 ~ 0x30ffffff : Download Area (16MB) Cacheable
// 0x31000000 ~ 0x33feffff : Non-Cacheable Area
// 0x33ff0000 ~ 0x33ff47ff : Heap & RW Area
// 0x33ff4800 ~ 0x33ff7fff : FIQ ~ User Stack Area
// 0x33ff8000 ~ 0x33fffeff : Not Useed Area
// 0x33ffff00 ~ 0x33ffffff : Exception & ISR Vector Table

#define _RAM_STARTADDRESS		(0x30000000)
#define _ISR_STARTADDRESS		(0x33ffff00)
#define _MMUTT_STARTADDRESS	(0x33ff8000)
#define _STACK_BASEADDRESS	(0x33ff8000)
#define HEAPEND					(0x33ff0000)

#define SB_NEED_EXT_ADDR				1
#define LB_NEED_EXT_ADDR				1

//If you use ADS1.x, please define ADS10
#define ADS10 TRUE

#endif	/*__OPTION_H__*/

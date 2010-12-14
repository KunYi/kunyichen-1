#ifndef UTILS_DAVID_H
#define  UTILS_DAVID_H

#ifdef __cplusplus
extern "C" {
#endif
 //[david.modify] 2007-11-27 14:10
 // fixbug: 下面两个函数主要用于DEBUG
void Write2PhyAdrr(UINT32 u32PhyAddr, UINT32 *pWrite, UINT32 u32DWord);
void ReadFromPhyAddr(UINT32 u32PhyAddr, UINT32 *pRead, UINT32 u32Readed);
UINT8 ReadSerialByte() ;
// Like library function
unsigned long strtoul(const char *nptr, char **endptr, int base);

#define isdigit2(c)	(('0' <= (c) && (c) <= '9') )


#define isxdigit2(c)	(('0' <= (c) && (c) <= '9') \
			 || ('a' <= (c) && (c) <= 'f') \
			 || ('A' <= (c) && (c) <= 'F'))
		 
int scanf_david(const char *format, UINT32 *pu32);
void PrintMsg(UINT32 u32Addr, UINT32 u32Len, UINT8 u8BytesType );

char *strWhichGPIO(UINT32 i);


int Print_IO_USB(void* pGPIOVirtBaseAddr);
void PhyAddr_RW_Test();


int Init_IO_USB(void* pGPIOVirtBaseAddr);
int Init_IO_LCD(void* pGPIOVirtBaseAddr);
int Init_IO_AUDIO(void* pGPIOVirtBaseAddr);
int Init_IO_KEYPAD(void* pGPIOVirtBaseAddr);
int Init_IO_GPS(void* pGPIOVirtBaseAddr);
int Init_IO_SD(void* pGPIOVirtBaseAddr);
int Init_IO_MISC(void* pGPIOVirtBaseAddr);
#ifdef __cplusplus
}
#endif
#endif


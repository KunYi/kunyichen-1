//[david.modify] 2008-04-17 15:21
//=====================================
#include "eboot_inc_david.h"
#ifndef NULL
#define NULL	0
#endif
 //[david.modify] 2007-11-27 14:10
 // fixbug: 下面两个函数主要用于DEBUG
 void Write2PhyAdrr(UINT32 u32PhyAddr, UINT32 *pWrite, UINT32 u32DWord)
 {
 	UINT32 i;
 	UINT32 *pAddr = (UINT32 *)OALPAtoVA(u32PhyAddr, FALSE);	
	for(i=0;i<u32DWord;i++) {
		pAddr[i] = pWrite[i];
	}

 }

 void ReadFromPhyAddr(UINT32 u32PhyAddr, UINT32 *pRead, UINT32 u32Readed)
 {
 	UINT32 i;
 	UINT32 *pAddr = (UINT32 *)OALPAtoVA(u32PhyAddr, FALSE);	
	for(i=0;i<u32Readed;i++) {
		pRead[i] = pAddr[i];
	}
	
 }

UINT8 ReadSerialByte() 
{
	int ret=0;
	while(1) {
	ret = OEMReadDebugByte();
	if(ret<0) {
		// delay ms
		continue;
	}else {
		return (UINT8)(ret);
	}
	};
}


// Like library function
unsigned long strtoul(const char *nptr, char **endptr, int base)
{
    unsigned long oflow_mul = (unsigned long)(ULONG_MAX / (unsigned long) base);
    unsigned long oflow_add = (unsigned long)(ULONG_MAX % (unsigned long) base);
    unsigned long i = 0;
    unsigned long n;

    // Defensive programming
    if (base < 2 || base > 36)
    {
//        DEBUGGERMSG(KDZONE_DBG, (L"Invalid base (%u) passed to strtoul!\r\n", base));
        return ULONG_MAX;
    }

    while((*nptr >= '0' && *nptr <= '9') || // numeric
          (*nptr >= 'a' && *nptr <= 'z') || // lower-case alpha
          (*nptr >= 'A' && *nptr <= 'Z'))   // upper-case alpha
    {
        if (*nptr >= '0' && *nptr <= '9')
            n = (*nptr - '0');
        else if (*nptr >= 'a' && *nptr <= 'z')
            n = (10 + *nptr - 'a');
        else // if (*nptr >= 'A' && *nptr <= 'Z')
            n = (10 + *nptr - 'A');

        // Validate input
        if (n >= (unsigned long) base)
            break;

        // Check for overflow
        if (i > oflow_mul || (i == oflow_mul && n > oflow_add))
        {
            i = ULONG_MAX;
            break;
        }

        i = (i * base) + n;
        nptr++;
    }

    if (endptr != NULL)
        *endptr = (char *) nptr;

    return i;
}




#define MAX_SCAN_NUM 16
int scanf_david(const char *format, UINT32 *pu32)
{
	UINT8 u8Temp = 0;
	UINT8 u8cnt=0;
	UINT8 u8Type = 0;
	UINT32 u32Value = 0;
	char strBuffer[MAX_SCAN_NUM]={0};
	char *pStr = format;
	char ch=0;

//	DPSTR(format);
	while(1) {
		ch=*pStr;
//		OEMWriteDebugByte(ch);
		if(ch==0) break;
		
		if(ch=='%') {
			if( (*(pStr+1)=='X')|| (*(pStr+1)=='x') ){
				*pStr = 0;
				u8Type = 16;
				DPSTR(format);	
				*pStr =ch;
				break;
			}else if( (*(pStr+1)=='D')|| (*(pStr+1)=='d') ){
				*pStr = 0;
				u8Type = 10;
				DPSTR(format);				
				*pStr =ch;				
				break;				
			}else{
				return;
			}
		}
		pStr++;
	}

	if(u8Type==0) return ;
//	DPNOK(u8Type);
	
	while(1) {
		u8Temp = ReadSerialByte();
		if( (u8Temp=='\r')||(u8Temp=='\n') )
			break;		

		if(u8Type==16) {
			if(isxdigit2(u8Temp)){
				strBuffer[u8cnt++] = u8Temp;
				OEMWriteDebugByte(u8Temp);
			}
		}else if(u8Type==10){
			if(isdigit2(u8Temp)){
				strBuffer[u8cnt++] = u8Temp;
				OEMWriteDebugByte(u8Temp);
			}
		}
		
		if(u8cnt>=(MAX_SCAN_NUM-1))
			return -1;			
	}
	strBuffer[u8cnt]=0;
//	DPSTR(strBuffer);
	u32Value =  strtoul(strBuffer, (char**)0, u8Type);
//	DPNOK(u32Value);
	*pu32 = u32Value;
	return u32Value;
}


void PrintMsg(UINT32 u32Addr, UINT32 u32Len, UINT8 u8BytesType )
{
	char u8Fmt[16];
	char u8Buf[32];
	int ii, temp;
	UINT8 *p8Addr;
	UINT16 *p16Addr;
	UINT32 *p32Addr;

	UINT32 u32PHYAddr = 0;
	u32PHYAddr = (UINT32)OALVAtoPA((void*)u32Addr);

//	DPNOK(0);
	EPRINT("\r\n=======u32PHYAddr=%X u32Addr=%X, u32Len=%X, uBytes=%X ==========", u32PHYAddr, u32Addr, u32Len,u8BytesType);


	switch(u8BytesType) {
	case 1:
		p8Addr = (UINT8 *)u32Addr;
	for(ii=0;ii<u32Len;ii++) {
		temp = ii*1;
		if( (temp%16)==0 ) {
			EPRINT("\r\n %X: ", u32PHYAddr+temp);
		}
		
		EPRINT("%B ", p8Addr[ii]);
		u32PHYAddr+=sizeof(UINT8);			
	}			
		break;
	case 2:
		p16Addr = (UINT16 *)u32Addr;		
	for(ii=0;ii<u32Len;ii++) {
		temp = ii*2;
		if( (temp%16)==0 ) {
			EPRINT("\r\n %X: ", u32PHYAddr+temp);
		}	
		EPRINT("%H ", p16Addr[ii]);
//		u32PHYAddr+=sizeof(UINT16);		
	}			

		break;			
	case 4:
	default:
		u8BytesType = 4;
		p32Addr = (UINT32 *)u32Addr;				
		for(ii=0;ii<u32Len;ii++) {
			temp = ii*4;
			if( (temp%16)==0 ) {
				EPRINT("\r\n %X: ", u32PHYAddr+temp);
			}		
			EPRINT("%X ", p32Addr[ii]);
//			u32PHYAddr+=sizeof(UINT32);
		}	
		break;				
	}	
	EPRINT("\r\n=============================================== \r\n");	
}



//#define GetRegBit(reg, bit) 	((reg>>bit)&1)
//
//#include "types_david.h"
#include "xllp_gpio_david.h"
#include "s3c2450_base_regs.h"
#include "s3c2450_ioport.h"

#define GPIO_DEBUG 0
int GetGPIOInfo(
	stGPIOInfo *pstGPIOInfo,	// [out] GPIO信息
	void* pGPIOVirtBaseAddr	// [in] GPIO虚拟基地址
	)
{
	S3C2450_IOPORT_REG *pGPIO = (S3C2450_IOPORT_REG *)pGPIOVirtBaseAddr;
	if(0==pstGPIOInfo) return -1;
	if(0==pGPIOVirtBaseAddr) return -2;

	// GPA 
	if( (pstGPIOInfo->u32PinNo>=GPA0) &&(pstGPIOInfo->u32PinNo<=GPA_END) )
	{
#if 	GPIO_DEBUG
		DPSTR("GPA");	DPNDEC(pstGPIOInfo->u32PinNo);
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPACON>> (pstGPIOInfo->u32PinNo-GPA0))&0x1;
		pstGPIOInfo->u32Stat =  (pGPIO->GPADAT>> (pstGPIOInfo->u32PinNo-GPA0))&0x1;
		//s3c2443 GPA没有u32PullUpdown;读出也无效;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPBUDP>> (2*(pstGPIOInfo->u32PinNo-GPA0)))&0x3;				
		
		return TRUE;
	}

	//GPB
	if( (pstGPIOInfo->u32PinNo>=GPB0) &&(pstGPIOInfo->u32PinNo<=GPB_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPB");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif

		pstGPIOInfo->u32AltFunc = (pGPIO->GPBCON>> (2*(pstGPIOInfo->u32PinNo-GPB0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPBDAT>> (1*(pstGPIOInfo->u32PinNo-GPB0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPBUDP>> (2*(pstGPIOInfo->u32PinNo-GPB0)))&0x3;		
		return TRUE;
	}	

	//GPC
	if( (pstGPIOInfo->u32PinNo>=GPC0) &&(pstGPIOInfo->u32PinNo<=GPC_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPC");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPCCON>> (2*(pstGPIOInfo->u32PinNo-GPC0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPCDAT>> (1*(pstGPIOInfo->u32PinNo-GPC0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPCUDP>> (2*(pstGPIOInfo->u32PinNo-GPC0)))&0x3;		
		return TRUE;
	}		

	//GPD
	if( (pstGPIOInfo->u32PinNo>=GPD0) &&(pstGPIOInfo->u32PinNo<=GPD_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPD");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPDCON>> (2*(pstGPIOInfo->u32PinNo-GPD0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPDDAT>> (1*(pstGPIOInfo->u32PinNo-GPD0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPDUDP>> (2*(pstGPIOInfo->u32PinNo-GPD0)))&0x3;		
		return TRUE;
	}	

	//GPE
	if( (pstGPIOInfo->u32PinNo>=GPE0) &&(pstGPIOInfo->u32PinNo<=GPE_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPE");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPECON>> (2*(pstGPIOInfo->u32PinNo-GPE0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPEDAT>> (1*(pstGPIOInfo->u32PinNo-GPE0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPEUDP>> (2*(pstGPIOInfo->u32PinNo-GPE0)))&0x3;		
		return TRUE;
	}	

	//GPF
	if( (pstGPIOInfo->u32PinNo>=GPF0) &&(pstGPIOInfo->u32PinNo<=GPF_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPF");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPFCON>> (2*(pstGPIOInfo->u32PinNo-GPF0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPFDAT>> (1*(pstGPIOInfo->u32PinNo-GPF0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPFUDP>> (2*(pstGPIOInfo->u32PinNo-GPF0)))&0x3;		
		return TRUE;
	}	

	//GPG
	if( (pstGPIOInfo->u32PinNo>=GPG0) &&(pstGPIOInfo->u32PinNo<=GPG_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPG");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPGCON>> (2*(pstGPIOInfo->u32PinNo-GPG0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPGDAT>> (1*(pstGPIOInfo->u32PinNo-GPG0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPGUDP>> (2*(pstGPIOInfo->u32PinNo-GPG0)))&0x3;		
		return TRUE;
	}	

	//GPH
	if( (pstGPIOInfo->u32PinNo>=GPH0) &&(pstGPIOInfo->u32PinNo<=GPH_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPH");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPHCON>> (2*(pstGPIOInfo->u32PinNo-GPH0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPHDAT>> (1*(pstGPIOInfo->u32PinNo-GPH0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPHUDP>> (2*(pstGPIOInfo->u32PinNo-GPH0)))&0x3;		
		return TRUE;
	}	

	//GPJ
	if( (pstGPIOInfo->u32PinNo>=GPJ0) &&(pstGPIOInfo->u32PinNo<=GPJ_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPJ");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPJCON>> (2*(pstGPIOInfo->u32PinNo-GPJ0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPJDAT>> (1*(pstGPIOInfo->u32PinNo-GPJ0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPJUDP>> (2*(pstGPIOInfo->u32PinNo-GPJ0)))&0x3;		
		return TRUE;
	}	

	//GPK
	// GPKUDP REGISTER 跟其它有不同
/*
// from S3C2443 DATASHEET
GPKUDP0- [0] - RDATA[15:0] pull-down enable control. (1:disable, 0:enable)
GPKUDP1- [1] - SDATA[15:0] pull-down enable control. (1:disable, 0:enable)
GPKUDP2- [2] - RDATA[31:16] pull-down enable control. (1:disable, 0:enable)
GPKUDP3- [3] - DQS[1:0] pull-down enable control. (1:disable, 0:enable)
GPKUDP4- [4] - SCLK pull-down enable control. (1:disable, 0:enable)
GPKUDP5- [5] - SCKE pull-down enable control. (1:disable, 0:enable)
*/
	if( (pstGPIOInfo->u32PinNo>=GPK0) &&(pstGPIOInfo->u32PinNo<=GPK_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPK");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPKCON>> (2*(pstGPIOInfo->u32PinNo-GPK0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPKDAT>> (1*(pstGPIOInfo->u32PinNo-GPK0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPKUDP>> (1*(pstGPIOInfo->u32PinNo-GPK0)))&0x3;		
		return TRUE;
	}	
	//GPL
	if( (pstGPIOInfo->u32PinNo>=GPL0) &&(pstGPIOInfo->u32PinNo<=GPL_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPL");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPLCON>> (2*(pstGPIOInfo->u32PinNo-GPL0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPLDAT>> (1*(pstGPIOInfo->u32PinNo-GPL0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPLUDP>> (2*(pstGPIOInfo->u32PinNo-GPL0)))&0x3;		
		return TRUE;
	}	

	//GPM
	if( (pstGPIOInfo->u32PinNo>=GPM0) &&(pstGPIOInfo->u32PinNo<=GPM_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPM");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pstGPIOInfo->u32AltFunc = (pGPIO->GPMCON>> (2*(pstGPIOInfo->u32PinNo-GPM0)))&0x3;
		pstGPIOInfo->u32Stat =  (pGPIO->GPMDAT>> (1*(pstGPIOInfo->u32PinNo-GPM0)))&0x1;
		pstGPIOInfo->u32PullUpdown =  (pGPIO->GPMUDP>> (2*(pstGPIOInfo->u32PinNo-GPM0)))&0x3;		
		return TRUE;
	}		

	return TRUE;
}


int SetGPIOInfo(
	stGPIOInfo *pstGPIOInfo, 	// [in] 要设置的GPIO信息
	void* pGPIOVirtBaseAddr	// [in] GPIO虚拟基地址
	)
{
	S3C2450_IOPORT_REG *pGPIO = (S3C2450_IOPORT_REG *)pGPIOVirtBaseAddr;
	if(0==pstGPIOInfo) return -1;
	if(0==pGPIOVirtBaseAddr) return -2;
	
	// GPA 
	if( (pstGPIOInfo->u32PinNo>=GPA0) &&(pstGPIOInfo->u32PinNo<=GPA_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPA");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPACON &= ~(0x1<<(pstGPIOInfo->u32PinNo-GPA0));
		pGPIO->GPACON |= (pstGPIOInfo->u32AltFunc<<(pstGPIOInfo->u32PinNo-GPA0));
		pGPIO->GPADAT &= ~(0x1<<(pstGPIOInfo->u32PinNo-GPA0));
		pGPIO->GPADAT |= (pstGPIOInfo->u32Stat<<(pstGPIOInfo->u32PinNo-GPA0));
		return TRUE;
	}

	// GPB
	if( (pstGPIOInfo->u32PinNo>=GPB0) &&(pstGPIOInfo->u32PinNo<=GPB_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPB");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPBCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPB0)));
		pGPIO->GPBCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPB0)));

		pGPIO->GPBDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPB0)));		
		pGPIO->GPBDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPB0)));		

		pGPIO->GPBUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPB0)));		
		pGPIO->GPBUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPB0)));		
	
		return TRUE;
	}		
	
	// GPC
	if( (pstGPIOInfo->u32PinNo>=GPC0) &&(pstGPIOInfo->u32PinNo<=GPC_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPC");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPCCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPC0)));
		pGPIO->GPCCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPC0)));

		pGPIO->GPCDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPC0)));		
		pGPIO->GPCDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPC0)));		

		pGPIO->GPCUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPC0)));		
		pGPIO->GPCUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPC0)));		
	
		return TRUE;
	}		

	
	// GPD
	if( (pstGPIOInfo->u32PinNo>=GPD0) &&(pstGPIOInfo->u32PinNo<=GPD_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPD");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPDCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPD0)));
		pGPIO->GPDCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPD0)));

		pGPIO->GPDDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPD0)));		
		pGPIO->GPDDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPD0)));		

		pGPIO->GPDUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPD0)));		
		pGPIO->GPDUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPD0)));		
	
		return TRUE;
	}		
		
	// GPE
	if( (pstGPIOInfo->u32PinNo>=GPE0) &&(pstGPIOInfo->u32PinNo<=GPE_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPE");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPECON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPE0)));
		pGPIO->GPECON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPE0)));

		pGPIO->GPEDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPE0)));		
		pGPIO->GPEDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPE0)));		

		pGPIO->GPEUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPE0)));		
		pGPIO->GPEUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPE0)));		
	
		return TRUE;
	}	

	// GPF
	if( (pstGPIOInfo->u32PinNo>=GPF0) &&(pstGPIOInfo->u32PinNo<=GPF_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPF");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPFCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPF0)));
		pGPIO->GPFCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPF0)));

		pGPIO->GPFDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPF0)));		
		pGPIO->GPFDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPF0)));		

		pGPIO->GPFUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPF0)));		
		pGPIO->GPFUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPF0)));		
	
		return TRUE;
	}	


	// GPG
	if( (pstGPIOInfo->u32PinNo>=GPG0) &&(pstGPIOInfo->u32PinNo<=GPG_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPG");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPGCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPG0)));
		pGPIO->GPGCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPG0)));

		pGPIO->GPGDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPG0)));		
		pGPIO->GPGDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPG0)));		

		pGPIO->GPGUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPG0)));		
		pGPIO->GPGUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPG0)));		
	
		return TRUE;
	}	


	// GPH
	if( (pstGPIOInfo->u32PinNo>=GPH0) &&(pstGPIOInfo->u32PinNo<=GPH_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPH");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPHCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPH0)));
		pGPIO->GPHCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPH0)));

		pGPIO->GPHDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPH0)));		
		pGPIO->GPHDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPH0)));		

		pGPIO->GPHUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPH0)));		
		pGPIO->GPHUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPH0)));		
	
		return TRUE;
	}	

#if (BSP_TYPE == BSP_SMDK2443)	// 2450 没有GPJ系列
	// GPJ
	if( (pstGPIOInfo->u32PinNo>=GPJ0) &&(pstGPIOInfo->u32PinNo<=GPJ_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPJ");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPJCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPJ0)));
		pGPIO->GPJCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPJ0)));

		pGPIO->GPJDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPJ0)));		
		pGPIO->GPJDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPJ0)));		

		pGPIO->GPJUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPJ0)));		
		pGPIO->GPJUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPJ0)));		
	
		return TRUE;
	}	
#endif



	//GPK
	// GPKUDP REGISTER 跟其它有不同
/*
// from S3C2443 DATASHEET
GPKUDP0- [0] - RDATA[15:0] pull-down enable control. (1:disable, 0:enable)
GPKUDP1- [1] - SDATA[15:0] pull-down enable control. (1:disable, 0:enable)
GPKUDP2- [2] - RDATA[31:16] pull-down enable control. (1:disable, 0:enable)
GPKUDP3- [3] - DQS[1:0] pull-down enable control. (1:disable, 0:enable)
GPKUDP4- [4] - SCLK pull-down enable control. (1:disable, 0:enable)
GPKUDP5- [5] - SCKE pull-down enable control. (1:disable, 0:enable)
*/
	if( (pstGPIOInfo->u32PinNo>=GPK0) &&(pstGPIOInfo->u32PinNo<=GPK_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPK");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPKCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPK0)));
		pGPIO->GPKCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPK0)));

		pGPIO->GPKDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPK0)));		
		pGPIO->GPKDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPK0)));		

		pGPIO->GPKUDP &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPK0)));		
		pGPIO->GPKUDP |= (pstGPIOInfo->u32PullUpdown<< (1*(pstGPIOInfo->u32PinNo-GPK0)));		
	
		return TRUE;
	}	

	// GPL
	if( (pstGPIOInfo->u32PinNo>=GPL0) &&(pstGPIOInfo->u32PinNo<=GPL_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPL");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPLCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPL0)));
		pGPIO->GPLCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPL0)));

		pGPIO->GPLDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPL0)));		
		pGPIO->GPLDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPL0)));		

		pGPIO->GPLUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPL0)));		
		pGPIO->GPLUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPL0)));		
	
		return TRUE;
	}	
	
	// GPM
	if( (pstGPIOInfo->u32PinNo>=GPM0) &&(pstGPIOInfo->u32PinNo<=GPM_END) )
	{
#if 	GPIO_DEBUG	
		DPSTR("GPM");	DPNDEC(pstGPIOInfo->u32PinNo);	
#endif
		pGPIO->GPMCON &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPM0)));
		pGPIO->GPMCON |= (pstGPIOInfo->u32AltFunc<<(2*(pstGPIOInfo->u32PinNo-GPM0)));

		pGPIO->GPMDAT &= ~(0x1<< (1*(pstGPIOInfo->u32PinNo-GPM0)));		
		pGPIO->GPMDAT |= (pstGPIOInfo->u32Stat<< (1*(pstGPIOInfo->u32PinNo-GPM0)));		

		pGPIO->GPMUDP &= ~(0x3<< (2*(pstGPIOInfo->u32PinNo-GPM0)));		
		pGPIO->GPMUDP |= (pstGPIOInfo->u32PullUpdown<< (2*(pstGPIOInfo->u32PinNo-GPM0)));		
	
		return TRUE;
	}	

	return TRUE;
}
 
char  stringGPIO[16]={0};
char *strWhichGPIO(UINT32 i)
{
	char* strBuffer = stringGPIO;
		if(i<=GPA_END) {
			FormatString(strBuffer, "GPA%d", (i-GPA0));
		}else if(i<=GPB_END) {
			FormatString(strBuffer, "GPB%d", (i-GPB0));
		}else if(i<=GPC_END) {
			FormatString(strBuffer, "GPC%d", (i-GPC0));
		}else if(i<=GPD_END) {
			FormatString(strBuffer, "GPD%d", (i-GPD0));
		}else if(i<=GPE_END) {
			FormatString(strBuffer, "GPE%d", (i-GPE0));
		}else if(i<=GPF_END) {
			FormatString(strBuffer, "GPF%d", (i-GPF0));
		}else if(i<=GPG_END) {
			FormatString(strBuffer, "GPG%d", (i-GPG0));
		}else if(i<=GPH_END) {
			FormatString(strBuffer, "GPH%d", (i-GPH0));
		}else if(i<=GPJ_END) {
			FormatString(strBuffer, "GPJ%d", (i-GPJ0));
		}else if(i<=GPK_END) {
			FormatString(strBuffer, "GPK%d", (i-GPK0));
		}else if(i<=GPL_END) {
			FormatString(strBuffer, "GPL%d", (i-GPL0));
		}else if(i<=GPM_END) {
			FormatString(strBuffer, "GPM%d", (i-GPM0));
		}
		return strBuffer;
}

UINT32 PrintAllGPIOStat()
{
	stGPIOInfo stGPIOInfo;
 	int i=0;
	volatile S3C2450_IOPORT_REG *s2443IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);

	DPSTR("GPA");	DPNDEC(GPA0);DPNDEC(GPA_END);
	DPSTR("GPB");	DPNDEC(GPB0);DPNDEC(GPB_END);
	DPSTR("GPC");	DPNDEC(GPC0);DPNDEC(GPC_END);
	DPSTR("GPD");	DPNDEC(GPD0);DPNDEC(GPD_END);
	DPSTR("GPE");	DPNDEC(GPE0);DPNDEC(GPE_END);
	DPSTR("GPF");	DPNDEC(GPF0);DPNDEC(GPF_END);
	DPSTR("GPG");	DPNDEC(GPG0);DPNDEC(GPG_END);	
	DPSTR("GPH");	DPNDEC(GPH0);DPNDEC(GPH_END);
	DPSTR("GPJ");	DPNDEC(GPJ0);DPNDEC(GPJ_END);
	DPSTR("GPK");	DPNDEC(GPK0);DPNDEC(GPK_END);
	DPSTR("GPL");	DPNDEC(GPL0);DPNDEC(GPL_END);	
	DPSTR("GPM");	DPNDEC(GPM0);DPNDEC(GPM_END);		


	DPNDEC(S3C24XX_GPIOS);
	EPRINT("GPXX(NO)= [u32Stat-u32AltFunc(0-in, 01-out, 2,3-func)-u32PullUpdown(2,3-disable; 0,1-enable)] \r\n");		
	EPRINT("=============================================== \r\n");	
	for(i=0;i<S3C24XX_GPIOS;i++){
		stGPIOInfo.u32PinNo = 	i;	
		GetGPIOInfo(&stGPIOInfo, s2443IOP);
		EPRINT("%s (%d)= [%d %d %d] \r\n",
			strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
			stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		
		
	}
	EPRINT("\r\n=============================================== \r\n");		
	return 1;
}

#if 0
// smdk2443
g_oalAddressTable
	[ {TRUE}
	DCD     0x80000000, 0x33E00000, 2       ; 64 MB DRAM BANK 6
	DCD     0x8C000000, 0x30000000, 62      ; 62 MB DRAM BANK 6
	
	DCD     0x84000000, 0x10000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 2
	DCD     0x86000000, 0x18000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 3
	DCD     0x88000000, 0x20000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 4
	DCD     0x8A000000, 0x28000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 5
;	DCD     0x8C000000, 0x08000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 1
	
	DCD     0x90800000, 0x48000000,  1      ; SDRAM control register
	DCD     0x90900000, 0x48800000,  1      ; EBI control register
	DCD     0x90A00000, 0x49000000,  1      ; USB Host control register
	DCD     0x90B00000, 0x49800000,  1      ; USB Device control register
	DCD     0x90C00000, 0x4A000000,  1      ; Interrupt control register
	DCD     0x90D00000, 0x4A800000,  1      ; HS-MMC control register
	DCD     0x90E00000, 0x4B000000,  1      ; DMA control register
	DCD     0x90F00000, 0x4B800000,  1      ; CF Card control register
	DCD     0x91000000, 0x4C000000,  1      ; SYSCON register
	DCD     0x91100000, 0x4C800000,  1      ; TFT-LCD control register
;	DCD     0x91200000, 0x4D000000,  1      ; STN-LCD control register
	DCD     0x91300000, 0x4D800000,  1      ; Camera control register
	DCD     0x91400000, 0x4E000000,  1      ; NAND flash control register
	DCD     0x91500000, 0x4E800000,  1      ; Matrix control register
	DCD     0x91600000, 0x4F000000,  1      ; SSMC control register
	DCD     0x91700000, 0x4F800000,  1      ; TIC control register
	DCD     0x91800000, 0x50000000,  1      ; UART control register
	DCD     0x91C00000, 0x51000000,  1      ; PWM control register
	DCD     0x91D00000, 0x52000000,  1      ; HS-SPI and SPI0 control register
	DCD     0x91E00000, 0x53000000,  1      ; WDT control register
	DCD     0x91F00000, 0x54000000,  1      ; IIC control register
	DCD     0x92000000, 0x55000000,  1      ; IIS control register
	DCD     0x92100000, 0x56000000,  1      ; I/O Port register
	DCD     0x92200000, 0x57000000,  1      ; RTC Port register
	DCD     0x92300000, 0x58000000,  1      ; TSADC Port register
	DCD     0x92400000, 0x59000000,  1      ; SPI Port register
	DCD     0x92500000, 0x5A000000,  1      ; SDI Port register
	DCD     0x92600000, 0x5B000000,  1      ; AC97 Port register

	DCD     0x93000000, 0x00000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 0
        DCD     0x00000000, 0x00000000,  0      ; end of table
	]


// smdk2450
; //[david.modify] 2008-05-06 09:58
	IF :DEF: BSP_32MSDRAM
	DCD     0x80000000, 0x30000000, 32      ; 64 MB DRAM BANK 6ENDIF
	ELSE
	DCD     0x80000000, 0x30000000, 64      ; 64 MB DRAM BANK 6ENDIF
	ENDIF

; //[david. end] 2008-05-06 09:59
;==============================   
	
;       DCD     0x80000000, 0x30000000, 64      ; 64 MB DRAM BANK 6
;       DCD     0x80000000, 0x30000000, 32      ; 64 MB DRAM BANK 6
;        //[david. end] 2008-04-28 11:20

       ; DCD     0x84000000, 0x10000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 2
       ; DCD     0x86000000, 0x18000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 3
       ; DCD     0x88000000, 0x20000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 4
       ; DCD     0x8A000000, 0x28000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 5
       ; DCD     0x8C000000, 0x08000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 1

	DCD     0x90800000, 0x48000000,  1      ; SDRAM control register
	DCD     0x90900000, 0x48800000,  1      ; EBI control register
	DCD     0x90A00000, 0x49000000,  1      ; USB Host control register
	DCD     0x90B00000, 0x49800000,  1      ; USB Device control register
	DCD     0x90C00000, 0x4A000000,  1      ; Interrupt control register
	DCD     0x90D00000, 0x4A800000,  1      ; HS-MMC control register

	DCD     0x90E00000, 0x4B000000,  1      ; DMA0~7 control register

	
	DCD     0x90F00000, 0x4B800000,  1      ; CF Card control register
	DCD     0x91000000, 0x4C000000,  1      ; SYSCON register
	DCD     0x91100000, 0x4C800000,  1      ; TFT-LCD control register
	DCD     0x91200000, 0x4D000000,  1      ; STN-LCD control register
	DCD     0x91300000, 0x4D408000,  1      ; 2D
	DCD     0x91400000, 0x4D800000,  1      ; Camera control register
	DCD     0x91500000, 0x4E000000,  1      ; NAND flash control regist
	DCD     0x91600000, 0x4E800000,  1      ; Matrix control register
	DCD     0x91700000, 0x4F000000,  1      ; SSMC control register
	DCD     0x91800000, 0x4F800000,  1      ; TIC control register
	DCD     0x91900000, 0x50000000,  1      ; UART control register
	DCD     0x91A00000, 0x51000000,  1      ; PWM control register
	DCD     0x91B00000, 0x52000000,  1      ; HS-SPI and SPI0 control register
	DCD     0x91C00000, 0x53000000,  1      ; WDT control register
	DCD     0x91D00000, 0x54000000,  1      ; IIC control register
	DCD     0x92000000, 0x55000000,  1      ; IIS control register
	DCD     0x92100000, 0x56000000,  1      ; I/O Port register
	DCD     0x92200000, 0x57000000,  1      ; RTC Port register
	DCD     0x92300000, 0x58000000,  1      ; TSADC Port register
	DCD     0x92400000, 0x59000000,  1      ; SPI Port register
	DCD     0x92500000, 0x5A000000,  1      ; SDI Port register
	DCD     0x92600000, 0x5B000000,  1      ; AC97 Port register
	DCD     0x92700000, 0x5C000000,  1      ; PCM 0 ~ 1 control register
	DCD     0x92800000, 0x5D000000,  1      ; GPS control register	
	DCD     0x92900000, 0x5E000000,  1      ; CHIP ID control register

	DCD     0x93000000, 0x00000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 0
        DCD     0x00000000, 0x00000000,  0      ; end of table
	
#endif

typedef struct{
char *strRegName;	// s3c2443 module register name
UINT32 u32Base;		// 起始物理地址
UINT32 u32End;		//结束物理地址
}stCPURegAddr;	

#if 0
stCPURegAddr g_stS3c2443Reg[]={
{"DRAM", 0x48000000, 0x48000014},
{"USB HOST", 0x49000000, 0x49000058},
{"USB Device", 0x49800000, 0x49800104},
{"Interrupt", 0x4A000000, 0x4A000034},
{"HS-MMC", 0x4A800000, 0x4A8000FE},
{"DMA", 0x4B000000, 0x4B000524},
{"CF", 0x4B800000, 0x4B801994},
{"SYSCON", 0x4C000000, 0x4C00008C},
{"TFT-LCD", 0x4C800000, 0x4C800BFC},
{"STN-LCD", 0x4D000000, 0x4D00005C},
{"Camera", 0x4D800000, 0x4D8000D8},
{"NAND", 0x4E000000, 0x4E000040},
{"Matrix", 0x4E800000, 0x4E800008},	
{"SSMC", 0x4F000000, 0x4F000204},
//{"TIC", 0x4F800000, 0x4F800000},
//{"UART", 0x50000000, 0x5000C02C},
{"UART0", 0x50000000, 0x5000002C},
{"UART1", 0x50004000, 0x5000402C},
{"UART2", 0x50008000, 0x5000802C},
{"UART3", 0x5000C000, 0x5000C02C},

{"PWM", 0x51000000, 0x51000040},
{"HS-SPI", 0x52000000, 0x52000024},
{"WDT", 0x53000000, 0x53000008},
{"IIC", 0x54000000, 0x54000010},	
{"IIS", 0x55000000, 0x55000014},
{"I/O", 0x56000000, 0x560000E8},
{"RTC", 0x57000000, 0x57000097},
{"TSADC", 0x58000000, 0x58000018},
{"SPI", 0x59000000, 0x59000028},
{"SDI", 0x5A000000, 0x5A000043},
{"AC97", 0x5B000000, 0x5B00001C}
};
#endif

stCPURegAddr g_stS3c2450Reg[]={
{"DRAM", 0x48000000, 0x48000014},
{"USB HOST", 0x49000000, 0x49000058},
{"USB Device", 0x49800000, 0x49800104},
{"Interrupt", 0x4A000000, 0x4A000074},	 //[david.modify] 2008-05-08 16:48
{"HS-MMC", 0x4A800000, 0x4A8000FE},
{"DMA", 0x4B000000, 0x4B000524},
//{"CF", 0x4B800000, 0x4B801994},
{"SYSCON", 0x4C000000, 0x4C00008C},
{"TFT-LCD", 0x4C800000, 0x4C800BFC},
//{"STN-LCD", 0x4D000000, 0x4D00005C},
//{"Camera", 0x4D800000, 0x4D8000D8},
{"NAND", 0x4E000000, 0x4E000064},
{"Matrix", 0x4E800000, 0x4E800008},	
{"SSMC", 0x4F000000, 0x4F000204},
//{"TIC", 0x4F800000, 0x4F800000},
//{"UART", 0x50000000, 0x5000C02C},
{"UART0", 0x50000000, 0x5000002C},
{"UART1", 0x50004000, 0x5000402C},
{"UART2", 0x50008000, 0x5000802C},
{"UART3", 0x5000C000, 0x5000C02C},

{"PWM", 0x51000000, 0x51000040},
{"HS-SPI", 0x52000000, 0x5200002C}, //[david.modify] 2008-05-08 16:56
{"WDT", 0x53000000, 0x53000008},
{"IIC", 0x54000000, 0x54000010},	
{"IIS", 0x55000000, 0x55000014},
{"I/O", 0x56000000, 0x56000118}, //[david.modify] 2008-05-08 16:57
{"RTC", 0x57000000, 0x57000090}, //[david.modify] 2008-05-08 16:57
{"TSADC", 0x58000000, 0x58000018},
//{"SPI", 0x59000000, 0x59000028},
//{"SDI", 0x5A000000, 0x5A000043},
{"AC97", 0x5B000000, 0x5B00001C}
};
int PrintCPURegMemory(stCPURegAddr *pstCPUReg, UINT8 u8ShowMode)
{
	UINT32 u32VirtAddr = 0;
	UINT32 u32Bytes = 0;
	if(0==pstCPUReg) return -1;

	u32VirtAddr = OALPAtoVA(pstCPUReg->u32Base, FALSE);
	u32Bytes = (pstCPUReg->u32End-pstCPUReg->u32Base)+sizeof(UINT32);				

#if 0
	DPNOK(u32VirtAddr);
	DPNOK(u32Bytes);	
	DPSTR(pstCPUReg->strRegName);	
#endif	

	EPRINT("===%s===", pstCPUReg->strRegName);
	switch(u8ShowMode) {
	case 1:		
		PrintMsg(u32VirtAddr, u32Bytes/sizeof(UINT8),sizeof(UINT8));				
		break;
	case 2:
		PrintMsg(u32VirtAddr, u32Bytes/sizeof(UINT16),sizeof(UINT16));				
		break;
	case 4:
	default:
		PrintMsg(u32VirtAddr, u32Bytes/sizeof(UINT32),sizeof(UINT32));				
		break;

	}

	return 1;
}

void PrintAllCPURegMemory()
{
	int i=0;
	int n= sizeof(g_stS3c2450Reg)/sizeof(g_stS3c2450Reg[0]);
	for(i=0;i<n;i++) {
		PrintCPURegMemory(&g_stS3c2450Reg[i], sizeof(UINT32));
	}
}

//for 2416 pnd code
//[david.modify] 2008-04-23 09:44



int Init_IO_USB(void* pGPIOVirtBaseAddr)
{
	int i=0;
	stGPIOInfo stGPIOInfo[]={
	{ USB_DET,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},
	{ CH_nFULL,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},
	{ CH_CON,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ PWREN_USB,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
	};


	for(i=0;i<sizeof(stGPIOInfo)/sizeof(stGPIOInfo[0]);i++) {
		SetGPIOInfo(&stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}

	//PRINT RESULT
	Print_IO_USB(pGPIOVirtBaseAddr);
	return TRUE;
}

int Print_IO_USB(void* pGPIOVirtBaseAddr)
{
	stGPIOInfo stGPIOInfo;	

	if(0==pGPIOVirtBaseAddr) 
		return -1;

	// get	
	EPRINT("==USB IO==\r\n");
	stGPIOInfo.u32PinNo = USB_DET;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("USB_DET=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
			
	stGPIOInfo.u32PinNo = CH_nFULL;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("CH_nFULL=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = CH_CON;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("CH_nFULL=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = PWREN_USB;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("PWREN_USB=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
	
	return 1;
}



int Print_IO_LCD(void* pGPIOVirtBaseAddr)
{
	stGPIOInfo stGPIOInfo;	

	if(0==pGPIOVirtBaseAddr) 
		return -1;

	// get	
	EPRINT("==LCD IO==\r\n");
	stGPIOInfo.u32PinNo = LCDRST;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDRST=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
			
	stGPIOInfo.u32PinNo = LCDCS;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDCS=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = LCDCLK;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDCLK=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = LCDSDA;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDSDA=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	stGPIOInfo.u32PinNo = LCDCLK;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDCLK=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	


	stGPIOInfo.u32PinNo = LCDHSYNC;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDHSYNC=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	stGPIOInfo.u32PinNo = LCDVSYNC;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDVSYNC=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	stGPIOInfo.u32PinNo = LCDPCLK;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDPCLK=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	stGPIOInfo.u32PinNo = LCDDE;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCDDE=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		
	stGPIOInfo.u32PinNo = BACKLIGHT_PWM;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("BACKLIGHT_PWM=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	stGPIOInfo.u32PinNo = LCD_PWREN;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("LCD_PWREN=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);			
	
	return 1;
}

int Init_IO_LCD(void* pGPIOVirtBaseAddr)
{
	int i=0;
	stGPIOInfo stGPIOInfo[]={
		 //[david.modify] 2008-08-29 17:50
		 // 比亚迪的屏不使用DE，DE要保持常高
		 //设成功能脚的话， 会黑屏
	{ LCDDE,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
//	{ LCDDE,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
//	{ BACKLIGHT_PWM,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	 //[david.modify] 2008-06-16 11:17
//	{ LCD_PWREN,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	

	{ LCDRST,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ LCDCS,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ LCDCLK,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ LCDSDA,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
	
	};

	for(i=0;i<sizeof(stGPIOInfo)/sizeof(stGPIOInfo[0]);i++) {
		SetGPIOInfo(&stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}

	//PRINT RESULT
	Print_IO_LCD(pGPIOVirtBaseAddr);
	return TRUE;
}





int Print_IO_AUDIO(void* pGPIOVirtBaseAddr)
{
	stGPIOInfo stGPIOInfo;	

	if(0==pGPIOVirtBaseAddr) 
		return -1;

	// get	
	EPRINT("==AUDIO IO==\r\n");
	stGPIOInfo.u32PinNo = I2SLRCK;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("I2SLRCK=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
			
	stGPIOInfo.u32PinNo = I2SSCLK;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("I2SSCLK=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = I2SSDI;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("I2SSDI =%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		
	
	stGPIOInfo.u32PinNo = I2SSDO;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("I2SSDO=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);			

	stGPIOInfo.u32PinNo = I2CSCL;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("I2CSCL=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = I2CSDA;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("I2CSDA=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);			
	return 1;
}


int Init_IO_AUDIO(void* pGPIOVirtBaseAddr)
{
	int i=0;
	stGPIOInfo stGPIOInfo[]={
	{ I2SLRCK,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ I2SSCLK,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ I2SSDI,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},	
	{ I2SSDO,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},		
	{ I2CSCL,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},		
	{ I2CSDA,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},			
	};

	for(i=0;i<sizeof(stGPIOInfo)/sizeof(stGPIOInfo[0]);i++) {
		SetGPIOInfo(&stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}

	//PRINT RESULT
	Print_IO_AUDIO(pGPIOVirtBaseAddr);
	return TRUE;
}






int Print_IO_KEYPAD(void* pGPIOVirtBaseAddr)
{
	stGPIOInfo stGPIOInfo;	

	if(0==pGPIOVirtBaseAddr) 
		return -1;

	// get	
	EPRINT("==KEYPAD IO==\r\n");
	stGPIOInfo.u32PinNo = PWR_KEY;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("PWR_KEY=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
			
	stGPIOInfo.u32PinNo = KEY6;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("KEY6(VOL+)=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = KEY3;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("KEY3(VOL-)=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = KEY4;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("KEY4(ENTER)=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
	
	return 1;
}

 //[david.modify] 2008-07-31 17:35
BOOL IsPwrKeyPressed(void)
{
	stGPIOInfo stGPIOInfo[]={
	{ PWR_KEY,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},
		
	};
    BOOL        result = FALSE;
    int         count = 5;
    volatile 	S3C2450_IOPORT_REG *s24500IOP = (volatile S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    s24500IOP->GPFCON &= ~(0x3<<2);   // as input

 //[david.modify] 2008-07-31 17:33 	
	SetGPIOInfo(&stGPIOInfo[0],  s24500IOP);
	GetGPIOInfo(&stGPIOInfo[0],  s24500IOP);
 	if(0==stGPIOInfo[0].u32Stat) {		//有卡
	return TRUE;
	}else{
	return FALSE;
	}

}


 //[david.modify] 2008-08-02 10:05
 // 拿掉确认键
int Init_IO_KEYPAD(void* pGPIOVirtBaseAddr)
{
	int i=0;
	volatile S3C2450_IOPORT_REG *s2443IOP;
	stGPIOInfo stGPIOInfo[]={
	{ PWR_KEY,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},
	{ KEY6,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},	
	{ KEY3,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},	
//	{ KEY4,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},		
	};

	for(i=0;i<sizeof(stGPIOInfo)/sizeof(stGPIOInfo[0]);i++) {
		SetGPIOInfo(&stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}
//[david.modify] 2008-05-16 16:10
	s2443IOP = (S3C2450_IOPORT_REG*)pGPIOVirtBaseAddr;
// 打开GPG8,9,10	IO中断
// 	s2443IOP->EINTMASK &=~((0x1<<8)|(0x1<<9)|(0x1<<10));
 	s2443IOP->EINTMASK &=~((0x1<<8)|(0x1<<10));
//低电平中断
//	s2443IOP->EXTINT1 &= ~((0xF<<0)|(0xF<<4)|(0xF<<8));
	s2443IOP->EXTINT1 &= ~((0xF<<0)|(0xF<<8));
//	s2443IOP->EXTINT1 |= ((0xE<<0)|(0xE<<4)|(0xE<<8));		
	s2443IOP->EXTINT1 |= ((0xE<<0)|(0xE<<8));		

	//PRINT RESULT
	Print_IO_KEYPAD(pGPIOVirtBaseAddr);
	return TRUE;
}





int Print_IO_GPS(void* pGPIOVirtBaseAddr)
{
	stGPIOInfo stGPIOInfo;	

	if(0==pGPIOVirtBaseAddr) 
		return -1;

	// get	
	EPRINT("==GPS IO==\r\n");
	stGPIOInfo.u32PinNo = GPS_BOOT;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("GPS_BOOT=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
			
	stGPIOInfo.u32PinNo = GPS_RESET;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("GPS_RESET=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = GPS_POWER_EN;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("GPS_POWER_EN=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = GPS_RXD;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("GPS_RXD=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	stGPIOInfo.u32PinNo = GPS_TXD;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("GPS_TXD=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		
	return 1;
}


int Init_IO_GPS(void* pGPIOVirtBaseAddr)
{
	int i=0;
	stGPIOInfo stGPIOInfo[]={
// 0 - 正常操作模式1-更新模式		
	{ GPS_BOOT,  0, GPA_ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
// 0低复位	
	{ GPS_RESET,  1, GPA_ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
	{ GPS_POWER_EN,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ GPS_RXD,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},		
	{ GPS_TXD,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},			
	};

	for(i=0;i<sizeof(stGPIOInfo)/sizeof(stGPIOInfo[0]);i++) {
		SetGPIOInfo(&stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}

	//PRINT RESULT
	Print_IO_GPS(pGPIOVirtBaseAddr);
	return TRUE;
}



int Print_IO_SD(void* pGPIOVirtBaseAddr)
{
	stGPIOInfo stGPIOInfo;	

	if(0==pGPIOVirtBaseAddr) 
		return -1;

	// get	
	EPRINT("==SD IO==\r\n");
	stGPIOInfo.u32PinNo = SD0_DATA0;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SD0_DATA0=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
			
	stGPIOInfo.u32PinNo = SD0_DATA1;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SD0_DATA1=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = SD0_DATA2;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SD0_DATA2=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = SD0_DATA3;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SD0_DATA3=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	stGPIOInfo.u32PinNo = SDCLK;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SDCLK=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = SDCMD;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SDCMD=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
	
	stGPIOInfo.u32PinNo = SD0_NCD;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SD0_NCD=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = SD0_WP;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SD0_WP=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = SD_PWREN;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("SD_PWREN=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);			
	
	return 1;
}



 //[david.modify] 2008-07-31 17:35
BOOL IsCardInserted(void)
{
	stGPIOInfo stGPIOInfo[]={
	{ SD0_NCD,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},		
		
	};

    BOOL        result = FALSE;
    int         count = 5;
    volatile 	S3C2450_IOPORT_REG *s24500IOP = (volatile S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    s24500IOP->GPFCON &= ~(0x3<<2);   // as input

 //[david.modify] 2008-07-31 17:33 	
	GetGPIOInfo(&stGPIOInfo[0],  s24500IOP);
 	if(0==stGPIOInfo[0].u32Stat) {		//有卡
	return TRUE;
	}else{
	return FALSE;
	}

}


int Init_IO_SD(void* pGPIOVirtBaseAddr)
{
	int i=0;
	stGPIOInfo stGPIOInfo[]={
	{ SD0_DATA0,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ SD0_DATA1,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ SD0_DATA2,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ SD0_DATA3,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},	
	
	{ SDCLK,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ SDCMD,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},	
	{ SD0_NCD,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},		
	{ SD0_WP,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},			
	{ SD_PWREN,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},				
	};

	for(i=0;i<sizeof(stGPIOInfo)/sizeof(stGPIOInfo[0]);i++) {
		SetGPIOInfo(&stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}

	//PRINT RESULT
	Print_IO_SD(pGPIOVirtBaseAddr);
	return TRUE;
}



int Print_IO_MISC(void* pGPIOVirtBaseAddr)
{
	stGPIOInfo stGPIOInfo;	

	if(0==pGPIOVirtBaseAddr) 
		return -1;

	// get	
	EPRINT("==MISC IO==\r\n");
	stGPIOInfo.u32PinNo = TMC_ANT_DET;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("TMC_ANT_DET=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
			
	stGPIOInfo.u32PinNo = RTS0;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("RTS0=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = CTS0;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("CTS0=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = RXD0;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("RXD0=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	stGPIOInfo.u32PinNo = TXD0;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("TXD0=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = nFCE2;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("nFCE2=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
	
	stGPIOInfo.u32PinNo = TMC_ON;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("TMC_ON=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = GLED;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("GLED=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = CPUCORE_CON;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT("GLED=%s=%d: [%d %d %d] \r\n",
		strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	return 1;
}


int Init_IO_MISC(void* pGPIOVirtBaseAddr)
{
	int i=0;
	stGPIOInfo stGPIOInfo[]={
	{ TMC_ANT_DET,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},
	{ RTS0,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ CTS0,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},
	{ RXD0,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},	
	{ TXD0,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE},		
	{ ULC_RST,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},			
//	{nFCE2,  0, 1, PULL_UPDOWN_DISABLE},			
//	{ TMC_ON,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},				
	{ GLED,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},					

	{ CPUCORE_CON,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},			

	
	};

	for(i=0;i<sizeof(stGPIOInfo)/sizeof(stGPIOInfo[0]);i++) {
		SetGPIOInfo(&stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}

	//PRINT RESULT
	Print_IO_MISC(pGPIOVirtBaseAddr);
	return TRUE;
}






 //[david.modify] 2008-05-08 16:41


void PhyAddr_RW_Test()
{
	UINT8 u8Temp = 0;
	stGPIOInfo stGPIOInfo;	
	int ret=0, nRet=0, i=0, n=0;
//       SYSTEMTIME stGsmTime;	
	UINT32 u32Tmp[4]	  ={0};

 //[david.modify] 2008-05-08 16:43
	UINT32* pu32RamTmpBuf = (UINT32*)FLSCACHE_PHYADDR;			// 缓冲区
	UINT32 u32MaxRamTmpBufSize = 0x1000;

	volatile S3C2450_IOPORT_REG *s2443IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);

	while(1){		
              EPRINT ( "1)ReadFromPhyAddr \r\n");
              EPRINT ( "2)Write2PhyAdrr \r\n");
              EPRINT ( "3)Print GPIO memory \r\n");			  
              EPRINT ( "4)PrintAllCPURegMemory \r\n");				  
              EPRINT ( "7)Init GPIO for module\r\n");				  			  
              EPRINT ( "8)Print all GPIO\r\n");				  
              EPRINT ( "9)Set GPIO state:\r\n");		  
              EPRINT ( "Please select: ");	
		u8Temp = ReadSerialByte();
		if('0'==u8Temp) {
			return;
		}
		else if('1'==u8Temp){
			scanf_david("READ ADDR= %x:\r\n", &u32Tmp[0]);
			scanf_david("UINT32 NO= %x:\r\n", &u32Tmp[1]);			
 			DPNOK(u32Tmp[0]); 	DPNOK(u32Tmp[1]);
			if(u32Tmp[1]<=u32MaxRamTmpBufSize) {
				ReadFromPhyAddr(u32Tmp[0], pu32RamTmpBuf, u32Tmp[1]);
				PrintMsg((UINT32)pu32RamTmpBuf, u32Tmp[1], sizeof(UINT32));
			}
		}
		else if('2'==u8Temp){
			scanf_david("WRITE ADDR= %x:\r\n", &u32Tmp[0]);
			scanf_david("UINT32 NO= %x:\r\n", &u32Tmp[1]);			
			scanf_david("UINT32 VALUE= %x:\r\n", &u32Tmp[2]);						
 			DPNOK(u32Tmp[0]); 	DPNOK(u32Tmp[1]);	DPNOK(u32Tmp[2]);			
			if(u32Tmp[1]<=u32MaxRamTmpBufSize) {
				memset(pu32RamTmpBuf, 	u32Tmp[2], sizeof(UINT32)*u32Tmp[1]);				
				Write2PhyAdrr(u32Tmp[0], pu32RamTmpBuf, u32Tmp[1]);
				PrintMsg((UINT32)pu32RamTmpBuf, u32Tmp[1], sizeof(UINT32));
			}
		}else if('3'==u8Temp){
			DPNOK(0);
			PrintMsg(s2443IOP, sizeof(S3C2450_IOPORT_REG)/sizeof(UINT32),sizeof(UINT32));
		}else if('4'==u8Temp){
			DPNOK(0);
			PrintAllCPURegMemory();
		}else if('7'==u8Temp){
			DPNOK(0);
			PrintAllGPIOStat();
			Init_IO_USB(s2443IOP);
			Init_IO_LCD(s2443IOP);			
			Init_IO_AUDIO(s2443IOP);
			Init_IO_KEYPAD(s2443IOP);
			Init_IO_GPS(s2443IOP);
			Init_IO_SD(s2443IOP);
			Init_IO_MISC(s2443IOP);
			PrintAllGPIOStat();			

		}else if('8'==u8Temp){
			DPNOK(0);
			PrintAllGPIOStat();
			Print_IO_USB(s2443IOP);
			Print_IO_LCD(s2443IOP);			
			Print_IO_AUDIO(s2443IOP);
			Print_IO_KEYPAD(s2443IOP);
			Print_IO_GPS(s2443IOP);
			Print_IO_SD(s2443IOP);
			Print_IO_MISC(s2443IOP);

		}else if('9'==u8Temp){
			DPNOK(0);
			EPRINT("GPXX(NO)= [u32Stat-u32AltFunc(0-in, 01-out, 2,3-func)-u32PullUpdown(2,3-disable; 0,1-enable)] \r\n");					
			scanf_david("GPIO u32PinNo: %d", &stGPIOInfo.u32PinNo);
			// get	
			GetGPIOInfo(&stGPIOInfo, s2443IOP, stGPIOInfo.u32PinNo);
//		       EPRINT("PIN %d: [%d %d %d %d] \r\n",
//		       	stGPIOInfo.u32PinNo, stGPIOInfo.u32Stat, stGPIOInfo.u32Dir, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);					
			EPRINT("%s=%d: [%d %d %d] \r\n",
				strWhichGPIO(stGPIOInfo.u32PinNo), stGPIOInfo.u32PinNo, 
				stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		
			
			DPNOK(stGPIOInfo.u32PinNo);
			scanf_david("GPIO u32Stat: %d", &stGPIOInfo.u32Stat);
			DPNOK(stGPIOInfo.u32Stat);			
//			scanf_david("GPIO u32Dir: %d", &stGPIOInfo.u32Dir);
//			DPNOK(stGPIOInfo.u32Dir);			
			scanf_david("GPIO u32AltFunc: %d", &stGPIOInfo.u32AltFunc);			
			DPNOK(stGPIOInfo.u32AltFunc);		
			scanf_david("GPIO u32PullUpdown: %d", &stGPIOInfo.u32PullUpdown);			
			DPNOK(stGPIOInfo.u32PullUpdown);		
			
			ret = SetGPIOInfo(&stGPIOInfo, s2443IOP);
			DPNOK(ret);
		}
	}


}

//============================================================





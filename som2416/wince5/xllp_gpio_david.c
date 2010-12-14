/*
 * file: xllp_gpio.c
 * description: GPIO control function
 * rev history
 * version	time				author
 *==========================
 *  v0.1 		2008-04-17 09:47 	david
 */
#include "types_david.h"
#include "xllp_gpio_david.h"
#include "s3c2450_ioport.h"
#include "dbgmsg_david.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



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
	
	if( pstGPIOInfo->u32PinNo == LCD_PWREN )
	{
		RETAILMSG(1, (TEXT("\r\n set LCD POWER: %d \r\n"), pstGPIOInfo->u32Stat ));
	}
	else if( pstGPIOInfo->u32PinNo == BACKLIGHT_PWM)
	{
		RETAILMSG(1, (TEXT("\r\n set backlight POWER: %d \r\n"), pstGPIOInfo->u32Stat ));	
	}
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


 //[david.modify] 2008-06-06 14:20
 //==================================
 int SetMultiGPIO(UINT32 u32StartIO, UINT32 u32EndIO, stGPIOInfo *pstGPIOInfo, void* pGPIOVirtBaseAddr )
 {
 	UINT32 i=0;
 	stGPIOInfo stGPIOInfo;

	memcpy(&stGPIOInfo, pstGPIOInfo, sizeof(stGPIOInfo));
	for(i=u32StartIO;i<=u32EndIO;i++) {
		stGPIOInfo.u32PinNo = i;
		SetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	}
	return TRUE;
 }

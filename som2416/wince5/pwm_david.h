#ifndef PWM_DAVID_H
#define PWM_DAVID_H

#include <s3c2450_ioport.h>
#include <xllp_gpio_david.h>

//[david.modify] 2008-05-30 16:05
#define TCON_TIMER_SHIFT 12

 //[david.modify] 2008-07-11 19:07

int PWM_Init()
{
	volatile S3C2450_IOPORT_REG *v_p2450IOP=NULL;
	volatile S3C2450_PWM_REG * v_pPWMregs=NULL;	
 //[david.modify] 2008-07-11 15:01
 //背光新板
	stGPIOInfo stGPIOInfo={
//		BACKLIGHT_PWM,  1, ALT_FUNC_02, PULL_UPDOWN_DISABLE
		BACKLIGHT_PWM,  1, ALT_FUNC_02, PULL_UPDOWN_DISABLE
	};

	if(NULL==v_pPWMregs)
		v_pPWMregs = (S3C2450_PWM_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_PWM, FALSE);
	if(NULL==v_p2450IOP)
		v_p2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);	




 //[david.modify] 2008-06-04 16:01
 // 背光原来使用GPB3，但与TOUCH冲突
 //改成GPB2,但前期装机的机器硬件没改，还是用GPB3
 //==================================
 #if 1
//	SetGPIOInfo(&stGPIOInfo_old,  v_p2450IOP);
	SetGPIOInfo(&stGPIOInfo,  v_p2450IOP);
 
 #endif
 //===================================
	
	
	GetGPIOInfo(&stGPIOInfo, v_p2450IOP, stGPIOInfo.u32PinNo);
	EPRINT("BACKLIGHT_PWM=%d: [%d %d %d] \r\n",
		 stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	/* Backlight PWM Setting*/
	//v_pPWMregs->TCFG0  |= 24 << 8;	/*Timer 0,1 prescaler value*/
	v_pPWMregs->TCFG1  &= ~(0xf << 8);/* Timer3's Divider Value*/
	v_pPWMregs->TCFG1  |=  (3   << 8);/* 1/16	*/
	DPNOK(v_pPWMregs->TCFG1);
	v_pPWMregs->TCNTB2=150;/*Origin Value*/
	v_pPWMregs->TCMPB2= MAX_PWM_VALUE;

	v_pPWMregs->TCON &= ~(0xf<<TCON_TIMER_SHIFT);
	v_pPWMregs->TCON |=  (0x2<<TCON_TIMER_SHIFT);
	v_pPWMregs->TCON |=  (0x9<<TCON_TIMER_SHIFT);
	v_pPWMregs->TCON |=  (0x9<<TCON_TIMER_SHIFT);
	v_pPWMregs->TCON &=  ~(0x2<<TCON_TIMER_SHIFT);	/*1.69KHz*/

	return 1;

}

int GetPWMValue1()
{
	volatile S3C2450_PWM_REG * v_pPWMregs=NULL;	
	if(NULL==v_pPWMregs) {
		v_pPWMregs = (S3C2450_PWM_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_PWM, FALSE);
	}
	DPNOK(v_pPWMregs->TCMPB2);
	return v_pPWMregs->TCMPB2 ;
}
	
	
void SetPWMValue1(int LightLevel)
{
	volatile S3C2450_PWM_REG * v_pPWMregs=NULL;	
	if(NULL==v_pPWMregs) {
		v_pPWMregs = (S3C2450_PWM_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_PWM, FALSE);
	}

	/*Set the Set the TCNTBn & the TCMPBn Lightlevel*/
#if 0	
	if((0 == g_pBspArgs->misc.ExtPowerStatus)&&(1 == g_pBspArgs->pwr.BatteryVeryLow))
	{
		v_pPWMregs->TCMPB3=(DWORD)(LightLevel*0.5);	
	}
	else 
	{
	v_pPWMregs->TCMPB3=LightLevel;	
	}
#endif

	v_pPWMregs->TCMPB2 = LightLevel;
	/*Clear manual_update bit*/
	v_pPWMregs->TCON &=  ~(0x2<<TCON_TIMER_SHIFT);
	/*Off the inverter*/
	v_pPWMregs->TCON &=  ~(0x4<<TCON_TIMER_SHIFT);
	/*Set the AutoReload*/
	v_pPWMregs->TCON  |= (0x8 <<TCON_TIMER_SHIFT);
	/*Set the start bit*/
	v_pPWMregs->TCON |= (0x1 <<TCON_TIMER_SHIFT);
}




int Backlight_OnOff(int uOn, volatile S3C2450_IOPORT_REG *s2450IOP)
{
 //[david.modify] 2008-06-04 16:01
 // 背光原来使用GPB3，但与TOUCH冲突
 //改成GPB2,但前期装机的机器硬件没改，还是用GPB3
 //==================================
	stGPIOInfo stGPIOInfo_old={
//		BACKLIGHT_PWM,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE
		BACKLIGHT_PWM,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE
	};
	
	if (uOn) {
		stGPIOInfo_old.u32Stat=1;
		SetGPIOInfo(&stGPIOInfo_old,  s2450IOP);
	}
	else{
		stGPIOInfo_old.u32Stat=0;
		SetGPIOInfo(&stGPIOInfo_old,  s2450IOP);
	}
	DPNOK(uOn);
	return 1;
}

int LCDPwr_OnOff(int uOn,  volatile S3C2450_IOPORT_REG *s2450IOP)
{
 //[david.modify] 2008-06-04 16:01
 // 背光原来使用GPB3，但与TOUCH冲突
 //改成GPB2,但前期装机的机器硬件没改，还是用GPB3
 //==================================
	stGPIOInfo stGPIOInfo={
//		BACKLIGHT_PWM,  0, ALT_FUNC_02, PULL_UPDOWN_DISABLE
		LCD_PWREN,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE
	};
	
	if (uOn) {
		stGPIOInfo.u32Stat=1;
		SetGPIOInfo(&stGPIOInfo,  s2450IOP);
	}
	else{
		stGPIOInfo.u32Stat=0;
		SetGPIOInfo(&stGPIOInfo,  s2450IOP);
	}
	DPNOK(uOn);
	return 1;
}

#endif

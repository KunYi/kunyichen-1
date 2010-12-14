#ifndef SPI_DAVID_H
#define SPI_DAVID_H

#include <s3c2450_ioport.h>
#include <xllp_gpio_david.h>


#define LCD_43INCH_WIDTH 480
#define LCD_43INCH_HEIGHT 272
#define LCD_ICPOWER_IO_GPC0 GPC0
#define LCD_BIAS_VOL_IO_GPE12 GPE12

#if 1
#define DbgDelay(count){ unsigned int __dbgcnt; for(__dbgcnt=0; __dbgcnt<count; __dbgcnt++);}	
#else
void WaitUS (DWORD dwUS)
{
	volatile DWORD dCount,dwIndex;

	for (dwIndex=0;dwIndex<dwUS;dwIndex++) {
		for (dCount=0;dCount<100L;dCount++);
	}
}
#define DbgDelay WaitUS

#endif

#ifndef LCD_WIDTH
#define LCD_WIDTH	320
#define LCD_HEIGHT	240
#endif

#ifndef LCD_DEN
#define LCD_DEN		(1<<5)
#define LCD_DSERI	(1<<10)
#define LCD_DCLK	(1<<13)	 //[david.modify] 2008-05-12 18:01

#define LCD_nRESET		9 //	1
#endif




#define LCD_DEN_Lo		(s2450IOP->GPBDAT &= ~LCD_DEN)
#define LCD_DEN_Hi		(s2450IOP->GPBDAT |=	LCD_DEN)
//#define LCD_DCLK_Lo		(s2450IOP->GPBDAT &= ~LCD_DCLK)
//#define LCD_DCLK_Hi		(s2450IOP->GPBDAT |=	LCD_DCLK)
 //[david.modify] 2008-05-12 18:00
 // 因为GPB6用于JTAG用， 现在用GPE13替换GPB6功能
#define LCD_DCLK_Lo		(s2450IOP->GPEDAT &= ~LCD_DCLK)
#define LCD_DCLK_Hi		(s2450IOP->GPEDAT |=	LCD_DCLK)
#define LCD_DSERI_Lo	(s2450IOP->GPBDAT &= ~LCD_DSERI)
#define LCD_DSERI_Hi	(s2450IOP->GPBDAT |=	LCD_DSERI)


#define LCDCSLow LCD_DEN_Lo
#define LCDCSHigh LCD_DEN_Hi
#define LCDClockLow LCD_DCLK_Lo
#define LCDClockHigh LCD_DCLK_Hi
#define LCDDataLow LCD_DSERI_Lo
#define LCDDataHigh LCD_DSERI_Hi

//#define LCD_DELAY_1MS_DAVID	180000	//180000	//on the basis of 540MHz
#if 1
#define LCD_DELAY_1MS_DAVID	180000	//180000	//on the basis of 540MHz
#define LCD_Write_Sharp_DELAY 10000
#else
#define LCD_DELAY_1MS_DAVID	(180000/10)
#define LCD_Write_Sharp_DELAY (10000/10)

#endif


int InitSPIGPIO(volatile S3C2450_IOPORT_REG *s2450IOP)
{
	DPNOK(s2450IOP);
	// LCD_DEN 输出
	s2450IOP->GPBCON &= ~(0x3<< (2*(LCDCS-GPB0)));
	s2450IOP->GPBCON |= (0x1<< (2*(LCDCS-GPB0)));

	// LCD_DCLK 输出
	s2450IOP->GPECON &= ~(0x3<< (2*(LCDCLK-GPE0)));
	s2450IOP->GPECON |= (0x1<<(2*(LCDCLK-GPE0)));

	// LCD_DSERI 输出
	s2450IOP->GPBCON &= ~(0x3<< (2*(LCDSDA-GPB0)));
	s2450IOP->GPBCON |= (0x1<< (2*(LCDSDA-GPB0)));
	


	LCDCSHigh;
	LCDClockHigh;
	LCDDataHigh;
	


	return 1;
}


//写16位数据
static void SDATA_OUT(volatile S3C2450_IOPORT_REG *s2450IOP, UINT16 odata, int count)
{

 int i;
 for (i=0; i<count; i++) 
 {
  LCDClockLow;
  if (odata & 0x8000) 
   {
    LCDDataHigh;
   }
  else 
   {
    LCDDataLow;
   }
  DbgDelay(2);
  LCDClockHigh;
  DbgDelay(2);
  odata <<= 1;
 }
}

void LCD_Write_Sharp (volatile S3C2450_IOPORT_REG *s2450IOP, char regnum, UINT16 instdata)
{
 UINT16 idx;
// 	volatile volatile S3C2450_IOPORT_REG *s2450IOP = (volatile S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
//DPNOK(0);
 
 LCDClockHigh;
   
    LCDCSLow;  
    idx = (regnum)<<7;
    SDATA_OUT(s2450IOP, idx, 9);
    DbgDelay(500); 
    LCDCSHigh;
 
    DbgDelay(1000);
 
    LCDCSLow;
 idx = ((instdata>>8)|0x0100)<<7;
    SDATA_OUT(s2450IOP, idx, 9);
 DbgDelay(500);
    LCDCSHigh;
 
 LCDCSLow;
 idx = ((instdata&0x00ff)|0x0100)<<7;
    SDATA_OUT(s2450IOP, idx, 9);
 DbgDelay(500);
    LCDCSHigh;
 
    DbgDelay(LCD_Write_Sharp_DELAY);

//DPNOK(0);	
}

//Innolux, 群创
void LCD_Write_Innolux(volatile S3C2450_IOPORT_REG *s2450IOP, char regnum, UINT16 instdata)
{
	WORD idx = 0;
	//Clock High
	LCDClockHigh;

	// CS low
	LCDCSLow;
	idx = (regnum & 0x3f) << 10;
	idx |= (0x0200 | instdata);
    SDATA_OUT(s2450IOP, idx, 16);

	LCDCSHigh;

	DbgDelay(10000);
}

void InitLDI_LTV350_sharp(volatile S3C2450_IOPORT_REG *s2450IOP)
{
	DPNOK(0);		
	if(0==s2450IOP) {
		DPN(-1);
		return;
	}
	InitSPIGPIO(s2450IOP);	
	// LCD module reset
	s2450IOP->GPBDAT |= (1<<(LCD_nRESET*2));
	s2450IOP->GPBDAT &= ~(1<<(LCD_nRESET*2)); // goes to LOW
	// delay about 5ms
	DbgDelay(LCD_DELAY_1MS_DAVID*10);
	s2450IOP->GPBDAT |= (1<<(LCD_nRESET*2));  // goes to HIGH



	// delay about 5ms
	DbgDelay(LCD_DELAY_1MS_DAVID*10);

	LCDCSHigh;
	LCDClockHigh;
	LCDDataHigh;


	///////////////////////////////////////////////////////////////////
	// Init_Lcd_Function
	//////////////////////////////////////////////////////////////////
LCD_Write_Sharp(s2450IOP,0x01,0x2AEF); //   zheng:0x2AEF, fang:0x29EF
 
  LCD_Write_Sharp(s2450IOP,0x02,0x0300);
  LCD_Write_Sharp(s2450IOP,0x03,0x7A7E); 
  LCD_Write_Sharp(s2450IOP,0x0B,0xDC00); 
  LCD_Write_Sharp(s2450IOP,0x0C,0x0005);
  LCD_Write_Sharp(s2450IOP,0x0D,0x0002);
  LCD_Write_Sharp(s2450IOP,0x0E,0x2900);
  LCD_Write_Sharp(s2450IOP,0x0F,0x0000);
  LCD_Write_Sharp(s2450IOP,0x16,0x9F86);
  LCD_Write_Sharp(s2450IOP,0x17,0x0002);
  LCD_Write_Sharp(s2450IOP,0x1E,0x0000);
  
  LCD_Write_Sharp(s2450IOP,0x28,0x0006);
  LCD_Write_Sharp(s2450IOP,0x2C,0xC88C);
  LCD_Write_Sharp(s2450IOP,0x2E,0xB945);
  LCD_Write_Sharp(s2450IOP,0x30,0x0000);
  
  LCD_Write_Sharp(s2450IOP,0x31,0x0707);
  LCD_Write_Sharp(s2450IOP,0x32,0x0003);
  LCD_Write_Sharp(s2450IOP,0x33,0x0401);
  LCD_Write_Sharp(s2450IOP,0x34,0x0307);
  LCD_Write_Sharp(s2450IOP,0x35,0x0000);
  LCD_Write_Sharp(s2450IOP,0x36,0x0707);
  LCD_Write_Sharp(s2450IOP,0x37,0x0204);
  LCD_Write_Sharp(s2450IOP,0x3A,0x0D0B);
  LCD_Write_Sharp(s2450IOP,0x3B,0x0D0B);
  LCD_Write_Sharp(s2450IOP,0x40,0x0000);
  LCD_Write_Sharp(s2450IOP,0x41,0x0707);
  LCD_Write_Sharp(s2450IOP,0x42,0x0003);
  
  LCD_Write_Sharp(s2450IOP,0x43,0x0401);
  LCD_Write_Sharp(s2450IOP,0x44,0x0307);
  LCD_Write_Sharp(s2450IOP,0x45,0x0000);
  LCD_Write_Sharp(s2450IOP,0x46,0x0707);
  LCD_Write_Sharp(s2450IOP,0x47,0x0204);
  LCD_Write_Sharp(s2450IOP,0x4A,0x0D0B);
  LCD_Write_Sharp(s2450IOP,0x4B,0x0D0B);
  LCD_Write_Sharp(s2450IOP,0x50,0x0000);
  LCD_Write_Sharp(s2450IOP,0x51,0x0707);
  LCD_Write_Sharp(s2450IOP,0x52,0x0003);
  LCD_Write_Sharp(s2450IOP,0x53,0x0401);
  LCD_Write_Sharp(s2450IOP,0x54,0x0307);
  LCD_Write_Sharp(s2450IOP,0x55,0x0000);
  LCD_Write_Sharp(s2450IOP,0x56,0x0707);
  LCD_Write_Sharp(s2450IOP,0x57,0x0204);
  LCD_Write_Sharp(s2450IOP,0x5A,0x0D0B);
  LCD_Write_Sharp(s2450IOP,0x5B,0x0D0B);

}


void InitLDI_LTV350_sharp2_org(volatile S3C2450_IOPORT_REG *s2450IOP)
{

	DPNOK(0);		

	if(0==s2450IOP) {
		DPN(-1);
		return;
	}
	InitSPIGPIO(s2450IOP);
	
	// LCD module reset
	s2450IOP->GPBDAT |= (1<<(LCD_nRESET*2));
	s2450IOP->GPBDAT &= ~(1<<(LCD_nRESET*2)); // goes to LOW
	// delay about 5ms
	DbgDelay(LCD_DELAY_1MS_DAVID*10);
	s2450IOP->GPBDAT |= (1<<(LCD_nRESET*2));  // goes to HIGH



	// delay about 5ms
	DbgDelay(LCD_DELAY_1MS_DAVID*10);

	LCDCSHigh;
	LCDClockHigh;
	LCDDataHigh;

	///////////////////////////////////////////////////////////////////
	// Init_Lcd_Function
	//////////////////////////////////////////////////////////////////
//		DbgDelay(LCD_DELAY_1MS_DAVID*10);
#if 0
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x01,0x2AEF); //   zheng:0x2AEF, fang:0x29EF

 
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x02,0x0300);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x03,0x7A7E); 
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x0C,0x0005);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x0D,0x0002);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x0E,0x2600);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x1E,0x0000);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x2E,0xB945);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x30,0x0707);
  
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x31,0x0307);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x32,0x0507);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x33,0x0104);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x34,0x0002);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x35,0x0004);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x36,0x0000);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x37,0x0501);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x3A,0x0D0B);
   LCD_Write_Sharp(s2450IOP,s2450IOP, 0x3B,0x0D0B);
  
    LCD_Write_Sharp(s2450IOP,s2450IOP, 0x28,0x0006);
    LCD_Write_Sharp(s2450IOP,s2450IOP, 0x2c,0xc88c);
#else
// better your see angle
  LCD_Write_Sharp(s2450IOP, 0x01,0x2aEF); 
  LCD_Write_Sharp(s2450IOP, 0x02,0x0300);
  LCD_Write_Sharp(s2450IOP, 0x03,0x7A7E); 
  LCD_Write_Sharp(s2450IOP, 0x0B,0xDC00); 
  LCD_Write_Sharp(s2450IOP, 0x0C,0x0005);
 //delay_50ns(100);
 DbgDelay(LCD_DELAY_1MS_DAVID); 
  LCD_Write_Sharp(s2450IOP, 0x0D,0x0002);
// delay_50ns(100);
 DbgDelay(LCD_DELAY_1MS_DAVID); 
 LCD_Write_Sharp(s2450IOP, 0x0E,0x3200); 
// delay_50ns(100);
 DbgDelay(LCD_DELAY_1MS_DAVID); 
  LCD_Write_Sharp(s2450IOP, 0x0F,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x16,0x9F86);
  LCD_Write_Sharp(s2450IOP, 0x17,0x0002);
// delay_50ns(100);
  DbgDelay(LCD_DELAY_1MS_DAVID); 
  LCD_Write_Sharp(s2450IOP, 0x1E,0x00b2);
// delay_50ns(100);
 DbgDelay(LCD_DELAY_1MS_DAVID); 
  LCD_Write_Sharp(s2450IOP, 0x2E,0xB945);
  LCD_Write_Sharp(s2450IOP, 0x30,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x31,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x32,0x0300);
  LCD_Write_Sharp(s2450IOP, 0x33,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x34,0x0705);
  LCD_Write_Sharp(s2450IOP, 0x35,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x36,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x37,0x0700);
  LCD_Write_Sharp(s2450IOP, 0x3A,0x0D0B); 
  LCD_Write_Sharp(s2450IOP, 0x3B,0x0D0B); 
  LCD_Write_Sharp(s2450IOP, 0x28,0x0006);
  LCD_Write_Sharp(s2450IOP, 0x2C,0xC88C);
#endif
   

}

#define delay_50ns(count) { unsigned int __dbgcnt; for(__dbgcnt=0; __dbgcnt<count*1000; __dbgcnt++);}	

#define NEW_SHARP 1


void InitLDI_LTV350_sharp2(volatile S3C2450_IOPORT_REG *s2450IOP)
{
UINT8 g_u8Temp;
	DPNOK(0);		

	if(0==s2450IOP) {
		DPN(-1);
		return;
	}
	InitSPIGPIO(s2450IOP);
	
	// LCD module reset
	s2450IOP->GPBDAT |= (1<<(LCD_nRESET*2));
	s2450IOP->GPBDAT &= ~(1<<(LCD_nRESET*2)); // goes to LOW
	// delay about 5ms
	DbgDelay(LCD_DELAY_1MS_DAVID*10);
	s2450IOP->GPBDAT |= (1<<(LCD_nRESET*2));  // goes to HIGH



	// delay about 5ms
	DbgDelay(LCD_DELAY_1MS_DAVID*10);

	LCDCSHigh;
	LCDClockHigh;
	LCDDataHigh;

	///////////////////////////////////////////////////////////////////
	// Init_Lcd_Function
	//////////////////////////////////////////////////////////////////
//	DPSTR("1-NEW_SHARP 2-t2 3--old");
//			g_u8Temp = ReadSerialByte();
g_u8Temp='1';				
//		DbgDelay(LCD_DELAY_1MS_DAVID*10);
if('1'==g_u8Temp){
  LCD_Write_Sharp(s2450IOP, 0x01,0x2aEF);
  LCD_Write_Sharp(s2450IOP, 0x02,0x0300);
  LCD_Write_Sharp(s2450IOP, 0x03,0x7A7E);
  LCD_Write_Sharp(s2450IOP, 0x0B,0xDC00);
  LCD_Write_Sharp(s2450IOP, 0x0C,0x0005);
  delay_50ns(40);
  LCD_Write_Sharp(s2450IOP, 0x0D,0x0002);
  delay_50ns(40);
  LCD_Write_Sharp(s2450IOP, 0x0E,0x3200);
  delay_50ns(40);
  LCD_Write_Sharp(s2450IOP, 0x0F,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x16,0x9F86);
  LCD_Write_Sharp(s2450IOP, 0x17,0x0002);
  delay_50ns(40);
  LCD_Write_Sharp(s2450IOP, 0x1E,0x00b2);
  delay_50ns(40);

  LCD_Write_Sharp(s2450IOP, 0x28,0x0006);
  LCD_Write_Sharp(s2450IOP, 0x2C,0xC88C);
  LCD_Write_Sharp(s2450IOP, 0x2E,0xB945);

  LCD_Write_Sharp(s2450IOP, 0x30,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x31,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x32,0x0300);
  LCD_Write_Sharp(s2450IOP, 0x33,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x34,0x0705);
  LCD_Write_Sharp(s2450IOP, 0x35,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x36,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x37,0x0700);
  LCD_Write_Sharp(s2450IOP, 0x3A,0x0D0B);
  LCD_Write_Sharp(s2450IOP, 0x3B,0x0D0B);

  LCD_Write_Sharp(s2450IOP, 0x40,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x41,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x42,0x0003);

  LCD_Write_Sharp(s2450IOP, 0x43,0x0401);
  LCD_Write_Sharp(s2450IOP, 0x44,0x0307);
  LCD_Write_Sharp(s2450IOP, 0x45,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x46,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x47,0x0204);
  LCD_Write_Sharp(s2450IOP, 0x4A,0x0D0B);
  LCD_Write_Sharp(s2450IOP, 0x4B,0x0D0B);
  LCD_Write_Sharp(s2450IOP, 0x50,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x51,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x52,0x0003);
  LCD_Write_Sharp(s2450IOP, 0x53,0x0401);
  LCD_Write_Sharp(s2450IOP, 0x54,0x0307);
  LCD_Write_Sharp(s2450IOP, 0x55,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x56,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x57,0x0204);
  LCD_Write_Sharp(s2450IOP, 0x5A,0x0D0B);
  LCD_Write_Sharp(s2450IOP, 0x5B,0x0D0B);

  delay_50ns(500);  
}
else if('2'==g_u8Temp){
  delay_50ns(100);
  LCD_Write_Sharp(s2450IOP, 0x01,0x2AEF); //   zheng:0x2AEF, fang:0x29EF

  LCD_Write_Sharp(s2450IOP, 0x02,0x0300);
//  LCD_Write_Sharp(s2450IOP, 0x03,0x7A7E);
  LCD_Write_Sharp(s2450IOP, 0x03,0x7A72);
  LCD_Write_Sharp(s2450IOP, 0x0B,0xDC00);
  LCD_Write_Sharp(s2450IOP, 0x0C,0x0005);
  LCD_Write_Sharp(s2450IOP, 0x0D,0x0002);
//  LCD_Write_Sharp(s2450IOP, 0x0E,0x2900);
  LCD_Write_Sharp(s2450IOP, 0x0E,0x2800);

  LCD_Write_Sharp(s2450IOP, 0x0F,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x16,0x9F86);
  LCD_Write_Sharp(s2450IOP, 0x17,0x0002);
//  LCD_Write_Sharp(s2450IOP, 0x1E,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x1E,0x00AA);

  LCD_Write_Sharp(s2450IOP, 0x28,0x0006);
  LCD_Write_Sharp(s2450IOP, 0x2C,0xC88C);
  LCD_Write_Sharp(s2450IOP, 0x2E,0xB945);
  LCD_Write_Sharp(s2450IOP, 0x30,0x0000);
//LCD_Write_Sharp(s2450IOP, 0x30,0x0707);

  LCD_Write_Sharp(s2450IOP, 0x31,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x32,0x0003);
  LCD_Write_Sharp(s2450IOP, 0x33,0x0000);		// PRP  0X0401
  LCD_Write_Sharp(s2450IOP, 0x34,0x0307);
  LCD_Write_Sharp(s2450IOP, 0x35,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x36,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x37,0x0102);		// PRN 0X0204
  LCD_Write_Sharp(s2450IOP, 0x3A,0x070F);		// VRP 0X070F
  LCD_Write_Sharp(s2450IOP, 0x3B,0x0F03);		// VRN // 0X0F03

  LCD_Write_Sharp(s2450IOP, 0x40,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x41,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x42,0x0003);

  LCD_Write_Sharp(s2450IOP, 0x43,0x0401);
  LCD_Write_Sharp(s2450IOP, 0x44,0x0307);
  LCD_Write_Sharp(s2450IOP, 0x45,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x46,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x47,0x0204);
  LCD_Write_Sharp(s2450IOP, 0x4A,0x0D0B);
  LCD_Write_Sharp(s2450IOP, 0x4B,0x0D0B);
  LCD_Write_Sharp(s2450IOP, 0x50,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x51,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x52,0x0003);
  LCD_Write_Sharp(s2450IOP, 0x53,0x0401);
  LCD_Write_Sharp(s2450IOP, 0x54,0x0307);
  LCD_Write_Sharp(s2450IOP, 0x55,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x56,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x57,0x0204);
  LCD_Write_Sharp(s2450IOP, 0x5A,0x0D0B);
  LCD_Write_Sharp(s2450IOP, 0x5B,0x0D0B);

  delay_50ns(500);  
}
else if('3'==g_u8Temp){
// better your see angle
  LCD_Write_Sharp(s2450IOP, 0x01,0x2aEF); 
  LCD_Write_Sharp(s2450IOP, 0x02,0x0300);
  LCD_Write_Sharp(s2450IOP, 0x03,0x7A7E); 
  LCD_Write_Sharp(s2450IOP, 0x0B,0xDC00); 
  LCD_Write_Sharp(s2450IOP, 0x0C,0x0005);
 //delay_50ns(100);
 DbgDelay(LCD_DELAY_1MS_DAVID); 
  LCD_Write_Sharp(s2450IOP, 0x0D,0x0002);
// delay_50ns(100);
 DbgDelay(LCD_DELAY_1MS_DAVID); 
 LCD_Write_Sharp(s2450IOP, 0x0E,0x3200); 
// delay_50ns(100);
 DbgDelay(LCD_DELAY_1MS_DAVID); 
  LCD_Write_Sharp(s2450IOP, 0x0F,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x16,0x9F86);
  LCD_Write_Sharp(s2450IOP, 0x17,0x0002);
// delay_50ns(100);
  DbgDelay(LCD_DELAY_1MS_DAVID); 
  LCD_Write_Sharp(s2450IOP, 0x1E,0x00b2);
// delay_50ns(100);
 DbgDelay(LCD_DELAY_1MS_DAVID); 
  LCD_Write_Sharp(s2450IOP, 0x2E,0xB945);
  LCD_Write_Sharp(s2450IOP, 0x30,0x0000);
  LCD_Write_Sharp(s2450IOP, 0x31,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x32,0x0300);
  LCD_Write_Sharp(s2450IOP, 0x33,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x34,0x0705);
  LCD_Write_Sharp(s2450IOP, 0x35,0x0007);
  LCD_Write_Sharp(s2450IOP, 0x36,0x0707);
  LCD_Write_Sharp(s2450IOP, 0x37,0x0700);
  LCD_Write_Sharp(s2450IOP, 0x3A,0x0D0B); 
  LCD_Write_Sharp(s2450IOP, 0x3B,0x0D0B); 
  LCD_Write_Sharp(s2450IOP, 0x28,0x0006);
  LCD_Write_Sharp(s2450IOP, 0x2C,0xC88C);


}



}




void SetLCDColor(UINT16 u16Color)
{
	int i = 0;
	UINT16 * pFB = NULL;
#if 1
	pFB = (unsigned short *)IMAGE_FRAMEBUFFER_UA_BASE;
	for (i=0; i<LCD_WIDTH*(LCD_HEIGHT); i++){
		*pFB++ =u16Color;		// red
	}	
#endif	

}

void SetLCDColorRow(UINT16 u16Color, UINT16 nStartRow, UINT16 nEndRow)
{
	int i = 0, j=0;
	UINT16 * pFB = NULL;

	pFB = (unsigned short *)IMAGE_FRAMEBUFFER_UA_BASE;
	for (i=nStartRow; i<=nEndRow; i++){
		pFB = (unsigned short *)IMAGE_FRAMEBUFFER_UA_BASE+i*LCD_WIDTH;
		for(j=0;j<LCD_WIDTH;j++) {
			*pFB++ =u16Color;		// red
		}
	}	
}


void LCD_Sharp_Enter2Sleep(volatile S3C2450_IOPORT_REG *s2450IOP )
{
 //  	volatile volatile S3C2450_IOPORT_REG *s2450IOP = (volatile S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
//    	volatile S3C2450_LCD_REG    *s2450LCD = (S3C2450_LCD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
	UINT16 * pFB = NULL;
	int i=0;

	if(0==s2450IOP) {
		DPN(-1);
		return;
	}
	InitSPIGPIO(s2450IOP);	
	
	DPNOK(0);		

	LCDCSHigh;
	LCDClockHigh;
	LCDDataHigh;

// 1. Write White Data (all high level)
#if 1
	pFB = (unsigned short *)IMAGE_FRAMEBUFFER_UA_BASE;
	for (i=0; i<LCD_WIDTH*(LCD_HEIGHT); i++){
		*pFB++ = 0xFFFF;		// red
	}	
#endif	
	DbgDelay(LCD_DELAY_1MS_DAVID*50);
	
// 2. SPI CMD ( 0x28 = 0x0006 ; 0x2d = 0x7f06 )
	  LCD_Write_Sharp(s2450IOP, 0x28,0x0006);
	 DbgDelay(LCD_DELAY_1MS_DAVID); 
	  LCD_Write_Sharp(s2450IOP, 0x2d,0x7f06);
	 DbgDelay(LCD_DELAY_1MS_DAVID); 

	DbgDelay(LCD_DELAY_1MS_DAVID*50);	 

}

void LCD_Sharp_ExitFromSleep(volatile S3C2450_IOPORT_REG *s2450IOP )
{
//    	volatile volatile S3C2450_IOPORT_REG *s2450IOP = (volatile S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
//    	volatile S3C2450_LCD_REG    *s2450LCD = (S3C2450_LCD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);
	UINT16 * pFB = NULL;
	int i=0;

	if(0==s2450IOP) {
		DPN(-1);
		return;
	}
	InitSPIGPIO(s2450IOP);	
	
	DPNOK(0);		

	LCDCSHigh;
	LCDClockHigh;
	LCDDataHigh;

// 1. Write White Data (all high level)
#if 0
	pFB = (unsigned short *)IMAGE_FRAMEBUFFER_UA_BASE;
	for (i=0; i<LCD_WIDTH*(LCD_HEIGHT); i++){
		*pFB++ = 0xFFFF;		// red
	}	

	DbgDelay(LCD_DELAY_1MS_DAVID*50);
#endif	
	
// 2. SPI CMD ( 0x28 = 0x0006 ; 0x2d = 0x7f06 )
	  LCD_Write_Sharp(s2450IOP, 0x28,0x0006);
	 DbgDelay(LCD_DELAY_1MS_DAVID); 
	  LCD_Write_Sharp(s2450IOP, 0x2d,0x7f04);
	 DbgDelay(LCD_DELAY_1MS_DAVID); 

	 DbgDelay(LCD_DELAY_1MS_DAVID*50);	 
// 	 memcpy((void *)IMAGE_FRAMEBUFFER_UA_BASE, prayer16bpp, LCD_ARRAY_SIZE_TFT_16BIT);	

}





//[david.modify] 2008-08-28 11:27
//============================================
#if 1
DWORD	g_dwSysIoClock;

struct LCDCmdStruct {
	char regidx;
	WORD instcode;
};

typedef struct  {
	char regIndex;
	char RegValud;	
}NOVATEKINITREG;
//Himax
typedef struct  {
	WORD regIndex;
	WORD RegValud;	
}STRHIMAX;
//*


#define DEBUG_NEW 1



 //[david.modify] 2008-10-14 16:06






//biyadi 比亚迪
 STRHIMAX BiyadiCmd[]=
{


 //[david.modify] 2008-10-14 16:07
 // from 台湾吴工给的参数
#if 1
{0x0001, 0x6300},
{0x0002, 0x0200},
//{0x0002, 0x0000},
{0x0003, 0x6164},
{0x0004, 0x0447},
{0x0005, 0xBCD4},
{0x0006, 0xE860},	//DRIVE CAP
//{0x000a, 0x3F08},
{0x000a, 0x4008},
{0x000b, 0xD400},
{0x000d, 0x123A},
{0x000e, 0x2C00},
{0x000f, 0x0000},
{0x0016, 0x9F80},
{0x0017, 0x2212},
//{0x001e, 0x00D7},
 //[david.modify] 2008-10-21 11:12
{0x001e, 0x00DF},

{0x0030, 0x0000},
{0x0031, 0x0004},
{0x0032, 0x0000},
{0x0033, 0x0000},
{0x0034, 0x0704},
{0x0035, 0x0307},
{0x0036, 0x0707},
{0x0037, 0x0000},
{0x003a, 0x140B},
{0x003b, 0x140B}

#endif



 //[david.modify] 2008-10-11 11:03
 // from刘工
 //=======================
#if 0
#if 0
  {0x000e,0x2ec0}, //   0x2ec0 0x2bc0
  {0X0017,0X2295},
  {0x001e,0x00dB}
#else
 {0x0001,0x6300},
 {0x0002,0x0200},
 {0x0003,0x7166},
 {0x0004,0x0447},
 {0x0005,0xbcd4},
// {0x0005,0xb4d4},	 //[david.modify] 2008-10-14 17:07
 {0x000A,0x3f08},
 {0x000B,0xd400},
 {0x000d,0x123a},
 {0x000e,0x3100},
//  {0x000e,0x2ec0},
  {0x000e,0x28c0},  //[david.modify] 2008-10-13 15:06
 {0x000f,0x0000},
// {0x0016,0x9f86},
 {0x0016,0x9f80},  //[david.modify] 2008-10-14 17:08
 {0x0017,0x2212},	//hbp=68
// {0x001e,0x00e3} 	
 {0x001e,0x00d5} 	 //[david.modify] 2008-10-13 15:06

//  {0X0017,0X2295},
//  {0x001e,0x00dB}
#endif
#endif

 //=======================
 
	//35B Ver1.2
#if 0
	{0x0001,0x633f},
	{0x0002,0x0200},
	{0x0003,0xa164},
	{0x0004,0x0447},
	{0x0005,0xfcd4},
	{0x000A,0x4008},
	{0x000B,0xc470},
	{0x000d,0x123A},

	 //[david.modify] 2008-10-09 18:02
#if DEBUG_NEW	 
	{0x000e,0x28C0},	
#else	 
	{0x000e,0x2aC0},	//0x2c00 0x2BC0
#endif	
	{0x000f,0x0000},
	{0x0016,0x9f86},

#if DEBUG_NEW	 
	 {0x001e,0x00d5},
#else	 
	{0x001e,0x00d5},	//0x00d6
#endif		

	{0x0030,0x0507},
	{0x0031,0x0004},
	{0x0032,0x0707},
	{0x0033,0x0000},
	{0x0034,0x0000},
	{0x0035,0x0307},
	{0x0036,0x0405},
	{0x0037,0x0703},
	{0x003a,0x140B},
	{0x003b,0x140B}	
#endif

//35K GPS35J1 ver1.0
#if 0
	//old 35G 60 Pin alse use this
	/*
	{0x0001,0x633f},
	{0x0002,0x0200},
	{0x0003,0xa164},
	{0x0004,0x0447},
	{0x0005,0xfcd4},
	{0x000A,0x4008},
	{0x000B,0xc470},
	{0x000d,0x123A},
	*/

	{0x000e,0x2ec0},	//2ec0 0x2bc0

	/*
	{0x000f,0x0000},
	{0x0016,0x9f86},
	*/

	{0x0017,0x1691},
	{0x001e,0x00d8}		//00d8 

	/*
	{0x0030,0x0507},
	{0x0031,0x0004},
	{0x0032,0x0707},
	{0x0033,0x0000},
	{0x0034,0x0000},
	{0x0035,0x0307},
	{0x0036,0x0405},
	{0x0037,0x0703},
	{0x003a,0x140B},
	{0x003b,0x140B}
	*/

	//GPS35K41 Ver1.41:
	/*
	{0x000e,0x2ec0},
	{0x001e,0x00db}
	*/
	//35K高亮
	/*
	{0X000E,0X2EF2},
	{0X0017,0X2591},
	{0X001E,0X00D6}
	*/
#endif


 //[david.modify] 2008-10-14 16:37
 //from 4281l(hx8238)initial.txt
 #if 0

      {0x0001, 0x633f},		//driver output
      {0x0002, 0x0200},			//LCD driving waveform

      {0x0003, 0xa164},			//power control 1
      {0x0004, 0x0447},			//input data and color filter
      {0x0005, 0xfcd4},			//function control

      {0x000A, 0x4008},			//contrast/brightness
      {0x000B, 0xc470},		//frame cycle control
      {0x000d, 0x123A},			//power control 2
      {0x000e, 0x2c00},		//power control 3
      {0x000f, 0x0000},		//gata scan position
      {0x0016, 0x9f86},		//horizontal porch
      {0x0017, 0x2212},			//vertical porch
      {0x001e, 0x00d6},			//power control 4
 
      {0x0030, 0x0507},			//gamma control
      {0x0031, 0x0004},			//
      {0x0032, 0x0707},		//
      {0x0033, 0x0000},			//
      {0x0034, 0x0000},			//
      {0x0035, 0x0307},			//
      {0x0036, 0x0405},			//
      {0x0037, 0x0703},			//
      {0x003a, 0x140B},		//
      {0x003b, 0x140B}		//
 	
 
 #endif


};

//Sharp GPS35B Ver 1.2
struct LCDCmdStruct SharpSetCmd[] = 
{		
	0x01,0x2AEF, 
	0x02,0x0300, 
	0x03,0x7A7E, 	
	0x0C,0x0005, 
	0x0D,0x0002,  
	0x0E,0x2700,	
	0x1E,0x0002,
	0x2E,0xB945,
	0x28,0x0006,
	0x2C,0xC88C,
	0x30,0x0000,		//0x0000, 0x0707
	0x31,0x0707,
	0x32,0x0003,
	0x33,0x0402,
 	0x34,0x0307,
	0x35,0x0000,
 	0x36,0x0707,
 	0x37,0x0204,
 	0x3A,0x0D0B,
	0x3B,0x0D0B,
};

//群创
NOVATEKINITREG Innolux[]=
{	
	{0x00, 0x03},
	{0x01, 0x40},
	{0x02, 0x19},
	{0x03, 0xcc},
	{0x04, 0x30},
	{0x05, 0x12},
	{0x07, 0x03},
	{0x08, 0x08},
	{0x09, 0x40},
	{0x0a, 0x88},
	{0x0b, 0x88},
	{0x0c, 0x20},
	{0x0d, 0x20} 	
};

//写32位数据
void SDATA_OUT2(volatile S3C2450_IOPORT_REG *s2450IOP, DWORD odata, int count)
{
	int i;
	for (i=0; i<count; i++) 
	{
		LCDClockLow;
#if 0
	g_TmpstGPIOInfo.u32PinNo = LCDCLK;
	g_TmpstGPIOInfo.u32AltFunc  = ALT_FUNC_OUT;
	g_TmpstGPIOInfo.u32Stat= 0;
	SetGPIOInfo(&g_TmpstGPIOInfo, s2450IOP);	
//	DPSTR("LCDCLK=0");
//	g_u8Temp = ReadSerialByte();		
#endif
	
		if (odata & 0x80000000) 
		{
			LCDDataHigh;
		}
		else 
		{
			LCDDataLow;
		}
		DbgDelay(2);		
		LCDClockHigh;
#if 0
	g_TmpstGPIOInfo.u32PinNo = LCDCLK;
	g_TmpstGPIOInfo.u32AltFunc  = ALT_FUNC_OUT;
	g_TmpstGPIOInfo.u32Stat= 1;
	SetGPIOInfo(&g_TmpstGPIOInfo, s2450IOP);	
//	DPSTR("LCDCLK=1");	
//	g_u8Temp = ReadSerialByte();		
#endif		
		DbgDelay(2);
		odata <<= 1;
	}
}



//Himax
void LCD_Write_Himax(volatile S3C2450_IOPORT_REG *s2450IOP,WORD regNum, WORD instdata)
{	
    DWORD idx=0;

//		DPNOK(regNum);
	LCDClockHigh;
    // 1. CSB low
    LCDCSLow; 
    idx = regNum;
	idx = ((idx<<8)|0x70000000); 

	// 2. set register index
    SDATA_OUT2(s2450IOP,idx, 24);
	//SDATA_OUT(0x70, 8);
	//SDATA_OUT(regNum, 16);
    DbgDelay(500);	
    LCDCSHigh;
    DbgDelay(1000);
    LCDCSLow;	
    idx = instdata;
	idx = (idx<<8)|0x72000000;

	// 3. write instruction
    SDATA_OUT2(s2450IOP,idx, 24);
	//DbgDelay(500);
	//SDATA_OUT(0x72, 8);
	//SDATA_OUT(instdata, 16);
    LCDCSHigh;	
    DbgDelay(10000);
	
}


void SDATA_OUT_HimaxCIA(volatile S3C2450_IOPORT_REG *s2450IOP, WORD odata, int count)
{
	int i;
	for (i=0; i<count; i++) 
	{
		//LCDClockLow();
		if (odata & 0x8000) 
		{
			LCDDataHigh;
		}
		else 
		{
			LCDDataLow;
		}

		DbgDelay(1);

		LCDClockHigh;
		DbgDelay(1);
		LCDClockLow;
		DbgDelay(1);
		
		//DbgDelay(2);
		odata <<= 1;
	}

	DbgDelay(1);
	LCDDataHigh;
	LCDCSHigh;
    LCDClockHigh;  

}


//Himax CIA
void LCD_Write_HimaxCIA(volatile S3C2450_IOPORT_REG *s2450IOP,WORD value, int count)
{
	LCDCSHigh;
  	LCDDataHigh;
    LCDClockHigh;  
    LCDCSLow;  
    LCDClockLow; 
    LCDDataLow;
	SDATA_OUT_HimaxCIA(s2450IOP,value, count);    
	//SDATA_OUT(0x030c, 16);
	//SDATA_OUT(0x59, 8);
    DbgDelay(50);	
    LCDCSHigh;
}




static stGPIOInfo g_TmpstGPIOInfo2;
void BspLcdPowerUpPanel(volatile S3C2450_IOPORT_REG *s2450IOP, UINT32 u32LCDType )
{
	int i;
	UINT32 g_u32Temp;

	DPNOK(u32LCDType);		

	if(0==s2450IOP) {
		DPN(-1);
		return;
	}
	InitSPIGPIO(s2450IOP);

	// LCD module reset
	DPNOK(s2450IOP);
	g_TmpstGPIOInfo2.u32PullUpdown = PULL_UPDOWN_DISABLE;	
	g_TmpstGPIOInfo2.u32PinNo = LCDRST;
	g_TmpstGPIOInfo2.u32AltFunc  = ALT_FUNC_OUT;
	g_TmpstGPIOInfo2.u32Stat= 0;
	SetGPIOInfo(&g_TmpstGPIOInfo2, s2450IOP);	

//   	DbgDelay(40000); 
   	DbgDelay(40000); 
//	DPSTR("1-reset=low?");
	
	// LCD module reset
	g_TmpstGPIOInfo2.u32PullUpdown = PULL_UPDOWN_DISABLE;	
	g_TmpstGPIOInfo2.u32PinNo = LCDRST;
	g_TmpstGPIOInfo2.u32AltFunc  = ALT_FUNC_OUT;
	g_TmpstGPIOInfo2.u32Stat= 1;
	SetGPIOInfo(&g_TmpstGPIOInfo2, s2450IOP);	
//	DPSTR("1-reset=low?");
//	g_u8Temp = ReadSerialByte();	
	
// delay about 5ms
//	DbgDelay(LCD_DELAY_1MS_DAVID*10);

	LCDCSHigh;
	LCDClockHigh;
	LCDDataHigh;

	///////////////////////////////////////////////////////////////////
	// Init_Lcd_Function
	//////////////////////////////////////////////////////////////////
#if 0	
	DPSTR("1-BSP_LCD_BYD_4281L 2-BSP_LCD_YASSY_YF35F03CIB 3--BSP_LCD_SHARP_LQ035Q1");
	g_u32Temp = ReadSerialByte();
	switch(g_u32Temp){
	case '1': g_u32Temp=BSP_LCD_BYD_4281L;break;
	case '2': g_u32Temp=BSP_LCD_YASSY_YF35F03CIB;break;
	case '3': g_u32Temp=BSP_LCD_SHARP_LQ035Q1;break;	
	default:	g_u32Temp=0;break;
	}	
#else
	g_u32Temp=u32LCDType;
#endif


	if(BSP_LCD_YASSY_YF35F03CIB==g_u32Temp) {
	 //[david.modify] 2008-08-29 18:47 BSP_ATLAS_YASSY == 1
	 //我们用此屏//YASSY YF3503CIA
		////////////////////////////////////////////////////////////////
		//yassy cia not use DEN ,in 35K
 		LCD_Write_HimaxCIA(s2450IOP, 0x0B01, 16); //not use DEN
		LCD_Write_HimaxCIA(s2450IOP, 0x0112, 16); 
 		LCD_Write_HimaxCIA(s2450IOP, 0x066f, 16); 
 		LCD_Write_HimaxCIA(s2450IOP, 0x0a51, 16);	
	}
	else if(BSP_LCD_BYD_4281L==g_u32Temp){

	//Biyadi 60 Pin 35K31 Ver1.31
	//比亚迪4281L
	for(i=0;i<sizeof(BiyadiCmd)/sizeof(BiyadiCmd[0]);i++)
		LCD_Write_Himax(s2450IOP, BiyadiCmd[i].regIndex,BiyadiCmd[i].RegValud);

	}
	else if(BSP_LCD_SHARP_LQ035Q1==g_u32Temp){
	//SHARP LCD

	for (i=0; i < sizeof(SharpSetCmd)/sizeof(SharpSetCmd[0]); i++) 
	{
		LCD_Write_Sharp(s2450IOP,SharpSetCmd[i].regidx, SharpSetCmd[i].instcode);
	} 

/*
	DPSTR("+InitLDI_LTV350_sharp2");
		InitLDI_LTV350_sharp2(s2450IOP);
	DPSTR("-InitLDI_LTV350_sharp2");
*/	
	}
	else if( BSP_LCD_INNOLUX_35==g_u32Temp ) {
		//RETAILMSG(1, (TEXT("!!LCD_Write_Innolux\r\n")));
		for( i = 0; i < sizeof(Innolux)/sizeof(Innolux[0]); i++)
			LCD_Write_Innolux(s2450IOP,Innolux[i].regIndex ,Innolux[i].RegValud );
	
	}
	DbgDelay(70000);
	//msWait(15);
	
	return;

}


void BspLcdPowerDownPanel(volatile S3C2450_IOPORT_REG *s2450IOP, UINT32 u32LCDType )
{
	//int i;
//	UINT8 g_u8Temp; //why??f.w.lin
	UINT32 g_u32Temp=0;


	if(BSP_LCD_YASSY_YF35F03CIB==u32LCDType) {

	}
	else if(BSP_LCD_BYD_4281L==u32LCDType){


	}
	else if(BSP_LCD_SHARP_LQ035Q1==u32LCDType){
		DPSTR("+LCD_Sharp_Enter2Sleep");
		LCD_Sharp_Enter2Sleep(s2450IOP);
		DPSTR("-LCD_Sharp_Enter2Sleep");	
	}
	 //[david.modify] 2008-11-11 15:08
	 // 解决屏关的时候，背影一闪问题
	 // 关掉4.3inch，多出的两路控制电压
	else if(BSP_LCD_BYD_43INCH_480X272==u32LCDType || BSP_LCD_INNOLUX_43==u32LCDType ){
#if 0		
		g_TmpstGPIOInfo2.u32PullUpdown = PULL_UPDOWN_DISABLE;
		g_TmpstGPIOInfo2.u32PinNo = LCD_ICPOWER_IO_GPC0;
		g_TmpstGPIOInfo2.u32AltFunc  = ALT_FUNC_OUT;
		//g_TmpstGPIOInfo2.u32Stat= 1;
		DPSTR("0-LCD_ICPOWER_IO_GPC0=0 1-LCD_ICPOWER_IO_GPC0=1");
//		g_u32Temp = ReadSerialByte();
		g_u32Temp='0';
		if('0'==g_u32Temp) {
			g_TmpstGPIOInfo2.u32Stat= 0;
		}else{
			g_TmpstGPIOInfo2.u32Stat= 1;
		}
		DPNOK(g_TmpstGPIOInfo2.u32Stat);
		SetGPIOInfo(&g_TmpstGPIOInfo2, s2450IOP);


		g_TmpstGPIOInfo2.u32PullUpdown = PULL_UPDOWN_DISABLE;
		g_TmpstGPIOInfo2.u32PinNo = LCD_BIAS_VOL_IO_GPE12;
		g_TmpstGPIOInfo2.u32AltFunc  = ALT_FUNC_OUT;
		//g_TmpstGPIOInfo2.u32Stat= 1;
		DPSTR("0-LCD_BIAS_VOL_IO_GPE12=0 1-LCD_BIAS_VOL_IO_GPE12=1");
//		g_u32Temp = ReadSerialByte();
		g_u32Temp='0';
		if('0'==g_u32Temp) {
			g_TmpstGPIOInfo2.u32Stat= 0;
		}else{
			g_TmpstGPIOInfo2.u32Stat= 1;
		}
		DPNOK(g_TmpstGPIOInfo2.u32Stat);			
		SetGPIOInfo(&g_TmpstGPIOInfo2, s2450IOP);	
#endif		
	}	

	g_TmpstGPIOInfo2.u32PullUpdown = PULL_UPDOWN_DISABLE;
	g_TmpstGPIOInfo2.u32PinNo = LCD_PWREN;
	g_TmpstGPIOInfo2.u32AltFunc  = ALT_FUNC_OUT;
	g_TmpstGPIOInfo2.u32Stat= 0;
	SetGPIOInfo(&g_TmpstGPIOInfo2, s2450IOP);		
	
}










//============================================

#endif
#endif


#ifndef XLLP_GPIO_H
#define XLLP_GPIO_H 
#ifdef __cplusplus
extern "C" {
#endif
/*
S3C2443X has 171 multi-functional input/output port pins and there are 12 ports as shown below:
�� Port A(GPA): 27-output port
�� Port B(GPB): 11-input/output port
�� Port C(GPC): 16-input/output port
�� Port D(GPD): 16-input/output port
�� Port E(GPE): 16-input/output port
�� Port F(GPF): 8-input/output port
�� Port G(GPG): 16-input/output port
�� Port H(GPH): 15-input/output port
�� Port J(GPJ): 16-input/output port
�� Port K(GPK): 16-input/output port
�� Port L(GPL): 15-input/output port
�� Port M(GPM): 2-input port

S3C2416X has 138 multi-functional input/output port pins and there are 11 ports as shown below:
�� Port A(GPA) : 25-output port
�� Port B(GPB) : 9-input/output port
�� Port C(GPC) : 16-input/output port
�� Port D(GPD) : 16-input/output port
�� Port E(GPE) : 16-input/output port
�� Port F(GPF) : 8-input/output port
�� Port G(GPG) : 8-input/output port
�� Port H(GPH) : 15-input/output port
�� Port K(GPK) : 16-input/output port
�� Port L(GPL) : 7-input/output port
�� Port M(GPM) : 2-input port
*/
#if (BSP_TYPE == BSP_SMDK2450)
#define RESERVED_GPIO 0xffff
// �ܼ�28, ����RESERVED: GPA11, 27; ����GPA10=RDATA_OEN
// 25-OUPUT
typedef enum {
GPA0=0,GPA1,GPA2,GPA3,GPA4, GPA5,GPA6,GPA7,GPA8,GPA9,
GPA10,GPA11,GPA12,GPA13,GPA14, GPA15,GPA16,GPA17,GPA18,GPA19,
GPA20,GPA21,GPA22,GPA23,GPA24, GPA25, GPA26, GPA27
}GPA_GPIO;
#define GPA_END GPA27

// �ܼ�11,����RESERVED:GPB7,8
typedef enum {
GPB0=GPA_END+1,GPB1,GPB2,GPB3,GPB4, GPB5,GPB6,GPB7,GPB8,GPB9,GPB10
}GPB_GPIO;
#define GPB_END GPB10

//�ܼ�16
typedef enum {
GPC0=GPB_END+1,GPC1,GPC2,GPC3,GPC4, GPC5,GPC6,GPC7,GPC8,GPC9,
GPC10,GPC11,GPC12,GPC13,GPC14, GPC15}GPC_GPIO;
#define GPC_END GPC15

//�ܼ�16
typedef enum {
GPD0=GPC_END+1,GPD1,GPD2,GPD3,GPD4, GPD5,GPD6,GPD7,GPD8,GPD9,
GPD10,GPD11,GPD12,GPD13,GPD14, GPD15
}GPD_GPIO;
#define GPD_END GPD15

//�ܼ�16
typedef enum {
GPE0=GPD_END+1,GPE1,GPE2,GPE3,GPE4, GPE5,GPE6,GPE7,GPE8,GPE9,
GPE10,GPE11,GPE12,GPE13,GPE14, GPE15
}GPE_GPIO;
#define GPE_END GPE15

//�ܼ�8
typedef enum {
GPF0=GPE_END+1,GPF1,GPF2,GPF3,GPF4,GPF5,GPF6, GPF7
}GPF_GPIO;
#define GPF_END GPF7

//�ܼ�16, reserved: GPG8,9,10,11,12,13,14,15
typedef enum {
GPG0=GPF_END+1,GPG1,GPG2,GPG3,GPG4, GPG5,GPG6,GPG7,GPG8,GPG9,GPG10,GPG11, GPG12,GPG13,GPG14,GPG15
}GPG_GPIO;
#define GPG_END GPG15

//�ܼ�15
typedef enum {
GPH0=GPG_END+1,GPH1,GPH2,GPH3,GPH4, GPH5,GPH6,GPH7,GPH8,GPH9,
GPH10,GPH11,GPH12,GPH13,GPH14
}GPH_GPIO;
#define GPH_END GPH14

// GPJû��
#define GPJ0 GPH_END
#define GPJ_END GPJ0

//�ܼ�16
typedef enum {
GPK0=GPJ_END+1,GPK1,GPK2,GPK3,GPK4, GPK5,GPK6,GPK7,GPK8,GPK9,
GPK10,GPK11,GPK12,GPK13,GPK14,GPK15
}GPK_GPIO;
#define GPK_END GPK15

//�ܼ�15, ����GPL4,5,6,7,10,11,12,14 reserved
typedef enum {
GPL0=GPK_END+1,GPL1,GPL2,GPL3,GPL4, GPL5,GPL6,GPL7,GPL8,GPL9,GPL10,GPL11,GPL12,GPL13,GPL14
}GPL_GPIO;
#define GPL_END GPL14

typedef enum {GPM0=GPL_END+1, GPM1}GPM_GPIO;
#define GPM_END GPM1

#define S3C24XX_GPIOS (GPM_END+1)
#endif




#if (BSP_TYPE == BSP_SMDK2443)
typedef enum {
GPA0=0,GPA1,GPA2,GPA3,GPA4, GPA5,GPA6,GPA7,GPA8,GPA9,
GPA10,GPA11,GPA12,GPA13,GPA14, GPA15,GPA16,GPA17,GPA18,GPA19,
GPA20,GPA21,GPA22,GPA23,GPA24, GPA25, GPA26
}GPA_GPIO;
#define GPA_END GPA26

typedef enum {
GPB0=GPA_END+1,GPB1,GPB2,GPB3,GPB4, GPB5,GPB6,GPB7,GPB8,GPB9, GPB10
}GPB_GPIO;
#define GPB_END GPB10

typedef enum {
GPC0=GPB_END+1,GPC1,GPC2,GPC3,GPC4, GPC5,GPC6,GPC7,GPC8,GPC9,
GPC10,GPC11,GPC12,GPC13,GPC14, GPC15}GPC_GPIO;
#define GPC_END GPC15

typedef enum {
GPD0=GPC_END+1,GPD1,GPD2,GPD3,GPD4, GPD5,GPD6,GPD7,GPD8,GPD9,
GPD10,GPD11,GPD12,GPD13,GPD14, GPD15
}GPD_GPIO;
#define GPD_END GPD15

typedef enum {
GPE0=GPD_END+1,GPE1,GPE2,GPE3,GPE4, GPE5,GPE6,GPE7,GPE8,GPE9,
GPE10,GPE11,GPE12,GPE13,GPE14, GPE15
}GPE_GPIO;
#define GPE_END GPE15

typedef enum {
GPF0=GPE_END+1,GPF1,GPF2,GPF3,GPF4,GPF5,GPF6, GPF7
}GPF_GPIO;
#define GPF_END GPF7

typedef enum {
GPG0=GPF_END+1,GPG1,GPG2,GPG3,GPG4, GPG5,GPG6,GPG7,GPG8,GPG9,
GPG10,GPG11,GPG12,GPG13,GPG14,GPG15
}GPG_GPIO;
#define GPG_END GPG15

typedef enum {
GPH0=GPG_END+1,GPH1,GPH2,GPH3,GPH4, GPH5,GPH6,GPH7,GPH8,GPH9,
GPH10,GPH11,GPH12,GPH13,GPH14
}GPH_GPIO;
#define GPH_END GPH14

typedef enum {
GPJ0=GPH_END+1,GPJ1,GPJ2,GPJ3,GPJ4, GPJ5,GPJ6,GPJ7,GPJ8,GPJ9,
GPJ10,GPJ11,GPJ12,GPJ13,GPJ14,GPJ15
}GPJ_GPIO;
#define GPJ_END GPJ15

typedef enum {
GPK0=GPJ_END+1,GPK1,GPK2,GPK3,GPK4, GPK5,GPK6,GPK7,GPK8,GPK9,
GPK10,GPK11,GPK12,GPK13,GPK14,GPK15
}GPK_GPIO;
#define GPK_END GPK15

typedef enum {
GPL0=GPK_END+1,GPL1,GPL2,GPL3,GPL4, GPL5,GPL6,GPL7,GPL8,GPL9,
GPL10,GPL11,GPL12,GPL13,GPL14
}GPL_GPIO;
#define GPL_END GPL14

typedef enum {GPM0=GPL_END+1, GPM1}GPM_GPIO;
#define GPM_END GPM1

#define S3C24XX_GPIOS (GPM_END+1)
#endif


typedef struct {
	UINT32 u32PinNo;
	UINT32 u32Stat;
//	UINT32 u32Dir;
	UINT32 u32AltFunc;
	UINT32 u32PullUpdown;	// 
}stGPIOInfo;

#define ALT_FUNC_00 0x0
#define ALT_FUNC_01 0x1
#define ALT_FUNC_02 0x2
#define ALT_FUNC_03 0x3

#define ALT_FUNC_OUT 0x1
#define ALT_FUNC_IN 0x0

#define GPA_ALT_FUNC_OUT 0x0



//#if (BSP_TYPE == BSP_SMDK2450)
#define PULL_UPDOWN_DISABLE 0x0
#define PULL_DOWN_ENABLE 0x1
#define PULL_UP_ENABLE 0x2
#define PULL_UPDOWN_NOTAVL 0x3	//NOT AVAILABLE
//#endif




// s805g GPIO USING  //[david.modify] 2008-05-08 17:49
//====================================

// POWER
//=============================
// VDD_CORE 1.2V -- GPB1 ����
// VDD_SMEM 1.8V
// VCC_3.3V 3.3V
// LCD_3.3V 3.3V
// VDD_USB33 3.3V
// VDD_ALILVE 1.2V
// VDD_RTC 3.3V
// TMC_VCC 3.3V
// VCC_GSP 3.85V
// VCC_1.5V 1.2V RTC BACKUP
// LED+, LED- 22.5V LCD ����

#define USB_DET GPF2		//USB��� in
#define CH_nFULL GPG7	//���ģʽ���� in
#define CH_CON GPG6		// ���ģʽ���� OUT, ˯��L
#define PWREN_USB GPB4	//USB��Դ����

//[david.modify] 2008-05-27 17:47
// 0-1.0V , 1-��ʾ1.3v (����Ӧ��VCORE=1.2V) 
#define CPUCORE_CON GPB1


// LCD MODULE
//====================
#define LCDRST GPB9 // ��λ
#define LCDCS GPB5 //SPIƬѡ	LCD͸��SPI�Ͳ�����ʼ��LCD
//#define LCDCLK GPB6 // SPI CLOCK

//[david.modify] 2008-05-12 17:59
#define LCDCLK GPE13	// ��ΪJTAG�õ�GPB6��
#define LCDSDA GPB10 //SPI DATA

//[david.modify] 2008-06-04 11:39
// ����IO��GPB3-->GPB2,��ΪTIMER3��TOUCHռ�� 
// 28khz pwm frequency
//#define BACKLIGHT_PWM GPB3

#define BACKLIGHT_PWM GPB2
#define BACKLIGHT_PWM_OLD GPB3
#define LCD_PWREN GPG4	//LCD����Դʹ��

//LCD DATA
#define LCD_B0 GPC11 
#define LCD_B1 GPC12 
#define LCD_B2 GPC13 
#define LCD_B3 GPC14 
#define LCD_B4 GPC15 
#define LCD_G0 GPD2 
#define LCD_G1 GPD3 
#define LCD_G2 GPD4 
#define LCD_G3 GPD5 
#define LCD_G4 GPD6 
#define LCD_G5 GPD7 
#define LCD_R0 GPD11 
#define LCD_R1 GPD12 
#define LCD_R2 GPD13 
#define LCD_R3 GPD14 
#define LCD_R4 GPD15
#define LCDHSYNC GPC4	//��ʱ��
#define LCDVSYNC GPC2 	//֡ʱ��
#define LCDPCLK GPC1 	// PIX ʱ��
#define LCDDE GPC3	// CHIP ENABLE

#if 0
// TOUCH�ӵ�CPU��ADC
AIN9	TSXP	������X+	I
AIN8	TSXM	������X-	I
AIN7	TSYP	������Y+	I
AIN6	TSYM	������Y-	I
#endif



// AUDIO
//=====================
#define I2SLRCK GPE0 	// ������Ƶͬ���ź�
#define I2SSCLK GPE1	//������Ƶ����CLK
#define I2SSDI GPE3	// ����ADC��������
#define I2SSDO GPE4	//����DAC�������
#define I2CSCL GPE14	// I2C CLK
#define I2CSDA GPE15	// I2C DATA
#if 0
FM_R // FM���룬��δ��
FM_L // FM���룬��δ��
AUDIO_SW // 0���������룬1�޶������룬����źŽӵ�ALC5621�ϡ���ͨ��I2C����ȡ
#endif


//KEYPAD
//==============
//������Ĭ��״̬Ϊ�ߣ���������ʱΪ�͡�
#define PWR_KEY GPF0	//��Դ��
#define KEY6 GPG2		//����+		//ent10
#define KEY3	GPG0		//����-		// ent8
#define KEY4 GPG1		//ȷ�ϼ�		//ent9

//GPS
//===============
#define GPS_BOOT GPA13
#define GPS_RESET GPA15
#define GPS_POWER_EN GPG5
#define GPS_RXD GPH5
#define GPS_TXD GPH4

// SD CARD
//=====================
#define SD0_DATA0 GPE7
#define SD0_DATA1 GPE8
#define SD0_DATA2 GPE9
#define SD0_DATA3 GPE10
#define SDCLK GPE5
#define SDCMD GPE6
#define SD0_NCD GPF1 	//SD����⣬0���п���1��û��
 //[david.modify] 2008-07-11 15:57
 //�ϰ���: GPB3--������� GPB2--SDд����
 // �°���: GPB3--SDд���� GPB2--�������
//#define SD0_WP GPB2		//SD��д������0��д�����أ�1��д������
#define SD0_WP GPB3		//SD��д������0��д�����أ�1��д������
#define SD_PWREN GPA14	//SD����Դʹ�ܣ�1������Դ��0���ص�Դ

// MISC
//================
#define TMC_ANT_DET GPG3
#define RTS0 GPH9
#define CTS0 GPH8
#define RXD0 GPH0
#define TXD0 GPH1
#define ULC_RST GPC0	//��λ����豸

#define nFCE2 GPA12		//???������˫DIE FLASH, Ƭѡ
#define TMC_ON GPH12	//����TMC��Դʹ�ܣ�1������Դ��0���ص�Դ����û��
//#define TMC_RXD RXD3	//����TMC��������
//#define TMC_TXD TXD3	//����TMC�������
#define GLED GPB0		//����ָʾ�ƣ�0��������1������
// HW_nRESET //ϵͳӲ����λ



 //[david.modify] 2008-09-09 14:27
 //========================================
 // 4.3INCH ��ʹ��GPF4��ΪMENU_KEY
 // 3.5INCH��Ŀ��û�ӳ���PIN
#if(LCD_MODULE_TYPE==BSP_LCD_BYD_43INCH_480X272 || LCD_MODULE_TYPE==BSP_LCD_INNOLUX_43)
#define MENU_KEY GPF4
#elif(LCD_MODULE_TYPE==BSP_LCD_YASSY_43INCH_480X272)
#define MENU_KEY GPF4
#endif

 //[david.modify] 2008-09-11 11:07
 //���������¼��ʱ��GPC0����������IO
#ifdef BSP_AS_BURNING
#define SPK_OUT GPC0
#endif
 //========================================

//added by f.w.lin for bt, fm1182
#define BT_PWM 	GPL13
#define BT_RST 	GPE11

#define AKM_RST	GPF7

//end added

 //[david.modify] 2008-05-30 16:39
 /*
 11:01 2008-7-18
�������	˯�߻��Ѻ����(ma)		˯��ʱ����(ma)		����SPEAKER�����
=============================================================================
0		63.2				3.6-3.7 
10		76.5				3.7
20		86.3				3.7
30		96.0				3.7	
40		105.4				3.7	
50		113.1				3.7
60		123.4				3.7
70		132.3				3.7
80		142.0				3.7
90		151.2				3.7
100		160.3				3.7		----------
110		169.0				3.7
120		176.9				3.7
130		183.7				3.7
140		189.1				3.7
145		191.3				3.7
*/
 // �������ȼ�
 #define MIN_PWM_VALUE 15
#define MAX_PWM_VALUE 145
#define BOOTLOADER_PWM_VALUE1  (60)
#define OS_PWM_VALUE1  (100)

// ȱʡ�������
//	"Lightness"=dword:50
#define DEF_PWM_VALUE 80




//[david.modify] 2008-07-21 18:34
// ��ص�ѹ�趨ֵ
//=========================================
 /*
* dwBattPercent= 
*(BYTE)(((float)dwVoltage  - BATTERY_MIN_VOLTAGE)* 100 /(BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)+1);
*
*/
#define BATTERY_MAX_VOLTAGE     4200	//mv 
#define BATTERY_MIN_VOLTAGE     3500	//mv 
#define BATTERY_AUTOSLEEP_VOLTAGE 3550 	//mv Ҫ�����BATTERY_MIN_VOLTAGE

#define BATTERY_REF_VOLTAGE 3330	// 1024 BIT ADC, �ο���ѹ3.33v
#define VBAT_EBOOT_AUTOSLEEP_VOLTAGE 3500	//
#define VBAT_OS_AUTOSLEEP_VOLTAGE 3550	//

//#define BATTERY_AUTOSLEEP_VOLTAGE_PERCENT ( (BATTERY_AUTOSLEEP_VOLTAGE - BATTERY_MIN_VOLTAGE)* 100/(BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)+1) +1)
//#define PERCENT_CRITICAL2LOW (BATTERY_AUTOSLEEP_VOLTAGE_PERCENT+4)
//=========================================


int GetGPIOInfo(stGPIOInfo *pstGPIOInfo, void* pGPIOVirtBaseAddr);
int SetGPIOInfo(stGPIOInfo *pstGPIOInfo, void* pGPIOVirtBaseAddr);
 int SetMultiGPIO(UINT32 u32StartIO, UINT32 u32EndIO, stGPIOInfo *pstGPIOInfo, void* pGPIOVirtBaseAddr);

#ifdef __cplusplus
}
#endif

#endif

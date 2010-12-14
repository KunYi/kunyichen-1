#ifndef BSPBATTPADD_H
#define  BSPBATTPADD_H

BOOL bInPowerHandler; 	//Sharing variable with i2c.c in drvlib




#define BATTERY_MAX_VOLTAGE_AC     4200	//mv 	// ����ѹ
#define BATTERY_MIN_VOLTAGE_AC     3400	//mv 



 //[david.modify] 2008-06-16 15:51
 //===================================
 #define BATT_DRIVER_REGISTRY TEXT("\\Drivers\\BuiltIn\\Battery")
#define DEF_u32LowPowerSleep 1 //0 Awisc.Chen modify 2008-07-14 18:02
#define DEF_u32FullPercentVbat 4100 // 4.1v��100%

#define CHARGING_BIT 0x1 // �������λ����UI������ʾ���״̬
#define SHOWVBAT_BIT  0x2
#define DEF_u32Debug (0)


//[david.modify] 2007-04-18 14:06
// ȡ���ʱ,�ĳ�������
#define SAMPLE_VBAT_TIMES 1	
#define DEF_u32SampleVbatTimes SAMPLE_VBAT_TIMES

//�����ٴ�ȡƽ��ֵ
#define ADC_COUNT   6

// ʵ�ʵ�ѹ = 10bit adc/1024 *2 *3.3v
// ÿ��һ��ADC���ʱ��(ms)
#define VBAT_SAMPLE_TIME1   1	
#define DEF_u32SampleVbatDelay VBAT_SAMPLE_TIME1

#define DEF_u32AutoSleepVbat BATTERY_AUTOSLEEP_VOLTAGE

 typedef struct
{
	unsigned int u32Debug;	// ����ʹ��
	unsigned int u32LowPowerSleep;	//�Ƿ�͵�˯��
	unsigned int u32FullPercentVbat;	 //����4200����������
	unsigned int u32SampleVbatTimes;	//ȡ��ѹʱ,SAMPLE���ٴ�adc
	unsigned int u32SampleVbatDelay;	//ȡ��ѹʱ,SAMPLE1 ����ʱ����
	unsigned int u32AutoSleepVbat;	//�Զ�˯��ʱ��ѹ
	
	
	
}stBattParam;
 //===================================

typedef enum {
    Noevent,
    PowerButton,
    ACOut,
    ACIn,
    BatteryLow,
    GPSStatic,
    SectJobDone
} APM_MONITOR_EVENT;

enum PDDChargerStatus {PDD_NO_CHARGE,
					   PDD_CONSTANT_CURRENT_CHARGE,
					   PDD_CVM_CHARGE,
					   PDD_END_OF_CHARGE,
					   PDD_FUEL_GA,
					   PDD_FUEL_GAUGE_CAL_MODE};

enum PDDBatteryFlag {PDD_NO_BATTERY,
					PDD_BATTERY_CHARGING,
					PDD_BATTERY_ON,
					PDD_BATTERY_EOC};

BOOL BspBattAllocResource(void);
UCHAR  BspFuelGaugeGetBatStatusInPercent(void);
USHORT BspFuelGaugeResetBatStatusInPercent(void);
VOID BspNotifyLed(BOOL bLedOn);
BOOL BspAdapterDetect (void);
BOOL BspUSBChargerDetect (void);
BOOL BspGetACStatus(void);
UCHAR BspGetBatteryFlag(void);
VOID BspBattDeallocResource(void);
VOID BspBattCriticalLow(void);
VOID BspPowerHandleOn(void);
VOID BspPowerHandleOff(void);
BOOL BspBatteryDetect(void);
BOOL BspPowerStatesManagerInit(void);
VOID BspPowerStatesManagerDeInit(void);

#endif

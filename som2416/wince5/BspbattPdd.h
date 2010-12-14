#ifndef BSPBATTPADD_H
#define  BSPBATTPADD_H

BOOL bInPowerHandler; 	//Sharing variable with i2c.c in drvlib




#define BATTERY_MAX_VOLTAGE_AC     4200	//mv 	// 最大电压
#define BATTERY_MIN_VOLTAGE_AC     3400	//mv 



 //[david.modify] 2008-06-16 15:51
 //===================================
 #define BATT_DRIVER_REGISTRY TEXT("\\Drivers\\BuiltIn\\Battery")
#define DEF_u32LowPowerSleep 1 //0 Awisc.Chen modify 2008-07-14 18:02
#define DEF_u32FullPercentVbat 4100 // 4.1v算100%

#define CHARGING_BIT 0x1 // 如果开此位，则UI不能显示充电状态
#define SHOWVBAT_BIT  0x2
#define DEF_u32Debug (0)


//[david.modify] 2007-04-18 14:06
// 取电池时,的抽样次数
#define SAMPLE_VBAT_TIMES 1	
#define DEF_u32SampleVbatTimes SAMPLE_VBAT_TIMES

//调多少次取平均值
#define ADC_COUNT   6

// 实际电压 = 10bit adc/1024 *2 *3.3v
// 每读一次ADC间隔时间(ms)
#define VBAT_SAMPLE_TIME1   1	
#define DEF_u32SampleVbatDelay VBAT_SAMPLE_TIME1

#define DEF_u32AutoSleepVbat BATTERY_AUTOSLEEP_VOLTAGE

 typedef struct
{
	unsigned int u32Debug;	// 调试使用
	unsigned int u32LowPowerSleep;	//是否低电睡眠
	unsigned int u32FullPercentVbat;	 //低于4200多少算满格
	unsigned int u32SampleVbatTimes;	//取电压时,SAMPLE多少次adc
	unsigned int u32SampleVbatDelay;	//取电压时,SAMPLE1 次延时多少
	unsigned int u32AutoSleepVbat;	//自动睡眠时电压
	
	
	
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

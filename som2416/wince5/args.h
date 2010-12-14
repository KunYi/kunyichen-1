#ifndef __ARGS_H
#define __ARGS_H

//------------------------------------------------------------------------------
//
// File:        args.h
//
// Description: This header file defines device structures and constant related
//              to boot configuration. BOOT_CFG structure defines layout of
//              persistent device information. It is used to control boot
//              process. BSP_ARGS structure defines information passed from
//              boot loader to kernel HAL/OAL. Each structure has version
//              field which should be updated each time when structure layout
//              change.
//
//------------------------------------------------------------------------------

#include "oal_args.h"
#include "oal_kitl.h"

#define BSP_ARGS_VERSION    1
#define PAD(label,amt)  UCHAR Pad##label[amt]



//chul_idle_0525_s
#define	BUSY_BIT_TOUCH	(1<<0)
#define	BUSY_BIT_USB	(1<<1)
#define	BUSY_BIT_BLUETOOTH	(1<<2)
#define	BUSY_BIT_CRITICAL_INTERRUPT	(1<<30)
#define	BUSY_BIT_INTERRUPT	(1<<31)
//chul_idle_0525_e

// POWER Global
typedef struct _POWER_GLOBALS 
{
	DWORD BatteryCoverOpen;
	DWORD BatteryPIOWakeup;
    DWORD BatteryPOKWakeup;
    DWORD NoBattery;
    DWORD ACChargeRate;         // 5
    DWORD BatteryWarningCycle;
    DWORD BatteryUserMessage;
    DWORD SystemReady;
    DWORD ACUnplugMessage;
    DWORD BatterySK;            // 10
    DWORD BatteryMiser;
    DWORD CalibrationCompleted;
    DWORD ACEventReady;
    DWORD CurrentUSBChargingStatus;
    DWORD IsCharging;           // 15
    DWORD BattSwapFlag;
    DWORD BatterySwapHappended;
	DWORD dwEINTPEND;		// shadow of External Interrupt Pending Register while resuming
	DWORD dwSRCPND;		// shadow of Source Pending Register while resuming
	DWORD BatteryStandbyState;  // 20
	DWORD TemperatureException;
    DWORD BatteryVeryLow;           // Set 1 when battery is at VeryLow state(10%)
    DWORD BatteryLearnCycleInProgress;
    DWORD BatteryCriticalLow;
    DWORD FullyWakeUpEvent; //25 // 0 is no action, 1 is fully wake up, 2 is sleep by cover switch, 3 is checking real RTC.
    DWORD TrickleCharging;
    DWORD DisableWLAN;
    DWORD DisableBT;
    DWORD DisableGPS;          
    DWORD dwBusyDeviceFlag;     // 30		//chul_idle_0525
	DWORD dwI2CBusy;		//chul_idle_0525
    DWORD dwSleepNResumeTest;	
	DWORD BatteryPIOWakeup2;
	DWORD dwWakeupSource;		// 34
    DWORD UnderZeroPercentage;      // Battery is under 0%. Need to check voltage regularly during sleep mode.
    DWORD UserRTCWakeup;     //36   // Wake up by User RTC.
    DWORD EnableGPS; //  jbshim 2006.07.13
	DWORD BattOverHeated; // 38
    DWORD dwBeforeSuspendWLANStatus;
	DWORD WLANOverheated;
	PAD(0, 256-(4*40));
} POWER_GLOBALS, *PPOWER_GLOBALS;

 //[david.modify] 2008-05-17 11:30
// Reset Cause flag define
#define HARD_RESET_FLAG					0x00000001
#define SLEEP_RESET_FLAG				0x00000002
#define WDOG_RESET_FLAG					0x00000004
#define PARTIAL_FACTORY_RESET_FLAG		0x00000008
#define FACTORY_RESET_FLAG				0x00000010

//Eboot launch mode define
#define OS_LAUNCH				0x00010000
//#define PREEBOOT_LAUNCH		0x00020000
#define EBOOT_LAUNCH			0x00040000
#define PARTIAL_FACTORY_LAUNCH	0x00080000
#define FACTORY_LAUNCH			0x00100000

// Un-initialized Miscellaneous
typedef struct _UNINIT_MISK_GLOBALS {  // size => 0x100
	DWORD dwResetCause;			// SPEC. Reset Type save
	DWORD dwEootFlag;			// SPEC. Booting Flag
	DWORD dwHWSKU;				// SPEC. reserved...
	DWORD szPreBootVersion;
	DWORD gProgressNcomplete;
	DWORD Eardet;   			// jbshim
	DWORD m_dwEardet;
	DWORD m_dwMicdet;
	DWORD dwButtonType;
	DWORD dwWheelType;	
	DWORD DisplayResume;/*Display*/
	DWORD BacklightBrightnessPower;/*backlight*/
	DWORD BacklightBrightnessBattery;/*backlight*/
	DWORD DisplayPowerOnOff;/*Display*/
	DWORD USBAttach;
	DWORD dwWakeUpLockBtn; 		//Seonah. Lock Appbutton in sleep mode
	DWORD dwWakeUpLockRecBtn; 		//Seonah. Lock Record button in sleep mode
	DWORD bCleanSystemHive;	//bgkim, Clean System Hive flag for Logo Test #5041
	DWORD bCleanUserHive;		//bgkim, Clean User Hive flag for Logo Test #5041
	DWORD BeingSDForUSB;// TERRY for Event in Sd card
	DWORD dwUSBConfigurationDone;  // TERRY for USB
	DWORD NandFlashSize;		//bgkim, for getting mdoc size
	DWORD dwWLANIntStatus;		//DY Moh
	PAD(0,(256-92));
} UNINIT_MISK_GLOBALS, *PUNINIT_MISK_GLOBALS;
// Misc Global
typedef struct _MISC_GLOBALS 
{
    DWORD ACLineStatus;     
    DWORD ExtPowerStatus;			// AC Line or USB Power
	DWORD FullChargedBattery;
	DWORD light_status;/*backlight*/
		DWORD UsbConnection;/*USBFN*/
    DWORD BattryCaliAbort;
    DWORD gSDDeviceType; //ESLEE
    DWORD g_bSDWakingUp; //ESLEE
    DWORD UsbConnectionB; //USB charging is 500mA when reset.
    DWORD UsbSuspend; //When Host is suspended.
    DWORD BackLight;/*juninjx*/
    DWORD TouchSet;/*juninjx*/
    DWORD Hostsleepresumeroutine;
    DWORD informtoclass;
    PAD(0,(256-(4*14)));
} MISC_GLOBALS, *PMISC_GLOBALS;

#define WAKEUP_MODE1_SHORTPRESS 0
#define WAKEUP_MODE2_LONGPRESS 1

#define u32DebugDavid_BIT0_DISPLAY 1
#define u32DebugDavid_BIT1
#define DEF_u32DebugDavid 0

typedef struct {
    OAL_ARGS_HEADER header;
    UINT8 deviceId[16];                 // Device identification
    OAL_KITL_ARGS kitl;

#if MAGNETO		
	DWORD	nfsblk;
#endif

 //[david.modify] 2008-06-02 10:30
    UINT32 dbgSerPhysAddr;              // Debug serial physical address
	HANDLE g_SDCardDetectEvent; //kim
	DWORD g_SDCardState ;

	 //[david.modify] 2008-05-16 17:33

// 增加一项用于在BOOTLOADER中控制OALLogSetZones
   UINT32 u32OALLogSetZones;	 
   UINT32 u32WakeUpMode;		//0-短按 1-长按
   UINT32 u32WakeupHoldTime;		// 保挂时间
   UINT32 u32DebugDavid;		// 用于控制从EBOOT传递参数到OS
   UINT32 u32LCDType;
   UINT32 u32BootLogoAddr;   	// logo缓冲地址

//[david.modify] 2008-05-16 11:47
//================================
#if MAGNETO		
	PAD(0, (0x100 - 0x5a-4));	
#else
	PAD(0, (0x100 - 0x5a));	
#endif
//================================	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	UNINIT_MISK_GLOBALS uninit_misc;	// Offset 0x100	
    POWER_GLOBALS	pwr;				// Offset 0x4400
    MISC_GLOBALS    misc;				// Offset 0x4300	
	
	
} BSP_ARGS;

//------------------------------------------------------------------------------

#endif

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

//
// This module contains a stub implementation of the battery PDD.  OEMs
// that want to support the battery APIs on their platform can copy this
// file to their platform and link it into their own version of the power
// manager DLL.
//
// If the platform-specific power manager provides its own version of these
// entry points, this module will not be pulled into the link map from
// the pm_battapi library.
//

//--------------------------------------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------------------------------------
#include <battimpl.h>
#include <ceddk.h>
#include <bsp.h>
#include "bspbattpdd.h"
#include <xllp_gpio_david.h>
#include <dbgmsg_david.h>
//#include <s3c2448a.h>


 //[david.modify] 2008-05-31 14:31
 //============================

stBattParam g_stBattParam;


int RegInfo_Init()
{
    g_stBattParam.u32Debug=DEF_u32Debug;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, BATT_DRIVER_REGISTRY, TEXT("u32Debug"), &g_stBattParam.u32Debug))
    {    	
    }	
    g_stBattParam.u32LowPowerSleep=DEF_u32LowPowerSleep;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, BATT_DRIVER_REGISTRY, TEXT("u32LowPowerSleep"), &g_stBattParam.u32LowPowerSleep))
    {    	
    }		

    g_stBattParam.u32FullPercentVbat=DEF_u32FullPercentVbat;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, BATT_DRIVER_REGISTRY, TEXT("u32FullPercentVbat"), &g_stBattParam.u32FullPercentVbat))
    {    	
    }		
	
    g_stBattParam.u32SampleVbatTimes=DEF_u32SampleVbatTimes;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, BATT_DRIVER_REGISTRY, TEXT("u32SampleVbatTimes"), &g_stBattParam.u32SampleVbatTimes))
    {    	
    }		

    g_stBattParam.u32SampleVbatDelay=DEF_u32SampleVbatDelay;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, BATT_DRIVER_REGISTRY, TEXT("u32SampleVbatDelay"), &g_stBattParam.u32SampleVbatDelay))
    {    	
    }		

    g_stBattParam.u32AutoSleepVbat=DEF_u32AutoSleepVbat;
    if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, BATT_DRIVER_REGISTRY, TEXT("u32AutoSleepVbat"), &g_stBattParam.u32AutoSleepVbat))
    {    	
    }			
		
	
    DPNOK(g_stBattParam.u32Debug);	
    DPNOK(g_stBattParam.u32LowPowerSleep);	
    DPNOK( g_stBattParam.u32FullPercentVbat);
    DPNOK(g_stBattParam.u32SampleVbatTimes);	
    DPNOK(g_stBattParam.u32SampleVbatDelay);
    DPNOK(g_stBattParam.u32AutoSleepVbat);	
    return 1;
}

 //============================

//Debug 
#define ZONE_BATT	0
#define ZONE_BATT_ERROR	1
#define ZONE_REG_PRINT 0

// typedefs for APIs that require the "nkmapfile" component
typedef WINBASEAPI HANDLE (WINAPI *PFN_CreateFileMappingW) (
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName
    );
typedef LPVOID (WINAPI *PFN_MapViewOfFile) (
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    );
typedef BOOL (WINAPI *PFN_UnmapViewOfFile) (
    LPCVOID lpBaseAddress
    );


typedef BOOL (WINAPI * PFN_GwesPowerDown)(void);

#pragma pack(4)
typedef struct _BatteryStatus_tag {
    SYSTEM_POWER_STATUS_EX2 sps;
    WORD    wMainLevels;
    WORD    wBackupLevels;
    BOOL    fSupportsChange;
    BOOL    fChanged;
} BATTERY_STATUS, *PBATTERY_STATUS;
#pragma pack()

#define BATTERY_STATUS_FILE TEXT("Battery File")
#define BATTERY_FILE_MUTEX  TEXT("Battery File Mutex")

#define MUTEX_TIMEOUT 5000

static PBATTERY_STATUS gpStatus;
static HANDLE ghMutex;
static HANDLE ghFileMap;
static PFN_CreateFileMappingW gpfnCreateFileMappingW;
static PFN_MapViewOfFile gpfnMapViewOfFile;
static PFN_UnmapViewOfFile gpfnUnmapViewOfFile;

//Global Variables
enum PDDBatteryFlag battery_status = PDD_NO_BATTERY;

// this routine takes the battery mutex
DWORD
LockBattery(void)
{
    DWORD dwStatus;
    SETFNAME(_T("LockBattery"));
	DPSTR("+LockBattery");

    DEBUGCHK(ghMutex != NULL);

    dwStatus = WaitForSingleObject(ghMutex, MUTEX_TIMEOUT);
    if(dwStatus == WAIT_OBJECT_0) {
        dwStatus = ERROR_SUCCESS;
    } else {
        dwStatus = GetLastError();
        DEBUGCHK(dwStatus != ERROR_SUCCESS);
    }
    
    DEBUGMSG(dwStatus != ERROR_SUCCESS && ZONE_WARN,
        (_T("%s: WaitForSingleObject() failed %d\r\n"), TEXT(__FUNCTION__), 
        GetLastError()));
    DEBUGCHK(dwStatus == ERROR_SUCCESS);

	DPSTR("-LockBattery");
    return dwStatus;
}

// this routine releases the battery mutex
DWORD
UnlockBattery(void)
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL fOk;
    SETFNAME(_T("UnlockBattery"));
	DPSTR("+UnlockBattery");	

    DEBUGCHK(ghMutex != NULL);

    fOk = ReleaseMutex(ghMutex);
    if(!fOk) {
        dwStatus = GetLastError();
        DEBUGCHK(dwStatus != ERROR_SUCCESS);
    }
    
    DEBUGMSG(dwStatus != ERROR_SUCCESS && ZONE_WARN,
        (_T("%s: ReleaseMutex() failed %d\r\n"), TEXT(__FUNCTION__), GetLastError()));
    DEBUGCHK(dwStatus == ERROR_SUCCESS);
	DPSTR("-UnlockBattery");
    return dwStatus;
}

BOOL WINAPI 
BatteryPDDInitialize(LPCTSTR pszRegistryContext)
{
    BOOL fOk = TRUE;
    SYSTEM_POWER_STATUS_EX2 sps;
    WORD wMainLevels = 3, wBackupLevels = 0;
    BOOL fSupportsChange = FALSE;
    SETFNAME(_T("BatteryPDDInitialize"));

    DEBUGCHK(ghMutex == NULL);
    DEBUGCHK(ghFileMap == NULL);
    DEBUGCHK(gpStatus == NULL);
    DEBUGCHK(pszRegistryContext != NULL);
    
    RETAILMSG(ZONE_REG_PRINT, (TEXT("+++BatteryPDDInitialize... +++ \r\n"))); 

 //[david.modify] 2008-05-31 14:38
    RegInfo_Init();


    //Check Hardware status and system interface first
    if(!BspBattAllocResource()) goto _ERR_BATT;



    // intialize the battery status structure -- assume AC power, no battery info

    sps.ACLineStatus               = AC_LINE_ONLINE;
    sps.BatteryFlag                = BATTERY_FLAG_HIGH;
    sps.BatteryLifePercent         = BATTERY_PERCENTAGE_UNKNOWN;
    sps.Reserved1                  = 0;
    sps.BatteryLifeTime            = BATTERY_LIFE_UNKNOWN;
    sps.BatteryFullLifeTime        = BATTERY_LIFE_UNKNOWN;
    sps.Reserved2                  = 0;
    sps.BackupBatteryFlag          = BATTERY_FLAG_UNKNOWN;
    sps.BackupBatteryLifePercent   = BATTERY_PERCENTAGE_UNKNOWN;
    sps.Reserved3                  = 0;
    sps.BackupBatteryLifeTime      = BATTERY_LIFE_UNKNOWN;
    sps.BackupBatteryFullLifeTime  = BATTERY_LIFE_UNKNOWN;
    sps.BatteryChemistry           = BATTERY_CHEMISTRY_UNKNOWN;
    sps.BatteryVoltage             = 0;
    sps.BatteryCurrent             = 0;
    sps.BatteryAverageCurrent      = 0;
    sps.BatteryAverageInterval     = 0;
    sps.BatterymAHourConsumed      = 0;
    sps.BatteryTemperature         = 0;
    sps.BackupBatteryVoltage       = 0;

    // allocate resources
    if((ghMutex = CreateMutex(NULL, FALSE, BATTERY_FILE_MUTEX)) == NULL) {
        DEBUGMSG(ZONE_ERROR || ZONE_PDD || ZONE_INIT,
            (_T("%s: Could not aquire battery info file mutex handle\n"), TEXT(__FUNCTION__)));
        fOk = FALSE;
    } else {
        HINSTANCE hiCoreDll = NULL; 
        BOOL fNewMapping = TRUE;
        
        // get pointers to file-mapping functions
        hiCoreDll = LoadLibrary(_T("coredll.dll"));
        if(hiCoreDll != NULL) {
            gpfnCreateFileMappingW = (PFN_CreateFileMappingW) GetProcAddress((HMODULE) hiCoreDll, _T("CreateFileMappingW"));
            gpfnMapViewOfFile = (PFN_MapViewOfFile) GetProcAddress((HMODULE) hiCoreDll, _T("MapViewOfFile"));
            gpfnUnmapViewOfFile = (PFN_UnmapViewOfFile) GetProcAddress((HMODULE) hiCoreDll, _T("UnmapViewOfFile"));
        }
        FreeLibrary(hiCoreDll);		// we're already linked to coredll
        
        // serialize access to the mapping file
        LockBattery();
        
        // create the mapping
        if(gpfnCreateFileMappingW == NULL ) {
            // no file mapping, use a global variable
            static BATTERY_STATUS sBatteryStatus;
            gpStatus = &sBatteryStatus;
        } else if((ghFileMap = gpfnCreateFileMappingW((HANDLE)INVALID_HANDLE_VALUE, NULL, 
            PAGE_READWRITE, 0, sizeof(BATTERY_STATUS), BATTERY_STATUS_FILE)) == NULL) {
            DEBUGMSG(ZONE_ERROR || ZONE_PDD || ZONE_INIT,
                (_T("%s: Could not create file mapping for battery info file\n"), TEXT(__FUNCTION__)));
            fOk = FALSE;
        } else {
            // is this a new mapping?
            if(GetLastError() == ERROR_ALREADY_EXISTS) {
                fNewMapping = FALSE;
            }
            
            // map the object into our address space
            if(gpfnMapViewOfFile == NULL 
            || (gpStatus = (PBATTERY_STATUS) gpfnMapViewOfFile(ghFileMap, FILE_MAP_ALL_ACCESS, 
                0, 0, sizeof(BATTERY_STATUS))) == NULL) {
                DEBUGMSG(ZONE_ERROR || ZONE_PDD || ZONE_INIT,
                    (_T("Could not map view of battery info file into process address space\n"), TEXT(__FUNCTION__)));
                fOk = FALSE;
            } 
        }
        
        // should we initialize our structure?
        if(fOk && fNewMapping) {
            // initialize the memory mapped object
            memcpy(&gpStatus->sps, &sps, sizeof(gpStatus->sps));
            gpStatus->fSupportsChange = fSupportsChange;
            gpStatus->fChanged = FALSE;
            gpStatus->wMainLevels = wMainLevels;
            gpStatus->wBackupLevels = wBackupLevels;
        }
        
        // allow access to the battery buffer
        UnlockBattery();
    }
    
    // clean up if necessary
    if(!fOk) {
        if(gpStatus != NULL && gpfnUnmapViewOfFile != NULL) gpfnUnmapViewOfFile(gpStatus);
        if(ghFileMap != NULL) CloseHandle(ghFileMap);
        if(ghMutex != NULL) CloseHandle(ghMutex);
        gpStatus = NULL;
        ghFileMap = NULL;
        ghMutex = NULL;
    }
    
    return fOk;

_ERR_BATT:
	
    RETAILMSG(ZONE_BATT_ERROR, (TEXT("+Battery ERROR OR UNKNOWN STATUS...  \r\n"))); 
    BspBattDeallocResource();	
    SetLastError(ERROR_NOT_SUPPORTED);	
    return  FALSE;		

}

//had not call-in
void WINAPI 
BatteryPDDDeinitialize(void)
{
    RETAILMSG(ZONE_REG_PRINT, (TEXT("+BatteryPDDDeInitialize...  \r\n"))); 
    BspBattDeallocResource();	

    if(gpStatus != NULL && gpfnUnmapViewOfFile != NULL) gpfnUnmapViewOfFile(gpStatus);
    if(ghFileMap != NULL) CloseHandle(ghFileMap);
    if(ghMutex != NULL) CloseHandle(ghMutex);
    gpStatus = NULL;
    ghFileMap = NULL;
    ghMutex = NULL;
}

//had not call-in
void WINAPI 
BatteryPDDResume(void)
{
    RETAILMSG(ZONE_REG_PRINT, (TEXT("+BatteryPDDResume...  \r\n"))); 
}

void WINAPI 
BatteryPDDPowerHandler(BOOL bOff)
{
    RETAILMSG(ZONE_REG_PRINT, (TEXT("+BatteryPDDPowerHandler 0x%x...  \r\n"),bOff));
/*
    if(PDD_NO_BATTERY == battery_status)
    {
        RETAILMSG(1, (TEXT("+BatteryPDDPowerHandler QUIT 0x%x...  \r\n"),bOff));
        return;
    }
*/    
    bInPowerHandler = TRUE;	
    if(bOff) 
    {
        BspPowerHandleOff();
        RETAILMSG(ZONE_REG_PRINT, (TEXT("+BatteryPDDPowerHandler: Disable PDD interrupt..  \r\n")));
    }
    else
    {
        BspPowerHandleOn();
        RETAILMSG(ZONE_REG_PRINT, (TEXT("+BatteryPDDPowerHandler: Enable PDD interrupt..  \r\n")));
    }
    bInPowerHandler = FALSE;
}

// This routine indicates how many battery levels will be reported
// in the BatteryFlag and BackupBatteryFlag fields of the PSYSTEM_POWER_STATUS_EX2
// filed in by BatteryPDDGetStatus().  This number ranges from 0 through 3 --
// see the Platform Builder documentation for details.  The main battery
// level count is reported in the low word of the return value; the count 
// for the backup battery is in the high word.
//have not used currently
LONG
BatteryPDDGetLevels(void)
{
    LONG lLevels;
    SETFNAME(_T("BatteryPDDPowerHandler"));

    RETAILMSG(ZONE_REG_PRINT, (TEXT("+BatteryPDDGetLevels...  \r\n"))); 

    PREFAST_DEBUGCHK(gpStatus != NULL);

    LockBattery();
    lLevels = MAKELONG (gpStatus->wMainLevels, gpStatus->wBackupLevels);
	DPNOK(lLevels);
    UnlockBattery();

    DEBUGMSG(ZONE_PDD, (_T("%s: returning %u (%d main levels, %d backup levels)\r\n"),
        TEXT(__FUNCTION__), lLevels, LOWORD(lLevels), HIWORD(lLevels)));

    return lLevels;
}

// This routine returns TRUE to indicate that the pfBatteriesChangedSinceLastCall
// value filled in by BatteryPDDGetStatus() is valid.  If there is no way to
// tell that the platform's batteries have been changed this routine should
// return FALSE.
//Not support
BOOL
BatteryPDDSupportsChangeNotification(void)
{
    BOOL fSupportsChange;
    SETFNAME(_T("BatteryPDDPowerHandler"));
    RETAILMSG(ZONE_REG_PRINT, (TEXT("+BatteryPDDSupportsChangeNotification...  \r\n"))); 

    PREFAST_DEBUGCHK(gpStatus != NULL);

    LockBattery();
    fSupportsChange = gpStatus->fSupportsChange;
		DPNOK(fSupportsChange);
    UnlockBattery();

    return fSupportsChange;

}

// This routine obtains the most current battery/power status available
// on the platform.  It fills in the structures pointed to by its parameters
// and returns TRUE if successful.  If there's an error, it returns FALSE.
/********************************************************************************
//ACLineStatus -- 0 Offline  1 Online  255 Unknown status 
//
//
//*******************************************************************************/
BOOL WINAPI
BatteryPDDGetStatus(
    PSYSTEM_POWER_STATUS_EX2 pstatus,  PBOOL pfBatteriesChangedSinceLastCall)
{
    BOOL fOk = TRUE;
    SYSTEM_POWER_STATUS_EX2 sps;
    BOOL ACStatus=FALSE;
    UCHAR ucBatteryPer = 0;
    UINT32 BATTERY_AUTOSLEEP_VOLTAGE_PERCENT ;
    UINT32 PERCENT_CRITICAL2LOW;

    BATTERY_AUTOSLEEP_VOLTAGE_PERCENT= 	( (g_stBattParam.u32AutoSleepVbat - BATTERY_MIN_VOLTAGE)* 100/(BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)+1) ;
    PERCENT_CRITICAL2LOW	= BATTERY_AUTOSLEEP_VOLTAGE_PERCENT + 4;

    DPNOK(BATTERY_AUTOSLEEP_VOLTAGE_PERCENT);	
    DPNOK(PERCENT_CRITICAL2LOW);	
   
    SETFNAME(_T("BatteryPDDGetStatus"));

    RETAILMSG(ZONE_REG_PRINT, (TEXT("+BatteryPDDGetStatus...  \r\n")));
  
    DEBUGCHK(pstatus != NULL);
    DEBUGCHK(pfBatteriesChangedSinceLastCall != NULL);

    /* Fill 	ACLineStatus part */
    sps.ACLineStatus          = AC_LINE_UNKNOWN;	
    if(BspGetACStatus())
    {
        sps.ACLineStatus          = AC_LINE_ONLINE;
    }
    else
    {
        sps.ACLineStatus          = AC_LINE_OFFLINE;
    }

    /* Fill Battery status part */
    battery_status = BspGetBatteryFlag();

    if (PDD_NO_BATTERY == battery_status )
    {
        BspNotifyLed(FALSE);
        sps.BatteryFlag  = BATTERY_FLAG_NO_BATTERY;
        sps.BatteryLifePercent = BATTERY_PERCENTAGE_UNKNOWN;
    }
    else if (PDD_BATTERY_CHARGING == battery_status )
    {
        RETAILMSG(ZONE_REG_PRINT, (TEXT("+Battery status PDD_BATTERY_CHARGING...  \r\n"))); 
        BspFuelGaugeResetBatStatusInPercent();	   
        BspNotifyLed(FALSE);
        sps.BatteryFlag  = BATTERY_FLAG_CHARGING;
        sps.BatteryLifePercent = BATTERY_PERCENTAGE_UNKNOWN;
//  //[david.modify] 2008-05-31 14:49
// ڴ˴óʱҲʾٷֱ
//=======================================================
	   if(CHARGING_BIT&g_stBattParam.u32Debug) {
          ucBatteryPer = BspFuelGaugeGetBatStatusInPercent();

            if(ucBatteryPer >= PERCENT_CRITICAL2LOW)
            {
                sps.BatteryFlag  = BATTERY_FLAG_HIGH;
  	         BspNotifyLed(FALSE);
            }		 
            else if (( ucBatteryPer <PERCENT_CRITICAL2LOW) && (ucBatteryPer >=BATTERY_AUTOSLEEP_VOLTAGE_PERCENT))
            {
                 sps.BatteryFlag  = BATTERY_FLAG_LOW;
		   BspNotifyLed(TRUE);		 
            }		
            else if(ucBatteryPer<=BATTERY_AUTOSLEEP_VOLTAGE_PERCENT)
            {
                 sps.BatteryFlag  = BATTERY_FLAG_CRITICAL;
		   BspNotifyLed(TRUE);	
            }
            sps.BatteryLifePercent = 	ucBatteryPer;
	   }
//=======================================================		

		
    }
    else	
    {
        if(PDD_BATTERY_EOC == battery_status)
        {
            RETAILMSG(0, (TEXT("+Battery status PDD_BATTERY_EOC...  \r\n"))); 
            BspFuelGaugeResetBatStatusInPercent();	   
            BspNotifyLed(FALSE);
            sps.BatteryFlag  = BATTERY_FLAG_HIGH;
            sps.BatteryLifePercent =  100;
        } //end if PDD_BATTERY_EOC == battery_status
        else 
        {
            RETAILMSG(0, (TEXT("+Get Battery status PDD_BATTERY_ON...  \r\n"))); 
            ucBatteryPer = BspFuelGaugeGetBatStatusInPercent();

            if(ucBatteryPer >= PERCENT_CRITICAL2LOW)
            {
            ///// //Awisc.Chen add 2008-07-14 18:36 start
            #if 0//awisc add for test
            	  DPSTR("ucBatteryPer >= 20");
                 sps.BatteryFlag  = BATTERY_FLAG_CRITICAL;
		   BspNotifyLed(TRUE);	
		    //[david.modify] 2008-05-31 14:39
		   if(g_stBattParam.u32LowPowerSleep) {
		   	BspBattCriticalLow();}
		#endif
		///// //Awisc.Chen add 2008-07-14 18:35 end
                sps.BatteryFlag  = BATTERY_FLAG_HIGH; //Awisc.Chen delete 2008-07-14 18:36
  	         BspNotifyLed(FALSE); //Awisc.Chen delete 2008-07-14 18:36
            }		 
            else if (( ucBatteryPer <PERCENT_CRITICAL2LOW) && (ucBatteryPer >=BATTERY_AUTOSLEEP_VOLTAGE_PERCENT))
            {
                 sps.BatteryFlag  = BATTERY_FLAG_LOW;
		   BspNotifyLed(TRUE);		 
            }		
            else if(ucBatteryPer<=BATTERY_AUTOSLEEP_VOLTAGE_PERCENT) 
            {
            	  DPSTR("BATTERY_FLAG_CRITICAL");
                 sps.BatteryFlag  = BATTERY_FLAG_CRITICAL;
		   BspNotifyLed(TRUE);	
		    //[david.modify] 2008-05-31 14:39
		   if(g_stBattParam.u32LowPowerSleep) {
		   	BspBattCriticalLow();
		   }
            }
            sps.BatteryLifePercent = 	ucBatteryPer;
        }	//end else PDD_BATTERY_ONLY 	
    }//end else  PDD_BATTERY_CHARGING == battery_status

    sps.Reserved1                  = 0;
    sps.BatteryLifeTime            = BATTERY_LIFE_UNKNOWN;
    sps.BatteryFullLifeTime        = BATTERY_LIFE_UNKNOWN;
	
    sps.Reserved2                  = 0;
    sps.BackupBatteryFlag          = BATTERY_FLAG_UNKNOWN;
    sps.BackupBatteryLifePercent   = 0;
    sps.Reserved3                  = 0;
    sps.BackupBatteryLifeTime      = BATTERY_LIFE_UNKNOWN;
    sps.BackupBatteryFullLifeTime  = BATTERY_LIFE_UNKNOWN;
	
    sps.BatteryChemistry           = BATTERY_CHEMISTRY_LIPOLY;
//  sps.BatteryVoltage             = (unsigned long) (voltage_percent * 4.1); ;
    sps.BatteryCurrent             = 0;
    sps.BatteryAverageCurrent      = 0;
    sps.BatteryAverageInterval     = 0;
    sps.BatterymAHourConsumed      = 0;
    sps.BatteryTemperature         = 0;//temperature;
    sps.BackupBatteryVoltage       = 0;

    memcpy(&gpStatus->sps, &sps, sizeof(gpStatus->sps));
    
    // get battery status information from the shared structure
    LockBattery();
    memcpy(pstatus, &gpStatus->sps, sizeof(*pstatus));
    *pfBatteriesChangedSinceLastCall = gpStatus->fChanged;
    if(*pfBatteriesChangedSinceLastCall) {
        gpStatus->fChanged = FALSE;		// reset changed flag if it's set
    }
    UnlockBattery();
    
    DEBUGMSG(ZONE_PDD, (_T("%s: returning %d\r\n"), TEXT(__FUNCTION__), fOk));
    return (fOk);
}




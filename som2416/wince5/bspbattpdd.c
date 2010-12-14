/* 
 * $Header$  
 * Copyright (c) 2005-2006 Centrality Comunications Inc.  
 * All Rights Reserved.
 *
 * Confidential and Proprietary to Centrality Communications Inc.
 *
 * Module Name:
 *        Batt_PDD.c	
 *
 * Abstract:
 *   Implementation battery  PDD driver
 *
 * Author: Xiang.Li
 *
 * Functions:
 *
 *
 * $Log$
 *
*/

#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include "windev.h"
#include "BspBattPdd.h"
#include <bsp.h>
#include <dbgmsg_david.h>
#include <xllp_gpio_david.h>
#include <ceddk.h>


#define ZONE_BATT               0
#define ZONE_BATT_ERROR               0


#define TCH_CTRL2_BATTERY_MODE  0xc000
#define TCH_CTRL1_BATTERY_MODE  0xe000
#define TCH_DATA_BATTERY_SRC    0x6000
#define TCH_DATA_BATTERY_VALUE  0x0fff
#define WM9712_TCH_CTRL1		        0x76
#define WM9712_TCH_CTRL2		        0x78
#define WM9712_TCH_DATA				0x7A
#define TCH_CTRL2_DEFAULT_MODE	0xc203


//[david.modify] 2008-05-31 11:00
//确定最低电池电压的ADC值和最高电池电压的ADC值
//=======================
//[david.modify] 2008-05-22 19:03
// 实际电压 = 10bit adc/1024 *2 *3.3v
// 4200 = ADC/1024 * 2 * 3.3 *1000 
// --> ADC=4200 *1024 / (1000 *2*3.3) = 4200/6.445
// 每读一次ADC间隔时间(ms)
// 	vol			adc			adc*2
//=====================================
// 4200			651			1303
// 3400			527			1055
// 3700			574			1148
//=====================================

 //Awisc.Chen add 2008-07-14 18:17 start
HANDLE g_hEvent = NULL;
HANDLE g_hEvent2 = NULL;

 //Awisc.Chen add 2008-07-14 18:17 end
#define MAX_VOLTAGE_OFFSET 100 	// MV
#define BATTERY_MAX_ADC ((BATTERY_MAX_VOLTAGE+MAX_VOLTAGE_OFFSET)*1000/6445)		
#define BATTERY_MIN_ADC (BATTERY_MIN_VOLTAGE*1000/6445)		//


//=======================


#define WM9712_MAX_DATA       1385		
#define WM9712_MIN_DATA       1149		

#define WM9712_TO_BATTERY_H      3113// 3050		
#define WM9712_TO_BATTERY_M       3095//3045		
#define WM9712_TO_BATTERY_L       3068//3040		


#define PDD_FUELGAUGE_POLLING       24      /*Fuel gauge polling time*/
#define CHARGER_VOLTAGE_GAUGE_COUNT      12      /*Gauge voltage og charger*/
#define BATTERY_CRITICAL_WAITING    3      /*Waiting time for battery critical low*/

#define BATTERY_STABLE_TIME	20
#define BATTERY_CAPACITY_TIME	9200
#define BATTERY_EMPTY_TIME	    8800


typedef DWORD (*PFN_SetSystemPowerState)(LPCWSTR, DWORD, DWORD);
PFN_SetSystemPowerState gpfnSetSystemPowerState = NULL;


USHORT  g_usChargerVoltageGaugeCount=0;

//Mutex variable
HANDLE g_hADCMutex = NULL;

#define PM_POWER_BATTERY_STATUS_EVENT    	_T("POWER_BATTERY_STATUS")


TCHAR* g_pstrADCMutex = PIN_MUX_WM9712_ADC_MUTEX;

__inline BOOL RequestMutex(HANDLE hMutex, UINT iTimeout)
{
	if(WaitForSingleObject (hMutex, iTimeout) == WAIT_OBJECT_0)
		return TRUE;
	return FALSE;
}



#define REQUEST_ADC_MUTEX()       RequestMutex(g_hADCMutex, INFINITE)
#define RELEASE_ADC_MUTEX()       ReleaseMutex(g_hADCMutex)


HANDLE g_hNotifyLedThread=NULL;
static HANDLE g_hevLEDStatus = NULL;
static BOOL g_bLedThreadExit=FALSE;


USHORT  usLastMinutes = 0;
USHORT  BatCriticalLowWaitCount = 0;

BOOL g_bLedStatus=FALSE;

static HANDLE g_hevBatteryStatus = NULL;
static BYTE g_ACLineStatusCheck = AC_LINE_UNKNOWN;

volatile BSP_ARGS *g_pBspArgs = NULL;

 //[david.modify] 2008-05-23 15:05
//=================================
extern stBattParam g_stBattParam;




volatile S3C2450_IOPORT_REG *v_p2450IOP=NULL;
volatile S3C2450_ADC_REG * v_pADCregs=NULL;
volatile S3C2450_USBD_REG *v_pUSBCtrlAddr = NULL;

//
// Take n readings from the A/D and returned the averaged value.
// Returns milli-volts on success, or (-1) on error.
//

int BatteryGetVol()
{

	UINT32 u32Tmp[4]={0};
	int nRet = 0;	
	DWORD dwADCCONBak, dwADCTSCBak, dwADCADCMUXBak;	


	if(NULL==v_pADCregs) {
//		v_pADCregs = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_ADC, FALSE);	
		nRet = -1;
		DPN(nRet);	
		return nRet;
	}

// 互斥
//===============
       REQUEST_ADC_MUTEX();
	   
	// Backup ADC register
	dwADCCONBak = v_pADCregs->ADCCON;
	dwADCTSCBak = v_pADCregs->ADCTSC;
	dwADCADCMUXBak = v_pADCregs->ADCMUX;
	
#define AIN0_CHANNEL 0     
	v_pADCregs->ADCMUX = AIN0_CHANNEL;

	v_pADCregs->ADCCON = (1<<14) | (49<< 6);	
	v_pADCregs->ADCTSC &= ~(1 << 2);		// Normal ADC conversion
	v_pADCregs->ADCCON |=  (
					(0x0 << 3) |		 // 10 bit
					(1 << 0) |			// ENABLE_START
					(0 ));				/* Start Auto conversion				*/
	while (v_pADCregs->ADCCON & 0x1);			// check if Enable_start is low	
	while (!(v_pADCregs->ADCCON & (1 << 15)));	// Check ECFLG		
	u32Tmp[0] = v_pADCregs->ADCDAT1;
	u32Tmp[1] = v_pADCregs->ADCDAT0;	
	DPNDEC(u32Tmp[0]);
	DPNDEC(u32Tmp[1]);	

    	v_pADCregs->ADCCON |= (1<<2);		// ADC StanbyMode

	// Restore ADC register
	 v_pADCregs->ADCCON = dwADCCONBak;
	v_pADCregs->ADCTSC = dwADCTSCBak;
	v_pADCregs->ADCMUX = dwADCADCMUXBak;
	u32Tmp[1]=u32Tmp[1]&0X3FF;


// 互斥
//===============
        RELEASE_ADC_MUTEX();

//[david.modify] 2008-07-14 10:03
//理论上0--0X3FF都是测出值；
//只不过值偏大或偏小可能测出就不是准确电池电压值
#if 1
	nRet = u32Tmp[1];
	DPNOK(nRet);
	return nRet;
#else
	if( (u32Tmp[1]>=BATTERY_MIN_ADC) &&(u32Tmp[1]<=BATTERY_MAX_ADC) )
	{
		nRet = u32Tmp[1];
		DPNOK(nRet);
		return nRet;
	}else{
		nRet=-2;
		DPNOK(nRet);
		return nRet;
	}
#endif
	
}

//[david.modify] 2008-05-22 19:03
	

UINT32 BatteryGetAvlVol(UINT32 u32Times)
{
	UINT32 u32Temp = 0;
	UINT32 u32AvlADC=0;		//平均ADC
	UINT32 u32AvlMV=0;		//平均VOL=（ mv)
	UINT32 u32Sum=0;
	UINT32 u32Times2=0;	//实际有效ADC
	int i=0;
	int nRet = 0;
	for(i=0;i<u32Times;i++) {

		nRet =BatteryGetVol();
		DPNDEC(nRet);		
		if(nRet<0) {
			// 不应该出现这种情况
		}else{
			u32Temp=(UINT32)nRet;
			u32Sum+=u32Temp;
			u32Times2++;
		}
//		Sleep(VBAT_SAMPLE_TIME1);
		Sleep(g_stBattParam.u32SampleVbatDelay);

	}

 //[david.modify] 2008-07-12 12:10
 	if(u32Times2!=0)
	u32AvlADC =u32Sum/u32Times2;
	else
		u32AvlADC=u32AvlADC;
	DPNDEC(u32Temp);
//	EPRINT("AVL VOL=%d (%d) ", u32Temp, u32Times);
	u32AvlMV = (u32Temp*2*BATTERY_REF_VOLTAGE)/(1024);
	EPRINT(L"Avl Vbat=%d mv (adc=%d)  (cnt=%d) ", u32AvlMV, u32AvlADC, u32Times2);
	return u32AvlADC;
}


INT IsChargerIn()
{
	int i=0;
	int nState = 0;
	stGPIOInfo stGPIOInfo;
	
	if(NULL==v_p2450IOP){
//		v_p2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);	
		DPN(0);	
		return 0;
	}
	// get	
	EPRINT(L"==USB IO==\r\n");
	stGPIOInfo.u32PinNo = USB_DET;
	GetGPIOInfo(&stGPIOInfo, v_p2450IOP);
	EPRINT(L"USB_DET=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	if(stGPIOInfo.u32Stat) {	//高电平
		nState = 0;
	}else{
		nState = 1;			// 低电,表示检测到有USB 或adapter
	}
	
			
	stGPIOInfo.u32PinNo = CH_nFULL;
	GetGPIOInfo(&stGPIOInfo, v_p2450IOP);
	EPRINT(L"CH_nFULL=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = CH_CON;
	GetGPIOInfo(&stGPIOInfo, v_p2450IOP);
	EPRINT(L"CH_nFULL=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = PWREN_USB;
	GetGPIOInfo(&stGPIOInfo, v_p2450IOP);
	EPRINT(L"PWREN_USB=%d: [%d %d %d] \r\n",
		 stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	

	DPNOK(nState);
	if(nState) 
		DPSTR("CHARGER IN!");
	else 
		DPSTR("CHARGER OUT!");
	
	return nState;
}


enum {NO_CHARGER=0, USB_CHARGER=1, ADAPTER_CHAGER=2, ERR_CHARGER=-1};
// CHARGER IN=true, FARR==1 --- USB CABLE
// CHARGER IN=true, FARR==0 --- ADAPTER
int  IsWhichChargerConnected()
{
	int nRet = 0;

 //[david.modify] 2008-05-23 11:53
 	nRet = IsChargerIn();
 	if(!nRet) {
		DPNOK(nRet);
		return NO_CHARGER;
	}
	
	if(v_pUSBCtrlAddr==NULL) {
//		v_pUSBCtrlAddr=(S3C2450_USBD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_USBD, FALSE);
		DPN(0);
		return ERR_CHARGER;
	}

//	PrintMsg(v_pUSBCtrlAddr, sizeof(S3C2450_USBD_REG)/sizeof(UINT32), sizeof(UINT32));
	DPNOK(v_pUSBCtrlAddr->FARR);
	if(v_pUSBCtrlAddr->FARR&0X7F) {
		nRet=USB_CHARGER;		//接PC USB后,v_pUSBCtrlAddr->FARR=1
		DPSTR("USB CABLE!");		
	}else {
		nRet=ADAPTER_CHAGER;
		DPSTR("ADAPTER!");				

	}
	DPNOK(nRet);
	
	return nRet;
}




 __inline HANDLE InitPinMuxMutex(TCHAR* pstrName)
{
	return (CreateMutex(NULL, FALSE, pstrName));
}

__inline VOID DeinitPinMuxMutex(HANDLE hMutex)
{
	CloseHandle(hMutex);
}
//=================================




/*************************************************************************************
// 		Function: SystemTimeMinutesGone
// 	     Purpose: gauge system minutes gone 
//       Returns: Minutes gone
//***********************************************************************************/
static USHORT
SystemTimeMinutesGone(void)
{
    SYSTEMTIME st;
    USHORT usMinutesGone=0;	
    GetSystemTime(&st);
    RETAILMSG(1, (TEXT("++TIME: At %d O'clock %d Minutes %d Seconds\r\n"),st.wHour, st.wMinute, st.wSecond));
    usMinutesGone = st.wMinute-usLastMinutes;
    usLastMinutes = st.wMinute;	
    return  usMinutesGone;
}


#include <Nled.h>
void SetNldStatus(int nLedNum , int nLedStatus)//振动func
{
	struct NLED_SETTINGS_INFO g_LedSetInfo;	
	HANDLE g_hNled;
	LPVOID g_pInBuf;	//数据buffer指针	

	
	//控制LED的振动
	if((g_hNled = CreateFile(L"NLD1:", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE)
	{
		DPN(g_hNled);
		goto err_exit;
	}
	DPNOK(g_hNled);	
	g_LedSetInfo.LedNum = nLedNum ;//LED编号
	g_LedSetInfo.OffOnBlink = nLedStatus;// LED状态
	g_pInBuf = &g_LedSetInfo;
	DeviceIoControl(g_hNled , IOCTL_NLED_SETDEVICE , g_pInBuf , sizeof(g_LedSetInfo) , NULL, 0, NULL, NULL);
err_exit:
	if(g_hNled)
		CloseHandle(g_hNled);
	
}





static void NotificationLed(BOOL bLed)
{
 //[david.modify] 2008-05-23 15:07
 // 直接控制GPIO控制LED,或者用驱动调用NLED驱动
 //================================
	DPNOK(bLed);
	if(bLed)
	{
	// nld - S805G上,已经OK；
		SetNldStatus(0, 1);	
	}
	else
	{
		SetNldStatus(0, 0);	
	}
 //================================	
}

int WINAPI NotifyLedThread()
{
    DWORD dwWait = INFINITE;
    DWORD dwRet;

    while (!g_bLedThreadExit) 
    {
        dwRet = WaitForSingleObject(g_hevLEDStatus,dwWait);
        if (g_bLedStatus)
        {
            dwWait = 1000;
            NotificationLed(TRUE);
	     Sleep(dwWait);		
            NotificationLed(FALSE);
        }		
        else
        {
            dwWait = INFINITE;
            NotificationLed(FALSE);
        }
        if (dwRet == WAIT_FAILED)
        {
            RETAILMSG(ZONE_BATT_ERROR||1,(TEXT("+++ LEDCtrlThread +++ wait g_hevLEDStatus event failed!!!, exit thread\r\n")));
            g_bLedThreadExit = TRUE;
        }
    }
    return 0;
}

// 		Function: BspNotifyLed
// 	     Purpose: Control notification LED  
//       Returns: None 
//***********************************************************************************/
void BspNotifyLed(BOOL bLedOn)
{

    if(bLedOn)
    {
	 if(!g_bLedStatus)
	 {
	      g_bLedStatus = TRUE;
	      SetEvent(g_hevLEDStatus);	 
	 }	
    }
    else
    {
	  if(g_bLedStatus)	
	  {
             g_bLedStatus = FALSE;
	      SetEvent(g_hevLEDStatus);
	  }
    }
		
}
/*************************************************************************************
// 		Function: BspBattModuleConfig
// 	     Purpose: Config and preset  battery related register and GPIOs  
//       Returns: TRUE--config succeed FALSE:failed config 
//***********************************************************************************/
stGPIOInfo g_stGPIOInfo[]={
	{ USB_DET,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},
	{ CH_nFULL,  0, ALT_FUNC_IN, PULL_UPDOWN_DISABLE},
	{ CH_CON,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},	
	{ PWREN_USB,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
};
int Print_IO_USB(void* pGPIOVirtBaseAddr)
{
	stGPIOInfo stGPIOInfo;	

	if(0==pGPIOVirtBaseAddr) 
		return -1;

	// get	
	EPRINT(L"==USB IO==\r\n");
	stGPIOInfo.u32PinNo = USB_DET;
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT(L"USB_DET=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
			
	stGPIOInfo.u32PinNo = CH_nFULL;
	//GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr, stGPIOInfo.u32PinNo);
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT(L"CH_nFULL=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = CH_CON;
	//GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr, stGPIOInfo.u32PinNo);
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT(L"CH_nFULL=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	stGPIOInfo.u32PinNo = PWREN_USB;
	//GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr, stGPIOInfo.u32PinNo);
	GetGPIOInfo(&stGPIOInfo, pGPIOVirtBaseAddr);
	EPRINT(L"PWREN_USB=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);	
	
	return 1;
}
int Init_IO_USB(void* pGPIOVirtBaseAddr)
{
	int i=0;

	for(i=0;i<sizeof(g_stGPIOInfo)/sizeof(g_stGPIOInfo[0]);i++) {
//		SetGPIOInfo(&g_stGPIOInfo[i],  pGPIOVirtBaseAddr);
	}

	//PRINT RESULT
	Print_IO_USB(pGPIOVirtBaseAddr);
	return TRUE;
}



static BOOL
BspBattModuleConfig(void)
{
 //[david.modify] 2008-05-23 15:04
// 此处配置GPIO,及硬件初始化
//================================
	Init_IO_USB(v_p2450IOP);
//================================
    return TRUE;
}


/*************************************************************************************
// 		Function: BspPowerHandleON
// 	     Purpose: Init bus which shutdhow when enter sleep   
//       Returns: no Return
//***********************************************************************************/
void 
BspPowerHandleOn(VOID)
{
	DPSTR("BspPowerHandleOn");
	BspBattModuleConfig();
	BspFuelGaugeResetBatStatusInPercent();	

}

/*************************************************************************************
// 		Function: BspPowerHandleOff
// 	     Purpose: Deinit bus which shutdhow when enter sleep   
//       Returns: no Return
//***********************************************************************************/
void 
BspPowerHandleOff(VOID)
{
	DPSTR("BspPowerHandleOff");
    BspNotifyLed(FALSE);
    BatCriticalLowWaitCount=0;	   

}

DWORD BattGetPercentFromVoltage_org(DWORD dwVoltage)
{
    DWORD dwBattPercent;    
    DWORD dwBattUsedTime;

    RETAILMSG(0, (TEXT("+BattGetPercentFromVoltage dwVoltage =%d  \r\n"),dwVoltage));

    if(dwVoltage>=4100)
        return 100;
    else if(dwVoltage>=4020)
    {
        dwBattUsedTime =(706576 - 172*dwVoltage)/100;
        RETAILMSG(0, (TEXT("+>4000 dwVoltage =%d  dwBattUsedTime = %d  \r\n"),dwVoltage,dwBattUsedTime));
    }
    else if(dwVoltage>=3720)
    {
        dwBattUsedTime = (822727 - 204*dwVoltage)/10;            
        RETAILMSG(0, (TEXT("+>=3720  dwVoltage =%d dwBattUsedTime = %d  \r\n"),dwVoltage,dwBattUsedTime));

    }
    else
    {
          dwBattUsedTime = 140129 - 36*dwVoltage;
        RETAILMSG(0, (TEXT("+<<3720  dwVoltage =%d dwBattUsedTime = %d  \r\n"),dwVoltage,dwBattUsedTime));
    }
    if(dwBattUsedTime>BATTERY_EMPTY_TIME )
        dwBattUsedTime = BATTERY_EMPTY_TIME;
    dwBattPercent = 100-(dwBattUsedTime*100/BATTERY_CAPACITY_TIME);
//    RETAILMSG(1, (TEXT("+BattGetPercentFromVoltage  dwBattPercent = %d  \r\n"),dwBattPercent));

/*    
    if (dwVoltage>=4100) 
        dwBattPercent = 100;    // 4.10v < Voltage <4.2v
//        dwBattPercent = 85+((dwVoltage-4160)*15/40);
    else if (dwVoltage >= 4050) // 4.05v < Voltage < 4.16v    
        dwBattPercent = 71+((dwVoltage-4050)*29/50);
    else if (dwVoltage > 3750)  // 3.75v < Voltage < 4.05v  
        dwBattPercent = 5+((dwVoltage-3750)*11/50);
    else 
        dwBattPercent = (dwVoltage-3500)/50;      //  Voltage < 3.75v  
*/
    return dwBattPercent;
}


DWORD BattGetPercentFromVoltage(DWORD dwVoltage)
{
    DWORD dwBattPercent;    
    DWORD dwBattUsedTime;

    RETAILMSG(0, (TEXT("+BattGetPercentFromVoltage dwVoltage =%d  \r\n"),dwVoltage));


// Level Indicator 
//get gpStatus->sps.BatteryLifePercent 	

//[david.modify] 2008-06-16 15:29
//因为电池电压在4.1-4.2 V间容易用光，所以在4.1V我们将它设成100，显示满格
//====================================================

//	if(dwVoltage > BATTERY_MAX_VOLTAGE-100)
	if(dwVoltage >  g_stBattParam.u32FullPercentVbat)
		dwBattPercent = 100;
	else if(dwVoltage < BATTERY_MIN_VOLTAGE)
		dwBattPercent= 0;
	else
		dwBattPercent= (BYTE)(((float)dwVoltage  - BATTERY_MIN_VOLTAGE)* 100 /(BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)+1);
   if(dwBattPercent>100) dwBattPercent=100;

    DPNOK(dwBattPercent);   	
    return dwBattPercent;
}


/*************************************************************************************
// 		Function: OEM_GetBattVoltage
// 	     Purpose: Read current battery voltage
//       Returns: N/A
//***********************************************************************************/
static BOOL 
OEM_GetBattVoltage(USHORT *sample) 
{
	BOOL            noError = FALSE;
	UINT32 u32Temp = 0;
//	*sample = 0;
//	u32Temp = BatteryGetAvlVol(SAMPLE_VBAT_TIMES);
	DPNOK(g_stBattParam.u32SampleVbatTimes);
	u32Temp = BatteryGetAvlVol(g_stBattParam.u32SampleVbatTimes);
	DPNOK(u32Temp);
	*sample = (UINT16)u32Temp;
	noError = TRUE;

	return(noError);
}


/*************************************************************************************
// 		Function: SYSCheckBattVoltage
// 	     Purpose: Check battery voltage and Calculate Battery status in percent
//       Returns: Battery status in percent
//***********************************************************************************/
static USHORT
SYSCheckBattVoltage(BOOL bSet)
{
 //[david.modify] 2008-05-24 14:51
 // 这里应该有更复杂的算法
 //应该测一段时间内的平均电压
 //=================================
	USHORT usBatteryPer=0;
 	USHORT avgVol=0;
	USHORT avgADC=0;
	USHORT u16Vbat=0;	
	USHORT usCurrentVoltage=0;	
	UINT32 u32Temp = 0;
	int iCount=0;
    	USHORT usBatteryLowLevel=0;
    	USHORT usBatteryHighLevel=0;		
	SYSTEMTIME      st;	
	
//Set battery percent when battery full
	if(bSet)
	{	  
	  	avgVol =0;
		DPNOK(bSet);
		return 100;
	} 		  
//	OEM_GetBattVoltage(&avgVol);

	for(iCount=0;iCount<ADC_COUNT;iCount++)
	{
		OEM_GetBattVoltage(&avgADC);
		u32Temp+=avgADC;
		Sleep(10);
	}
    	avgADC=u32Temp/ADC_COUNT;	
	DPNOK(avgADC);	

	// 方法1:最简单算法，根据ADC值，固定公式算出电压值
	//===============================================
	avgVol = (avgADC*2*BATTERY_REF_VOLTAGE)/(1024);


 //[david.modify] 2008-06-16 15:16
 // 从现像看, ADC测出值比实际电源输出值小 120mv;
 //=======================
// #define REALVBAT_ADC_OFFSET 120
 #define REALVBAT_ADC_OFFSET 0	//为了保持和EBOOT中一样
	avgVol+=REALVBAT_ADC_OFFSET;

	
	// 方法2:最简单算法，根据ADC值，在ADC不同范围内
	// 乘以的参数不同
	//===============================================
//[david.modify] 2008-05-31 11:00
//确定最低电池电压的ADC值和最高电池电压的ADC值
//=======================
//[david.modify] 2008-05-22 19:03
// 实际电压 = 10bit adc/1024 *2 *3.3v
// 4200 = ADC/1024 * 2 * 3.3 *1000 
// --> ADC=4200 *1024 / (1000 *2*3.3) = 4200/6.445
// 每读一次ADC间隔时间(ms)
// 	vol			adc			adc*2
//=====================================
// 4200			651			1303
// 3400			527			1055
// 3700			574			1148
//=====================================
	GetSystemTime(&st);
	usBatteryPer = (USHORT)BattGetPercentFromVoltage(avgVol);
	DPNOK(avgVol);	
	DPNOK(usBatteryPer);

	if(g_stBattParam.u32Debug&SHOWVBAT_BIT) {
		NKDbgPrintfW(L"Avl Vbat=%d%%, %d mv (adc=%d)  (cnt=%d) [%d:%d:%d]\r\n", usBatteryPer, avgVol, avgADC, ADC_COUNT,
			st.wHour, st.wMinute, st.wSecond );
	}
#ifdef DEBUG	
	EPRINT(L"Avl Vbat=%d%%, %d mv (adc=%d)  (cnt=%d) [%d:%d:%d]\r\n ", usBatteryPer, avgVol, avgADC, ADC_COUNT,
		st.wHour, st.wMinute, st.wSecond );
#endif	
	return (usBatteryPer);		
 //=================================	

}


/*************************************************************************************
// 		Function: BspFuelGaugeResetBatStatusInPercent
// 	     Purpose: reset battery voltage gauge average value
//       Returns: TBD
//***********************************************************************************/
USHORT 
BspFuelGaugeResetBatStatusInPercent(void)						
{
    return SYSCheckBattVoltage(TRUE);
}//end of FuelGaugeResetBatStatusInPercent

/*************************************************************************************
// 		Function: BspFuelGaugeGetBatStatusInPercent
// 	     Purpose: Calculate Battery status in percent
//       Returns: Battery status in percent
//***********************************************************************************/
UCHAR 
BspFuelGaugeGetBatStatusInPercent(void)						
{
    return (UCHAR)SYSCheckBattVoltage(FALSE);;
}//end of BspFuelGaugeGetBatStatusInPercent

/*************************************************************************************
// 		Function: ForceEnterSleep
// 	     Purpose:Force system enter sleep mode
//       Returns: N/A
//	Note:maybe we should put this function into CSP,but we will use it in our current PM function	
//***********************************************************************************/
void 
ForceEnterSleep( void)
{
    if (IsAPIReady(SH_GDI) && IsAPIReady(SH_WMGR))
    {
        RETAILMSG(1, (TEXT("++Force enter sleep....\r\n")));

//        v_pDriverGlobals->misc.bSysGoingSleep = 1;

//        if(g_oalSysInfo.pwrbtnInfo.bBroadcastSleepWakup)
//	        PostMessage( HWND_BROADCAST,WM_ATLAS_SLEEP, 0,0);

        // Check for a power switch event
        if(gpfnSetSystemPowerState != NULL)
        {
            gpfnSetSystemPowerState(NULL, POWER_STATE_SUSPEND, POWER_FORCE);
        }
        else
        {
            RETAILMSG(ZONE_BATT, (TEXT("++Force enter sleep. PowerOffSystem...\r\n")));
            PowerOffSystem();
        }
    }
}

  //[david.modify] 2008-08-18 11:03
  #include <sdk_api_david.h>
  int OEM_IOCTL_Backlight_ONOFF(int nONOff)
  {
  	int nRet;
  	HANDLE hBacklight;

	DPNOK(nONOff);

	if((hBacklight = CreateFile(L"BKL1:", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE)
	{
		DPN(-1);
		return -1;
	}else{	
		DPNOK(hBacklight);
		if(1==nONOff){
		nRet = DeviceIoControl(hBacklight , OEM_BKL_SET_ON, NULL , NULL , NULL, 0, NULL, NULL);
		}else{
		nRet = DeviceIoControl(hBacklight , OEM_BKL_SET_OFF , NULL , NULL , NULL, 0, NULL, NULL);
		}
		CloseHandle(hBacklight);			
	}
	DPNOK(hBacklight);
	return nRet;
  }


/*************************************************************************************
// 		Function: BspBatteryCriticalLow
// 	     Purpose:wait a minutes and  force enter sleep mode
//       Returns: N/A
//***********************************************************************************/

void 
BspBattCriticalLow( void)
{
    DPSTR("endter BspBattCriticalLow");
    if(BatCriticalLowWaitCount <BATTERY_CRITICAL_WAITING)
    {
        RETAILMSG(ZONE_BATT, (TEXT("++WARNING::BatteryCriticalLow !!!\r\n")));
		BatCriticalLowWaitCount++;
	}
	else
	{
        BatCriticalLowWaitCount =0;
//        if (!g_oalSysInfo.apmInfo.bEnabled)
 //[david.modify] 2008-05-23 15:31
	if(1)
        {
		RETAILMSG(ZONE_BATT, (TEXT("++BatteryCriticalLow enter sleep\r\n")));
		if(!(BspAdapterDetect()||BspUSBChargerDetect())) 
			{
//================================================ //Awisc.Chen add 2008-07-14 18:10 start

 //[david.modify] 2008-08-18 09:33
#if 0
				DPSTR("endter event");
				DPNOK(g_hEvent);
				g_hEvent = 	CreateEvent(NULL , FALSE , FALSE  ,  EVENT_OVL);
				DPNOK(g_hEvent);
				if(g_hEvent != INVALID_HANDLE_VALUE)
				{	 
					SetEventData(g_hEvent , BITMAP_LOWBAT_ID);	//显示图片
					SetEvent (g_hEvent);
					 g_hEvent2 = CreateEvent(NULL , FALSE , FALSE  ,  EVENT_OVL_END);	
					DPNOK(g_hEvent2);
					if(g_hEvent2 != INVALID_HANDLE_VALUE)
					{
					DPNOK(g_hEvent2);
						WaitForSingleObject(g_hEvent2, 10000);	//等待10s等待显示完毕
						SetEventData(g_hEvent , BITMAP_HIDE_ID);	//隐藏图片
						SetEvent (g_hEvent);		
						WaitForSingleObject(g_hEvent2, 10000);	//等待10s	等待隐藏完毕//用时40MS					
						CloseHandle(g_hEvent2);		
						g_hEvent2 = NULL;
					}
					CloseHandle(g_hEvent);		
					g_hEvent = NULL;
				}
#else


	DPSTR("+BITMAP_LOWBAT_ID");
	SendOvlEvent(BITMAP_LOWBAT_ID);
	DPSTR("-BITMAP_LOWBAT_ID");
	Sleep(2000);

	// 关背光
	OEM_IOCTL_Backlight_ONOFF(0);		
	
	DPSTR("+BITMAP_HIDE_ID");	
	SendOvlEvent(BITMAP_HIDE_ID);
	DPSTR("-BITMAP_HIDE_ID");

	Sleep(150);
	
#endif

//================================================//Awisc.Chen add 2008-07-14 18:10 end
				ForceEnterSleep();
			} 
         }else
         {
         	//notify the APM driver
		if (g_hevBatteryStatus)
		{
			SetEventData(g_hevBatteryStatus, BatteryLow);
			SetEvent(g_hevBatteryStatus);
		}
         }
	}
}

/*************************************************************************************
// 		Function: BspBattAllocResource
// 	     Purpose:alloc resource for battery driver
//       Returns: TRUE--succeed FALSE--fail 
//***********************************************************************************/
BOOL 
BspBattAllocResource( void)
{
	HMODULE hmCore;

	DPNOK(0);
 //[david.modify] 2008-05-23 15:49
 //===========================
 	if (g_pBspArgs == NULL)
	{
		PHYSICAL_ADDRESS BspArgsPA = { IMAGE_SHARE_ARGS_PA_START, 0 };
		g_pBspArgs = (BSP_ARGS *) MmMapIoSpace(BspArgsPA, sizeof(BSP_ARGS), FALSE);
	}

 	if (v_p2450IOP == NULL)
	{
		PHYSICAL_ADDRESS GpioPA = { S3C2450_BASE_REG_PA_IOPORT, 0 };
		v_p2450IOP = (S3C2450_IOPORT_REG *) MmMapIoSpace(GpioPA, sizeof(S3C2450_IOPORT_REG), FALSE);
	}

 	if (v_pADCregs == NULL)
	{
		PHYSICAL_ADDRESS AdcPA = { S3C2450_BASE_REG_PA_ADC, 0 };
		v_pADCregs = (S3C2450_ADC_REG *) MmMapIoSpace(AdcPA , sizeof(S3C2450_ADC_REG), FALSE);
	}

 	if (v_pUSBCtrlAddr == NULL)
	{
		PHYSICAL_ADDRESS UsbdevPA = { S3C2450_BASE_REG_PA_USBD, 0 };
		v_pUSBCtrlAddr = (S3C2450_USBD_REG *) MmMapIoSpace(UsbdevPA, sizeof(S3C2450_USBD_REG), FALSE);
	}	



	// get pointers to APIs for shutting down the system -- this is only necessary if
	// we are concerned that somebody might sysgen a version of the OS that doesn't
	// contain the appropriate APIs.
	gpfnSetSystemPowerState = NULL;
	hmCore = (HMODULE) LoadLibrary(_T("coredll.dll"));

	if(hmCore != NULL) 
	{
		gpfnSetSystemPowerState = (PFN_SetSystemPowerState) GetProcAddress(hmCore, _T("SetSystemPowerState"));
		FreeLibrary(hmCore);
	}
	else
	{	
		RETAILMSG(ZONE_BATT_ERROR, (TEXT("Batt_LTC:Load coredll.dll failed\r\n")));
		return FALSE;
	}


    BspBattModuleConfig();

    if (g_hADCMutex == NULL)
        g_hADCMutex = InitPinMuxMutex(g_pstrADCMutex);


//    if (g_oalSysInfo.apmInfo.bEnabled)
    if(1) 	
    {
           g_hevBatteryStatus =  CreateEvent(NULL,FALSE,FALSE, PM_POWER_BATTERY_STATUS_EVENT);
	    if ( g_hevBatteryStatus == NULL) 
	    {
		    RETAILMSG(ZONE_BATT_ERROR, (TEXT("Error!  Failed to create g_hevBatteryStatus event.\r\n")));
	    }  
    }

    if(g_hNotifyLedThread == NULL)
    {
	    g_hNotifyLedThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) NotifyLedThread, NULL, 0, NULL);
	    if ( g_hNotifyLedThread == NULL) 
	    {
		    RETAILMSG(ZONE_BATT_ERROR, (TEXT("Error!  Failed to create NotifyLedThread threads.\r\n")));
	    }  
     }

    g_hevLEDStatus = CreateEvent(NULL,FALSE,FALSE, NULL);
    if (g_hevLEDStatus == NULL)
    {
        RETAILMSG(ZONE_BATT_ERROR,(TEXT("Error!  Failed to create event LEDSTATUS error (%d)\r\n"),GetLastError()));
    }

    SetEvent(g_hevLEDStatus);


    return TRUE;


}//end of function BspBattAllocResource

/*************************************************************************************
// 		Function: BspBattDeallocResource
// 	     Purpose:dellloc resource for battery driver
//       Returns: N/A
//***********************************************************************************/
void 
BspBattDeallocResource( void)
{
	DPNOK(0);
    DeinitPinMuxMutex(g_hADCMutex);
    g_hADCMutex = NULL;

    g_bLedThreadExit = TRUE;
    if(g_hevLEDStatus)
        CloseHandle(g_hevLEDStatus);
        g_hevLEDStatus = NULL;	
    if(g_hNotifyLedThread)
        CloseHandle(g_hNotifyLedThread);
        g_hNotifyLedThread = NULL;	
    if (g_hevBatteryStatus)
	CloseHandle(g_hevBatteryStatus);
    g_hevBatteryStatus = NULL;

#if 1
	DPSTR("MmUnmapIoSpace");
	MmUnmapIoSpace(g_pBspArgs, sizeof(BSP_ARGS));
	MmUnmapIoSpace(v_p2450IOP, sizeof(S3C2450_IOPORT_REG));
	MmUnmapIoSpace(v_pADCregs, sizeof(S3C2450_ADC_REG));
	MmUnmapIoSpace(v_pUSBCtrlAddr, sizeof(S3C2450_USBD_REG));	
#endif
 	

	
	
//    BspPowerStatesManagerDeInit();
//    CspRegUnMap();
}//endof BspBatDeallocResource



/*************************************************************************************
// 		Function: BspBatteryDetect
// 	     Purpose: Detect is there a system battery 
//       Returns: TRUE--System battery detected FALSE--NO system battery
//***********************************************************************************/
BOOL
BspCorrectVoltageGauge(void)
{
    UINT32 u32Temp=0;
    OEM_GetBattVoltage(&u32Temp);
    g_usChargerVoltageGaugeCount++;	
    RETAILMSG(0, (TEXT("++BspCorrectVoltageGauge: VBAT %d \r\n"),u32Temp));

    return TRUE;
}	
/*************************************************************************************
// 		Function: BspBatteryDetect
// 	     Purpose: Detect is there a system battery 
//       Returns: TRUE--System battery detected FALSE--NO system battery
//***********************************************************************************/
BOOL
BspBatteryDetect(void)
{
	DPNOK(0);
      return TRUE;
}	


/*************************************************************************************
// 	       Function: BspChargerDetect
// 	     Purpose: Give a AC status to upper layer
//       Returns: TRUE--ACOnline  FALSE--ACOff
//***********************************************************************************/
BOOL 
BspChargerDetect(void)
{
	int nRet = 0;
	nRet = IsChargerIn();
	DPNOK(nRet);
	if(nRet) 
		return TRUE;
	else
		return FALSE;
}


/*************************************************************************************
// 	       Function: BspAdapterDetect
// 	     Purpose: Give a AC status to upper layer
//       Returns: TRUE--ACOnline  FALSE--ACOff
//***********************************************************************************/
BOOL 
BspUSBChargerDetect(void)
{
	
	BOOL bRet = FALSE;
	int nRet = 0;
	nRet = IsWhichChargerConnected();
	DPNOK(nRet);
	if(nRet==USB_CHARGER)
	{		
		return TRUE;
	}
	else
	{
		return FALSE;
	}	

}


/*************************************************************************************
// 	       Function: BspAdapterDetect
// 	     Purpose: Give a AC status to upper layer
//       Returns: TRUE--ACOnline  FALSE--ACOff
//***********************************************************************************/
BOOL 
BspAdapterDetect(void)
{
	BOOL bRet = FALSE;
	int nRet = 0;
	nRet = IsWhichChargerConnected();
	DPNOK(nRet);
	if(nRet==ADAPTER_CHAGER)
	{
		return TRUE;
	}else{
		return FALSE;
	}	
}


/*************************************************************************************
// 	       Function: BspGetACStatus
// 	     Purpose: Give a AC status to upper layer
//       Returns: TRUE--ACOnline  FALSE--ACOff
//***********************************************************************************/
BOOL 
BspGetACStatus(void)
{
	BOOL bRet = FALSE;
	
      if(BspAdapterDetect()||BspUSBChargerDetect())
		bRet = TRUE;
      	
	// if we support APM, then check the AC status and notify to APM
//	if (g_oalSysInfo.apmInfo.bEnabled)
	{
		BYTE ACLineStatus = AC_LINE_OFFLINE;

		if (bRet)
			ACLineStatus = AC_LINE_ONLINE;

		if ((g_ACLineStatusCheck != AC_LINE_UNKNOWN) 
			&& (g_ACLineStatusCheck != ACLineStatus))
		{
			//notify the APM driver
			if (g_hevBatteryStatus)
			{
				if (ACLineStatus == AC_LINE_ONLINE)
					SetEventData(g_hevBatteryStatus, ACIn);
				else
					SetEventData(g_hevBatteryStatus, ACOut);					
				SetEvent(g_hevBatteryStatus);
			}
		}

		g_ACLineStatusCheck = ACLineStatus;
	}

	return bRet;
}

/*************************************************************************************
// 	       Function: BspGetBatteryFlag
// 	     Purpose: Give a AC status to upper layer
//       Returns: TRUE--ACOnline  FALSE--ACOff
//***********************************************************************************/
UCHAR 
BspGetBatteryFlag_old (void)
{
 //           return PDD_BATTERY_ON;

    if(BspBatteryDetect())
    {
        if(BspChargerDetect())
            return PDD_BATTERY_CHARGING;
        else if(BspAdapterDetect()||BspUSBChargerDetect())
           {
      		  RETAILMSG(0, (TEXT("++PDD_BATTERY_EOC \r\n")));
	         BspCorrectVoltageGauge();
                return PDD_BATTERY_EOC;
           }		
        else
            return PDD_BATTERY_ON;
    }
    else
    {
        return PDD_NO_BATTERY;
    }
}//endof BspGetBatteryStatus


 //[david.modify] 2008-10-13 10:19
 //=============================================
// 充电检测
// 当USB_DET为低时表示USB插入或者充电器插入
// 在USB_DET为低后，当CH_nFULL为低时，表示电池充满，否则正在充电
#define BAT_CHARGING_STAT 0
#define BAT_FULL_STAT 1
INT IsChargingOrFull()
{
	int i=0;
	int nState = 0;
	stGPIOInfo stGPIOInfo;
	
	if(NULL==v_p2450IOP){
//		v_p2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);	
		DPN(0);	
		return -1;
	}
	// get	

			
	stGPIOInfo.u32PinNo = CH_nFULL;
	GetGPIOInfo(&stGPIOInfo, v_p2450IOP);
	EPRINT(L"CH_nFULL=%d: [%d %d %d] \r\n",
		stGPIOInfo.u32PinNo, 
		stGPIOInfo.u32Stat, stGPIOInfo.u32AltFunc, stGPIOInfo.u32PullUpdown);		

	if(stGPIOInfo.u32Stat) {
		nState = BAT_CHARGING_STAT;
		DPSTR("Charging!!");
	}else{
		nState = BAT_FULL_STAT;
		DPSTR("FULL!!");
	}	
	return nState;
}


 UCHAR 
BspGetBatteryFlag (void)
{
 //           return PDD_BATTERY_ON;

    if(BspBatteryDetect())
    {
        if(BspChargerDetect()) {

		if(BAT_FULL_STAT==IsChargingOrFull()) {
			DPSTR("PDD_BATTERY_EOC");
	              return PDD_BATTERY_EOC;
		}
		DPSTR("PDD_BATTERY_CHARGING");
	       return PDD_BATTERY_CHARGING;
        }
        else{
		DPSTR("PDD_BATTERY_ON");			
	       return PDD_BATTERY_ON;
        }
    }
    else
    {
	DPSTR("PDD_NO_BATTERY");    
	return PDD_NO_BATTERY;
    }
}//endof BspGetBatteryStatus
 //=============================================



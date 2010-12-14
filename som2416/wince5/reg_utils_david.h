#ifndef REG_UTILS_DAVID_H
#define  REG_UTILS_DAVID_H




 //[david.modify] 2008-05-31 12:16
 //share pin mux mutex name
#define PIN_MUX_WM9712_ADC_MUTEX        TEXT("PIN_MUX_WM9712_ADC_MUTEX")

 //[david.modify] 2008-06-19 17:57
 /*
** hotkey common event name
*/
#define ATLAS_AUDIO_VOL_EVENT		_T("_ATLAS_AUDIO_VOL_EVENT")
#define ATLAS_AUDIO_VOL_MSG			_T("_ATLAS_AUDIO_VOL_MSG")

 //[david.modify] 2008-07-11 11:52
 //OVERLAY
#define EVENT_OVL			_T("EVENT_OVL_LEADER")
#define EVENT_OVL_END		_T("EVENT_OVL_END_LEADER")
#define PM_POWER_NOTIFY_EVENT2 _T("SHARE_POWER_NOTIFY_EVENT2") 


#define BITMAP_LOWBAT_ID	1	// lowbat_bg.bmp
#define BITMAP_USBDISK_ID	2	// usb_bg.bmp
//#define BITMPA_NOPLUG_ID 3		//noplug_bg.bmp
#define BITMPA_SLEEP_ID 4		//sleep_bg.bmp
//#define BITMPA_SLEEP_ID 5		//sleep_bg_blue.bmp
//#define BITMPA_SLEEP_ID 6		//sleep_bg_gps.bmp
//#define BITMPA_SLEEP_ID 7		//sleep_bg_lonston.bmp
#define BITMAP_HIDE_ID 	100



#include <windows.h>
//typedef int             int;
#define false 0
#define true 1
static int GetRegistryDWORD(const HKEY hRoot, const TCHAR* const psKeyName, const TCHAR* const psValueName, DWORD *const pdwValue)
	{
	int bSuccess = false;
	HKEY hKey;
	DWORD dwDWORD;
	DWORD dwDisposition;
	if((0 != pdwValue) && (ERROR_SUCCESS == RegCreateKeyEx(hRoot, psKeyName, 0, TEXT(""), 0, KEY_READ, 0, &hKey, &dwDisposition)))
		{
		DWORD dwValueSize = sizeof(dwDWORD);
		bSuccess = ((ERROR_SUCCESS == RegQueryValueEx(hKey, psValueName, 0, 0, (BYTE*)(&dwDWORD), &dwValueSize)) && (sizeof(dwDWORD) == dwValueSize));
		RegCloseKey(hKey);
		if(bSuccess)
			{
			*pdwValue = dwDWORD;
			}
		}
	return bSuccess;
	}
static int SetRegistryDWORD(const HKEY hRoot, const TCHAR* const psKeyName, const TCHAR* const psValueName, const DWORD dwValue)
	{
	int bSuccess = false;
	HKEY hKey;
	DWORD dwDisposition;
	if(ERROR_SUCCESS == RegCreateKeyEx(hRoot, psKeyName, 0, TEXT(""), 0, KEY_WRITE, 0, &hKey, &dwDisposition))
		{
		bSuccess = (ERROR_SUCCESS == RegSetValueEx(hKey, psValueName, 0, REG_DWORD, (BYTE*)(&dwValue), sizeof(dwValue)));
		RegCloseKey(hKey);
		}
	return bSuccess;
	}

// end

 //[david.modify] 2008-07-23 19:34
 //=============================
static int SendOvlEvent(UINT32 u32ID)
{

#if 0
#else
	HANDLE hEvent=NULL;
	
	DPNOK(u32ID);
	hEvent = 	CreateEvent(NULL , FALSE , FALSE  ,  EVENT_OVL);
	DPNOK(hEvent);
	if(hEvent != INVALID_HANDLE_VALUE)
	{	 
		SetEventData(hEvent , u32ID);	//œ‘ æÕº∆¨
		SetEvent (hEvent);
				
		CloseHandle(hEvent);		
		hEvent = NULL;
	}
#endif	
	return 1;
}
 //=============================

#endif

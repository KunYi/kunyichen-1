
#include <windows.h> 
//#include <cspregs.h>
#include <memory.h>
//#include "drvglob.h"
//#include "utlwrap.h"
//#include "macros.h"
//#include <drvlib.h>
#include "windev.h"

//#define A3_FM1182   //mark the different code

// #define IOCTL_FM1182_POWER 501
#define MEM_WRITE_LENTH 5
#define FM1182_BAUDRATE 9600

BOOL g_hFMKillThread = FALSE;
BOOL g_bFM1182PowerStatus = FALSE; 
HANDLE g_hFMControlThread = NULL;
HANDLE g_hFMKillThreadEvent = NULL;
HANDLE g_hFMControlEvent = NULL;
HANDLE g_hFMControlComPort = NULL;
const TCHAR *szCOMPort =_T("COM3:"); //which com port is mapped to USP0.

//const unsigned char ConfigBaudrate[] ={0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
//const unsigned char LetItGo[] ={0xA5, 0xA5, 0xA5, 0xA5, 0xA5};

#undef DEBUGMSG
#define DEBUGMSG(cond, msg)	RETAILMSG(1, msg)
#define ZONE_ERROR   1
#define ZONE_FUNCTION   1

//#define FM_TEST   //f.w.lin for test

#ifdef FM_TEST
HANDLE 	g_hFMPort = NULL;
#endif

/*
 * Function: OpenAndConfigureComPort
 * ---------------------------------
 * This utility funtion opens a COM port configured to 9600 baud with 8N1 setting.
 * The port string is passed in as the only parameter.  Upon success, the handle to
 * the newly opened COM port is returned.
 */
static HANDLE OpenAndConfigureComPort (const TCHAR *portStr)
{
    const DWORD dbBaudRate=FM1182_BAUDRATE;
    DCB dcb;
    HANDLE hComPort;
    COMMTIMEOUTS timeouts;

    RETAILMSG(1, (TEXT("+OpenAndConfigureComPort+\r\n")));

    hComPort =  CreateFile(portStr,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_SYSTEM,
                        NULL);

    if (hComPort == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(1, (TEXT("Unable to open %s to communicate\r\n"), portStr));
        return hComPort;
    }
  #ifdef FM_TEST    
    g_hFMPort = hComPort;
  #endif

    RETAILMSG(1, (TEXT("Opened %s successfully\r\n"), portStr));

    if (!GetCommState(hComPort, &dcb))
    {
        CloseHandle(hComPort);
        RETAILMSG(1, (TEXT("Unable to get comport state.\r\n")));
        return INVALID_HANDLE_VALUE;
    }

    dcb.DCBlength = sizeof(DCB);
    dcb.BaudRate = dbBaudRate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(hComPort, &dcb))
    {
        CloseHandle(hComPort);
        RETAILMSG(1, (TEXT("Unable to set comport state\r\n")));
        return INVALID_HANDLE_VALUE;
    }


    //Timeout value is depending on IrDA remote controller hardware spec.
    //25 ms is one experience value for #4136
    GetCommTimeouts(hComPort, &timeouts);
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.ReadTotalTimeoutMultiplier = 25;
    timeouts.ReadIntervalTimeout = 25;          //We are assuming Rmt controller send 2 character with 50 ms

    if ( !SetCommTimeouts(hComPort, &timeouts) )
    {
        CloseHandle(hComPort);
        RETAILMSG(1,(TEXT("Unable to set COMM TIMOUT values\r\n")));
        return INVALID_HANDLE_VALUE;
    }

    DEBUGMSG(1, (TEXT("-OpenAndConfigureComPort-\r\n")));
    return hComPort;
}

BOOL FM1182ConfigToCom(HANDLE hComPort)
{
    BOOL bResult; 
    DWORD dwBytesWritten;
    int i=0;
	#if 1
    //RETAILMSG(1,(TEXT("***FM1182ConfigToCom+++\r\n")));//f.w.lin
    unsigned char FmInitStr[][7] ={
     	{0xFC, 0xF3, 0x3B, 0x0F,0xA9,0x00,0x80},    //
     	{0xFC, 0xF3, 0x3B, 0x1E,0x0F,0x42,0x00},    //1E0F SPK volume drop(dB)
     	{0xFC, 0xF3, 0x3B, 0x1E,0x34,0x00,0x88},	//IE34 MIC pgagain
     	{0xFC, 0xF3, 0x3B, 0x1E,0x3D,0x03,0x00},	//1E3D 0100//mic volume//0300
     	//{0xFC, 0xF3, 0x3B, 0x1E,0x3E,0x04,0x20},	//1E3E 0A00//speak volume//0a00
        {0xFC, 0xF3, 0x3B, 0x1E,0x3E,0xFE,0x00},	//1E3E speak volume//0a00
     	{0xFC, 0xF3, 0x3B, 0x1E,0x41,0x00,0x02},	//1E41 num_of_mics
     	{0xFC, 0xF3, 0x3B, 0x1E,0x45,0x43,0xDE},	//1E45 Sp_flag
     	{0xFC, 0xF3, 0x3B, 0x1E,0x4D,0x01,0x20},	//1E46 Ft_flag
     	{0xFC, 0xF3, 0x3B, 0x1E,0x52,0x00,0x13},	//1E52 DSP_Mips
     	{0xFC, 0xF3, 0x3B, 0x1E,0x63,0x00,0x01},	//1E63 Aec_delay_length 
     	{0xFC, 0xF3, 0x3B, 0x1E,0x86,0x00,0x07},	//1E8C 0380
     	{0xFC, 0xF3, 0x3B, 0x1E,0x87,0x00,0x03},	//1EDA 3000
     	{0xFC, 0xF3, 0x3B, 0x1E,0xC5,0x10,0x00},	//1EDB 1A00
     	{0xFC, 0xF3, 0x3B, 0x1E,0xDA,0x36,0x00},	//1E1D 0200
     	{0xFC, 0xF3, 0x3B, 0x1E,0x3A,0x00,0x00}	
    	};	

	for(i=0;i<15;i++)
    {
        bResult = WriteFile (hComPort, &FmInitStr[i][0], 7, &dwBytesWritten, NULL);
    }
   
#else
		unsigned char FmInitStr[][7] ={
     	{0xFC, 0xF3, 0x3B, 0x1E,0x34,0x00,0xAA},  //1E34 009C
     	{0xFC, 0xF3, 0x3B, 0x1E,0x3D,0x01,0x20},	//1E3D 0100//mic volume//0180
     	{0xFC, 0xF3, 0x3B, 0x1E,0x3E,0x08,0x00},	//1E3E 0A00//volume//0a00
     	{0xFC, 0xF3, 0x3B, 0x1E,0x41,0x00,0x01},	//1E44 0005
     	{0xFC, 0xF3, 0x3B, 0x1E,0x45,0x43,0xDE},	//1E45 4BFC
     	{0xFC, 0xF3, 0x3B, 0x1E,0x4D,0x02,0x00},	//1E46 0075
     	{0xFC, 0xF3, 0x3B, 0x1E,0x4E,0x03,0x98},	//1E46 0075
     	{0xFC, 0xF3, 0x3B, 0x1E,0x63,0x00,0x01},	//1E59 0fff//avoid wrap around
     	{0xFC, 0xF3, 0x3B, 0x1E,0x86,0x00,0x09},	//1E86 0007
     	{0xFC, 0xF3, 0x3B, 0x1E,0x87,0x00,0x05},	//1E87 0002  
     	{0xFC, 0xF3, 0x3B, 0x1E,0x8C,0x01,0x00},	//1E8C 0380
     	{0xFC, 0xF3, 0x3B, 0x1E,0xDA,0x30,0x00},	//1EDA 3000
     	{0xFC, 0xF3, 0x3B, 0x1E,0xDF,0x32,0x00},	//1EDB 1A00
     	{0xFC, 0xF3, 0x3B, 0x1E,0xA9,0x00,0x80},	//1E1D 0200
     	{0xFC, 0xF3, 0x3B, 0x1E,0x3A,0x00,0x00}	
    	};	

	for(i=0;i<15;i++)
    {
        bResult = WriteFile (hComPort, &FmInitStr[i][0], 7, &dwBytesWritten, NULL);    
    }
#endif	
/*    unsigned char FmInitStr[][7] ={
        {0xFC, 0xF3, 0x3B, 0x1E,0x1D,0x02,0x00},
        {0xFC, 0xF3, 0x3B, 0x1F,0x08,0x01,0x00},
        {0xFC, 0xF3, 0x3B, 0x1E,0x34,0x00,0xAC},  //00 AC
        {0xFC, 0xF3, 0x3B, 0x1E,0x38,0x00,0x0F},  //00 1F
        {0xFC, 0xF3, 0x3B, 0x1E,0x3D,0x08,0x00},  //04 00 ***
        {0xFC, 0xF3, 0x3B, 0x1E,0x86,0x00,0x06},
        {0xFC, 0xF3, 0x3B, 0x1E,0x87,0x00,0x02},  //00 04
        {0xFC, 0xF3, 0x3B, 0x1E,0x9A,0x04,0x00},  
        {0xFC, 0xF3, 0x3B, 0x1E,0x63,0x00,0x16},  //00 08
        {0xFC, 0xF3, 0x3B, 0x1E,0x46,0x00,0x75},  //00 75
        {0xFC, 0xF3, 0x3B, 0x1E,0x44,0x00,0x05},
        {0xFC, 0xF3, 0x3B, 0x1E,0x45,0x4B,0xFC},  //0xCF,0xDE},          
        {0xFC, 0xF3, 0x3B, 0x1E,0x3A,0x00,0x00},
        };
*/
    if (bResult)
        RETAILMSG(1,(TEXT("++Config FM1182 successful++ \r\n")));
    else
        RETAILMSG(1,(TEXT("++Config FM1182 failed++ \r\n")));
    return bResult;
}

VOID FM1182PowerOn(BOOL bPower)
{
    if(bPower)
    {
        //Power On FM1182
        RETAILMSG(1,(TEXT("FM1182CTRL::Power UP TBD...\r\n")));
    }
    else
    {
        //Power down FM1182
        RETAILMSG(1,(TEXT("FM1182CTRL::Power down TBD...\r\n")));
    }
}

int WINAPI FM1182ControlThread()
{
    RETAILMSG (1, (TEXT("++FM1182ControlThread  \r\n")));

    while(!g_hFMKillThread && g_hFMControlEvent)
    {
        WaitForSingleObject(g_hFMControlEvent, INFINITE);

        if (g_bFM1182PowerStatus)
        {
            FM1182PowerOn(TRUE);
            if (!FM1182ConfigToCom(g_hFMControlComPort))
            {
                RETAILMSG(1, (_T("FM1182ControlThread: FM1182 Configure Failed.\r\n")));
            }
        }
        else
        {
            FM1182PowerOn(g_bFM1182PowerStatus);        
        }
    }
    
    SetEvent(g_hFMKillThreadEvent);
	RETAILMSG (1, (TEXT("FM1182ControlThread exiting.\r\n")));
	return (0);
}

HANDLE FME_Open (DWORD dwData, DWORD dwAccess, DWORD dwShareMode) 
{
    if (g_hFMControlComPort) return (HANDLE)FALSE;

	RETAILMSG(1, (TEXT("FME_Open:\r\n")));

    g_bFM1182PowerStatus = TRUE;
    FM1182PowerOn(TRUE);

  #ifdef A3_FM1182
	IOW_REG_OR(ULONG,  &(v_pGpioRegs->gpio[3].paden), GPIO19);
    PIO_OUTPUT_ZERO (GPIO, GPIO_GROUP(3), GPIO_INDEX(19));       // PA PWR , Active High
    PIO_OUTPUT_ENABLE (GPIO, GPIO_GROUP(3), GPIO_INDEX(19));

	msWait(200);

	PIO_OUTPUT_ONE(GPIO, GPIO_GROUP(3), GPIO_INDEX(19));       // PA PWR , Active High
    PIO_OUTPUT_ENABLE (GPIO, GPIO_GROUP(3), GPIO_INDEX(19));

	msWait(200);
  #endif
  
	g_hFMControlComPort = OpenAndConfigureComPort(szCOMPort);
    if (!g_hFMControlComPort)
    {
        RETAILMSG(1,(TEXT("FME_Open: Failed to open %s port, exiting\r\n"), szCOMPort));
        goto OPRN_FAIL;
    }

    if (!FM1182ConfigToCom(g_hFMControlComPort))
    {
        RETAILMSG(1, (_T("FM1182PowerHandler: FM1182 Configure Failed.\r\n")));
        goto OPRN_FAIL;
    }

    /* creat event */
    g_hFMControlEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
    if (!g_hFMControlEvent)
    {
        RETAILMSG(1, (_T("FME_Open: Create FM control Event failed, err(%d)\r\n"), GetLastError() ));
        goto OPRN_FAIL;
    }
    g_hFMKillThreadEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
    if (!g_hFMKillThreadEvent)
    {
        RETAILMSG(1, (_T("FME_Open: Create FM Kill thread Event failed, err(%d)\r\n"), GetLastError() ));
        goto OPRN_FAIL;
    }
    
    /* creat thread */
    g_hFMControlThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FM1182ControlThread, NULL, 0, NULL);
    if ( g_hFMControlThread == NULL)
    {
        RETAILMSG(1, (_T("FME_Open!  Failed to create FM1182ControlThread.\r\n")));
        goto OPRN_FAIL;
    }  
    
    RETAILMSG(1, (TEXT("FME_Open: control handle 0x%x\r\n"), g_hFMControlComPort));
    return (HANDLE)dwData;
	
OPRN_FAIL:
    if (g_hFMControlComPort) CloseHandle(g_hFMControlComPort);
    if (g_hFMControlEvent) CloseHandle(g_hFMControlEvent);
    if (g_hFMKillThreadEvent) CloseHandle(g_hFMKillThreadEvent);
        
    g_bFM1182PowerStatus = FALSE;
    g_hFMKillThread = FALSE;
    g_hFMControlComPort = NULL;
    g_hFMControlEvent = NULL;
    g_hFMKillThreadEvent = NULL;
    g_hFMControlThread = NULL;

    RETAILMSG(1, (TEXT("FME_Open: failed\r\n")));
    return (HANDLE)FALSE;
}

BOOL  FME_Close(DWORD dwData)
{
  	RETAILMSG(1, (TEXT("FME_Close:\r\n")));
    g_bFM1182PowerStatus = FALSE;

    /* kill thread */
    g_hFMKillThread = TRUE;
    SetEvent(g_hFMControlEvent);
    WaitForSingleObject(g_hFMKillThreadEvent, INFINITE);
    Sleep(10);

    /* close handle */
    CloseHandle(g_hFMControlThread);   
    if (g_hFMControlComPort) CloseHandle(g_hFMControlComPort);
    if (g_hFMControlEvent) CloseHandle(g_hFMControlEvent);
    if (g_hFMKillThreadEvent) CloseHandle(g_hFMKillThreadEvent); 

    g_hFMKillThread = FALSE;
    g_hFMControlComPort = NULL;
    g_hFMControlEvent = NULL;
    g_hFMKillThreadEvent = NULL;
    g_hFMControlThread = NULL;

    return (TRUE);
}

BOOL  FME_PowerUp  (VOID)
{
	RETAILMSG(1, (TEXT("FME_PowerUp:\r\n")));
    if (g_hFMControlEvent && g_hFMControlComPort)
    {
	    RETAILMSG(1,(TEXT("++FME_PowerUp++ Enter\r\n")));
	    g_bFM1182PowerStatus = TRUE;
	    SetEvent(g_hFMControlEvent);
    }
	return (TRUE);
}

BOOL  FME_PowerDown(VOID)
{
    if (g_hFMControlComPort)
    {
	    RETAILMSG(1,(TEXT("++FME_PowerDown++ Enter\r\n")));
	    FM1182PowerOn(FALSE); 
	}
	return (TRUE);
}

BOOL FME_IOControl( DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut )
{
    return TRUE;
}

BOOL WINAPI FME_DllEntry(HANDLE  hInstDll, DWORD   dwReason, LPVOID  lpvReserved)
{
	switch ( dwReason )
	{
	case DLL_PROCESS_ATTACH:
		DEBUGREGISTER((HINSTANCE) hInstDll);
		DEBUGMSG(ZONE_INIT,(TEXT("+FM1182CTRL::DLL_PROCESS_ATTACH\r\n")));
		DisableThreadLibraryCalls((HMODULE) hInstDll);
		break;

	case DLL_PROCESS_DETACH:
		DEBUGMSG(ZONE_INIT,(TEXT("+FM1182CTRL::DLL_PROCESS_DETACH\r\n")));
		break;
	}

	return TRUE;
}

HANDLE WINAPI FME_Init(ULONG Identifier) { return (HANDLE)TRUE; }
BOOL  FME_Deinit(DWORD dwData) { return TRUE; }

#ifdef FM_TEST
DWORD FME_Read (DWORD dwData,  LPVOID pBuf, DWORD Len)
{
    DWORD ReadByte = 0;

	if(g_hFMPort != NULL)
	{
		if( !ReadFile(g_hFMPort, pBuf, Len, &ReadByte, NULL) )
		{
			RETAILMSG(1,(_T("++FM_Read++ FAIL \r\n")));
		}
	}
	//RETAILMSG(1, (TEXT("FM_Read,len:%d,result:%d\r\n"), Len,ReadByte));

	return ReadByte;
}

DWORD FME_Write(DWORD dwData, LPCVOID pBuf, DWORD Len)
{
	ULONG lResult = NULL;

	if(g_hFMPort != NULL) 
	{
		if(!WriteFile(g_hFMPort, pBuf, Len, &lResult, NULL))
		{
			RETAILMSG(1,(_T("++FM_Write++ FAIL \r\n")));
		}
	}
	//RETAILMSG(1, (TEXT("FM_Write,len:%d,result:%d\r\n"), Len,lResult));

	return lResult;

}

#else
DWORD FME_Read (DWORD dwData,  LPVOID pBuf, DWORD Len) {return (0);}
DWORD FME_Write(DWORD dwData, LPCVOID pBuf, DWORD Len) {return (0);}
#endif

DWORD FME_Seek (DWORD dwData, long pos, DWORD type) {return (DWORD)-1;}


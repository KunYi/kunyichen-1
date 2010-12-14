
#include <windows.h>
#include <service.h>
#include <types.h>
#include "Pegdser.h"
//#include "gpio.h"
//#include "drvlib.h"
//#include <at_serfifo.h>
//#include <at_serpriv.h>
#include "SerialCSR.h"
#include "ubcsp.h"
#include "PSConfig.h"

#include <xllp_gpio_david.h>
#include "S3C2450_ioport.h"
#include "S3C2450_base_regs.h"


#define DEFAULTBAUD  115200 //38400  
#define REAL_UART   TEXT("COM2:")  //serial 1

HANDLE 	g_hPort = NULL;
BOOL    g_bUpdatePS = TRUE;

BOOL g_hKillThread = FALSE;
HANDLE g_hControlThread = NULL;
HANDLE g_hKillThreadEvent = NULL;
HANDLE g_hControlEvent = NULL;

volatile S3C2450_IOPORT_REG *v_pIOPregs;

stGPIOInfo g_BT_GPIOInfo[]={
	{ BT_PWM, 0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
	{ BT_RST, 0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},        
	//{ GLED,  0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
	};

//extern "C" int SetGPIOInfo( stGPIOInfo *pstGPIOInfo, void* pGPIOVirtBaseAddr );

int WINAPI CSRControlThread()
{
    RETAILMSG (1, (TEXT("++CSRControlThread  \r\n")));

    while(!g_hKillThread && g_hControlEvent)
    {
        WaitForSingleObject(g_hControlEvent, INFINITE);
        RETAILMSG (1, (TEXT("++CSRControlThread  wait event ok\r\n")));
        if (!g_hKillThread)
        {
            HANDLE hDev = CreateFile (L"BTD0:", GENERIC_READ | GENERIC_WRITE,
									FILE_SHARE_READ | FILE_SHARE_WRITE,
									NULL, OPEN_EXISTING, 0, NULL);

            if (hDev == INVALID_HANDLE_VALUE)
            {
                RETAILMSG(1, (TEXT("REFRESH: Unable to open %s to communicate\r\n"), "BTD0:"));
                continue;
            }

            RETAILMSG(1,(_T("IOCTL_SERVICE_REFRESH starting...\r\n")));
            if(!DeviceIoControl (hDev, IOCTL_SERVICE_REFRESH, NULL, 0, NULL, NULL, NULL, NULL))
            {
	        	RETAILMSG(1,(_T("IOCTL_SERVICE_REFRESH error\r\n")));
            }
            RETAILMSG(1, (TEXT("REFRESH: IOCTL_SERVICE_REFRESH finished!\r\n")));

	        CloseHandle (hDev);
        }
    }

    SetEvent(g_hKillThreadEvent);
	RETAILMSG (1, (TEXT("CSRControlThread exiting.\r\n")));
	return(0);
}

//#define CSR_TEST
HANDLE WINAPI CSR_Open(HANDLE pContext, DWORD AccessCode, DWORD ShareMode)
{
    RETAILMSG(1,(_T("--CSR_Open++ Enter:\r\n")));
    //ASSERT(v_pIOPregs);

    /* power on CSR */
    //GetGPIOInfo(&g_BT_GPIOInfo[0], (void*)v_pIOPregs);
    //RETAILMSG(1,(_T("pin:%d,stat:%d,func:%d,updown:%d\r\n"),g_BT_GPIOInfo[0].u32PinNo,g_BT_GPIOInfo[0].u32Stat,\
    //               g_BT_GPIOInfo[0].u32AltFunc,g_BT_GPIOInfo[0].u32PullUpdown));
    //g_BT_GPIOInfo[0].u32Stat = 1;
    #ifdef CSR_TEST
    if( g_BT_GPIOInfo[0].u32Stat == 0 )
        g_BT_GPIOInfo[0].u32Stat = 1;
    else
        g_BT_GPIOInfo[0].u32Stat = 0;
    #else
    //RETAILMSG(1,(_T("--set gpio gpl13=1:\r\n")));
    g_BT_GPIOInfo[0].u32Stat = 1;
    #endif

    SetGPIOInfo(&g_BT_GPIOInfo[0], (void*)v_pIOPregs);

    //reset
    g_BT_GPIOInfo[1].u32Stat = 0;
    SetGPIOInfo(&g_BT_GPIOInfo[1], (void*)v_pIOPregs);

	Sleep(100);

    g_BT_GPIOInfo[1].u32Stat = 1;
    SetGPIOInfo(&g_BT_GPIOInfo[1], (void*)v_pIOPregs);

    //test
    #ifdef CSR_TEST
    GetGPIOInfo(&g_BT_GPIOInfo[0], (void*)v_pIOPregs);
    RETAILMSG(1,(_T("CSR pin:%d,stat:%d,func:%d,updown:%d 22\r\n"),g_BT_GPIOInfo[0].u32PinNo,g_BT_GPIOInfo[0].u32Stat,\
                   g_BT_GPIOInfo[0].u32AltFunc,g_BT_GPIOInfo[0].u32PullUpdown));
    #endif

    if (g_hPort) return (HANDLE)FALSE;

    g_hPort = CreateFile(REAL_UART, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (g_hPort == (HANDLE)-1)
    {
    	RETAILMSG(1, (TEXT("CSR_Open: Unable to CreateFile %s\r\n"), REAL_UART));
    	goto OPRN_FAIL;
    }

    /* configure CSR */
    g_bUpdatePS = FALSE;
    if (!UpdatePersistentStore(g_hPort)) 
    {
        RETAILMSG(1, (TEXT("CSR_Open: UpdatePersistentStore: failed!\r\n")));
        goto OPRN_FAIL;
    }

    RETAILMSG(1,(_T("--CSR_Open++ Leave: g_hPort = %x \r\n"), g_hPort));
	return pContext;

OPRN_FAIL:
    if (g_hPort) CloseHandle(g_hPort);
    g_hPort = NULL;
    g_bUpdatePS = TRUE;
        
    RETAILMSG(1, (TEXT("CSR_Open: failed\r\n")));
    return (HANDLE)FALSE;    
}

BOOL WINAPI CSR_Close(DWORD pContext)
{
 	RETAILMSG(1, (_T("++CSR_Close++ Enter\r\n")));
 	
    if (g_hPort) CloseHandle(g_hPort);
    g_hPort = NULL;
    g_bUpdatePS = TRUE;

    /* power off CSR */
    GetGPIOInfo(&g_BT_GPIOInfo[0], (void*)v_pIOPregs);
    ASSERT(g_BT_GPIOInfo[0].u32Stat == 1);
    g_BT_GPIOInfo[0].u32Stat = 0;
    SetGPIOInfo(&g_BT_GPIOInfo[0], (void*)v_pIOPregs);
    
	return TRUE;
}

BOOL WINAPI CSR_PowerUp(HANDLE pContext)
{
    RETAILMSG(1,(_T("++CSR_PowerUp++ Enter\r\n ")));
     
    //IOW_REG_OR(ULONG,  &(v_pGpioRegs->gpio[5].paden), GPIO18);
    //PIO_OUTPUT_ONE (GPIO, GPIO_GROUP(5), GPIO_INDEX(18));       // PA PWR , Active High
    //PIO_OUTPUT_ENABLE (GPIO, GPIO_GROUP(5), GPIO_INDEX(18)); 
     
    SetEvent(g_hControlEvent);

	return TRUE;
}

BOOL WINAPI CSR_PowerDown(HANDLE pContext)
{
    RETAILMSG(1,(_T("++CSR_PowerDown++ Enter\r\n ")));
    /* power off CSR */
//    PIO_OUTPUT_ZERO (GPIO, GPIO_GROUP(5), GPIO_INDEX(18));
//    PIO_OUTPUT_ENABLE ( GPIO, GPIO_GROUP_INDEX (5),  GPIO_INDEX_IN_GROUP (18) );

	return TRUE;
}

BOOL WINAPI CSR_IOControl(DWORD dwOpenData, DWORD dwCode, PBYTE pBufIn, 
								 DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, 
								 PDWORD pdwActualOut)
{  
    BOOL lResult = NULL;

    if (!g_hPort)
    {
        RETAILMSG(1,(_T("++CSR_IOControl error: g_hPort is NULL\r\n")));
        return FALSE;
    }
#if 1
    if(IOCTL_SERIAL_SET_DCB == dwCode )
    {
        DCB* pdcb;
        //RETAILMSG(1,(_T("++CSR_IOControl++  IOCTL_SERIAL_SET_DCB \r\n")));
        pdcb = (DCB*) (pBufIn);
        if(pdcb->Parity == EVENPARITY)
        {
            pdcb->BaudRate = DEFAULTBAUD;
            pdcb->fOutxDsrFlow = FALSE;    
            pdcb->fDsrSensitivity = FALSE;
            pdcb->Parity = NOPARITY;
            pdcb->fBinary = TRUE;
            pdcb->fTXContinueOnXoff=TRUE;
            pdcb->StopBits = TWOSTOPBITS;
            //RETAILMSG(0,(_T("++CSR_IOControl++  EVENPARITY: pdwActualOut = %d \r\n"), pdwActualOut));
        }
    }
#endif
/*
	switch (dwCode)
	    	{
	    		case IOCTL_SERIAL_SET_BREAK_ON:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_BREAK_ON \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_BREAK_OFF:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_BREAK_OFF \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_DTR:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_DTR \r\n")));
	    			break;
	    		case IOCTL_SERIAL_CLR_DTR:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_CLR_DTR \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_RTS:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_RTS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_CLR_RTS:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_CLR_RTS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_XOFF:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_XOFF \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_XON:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_XON \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_WAIT_MASK:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_GET_WAIT_MASK \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_WAIT_MASK:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_WAIT_MASK \r\n")));
	    			break;
	    		case IOCTL_SERIAL_WAIT_ON_MASK:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_WAIT_ON_MASK \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_COMMSTATUS:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_GET_COMMSTATUS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_MODEMSTATUS:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_GET_MODEMSTATUS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_PROPERTIES:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_GET_PROPERTIES \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_TIMEOUTS:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_TIMEOUTS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_TIMEOUTS:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_GET_TIMEOUTS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_PURGE:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_PURGE \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_QUEUE_SIZE:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_QUEUE_SIZE \r\n")));
	    			break;
	    		case IOCTL_SERIAL_IMMEDIATE_CHAR:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_IMMEDIATE_CHAR \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_DCB:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_GET_DCB \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_DCB:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_SET_DCB \r\n")));
	    			break;
	    		case IOCTL_SERIAL_ENABLE_IR:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_ENABLE_IR \r\n")));
	    			break;
	    		case IOCTL_SERIAL_DISABLE_IR:
	    			RETAILMSG(1,(_T("++CSR_IOControl : IOCTL_SERIAL_DISABLE_IR \r\n")));
	    			break;
	    		}*/
    lResult = DeviceIoControl(g_hPort, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut, 0);
	
    if (!lResult)
    {
        switch (dwCode)
	    {
	    		case IOCTL_SERIAL_SET_BREAK_ON:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_BREAK_ON \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_BREAK_OFF:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_BREAK_OFF \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_DTR:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_DTR \r\n")));
	    			break;
	    		case IOCTL_SERIAL_CLR_DTR:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_CLR_DTR \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_RTS:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_RTS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_CLR_RTS:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_CLR_RTS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_XOFF:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_XOFF \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_XON:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_XON \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_WAIT_MASK:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_GET_WAIT_MASK \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_WAIT_MASK:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_WAIT_MASK \r\n")));
	    			break;
	    		case IOCTL_SERIAL_WAIT_ON_MASK:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_WAIT_ON_MASK \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_COMMSTATUS:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_GET_COMMSTATUS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_MODEMSTATUS:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_GET_MODEMSTATUS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_PROPERTIES:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_GET_PROPERTIES \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_TIMEOUTS:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_TIMEOUTS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_TIMEOUTS:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_GET_TIMEOUTS \r\n")));
	    			break;
	    		case IOCTL_SERIAL_PURGE:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_PURGE \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_QUEUE_SIZE:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_QUEUE_SIZE \r\n")));
	    			break;
	    		case IOCTL_SERIAL_IMMEDIATE_CHAR:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_IMMEDIATE_CHAR \r\n")));
	    			break;
	    		case IOCTL_SERIAL_GET_DCB:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_GET_DCB \r\n")));
	    			break;
	    		case IOCTL_SERIAL_SET_DCB:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_SET_DCB \r\n")));
	    			break;
	    		case IOCTL_SERIAL_ENABLE_IR:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_ENABLE_IR \r\n")));
	    			break;
	    		case IOCTL_SERIAL_DISABLE_IR:
	    			RETAILMSG(1,(_T("++CSR_IOControl error: IOCTL_SERIAL_DISABLE_IR \r\n")));
	    			break;
	    }
        return FALSE;
    }
        
	return TRUE;
}       
        
BOOL UpdatePersistentStore(HANDLE hPort)
{       
	BOOL               bRet                   = FALSE;
	BOOL               bRestoreSettings       = FALSE;
	DWORD              dwLen;
	DCB                dcbOrigConfig;
    COMMTIMEOUTS       ctoTimeouts;
	SERIAL_QUEUE_SIZE  sqsQueue;
	    
	RETAILMSG(1, (TEXT("UpdatePersistentStore: enter, hPort = 0x%x\r\n"), hPort));
        
//return TRUE;
 	// Store current DCB settings
	memset(&dcbOrigConfig, 0, sizeof (dcbOrigConfig));
	dcbOrigConfig.DCBlength     = sizeof(DCB); 
    dwLen = sizeof(DCB);

	bRestoreSettings = CSR_IOControl((DWORD)hPort, IOCTL_SERIAL_GET_DCB, NULL, 0, (PBYTE)&dcbOrigConfig, sizeof(dcbOrigConfig), &dwLen);
        
// *** WARNING - IOCTL_SERIAL_GET_DCB fails with Widcomm bluetooth driver.  No fix completed as this is for MS + out of time
        
	// Setup DCB
	DCB dcbConfig;
	memset(&dcbConfig, 0, sizeof (dcbConfig));
	dcbConfig.DCBlength         = sizeof(DCB); 
	dcbConfig.BaudRate          = DEFAULTBAUD;
	dcbConfig.Parity            = NOPARITY;//EVENPARITY;        
	dcbConfig.ByteSize          = 8;                 
	dcbConfig.StopBits          = TWOSTOPBITS;//TWOSTOPBITS;//STOPBITS_20;//ONESTOPBIT;   
	dcbConfig.fBinary         = TRUE;;   
    if(!CSR_IOControl((DWORD)hPort, IOCTL_SERIAL_SET_DCB, (PBYTE)&dcbConfig, sizeof(dcbConfig), NULL, 0, &dwLen))
    {   
        DebugMessage(1,(_T("++UpdatePersistentStore++ IOCTL_SERIAL_SET_DCB Fail\r\n")));
        return FALSE;
    }   
    // Setup the com port timeouts
    ctoTimeouts.ReadIntervalTimeout         = MAXDWORD;
    ctoTimeouts.ReadTotalTimeoutConstant    = 0;
    ctoTimeouts.ReadTotalTimeoutMultiplier  = 0;
    ctoTimeouts.WriteTotalTimeoutConstant   = 10;
    ctoTimeouts.WriteTotalTimeoutMultiplier = 1000;
    if(!CSR_IOControl((DWORD)hPort, IOCTL_SERIAL_SET_TIMEOUTS, (PBYTE)&ctoTimeouts, sizeof(ctoTimeouts), 0, 0, 0))
    {
        DebugMessage(1,(_T("++UpdatePersistentStore++ IOCTL_SERIAL_SET_TIMEOUTS Fail \r\n")));
        return FALSE;
    }

    // Queues
    sqsQueue.InSize  = 16;
    sqsQueue.OutSize = 16;
    if(!CSR_IOControl((DWORD)hPort, IOCTL_SERIAL_SET_QUEUE_SIZE, (PBYTE)&sqsQueue, sizeof(sqsQueue), 0, 0, 0))
    {
        DebugMessage(1,(_T("++UpdatePersistentStore++ IOCTL_SERIAL_SET_QUEUE_SIZE Fail\r\n")));
        return FALSE;
    }

    DebugMessage(1,(_T("++UpdatePersistentStore++ Succeed configure Bluetooth COM port\r\n")));

	// Call uBCSP update routine
	if(hPort)
		bRet = PSConfig(hPort);

//    DebugMessage(1,(_T("++UpdatePersistentStore++ Restore Bluetooth COM port  Original setting  \r\n")));

	// Reapply stored settings
//    if(bRestoreSettings)
//        CSR_IOControl((DWORD)hPort, IOCTL_SERIAL_SET_DCB, (unsigned char*)&dcbOrigConfig, sizeof(dcbOrigConfig), 0, 0, 0);

	return bRet;
}

ULONG WINAPI CSR_Read(HANDLE pContext, PUCHAR pTargetBuffer, ULONG BufferLength, PULONG pBytesRead)
{
    DWORD ReadByte = 0;

	if(g_hPort != NULL)
	{
		if( !ReadFile(g_hPort, pTargetBuffer, BufferLength, &ReadByte, NULL) )
		{
			RETAILMSG(1,(_T("++CSR_Read++ FAIL \r\n")));
		}
	}

	//RETAILMSG(1, (TEXT("CSR_Read,len:%d,result:%d\r\n"), BufferLength,ReadByte));
#if 0//for test,f.w.lin
        RETAILMSG(1, (TEXT("CSR_Read(%d):"), ReadByte));
        for( ULONG i=0; i < ReadByte; i++ )
            RETAILMSG(1, (TEXT("%02x "), pTargetBuffer[i]));

        RETAILMSG(1, (TEXT("\r\n")));
#endif

	return ReadByte;
}

ULONG WINAPI CSR_Write(HANDLE pContext, PUCHAR pSourceBytes, ULONG NumberOfBytes)
{
	ULONG lResult = NULL;

	if(g_hPort != NULL) 
	{
		if(!WriteFile(g_hPort, pSourceBytes, NumberOfBytes, &lResult, NULL))
		{
			RETAILMSG(1,(_T("++CSR_Write++ FAIL \r\n")));
		}
	}

	//RETAILMSG(1, (TEXT("CSR_Write,len:%d,result:%d\r\n"), NumberOfBytes,lResult));
#if 0 //for test,f.w.lin
    RETAILMSG(1, (TEXT("CSR_Write(%d):"), lResult));
    for( ULONG i=0; i < lResult; i++ )
        RETAILMSG(1, (TEXT("%02x "), pSourceBytes[i]));

    RETAILMSG(1, (TEXT("\r\n")));
#endif
	return lResult;
}

BOOL WINAPI CSR_DllEntry(HANDLE hModule, DWORD  ul_reason_for_call, 
					   LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
		RETAILMSG(1,(_T("\r\n CSR_DllEntry DLL_PROCESS_ATTACH \r\n")));
		break;
    }
    return TRUE;
}

HANDLE WINAPI CSR_Init(ULONG Identifier)
{
    /* creat event */
    g_hControlEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
    if (!g_hControlEvent)
    {
        RETAILMSG(1, (_T("CSR_Init: Create control Event failed, err(%d)\r\n"), GetLastError() ));
        goto INIT_FAIL;
    }
    g_hKillThreadEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
    if (!g_hKillThreadEvent)
    {
        RETAILMSG(1, (_T("CSR_Init: Create Kill thread Event failed, err(%d)\r\n"), GetLastError() ));
        goto INIT_FAIL;
    }
    
    /* creat thread */
    g_hControlThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CSRControlThread, NULL, 0, NULL);
    if ( g_hControlThread == NULL)
    {
        RETAILMSG(1, (_T("CSR_Init!  Failed to create CSRControlThread.\r\n")));
        goto INIT_FAIL;
    }

	v_pIOPregs = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE );

    RETAILMSG(1, (TEXT("CSR_Init: ok!\r\n")));
    return (HANDLE)TRUE;

INIT_FAIL:
    if (g_hControlEvent) CloseHandle(g_hControlEvent);
    if (g_hControlThread) CloseHandle(g_hControlThread);
    if (g_hKillThreadEvent) CloseHandle(g_hKillThreadEvent);

    g_hKillThread = FALSE;
    g_hControlEvent = NULL;
    g_hControlThread = NULL;
    g_hKillThreadEvent = NULL;

    RETAILMSG(1, (TEXT("CSR_Init: failed\r\n")));
    return (HANDLE)FALSE;
}

BOOL WINAPI CSR_Deinit(void)
{
	RETAILMSG(1, (TEXT("CSR_Deinit\r\n")));
    g_hKillThread = TRUE;
    SetEvent(g_hControlEvent);
    WaitForSingleObject(g_hKillThreadEvent, INFINITE);
    Sleep(10);

    /* close handle */
    CloseHandle(g_hControlThread);
    if (g_hControlEvent) CloseHandle(g_hControlEvent);
    if (g_hControlThread) CloseHandle(g_hControlThread);

    g_hKillThread = FALSE;
    g_hControlEvent = NULL;
    g_hControlThread = NULL;
    g_hKillThreadEvent = NULL;

    VirtualFree((PVOID)v_pIOPregs, 0, MEM_RELEASE);

    return TRUE;
}

ULONG WINAPI CSR_Seek(HANDLE pHead, LONG Position, DWORD Type)
{
    return(ULONG)-1;
}


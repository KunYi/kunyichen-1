// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-2000 Microsoft Corporation.  All rights reserved.
//
// -----------------------------------------------------------------------------
#include "wavemain.h"

 //[david.modify] 2008-06-19 19:25
#include <dbgmsg_david.h>


#define DBGMSG_ON	0

int	g_NeedtoSleep;

BOOL CALLBACK DllMain(HANDLE hDLL,
                      DWORD dwReason,
                      LPVOID lpvReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH :
            DEBUGREGISTER((HINSTANCE)hDLL);
            DisableThreadLibraryCalls((HMODULE) hDLL);
            break;

        case DLL_PROCESS_DETACH :
            break;

        case DLL_THREAD_DETACH :
            break;

        case DLL_THREAD_ATTACH :
            break;

        default :
            break;
    }
    return(TRUE);
}


// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @topic   WAV Device Interface | Implements the WAVEDEV.DLL device
//          interface. These functions are required for the device to
//          be loaded by DEVICE.EXE.
//
// @xref                          <nl>
//          <f WAV_Init>,         <nl>
//          <f WAV_Deinit>,       <nl>
//          <f WAV_Open>,         <nl>
//          <f WAV_Close>,        <nl>
//          <f WAV_Read>,         <nl>
//          <f WAV_Write>,        <nl>
//          <f WAV_Seek>,         <nl>
//          <f WAV_PowerUp>,      <nl>
//          <f WAV_PowerDown>,    <nl>
//          <f WAV_IOControl>     <nl>
//
// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @topic   Designing a Waveform Audio Driver |
//          A waveform audio driver is responsible for processing messages
//          from the Wave API Manager (WAVEAPI.DLL) to playback and record
//          waveform audio. Waveform audio drivers are implemented as
//          dynamic link libraries that are loaded by DEVICE.EXE The
//          default waveform audio driver is named WAVEDEV.DLL (see figure).
//          The messages passed to the audio driver are similar to those
//          passed to a user-mode Windows NT audio driver (such as mmdrv.dll).
//
//          <bmp blk1_bmp>
//
//          Like all device drivers loaded by DEVICE.EXE, the waveform
//          audio driver must export the standard device functions,
//          XXX_Init, XXX_Deinit, XXX_IoControl, etc (see
//          <t WAV Device Interface>). The Waveform Audio Drivers
//          have a device prefix of "WAV".
//
//          Driver loading and unloading is handled by DEVICE.EXE and
//          WAVEAPI.DLL. Calls are made to <f WAV_Init> and <f WAV_Deinit>.
//          When the driver is opened by WAVEAPI.DLL calls are made to
//          <f WAV_Open> and <f WAV_Close>.  On system power up and power down
//          calls are made to <f WAV_PowerUp> and <f WAV_PowerDown>. All
//          other communication between WAVEAPI.DLL and WAVEDEV.DLL is
//          done by calls to <f WAV_IOControl>. The other WAV_xxx functions
//          are not used.
//
// @xref                                          <nl>
//          <t Designing a Waveform Audio PDD>    <nl>
//          <t WAV Device Interface>              <nl>
//          <t Wave Input Driver Messages>        <nl>
//          <t Wave Output Driver Messages>       <nl>
//
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Init | Device initialization routine
//
//  @parm   DWORD | dwInfo | info passed to RegisterDevice
//
//  @rdesc  Returns a DWORD which will be passed to Open & Deinit or NULL if
//          unable to initialize the device.
//
// -----------------------------------------------------------------------------
DWORD WAV_Init(DWORD Index)
{
	    return((DWORD)HardwareContext::CreateHWContext(Index));
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Deinit | Device deinitialization routine
//
//  @parm   DWORD | dwData | value returned from WAV_Init call
//
//  @rdesc  Returns TRUE for success, FALSE for failure.
//
// -----------------------------------------------------------------------------
BOOL WAV_Deinit(DWORD dwData)
{
    return(g_pHWContext->Deinit());
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Open    | Device open routine
//
//  @parm   DWORD | dwData      | Value returned from WAV_Init call (ignored)
//
//  @parm   DWORD | dwAccess    | Requested access (combination of GENERIC_READ
//                                and GENERIC_WRITE) (ignored)
//
//  @parm   DWORD | dwShareMode | Requested share mode (combination of
//                                FILE_SHARE_READ and FILE_SHARE_WRITE) (ignored)
//
//  @rdesc  Returns a DWORD which will be passed to Read, Write, etc or NULL if
//          unable to open device.
//
// -----------------------------------------------------------------------------
DWORD WAV_Open( DWORD dwData,
              DWORD dwAccess,
              DWORD dwShareMode)
{
    return(4);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   BOOL | WAV_Close | Device close routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call
//
//  @rdesc  Returns TRUE for success, FALSE for failure
//
// -----------------------------------------------------------------------------
BOOL WAV_Close(DWORD dwData)
{
    return(TRUE);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Read | Device read routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   LPVOID | pBuf | Buffer to receive data (ignored)
//
//  @parm   DWORD | len | Maximum length to read (ignored)
//
//  @rdesc  Returns 0 always. WAV_Read should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
DWORD WAV_Read(DWORD dwData,
               LPVOID pBuf,
               DWORD Len)
{
    // Return length read
    return(0);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Write | Device write routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   LPCVOID | pBuf | Buffer containing data (ignored)
//
//  @parm   DWORD | len | Maximum length to write (ignored)
//
//  @rdesc  Returns 0 always. WAV_Write should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
DWORD WAV_Write(DWORD dwData,
                LPCVOID pBuf,
                DWORD Len)
{
    // return number of bytes written (or -1 for error)
    return(0);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Seek | Device seek routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   long | pos | Position to seek to (relative to type) (ignored)
//
//  @parm   DWORD | type | FILE_BEGIN, FILE_CURRENT, or FILE_END (ignored)
//
//  @rdesc  Returns -1 always. WAV_Seek should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
DWORD WAV_Seek(DWORD dwData,
               long pos,
               DWORD type)
{
    // return an error
    return((DWORD)-1);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   void | WAV_PowerUp | Device powerup routine
//
//  @comm   Called to restore device from suspend mode.  Cannot call any
//          routines aside from those in the dll in this call.
//
// -----------------------------------------------------------------------------
VOID WAV_PowerUp(VOID)
{
#if (BSP_TYPE == BSP_SMDK2443)
    g_pHWContext->PowerUp();
#elif (BSP_TYPE == BSP_SMDK2450)
#endif

    return;
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   void | WAV_PowerDown | Device powerdown routine
//
//  @comm   Called to suspend device.  Cannot call any routines aside from
//          those in the dll in this call.
//
// -----------------------------------------------------------------------------
VOID WAV_PowerDown(VOID)
{
#if (BSP_TYPE == BSP_SMDK2443)
    g_pHWContext->PowerDown();
#elif (BSP_TYPE == BSP_SMDK2450)
#endif

    return;
}


 //[david.modify] 2008-06-21 12:22
extern  UINT32 g_u32ArrWinceVol[];//={0x0, 0x33333333, 0x66666666,0x99999999,0xcccccccc,0xffffffff};
extern UINT32 g_u32ArrWinceVol2[];

BOOL HandleWaveMessage(PMMDRV_MESSAGE_PARAMS pParams, DWORD *pdwResult)
{
    //  set the error code to be no error first
    SetLastError(MMSYSERR_NOERROR);

    UINT uMsg = pParams->uMsg;
    UINT uDeviceId = pParams->uDeviceId;
    DWORD dwParam1 = pParams->dwParam1;
    DWORD dwParam2 = pParams->dwParam2;
    DWORD dwUser   = pParams->dwUser;
    StreamContext *pStreamContext = (StreamContext *)dwUser;

    DWORD dwRet;
	DWORD i=0; //[david.modify] 2008-06-21 13:05

	RETAILMSG(DBGMSG_ON, (TEXT("HandleWaveMessage(uMsg:%d)->"), uMsg));
	
    g_pHWContext->Lock();
    switch (uMsg)
    {
    case WODM_GETNUMDEVS:
        {
            dwRet = g_pHWContext->GetNumOutputDevices();

            break;
        }

    case WIDM_GETNUMDEVS:
        {
            dwRet = g_pHWContext->GetNumInputDevices();
            break;
        }

    case WODM_GETDEVCAPS:
        {
            DeviceContext *pDeviceContext;
            UINT NumDevs = g_pHWContext->GetNumOutputDevices();


            if (pStreamContext)
            {
                pDeviceContext=pStreamContext->GetDeviceContext();
            }
            else
            {
                pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            }

            dwRet = pDeviceContext->GetDevCaps((PVOID)dwParam1,dwParam2);
            break;
        }


    case WIDM_GETDEVCAPS:
        {
            DeviceContext *pDeviceContext;
            UINT NumDevs = g_pHWContext->GetNumInputDevices();

            if (pStreamContext)
            {
                pDeviceContext=pStreamContext->GetDeviceContext();
            }
            else
            {
                pDeviceContext = g_pHWContext->GetInputDeviceContext(uDeviceId);
            }

            dwRet = pDeviceContext->GetDevCaps((PVOID)dwParam1,dwParam2);
            break;
        }

    case WODM_GETEXTDEVCAPS:
        {
            DeviceContext *pDeviceContext;
            UINT NumDevs = g_pHWContext->GetNumOutputDevices();

            if (pStreamContext)
            {
                pDeviceContext=pStreamContext->GetDeviceContext();
            }
            else
            {
                pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            }

            dwRet = pDeviceContext->GetExtDevCaps((PVOID)dwParam1,dwParam2);
            break;
        }
    case WODM_OPEN:
        {
            // DEBUGMSG(1, (TEXT("WODM_OPEN\r\n")));
            RETAILMSG(DBGMSG_ON, (TEXT("WODM_OPEN\r\n")));

            DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            dwRet = pDeviceContext->OpenStream((LPWAVEOPENDESC)dwParam1, dwParam2, (StreamContext **)dwUser);
            break;
        }

    case WIDM_OPEN:
        {
            // DEBUGMSG(1, (TEXT("WIDM_OPEN\r\n")));
            RETAILMSG(DBGMSG_ON, (TEXT("WIDM_OPEN\r\n")));           

            DeviceContext *pDeviceContext = g_pHWContext->GetInputDeviceContext(uDeviceId);
            dwRet = pDeviceContext->OpenStream((LPWAVEOPENDESC)dwParam1, dwParam2, (StreamContext **)dwUser);
            break;
        }

    case WODM_CLOSE:
    case WIDM_CLOSE:
        {
            // DEBUGMSG(1, (TEXT("WIDM_CLOSE/WODM_CLOSE\r\n")));

            dwRet = pStreamContext->Close();

            // Release stream context here, rather than inside StreamContext::Close, so that if someone
            // (like CMidiStream) has subclassed Close there's no chance that the object will get released
            // out from under them.
            if (dwRet==MMSYSERR_NOERROR)
            {
                pStreamContext->Release();
            }
            break;
        }

    case WODM_RESTART:
    case WIDM_START:
        {
			dwRet = pStreamContext->Run();
			
            break;
        }

    case WODM_PAUSE:
    case WIDM_STOP:
        {
            dwRet = pStreamContext->Stop();
			
            break;
        }

    case WODM_GETPOS:
    case WIDM_GETPOS:
        {
		RETAILMSG(DBGMSG_ON, (TEXT("WI/ODM_GETPOS\r\n")));
            dwRet = pStreamContext->GetPos((PMMTIME)dwParam1);
            break;
        }

    case WODM_RESET:
    case WIDM_RESET:
        {
            dwRet = pStreamContext->Reset();
            break;
        }

    case WODM_WRITE:
    case WIDM_ADDBUFFER:
        {
            // DEBUGMSG(1, (TEXT("WODM_WRITE/WIDM_ADDBUFFER, Buffer=0x%x\r\n"),dwParam1);
		//RETAILMSG(1, (TEXT("+WODM_WRITE/WIDM_ADDBUFFER, Buffer=0x%x\r\n"),((WAVEHDR *)dwParam1)->dwBufferLength));
		if (((WAVEHDR *)dwParam1)->dwBufferLength <= 0x500)
			g_NeedtoSleep = TRUE;
		else
			g_NeedtoSleep = FALSE;
            dwRet = pStreamContext->QueueBuffer((LPWAVEHDR)dwParam1);
            break;
        }

    case WODM_GETVOLUME:
        {
		RETAILMSG(DBGMSG_ON, (TEXT("WODM_GETVOLUME\r\n")));
            PULONG pdwGain = (PULONG)dwParam1;
            UINT NumDevs = g_pHWContext->GetNumOutputDevices();

            if (pStreamContext)
            {
                *pdwGain = pStreamContext->GetGain();
				 //[david.modify] 2008-06-19 19:26
		DPNOK(*pdwGain);
            }
            else
            {
                DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
                *pdwGain = pDeviceContext->GetGain();
				 //[david.modify] 2008-06-19 19:26
		DPNOK(*pdwGain);				
            }
	 //[david.modify] 2008-06-19 19:26
	 //==================================     
		DPNOK(*pdwGain);
		for(i=0;i<6;i++) 
		{
			if((UINT32)(*pdwGain)<=g_u32ArrWinceVol2[i]) {
				*pdwGain= g_u32ArrWinceVol[i];
				break;
			}

		}
		DPNOK(*pdwGain);
	 //==================================
			
            dwRet = MMSYSERR_NOERROR;
            break;
        }

    case WODM_SETVOLUME:
        {
		//RETAILMSG(1, (TEXT("WODM_SETVOLUME\r\n")));
            UINT NumDevs = g_pHWContext->GetNumOutputDevices();
            LONG dwGain = dwParam1;
            if (pStreamContext)
            {
				 //[david.modify] 2008-06-19 19:26
		DPNOK(dwGain);            
                dwRet = pStreamContext->SetGain(dwGain);
            }
            else
            {
	 //[david.modify] 2008-06-19 19:26
	 //==================================      
                DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
		DPNOK(dwGain);
		for(i=0;i<6;i++) 
		{
			if((UINT32)dwGain<=g_u32ArrWinceVol[i]) {
				dwGain= g_u32ArrWinceVol2[i];
				break;
			}

		}
		DPNOK(dwGain);
                dwRet = pDeviceContext->SetGain(dwGain);
	 //==================================

            }
            break;
        }

    case WODM_BREAKLOOP:
        {
            dwRet = pStreamContext->BreakLoop();
            break;
        }

    case WODM_SETPLAYBACKRATE:
        {
            WaveStreamContext *pWaveStream = (WaveStreamContext *)dwUser;
            dwRet = pWaveStream->SetRate(dwParam1);
            break;
        }

    case WODM_GETPLAYBACKRATE:
        {
            WaveStreamContext *pWaveStream = (WaveStreamContext *)dwUser;
            dwRet = pWaveStream->GetRate((DWORD *)dwParam1);
            break;
        }

    case MM_WOM_SETSECONDARYGAINCLASS:
        {
            dwRet = pStreamContext->SetSecondaryGainClass(dwParam1);
            break;
        }

    case MM_WOM_SETSECONDARYGAINLIMIT:
        {
            DeviceContext *pDeviceContext;
            if (pStreamContext)
            {
                pDeviceContext = pStreamContext->GetDeviceContext();
            }
            else
            {
                pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            }
            dwRet = pDeviceContext->SetSecondaryGainLimit(dwParam1,dwParam2);
            break;
        }

    case MM_MOM_MIDIMESSAGE:
        {
            CMidiStream *pMidiStream = (CMidiStream *)dwUser;
            dwRet = pMidiStream->MidiMessage(dwParam1);
            break;
        }
    case WPDM_PRIVATE_WRITE_CODEC:
        dwRet = g_pHWContext->MsgWriteCodec(dwParam1,dwParam2 );
        break;

    case WPDM_PRIVATE_READ_CODEC:
        dwRet = g_pHWContext->MsgReadCodec( dwParam1,dwParam2 );
        break;

 //[david.modify] 2008-06-19 18:54
 //==============================
 	case WODM_GET_AUDIOLEVEL:
		{
//			dwRet = g_pHWContext->GetAudioVolLevel();
			break;
		}
	case WODM_SET_AUDIOLEVEL:
		{
//			LONG dwAudioLevel = dwParam1;
//			dwRet = g_pHWContext->SetAudioVolLevel(dwAudioLevel);
			break;
		}
	case WODM_GET_MAX_AUDIOLEVEL:
		{
//			dwRet = g_pHWContext->GetMaxAudioVolLevel();
			break;
		}
	case WODM_SET_MAX_AUDIOLEVEL:
		{
			int nMaxLevel = dwParam1;
//			dwRet = g_pHWContext->SetMaxAudioVolLevel(nMaxLevel);
			break;
		}	
//===========================================	

		

	case WPDM_PRIVATE_CONFIG_HW_EQ:		//To enable/disable HW EQ
	case WPDM_PRIVATE_ENABLE_3D:		//To enable 3D
	case WPDM_PRIVATE_DISABLE_3D:		//To disable 3D
	case WPDM_PRIVATE_SET_3D_PARAMETER:		//to set 3D parameter
	case WPDM_PRIVATE_CONFIG_PSEUDO:	//To enable/disable Pseudo stereo
	case WPDM_PRIVATE_CONFIG_AVC:		//To enable/disable Auto Volume Control of inpust stream
		dwRet = g_pHWContext->RT_AudioMessage(uMsg,dwParam1,dwParam2);
		break;

    case WODM_GETPITCH:
    case WODM_SETPITCH:
    case WODM_PREPARE:
    case WODM_UNPREPARE:
    case WIDM_PREPARE:
    case WIDM_UNPREPARE:
        default:
        dwRet  = MMSYSERR_NOTSUPPORTED;
    }
    g_pHWContext->Unlock();

    // Pass the return code back via pBufOut
    if (pdwResult)
    {
        *pdwResult = dwRet;
    }

    return(TRUE);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   BOOL | WAV_IOControl | Device IO control routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call
//
//  @parm   DWORD | dwCode |
//          IO control code for the function to be performed. WAV_IOControl only
//          supports one IOCTL value (IOCTL_WAV_MESSAGE)
//
//  @parm   PBYTE | pBufIn |
//          Pointer to the input parameter structure (<t MMDRV_MESSAGE_PARAMS>).
//
//  @parm   DWORD | dwLenIn |
//          Size in bytes of input parameter structure (sizeof(<t MMDRV_MESSAGE_PARAMS>)).
//
//  @parm   PBYTE | pBufOut | Pointer to the return value (DWORD).
//
//  @parm   DWORD | dwLenOut | Size of the return value variable (sizeof(DWORD)).
//
//  @parm   PDWORD | pdwActualOut | Unused
//
//  @rdesc  Returns TRUE for success, FALSE for failure
//
//  @xref   <t Wave Input Driver Messages> (WIDM_XXX) <nl>
//          <t Wave Output Driver Messages> (WODM_XXX)
//
// -----------------------------------------------------------------------------
BOOL WAV_IOControl(DWORD  dwOpenData,
                   DWORD  dwCode,
                   PBYTE  pBufIn,
                   DWORD  dwLenIn,
                   PBYTE  pBufOut,
                   DWORD  dwLenOut,
                   PDWORD pdwActualOut)
{

    _try
    {
        switch (dwCode)
        {
        case IOCTL_MIX_MESSAGE:
                        return HandleMixerMessage((PMMDRV_MESSAGE_PARAMS)pBufIn, (DWORD *)pBufOut);

        case IOCTL_WAV_MESSAGE:
            return HandleWaveMessage((PMMDRV_MESSAGE_PARAMS)pBufIn, (DWORD *)pBufOut);
        // Power management functions.
        case IOCTL_POWER_CAPABILITIES:
        case IOCTL_POWER_SET:
        case IOCTL_POWER_GET:
        case IOCTL_POWER_QUERY:
        	return g_pHWContext->IOControl
                                (dwOpenData, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
                                
        }
    }
    _except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        RETAILMSG(1, (TEXT("EXCEPTION IN WAV_IOControl!!!!\r\n")));
        SetLastError(E_FAIL);
    }

    return(FALSE);
}

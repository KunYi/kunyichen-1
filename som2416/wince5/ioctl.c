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
//------------------------------------------------------------------------------
//
//  File: ioctl.c           
//
//  This file implements the OEM's IO Control (IOCTL) functions and declares
//  global variables used by the IOCTL component.
//
#include <Storemgr.h>
#include <bldver.h>
#include <bsp.h>
 //[david.modify] 2008-06-02 18:56
 #include <halio_david.h>

#define pBSPArgs					((BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START)
//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlPlatformType/OEM    
//
//  Platform Type/OEM
//
//[david.modify] 2008-06-02 17:01
//==============================

#if 0
 //[david.modify] 2008-06-18 13:57
//LPCWSTR g_oalIoCtlPlatformType = L"SMDK2450 Board";
LPCWSTR g_oalIoCtlPlatformType = L"LD2416";
 //[david.modify] 2008-06-18 13:57
//LPCWSTR g_oalIoCtlPlatformOEM  = L"Leader OEM Board"; 
 //[david.modify] 2008-10-29 10:39
 // 出给MAXICOM这个客户
LPCWSTR g_oalIoCtlPlatformOEM  = L"MAXCOM 353"; 
//LPCWSTR g_oalIoCtlPlatformOEM  = L"Samsung Electronics";
#else
//[david.modify] 2008-11-10 18:16
// LCP客户可能采用如下字串加密其应用
LPCWSTR g_oalIoCtlPlatformType = L"AT4X0A";
LPCWSTR g_oalIoCtlPlatformOEM  = L"Leader OEM Board";
#endif
//==============================
//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlProcessorVendor/Name/Core
//
//  Processor information
//
 //[david.modify] 2008-06-02 19:45
#if 1 
LPCWSTR g_oalIoCtlProcessorVendor = L"Samsung Electronics";
LPCWSTR g_oalIoCtlProcessorName   = L"S3C2450";
LPCWSTR g_oalIoCtlProcessorCore   = L"ARM920T";
#else
#define ATLAS_IOCTL_PROCESSOR_VENDOR              (L"Centrality")
#define ATLAS_IOCTL_PROCESSOR_NAME                (L"AT4X0A")
#define ATLAS_IOCTL_PROCESSOR_CORE                (L"ARM926T")
LPCWSTR g_oalIoCtlProcessorVendor = ATLAS_IOCTL_PROCESSOR_VENDOR;
LPCWSTR g_oalIoCtlProcessorName   = ATLAS_IOCTL_PROCESSOR_NAME;
LPCWSTR g_oalIoCtlProcessorCore   = ATLAS_IOCTL_PROCESSOR_CORE;
#endif

const UINT8 g_oalIoCtlVendorId[6] = { 0x00, 0x50, 0xBF, 0x19, 0x77, 0xE0 };
UINT8 g_oalIoCtlFlashId[16] = {0, };

//[david.modify] 2007-09-11 16:51
//======================================
//#include <dbgmsg_david.h>
#define MYLOGO TEXT("DAVID") 
#define DPNOK(x) \
    NKDbgPrintfW(TEXT("[%s][%s]: line %d OK=%x\r\n"),MYLOGO,TEXT(__FUNCTION__),__LINE__,(x))   

#define OEM_SW_MAJOR_VER 2
#define OEM_SW_MINOR_VER 0

 //[david.modify] 2007-12-19 10:15
 // CTA NEED: 硬件版本:KINGZOO VER C0 软件版本: 2.00
#define OEM_HW_MAJOR_VER 3
#define OEM_HW_MINOR_VER 0
//const WCHAR strCompileTime[]= TEXT(__DATE__);


//[david.modify] 2008-11-01 10:34
//==============================================
/* 
 35C16L54P04G756A
 
35 LCD屏大小（35：3.5寸，43：4.3寸）
C16 CPU类型 （16：2416，50：2450，51：2451）
L54 LCD屏连接器PIN数（54：54PIN连接器，还应有60，40）
P04  PCB层数（0表示普通板，1表示激光板，4表示4层）
G756 GPS芯片类型（756：GLONAV 7560，454：GLONAV 4540，NX3：NEMORIX NX3）

V去掉，改为A，B，C,D.....
其中：A表示3.5
     B表示4.3
     C表示核心板
     D表示采用核心板的4.3

*/
//#define OEM_HW_VER_STR L"35C16L54P06G454_A01"
#define OEM_HW_VER_STR L"35C16L54P04G756_A03"

/*
         43JL-162-PBC-128D-64S-BYD43NU00001-147-1018111-081022.img
         43JL机型 35K/43C/35M/43JSL等
         162 三位硬件版本号
         PBC 三位客户代号 NEX nexstar， PLC等等
         128D 128DDR内存， 64S 64MB sdram
         64S 64MB slc，1MLC 1GB MLC       
         BYD43111，BYD屏供应商代号，YAS等等 43/35屏尺寸，NU00001七位屏版本号
，区分同一厂商不同型号(最好用屏的真是产品编码)
147软件版本号，三星平台可以长一些
         10198111 七位订单号
     081022  出版本年月日
*/


#define OEM_SW_VER_STR L"35JSC40064D21GM"
//===============================================
typedef struct
{
	const WCHAR strCPUType[32];
	const WCHAR strOSType[32];
	const WCHAR strCompileTime[32];	
	UINT32 u32Frequency;
	UINT32 u32BusWidth;
	UINT32 u32RomSize;
	UINT32 u32RAMSize;
	UINT32 u32HWVer;	// 主版本在高16bit;
	UINT32 u32SWVer;	// 主版本在高16bit;
 //[david.modify] 2008-10-07 14:10
 //增加的选 项
 //=============================
	UINT32 u32LCDType;

 //[david.modify] 2008-11-01 10:31
	const WCHAR strHWVER[32];
	const WCHAR strSWVER[32]; 
 
 //=============================	
}stOEMInformation;

stOEMInformation g_stOEMInformation={
L"Samsung",
L"Wince 5.0",
TEXT(__DATE__ )L"-" TEXT(__TIME__ ),
400,
16,
1024,
64,
(OEM_HW_MAJOR_VER<<16)|(OEM_HW_MINOR_VER&0xFFFF), //v1.0
(OEM_SW_MAJOR_VER<<16)|(OEM_SW_MINOR_VER&0xFFFF), //v1.0
0,
OEM_HW_VER_STR,
OEM_SW_VER_STR
};
//======================================
 //[david. end] 2007-09-11 16:55


//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlInstructionSet
//
//  Processor instruction set identifier
//
UINT32 g_oalIoCtlInstructionSet = 0; //IOCTL_PROCESSOR_INSTRUCTION_SET;
extern UINT32 g_oalIoCtlClockSpeed;// = 0;//IOCTL_PROCESSOR_CLOCK_SPEED;



BOOL OALIoCtlHalQueryDisplaySettings (UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    DWORD dwErr = 0;

	RETAILMSG(0, (TEXT("In OALIoCtlHalQueryDisplaySettings^^^^^\r\n")));

    if (lpBytesReturned) {
        *lpBytesReturned = 0;
    }

    if (!lpOutBuf) {
        dwErr = ERROR_INVALID_PARAMETER;
    } else if (sizeof(DWORD)*3 > nOutBufSize) {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } else {
        // Check the boot arg structure for the default display settings.
        __try {

            ((PDWORD)lpOutBuf)[0] = (DWORD)320;
            ((PDWORD)lpOutBuf)[1] = (DWORD)240;
            ((PDWORD)lpOutBuf)[2] = (DWORD)16;

            if (lpBytesReturned) {
                *lpBytesReturned = sizeof (DWORD) * 3;
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}

//------------------------------------------------------------------------------
//
//  define PSII control
//

#define __PSII_DEFINED__

CRITICAL_SECTION csPocketStoreVFL;

#if defined(__PSII_DEFINED__)
UINT32 PSII_HALWrapper(VOID *pPacket, VOID *pInOutBuf, UINT32 *pResult);
#endif //#if defined(__PSII_DEFINED__)

#define IOCTL_POCKETSTORE_CMD	CTL_CODE(FILE_DEVICE_HAL, 4070, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_POCKETSTOREII_CMD	CTL_CODE(FILE_DEVICE_HAL, 4080, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL OALIoCtlPostInit(
	UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
	UINT32 outSize, UINT32 *pOutSize)
{
	RETAILMSG(1,(TEXT("[OEMIO:INF]  + IOCTL_HAL_POSTINIT\r\n")));
	InitializeCriticalSection(&csPocketStoreVFL);
	RETAILMSG(1,(TEXT("[OEMIO:INF]  - IOCTL_HAL_POSTINIT\r\n")));
	
	return TRUE;
}

BOOL OALIoCtlPocketStoreCMD(
	UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
	UINT32 outSize, UINT32 *pOutSize)
{
	BOOL bResult;

	EnterCriticalSection(&csPocketStoreVFL);
	bResult = PSII_HALWrapper(pInpBuffer, pOutBuffer, pOutSize);
	LeaveCriticalSection(&csPocketStoreVFL);

	if (bResult == FALSE)
	{
		RETAILMSG(1,(TEXT("[OEMIO:INF]  * IOCTL_POCKETSTOREII_CMD Failed\r\n")));
		return FALSE;
	}
	return TRUE;
}



 //[david.modify] 2008-06-02 11:56
 //=======================
 BOOL OALIoCtlHalGetHiveCleanFlag(UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned )
{
	// volatile S3C2440X_IOPORT_REG *pOalPortRegs;
	BOOL HiveCleanFlag = FALSE;

	/*	
    pOalPortRegs = OALPAtoVA(S3C2440X_BASE_REG_PA_IOPORT, FALSE);
	if(INREG32(&pOalPortRegs->GPFDAT) & (1<<4))		//Power Button
	{
		RETAILMSG(1, (TEXT("[OALIoCtlHalGetHiveCleanFlag] GPFDAT - 0x%x\r\n"), INREG32(&pOalPortRegs->GPFDAT)));
		HiveCleanFlag = TRUE;
	}
	*/

	RETAILMSG(1, (TEXT("[OALIoCtlHalGetHiveCleanFlag] \n\r")));

	if ((pBSPArgs->uninit_misc.dwResetCause == PARTIAL_FACTORY_RESET_FLAG) ||
		(pBSPArgs->uninit_misc.dwEootFlag == PARTIAL_FACTORY_LAUNCH) ||
	     	(pBSPArgs->uninit_misc.dwResetCause == FACTORY_RESET_FLAG) ||
	     	(pBSPArgs->uninit_misc.dwEootFlag == FACTORY_LAUNCH))
	{
		RETAILMSG(1, (TEXT("[OALIoCtlHalGetHiveCleanFlag] clean registry\n\r")));
		HiveCleanFlag = TRUE;
	}
	
	if (!lpInBuf || (nInBufSize != sizeof(DWORD)) || !lpOutBuf || (nOutBufSize != sizeof(BOOL))) 
	{
		NKSetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	} else 
	{
		DWORD *pdwFlags = (DWORD*)lpInBuf;
		BOOL  *pfClean  = (BOOL*)lpOutBuf;
		if (*pdwFlags == HIVECLEANFLAG_SYSTEM && pBSPArgs->uninit_misc.bCleanSystemHive) {
			if(HiveCleanFlag){
				RETAILMSG(1, (TEXT("OEM: Cleaning system hive\r\n")));
				*pfClean = TRUE;
				pBSPArgs->uninit_misc.bCleanSystemHive = FALSE;
			}
			else{
				RETAILMSG(1, (TEXT("OEM: No Cleaning system hive\r\n")));
				*pfClean = FALSE;
			}
		} else if (*pdwFlags == HIVECLEANFLAG_USERS && pBSPArgs->uninit_misc.bCleanUserHive) {
			if(HiveCleanFlag){
				RETAILMSG(1, (TEXT("OEM: Cleaning user profiles\r\n")));
				*pfClean = TRUE;
				pBSPArgs->uninit_misc.bCleanUserHive = FALSE;
			}
			else{
				RETAILMSG(1, (TEXT("OEM: NO Cleaning user profiles\r\n")));
				*pfClean = FALSE;
			}
		}
	}
	
	return TRUE;
}




BOOL OALIoCtlHalQueryFormatPartition(UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned )
{
	// S3C2440X_IOPORT_REG *pOalPortRegs;
	BOOL g_fFormatRootFS = FALSE;
	BOOL g_fFormatBootableFS = FALSE;
	STORAGECONTEXT* pContext = (STORAGECONTEXT*)lpInBuf;

	/*
    pOalPortRegs = OALPAtoVA(S3C2440X_BASE_REG_PA_IOPORT, FALSE);
	if(INREG32(&pOalPortRegs->GPFDAT) & (1<<4))	// Power Button
 	{
		RETAILMSG(1, (TEXT("[OALIoCtlHalQueryFormatPartition] GPFDAT - 0x%x\r\n"), INREG32(&pOalPortRegs->GPFDAT)));
		g_fFormatRootFS = TRUE;
		g_fFormatBootableFS = TRUE;
	}
	*/

	RETAILMSG(1, (TEXT("[OALIoCtlHalQueryFormatPartition] \n\r")));

	if ((pBSPArgs->uninit_misc.dwResetCause == PARTIAL_FACTORY_RESET_FLAG) ||
		(pBSPArgs->uninit_misc.dwEootFlag == PARTIAL_FACTORY_LAUNCH) ||
	     	(pBSPArgs->uninit_misc.dwResetCause == FACTORY_RESET_FLAG) ||
	     	(pBSPArgs->uninit_misc.dwEootFlag == FACTORY_LAUNCH))
	{
		g_fFormatRootFS = TRUE;
		g_fFormatBootableFS = TRUE;
		RETAILMSG(1, (TEXT("[OALIoCtlHalQueryFormatPartition] enable format !\n\r")));
	}
	
	// validate parameters
	if (sizeof(STORAGECONTEXT) != nInBufSize || !lpInBuf ||
		sizeof(BOOL) != nOutBufSize || !lpOutBuf) {
		NKSetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	// by default, do not format any partitions
	*(BOOL*)lpOutBuf = FALSE; 
//
// format the root file system? (MountAsRoot=dword:1)
//
	if (g_fFormatRootFS && (AFS_FLAG_ROOTFS & pContext->dwFlags)) {
		*(BOOL*)lpOutBuf = TRUE;
		RETAILMSG(1, (TEXT("format the root file system\n\r")));
	}
//
// format the bootable file system? (MountAsBootable=dword:1)
//
	if (g_fFormatBootableFS && (AFS_FLAG_BOOTABLE & pContext->dwFlags)) {
		*(BOOL*)lpOutBuf = TRUE; 
		RETAILMSG(1, (TEXT("format the bootable file system\n\r")));
	}
	return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetUUID
//
//  Implements the IOCTL_HAL_GET_UUID handler. This function fills in a 
//  GUID structure.
//
BOOL OALIoCtlHalGetUUID ( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    BOOL rc = FALSE;
    const GUID *pUuid;
    LPCSTR pDeviceId;
//   char temp[17];
	
    // Check buffer size
    if (pOutSize != NULL) *pOutSize = sizeof(GUID);
    if (pOutBuffer == NULL || outSize < sizeof(GUID)) {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetUUID: Buffer too small\r\n"));
        goto cleanUp;
    }

    // Does BSP specific UUID exist?
    pUuid = (const GUID *)OALArgsQuery(OAL_ARGS_QUERY_UUID);

    // try using device id if BSP specific UUID doesnt' exist
    if (pUuid == NULL) {
        pDeviceId = OALArgsQuery(OAL_ARGS_QUERY_DEVID);
        if (pDeviceId == NULL) {
            NKSetLastError(ERROR_NOT_SUPPORTED);
            OALMSG(OAL_WARN, (
                L"ERROR: OALIoCtlHalGetUUID: Device doesn't support UUID\r\n"
            ));
            goto cleanUp;
        }            

        // Use the last sizeof(GUID) bytes of the device id as GUID
         //[david.modify] 2008-07-11 11:11
         //=================
//      pUuid = (const GUID *)(pDeviceId + strlen(pDeviceId) - sizeof(GUID));
      pUuid = (const GUID *)(pDeviceId + 16- sizeof(GUID));
         //=================		
    }

    // Return good value
    memcpy(pOutBuffer, pUuid, sizeof(GUID));
//    memcpy(pOutBuffer, "Leader0703090009", sizeof(GUID));

/*	 
	memset(temp,0,sizeof(temp));
	memcpy(temp,pOutBuffer,sizeof(GUID));
	if(temp[0])
	{
 		RETAILMSG(1,(TEXT("\r\n\r\n**********UUID:%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c*************\r\n\r\n"),
			temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7],
			temp[8],temp[9],temp[10],temp[11],temp[12],temp[13],temp[14],temp[15]));
	}
	else
	{
		RETAILMSG(1,(TEXT("\r\n\r\n**********UUID error*************\r\n\r\n")));
	}
*/	

    // Done
    rc = TRUE;

cleanUp:    
    return rc;
}



//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetDeviceId/OALIoCtlHalGetUUID
//
//  Implements the IOCTL_HAL_GET_DEVICEID and IOCTL_HAL_GET_UUID handlers.
//
//  A "universally unique" 128 bit identifier is required on all Windows
//  Mobile devices. At a minimum, the 64 bit Device Identifier described
//  below must be permanent (i.e., cannot be modified) and the UUID in
//  total cannot duplicate any other existing UUID. 
//
//  The UUID is composed of 128 bits in the following order: 
//  *   48 bit Manufacturer Code stored in the OAL. The 48 bit Manufacturer
//      Code is specific to the manufacturer and will be provided to each
//      manufacturer by Microsoft. 
//  *   16 bit Version/Variant Code stored in the Kernel. The 16 bit
//      Version/Variant Code describes the version and variant of the UUID.
//      The version is "1" and the variant is "8".
//  *   64 bit Device Identifier Code stored in the hardware (see
//      Realization in OEM documentation). The Device Identifier Code
//      must be stored in hardware and be un-alterable without destroying
//      the device. 
//
//  There are two ways how to call this IOCTL. First will pass buffer with
//  size 16 bytes (size of UUID structure). In such case 
//
 //[david.modify] 2008-06-02 16:08
BOOL OALIoCtlHalGetDeviceId_S3C2416(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize
) {
    DEVICE_ID *pId = (DEVICE_ID *)pOutBuffer;
    UINT32 size;
    BOOL rc = FALSE;

    // First, handle the special case where we care called with a buffer size of 16 bytes
    if ( outSize == sizeof(UUID) )
    {
        rc = OALIoCtlHalGetUUID(IOCTL_HAL_GET_UUID, pInpBuffer, inpSize, pOutBuffer, outSize, pOutSize);
    }
    else // If buffer size is not 16 bytes, return a DEVICE_ID structure
    {
        UCHAR uuid[sizeof(UUID)]; 

        // Compute required structure size
        size = sizeof(DEVICE_ID) + sizeof(uuid);

        // update size if pOutSize is specified
        if (pOutSize != NULL) 
        {
            *pOutSize = size;
        }

        // Check for invalid parameters        
        if (pOutBuffer == NULL || outSize < sizeof(DEVICE_ID))
        {
            NKSetLastError(ERROR_INVALID_PARAMETER);
            OALMSG(OAL_WARN, (
                L"WARN: OALIoCtlHalGetDeviceID: Invalid parameter\r\n"
            ));                
            goto cleanUp;            
        }

        // Set size to DEVICE_ID structure
        pId->dwSize = size;

        // If the size is too small, indicate the correct size 
        if (outSize < size) 
        {
            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto cleanUp;
        }

        // Get UUID
        if (OALIoCtlHalGetUUID(IOCTL_HAL_GET_UUID, NULL, 0, uuid, sizeof(uuid), NULL) == FALSE)
        {
            // IOCTL_HAL_GET_UUID handler will set last error, but this really should never happen.
            goto cleanUp;
        }

        // Populate DEVICE_ID structure members with fields from the UUID
        // The manufacturer ID portion of the UUID maps to the "PresetIDBytes" field
        // The HW-unique portion of the UUID maps to the "PlatformIDBytes" field

        // PresetIDBytes
        /*
        pId->dwPresetIDOffset = size - sizeof(g_oalIoCtlVendorId);
        pId->dwPresetIDBytes = sizeof(g_oalIoCtlVendorId);

        // PlatformIDBytes
        pId->dwPlatformIDOffset = sizeof(DEVICE_ID);
        pId->dwPlatformIDBytes  = sizeof(uuid) - sizeof(g_oalIoCtlVendorId);
        */

        // PresetIDBytes
        pId->dwPresetIDOffset = sizeof(DEVICE_ID);
        pId->dwPresetIDBytes = sizeof(uuid) - sizeof(g_oalIoCtlVendorId);

        // PlatformIDBytes
        pId->dwPlatformIDOffset = size - sizeof(g_oalIoCtlVendorId);
        pId->dwPlatformIDBytes  = sizeof(g_oalIoCtlVendorId);

        memcpy((UINT8*)pId + sizeof(DEVICE_ID), uuid, sizeof(uuid));

        // We are done
        rc = TRUE;        
    }

cleanUp:

    return rc;
}


#if 0		// ppc5 
#define TOTALPLATFORMS 2
const PLATFORMVERSION HALPlatformVersion[TOTALPLATFORMS] = {{4, 0}, {5, 0}};

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlGetDeviceInfo
//
//  Implements the IOCTL_HAL_GET_DEVICE_INFO handler
//
BOOL OALIoCtlHalGetDeviceInfo( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    BOOL rc = FALSE;
    UINT32 length;

    RETAILMSG(1, (L"+OALIoCtlHalGetDeviceInfo(...)\r\n"));

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(UINT32)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        RETAILMSG(1, (
            L"ERROR: OALIoCtlHalGetDeviceInfo: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    // Process according to input request
    switch (*(UINT32*)pInpBuffer) {
    case SPI_GETPLATFORMTYPE:
        // Validate output buffer size

        // on PocketPC and Smartphone platforms the platform type 
        // string may contain NULLs and is terminated with a 
        // double NULL.
        {
            const WCHAR* pwszTemp;
            UINT32 nSubStrLen;
            
            length = sizeof(WCHAR);
            pwszTemp = g_oalIoCtlPlatformType;
            while( *pwszTemp != 0 )
            {
                nSubStrLen = NKwcslen(pwszTemp) + 1;
                length += nSubStrLen * sizeof(WCHAR);
                pwszTemp += nSubStrLen;
            }
        }

        // Return required size
        if (pOutSize != NULL) *pOutSize = length;
        // If there isn't output buffer or it is small return error
        if (pOutBuffer == NULL || outSize < length) {
            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            RETAILMSG(1, (
                L"WARN: OALIoCtlHalGetDeviceInfo: Insufficient buffer(SPI_GETPLATFORMTYPE)\r\n"
            ));
            break;
        }
        // Copy requested data to caller's buffer, set output length
        memcpy(pOutBuffer, g_oalIoCtlPlatformType, length);
        rc = TRUE;
        break;

    case SPI_GETOEMINFO:
        // Validate output buffer size
        length = (NKwcslen(g_oalIoCtlPlatformOEM) + 1) * sizeof(WCHAR);
        // Return required size
        if (pOutSize != NULL) *pOutSize = length;
        // If there isn't output buffer or it is small return error
        if (pOutBuffer == NULL || outSize < length) {
            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            RETAILMSG(1, (
                L"WARN: OALIoCtlHalGetDeviceInfo: Insufficient buffer(SPI_GETOEMINFO)\r\n"
            ));
            break;
        }
        // Copy requested data to caller's buffer, set output length
        memcpy(pOutBuffer, g_oalIoCtlPlatformOEM, length);
        rc = TRUE;
        break;

    case SPI_GETPLATFORMVERSION:
        length = sizeof HALPlatformVersion;
        // Return required size
        if (pOutSize != NULL) *pOutSize = length;
        // Return platform version
        if (outSize < length)
        {
            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            RETAILMSG(1, (
                L"WARN: OALIoCtlHalGetDeviceInfo: Insufficient buffer(SPI_GETPLATFORMVERSION)\r\n"
            ));
            memcpy(pOutBuffer, HALPlatformVersion, outSize);
	        if (pOutSize != NULL) *pOutSize = outSize;
            rc = TRUE;
        }
        else if (!pOutBuffer)
        {
            NKSetLastError(ERROR_INVALID_PARAMETER);
            RETAILMSG(1, (
                L"WARN: OALIoCtlHalGetDeviceInfo: Invalid parameter\r\n"
            ));
        }
        else
        {
            memcpy(pOutBuffer, HALPlatformVersion, length);
            rc = TRUE;
        }
        break;
    default:
        RETAILMSG(1, (
            L"ERROR: OALIoCtlHalGetDeviceInfo: Invalid request\r\n"
        ));
        break;
    }

cleanUp:
    // Indicate status
    RETAILMSG(1, (
        L"-OALIoCtlHalGetDeviceInfo(rc = %d)\r\n", rc
    ));
    return rc;
}

#endif


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetDeviceId
//
//  Implements the IOCTL_HAL_GET_DEVICE_ID handler. This function fills in a 
//  DEVICE_ID structure.
//
BOOL OALIoCtlHalGetDeviceId( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    BOOL rc = FALSE;
    DEVICE_ID *pId = (DEVICE_ID *)pOutBuffer;
    LPSTR pDeviceId = NULL;
    UINT32 size, length1, length2, offset;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoctlHalGetDeviceID(...)\r\n"));

    // Get device unique id from arguments
    pDeviceId = OALArgsQuery(OAL_ARGS_QUERY_DEVID);
    if (pDeviceId == NULL) pDeviceId = "";
    
    // Compute required size (first is unicode, second multibyte!)
    length1 = (NKwcslen(g_oalIoCtlPlatformType) + 1) * sizeof(WCHAR);
//    length2 = strlen(pDeviceId) + 1;
    length2 = 15 + 1;
//    length2 = 16;
    size = sizeof(DEVICE_ID) + length1 + length2;

    // update size if pOutSize is specified
    if (pOutSize) *pOutSize = size;

    // Validate inputs (do it after we can return required size)
    if (pOutBuffer == NULL || outSize < sizeof(DEVICE_ID)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: OALIoCtlHalGetDeviceID: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    // Set size to DEVICE_ID structure
    pId->dwSize = size;

    // If the size is too small, indicate the correct size 
    if (outSize < size) {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto cleanUp;
    }

    // Fill in the Device ID type
    offset = sizeof(DEVICE_ID);

    // Copy in PlatformType data
    pId->dwPresetIDOffset = offset;
    pId->dwPresetIDBytes = length1;
    memcpy((UINT8*)pId + offset, g_oalIoCtlPlatformType, length1);
    offset += length1;

    // Copy device id data
    pId->dwPlatformIDOffset = offset;
    pId->dwPlatformIDBytes  = length2;
    memcpy((UINT8*)pId + offset, pDeviceId, length2);

    // We are done
    rc = TRUE;
    
cleanUp:
    // Indicate status
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetDeviceID(rc = %d)\r\n", rc));
    return rc;	
}





//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlGetDeviceInfo
//
//  Implements the IOCTL_HAL_GET_DEVICE_INFO handler
//
BOOL OALIoCtlHalGetDeviceInfo( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    BOOL rc = FALSE;
    UINT32 length;
    PLATFORMVERSION *pVersion = (PLATFORMVERSION*)pOutBuffer;


    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetDeviceInfo(...)\r\n"));

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(UINT32)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalGetDeviceInfo: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    // Process according to input request
    switch (*(UINT32*)pInpBuffer) {
    case SPI_GETPLATFORMTYPE:
        // Validate output buffer size
        length = (NKwcslen(g_oalIoCtlPlatformType) + 1) * sizeof(WCHAR);
        // Return required size
        if (pOutSize != NULL) *pOutSize = length;
        // If there isn't output buffer or it is small return error
        if (pOutBuffer == NULL || outSize < length) {
            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            OALMSG(OAL_WARN, (
                L"WARN: OALIoCtlHalGetDeviceInfo: Insufficient buffer\r\n"
            ));
            break;
        }
        // Copy requested data to caller's buffer, set output length
        memcpy(pOutBuffer, g_oalIoCtlPlatformType, length);
        rc = TRUE;
        break;

    case SPI_GETOEMINFO:
        // Validate output buffer size
        length = (NKwcslen(g_oalIoCtlPlatformOEM) + 1) * sizeof(WCHAR);
        // Return required size
        if (pOutSize != NULL) *pOutSize = length;
        // If there isn't output buffer or it is small return error
        if (pOutBuffer == NULL || outSize < length) {
            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            OALMSG(OAL_WARN, (
                L"WARN: OALIoCtlHalGetDeviceInfo: Insufficient buffer\r\n"
            ));
            break;
        }
        // Copy requested data to caller's buffer, set output length
        memcpy(pOutBuffer, g_oalIoCtlPlatformOEM, length);
        rc = TRUE;
        break;

 //[david.modify] 2007-09-12 13:41
 //==================================================================
            case SPI_GETOEMINFO+1:			
// 		     wsprintf(szVersion, L"[%s] [v%d.%02d] on [%s]",
//		              HALOEMStr, OEM_MAJOR_VER, OEM_MINOR_VER, strCompileTime);
//			kstrcpyW(szVersion, HALOEMStr);			
//[david. end] 2007-09-11 17:06		
//                    len = (strlenW(HALOEMStr)+1)*sizeof(WCHAR);

 //[david.modify] 2008-10-07 14:13
			g_stOEMInformation.u32LCDType=pBSPArgs->u32LCDType;
			
			length = sizeof(g_stOEMInformation);
			DPNOK(length);
                    if (pOutSize)
                        *pOutSize = length;
                    if (outSize >= length && pOutBuffer)
                    {
                            memcpy(pOutBuffer,&g_stOEMInformation,length);
                            rc = TRUE;
                    } else
                            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
                    break;					
 //==================================================================
             		

    case SPI_GETPLATFORMVERSION:
        // Return required size
        if (pOutSize != NULL) *pOutSize = sizeof(PLATFORMVERSION);
        if (outSize < sizeof(PLATFORMVERSION)) {
            NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            OALMSG(OAL_WARN, (
                L"WARN: OALIoCtlHalGetDeviceInfo: Insufficient buffer\r\n"
            ));
            break;
        }
        pVersion->dwMajor = CE_MAJOR_VER;
        pVersion->dwMinor = CE_MINOR_VER;
        rc = TRUE;
        break;
    default:
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalGetDeviceInfo: Invalid request\r\n"
        ));
        break;
    }

cleanUp:
    // Indicate status
    OALMSG(OAL_FUNC&&OAL_IOCTL, (
        L"-OALIoCtlHalGetDeviceInfo(rc = %d)\r\n", rc
    ));
    return rc;
}

//from atlas
//------------------------------------------------------------------------
//
//  Function:  OALIoCtlProcessorInformation
//
//  Implements the IOCTL_PROCESSOR_INFORMATION handler.
//
BOOL OALIoCtlProcessorInfo(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    BOOL rc = FALSE;
    PROCESSOR_INFO *pInfo = (PROCESSOR_INFO*)pOutBuffer;
    UINT32 length1, length2, length3;

    OALMSG(OAL_FUNC, (L"+OALIoCtlProcessorInfo(...)\r\n"));

    // Set required/returned size if pointer isn't NULL
    if (pOutSize != NULL) *pOutSize = sizeof(PROCESSOR_INFO);
    
    // Validate inputs
    if (pOutBuffer == NULL || outSize < sizeof(PROCESSOR_INFO)) {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (
            L"WARN: OALIoCtlProcessorInfo: Buffer too small\r\n"
        ));
        goto cleanUp;
    }

    // Verify OAL lengths
    length1 = (NKwcslen(g_oalIoCtlProcessorCore) + 1) * sizeof(WCHAR);
    if (length1 > sizeof(pInfo->szProcessCore)) {
        OALMSG(OAL_ERROR, (
            L"ERROR:OALIoCtlProcessorInfo: Core value too big\r\n"
        ));
        goto cleanUp;
    }

    length2 = (NKwcslen(g_oalIoCtlProcessorName) + 1) * sizeof(WCHAR);
    if (length2 > sizeof(pInfo->szProcessorName)) {
        OALMSG(OAL_ERROR, (
            L"ERROR:OALIoCtlProcessorInfo: Name value too big\r\n"
        ));
        goto cleanUp;
    }

    length3 = (NKwcslen(g_oalIoCtlProcessorVendor) + 1) * sizeof(WCHAR);
    if (length3 > sizeof(pInfo->szVendor)) {
        OALMSG(OAL_ERROR, (
            L"ERROR:OALIoCtlProcessorInfo: Vendor value too big\r\n"
        ));
        goto cleanUp;
    }

    // Copy in processor information    
    pInfo->wVersion = 1;
    memset(pInfo, 0, sizeof(PROCESSOR_INFO));
    memcpy(pInfo->szProcessCore, g_oalIoCtlProcessorCore, length1);
    memcpy(pInfo->szProcessorName, g_oalIoCtlProcessorName, length2);
    memcpy(pInfo->szVendor, g_oalIoCtlProcessorVendor, length3);
    pInfo->dwInstructionSet = g_oalIoCtlInstructionSet;
    pInfo->dwClockSpeed  = g_oalIoCtlClockSpeed;

    // Indicate success
    rc = TRUE;

cleanUp:
    OALMSG(OAL_FUNC, (L"-OALIoCtlProcessorInfo(rc = %d)\r\n", rc));
    return rc;
}


BOOL OALIoCtlHalGetMAPVENDOR( 
    UINT32 code,
    VOID *pInpBuffer, UINT32 inpSize, 
    VOID *pOutBuffer, UINT32 outSize, 
    UINT32 *pOutSize)
{
//	TCHAR outputBuffer[] = TEXT("iGOSEA@1");//TEXT("iGOEEUNNG@1");//iGORUS@1

//这次出货，两个OS版本
//	TCHAR outputBuffer[] = TEXT("iGODACH@1");//TEXT("iGOEEUNNG@1");//iGORUS@1
	TCHAR outputBuffer[] = TEXT("iGOGER@1");//TEXT("iGOEEUNNG@1");//iGORUS@1
	memcpy(pOutBuffer, outputBuffer, sizeof(outputBuffer));
	return TRUE;
}


BOOL OALIoCtlHalGetMAPEXTRA( 
    UINT32 code,
    VOID *pInpBuffer, UINT32 inpSize, 
    VOID *pOutBuffer, UINT32 outSize, 
    UINT32 *pOutSize)
{
//	TCHAR outputBuffer[] = TEXT("JIGATEK335A");    //JIGATEK335A
	TCHAR outputBuffer[] = TEXT("GPS35JS");    //JIGATEK335A
	memcpy(pOutBuffer, outputBuffer, sizeof(outputBuffer));
	return TRUE;
}

//------------------------------------------------------------------------------
//
//  Global: g_oalIoCtlTable[]    
//
//  IOCTL handler table. This table includes the IOCTL code/handler pairs  
//  defined in the IOCTL configuration file. This global array is exported 
//  via oal_ioctl.h and is used by the OAL IOCTL component.
//
const OAL_IOCTL_HANDLER g_oalIoCtlTable[] = {
#if defined(__PSII_DEFINED__)
{ IOCTL_POCKETSTOREII_CMD,                  0,  OALIoCtlPocketStoreCMD      },
{ IOCTL_HAL_POSTINIT,                       0,  OALIoCtlPostInit            },
#endif //#if defined(__PSII_DEFINED__)

#include "ioctl_tab.h"
};

//------------------------------------------------------------------------------


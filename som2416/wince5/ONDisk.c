/*****************************************************************************/
/*                                                                           */
/* PROJECT : PocketStoreII                                                   */
/* MODULE  : Block Driver for supporting FAT File system                     */
/* FILE    : ONDisk.c                                                        */
/* PURPOSE : This file implements Windows CE Block device driver interface   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*          COPYRIGHT 2003-2006 SAMSUNG ELECTRONICS CO., LTD.                */
/*                          ALL RIGHTS RESERVED                              */
/*                                                                           */
/*   Permission is hereby granted to licensees of Samsung Electronics        */
/*   Co., Ltd. products to use or abstract this computer program for the     */
/*   sole purpose of implementing a product based on Samsung                 */
/*   Electronics Co., Ltd. products. No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in whole,      */
/*   are granted.                                                            */
/*                                                                           */
/*   Samsung Electronics Co., Ltd. makes no representation or warranties     */
/*   with respect to the performance of this computer program, and           */
/*   specifically disclaims any responsibility for any damages,              */
/*   special or consequential, connected with the use of this program.       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* REVISION HISTORY                                                          */
/*                                                                           */
/*   18-OCT-2006 [Seungkyu Kim]   : first writing                            */
/*                                                                           */
/*****************************************************************************/

#include <windows.h>
#include <bldver.h>
#include <windev.h>
#include <types.h>
#include <excpt.h>
#include <tchar.h>
#include <devload.h>
#include <diskio.h>
#include <storemgr.h>

#include <WMRConfig.h>
#include <WMRTypes.h>
#include "ONDisk.h"
#include <VFLBuffer.h>
#include <HALWrapper.h>
#include <config.h>
#include <WMR.h>
#include <WMR_Utils.h>

/*****************************************************************************/
/* STL Configuration                                                         */
/*****************************************************************************/
#define     PSII_MAX_READ_BUF_SIZE      (WMR_SECTOR_SIZE * 16)
/*****************************************************************************/
/* Debug Definitions                                                         */
/*****************************************************************************/
#define STDRV_RTL_PRINT(x)          PSII_DBG_PRINT(x)

#if STDRV_ERR_MSG_ON
#define STDRV_ERR_PRINT(x)          PSII_DBG_PRINT(x)
#else
#define STDRV_ERR_PRINT(x)
#endif /* #if STDRV_ERR_MSG_ON */

#if STDRV_LOG_MSG_ON
#define STDRV_LOG_PRINT(x)          PSII_DBG_PRINT(x)
#else
#define STDRV_LOG_PRINT(x)
#endif  /* #if STDRV_LOG_MSG_ON */

#if STDRV_INF_MSG_ON
#define STDRV_INF_PRINT(x)          PSII_DBG_PRINT(x)
#else
#define STDRV_INF_PRINT(x)
#endif  /* #if STDRV_INF_MSG_ON */

#define STL_DELETE_DBG 0
/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
CRITICAL_SECTION v_DiskCrit;
PDISK            v_DiskList;                // initialized to 0 in bss

/* Debug Zones.
 */
#ifdef DEBUG

DBGPARAM dpCurSettings = {
    L"ONDISK", {
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0000
};

#endif

/*****************************************************************************/
/* Imported variable declarations                                            */
/*****************************************************************************/


/*****************************************************************************/
/* Imported function declarations                                            */
/*****************************************************************************/


/*****************************************************************************/
/* Local #define                                                             */
/*****************************************************************************/
#undef RESERVED_REGION_SUPPORT

/*****************************************************************************/
// Local constant definitions
/*****************************************************************************/

/*****************************************************************************/
// Local typedefs
/*****************************************************************************/

/*****************************************************************************/
// Local function prototypes
/*****************************************************************************/
static HKEY  OpenDriverKey(LPTSTR ActiveKey);
static BOOL  GetFolderName(PDISK pDisk, LPWSTR FolderName, DWORD cBytes, DWORD *pcBytes);
static BOOL GetBmlPartitionId(PDISK pDisk);
//static BOOL  GetFSDName(PDISK pDisk, LPWSTR FSDName, DWORD cBytes, DWORD *pcBytes);
static VOID  CloseDisk(PDISK pDisk);
static DWORD DoDiskRead(PDISK pDisk, PVOID pData);
static DWORD DoDiskWrite(PDISK pDisk, PVOID pData);

static DWORD DoDiskDeleteSectors(PDISK pDisk, PVOID pData);
static BOOL GetDeviceInfo(PDISK pDisk, PSTORAGEDEVICEINFO psdi);
static BOOL GetStorageID(PDISK pDisk, PSTORAGE_IDENTIFICATION psid, DWORD cBytes, DWORD * pcBytes);

static DWORD GetDiskInfo        (PDISK pDisk, PDISK_INFO pInfo);
static DWORD SetDiskInfo        (PDISK pDisk, PDISK_INFO pInfo);
static PDISK CreateDiskObject   (VOID);
static BOOL  IsValidDisk        (PDISK pDisk);
static BOOL  InitializeFTL      (PDISK pDisk);
static BOOL  InitDisk           (PDISK pDisk, LPTSTR ActiveKey);


/*****************************************************************************/
// Function definitions
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      OpenDriverKey                                                        */
/* DESCRIPTION                                                               */
/*      This function opens the driver key specified by the active key       */
/* PARAMETERS                                                                */
/*      ActiveKey       Handle to a currently open key or any of the         */
/*                      following predefined reserved handle values          */
/* RETURN VALUES                                                             */
/*      Return values is HKEY value of "[ActiveKey]\[Key]", The caller is    */
/*      responsible for closing the returned HKEY                            */
/*                                                                           */
/*****************************************************************************/
static HKEY
OpenDriverKey(LPTSTR ActiveKey)
{
    TCHAR DevKey[256];
    HKEY  hDevKey;
    HKEY  hActive;
    DWORD ValType;
    DWORD ValLen;
    DWORD status = 0;

    STDRV_RTL_PRINT((TEXT("[OND:OUT] ++OpenDriverKey()\r\n")));
    //
    // Get the device key from active device registry key
    //
    STDRV_RTL_PRINT((TEXT("[OND:   ] OpenDriverKey RegOpenKeyEx(HLM\\%s) returned %d!!!\r\n"),
		ActiveKey, status));
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,/* Handle to a currently open key       */
		ActiveKey,         /* Pointer to subkey                    */
		0,                 /* Option : Reserved - set to 0         */
		0,                 /* samDesired : Not supported - set to 0 */
		&hActive);         /* Pointer for receved handele          */

    if (ERROR_SUCCESS != status)
    {
        STDRV_LOG_PRINT((TEXT("[OND:   ] OpenDriverKey RegOpenKeyEx(HLM\\%s) returned %d!!!\r\n"),
			ActiveKey, status));
        STDRV_RTL_PRINT((TEXT("[OND:   ] OpenDriverKey RegOpenKeyEx(HLM\\%s) returned %d!!!\r\n"),
			ActiveKey, status));
        return NULL;
    }

    hDevKey = NULL;
    ValLen = sizeof(DevKey);

    STDRV_RTL_PRINT((TEXT("[OND:   ] OpenDriverKey - RegQueryValueEx(%s) returned %d\r\n"),
		DEVLOAD_DEVKEY_VALNAME, status));
    status = RegQueryValueEx(hActive,               /* Handle to a currently open key   */
		DEVLOAD_DEVKEY_VALNAME,/* Pointer to quary                 */
		NULL,                  /* Reserved - set to NULL           */
		&ValType,              /* Pointer to type of data          */
		(PUCHAR)DevKey,        /* Pointer to data                  */
		&ValLen);              /* the Length of data               */

    if (ERROR_SUCCESS != status)
    {
        STDRV_LOG_PRINT((TEXT("[OND:   ] OpenDriverKey - RegQueryValueEx(%s) returned %d\r\n"),
			DEVLOAD_DEVKEY_VALNAME, status));
        STDRV_RTL_PRINT((TEXT("[OND:   ] OpenDriverKey - RegQueryValueEx(%s) returned %d\r\n"),
			DEVLOAD_DEVKEY_VALNAME, status));

        RegCloseKey(hActive);
        return hDevKey;
    }

    //
    // Get the geometry values from the device key
    //
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,   /* Handle to a currently open key       */
		DevKey,               /* Pointer to subkey                    */
		0,                    /* Option : Reserved - set to 0         */
		0,                    /* samDesired : Not supported - set to 0 */
		&hDevKey);            /* Pointer for receved handele          */

    if (ERROR_SUCCESS != status)
    {
        hDevKey = NULL;
        STDRV_LOG_PRINT((TEXT("[OND:   ] OpenDriverKey RegOpenKeyEx - DevKey(HLM\\%s) returned %d!!!\r\n"),
			DevKey, status));
        STDRV_RTL_PRINT((TEXT("[OND:   ] OpenDriverKey RegOpenKeyEx - DevKey(HLM\\%s) returned %d!!!\r\n"),
			DevKey, status));
    }

    RegCloseKey(hActive);
    STDRV_RTL_PRINT((TEXT("[OND:OUT] --OpenDriverKey()\r\n")));

    return hDevKey;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      GetFolderName                                                        */
/* DESCRIPTION                                                               */
/*      Function to retrieve the folder name value from the driver key       */
/*      The folder name is used by File System Driver to name this disk      */
/*      volume                                                               */
/* PARAMETERS                                                                */
/*      pDisk       PocketStoreII driver own structure pointer               */
/*      FolderName                                                           */
/*      cBytes                                                               */
/*      pcBytes                                                              */
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static BOOL
GetFolderName(PDISK  pDisk,
              LPWSTR FolderName,
              DWORD  cBytes,
              DWORD *pcBytes)
{
    HKEY  DriverKey;
//    HKEY  hActive;
    DWORD ValType;
    DWORD status;

    // Get Profile from Builtin Key
    EnterCriticalSection(&(pDisk->d_DiskCardCrit));

    {
        DriverKey = OpenDriverKey(pDisk->d_ActivePath);
        if (NULL == DriverKey)
        {
            return FALSE;
        }

        *pcBytes = cBytes;
        status = RegQueryValueEx(DriverKey,
			TEXT("Folder"),
			NULL,
			&ValType,
			(PUCHAR)FolderName,
			pcBytes);

        if (ERROR_SUCCESS != status)
        {
            STDRV_RTL_PRINT((TEXT("[OND:   ]  GetFolderName - RegQueryValueEx(Folder) returned %d\r\n"),
				status));
            *pcBytes = 0;
        }
        else
        {
            STDRV_RTL_PRINT((TEXT("[OND:MSG]  GetFolderName - Folder = %s, length = %d\r\n"),
				FolderName, *pcBytes));
            *pcBytes += sizeof(WCHAR); // account for terminating 0.
        }

        RegCloseKey(DriverKey);
        if ((ERROR_SUCCESS != status) || (0 == *pcBytes))
        {
            return FALSE;
        }
    }

    LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

    return TRUE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      GetBmlPartitionId                                                    */
/* DESCRIPTION                                                               */
/*      Function to retrieve BML partition ID for STL API                    */
/* PARAMETERS                                                                */
/*      pDisk       PocketStoreII driver own structure pointer               */
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static BOOL
GetBmlPartitionId(PDISK  pDisk)
{
    HKEY  DriverKey;
    DWORD ValType;
    DWORD status;
    DWORD nBytes=4;

    EnterCriticalSection(&(pDisk->d_DiskCardCrit));

    DriverKey = OpenDriverKey(pDisk->d_ActivePath);
    if (NULL == DriverKey)
    {
        return FALSE;
    }

    // Get BmlPartitionId from Builtin Key
    status = RegQueryValueEx(DriverKey,
		TEXT("BmlPartitionId"),
		NULL,
		&ValType,
		(LPBYTE) &(pDisk->nBmlPartitionId),
		&nBytes);

    if ((ERROR_SUCCESS != status) || (0 == nBytes))
    {
        STDRV_ERR_PRINT((TEXT("[OND:ERR]  GetBmlId - RegQueryValueEx() returned %d, nBytes=%d\r\n"),
			status, nBytes));
    }
    else
    {
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  GetBmlId - BmlPartitionId = %d\r\n"),
			pDisk->nBmlPartitionId));
    }

#if ONDISK_MULTIPARTITION
	// Get WMRStartSector from Builtin key
	status = RegQueryValueEx(DriverKey,
		TEXT("WMRStartSector"),
		NULL,
		&ValType,
		(LPBYTE) &(pDisk->nWMRStartSector),
		&nBytes);

	if ((ERROR_SUCCESS != status) || (0 == nBytes))
	{
        STDRV_ERR_PRINT((TEXT("[OND:ERR]  GetBmlId - RegQueryValueEx() returned %d, nBytes=%d\r\n"),
			status, nBytes));
	}
	else
	{
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  GetBmlId - StartSector = %d\r\n"),
			pDisk->nWMRStartSector));
	}

	// Get WMRStartSector from Builtin key
	status = RegQueryValueEx(DriverKey,
		TEXT("WMRNumOfSector"),
		NULL,
		&ValType,
		(LPBYTE) &(pDisk->nWMRNumOfSector),
		&nBytes);

	if ((ERROR_SUCCESS != status) || (0 == nBytes))
	{
        STDRV_ERR_PRINT((TEXT("[OND:ERR]  GetBmlId - RegQueryValueEx() returned %d, nBytes=%d\r\n"),
			status, nBytes));
	}
	else
	{
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  GetBmlId - NumOfSector = %u\r\n"),
			(UINT32)(pDisk->nWMRNumOfSector)));
	}
#endif

	// Get BmlVolumeId from Builtin Key
	status = RegQueryValueEx(DriverKey,
		TEXT("BmlVolumeId"),
		NULL,
		&ValType,
		(LPBYTE) &(pDisk->nVol),
		&nBytes);

    if ((ERROR_SUCCESS != status) || (0 == nBytes))
    {
        STDRV_ERR_PRINT((TEXT("[OND:ERR]  GetBmlId - RegQueryValueEx() returned %d, nBytes=%d\r\n"),
			status, nBytes));
    }
    else
    {
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  GetBmlId - BmlVolumeId = %d\r\n"),
			pDisk->nVol));
    }

    status = RegCloseKey(DriverKey);

    if (ERROR_SUCCESS != status)
    {
    	STDRV_ERR_PRINT((TEXT("[OND:ERR]  GetBmlId - RegCloseKey() returned %d\r\n"),
			status));
        return FALSE;
    }

    LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

    return TRUE;
}


static BOOL
GetDeviceInfo(PDISK                 pDisk,
              PSTORAGEDEVICEINFO    psdi)
{
    HKEY  DriverKey;
    DWORD ValType;
    DWORD status;
    DWORD dwSize;
	DWORD dwPartType=0;

    STDRV_RTL_PRINT((TEXT("[OND:OUT] ++GetDeviceInfo()\r\n")));

    EnterCriticalSection(&(pDisk->d_DiskCardCrit));

    DriverKey = OpenDriverKey(pDisk->d_ActivePath);
    if (DriverKey)
    {
        dwSize = sizeof(psdi->szProfile);
        status = RegQueryValueEx(DriverKey,
			TEXT("Profile"),
			NULL,
			&ValType,
			(LPBYTE)psdi->szProfile,
			&dwSize);

        if ((status != ERROR_SUCCESS) || (dwSize > sizeof(psdi->szProfile)))
        {
            STDRV_LOG_PRINT((TEXT("[OND:   ] GetFolderName - RegQueryValueEx(Profile) returned %d\r\n"),
				status));
            STDRV_RTL_PRINT((TEXT("[OND:   ] GetFolderName - RegQueryValueEx(Profile) returned %d\r\n"),
				status));
            wcscpy( psdi->szProfile, L"Default");
        }
        else
        {
            STDRV_LOG_PRINT((TEXT("[OND:   ] GetProfileName - Profile = %s, length = %d\r\n"),
				psdi->szProfile, dwSize));
            STDRV_RTL_PRINT((TEXT("[OND:   ] GetProfileName - Profile = %s, length = %d\r\n"),
				psdi->szProfile, dwSize));
        }
        dwSize=sizeof(DWORD);
        status = RegQueryValueEx(DriverKey,
			TEXT("Order"),
			NULL,
			&ValType,
			(LPBYTE)dwPartType,
			&dwSize);

        if (status == ERROR_SUCCESS)
        {
            STDRV_LOG_PRINT((TEXT("[OND:   ] Order = %d, length = %d\r\n"),
				dwPartType, dwSize));
            STDRV_RTL_PRINT((TEXT("[OND:   ] Order = %d, length = %d\r\n"),
				dwPartType, dwSize));
        }

        RegCloseKey(DriverKey);
    }

    psdi->cbSize        = sizeof(STORAGEDEVICEINFO);
    psdi->dwDeviceClass = STORAGE_DEVICE_CLASS_BLOCK;
    psdi->dwDeviceType  = STORAGE_DEVICE_TYPE_FLASH;
    psdi->dwDeviceFlags = STORAGE_DEVICE_FLAG_READWRITE;

    LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

    STDRV_RTL_PRINT((TEXT("[OND:OUT] --GetDeviceInfo()\r\n")));

    return TRUE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      GetStorageID                                                         */
/* DESCRIPTION                                                               */
/*      free all resources associated with the specified disk                */
/* PARAMETERS                                                                */
/*      pDisk       NFLAT_PS driver own structure pointer                    */
/* RETURN VALUES                                                             */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/
static BOOL
GetStorageID(PDISK                      pDisk,
             PSTORAGE_IDENTIFICATION    psid,
             DWORD                      cBytes,
             DWORD                     *pcBytes)
{
    STDRV_RTL_PRINT((TEXT("[OND:MSG] ++GetStorageID \r\n")));

    if (cBytes < sizeof(*psid))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else
    {
        // initialize the error status in case there are no errors
        SetLastError(ERROR_SUCCESS);
    }

    __try
    {
        psid->dwSize                = sizeof(*psid);
        psid->dwFlags               = 0;  // can be or of {MANUFACTUREID,SERIALNUM}_INVALID
        psid->dwManufactureIDOffset = psid->dwSerialNumOffset = 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    STDRV_RTL_PRINT((TEXT("[OND:MSG] --GetStorageID \r\n")));

    return TRUE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      CloseDisk                                                            */
/* DESCRIPTION                                                               */
/*      free all resources associated with the specified disk                */
/* PARAMETERS                                                                */
/*      pDisk       PocketStoreII driver own structure pointer               */
/* RETURN VALUES                                                             */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/
static VOID
CloseDisk(PDISK pDisk)
{
    PDISK pd;

    STDRV_LOG_PRINT((TEXT("[OND: IN]  +CloseDisk(pDisk=0x%x)\r\n"), pDisk));

    EnterCriticalSection(&(pDisk->d_DiskCardCrit));

    /*-----------------------------------------*/
    /* Remove it from the global list of disks */
    /*-----------------------------------------*/
    if (pDisk == v_DiskList)
    {
        v_DiskList = pDisk->pd_next;
    }
    else
    {
        pd = v_DiskList;
        while (pd->pd_next != NULL)
        {
            if (pd->pd_next == pDisk)
            {
                pd->pd_next = pDisk->pd_next;
                break;
            }
            pd = pd->pd_next;
        }
    }

    //
    // Try to ensure this is the only thread holding the disk crit sec
    //
    Sleep(50);
    EnterCriticalSection (&(pDisk->d_DiskCardCrit));
    LeaveCriticalSection (&(pDisk->d_DiskCardCrit));
    DeleteCriticalSection(&(pDisk->d_DiskCardCrit));

    /* Dealloc pDisk handle */
    LocalFree(pDisk);
    LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

    STDRV_LOG_PRINT((TEXT("[OND:OUT] --CloseDisk()\r\n")));
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DoDiskRead                                                           */
/* DESCRIPTION                                                               */
/*      Do read operation from NAND flash memory                             */
/* PARAMETERS                                                                */
/*      pDisk       PocketStoreII driver own structure pointer               */
/*      pData       PSQ_REQ structure pointer,it contains request information*/
/*                  for read operations                                      */
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static DWORD
DoDiskRead(PDISK pDisk,
           PVOID pData)
{
    DWORD   status = ERROR_SUCCESS;
    DWORD   nSizeOfSGBuf;
    PSG_REQ pSgr;
    PSG_BUF pSg;
    PUCHAR  pSGBuf;
    UINT    nStartSecNo, nNumOfSec, nSecNoIdx;
    UINT    nNumOfSG, nSGBufLen, nSGBufNum;
    UINT    nSizeOfSec;
    UCHAR  *pSecBuf;
    UINT    nSecBufOffset;

    /* For FTL_Read Function */
    INT32   nRet;
    UINT32  nNumOfScts;
    UINT32  nDstOffset;
    UINT32  nSrcOffset;
    UINT32  nCpBytes;

    EnterCriticalSection(&(pDisk->d_DiskCardCrit));

    pSgr        = (PSG_REQ)pData;
    nStartSecNo = pSgr->sr_start;
    nNumOfSec   = pSgr->sr_num_sec;
    nNumOfSG    = pSgr->sr_num_sg;
    nSizeOfSec  = pDisk->d_DiskInfo.di_bytes_per_sect;

    if (pDisk->bIsFTLOpen == FALSE)
    {
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    if (nNumOfSG > MAX_SG_BUF)
    {
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    nSGBufLen = 0;

    // calculate total buffer space of scatter gather buffers
    for (nSGBufNum = 0; nSGBufNum < nNumOfSG; nSGBufNum++)
    {
        nSGBufLen += pSgr->sr_sglist[nSGBufNum].sb_len;
    }

    // check total SG buffer space is enough for reqeusted size
    if (nSGBufLen < (nNumOfSec * nSizeOfSec))
    {
        STDRV_ERR_PRINT((TEXT("[OND:ERR] DoDiskRead: SG Buffer space %d bytes less than block read size %d bytes\r\n"),
			nSGBufLen, nNumOfSec * nSizeOfSec));

        status = ERROR_GEN_FAILURE;
        goto ddi_exit;
    }

    pSgr->sr_status = ERROR_IO_PENDING;

    //
    // Make sure request doesn't exceed the disk
    //
    if ((nStartSecNo + nNumOfSec - 1) > pDisk->d_DiskInfo.di_total_sectors)
    {
        status = ERROR_SECTOR_NOT_FOUND;
        goto ddi_exit;
    }
    status = ERROR_SUCCESS;

    pSg = &(pSgr->sr_sglist[0]);

    // Index of Sector Number
    nSecNoIdx       = 0;
    nSecBufOffset   = 0;
    pSecBuf         = NULL;

#if ONDISK_MULTIPARTITION
	nStartSecNo += (pDisk->nWMRStartSector >> OND_SECTOR_SHIFT);  // remapping sector
#endif

    /* Number of SG is  1 (Most of cases) */
    if (nNumOfSG == 1)
    {
        nNumOfScts = nSGBufLen / (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);

        //pSGBuf = MapPtrToProcess((LPVOID)pSg->sb_buf, GetCallerProcess());
		pSGBuf = MapCallerPtr((LPVOID)pSg->sb_buf,nSGBufLen);

        if (nNumOfScts != 0)
        {
            nRet = FTL_Read((nStartSecNo << OND_SECTOR_SHIFT),    /* Start Logical Sector Number  */
                            (nNumOfScts << OND_SECTOR_SHIFT),     /* Number of Sectors            */
                            pSGBuf);        /* Pointer ot buffer            */

            if (nRet != FTL_SUCCESS)
            {
                STDRV_ERR_PRINT((TEXT("[OND:ERR](DoDiskRead) FTL Read Error=0x%x\r\n"), nRet));
                status = ERROR_SECTOR_NOT_FOUND;
                goto ddi_req_done;
            }
            else STDRV_INF_PRINT((TEXT("[OND:INF](DoDiskRead) FTL Read OK+\r\n")));
            nStartSecNo += nNumOfScts;
        }

        /* To solve 512 byte align problem */
        if ((nSGBufLen % (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT)) != 0)
        {
            UINT  nCpResumeOffset;
            UINT  nCpLeftSize;

            /* Memory Allocation for 1 sector read */
            pSecBuf = (UCHAR*)malloc(WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);

            nCpResumeOffset = nNumOfScts * (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);
            nCpLeftSize     = nSGBufLen  % (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);

            nRet = FTL_Read((nStartSecNo << OND_SECTOR_SHIFT),    /* Start Logical Sector Number  */
                            (1 << OND_SECTOR_SHIFT),              /* Number of Sectors            */
                            pSecBuf);       /* Pointer ot buffer            */

            if (nRet != FTL_SUCCESS)
            {
                STDRV_ERR_PRINT((TEXT("[OND:ERR](DoDiskRead) FTL Read Error=0x%x\r\n"), nRet));
                free(pSecBuf);
                status = ERROR_SECTOR_NOT_FOUND;
                goto ddi_req_done;
            }
            else STDRV_INF_PRINT((TEXT("[OND:INF](DoDiskRead) FTL Read OK++\r\n")));

            memcpy((UCHAR*)(pSGBuf + nCpResumeOffset), pSecBuf, nCpLeftSize);

            free(pSecBuf);
        }
    }
    /* Number of SG is greater than 1 */
    else
    {
        nDstOffset = 0;

        while (nSGBufLen)
        {
            if (nSGBufLen >= PSII_MAX_READ_BUF_SIZE)
            {
                nNumOfScts = PSII_MAX_READ_BUF_SIZE / (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);
                nCpBytes   = (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts;
            }
            else
            {
                nNumOfScts = nSGBufLen / (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);
                nCpBytes   = (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts;
                if (nSGBufLen % (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT))
                {
                    nNumOfScts++;
                    nCpBytes += (nSGBufLen % (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT));
                }
            }

            pSecBuf = (UCHAR *)malloc((WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts);

            nRet = FTL_Read((nStartSecNo << OND_SECTOR_SHIFT),    /* Start Logical Sector Number  */
                            (nNumOfScts << OND_SECTOR_SHIFT),     /* Number of Sectors            */
                            pSecBuf);       /* Pointer ot buffer            */

            if (nRet != FTL_SUCCESS)
            {
                STDRV_ERR_PRINT((TEXT("[OND:ERR](DoDiskRead) FTL Read Error=0x%x\r\n"), nRet));
                status = ERROR_SECTOR_NOT_FOUND;
                free(pSecBuf);
                goto ddi_req_done;
            }
            else STDRV_INF_PRINT((TEXT("[OND:INF](DoDiskRead) FTL Read OK+++\r\n")));

            nStartSecNo += nNumOfScts;
            nSrcOffset   = 0;

            while(nCpBytes)
            {
                /* Get New Sg buffer */
                if (nDstOffset == 0)
                {
                    nSizeOfSGBuf = pSg->sb_len;
                    pSGBuf       = MapPtrToProcess((LPVOID)pSg->sb_buf, GetCallerProcess());
                }

                if (nSizeOfSGBuf <= nCpBytes)
                {
                    memcpy((UCHAR*)(pSGBuf + nDstOffset), (UCHAR*)(pSecBuf + nSrcOffset), nSizeOfSGBuf);
                    nSrcOffset += nSizeOfSGBuf;
                    nDstOffset  = 0;
                    nCpBytes   -= nSizeOfSGBuf;
                    pSg++;
                }
                else
                {
                    memcpy((UCHAR*)(pSGBuf + nDstOffset), (UCHAR*)pSecBuf + nSrcOffset, nCpBytes);
                    nDstOffset   += nCpBytes;
                    nSizeOfSGBuf -= nCpBytes;
                    nCpBytes = 0;
                }
            }

            if (nSGBufLen < ((WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts))
            {
                free(pSecBuf);
                break;
            }
            else
            {
                nSGBufLen -= ((WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts);
            }

            free(pSecBuf);
        }
    }
ddi_req_done:

ddi_exit:
    pSgr->sr_status = status;
    LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

    return status;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DoDiskWrite                                                          */
/* DESCRIPTION                                                               */
/*      Do write operation into NAND flash memory                            */
/* PARAMETERS                                                                */
/*      pDisk       PocketStoreII driver own structure pointer               */
/*      pData       PSQ_REQ structure pointer,it contains request information*/
/*                  for write operations                                     */
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static DWORD
DoDiskWrite(PDISK pDisk,
            PVOID pData)
{
    DWORD   status = ERROR_SUCCESS;
    DWORD   nSizeOfSGBuf;
    PSG_REQ pSgr;
    PSG_BUF pSg;
    PUCHAR  pSGBuf;
    UINT    nStartSecNo, nNumOfSec, nSecNoIdx;
    UINT    nNumOfSG, nSGBufLen, nSGBufNum;
    UINT    nSecBufOffset;
    UINT    nSizeOfSec;
    UCHAR  *pSecBuf;
    INT32   nRet;
    UINT32  nNumOfScts;
    UINT32  nDstOffset;
    UINT32  nSrcOffset;
    UINT32  nCpBytes;


    STDRV_LOG_PRINT((TEXT("[OND: IN] ++DoDiskWrite()\r\n")));
    EnterCriticalSection(&(pDisk->d_DiskCardCrit));

    pSgr        = (PSG_REQ) pData;
    nStartSecNo = pSgr->sr_start;
    nNumOfSec   = pSgr->sr_num_sec;
    nNumOfSG    = pSgr->sr_num_sg;
    nSizeOfSec  = pDisk->d_DiskInfo.di_bytes_per_sect;

    if (pDisk->bIsFTLOpen == FALSE)
    {
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    if (nNumOfSG > MAX_SG_BUF)
    {
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    nSGBufLen = 0;

    // calculate total buffer space of scatter gather buffers
    for (nSGBufNum = 0; nSGBufNum < nNumOfSG; nSGBufNum ++)
    {
        nSGBufLen += pSgr->sr_sglist[nSGBufNum].sb_len;
    }

    // check total SG buffer space is enough for reqeusted size
    if (nSGBufLen < (nNumOfSec * nSizeOfSec))
    {
        STDRV_ERR_PRINT((TEXT("DoDiskRead : SG Buffer space %d bytes less than block read size %d bytes\r\n"),
			nSGBufLen, nNumOfSec * nSizeOfSec));

        status = ERROR_GEN_FAILURE;
        goto ddi_exit;
    }

    pSgr->sr_status = ERROR_IO_PENDING;

    //
    // Make sure request doesn't exceed the disk
    //
    if ((nStartSecNo + nNumOfSec - 1) > pDisk->d_DiskInfo.di_total_sectors)
    {
        status = ERROR_SECTOR_NOT_FOUND;
        goto ddi_exit;
    }
    status = ERROR_SUCCESS;

    pSg = &(pSgr->sr_sglist[0]);

    // Index of Sector Number
    nSecNoIdx       = 0;
    nSecBufOffset   = 0;
    pSecBuf         = NULL;

    STDRV_INF_PRINT((TEXT("[OND:INF]  nNumOfSG  = %d\r\n"), nNumOfSG));
    STDRV_INF_PRINT((TEXT("[OND:INF]  nSGBufLen = %d(0x%x)\r\n"), nSGBufLen, nSGBufLen));

#if ONDISK_MULTIPARTITION
	nStartSecNo += (pDisk->nWMRStartSector >> OND_SECTOR_SHIFT);  // remapping sector
#endif

    /* Number of SG is  1 (Most of cases) */
    if (nNumOfSG == 1)
    {
        nNumOfScts = nSGBufLen / (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);

        //pSGBuf = MapPtrToProcess((LPVOID)pSg->sb_buf, GetCallerProcess());
		pSGBuf = MapCallerPtr((LPVOID)pSg->sb_buf,nSGBufLen);
        if (nNumOfScts != 0)
        {
//		RETAILMSG(1, (TEXT("FTL_Write1 : nStartSecNo = 0x%x, nNumOfSects = 0x%x \r\n"), nStartSecNo << OND_SECTOR_SHIFT, nNumOfScts << OND_SECTOR_SHIFT));
            nRet = FTL_Write((nStartSecNo << OND_SECTOR_SHIFT),   /* Start Logical Sector Number  */
				(nNumOfScts << OND_SECTOR_SHIFT),    /* Number of Sectors            */
				pSGBuf);       /* Pointer ot buffer            */

            if (nRet != FTL_SUCCESS)
            {
                STDRV_ERR_PRINT((TEXT("[OND:ERR](DoDiskWrite) FTL_Write Error=0x%x\r\n"), nRet));
                status = ERROR_SECTOR_NOT_FOUND;
                goto ddi_req_done;
            }

            nStartSecNo += nNumOfScts;
        }

        /* To solve 512 byte align problem */
        if ((nSGBufLen % (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT)) != 0)
        {
            UINT  nCpResumeOffset;
            UINT  nCpLeftSize;

            /* Memory Allocation for 1 sector read */
            pSecBuf = (UCHAR*)malloc((WMR_SECTOR_SIZE << OND_SECTOR_SHIFT));

            nCpResumeOffset = nNumOfScts * (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);
            nCpLeftSize     = nSGBufLen  % (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);

            memset((UCHAR*)pSecBuf, 0xFF, (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT));
            memcpy((UCHAR*)pSecBuf, (UCHAR*)(pSGBuf + nCpResumeOffset), nCpLeftSize);

//		RETAILMSG(1, (TEXT("FTL_Write2 : nStartSecNo = 0x%x, nNumOfSects = 0x%x \r\n"), nStartSecNo, nNumOfScts));
            nRet = FTL_Write((nStartSecNo << OND_SECTOR_SHIFT),    /* Start Logical Sector Number  */
				(1 << OND_SECTOR_SHIFT),              /* Number of Sectors            */
				pSecBuf);       /* Pointer ot buffer            */

            if (nRet != FTL_SUCCESS)
            {
                STDRV_ERR_PRINT((TEXT("[OND:ERR](DoDiskWrite) FTL_Write Error=0x%x\r\n"), nRet));
                free(pSecBuf);
                status = ERROR_SECTOR_NOT_FOUND;
                goto ddi_req_done;
            }

            free(pSecBuf);
        }
    }
    /* Number of SG is greater than 1 */
    else
    {
        nDstOffset = 0;

        while (nSGBufLen)
        {
            UINT32  nNumOfScts;

            if (nSGBufLen >= PSII_MAX_READ_BUF_SIZE)
            {
                nNumOfScts = PSII_MAX_READ_BUF_SIZE / (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);
                nCpBytes   = (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts;

            }
            else
            {
                nNumOfScts = nSGBufLen / (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);
                nCpBytes   = (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts;
                if (nSGBufLen % (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT))
                {
                    nNumOfScts++;
                    nCpBytes += (nSGBufLen % (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT));
                }
            }

            pSecBuf    = (UCHAR *)malloc((WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts);
            memset(pSecBuf, 0xFF, (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts);
            nSrcOffset = 0;

            while(nCpBytes)
            {
                if (nDstOffset == 0)
                {
                    nSizeOfSGBuf = pSg->sb_len;
                    pSGBuf = MapPtrToProcess((LPVOID)pSg->sb_buf, GetCallerProcess());
                }

                if (nSizeOfSGBuf <= nCpBytes)
                {
                    memcpy((UCHAR*)(pSecBuf + nSrcOffset), (UCHAR*)(pSGBuf + nDstOffset), nSizeOfSGBuf);
                    nSrcOffset += nSizeOfSGBuf;
                    nDstOffset = 0;
                    nCpBytes  -= nSizeOfSGBuf;
                    pSg++;
                }
                else
                {
                    memcpy((UCHAR*)(pSecBuf + nSrcOffset), (UCHAR*)(pSGBuf + nDstOffset), nCpBytes);
                    nDstOffset   += nCpBytes;
                    nSizeOfSGBuf -= nCpBytes;
                    nCpBytes = 0;
                }
            }

//		RETAILMSG(1, (TEXT("FTL_Write3 : nStartSecNo = 0x%x, nNumOfSects = 0x%x \r\n"), nStartSecNo, nNumOfScts));
		
            nRet = FTL_Write((nStartSecNo << OND_SECTOR_SHIFT),   /* Start Logical Sector Number  */
				(nNumOfScts << OND_SECTOR_SHIFT),    /* Number of Sectors            */
				pSecBuf);      /* Pointer ot buffer            */

            if (nRet != FTL_SUCCESS)
            {
                STDRV_ERR_PRINT((TEXT("[OND:ERR](DoDiskWrite) FTL_Write Error=0x%x\r\n"), nRet));
                free(pSecBuf);
                status = ERROR_SECTOR_NOT_FOUND;
                goto ddi_req_done;
            }

            nStartSecNo += nNumOfScts;


            if (nSGBufLen < ((WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts))
            {
                free(pSecBuf);
                break;
            }
            else
            {
                nSGBufLen -= ((WMR_SECTOR_SIZE << OND_SECTOR_SHIFT) * nNumOfScts);
            }

            free(pSecBuf);
        }
    }

ddi_req_done:

ddi_exit:
    pSgr->sr_status = status;
    LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

    STDRV_LOG_PRINT((TEXT("[OND: IN] --DoDiskWrite()\r\n")));

    return status;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DoDiskDeleteSectors                                                  */
/* DESCRIPTION                                                               */
/*      Do delete operation from NAND flash memory                           */
/* PARAMETERS                                                                */
/*      pDisk       PocketStoreII driver own structure pointer               */
/*      pData       PSQ_REQ structure pointer,it contains request information*/
/*                  for delete operations                                    */
/* RETURN VALUES                                                             */
/*      If it successes, it returns TRUE. otherwize it returns FALSE         */
/*                                                                           */
/*****************************************************************************/
static DWORD
DoDiskDeleteSectors(PDISK pDisk,
                    PVOID pData)
{
    DWORD               status = ERROR_SUCCESS;
    UINT                iStartSector, iNumSectors;
    DELETE_SECTOR_INFO *pSecInfo;
    INT32               nRet;

    STDRV_LOG_PRINT((TEXT("[OND:LOG] ++DoDiskDeleteSectors()\r\n")));
    EnterCriticalSection(&(pDisk->d_DiskCardCrit));

    pSecInfo     = (DELETE_SECTOR_INFO *) pData;
    iStartSector = pSecInfo->startsector;
    iNumSectors  = pSecInfo->numsectors;

    if (pDisk->bIsFTLOpen == FALSE)
    {
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    /* Bound Check for StartSector */
    if (iStartSector > pDisk->d_DiskInfo.di_total_sectors ||
        iStartSector < 0)
    {
        STDRV_ERR_PRINT((TEXT("[OND:SECT DEL ERR] at iStartSector %5d\r\n"), iStartSector));
        status = ERROR_SECTOR_NOT_FOUND;
        goto ddi_exit;
    }

    /* Bound Check for FinalSector */
    if ((iStartSector + iNumSectors) > pDisk->d_DiskInfo.di_total_sectors ||
        (iStartSector + iNumSectors) < 0)
    {
        STDRV_ERR_PRINT((TEXT("[OND:SECT DEL ERR] at iEndSector %5d\r\n"), iStartSector + iNumSectors - 1));
        status = ERROR_SECTOR_NOT_FOUND;
        goto ddi_exit;
    }


    STDRV_INF_PRINT((TEXT("[OND:INF]  >>DoDiskDeleteSectors<< at sector[%5d ~ %5d]\r\n"),
		iStartSector, iStartSector + iNumSectors - 1));
    STDRV_INF_PRINT((TEXT("[OND:INF] &DD:SEC:%08d,SN:%04d\r\n"),iStartSector, iNumSectors));

    status = ERROR_SUCCESS;

#if STL_DELETE_DBG
    nRet = STL_Delete(pDisk->nVol,
		pDisk->nBmlPartitionId,
		(iStartSector << OND_SECTOR_SHIFT),
		(iNumSectors << OND_SECTOR_SHIFT));
#else
    STDRV_INF_PRINT((TEXT("[OND:INF] Call STL_Delete func. Return FTL_SUCCESS forcefully for debugging.\r\n")));
	nRet = FTL_SUCCESS;
#endif

    if (nRet != FTL_SUCCESS)
    {
        STDRV_ERR_PRINT((TEXT("[OND:SEC DEL ERR] at sector %5d to \r\n"), iStartSector, iNumSectors));
        status = ERROR_SECTOR_NOT_FOUND;
    }


ddi_exit:
    LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

	STDRV_LOG_PRINT((TEXT("[OND:LOG] --DoDiskDeleteSectors()\r\n")));
    return status;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      GetDiskInfo                                                          */
/* DESCRIPTION                                                               */
/*      Get disk information from pDisk structure                            */
/* PARAMETERS                                                                */
/*      pDisk       PocketStoreII driver own structure pointer               */
/*      pInfo       DISK Information structure pointer                       */
/* RETURN VALUES                                                             */
/*      it always returns ERROR_SUCCESS                                      */
/*                                                                           */
/*****************************************************************************/
static DWORD
GetDiskInfo(PDISK       pDisk,
            PDISK_INFO  pInfo)
{
    STDRV_LOG_PRINT((TEXT("[OND: IN] ++GetDiskInfo()\r\n")));

    EnterCriticalSection(&(pDisk->d_DiskCardCrit));

    *pInfo = pDisk->d_DiskInfo;

    LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

    STDRV_LOG_PRINT((TEXT("[OND: IN] --GetDiskInfo()\r\n")));

    return ERROR_SUCCESS;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      SetDiskInfo                                                          */
/* DESCRIPTION                                                               */
/*      Set disk information to pDisk structure                              */
/* PARAMETERS                                                                */
/*      pDisk       PocketStoreII driver own structure pointer               */
/*      pInfo       DISK Information structure pointer                       */
/* RETURN VALUES                                                             */
/*      it always returns ERROR_SUCCESS                                      */
/*                                                                           */
/*****************************************************************************/
static DWORD
SetDiskInfo(PDISK       pDisk,
            PDISK_INFO  pInfo)
{
    STDRV_LOG_PRINT((TEXT("[OND: IN] ++SetDiskInfo()\r\n")));

    pDisk->d_DiskInfo = *pInfo;

    STDRV_LOG_PRINT((TEXT("[OND:OUT] --SetDiskInfo()\r\n")));

    return ERROR_SUCCESS;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      CreateDiskObject                                                     */
/* DESCRIPTION                                                               */
/*      Create a DISK structure, init some fields and link it.               */
/* PARAMETERS                                                                */
/*      none                                                                 */
/* RETURN VALUES                                                             */
/*      new DISK structure pointer                                           */
/*                                                                           */
/*****************************************************************************/
static PDISK
CreateDiskObject(VOID)
{
    PDISK pDisk;

    STDRV_LOG_PRINT((TEXT("[OND: IN] ++CreateDiskObject()\r\n")));

    pDisk = LocalAlloc(LPTR, sizeof(DISK));

    if (pDisk != NULL)
    {
        /* Disk Handle data init */
        pDisk->hDevice          = NULL;
        pDisk->d_OpenCount      = 0;
        pDisk->d_ActivePath     = NULL;
        pDisk->DoThreading      = TRUE;
        pDisk->SecPerCluster    = 0;
        pDisk->FirstDataSector  = 0;
        pDisk->bIsFTLOpen       = FALSE;

        /* Initialize Critical Section handle for each disk handle */
        InitializeCriticalSection(&(pDisk->d_DiskCardCrit));

        pDisk->pd_next   = v_DiskList;
        v_DiskList      = pDisk;
    }

    STDRV_LOG_PRINT((TEXT("[OND:   ] --CreateDiskObject()\r\n")));

    return pDisk;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      IsValidDisk                                                          */
/* DESCRIPTION                                                               */
/*      This function checks Disk validation                                 */
/*      IsValidDisk - verify that pDisk points to something in our list      */
/* PARAMETERS                                                                */
/*      pDisk                                                                */
/*          Pointer to disk handle                                           */
/* RETURN VALUES                                                             */
/*      Return TRUE if pDisk is valid, FALSE if not.                         */
/*                                                                           */
/*****************************************************************************/
static BOOL
IsValidDisk(PDISK pDisk)
{
    PDISK   pd;
    BOOL    bRet = FALSE;

    STDRV_LOG_PRINT((TEXT("[OND: IN] ++IsValidDisk()\r\n")));

    pd = v_DiskList;
    while (pd)
    {
        if (pd == pDisk)
        {
            bRet = TRUE;
            break;
        }
        pd = pd->pd_next;
    }

    STDRV_LOG_PRINT((TEXT("[OND:OUT] --IsValidDisk()\r\n")));

    return bRet;

}   // IsValidDisk


//#define _SUPPORT_HAL_WRAPPER_

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      InitializeNAND                                                       */
/* DESCRIPTION                                                               */
/*      This function initializes NAND Disk Handle                           */
/* PARAMETERS                                                                */
/*      pDisk                                                                */
/*          Disk handle                                                      */
/* RETURN VALUES                                                             */
/*      Return TRUE if pDisk is valid, FALSE if not.                         */
/* NOTE                                                                      */
/*      call from InitDisk from DSK_Init                                     */
/*                                                                           */
/*****************************************************************************/
static BOOL
InitializeNAND(PDISK pDisk)
{
#if defined(_SUPPORT_HAL_WRAPPER_)
    VFLPacket   stPacket;
    INT         nResult;
#endif
    BOOL        bRet = FALSE;

    STDRV_LOG_PRINT((TEXT("[OND: IN] ++InitializeNAND()\r\n")));

#if defined(_SUPPORT_HAL_WRAPPER_)
    do {
        STDRV_LOG_PRINT((TEXT("[OND: IN] _SUPPORT_HAL_WRAPPER_\r\n")));

        /* VFL Init */
        stPacket.nCtrlCode = PM_HAL_VFL_INIT;

        KernelIoControl(IOCTL_POCKETSTOREII_CMD,  /* Io Control Code */
                        &stPacket,                /* Input buffer (Additional Control Code) */
                        sizeof(stPacket),         /* Size of Input buffer */
                        NULL,                     /* Output buffer */
                        0,                        /* Size of Output buffer */
                        &nResult);                /* Error Return */

        if (nResult != VFL_SUCCESS)
        {
            STDRV_ERR_PRINT((TEXT("[OND:ERR]  VFL_Init() failure. ERR Code=%x\r\n"), nResult));

            break;
        }

        /* WFL Open */
        stPacket.nCtrlCode = PM_HAL_VFL_OPEN;

        KernelIoControl(IOCTL_POCKETSTOREII_CMD,  /* Io Control Code */
                        &stPacket,                /* Input buffer (Additional Control Code */
                        sizeof(stPacket),         /* Size of Input buffer */
                        NULL,                     /* Output buffer */
                        0,                        /* Size of Output buffer */
                        &nResult);                /* Error Return */

        if (nResult != VFL_SUCCESS)
        {
            STDRV_ERR_PRINT((TEXT("[OND:ERR]  VFL_Open() failure. ERR Code=%x\r\n"), nResult));

            break;
        }

        bRet = TRUE;

    } while(0);

#else   //_SUPPORT_HAL_WRAPPER_

    do {
        STDRV_LOG_PRINT((TEXT("[OND: IN] not _SUPPORT_HAL_WRAPPER_\r\n")));

        if (FIL_Init() != VFL_SUCCESS)
        {
            STDRV_ERR_PRINT((TEXT("[OND:ERR]  FIL_Init() failure.\r\n")));

            break;
        }

        if (VFL_Init() != VFL_SUCCESS)
        {
            STDRV_ERR_PRINT((TEXT("[OND:ERR]  VFL_Init() failure.\r\n")));

            break;
        }

        if (VFL_Open() != VFL_SUCCESS)
        {
            STDRV_ERR_PRINT((TEXT("[OND:ERR]  VFL_Open() failure.\r\n")));

            break;
        }

        bRet = TRUE;

    } while(0);

#endif  //_SUPPORT_HAL_WRAPPER_

    STDRV_LOG_PRINT((TEXT("[OND:OUT] --InitializeNAND()\r\n")));

    return bRet;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      InitializeFTL                                                        */
/* DESCRIPTION                                                               */
/*      This function initializes Disk handle                                */
/* PARAMETERS                                                                */
/*      pDisk                                                                */
/*          Disk handle                                                      */
/*      ActiveKey                                                            */
/*          Pointer to active key                                            */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      call from DSK_Init (actually InitDisk)                               */
/*                                                                           */
/*****************************************************************************/
static BOOL
InitializeFTL(PDISK pDisk)
{
	INT         nSTLRet;
	BOOL        bIsFormatNeeded;
	INT         nTryCnt;
	BOOL        bRet = FALSE;
	UINT32      TotalScts;

	STDRV_LOG_PRINT((TEXT("[OND: IN] ++InitializeFTL()\r\n")));

	bIsFormatNeeded = FALSE;

	nTryCnt = 10;

	while (nTryCnt --)
	{
		if (nTryCnt < 0)
		{
			STDRV_ERR_PRINT((L"[OND:   ]  FTL_Open try is 10 times. "
			L"ERROR:too many try..\r\n"));
			goto InitializeFTLError;
		}

		nSTLRet = FTL_Init();

		if (nSTLRet == FTL_CRITICAL_ERROR)
		{
			STDRV_ERR_PRINT((TEXT("[OND:ERR]  FTL_Init : errno = %d\r\n"), nSTLRet));
		}

		if (GetBmlPartitionId(pDisk) == FALSE)
		{
			STDRV_ERR_PRINT((TEXT("[OND:ERR]  GetBmlPartitionId() Error\r\n")));
			break;
		}

		STDRV_LOG_PRINT((TEXT("[OND:   ]  %dth Init FTL Try\r\n"), 10 - nTryCnt));
		STDRV_LOG_PRINT((TEXT("[OND:   ]  FTL_Init Success (RetCode = %d)\r\n"), nSTLRet));

		TotalScts                 = 0;  /* Output : The number of total logical sectors */

		/* Input  : size of buffer for SAM (percentage) */
		/*         if nSamBufFactor == 0, STL factor set*/
		/*         it as 100                            */

		/* FTL Open */
		nSTLRet = FTL_Open(&TotalScts);

		if (nSTLRet == FTL_SUCCESS)
		{
			bIsFormatNeeded = FALSE;
			STDRV_INF_PRINT((TEXT("[OND:INF]  TotalLogScts   = %d\r\n"), TotalScts));

#if ONDISK_MULTIPARTITION
			if (pDisk->nWMRNumOfSector == 0xFFFFFFFF)
			{
				pDisk->nWMRNumOfSector = TotalScts - pDisk->nWMRStartSector;  // calculate remained sector on multiple partition
			}
			else if (pDisk->nWMRNumOfSector > TotalScts)
			{
				STDRV_ERR_PRINT((TEXT("[OND:ERR] WMRNumOfSector size(%u) exceeded total sectors(%u).\r\n"), pDisk->nWMRNumOfSector, TotalScts));
				pDisk->nWMRNumOfSector = TotalScts;
				STDRV_ERR_PRINT((TEXT("[OND:ERR] WMRNumOfSector sets total sectors now.\r\n")));
			}

			pDisk->d_DiskInfo.di_total_sectors  = (pDisk->nWMRNumOfSector >> OND_SECTOR_SHIFT);
#else
			pDisk->d_DiskInfo.di_total_sectors  = (TotalScts >> OND_SECTOR_SHIFT);
#endif
			pDisk->d_DiskInfo.di_bytes_per_sect = (WMR_SECTOR_SIZE << OND_SECTOR_SHIFT);
			pDisk->d_DiskInfo.di_cylinders		= 0;
			pDisk->d_DiskInfo.di_heads			= 0;
			pDisk->d_DiskInfo.di_sectors		= 0;
			pDisk->d_DiskInfo.di_flags			=
			DISK_INFO_FLAG_CHS_UNCERTAIN | DISK_INFO_FLAG_PAGEABLE;

			STDRV_INF_PRINT((TEXT("[OND:	 ] total_sectors  = %d\r\n"), pDisk->d_DiskInfo.di_total_sectors));
			STDRV_INF_PRINT((TEXT("[OND:	 ] bytes_per_sect = %d\r\n"), pDisk->d_DiskInfo.di_bytes_per_sect));
			STDRV_INF_PRINT((TEXT("[OND:	 ] storage size   = %d\r\n"),
			pDisk->d_DiskInfo.di_total_sectors * pDisk->d_DiskInfo.di_bytes_per_sect));

			bRet = TRUE;
			break;  // exit while (1)
		}
		else if (nSTLRet == FTL_CRITICAL_ERROR)
		{
			STDRV_ERR_PRINT((TEXT("[OND:ERR] FTL_Open occurs Critical Error\r\n")));
			bIsFormatNeeded = TRUE;
		}
		else
		{
			STDRV_ERR_PRINT((TEXT("[OND:   ] FTL_Open unknown error\r\n")));
			goto InitializeFTLError;
		}

		STDRV_INF_PRINT((TEXT("[OND:INF] just before FTL formatted\r\n")));

		/* FTL_Format */
		if (bIsFormatNeeded == TRUE)
		{
			STDRV_INF_PRINT((TEXT("[OND:INF]  FTL_Format Start\r\n")));

			nSTLRet = FTL_Format();

			if (nSTLRet != FTL_SUCCESS)
			{
				STDRV_ERR_PRINT((TEXT("[OND:ERR]  FTL_Format Error = 0x%x\r\n"), nSTLRet));

				if (IsAPIReady(SH_WMGR) == TRUE)
				{
					MessageBoxW(NULL,
					TEXT("Can't be formatted"),
					TEXT("Low Format"),
					MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
				}
				goto InitializeFTLError;
			}
			else
			{
				STDRV_INF_PRINT((TEXT("[OND:INF]  FTL_Format Complete\r\n")));

				// reset bIsFormatNeeded flag to FALSE.
				bIsFormatNeeded = FALSE;
			}
		}
	}

InitializeFTLError:

    return bRet;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      InitDisk                                                             */
/* DESCRIPTION                                                               */
/*      This function initializes Disk handle                                */
/* PARAMETERS                                                                */
/*      pDisk                                                                */
/*          Disk handle                                                      */
/*      ActiveKey                                                            */
/*          Pointer to active key                                            */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      call from DSK_Init                                                   */
/*                                                                           */
/*****************************************************************************/
static BOOL
InitDisk(PDISK  pDisk,
         LPTSTR ActiveKey)
{
	BOOL         bRet = FALSE;

	//
	// initialization by determining the NAND Flash capacity
	//
	pDisk->fBusy = FALSE;

	STDRV_LOG_PRINT((TEXT("[OND: IN] ++InitDisk()\r\n")));

	if (InitializeNAND(pDisk) == FALSE)
	{
		pDisk->bIsFTLOpen = FALSE;
		STDRV_ERR_PRINT((TEXT("[OND:ERR]  InitializeNAND Error\r\n")));
	}
	else
	{
		if (InitializeFTL(pDisk) == FALSE)
		{
			pDisk->bIsFTLOpen = FALSE;
			STDRV_ERR_PRINT((TEXT("[OND:ERR]  InitializeFTL Error\r\n")));
		}
		else
		{
			pDisk->bIsFTLOpen = TRUE;
			bRet = TRUE;
		}
	}

	return bRet;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_Init                                                             */
/* DESCRIPTION                                                               */
/*      Create Disk Object, Initialize Disk Object                           */
/* PARAMETERS                                                                */
/*      dwContext   STDRVIF_PS driver own structure pointer                   */
/*                  registry path for this device's active key               */
/* RETURN VALUES                                                             */
/*      Returns context data for this Init instance                          */
/*                                                                           */
/*****************************************************************************/
DWORD
DSK_Init(DWORD dwContext)
{
    PDISK  pDisk;                           /* Disk Handle */
    LPWSTR ActivePath  = (LPWSTR)dwContext;
    LPWSTR ActivePath2 = (LPWSTR)dwContext;

    STDRV_LOG_PRINT((TEXT("[OND: IN] ++DSK_Init()\r\n")));

    EnterCriticalSection(&v_DiskCrit);

    if (v_DiskList == NULL)
    {
        STDRV_RTL_PRINT((TEXT("[OND:INF]  InitializeCriticalSection(&v_DiskCrit)\r\n")));
    }

    /* Disk Handle Creation */
    pDisk = CreateDiskObject();
    if (pDisk == NULL)
    {
        STDRV_ERR_PRINT((TEXT("[OND:ERR]  CreateDiskObject(PDISK) failed %d\r\n"), GetLastError()));
        return (DWORD)0;
    }

    /* if dwContext != (DWORD)NULL */
    if (ActivePath)
    {
        /* for using loader                     */
        /* get actual ActivePath memory address */
        //ActivePath2 = (LPWSTR) MapPtrToProcess((LPVOID)ActivePath, GetCallerProcess());
        ActivePath2 = (LPWSTR) MapCallerPtr((LPVOID)ActivePath,wcslen(ActivePath) * sizeof(WCHAR) + sizeof(WCHAR));

        if (NULL != ActivePath2)
        {
            ActivePath = ActivePath2;
        }

        /* Copy Active Path to pDisk Handle */
        if (pDisk->d_ActivePath = LocalAlloc(LPTR, wcslen(ActivePath) * sizeof(WCHAR) + sizeof(WCHAR)))
        {
            wcscpy(pDisk->d_ActivePath, ActivePath);
        }

        STDRV_INF_PRINT((TEXT("[OND:INF] : ActiveKey (copy) = %s (@ 0x%08X)\r\n"), pDisk->d_ActivePath, pDisk->d_ActivePath));
    }

    /* Initialize pDisk Handle */
    if (InitDisk(pDisk, ActivePath) == FALSE)
    {
        if (pDisk)
        {
            if (pDisk->d_ActivePath)
            {
                LocalFree(pDisk->d_ActivePath);
                pDisk->d_ActivePath = NULL;
            }
            CloseDisk(pDisk);
        }
        return 0;
    }

    // ToDo, FATCleanser Initialization code
    LeaveCriticalSection(&v_DiskCrit);

    STDRV_LOG_PRINT((TEXT("[OND:OUT] --DSK_Init() returning 0x%x\r\n"), pDisk));

    return (DWORD)pDisk;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_Close                                                            */
/* DESCRIPTION                                                               */
/*      This function Close Disk                                             */
/* PARAMETERS                                                                */
/*      Handle                                                               */
/*          Disk handle                                                      */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      call from DSK_Deinit                                                 */
/*                                                                           */
/*****************************************************************************/
BOOL
DSK_Close(DWORD Handle)
{
    PDISK pDisk = (PDISK)Handle;
    BOOL bClose = FALSE;
    BOOL bRet   = FALSE;

    STDRV_LOG_PRINT((TEXT("[OND: IN] ++DSK_Close()\r\n")));

    EnterCriticalSection(&v_DiskCrit);

    do {
        if (!IsValidDisk(pDisk))
        {
            break;
        }

        pDisk->d_OpenCount --;
        if (pDisk->d_OpenCount <= 0)
        {
            pDisk->d_OpenCount = 0;
        }


        bRet = TRUE;

    } while(0);

    LeaveCriticalSection(&v_DiskCrit);

    STDRV_LOG_PRINT((TEXT("[OND:OUT] --DSK_Close{}\r\n")));

    return bRet;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_Deinit                                                           */
/* DESCRIPTION                                                               */
/*      This function deinitializes Disk                                     */
/* PARAMETERS                                                                */
/*      dwContext                                                            */
/*          Disk handle                                                      */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      Device deinit - devices are expected to close down.                  */
/*      The device manager does not check the return code.                   */
/*                                                                           */
/*****************************************************************************/
BOOL
DSK_Deinit(DWORD dwContext)     // future: pointer to the per disk structure
{
    PDISK pDisk = (PDISK) dwContext;

    BOOL  bRet = FALSE;
    INT   nRet;

    STDRV_LOG_PRINT((TEXT("[OND: IN] ++DSK_Deinit()\r\n")));
    EnterCriticalSection(&v_DiskCrit);

    do {

        DSK_Close(dwContext);

        CloseDisk((PDISK)dwContext);

        // FTL_Close
        if (pDisk->bIsFTLOpen == TRUE)
        {
            nRet = FTL_Close();

            if (nRet != FTL_SUCCESS)
            {
                STDRV_ERR_PRINT((TEXT("[OND:ERR]  FTL_Close Error=0x%x"), nRet));
                break;
            }

            pDisk->bIsFTLOpen = FALSE;    //+ ksk 20061108
        }

        pDisk->DoThreading = FALSE;
        bRet = TRUE;

    } while(0);
    LeaveCriticalSection(&v_DiskCrit);

    STDRV_LOG_PRINT((TEXT("[OND:OUT] --DSK_Deinit()\r\n")));

    return bRet;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_Open                                                             */
/* DESCRIPTION                                                               */
/*      This function opens Disk                                             */
/* PARAMETERS                                                                */
/*      dwData                                                               */
/*          Disk handle                                                      */
/*      dwAccess                                                             */
/*          Not used                                                         */
/*      dwShareMode                                                          */
/*          Not used                                                         */
/* RETURN VALUES                                                             */
/*      Return address of pDisk(disk handle)                                 */
/* NOTE                                                                      */
/*                                                                           */
/*****************************************************************************/
DWORD
DSK_Open(DWORD dwData,
         DWORD dwAccess,
         DWORD dwShareMode)
{
    PDISK pDisk = (PDISK)dwData;
    DWORD dwRet = 0;
	INT32 	nRet;

    STDRV_LOG_PRINT((TEXT("[OND: IN] ++DSK_Open(0x%x)\r\n"),dwData));
    EnterCriticalSection(&v_DiskCrit);

    do {
        if (IsValidDisk(pDisk) == FALSE)
        {
            STDRV_ERR_PRINT((TEXT("[OND:ERR] DSK_Open - Passed invalid disk handle\r\n")));
            break;
        }

        EnterCriticalSection(&(pDisk->d_DiskCardCrit));
        {
            pDisk->d_OpenCount ++;
        }
        LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

        dwRet = (DWORD)pDisk;

		#if (WMR_READ_RECLAIM)
		nRet = FTL_ReadReclaim();

		if (nRet != FTL_SUCCESS)
		{
            STDRV_ERR_PRINT((TEXT("[OND:ERR] FTL_ReadReclaim failed on DSK_Open\r\n")));
			//return WMR_CRITICAL_ERROR;
		}

		STDRV_RTL_PRINT((TEXT("[OND:MSG] FTL_ReadReclaim		[OK]\n")));
		#endif

    } while(0);

    STDRV_LOG_PRINT((TEXT("[OND:OUT] --DSK_Open(0x%x) returning %d\r\n"),dwData, dwRet));
    LeaveCriticalSection(&v_DiskCrit);

    return dwRet;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DSK_IOControl                                                        */
/* DESCRIPTION                                                               */
/*      This function is Disk IO Control                                     */
/* PARAMETERS                                                                */
/*      Handle                                                               */
/*          Disk handle                                                      */
/*      dwIoControlCode                                                      */
/*          IO Control Code                                                  */
/*      pInBuf                                                               */
/*          Pointer to input buffer                                          */
/*      nInBufSize                                                           */
/*          Size of input buffer                                             */
/*      pOutBuf                                                              */
/*          Pointer to output buffer                                         */
/*      nOutBufSize                                                          */
/*          Size of output buffer                                            */
/*      pBytesReturned                                                       */
/*          Pointer to byte returned                                         */
/* RETURN VALUES                                                             */
/*      Return TRUE if Operation success, FALSE if not.                      */
/* NOTE                                                                      */
/*      I/O Control function - responds to info, read and write control codes*/
/*      The read and write take a scatter/gather list in pInBuf              */
/*                                                                           */
/*****************************************************************************/
#if defined(RESERVED_REGION_SUPPORT)
#include <fmd.h>
#define NUM_RESERVES		2
#define RESERVE_BLOCK_SIZE	256*512
ReservedEntry stRsvTable[NUM_RESERVES] = { { "GSMFFS", 0, 1 }, { "GSM", 0, 32 } };
#endif

BOOL
DSK_IOControl(DWORD  Handle,
              DWORD  dwIoControlCode,
              PBYTE  pInBuf,
              DWORD  nInBufSize,
              PBYTE  pOutBuf,
              DWORD  nOutBufSize,
              PDWORD pBytesReturned)
{
    PSG_REQ pSG;
    PDISK   pDisk = (PDISK)Handle;
    BOOL    bRes  = TRUE;

    STDRV_LOG_PRINT((TEXT("[OND:INFO] ++DSK_IOControl()\r\n")));
    EnterCriticalSection(&v_DiskCrit);

    if (IsValidDisk(pDisk) == FALSE)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        STDRV_ERR_PRINT((TEXT("[OND:ERR]   DSK_IOControl (invalid disk) \r\n")));
        return FALSE;
    }

    //
    // Check parameters
    //
    switch (dwIoControlCode)
    {
    case DISK_IOCTL_READ:
    case IOCTL_DISK_READ:

    case DISK_IOCTL_WRITE:
    case IOCTL_DISK_WRITE:

	case DISK_IOCTL_GETINFO:

    case DISK_IOCTL_SETINFO:
    case IOCTL_DISK_SETINFO:

    case IOCTL_DISK_DELETE_CLUSTER:
    case IOCTL_DISK_DELETE_SECTORS:

    case IOCTL_DISK_DEVICE_INFO:

    case DISK_IOCTL_INITIALIZED:
        if (!pInBuf)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        break;

    case IOCTL_DISK_GETINFO:

    case DISK_IOCTL_GETNAME:
    case IOCTL_DISK_GETNAME:

    case IOCTL_DISK_GET_STORAGEID:
		if (!pOutBuf)
		{
			STDRV_ERR_PRINT((TEXT("[OND:ERR] Get_Info/Name/StorageId pOutBuf=NULL\r\n")));
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		}
		break;

    case DISK_IOCTL_FORMAT_MEDIA:
    case IOCTL_DISK_FORMAT_MEDIA:
		break;
#if defined(RESERVED_REGION_SUPPORT)
	case IOCTL_FMD_GET_RESERVED_TABLE:
		if (!pOutBuf)
		{
			*pBytesReturned = sizeof(stRsvTable);
			return TRUE;
		} else {
			if (nOutBufSize != sizeof(stRsvTable))
				return FALSE;
		}
		break;

	case IOCTL_FMD_READ_RESERVED:
	case IOCTL_FMD_WRITE_RESERVED:
		if (!pInBuf || nInBufSize < sizeof(ReservedReq))
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		}
		break;
#endif
    case IOCTL_DISK_FLUSH_CACHE:
        // do nothing
        return TRUE;

    default:
        STDRV_ERR_PRINT((TEXT("[OND:ERR]  DSK_IOControl unkonwn code(%04x)\r\n"),
			dwIoControlCode));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    STDRV_INF_PRINT((TEXT("[OND:INF]  DSK_IOControl Code=(%04x)\r\n"), dwIoControlCode));

/*	This definitions are defined in diskio.h file.
#define DISK_IOCTL_GETINFO      1
#define DISK_IOCTL_SETINFO      5
#define DISK_IOCTL_READ         2
#define DISK_IOCTL_WRITE        3
#define DISK_IOCTL_INITIALIZED  4
#define DISK_IOCTL_FORMAT_MEDIA 6
#define DISK_IOCTL_GETNAME      9
*/
    /*--------------------------*/
    /* Execute dwIoControlCode  */
    /*--------------------------*/
    switch (dwIoControlCode)
    {
    case DISK_IOCTL_READ:
    case IOCTL_DISK_READ:
        pSG = (PSG_REQ)pInBuf;
        pDisk->fBusy = TRUE;
        DoDiskRead(pDisk, (PVOID)pSG);
        pDisk->fBusy = FALSE;

        if (ERROR_SUCCESS != pSG->sr_status)
        {
            SetLastError(pSG->sr_status);
            bRes = FALSE;
        }
        else
        {
            bRes = TRUE;
        }
        break;

    case DISK_IOCTL_WRITE:
    case IOCTL_DISK_WRITE:
        pSG = (PSG_REQ)pInBuf;
        pDisk->fBusy = TRUE;
        DoDiskWrite(pDisk, (PVOID)pSG);
        pDisk->fBusy = FALSE;

        if (ERROR_SUCCESS != pSG->sr_status)
        {
            SetLastError(pSG->sr_status);
            bRes = FALSE;
        }
        else
        {
            bRes = TRUE;
        }
        break;

    case IOCTL_DISK_DELETE_SECTORS:
        pDisk->fBusy = TRUE;
        DoDiskDeleteSectors(pDisk, (PVOID) pInBuf);
        pDisk->fBusy = FALSE;

        bRes = TRUE;
        break;

    case DISK_IOCTL_GETINFO:
        SetLastError(GetDiskInfo(pDisk, (PDISK_INFO) pInBuf));
        bRes = TRUE;
        break;

    case IOCTL_DISK_GETINFO:
        SetLastError(GetDiskInfo(pDisk, (PDISK_INFO) pOutBuf));
        bRes = TRUE;
        break;

    case DISK_IOCTL_SETINFO:
    case IOCTL_DISK_SETINFO:
        SetLastError(SetDiskInfo(pDisk, (PDISK_INFO) pInBuf));
        bRes = TRUE;
        break;

    case DISK_IOCTL_INITIALIZED:
        if (IsStorageManagerRunning())
        {
            STDRV_ERR_PRINT(
				(TEXT("[OND:INF]  Not loading FSD since StorageManager is active\r\n")));
        }
        else
        {
            STDRV_RTL_PRINT((TEXT("[OND:MSG]  loading FSD\r\n")));
        }
        bRes = TRUE;
        break;

    case DISK_IOCTL_GETNAME:
    case IOCTL_DISK_GETNAME:
        bRes = GetFolderName(pDisk, (LPWSTR) pOutBuf, nOutBufSize, pBytesReturned);
        if (bRes == TRUE)
            STDRV_RTL_PRINT((TEXT("[OND:MSG]  bRes == TRUE \r\n")));
        else
            STDRV_RTL_PRINT((TEXT("[OND:MSG]  bRes == FALSE \r\n")));
        break;

    case IOCTL_DISK_DEVICE_INFO:
        bRes = GetDeviceInfo(pDisk, (PSTORAGEDEVICEINFO) pInBuf);
        *pBytesReturned = sizeof(STORAGEDEVICEINFO);
        break;

    case IOCTL_DISK_GET_STORAGEID:
        bRes = GetStorageID(pDisk, (PSTORAGE_IDENTIFICATION) pOutBuf,
			nOutBufSize, pBytesReturned);
        break;

    case DISK_IOCTL_FORMAT_MEDIA:
    case IOCTL_DISK_FORMAT_MEDIA:
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  +-------------------------+\r\n")));
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  | IOCTL_DISK_FORMAT_MEDIA |\r\n")));
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  +-------------------------+\r\n")));
        STDRV_RTL_PRINT((TEXT("<log P1=\"100\" P2=\"FORMAT\" />\n")));
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  Volume Number(pDisk->nVol) = %d\r\n"), pDisk->nVol));
        STDRV_RTL_PRINT((TEXT("[OND:MSG]  Partition ID(pDisk->nBmlPartitionId) = %d\r\n"), pDisk->nBmlPartitionId));
		
#if 0
		{
			INT nRet;
			UINT32 nTotalSector;

			STDRV_LOG_PRINT((TEXT("[OND:INF]  FTL_Format(sec:0 ~ sec:%d)\r\n"),
				((pDisk->d_DiskInfo.di_total_sectors) << OND_SECTOR_SHIFT)));

			if (FTL_Close() == FTL_SUCCESS)
			{
				STDRV_ERR_PRINT((TEXT("[OND:INF]  FTL_Close() success\r\n")));
			}
			else
			{
				STDRV_ERR_PRINT((TEXT("[OND:INF]  FTL_Close() failure\r\n")));
				bRes = FALSE;
				break;
			}

			nRet = WMR_Format_FTL();
			if (nRet == TRUE32)
			{
				STDRV_ERR_PRINT((TEXT("[OND:INF]  FTL_Format() success\r\n")));
				bRes = TRUE;
			}
			else
			{
				STDRV_ERR_PRINT((TEXT("[OND:ERR]  FTL_Format() failure\r\n")));
				bRes = FALSE;
				break;
			}

			if (FTL_Open(&nTotalSector) == FTL_SUCCESS)
			{
				STDRV_ERR_PRINT((TEXT("[OND:INF]  FTL_Open() success\r\n")));
			}
			else
			{
				STDRV_ERR_PRINT((TEXT("[OND:INF]  FTL_Open() failure\r\n")));
			}
		}
#else
			bRes = TRUE;
#endif
		break;
#if defined(RESERVED_REGION_SUPPORT)
    case IOCTL_FMD_GET_RESERVED_TABLE:
        {
            memcpy(pOutBuf , stRsvTable, sizeof(stRsvTable));
            *pBytesReturned = NUM_RESERVES*sizeof(ReservedEntry);
            bRes = TRUE;
            STDRV_RTL_PRINT((TEXT("[OND:MSG] IOCTL_FMD_GET_RESERVED_TABLE\r\n")));

            break;
        }

    case IOCTL_FMD_READ_RESERVED:
    case IOCTL_FMD_WRITE_RESERVED:
        {
            XSRPartEntry stPartEntry;
            UINT32 nBmlErr, nStartVbn, nEndVbn, nStartVsn, nScts, nIndex, i;
            ReservedReq *pReservedReq = (ReservedReq*) pInBuf;

            STDRV_RTL_PRINT((L"[OND:MSG]  ++%s\r\n",
                (dwIoControlCode == IOCTL_FMD_READ_RESERVED ?
                L"IOCTL_FMD_READ_RESERVED" : L"IOCTL_FMD_WRITE_RESERVED") ));
            STDRV_RTL_PRINT((L"pReservedReq->szName=\"%hs\"\r\n",
                pReservedReq->szName));

            // look up requested name on internal name table
            for (i = 0, nIndex = NUM_RESERVES; i < NUM_RESERVES; i++)
            {
                STDRV_RTL_PRINT((L"stRsvTable[%d].szName=\"%hs\"\r\n",
                    i, stRsvTable[i].szName));
                if (_stricmp(pReservedReq->szName, stRsvTable[i].szName) == 0)
                    nIndex = i;
            }

            STDRV_RTL_PRINT( (L"nIndex=%d\r\n", nIndex));

            if (nIndex == NUM_RESERVES)
            {
                SetLastError(ERROR_FILE_NOT_FOUND);
                bRes = FALSE;
                break;
            }

            // parameter check
            if ((pReservedReq->dwLen % RESERVE_BLOCK_SIZE != 0) ||
                pReservedReq->dwStart != 0)
            {
                STDRV_ERR_PRINT((L"invalid parameter : dwLen=%08x, dwStart=%08x\r\n",
                    pReservedReq->dwLen, pReservedReq->dwStart));
                SetLastError(ERROR_INVALID_PARAMETER);
                bRes = FALSE;
                break;
            }

            // query BML partition info for GSM
            nBmlErr = BML_LoadPIEntry(pDisk->nVol, PARTITION_ID_RESERVED, &stPartEntry);

            if (nBmlErr != BML_SUCCESS)
            {
                STDRV_ERR_PRINT((TEXT("[OND:ERR]  BML_LoadEntry fail\r\n")));
                SetLastError(ERROR_DISK_OPERATION_FAILED);
                bRes = FALSE;
                break;
            }

            // look up the region on gsm partition
            nStartVbn = stPartEntry.n1stVbn;
            nStartVsn = stPartEntry.n1stVbn*256;

            for (i = 0; i < nIndex; i++)
            {
                nStartVbn += stRsvTable[i].dwNumBlocks;
                nStartVsn += stRsvTable[i].dwNumBlocks*256;
            }
            nEndVbn = nStartVbn + stRsvTable[nIndex].dwNumBlocks;
            nScts = pReservedReq->dwLen/512;

            STDRV_RTL_PRINT((L"nStartVbn=%08x, nEndVbn=%08x, nStartVsn=%08x\r\n"
                L"dwLen=%08x, dwStart=%08x, nScts=%08x\r\n",
                nStartVbn, nEndVbn, nStartVsn,
                pReservedReq->dwLen, pReservedReq->dwStart, nScts));

            if (dwIoControlCode == IOCTL_FMD_READ_RESERVED)
            {
                // read the entire region
                nBmlErr = BML_MRead(pDisk->nVol, nStartVsn, nScts,
					pReservedReq->pBuffer, NULL,
					BML_FLAG_SYNC_OP | BML_FLAG_ECC_ON);
            } else {
                for (i = nStartVbn; i < nEndVbn; i++)
                {
                    nBmlErr = BML_EraseBlk(pDisk->nVol, i, BML_FLAG_SYNC_OP);
                    if (nBmlErr != BML_SUCCESS) break;
                }

                if (nBmlErr == BML_SUCCESS)
                {
                    for (i = 0; i < nScts; i += 4)
                    {
                        nBmlErr = BML_Write(pDisk->nVol, nStartVsn+i, 4,
                                            pReservedReq->pBuffer+i*512, NULL,
                                            BML_FLAG_SYNC_OP | BML_FLAG_ECC_ON);
                        if (nBmlErr != BML_SUCCESS) break;
                    }
                }
            }

            if (nBmlErr != BML_SUCCESS)
            {
                STDRV_ERR_PRINT((L"[OND:ERR]  BML_Read/EraseBlk/Write error\r\n"));
                SetLastError(ERROR_DISK_OPERATION_FAILED);
                bRes = FALSE;
                break;
            }

            bRes = TRUE;

            STDRV_RTL_PRINT((L"[OND:MSG]  --%s\r\n",
                (dwIoControlCode == IOCTL_FMD_READ_RESERVED ?
                L"IOCTL_FMD_READ_RESERVED" : L"IOCTL_FMD_WRITE_RESERVED") ));
            break;
        }
#endif
    default:
        STDRV_LOG_PRINT((TEXT("-DSK_IOControl (default) \r\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        bRes = FALSE;
        break;
    }
    LeaveCriticalSection(&v_DiskCrit);

    return bRes;
}


VOID DSK_PowerUp(VOID)
{
	INT32 	nRet;

    STDRV_LOG_PRINT((TEXT("[OND:LOG]  DSK_PowerUp - Do nothing\r\n")));

	#if (WMR_READ_RECLAIM)
	nRet = FTL_ReadReclaim();

	if (nRet != FTL_SUCCESS)
	{
        STDRV_ERR_PRINT((TEXT("[OND:ERR] FTL_ReadReclaim failed on DSK_PowerUp\r\n")));
		//return WMR_CRITICAL_ERROR;
	}

	STDRV_RTL_PRINT((TEXT("[OND:MSG] FTL_ReadReclaim		[OK]\n")));
	#endif

	return;
}


VOID DSK_PowerDown(VOID) 
{
    STDRV_LOG_PRINT((TEXT("[OND:LOG]  DSK_PowerDown - Do nothing\r\n")));
}


DWORD
DSK_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
    STDRV_LOG_PRINT((TEXT("[OND:LOG]  DSK_Read - Do nothing\r\n")));
    return 0;
}


DWORD
DSK_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    STDRV_LOG_PRINT((TEXT("[OND:LOG]  DSK_Write - Do nothing\r\n")));
    return 0;
}


DWORD
DSK_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
    STDRV_LOG_PRINT((TEXT("[OND:LOG]  DSK_Seek - Do nothing\r\n")));
    return 0;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      DllMainCRTStartup                                                       */
/* DESCRIPTION                                                               */
/*      This function is PocketStoreII DLL Entry Point                       */
/* PARAMETERS                                                                */
/*      DllInstance                                                          */
/*      Reason                                                               */
/*      Reserved                                                             */
/* RETURN VALUES                                                             */
/*      it always returns TRUE                                               */
/*                                                                           */
/*****************************************************************************/
BOOL WINAPI
DllMainCRTStartup(HINSTANCE DllInstance, ULONG Reason, LPVOID Reserved)
{
    switch(Reason) {
	case DLL_PROCESS_ATTACH:
		STDRV_LOG_PRINT((TEXT("[OND:   ] DLL_PROCESS_ATTACH\r\n")));
        InitializeCriticalSection(&v_DiskCrit);
		DEBUGREGISTER(DllInstance);
		break;

	case DLL_PROCESS_DETACH:
		STDRV_LOG_PRINT((TEXT("[OND:   ] DLL_PROCESS_DETACH\r\n")));
		DeleteCriticalSection(&v_DiskCrit);
		break;
    }
    return TRUE;
}

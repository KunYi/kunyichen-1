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
#include <atamain.h>
#include <bsp_cfg.h>
/*++

Module Name:
    diskmain.cpp

Abstract:
    Base ATA/ATAPI device abstraction.

Revision History:

--*/

// 48-bit LBA support
#define  ATA_CMD_FLUSH_CACHE_EXT                0xEA
#define  ATA_CMD_READ_DMA_EXT                   0x25
#define  ATA_CMD_READ_DMA_QUEUED_EXT            0x26
#define  ATA_CMD_READ_LOG_EXT                   0x2F
#define  ATA_CMD_READ_MULTIPLE_EXT              0x29
#define  ATA_CMD_READ_NATIVE_MAX_ADDRESS_EXT    0x27
#define  ATA_CMD_READ_SECTOR_EXT                0x24
#define  ATA_CMD_READ_VERIFY_SECTOR             0x42
#define  ATA_CMD_SET_MAX_ADDRESS_EXT            0x37
#define  ATA_CMD_WRITE_DMA_EXT                  0x35
#define  ATA_CMD_DMA_QUEUED_EXT                 0x36
#define  ATA_CMD_WRITE_LOG_EXT                  0x3F
#define  ATA_CMD_WRITE_MULTIPLE_EXT             0x39
#define  ATA_CMD_WRITE_SECTOR_EXT               0x34

static HANDLE g_hTestUnitReadyThread = NULL;

// ----------------------------------------------------------------------------
// Function: CDisk
//     Constructor
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CDisk::CDisk(
    )
{
    // empty
}

// ----------------------------------------------------------------------------
// Function: CDisk
//     Constructor
//
// Parameters:
//     hKey -
// ----------------------------------------------------------------------------

CDisk::CDisk(
    HKEY hKey
    )
{
    m_dwDeviceFlags = 0;
    m_pNextDisk = NULL;
    m_pATAReg = NULL;
    m_pATARegAlt = NULL;
    m_dwDevice = 0;
    m_hDevKey = hKey;
    m_dwDeviceId = 0;
    m_dwPort = 0;
    m_f16Bit = FALSE;
    m_fUseLBA48 = FALSE;
    m_fAtapiDevice = FALSE;
    m_fInterruptSupported = FALSE;
    m_szDiskName = NULL;
    m_fDMAActive = FALSE;
    m_dwOpenCount = 0;
    m_dwUnitReadyTime = 0;
    m_dwStateFlag = 0;
    m_dwLastCheckTime = 0;
    m_dwStride = 1;
    m_pDiskPower = NULL;
    m_rgbDoubleBuffer = NULL;

    m_pPort = NULL;

    // init generic structures
    InitializeCriticalSection(&m_csDisk);
    memset(&m_Id, 0, sizeof(IDENTIFY_DATA));
    memset(&m_DiskInfo, 0, sizeof(DISK_INFO));
    memset(&m_InqData, 0, sizeof(INQUIRY_DATA));

	m_dwCurrentUDMAMode = 0;
	m_pDMAVirtualAddress = NULL;
 }

// ----------------------------------------------------------------------------
// Function: ~CDisk
//     Destructor
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CDisk::~CDisk(
    )
{
    if (m_hDevKey) {
        RegCloseKey(m_hDevKey);
    }

    if(m_pDiskPower != NULL) {
        delete m_pDiskPower;
    }

    DeleteCriticalSection(&m_csDisk);

    // deallocate double buffer, if present
    if (NULL != m_rgbDoubleBuffer) {
        LocalFree((HLOCAL)m_rgbDoubleBuffer);
    }

	if (m_pPort->m_pDskReg[m_dwDeviceId]->dwEnablePDMA)
	{
		HalFreeCommonBuffer( NULL, 0, m_DMAPhyaddress, m_pDMAVirtualAddress, FALSE );	
	}
}

// ----------------------------------------------------------------------------
// Function: StallExecution
//     Stall execution for the specified period of time
//
// Parameters:
//     dwTime -
// ----------------------------------------------------------------------------

void
CDisk::StallExecution(
    DWORD dwTime
    )
{
    if ((dwTime >= 100) && (m_dwDeviceFlags & DFLAGS_DEVICE_CDROM)) {
        Sleep (dwTime / 100);
    }
    else {
        ::StallExecution(dwTime * 10);
    }
}

#define HELPER_

// These functions should be inlined or converted to macros
void CDisk::TakeCS()                    { EnterCriticalSection(&m_csDisk); }
void CDisk::ReleaseCS()                 { LeaveCriticalSection(&m_csDisk); }
void CDisk::Open()                      { InterlockedIncrement((LONG *)&m_dwOpenCount); }
void CDisk::Close()                     { InterlockedDecrement((LONG *)&m_dwOpenCount); }
BOOL CDisk::IsAtapiDevice()             { return m_fAtapiDevice; }
BOOL CDisk::IsCDRomDevice()             { return (((m_Id.GeneralConfiguration >> 8) & 0x1f) == ATA_IDDEVICE_CDROM); }
BOOL CDisk::IsDVDROMDevice()            { return TRUE; }
BOOL CDisk::IsDiskDevice()              { return (((m_Id.GeneralConfiguration >> 8) & 0x1f) == ATA_IDDEVICE_DISK); }
BOOL CDisk::IsRemoveableDevice()        { return (m_Id.GeneralConfiguration & IDE_IDDATA_REMOVABLE); }
BOOL CDisk::IsDMASupported()            { return ((m_Id.Capabilities & IDENTIFY_CAPABILITIES_DMA_SUPPORTED) && m_fDMAActive); }
BOOL CDisk::IsDRQTypeIRQ()              { return ((m_Id.GeneralConfiguration >> 5) & 0x0003) == ATA_DRQTYPE_INTRQ; }
WORD CDisk::GetPacketSize()             { return m_Id.GeneralConfiguration & 0x0003 ? 16 : 12; }
BOOL CDisk::IsValidCommandSupportInfo() { return ((m_Id.CommandSetSupported2 & (1 << 14)) && !(m_Id.CommandSetSupported2 & (1 << 15))); }
BOOL CDisk::IsWriteCacheSupported()     { return ((m_Id.CommandSetSupported1 & COMMAND_SET_WRITE_CACHE_SUPPORTED) && IsValidCommandSupportInfo()); }
BOOL CDisk::IsPMSupported()             { return (m_Id.CommandSetSupported1 & COMMAND_SET_POWER_MANAGEMENT_SUPPORTED && IsValidCommandSupportInfo()); }
BOOL CDisk::IsPMEnabled()               { return (IsPMSupported() && (m_Id.CommandSetFeatureEnabled1 & COMMAND_SET_POWER_MANAGEMENT_ENABLED)); }

// These functions are called (1x) in atamain and should be inlined
void CDisk::SetActiveKey(TCHAR *szActiveKey)
{
    wcsncpy(m_szActiveKey, szActiveKey, MAX_PATH - 1);
    m_szActiveKey[MAX_PATH - 1] = 0;
}

void CDisk::SetDeviceKey(TCHAR *szDeviceKey)
{
    wcsncpy(m_szDeviceKey, szDeviceKey, MAX_PATH - 1);
    m_szDeviceKey[MAX_PATH - 1] = 0;
}

#define _HELPER

// ----------------------------------------------------------------------------
// Function: InitController
//     Reset the controller and determine whether a device is present on the
//     channel; if a device is present, then query and store its capabilities
//
// Parameters:
//     fForce -
// ----------------------------------------------------------------------------

BOOL
CDisk::InitController(
    BOOL fForce
    )
{
    BOOL bRet = TRUE;

    // if the controller has not already been reset, then perform a soft-reset
    // to enable the channel

    if (TRUE || !(m_dwDeviceFlags & DFLAGS_DEVICE_INITIALIZED)) { // for Wakeup 070625 Hsjang

        // perform a soft-reset on the controller; if we don't do this, then
        // we won't be able to detect whether or not devices are present on the
        // channel

        bRet = ResetController(FALSE);
        if (!bRet) {
            goto exit;
        }

        // if interrupt is supported, enable interrupt

        if (m_fInterruptSupported) {
            SelectDevice();
            WriteAltDriveController(ATA_CTRL_ENABLE_INTR);
            EnableInterrupt();
        }
    }

    // issue IDENTIFY DEVICE and/or IDENTIFY PACKET DEVICE

    bRet = Identify();
    if (!bRet) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Atapi!CDisk::InitController> Device did not respond to identify\r\n"
            )));
        RETAILMSG(1, (_T(
            "Atapi!CDisk::InitController> Device did not respond to identify\r\n"
            )));            
        goto exit;
    }
    else {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Atapi!CDisk::InitController> Device responded to identify\r\n"
            )));
        m_dwDeviceFlags |= DFLAGS_DEVICE_INITIALIZED;
    }

exit:;
    return bRet;
}

// ----------------------------------------------------------------------------
// Function: ConfigureRegisterBlock
//     This function is called by DSK_Init before any other CDisk function to
//     set up the register block.
//
// Parameters:
//     dwStride -
// ----------------------------------------------------------------------------

VOID
CDisk::ConfigureRegisterBlock(
    DWORD dwStride
    )
{
    m_dwStride = dwStride;
    m_dwDataDrvCtrlOffset = ATA_REG_DATA * dwStride;
    m_dwFeatureErrorOffset = ATA_REG_FEATURE * dwStride;
    m_dwSectCntReasonOffset = ATA_REG_SECT_CNT * dwStride;
    m_dwSectNumOffset = ATA_REG_SECT_NUM * dwStride;
    m_dwDrvHeadOffset = ATA_REG_DRV_HEAD * dwStride;
    m_dwCommandStatusOffset = ATA_REG_COMMAND * dwStride;
    m_dwByteCountLowOffset = ATA_REG_BYTECOUNTLOW * dwStride;
    m_dwByteCountHighOffset = ATA_REG_BYTECOUNTHIGH * dwStride;
    m_dwAltStatusOffset = ATA_REG_ALT_STATUS_CS1 * dwStride;
    m_dwAltDrvCtrl = ATA_REG_DRV_CTRL_CS1 * dwStride;
}

// ----------------------------------------------------------------------------
// Function: Init
//     This function is called by the IDE/ATA controller enumerator to trigger
//     the initialization of a device
//
// Parameters:
//     hActiveKey -
// ----------------------------------------------------------------------------

BOOL
CDisk::Init(
    HKEY hActiveKey
    )
{
    BOOL fRet = FALSE;

    // replicate CDisk::ReadRegistry

    m_dwWaitCheckIter = m_pPort->m_pController->m_pIdeReg->dwStatusPollCycles;
    m_dwWaitSampleTimes = m_pPort->m_pController->m_pIdeReg->dwStatusPollsPerCycle;
    m_dwWaitStallTime = m_pPort->m_pController->m_pIdeReg->dwStatusPollCyclePause;

    m_dwDiskIoTimeOut = DEFAULT_DISK_IO_TIME_OUT;

    // replicate CDisk::ReadSettings

    m_dwUnitReadyTime = DEFAULT_MEDIA_CHECK_TIME;

    // if DMA=2 and this is not an ATAPI device, then we'll set m_fDMAActive in Identify
    if (1 == m_pPort->m_pDskReg[m_dwDeviceId]->dwDMA) { // 0=PIO, 1=DMA, 2=ATA DMA only
        m_fDMAActive = TRUE;
    }
    m_dwDMAAlign = m_pPort->m_pController->m_pIdeReg->dwDMAAlignment;

    // m_dwDeviceFlags |= DFLAGS_DEVICE_ISDVD; this is ignored

    if (m_pPort->m_pDskReg[m_dwDeviceId]->dwInterruptDriven) {
        m_fInterruptSupported = TRUE;
    }


    // initialize controller

    if (!InitController(TRUE)) {
        DEBUGMSG(ZONE_INIT, (_T(
            "Atapi!CDisk::Init> Failed to initialize device\r\n"
            )));
        goto exit;
    }

	if (m_pPort->m_pDskReg[m_dwDeviceId]->dwEnablePDMA && m_pDMAVirtualAddress == NULL)
	{
		DMA_ADAPTER_OBJECT dmaAdapter;
        dmaAdapter.ObjectSize = sizeof(dmaAdapter);
        dmaAdapter.InterfaceType = Internal;
        dmaAdapter.BusNumber = 0;
		m_pDMAVirtualAddress= (PBYTE)HalAllocateCommonBuffer( &dmaAdapter, m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize, &m_DMAPhyaddress, FALSE );
	}
	
    // set write cache mode, if write cache mode supported
		
	//SetPioMode(m_Id.AdvancedPIOxferreserved);

    if (m_Id.CommandSetSupported1 & 0x20) {
        if (SetWriteCacheMode(m_pPort->m_pDskReg[m_dwDeviceId]->dwWriteCache)) {
            if (m_pPort->m_pDskReg[m_dwDeviceId]->dwWriteCache) {
                m_dwDeviceFlags |= DFLAGS_USE_WRITE_CACHE;
                DEBUGMSG(ZONE_INIT, (_T(
                    "Atapi!CDisk::Init> Enabled write cache\r\n"
                    )));
                RETAILMSG(1, (_T(
                    "Atapi!CDisk::Init> Enabled write cache\r\n"
                    )));                    
            }
            else {
                m_dwDeviceFlags &= ~DFLAGS_USE_WRITE_CACHE;
                DEBUGMSG(ZONE_INIT, (_T(
                    "Atapi!CDisk::Init> Disabled on device write cache\r\n"
                    )));
                RETAILMSG(1, (_T(
                    "Atapi!CDisk::Init> Disabled on device write cache\r\n"
                    )));                    
            }
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Failed to set write cache mode\r\n"
                )));
            RETAILMSG(1, (_T(
                "Atapi!CDisk::Init> Failed to set write cache mode\r\n"
                )));                
        }
    }

    // set read look-ahead, if read look-ahead supported

    if ((m_Id.CommandSetSupported1 & 0x40) && m_pPort->m_pDskReg[m_dwDeviceId]->dwLookAhead) {
        if (SetLookAhead()) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Enabled read look-ahead\r\n"
                )));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Failed to enable read look-ahead\r\n"
                )));
        }
    }

    // set transfer mode, if a specific transfer mode was specified in the
    // device's instance key

    BYTE bTransferMode = (BYTE)m_pPort->m_pDskReg[m_dwDeviceId]->dwTransferMode;
    if (0xFF != bTransferMode) {
        if (0x00 == bTransferMode) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Selecting PIO default mode(0x%x)\r\n"
                ), bTransferMode));
        }
        else if (0x01 == bTransferMode) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Selecting PIO default mode(0x%x); disabled IORDY\r\n"
                ), bTransferMode));
        }
        else if ((bTransferMode & 0xF8) == 0x08) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Selecting PIO flow control mode %d (0x%x)\r\n"
                ), (bTransferMode & 0x08), bTransferMode));
        }
        else if ((bTransferMode & 0xF0) == 0x20) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Selecting Multi-word DMA mode %d (0x%x)\r\n"
                ), bTransferMode, bTransferMode));
        }
        else if ((bTransferMode & 0xF0) == 0x40) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Selecting Ultra DMA mode %d (0x%x)\r\n"
                ), bTransferMode, bTransferMode));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Unknown transfer mode(0x%x)\r\n"
                ), bTransferMode));
        }
        // @bTransferMode is a valid transfer mode
        if (!SetTransferMode(bTransferMode)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::Init> Failed to set transfer mode(0x%x)\r\n"
                ), bTransferMode));
        }
    }
	if (m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA && ( m_Id.UltraDMASupport & 0x3f))
	{	
		for(int i=5;i>=0;i--) {
		//RETAILMSG(1, (_T("### ATA-Disk supports UDMA to 0x%x 0x%x\r\n"), i,m_dwCurrentUDMAMode));  					
			if(m_Id.UltraDMASupport & (0x01<<i)) {
				m_dwCurrentUDMAMode = (i > 4) ? 4 : i;
				break; 
			}
		} 	
		SetPioMode(PIO0);		
		SetUdmaMode();
		RETAILMSG(1, (_T("### ATA-Disk supports UDMA to 0x%x 0x%x\r\n"), m_Id.UltraDMASupport,m_dwCurrentUDMAMode));  
		m_pPort->m_pDskReg[m_dwDeviceId]->dwEnablePDMA = FALSE;

	}
	else 
	{
		SetPioMode(m_Id.AdvancedPIOxferreserved);
		m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA = FALSE;;		
	}

    fRet = TRUE;



exit:;
    return fRet;
}

// ----------------------------------------------------------------------------
// Function: ResetController
//     Implement ATA/ATAPI-6 R3B 9.2 (Software reset protocol)
//
// Parameters:
//     bSoftReset -
// ----------------------------------------------------------------------------

BOOL
CDisk::ResetController(
    BOOL bSoftReset // ignore
    )
{
    DWORD dwAttempts = 0;
    BYTE bStatus = 0;
    BOOL fRet = FALSE;

    // we have to negate the RESET signal for 5 microseconds before we assert it

    WriteAltDriveController(0x00);
    ::StallExecution(25);

    // Set_SRST
    // --------
    // to enter Set_SRST state, set SRST in the Device Control register to one;
    // this will assert the RESET signal and reset both devices on the current
    // channel

    WriteAltDriveController(0x04); // 0x04 == SRST

    // remain in this state for at least 5 microseconds; i.e., assert RESET signal
    // for at least 5 microseconds
    // if this is a hardware reset, then assert RESET signal for at least 25
    // microseconds

    ::StallExecution(25); // this should be CEDDK implementation

    // Clear_wait
    // ----------
    // clear SRST in the Device Control register, i.e., negate RESET signal

    WriteAltDriveController(0x00);

    // remain in this state for at least 2 milliseconds

    Sleep(5);

HSR2_Check_status:;

    // Check_status
    // ------------
    // read the Status or Alternate Status register
    // if BSY is set to one, then re-enter this state
    // if BSY is cleared to zero, check the ending status in the Error register
    // and the signature (9.12) and transition to Host_Idle

    bStatus = GetAltStatus(); // read Status register
    if (bStatus & 0x80) {
        // BSY is set to one, re-enter this state
        DEBUGMSG(ZONE_INIT, (TEXT(
            "Atapi!CDisk::ResetController> Device is busy; %u seconds remaining\r\n"
            ), (m_pPort->m_pController->m_pIdeReg->dwSoftResetTimeout - dwAttempts)));
        Sleep(1000);
        dwAttempts += 1;
        // a device has at most 31 seconds to complete a software reset; we'll use 3 seconds
        if (dwAttempts == m_pPort->m_pController->m_pIdeReg->dwSoftResetTimeout) {
            DEBUGMSG(ZONE_INIT, (TEXT("Atapi!CDisk::ResetController> Timeout\r\n")));
            goto exit;
        }
        goto HSR2_Check_status;
    }
    DEBUGMSG(ZONE_INIT, (TEXT(
        "Atapi!CDisk::ResetController> Device is ready\r\n"
        )));

    // BSY is cleared to zero, check the ending status in the Error register
    // and the signature
    // TODO: Check the signature (9.12)

    // if ERR bit set to one, then the reset failed
    bStatus = GetAltStatus(); // read Status register
    if (bStatus & 0x01) {
        // ERR is set to one
        // the bits in the Error register are valid, but the Error register
        // doesn't provide any useful information in the case of SRST failing
        DEBUGMSG(ZONE_INIT, (TEXT(
            "Atapi!CDisk::ResetController> SRST failed\r\n"
            )));
        // TODO: Recover from error
        goto exit;
    }

    fRet = TRUE;

exit:;
    return fRet;
}



// ----------------------------------------------------------------------------
// Function: AtapiSoftReset
//     Issue ATAPI SOFT RESET command
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void
CDisk::AtapiSoftReset(
    )
{
    WriteCommand(ATAPI_CMD_SOFT_RESET);
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 400);
    WaitForDisc(WAIT_TYPE_READY, 500);
}

// ----------------------------------------------------------------------------
// Function: IsDevicePresent
//     Determine whether a device is present on the channel
//
// Parameters:
//     None
//
// Notes:
//     If a device is present on a channel, then the device's associated
//     Error register is populated with 0x1.  If a device is not present on
//     a channel, then the device's associated Error register is populated
//     with 0xa or 0xb, for master or slave, respectively.
// ----------------------------------------------------------------------------

BOOL
CDisk::IsDevicePresent(
    )
{
    BYTE bError;
    BYTE bStatus;

    // determine which device to select (i.e., which device this device is)

	
    if ((m_dwDevice == 0) || (m_dwDevice == 2) ) {
        // select device 0
        ATA_WRITE_BYTE(m_pATAReg + m_dwDrvHeadOffset, ATA_HEAD_DRIVE_1);
    }
    else {
        // select device 1
        ATA_WRITE_BYTE(m_pATAReg + m_dwDrvHeadOffset, ATA_HEAD_DRIVE_2);
    }

    // read Status register
    bStatus = GetAltStatus();

    // read Error register
    bError = GetError();
    
    // test Error register
    if (bError == 0x1) {
        DEBUGMSG(ZONE_INIT, (_T(
            "Atapi!CDisk::IsDevicePresent> Device %d is present\r\n"
            ), m_dwDevice));
        return TRUE;
    }
    RETAILMSG(1, (_T(
        "Atapi!CDisk::IsDevicePresent> Device %d is not present; Error register(0x%x)\r\n"
        ), m_dwDevice, bError));
    DEBUGMSG(ZONE_INIT, (_T(
        "Atapi!CDisk::IsDevicePresent> Device %d is not present; Error register(0x%x)\r\n"
        ), m_dwDevice, bError));
    return FALSE;
}

// ----------------------------------------------------------------------------
// Function: SendExecuteDeviceDiagnostic
//     Implement ATA/ATAPI-6 R3B 9.10 (Device diagnostic protocol)
//
// Parameters:
//     pbDiagnosticCode - diagnostic code returned by controller in Error
//                        register as a result of issuing EXECUTE DEVICE
//                        DIAGNOSTIC (8.11)
//
//     pfIsAtapi - whether device is an ATAPI device
// ----------------------------------------------------------------------------

BOOL
CDisk::SendExecuteDeviceDiagnostic(
    PBYTE pbDiagnosticCode,
    PBOOL pfIsAtapi
    )
{
    BYTE bStatus = 0;
    DWORD dwWaitAttempts = 1200;
    BOOL fReadSignature = FALSE;

    PREFAST_DEBUGCHK(NULL != pbDiagnosticCode);
    PREFAST_DEBUGCHK(NULL != pfIsAtapi);

    // HI4:HED0, write command

    WaitOnBusy(FALSE);
    WriteCommand(0x90); // EXECUTE DEVICE DIAGNOSTIC command code

    // HED0:Wait, wait for at least 2 milliseconds; see following Sleep(5)
    // HED2:Check_Status, wait on BSY=0

    while (1) {
        Sleep(5);                 // wait 5 milliseconds
        bStatus = GetAltStatus(); // get status
        // test error
        if (bStatus & ATA_STATUS_ERROR) {
            // error
            DEBUGMSG(ZONE_ERROR, (TEXT(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device failed to process command\r\n"
                ), m_dwDeviceId));
            break;
        }
        // test BSY=0
        if (!(bStatus & ATA_STATUS_BUSY)) break;
        // retry
        if (dwWaitAttempts-- == 0) {
            DEBUGMSG(ZONE_ERROR, (TEXT(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> No response; assuming channel is empty\r\n"
                ), m_dwDeviceId));
            break; // return FALSE;
        }
    }

    // inspect result of diagnosis (table 26, 8.11); select self

    SelectDevice();
	    
    *pbDiagnosticCode = GetError();
    if ((m_dwDevice == 0) || (m_dwDevice == 0)) {
        // device 0 (master)
        if (*pbDiagnosticCode == 0x01) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 passed, Device 1 passed or not present\r\n"
                )));
            RETAILMSG(1, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 passed, Device 1 passed or not present\r\n"
                )));                
            fReadSignature = TRUE; // read signature to determine if we're ATA or ATAPI
        }
        else if (*pbDiagnosticCode == 0x00 || (0x02 <= *pbDiagnosticCode && *pbDiagnosticCode <= 0x7F)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 failed, Device 1 passed or not present\r\n"
                )));
            RETAILMSG(1, (_T(
                "111Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 1failed, Device 1 passed or not present\r\n"
                )));                
        }
        else if (*pbDiagnosticCode == 0x81) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 passed, Device 1 failed\r\n"
                )));
            RETAILMSG(1, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 passed, Device 1 failed\r\n"
                )));                
            fReadSignature = TRUE; // read signature to determine if we're ATA or ATAPI
        }
        else if (*pbDiagnosticCode == 0x80 || (0x82 <= *pbDiagnosticCode && *pbDiagnosticCode <= 0xFF)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 failed, Device 1 failed\r\n"
                )));
            RETAILMSG(1, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 failed, Device 1 failed\r\n"
                )));                
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Unknown diagnostic code(0x%x)\r\n"
                ), *pbDiagnosticCode));
            RETAILMSG(1, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Unknown diagnostic code(0x%x)\r\n"
                ), *pbDiagnosticCode));                
        }
    }
    else {
        // device 1 (slave)
        if (*pbDiagnosticCode == 0x01) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 1 passed\r\n"
                )));
            fReadSignature = TRUE; // read signature to determine if we're ATA or ATAPI
        }
        else if (*pbDiagnosticCode == 0x00 || (0x02 <= *pbDiagnosticCode && *pbDiagnosticCode <= 0x7F)) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 1 failed\r\n"
                )));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Unknown diagnostic code(0x%x)\r\n"
                ), *pbDiagnosticCode));
        }
    }

    if (fReadSignature) {

        // we passed; read signature to determine if it's ATA or ATAPI

        // test for ATA
        if (
            ATA_READ_BYTE(m_pATAReg + m_dwSectNumOffset) == 0x01 &&
            ATA_READ_BYTE(m_pATAReg + m_dwByteCountLowOffset) == 0x00 &&
            ATA_READ_BYTE(m_pATAReg + m_dwByteCountHighOffset) == 0x00 // &&
            // ATA_READ_BYTE(m_pATAReg + m_dwDrvHeadOffset) == 0x00 &&
            // ATA_READ_BYTE(m_pATAReg + m_dwSectCntReasonOffset) == 0x01 &&
        ) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> ATA device\r\n"
                )));
            *pfIsAtapi = FALSE;
        }
        // test for ATAPI
        else if (
            ATA_READ_BYTE(m_pATAReg + m_dwSectNumOffset) == 0x01 &&
            ATA_READ_BYTE(m_pATAReg + m_dwByteCountLowOffset) == 0x14 &&
            ATA_READ_BYTE(m_pATAReg + m_dwByteCountHighOffset) == 0xEB // &&
            // ATA_READ_BYTE(m_pATAReg + m_dwDrvHeadOffset) == 0x00 &&
            // ATA_READ_BYTE(m_pATAReg + m_dwSectCntReasonOffset) == 0x01 &&
        ) {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> ATAPI device\r\n"
                )));
            *pfIsAtapi = TRUE;
        }
        // unknown
        else {
            DEBUGMSG(ZONE_INIT, (_T(
                "Atapi!CDisk::SendExecuteDeviceDiagnostic> Device 0 = Unknown device type (i.e., not ATA, not ATAPI)\r\n"
                )));
        }
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: SendIdentifyDevice
//     Issue IDENTIFY_DEVICE or IDENTIFY_PACKET_DEVICE depending on whether
//     fIsAtapi is TRUE.  Implement PIO data-in command protocol as per
//     ATA/ATAPI-6 R3B 9.2.
//     Implement ATA/ATAPI-6 R3B 9.10 (Device diagnostic protocol)
//
// Parameters:
//     fIsAtapi - if device is ATAPI, send IDENTIFY PACKET DEVICE
//
// Notes:
//     After issuing a PIO data-in command, if BSY=0 and DRQ=0, then the device
//     failed to process the command.  However, there exist devices that
//     require additional time to return status via the Status register.  As
//     such, a delayed retry has been introduced to faciliate such devices,
//     even though their actions do not comply with the specification.
// ----------------------------------------------------------------------------

#define HPIOI1_CHECK_STATUS_RETRIES 10
BOOL
CDisk::SendIdentifyDevice(
    BOOL fIsAtapi
    )
{
    BOOL fResult = TRUE;
    DWORD dwRetries = 0;
    BYTE bStatus;               // Status register
    DWORD cbIdentifyDeviceData; // IDENTIFY DEVICE data size

    // Host Idle protocol

    // select correct device
    SelectDevice();

    // HI1:Check_Status
    // ----------------
HI1_Check_Status:;
    bStatus = GetAltStatus();
    if ((bStatus & ATA_STATUS_BUSY) || (bStatus & ATA_STATUS_DATA_REQ)) { // BSY=1 or DRQ=1
        Sleep(5);
        goto HI1_Check_Status;
    }

    // HI3:Write_Parameters
    // --------------------
    // no paramters

    // HI4:Write_Command

	*((UINT32 *)(m_pATAReg + ATA_CFG)) |= 0x40;
	
    __try {
        WriteCommand(fIsAtapi ? ATAPI_CMD_IDENTIFY : ATA_CMD_IDENTIFY);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "Atapi!CDisk::SendIdentifyDevice> Exception writing to Command register\r\n"
            )));
        fResult = FALSE;
        goto exit;
    }

    // PIO data-in command protocol

    // HPIOI1:Check_Status
    // -------------------
HPIOI1_Check_Status:;
    __try {
        bStatus = GetAltStatus();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "Atapi!CDisk::SendIdentifyDevice> Exception\r\n"
            )));
        fResult = FALSE;
        goto exit;
    }
    if (!(bStatus & (ATA_STATUS_BUSY|ATA_STATUS_DATA_REQ))) { // BSY=0 and DRQ=0
        // an error occurred
        if (dwRetries < HPIOI1_CHECK_STATUS_RETRIES) {
            dwRetries++;
            Sleep(5);
            goto HPIOI1_Check_Status;
         }
         fResult = FALSE;
         goto exit;
    }
    if (bStatus & ATA_STATUS_BUSY) { // BSY=1
        goto HPIOI1_Check_Status;
    }
    if (!(bStatus & ATA_STATUS_BUSY) && (bStatus & ATA_STATUS_DATA_REQ)) { // BSY=0 and DRQ=1
        goto HPIOI2_Transfer_Data;
    }

    // HPIOI2:Transfer_Data
    // --------------------
    // (IDENTIFY [ATAPI] DEVICE only returns a single DRQ data block)
HPIOI2_Transfer_Data:;
    cbIdentifyDeviceData = sizeof(IDENTIFY_DATA);
    DEBUGCHK(cbIdentifyDeviceData <= BYTES_PER_SECTOR);
    // read result of IDENTIFY DEVICE/IDENTIFY PACKET DEVICE
    if (m_f16Bit) {
    
        USHORT temp[sizeof(IDENTIFY_DATA)/2];    
        USHORT value;
        
        cbIdentifyDeviceData /= 2;
        ReadWordBuffer((PWORD)temp, cbIdentifyDeviceData);

        for (DWORD i=0; i<cbIdentifyDeviceData ; i++)
        {
        	value = ((*(temp + i)& 0xff00) >> 8) & 0xff;
			value |= ((*(temp + i)& 0xff ) << 8) & 0xff00;
			
			*(((USHORT *)(&m_Id)) + i) = value;
		}
        	
    }
    else {
        ReadByteBuffer((PBYTE)&m_Id, cbIdentifyDeviceData);
    }
    // ignore extraneous data
    while (GetAltStatus() & ATA_STATUS_DATA_REQ ) {
        if (m_f16Bit) {
            ReadWord();
        }
        else {
            ReadByte();
        }
    }

	*((UINT32 *)(m_pATAReg + ATA_CFG)) &= ~(0x40);
    // Return to Host Idle protocol

exit:;
    return fResult;
}

// ----------------------------------------------------------------------------
// Function: Identify
//     This function initiates communication with the device.  If the
//     appropriate device is detected on the channel, then instruct the device
//     to execute a diagnostic.  Issue IDENTIFY DEVICE/IDENTIFY PACKET DEVICE.
//     Validate the IDENTIFY data.  Inspect IDENTIFY data (if ATA, determine
//     which read/write commands to use, store disk geometry, etc.)
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CDisk::Identify(
    )
{
    DWORD dwBlockSize = 0; // size of IDENTIFY DEVICE/IDENTIFY PACKET DEVICE information
    WORD wDevType = 0;     // command packet set implemented by device (e.g., direct-access, CD-ROM, etc.)
    DWORD dwCHS = 0;       // whether the registry specifies that the device is to use C/H/S mode
    BYTE bDiagnosticCode;  // SendExecuteDeviceDiagnostic argument
    BOOL fIsAtapi;         // SendExecuteDeviceDiagnostic argument


    TakeCS();

    // test for device present
    if (!IsDevicePresent()) {
        ReleaseCS();
        return FALSE;
    }

    // issue EXECUTE DEVICE DIAGNOSTIC; determine whether device is ATA or ATAPI
    // (ignore the result of this call, as old devices fail to respond correctly)

    SendExecuteDeviceDiagnostic(&bDiagnosticCode, &fIsAtapi);

    // try ATA device
    if (SendIdentifyDevice(FALSE)) { // fIsAtapi=FALSE
        m_fAtapiDevice = FALSE;
        // ALi IDE/ATA controller tweak for supporting a DMA-enabled ATAPI device
        if (2 == m_pPort->m_pDskReg[m_dwDeviceId]->dwDMA) { // 0=PIO, 1=DMA, 2=ATA DMA only
            m_fDMAActive = TRUE;
        }
    }
    else {
        // try ATAPI device
        if (SendIdentifyDevice(TRUE)) { // fIsAtapi=TRUE
            m_fAtapiDevice = TRUE;
        }
        else {
            DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
                "Atapi!CDisk::Identify> Device failed to respond to IDENTIFY DEVICE and IDENTIFY PACKET DEVICE\r\n"
                )));
            ReleaseCS();
            return FALSE;
        }
    }

    ReleaseCS();

    // validate IDENTIFY DEVICE/IDENTIFY PACKET DEVICE signature; any empty
    // channel may return invalid data

	RETAILMSG(1, (_T("### ATA-Disk Total Sector Size 0x%x\r\n"), m_Id.TotalUserAddressableSectors));            
	

    if ((m_Id.GeneralConfiguration == 0) || (m_Id.GeneralConfiguration == 0xffff) ||
        (m_Id.GeneralConfiguration == 0xff7f) ||
        (m_Id.GeneralConfiguration == 0x7fff) ||
       ((m_Id.GeneralConfiguration == m_Id.IntegrityWord) && (m_Id.NumberOfCurrentCylinders == m_Id.IntegrityWord))
    ) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
            "Atapi!CDisk::Identify> General configuration(%04X) not valid; device not present\r\n"
            ), m_Id.GeneralConfiguration));
        RETAILMSG(1, (_T(
            "Atapi!CDisk::Identify> General configuration(%04X) not valid; device not present\r\n"
            ), m_Id.GeneralConfiguration));            
        return FALSE;
    }

    // dump IDENTIFY DEVICE/IDENTIFY PACKET DEVICE data and supported transfer modes
    PIDENTIFY_DATA pId = &m_Id;
    DUMPIDENTIFY(pId);
    DUMPSUPPORTEDTRANSFERMODES(pId);

    // ATA/ATAPI-3 compatible devices store command packet set implemented by
    // device in bits 12-8 of word 0 of IDENTIFY DEVICE/IDENTIFY PACKET DEVICE
    // data (this information is retired in ATA/ATAPI-6)

    wDevType = (m_Id.GeneralConfiguration >> 8) & 0x1F;
    switch (wDevType) {
    case ATA_IDDEVICE_UNKNOWN:
        return FALSE;
    case ATA_IDDEVICE_CDROM:
        m_dwDeviceFlags |= DFLAGS_DEVICE_CDROM;
        break;
    case ATA_IDDEVICE_DISK:
        break;
    case ATA_IDDEVICE_OPTICAL_MEM:
        break;
    default:
        DEBUGMSG(ZONE_INIT, (_T("Atapi!CDisk::Identify> Assuming direct-access device (hard disk drive)\r\n")));
        break;
    }
    // this is redundant; but various routines use this information
    m_dwDeviceFlags |= DFLAGS_DEVICE_PRESENT;
    m_dwDeviceFlags |= (IsAtapiDevice()) ? DFLAGS_ATAPI_DEVICE : 0;
    m_dwDeviceFlags |= (IsRemoveableDevice()) ? DFLAGS_REMOVABLE_DRIVE : 0;

    // ATAPI devices use ATAPI read/write commands; ATA devices support
    // single- and multi-sector transfers; if this is an ATA device, then
    // select multi-sector transfers, if supported

    if (!IsAtapiDevice()) {

        // default to single-sector transfers
        m_bReadCommand = ATA_CMD_READ; m_bWriteCommand = ATA_CMD_WRITE;

        if (m_Id.MaximumBlockTransfer != 0) {

            // device supports multi-sector transfers; enable multi-sector
            // transfers; issue SET MULTIPLE MODE command
            SelectDevice();
            WriteSectorCount((BYTE)m_Id.MaximumBlockTransfer);
            WriteCommand(ATA_CMD_SET_MULTIPLE);

            if (!WaitOnBusy(FALSE) && (GetAltStatus() & ATA_STATUS_READY)) {
                m_bReadCommand = ATA_CMD_MULTIPLE_READ; m_bWriteCommand = ATA_CMD_MULTIPLE_WRITE;
                m_bSectorsPerBlock = m_Id.MaximumBlockTransfer;
            }
            else {
                DEBUGMSG(ZONE_INIT, (_T(
                    "Atapi!CDisk::Identify> (Warning) Failed to enable multi-sector transfers; using single-sector transfers\r\n"
                    )));
            }
        }
    }
    m_bDMAReadCommand = ATA_CMD_READ_DMA;
    m_bDMAWriteCommand = ATA_CMD_WRITE_DMA;

    m_fLBAMode = (m_Id.Capabilities & 0x0200) ? TRUE : FALSE;
    m_DiskInfo.di_flags = DISK_INFO_FLAG_MBR;                 // all ATA storage devices have an MBR
    m_DiskInfo.di_bytes_per_sect = BYTES_PER_SECTOR;          // start with 512, then go with SetInfo changes
    m_DiskInfo.di_cylinders = m_Id.NumberOfCylinders;
    m_DiskInfo.di_heads = m_Id.NumberOfHeads;
    m_DiskInfo.di_sectors = m_Id.SectorsPerTrack;
    if (m_fLBAMode) {
        m_DiskInfo.di_total_sectors = m_Id.TotalUserAddressableSectors;
        ConfigLBA48(); // set m_fUseLBA48 if applicable
    }
    else {
        m_DiskInfo.di_total_sectors= m_DiskInfo.di_cylinders*m_DiskInfo.di_heads*m_DiskInfo.di_sectors;
    }

    return TRUE;
}


// ----------------------------------------------------------------------------
// Function: ConfigLBA48
//     This is a helper function which is called after the IDENTIFY_DEVICE
//     command has been successfully executed.  It parses the results
//     of the IDENTIFY_DEVICE command to determine if 48-bit LBA is supported
//     by the device.
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void CDisk::ConfigLBA48(void)
{
    PIDENTIFY_DATA pId = (PIDENTIFY_DATA)&m_Id;

    // Word 87 (CommandSetFeatureDefault): 
    //         bit 14 is set and bit 15 is cleared if config data
    //         in word 86 (CommandSetFeatureEnabled2) is valid.
    // Note that this is only valid for non-ATAPI devices
    if ( !IsAtapiDevice() &&
         (pId->CommandSetFeatureDefault & (1 << 14)) &&
         !(pId->CommandSetFeatureDefault & (1 << 15)) &&
         (pId->CommandSetFeatureEnabled2 & (1 << 10)) )
   {
        DEBUGMSG(ZONE_INIT, (TEXT("Atapi!CDisk::Identify::ConfigLBA48> Device supports 48-bit LBA\r\n")));
        DEBUGMSG(ZONE_INIT, (TEXT("Atapi!CDisk::Identify::ConfigLBA48> Max LBA Address = 0x%08x%08x"),
                             pId->Reserved7[102-94] + (pId->Reserved7[103-94] << 16),
                             pId->Reserved7[100-94] + (pId->Reserved7[101-94] << 16)));

        m_fUseLBA48 = TRUE;

        // The CE file system currently supports a maximum of 32-bit sector addresses,
        // so we only use the lower DWORD of lMaxLBAAdress.
        // m_DiskInfo.di_total_sectors = pId->lMaxLBAAddress[0];
        m_DiskInfo.di_total_sectors = pId->Reserved7[100-94] + (pId->Reserved7[101-94] << 16) ;
        // CDisk::Identify has determined whether or not the device supports multi-sector transfers
        // Update read/write command to use [READ|WRITE] [SECTORS|MULTIPLE] EXT
        if (m_bReadCommand == ATA_CMD_READ) 
        {
            m_bReadCommand = ATA_CMD_READ_SECTOR_EXT;
            m_bWriteCommand = ATA_CMD_WRITE_SECTOR_EXT;
        }
        else // CDisk::Identify has determined that the devce supports multi-sector transfers
        {
            m_bReadCommand = ATA_CMD_READ_MULTIPLE_EXT;
            m_bWriteCommand = ATA_CMD_WRITE_MULTIPLE_EXT;
        }
        m_bDMAReadCommand = ATA_CMD_READ_DMA_EXT;
        m_bDMAWriteCommand = ATA_CMD_WRITE_DMA_EXT;
   }
   else  m_fUseLBA48 = FALSE;
}

// ----------------------------------------------------------------------------
// Function: ValidateSg
//     Map embedded pointers
//
// Parameters:
//     pSgReq -
//     InBufLen -
// ----------------------------------------------------------------------------

BOOL
CDisk::ValidateSg(
    PSG_REQ pSgReq,
    DWORD InBufLen
    )
{
    if (PSLGetCallerTrust() != OEM_CERTIFY_TRUST) {
        if (pSgReq && InBufLen >= (sizeof(SG_REQ) + sizeof(SG_BUF) * (pSgReq->sr_num_sg - 1))) {
            DWORD dwIndex;
            for (dwIndex = 0; dwIndex < pSgReq -> sr_num_sg; dwIndex++) {
                pSgReq->sr_sglist[dwIndex].sb_buf = (PUCHAR)MapCallerPtr((LPVOID)pSgReq->sr_sglist[dwIndex].sb_buf,pSgReq->sr_sglist[dwIndex].sb_len);
            }
        }
        else {
            return FALSE;
        }
    }
    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: ValidateSg
//     Map embedded pointers
//
// Parameters:
//     pCdrom -
//     InBufLen -
// ----------------------------------------------------------------------------

BOOL
CDisk::ValidateSg(
    PCDROM_READ pCdrom,
    DWORD InBufLen
    )
{
    if (PSLGetCallerTrust() != OEM_CERTIFY_TRUST) {
        if (pCdrom && InBufLen >= (sizeof(CDROM_READ) + sizeof(SGX_BUF) * (pCdrom->sgcount - 1))) {
            DWORD dwIndex;
            for (dwIndex = 0; dwIndex < pCdrom-> sgcount; dwIndex++) {
                pCdrom->sglist[dwIndex].sb_buf = (PUCHAR)MapCallerPtr((LPVOID)pCdrom->sglist[dwIndex].sb_buf,pCdrom->sglist[dwIndex].sb_len);
            }
        }
        else {
            return FALSE;
        }
    }
    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: SendDiskPowerCommand
//     Put the device into a specified power state.  The optional parameter is
//     programmed into the Sector Count register, which is used for the
//     ATA NEW CMD IDLE and ATA CMD STANDBY commands.
//
// Parameters:
//     bCmd -
//     bParam -
// ----------------------------------------------------------------------------

BOOL
CDisk::SendDiskPowerCommand(
    BYTE bCmd,
    BYTE bParam
    )
{
    BYTE bError, bStatus;
    BOOL fOk = TRUE;

    if(ZONE_CELOG) CeLogData(TRUE, CELID_ATAPI_POWERCOMMAND, &bCmd, sizeof(bCmd), 0, CELZONE_ALWAYSON, 0, FALSE);

    // HI:Check_Status (Host Idle); wait until BSY=0 and DRQ=0
    // read Status register
    while (1) {
        bStatus = GetAltStatus();
        if (!(bStatus & (0x80|0x08))) break; // BSY := Bit 7, DRQ := Bit 3
        Sleep(5);
    }

    // HI:Device_Select; select device
    SelectDevice();

    // HI:Check_Status (Host Idle); wait until BSY=0 and DRQ=0
    // read Status register
    while (1) {
        bStatus = GetAltStatus();
        if (!(bStatus & (0x80|0x08))) break; // BSY := Bit 7, DRQ := Bit 3
        Sleep(5);
    }

    // HI:Write_Parameters
    WriteSectorCount(bParam);
    // WriteAltDriveController(0x00); // disable interrupt (nIEN := Bit 1 of Device Control register)

    // HI:Write_Command
    WriteCommand(bCmd);

    // transition to non-data command protocol

    // HND:INTRQ_Wait
    // transition to HND:Check_Status
    // read Status register
    while (1) { // BSY := Bit 7
        bStatus = GetAltStatus();
        bError = GetError();
        if (bError & 0x04) { // ABRT := Bit 2
            // command was aborted
            DEBUGMSG(ZONE_ERROR, (_T(
                "Atapi!CDisk::SendDiskPowerCommand> Failed to send command 0x%x, parameter 0x%x\r\n"
                ), bCmd, bParam));
            fOk = FALSE;
            break;
        }
        if (!(bStatus & 0x80)) break; // BSY := Bit 7
        Sleep(5);
    }

    // transition to host idle protocol

    return fOk;
}

// ----------------------------------------------------------------------------
// Function: GetDiskPowerInterface
//     Return the power management object associated with this device
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CDiskPower *
CDisk::GetDiskPowerInterface(
    void
    )
{
    CDiskPower *pDiskPower = new CDiskPower;
    return pDiskPower;
}

// ----------------------------------------------------------------------------
// Function: SetDiskPowerState
//     Map a power state to an ATA power management command and issue the
//     command
//
// Parameters:
//     newDx -
// ----------------------------------------------------------------------------

BOOL
CDisk::SetDiskPowerState(
    CEDEVICE_POWER_STATE newDx
    )
{
    BYTE bCmd;

    if (ZONE_CELOG) {
        DWORD dwDx = (DWORD) newDx;
        CeLogData(TRUE, CELID_ATAPI_SETDEVICEPOWER, &dwDx, sizeof(dwDx), 0, CELZONE_ALWAYSON, 0, FALSE);
    }

    // on D0 go to IDLE to minimize latency during disk accesses
    if(newDx == D0 || newDx == D1) {
        bCmd = ATA_CMD_IDLE_IMMEDIATE;
    }
    else if(newDx == D2) {
        bCmd = ATA_CMD_STANDBY_IMMEDIATE;
    }
    else if(newDx == D3 || newDx == D4) {
        bCmd = ATA_CMD_SLEEP;
    }
    else {
        DEBUGMSG(ZONE_WARNING, (_T(
            "CDisk::SetDiskPowerState> Invalid power state value(%u)\r\n"
            ), newDx));
        return FALSE;
    }

    // update the disk power state
    return SendDiskPowerCommand(bCmd);
}

// ----------------------------------------------------------------------------
// Function: WakeUp
//     Wake the device up from sleep
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CDisk::WakeUp(
    )
{
    if (!ResetController(FALSE)) {
        return FALSE;
    }
    return SendIdentifyDevice(IsAtapiDevice());
}

// ----------------------------------------------------------------------------
// Function: MainIoctl
//     Process IOCTL_DISK_ and DISK_IOCTL_ I/O controls
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::MainIoctl(
    PIOREQ pIOReq
    )
{
    DWORD dwError = ERROR_SUCCESS;

    DEBUGMSG(ZONE_IOCTL, (TEXT(
        "Atapi!CDisk::MainIoctl> IOCTL(%x), device(%x)\r\n"
        ), pIOReq->dwCode, m_dwDeviceId));

    // device is powering down; fail

    if (m_dwDeviceFlags & DFLAGS_DEVICE_PWRDN) {
        SetLastError(ERROR_DEVICE_NOT_AVAILABLE);
        return FALSE;
    }

    switch(pIOReq->dwCode) {
        case IOCTL_DISK_GETINFO:
        case DISK_IOCTL_GETINFO:
            if (IsCDRomDevice()) {
                dwError = ERROR_BAD_COMMAND;
            }
            else {
                dwError = GetDiskInfo(pIOReq);
            }
            break;
        case IOCTL_DISK_DEVICE_INFO:
            dwError = GetDeviceInfo(pIOReq);
            break;
        case DISK_IOCTL_GETNAME:
        case IOCTL_DISK_GETNAME:
            dwError = GetDiskName(pIOReq);
            break;
        case DISK_IOCTL_SETINFO:
        case IOCTL_DISK_SETINFO:
            dwError = SetDiskInfo(pIOReq);
            break;
        case DISK_IOCTL_READ:
        case IOCTL_DISK_READ:
            if (!ValidateSg((PSG_REQ)pIOReq->pInBuf,pIOReq->dwInBufSize)) {
                dwError = ERROR_INVALID_PARAMETER;
            }
            else {
                if (IsDMASupported()) {
                    dwError = ReadWriteDiskDMA(pIOReq, TRUE);
                }
                else {
                    dwError = ReadWriteDisk(pIOReq, TRUE);
                }
            }
            break;
        case DISK_IOCTL_WRITE:
        case IOCTL_DISK_WRITE:
            if (!ValidateSg((PSG_REQ)pIOReq->pInBuf,pIOReq->dwInBufSize)) {
                dwError=ERROR_INVALID_PARAMETER;
            }
            else {
                if (IsDMASupported()) {
                    dwError = ReadWriteDiskDMA(pIOReq, FALSE);
                }
                else {
                    dwError = ReadWriteDisk(pIOReq, FALSE);
                }
            }
            break;
        case IOCTL_DISK_GET_STORAGEID:
            dwError = GetStorageId(pIOReq);
            break;
        case DISK_IOCTL_FORMAT_MEDIA:
        case IOCTL_DISK_FORMAT_MEDIA:
            dwError = ERROR_SUCCESS;
            break;;
        case IOCTL_DISK_FLUSH_CACHE:
            dwError = FlushCache();
            break;

        default:
            dwError = ERROR_NOT_SUPPORTED;
            break;
    }

    return dwError;
}

// ----------------------------------------------------------------------------
// Function: PerformIoctl
//     This is the top-most IOCTL processor and is used to trap IOCTL_POWER_
//     I/O controls to pass to the associated power management object
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

BOOL
CDisk::PerformIoctl(
    PIOREQ pIOReq
    )
{
    DWORD dwError = ERROR_SUCCESS;

    DEBUGMSG(ZONE_IOCTL, (TEXT(
        "Atapi!CDisk::PerformIoctl> IOCTL(%x), device(%x)\r\n"
        ), pIOReq->dwCode, m_dwDeviceId));

    if (pIOReq->pBytesReturned) {
        *(pIOReq->pBytesReturned) = 0;
    }

    TakeCS();
    m_pPort->TakeCS();

    if (ZONE_CELOG) CeLogData(TRUE, CELID_ATAPI_STARTIOCTL, pIOReq, sizeof(*pIOReq), 0, CELZONE_ALWAYSON, 0, FALSE);

    __try {

        if (pIOReq->dwCode == IOCTL_POWER_CAPABILITIES) {


            // instantiate DiskPower object on first use, if necessary

            if (m_pDiskPower == NULL) {
                CDiskPower *pDiskPower = GetDiskPowerInterface();
                if (pDiskPower == NULL) {
                    DEBUGMSG(ZONE_WARNING, (_T(
                        "Atapi!CDisk::PerformIoctl> Failed to create power management object\r\n"
                        )));
			    	                        
                }
                else if (!pDiskPower->Init(this)) {
                    DEBUGMSG(ZONE_WARNING, (_T(
                        "Atapi!CDisk::PerformIoctl> Failed to initialize power management\r\n"
                        )));
                    delete pDiskPower;
                }
                else {
                    m_pDiskPower = pDiskPower;
                }
            }
        }

        if (m_pDiskPower != NULL) {

            // is this a power IOCTL?
            dwError = m_pDiskPower->DiskPowerIoctl(pIOReq);
            if (dwError != ERROR_NOT_SUPPORTED) {
                goto done;
            }

            // request that the disk spin up (if it's not up already)
            if (!m_pDiskPower->RequestDevice()) {
                // the disk is powered down
                dwError = ERROR_RESOURCE_DISABLED;
                goto done;
            }
        }

        // call the driver
        dwError = MainIoctl(pIOReq);

        // indicate we're done with the disk
        if (m_pDiskPower != NULL) {
            m_pDiskPower->ReleaseDevice();
        }

done:;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = ERROR_GEN_FAILURE;
    }

    if (ZONE_CELOG) CeLogData(TRUE, CELID_ATAPI_COMPLETEIOCTL, &dwError, sizeof(dwError), 0, CELZONE_ALWAYSON, 0, FALSE);

    m_pPort->ReleaseCS();
    ReleaseCS();

    if (dwError != ERROR_SUCCESS) {
        SetLastError(dwError);
    }

    return (ERROR_SUCCESS == dwError);
}

// ----------------------------------------------------------------------------
// Function: PostInit
//     This function facilitates backward compatibility
//
// Parameters:
//     pPostInitBuf -
// ----------------------------------------------------------------------------

BOOL
CDisk::PostInit(
    PPOST_INIT_BUF pPostInitBuf
    )
{
    DWORD dwError = ERROR_SUCCESS;

    DEBUGMSG(ZONEID_INIT, (TEXT("Atapi!CDisk::PostInit> device(%d)\r\n"), m_dwDeviceId));

    m_hDevice = pPostInitBuf->p_hDevice;

    return (dwError == ERROR_SUCCESS);
}

// ----------------------------------------------------------------------------
// Function: GetDiskInfo
//     Implement IOCTL_DISK_GETINFO
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetDiskInfo(
    PIOREQ pIOReq
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DISK_INFO *pInfo = NULL;

    // for B/C, this call has three forms; only pInBuf, only pOutBuf, or both
    // if both, then use pOutBuf

    if (pIOReq->pInBuf) {
        if (pIOReq->dwInBufSize != sizeof(DISK_INFO)) {
            return ERROR_INVALID_PARAMETER;
        }
        pInfo = (DISK_INFO *)pIOReq->pInBuf;
    }

    if (pIOReq->pOutBuf) {
        if (pIOReq->dwOutBufSize!= sizeof(DISK_INFO)) {
            return ERROR_INVALID_PARAMETER;
        }
        pInfo = (DISK_INFO *)pIOReq->pOutBuf;
    }

    if (!pInfo) {
        DEBUGMSG(ZONE_ERROR|ZONE_IOCTL, (_T(
            "Atapi!CDisk::GetDiskInfo> bad argument; pInBuf/pOutBuf null\r\n")));
        return ERROR_INVALID_PARAMETER;
    }

    // TODO: if device is ATAPI, call AtapiGetDiskInfo

    if (ERROR_SUCCESS == dwError) {
        __try {
            memcpy(pInfo, &m_DiskInfo, sizeof(DISK_INFO));
            pInfo->di_flags |= DISK_INFO_FLAG_PAGEABLE;
            pInfo->di_flags &= ~DISK_INFO_FLAG_UNFORMATTED;
            if (pIOReq->pBytesReturned){
                *(pIOReq->pBytesReturned) = sizeof(DISK_INFO);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            dwError = ERROR_INVALID_PARAMETER;
        }
    }

    return dwError;
}

// ----------------------------------------------------------------------------
// Function: SetDiskInfo
//     Implement IOCTL_DISK_SETINFO
//
// Parameters:
//     pSgReq -
//     InBufLen -
// ----------------------------------------------------------------------------

DWORD
CDisk::SetDiskInfo(
    PIOREQ pIOReq
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DISK_INFO *pInfo = (DISK_INFO *)pIOReq->pInBuf;

    if ((pIOReq->pInBuf == NULL) || (pIOReq->dwInBufSize != sizeof(DISK_INFO))) {
        return ERROR_INVALID_PARAMETER;
    }

    memcpy(&m_DiskInfo, pInfo, sizeof(DISK_INFO));

    return dwError;
}

// ----------------------------------------------------------------------------
// Function: GetDeviceInfo
//     IOCTL_DISK_DEVICE_INFO
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetDeviceInfo(
    PIOREQ pIOReq
    )
{
    PSTORAGEDEVICEINFO psdi = (PSTORAGEDEVICEINFO)pIOReq->pInBuf;
    HKEY hKey;

    if ((pIOReq->dwInBufSize == 0) || (pIOReq->pInBuf == NULL)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pIOReq->pBytesReturned) {
        *(pIOReq->pBytesReturned) = sizeof(STORAGEDEVICEINFO);
    }

    psdi->dwDeviceClass = 0;
    psdi->dwDeviceType = 0;
    psdi->dwDeviceFlags = 0;

    PTSTR szProfile = psdi->szProfile;

    wcscpy(szProfile, L"Default");

    if (ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, m_szDeviceKey, 0, 0, &hKey)) {
        hKey = NULL;
    }

    if (IsAtapiDevice() && IsCDRomDevice()) {

        psdi->dwDeviceClass = STORAGE_DEVICE_CLASS_MULTIMEDIA;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_REMOVABLE_MEDIA;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_ATAPI;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_PCIIDE;
        psdi->dwDeviceFlags |= STORAGE_DEVICE_FLAG_MEDIASENSE;
        psdi->dwDeviceFlags |= STORAGE_DEVICE_FLAG_READONLY;

        if (!hKey || !AtaGetRegistryString(hKey, REG_VALUE_CDPROFILE, &szProfile, sizeof(psdi->szProfile))) {
            wcscpy(psdi->szProfile, REG_VALUE_CDPROFILE);
        }

    }
    else {

        psdi->dwDeviceClass = STORAGE_DEVICE_CLASS_BLOCK;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_PCIIDE;
        psdi->dwDeviceType |= STORAGE_DEVICE_TYPE_ATA;
        psdi->dwDeviceFlags |= STORAGE_DEVICE_FLAG_READWRITE;

        if (!hKey || !AtaGetRegistryString(hKey, REG_VALUE_2450_CFPROFILE, &szProfile, sizeof(psdi->szProfile))) {
            wcscpy(psdi->szProfile, REG_VALUE_2450_CFPROFILE);
        }

    }

    return ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------
// Function: GetDiskName
//     Implement IOCTL_DISK_GETNAME
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetDiskName(
    PIOREQ pIOReq
    )
{
    static PTCHAR szDefaultDiscDrive = (_T("External Volume"));
    PTCHAR szDiskName = NULL;
    DWORD dwSize;

    DEBUGMSG(ZONE_IOCTL, (_T("Atapi!GeDisktName\r\n")));

    if ((pIOReq->pBytesReturned == NULL) || (pIOReq->dwOutBufSize == 0) || (pIOReq->pOutBuf == NULL)) {
        return ERROR_INVALID_PARAMETER;
    }

    *(pIOReq->pBytesReturned) = 0;

    if (m_szDiskName) {
        if (wcslen(m_szDiskName)) {
            szDiskName = m_szDiskName;
        }
        else {
            return ERROR_NOT_SUPPORTED;
        }
    }
    else {
        szDiskName = szDefaultDiscDrive;
    }

    dwSize = (wcslen(szDiskName) + 1) * sizeof(TCHAR);

    if (pIOReq->dwOutBufSize < dwSize) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    wcscpy((PTCHAR) pIOReq->pOutBuf, szDiskName);

    *(pIOReq->pBytesReturned) = dwSize;

    return ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------
// Function: ReadWriteDisk
//     Implement ATA/ATAPI-6 R3B 8.34 (READ SECTOR(S)) and 8.62 (WRITE SECTOR(S)).
//     Implement ATA/ATAPI-6 R3B 9.5 (PIO data-in command protocol) and 9.6
//     (PIO data-out command protocol).
//     This function reads from/writes to an ATA device.
//
// Parameters:
//     pIOReq -
//     fRead -
//
// Notes:
//     READ SECTOR(S) and WRITE SECTOR(S) can transfer up to 256 sectors; however,
//     every transfer is segmented, as per the PIO data-in/out protocol.  A segment
//     of a multiple-block transfer is called a "DRQ data block" and is the length
//     of a sector; see ATA/ATAPI-6 R3B 9.5 and 9.6 for more information.
// ----------------------------------------------------------------------------

DWORD
CDisk::ReadWriteDisk(
    PIOREQ pIOReq,
    BOOL fRead
    )
{
    DWORD dwError = ERROR_SUCCESS; // result

    PSG_REQ pSgReq = (PSG_REQ) pIOReq->pInBuf; // scatter/gather request
    PSG_BUF pSgBuf;                            // scatter/gather buffer

    BYTE bStatus; // device Status register

    BYTE bCmd = fRead ? m_bReadCommand : m_bWriteCommand; // command

    DWORD dwCurBufNum;           // current scatter/gather buffer
    DWORD dwCurBufPos;           // current position in current scatter/gather buffer
    PBYTE pBuffer;               // pointer to current address of current scatter/gather buffer
    DWORD dwCurDoubleBufPos;     // current position in current double buffer
    DWORD dwCurDoubleBufLen;     // length of current double buffer (for reads)
    PBYTE pDoubleBuffer;         // pointer to current address of double buffer
    DWORD dwCurByte;             // current byte (to increment current sector of transfer) of "global" transfer
    DWORD dwCurSec;              // current sector of "global" transfer
    DWORD dwCurDRQDataBlockByte; // current byte of current DRQ data block transfer
    BOOL fWriteComplete;         // to facilitate PIO data-out protocol, flag transfer complete
	DWORD dwCount;	    
	DWORD dwBufferLengthAlign = TRUE;

    // validate arguments
    if ((pSgReq == NULL) || (pIOReq->dwInBufSize < sizeof(SG_REQ))) {
        return ERROR_INVALID_PARAMETER;
    }
    if ((pSgReq->sr_num_sec == 0) || (pSgReq->sr_num_sg == 0)) {
        return  ERROR_INVALID_PARAMETER;
    }
    if ((pSgReq->sr_start + pSgReq->sr_num_sec) > m_DiskInfo.di_total_sectors) {
        return ERROR_SECTOR_NOT_FOUND;
    }

    // do we have to allocate the double buffer?
    if (NULL == m_rgbDoubleBuffer) {
        DEBUGMSG(ZONE_INIT, (TEXT(
            "Atapi!CDisk::ReadWriteDisk> Allocating double buffer [first use]\r\n"
            )));
        m_rgbDoubleBuffer = (PBYTE)LocalAlloc(LPTR, m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize);
        if (NULL == m_rgbDoubleBuffer) {
            DEBUGMSG(ZONE_ERROR, (TEXT(
                "Atapi!CDisk::ReadWriteDisk> Failed to allocate double buffer\r\n"
                )));
            dwError = ERROR_OUTOFMEMORY;
            goto exit;
        }
    }

    // clear interrupt, if set
    // TODO: Is this necessary?
    // GetBaseStatus();

    // initialize "global" transfer state
    dwCurSec = pSgReq->sr_start;
    dwCurByte = 0;

    // fetch first scatter/gather buffer
    dwCurBufNum = 0;
    dwCurBufPos = 0;
    pSgBuf = &(pSgReq->sr_sglist[0]);
    // map nested pointer and test for security violation
    pBuffer = (PBYTE)MapCallerPtr((LPVOID)pSgBuf->sb_buf, pSgBuf->sb_len);
    
    if (pSgBuf->sb_buf != NULL && pBuffer == NULL) {
        // security violation
        DEBUGMSG(ZONE_ERROR, (TEXT(
            "Atapi!CDisk::ReadWriteDisk> Failed to map pointer to caller\r\n"
            )));
        return ERROR_INVALID_PARAMETER;
    }
/*    
	RETAILMSG(1, (_T(
				"ReadWrite()::: this is %d start sec 0x%x num sec 0x%x\r\n"
			),bCmd,dwCurSec,pSgReq->sr_num_sec));
*/
    // is this a read or a write?

    if ( ((pSgBuf->sb_len )% BYTES_PER_SECTOR != 0) || (pSgReq->sr_num_sg > 1) || ((UINT32)pBuffer % 4 != 0))
    	dwBufferLengthAlign = FALSE;
    	
    if (fRead) {

        // --------------------------------------------------------------------
        // ATA/ATAPI-6 R3B 9.5 (PIO data-in protocol)
        // --------------------------------------------------------------------

        // Host_Idle
        // ---------
        // issue command
PIO_Data_In_Read_Command:;
        // determine size of transfer
        if ((((pSgReq->sr_start + pSgReq->sr_num_sec) - dwCurSec) * m_DiskInfo.di_bytes_per_sect) > m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize) {
            dwCurDoubleBufLen = m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize;
        }
        else {
            dwCurDoubleBufLen = ((pSgReq->sr_start + pSgReq->sr_num_sec) - dwCurSec) * m_DiskInfo.di_bytes_per_sect;
        }

        // issue command
		if ( m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA && dwBufferLengthAlign )
		{
//			RETAILMSG(1, (_T(
//				"### Before SendIOCommand() 0x%x 0x%x \r\n"),dwCurSec,dwCurDoubleBufLen));		
			if (!SendIOCommand(dwCurSec, dwCurDoubleBufLen / m_DiskInfo.di_bytes_per_sect, 0xc8)) {
				DEBUGMSG(ZONE_ERROR, (_T(
					"Atapi!CDisk::ReadWriteDisk> Failed to issue read/write command\r\n"
				)));
				RETAILMSG(1, (_T(
					"Atapi!CDisk::ReadWriteDisk> Failed to issue read command\r\n"
				)));			
			dwError = fRead ? ERROR_READ_FAULT : ERROR_WRITE_FAULT;
			goto exit;
			}		
		}
		
		
		else
		{
			if (!SendIOCommand(dwCurSec, dwCurDoubleBufLen / m_DiskInfo.di_bytes_per_sect, bCmd)) {
				DEBUGMSG(ZONE_ERROR, (_T(
					"Atapi!CDisk::ReadWriteDisk> Failed to issue read/write command\r\n"
				)));
				RETAILMSG(1, (_T(
					"Atapi!CDisk::ReadWriteDisk> Failed to issue read command\r\n"
				)));			
			dwError = fRead ? ERROR_READ_FAULT : ERROR_WRITE_FAULT;
			goto exit;
			}
		}

		pDoubleBuffer = m_rgbDoubleBuffer;
		dwCurDoubleBufPos = 0;		
        // INTRQ_Wait
        // ----------
        // wait for interrupt if nIEN=0 (i.e., if interrupt enabled)
HPIOI_INTRQ_Wait:;
        // if nIEN=0 (interrupt enabled), wait for interrupt
        if (m_fInterruptSupported) {
     
            if (!WaitForInterrupt(m_dwDiskIoTimeOut) || (CheckIntrState() == ATA_INTR_ERROR)) {
                DEBUGMSG(ZONE_IO|ZONE_WARNING, (_T(
                    "Atapi!CDisk::ReadWriteDisk> Failed to wait for interrupt (m_dwDeviceId=%d)\r\n"
                    ), m_dwDeviceId));                    
                RETAILMSG(1, (_T(
                   "Atapi!CDisk::ReadWriteDisk> Failed to wait for interrupt (m_dwDeviceId=%d)\r\n"
                   ), m_dwDeviceId));
                dwError = ERROR_READ_FAULT;
                goto exit;
            }
            //goto HPIOI_Transfer_Data_Setup;
        }

        // Check_Status
        // ------------
        // if BSY=0 and DRQ=0, transition to Host_Idle
        // if BSY=1, re-enter this state
        // if BSY=0 and DRQ=1, transition to Transfer_Data
HPIOI_Check_Status:;
        bStatus = GetAltStatus();                                  // read Status register
        if ((!(bStatus & 0x80)) && (!(bStatus & 0x08))) goto exit; // BSY=0, DRQ=0
        if (bStatus & 0x80) goto HPIOI_Check_Status;               // BSY=1
        if ((!(bStatus & 0x80)) && (bStatus & 0x08)) goto HPIOI_Transfer_Data_Setup; // BSY=0, DRQ=1
        DEBUGCHK(FALSE);
        goto exit;

        // Transfer_Data
        // -------------
        // if read Data register, DRQ data block transfer not complete, re-enter this state
        // if raad Data register, all data for command transferred, transition to Host_Idle
        // if read Data register, DRQ data block transferred, all data for command not transferred,
        //     and nIEN=1, transition to Check_Status
        // if read Data register, DRQ data block transferred, all data for command not transferred,
        //     and nIEN=0, transition to INTRQ_Wait
HPIOI_Transfer_Data_Setup:;
        dwCurDRQDataBlockByte = 0;                      // reset DRQ data block
HPIOI_Transfer_Data:;
		if ( m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA && dwBufferLengthAlign )
		{
			DWORD dwStatus;
			/*Track Buffer 1 Setting*/
			*(UINT32 *)(m_pATAReg + ATA_TBUF_START) = (UINT32)m_DMAPhyaddress.LowPart;
			*(UINT32 *)(m_pATAReg + ATA_TBUF_SIZE) = dwCurDoubleBufLen;
			*(UINT32 *)(m_pATAReg + ATA_XFR_NUM) = dwCurDoubleBufLen;
			
			dwCount = 0x1000000;			
	        SetConfigMode(UDMA, FALSE);

 		    *(UINT32 *)(m_pATAReg + ATA_IRQ) = 0xff;   
			InterruptDone(m_pPort->m_dwSysIntr);
			*(UINT32 *)(m_pATAReg + ATA_IRQ_MASK) = 0x1e;   

			WaitForDeviceAccessReady();
			SetTransferCommand(ATA_CMD_START); // trigger PMDA

		    dwStatus = WaitForSingleObject(m_pPort->m_hIRQEvent, (dwCurDoubleBufLen/m_DiskInfo.di_bytes_per_sect)*10);
		    if (dwStatus == WAIT_TIMEOUT) 
		    {
	        	RETAILMSG(1,(TEXT("######### Wait Time out at Interrupt########\n")));
                dwError = ERROR_READ_FAULT;
                goto exit;	        	
		    }
			

/*			
	        while ( dwCount-- )
	        {
	        	if (*(UINT32 *)(m_pATAReg + ATA_IRQ) & 0x1)
	        		break;
	        }

	        if (dwCount <= 0)
	        {
	        	RETAILMSG(1,(TEXT("######### Wait Time out ########\n")));
	        	dwError =  ERROR_READ_FAULT;
	        	goto exit;
			}
*/
			
			//SetTransferCommand(ATA_CMD_ABORT); // trigger PMDA

	        SetConfigMode(PIO_CPU, FALSE);

	        //WaitForNoBusyStatus();

			*(UINT32 *)(m_pATAReg + ATA_IRQ) |= 0x1;  // clear trans_done status bit.

			dwCurSec += dwCurDoubleBufLen/m_DiskInfo.di_bytes_per_sect;

			*(UINT32 *)(m_pATAReg + ATA_CFG) &= ~(0x200);
		
			dwCurDoubleBufPos = dwCurDoubleBufLen;
			if ( dwCurDoubleBufPos == dwCurDoubleBufLen )
				goto HPIOI_Distribute_Double_Buffer;

			goto HPIOI_INTRQ_Wait;
	
		}
		

		if (m_pPort->m_pDskReg[m_dwDeviceId]->dwEnablePDMA && dwBufferLengthAlign)
		{
 		    *(UINT32 *)(m_pATAReg + ATA_IRQ) = 0xff;   
			/*Track Buffer 1 Setting*/
			*(UINT32 *)(m_pATAReg + ATA_TBUF_START) = (UINT32)m_DMAPhyaddress.LowPart + (UINT32)dwCurDoubleBufPos;
			*(UINT32 *)(m_pATAReg + ATA_TBUF_SIZE) = m_DiskInfo.di_bytes_per_sect;
			*(UINT32 *)(m_pATAReg + ATA_XFR_NUM) = m_DiskInfo.di_bytes_per_sect;
			
			dwCount = 0x10000;			
	        SetConfigMode(PIO_DMA, FALSE);

			WaitForDeviceAccessReady();
			SetTransferCommand(ATA_CMD_START); // trigger PMDA
			
	        while ( dwCount-- )
	        {
	        	if (*(UINT32 *)(m_pATAReg + ATA_IRQ) & 0x1)
	        		break;
	        }

	        if (dwCount <= 0)
	        {
	        	RETAILMSG(1,(TEXT("######### Wait Time out ########\n")));
	        	dwError =  ERROR_READ_FAULT;
	        	goto exit;
			}

	        SetConfigMode(PIO_CPU, FALSE);

			*(UINT32 *)(m_pATAReg + ATA_IRQ) |= 0x1;  // clear trans_done status bit.

			dwCurSec++;
		
			dwCurDoubleBufPos += m_DiskInfo.di_bytes_per_sect;
			if ( dwCurDoubleBufPos == dwCurDoubleBufLen )
				goto HPIOI_Distribute_Double_Buffer;

			goto HPIOI_INTRQ_Wait;
			
		}  
		
		else
		{
	        if (m_f16Bit) {
	            *((PWORD)pDoubleBuffer) = ReadWord();       // read 16-bit Data register
	            pDoubleBuffer++;
	            dwCurDRQDataBlockByte += 1;                 // increment DRQ data block
	        }
	        else {
	            *((PBYTE)pDoubleBuffer) = (BYTE)ReadByte(); // read 8-bit Data register
	        }
	        pDoubleBuffer++;
	        dwCurDRQDataBlockByte += 1;                     // increment DRQ data block
	        // is DRQ data block transferred?
	        if (dwCurDRQDataBlockByte == m_pPort->m_pDskReg[m_dwDeviceId]->dwDrqDataBlockSize) {
	            dwCurDoubleBufPos += m_pPort->m_pDskReg[m_dwDeviceId]->dwDrqDataBlockSize;
	            // has all data for command been transferred?
	            if (dwCurDoubleBufPos == dwCurDoubleBufLen) goto HPIOI_Empty_Double_Buffer;
	            // DRQ data block transferred, all data for command not transferred
	            goto HPIOI_INTRQ_Wait;
	        }
	        // DRQ data block transfer not complete
	        goto HPIOI_Transfer_Data;
		}
		
HPIOI_Empty_Double_Buffer:;

        // the double buffer has been filled, i.e., a read command has been
        // completed; distribute double buffer to scatter/gather buffers

        // initialize double buffer for empty
        pDoubleBuffer = m_rgbDoubleBuffer;
        dwCurDoubleBufPos = 0;
        // empty current double buffer into current scatter/gather buffer
HPIOI_Distribute_Double_Buffer:;
        // copy a byte from the double buffer to the current scatter/gather buffer

		if ((m_pPort->m_pDskReg[m_dwDeviceId]->dwEnablePDMA || m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA) && dwBufferLengthAlign)
		{
			
			memcpy(pBuffer + dwCurBufPos , m_pDMAVirtualAddress, dwCurDoubleBufLen);
			
			//dwCurSec += dwCurDoubleBufLen/m_DiskInfo.di_bytes_per_sect;
			dwCurBufPos += dwCurDoubleBufLen;
	        // have we filled the current scatter/gather buffer?
	        if (dwCurBufPos == pSgBuf->sb_len) {
/*	        
	        	if ( pSgBuf->sb_len == 512 && pSgReq->sr_start== 0x42f)
	        	{	
	        		for(int i=0;i<512;i++)
	        		{
	        			RETAILMSG(1,(TEXT("%2x "),*(m_pDMAVirtualAddress+i)));
	        			if ( i % 8 == 7 )
	        				RETAILMSG(1,(TEXT("\r\n")));	
	        		}
	        	}
*/	        	
	            // have we filled the scatter/gather request?
	            if ((dwCurBufNum + 1) == pSgReq->sr_num_sg) goto exit;
	            // fetch next scatter/gather buffer
	            dwCurBufNum += 1;
	            dwCurBufPos = 0;
	            pSgBuf = &(pSgReq->sr_sglist[dwCurBufNum]);
	            pBuffer = (PBYTE)MapCallerPtr((LPVOID)pSgBuf->sb_buf, pSgBuf->sb_len );
	            if (pSgBuf->sb_buf != NULL && pBuffer == NULL) {
	                // security violation
	                DEBUGMSG(ZONE_ERROR, (TEXT(
	                    "Atapi!CDisk::ReadWriteDisk> Failed to map pointer to caller\r\n"
	                    )));
	                return ERROR_INVALID_PARAMETER;
	            }

				if ( (pSgBuf->sb_len )% BYTES_PER_SECTOR != 0 )
			    	dwBufferLengthAlign = FALSE;
	        }
			goto PIO_Data_In_Read_Command;
		} 
		
		else
		{
	        *pBuffer = m_rgbDoubleBuffer[dwCurDoubleBufPos];
	        pBuffer++;
	        dwCurBufPos += 1;
	        dwCurDoubleBufPos += 1;
	        // increment current sector, if necessary
	        dwCurByte += 1;
	        if (dwCurByte == m_DiskInfo.di_bytes_per_sect) {
	            dwCurSec += 1;
	            dwCurByte = 0;
	        }
		
	        // have we filled the current scatter/gather buffer?
	        if (dwCurBufPos == pSgBuf->sb_len) {
	            // have we filled the scatter/gather request?
	            if ((dwCurBufNum + 1) == pSgReq->sr_num_sg) goto exit;
	            // fetch next scatter/gather buffer
	            dwCurBufNum += 1;
	            dwCurBufPos = 0;
	            pSgBuf = &(pSgReq->sr_sglist[dwCurBufNum]);
	            pBuffer = (PBYTE)MapCallerPtr((LPVOID)pSgBuf->sb_buf, pSgBuf->sb_len);
	            if (pSgBuf->sb_buf != NULL && pBuffer == NULL) {
	                // security violation
	                DEBUGMSG(ZONE_ERROR, (TEXT(
	                    "Atapi!CDisk::ReadWriteDisk> Failed to map pointer to caller\r\n"
	                    )));
	                return ERROR_INVALID_PARAMETER;
	            }
	        }
	        // have we emptied the double buffer?
	        if (dwCurDoubleBufPos == dwCurDoubleBufLen) goto PIO_Data_In_Read_Command;
	        goto HPIOI_Distribute_Double_Buffer;
		}
	}
    
    else {

        // --------------------------------------------------------------------
        // ATA/ATAPI-6 R3B 9.6 (PIO data-out protocol)
        // --------------------------------------------------------------------

        // Host_Idle
        // ---------
        // issue command
PIO_Data_In_Write_Command:;
        // determine size of transfer
        if ((((pSgReq->sr_start + pSgReq->sr_num_sec) - dwCurSec) * m_DiskInfo.di_bytes_per_sect) > m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize) {
            dwCurDoubleBufLen = m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize;
        }
        else {
            dwCurDoubleBufLen = ((pSgReq->sr_start + pSgReq->sr_num_sec) - dwCurSec) * m_DiskInfo.di_bytes_per_sect;
        }
        // issue command
		if ( m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA && dwBufferLengthAlign )
		{
//			RETAILMSG(1, (_T(
//				"### Before SendIOCommand() 0x%x 0x%x \r\n"),dwCurSec,dwCurDoubleBufLen));		
			if (!SendIOCommand(dwCurSec, dwCurDoubleBufLen / m_DiskInfo.di_bytes_per_sect, 0xca)) {
				DEBUGMSG(ZONE_ERROR, (_T(
					"Atapi!CDisk::ReadWriteDisk> Failed to issue read/write command\r\n"
				)));
				RETAILMSG(1, (_T(
					"Atapi!CDisk::ReadWriteDisk> Failed to issue read command\r\n"
				)));			
			dwError = fRead ? ERROR_READ_FAULT : ERROR_WRITE_FAULT;
			goto exit;
			}		
		}
		else
		{
	        if (!SendIOCommand(dwCurSec, dwCurDoubleBufLen / m_DiskInfo.di_bytes_per_sect, bCmd)) {
	            DEBUGMSG(ZONE_ERROR, (_T(
	                "Atapi!CDisk::ReadWriteDisk> Failed to issue read/write command\r\n"
	                )));
				RETAILMSG(1, (_T(
					"Atapi!CDisk::ReadWriteDisk> Failed to issue write command\r\n"
				)));	
	            dwError = fRead ? ERROR_READ_FAULT : ERROR_WRITE_FAULT;
	            goto exit;
	        }
        }

        fWriteComplete = FALSE; // mark write as in progress

        // the double buffer is empty, i.e., a write command is about to be
        // issued; fill the double buffer from the scatter/gather buffers

        // initialize double buffer for fill
        pDoubleBuffer = m_rgbDoubleBuffer;
        dwCurDoubleBufPos = 0;
        // fill current double buffer from current scatter/gather buffer

		if ((m_pPort->m_pDskReg[m_dwDeviceId]->dwEnablePDMA || m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA) && dwBufferLengthAlign )
		{
			
			memcpy(m_pDMAVirtualAddress , pBuffer + dwCurBufPos, dwCurDoubleBufLen);
			
			dwCurSec += dwCurDoubleBufLen/m_DiskInfo.di_bytes_per_sect;
			dwCurBufPos += dwCurDoubleBufLen;
	        // have we filled the current scatter/gather buffer?

			if (dwCurBufPos == pSgBuf->sb_len ) {
		       // have we filled the scatter/gather request?
	            if ((dwCurBufNum + 1) == pSgReq->sr_num_sg) 
					fWriteComplete = TRUE;
				else
				{
	            	// fetch next scatter/gather buffer
	                dwCurBufNum += 1;
	                dwCurBufPos = 0;
	                pSgBuf = &(pSgReq->sr_sglist[dwCurBufNum]);
	                pBuffer = (PBYTE)MapCallerPtr((LPVOID)pSgBuf->sb_buf, pSgBuf->sb_len);
	                if (pSgBuf->sb_buf != NULL && pBuffer == NULL) {
	                    // security violation
	                    DEBUGMSG(ZONE_ERROR, (TEXT(
	                        "Atapi!CDisk::ReadWriteDisk> Failed to map pointer to caller\r\n"
	                        )));
	                    return ERROR_INVALID_PARAMETER;
                	}
				}
				if ( (pSgBuf->sb_len )% BYTES_PER_SECTOR != 0 )
			    	dwBufferLengthAlign = FALSE;				
			}
		} 
		else
		{
	        while (1) {
	            // copy a byte from the scatter/gather buffer to the double buffer
	            PREFAST_DEBUGCHK(pBuffer);
	            m_rgbDoubleBuffer[dwCurDoubleBufPos] = *pBuffer;
	            pBuffer++;
	            dwCurDoubleBufPos += 1;
	            dwCurBufPos += 1;
	            // increment current sector, if necessary
	            dwCurByte++;
	            if (dwCurByte == m_DiskInfo.di_bytes_per_sect) {
	                dwCurSec += 1;
	                dwCurByte = 0;
	            }
	            // have we emptied the current scatter/gather buffer?
	            if (dwCurBufPos == pSgBuf->sb_len) {
	                // have we emptied the scatter/gather request?
	                if ((dwCurBufNum + 1) == pSgReq->sr_num_sg) {
	                    fWriteComplete = TRUE; // mark write as complete
	                    break;
	                }
	                // fetch next scatter/gather buffer
	                dwCurBufNum += 1;
	                dwCurBufPos = 0;
	                pSgBuf = &(pSgReq->sr_sglist[dwCurBufNum]);
	                pBuffer = (PBYTE)MapCallerPtr((LPVOID)pSgBuf->sb_buf, pSgBuf->sb_len);
	                if (pSgBuf->sb_buf != NULL && pBuffer == NULL) {
	                    // security violation
	                    DEBUGMSG(ZONE_ERROR, (TEXT(
	                        "Atapi!CDisk::ReadWriteDisk> Failed to map pointer to caller\r\n"
	                        )));
	                    return ERROR_INVALID_PARAMETER;
	                }
	            }
	            // have we filled the double buffer?
	            if (dwCurDoubleBufPos == m_pPort->m_pDskReg[m_dwDeviceId]->dwDoubleBufferSize) break;
	        } // while
		}
        // initialize current double buffer for empty
        pDoubleBuffer = m_rgbDoubleBuffer;

        // Check_Status
        // ------------
        // if BSY=0 and DRQ=0, transition to Host_Idle
        // if BSY=1, re-enter this state
        // if BSY=0 and DRQ=1, transition to Transfer_Data
HPIOO_Check_Status:;
        bStatus = GetAltStatus();                         // read Status register
        if ((!(bStatus & 0x80)) && (!(bStatus & 0x08))) { // BSY=0, DRQ=0
           		if (fWriteComplete) goto exit;                // if the entire write is complete, exit
            goto PIO_Data_In_Write_Command;               // entire write is not complete, issue next transfer
        }
        if (bStatus & 0x80) goto HPIOO_Check_Status;      // BSY=1
        if ((!(bStatus & 0x80)) && (bStatus & 0x08)) goto HPIOO_Transfer_Data_Reset_DRQ_Data_Block; // BSY=0, DRQ=1
        DEBUGCHK(FALSE);
        goto exit;

        // Transfer_Data
        // -------------
        // if write Data register, DRQ data block transfer not complete, re-enter this state
        // if write Data register, all data for command transferred, transition to Host_Idle
        // if write Data register, DRQ data block transferred, all data for command not transferred,
        //     and nIEN=1, transition to Check_Status
        // if write Data register, DRQ data block transferred, all data for command not transferred,
        //     and nIEN=0, transition to INTRQ_Wait
HPIOO_Transfer_Data_Reset_DRQ_Data_Block:;
        dwCurDRQDataBlockByte = 0;                            // reset DRQ data block
HPIOO_Transfer_Data:;
		if (m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA && dwBufferLengthAlign)
		{
			DWORD dwStatus;
 		    *(UINT32 *)(m_pATAReg + ATA_IRQ) = 0xff;   
			/*Track Buffer 1 Setting*/
			*(UINT32 *)(m_pATAReg + ATA_SBUF_START) = (UINT32)m_DMAPhyaddress.LowPart;
			*(UINT32 *)(m_pATAReg + ATA_SBUF_SIZE) = dwCurDoubleBufLen;
			*(UINT32 *)(m_pATAReg + ATA_XFR_NUM) = dwCurDoubleBufLen;
			
			dwCount = 0x1000000;			
	        SetConfigMode(UDMA, TRUE);

 		    *(UINT32 *)(m_pATAReg + ATA_IRQ) = 0xff;   
			InterruptDone(m_pPort->m_dwSysIntr);
			*(UINT32 *)(m_pATAReg + ATA_IRQ_MASK) = 0x1e; 
			
			WaitForDeviceAccessReady();
			SetTransferCommand(ATA_CMD_START); // trigger PMDA

		    dwStatus = WaitForSingleObject(m_pPort->m_hIRQEvent, (dwCurDoubleBufLen/m_DiskInfo.di_bytes_per_sect)*10);
		    if (dwStatus == WAIT_TIMEOUT) 
		    {
	        	RETAILMSG(1,(TEXT("######### Wait Time out at Interrupt########\n")));
                dwError = ERROR_READ_FAULT;
                goto exit;	        	
		    }

			
/*
	        while ( dwCount-- )
	        {
	        	if (*(UINT32 *)(m_pATAReg + ATA_IRQ) & 0x1)
	        		break;
	        }

	        if (dwCount <= 0)
	        {
	        	RETAILMSG(1,(TEXT("######### Wait Time out ########\n")));
	        	dwError =  ERROR_WRITE_FAULT;
	        	goto exit;
			}
*/
//			SetTransferCommand(ATA_CMD_ABORT); 
	        SetConfigMode(PIO_CPU, FALSE);

	        //SetTransferCommand(ATA_CMD_ABORT); // trigger PMDA

			*(UINT32 *)(m_pATAReg + ATA_IRQ) |= 0x1;  // clear trans_done status bit.

			dwCurDoubleBufPos += dwCurDoubleBufLen;
			
			goto HPIOO_Check_Status;
		}
		
		else if (m_pPort->m_pDskReg[m_dwDeviceId]->dwEnablePDMA && dwBufferLengthAlign)
		{
 		    *(UINT32 *)(m_pATAReg + ATA_IRQ) = 0xff;   
			/*Track Buffer 1 Setting*/
			*(UINT32 *)(m_pATAReg + ATA_SBUF_START) = (UINT32)m_DMAPhyaddress.LowPart + (UINT32)dwCurDoubleBufPos;
			*(UINT32 *)(m_pATAReg + ATA_SBUF_SIZE) = m_DiskInfo.di_bytes_per_sect;
			*(UINT32 *)(m_pATAReg + ATA_XFR_NUM) = m_DiskInfo.di_bytes_per_sect;
			
			dwCount = 0x10000;			
	        SetConfigMode(PIO_DMA, TRUE);

			WaitForDeviceAccessReady();
			SetTransferCommand(ATA_CMD_START); // trigger PMDA
			
	        while ( dwCount-- )
	        {
	        	if (*(UINT32 *)(m_pATAReg + ATA_IRQ) & 0x1)
	        		break;
	        }

	        if (dwCount <= 0)
	        {
	        	RETAILMSG(1,(TEXT("######### Wait Time out ########\n")));
	        	dwError =  ERROR_WRITE_FAULT;
	        	goto exit;
			}

//			SetTransferCommand(ATA_CMD_ABORT); 
	        SetConfigMode(PIO_CPU, FALSE);

	        //SetTransferCommand(ATA_CMD_ABORT); // trigger PMDA

			*(UINT32 *)(m_pATAReg + ATA_IRQ) |= 0x1;  // clear trans_done status bit.

			dwCurDoubleBufPos += m_DiskInfo.di_bytes_per_sect;
			
			goto HPIOO_Check_Status;
			
		}
		else
		{
	        if (m_f16Bit) {
	            WriteWord(*((PWORD)pDoubleBuffer));               // write 16-bit Data register
	            pDoubleBuffer++;
	            dwCurDRQDataBlockByte += 1;                       // increment DRQ data block
	        }
	        else {
	            WriteByte(*((PBYTE)pDoubleBuffer));               // write 8-bit Data register
	        }
	        pDoubleBuffer++;
	        dwCurDRQDataBlockByte += 1;                           // increment DRQ data block
	        // is DRQ data block transferred?
	        if (dwCurDRQDataBlockByte == m_pPort->m_pDskReg[m_dwDeviceId]->dwDrqDataBlockSize) {
	            // is the transfer complete?
	            if (m_fInterruptSupported) goto HPIOO_INTRQ_Wait; // DRQ data block transferred, nIEN=0
	            goto HPIOO_Check_Status;                          // DRQ data block transferred, nIEN=1
	        }
	        goto HPIOO_Transfer_Data;                             // DRQ data block transfer not complete
		}
	        // INTRQ_Wait
	        // ----------
	        // wait for interrupt if nIEN=0 (i.e., if interrupt enabled)
HPIOO_INTRQ_Wait:;
        // if nIEN=0 (interrupt enabled), wait for interrupt
        if (!WaitForInterrupt(m_dwDiskIoTimeOut) || (CheckIntrState() == ATA_INTR_ERROR)) {
            DEBUGMSG(ZONE_IO|ZONE_WARNING, (_T(
                "Atapi!CDisk::ReadWriteDisk> Failed to wait for interrupt (m_dwDeviceId=%d)\r\n"
                ), m_dwDeviceId));
            dwError = ERROR_READ_FAULT;
            goto exit;
        }
        goto HPIOO_Check_Status;
    } // if

exit:;
    pSgReq->sr_status = dwError;
    return dwError;
}

// ----------------------------------------------------------------------------
// Function: ReadWriteDiskDMA
//     This function reads from/writes to an ATA device
//
// Parameters:
//     pIOReq -
//     fRead -
// ----------------------------------------------------------------------------

DWORD
CDisk::ReadWriteDiskDMA(
    PIOREQ pIOReq,
    BOOL fRead
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSG_REQ pSgReq = (PSG_REQ)pIOReq->pInBuf;
    DWORD dwSectorsToTransfer;
    SG_BUF CurBuffer[MAX_SG_BUF];
    BYTE bCmd;

    if ((pSgReq == NULL) || (pIOReq->dwInBufSize < sizeof(SG_REQ))) {
        return ERROR_INVALID_PARAMETER;
    }
    if ((pSgReq->sr_num_sec == 0) || (pSgReq->sr_num_sg == 0)) {
        return  ERROR_INVALID_PARAMETER;
    }
    if ((pSgReq->sr_start + pSgReq->sr_num_sec) > m_DiskInfo.di_total_sectors) {
        return ERROR_SECTOR_NOT_FOUND;
    }

    DEBUGMSG(ZONE_IO, (_T(
        "Atapi!ReadWriteDiskDMA> sr_start(%ld), sr_num_sec(%ld), sr_num_sg(%ld)\r\n"
        ), pSgReq->sr_start, pSgReq->sr_num_sec, pSgReq->sr_num_sg));

    GetBaseStatus(); // clear pending interrupt

    DWORD dwStartBufferNum = 0, dwEndBufferNum = 0, dwEndBufferOffset = 0;
    DWORD dwNumSectors = pSgReq->sr_num_sec;
    DWORD dwStartSector = pSgReq->sr_start;

    // process scatter/gather buffers in groups of MAX_SEC_PER_COMMAND sectors
    // each DMA request handles a new SG_BUF array which is a subset of the
    // original request, and may start/stop in the middle of the original buffer

    while (dwNumSectors) {

        // determine number of sectors to transfer
        dwSectorsToTransfer = (dwNumSectors > MAX_SECT_PER_COMMAND) ? MAX_SECT_PER_COMMAND : dwNumSectors;

        // determine size (in bytes) of transfer
        DWORD dwBufferLeft = dwSectorsToTransfer * BYTES_PER_SECTOR;

        DWORD dwNumSg = 0;

        // while the transfer is not complete
        while (dwBufferLeft) {

            // determine the size of the current scatter/gather buffer
            DWORD dwCurBufferLen = pSgReq->sr_sglist[dwEndBufferNum].sb_len - dwEndBufferOffset;

            if (dwBufferLeft < dwCurBufferLen) {
                CurBuffer[dwEndBufferNum - dwStartBufferNum].sb_buf = pSgReq->sr_sglist[dwEndBufferNum].sb_buf + dwEndBufferOffset;
                CurBuffer[dwEndBufferNum - dwStartBufferNum].sb_len = dwBufferLeft;
                dwEndBufferOffset += dwBufferLeft;
                dwBufferLeft = 0;
            }
            else {
                CurBuffer[dwEndBufferNum - dwStartBufferNum].sb_buf = pSgReq->sr_sglist[dwEndBufferNum].sb_buf + dwEndBufferOffset;
                CurBuffer[dwEndBufferNum - dwStartBufferNum].sb_len = dwCurBufferLen;
                dwEndBufferOffset = 0;
                dwEndBufferNum++;
                dwBufferLeft -= dwCurBufferLen;
            }
            dwNumSg++;
        }

		bCmd = fRead ? m_bDMAReadCommand : m_bDMAWriteCommand ;

        WaitForInterrupt(0);

        // setup the DMA transfer
        if (!SetupDMA(CurBuffer, dwNumSg, fRead)) {
            dwError = fRead ? ERROR_READ_FAULT : ERROR_WRITE_FAULT;
            goto ExitFailure;
        }

        // write the read/write command
        if (!SendIOCommand(dwStartSector, dwSectorsToTransfer, bCmd)) {
            dwError = fRead ? ERROR_READ_FAULT : ERROR_WRITE_FAULT;
            AbortDMA();
            goto ExitFailure;
        }

        // start the DMA transfer
        if (BeginDMA(fRead)) {
            if (m_fInterruptSupported) {
                if (!WaitForInterrupt(m_dwDiskIoTimeOut) || (CheckIntrState() == ATA_INTR_ERROR)) {
                    DEBUGMSG(ZONE_IO || ZONE_WARNING, (_T(
                        "Atapi!CDisk::ReadWriteDiskDMA> Failed to wait for interrupt; device(%d)\r\n"
                        ), m_dwDeviceId));
                    dwError = ERROR_READ_FAULT;
                    AbortDMA();
                    goto ExitFailure;
                }
            }
            // stop the DMA transfer
            if (EndDMA()) {
                WaitOnBusy(FALSE);
                CompleteDMA(CurBuffer, pSgReq->sr_num_sg, fRead);
            }
        }

        // update transfer
        dwStartSector += dwSectorsToTransfer;
        dwStartBufferNum = dwEndBufferNum;
        dwNumSectors -= dwSectorsToTransfer;
    }

ExitFailure:
    if (ERROR_SUCCESS != dwError) {
        ResetController();
    }
    pSgReq->sr_status = dwError;
    return dwError;
}

// ----------------------------------------------------------------------------
// Function: GetStorageId
//     Implement IOCTL_DISK_GET_STORAGEID
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CDisk::GetStorageId(
    PIOREQ pIOReq
    )
{
    DWORD dwBytesLeft;
    PBYTE pDstOffset;
    PSTORAGE_IDENTIFICATION pStorageId = (PSTORAGE_IDENTIFICATION)pIOReq->pOutBuf;

    // validate arguments
    if (!pStorageId || (pIOReq->dwOutBufSize < sizeof(STORAGE_IDENTIFICATION)) || !pIOReq->pBytesReturned) {
        return ERROR_INVALID_PARAMETER;
    }

    // prepare return structure
    pStorageId->dwSize = sizeof(STORAGE_IDENTIFICATION);
    pStorageId->dwFlags = 0; // {MANUFACTUREID,SERIALNUM}_INVALID

    // prepare return structure indicies, for write
    dwBytesLeft = pIOReq->dwOutBufSize - sizeof(STORAGE_IDENTIFICATION);
    pDstOffset = (PBYTE)(pStorageId + 1);
    pStorageId->dwManufactureIDOffset = pDstOffset - (PBYTE)pStorageId;

    SetLastError(ERROR_SUCCESS);

    // fetch manufacturer ID
    if (!ATAParseIdString((PBYTE)m_Id.ModelNumber, sizeof(m_Id.ModelNumber), &(pStorageId->dwManufactureIDOffset), &pDstOffset, &dwBytesLeft)) {
        pStorageId->dwFlags |= MANUFACTUREID_INVALID;
    }
    pStorageId->dwSerialNumOffset = pDstOffset - (PBYTE)pStorageId;
    // fetch serial number
    if (!ATAParseIdString((PBYTE)m_Id.SerialNumber, sizeof(m_Id.SerialNumber), &(pStorageId->dwSerialNumOffset), &pDstOffset, &dwBytesLeft)) {
        pStorageId->dwFlags |= SERIALNUM_INVALID;
    }
    pStorageId->dwSize = pDstOffset - (PBYTE)pStorageId;

    // store bytes written
    *(pIOReq->pBytesReturned)= min(pStorageId->dwSize, pIOReq->dwOutBufSize);

    return GetLastError();
}

// ----------------------------------------------------------------------------
// Function: SetWriteCacheMode
//     Issue SET FEATURES enable write cache command
//
// Parameters:
//     fEnable -
// ----------------------------------------------------------------------------

BOOL
CDisk::SetWriteCacheMode(
    BOOL fEnable
    )
{
    BYTE bError, bStatus;

    // select device
    SelectDevice();

    // wait for device to acknowledge selection
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 100);
    WaitForDisc(WAIT_TYPE_READY, 1000);
    WaitOnBusy(TRUE);

    // write command
    SelectDevice();
    WriteFeature(fEnable ? ATA_ENABLE_WRITECACHE : ATA_DISABLE_WRITECACHE);
    WriteCommand(ATAPI_CMD_SET_FEATURES);

    // wait for device to respond to command
    SelectDevice();
    WaitOnBusy(TRUE);
    SelectDevice();
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 200);

    // check response
    bStatus = GetBaseStatus();
    bError = GetError();
    if ((bStatus & ATA_STATUS_ERROR) && (bError & ATA_ERROR_ABORTED)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "Atapi!CDisk::SetWriteCacheMode> Failed to enable write cache; status(%02X), error(%02X)\r\n"
            ), bStatus, bError));
        ResetController(FALSE);
        return FALSE;
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: SetLookAhead
//     Issue SET FEATURES enable read look-ahead command
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CDisk::SetLookAhead(
    )
{
    BYTE bError = 0;
    BYTE bStatus = 0;

    // select device
    SelectDevice();

    // wait for device to acknowledge selection
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 100);
    WaitForDisc(WAIT_TYPE_READY, 1000);
    WaitOnBusy(TRUE);

    // write command
    SelectDevice();
    WriteFeature(ATA_ENABLE_LOOKAHEAD);
    WriteCommand(ATAPI_CMD_SET_FEATURES);

    // wait for device to respond to command
    SelectDevice();
    WaitOnBusy(TRUE);
    SelectDevice();
    WaitForDisc(WAIT_TYPE_NOT_BUSY, 200);

    // check response
    bStatus = GetBaseStatus();
    bError = GetError();
    if ((bStatus & ATA_STATUS_ERROR) && (bError & ATA_ERROR_ABORTED)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "Atapi!CDisk::SetLookAhead> Failed to enable read look-ahead; status(%02X), error(%02X)\r\n"
            ), bStatus, bError));
        ResetController(FALSE);
        return FALSE;
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: FlushCache
//     Issue FLUSH CACHE command; this command may take up to 30s to complete
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

DWORD
CDisk::FlushCache(
    )
{
    BYTE bError = 0;
    BYTE bStatus = 0;
    BOOL fOk = TRUE;

    // if write cache is not enabled, then fail
    if (!(m_dwDeviceFlags & DFLAGS_USE_WRITE_CACHE)) {
        return ERROR_NOT_SUPPORTED;
    }

    // HI:Check_Status (Host Idle); wait until BSY=0 and DRQ=0
    // read Status register
    while (1) {
        bStatus = GetAltStatus();
        if (!(bStatus & (0x80|0x08))) break; // BSY := Bit 7, DRQ := Bit 3
        Sleep(5);
    }

    // HI:Device_Select; select device
    SelectDevice();

    // HI:Check_Status (Host Idle); wait until BSY=0 and DRQ=0
    // read Status register
    while (1) {
        bStatus = GetAltStatus();
        if (!(bStatus & (0x80|0x08))) break; // BSY := Bit 7, DRQ := Bit 3
        Sleep(5);
    }

    // HI:Write_Command
    WriteCommand(ATA_CMD_FLUSH_CACHE);

    // transition to non-data command protocol

    // HND:INTRQ_Wait
    // transition to HND:Check_Status
    // read Status register
    while (1) { // BSY := Bit 7
        bStatus = GetAltStatus();
        bError = GetError();
        if (bError & 0x04) { // ABRT := Bit 2
            // command was aborted
            DEBUGMSG(ZONE_ERROR, (_T(
                "Atapi!CDisk::FlushCache> Failed to send FLUSH CACHE\r\n"
                )));
            fOk = FALSE;
            break;
        }
        if (!(bStatus & 0x80)) break; // BSY := Bit 7
        Sleep(5);
    }

    // transition to host idle protocol

    return (fOk ? ERROR_SUCCESS : ERROR_GEN_FAILURE);
}

int
CDisk::WaitForDeviceAccessReady(
    )
{
	UINT32 tempRead;
	UINT8  retVal=TRUE;
	UINT32 count=1;

	do {
		//Inp32(ATA_FIFO_STATUS, tempRead); // modified by Bryan W. Lee (Oct.19th, 2005)
		if ( count == 10000000)
		{
			retVal=FALSE;
			RETAILMSG(1, (TEXT("+++ ATAPI_CF : WaitForDeviceAccessReady error..\r\n")));	
			break;
		}
		count++;
		tempRead = *((UINT32 *)(m_pATAReg + ATA_FIFO_STATUS));
	} while((tempRead>>28)!=0);
	return retVal;
}

BYTE 
CDisk::ATA_READ_BYTE(PBYTE p)
{
	BYTE ret;

	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_READ_BYTE();;;;;;;;;;;;;;\n")));
	}
	ret = READ_PORT_UCHAR(p);
	
	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_READ_BYTE();;;;;;;;;;;;;;\n")));
	}
	ret = ReadByteFromRDATA();
	
	return ret;
}


USHORT 
CDisk::ATA_READ_WORD(PUSHORT p)
{
	WORD ret;
	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_READ_USHORT();;;;;;;;;;;;;;\n")));
	}	
	ret = READ_PORT_USHORT(p);
	
	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_READ_USHORT();;;;;;;;;;;;;;\n")));
	}
	ret = ReadWordFromRDATA();
	
	return ret;
}

ULONG 
CDisk::ATA_READ_DWORD(PULONG p)
{
	ULONG ret;
	
	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_READ_ULONG();;;;;;;;;;;;;;\n")));
	}	
	ret = READ_PORT_ULONG(p);
	
	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_READ_ULONG();;;;;;;;;;;;;;\n")));
	}
	ret = ReadDWordFromRDATA();
	
	return ret;
}

void
CDisk::ATA_WRITE_BYTE(PBYTE p,BYTE v)
{
	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_WRITE_UCHAR();;;;;;;;;;;;;;\n")));
	}
	
	WRITE_PORT_UCHAR(p,v);
	
}

void
CDisk::ATA_WRITE_WORD(PUSHORT p,USHORT v)
{
	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_WRITE_USHORT();;;;;;;;;;;;;;\n")));
	}

	WRITE_PORT_USHORT(p,v);
}

void
CDisk::ATA_WRITE_DWORD(PULONG p,ULONG v)
{
	if (!WaitForDeviceAccessReady())
	{
		RETAILMSG(1,(TEXT("Error in ATA_WRITE_ULONG();;;;;;;;;;;;;;\n")));
	}

	WRITE_PORT_ULONG(p,v);
}

void 
CDisk::ReadByteBuffer(PBYTE pBuffer, DWORD dwCount)
{
	PBYTE temp = pBuffer;
	
	for (DWORD i = 0 ; i < dwCount ; i++ )
	{	
		*(temp+i) = ATA_READ_BYTE(m_pATAReg + ATA_PIO_DTR);
	}
}

void 
CDisk::ReadWordBuffer(PWORD pBuffer, DWORD dwCount)
{
	PWORD temp = pBuffer;
	
	for (DWORD i = 0 ; i < dwCount ; i++ )
	{	
		*(temp+i) = ATA_READ_WORD((PWORD)(m_pATAReg + ATA_PIO_DTR));
	}
}

void 
CDisk::WriteByteBuffer(PBYTE pBuffer, DWORD dwCount)
{
	PBYTE temp = pBuffer;
	
	for (DWORD i = 0 ; i < dwCount ; i++ )
	{	
		ATA_WRITE_BYTE((m_pATAReg + ATA_PIO_DTR),*(temp+i));
	}
}

void 
CDisk::WriteWordBuffer(PWORD pBuffer, DWORD dwCount)
{
	PWORD temp = pBuffer;
	
	for (DWORD i = 0 ; i < dwCount ; i++ )
	{	
		ATA_WRITE_WORD((PWORD)(m_pATAReg + ATA_PIO_DTR),*(temp+i));
	}
}


int 
CDisk::WaitForNoBusyStatus(void)
{
	UINT8 tempRead;
	UINT32 count=1;
	UINT16 retVal=TRUE;

	while(1) 
	{
		count++;
		tempRead = ATA_READ_BYTE(m_pATAReg + ATA_PIO_DAD);
		tempRead = ATA_READ_BYTE(m_pATAReg + ATA_PIO_CSD);

		if((tempRead & ATA_STATUS_BUSY) == 0) 
			break;
		if(count == 1000000 )
		{
			retVal = FALSE;
			break;
		}
	}
	return retVal;
}



void 
CDisk::SetConfigMode(int mode, int isWriteMode)
{
	UINT32 tempCfg;

	tempCfg = *(UINT32 *)(m_pATAReg + ATA_CFG);
	switch(mode)
	{
		case PIO_CPU:
			tempCfg = ((tempCfg&0x1F3) | (0<<2)); // set PIO_CPU class
			break;
		case PIO_DMA:
			tempCfg = ((tempCfg&0x1e3) | (1<<2)); // set PDMA class
			if (isWriteMode == TRUE)
				tempCfg |= 0x10; // DMA write mode
			//else
			//	tempCfg &= ~(0x10); // DMA read mode
			break;
		case UDMA:
			tempCfg = ((tempCfg&0x1F3) | (2<<2)); // set UDMA class
			tempCfg |= 0x200; // set ATA DMA auto mode (enable multi block transfer)
			if (isWriteMode == TRUE)
				tempCfg |= 0x10; // DMA write mode
			else
				tempCfg &= (~0x10); // DMA read mode
			break;			
		default:
			break;
	}
	*(UINT32 *)(m_pATAReg + ATA_CFG) = tempCfg;
}


int 
CDisk::SetTransferCommand(UINT32 command)
{
	if (!(WaitForDeviceAccessReady()))
		return FALSE;
	*(UINT32 *)(m_pATAReg + ATA_COMMAND) = command;
	return TRUE;
}




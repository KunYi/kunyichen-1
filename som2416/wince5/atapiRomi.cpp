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

/*++

Module Name:
    atapiRomi.cpp
        based on atapipci.cpp

Abstract:
    ATA/ATAPI for 2440 Ref Board.

Revision History:

--*/

#include "atapiRomi.h"
#include "atapiRomipm.h"
#include "bsp_cfg.h"

#define RTL_MSG 1


LONG CRomiDisk::m_lDeviceCount = 0;

static TCHAR *g_szPCICDRomDisk = TEXT("CD-ROM");
static TCHAR *g_szPCIHardDisk = TEXT("Hard Disk");

// ----------------------------------------------------------------------------
// Function: CreatePCIHD
//     Spawn function called by IDE/ATA controller enumerator
//
// Parameters:
//     hDevKey -
// ----------------------------------------------------------------------------


EXTERN_C
CDisk *
CreateRomi(
    HKEY hDevKey
    )
{
    return new CRomiDisk(hDevKey);
}

// ----------------------------------------------------------------------------
// Function: CRomiDisk
//     Constructor
//
// Parameters:
//     hKey -
// ----------------------------------------------------------------------------

CRomiDisk::CRomiDisk(
    HKEY hKey
    ) : CDisk(hKey)
{
    m_pStartMemory = NULL;
    m_pPort = NULL;
    m_pPhysList = NULL;
    m_pSGCopy = NULL;
    m_pPFNs = NULL;
    m_pPRDPhys = 0;
    m_pPRD = NULL;
    m_dwPhysCount = 0;
    m_dwSGCount = 0;
    m_dwPRDSize = 0;
    m_pBMCommand = NULL;

    InterlockedIncrement(&m_lDeviceCount);

    DEBUGMSG(ZONE_INIT|ZONE_PCI, (_T(
        "Atapi!CRomiDisk::CRomiDisk> device count(%d)\r\n"
        ), m_lDeviceCount));
}

// ----------------------------------------------------------------------------
// Function: ~CRomiDisk
//     Destructor
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

CRomiDisk::~CRomiDisk(
    )
{
    FreeDMABuffers();

/*
	if ( m_vpATAPIRegs )
	{
		VirtualFree((PVOID)m_vpATAPIRegs, 0, MEM_RELEASE); 
		m_vpATAPIRegs = NULL;
	}
*/
	if ( m_vpEBIRegs )
	{
		VirtualFree((PVOID)m_vpEBIRegs, 0, MEM_RELEASE); 
		m_vpEBIRegs= NULL;
	}

	if ( m_vpIOPORTRegs )
	{
		VirtualFree((PVOID)m_vpIOPORTRegs, 0, MEM_RELEASE); 
		m_vpIOPORTRegs = NULL;
	}


    InterlockedDecrement(&m_lDeviceCount);

    DEBUGMSG(ZONE_INIT|ZONE_PCI, (_T(
        "Atapi!CRomiDisk::~CRomiDisk> device count(%d)\r\n"
        ), m_lDeviceCount));
}

// ----------------------------------------------------------------------------
// Function: FreeDMABuffers
//     Deallocate DMA buffers
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void
CRomiDisk::FreeDMABuffers(
    )
{
    if (m_pPRD) {
        FreePhysMem(m_pPRD);
        m_pPRDPhys = NULL;
        m_pPRD = NULL;
    }

    if (m_pPhysList) {
        // free the fixed pages; the variable pages should already be free
        for (DWORD i = 0; i < MIN_PHYS_PAGES; i++) {
            FreePhysMem(m_pPhysList[i].pVirtualAddress);
        }
        VirtualFree(m_pPhysList, UserKInfo[KINX_PAGESIZE], MEM_DECOMMIT);
        m_pPhysList = NULL;
    }

    if (m_pSGCopy) {
        VirtualFree(m_pSGCopy, UserKInfo[KINX_PAGESIZE], MEM_DECOMMIT);
        m_pSGCopy = NULL;
    }

    if (m_pPFNs) {
        VirtualFree(m_pPFNs, UserKInfo[KINX_PAGESIZE], MEM_DECOMMIT);
        m_pSGCopy = NULL;
    }

    VirtualFree(m_pStartMemory, 0, MEM_RELEASE);
    m_pStartMemory = NULL;

    m_dwPhysCount = 0;
    m_dwSGCount = 0;
}

// ----------------------------------------------------------------------------
// Function: CopyDiskInfoFromPort
//     This function is not used
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void
CRomiDisk::CopyDiskInfoFromPort(
    )
{
    ASSERT(m_pPort->m_dwRegBase != 0);
    m_pATAReg = (PBYTE)m_pPort->m_dwRegBase;
    m_pATARegAlt = (PBYTE)m_pPort->m_dwRegAlt;

    ASSERT(m_pPort->m_dwBMR != 0);
    m_pBMCommand = (LPBYTE)m_pPort->m_dwBMR;
}

// ----------------------------------------------------------------------------
// Function: WaitForInterrupt
//     Wait for interrupt
//
// Parameters:
//     dwTimeOut -
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::WaitForInterrupt(
    DWORD dwTimeOut
    )
{
    BYTE bStatus;
    BOOL fRet = TRUE;
    DWORD dwRet;

    // wait for interrupt
    dwRet = WaitForSingleObject(m_pPort->m_hIRQEvent, dwTimeOut);
    if (dwRet == WAIT_TIMEOUT) {
        fRet = FALSE;
    }
    else {
        if (dwRet != WAIT_OBJECT_0) {
            if (!WaitForDisc(WAIT_TYPE_DRQ, dwTimeOut, 10)) {
                fRet = FALSE;
            }
        }
    }

    // read status; acknowledge interrupt
    bStatus = GetBaseStatus();
    if (bStatus & ATA_STATUS_ERROR) {
        bStatus = GetError();
        fRet = FALSE;
    }

    // signal interrupt done
    InterruptDone(m_pPort->m_dwSysIntr);

    return fRet;
}

// ----------------------------------------------------------------------------
// Function: EnableInterrupt
//     Enable channel interrupt
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

void
CRomiDisk::EnableInterrupt(
    )
{
    GetBaseStatus(); // acknowledge interrupt, if pending

    // signal interrupt done
    InterruptDone(m_pPort->m_dwSysIntr);
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
CRomiDisk::ConfigureRegisterBlock(
    DWORD dwStride
    )
{

    m_dwStride = dwStride;
    m_dwDataDrvCtrlOffset = ATA_PIO_DTR;//ATA_REG_DATA * dwStride;
    m_dwFeatureErrorOffset = ATA_PIO_FED;//ATA_REG_FEATURE * dwStride;
    m_dwSectCntReasonOffset = ATA_PIO_SCR;//ATA_REG_SECT_CNT * dwStride;
    m_dwSectNumOffset = ATA_PIO_LLR;//ATA_REG_SECT_NUM * dwStride;
    m_dwByteCountLowOffset = ATA_PIO_LMR;//ATA_REG_BYTECOUNTLOW * dwStride;
    m_dwByteCountHighOffset = ATA_PIO_LHR;//ATA_REG_BYTECOUNTHIGH * dwStride;
    m_dwDrvHeadOffset = ATA_PIO_DVR;//ATA_REG_DRV_HEAD * dwStride;
    m_dwCommandStatusOffset =ATA_PIO_CSD;// ATA_REG_COMMAND * dwStride;

    // PCI ATA implementations don't assign I/O resources for the first four
    // bytes, as they are unused

    m_dwAltStatusOffset = ATA_PIO_DAD;//8 * dwStride;//ATA_REG_ALT_STATUS * dwStride;
    m_dwAltDrvCtrl = ATA_PIO_DAD;//8 * dwStride;//ATA_REG_DRV_CTRL * dwStride;
}

// ----------------------------------------------------------------------------
// Function: Init
//     Initialize channel
//
// Parameters:
//     hActiveKey -
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::Init(
    HKEY hActiveKey
    )
{
    BOOL bRet = FALSE;

    m_f16Bit = TRUE; // PCI is 16-bit


    // configure port
    if (!ConfigPort()) {
        DEBUGMSG(ZONE_INIT, (_T(
            "Atapi!CRomiDisk::Init> Failed to configure port; device(%u)\r\n"
            ), m_dwDeviceId));
        goto exit;
    }

    // assign the appropriate folder name
    //m_szDiskName = (IsCDRomDevice() ? g_szPCICDRomDisk : g_szPCIHardDisk);

    // reserve memory for DMA buffers
    m_pStartMemory = (LPBYTE)VirtualAlloc(NULL, 0x10000, MEM_RESERVE, PAGE_READWRITE);
    if (!m_pStartMemory) {
        bRet = FALSE;
    }

	*(UINT32 *)(m_pATAReg + ATA_CONTROL) |= 1;
	//*(UINT32 *)(m_pATAReg + ATA_CFG ) |= 0x40; // set big endian hsjang
    // finish intialization; i.e., initialize device
    bRet = CDisk::Init(hActiveKey);
	//*(UINT32 *)(m_pATAReg + ATA_CFG) &= ~(0x40); // set little endian hsjang
    if (!bRet) {
        goto exit;
    }

exit:;
    return bRet;
}

// ----------------------------------------------------------------------------
// Function: MainIoctl
//     This is redundant
//
// Parameters:
//     pIOReq -
// ----------------------------------------------------------------------------

DWORD
CRomiDisk::MainIoctl(
    PIOREQ pIOReq
    )
{
    DEBUGMSG(ZONE_IOCTL, (_T(
        "Atapi!CRomiDisk::MainIoctl> IOCTL(%d), device(%d) \r\n"
        ), pIOReq->dwCode, m_dwDeviceId));

    return CDisk::MainIoctl(pIOReq);
}

// ----------------------------------------------------------------------------
// Function: ConfigPort
//     Initialize IST/ISR
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::ConfigPort(
    )
{
//    int i;
	int RetValue=TRUE;
    DWORD dwCleanUp = 0; // track progress for undo on clean up

 
    m_pATAReg = (PBYTE)m_pPort->m_dwRegBase;
  
    m_pATARegAlt = (PBYTE)m_pPort->m_dwRegAlt;
    m_pBMCommand = (PBYTE)m_pPort->m_dwBMR;

	m_vpIOPORTRegs= (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (m_vpIOPORTRegs == NULL) 
	{
		RETAILMSG(1,(TEXT("For m_vpATAPIRegs : VirtualAlloc failed! error code %d\r\n"),GetLastError()));
		RetValue = FALSE;
	}
	else 
	{
		if (!VirtualCopy((PVOID)m_vpIOPORTRegs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT>>8),
			sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
		{
			RETAILMSG(1,(TEXT("For INTRregs: VirtualCopy failed IOPORT!\r\n")));
			RetValue = FALSE;
		}
	}    


	m_vpEBIRegs = (volatile S3C2450_MATRIX_REG *)VirtualAlloc(0, sizeof(S3C2450_MATRIX_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (m_vpEBIRegs == NULL) 
	{
		RETAILMSG(1,(TEXT("For m_vpATAPIRegs : VirtualAlloc failed error code %d\r\n"),GetLastError()));
		return FALSE;
	}
	else 
	{
		if (!VirtualCopy((PVOID)m_vpEBIRegs, (PVOID)(S3C2450_BASE_REG_PA_MATRIX>> 8),
			sizeof(S3C2450_MATRIX_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
		{
			RETAILMSG(1,(TEXT("For INTRregs: VirtualCopy failed EBI!\r\n")));
			return FALSE;
		}
	}

	m_vpEBIRegs->EBICON |=  (1<<10)|(1<<9);  // bank3_cfg->CF,bank2_cfg->CF 
	m_vpIOPORTRegs->GPGCON &= ~(0x3ff<<22);
	m_vpIOPORTRegs->GPGCON |= (3<<30)|(3<<28)|(1<<26)|(3<<24)|(3<<22); //nCARD_PWREN, RESET_CF,nRE3G_CF,nINPACK,nIREQ_CF
	
    // This is board dependent
    m_vpIOPORTRegs->GPACON |= (1<<27)|(1<<11)|(1<<14)|(1<<13);  // nWE_CF,nOE_CF,nRCS3,nRCS2 enable //S3C2450X01
//  m_vpIOPORTRegs->GPACON &= ~(0x1<<10);   // GPA10 RDATA_OEN setting

	m_vpIOPORTRegs->MISCCR &=(~(1<<30)); // card detect when card is detected ,the bit should be '0'.
	m_vpIOPORTRegs->GPADAT &=~(0x1<<13);
	Sleep(2);
	
	*((UINT32 *)(m_pATAReg + MUX_REG)) = 0x07;
	Sleep(2);
	*((UINT32 *)(m_pATAReg + MUX_REG)) = 0x03;
	Sleep(2);
	*((UINT32 *)(m_pATAReg + MUX_REG)) = 0x01;
	Sleep(500);

	*((UINT32 *)(m_pATAReg + ATA_PIO_TIME)) = 0x1C238;
	*((UINT32 *)(m_pATAReg + ATA_UDMA_TIME)) = 0x20B1362 ; 

	*((UINT32 *)(m_pATAReg + ATA_IRQ)) |= 0x1f; 	
	*((UINT32 *)(m_pATAReg + ATA_IRQ_MASK)) |= 0x1f;	
	

	*((UINT32 *)(m_pATAReg + ATA_CONTROL)) |= 0x1;
	Sleep(200);

    // this function is called for the master and slave on this channel; if
    // this has already been called, then exit

	if (m_pPort->m_pDskReg[m_dwDeviceId]->dwInterruptDriven || m_pPort->m_pDskReg[m_dwDeviceId]->dwEnableUDMA)
	{
	    if (m_pPort->m_hIRQEvent) {
	        m_dwDeviceFlags |= DFLAGS_DEVICE_INITIALIZED;
	        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T("atapiRomi already initialized\n")));
	        return TRUE;
	    }
	    // create interrupt event
	    if (NULL == (m_pPort->m_hIRQEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
	        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
	            "Atapi!CRomiDisk::ConfigPort> Failed to create interrupt event for device(%d)\r\n"
	            ), m_dwDeviceId));
	        return FALSE;
	    }

	    // associate interrupt event with IRQ
	    
	    if (!InterruptInitialize(
	        m_pPort->m_dwSysIntr,
	        m_pPort->m_hIRQEvent,
	        NULL,
	        0)
	    ) {
	        DEBUGMSG(ZONE_INIT|ZONE_ERROR, (_T(
	            "Atapi!CRomiDisk::ConfigPort> Failed to initialize interrupt(SysIntr(%d)) for device(%d)\r\n"
	            ), m_pPort->m_dwSysIntr, m_dwDeviceId));
	        return FALSE;
	    }
	}

	return RetValue;
	
    //WriteDriveHeadReg(0x40);
}

// ----------------------------------------------------------------------------
// Function: TranslateAddress
//     Translate a system address to a bus address for the DMA controller
//
// Parameters:
//     pdwAddr -
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::TranslateAddress(
    PDWORD pdwAddr
    )
{
    // translate a system address to a bus address for the DMA bus controller

    PHYSICAL_ADDRESS SystemLogicalAddress, TransLogicalAddress;
    DWORD dwBus;

    // fetch bus number/type
    // if (m_pPort->m_pCNTRL != NULL) {
    //     dwBus = m_pPort->m_pCNTRL->m_dwBus;
    // }
    // else {
    //     dwBus = 0;
    // }

    dwBus = m_pPort->m_pController->m_dwi.dwBusNumber;

    // translate address
    SystemLogicalAddress.HighPart = 0;
    SystemLogicalAddress.LowPart = *pdwAddr;
    if (!HalTranslateSystemAddress(PCIBus, dwBus, SystemLogicalAddress, &TransLogicalAddress)) {
        return FALSE;
    }

    *pdwAddr = TransLogicalAddress.LowPart;

    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: SetupDMA
//     Prepare DMA transfer
//
// Parameters:
//     pSgBuf -
//     dwSgCount -
//     fRead -
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::SetupDMA(
    PSG_BUF pSgBuf,
    DWORD dwSgCount,
    BOOL fRead
    )
{
    DWORD dwAlignMask = m_dwDMAAlign - 1;
    DWORD dwPageMask = UserKInfo[KINX_PAGESIZE] - 1;

    DWORD iPage = 0, iPFN, iBuffer;
    BOOL fUnalign = FALSE;

    DMA_ADAPTER_OBJECT Adapter;

    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    Adapter.InterfaceType = (INTERFACE_TYPE)m_pPort->m_pController->m_dwi.dwInterfaceType;
    Adapter.BusNumber = m_pPort->m_pController->m_dwi.dwBusNumber;

    DEBUGMSG(ZONE_DMA, (_T(
        "Atapi!CRomiDisk::SetupDMA> Request(%s), SgCount(%d)\r\n"
        ), fRead ? (_T("Read")) : (_T("Write")), dwSgCount));

    // disable bus master
    WriteBMCommand(0);

    if (!m_pPRD) {
        m_pPRD = (PDMATable)HalAllocateCommonBuffer(&Adapter,
            UserKInfo[KINX_PAGESIZE], (PPHYSICAL_ADDRESS)&m_pPRDPhys, FALSE);
        if (!m_pPRD) {
            goto ExitFailure;
        }
    }

    // m_pPhysList tracks pages used for DMA buffers when the scatter/gather
    // buffer is unaligned
    if (!m_pPhysList) {
        m_pPhysList = (PPhysTable)VirtualAlloc(m_pStartMemory, UserKInfo[KINX_PAGESIZE], MEM_COMMIT, PAGE_READWRITE);
        if (!m_pPhysList) {
            goto ExitFailure;
        }
        // allocate the minimum number of fixed pages
        for (DWORD i = 0; i < MIN_PHYS_PAGES; i++) {
            PHYSICAL_ADDRESS PhysicalAddress = {0};
            m_pPhysList[i].pVirtualAddress = (LPBYTE)HalAllocateCommonBuffer(&Adapter,
                UserKInfo[KINX_PAGESIZE], &PhysicalAddress, FALSE);
            m_pPhysList[i].pPhysicalAddress = (LPBYTE)PhysicalAddress.QuadPart;
            if (!m_pPhysList[i].pVirtualAddress) {
                goto ExitFailure;
            }
        }
    }
    m_dwPhysCount = 0;

    // m_pSGCopy tracks the mapping between scatter/gather buffers and DMA
    // buffers when the scatter/gather buffer is unaligned and we are reading,
    // so we can copy the read data back to the scatter/gather buffer; when the
    // scatter/gather buffer is aligned, m_pSGCopy tracks the scatter/gather
    // buffers of a particular DMA transfer, so we can unlock the buffers at
    // completion

    if (!m_pSGCopy) {
        m_pSGCopy = (PSGCopyTable)VirtualAlloc(
            m_pStartMemory + UserKInfo[KINX_PAGESIZE],
            UserKInfo[KINX_PAGESIZE],
            MEM_COMMIT,
            PAGE_READWRITE);
        if (!m_pSGCopy) {
            goto ExitFailure;
        }
    }
    m_dwSGCount = 0;

    if (!m_pPFNs) {
        m_pPFNs = (PDWORD)VirtualAlloc(
            m_pStartMemory + 2*UserKInfo[KINX_PAGESIZE],
            UserKInfo[KINX_PAGESIZE],
            MEM_COMMIT,
            PAGE_READWRITE);
        if (!m_pPFNs) {
            goto ExitFailure;
        }
    }

    // determine whether the a buffer or the buffer length is unaligned
    for (iBuffer = 0; iBuffer < dwSgCount; iBuffer++) {
        if (
            ((DWORD)pSgBuf[iBuffer].sb_buf & dwAlignMask) ||
            ((DWORD)pSgBuf[iBuffer].sb_len & dwAlignMask)
        ) {
            fUnalign = TRUE;
            break;
        }
    }

    if (fUnalign) {

        DWORD dwCurPageOffset = 0;

        for (iBuffer = 0; iBuffer < dwSgCount; iBuffer++) {

            // Map address and check for security violation
            LPBYTE pBuffer = (LPBYTE)MapCallerPtr((LPVOID)pSgBuf[iBuffer].sb_buf, pSgBuf[iBuffer].sb_len);
            if (pSgBuf[iBuffer].sb_buf != NULL && pBuffer == NULL) {
                // security violation
                DEBUGMSG(ZONE_ERROR, (TEXT(
                    "Atapi!CRomiDisk::SetupDMA> Failed to map pointer to caller\r\n"
                    )));
                goto ExitFailure;
            }

            DWORD dwBufferLeft = pSgBuf[iBuffer].sb_len;
            while (dwBufferLeft) {

                DWORD dwBytesInCurPage = UserKInfo[KINX_PAGESIZE] - dwCurPageOffset;
                DWORD dwBytesToTransfer = (dwBufferLeft > dwBytesInCurPage) ? dwBytesInCurPage : dwBufferLeft;

                // allocate a new page, if necessary
                if ((dwCurPageOffset == 0) && (m_dwPhysCount >= MIN_PHYS_PAGES)) {
                    PHYSICAL_ADDRESS PhysicalAddress = {0};
                    m_pPhysList[m_dwPhysCount].pVirtualAddress = (LPBYTE)HalAllocateCommonBuffer(
                        &Adapter, UserKInfo[KINX_PAGESIZE], &PhysicalAddress, FALSE);
                    m_pPhysList[m_dwPhysCount].pPhysicalAddress = (LPBYTE)PhysicalAddress.QuadPart;
                    if (!m_pPhysList[m_dwPhysCount].pVirtualAddress) {
                        goto ExitFailure;
                    }
                }

                if (fRead) {

                    // prepare a scatter/gather copy entry on read, so we can
                    // copy data from the DMA buffer to the scatter/gather
                    // buffer after this DMA transfer is complete

                    m_pSGCopy[m_dwSGCount].pSrcAddress = m_pPhysList[m_dwPhysCount].pVirtualAddress + dwCurPageOffset;
                    m_pSGCopy[m_dwSGCount].pDstAddress = pBuffer;
                    m_pSGCopy[m_dwSGCount].dwSize = dwBytesToTransfer;
                    m_dwSGCount++;

                }
                else {
                    memcpy(m_pPhysList[m_dwPhysCount].pVirtualAddress + dwCurPageOffset, pBuffer, dwBytesToTransfer);
                }

                // if this buffer is larger than the space remaining on the page,
                // then finish processing this page by setting @dwCurPageOffset<-0

                if (dwBufferLeft >= dwBytesInCurPage) {
                    dwCurPageOffset = 0;
                }
                else {
                    dwCurPageOffset += dwBytesToTransfer;
                }

                // have we finished a page? (i.e., offset was reset or this is the last buffer)
                if ((dwCurPageOffset == 0) || (iBuffer == (dwSgCount - 1))) {
                    // add this to the PRD table
                    m_pPRD[m_dwPhysCount].physAddr = (DWORD)m_pPhysList[m_dwPhysCount].pPhysicalAddress;
                    m_pPRD[m_dwPhysCount].size = dwCurPageOffset ? (USHORT)dwCurPageOffset : (USHORT)UserKInfo[KINX_PAGESIZE];
                    m_pPRD[m_dwPhysCount].EOTpad = 0;
                    m_dwPhysCount++;
                }

                // update transfer
                dwBufferLeft -= dwBytesToTransfer;
                pBuffer += dwBytesToTransfer;
           }
        }

        m_pPRD[m_dwPhysCount - 1].EOTpad = 0x8000;

    }
    else {

        DWORD dwTotalBytes = 0;

        for (iBuffer = 0; iBuffer < dwSgCount; iBuffer++) {

            // Map address and check for security violation
            LPBYTE pBuffer = (LPBYTE)MapCallerPtr((LPVOID)pSgBuf[iBuffer].sb_buf, pSgBuf[iBuffer].sb_len);
            if (pSgBuf[iBuffer].sb_buf != NULL && pBuffer == NULL) {
                // security violation
                DEBUGMSG(ZONE_ERROR, (TEXT(
                    "Atapi!CRomiDisk::SetupDMA> Failed to map pointer to caller\r\n"
                    )));
                goto ExitFailure;
            }

            // determine the number of bytes remaining to be placed in PRD
            dwTotalBytes = pSgBuf[iBuffer].sb_len;
            if (!LockPages (
                pBuffer,
                dwTotalBytes,
                m_pPFNs,
                fRead ? LOCKFLAG_WRITE : LOCKFLAG_READ)
            ) {
                goto ExitFailure;
            }

            // add a scatter/gather copy entry for the area we lock, so that
            // we can unlock it when we are finished
            m_pSGCopy[m_dwSGCount].pSrcAddress = pBuffer;
            m_pSGCopy[m_dwSGCount].pDstAddress = 0;
            m_pSGCopy[m_dwSGCount].dwSize = dwTotalBytes;
            m_dwSGCount++;

            iPFN = 0;
            while (dwTotalBytes) {

                DWORD dwBytesToTransfer = UserKInfo[KINX_PAGESIZE];

                if ((DWORD)pBuffer & dwPageMask) {
                    // the buffer is not page aligned; use up the next page
                    // boundary
                    dwBytesToTransfer = UserKInfo[KINX_PAGESIZE] - ((DWORD)pBuffer & dwPageMask);
                }

                if (dwTotalBytes < dwBytesToTransfer) {
                    // use what remains
                    dwBytesToTransfer = dwTotalBytes;
                }

                m_pPRD[iPage].physAddr = (m_pPFNs[iPFN] << UserKInfo[KINX_PFN_SHIFT]) + ((DWORD)pBuffer & dwPageMask);

                if (!TranslateAddress(&m_pPRD[iPage].physAddr)) {
                    goto ExitFailure;
                }

                m_pPRD[iPage].size = (USHORT)dwBytesToTransfer;
                m_pPRD[iPage].EOTpad = 0;

                iPage++;
                iPFN++;

                // update transfer
                pBuffer += dwBytesToTransfer;
                dwTotalBytes -= dwBytesToTransfer;
            }
        }

        m_dwPhysCount = 0;
        m_pPRD[iPage-1].EOTpad = 0x8000;
    }

    return TRUE;

ExitFailure:

    DEBUGCHK(0);

    // clean up
    // FreeDMABuffers();

    return FALSE;
}

// ----------------------------------------------------------------------------
// Function: BeginDMA
//     Begin DMA transfer
//
// Parameters:
//     fRead -
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::BeginDMA(
    BOOL fRead
    )
{
    BYTE bStatus, bCommand;

    CacheSync(CACHE_SYNC_DISCARD);

    WriteBMCommand(0);
    WriteBMTable(m_pPRDPhys);

    bStatus = ReadBMStatus();
    bStatus |= 0x06;
    // bStatus |= 0x66;

    WriteBMStatus(bStatus);

    if (fRead) {
        bCommand = 0x08 | 0x01;
    }
    else {
        bCommand = 0x00 | 0x01;
    }

    WriteBMCommand(bCommand);

    bStatus = ReadBMStatus();

    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: EndDMA
//     End DMA transfer
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::EndDMA(
    )
{
    BYTE bStatus = ReadBMStatus();

    if ((bStatus & BM_STATUS_INTR) && (bStatus & BM_STATUS_ACTIVE)) {
        DEBUGMSG(ZONE_DMA, (_T(
            "Atapi!CRomiDisk::EndDMA> Status: active; status(0x%x)\r\n"
            ), bStatus));
    }
    else if ((bStatus & BM_STATUS_INTR) && !(bStatus & BM_STATUS_ACTIVE)) {
        DEBUGMSG(ZONE_DMA, (_T(
            "Atapi!CRomiDisk::EndDMA> Status: inactive; status(0x%x)\r\n"
            ), bStatus));
    }
    else if (!(bStatus & BM_STATUS_INTR)&& (bStatus & BM_STATUS_ACTIVE)) {

        DEBUGMSG(ZONE_ERROR|ZONE_DMA, (_T(
            "Atapi!CRomiDisk::EndDMA> Interrupt delayed; status(0x%x)\r\n"
            ), bStatus));

        BOOL bCount = 0;

        while (TRUE) {

            StallExecution(100);

            bCount++;
            bStatus = ReadBMStatus();

            if ((bStatus & BM_STATUS_INTR) && !(bStatus & BM_STATUS_ACTIVE)) {
                DEBUGMSG(ZONE_DMA, (_T(
                    "Atapi!CRomiDisk::EndDMA> DMA complete after delay; status(0x%x)\r\n"
                    ), bStatus));
                break;
            }
            else {
                DEBUGMSG(ZONE_ERROR|ZONE_DMA, (_T(
                    "Atapi!CRomiDisk::EndDMA> Interrupt still delayed; status(0x%x)\r\n"
                    ), bStatus));
                if (bCount > 10) {
                    WriteBMCommand(0);
                    return FALSE;
                }
            }
        }
    }
    else {
        if (bStatus & BM_STATUS_ERROR) {
            DEBUGMSG(ZONE_ERROR|ZONE_DMA, (_T(
                "Atapi!CRomiDisk::EndDMA> Error; (0x%x)\r\n"
                ), bStatus));
            DEBUGCHK(0);
            return FALSE;
        }
    }

    WriteBMCommand(0);

    return TRUE;
}

// ----------------------------------------------------------------------------
// Function: AbortDMA
//     Abort DMA transfer
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::AbortDMA(
    )
{
    DWORD i;

    WriteBMCommand(0);

    for (i = 0; i < m_dwSGCount; i++) {
        if (!m_pSGCopy[i].pDstAddress) {
            UnlockPages(m_pSGCopy[i].pSrcAddress, m_pSGCopy[i].dwSize);
        }
    }

    // free all but the first @MIN_PHYS_PAGES pages; these are fixed
    for (i = MIN_PHYS_PAGES; i < m_dwPhysCount; i++) {
        FreePhysMem(m_pPhysList[i].pVirtualAddress);
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
// Function: CompleteDMA
//     Complete DMA transfer
//
// Parameters:
//     pSgBuf -
//     dwSgCount -
//     fRead -
// ----------------------------------------------------------------------------

BOOL
CRomiDisk::CompleteDMA(
    PSG_BUF pSgBuf,
    DWORD dwSgCount,
    BOOL fRead
    )
{
    DWORD i;

    for (i = 0; i < m_dwSGCount; i++) {
        if (m_pSGCopy[i].pDstAddress) {
            // this corresponds to an unaligned region; copy it back to the
            // scatter/gather buffer
            memcpy(m_pSGCopy[i].pDstAddress, m_pSGCopy[i].pSrcAddress, m_pSGCopy[i].dwSize);
        }
        else {
            // this memory region needs to be unlocked
            UnlockPages(m_pSGCopy[i].pSrcAddress, m_pSGCopy[i].dwSize);
        }
    }

    // free all but the first @MIN_PHYS_PAGES pages; the first @MIN_PHYS_PAGES
    // pages are fixed

    for (i = MIN_PHYS_PAGES; i < m_dwPhysCount; i++) {
        FreePhysMem(m_pPhysList[i].pVirtualAddress);
    }

    return TRUE;
}

BOOL
CRomiDisk::WakeUp(
    )
{
//    BOOL retVal;
    //jungpil

    //memory controller settings are not recovered from Wakeup
	m_vpEBIRegs->EBICON |=  (1<<10)|(1<<9); 
	m_vpIOPORTRegs->GPGCON |= (3<<30)|(3<<28)|(3<<26)|(3<<24)|(3<<22);

    m_vpIOPORTRegs->GPACON |= (1<<27)|(1<<11)|(1<<14)|(1<<13);  // nWE_CF,nOE_CF,nRCS3,nRCS2 enable //S3C2450X01
//  m_vpIOPORTRegs->GPACON &= ~(0x1<<10);   // GPA10 RDATA_OEN setting

	m_vpIOPORTRegs->MISCCR &=(~(1<<30)); // card detect when card is detected ,the bit should be '0'.
	m_vpIOPORTRegs->GPADAT &=~(0x1<<13);
	Sleep(2);
	
	*((UINT32 *)(m_pATAReg + MUX_REG)) = 0x07;
	Sleep(2);
	*((UINT32 *)(m_pATAReg + MUX_REG)) = 0x03;
	Sleep(2);
	*((UINT32 *)(m_pATAReg + MUX_REG)) = 0x01;
	Sleep(500);

	*((UINT32 *)(m_pATAReg + ATA_PIO_TIME)) = 0x1c238;
	*((UINT32 *)(m_pATAReg + ATA_UDMA_TIME)) = 0x20B1362 ; 

	*((UINT32 *)(m_pATAReg + ATA_IRQ)) |= 0x1f; 	
	*((UINT32 *)(m_pATAReg + ATA_IRQ_MASK)) |= 0x1f;	
	
	*((UINT32 *)(m_pATAReg + ATA_CONTROL)) |= 0x1; 	

    //power on
    return CDisk::Init(NULL); // It is the simplest way and best way.
}

CDiskPower *
CRomiDisk::GetDiskPowerInterface(
    void
    )
{
    CDiskPower *pDiskPower = new CRomiDiskPower;
    return pDiskPower;
}

void 
CRomiDisk::SetPioMode(UCHAR  pmode) 
{
	UINT8 nMode;
	UINT32 uT1;
	UINT32 uT2;
	UINT32 uTeoc;
	UINT32 i;
	
	UINT32 uPioTime[5];
	UINT32 m_uPioT1[5] = {100,60,40,40,40};     // min = {70,50,30,30,25};
	UINT32 m_uPioT2[5] = {400,300,290,130,70}; // min = {290,290,290,80,70};
	UINT32 m_uPioTeoc[5] = {100,25,10,10,10};  // min = {20,15,10,10,10};
	
	UINT32 uCycleTime = (UINT32)(1000000000/S3C2450_HCLK);
	UINT32 uTemp = *((UINT32 *)(m_pATAReg + ATA_CFG));

	if ((pmode & (UCHAR)0x3) == 1) 
		nMode = PIO3;
	else if ((pmode & (UCHAR)0x3) == 3) 
		nMode = PIO4;
	else 
		nMode = PIO2 ;


	for (i=0; i<5; i++)
	{
		uT1   = (m_uPioT1[i]  /uCycleTime + 1)&0xff;
		uT2   = (m_uPioT2[i]  /uCycleTime + 1)&0xff;
		uTeoc = (m_uPioTeoc[i]/uCycleTime + 1)&0x0f;
		uPioTime[i] = (uTeoc<<12)|(uT2<<4)|uT1;
	}
	
//	g_vATAPIRegs->ATA_IRQ=0xff;
//	g_vATAPIRegs->ATA_IRQ_MASK = ATAPI_MASK;

	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_DAD, 0x0);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_FED, 0x3);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_SCR, (0x8 |(nMode&0x7)));
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_LLR, 0x0);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_LMR, 0x0);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_LHR, 0x0);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_DVR, 0x40);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_CSD, ATAPI_CMD_SET_FEATURES);	
	WaitForNoBusyStatus();
	switch(pmode) { // modified by Bryan W. Lee (Oct. 19th, 2005)
		case PIO1:
			uTemp &= (~0x2); //IORDY disable
			*(UINT32 *)(m_pATAReg + ATA_PIO_TIME) = uPioTime[1];
			break;
		case PIO2:
			uTemp &= (~0x2); //IORDY disable
			*(UINT32 *)(m_pATAReg + ATA_PIO_TIME) = uPioTime[2];

			break;
		case PIO3:
			uTemp |= 0x2; //IORDY enable
			*(UINT32 *)(m_pATAReg + ATA_PIO_TIME) = uPioTime[3];			
			break;
		case PIO4:
			uTemp |= 0x2; //IORDY enable
			*(UINT32 *)(m_pATAReg + ATA_PIO_TIME) = uPioTime[4];
			break;
		default:
			uTemp &= (~0x2); //IORDY disable
			*(UINT32 *)(m_pATAReg + ATA_PIO_TIME) = uPioTime[0];
			break;
		}
		
	*(UINT32 *)(m_pATAReg + ATA_CFG) = uTemp;
}


void 
CRomiDisk::SetUdmaMode() 
{

	UINT32 uTdvh1;
	UINT32 uTdvs;
	UINT32 uTrp;
	UINT32 uTss;
	UINT32 uTackenv;
	UINT32 i;

	UINT32 uUdmaTime[5];
	UINT32 uUdmaTdvh[5] = {20,20,20,20,10}; //{7,7,7,7,7};
	UINT32 uUdmaTdvs[5] = {100,60,40,25,20}; //{70,48,31,20,7};
	UINT32 uUdmaTrp[5] = {160,125,100,100,100};
	UINT32 uUdmaTss[5] = {50,50,50,50,50};
	UINT32 uUdmaTackenvMin[5] = {20,20,20,20,20};
//	UINT32 uUdmaTackenvMin[5] = {70,70,70,55,55};	
	UINT32 uCycleTime = (UINT32)(1000000000/S3C2450_HCLK);

#if 1
	m_vpIOPORTRegs->GPADAT &= ~(1<<5); // GPA10 RDATA_fOEN setting
//	rGPACDH = 0xaa8a; // GPA10 RDATA_OEN setting
//	rGPBCON = rGPBCON & ~(3<<12) | (1<<12); // GPB6 output setting (nXBREQ)
//	rGPBDAT &= ~(1<<6); // GPB6 -> L, buffer Output enable 
#else
	g_vIOPORTRegs->GPBCON = (g_vIOPORTRegs->GPBCON & ~(0xf<<10)) | (5<<10); // GPB5,6 output mode
	g_vIOPORTRegs->GPBCON = (g_vIOPORTRegs->GPBCON &  ~(0x3<<8)) |( 1<<8); // GPB4 output mode		
	g_vIOPORTRegs->GPBCON = (g_vIOPORTRegs->GPBCON &  ~(0x3<<0)) |( 1<<0); // GPB4 output mode			
	g_vIOPORTRegs->GPBDAT |= (3<<5); // GPB5,6 -> H	
#endif	
	
//	ChangeBufferControl(PIO_CPU);    
	
	for (i=0; i<5; i++)
	{
		uTdvh1	= (uUdmaTdvh[i] / uCycleTime + 1)&0x0f;
		uTdvs	= (uUdmaTdvs[i] / uCycleTime + 1)&0xff;
		uTrp	= (uUdmaTrp[i]  / uCycleTime + 1)&0xff;
		uTss	= (uUdmaTss[i]  / uCycleTime + 1)&0x0f;
		uTackenv= (uUdmaTackenvMin[i]/uCycleTime + 1)&0x0f;
		uUdmaTime[i] = (uTdvh1<<24)|(uTdvs<<16)|(uTrp<<8)|(uTss<<4)|uTackenv;
	}	

	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_DAD, 0x0);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_FED, 0x3);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_SCR, (0x40 |((BYTE)m_dwCurrentUDMAMode&0x7)));
//	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_SCR, (0x40 |(3&0x7)));
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_LLR, 0x0);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_LMR, 0x0);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_LHR, 0x0);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_DVR, 0x40);
	ATA_WRITE_BYTE(m_pATAReg + ATA_PIO_CSD, ATAPI_CMD_SET_FEATURES);

	WaitForNoBusyStatus();

	
	switch(m_dwCurrentUDMAMode) 
//	switch(3) 
	{
		case UDMA0:
			*(UINT32 *)(m_pATAReg + ATA_UDMA_TIME) = uUdmaTime[0];
			break;		
		case UDMA1:
			*(UINT32 *)(m_pATAReg + ATA_UDMA_TIME) = uUdmaTime[1];
			break;
		case UDMA2:
			*(UINT32 *)(m_pATAReg + ATA_UDMA_TIME) = uUdmaTime[2];
			break;
		case UDMA3:
			*(UINT32 *)(m_pATAReg + ATA_UDMA_TIME) = uUdmaTime[3];
			break;
		case UDMA4:
			*(UINT32 *)(m_pATAReg + ATA_UDMA_TIME) = uUdmaTime[4];
			break;
		default:
			RETAILMSG(1,(TEXT("UDMA mode is supported between 0 to 4 !!!! . %d\r\n"),m_dwCurrentUDMAMode));
			break;
	}
}



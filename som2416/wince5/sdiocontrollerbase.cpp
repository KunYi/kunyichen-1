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

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <s3c2450.h>
#include <s3c2450_dmatransfer.h>
#include <creg.hxx>
#include <ddkreg.h>
#include "SDCardDDK.h"
#include <SDHCD.h>
#include <bsp.H>
#include "SDIOControllerBase.h"


#define SDIO_DMA_PHYSICAL_MEMORY_SIZE	  0x00010000

#define MMCFIFO_DWORD_PHYS 0x5A000040
#define MMCFIFO_WORD_PHYS  0x5A000044
#define MMCFIFO_BYTE_PHYS  0x5A000048
/////////////////////////////////////////////////////////////////////////

#define WAIT_TIME                       0x200000        // while loop delay

#define START_BIT                           0x00
#define TRANSMISSION_BIT                    0x00
#define START_RESERVED                      0x3F
#define END_RESERVED                        0xFE
#define END_BIT                             0x01

#define MAXIMUM_DMA_TRANSFER_SIZE               SDIO_DMA_PHYSICAL_MEMORY_SIZE

#define MAXIMUM_BLOCK_TRANSFER_SIZE             (MAXIMUM_DMA_TRANSFER_SIZE / 0x200)
#define MINIMUM_BLOCK_TRANSFER_SIZE             1

#define IS_BUFFER_DWORD_ALIGNED(x)              ((((DWORD)x) % 4) == 0)
#define IS_BUFFER_WORD_ALIGNED(x)               ((((DWORD)x) % 2) == 0)

#define IS_BUFFER_SIZE_A_DWORD_MULTPLE(x)       ((((DWORD)x) % 4 )== 0)
#define IS_BUFFER_SIZE_A_WORD_MULTPLE(x)        ((((DWORD)x) % 2 )== 0)

#define DMA_CHANNEL_TEXT TEXT("DMAChannel")
#define DMA_IRQ_TEXT TEXT("DMAIrq")
#define DMA_IST_PRIORITY_TEXT TEXT("DMA_IST_Priority")

#define SDIO_IRQ_TEXT TEXT("SDIOIrq")
#define SD_DETECT_IRQ_TEXT TEXT("SDDetectIrq")
#define SDIO_IST_PRIORITY_TEXT TEXT("SDIO_IST_Priority")

#define POLLING_TIMEOUT_TEXT TEXT("PollingTimeout")
#define CARD_DETECT_THREAD_PRIORITY_TEXT TEXT("CardDetect_Thread_Priority")
#define HANDLE_BUSY_FINISH_ON_COMMAND38_TEXT TEXT("HandleBusyFinishOnCommand38")
#define DMA_TRANSFER_TIMEOUT_FACTOR_TEXT TEXT("DmaTransferTimeoutFactor")
#define DMA_TRANSFER_TIMEOUT_CONSTANT_TEXT TEXT("DmaTransferTimeoutConstant")

#define Lock()		EnterCriticalSection(&m_ControllerCriticalSection)
#define UnLock()		LeaveCriticalSection(&m_ControllerCriticalSection)

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::CSDIOControllerBase - CSDIOControllerBase constructor
//  Input:  pHCContext - hardware context
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
CSDIOControllerBase::CSDIOControllerBase( PSDCARD_HC_CONTEXT pHCContext )
{
    vm_pSDIReg = NULL;
    vm_pIOPreg = NULL;
    vm_pCLKPWR = NULL;
    vm_pDMAreg = NULL;
    m_pDMABuffer = NULL;
    m_dwDMAChannel = 0;

    m_pHCContext = pHCContext;

    m_hResponseReceivedEvent = NULL;
    m_nCardDetectIstThreadPriority = 0;
    m_hCardInsertInterruptEvent = NULL;
    m_hCardInsertInterruptThread = NULL;

    m_nSDIOIstThreadPriority = 0;
    m_hSDIOInterruptEvent = NULL;
    m_hSDIOInterruptThread = NULL;

    m_nControllerIstThreadPriority = 0;
    m_hControllerInterruptEvent = NULL;
    m_hControllerInterruptThread = NULL;

    m_DMAIstThreadPriority = 0;
    m_hDMAInterruptEvent = NULL;
    m_hDMAInterruptThread = NULL;

    m_bUseDMAForTransfer = FALSE;
  	m_bUseDMABurst = FALSE;
	m_DATATransferSize = 1;
    m_bDriverShutdown = FALSE;

    m_bDevicePresent = FALSE;
    m_CurrentState = Idle;
    m_bSendInitClocks = TRUE;
    m_dwLastTypeOfTransfer = SD_READ;

    m_dwNumBytesToTransfer = 0;
    m_dwNumBytesUnaligned = 0;
    m_dwNumBytesExtra = 0;

    m_rgchRegPath[0] = 0;
    m_dwSDIBusWidth = 0;

    m_bReinsertTheCard = FALSE;

    m_fHandleBusyCheckOnCommand38 = FALSE;
    m_dwDMATransferTimeoutFactor = 8;
    m_dwDMATransferTimeoutConstant = 3000;

    m_fCardInTheSlot = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::~CSDIOControllerBase - CSDIOControllerBase destructor
//  Input:  
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
CSDIOControllerBase::~CSDIOControllerBase()
{
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::PreDeinit() - PreDeinit routine
//  Input:  
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
void CSDIOControllerBase::PreDeinit()
{
    if( !m_bDriverShutdown )
    {
        Deinitialize();
    }
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::Initialize - Initializes the the SDIO Host Controller
//  Input:  pHCContext - newly allocated hardware context
//          
//  Output: 
//  Return: SD_API_STATUS code
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::Initialize()
{
    DEBUGMSG (SDCARD_ZONE_INIT,(TEXT("SDHCDInitialize starts\r\n")));
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // intermediate status
    DWORD         threadID;                         // thread ID

    InitializeCriticalSection(&m_ControllerCriticalSection);


    //----- 1. Map the GPIO registers needed to enable the SDI controller -----
    
    vm_pIOPreg = (S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
    if (vm_pIOPreg == NULL) 
    {
        DEBUGMSG (1,(TEXT("GPIO registers not allocated\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }
    if (!VirtualCopy((PVOID)vm_pIOPreg, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
        DEBUGMSG (1,(TEXT("GPIO registers not mapped\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }
    DEBUGMSG (1,(TEXT("GPIO registers mapped to %x\r\n"), vm_pIOPreg));
	
    //----- 2. Map the SDI control registers into the device drivers address space -----
    vm_pSDIReg = (S3C2450_SDI_REG *)VirtualAlloc(0, sizeof(S3C2450_SDI_REG), MEM_RESERVE, PAGE_NOACCESS);
    if (vm_pSDIReg == NULL) 
    {
        DEBUGMSG (1,(TEXT("SDI control registers not allocated\n\r")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }
    if (!VirtualCopy((PVOID)vm_pSDIReg, (PVOID)(S3C2450_BASE_REG_PA_SDI >> 8), sizeof(S3C2450_SDI_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
        DEBUGMSG (1,(TEXT("SDI control registers not mapped\n\r")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }
    DEBUGMSG (1,(TEXT("SDI control registers mapped to %x\n\r"), vm_pSDIReg));
	
    vm_pCLKPWR = (S3C2450_CLKPWR_REG *)VirtualAlloc(0, sizeof(S3C2450_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
    if (vm_pCLKPWR == NULL) 
    {
        DEBUGMSG (1,(TEXT("Clock & Power Management Special Register not allocated\n\r")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }
    if (!VirtualCopy((PVOID)vm_pCLKPWR, (PVOID)(S3C2450_BASE_REG_PA_CLOCK_POWER >> 8), sizeof(S3C2450_CLKPWR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
        DEBUGMSG (1,(TEXT("Clock & Power Management Special Register not mapped\n\r")));
        goto INIT_ERROR;
    }
    DEBUGMSG (1,(TEXT("Clock & Power Management Special Register mapped to %x\n\r"), vm_pCLKPWR));

    if( m_dwDMAChannel != 0xffffffff )
    {
        //----- 3. Map the DMA control registers used for SDI data transfers -----
        vm_pDMAreg = (S3C2450_DMA_REG *)VirtualAlloc(0, sizeof(S3C2450_DMA_REG), MEM_RESERVE, PAGE_NOACCESS);
        if (vm_pDMAreg == NULL) 
        {
            DEBUGMSG (1,(TEXT("DMA Register not allocated\n\r")));
            status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
            goto INIT_ERROR;
        }
        if (!VirtualCopy((PVOID)vm_pDMAreg, (PVOID)(S3C2450_BASE_REG_PA_DMA >> 8), sizeof(S3C2450_DMA_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
            DEBUGMSG (1,(TEXT("DMA Register not mapped\n\r")));
            status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
            goto INIT_ERROR;
        }
        DEBUGMSG (1,(TEXT("DMA Register mapped to %x\n\r"), vm_pDMAreg));

        //----- 4. Allocate a block of memory for DMA transfers -----
        DMA_ADAPTER_OBJECT dmaAdapter;
        dmaAdapter.ObjectSize = sizeof(dmaAdapter);
        dmaAdapter.InterfaceType = Internal;
        dmaAdapter.BusNumber = 0;
        m_pDMABuffer = (PBYTE)HalAllocateCommonBuffer( &dmaAdapter, MAXIMUM_DMA_TRANSFER_SIZE, &m_pDMABufferPhys, FALSE );
        if( m_pDMABuffer == NULL )
        {
            RETAILMSG(1, (TEXT("SHCDriver: - Unable to allocate memory for DMA buffers!\r\n")));
            status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
            goto INIT_ERROR;
        }
    }

    //  Supply the clock to the SDI controller
    MMC_Hardware_PowerUp();

    //----- 6. Configure the GPIO lines for SDI mode and enable the pullup resistor -----
    vm_pIOPreg->GPEUDP  &= 0xC003FF;
    vm_pIOPreg->GPECON = (vm_pIOPreg->GPECON & ~(0x3ffc00)) | 0x2AA800;

    //----- 7. Set the SD/SDI controller to some reasonable default values -----
    SetClockRate(SD_DEFAULT_CARD_ID_CLOCK_RATE);        // 100Khz

    vm_pSDIReg->SDICON     |= LITTLE_ENDIAN_BYTE_ORDER;    // Windows CE is always Little Endian.
    vm_pSDIReg->SDIFSTA     |= FIFO_RESET;                  // Reset the FIFO
    vm_pSDIReg->SDIBSIZE    = BYTES_PER_SECTOR; 
    vm_pSDIReg->SDIDTIMER   = MAX_DATABUSY_TIMEOUT;        // Data/busy timeout

    //----- 8. Setup the thread for detecting card insertion/deletion -----
    m_hCardInsertInterruptEvent = CreateEvent(NULL, FALSE, FALSE,NULL);

    if(NULL == m_hCardInsertInterruptEvent) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }


    m_bDevicePresent                    = FALSE;

    m_hCardInsertInterruptThread = CreateThread(NULL, 0, 
                                                (LPTHREAD_START_ROUTINE)SD_CardDetectThread,
                                                this, 0, &threadID);

    if(NULL == m_hCardInsertInterruptThread) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }

    if(!InterruptInitialize (m_dwSDDetectSysIntr, m_hCardInsertInterruptEvent,
                             NULL, 0)) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }

    //----- 9 . Setup an event used for signaling our response thread -----
    m_hResponseReceivedEvent = CreateEvent(NULL, FALSE, FALSE,NULL);
    
    if(NULL == m_hResponseReceivedEvent) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }


    //----- 10. Setup the IST for handling SDIO data transfer interrupts -----
    m_hSDIOInterruptEvent = CreateEvent(NULL, FALSE, FALSE,NULL);
    
    if(NULL == m_hSDIOInterruptEvent) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }

    // initialize the card insertion interrupt event
    if(!InterruptInitialize (m_dwSDIOSysIntr, m_hSDIOInterruptEvent,
                             NULL, 0)) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }

    m_hSDIOInterruptThread = CreateThread(NULL, 0,
                                          (LPTHREAD_START_ROUTINE)SD_IOInterruptIstThread,
                                          this, 0, &threadID);
    
    if(NULL == m_hSDIOInterruptThread) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }

    //----- 11. Setup the interrupt event for handling SDIO DMA data transfers -----
    m_hDMAInterruptEvent = CreateEvent(NULL, FALSE, FALSE,NULL);
    
    if(NULL == m_hDMAInterruptEvent) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }

    // initialize the dma transfer interrupt event
    if(!InterruptInitialize (m_dwDMASysIntr, m_hDMAInterruptEvent,
                             NULL, 0)) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }

    m_hDMAInterruptThread = CreateThread(NULL, 0,
                                                (LPTHREAD_START_ROUTINE)SD_TransferIstThread,
                                                this, 0, &threadID);
    
    if(NULL == m_hDMAInterruptThread) 
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto INIT_ERROR;
    }

    m_dwLastTypeOfTransfer = SD_READ;   
    m_bUseDMAForTransfer = FALSE;
   	m_bUseDMABurst = FALSE;
	m_DATATransferSize = 1;

	
    if( !InitializeHardware() )
    {
        DEBUGMSG (1,(TEXT("InitializeHardware failed!\n\r")));
        status = SD_API_STATUS_UNSUCCESSFUL;
        goto INIT_ERROR;
    }

    SetEvent( m_hCardInsertInterruptEvent );

INIT_ERROR:

    if(!SD_API_SUCCESS(status)) 
    {
        Deinitialize();
    }
    DEBUGMSG (SDCARD_ZONE_INIT,(TEXT("SDHCDInitialize ends\r\n")));
    return status;
}


///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::DeInitialize - Deinitialize the the SDIO Controller
//  Input:  pHCContext - Host controller context
//          
//  Output: 
//  Return: SD_API_STATUS code
//  Notes:  
//         
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::Deinitialize()
{
    DEBUGMSG (SDCARD_ZONE_INIT,(TEXT("SDHCDDeinitialize starts\r\n")));
    //----- 1. Set the controller state to "shutdown" -----
    m_bDriverShutdown = TRUE;

    //----- 2. Disable and cleanup the ISTs/events ------
    InterruptDisable (m_dwSDIOSysIntr);
    SetEvent(m_hResponseReceivedEvent);
    InterruptDisable (m_dwDMASysIntr);

    if(NULL != m_hResponseReceivedEvent) 
    {
        CloseHandle(m_hResponseReceivedEvent);
        m_hResponseReceivedEvent = NULL;
    }

    if(NULL != m_hSDIOInterruptThread) 
    {           
        WaitForSingleObject(m_hSDIOInterruptThread, INFINITE); 
        CloseHandle(m_hSDIOInterruptThread);
        m_hSDIOInterruptThread = NULL;
    }
        
    if(NULL != m_hSDIOInterruptEvent) 
    {
        CloseHandle(m_hSDIOInterruptEvent);
        m_hSDIOInterruptEvent = NULL;
    }

    if(NULL != m_hDMAInterruptThread) 
    {           
        WaitForSingleObject(m_hDMAInterruptThread, INFINITE); 
        CloseHandle(m_hDMAInterruptThread);
        m_hDMAInterruptThread = NULL;
    }
    
    if(NULL != m_hDMAInterruptEvent) 
    {
        CloseHandle(m_hDMAInterruptEvent);
        m_hDMAInterruptEvent = NULL;
    }

    if(NULL != m_hCardInsertInterruptThread) 
    {           
        WaitForSingleObject(m_hCardInsertInterruptThread, INFINITE); 
        CloseHandle(m_hCardInsertInterruptThread);
        m_hCardInsertInterruptThread = NULL;
    }
        
    if(NULL != m_hCardInsertInterruptEvent) 
    {
        CloseHandle(m_hCardInsertInterruptEvent);
        m_hCardInsertInterruptEvent = NULL;
    }

    if( m_dwDMAChannel != 0xffffffff )
    {
        //----- 3. Free the DMA memory -----
        HalFreeCommonBuffer( NULL, 0, m_pDMABufferPhys, m_pDMABuffer, FALSE );
    }

    //----- 4. Close the handle to the utility driver (used for fast driver-->driver calling -----
    MMC_Hardware_PowerDown();

    DeleteCriticalSection(&m_ControllerCriticalSection);

    DEBUGMSG (SDCARD_ZONE_INIT,(TEXT("SDHCDDeinitialize ends\r\n")));
    return SD_API_STATUS_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::CancelIoHandler - io cancel handler 
//  Input:  pHostContext - host controller context
//          Slot - slot the request is going on
//          pRequest - the request to be cancelled
//          
//  Output: 
//  Return: TRUE if the request was cancelled
//  Notes:  
//          
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN CSDIOControllerBase::CancelIoHandler(DWORD dwSlot, PSD_BUS_REQUEST pRequest)
{
    DEBUGMSG (SDCARD_ZONE_FUNC,(TEXT("SDHCDCancelIoHandler starts\r\n")));

    //--- Stop hardware, cancel the request!
    Stop_SDI_Hardware();

    // release the lock before we complete the request
    SDHCDReleaseHCLock(m_pHCContext);
 
    // complete the request with a cancelled status
    SDHCDIndicateBusRequestComplete(m_pHCContext, pRequest,SD_API_STATUS_CANCELED);

    DEBUGMSG (SDCARD_ZONE_FUNC,(TEXT("SDHCDCancelIoHandler ends\r\n")));
    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::SlotOptionHandler - handler for slot option changes
//  Input:  SlotNumber   - the slot the change is being applied to
//          Option       - the option code
//          pData        - data associated with the option
//          OptionSize   - size of option data
//  Output: 
//  Return: SD_API_STATUS code
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::SlotOptionHandler(DWORD dwSlot, SD_SLOT_OPTION_CODE Option, 
                                     PVOID pData, ULONG OptionSize)
{
    DEBUGMSG (SDCARD_ZONE_FUNC,(TEXT("SDHCDSlotOptionHandler starts\r\n")));

    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // status
    PSD_HOST_BLOCK_CAPABILITY pBlockCaps;           // block capabilities

    switch(Option) 
    {
        case SDHCDSetSlotPower:
            DEBUGMSG (SDCARD_ZONE_INFO,(TEXT("SDHCDSlotOptionHandler option=SDHCDSetSlotPower\r\n")));
            // Nothing to do because this system only operates at the reported 3.3V
            break;

        case SDHCDSetSlotInterface:
            DEBUGMSG (SDCARD_ZONE_INFO,(TEXT("SDHCDSlotOptionHandler option=SDHCDSetSlotInterface\r\n")));
            // First set the bus width 
            if(((PSD_CARD_INTERFACE)pData)->InterfaceMode == SD_INTERFACE_SD_4BIT)
            {
                Set_SDI_Bus_Width_4Bit();
            }else
            {
                // Standard (i.e. 1bit) bus width
                Set_SDI_Bus_Width_1Bit();
            }

            // Next, set the clock rate
            ((PSD_CARD_INTERFACE)pData)->ClockRate = SetClockRate(((PSD_CARD_INTERFACE)pData)->ClockRate);
            break;

        case SDHCDEnableSDIOInterrupts:                                 
            DEBUGMSG (SDCARD_ZONE_INFO,(TEXT("SDHCDSlotOptionHandler option=SDHCDEnableSDIOInterrupts\r\n")));
            Enable_SDIO_Interrupts();
            break;

        case SDHCDDisableSDIOInterrupts:                                
            DEBUGMSG (SDCARD_ZONE_INFO,(TEXT("SDHCDSlotOptionHandler option=SDHCDDisableSDIOInterrupts\r\n")));
            Disable_SDIO_Interrupts();
            break;

        case SDHCDAckSDIOInterrupt:                                     
            
            DEBUGMSG (SDCARD_ZONE_INFO,(TEXT("SDHCDSlotOptionHandler option=SDHCDAckSDIOInterrupt\r\n")));
            //----- 2. Clear the SDIO interrupt pending bit -----
            Ack_SDIO_Interrupts();
            InterruptDone(m_dwSDIOSysIntr);
            break;

        case SDHCDGetWriteProtectStatus:
            DEBUGMSG (SDCARD_ZONE_INFO,(TEXT("SDHCDSlotOptionHandler option=SDHCDGetWriteProtectStatus\r\n")));
            ((PSD_CARD_INTERFACE)pData)->WriteProtected = IsCardWriteProtected();
            break;

        case SDHCDQueryBlockCapability:

            DEBUGMSG (SDCARD_ZONE_INFO,(TEXT("SDHCDSlotOptionHandler option=SDHCDQueryBlockCapability\r\n")));
            pBlockCaps = (PSD_HOST_BLOCK_CAPABILITY)pData;

            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHCD:SDHCDSlotOptionHandler() - Read Block Length: %d , Read Blocks: %d\r\n"), 
                                                pBlockCaps->ReadBlockSize, pBlockCaps->ReadBlocks));

            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("SDHCD:SDHCDSlotOptionHandler() - Write Block Length: %d , Write Blocks: %d\r\n"), 
                                                pBlockCaps->WriteBlockSize, pBlockCaps->WriteBlocks));

            pBlockCaps = (PSD_HOST_BLOCK_CAPABILITY)pData;

            //----- Validate block transfer properties -----
            if (pBlockCaps->ReadBlockSize < MINIMUM_BLOCK_TRANSFER_SIZE )
            {
                pBlockCaps->ReadBlockSize = MINIMUM_BLOCK_TRANSFER_SIZE;
            }

            if (pBlockCaps->WriteBlockSize < MINIMUM_BLOCK_TRANSFER_SIZE )
            {
                pBlockCaps->WriteBlockSize = MINIMUM_BLOCK_TRANSFER_SIZE;
            }

            if (pBlockCaps->ReadBlockSize > MAXIMUM_BLOCK_TRANSFER_SIZE )
            {
                pBlockCaps->ReadBlockSize = MAXIMUM_BLOCK_TRANSFER_SIZE;
            }
 
            if (pBlockCaps->WriteBlockSize > MAXIMUM_BLOCK_TRANSFER_SIZE )
            {
                pBlockCaps->WriteBlockSize = MAXIMUM_BLOCK_TRANSFER_SIZE;
            }            
            break;

        case SDHCDGetSlotInfo:
            DEBUGMSG (SDCARD_ZONE_INFO,(TEXT("SDHCDSlotOptionHandler option=SDHCDGetSlotInfo\r\n")));
            if( OptionSize != sizeof(SDCARD_HC_SLOT_INFO) || pData == NULL )
            {
                status = SD_API_STATUS_INVALID_PARAMETER;
            }
            else
            {
                PSDCARD_HC_SLOT_INFO pSlotInfo = (PSDCARD_HC_SLOT_INFO)pData;

                // set the slot capabilities
                SDHCDSetSlotCapabilities(pSlotInfo, SD_SLOT_SD_4BIT_CAPABLE | 
                    SD_SLOT_SD_1BIT_CAPABLE | 
                    SD_SLOT_SDIO_CAPABLE    |
                    SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK);

                SDHCDSetVoltageWindowMask(pSlotInfo, (SD_VDD_WINDOW_3_2_TO_3_3 | SD_VDD_WINDOW_3_3_TO_3_4)); 

                // Set optimal voltage
                SDHCDSetDesiredSlotVoltage(pSlotInfo, SD_VDD_WINDOW_3_2_TO_3_3);

                SDHCDSetMaxClockRate(pSlotInfo, MAX_SDI_BUS_TRANSFER_SPEED);  

                // Set power up delay. We handle this in SetVoltage().
                SDHCDSetPowerUpDelay(pSlotInfo, 300);
            }
            break;

        default:
            DEBUGMSG (SDCARD_ZONE_WARN,(TEXT("SDHCDSlotOptionHandler option=SD_API_STATUS_INVALID_PARAMETER\r\n")));
           status = SD_API_STATUS_INVALID_PARAMETER;

    }
    DEBUGMSG (SDCARD_ZONE_FUNC,(TEXT("SDHCDSlotOptionHandler ends\r\n")));
    return status;
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::BusRequestHandler - bus request handler 
//  Input:  Slot - slot the request is going on
//          pRequest - the request
//          
//  Output: 
//  Return: SD_API_STATUS Code
//  Notes:  The request passed in is marked as uncancelable, this function
//          has the option of making the outstanding request cancelable    
//          
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CSDIOControllerBase::BusRequestHandler(DWORD dwSlot, PSD_BUS_REQUEST pRequest) 
{
	RETAILMSG (0,(TEXT("BusRequestHandler starts (CMD:%d)\r\n"), pRequest->CommandCode));
    DEBUGMSG (SDCARD_ZONE_FUNC,(TEXT("BusRequestHandler starts (CMD:%d)\r\n"), pRequest->CommandCode));
    SD_API_STATUS   status;

    // Reset FIFO and status registers
    vm_pSDIReg->SDIFSTA |= FIFO_RESET;
    vm_pSDIReg->SDIDSTA = 0xffff;
    vm_pSDIReg->SDIDCON = 0;
    vm_pSDIReg->SDICSTA = 0xffff;
    vm_pSDIReg->SDICCON = 0;	

   Start_SDI_Clock();

    //----- 1. Determine the type of command (data vs. no data) and send the command -----
    m_dwNumBytesToTransfer  = pRequest->BlockSize * pRequest->NumBlocks;


    //----- 2 Can we schedule a DMA operation using the caller's buffer as-is?  -----
#if 1
    if( ( m_dwDMAChannel == 0xffffffff ) || m_dwNumBytesToTransfer > MAXIMUM_DMA_TRANSFER_SIZE )
    {   
        m_bUseDMAForTransfer = FALSE;
        SetupPollingXfer(pRequest);                     // Use polling I/O routines for data transfer
    }
    else
    {
    	if((IS_BUFFER_DWORD_ALIGNED(pRequest->pBlockBuffer)) && (IS_BUFFER_SIZE_A_DWORD_MULTPLE(pRequest->BlockSize)) )
    	{
	       	m_DATATransferSize = 4;							// DMA transfer size DWORD

	    	if(pRequest->BlockSize == BYTES_PER_SECTOR)
    	    	m_bUseDMABurst = FALSE;
	    	else
    	    	m_bUseDMABurst = FALSE;
		}
	    else if ((IS_BUFFER_WORD_ALIGNED(pRequest->pBlockBuffer)) && (IS_BUFFER_SIZE_A_WORD_MULTPLE(pRequest->BlockSize)) )
		{
	       	m_DATATransferSize = 2;							// DMA transfer size WORD
	    }
	    else
	    {
	       	m_DATATransferSize = 1;							// DMA transfer size BYTE
	    }

        m_bUseDMAForTransfer = TRUE;
        SetupDmaXfer(pRequest);                         // Use DMA for data transfer
    }
#endif


    if(pRequest->TransferClass == SD_COMMAND) 
    {
        // Command only
        status = SendCommand(pRequest->CommandCode, pRequest->CommandArgument, pRequest->CommandResponse.ResponseType, FALSE);

        if(!SD_API_SUCCESS(status))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHCDBusRequestHandler() - Error sending command:0x%02x\r\n"), pRequest->CommandCode));
            goto BUS_REQUEST_COMPLETE;      
        }

        //----- 2. Is this the first command sent to the card?  If so, delay the 74 (or 80) clock cycles per the SD spec -----
        if(m_bSendInitClocks)
        {
            m_bSendInitClocks = FALSE;
            Wait_80_SDI_Clock_Cycles();
        }
    }

    else
    {   
        // Command with data transfer
        status = SendCommand(pRequest->CommandCode, pRequest->CommandArgument, pRequest->CommandResponse.ResponseType, TRUE);

        if(!SD_API_SUCCESS(status))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHCDBusRequestHandler() - Error sending command:0x%02x\r\n"), pRequest->CommandCode));
            goto BUS_REQUEST_COMPLETE;      
        }
    }

    //----- 3. Signal an IST that processes response information and cache our state -----
    //         NOTE: It is critical that this API NOT complete the bus request directly (which
    //               is why another thread is used).  The bus driver expects bus requests to complete
    //               asynchronously and a stack overflow WILL eventually occur if this rule isn't honored.

    SetEvent(m_hResponseReceivedEvent);

    m_CurrentState = CommandSent;
    status = SD_API_STATUS_PENDING;

BUS_REQUEST_COMPLETE:
    DEBUGMSG (SDCARD_ZONE_FUNC,(TEXT("SDHCD:BusRequestHandler ends\r\n")));
    return status;
}


//-------------------------------------- Interrupt Service Threads---------------------------------------


///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::TransferIstThread - IST thread for DMA channel dedicated to SDIO
//  Input:  pController - the controller instance
//  Output: 
//  Return: Thread exit code
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD CSDIOControllerBase::TransferIstThread()
{
    PSD_BUS_REQUEST     pRequest = NULL;       // the request to complete
    SD_API_STATUS       status;

    if( m_DMAIstThreadPriority != 0xffffffff && !CeSetThreadPriority( GetCurrentThread(), m_DMAIstThreadPriority ) )
    {
        DEBUGMSG(SDCARD_ZONE_WARN,(TEXT("SDHCDriver:TransferIstThread(): warning, failed to set CEThreadPriority \r\n")));
    }
    
    for(;;)
    {
        //----- 1. Wait for the command response -----
        status = SD_API_STATUS_PENDING;
        if(WaitForSingleObject(m_hResponseReceivedEvent, INFINITE) == WAIT_FAILED)
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCDriver:TransferIstThread(): Wait Failed!\r\n")));
            return FALSE;
        }

        if(m_bDriverShutdown) 
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCD:TransferIstThread(): Thread Exiting\r\n")));
            return FALSE;
        }

        //----- 2. Get and lock the current bus request -----
        if(pRequest == NULL)
        {
            if((pRequest = SDHCDGetAndLockCurrentRequest(m_pHCContext, 0)) == NULL)
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Unable to get/lock current request!\r\n")));
                status = SD_API_STATUS_INVALID_DEVICE_REQUEST;
                goto TRANSFER_DONE;
            }
        }

        //----- 3. Get the response information -----
        if(pRequest->CommandResponse.ResponseType == NoResponse)
        {
            goto TRANSFER_DONE;
        }else{
            status = GetCommandResponse(pRequest);

            if(!SD_API_SUCCESS(status))
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHCDBusRequestHandler() - Error getting response for command:0x%02x\r\n"), pRequest->CommandCode));
                goto TRANSFER_DONE;     
            }
        }

        //----- 4. SPECIAL CASE: The SD_CMD_STOP_TRANSMISSION that is sent after a SD_CMD_WRITE_MULTIPLE_BLOCK command -----
        //                       requires a BUSY_CHECK
        if( ( m_fHandleBusyCheckOnCommand38 && pRequest->CommandCode == SD_CMD_ERASE ) ||
            ( ( pRequest->CommandCode == SD_CMD_STOP_TRANSMISSION ) && ( m_dwLastTypeOfTransfer == SD_WRITE ) ) )
        {
            DWORD dwWaitCount = 0;
            //----- 5. Wait until the I/O transfer is complete -----
            while(!(vm_pSDIReg->SDIDSTA & BUSY_CHECKS_FINISH))
            {
                dwWaitCount++;
                if( dwWaitCount > WAIT_TIME )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - timeout waiting for BUSY_CHECKS to finish!\r\n")));
                    status = SD_API_STATUS_DATA_TIMEOUT;
                    goto TRANSFER_DONE;
                }

                if( !IsCardPresent() )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Card ejected!\r\n")));
                    status = SD_API_STATUS_DEVICE_REMOVED;
                    goto TRANSFER_DONE;
                }
                if((vm_pSDIReg->SDIFSTA & FIFO_FAIL_ERROR))
                {
                    vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH;
                    vm_pSDIReg->SDIFSTA &= FIFO_FAIL_ERROR;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - FIFO Error waiting for BUSY_CHECKS to finish!\r\n")));
                    status = SD_API_STATUS_DATA_ERROR;
                    goto TRANSFER_DONE;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_TRANSMIT_CRC_ERROR))
                {
                    vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH | DATA_TRANSMIT_CRC_ERROR;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Transmit CRC Error waiting for BUSY_CHECKS to finish!\r\n")));
                    status = SD_API_STATUS_DATA_ERROR;
                    goto TRANSFER_DONE;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_RECEIVE_CRC_ERROR))
                {
                    vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH | DATA_RECEIVE_CRC_ERROR;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Receive CRC Error waiting for BUSY_CHECKS to finish!\r\n")));
                    status = SD_API_STATUS_DATA_ERROR;
                    goto TRANSFER_DONE;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_TIME_OUT))
                {
                    vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH | DATA_TIME_OUT;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Data timeout waiting for BUSY_CHECKS to finish!\r\n")));
                    status = SD_API_STATUS_DATA_TIMEOUT;
                    goto TRANSFER_DONE;
                }
            }
            vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH;
        }


        //----- 6. If this is a data transfer, start the I/O operation; otherwise, finish the request -----
        if(pRequest->TransferClass == SD_COMMAND)
        {
            goto TRANSFER_DONE;
        }


        //----- 7. If this is a DMA transfer, we enable interrupts and wait for the DMA interrupt.  Otherwise, -----
        //         we use our polling routines to complete the I/O.
        if(m_bUseDMAForTransfer == FALSE)
        {

            //----- 8. Polling I/O, use our special routines to handle this request
            switch(pRequest->TransferClass)
            {
            case SD_READ:
                if(!PollingReceive(pRequest, m_dwNumBytesToTransfer))
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:BusRequestHandler() - PollingReceive() failed\r\n")));
                    goto TRANSFER_DONE;
                }
                break;

            case SD_WRITE:
                if(!PollingTransmit(pRequest, m_dwNumBytesToTransfer))
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:BusRequestHandler() - PollingReceive() failed\r\n")));
                    goto TRANSFER_DONE;
                }
                break;
            }

            status = SD_API_STATUS_SUCCESS;
        }else
        {


            //----- 9. For WRITE requests, be sure to copy the write data from the caller's buffer into DMA memory-----
            if(pRequest->TransferClass == SD_WRITE)
            {

                BOOL fNoException;
                DEBUGCHK(m_dwNumBytesToTransfer <= MAXIMUM_DMA_TRANSFER_SIZE);
            
                SD_SET_PROC_PERMISSIONS_FROM_REQUEST( pRequest ) {               
                    fNoException = SDPerformSafeCopy( m_pDMABuffer, pRequest->pBlockBuffer, m_dwNumBytesToTransfer );
                } SD_RESTORE_PROC_PERMISSIONS();

                if (fNoException == FALSE) {
                    status = SD_API_STATUS_ACCESS_VIOLATION;
                    goto TRANSFER_DONE;
                }
            }

            //----- 10. DMA I/O, enable the DMA channel -----
            Enable_SDIO_DMA_Channel();

            //----- 11. Wait for a DMA interrupt -----
            // first estimate the maximum DMA transfer time
            DWORD dwDelay = m_dwNumBytesToTransfer * m_dwDMATransferTimeoutFactor / ( m_dwClockRate / 2000 );
            if( Get_SDI_Bus_Width() != WIDE_BUS_ENABLE )
            {
                dwDelay *= 4;
            }
            dwDelay += m_dwDMATransferTimeoutConstant;

            // now wait for the interrupt
            if(WaitForSingleObject(m_hDMAInterruptEvent, dwDelay) != WAIT_OBJECT_0)
            {
                DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("TransferIstThread(): Wait Failed!\r\n")));
                Stop_SDIO_DMA_Channel();
                status = SD_API_STATUS_DATA_TIMEOUT;
                goto TRANSFER_DONE;
            }

            if(m_bDriverShutdown) 
            {
                DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCD:TransferIstThread(): Thread Exiting\r\n")));
                return FALSE;
            }

            //----- 12. ACK the DMA completion interrupt and stop the DMA channel -----
            InterruptDone(m_dwDMASysIntr);
            Stop_SDIO_DMA_Channel();

            DWORD dwWaitCount = 0;
            //----- 13. Wait until the I/O transfer is complete -----
            while(!(vm_pSDIReg->SDIDSTA & DATA_TRANSMIT_FINISHED))
            {
                dwWaitCount++;
                if( dwWaitCount > WAIT_TIME )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - timeout waiting for DMA transfer completion!\r\n")));
                    status = SD_API_STATUS_DATA_TIMEOUT;
                    goto TRANSFER_DONE;
                }

                if( !IsCardPresent() )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Card ejected!\r\n")));
                    status = SD_API_STATUS_DEVICE_REMOVED;
                    goto TRANSFER_DONE;
                }
                if((vm_pSDIReg->SDIFSTA & FIFO_FAIL_ERROR))
                {
                    vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH;
                    vm_pSDIReg->SDIFSTA &= FIFO_FAIL_ERROR;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - FIFO Error waiting for DMA transfer completion!\r\n")));
                    status = SD_API_STATUS_DATA_ERROR;
                    goto TRANSFER_DONE;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_TRANSMIT_CRC_ERROR))
                {
                    vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH | DATA_TRANSMIT_CRC_ERROR;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Transmit CRC Error waiting for DMA transfer completion!\r\n")));
                    status = SD_API_STATUS_DATA_ERROR;
                    goto TRANSFER_DONE;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_RECEIVE_CRC_ERROR))
                {
                    vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH | DATA_RECEIVE_CRC_ERROR;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Receive CRC Error waiting for DMA transfer completion!\r\n")));
                    status = SD_API_STATUS_DATA_ERROR;
                    goto TRANSFER_DONE;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_TIME_OUT))
                {
                    vm_pSDIReg->SDIDSTA = BUSY_CHECKS_FINISH | DATA_TIME_OUT;
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDHControllerIstThread() - Data timeout waiting for DMA transfer completion!\r\n")));
                    status = SD_API_STATUS_DATA_TIMEOUT;
                    goto TRANSFER_DONE;
                }
            }

            vm_pSDIReg->SDIDSTA = DATA_TRANSMIT_FINISHED;

            //----- 14. For READ requests, be sure to copy the data read from the DMA memory into the caller's buffer -----
            if(pRequest->TransferClass == SD_READ)
            {
                BOOL fNoException;
                DEBUGCHK(m_dwNumBytesToTransfer <= MAXIMUM_DMA_TRANSFER_SIZE);
            
                SD_SET_PROC_PERMISSIONS_FROM_REQUEST( pRequest ) {               
                    fNoException = SDPerformSafeCopy( pRequest->pBlockBuffer, m_pDMABuffer, m_dwNumBytesToTransfer );
                } SD_RESTORE_PROC_PERMISSIONS();

                if (fNoException == FALSE) {
                    status = SD_API_STATUS_ACCESS_VIOLATION;
                    goto TRANSFER_DONE;
                }
            }

            //----- 15. I/O is complete.  Finish the bus request! -----
            status = SD_API_STATUS_SUCCESS;
        }

TRANSFER_DONE:
        if( !( Is_SDIO_Interrupt_Enabled() && ( Get_SDI_Bus_Width() == WIDE_BUS_ENABLE ) ) )
        {
            Stop_SDI_Clock();
        }
        m_CurrentState = CommandComplete;   
        SDHCDIndicateBusRequestComplete(m_pHCContext, pRequest, status);
        pRequest = NULL;
    }

    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::IOInterruptIstThread - IST thread for SDIO Interrupts
//  Input:  pHCDevice - the controller instance
//  Output: 
//  Return: thread exit code
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
DWORD CSDIOControllerBase::IOInterruptIstThread()
{

    if( m_nSDIOIstThreadPriority != 0xffffffff && !CeSetThreadPriority( GetCurrentThread(), m_nSDIOIstThreadPriority ) ) 
    {
        DEBUGMSG(SDCARD_ZONE_WARN,(TEXT("SDHCDriver:SDIOInterruptIstThread(): warning, failed to set CEThreadPriority \r\n")));
    }
    
    for(;;)
    {
        //----- 1. Wait for a SDIO interrupt -----
        if(WaitForSingleObject(m_hSDIOInterruptEvent, INFINITE) != WAIT_OBJECT_0)
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCD:SDIOInterruptIstThread(): Wait Failed!\r\n")));
            return FALSE;
        }
        
        if(m_bDriverShutdown) 
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCD:SDIOInterruptIstThread(): Thread Exiting\r\n")));
            return FALSE;
        }

        if(m_bDevicePresent && IsCardPresent())
        {
            // indicate that the card is interrupting
            SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceInterrupting);
        }

        // NOTE: SDHCDIndicateSlotStateChange() is called above to inform the bus driver
        //       that the SDIO card has generated an interrupt.  After this notification, the
        //       bus driver will eventually call back SDHCDSlotOptionHandler() to enable/disable
        //       and ACK the SDIO interrupt as necessary.  Consequently, we DO NOT acknowledge 
        //       the interrupt here...

    }
}



///////////////////////////////////////////////////////////////////////////////
//  CSDIOControllerBase::CardDetectThread - thread for card insert/removal detection
//  Input:  
//  Output: 
//
//  Return: thread exit code
///////////////////////////////////////////////////////////////////////////////
DWORD CSDIOControllerBase::CardDetectThread()
{
    BOOL  bSlotStateChanged = FALSE;
    DWORD dwWaitResult  = WAIT_TIMEOUT;

    if( m_nCardDetectIstThreadPriority != 0xffffffff && !CeSetThreadPriority( GetCurrentThread(), m_nCardDetectIstThreadPriority ) )
    {
        DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCDriver:CardDetectThread(): warning, failed to set CEThreadPriority \r\n")));
    }

    for(;;)
    {
        //----- 1. Wait for the next insertion/removal interrupt -----
        dwWaitResult = WaitForSingleObject(m_hCardInsertInterruptEvent, INFINITE);
	Lock();
        //RETAILMSG(1,(TEXT("Card insert/remove detected......................!!!!\r\n"))); 
        if(m_bDriverShutdown) 
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (TEXT("SDHCardDetectIstThread: Thread Exiting\r\n")));
            return FALSE;
        }
#if 0
        // Test if a card is present
        if( IsCardPresent() == m_bDevicePresent )
        {
            bSlotStateChanged = FALSE;
        }
        else
        {
            bSlotStateChanged = TRUE;
        }

        if( bSlotStateChanged || m_bReinsertTheCard )
        {
            m_bReinsertTheCard = FALSE;

            // If a card is inserted, unload the driver...
            if(m_bDevicePresent == TRUE)
            {
                m_fCardInTheSlot = FALSE;
                // indicate the slot change 
                SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceEjected); 
                m_bDevicePresent = FALSE;

                Stop_SDI_Clock();
            }

            if(IsCardPresent())
            {
                m_fCardInTheSlot = TRUE;
                m_bDevicePresent = TRUE;

                //----- 5. Reset the clock to the ID rate -----
                vm_pSDIReg->SDICON     |= LITTLE_ENDIAN_BYTE_ORDER;    // Windows CE is always Little Endian.
                vm_pSDIReg->SDIFSTA     |= FIFO_RESET;                  // Reset the FIFO
                vm_pSDIReg->SDIBSIZE    = BYTES_PER_SECTOR; 
                vm_pSDIReg->SDIDTIMER   = MAX_DATABUSY_TIMEOUT;        // Data/busy timeout
                SetClockRate(SD_DEFAULT_CARD_ID_CLOCK_RATE);

                // give the card enough time for initialization
                Start_SDI_Clock();
                Wait_80_SDI_Clock_Cycles();

                //----- 6. Inform the bus handler that this is the first command sent -----
                m_bSendInitClocks = TRUE;

                //----- 7. Indicate the slot change -----
                SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceInserted);
            }
        } // if
#else
	if (m_bReinsertTheCard)  // when it resumes from suspend.
	{
		RETAILMSG(1,(TEXT("SDHC wakeuped\r\n"))); 
		if ( m_bDevicePresent ) // if when it gets suspend a t-flash card existed
        	{
	        	RETAILMSG(1,(TEXT("Unload sdmemory\r\n"))); 
    			m_fCardInTheSlot = FALSE;
	                // indicate the slot change 
			SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceEjected); 
			m_bDevicePresent = FALSE;
			Stop_SDI_Clock();
		}
		m_bReinsertTheCard = FALSE;		
	}
	//RETAILMSG(1,(TEXT("IsCardPresent = %d, m_bDevicePresent = %d\n"),IsCardPresent(),m_bDevicePresent));

	if( IsCardPresent() && !m_bDevicePresent ) // TF-detect INT occured. and a card exists.
	{
		RETAILMSG(1,(TEXT("Card was detected\r\n")));                    
                m_fCardInTheSlot = TRUE;
                m_bDevicePresent = TRUE;

                //----- 5. Reset the clock to the ID rate -----
                vm_pSDIReg->SDICON     |= LITTLE_ENDIAN_BYTE_ORDER;    // Windows CE is always Little Endian.
                vm_pSDIReg->SDIFSTA     |= FIFO_RESET;                  // Reset the FIFO
                vm_pSDIReg->SDIBSIZE    = BYTES_PER_SECTOR; 
                vm_pSDIReg->SDIDTIMER   = MAX_DATABUSY_TIMEOUT;        // Data/busy timeout
                SetClockRate(SD_DEFAULT_CARD_ID_CLOCK_RATE);

                // give the card enough time for initialization
                Start_SDI_Clock();
                Wait_80_SDI_Clock_Cycles();

                //----- 6. Inform the bus handler that this is the first command sent -----
                m_bSendInitClocks = TRUE;

                //----- 7. Indicate the slot change -----
                SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceInserted);
        }
		   
	else if ( !IsCardPresent() && m_bDevicePresent ) // TF-detect INT occured. but card doesn't exist.
       {
                m_fCardInTheSlot = FALSE;
                // indicate the slot change 
               SDHCDIndicateSlotStateChange(m_pHCContext, 0, DeviceEjected); 
                m_bDevicePresent = FALSE;
		RETAILMSG(1,(TEXT("Card was removed\r\n"))); 
              Stop_SDI_Clock();
       }
	UnLock();
	InterruptDone(m_dwSDDetectSysIntr);				
#endif
    } // for

    return TRUE;
}


//-------------------------------------------- Helper Functions ---------------------------------------

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       CSDIOControllerBase::SendCommand()

Description:    Issues the specified SDI command

Returns:        SD_API_STATUS status code.
-------------------------------------------------------------------*/
SD_API_STATUS CSDIOControllerBase::SendCommand(UINT16 Cmd, UINT32 Arg, UINT16 respType, BOOL bDataTransfer)
{
    unsigned int uiNewCmdRegVal = 0;
    DWORD dwWaitCount = 0;
    RETAILMSG (0,(TEXT("SendCommand (0x%08x, 0x%04x, 0x%08x, 0x%04x, 0x%x) starts\r\n"), 
                this, Cmd, Arg, respType, bDataTransfer));
    DEBUGMSG (SDHC_SEND_ZONE,(TEXT("SendCommand (0x%08x, 0x%04x, 0x%08x, 0x%04x, 0x%x) starts\r\n"), 
                this, Cmd, Arg, respType, bDataTransfer));
	//RETAILMSG (1,(TEXT("SendCommand (0x%08x, 0x%04x, 0x%08x, 0x%04x, 0x%x) starts\r\n"), 
        //        this, Cmd, Arg, respType, bDataTransfer));                
    //----- 1. Reset any pending status flags -----
    vm_pSDIReg->SDICSTA = (CRC_CHECK_FAILED | COMMAND_SENT | COMMAND_TIMED_OUT | RESPONSE_RECEIVED);

    //----- 2. Specify the command's argument -----
    vm_pSDIReg->SDICARG = Arg;

    //----- 3. Specify the command's data transfer requirements -----
    if(bDataTransfer == TRUE)
    {
        vm_pSDIReg->SDICCON  |= SDIO_COMMAND_WITH_DATA;
    }else
    {
        vm_pSDIReg->SDICCON  &= ~SDIO_COMMAND_WITH_DATA;
    }

    //----- 4. Send the command to the MMC controller -----
    switch(respType)
    {
    case NoResponse:                // Response is not required, but make sure the command was sent
        DEBUGMSG (SDHC_RESPONSE_ZONE,(TEXT("SendCommand no response required\r\n")));
        vm_pSDIReg->SDICCON = START_COMMAND | COMMAND_START_BIT | (Cmd & MAX_CMD_VALUE);

        while(!(vm_pSDIReg->SDICSTA & COMMAND_SENT))
        {
            dwWaitCount++;
            if( dwWaitCount > WAIT_TIME )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SendCommand() - timeout waiting for command completion!\r\n")));
                return SD_API_STATUS_RESPONSE_TIMEOUT;
            }

            if( !IsCardPresent() )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SendCommand() - Card ejected!\r\n")));
                return SD_API_STATUS_DEVICE_REMOVED;
            }
            if(vm_pSDIReg->SDICSTA & COMMAND_TIMED_OUT)
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SendCommand() - Command 0x%04x timed out!\r\n"), Cmd));
                vm_pSDIReg->SDICSTA = COMMAND_TIMED_OUT;                // Clear the error
                return SD_API_STATUS_RESPONSE_TIMEOUT;
            }
        }
        vm_pSDIReg->SDICSTA = COMMAND_SENT;                         // Clear the status 
        break;

    case ResponseR1:                // Short response required
    case ResponseR1b:
    case ResponseR3:
    case ResponseR4:
    case ResponseR5:                
    case ResponseR6:  
#ifdef MMC_SPEC_40
/*************************************************************************/
/****** Date : 07.05.14                                         	******/
/****** Developer : HS.JANG											******/
/****** Description : to support SD SPEC20 card. Response7 is added	******/
/****** 			  for supporting SD SPEC20 card					******/
/*************************************************************************/

    case ResponseR7:
/*************************************************************************/
#endif 
        DEBUGMSG (SDHC_RESPONSE_ZONE,(TEXT("sendSDICommand short response required\r\n")));
//      vm_pSDIReg->SDICCON = uiNewCmdRegVal | WAIT_FOR_RESPONSE | START_COMMAND | COMMAND_START_BIT | (Cmd & MAX_CMD_VALUE);
        vm_pSDIReg->SDICCON = WAIT_FOR_RESPONSE | START_COMMAND | COMMAND_START_BIT | (Cmd & MAX_CMD_VALUE);
        break;

    case ResponseR2:                // Long response required       
        DEBUGMSG (SDHC_RESPONSE_ZONE,(TEXT("sendSDICommand long response required\r\n")));
//      vm_pSDIReg->SDICCON = uiNewCmdRegVal | LONG_RESPONSE | WAIT_FOR_RESPONSE | START_COMMAND | COMMAND_START_BIT | (Cmd & MAX_CMD_VALUE);
        vm_pSDIReg->SDICCON = LONG_RESPONSE | WAIT_FOR_RESPONSE | START_COMMAND | COMMAND_START_BIT | (Cmd & MAX_CMD_VALUE);
        break;

    default:    
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:sendSDICommand() - Invalid response type.  Command not sent!\r\n")));
        return SD_API_STATUS_NOT_IMPLEMENTED;
        break;
    }
    
    return SD_API_STATUS_SUCCESS;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       CSDIOControllerBase::GetCommandResponse()

Description:    Retrieves the response info for the last SDI command
                issues.

Notes:          This routine assumes that the caller has already locked
                the current request and checked for errors.

Returns:        SD_API_STATUS status code.
-------------------------------------------------------------------*/
SD_API_STATUS CSDIOControllerBase::GetCommandResponse(PSD_BUS_REQUEST pRequest)
{
    DEBUGMSG (SDHC_RESPONSE_ZONE,(TEXT("GetCommandResponse started\r\n")));
    PUCHAR              respBuff;       // response buffer
    DWORD dwWaitCount = 0;

    //----- 1. Wait for the response information to get arrive at the controller -----
    while(!(vm_pSDIReg->SDICSTA & RESPONSE_RECEIVED))
    {
        dwWaitCount++;
        if( dwWaitCount > WAIT_TIME )
        {
        	RETAILMSG(1, (TEXT("SDHCD:GetCommandResponse() - timeout waiting for command response!\r\n")));
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:GetCommandResponse() - timeout waiting for command response!\r\n")));
            return SD_API_STATUS_RESPONSE_TIMEOUT;
        }

        if( !IsCardPresent() )
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:GetCommandResponse() - Card ejected!\r\n")));
            return SD_API_STATUS_DEVICE_REMOVED;
        }
        if(vm_pSDIReg->SDICSTA & COMMAND_TIMED_OUT)
        {
        	RETAILMSG(1, (TEXT("GetCommandResponse returned SD_API_STATUS_RESPONSE_TIMEOUT (COMMAND_TIMED_OUT)!\r\n")));
            vm_pSDIReg->SDICSTA = COMMAND_TIMED_OUT;                // Clear the error
            DEBUGMSG (SDCARD_ZONE_ERROR,(TEXT("GetCommandResponse returned SD_API_STATUS_RESPONSE_TIMEOUT (COMMAND_TIMED_OUT)\r\n")));
            return SD_API_STATUS_RESPONSE_TIMEOUT;
        }

        if(vm_pSDIReg->SDIDSTA & CRC_CHECK_FAILED)
        {
            vm_pSDIReg->SDIDSTA = CRC_CHECK_FAILED;             // Clear the error
            DEBUGMSG (SDCARD_ZONE_ERROR,(TEXT("GetCommandResponse returned SD_API_STATUS_CRC_ERROR (CRC_CHECK_FAILED)\r\n")));
            return SD_API_STATUS_CRC_ERROR;
        }

        if(vm_pSDIReg->SDIDSTA & DATA_TRANSMIT_CRC_ERROR)
        {
            vm_pSDIReg->SDIDSTA = DATA_TRANSMIT_CRC_ERROR;      // Clear the error
            DEBUGMSG (SDCARD_ZONE_ERROR,(TEXT("getSDICommandResponse returned SD_API_STATUS_CRC_ERROR (DATA_TRANSMIT_CRC_ERROR)\r\n")));
            return SD_API_STATUS_CRC_ERROR; 
        }

        if(vm_pSDIReg->SDIDSTA & DATA_RECEIVE_CRC_ERROR)
        {
            vm_pSDIReg->SDIDSTA = DATA_RECEIVE_CRC_ERROR;           // Clear the error
            DEBUGMSG (SDCARD_ZONE_ERROR,(TEXT("GetCommandResponse returned SD_API_STATUS_CRC_ERROR (DATA_RECEIVE_CRC_ERROR)\r\n")));
            return SD_API_STATUS_CRC_ERROR;
        }

        if(vm_pSDIReg->SDIDSTA & DATA_TIME_OUT)
        {
            vm_pSDIReg->SDIDSTA = DATA_TIME_OUT;                    // Clear the error
            DEBUGMSG (SDCARD_ZONE_ERROR,(TEXT("GetCommandResponse returned SD_API_STATUS_DATA_TIMEOUT (DATA_TIME_OUT)\r\n")));
            return SD_API_STATUS_DATA_TIMEOUT;
        }
    }
    RETAILMSG(0, (TEXT("RESPONSE_RECEIVED\r\n")));
    vm_pSDIReg->SDICSTA = RESPONSE_RECEIVED;                        // Clear the status 


    //----- 2. Copy the response information to our "response buffer" -----
    //         NOTE: All START_BIT and TRANSMISSION_BIT bits ='0'. All END_BIT bits ='0'. All RESERVED bits ='1'
    respBuff = pRequest->CommandResponse.ResponseBuffer;
    switch(pRequest->CommandResponse.ResponseType)
    {
        case NoResponse:
            break;

        case ResponseR1:                
        case ResponseR1b:
#ifdef MMC_SPEC_40
/*************************************************************************/
/****** Date : 07.05.14                                         	******/
/****** Developer : HS.JANG											******/
/****** Description : to support SD SPEC20 card. Response7 is added	******/
/****** 			  for supporting SD SPEC20 card					******/
/*************************************************************************/

		case ResponseR7:
/*************************************************************************/
#endif 
            //--- SHORT RESPONSE (48 bits total)--- 
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | COMMAND_INDEX(6) | CARD_STATUS(32) | CRC7(7) | END_BIT(1) }
            // NOTE: START_BIT and TRANSMISSION_BIT = 0, END_BIT = 1
            //
            *(respBuff    ) = (BYTE)(START_BIT | TRANSMISSION_BIT | pRequest->CommandCode);
            *(respBuff + 1) = (BYTE)(vm_pSDIReg->SDIRSP0      );
            *(respBuff + 2) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 8 );
            *(respBuff + 3) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 16);
            *(respBuff + 4) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 24);
            *(respBuff + 5) = (BYTE)(END_RESERVED | END_BIT);
           
           	RETAILMSG(0,(TEXT("*(vm_pSDIReg->SDIRSP0) = 0x%x\n"),(vm_pSDIReg->SDIRSP0)));
            break;

        case ResponseR3:
        case ResponseR4:
            //--- SHORT RESPONSE (48 bits total)--- 
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | RESERVED(6) | CARD_STATUS(32) | RESERVED(7) | END_BIT(1) }
            //
            *(respBuff    ) = (BYTE)(START_BIT | TRANSMISSION_BIT | START_RESERVED);
            *(respBuff + 1) = (BYTE)(vm_pSDIReg->SDIRSP0      );
            *(respBuff + 2) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 8 );
            *(respBuff + 3) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 16);
            *(respBuff + 4) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 24);
            *(respBuff + 5) = (BYTE)(END_RESERVED | END_BIT);  
            RETAILMSG(0,(TEXT("*(vm_pSDIReg->SDIRSP0) = 0x%x\n"),(vm_pSDIReg->SDIRSP0)));
            break;

        case ResponseR5:                
        case ResponseR6:
            //--- SHORT RESPONSE (48 bits total)--- 
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | COMMAND_INDEX(6) | RCA(16) | CARD_STATUS(16) | CRC7(7) | END_BIT(1) }
            //
            *(respBuff    ) = (BYTE)(START_BIT | TRANSMISSION_BIT | pRequest->CommandCode);
            *(respBuff + 1) = (BYTE)(vm_pSDIReg->SDIRSP0      );
            *(respBuff + 2) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 8 );
            *(respBuff + 3) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 16);
            *(respBuff + 4) = (BYTE)(vm_pSDIReg->SDIRSP0 >> 24);
            *(respBuff + 5) = (BYTE)(vm_pSDIReg->SDIRSP1 >> 24);   
            RETAILMSG(0,(TEXT("*(vm_pSDIReg->SDIRSP0) = 0x%x\n"),(vm_pSDIReg->SDIRSP0)));
            break;

        case ResponseR2:
            //--- LONG RESPONSE (136 bits total)--- 
            // Format: { START_BIT(1) | TRANSMISSION_BIT(1) | RESERVED(6) | CARD_STATUS(127) | END_BIT(1) }
            //
            // NOTE: In this implementation, the caller doesn't require the upper 8 bits of reserved data.
            //       Consequently, these bits aren't included and the response info is copied directly into
            //       the beginning of the supplied buffer.      
            //
            //*(respBuff    )   = (BYTE)(START_BIT | TRANSMISSION_BIT | START_RESERVED);
            //
            *(respBuff + 0) = (BYTE)(vm_pSDIReg->SDIRSP3      );
            *(respBuff + 1) = (BYTE)(vm_pSDIReg->SDIRSP3 >> 8 );
            *(respBuff + 2) = (BYTE)(vm_pSDIReg->SDIRSP3 >> 16);
            *(respBuff + 3) = (BYTE)(vm_pSDIReg->SDIRSP3 >> 24);

            *(respBuff + 4) = (BYTE)(vm_pSDIReg->SDIRSP2      );
            *(respBuff + 5) = (BYTE)(vm_pSDIReg->SDIRSP2 >> 8 );
            *(respBuff + 6) = (BYTE)(vm_pSDIReg->SDIRSP2 >> 16);
            *(respBuff + 7) = (BYTE)(vm_pSDIReg->SDIRSP2 >> 24);

            *(respBuff + 8) = (BYTE)(vm_pSDIReg->SDIRSP1      );
            *(respBuff + 9) = (BYTE)(vm_pSDIReg->SDIRSP1 >> 8 );
            *(respBuff + 10)= (BYTE)(vm_pSDIReg->SDIRSP1 >> 16);
            *(respBuff + 11)= (BYTE)(vm_pSDIReg->SDIRSP1 >> 24);

            *(respBuff + 12)= (BYTE)(vm_pSDIReg->SDIRSP0      );
            *(respBuff + 13)= (BYTE)(vm_pSDIReg->SDIRSP0 >> 8 );
            *(respBuff + 14)= (BYTE)(vm_pSDIReg->SDIRSP0 >> 16);
            *(respBuff + 15)= (BYTE)(vm_pSDIReg->SDIRSP0 >> 24);
            RETAILMSG(0,(TEXT("*(vm_pSDIReg->SDIRSP3) = 0x%x\n"),(vm_pSDIReg->SDIRSP3)));
            RETAILMSG(0,(TEXT("*(vm_pSDIReg->SDIRSP2) = 0x%x\n"),(vm_pSDIReg->SDIRSP2)));
			RETAILMSG(0,(TEXT("*(vm_pSDIReg->SDIRSP1) = 0x%x\n"),(vm_pSDIReg->SDIRSP1)));
            RETAILMSG(0,(TEXT("*(vm_pSDIReg->SDIRSP0) = 0x%x\n"),(vm_pSDIReg->SDIRSP0)));
            break;

        default:
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:GetCmdResponse(): Unrecognized response type!\r\n")));
            break;
    }

    DEBUGMSG (SDHC_RESPONSE_ZONE,(TEXT("GetCommandResponse returned SD_API_STATUS_SUCCESS\r\n")));
    return SD_API_STATUS_SUCCESS;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       CSDIOControllerBase::SetupDmaXfer()

Description:    Prepares the SDI device for a transfer

Returns:        SD_API_STATUS code.
-------------------------------------------------------------------*/
BOOL CSDIOControllerBase::SetupDmaXfer(PSD_BUS_REQUEST pRequest)
{
    //----- 1. Check the parameters -----
    if(!pRequest)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SetupDmaXfer() - Invalid parameters!\r\n")));
        return FALSE;
    }

    //----- 2. Setup the SD/MMC controller according to the type of transfer -----
    switch(pRequest->TransferClass)
    {
    case SD_READ:

        //********* 3. READ request *********
        m_dwLastTypeOfTransfer = SD_READ;

        //----- 3a. Reset the FIFO -----
        vm_pSDIReg->SDIDTIMER   = MAX_DATABUSY_TIMEOUT;        
        vm_pSDIReg->SDIBSIZE      = pRequest->BlockSize;
        Stop_SDIO_DMA_Channel();

//hsJang start
        switch( m_dwDMAChannel )
        {
        case 0:
            //----- 3b. Initialize the DMA channel for input mode -----
	        if (m_DATATransferSize == 4)
	            vm_pDMAreg->DISRC0   = (int)MMCFIFO_DWORD_PHYS;   
	        else if (m_DATATransferSize == 2)
	            vm_pDMAreg->DISRC0   = (int)MMCFIFO_WORD_PHYS;
	        else
	            vm_pDMAreg->DISRC0   = (int)MMCFIFO_BYTE_PHYS;
            vm_pDMAreg->DISRCC0 |= (SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS); // Source is periperal bus, fixed addr
            vm_pDMAreg->DIDST0   = (int)m_pDMABufferPhys.LowPart;
            vm_pDMAreg->DIDSTC0 &= ~(DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS); // Destination is system bus, increment addr
            //----- 3c. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
            //         single RX, single service,, MMC request, no auto-reload, word (32 bits), RX count
   	        vm_pDMAreg->DCON0  = (  HANDSHAKE_MODE | GENERATE_INTERRUPT 
	       	                            | NO_DMA_AUTO_RELOAD );
		vm_pDMAreg ->DMAREQSEL0 = ( DMAREQSEL_SDMMC | DMA_TRIGGERED_BY_HARDWARE );
	        if (m_DATATransferSize == 4)
	        	if (m_bUseDMABurst)
    	        	vm_pDMAreg->DCON0  |= ( SELECT_BURST_TRANSFER | TRANSFER_WORD | ((pRequest->BlockSize >> 4) * pRequest->NumBlocks));
	        	else
    	        	vm_pDMAreg->DCON0  |= ( TRANSFER_WORD | ((pRequest->BlockSize >> 2) * pRequest->NumBlocks));
	        else if (m_DATATransferSize == 2)
    	        vm_pDMAreg->DCON0  |= ( TRANSFER_HALF_WORD | ((pRequest->BlockSize >> 1) * pRequest->NumBlocks));
	        else
    	        vm_pDMAreg->DCON0  |= ( TRANSFER_BYTE | ((pRequest->BlockSize) * pRequest->NumBlocks));
            break;

        case 1:
            //----- 3b. Initialize the DMA channel for input mode -----
	        if (m_DATATransferSize == 4)
	            vm_pDMAreg->DISRC1   = (int)MMCFIFO_DWORD_PHYS;   
	        else if (m_DATATransferSize == 2)
	            vm_pDMAreg->DISRC1   = (int)MMCFIFO_WORD_PHYS;
	        else
	            vm_pDMAreg->DISRC1   = (int)MMCFIFO_BYTE_PHYS;
            vm_pDMAreg->DISRCC1    |= (SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);              // Source is periperal bus, fixed addr
            vm_pDMAreg->DIDST1 = (int)m_pDMABufferPhys.LowPart; 
            vm_pDMAreg->DIDSTC1 &= ~(DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);  // Destination is system bus, increment addr
            //----- 3c. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
            //         single RX, single service,, MMC request, no auto-reload, word (32 bits), RX count
   	        vm_pDMAreg->DCON1  = (  HANDSHAKE_MODE | GENERATE_INTERRUPT | NO_DMA_AUTO_RELOAD );
		vm_pDMAreg ->DMAREQSEL1 = (DMAREQSEL_SDMMC | DMA_TRIGGERED_BY_HARDWARE);
	        if (m_DATATransferSize == 4)
	        	if (m_bUseDMABurst)
    	        	vm_pDMAreg->DCON1  |= ( SELECT_BURST_TRANSFER | TRANSFER_WORD | ((pRequest->BlockSize >> 4) * pRequest->NumBlocks));
	        	else
    	        	vm_pDMAreg->DCON1  |= ( TRANSFER_WORD | ((pRequest->BlockSize >> 2) * pRequest->NumBlocks));
	        else if (m_DATATransferSize == 2)
    	        vm_pDMAreg->DCON1  |= ( TRANSFER_HALF_WORD | ((pRequest->BlockSize >> 1) * pRequest->NumBlocks));
	        else
    	        vm_pDMAreg->DCON1  |= ( TRANSFER_BYTE | ((pRequest->BlockSize) * pRequest->NumBlocks));
            break;

        case 2:
            //----- 3b. Initialize the DMA channel for input mode -----
	        if (m_DATATransferSize == 4)
	            vm_pDMAreg->DISRC2   = (int)MMCFIFO_DWORD_PHYS;   
	        else if (m_DATATransferSize == 2)
	            vm_pDMAreg->DISRC2   = (int)MMCFIFO_WORD_PHYS;
	        else
	            vm_pDMAreg->DISRC2   = (int)MMCFIFO_BYTE_PHYS;
            vm_pDMAreg->DISRCC2    |= (SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);              // Source is periperal bus, fixed addr
            vm_pDMAreg->DIDST2 = (int)m_pDMABufferPhys.LowPart; 
            vm_pDMAreg->DIDSTC2 &= ~(DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);  // Destination is system bus, increment addr
            //----- 3c. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
            //         single RX, single service,, MMC request, no auto-reload, word (32 bits), RX count
   	        vm_pDMAreg->DCON2  = (  HANDSHAKE_MODE | GENERATE_INTERRUPT  
	       	                            | NO_DMA_AUTO_RELOAD );
		vm_pDMAreg ->DMAREQSEL2 = ( DMAREQSEL_SDMMC | DMA_TRIGGERED_BY_HARDWARE );			
	        if (m_DATATransferSize == 4)
	        	if (m_bUseDMABurst)
    	        	vm_pDMAreg->DCON2  |= ( SELECT_BURST_TRANSFER | TRANSFER_WORD | ((pRequest->BlockSize >> 4) * pRequest->NumBlocks));
	        	else
    	        	vm_pDMAreg->DCON2  |= ( TRANSFER_WORD | ((pRequest->BlockSize >> 2) * pRequest->NumBlocks));
	        else if (m_DATATransferSize == 2)
    	        vm_pDMAreg->DCON2  |= ( TRANSFER_HALF_WORD | ((pRequest->BlockSize >> 1) * pRequest->NumBlocks));
	        else
    	        vm_pDMAreg->DCON2  |= ( TRANSFER_BYTE | ((pRequest->BlockSize) * pRequest->NumBlocks));
            break;

        case 3:
            //----- 3b. Initialize the DMA channel for input mode -----
	        if (m_DATATransferSize == 4)
	            vm_pDMAreg->DISRC3   = (int)MMCFIFO_DWORD_PHYS;   
	        else if (m_DATATransferSize == 2)
	            vm_pDMAreg->DISRC3   = (int)MMCFIFO_WORD_PHYS;
	        else
	            vm_pDMAreg->DISRC3   = (int)MMCFIFO_BYTE_PHYS;
            vm_pDMAreg->DISRCC3    |= (SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);              // Source is periperal bus, fixed addr
            vm_pDMAreg->DIDST3 = (int)m_pDMABufferPhys.LowPart; 
            vm_pDMAreg->DIDSTC3 &= ~(DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);  // Destination is system bus, increment addr
            //----- 3c. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
            //         single RX, single service,, MMC request, no auto-reload, word (32 bits), RX count
   	        vm_pDMAreg->DCON3  = (  HANDSHAKE_MODE | GENERATE_INTERRUPT  
	       	                            | NO_DMA_AUTO_RELOAD );
		vm_pDMAreg ->DMAREQSEL3 = ( DMAREQSEL_SDMMC | DMA_TRIGGERED_BY_HARDWARE );			
	        if (m_DATATransferSize == 4)
	        	if (m_bUseDMABurst)
    	        	vm_pDMAreg->DCON3  |= ( SELECT_BURST_TRANSFER | TRANSFER_WORD | ((pRequest->BlockSize >> 4) * pRequest->NumBlocks));
	        	else
    	        	vm_pDMAreg->DCON3  |= ( TRANSFER_WORD | ((pRequest->BlockSize >> 2) * pRequest->NumBlocks));
	        else if (m_DATATransferSize == 2)
    	        vm_pDMAreg->DCON3  |= ( TRANSFER_HALF_WORD | ((pRequest->BlockSize >> 1) * pRequest->NumBlocks));
	        else
    	        vm_pDMAreg->DCON3  |= ( TRANSFER_BYTE | ((pRequest->BlockSize) * pRequest->NumBlocks));
            break;

        default:
            ASSERT(0); // invalid DMA Channel... we should never get here
        }   

        //----- 3d. Setup the controller and DMA channel appropriately -----
       	vm_pSDIReg->SDIDCON   = RECEIVE_AFTER_COMMAND | TRANSFER_BLOCK_MODE | Get_SDI_Bus_Width() | DMA_ENABLE | DATA_TRANS_START | DATA_RECEIVE_START | pRequest->NumBlocks;
        if (m_DATATransferSize == 4)
        	if (m_bUseDMABurst)
        		vm_pSDIReg->SDIDCON  |= (BURST4_ENABLE | DATA_SIZE_WORD);
        	else
        		vm_pSDIReg->SDIDCON  |= DATA_SIZE_WORD;
        else if(m_DATATransferSize == 2)
         	vm_pSDIReg->SDIDCON  |= DATA_SIZE_HWORD;
        else
        	vm_pSDIReg->SDIDCON  |= DATA_SIZE_BYTE;

        break;
    
    case SD_WRITE:

        //********* 4. WRITE request *********
        m_dwLastTypeOfTransfer = SD_WRITE;

        //----- 4a. Reset the FIFO -----
        vm_pSDIReg->SDIDTIMER   = MAX_DATABUSY_TIMEOUT;        
        vm_pSDIReg->SDIBSIZE      = pRequest->BlockSize;
        Stop_SDIO_DMA_Channel();

        switch( m_dwDMAChannel )
        {
        case 0:
            //----- 4b. Initialize the DMA channel for output mode -----
            vm_pDMAreg->DISRC0 = (int)m_pDMABufferPhys.LowPart; 
            vm_pDMAreg->DISRCC0    &= ~(SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);             // Source is periperal bus, fixed addr

	        if (m_DATATransferSize == 4)
	            vm_pDMAreg->DIDST0   = (int)MMCFIFO_DWORD_PHYS;   
	        else if (m_DATATransferSize == 2)
	            vm_pDMAreg->DIDST0   = (int)MMCFIFO_WORD_PHYS;
	        else
	            vm_pDMAreg->DIDST0   = (int)MMCFIFO_BYTE_PHYS;
            vm_pDMAreg->DIDSTC0 |= (DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);       // Destination is system bus, increment addr

            //----- 4c. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
            //          single TX, single service, MMC request, no auto-reload, word (32 bits), TX count
   	        vm_pDMAreg->DCON0  = (  HANDSHAKE_MODE | GENERATE_INTERRUPT  
       	                                    | NO_DMA_AUTO_RELOAD);
		vm_pDMAreg ->DMAREQSEL0 = ( DMAREQSEL_SDMMC | DMA_TRIGGERED_BY_HARDWARE );			
	        if (m_DATATransferSize == 4)
               	if (m_bUseDMABurst)
	    	        vm_pDMAreg->DCON0  |= (  SELECT_BURST_TRANSFER | TRANSFER_WORD | ((pRequest->BlockSize >> 4) * pRequest->NumBlocks));
				else
	    	        vm_pDMAreg->DCON0  |= (  TRANSFER_WORD | ((pRequest->BlockSize >> 2) * pRequest->NumBlocks));
			else if(m_DATATransferSize == 2)
    	        vm_pDMAreg->DCON0  |= (  TRANSFER_HALF_WORD | ((pRequest->BlockSize >> 1) * pRequest->NumBlocks));
			else
    	        vm_pDMAreg->DCON0  |= (  TRANSFER_BYTE | ((pRequest->BlockSize) * pRequest->NumBlocks));
            break;

        case 1:
            //----- 4b. Initialize the DMA channel for output mode -----
            vm_pDMAreg->DISRC1 = (int)m_pDMABufferPhys.LowPart; 
            vm_pDMAreg->DISRCC1    &= ~(SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);             // Source is periperal bus, fixed addr

	        if (m_DATATransferSize == 4)
	            vm_pDMAreg->DIDST1   = (int)MMCFIFO_DWORD_PHYS;   
	        else if (m_DATATransferSize == 2)
	            vm_pDMAreg->DIDST1   = (int)MMCFIFO_WORD_PHYS;
	        else
	            vm_pDMAreg->DIDST1   = (int)MMCFIFO_BYTE_PHYS;
            vm_pDMAreg->DIDSTC1 |= (DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);       // Destination is system bus, increment addr

            //----- 4c. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
            //          single TX, single service, MMC request, no auto-reload, word (32 bits), TX count
   	        vm_pDMAreg->DCON1  = (  HANDSHAKE_MODE | GENERATE_INTERRUPT | NO_DMA_AUTO_RELOAD);
		vm_pDMAreg ->DMAREQSEL1 = ( DMAREQSEL_SDMMC | DMA_TRIGGERED_BY_HARDWARE );
			
	        if (m_DATATransferSize == 4)
               	if (m_bUseDMABurst)
	    	        vm_pDMAreg->DCON1  |= (  SELECT_BURST_TRANSFER | TRANSFER_WORD | ((pRequest->BlockSize >> 4) * pRequest->NumBlocks));
				else
	    	        vm_pDMAreg->DCON1  |= (  TRANSFER_WORD | ((pRequest->BlockSize >> 2) * pRequest->NumBlocks));
			else if(m_DATATransferSize == 2)
    	        vm_pDMAreg->DCON1  |= (  TRANSFER_HALF_WORD | ((pRequest->BlockSize >> 1) * pRequest->NumBlocks));
			else
    	        vm_pDMAreg->DCON1  |= (  TRANSFER_BYTE | ((pRequest->BlockSize) * pRequest->NumBlocks));
            break;

        case 2:
            //----- 4b. Initialize the DMA channel for output mode -----
            vm_pDMAreg->DISRC2 = (int)m_pDMABufferPhys.LowPart; 
            vm_pDMAreg->DISRCC2    &= ~(SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);             // Source is periperal bus, fixed addr

	        if (m_DATATransferSize == 4)
	            vm_pDMAreg->DIDST2   = (int)MMCFIFO_DWORD_PHYS;   
	        else if (m_DATATransferSize == 2)
	            vm_pDMAreg->DIDST2   = (int)MMCFIFO_WORD_PHYS;
	        else
	            vm_pDMAreg->DIDST2   = (int)MMCFIFO_BYTE_PHYS;
            vm_pDMAreg->DIDSTC2 |= (DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);       // Destination is system bus, increment addr

            //----- 4c. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
            //          single TX, single service, MMC request, no auto-reload, word (32 bits), TX count
   	        vm_pDMAreg->DCON2  = (  HANDSHAKE_MODE | GENERATE_INTERRUPT  
       	                                    | NO_DMA_AUTO_RELOAD);
		vm_pDMAreg ->DMAREQSEL2 = ( DMAREQSEL_SDMMC | DMA_TRIGGERED_BY_HARDWARE );			
	        if (m_DATATransferSize == 4)
               	if (m_bUseDMABurst)
	    	        vm_pDMAreg->DCON2  |= (  SELECT_BURST_TRANSFER | TRANSFER_WORD | ((pRequest->BlockSize >> 4) * pRequest->NumBlocks));
				else
	    	        vm_pDMAreg->DCON2  |= (  TRANSFER_WORD | ((pRequest->BlockSize >> 2) * pRequest->NumBlocks));
			else if(m_DATATransferSize == 2)
    	        vm_pDMAreg->DCON2  |= (  TRANSFER_HALF_WORD | ((pRequest->BlockSize >> 1) * pRequest->NumBlocks));
			else
    	        vm_pDMAreg->DCON2  |= (  TRANSFER_BYTE | ((pRequest->BlockSize) * pRequest->NumBlocks));
            break;

        case 3:
            //----- 4b. Initialize the DMA channel for output mode -----
            vm_pDMAreg->DISRC3 = (int)m_pDMABufferPhys.LowPart; 
            vm_pDMAreg->DISRCC3    &= ~(SOURCE_PERIPHERAL_BUS | FIXED_SOURCE_ADDRESS);             // Source is periperal bus, fixed addr

	        if (m_DATATransferSize == 4)
	            vm_pDMAreg->DIDST3   = (int)MMCFIFO_DWORD_PHYS;   
	        else if (m_DATATransferSize == 2)
	            vm_pDMAreg->DIDST3   = (int)MMCFIFO_WORD_PHYS;
	        else
	            vm_pDMAreg->DIDST3   = (int)MMCFIFO_BYTE_PHYS;
            vm_pDMAreg->DIDSTC3 |= (DESTINATION_PERIPHERAL_BUS | FIXED_DESTINATION_ADDRESS);       // Destination is system bus, increment addr

            //----- 4c. Configure the DMA channel's transfer characteristics: handshake, sync PCLK, interrupt, -----
            //          single TX, single service, MMC request, no auto-reload, word (32 bits), TX count
   	        vm_pDMAreg->DCON3  = (  HANDSHAKE_MODE | GENERATE_INTERRUPT  
       	                                    | NO_DMA_AUTO_RELOAD);
		vm_pDMAreg ->DMAREQSEL3 = ( DMAREQSEL_SDMMC | DMA_TRIGGERED_BY_HARDWARE );			
	        if (m_DATATransferSize == 4)
               	if (m_bUseDMABurst)
	    	        vm_pDMAreg->DCON3  |= (  SELECT_BURST_TRANSFER | TRANSFER_WORD | ((pRequest->BlockSize >> 4) * pRequest->NumBlocks));
				else
	    	        vm_pDMAreg->DCON3  |= (  TRANSFER_WORD | ((pRequest->BlockSize >> 2) * pRequest->NumBlocks));
			else if(m_DATATransferSize == 2)
    	        vm_pDMAreg->DCON3  |= (  TRANSFER_HALF_WORD | ((pRequest->BlockSize >> 1) * pRequest->NumBlocks));
			else
    	        vm_pDMAreg->DCON3  |= (  TRANSFER_BYTE | ((pRequest->BlockSize) * pRequest->NumBlocks));
			
            break;

//hsJang end
            

        default:
            ASSERT(0); // invalid DMA Channel... we should never get here
        }

        //----- 4d. Setup the controller and DMA channel appropriately -----
        vm_pSDIReg->SDIDCON  = TRANSMIT_AFTER_RESPONSE | TRANSFER_BLOCK_MODE | Get_SDI_Bus_Width() | DMA_ENABLE | DATA_TRANS_START | DATA_TRANSMIT_START | pRequest->NumBlocks;
        if (m_DATATransferSize == 4)
        	if (m_bUseDMABurst)
		        vm_pSDIReg->SDIDCON  |= (BURST4_ENABLE | DATA_SIZE_WORD);
			else
		        vm_pSDIReg->SDIDCON  |= DATA_SIZE_WORD;
        else if(m_DATATransferSize == 2)
	        vm_pSDIReg->SDIDCON  |= DATA_SIZE_HWORD;
		else
	        vm_pSDIReg->SDIDCON  |= DATA_SIZE_BYTE;
		
        break;


    case SD_COMMAND:

        //----- 5. SPECIAL CASE: The SD_CMD_STOP_TRANSMISSION command requires that BUSY_AFTER_COMMAND be setup. -----
        if( ( m_fHandleBusyCheckOnCommand38 && pRequest->CommandCode == SD_CMD_ERASE ) ||
            ( ( pRequest->CommandCode == SD_CMD_STOP_TRANSMISSION ) && ( m_dwLastTypeOfTransfer == SD_WRITE ) ) )
        {
            vm_pSDIReg->SDIDCON = BUSY_AFTER_COMMAND | TRANSFER_BLOCK_MODE | DATA_TRANS_START | DATA_BUSY | pRequest->NumBlocks;
        }
        break;

    default:
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SetupDmaXfer(): Unrecognized transfer mode!\r\n")));
        return FALSE;
    }

    return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       CSDIOControllerBase::IsCardBusy()

Description:    Determines if the SDI card is busy

Returns:        Boolean indicating if SDI card is busy.
-------------------------------------------------------------------*/
BOOL CSDIOControllerBase::IsCardBusy(UINT16 inData)
{
    //----- 1. Is there a command currently in progress? -----
    if(vm_pSDIReg->SDICSTA & COMMAND_IN_PROGRESS)
        return TRUE;

    //----- 2. Is there a transfer in progress? -----
    if((vm_pSDIReg->SDIDSTA & DATA_TRANSMIT_IN_PROGRESS) || 
       (vm_pSDIReg->SDIDSTA & DATA_RECIEVE_IN_PROGRESS))
       return TRUE;

    //----- 3. Is there a READ-WAIT request in progress? -----
    if(vm_pSDIReg->SDIDSTA & READ_WAIT_REQUEST_OCCURED)
        return TRUE;

    return FALSE;
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       CSDIOControllerBase::SetClockRate()

Description:    Initializes the SDI clock to the specified baud rate.

Notes:          Sets the SDI clock rate the specified value.  This routine 
                first makes sure that the SDI clock is stopped but the caller 
                is responsible for starting the clock back up again.

Returns:        Actual clock rate set
-------------------------------------------------------------------*/
DWORD CSDIOControllerBase::SetClockRate(DWORD dwClockRate)
{
    unsigned int PCLK;
    SetKMode(TRUE);
    PCLK = S3C2450_PCLK;
    SetKMode(FALSE);
	
    if((dwClockRate < 0) || (dwClockRate > SD_FULL_SPEED_RATE))
    {
        DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDHCDriver:SetClockRate() - invalid clock rate %d !\r\n"), dwClockRate));
        goto DONE;
    }

    //----- BUGBUG -----
    // HARDWARE BUG WORKAROUND
    //
    // The maximum reliable speed for MMC cards is 20Mhz.
    if(dwClockRate == MMC_FULL_SPEED_RATE)
    {
    	vm_pSDIReg->SDICON |= SDCLK_TYPE_MMC;
		dwClockRate = 20000000;
    }
    else
    {
    	vm_pSDIReg->SDICON &= ~SDCLK_TYPE_MMC;
    }

    //----- 2. Calculate the clock rate  -----
    //         NOTE: Using the prescale value, the clock's baud rate is
    //               determined as follows: baud_rate = (PCLK / (prescale + 1))
    //
    //               Hence, the prescale value can be computed as follows:
    //               prescale = ((PCLK / (baud_rate)) - 1

	
    DWORD dwPrescale = dwClockRate ? ( (PCLK / (dwClockRate)) - 1 ) : 0xff;
    DWORD dwActualRate = PCLK / ( dwPrescale + 1 );
	//RETAILMSG(1, (TEXT("PCLK:dwActualRate:%d,%d\n"), PCLK, dwActualRate));

    // adjust the rate if too high
    if( dwActualRate > dwClockRate )
    {
        dwPrescale++; // set to next supported lower rate
    
        // recalculate the actual rate
        dwActualRate = PCLK / ( dwPrescale + 1 );
    }

    // adjust the rate if too low
    if( dwPrescale > 0xff )
    {
        dwPrescale = 0xff; // set to slowest supported rate
    
        // recalculate the actual rate
        dwActualRate = PCLK / ( dwPrescale + 1 );
    }

    BOOL fClockIsRunning = Is_SDI_Clock_Running();

    //----- 2. Make sure the clock is stopped -----
    if( fClockIsRunning )
    {
        Stop_SDI_Clock();
    }

    //----- 3. Set the clock rate  -----
    vm_pSDIReg->SDIPRE = dwPrescale;

    DEBUGMSG(SDHC_CLOCK_ZONE, (TEXT("SDHCD:SetClockRate() - Clock rate set to %d Hz\r\n"), dwActualRate));
    RETAILMSG(1, (TEXT("SDHCD:SetClockRate() - Clock rate set to %d Hz\r\n"), dwActualRate));

    if( fClockIsRunning )
    {
        Start_SDI_Clock();
    }

    // remember the current clock rate
    m_dwClockRate = dwActualRate;

DONE:
    return m_dwClockRate;
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       CSDIOControllerBase::PollingTransmit()

Description:    Transmits the specified data from the SD card via polling.

Notes:          Some SDIO client drivers need to issue short (i.e. 2 byte)
                I/O requests to their hardware.  For these types of transfers,
                DMA will *NOT* work.

                This routine DOES handle the case when the supplied buffer is NOT 
                DWORD-aligned.

Returns:        Boolean indicating success.
-------------------------------------------------------------------*/
BOOL CSDIOControllerBase::PollingTransmit(PSD_BUS_REQUEST pRequest, DWORD dwLen)
{
    BOOL fRetVal = TRUE;
    PBYTE pBuff = pRequest->pBlockBuffer;
 
    SD_SET_PROC_PERMISSIONS_FROM_REQUEST( pRequest ) {               
        __try { 
            //----- 1. Send the data to the FIFO -----
            while(dwLen > 0)
            {
                if( !m_fCardInTheSlot )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - Card ejected!\r\n")));
                    fRetVal = FALSE;
                    break;
                }
                if((vm_pSDIReg->SDIFSTA & FIFO_FAIL_ERROR))
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - FIFO Error!\r\n")));
                    fRetVal = FALSE;
                    break;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_TRANSMIT_CRC_ERROR))
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - Transmit CRC Error!\r\n")));
                    fRetVal = FALSE;
                    break;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_TIME_OUT))
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - Data timeout!\r\n")));
                    fRetVal = FALSE;
                    break;
                }
                //----- 2. Is the FIFO ready to accept more data? -----
                if((vm_pSDIReg->SDIFSTA & FIFO_AVAIL_FOR_TX))
                {
                    vm_pSDIReg->SDIDAT8 = *pBuff++;
                    dwLen--;
                }
            }
        } __except (SDProcessException(GetExceptionInformation())) {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("--- SDHC Driver: Exception caught in PollingTransmit\r\n")));
            fRetVal = FALSE;
        }
    } SD_RESTORE_PROC_PERMISSIONS();

    if( fRetVal )
    {
        DWORD dwWaitCount = 0;
        //----- 4. Make sure the transmit completes! -----
        while(!(vm_pSDIReg->SDIDSTA & DATA_TRANSMIT_FINISHED))
        {
            dwWaitCount++;
            if( dwWaitCount > WAIT_TIME )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - timeout waiting for DATA_TRANSMIT_FINISHED!\r\n")));
                return SD_API_STATUS_RESPONSE_TIMEOUT;
            }

            if( !IsCardPresent() )
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - Card ejected!\r\n")));
                fRetVal = FALSE;
                break;
            }
            if((vm_pSDIReg->SDIFSTA & FIFO_FAIL_ERROR))
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - FIFO Error waiting for DATA_TRANSMIT_FINISHED!\r\n")));
                fRetVal = FALSE;
                break;
            }
            if((vm_pSDIReg->SDIDSTA & DATA_TRANSMIT_CRC_ERROR))
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - Transmit CRC Error waiting for DATA_TRANSMIT_FINISHED!\r\n")));
                fRetVal = FALSE;
                break;
            }
            if((vm_pSDIReg->SDIDSTA & DATA_TIME_OUT))
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:PollingTransmit() - Data timeout waiting for DATA_TRANSMIT_FINISHED!\r\n")));
                fRetVal = FALSE;
                break;
            }
        }
        vm_pSDIReg->SDIDSTA = DATA_TRANSMIT_FINISHED;
    }

    return fRetVal;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       CSDIOControllerBase::PollingReceive()

Description:    Receives the specified data from the SD card via polling.

Notes:          Some SDIO client drivers need to issue short (i.e. 2 byte)
                I/O requests to their hardware.  For these types of transfers,
                DMA will *NOT* work.

                This routine DOES handle the case when the supplied buffer is NOT 
                DWORD-aligned.

Returns:        Boolean indicating success.
-------------------------------------------------------------------*/
BOOL CSDIOControllerBase::PollingReceive(PSD_BUS_REQUEST pRequest, DWORD dwLen)
{
    BOOL fRetVal = TRUE;
    DEBUGCHK(m_dwNumBytesToTransfer <= MAXIMUM_DMA_TRANSFER_SIZE);
    PBYTE pBuff = pRequest->pBlockBuffer;
 
    SD_SET_PROC_PERMISSIONS_FROM_REQUEST( pRequest ) {               
        __try { 
            //----- 1. Read the data from the FIFO -----
            while(dwLen > 0)
            {
                if( !m_fCardInTheSlot )
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDIPollingReceive() - Card ejected!\r\n")));
                    fRetVal = FALSE;
                    break;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_TIME_OUT))
                {
                    DEBUGMSG(ZONE_ENABLE_ERROR, (TEXT("SDHCD:SDIPollingReceive() - Data receive time out!\r\n")));
                    fRetVal = FALSE;
                    break;
                }
                if((vm_pSDIReg->SDIFSTA & FIFO_FAIL_ERROR))
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDIPollingReceive() - FIFO Error!\r\n")));
                    fRetVal = FALSE;
                    break;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_RECEIVE_CRC_ERROR))
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDIPollingReceive() - Receive CRC Error!\r\n")));
                    fRetVal = FALSE;
                    break;
                }
                if((vm_pSDIReg->SDIDSTA & DATA_TIME_OUT))
                {
                    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SDIPollingReceive() - Data timeout!\r\n")));
                    fRetVal = FALSE;
                    break;
                }

                //----- 2. Does the FIFO have more data ready? -----
                if((vm_pSDIReg->SDIFSTA & FIFO_AVAIL_FOR_RX))
                {
                    *pBuff++ = vm_pSDIReg->SDIDAT8;
                    dwLen--;
                }
            }
        } __except (SDProcessException(GetExceptionInformation())) {
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("--- SDHC Driver: Exception caught in PollingTransmit\r\n")));
            fRetVal = FALSE;
        }
    } SD_RESTORE_PROC_PERMISSIONS();

    return fRetVal;   
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       CSDIOControllerBase::SetupPollingXfer()

Description:    Initializes the SDIO controller for polling I/O.

Notes:          Some SDIO client drivers need to issue short (i.e. 2 byte)
                I/O requests to their hardware.  For these types of transfers,
                DMA will *NOT* work.

Returns:        Boolean indicating success.
-------------------------------------------------------------------*/
BOOL CSDIOControllerBase::SetupPollingXfer(PSD_BUS_REQUEST pRequest)
{
    //----- 1. Check the parameters -----
    if(!pRequest)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SetupSDIXfer() - Invalid parameters!\r\n")));
        return FALSE;
    }

    //----- 2. Reset the FIFO -----
    vm_pSDIReg->SDIFSTA   |= FIFO_RESET;    
    vm_pSDIReg->SDIFSTA  &= FIFO_FAIL_ERROR;

    //----- 3. Setup the SDI controller according to the type of transfer -----
    switch(pRequest->TransferClass)
    {

    case SD_READ:
    	m_dwLastTypeOfTransfer = SD_READ;
        vm_pSDIReg->SDIBSIZE       = pRequest->BlockSize;
        vm_pSDIReg->SDIDCON  = RECEIVE_AFTER_COMMAND | TRANSFER_BLOCK_MODE | Get_SDI_Bus_Width() | DATA_TRANS_START | DATA_RECEIVE_START | pRequest->NumBlocks;
        break;
        

    case SD_WRITE:
    	m_dwLastTypeOfTransfer = SD_WRITE;
        vm_pSDIReg->SDIBSIZE   = pRequest->BlockSize;
        vm_pSDIReg->SDIDCON  = TRANSMIT_AFTER_RESPONSE | TRANSFER_BLOCK_MODE | Get_SDI_Bus_Width() | DATA_TRANS_START | DATA_TRANSMIT_START | pRequest->NumBlocks;
        break;

    case SD_COMMAND:
        //----- 4. SPECIAL CASE: The SD_CMD_STOP_TRANSMISSION command requires that BUSY_AFTER_COMMAND be setup. -----
        if((pRequest->CommandCode == SD_CMD_STOP_TRANSMISSION) && (m_dwLastTypeOfTransfer == SD_WRITE))
        {
            vm_pSDIReg->SDIBSIZE  = pRequest->BlockSize;
            vm_pSDIReg->SDIDCON = BUSY_AFTER_COMMAND | TRANSFER_BLOCK_MODE | DATA_TRANS_START | DATA_BUSY | pRequest->NumBlocks;
        }
        break;

    default:
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHCD:SetupPollingXfer(): Unrecognized transfer mode!\r\n")));
        return FALSE;
    }
    return TRUE;
}

DWORD CSDIOControllerBase::SD_CardDetectThread(CSDIOControllerBase *pController)
{
    return pController->CardDetectThread();
}

DWORD CSDIOControllerBase::SD_IOInterruptIstThread(CSDIOControllerBase *pController)
{
    return pController->IOInterruptIstThread();
}

DWORD CSDIOControllerBase::SD_TransferIstThread(CSDIOControllerBase *pController)
{
    return pController->TransferIstThread();
}

//------------------------------------------------------------------------------------------------------------------

SD_API_STATUS CSDIOControllerBase::SDHCDInitialize(PSDCARD_HC_CONTEXT pHCContext)
{
    // get our extension 
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->Initialize();
}

SD_API_STATUS CSDIOControllerBase::SDHCDDeinitialize(PSDCARD_HC_CONTEXT pHCContext)
{
    // get our extension 
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->Deinitialize();
}

BOOLEAN CSDIOControllerBase::SDHCDCancelIoHandler(PSDCARD_HC_CONTEXT pHCContext, DWORD dwSlot, PSD_BUS_REQUEST pRequest)
{
    // get our extension 
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->CancelIoHandler(dwSlot, pRequest);
}

SD_API_STATUS CSDIOControllerBase::SDHCDBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext, DWORD dwSlot, PSD_BUS_REQUEST pRequest)
{
    // get our extension 
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->BusRequestHandler(dwSlot, pRequest);
}

SD_API_STATUS CSDIOControllerBase::SDHCDSlotOptionHandler(PSDCARD_HC_CONTEXT pHCContext, DWORD dwSlot,
                                       SD_SLOT_OPTION_CODE Option, PVOID pData, ULONG OptionSize)
{
    // get our extension 
    CSDIOControllerBase *pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    return pController->SlotOptionHandler(dwSlot, Option, pData, OptionSize);
}

BOOL CSDIOControllerBase::InterpretCapabilities( LPCTSTR pszRegistryPath )
{
    BOOL fRetVal = TRUE;
    CReg regDevice; // encapsulated device key
    HKEY hKeyDevice = OpenDeviceKey(pszRegistryPath);
    if ( (hKeyDevice == NULL) || !regDevice.Open(hKeyDevice, NULL) ) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("CSDIOControllerBase::InterpretCapabilities: Failed to open device key\r\n")));
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }

    // read registry settings

    // read the SDIO SYSINTR value
    m_dwSDIOIrq = regDevice.ValueDW( SDIO_IRQ_TEXT, 0xffffffff );
    if( m_dwSDIOIrq == 0xffffffff )
    {
        // invalid SDIO IRQ value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("invalid SDIO IRQ value!\r\n")));
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }

    // convert the SDI hardware IRQ into a logical SYSINTR value
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &m_dwSDIOIrq, sizeof(DWORD), &m_dwSDIOSysIntr, sizeof(DWORD), NULL))
    {
        // invalid SDIO SYSINTR value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("invalid SDIO SYSINTR value!\r\n")));
        m_dwSDIOSysIntr = SYSINTR_UNDEFINED;
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    } 

	m_dwSDDetectIrq = regDevice.ValueDW( SD_DETECT_IRQ_TEXT, 0xffffffff );
    if( m_dwSDDetectIrq == 0xffffffff )
    {
        // invalid SDIO IRQ value!
        RETAILMSG(1, (TEXT("invalid SD detect IRQ value!\r\n")));
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }

    // convert the SDI hardware IRQ into a logical SYSINTR value
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &m_dwSDDetectIrq, sizeof(DWORD), &m_dwSDDetectSysIntr, sizeof(DWORD), NULL))
    {
        // invalid SDDetect SYSINTR value!
        RETAILMSG(1, (TEXT("invalid SD detect SYSINTR value!\r\n")));
        m_dwSDDetectSysIntr = SYSINTR_UNDEFINED;
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    } 
    // read the SDIO IST priority
    m_nSDIOIstThreadPriority = regDevice.ValueDW( SDIO_IST_PRIORITY_TEXT, 0xffffffff );

    // read the DMA channel number
    m_dwDMAChannel = regDevice.ValueDW( DMA_CHANNEL_TEXT, 0xffffffff );

    // read the DMA IRQ value
    m_dwDMAIrq = regDevice.ValueDW( DMA_IRQ_TEXT, 0xffffffff );
    if( m_dwDMAIrq == 0xffffffff )
    {
        // invalid DMA IRQ value!
        m_dwDMAChannel = 0xffffffff; // disable DMA
    }

    // convert the DMA hardware IRQ into a logical SYSINTR value.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &m_dwDMAIrq, sizeof(DWORD), &m_dwDMASysIntr, sizeof(DWORD), NULL))
    {
        // invalid DMA IRQ value!
        m_dwDMAChannel = 0xffffffff; // disable DMA
    } 

    // read the DMA IST priority
    m_DMAIstThreadPriority = regDevice.ValueDW( DMA_IST_PRIORITY_TEXT, 0xffffffff );


    // read the card detect thread polling timeout (default 100 ms)
    m_dwPollingTimeout = regDevice.ValueDW( POLLING_TIMEOUT_TEXT, 150 );

    // read the card detect thread priority
    m_nCardDetectIstThreadPriority = regDevice.ValueDW( CARD_DETECT_THREAD_PRIORITY_TEXT, 0xffffffff );

    // read the HandleBusyFinishOnCommand39
    m_fHandleBusyCheckOnCommand38 = regDevice.ValueDW( HANDLE_BUSY_FINISH_ON_COMMAND38_TEXT, 0 ) ? TRUE : FALSE;

    // read the DMA transfer timeout factor
    m_dwDMATransferTimeoutFactor = regDevice.ValueDW( DMA_TRANSFER_TIMEOUT_FACTOR_TEXT, 8 );

    // read the DMA transfer timeout constant
    m_dwDMATransferTimeoutConstant = regDevice.ValueDW( DMA_TRANSFER_TIMEOUT_CONSTANT_TEXT, 3000 );
 
    // call the custom (overrideable) routine for custom, hardware specific settings
    if( !CustomSetup( pszRegistryPath ) )
    {
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }
/*
    // set the slot capabilities
    SDHCDSetSlotCapabilities(m_pHCContext, SD_SLOT_SD_1BIT_CAPABLE | SD_SLOT_SD_4BIT_CAPABLE | 
                       SD_SLOT_SDIO_CAPABLE | SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK);

    // this platform has 3.3V tied directly to the slot  
    SDHCDSetVoltageWindowMask(m_pHCContext, (SD_VDD_WINDOW_3_2_TO_3_3 | SD_VDD_WINDOW_3_3_TO_3_4)); 

    // set on the low side 
    SDHCDSetDesiredSlotVoltage(m_pHCContext, SD_VDD_WINDOW_3_2_TO_3_3);     

    // the SD/MMC controller tops out at MAX_SDI_BUS_TRANSFER_SPEED
    SDHCDSetMaxClockRate(m_pHCContext, MAX_SDI_BUS_TRANSFER_SPEED);

    // power up settling time (estimated)
    SDHCDSetPowerUpDelay(m_pHCContext, 300); 
*/    
    // set the name
    SDHCDSetHCName(m_pHCContext, TEXT("SDH"));

    // set init handler
    SDHCDSetControllerInitHandler(m_pHCContext, CSDIOControllerBase::SDHCDInitialize);  
        
    // set deinit handler    
    SDHCDSetControllerDeinitHandler(m_pHCContext, CSDIOControllerBase::SDHCDDeinitialize);
        
    // set the Send packet handler
    SDHCDSetBusRequestHandler(m_pHCContext, CSDIOControllerBase::SDHCDBusRequestHandler);   
        
    // set the cancel I/O handler
    SDHCDSetCancelIOHandler(m_pHCContext, CSDIOControllerBase::SDHCDCancelIoHandler);   
        
    // set the slot option handler
    SDHCDSetSlotOptionHandler(m_pHCContext, CSDIOControllerBase::SDHCDSlotOptionHandler); 

FUNCTION_EXIT:
    if (hKeyDevice) RegCloseKey(hKeyDevice);
    return fRetVal;
}

PSDCARD_HC_CONTEXT CSDIOControllerBase::GetHostContext() const
{
    return m_pHCContext;
}

void CSDIOControllerBase::MMC_Hardware_PowerUp()
{
    vm_pCLKPWR->PCLKCON |= (1 << 5);
}

void CSDIOControllerBase::MMC_Hardware_PowerDown()
{
    vm_pCLKPWR->PCLKCON &= ~(1 << 5);
}

void CSDIOControllerBase::Stop_SDI_Hardware()
{ 
//////jayson    vm_pSDIReg->SDIDCON |= STOP_BY_FORCE;
}

void CSDIOControllerBase::Set_SDI_Bus_Width_1Bit()
{ 
    m_dwSDIBusWidth = 0;
}

void CSDIOControllerBase::Set_SDI_Bus_Width_4Bit()
{
    m_dwSDIBusWidth = WIDE_BUS_ENABLE;
}

DWORD CSDIOControllerBase::Get_SDI_Bus_Width()
{
    return m_dwSDIBusWidth;
}

void CSDIOControllerBase::Wait_80_SDI_Clock_Cycles()
{
    // delay 80 clock cycles... we may need a better way to do this
    Sleep(1); // sleep 1 ms
}

void CSDIOControllerBase::Start_SDI_Clock()
{
    vm_pSDIReg->SDICON |= CLOCK_ENABLE;  
    DEBUGMSG(SDHC_CLOCK_ZONE, (TEXT("SDHCD: Clock started...\r\n")));
}

void CSDIOControllerBase::Stop_SDI_Clock()
{ 
    vm_pSDIReg->SDICON &= ~CLOCK_ENABLE; 
    DEBUGMSG(SDHC_CLOCK_ZONE, (TEXT("SDHCD: Clock stopped...\r\n")));
}

BOOL CSDIOControllerBase::Is_SDI_Clock_Running()
{
    if( vm_pSDIReg->SDICON & CLOCK_ENABLE )
        return TRUE;
    else
        return FALSE;
}

void CSDIOControllerBase::Enable_SDIO_Interrupts()
{ 
    vm_pSDIReg->SDICON   |= SDIO_INTERRUPT_ENABLE;
    vm_pSDIReg->SDIIMSK  |= SDIO_HOST_IO_INT;
}

void CSDIOControllerBase::Disable_SDIO_Interrupts()
{
    vm_pSDIReg->SDICON   &= ~SDIO_INTERRUPT_ENABLE;
    vm_pSDIReg->SDIIMSK  &= ~SDIO_HOST_IO_INT;
}

BOOL CSDIOControllerBase::Is_SDIO_Interrupt_Enabled()
{
    if( vm_pSDIReg->SDICON & SDIO_INTERRUPT_ENABLE )
        return TRUE;
    else
        return FALSE;
}

void CSDIOControllerBase::Ack_SDIO_Interrupts()
{
    vm_pSDIReg->SDIDSTA = SDIO_INTERRUPT_DETECTED;
}

void CSDIOControllerBase::Enable_SDIO_DMA_Channel()
{
    switch( m_dwDMAChannel )
    {
    case 0:
        vm_pDMAreg->DMASKTRIG0 =  ENABLE_DMA_CHANNEL;
        break;
    case 1:
        vm_pDMAreg->DMASKTRIG1 =  ENABLE_DMA_CHANNEL;
        break;
    case 2:
        vm_pDMAreg->DMASKTRIG2 =  ENABLE_DMA_CHANNEL;
        break;
    case 3:
        vm_pDMAreg->DMASKTRIG3 =  ENABLE_DMA_CHANNEL;
        break;
    default:
        ASSERT(0); // Invalid DMA channel... we should never get here
    }
}

void CSDIOControllerBase::Disable_SDIO_DMA_Channel()
{
    switch( m_dwDMAChannel )
    {
    case 0:
        vm_pDMAreg->DMASKTRIG0 &= ~ENABLE_DMA_CHANNEL;
        break;
    case 1:
        vm_pDMAreg->DMASKTRIG1 &= ~ENABLE_DMA_CHANNEL;
        break;
    case 2:
        vm_pDMAreg->DMASKTRIG2 &= ~ENABLE_DMA_CHANNEL;
        break;
    case 3:
        vm_pDMAreg->DMASKTRIG3 &= ~ENABLE_DMA_CHANNEL;
        break;
    default:
        ASSERT(0); // Invalid DMA channel... we should never get here
    }
}

void CSDIOControllerBase::Stop_SDIO_DMA_Channel()
{
    switch( m_dwDMAChannel )
    {
    case 0:
        vm_pDMAreg->DMASKTRIG0 |= STOP_DMA_TRANSFER;
        break;
    case 1:
        vm_pDMAreg->DMASKTRIG1 |= STOP_DMA_TRANSFER;
        break;
    case 2:
        vm_pDMAreg->DMASKTRIG2 |= STOP_DMA_TRANSFER;
        break;
    case 3:
        vm_pDMAreg->DMASKTRIG3 |= STOP_DMA_TRANSFER;
        break;
    default:
        ASSERT(0); // Invalid DMA channel... we should never get here
    }
    vm_pSDIReg->SDIDCON  &= ~DMA_ENABLE;
}

void CSDIOControllerBase::OnPowerUp()
{
    //  Supply the clock to the SDI controller
    MMC_Hardware_PowerUp();

    RETAILMSG(1, (TEXT("SDHC@CSDIOControllerBase::OnPowerUP() \r\n")));	

    // Reconfigure the GPIO lines for SDI mode and enable the pullup resistor -----
    vm_pIOPreg->GPEUDP  &= 0xC003FF;
    vm_pIOPreg->GPECON = (vm_pIOPreg->GPECON & ~(0x3ffc00)) | 0x2AA800;

    // Reset the SD/SDI controller to some reasonable default values -----
    SetClockRate(SD_DEFAULT_CARD_ID_CLOCK_RATE);        // 100Khz

    vm_pSDIReg->SDICON     |= LITTLE_ENDIAN_BYTE_ORDER;    // Windows CE is always Little Endian.
    vm_pSDIReg->SDIFSTA     |= FIFO_RESET;                  // Reset the FIFO
    vm_pSDIReg->SDIBSIZE    = BYTES_PER_SECTOR; 
    vm_pSDIReg->SDIDTIMER   = MAX_DATABUSY_TIMEOUT;        // Data/busy timeout

    // reinitialize the hardware
    InitializeHardware( TRUE );

    // Start the SDI Clock
    Start_SDI_Clock();

    // Notify the SD Bus driver of the PowerUp event
    SDHCDPowerUpDown(m_pHCContext, TRUE, FALSE, 0);

    // simulate a card ejection/insertion
    m_bReinsertTheCard = TRUE;
    SetEvent( m_hCardInsertInterruptEvent );
}

void CSDIOControllerBase::OnPowerDown()
{
    // Notify the SD Bus driver of the PowerDown event
    SDHCDPowerUpDown(m_pHCContext, FALSE, FALSE, 0);

    // Stop the SDI Clock
    Stop_SDI_Clock();

    // Call DeinitializeHardware (By default, it does nothing, but OEM's may overwrite 
    // this function to provide hardware specific configuration changes on PowerDown event)
    DeinitializeHardware( TRUE );
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

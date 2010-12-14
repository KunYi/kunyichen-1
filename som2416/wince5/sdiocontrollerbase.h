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

#ifndef __SDIOCONTROLLER_H
#define __SDIOCONTROLLER_H

//#################################### Register Constants/Masks/etc. ######################################

// NOTE: the maximum SD speed allowed value of 25 MHz.  
#define MAX_SDI_BUS_TRANSFER_SPEED		25000000

#define BYTES_PER_SECTOR				512
#define SDI_CONTROLLER_FIFO_SIZE		64

//----- Register definitions for SDICON control register (global config register) -----
#define LITTLE_ENDIAN_BYTE_ORDER		0x00000000
#define SDMMC_RESET						0x00000100
#define SDCLK_TYPE_SD					0x00000000
#define SDCLK_TYPE_MMC					0x00000020
#define SDIO_INTERRUPT_ENABLE			0x00000008 
#define SDIO_READ_WAIT_ENABLE			0x00000004 
//#define RESET_FIFO						0x00000002 
#define CLOCK_ENABLE					0x00000001				


//----- Register definitions for SDICMDCON control register (command config register) -----
//		NOTE:  Lowest 8 bits are used for the actual command's opcode
#define SDIO_ABORT_COMMAND				0x00001000				// Works for CMD12 and CMD52
#define SDIO_COMMAND_WITH_DATA			0x00000800
#define LONG_RESPONSE					0x00000400				
#define WAIT_FOR_RESPONSE				0x00000200				
#define START_COMMAND					0x00000100				

#define COMMAND_START_BIT				0x00000040				// Commands are 6 bits, but the 7th bit must be set!
#define MAX_CMD_VALUE					0x0000007F				// In SD/MMC clock cycles
#define	NULL_CMD_ARGUMENT				0x00000000
															
//----- Register definitions for SDICMDSTA control register (command status register) -----
//		NOTE:  Lowest 8 bits indicate the command's response code
#define CRC_CHECK_FAILED				0x00001000				
#define COMMAND_SENT					0x00000800
#define COMMAND_TIMED_OUT				0x00000400				
#define RESPONSE_RECEIVED				0x00000200				
#define COMMAND_IN_PROGRESS				0x00000100				

#define SHORT_RESPONSE_LENGTH			0x00000006				// In bytes
#define LONG_RESPONSE_LENGTH			0x00000010				// In bytes


//----- Register definitions for SDIDTIMER control register (data/busy config register) -----
//		NOTES:	1) Valid range		= 0-8388607
//				2) Default value	= 0x2000 
#define NO_DATABUSY_TIMEOUT				0
#define MAX_DATABUSY_TIMEOUT			0x007FFFFF

//----- Register definitions for SDIDATCON control register (transfer config register) -----
//		NOTE:  Lowest 12 bits are used for the block number (don't care in stream mode)
#define BURST4_ENABLE					0x01000000
#define DATA_SIZE_WORD					0x00800000
#define DATA_SIZE_HWORD					0x00400000
#define DATA_SIZE_BYTE					0x00000000
#define SDIO_INTERRUPT_PERIOD_TYPE		0x00200000				// 0 = 2 cycles, 1 = more than 2 cycles
#define TRANSMIT_AFTER_RESPONSE			0x00100000
#define RECEIVE_AFTER_COMMAND			0x00080000
#define BUSY_AFTER_COMMAND				0x00040000
#define TRANSFER_BLOCK_MODE				0x00020000				// Clearing this bit enables stream mode
#define WIDE_BUS_ENABLE					0x00010000				
#define DMA_ENABLE						0x00008000				
//jayson #define STOP_BY_FORCE					0x00004000				
#define DATA_TRANS_START				0x00004000				

// Bits 12-13 are used to determine transfer mode:
#define DATA_TRANSMIT_START				0x00003000
#define DATA_RECEIVE_START				0x00002000
#define DATA_BUSY						0x00001000
#define DATA_READY						0x00000000


//----- Register definitions for SDIDATSTA control register (transfer status register) -----
#define NO_BUSY							0x00000800
#define READ_WAIT_REQUEST_OCCURED		0x00000400				
#define SDIO_INTERRUPT_DETECTED			0x00000200
//#define FIFO_FAIL_ERROR					0x00000100
#define DATA_TRANSMIT_CRC_ERROR			0x00000080
#define DATA_RECEIVE_CRC_ERROR			0x00000040
#define DATA_TIME_OUT					0x00000020
#define DATA_TRANSMIT_FINISHED			0x00000010
#define BUSY_CHECKS_FINISH				0x00000008
#define DATA_TRANSMIT_IN_PROGRESS		0x00000002
#define DATA_RECIEVE_IN_PROGRESS		0x00000001

//----- Register definitions for SDIFSTA control register (FIFO status register) -----
//		NOTE:  Lowest 7 bits indicate the number of data bytes in the FIFO
#define FIFO_RESET						0x00010000
#define FIFO_FAIL_ERROR					0x0000C000
#define FIFO_AVAIL_FOR_TX				0x00002000
#define FIFO_AVAIL_FOR_RX				0x00001000
#define TX_FIFO_HALF_FULL				0x00000800				// < 33 bytes
#define TX_FIFO_EMPTY					0x00000400				// = 0 bytes
#define RX_FIFO_LAST_DATA_READY			0x00000200
#define RX_FIFO_FULL					0x00000100				// = 64 bytes
#define RX_FIFO_HALF_FULL				0x00000080				// > 33 bytes


//----- Register definitions for SDIIMSK control register (interrupt mask register) -----
#define NOBUSY_INT						0x00040000
#define RESPONSE_CRC_ERROR_INT			0x00020000				
#define COMMAND_SENT_INT				0x00010000				
#define COMMAND_RESPONSE_TIMEOUT_INT	0x00008000				
#define COMMAND_RESPONSE_REC_INT		0x00004000				
#define READ_WAIT_REQUEST_INT			0x00002000				
#define SDIO_HOST_IO_INT				0x00001000				
#define FIFO_FAIL_ERROR_INT				0x00000800				
#define CRC_STATUS_ERROR_INT			0x00000400				
#define DATA_CRC_FAIL_INT				0x00000200				
#define DATA_TIMEOUT_INT				0x00000100				
#define DATA_COUNTER_ZERO_INT			0x00000080				
#define BUSY_CHECKS_COMPLETE_INT		0x00000040				
#define START_BIT_ERROR_INT				0x00000020				
#define TX_FIFO_HALF_FULL_INT			0x00000010				// < 33 bytes
#define TX_FIFO_EMPTY_INT				0x00000008				// = 0 bytes
#define RX_FIFO_LAST_DATA_READY_INT		0x00000004				
#define RX_FIFO_FULL_INT				0x00000002				// = 64 bytes
#define RX_FIFO_HALF_FULL_INT			0x00000001				// > 33 bytes

#define ERROR_INTERRUPTS_HANDLED		COMMAND_RESPONSE_TIMEOUT_INT | DATA_TIMEOUT_INT 


//###################################### Hardware Controller Structures ##################################
typedef enum {
    Idle				= -1,
    CommandSent			= 1,
    CommandComplete		= 2,
    ResponseWait		= 3,
    WriteDataTransfer	= 4,
    WriteDataTransferDone = 5,
    ProgramWait			= 6,
    WriteDataDone		= 7,
    ReadDataTransfer	= 8,
    ReadDataTransferDone = 9,
    ReadDataDone		= 10,

} SDHCDSTATE;

class CSDIOControllerBase
{
public:
    explicit CSDIOControllerBase( PSDCARD_HC_CONTEXT pHCContext );
    virtual ~CSDIOControllerBase();

    PSDCARD_HC_CONTEXT GetHostContext() const;

    virtual BOOL InterpretCapabilities( LPCTSTR pszRegistryPath );
    void OnPowerUp();
    void OnPowerDown();
    void PreDeinit();

protected:
    // override to customize for specific hardware
    virtual BOOL InitializeHardware( BOOL bOnPowerUp = FALSE ) = 0;
    virtual void DeinitializeHardware( BOOL bOnPowerDown = FALSE ) = 0;
    virtual BOOL CustomSetup( LPCTSTR pszRegistryPath ) = 0;
    virtual BOOL IsCardWriteProtected() = 0;
    virtual BOOL IsCardPresent() = 0;

protected:

    volatile S3C2450_SDI_REG    *vm_pSDIReg;                // pointer to the SDI special registers
    volatile S3C2450_IOPORT_REG *vm_pIOPreg;                // pointer to the GPIO control registers
    volatile S3C2450_CLKPWR_REG *vm_pCLKPWR;                // pointer to the clock control register
    volatile S3C2450_DMA_REG    *vm_pDMAreg;                // pointer to the DMA special registers

    DWORD                   m_dwSDIOIrq;                     // SDIO IRQ
    DWORD                   m_dwSDIOSysIntr;                 // SDIO SysIntr

    DWORD                   m_dwSDDetectIrq;                     // SDIO IRQ
    DWORD                   m_dwSDDetectSysIntr;                 // SDIO SysIntr
    PBYTE                   m_pDMABuffer;                    // pointer to buffers used for DMA transfers
    PHYSICAL_ADDRESS        m_pDMABufferPhys;                // physical address of the SMA buffer
    DWORD                   m_dwDMAChannel;                  // DMA channel to use for data transfers
    DWORD                   m_dwDMAIrq;                      // DMA IRQ
    DWORD                   m_dwDMASysIntr;                  // DMA SysIntr

    DWORD                   m_dwPollingTimeout;              // card detect thread polling timeout

    char                    m_chCardDetectGPIO;              // GPIO used for card detection
    DWORD                   m_dwCardDetectMask;
    DWORD                   m_dwCardDetectFlag;
    DWORD                   m_dwCardDetectControlMask;
    DWORD                   m_dwCardDetectControlFlag;
    DWORD                   m_dwCardDetectPullupMask;
    DWORD                   m_dwCardDetectPullupFlag;

    char                    m_chCardReadWriteGPIO;           // GPIO used for card read/write detection
    DWORD                   m_dwCardReadWriteMask;
    DWORD                   m_dwCardReadWriteFlag;
    DWORD                   m_dwCardReadWriteControlMask;
    DWORD                   m_dwCardReadWriteControlFlag;
    DWORD                   m_dwCardReadWritePullupMask;
    DWORD                   m_dwCardReadWritePullupFlag;

    PSDCARD_HC_CONTEXT      m_pHCContext;                    // the host controller context
    HANDLE                  m_hResponseReceivedEvent;		 // Used to post command response info asynchronously

    int                     m_nCardDetectIstThreadPriority;  // controller IST thread priority
    HANDLE                  m_hCardInsertInterruptEvent;     // card insert/remove interrupt event
    HANDLE                  m_hCardInsertInterruptThread;    // card insert/remove interrupt event

    int                     m_nSDIOIstThreadPriority;        // SDIO IST thread priority
    HANDLE                  m_hSDIOInterruptEvent;           // SDIO Interrupt event
    HANDLE                  m_hSDIOInterruptThread;          // SDIO Interrupt Thread Event

    int                     m_nControllerIstThreadPriority;  // controller IST thread priority
    HANDLE                  m_hControllerInterruptEvent;     // controller interrupt event
    HANDLE                  m_hControllerInterruptThread;    // controller interrupt thread

    int                     m_DMAIstThreadPriority;		     // DMA IST thread priority
    HANDLE                  m_hDMAInterruptEvent;			 // DMA interrupt event
    HANDLE                  m_hDMAInterruptThread;		     // DMA interrupt thread   

    BOOL                    m_bReinsertTheCard;              // Indicates if a card insertion should be simulated now
    BOOL                    m_bUseDMAForTransfer;			 // Indicates whether DMA is used for I/O requests
    BOOL                    m_bUseDMABurst;					 // Indicates used DMA burst
    char                    m_DATATransferSize;		 		 // Indicates whether DMA transfer size (BYTE, WORD, DWORD)
    BOOL                    m_bDriverShutdown;               // controller shutdown
    CRITICAL_SECTION        m_ControllerCriticalSection;     // controller critical section 
    BOOL                    m_bDevicePresent;                // indicates if device is present in the slot											  
    WCHAR                   m_rgchRegPath[256];              // reg path  
    SDHCDSTATE              m_CurrentState;                  // current transfer state
    BOOL                    m_bSendInitClocks;               // indicates if this is the first command sent
    DWORD			        m_dwLastTypeOfTransfer;          // inidcates the last type of data transfer initiated

    DWORD			        m_dwNumBytesToTransfer;          // # of bytes that still need to be transferred
    DWORD			        m_dwNumBytesUnaligned;           // # of bytes from a DWORD-aligned address
    DWORD			        m_dwNumBytesExtra;               // # of extra bytes in buffer that aren't a multiple of sizeof(DWORD)

    DWORD			        m_dwSDIBusWidth;                 // SD data transfer mode (1 bit or 4 bit) flag
    DWORD                   m_dwClockRate;                   // current clock rate

    BOOL                    m_fCardInTheSlot;				 // TRUE - a card is inserted in the slot, FALSE otherwise
    BOOL                    m_fHandleBusyCheckOnCommand38;
    DWORD                   m_dwDMATransferTimeoutFactor;
    DWORD                   m_dwDMATransferTimeoutConstant;

    SD_API_STATUS	SendCommand(UINT16 Cmd, UINT32 Arg, UINT16 respType, BOOL bDataTransfer);
    SD_API_STATUS   GetCommandResponse(PSD_BUS_REQUEST pRequest);
    DWORD           SetClockRate(DWORD dwClockRate);
    BOOL			IsCardBusy(UINT16 inData);

    BOOL			SetupDmaXfer(PSD_BUS_REQUEST pRequest);

    BOOL			SetupPollingXfer(PSD_BUS_REQUEST pRequest);
    BOOL			PollingTransmit(PSD_BUS_REQUEST pRequest, DWORD dwLen);
    BOOL			PollingReceive(PSD_BUS_REQUEST pRequest, DWORD dwLen);

    inline void     MMC_Hardware_PowerUp();
    inline void     MMC_Hardware_PowerDown();
    inline void     Stop_SDI_Hardware();
    inline void     Set_SDI_Bus_Width_1Bit();
    inline void     Set_SDI_Bus_Width_4Bit();
    inline DWORD    Get_SDI_Bus_Width();
    inline void     Wait_80_SDI_Clock_Cycles();
    inline void     Start_SDI_Clock();
    inline void     Stop_SDI_Clock();
    inline BOOL     Is_SDI_Clock_Running();
    inline void     Enable_SDIO_Interrupts();
    inline void     Disable_SDIO_Interrupts();
    inline BOOL     Is_SDIO_Interrupt_Enabled();
    inline void     Ack_SDIO_Interrupts();
    inline void     Enable_SDIO_DMA_Channel();
    inline void     Disable_SDIO_DMA_Channel();
    inline void     Stop_SDIO_DMA_Channel();

    // thread routines
    virtual DWORD   CardDetectThread();
    virtual DWORD   IOInterruptIstThread();
    virtual DWORD   TransferIstThread();

    // implementation of the callbacks for the SD Bus driver
    SD_API_STATUS	Deinitialize();
    SD_API_STATUS	Initialize();
    BOOLEAN			CancelIoHandler(DWORD dwSlot, PSD_BUS_REQUEST pRequest);
    SD_API_STATUS	BusRequestHandler(DWORD dwSlot, PSD_BUS_REQUEST pRequest);
    SD_API_STATUS	SlotOptionHandler(DWORD dwSlot, SD_SLOT_OPTION_CODE Option, PVOID pData, ULONG OptionSize);

    // thread start routines
    static DWORD            SD_CardDetectThread(CSDIOControllerBase *pController);
    static DWORD            SD_IOInterruptIstThread(CSDIOControllerBase *pController);
    static DWORD            SD_TransferIstThread(CSDIOControllerBase *pController);

    // SD Bus driver callback functions
    static SD_API_STATUS	SDHCDDeinitialize(PSDCARD_HC_CONTEXT pHCContext);
    static SD_API_STATUS	SDHCDInitialize(PSDCARD_HC_CONTEXT pHCContext);
    static BOOLEAN			SDHCDCancelIoHandler(PSDCARD_HC_CONTEXT pHCContext, DWORD dwSlot, PSD_BUS_REQUEST pRequest);
    static SD_API_STATUS	SDHCDBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext, DWORD dwSlot, PSD_BUS_REQUEST pRequest);
    static SD_API_STATUS	SDHCDSlotOptionHandler(PSDCARD_HC_CONTEXT pHCContext, DWORD dwSlot, 
									    SD_SLOT_OPTION_CODE Option, PVOID pData, ULONG OptionSize);
};

typedef CSDIOControllerBase *PCSDIOControllerBase;

#define GET_PCONTROLLER_FROM_HCD(pHCDContext) \
    GetExtensionFromHCDContext(PCSDIOControllerBase, pHCDContext)

CSDIOControllerBase *CreateSDIOController( PSDCARD_HC_CONTEXT pHCContext );

#define SDHC_INTERRUPT_ZONE    SDCARD_ZONE_0
#define SDHC_SEND_ZONE         SDCARD_ZONE_1
#define SDHC_RESPONSE_ZONE     SDCARD_ZONE_2
#define SDHC_RECEIVE_ZONE      SDCARD_ZONE_3
#define SDHC_CLOCK_ZONE        SDCARD_ZONE_4
#define SDHC_TRANSMIT_ZONE     SDCARD_ZONE_5

#define SDHC_INTERRUPT_ZONE_ON ZONE_ENABLE_0
#define SDHC_SEND_ZONE_ON      ZONE_ENABLE_1
#define SDHC_RESPONSE_ZONE_ON  ZONE_ENABLE_2
#define SDHC_RECEIVE_ZONE_ON   ZONE_ENABLE_3
#define SDHC_CLOCK_ZONE_ON     ZONE_ENABLE_4
#define SDHC_TRANSMIT_ZONE_ON  ZONE_ENABLE_5

#endif // __SDIOCONTROLLER_H

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

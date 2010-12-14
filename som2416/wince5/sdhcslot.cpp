//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#if (BSP_TYPE == BSP_SMDK2443)
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

#elif (BSP_TYPE == BSP_SMDK2450)
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
#endif

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

#if (BSP_TYPE == BSP_SMDK2443)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <s3c2450.h>

#elif (BSP_TYPE == BSP_SMDK2450)
#endif
#include "SDHC.h"
#include "SDHCSlot.h"
#include <bsp.h>

// 08.04.03 by KYS
// Below variable and macro lines are needed for working as a mass storage device
static volatile BSP_ARGS *v_gBspArgs;
#define CARD_INSERTED 1
#define CARD_REMOVED 2

#if (BSP_TYPE == BSP_SMDK2443)
//extern 	S3C2450_IOPORT_REG *pIOPreg;

#elif (BSP_TYPE == BSP_SMDK2450)
#endif

// Macros
#define DX_D1_OR_D2(cps)            ((cps) == D1 || (cps) == D2)
#define SETFNAME()  LPCTSTR pszFname = _T(__FUNCTION__) _T(":")

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
//#define _FB_ON_			// for Feedback clock on

// 2007.06.25 D.Baek
// Define the global variable to check the initial state of card slot. When the driver is loading at boot,
// Check whether the card is inserted or not. If inserted, the process of insertion is executed automatically.
DWORD g_initialInsertion = 1;
#endif

#ifdef DEBUG
// dump the current request info to the debugger
static 
VOID 
DumpRequest(
		PSD_BUS_REQUEST pRequest,
		DWORD dwZone
		)
{   
	PREFAST_DEBUGCHK(pRequest);

	if (dwZone) {
		DEBUGMSG(1, (TEXT("DumpCurrentRequest: 0x%08X\n"), pRequest)); 
		DEBUGMSG(1, (TEXT("\t Command: %d\n"),  pRequest->CommandCode));
		DEBUGMSG(1, (TEXT("\t Argument: 0x%08x\n"),  pRequest->CommandArgument));
		DEBUGMSG(1, (TEXT("\t ResponseType: %d\n"),  pRequest->CommandResponse.ResponseType)); 
		DEBUGMSG(1, (TEXT("\t NumBlocks: %d\n"),  pRequest->NumBlocks)); 
		DEBUGMSG(1, (TEXT("\t BlockSize: %d\n"),  pRequest->BlockSize)); 
		DEBUGMSG(1, (TEXT("\t HCParam: %d\n"),    pRequest->HCParam)); 
	}
}
#else
#define DumpRequest(ptr, dw)
#endif



CSDHCSlotBase::CSDHCSlotBase(
		)
{
	m_pregDevice = NULL;
	m_dwSlot = 0;
	m_pbRegisters = NULL;
	m_pHCDContext = NULL;
	m_dwSysIntr = 0;
	m_hBusAccess = NULL;
	m_interfaceType = InterfaceTypeUndefined;
	m_dwBusNumber = 0;  

	m_dwVddWindows = 0;
	m_dwMaxClockRate = 0;
	m_dwTimeoutControl = 0;
	m_dwMaxBlockLen = 0;

	m_pbDmaBuffer = NULL;
	m_paDmaBuffer = 0;
	m_wRegClockControl = 0; 
	m_wIntSignals = 0;
	m_cpsCurrent = D0;
	m_cpsAtPowerDown = D0;

	m_dwDefaultWakeupControl = 0;
	m_bWakeupControl = 0;

#ifdef DEBUG
	m_dwReadyInts = 0;
#endif
	m_fCommandCompleteOccurred = FALSE;

	m_fSleepsWithPower = FALSE;
	m_fPowerUpDisabledInts = FALSE;
	m_fIsPowerManaged = FALSE;
	m_fSDIOInterruptsEnabled = FALSE;
	m_fCardPresent = FALSE;
	m_fAutoCMD12Success = FALSE;
	m_fCheckSlot = TRUE;
	m_fCanWakeOnSDIOInterrupts = FALSE;
	m_f4BitMode = FALSE;
	m_fFakeCardRemoval = FALSE;

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
	m_pCurrentRequest = NULL;
	m_fCurrentRequestFastPath = FALSE;

	m_dwPollingModeSize = NUM_BYTE_FOR_POLLING_MODE ;
#endif
}


CSDHCSlotBase::~CSDHCSlotBase(
		)
{
	DEBUGCHK(m_pbDmaBuffer == NULL);
}


BOOL
CSDHCSlotBase::Init(
		DWORD               dwSlot,
		volatile BYTE      *pbRegisters,
		PSDCARD_HC_CONTEXT  pHCDContext,
		DWORD               dwSysIntr,
		HANDLE              hBusAccess,
		INTERFACE_TYPE      interfaceType, 
		DWORD               dwBusNumber,
		CReg               *pregDevice
		)
{
	BOOL fRet = TRUE;

	DEBUGCHK(dwSlot < SDHC_MAX_SLOTS);
	DEBUGCHK(pbRegisters);
	DEBUGCHK(pHCDContext);
	DEBUGCHK(hBusAccess);
	PREFAST_DEBUGCHK(pregDevice && pregDevice->IsOK());

	m_dwSlot = dwSlot;
	m_pbRegisters = pbRegisters;
	m_pHCDContext = pHCDContext;
	m_dwSysIntr = dwSysIntr;
	m_hBusAccess = hBusAccess;
	m_interfaceType = interfaceType;
	m_dwBusNumber = dwBusNumber;
	m_pregDevice = pregDevice;

	fRet = SoftwareReset(SOFT_RESET_ALL);
	if (fRet) { 
		Sleep(10); // Allow time for card to power down after a device reset

		SSDHC_CAPABILITIES caps = GetCapabilities();

		DEBUGMSG(SDCARD_ZONE_INIT && caps.bits.DMA,
				(_T("SDHC Will use DMA for slot %u\n"), m_dwSlot));

		m_dwVddWindows = DetermineVddWindows();
		m_dwMaxClockRate = DetermineMaxClockRate();
		m_dwMaxBlockLen = DetermineMaxBlockLen();
		m_dwTimeoutControl = DetermineTimeoutControl();
		m_dwDefaultWakeupControl = DetermineWakeupSources();
		m_fCanWakeOnSDIOInterrupts = m_pregDevice->ValueDW(SDHC_CAN_WAKE_ON_INT_KEY);
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
		m_dwPollingModeSize = m_pregDevice->ValueDW(SDHC_POLLINGMODE_SIZE, NUM_BYTE_FOR_POLLING_MODE);
#endif

		Validate();

		DumpRegisters();
	}

	// 08.04.03 by KYS
	// Below code lines are needed for working as a mass storage device
	v_gBspArgs = (volatile BSP_ARGS *)VirtualAlloc(0, sizeof(BSP_ARGS), MEM_RESERVE, PAGE_NOACCESS);
	if (!v_gBspArgs) {
		RETAILMSG(1, (TEXT("[HSMMC1] Common Argument Area  VirtualAlloc failed!\r\n")));
		return FALSE; 
	}
	if (!VirtualCopy((PVOID)v_gBspArgs, (PVOID)(IMAGE_SHARE_ARGS_UA_START), sizeof(BSP_ARGS), PAGE_READWRITE | PAGE_NOCACHE )) {
		RETAILMSG(1, (TEXT("[HSMMC1] BSP Common Argument Are VirtualCopy failed!\r\n")));
		return FALSE;
	}
	v_gBspArgs->g_SDCardState = CARD_REMOVED;//initialize
	v_gBspArgs->g_SDCardDetectEvent = CreateEvent(NULL, FALSE, FALSE,NULL);

	return fRet;
}


SD_API_STATUS
CSDHCSlotBase::Start(
		)
{
	Validate();

	SD_API_STATUS status;

	if (SoftwareReset(SOFT_RESET_ALL)) {
		// timeout control
#if (BSP_TYPE == BSP_SMDK2443)
		WriteByte(SDHC_TIMEOUT_CONTROL, (BYTE) m_dwTimeoutControl);

#elif (BSP_TYPE == BSP_SMDK2450)
		WriteByte(SDHC_TIMEOUT_CONTROL, (BYTE)0xE);
#endif
		// Enable error interrupt status and signals for all but the vendor
		// errors. This allows any normal error to generate an interrupt.
		WriteWord(SDHC_ERROR_INT_STATUS_ENABLE, ~0 & ~ERR_INT_STATUS_VENDOR);
		WriteWord(SDHC_ERROR_INT_SIGNAL_ENABLE, ~0 & ~ERR_INT_STATUS_VENDOR);

		// Enable all interrupt signals. During execution, we will enable 
		// and disable interrupt statuses as desired.
#if (BSP_TYPE == BSP_SMDK2443)
		WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE, 0xFFFF);
		WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, 
				NORMAL_INT_ENABLE_CARD_INSERTION | NORMAL_INT_ENABLE_CARD_REMOVAL);

#elif (BSP_TYPE == BSP_SMDK2450)
		WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE, 0x1F7);
		//WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, 
		//    NORMAL_INT_ENABLE_CARD_INSERTION | NORMAL_INT_ENABLE_CARD_REMOVAL);
		WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, 0x1F7);
#endif
		status = SD_API_STATUS_SUCCESS;
	}
	else {
		status = SD_API_STATUS_DEVICE_NOT_RESPONDING;
	}
	return status;
}


SD_API_STATUS
CSDHCSlotBase::Stop(
		)
{    
	if (m_fCardPresent) {
		// Remove device
		HandleRemoval(FALSE);
	}

	SoftwareReset(SOFT_RESET_ALL);

	// 08.04.03 by KYS
	// Below code lines are needed for working as a mass storage device
	if(NULL != v_gBspArgs->g_SDCardDetectEvent) 
	{
		CloseHandle(v_gBspArgs->g_SDCardDetectEvent);
		v_gBspArgs->g_SDCardDetectEvent = NULL;
	}

	return SD_API_STATUS_SUCCESS;
}


SD_API_STATUS 
CSDHCSlotBase::GetSlotInfo(
		PSDCARD_HC_SLOT_INFO pSlotInfo
		)
{
	PREFAST_DEBUGCHK(pSlotInfo);
	DEBUGCHK(m_pregDevice->IsOK());

	// set the slot capabilities
#if (BSP_TYPE == BSP_SMDK2443)
#ifdef _MMC_SPEC_42_
	/*************************************************************************/
	/****** Date : 07.05.14                                         	******/
	/****** Developer : HS.JANG											******/
	/****** Description : to support SD SPEC20 card. Response7 is added	******/
	/****** 			  for supporting SD SPEC20 card					******/
	/*************************************************************************/
	SDHCDSetSlotCapabilities(pSlotInfo, SD_SLOT_SD_4BIT_CAPABLE | 
			SD_SLOT_SD_1BIT_CAPABLE | 
			SD_SLOT_SDIO_CAPABLE    |
			SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK |
			SD_SLOT_HIGH_SPEED_CAPABLE );       
	/*************************************************************************/
#else
	SDHCDSetSlotCapabilities(pSlotInfo, SD_SLOT_SD_4BIT_CAPABLE | 
			SD_SLOT_SD_1BIT_CAPABLE | 
			SD_SLOT_SDIO_CAPABLE    |
			SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK );

#endif

#elif (BSP_TYPE == BSP_SMDK2450)
	DWORD dwCap = SD_SLOT_SD_4BIT_CAPABLE | 
		SD_SLOT_SD_1BIT_CAPABLE | 
		//SD_SLOT_SD_8BIT_CAPABLE | // CH0 does not support DAT 8 Bit.
		SD_SLOT_SDIO_CAPABLE    |
		SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK;
	if (GetCapabilities().bits.HighSpeed)
		dwCap |= SD_SLOT_HIGH_SPEED_CAPABLE;

	SDHCDSetSlotCapabilities(pSlotInfo,dwCap );
#endif

	SDHCDSetVoltageWindowMask(pSlotInfo, m_dwVddWindows); 

	// Set optimal voltage
	SDHCDSetDesiredSlotVoltage(pSlotInfo, GetDesiredVddWindow());     

	// Controller may be able to clock at higher than the max SD rate,
	// but we should only report the highest rate in the range.
	DWORD dwMaxClockRateInSDRange = SD_FULL_SPEED_RATE;
	SetClockRate(&dwMaxClockRateInSDRange);
	SDHCDSetMaxClockRate(pSlotInfo, dwMaxClockRateInSDRange);

	// Set power up delay. We handle this in SetVoltage().
	SDHCDSetPowerUpDelay(pSlotInfo, 1);

	return SD_API_STATUS_SUCCESS;
}


SD_API_STATUS
CSDHCSlotBase::SlotOptionHandler(
		SD_SLOT_OPTION_CODE   sdOption, 
		PVOID                 pData,
		DWORD                 cbData
		)
{
	SD_API_STATUS status = SD_API_STATUS_SUCCESS;
	RETAILMSG(0,(TEXT("CSDHCSlotBase::SlotOptionHandle\nr")));
	switch (sdOption) {
		case SDHCDSetSlotPower: {
									DEBUGCHK(cbData == sizeof(DWORD));
									PDWORD pdwVddSetting = (PDWORD) pData;
									SetVoltage(*pdwVddSetting);
									break;
								}

		case SDHCDSetSlotInterface: {
										DEBUGCHK(cbData == sizeof(SD_CARD_INTERFACE));
										PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;

										DEBUGMSG(SDCARD_ZONE_INFO, 
												(TEXT("CSDHCSlotBase::SlotOptionHandler: Clock Setting: %d\n"), 
												 pInterface->ClockRate));

#if (BSP_TYPE == BSP_SMDK2443)
										SetInterface(pInterface);

#elif (BSP_TYPE == BSP_SMDK2450)
										SD_CARD_INTERFACE_EX sdCardInterfaceEx;
										memset(&sdCardInterfaceEx,0, sizeof(sdCardInterfaceEx));
										sdCardInterfaceEx.InterfaceModeEx.bit.sd4Bit = (pInterface->InterfaceMode == SD_INTERFACE_SD_4BIT? 1: 0);
										/* 07.11.20 by KYS for HSMMC8BIT*/
										sdCardInterfaceEx.InterfaceModeEx.bit.hsmmc8Bit = (((pInterface->InterfaceMode) == SD_INTERFACE_MMC_8BIT)? 1:0);

										sdCardInterfaceEx.ClockRate = pInterface->ClockRate;
										sdCardInterfaceEx.InterfaceModeEx.bit.sdWriteProtected = (pInterface->WriteProtected?1:0);
										SetInterface(&sdCardInterfaceEx);
										// Update the argument back.
										if (sdCardInterfaceEx.InterfaceModeEx.bit.hsmmc8Bit != 0) {
											pInterface->InterfaceMode = SD_INTERFACE_MMC_8BIT;
										} else {
											pInterface->InterfaceMode = (sdCardInterfaceEx.InterfaceModeEx.bit.sd4Bit!=0?SD_INTERFACE_SD_4BIT:SD_INTERFACE_SD_MMC_1BIT);
										}
										pInterface->ClockRate =  sdCardInterfaceEx.ClockRate;
										pInterface->WriteProtected = (sdCardInterfaceEx.InterfaceModeEx.bit.sdWriteProtected!=0?TRUE:FALSE);
#endif
										break;
									}

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
		case SDHCDSetSlotInterfaceEx: {
										  DEBUGCHK(cbData == sizeof(SD_CARD_INTERFACE_EX));
										  PSD_CARD_INTERFACE_EX pInterface = (PSD_CARD_INTERFACE_EX) pData;

										  DEBUGMSG(SDCARD_ZONE_INFO, 
												  (TEXT("CSDHCSlotBase::SlotOptionHandler: Clock Setting: %d\n"), 
												   pInterface->ClockRate));

										  SetInterface((PSD_CARD_INTERFACE_EX)pInterface);
										  break;
									  }
#endif

		case SDHCDEnableSDIOInterrupts:
		case SDHCDAckSDIOInterrupt:
									  EnableSDIOInterrupts(TRUE);
									  break;

		case SDHCDDisableSDIOInterrupts:
									  EnableSDIOInterrupts(FALSE);
									  break;

		case SDHCDGetWriteProtectStatus: {
											 DEBUGCHK(cbData == sizeof(SD_CARD_INTERFACE));
											 PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;
											 pInterface->WriteProtected = IsWriteProtected();
											 RETAILMSG(0,(TEXT("Write Protected 0x%x \n"),pInterface->WriteProtected ));
											 break;
										 }

		case SDHCDQueryBlockCapability: {
											DEBUGCHK(cbData == sizeof(SD_HOST_BLOCK_CAPABILITY));
											PSD_HOST_BLOCK_CAPABILITY pBlockCaps = 
												(PSD_HOST_BLOCK_CAPABILITY)pData;

											DEBUGMSG(SDCARD_ZONE_INFO, 
													(TEXT("CSDHCSlotBase::SlotOptionHandler: Read Block Length: %d , Read Blocks: %d\n"), 
													 pBlockCaps->ReadBlockSize, pBlockCaps->ReadBlocks));
											DEBUGMSG(SDCARD_ZONE_INFO, 
													(TEXT("CSDHCSlotBase::SlotOptionHandler: Write Block Length: %d , Write Blocks: %d\n"), 
													 pBlockCaps->WriteBlockSize, pBlockCaps->WriteBlocks));

											pBlockCaps->ReadBlockSize = max(pBlockCaps->ReadBlockSize, SDHC_MIN_BLOCK_LENGTH);
											pBlockCaps->ReadBlockSize = min(pBlockCaps->ReadBlockSize, (USHORT) m_dwMaxBlockLen);

											pBlockCaps->WriteBlockSize = max(pBlockCaps->WriteBlockSize, SDHC_MIN_BLOCK_LENGTH);
											pBlockCaps->WriteBlockSize = min(pBlockCaps->WriteBlockSize, (USHORT) m_dwMaxBlockLen);
											break;
										}

		case SDHCDGetSlotPowerState: {
										 DEBUGCHK(cbData == sizeof(CEDEVICE_POWER_STATE));
										 PCEDEVICE_POWER_STATE pcps = (PCEDEVICE_POWER_STATE) pData;
										 *pcps = GetPowerState();
										 break;
									 }

		case SDHCDSetSlotPowerState: {
										 DEBUGCHK(cbData == sizeof(CEDEVICE_POWER_STATE));
										 PCEDEVICE_POWER_STATE pcps = (PCEDEVICE_POWER_STATE) pData;
										 SetPowerState(*pcps);
										 break;
									 }

		case SDHCDWakeOnSDIOInterrupts: {
											DEBUGCHK(cbData == sizeof(BOOL));
											PBOOL pfWake = (PBOOL) pData;

											if (m_fCanWakeOnSDIOInterrupts) {
												DWORD dwWakeupControl = m_dwDefaultWakeupControl;

												if (*pfWake) {
													dwWakeupControl |= WAKEUP_INTERRUPT;
												}

												m_bWakeupControl = (BYTE) dwWakeupControl;
											}
											else {
												status = SD_API_STATUS_UNSUCCESSFUL;
											}
											break;
										}

		case SDHCDGetSlotInfo: {
								   DEBUGCHK(cbData == sizeof(SDCARD_HC_SLOT_INFO));
								   PSDCARD_HC_SLOT_INFO pSlotInfo = (PSDCARD_HC_SLOT_INFO) pData;
								   status = GetSlotInfo(pSlotInfo);
								   break;
							   }

		default:
							   status = SD_API_STATUS_INVALID_PARAMETER;
	}

	return status;
}


DWORD
CSDHCSlotBase::DetermineMaxClockRate(
		)
{
	DEBUGCHK(m_pregDevice->IsOK());

	// We allow the registry to override what is in the capabilities register.
	DWORD dwMaxClockRate = m_pregDevice->ValueDW(SDHC_FREQUENCY_KEY);
	if (dwMaxClockRate == 0) {
		SSDHC_CAPABILITIES caps = GetCapabilities();

		dwMaxClockRate = caps.bits.ClkFreq * 1000000;
		if (dwMaxClockRate == 0) {
			// No clock frequency specified. Use the highest possible that
			// could have been specified so that a working clock divisor 
			// will be chosen.
			dwMaxClockRate = SDHC_MAX_CLOCK_FREQUENCY;
			DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC: No base clock frequency specified\n")));
			DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("SDHC: Using default frequency of %u\n"), 
						dwMaxClockRate));
		}
	}

	RETAILMSG(0,(TEXT("dwMaxClockRate = %d\n"),dwMaxClockRate));
	return dwMaxClockRate;
}


DWORD
CSDHCSlotBase::DetermineMaxBlockLen(
		)
{
	static const USHORT sc_rgusBlockLen[] = { 
		SDHC_CAPABILITIES_MAX_BLOCK_LENGTH_0,
		SDHC_CAPABILITIES_MAX_BLOCK_LENGTH_1,
		SDHC_CAPABILITIES_MAX_BLOCK_LENGTH_2
	};

	SSDHC_CAPABILITIES caps = GetCapabilities();

	// Determine the maximum block length
	DEBUGCHK(caps.bits.MaxBlockLen < dim(sc_rgusBlockLen));
	DWORD dwMaxBlockLen = sc_rgusBlockLen[caps.bits.MaxBlockLen];
	DEBUGCHK(dwMaxBlockLen <= SDHC_MAX_BLOCK_LENGTH);
	return dwMaxBlockLen;
}


DWORD
CSDHCSlotBase::DetermineTimeoutControl(
		)
{
	// We try to come as close to the desired timeout without going below it.

	DEBUGCHK(m_pregDevice->IsOK());

	SSDHC_CAPABILITIES caps = GetCapabilities();

	// Determine the DAT line timeout divisor.
	// We allow the registry to override what is in the capabilities register.
	DWORD dwTimeoutClock = m_pregDevice->ValueDW(SDHC_TIMEOUT_FREQUENCY_KEY);
	if (dwTimeoutClock == 0) {
		dwTimeoutClock = caps.bits.TOFreq * 1000;

		if (dwTimeoutClock == 0) {
			dwTimeoutClock = SDHC_MAX_CLOCK_FREQUENCY;
			DEBUGMSG(SDCARD_ZONE_ERROR, 
					(_T("SDHC: No timeout frequency specified. Using default of %u\n"), 
					 dwTimeoutClock));
		}
		else if (caps.bits.TimeoutUnit == 1) {
			// listing is in MHz, not KHz
			dwTimeoutClock *= 1000;
		}
	}

	DEBUGCHK(dwTimeoutClock != 0);

	DWORD dwTimeoutInMS = m_pregDevice->ValueDW(SDHC_TIMEOUT_KEY, 
			SDHC_DEFAULT_TIMEOUT);

	DOUBLE dTimeoutControl = SdhcTimeoutSecondsToControl(dwTimeoutClock, 
			dwTimeoutInMS / 1000.0);
	DWORD dwTimeoutControl;

	if (dTimeoutControl < 0) {
		dwTimeoutControl = 0;
	}
	else {
		dTimeoutControl = ceil(dTimeoutControl);
		dwTimeoutControl= (DWORD) dTimeoutControl;
	}

	dwTimeoutControl = min(dwTimeoutControl, SDHC_TIMEOUT_CONTROL_MAX);

#ifdef DEBUG
	{        
		TCHAR szTimeout[4];
		DOUBLE dActualTimeout = SdhcTimeoutControlToSeconds(dwTimeoutClock, 
				dwTimeoutControl);
		_sntprintf(szTimeout, dim(szTimeout), _T("%0.1f"), dActualTimeout);
		DEBUGCHK(szTimeout[dim(szTimeout) - 1] == 0);
		szTimeout[dim(szTimeout) - 1] = 0; // Null-terminate

		DEBUGMSG(SDCARD_ZONE_INIT, (_T("SDHC: Using timeout control value of 0x%x for %s seconds\n"), 
					dwTimeoutControl, szTimeout));
	}
#endif

	dwTimeoutControl	= 0xE;
	return dwTimeoutControl;
}


DWORD 
CSDHCSlotBase::DetermineWakeupSources(
		)
{
	DEBUGCHK(m_pregDevice->IsOK());
	DWORD dwWakeupSources = m_pregDevice->ValueDW(SDHC_WAKEUP_SOURCES_KEY);
	dwWakeupSources &= WAKEUP_ALL_SOURCES;

	// Waking on SDIO interrupts must be enabled by the bus driver.
	dwWakeupSources &= ~WAKEUP_INTERRUPT; 

	return dwWakeupSources;
}


VOID
CSDHCSlotBase::SetVoltage(
		DWORD dwVddSetting
		)
{
	Validate();

	RETAILMSG(0,(TEXT("CSDHCSlotBase::SetVoltage  0x%X\n"),dwVddSetting));

	UCHAR ucVoltageSelection = SDBUS_POWER_ON;
	UCHAR ucOldVoltage;

	DEBUGCHK(dwVddSetting & m_dwVddWindows);

	if ( dwVddSetting & 
			(SD_VDD_WINDOW_3_2_TO_3_3 | SD_VDD_WINDOW_3_3_TO_3_4) ) {
		ucVoltageSelection |= SDBUS_VOLTAGE_SELECT_3_3V;
	}
	else if ( dwVddSetting & 
			(SD_VDD_WINDOW_2_9_TO_3_0 | SD_VDD_WINDOW_3_0_TO_3_1) ) {
		ucVoltageSelection |= SDBUS_VOLTAGE_SELECT_3_0V;
	}
	else if ( dwVddSetting & 
			(SD_VDD_WINDOW_1_7_TO_1_8 | SD_VDD_WINDOW_1_8_TO_1_9) ) {
		ucVoltageSelection |= SDBUS_VOLTAGE_SELECT_1_8V;
	}

	ucOldVoltage = ReadByte(SDHC_POWER_CONTROL);
	if (ucOldVoltage != ucVoltageSelection) {
		// SD Bus Power must be initially set to 0 when changing voltages
		WriteByte(SDHC_POWER_CONTROL, 0);
		WriteByte(SDHC_POWER_CONTROL, ucVoltageSelection);
		RETAILMSG(0,( 
					TEXT("CSDHCSlotBase::SetVoltage: Set SDHC_POWER_CONTROL reg = 0x%02x     %d\n"),
					ReadByte(SDHC_POWER_CONTROL),GetPowerSupplyRampUpMs()));		

		DEBUGMSG(SDCARD_ZONE_INFO,( 
					TEXT("CSDHCSlotBase::SetVoltage: Set SDHC_POWER_CONTROL reg = 0x%02x\n"),
					ucVoltageSelection));

		Sleep(GetPowerSupplyRampUpMs());			
	}
}


// Set up the controller according to the interface parameters.
#if (BSP_TYPE == BSP_SMDK2443)
VOID 
CSDHCSlotBase::SetInterface(
		PSD_CARD_INTERFACE pInterface
		)
{            
	PREFAST_DEBUGCHK(pInterface);
	Validate();
	BYTE hostctl = 0;
	RETAILMSG(0,(TEXT("CSDHCSlotBase::SetInterface %d\n"),pInterface->InterfaceMode));
	if (SD_INTERFACE_SD_MMC_1BIT == pInterface->InterfaceMode) {
		DEBUGMSG(SDCARD_ZONE_INIT, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 1 bit mode \n")));
		RETAILMSG(1, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 1 bit mode , Clock Rate =%d \n"),pInterface->ClockRate));		
		//        WriteByte(SDHC_HOST_CONTROL, 0);
		hostctl = 0;
		m_f4BitMode = FALSE;
	} else if (SD_INTERFACE_SD_4BIT == pInterface->InterfaceMode) {
		DEBUGMSG(SDCARD_ZONE_INIT, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 4 bit mode \n")));
		RETAILMSG(1, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 4 bit mode , Clock Rate =%d \n"),pInterface->ClockRate));
		//       WriteByte(SDHC_HOST_CONTROL, HOSTCTL_DAT_WIDTH);
		hostctl |= HOSTCTL_DAT_WIDTH;
		m_f4BitMode = TRUE;
	} else if (SD_INTERFACE_MMC_8BIT == pInterface->InterfaceMode) {
		DEBUGMSG(SDCARD_ZONE_INIT, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 8 bit mode \n")));
		RETAILMSG(1, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 8 bit mode , Clock Rate =%d \n"),pInterface->ClockRate));		
		//        WriteByte(SDHC_HOST_CONTROL, HOSTCTL_DAT_WIDTH_8BIT|HOSTCTL_HIGHSPEED);
		hostctl |= HOSTCTL_DAT_WIDTH_8BIT;
		m_f4BitMode = TRUE;
	}else {
		DEBUGCHK(FALSE);
	}

	if(pInterface->ClockRate > 25000000)
	{
		hostctl |= HOSTCTL_HIGHSPEED;
	}

	WriteByte(SDHC_HOST_CONTROL, hostctl);
	SetClockRate(&pInterface->ClockRate);
}

#elif (BSP_TYPE == BSP_SMDK2450)
VOID 
CSDHCSlotBase::SetInterface(
		PSD_CARD_INTERFACE_EX pInterface
		)
{            
	PREFAST_DEBUGCHK(pInterface);
	Validate();

#if 0
	BYTE bHostCtr = 0;
	m_f4BitMode = (pInterface->InterfaceModeEx.bit.sd4Bit!=0);
	bHostCtr |= (m_f4BitMode?HOSTCTL_DAT_WIDTH:0);
	bHostCtr |= (pInterface->InterfaceModeEx.bit.sdHighSpeed!=0?HOSTCTL_HIGH_SPEED:0);
#endif
	BYTE hostctl = 0;
	if (1 == pInterface->InterfaceModeEx.bit.hsmmc8Bit) {
		DEBUGMSG(SDCARD_ZONE_INIT, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 8 bit mode\n")));
		RETAILMSG(1, 
				(TEXT("[HSMMC1] Setting for 8 bit mode , Clock Rate = %d Hz\n"),pInterface->ClockRate));
		hostctl |= HOSTCTL_DAT_WIDTH_8BIT;
		m_f4BitMode = TRUE;
	} else if (SD_INTERFACE_SD_MMC_1BIT == pInterface->InterfaceModeEx.bit.sd4Bit) {
		DEBUGMSG(SDCARD_ZONE_INIT, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 1 bit mode\n")));
		RETAILMSG(1, 
				(TEXT("[HSMMC1] Setting for 1 bit mode , Clock Rate = %d Hz\n"),pInterface->ClockRate));		
		hostctl = 0;
		m_f4BitMode = FALSE;
	} else if (SD_INTERFACE_SD_4BIT == pInterface->InterfaceModeEx.bit.sd4Bit) {
		DEBUGMSG(SDCARD_ZONE_INIT, 
				(TEXT("SHCSDSlotOptionHandler - Setting for 4 bit mode\n")));
		RETAILMSG(1, 
				(TEXT("[HSMMC1] Setting for 4 bit mode , Clock Rate = %d Hz\n"),pInterface->ClockRate));
		hostctl |= HOSTCTL_DAT_WIDTH;
		m_f4BitMode = TRUE;
	}
	else {
		DEBUGCHK(FALSE);
	}

	// 2007.09.01 D.Baek
	// In the S3C6400, the high-speed enable bit of HOST_CONTROL register must be disabled
	// even if the card inserted operates at high-speed mode. This is limited only to the S3C6400.
#if 0
	if( (pInterface->ClockRate > 26000000) || (pInterface->InterfaceModeEx.bit.sdHighSpeed!=0) )
	{
		RETAILMSG(1, (TEXT("[HSMMC1] Setting for high speed mode.\n")));
		hostctl |= HOSTCTL_HIGH_SPEED;
	}
#endif

	DEBUGMSG(SDCARD_ZONE_INIT, 
			(TEXT("SHCSDSlotOptionHandler - Setting Host Control Register %x \n"),bHostCtr));
	//    WriteByte(SDHC_HOST_CONTROL, bHostCtr);
	WriteByte(SDHC_HOST_CONTROL, hostctl);
	SetClockRate(&pInterface->ClockRate);
}
#endif


VOID
CSDHCSlotBase::SetPowerState(
		CEDEVICE_POWER_STATE cpsNew
		)
{
	DEBUGCHK(VALID_DX(cpsNew));

	m_fIsPowerManaged = TRUE;

	if (DX_D1_OR_D2(cpsNew)) {
		cpsNew = D0;
	}

	if (m_cpsCurrent != cpsNew) {
		SetHardwarePowerState(cpsNew);
	}
}


VOID
CSDHCSlotBase::PowerDown(
		)
{
	Validate();

	m_cpsAtPowerDown = m_cpsCurrent;

	if (!m_fIsPowerManaged) {
		CEDEVICE_POWER_STATE cps;

		if (m_bWakeupControl) {
			cps = D3;
		}
		else {
			cps = (_CEDEVICE_POWER_STATE)D4;
		}

		SetHardwarePowerState(cps);
	}

	BOOL fKeepPower = FALSE;
	if (m_fSleepsWithPower || m_cpsCurrent == D0) {
		DEBUGCHK(!m_fSleepsWithPower || m_cpsCurrent == D3);
		fKeepPower = TRUE;
	}

	PowerUpDown(FALSE, fKeepPower);
}


VOID 
CSDHCSlotBase::PowerUp(
		)
{
	Validate();

	if (!m_fIsPowerManaged) {
		SetHardwarePowerState(m_cpsAtPowerDown);
	}
	else if (m_fSleepsWithPower) {
		WORD wIntStatus = ReadWord(SDHC_NORMAL_INT_STATUS);
		if (wIntStatus == NORMAL_INT_STATUS_CARD_INT) {
			// We woke system through a card interrupt. We need to clear
			// this so that the IST will not be signalled.
			EnableSDIOInterrupts(FALSE);
			m_fPowerUpDisabledInts = TRUE;
		}
	}

	PowerUpDown(TRUE, TRUE);
	Start();	// added by JJG 06.11.11
}


#if (BSP_TYPE == BSP_SMDK2443)
SD_API_STATUS
CSDHCSlotBase::BusRequestHandler(
		PSD_BUS_REQUEST pRequest
		)
{
	SETFNAME();

	PREFAST_DEBUGCHK(pRequest);
	Validate();

	WORD            wRegCommand;
	SD_API_STATUS   status;
	WORD            wIntStatusEn;
	BOOL            fSuccess;

	DEBUGCHK(m_dwReadyInts == 0);
	DEBUGCHK(!m_fCommandCompleteOccurred);

	RETAILMSG(0, (TEXT("%s CMD:%d\n"), pszFname, pRequest->CommandCode));
	DEBUGMSG(SDHC_SEND_ZONE, (TEXT("%s CMD:%d\n"), pszFname, pRequest->CommandCode));

	// bypass CMD12 if AutoCMD12 was done by hardware
	if (pRequest->CommandCode == 12) {
		if (m_fAutoCMD12Success) {
			RETAILMSG(0, 
					(TEXT("%s AutoCMD12 Succeeded, bypass CMD12.\n"), pszFname));			
			DEBUGMSG(SDHC_SEND_ZONE, 
					(TEXT("%s AutoCMD12 Succeeded, bypass CMD12.\n"), pszFname));
			// The response for Auto CMD12 is in a special area
			UNALIGNED DWORD *pdwResponseBuffer = 
				(PDWORD) (pRequest->CommandResponse.ResponseBuffer + 1); // Skip CRC
			*pdwResponseBuffer = ReadDword(SDHC_R6);
			IndicateBusRequestComplete(pRequest, SD_API_STATUS_SUCCESS);
			status = SD_API_STATUS_SUCCESS;
			goto EXIT;
		}
	}

	m_fAutoCMD12Success = FALSE;

	// initialize command register with command code
	RETAILMSG(0,(TEXT("SendCommand  0x%x\n"),pRequest->CommandCode));
	wRegCommand = (pRequest->CommandCode << CMD_INDEX_SHIFT) & CMD_INDEX_MASK;

	// check for a response
	switch (pRequest->CommandResponse.ResponseType) {
		case NoResponse:
			break;

		case ResponseR2:
			wRegCommand |= CMD_RESPONSE_R2;
			break;

		case ResponseR3:
		case ResponseR4:
			wRegCommand |= CMD_RESPONSE_R3_R4;
			break;

		case ResponseR1:
		case ResponseR5:
		case ResponseR6:
#ifdef _MMC_SPEC_42_
			/*************************************************************************/
			/****** Date : 07.05.14                                         	******/
			/****** Developer : HS.JANG											******/
			/****** Description : to support SD SPEC20 card. Response7 is added	******/
			/****** 			  for supporting SD SPEC20 card					******/
			/*************************************************************************/

		case ResponseR7:
			/*************************************************************************/
#endif    

			wRegCommand |= CMD_RESPONSE_R1_R5_R6;
			break;

		case ResponseR1b:
			wRegCommand |= CMD_RESPONSE_R1B_R5B;   
			break;

		default:
			status = SD_API_STATUS_INVALID_PARAMETER;
			goto EXIT;
	}

	// Set up variable for the new interrupt sources. Note that we must
	// enable DMA and read/write interrupts in this routine (not in
	// HandleCommandComplete) or they will be missed.
	wIntStatusEn = ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE);
	wIntStatusEn |= NORMAL_INT_ENABLE_CMD_COMPLETE | NORMAL_INT_ENABLE_TRX_COMPLETE;

	// check command inhibit, wait until OK
	fSuccess = WaitForReg<DWORD>(ReadDword, SDHC_PRESENT_STATE, STATE_CMD_INHIBIT, 0);
	if (!fSuccess) {
		DEBUGMSG(SDCARD_ZONE_ERROR, (_T("%s Timeout waiting for CMD Inhibit\r\n"),
					pszFname));
		status = SD_API_STATUS_DEVICE_NOT_RESPONDING;
		goto EXIT;
	}

	// programming registers
	if (!TRANSFER_IS_COMMAND_ONLY(pRequest)) {
		WORD wRegTxnMode = 0;   

#ifdef _MMC_SPEC_42_
		/*************************************************************************/
		/****** Date : 07.05.14                                         	******/
		/****** Developer : HS.JANG											******/
		/****** Description : to support MMCMicro this code was added	 	******/
		/*************************************************************************/

		if ( (ReadByte(SDHC_TIMEOUT_CONTROL) != m_dwTimeoutControl) && 
				pRequest->CommandCode != MMC_CMD_SEND_EXT_CSD ) 
		{
			WriteByte(SDHC_TIMEOUT_CONTROL, (BYTE) m_dwTimeoutControl);
		}

		if ( pRequest->CommandCode == MMC_CMD_SEND_EXT_CSD ) 
		{
			WriteByte(SDHC_TIMEOUT_CONTROL, 0x3);
		}
		/*************************************************************************/
#endif


		wRegCommand |= CMD_DATA_PRESENT;

		if (UseDmaForRequest(pRequest)) {
			wIntStatusEn |= NORMAL_INT_ENABLE_DMA;

			BOOL fNoException;
			DEBUGCHK(TRANSFER_SIZE(pRequest) <= CB_DMA_BUFFER);

			if (TRANSFER_IS_WRITE(pRequest)) {
				DWORD cbToTransfer = TRANSFER_SIZE(pRequest);

				SD_SET_PROC_PERMISSIONS_FROM_REQUEST(pRequest) {
					fNoException = SDPerformSafeCopy(m_pbDmaBuffer,
							pRequest->pBlockBuffer, cbToTransfer);
				} SD_RESTORE_PROC_PERMISSIONS();

				if (fNoException == FALSE) {
					status = SD_API_STATUS_ACCESS_VIOLATION;
					goto EXIT;
				}
			}

			WriteDword(SDHC_SYSTEMADDRESS_LO, m_paDmaBuffer);
			wRegTxnMode |= TXN_MODE_DMA;
		}
		else {
			if (TRANSFER_IS_WRITE(pRequest)) {
				wIntStatusEn |= NORMAL_INT_ENABLE_BUF_WRITE_RDY;
			}
			else {
				wIntStatusEn |= NORMAL_INT_ENABLE_BUF_READ_RDY;
			}
		}

		// BlockSize
		// Note that for DMA we are programming the buffer boundary for 512K Bytes
		DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending command block size 0x%04X\r\n"), (WORD) pRequest->BlockSize));
		WriteWord(SDHC_BLOCKSIZE,  (WORD)((0x7<<12)| pRequest->BlockSize));

		// We always go into block mode even if there is only 1 block. 
		// Otherwise the Pegasus will occaissionally hang when
		// writing a single block with DMA.

		wRegTxnMode |= (TXN_MODE_MULTI_BLOCK | TXN_MODE_BLOCK_COUNT_ENABLE);
		/*
		   if(pRequest->CommandCode == 18 || pRequest->CommandCode == 25)
		   wRegTxnMode |= (TXN_MODE_MULTI_BLOCK | TXN_MODE_BLOCK_COUNT_ENABLE);
		   else
		   wRegTxnMode |= ( TXN_MODE_BLOCK_COUNT_ENABLE);*/


		// BlockCount
		DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending command block count 0x%04X\r\n"), 
					(WORD) pRequest->NumBlocks));            
		WriteWord(SDHC_BLOCKCOUNT, (WORD) pRequest->NumBlocks);

		//if (pRequest->Flags & SD_AUTO_ISSUE_CMD12 && *((DWORD *)(pRequest->hDevice)) != 0 ) {
		if (pRequest->Flags & SD_AUTO_ISSUE_CMD12) {
			wRegTxnMode |= TXN_MODE_AUTO_CMD12;
		}

		if (TRANSFER_IS_READ(pRequest)) {
			wRegTxnMode |= TXN_MODE_DATA_DIRECTION_READ;     
		}

		// check dat inhibit, wait until okay
		fSuccess = WaitForReg<DWORD>(ReadDword, SDHC_PRESENT_STATE, STATE_DAT_INHIBIT, 0); 
		if (!fSuccess) {
			DEBUGMSG(SDCARD_ZONE_ERROR, (_T("%s Timeout waiting for DAT Inhibit\r\n"),
						pszFname));
			RETAILMSG(1, (_T("%s Timeout waiting for DAT Inhibit\r\n"),
						pszFname));

			status = SD_API_STATUS_DEVICE_NOT_RESPONDING;
			goto EXIT;
		}

		DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending Transfer Mode 0x%04X\r\n"),wRegTxnMode));	
		WriteWord(SDHC_TRANSFERMODE, wRegTxnMode);
	} else {
		RETAILMSG(0,(TEXT("Command Only\n")));
		// Command-only
		if (pRequest->CommandCode == SD_CMD_STOP_TRANSMISSION) {
			wRegCommand |= CMD_TYPE_ABORT;
		}
		else if (TransferIsSDIOAbort(pRequest)) {
			// Use R5b For CMD52, Function 0, I/O Abort
			DEBUGMSG(SDHC_SEND_ZONE, (TEXT("Sending Abort command \r\n")));
			wRegCommand |= CMD_TYPE_ABORT | CMD_RESPONSE_R1B_R5B;
		}
		if (pRequest->CommandCode == 23) {
			wRegCommand |= CMD_INDEX_CHECK | CMD_CRC_CHECK | CMD_RESPONSE_LENGTH_48;
		}	

		// The following is required for the Pegasus. If it is not present,
		// command-only transfers will sometimes fail (especially R1B and R5B).
		WriteDword(SDHC_SYSTEMADDRESS_LO, 0);
		WriteWord(SDHC_BLOCKSIZE, 0);
		WriteWord(SDHC_BLOCKCOUNT, 0);
	}

	RETAILMSG(0,(TEXT("Sending command register 0x%04X\r\n"),wRegCommand));
	RETAILMSG(0,(TEXT("Sending command Argument 0x%08X\r\n"),pRequest->CommandArgument));
	RETAILMSG(0,(TEXT("Sending command IntStatusEn 0x%08X\r\n"),wIntStatusEn));

	DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending command register 0x%04X\r\n"),wRegCommand));
	DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending command Argument 0x%08X\r\n"),pRequest->CommandArgument));

	WriteDword(SDHC_ARGUMENT_0, pRequest->CommandArgument);

	// Enable transfer interrupt sources.
	WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, wIntStatusEn);

	// Turn the clock on. It is turned off in IndicateBusRequestComplete().
	SDClockOn();

	// Turn the LED on.
	EnableLED(FALSE);

	// Writing the upper byte of the command register starts the command.
	// All register initialization must already be complete by this point.
	WriteWord(SDHC_COMMAND, wRegCommand);

	status = SD_API_STATUS_PENDING;
	RETAILMSG(0,(TEXT("Sending command IntStatusEn 0x%08X\r\n"),ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE)));
EXIT:
	return status;
}

#elif (BSP_TYPE == BSP_SMDK2450)
	SD_API_STATUS
CSDHCSlotBase::BusRequestHandler( PSD_BUS_REQUEST pRequest)
{
	SETFNAME();
	SD_API_STATUS status;
	PREFAST_DEBUGCHK(pRequest);
	Validate();
	DEBUGMSG(SDHC_SEND_ZONE, (TEXT("%s CMD:%d\n"), pszFname, pRequest->CommandCode));
	RETAILMSG(0, (TEXT("CMD:%d\n"), pRequest->CommandCode));
	if (m_pCurrentRequest) { // We have outstand request.
		ASSERT(FALSE);
		IndicateBusRequestComplete(pRequest, SD_API_STATUS_CANCELED);
		m_pCurrentRequest = NULL;
	}
	if (!m_fCardPresent) {
		status= SD_API_STATUS_DEVICE_REMOVED;
	}
	else {
		WORD wIntSignals = ReadWord(SDHC_NORMAL_INT_SIGNAL_ENABLE);
		WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE,0);
		m_fCurrentRequestFastPath = FALSE;
		m_pCurrentRequest = pRequest ;
		// if no data transfer involved, use FAST PATH
		if ((pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE)!=0) { // Fastpath 
			m_fCurrentRequestFastPath = TRUE;
			status = SubmitBusRequestHandler( pRequest );

			if( status == SD_API_STATUS_PENDING ) { // Polling for completion.
				//while (m_pCurrentRequest && (ReadDword(SDHC_PRESENT_STATE) & STATE_CARD_INSERTED)!=0) {
				// 08.04.04 by KYS
				// To check the state on CARD INSERTED on Ch0, use IsCardPresent().
				BOOL fCardInserted = TRUE;
				while (m_pCurrentRequest && (fCardInserted = (IsCardPresent() & TRUE)!=0)) {
					if (IsOnlySDIOInterrupt() != TRUE)
					HandleInterrupt();
				}
				if(m_pCurrentRequest && fCardInserted){
					// Time out , need to switch to asyn.it will call callback after this
					pRequest->SystemFlags &= ~SD_FAST_PATH_AVAILABLE;
					m_fCurrentRequestFastPath = FALSE;
				}
				else {	// Fastpass complete
					status = m_FastPathStatus;
					// 08.01.27 by KYS
					// Below added code clear before status of fastpath.
					m_FastPathStatus = 0;
					if (m_pCurrentRequest) {
						ASSERT(FALSE);
						status = SD_API_STATUS_DEVICE_REMOVED;
					}
				}
			}
			if (status == SD_API_STATUS_SUCCESS) {
				status = SD_API_STATUS_FAST_PATH_SUCCESS;
			}

		// 08.04.21 by KYS
		// if there is error case. We don't notify the callback function either So.
		//ASSERT(m_fCurrentRequestFastPath);
		//m_pCurrentRequest = NULL;
		}
		else  
			status = SubmitBusRequestHandler( pRequest );

		// 08.04.21 by KYS
		// if there is error case. We don't notify the callback function either So.
		//if (status!=SD_API_STATUS_PENDING) { // This one has been finished.
		if (status!=SD_API_STATUS_PENDING && m_pCurrentRequest) {
			m_fCurrentRequestFastPath = TRUE;
			IndicateBusRequestComplete(pRequest, status);
			//m_pCurrentRequest = NULL;
		}
		WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE,wIntSignals);
	}
	return status;
}

SD_API_STATUS
CSDHCSlotBase::SubmitBusRequestHandler(
		PSD_BUS_REQUEST pRequest
		)
{
	SETFNAME();

	PREFAST_DEBUGCHK(pRequest);
	Validate();

	WORD            wRegCommand;
	SD_API_STATUS   status;
	WORD            wIntStatusEn;
	BOOL            fSuccess;

	DEBUGCHK(m_dwReadyInts == 0);
	DEBUGCHK(!m_fCommandCompleteOccurred);

	DEBUGMSG(SDHC_SEND_ZONE, (TEXT("%s CMD:%d\n"), pszFname, pRequest->CommandCode));

	// bypass CMD12 if AutoCMD12 was done by hardware
	if (pRequest->CommandCode == 12) {
		if (m_fAutoCMD12Success) {
			DEBUGMSG(SDHC_SEND_ZONE, 
					(TEXT("%s AutoCMD12 Succeeded, bypass CMD12.\n"), pszFname));
			// The response for Auto CMD12 is in a special area
			UNALIGNED DWORD *pdwResponseBuffer = 
				(PDWORD) (pRequest->CommandResponse.ResponseBuffer + 1); // Skip CRC
			*pdwResponseBuffer = ReadDword(SDHC_R6);
			IndicateBusRequestComplete(pRequest, SD_API_STATUS_SUCCESS);
			status = SD_API_STATUS_SUCCESS;
			goto EXIT;
		}
	}

	m_fAutoCMD12Success = FALSE;

	// initialize command register with command code
	wRegCommand = (pRequest->CommandCode << CMD_INDEX_SHIFT) & CMD_INDEX_MASK;

	// check for a response
	switch (pRequest->CommandResponse.ResponseType) {
		case NoResponse:
			break;

		case ResponseR2:
			wRegCommand |= CMD_RESPONSE_R2;
			break;

		case ResponseR3:
		case ResponseR4:
			wRegCommand |= CMD_RESPONSE_R3_R4;
			break;

		case ResponseR1:
		case ResponseR5:
		case ResponseR6:
		case ResponseR7:
			wRegCommand |= CMD_RESPONSE_R1_R5_R6_R7;
			break;

		case ResponseR1b:
			wRegCommand |= CMD_RESPONSE_R1B_R5B;   
			break;

		default:
			status = SD_API_STATUS_INVALID_PARAMETER;
			goto EXIT;
	}

	// Set up variable for the new interrupt sources. Note that we must
	// enable DMA and read/write interrupts in this routine (not in
	// HandleCommandComplete) or they will be missed.
	wIntStatusEn = ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE);
	wIntStatusEn |= NORMAL_INT_ENABLE_CMD_COMPLETE | NORMAL_INT_ENABLE_TRX_COMPLETE;

	fSuccess = WaitForReg<DWORD>(ReadDword, SDHC_PRESENT_STATE, STATE_CMD_INHIBIT, 0);
	if (!fSuccess) {
		DEBUGMSG(SDCARD_ZONE_ERROR, (_T("%s Timeout waiting for CMD Inhibit\r\n"),
					pszFname));
		status = SD_API_STATUS_DEVICE_NOT_RESPONDING;
		goto EXIT;
	}

	// programming registers
	if (!TRANSFER_IS_COMMAND_ONLY(pRequest)) {
		WORD wRegTxnMode = 0;
#ifdef _MMC_SPEC_42_
		// Date : 07.05.14
		// Developer : HS.JANG
		// Description : to support MMCMicro this code was added
		if ( (ReadByte(SDHC_TIMEOUT_CONTROL) != m_dwTimeoutControl) && 
				(pRequest->CommandCode != MMC_CMD_SEND_EXT_CSD) ) {
			WriteByte(SDHC_TIMEOUT_CONTROL, (BYTE) m_dwTimeoutControl);
		} else if ( (ReadByte(SDHC_TIMEOUT_CONTROL) == m_dwTimeoutControl) &&
				(pRequest->CommandCode == MMC_CMD_SEND_EXT_CSD) ) {
			WriteByte(SDHC_TIMEOUT_CONTROL, (BYTE)0x3);
		}
#endif

		wRegCommand |= CMD_DATA_PRESENT;

		if (UseDmaForRequest(pRequest)) {
			// 08.05.30 by KYS
			// Note that we cat not know the next address if use DMA interrupt, so DMA interrupt should be disabled!.
			//wIntStatusEn |= NORMAL_INT_ENABLE_DMA;

			BOOL fNoException;
			DEBUGCHK(TRANSFER_SIZE(pRequest) <= CB_DMA_BUFFER);

			if (TRANSFER_IS_WRITE(pRequest)) {
				DWORD cbToTransfer = TRANSFER_SIZE(pRequest);

				SD_SET_PROC_PERMISSIONS_FROM_REQUEST(pRequest) {
					fNoException = SDPerformSafeCopy(m_pbDmaBuffer,
							pRequest->pBlockBuffer, cbToTransfer);
				} SD_RESTORE_PROC_PERMISSIONS();

				if (fNoException == FALSE) {
					status = SD_API_STATUS_ACCESS_VIOLATION;
					goto EXIT;
				}
			}

			WriteDword(SDHC_SYSTEMADDRESS_LO, m_paDmaBuffer);
			wRegTxnMode |= TXN_MODE_DMA;
		}
		else {
			if (TRANSFER_IS_WRITE(pRequest)) {
				wIntStatusEn |= NORMAL_INT_ENABLE_BUF_WRITE_RDY;
			}
			else {
				wIntStatusEn |= NORMAL_INT_ENABLE_BUF_READ_RDY;
			}
		}

		// BlockSize
		// Note that for DMA we are programming the buffer boundary for 4k
		DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending command block size 0x%04X\r\n"), (WORD) pRequest->BlockSize));
		//WriteWord(SDHC_BLOCKSIZE, (WORD) pRequest->BlockSize);

		// 08.05.30 by KYS
		// BlockSize
		// Note that for DMA we are programming the buffer boundary for 512k beacause we cat not know the next address if use DMA interrupt.
		WriteWord(SDHC_BLOCKSIZE, (WORD)(pRequest->BlockSize & 0xfff) | (0x7<<12));

		// We always go into block mode even if there is only 1 block. 
		// Otherwise the Pegasus will occaissionally hang when
		// writing a single block with DMA.
		wRegTxnMode |= (TXN_MODE_MULTI_BLOCK | TXN_MODE_BLOCK_COUNT_ENABLE);

		// BlockCount
		DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending command block count 0x%04X\r\n"), 
					(WORD) pRequest->NumBlocks));            
		WriteWord(SDHC_BLOCKCOUNT, (WORD) pRequest->NumBlocks);

		if (pRequest->Flags & SD_AUTO_ISSUE_CMD12) {
			wRegTxnMode |= TXN_MODE_AUTO_CMD12;
		}

		if (TRANSFER_IS_READ(pRequest)) {
			wRegTxnMode |= TXN_MODE_DATA_DIRECTION_READ;     
		}

		// check dat inhibit, wait until okay
		fSuccess = WaitForReg<DWORD>(ReadDword, SDHC_PRESENT_STATE, STATE_DAT_INHIBIT, 0); 
		if (!fSuccess) {
			DEBUGMSG(SDCARD_ZONE_ERROR, (_T("%s Timeout waiting for DAT Inhibit\r\n"),
						pszFname));
			status = SD_API_STATUS_DEVICE_NOT_RESPONDING;
			goto EXIT;
		}

		DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending Transfer Mode 0x%04X\r\n"),wRegTxnMode));
		WriteWord(SDHC_TRANSFERMODE, wRegTxnMode);
	}
	else {
		// Command-only
		if (pRequest->CommandCode == SD_CMD_STOP_TRANSMISSION) {
			wRegCommand |= CMD_TYPE_ABORT;
		}
		else if (TransferIsSDIOAbort(pRequest)) {
			// Use R5b For CMD52, Function 0, I/O Abort
			DEBUGMSG(SDHC_SEND_ZONE, (TEXT("Sending Abort command \r\n")));
			wRegCommand |= CMD_TYPE_ABORT | CMD_RESPONSE_R1B_R5B;
		}

		// The following is required for the Pegasus. If it is not present,
		// command-only transfers will sometimes fail (especially R1B and R5B).
		//WriteDword(SDHC_SYSTEMADDRESS_LO, 0);
		//WriteWord(SDHC_BLOCKSIZE, 0);
		//WriteWord(SDHC_BLOCKCOUNT, 0);
	}

	DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending command register 0x%04X\r\n"),wRegCommand));
	DEBUGMSG(SDHC_SEND_ZONE,(TEXT("Sending command Argument 0x%08X\r\n"),pRequest->CommandArgument));

	WriteDword(SDHC_ARGUMENT_0, pRequest->CommandArgument);

	// Enable transfer interrupt sources.
	WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, wIntStatusEn);

	// 08.06.16, by KYS
	// Status Busy bit checking for clearing the interrupt status register before "CMD ISSUE".
	while(ReadDword(SDHC_CONTROL4)&(0x1));

	// Turn the clock on. It is turned off in IndicateBusRequestComplete().
	SDClockOn();

	// Turn the LED on.
	EnableLED(TRUE);

	// Writing the upper byte of the command register starts the command.
	// All register initialization must already be complete by this point.
	WriteWord(SDHC_COMMAND, wRegCommand);

	// 08.04.21 by KYS
	// Polling the command complete for "Real" FastPath
	PollingForCommandComplete();

	status = SD_API_STATUS_PENDING;

EXIT:
	return status;
}

// 08.04.21 by KYS
// This polling function is needed for Fastpath and perform polling the command complete.
BOOL CSDHCSlotBase::PollingForCommandComplete()
{
	BOOL            fContinue = TRUE;
	if (m_fFakeCardRemoval && m_fCardPresent) {
		m_fFakeCardRemoval = FALSE;
		HandleRemoval(TRUE);
	}
	else {
		// Assume we reading PCI register at 66 Mhz. for times of 100 us. it should be 10*1000 time
		for (DWORD dwIndex=0; fContinue  && dwIndex<10*1000; dwIndex ++ ) {
			WORD wIntStatus = ReadWord(SDHC_NORMAL_INT_STATUS);
			if (wIntStatus != 0) {
				DEBUGMSG(SDHC_INTERRUPT_ZONE,
						(TEXT("PollingForCommandComplete (%u) - Normal Interrupt_Status=0x%02x\n"),
						 m_dwSlot, wIntStatus));

				// Error handling. Make sure to handle errors first. 
				if ( wIntStatus & NORMAL_INT_STATUS_ERROR_INT ) {
					HandleErrors();
					// 08.01.27 by KYS
					// you can also activate below code "continue;" in order to ensure the fastpath.
					// then, you don't need to keep the fastpath status varible at "HandleCommandComplete" function.
					//continue;
					fContinue = FALSE;	// Comment out this orginal line by KYS
				}

				// Command Complete handling.
				if ( wIntStatus & NORMAL_INT_STATUS_CMD_COMPLETE ) {
					// Clear status
					m_fCommandCompleteOccurred = TRUE;
					fContinue = FALSE;
					WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_CMD_COMPLETE);

					if (HandleCommandComplete()) { // If completed. 
						WriteWord(SDHC_NORMAL_INT_STATUS, (wIntStatus & NORMAL_INT_STATUS_TRX_COMPLETE));
					}
				}
			}
		}
	}
	ASSERT(!fContinue);
	return (!fContinue);
}

#endif  // !(BSP_TYPE == BSP_SMDK2443)

VOID 
CSDHCSlotBase::EnableSDIOInterrupts(
		BOOL fEnable
		)
{
	Validate();

	if (fEnable) {
		m_fSDIOInterruptsEnabled = TRUE;
		DoEnableSDIOInterrupts(fEnable);
	}
	else {
		DoEnableSDIOInterrupts(fEnable);
		m_fSDIOInterruptsEnabled = FALSE;
	}
}


#if (BSP_TYPE == BSP_SMDK2443)
VOID 
CSDHCSlotBase::HandleInterrupt(
		)
{
	Validate();

	WORD wIntStatus = ReadWord(SDHC_NORMAL_INT_STATUS);
	RETAILMSG(0,(TEXT("Status = 0x%X\n"),wIntStatus));	

	if (m_fFakeCardRemoval && m_fCardPresent) {
		RETAILMSG(0,(TEXT("Wakeup !!!!!! m_fFakeCardRemoval && m_fCardPresent\n")));
		m_fFakeCardRemoval = FALSE;
		HandleRemoval(TRUE);
	}

	if (wIntStatus != 0) {
		DEBUGMSG(SDHC_INTERRUPT_ZONE, 
				(TEXT("HandleInterrupt (%u) - Normal Interrupt_Status=0x%02x\n"),
				 m_dwSlot, wIntStatus));

		// Error handling. Make sure to handle errors first. 
		if ( wIntStatus & NORMAL_INT_STATUS_ERROR_INT ) {
			HandleErrors();

		}
		//if ( wIntStatus & NORMAL_INT_STATUS_CMD_COMPLETE ) {
		// Command Complete handling.
		else if ( wIntStatus & NORMAL_INT_STATUS_CMD_COMPLETE ) {		// modifyed by JJG 06.10.11
			// Clear status
			m_fCommandCompleteOccurred = TRUE;
			WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_CMD_COMPLETE);
			if ( HandleCommandComplete() ) {
				wIntStatus &= ~NORMAL_INT_STATUS_TRX_COMPLETE; // this is command-only request. 
			}
		}

		// Sometimes at the lowest clock rate, the Read/WriteBufferReady
		// interrupt actually occurs before the CommandComplete interrupt.
		// This confuses our debug validation code and could potentially
		// cause problems. This is why we will verify that the CommandComplete
		// occurred before processing any data transfer interrupts.
		if (m_fCommandCompleteOccurred) {
			if (wIntStatus & NORMAL_INT_STATUS_DMA) {
				WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_DMA);
				HandleDma();
				// do not break here. Continue to check TransferComplete. 
			}

			// Buffer Read Ready handling
			if (wIntStatus & NORMAL_INT_STATUS_BUF_READ_RDY ) {
				// Clear status
				WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_BUF_READ_RDY);
				HandleReadReady();
				// do not break here. Continue to check TransferComplete. 
			}

			// Buffer Write Ready handling
			if (wIntStatus & NORMAL_INT_STATUS_BUF_WRITE_RDY ) {
				// Clear status
				WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_BUF_WRITE_RDY);
				HandleWriteReady();
				// do not break here. Continue to check TransferComplete. 
			}
		}
		else {
			// We received data transfer interrupt before command 
			// complete interrupt. Wait for the command complete before
			// processing the data interrupt.
		}

		// Transfer Complete handling
		if ( wIntStatus & NORMAL_INT_STATUS_TRX_COMPLETE ) {
			// Clear status
			WriteWord(SDHC_NORMAL_INT_STATUS, 
					NORMAL_INT_STATUS_TRX_COMPLETE | NORMAL_INT_STATUS_DMA);
			HandleTransferDone();
		}

		// SDIO Interrupt Handling
		if ( wIntStatus & NORMAL_INT_STATUS_CARD_INT ) {
			DEBUGCHK(m_fSDIOInterruptsEnabled);
			DEBUGMSG(SDHC_INTERRUPT_ZONE, (_T("SDHCControllerIst: Card interrupt!\n")));

			// Because SDIO Interrupt is level triggered, we are not able to clear
			// the status. The status should be cleared by the card
			// we just disable the interrupt from Interrupt Signal Register
			// and Interrupt Status Enable register, and indicate that 
			// the card is interrupting
			EnableSDIOInterrupts(FALSE);
			//WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_CARD_INT);
			//RETAILMSG(1,(TEXT("SDHC_NORMAL_INT_STATUS=0x%X\n"),ReadWord(SDHC_NORMAL_INT_STATUS)));			
			IndicateSlotStateChange(DeviceInterrupting);
		}

		// Card Detect Interrupt Handling
		if (wIntStatus & (NORMAL_INT_STATUS_CARD_INSERTION | NORMAL_INT_STATUS_CARD_REMOVAL)) {
			WriteWord(
					SDHC_NORMAL_INT_STATUS,
					NORMAL_INT_STATUS_CARD_INSERTION | NORMAL_INT_STATUS_CARD_REMOVAL);
			m_fCheckSlot = TRUE;
		}
	}			
	if (m_fCheckSlot) {
		m_fCheckSlot = FALSE;

		// check card inserted or removed
		DWORD dwPresentState = ReadDword(SDHC_PRESENT_STATE);
		if (dwPresentState & STATE_CARD_INSERTED) {
			RETAILMSG(1, (TEXT("+++++++++++++++++++++SDHCControllerIst - Card is Inserted! 0x%08X\n"),this));			
			DEBUGMSG(SDHC_INTERRUPT_ZONE, (TEXT("SDHCControllerIst - Card is Inserted! \n")));
			m_fFakeCardRemoval = FALSE;			// added by JJG 06.11.13
			if (m_fCardPresent == FALSE ) {
				HandleInsertion();
			}
		}
		else {
			RETAILMSG(1, (TEXT("------------------------SDHCControllerIst - Card is Removed! 0x%08X\n"),this));	

			DEBUGMSG(SDHC_INTERRUPT_ZONE, (TEXT("SDHCControllerIst - Card is Removed! \n")));
			m_fFakeCardRemoval = FALSE;			// added by JJG 06.11.13
			if (m_fCardPresent) {
				HandleRemoval(TRUE);
			}
		}
	}
}

#elif (BSP_TYPE == BSP_SMDK2450)

#ifndef _SMDK2450_CH1_EXTCD_
VOID CSDHCSlotBase::HandleInterrupt()
#else
	// 08.03.20 by KYS
	// New Interrupt handler function can process factors on new card detect interrupt of HSMMC ch0 on SMDK416.
VOID CSDHCSlotBase::HandleInterrupt(SDSLOT_INT_TYPE intType)
#endif
{
	Validate();
#ifdef _SMDK2450_CH1_EXTCD_
	// in case of it is occurred a card detect interrupt of HSMMC ch0 on SMDK2450.
	if (intType == SDSLOT_INT_CARD_DETECTED) {
		RETAILMSG(0,(TEXT("[HSMMC1] Card Insertion or Removal Detect!!\n")));	
		m_fCheckSlot = TRUE;
	} else {  
#endif
		WORD wIntStatus = 0;
		wIntStatus = ReadWord(SDHC_NORMAL_INT_STATUS);

		if (m_fFakeCardRemoval && m_fCardPresent) {
			m_fFakeCardRemoval = FALSE;
			HandleRemoval(TRUE);
		}

		if (wIntStatus != 0) {
			DEBUGMSG(SDHC_INTERRUPT_ZONE, 
					(TEXT("HandleInterrupt (%u) - Normal Interrupt_Status=0x%02x\n"),
					 m_dwSlot, wIntStatus));
			RETAILMSG(0, 
					(TEXT("HandleInterrupt (%u) - Normal Interrupt_Status=0x%02x\n"),
					 m_dwSlot, wIntStatus));

			// Error handling. Make sure to handle errors first. 
			if ( wIntStatus & NORMAL_INT_STATUS_ERROR_INT ) {
				HandleErrors();
			}

			// Command Complete handling.
			if ( wIntStatus & NORMAL_INT_STATUS_CMD_COMPLETE ) {
				// Clear status
				m_fCommandCompleteOccurred = TRUE;
					WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_CMD_COMPLETE);

				if ( HandleCommandComplete() ) {
					wIntStatus &= ~NORMAL_INT_STATUS_TRX_COMPLETE; // this is command-only request. 
				}
			}

			// Sometimes at the lowest clock rate, the Read/WriteBufferReady
			// interrupt actually occurs before the CommandComplete interrupt.
			// This confuses our debug validation code and could potentially
			// cause problems. This is why we will verify that the CommandComplete
			// occurred before processing any data transfer interrupts.
			if (m_fCommandCompleteOccurred) {
				if (wIntStatus & NORMAL_INT_STATUS_DMA) {
						WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_DMA);

					HandleDma();
					// do not break here. Continue to check TransferComplete. 
				}

				// Buffer Read Ready handling
				if (wIntStatus & NORMAL_INT_STATUS_BUF_READ_RDY ) {
					// Clear status
						WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_BUF_READ_RDY);

					HandleReadReady();
					// do not break here. Continue to check TransferComplete. 
				}

				// Buffer Write Ready handling
				if (wIntStatus & NORMAL_INT_STATUS_BUF_WRITE_RDY ) {
					// Clear status
						WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_BUF_WRITE_RDY);

					HandleWriteReady();
					// do not break here. Continue to check TransferComplete. 
				}
			}
			else {
				// We received data transfer interrupt before command 
				// complete interrupt. Wait for the command complete before
				// processing the data interrupt.
			}

			// Transfer Complete handling
			if ( wIntStatus & NORMAL_INT_STATUS_TRX_COMPLETE ) {
				// Clear status
					WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_TRX_COMPLETE | NORMAL_INT_STATUS_DMA);


				HandleTransferDone();
			}

			// SDIO Interrupt Handling
			if ( wIntStatus & NORMAL_INT_STATUS_CARD_INT ) {
				DEBUGCHK(m_fSDIOInterruptsEnabled);
				DEBUGMSG(SDHC_INTERRUPT_ZONE, (_T("SDHCControllerIst: Card interrupt!\n")));

				// Because SDIO Interrupt is level triggered, we are not able to clear
				// the status. The status should be cleared by the card
				// we just disable the interrupt from Interrupt Signal Register
				// and Interrupt Status Enable register, and indicate that 
				// the card is interrupting
				EnableSDIOInterrupts(FALSE);
				IndicateSlotStateChange(DeviceInterrupting);
			}

			// Card Detect Interrupt Handling
			if (wIntStatus & (NORMAL_INT_STATUS_CARD_INSERTION | NORMAL_INT_STATUS_CARD_REMOVAL)) {
					WriteWord(
							SDHC_NORMAL_INT_STATUS,
							NORMAL_INT_STATUS_CARD_INSERTION | NORMAL_INT_STATUS_CARD_REMOVAL);

				m_fCheckSlot = TRUE;
			}
		}
#ifdef _SMDK2450_CH1_EXTCD_
	} // The end of "if(intType == SDSLOT_INT_CARD_DETECTED)"
#endif

	if (m_fCheckSlot) {
		m_fCheckSlot = FALSE;
#ifdef _SMDK2450_CH1_EXTCD_
		// At this time, we have to validate the card present status.
		if((IsCardPresent() == TRUE) && (m_fCardPresent != TRUE))
#else	
			// check card inserted or removed
			DWORD dwPresentState = ReadDword(SDHC_PRESENT_STATE);
		if (dwPresentState & STATE_CARD_INSERTED)
#endif
		{
			DEBUGMSG(SDHC_INTERRUPT_ZONE, (TEXT("SDHCControllerIst - Card is Inserted! \n")));
			RETAILMSG(1, (TEXT("[HSMMC1] Card is Inserted! \n")));
			m_fFakeCardRemoval = FALSE;	// KYS
			if (m_fCardPresent == FALSE ) {
				Start();
				HandleInsertion();
			}
		}
#ifdef _SMDK2450_CH1_EXTCD_
		else if((IsCardPresent() == FALSE))
#else    
		else
#endif
		{
			DEBUGMSG(SDHC_INTERRUPT_ZONE, (TEXT("SDHCControllerIst - Card is Removed! \n")));
			RETAILMSG(1, (TEXT("[HSMMC1] Card is Removed! \n")));
			m_fFakeCardRemoval = FALSE;	// KYS
			if (m_fCardPresent) {
				HandleRemoval(TRUE);
			}
		}
	}
}
#endif  // !(BSP_TYPE == BSP_SMDK2443)


#ifdef _SMDK2450_CH1_EXTCD_
// New function can detect whether card is presented of HSMMC ch0 on SMDK2450.
BOOL CSDHCSlotBase::IsCardPresent()
{
	BOOL fRetVal;
	volatile S3C2450_IOPORT_REG *pIOPreg = NULL;
	pIOPreg = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (pIOPreg == NULL) {
		RETAILMSG(1,(TEXT("[HSMMC1] GPIO registers is *NOT* allocated.\n")));
		return FALSE;
	}
	if (!VirtualCopy((PVOID)pIOPreg, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8),
				sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE)) {
		RETAILMSG(1,(TEXT("[HSMMC1] GPIO registers is *NOT* mapped.\n")));
		return FALSE;
	}

	if ( (pIOPreg->GPFDAT & (0x1<<1)) == 0 ) {
		fRetVal = TRUE;
	}	else {
		fRetVal = FALSE;
	}

	RETAILMSG(0,(TEXT("IsCardPresent() : fRetVal = %d  GPFDAT = 0x%x\n"), fRetVal, pIOPreg->GPFDAT));

	VirtualFree((PVOID) pIOPreg, 0, MEM_RELEASE);

	return fRetVal;
}
#endif


VOID 
CSDHCSlotBase::HandleRemoval(
		BOOL fCancelRequest
		)
{    
	m_fCardPresent = FALSE;
	m_fIsPowerManaged = FALSE;
	m_fSleepsWithPower = FALSE;
	m_fPowerUpDisabledInts = FALSE;
	m_f4BitMode = FALSE;
	m_cpsCurrent = D0;

	// Wake on SDIO interrupt must be set by the client
	m_bWakeupControl &= ~WAKEUP_INTERRUPT;

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
	// To control the Data CRC error
	WORD wErrIntSignalEn = ReadWord(SDHC_ERROR_INT_SIGNAL_ENABLE);
	WORD wErrIntStatusEn = ReadWord(SDHC_ERROR_INT_STATUS_ENABLE);
	WORD wErrIntStatus = ReadWord(SDHC_ERROR_INT_STATUS);
	WriteWord(SDHC_ERROR_INT_SIGNAL_ENABLE, (wErrIntSignalEn & ~(0x20))); //Command and Data CRC error disable
	WriteWord(SDHC_ERROR_INT_STATUS_ENABLE, (wErrIntStatusEn & ~(0x20))); //Command and Data CRC error disable
#endif
	if (m_fSDIOInterruptsEnabled) {
		EnableSDIOInterrupts(FALSE);
	}

	IndicateSlotStateChange(DeviceEjected);

	// turn off clock and remove power from the slot
	SDClockOff();
	WriteByte(SDHC_POWER_CONTROL, 0);

	if (fCancelRequest) {
		// get the current request  
		PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

		if (pRequest != NULL) {
			DEBUGMSG(SDCARD_ZONE_WARN, 
					(TEXT("Card Removal Detected - Canceling current request: 0x%08X, command: %d\n"), 
					 pRequest, pRequest->CommandCode));
			DumpRequest(pRequest, SDHC_SEND_ZONE || SDHC_RECEIVE_ZONE);
			IndicateBusRequestComplete(pRequest, SD_API_STATUS_DEVICE_REMOVED);
		}
	}

	if (m_pbDmaBuffer) {
		DEBUGCHK(m_paDmaBuffer);
		FreePhysBuffer(m_pbDmaBuffer);
		m_pbDmaBuffer = NULL;
		m_paDmaBuffer = 0;

		// The Pegasus requires the following so that the next
		// insertion will work correctly.
		SoftwareReset(SOFT_RESET_CMD | SOFT_RESET_DAT);
		WriteDword(SDHC_SYSTEMADDRESS_LO, 0);
		WriteWord(SDHC_BLOCKSIZE, 0);
		WriteWord(SDHC_BLOCKCOUNT, 0);
#if (BSP_TYPE == BSP_SMDK2443)
		WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_DMA);

#elif (BSP_TYPE == BSP_SMDK2450)
		WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE, (ReadWord(SDHC_NORMAL_INT_SIGNAL_ENABLE) & ~(NORMAL_INT_STATUS_DMA)));
		WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE,(ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE) & ~(NORMAL_INT_STATUS_DMA)));
		do {
			WriteWord(SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_DMA);
		}while(ReadWord(SDHC_NORMAL_INT_STATUS) & NORMAL_INT_STATUS_DMA);
		WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, (ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE) | NORMAL_INT_STATUS_DMA));
		WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE, (ReadWord(SDHC_NORMAL_INT_SIGNAL_ENABLE) | NORMAL_INT_STATUS_DMA));

		// To diable the Data CRC error interrupt
		WriteWord(SDHC_ERROR_INT_SIGNAL_ENABLE,0);  
		do
		{
			WriteWord(SDHC_ERROR_INT_STATUS,wErrIntStatus);
		}while(ReadWord(SDHC_ERROR_INT_STATUS));
		WriteWord(SDHC_ERROR_INT_SIGNAL_ENABLE,wErrIntSignalEn);
#endif
	}

	// 08.04.03 by KYS
	// Below code lines are needed for working as a mass storage device
	v_gBspArgs->g_SDCardState = CARD_REMOVED;
	SetEvent( v_gBspArgs->g_SDCardDetectEvent ); 
}


VOID 
CSDHCSlotBase::HandleInsertion(
		)
{
	DWORD dwClockRate = SD_DEFAULT_CARD_ID_CLOCK_RATE;

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
	WORD wErrIntSignalEn = ReadWord(SDHC_ERROR_INT_SIGNAL_ENABLE);
	WORD wErrIntStatusEn = ReadWord(SDHC_ERROR_INT_STATUS_ENABLE);

	g_initialInsertion = 0;
#endif
	m_fCardPresent = TRUE;

	// Apply the initial voltage to the card.
	SetVoltage(GetMaxVddWindow());

	// Send at least 74 clocks to the card over the course of at least 1 ms
	// with allowance for power supply ramp-up time. (SD Phys Layer 6.4)
	// Note that power supply ramp-up time occurs in SetVoltage().
	RETAILMSG(0,(TEXT("++SetClockRate(&dwClockRate)\n")));
	SetClockRate(&dwClockRate);

	SDClockOn();
	DWORD dwSleepMs = (74 / (dwClockRate / 1000)) + 1;
	Sleep(dwSleepMs);

	SDClockOff();

	if (GetCapabilities().bits.DMA) {
		RETAILMSG(0,(TEXT("Use DMA\n")));
		PHYSICAL_ADDRESS SystemAddr = { 0 };
		PVOID pvBuffer = AllocPhysBuffer(CB_DMA_BUFFER, &SystemAddr.LowPart);        
		if (pvBuffer) {
			PHYSICAL_ADDRESS BusAddr;

			// Translate physical address to logical (bus-relative) address.
			BOOL fSuccess = TranslateSystemAddr(m_hBusAccess, m_interfaceType, 
					m_dwBusNumber, SystemAddr, &BusAddr);
			if (fSuccess) {
				RETAILMSG(0,(TEXT("Use DMA Success.\n")));				
				m_pbDmaBuffer= (PBYTE) pvBuffer;
				m_paDmaBuffer = BusAddr.LowPart;
			}
			else {
				// If we fail, we will simply fall back to PIO mode
				FreePhysBuffer(pvBuffer);
			}
		}
	}

	// Interrupts are not enabled on a newly inserted card.
	EnableSDIOInterrupts(FALSE);

	// Indicate device arrival
	IndicateSlotStateChange(DeviceInserted);
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
	WriteWord(SDHC_ERROR_INT_SIGNAL_ENABLE, (wErrIntSignalEn | (0x20))); //Command and Data CRC error disable
	WriteWord(SDHC_ERROR_INT_STATUS_ENABLE, (wErrIntStatusEn | (0x20))); //Command and Data CRC error disable
#endif

	// 08.04.03 by KYS
	// Below code lines are needed for working as a mass storage device
	v_gBspArgs->g_SDCardState = CARD_INSERTED;
	SetEvent( v_gBspArgs->g_SDCardDetectEvent );
}


BOOL 
CSDHCSlotBase::HandleCommandComplete(
		)
{
	SETFNAME();

	PSD_BUS_REQUEST pRequest;
	BOOL fRet = FALSE;
	int count = 0 ;
	RETAILMSG(0,(TEXT("CSDHCSlotBase::HandleCommandComplete\n")));
	// get the current request  
	pRequest = GetAndLockCurrentRequest();

	DEBUGCHK(m_dwReadyInts == 0);

	if (pRequest) {
		RETAILMSG(0,(TEXT("CSDHCSlotBase::HandleCommandComplete    pRequest\n")));		
		DEBUGCHK(pRequest->HCParam == 0);
#if (BSP_TYPE == BSP_SMDK2443)
		SD_API_STATUS transferStatus = SD_API_STATUS_SUCCESS;

#elif (BSP_TYPE == BSP_SMDK2450)
		// 08.02.03 by KYS
		// INT_TRX_COMPLETE is not suitable for s3c2450, so I comment out below code.
		// therefore, in order to keep fastpath status, those code like just below is needed.
		//SD_API_STATUS transferStatus = SD_API_STATUS_SUCCESS;
		SD_API_STATUS transferStatus = m_FastPathStatus>=0 ? SD_API_STATUS_SUCCESS : m_FastPathStatus;
#endif

		if (NoResponse != pRequest->CommandResponse.ResponseType) {
			// Copy response over to request structure. Note that this is a 
			// bus driver buffer, so we do not need to SetProcPermissions 
			// or __try/__except.
			UNALIGNED DWORD *pdwResponseBuffer = 
				(PDWORD) (pRequest->CommandResponse.ResponseBuffer + 1); // Skip CRC
			WORD wCommand = ReadWord(SDHC_COMMAND);

			if ((wCommand & CMD_RESPONSE_R1B_R5B) == CMD_RESPONSE_R1B_R5B) {
				// wait for Transfer Complete status, which indicate the busy wait is over.
				// we should not handle this TXN_COMPLETE interrupt in IST in this case

				/*
				   RETAILMSG(0,(TEXT("HandleCommandComplete Status: 0x%X\n"),ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE)));				
				   RETAILMSG(0,(TEXT("HandleCommandComplete Signal: 0x%X\n"),ReadWord(SDHC_NORMAL_INT_SIGNAL_ENABLE)));								
				   BOOL fSuccess = WaitForReg<WORD>(ReadWord, SDHC_NORMAL_INT_STATUS, 
				   NORMAL_INT_STATUS_TRX_COMPLETE, NORMAL_INT_STATUS_TRX_COMPLETE, 5000);
				   RETAILMSG(0, (_T("%s Go waiting for NORMAL_INT_STATUS_TRX_COMPLETE\r\n"),
				   pszFname));					
				   if (!fSuccess) {
				   RETAILMSG(0, (_T("%s Timeout waiting for NORMAL_INT_STATUS_TRX_COMPLETE\r\n"),
				   pszFname));					
				   DEBUGMSG(SDCARD_ZONE_ERROR, (_T("%s Timeout waiting for NORMAL_INT_STATUS_TRX_COMPLETE\r\n"),
				   pszFname));
				   transferStatus = SD_API_STATUS_RESPONSE_TIMEOUT;
				   }*/

				RETAILMSG(0,(TEXT("CSDHCSlotBase::HandleCommandComplete    SoftwareReset\n")));	
				// Reset cmd and dat circuits
				SoftwareReset(SOFT_RESET_CMD | SOFT_RESET_DAT);
			}

			pdwResponseBuffer[0] = ReadDword(SDHC_R0);
			RETAILMSG(0,(TEXT("pdwResponseBuffer[0]= 0x%X\n"),pdwResponseBuffer[0]));
			if (pRequest->CommandResponse.ResponseType == ResponseR2) {
				pdwResponseBuffer[1] = ReadDword(SDHC_R2);
				pdwResponseBuffer[2] = ReadDword(SDHC_R4);
				pdwResponseBuffer[3] = ReadDword(SDHC_R6);
				RETAILMSG(0,(TEXT("pdwResponseBuffer[1]= 0x%X\n"),pdwResponseBuffer[1]));
				RETAILMSG(0,(TEXT("pdwResponseBuffer[2]= 0x%X\n"),pdwResponseBuffer[2]));
				RETAILMSG(0,(TEXT("pdwResponseBuffer[3]= 0x%X\n"),pdwResponseBuffer[3]));			
			}
		}

		// check for command/response only
		if (TRANSFER_IS_COMMAND_ONLY(pRequest)) {
			IndicateBusRequestComplete(pRequest, transferStatus);
			fRet = TRUE;
		} else {
			// handle data phase transfer
			//pIOPreg->GPFDAT = (0x1<<7);
			//WaitForReg<WORD>(ReadWord, SDHC_NORMAL_INT_STATUS, NORMAL_INT_STATUS_TRX_COMPLETE, NORMAL_INT_STATUS_TRX_COMPLETE); 

			/*
			   do{
			   }while ((ReadWord(SDHC_NORMAL_INT_STATUS)&NORMAL_INT_STATUS_TRX_COMPLETE) != NORMAL_INT_STATUS_TRX_COMPLETE);		 */
			pRequest->HCParam = 0;
			fRet = FALSE;
		}
	}
	// else request must have been canceled due to an error

	return fRet;
}


VOID 
CSDHCSlotBase::HandleErrors(
		)
{
	SD_API_STATUS status = SD_API_STATUS_SUCCESS;

#if (BSP_TYPE == BSP_SMDK2443)
	// get the current request  
	PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

#elif (BSP_TYPE == BSP_SMDK2450)
	// 2007.06.10
	// S/W workaround for the following problem
	// Problem : When the interrupt is occurred, it is late to update the interrupt status register related to the interrupt source
	while((ReadWord(SDHC_ERROR_INT_STATUS) == 0));

	// get the current request
	PSD_BUS_REQUEST pRequest = NULL;
	pRequest = GetAndLockCurrentRequest();
#endif

	WORD wErrorStatus = ReadWord(SDHC_ERROR_INT_STATUS);
	DEBUGMSG(SDCARD_ZONE_ERROR, 
			(TEXT("HandleErrors - ERROR INT STATUS=0x%02X\n"), wErrorStatus));
	if (pRequest) {
		DumpRequest(pRequest, SDCARD_ZONE_ERROR);
		// 08.04.27 by KYS
  		// For simple debugging, we have to confirm the command number!
  		RETAILMSG(1, (TEXT("ERR CMD:%d : "), pRequest->CommandCode));
	}

	DEBUGCHK( (wErrorStatus & ERR_INT_STATUS_VENDOR) == 0 );

	if (wErrorStatus) {
		if ( wErrorStatus & ERR_INT_STATUS_CMD_TIMEOUT ) {
			status = SD_API_STATUS_RESPONSE_TIMEOUT;
			switch(pRequest->CommandCode) {
				case 1 : RETAILMSG(1, (TEXT("If the card is not a MMC, CMD 1 does not work in reason.\n")));
					break;
				case 5 : RETAILMSG(1, (TEXT("If the card is not a SDIO, CMD 5 does not work in reason.\n")));
					break;
				case 8 : RETAILMSG(1, (TEXT("If the card is not SD SPEC 2.0, CMD 8 does not work in reason.\n")));
					break;
				default :
					RETAILMSG(1, (TEXT("[HSMMC1] CMD Timeout Error...\n")));
					break;
			}
		}

		if ( wErrorStatus & ERR_INT_STATUS_CMD_CRC ) {
			status = SD_API_STATUS_CRC_ERROR;
			RETAILMSG(1, (TEXT("[HSMMC1] CMD CRC Error...\n")));
			if ( wErrorStatus & ERR_INT_STATUS_CMD_TIMEOUT )
				status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
			RETAILMSG(1, (TEXT("[HSMMC1] CMD Timeout Error...\n")));
		}

		if ( wErrorStatus & ERR_INT_STATUS_CMD_ENDBIT ) {
			status = SD_API_STATUS_RESPONSE_TIMEOUT;
			RETAILMSG(1, (TEXT("[HSMMC1] CMD ENDBIT Error...\n")));
		}

		if ( wErrorStatus & ERR_INT_STATUS_CMD_INDEX ) {
			status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
			RETAILMSG(1, (TEXT("[HSMMC1] CMD INDEX Error...\n")));
		}

		if ( wErrorStatus & ERR_INT_STATUS_DAT_TIMEOUT ) {
			status = SD_API_STATUS_DATA_TIMEOUT;
			RETAILMSG(1, (TEXT("[HSMMC1] DAT Timeout Error...\n")));
		}

		if ( wErrorStatus & ERR_INT_STATUS_DAT_CRC ) {
			status = SD_API_STATUS_CRC_ERROR;
			RETAILMSG(1, (TEXT("[HSMMC1] DAT CRC Error...\n")));
		}

		if ( wErrorStatus & ERR_INT_STATUS_DAT_ENDBIT ) {
			status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
			RETAILMSG(1, (TEXT("[HSMMC1] DAT ENDBIT Error...\n")));
		}

		if ( wErrorStatus & ERR_INT_STATUS_BUS_POWER ) {
			status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
			RETAILMSG(1, (TEXT("[HSMMC1] BUS POWER Error...\n")));
		}

		if ( wErrorStatus & ERR_INT_STATUS_AUTOCMD12 ) {
			status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
			RETAILMSG(1, (TEXT("[HSMMC1] AUTOCMD12 Error...\n")));
		}

		// Perform basic error recovery
		WORD wErrIntSignal = ReadWord(SDHC_ERROR_INT_SIGNAL_ENABLE);
		WriteWord(SDHC_ERROR_INT_SIGNAL_ENABLE, 0);
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
		WORD wErrIntStatusEn = ReadWord(SDHC_ERROR_INT_STATUS_ENABLE);
		WriteWord(SDHC_ERROR_INT_STATUS_ENABLE,0);
#endif
		if (IS_CMD_LINE_ERROR(wErrorStatus)) {
			// Reset CMD line
			DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("HandleErrors - Command line error (0x%x). Resetting CMD line.\r\n"),
						wErrorStatus));
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
			do {
				WriteWord(SDHC_ERROR_INT_STATUS, wErrorStatus);
			} while(ReadWord(SDHC_ERROR_INT_STATUS) && (ReadWord(SDHC_NORMAL_INT_STATUS) & NORMAL_INT_STATUS_ERROR_INT));
#endif
			SoftwareReset(SOFT_RESET_CMD);
		}

		if (IS_DAT_LINE_ERROR(wErrorStatus)) {
			// Reset DAT line
			DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("HandleErrors - Data line error (0x%x). Resetting DAT line.\r\n"),
						wErrorStatus));
			SoftwareReset(SOFT_RESET_DAT);
		}

		// clear all error status
#if (BSP_TYPE == BSP_SMDK2443)
		WriteWord(SDHC_ERROR_INT_STATUS, wErrorStatus);

#elif (BSP_TYPE == BSP_SMDK2450)
		do {
			WriteWord(SDHC_ERROR_INT_STATUS, wErrorStatus);
		}while(ReadWord(SDHC_ERROR_INT_STATUS));	// KYS
#endif

		// re-enable error interrupt signals
		WriteWord(SDHC_ERROR_INT_SIGNAL_ENABLE, wErrIntSignal);
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
		WriteWord(SDHC_ERROR_INT_STATUS_ENABLE, wErrIntStatusEn);
#endif
		RETAILMSG(0,(TEXT("Error Handler : 0x%X\n"),ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE)));
		// complete the request
		if (pRequest) {
			IndicateBusRequestComplete(pRequest, status);
		}
		else
		{
			WORD wIntEnable = ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE);
			wIntEnable |= NORMAL_INT_ENABLE_CMD_COMPLETE | NORMAL_INT_ENABLE_TRX_COMPLETE;
			WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE,wIntEnable);
			RETAILMSG(0,(TEXT("Error Handler : 0x%X\n"),ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE)));		
		}
	}
}


VOID 
CSDHCSlotBase::HandleTransferDone(
		)
{
	PSD_BUS_REQUEST pRequest;
	SD_API_STATUS   status = SD_API_STATUS_SUCCESS;

	// get the current request  
	pRequest = GetAndLockCurrentRequest();
	RETAILMSG(0,(TEXT("CSDHCSlotBase::HandleTransferDone\n")));
	if (pRequest) {
		if (!TRANSFER_IS_COMMAND_ONLY(pRequest)) {
			if (UseDmaForRequest(pRequest)) {
				DWORD cbTransfer = TRANSFER_SIZE(pRequest);
				DWORD cbRemainder = cbTransfer % CB_DMA_PAGE;

				if (cbRemainder == 0) {
					cbRemainder = CB_DMA_PAGE;
				}
				pRequest->HCParam += cbRemainder;
				RETAILMSG(0,(TEXT("pRequest->HCParam=%d, cbTransfer=%d, PAGE_SIZE=%d\n"),pRequest->HCParam,cbTransfer,PAGE_SIZE));				
				//pIOPreg->GPFDAT &= ~(0x1<<7);
				if (TRANSFER_IS_READ(pRequest)) {
					BOOL fNoException;
					DEBUGCHK(cbTransfer <= CB_DMA_BUFFER);

					SD_SET_PROC_PERMISSIONS_FROM_REQUEST(pRequest) {               
						fNoException = SDPerformSafeCopy(pRequest->pBlockBuffer,
								m_pbDmaBuffer, cbTransfer);
					} SD_RESTORE_PROC_PERMISSIONS();

					if (fNoException == FALSE) {
						status = SD_API_STATUS_ACCESS_VIOLATION;
					}
				}
			}
		}

		if (pRequest->HCParam != TRANSFER_SIZE(pRequest)) {
			// This means that a Command Complete interrupt occurred before
			// a Buffer Ready interrupt. Hardware should not allow this. 
			RETAILMSG(1,(TEXT("pRequest->HCParam != TRANSFER_SIZE(pRequest)\n")));
			DEBUGCHK(FALSE);
			status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
		}

		// complete the request
		//if ((pRequest->Flags & SD_AUTO_ISSUE_CMD12) && *((DWORD *)(pRequest->hDevice)) != 0) {
		if (pRequest->Flags & SD_AUTO_ISSUE_CMD12) {        
			RETAILMSG(0,(TEXT("SDSDSDCARD\n")));
			m_fAutoCMD12Success = TRUE;
		}
		IndicateBusRequestComplete(pRequest, status);
	}
	// else request must have been canceled due to an error
}


VOID 
CSDHCSlotBase::HandleReadReady(
		)
{
	DEBUGMSG(SDHC_RECEIVE_ZONE, (TEXT("HandleReadReady - HandleReadReady!\n"))); 

	// get the current request  
	PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

	if (pRequest) {
		DEBUGCHK(!UseDmaForRequest(pRequest));
		DEBUGCHK(pRequest->NumBlocks > 0);
		DEBUGCHK(pRequest->HCParam < TRANSFER_SIZE(pRequest));
		DEBUGCHK(TRANSFER_IS_READ(pRequest));

#ifdef DEBUG
		++m_dwReadyInts;
#endif

		DWORD dwOldPermissions = SetProcPermissions(pRequest->CurrentPermissions);
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
		BOOL fKMode = SetKMode( TRUE );
#endif

		__try {
			PDWORD pdwUserBuffer = (PDWORD) &pRequest->pBlockBuffer[pRequest->HCParam];
			PDWORD pdwBuffer = pdwUserBuffer;
			DWORD  rgdwIntermediateBuffer[SDHC_MAX_BLOCK_LENGTH / sizeof(DWORD)];
			BOOL   fUsingIntermediateBuffer = FALSE;
			DWORD  cDwords = pRequest->BlockSize / 4;
			DWORD  dwRemainder = pRequest->BlockSize % 4;

			PREFAST_DEBUGCHK(sizeof(rgdwIntermediateBuffer) >= pRequest->BlockSize);

			if (((DWORD) pdwUserBuffer) % 4 != 0) {
				// Buffer is not DWORD aligned so we must use an
				// intermediate buffer.
				pdwBuffer = rgdwIntermediateBuffer;
				fUsingIntermediateBuffer = TRUE;
			}

			DWORD dwDwordsRemaining = cDwords;
			pRequest->HCParam += dwDwordsRemaining * 4;

			// Read the data from the device
			while ( dwDwordsRemaining-- ) {
				*(pdwBuffer++) = ReadDword(SDHC_BUFFER_DATA_PORT_0);
			}

			if ( dwRemainder != 0 ) {
				DWORD dwLastWord = ReadDword(SDHC_BUFFER_DATA_PORT_0);
				memcpy(pdwBuffer, &dwLastWord, dwRemainder);
				pRequest->HCParam += dwRemainder;
			}

			if (fUsingIntermediateBuffer) {
				memcpy(pdwUserBuffer, rgdwIntermediateBuffer, pRequest->BlockSize);
			}
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
			SetKMode( fKMode );
#endif
			SetProcPermissions(dwOldPermissions);
		}
		__except(SDProcessException(GetExceptionInformation())) {
			DEBUGMSG(SDCARD_ZONE_ERROR, (_T("Exception reading from client buffer!\r\n")));
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
			SetKMode( fKMode );
#endif
			SetProcPermissions(dwOldPermissions);
			IndicateBusRequestComplete(pRequest, SD_API_STATUS_ACCESS_VIOLATION);
		}

		DEBUGCHK(pRequest->HCParam == (m_dwReadyInts * pRequest->BlockSize));
	}
	// else request must have been canceled due to an error
}


VOID 
CSDHCSlotBase::HandleWriteReady(
		)
{    
	DEBUGMSG(SDHC_TRANSMIT_ZONE, (TEXT("HandleWriteReady - HandleWriteReady! \n"))); 

	// get the current request  
	PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

	if (pRequest) {
		DEBUGCHK(TRANSFER_IS_WRITE(pRequest));
		DEBUGCHK(!UseDmaForRequest(pRequest));
		DEBUGCHK(pRequest->NumBlocks > 0);
		DEBUGCHK(pRequest->HCParam < TRANSFER_SIZE(pRequest));

#ifdef DEBUG
		++m_dwReadyInts;
#endif

		DWORD dwOldPermissions = SetProcPermissions(pRequest->CurrentPermissions);
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
		BOOL fKMode = SetKMode( TRUE );
#endif

		__try {
			PDWORD pdwUserBuffer = (PDWORD) &pRequest->pBlockBuffer[pRequest->HCParam];
			PDWORD pdwBuffer = pdwUserBuffer;
			DWORD  rgdwIntermediateBuffer[SDHC_MAX_BLOCK_LENGTH / sizeof(DWORD)];
			BOOL   fUsingIntermediateBuffer = FALSE;
			DWORD  cDwords = pRequest->BlockSize / 4;
			DWORD  dwRemainder = pRequest->BlockSize % 4;

			PREFAST_DEBUGCHK(sizeof(rgdwIntermediateBuffer) >= pRequest->BlockSize);

			if (((DWORD) pdwUserBuffer) % 4 != 0) {
				// Buffer is not DWORD aligned so we must use an
				// intermediate buffer.
				pdwBuffer = rgdwIntermediateBuffer;
				memcpy(rgdwIntermediateBuffer, pdwUserBuffer, pRequest->BlockSize);
			}

			DWORD dwDwordsRemaining = cDwords;
			pRequest->HCParam += dwDwordsRemaining * 4;

			// Write data to buffer data port
			while ( dwDwordsRemaining-- ) {
				WriteDword(SDHC_BUFFER_DATA_PORT_0, *(pdwBuffer++));
			}

			if ( dwRemainder != 0 ) {
				DWORD dwLastWord = 0;
				memcpy(&dwLastWord, pdwBuffer, dwRemainder);
				WriteDword(SDHC_BUFFER_DATA_PORT_0, dwLastWord);
				pRequest->HCParam += dwRemainder;
			}

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
			SetKMode( fKMode );
#endif
			SetProcPermissions(dwOldPermissions);
		}
		__except(SDProcessException(GetExceptionInformation())) {
			DEBUGMSG(SDCARD_ZONE_ERROR, (_T("Exception reading from client buffer!\r\n")));
#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
			SetKMode( fKMode );
#endif
			SetProcPermissions(dwOldPermissions);
			IndicateBusRequestComplete(pRequest, SD_API_STATUS_ACCESS_VIOLATION);
		}

		DEBUGCHK(pRequest->HCParam == (m_dwReadyInts * pRequest->BlockSize));
	}
	// else request must have been canceled due to an error
}


VOID 
CSDHCSlotBase::HandleDma(
		)
{
	// Get the current request
	PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

	if (pRequest) {
		DEBUGCHK(pRequest->NumBlocks > 0);
		DEBUGCHK(UseDmaForRequest(pRequest));
		RETAILMSG(0,(TEXT("CSDHCSlotBase::HandleDma\n")));
		RETAILMSG(0,(TEXT("pRequest->HCParam=%d, CB_DMA_PAGE=%d, TRANSFER_SIZE(pRequest)=%d\n"),pRequest->HCParam,CB_DMA_PAGE,TRANSFER_SIZE(pRequest)));
		DWORD dwNewHCParam = pRequest->HCParam + CB_DMA_PAGE;

		// Only update our transferred count if we have not completed the
		// transfer already, since some host controllers give an extra
		// DMA interrupt after the last block transfer.
		if (dwNewHCParam < TRANSFER_SIZE(pRequest)) {
			RETAILMSG(0,(TEXT("dwNewHCParam < TRANSFER_SIZE(pRequest)\n")));			
			pRequest->HCParam = dwNewHCParam;
			DWORD paCurrDmaBuffer = m_paDmaBuffer + pRequest->HCParam;
			DEBUGCHK( paCurrDmaBuffer < (m_paDmaBuffer + CB_DMA_BUFFER) );
			DEBUGCHK( paCurrDmaBuffer < (m_paDmaBuffer + TRANSFER_SIZE(pRequest)) );
			DEBUGCHK( paCurrDmaBuffer == ReadDword(SDHC_SYSTEMADDRESS_LO) );

			WriteDword(SDHC_SYSTEMADDRESS_LO, paCurrDmaBuffer);
		}
	}
	// else request must have been canceled due to an error
}


BOOL 
CSDHCSlotBase::UseDmaForRequest(
		PSD_BUS_REQUEST pRequest
		)
{
	PREFAST_DEBUGCHK(pRequest);

	BOOL fRet = FALSE;


	if (m_pbDmaBuffer) {
		DEBUGCHK(m_paDmaBuffer);

		if ( (pRequest->BlockSize % 4 == 0) &&
				(TRANSFER_SIZE(pRequest) <= CB_DMA_BUFFER) ) {
			DEBUGCHK(m_paDmaBuffer % 4 == 0);
#if (BSP_TYPE == BSP_SMDK2443)
			DEBUGCHK(PAGE_SIZE == CB_DMA_PAGE);

#elif (BSP_TYPE == BSP_SMDK2450)
			DEBUGCHK((PAGE_SIZE*128) == CB_DMA_PAGE);
#endif
			fRet = TRUE;
		}
	}

	return fRet;
}


PVOID
CSDHCSlotBase::AllocPhysBuffer(
		size_t cb,
		PDWORD pdwPhysAddr
		)
{
	PVOID pvUncached;
	PVOID pvRet = NULL;
	DWORD dwPhysAddr;

	pvUncached = AllocPhysMem(cb, PAGE_READWRITE, 0, 0, &dwPhysAddr);

	if (pvUncached) {
		*pdwPhysAddr = dwPhysAddr;
		pvRet = pvUncached;
	}

	return pvRet;
}


VOID
CSDHCSlotBase::FreePhysBuffer(
		PVOID pv
		)
{
	BOOL fSuccess;

	DEBUGCHK(pv);

	fSuccess = FreePhysMem(pv);
	DEBUGCHK(fSuccess);
}


VOID
CSDHCSlotBase::SetHardwarePowerState(
		CEDEVICE_POWER_STATE cpsNew
		)
{
	DEBUGCHK(VALID_DX(cpsNew));
	DEBUGCHK(!DX_D1_OR_D2(cpsNew));

	DEBUGCHK(m_cpsCurrent != cpsNew);
	CEDEVICE_POWER_STATE cpsCurrent = m_cpsCurrent;
	m_cpsCurrent = cpsNew;
	BYTE bWakeupControl = m_bWakeupControl;
	RETAILMSG(0, (TEXT("CSDHCSlotBase::SetHardwarePowerState 0x%08X\n"),this));
	if (cpsCurrent == D0) {
		SDClockOff();

		if (cpsNew == D3) {
			if ( m_fSDIOInterruptsEnabled &&  
					(bWakeupControl & WAKEUP_INTERRUPT) ) {
				DEBUGCHK(m_fCardPresent);
				m_fSleepsWithPower = TRUE;
				m_fPowerUpDisabledInts = FALSE;
			}
			else {
				// Wake on status changes only
				WriteByte(SDHC_POWER_CONTROL, 0);
				bWakeupControl &= ~WAKEUP_INTERRUPT;
			}

			// enable wakeup sources
			m_wIntSignals = ReadWord(SDHC_NORMAL_INT_SIGNAL_ENABLE);
			WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE, 0);
			WriteWord(SDHC_NORMAL_INT_STATUS, ReadWord(SDHC_NORMAL_INT_STATUS));
			WriteByte(SDHC_WAKEUP_CONTROL, bWakeupControl);
		}
		else {
			DEBUGCHK(cpsNew == D4);
			WriteByte(SDHC_CLOCK_CONTROL, 0);
			WriteByte(SDHC_POWER_CONTROL, 0);
		}
	}
	else if (cpsCurrent == D3) {
		// Coming out of wakeup state
		if (cpsNew == D0) {            
			WriteByte(SDHC_WAKEUP_CONTROL, 0);

			if (!m_fSleepsWithPower) {
				// Power was turned off to the socket. Re-enumerate card.
				if (m_fCardPresent) {
					HandleRemoval(TRUE);
				}

				m_fCheckSlot = TRUE;
				SetInterruptEvent();
			}
			else {
				if (m_fCardPresent) {
					// Do not do this if the card was removed or 
					// if power was not kept.
					if (m_fPowerUpDisabledInts) {
						EnableSDIOInterrupts(TRUE);
					}
				}
			}

			WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE, m_wIntSignals);
		}
		else {
			DEBUGCHK(cpsNew == D4);
			WriteByte(SDHC_CLOCK_CONTROL, 0);
			WriteByte(SDHC_WAKEUP_CONTROL, 0);
			WriteByte(SDHC_POWER_CONTROL, 0);
			WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE, m_wIntSignals);
		}

		m_fSleepsWithPower = FALSE;
	}
	else {
		DEBUGCHK(cpsCurrent == D4);

		// Coming out of unpowered state - signal card removal
		// so any card present will be re-enumerated.
		//
		// We do the same thing when we go to D3 as D0 because
		// the slot has lost power so it could have been removed
		// or changed. In other words, D3 is a meaningless state
		// after D4.
		m_cpsCurrent = D0; // Force to D0

		// Do not call HandleRemoval here because it could cause
		// a context switch in a PowerUp callback.
		m_fFakeCardRemoval = TRUE;

		m_fCheckSlot = TRUE;

		SetInterruptEvent();
	}
}



#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
// 08.02.03 by KYS
// In order to prevent infinite CARD INT occuring, below code is needed because of the architecture of HSMMC on s3c2450.
// this process was copied from CE old version.
#endif
	BOOL
CSDHCSlotBase::IsOnlySDIOInterrupt()
{
	WORD wIntStatus = ReadWord(SDHC_NORMAL_INT_STATUS);
	if (wIntStatus == NORMAL_INT_STATUS_CARD_INT && isSDIOInterrupt == TRUE ) {
		return TRUE;
	}
	return FALSE;
}

#if (BSP_TYPE == BSP_SMDK2443)
VOID
CSDHCSlotBase::DoEnableSDIOInterrupts(
		BOOL fEnable
		)
{
	WORD wIntStatusEn = ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE);
	if (fEnable) {
		isSDIOInterrupt = FALSE;
		RETAILMSG(0,(TEXT("DoEnableSDIOInterrupts TRUE\n")));		
		wIntStatusEn |= NORMAL_INT_ENABLE_CARD_INT;
	}
	else {
		isSDIOInterrupt = TRUE;		
		RETAILMSG(0,(TEXT("DoEnableSDIOInterrupts FALSE\n")));		
		wIntStatusEn &= (~NORMAL_INT_ENABLE_CARD_INT);      
	}

	WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, wIntStatusEn);
}

#elif (BSP_TYPE == BSP_SMDK2450)
VOID
CSDHCSlotBase::DoEnableSDIOInterrupts(
		BOOL fEnable
		)
{
	WORD wIntSignalEn = ReadWord(SDHC_NORMAL_INT_SIGNAL_ENABLE);
	if (fEnable) {
		isSDIOInterrupt = FALSE;
		wIntSignalEn |= NORMAL_INT_SIGNAL_CARD_INT;
	} else {
		isSDIOInterrupt = TRUE;
		wIntSignalEn &= (~NORMAL_INT_SIGNAL_CARD_INT);      
	}
	WriteWord(SDHC_NORMAL_INT_SIGNAL_ENABLE, wIntSignalEn);

	WORD wIntStatusEn = ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE);
	if (fEnable) {
		wIntStatusEn |= NORMAL_INT_ENABLE_CARD_INT;
	} else {
		wIntStatusEn &= (~NORMAL_INT_ENABLE_CARD_INT);      
	}
	WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, wIntStatusEn);
}
#endif


template<class T>
BOOL
CSDHCSlotBase::WaitForReg(
		T (CSDHCSlotBase::*pfnReadReg)(DWORD),
		DWORD dwRegOffset,
		T tMask,
		T tWaitForEqual,
		DWORD dwTimeout
		)
{
	SETFNAME();

	const DWORD dwStart = GetTickCount();

	T tValue;

	BOOL fRet = TRUE;
	DWORD dwIteration = 1;

	// Verify that reset has completed.
	do {
		tValue = (this->*pfnReadReg)(dwRegOffset);

		if ( (dwIteration % 16) == 0 ) {
			// Check time
			DWORD dwCurr = GetTickCount();

			// Unsigned arithmetic handles rollover.
			DWORD dwTotal = dwCurr - dwStart;

			if (dwTotal > dwTimeout) {
				// Timeout
				fRet = FALSE;
				DEBUGMSG(SDCARD_ZONE_WARN, (_T("%s Timeout (%u ms) waiting for (ReadReg<%u>(0x%02x) & 0x%08x) == 0x%08x\r\n"),
							pszFname, dwTimeout, sizeof(T), dwRegOffset, tMask, tWaitForEqual));
				break;
			}
		}

		++dwIteration;
	} while ((tValue & tMask) != tWaitForEqual);

	return fRet;
}


BOOL 
CSDHCSlotBase::SoftwareReset(
		BYTE bResetBits
		)
{
	SETFNAME();

	// Reset the controller
	WriteByte(SDHC_SOFT_RESET, bResetBits);
	BOOL fSuccess = WaitForReg<BYTE>(ReadByte, SDHC_SOFT_RESET, bResetBits, 0);
	if (!fSuccess) {
		DEBUGMSG(SDCARD_ZONE_ERROR, (_T("%s Timeout waiting for controller reset - 0x%02x\r\n"),
					pszFname, bResetBits));
	}


#if (BSP_TYPE == BSP_SMDK2443)
	// set Clock Source to EPLL	added by JJG	06.10.11
	WriteDword(SDHC_CONTROL2, (0x1<<15)|(0x1<<14)|(0x1<<8)|(0x2/*EPLL*/<<4) );
	WriteDword(SDHC_CONTROL3, (0<<31) | (1<<23) | (0<<15) | (1<<7) );

#elif (BSP_TYPE == BSP_SMDK2450)
	WriteDword(SDHC_CONTROL2, (ReadDword(SDHC_CONTROL2)|(0x1<<30)));	// Enable Command Conflict Mask
#endif

	return fSuccess;
}


VOID
CSDHCSlotBase::EnableLED(
		BOOL fEnable
		)
{
	BYTE bHostControl = ReadByte(SDHC_HOST_CONTROL);

	if (fEnable) {
		bHostControl |= HOSTCTL_LED_CONTROL;
	}
	else {
		bHostControl &= ~HOSTCTL_LED_CONTROL;
	}

	WriteByte(SDHC_HOST_CONTROL, bHostControl);
}


VOID 
CSDHCSlotBase::IndicateSlotStateChange(SD_SLOT_EVENT sdEvent) {
	SDHCDIndicateSlotStateChange(m_pHCDContext,
			(UCHAR) m_dwSlot, sdEvent);
}


PSD_BUS_REQUEST
CSDHCSlotBase::GetAndLockCurrentRequest() {
	return SDHCDGetAndLockCurrentRequest(m_pHCDContext,
			(UCHAR) m_dwSlot);
}


VOID CSDHCSlotBase::PowerUpDown(BOOL fPowerUp, BOOL fKeepPower) {
	SDHCDPowerUpDown(m_pHCDContext, fPowerUp, fKeepPower,
			(UCHAR) m_dwSlot);

}


VOID 
CSDHCSlotBase::IndicateBusRequestComplete(
		PSD_BUS_REQUEST pRequest,
		SD_API_STATUS status
		)
{
	RETAILMSG(0,(TEXT("CSDHCSlotBase::IndicateBusRequestComplete\n")));
	const WORD c_wTransferIntSources = 
		NORMAL_INT_STATUS_CMD_COMPLETE | 
		NORMAL_INT_STATUS_TRX_COMPLETE |
		NORMAL_INT_STATUS_DMA |
		NORMAL_INT_STATUS_BUF_WRITE_RDY | 
		NORMAL_INT_STATUS_BUF_READ_RDY;

	DEBUGCHK(pRequest);

	if ( (m_fSDIOInterruptsEnabled && m_f4BitMode) == FALSE ) {
		SDClockOff();
	}
	// else need to leave clock on in order to receive interrupts in 4 bit mode

	// Turn off LED.
#if (BSP_TYPE == BSP_SMDK2443)
	EnableLED(TRUE);

#elif (BSP_TYPE == BSP_SMDK2450)
	EnableLED(FALSE);
#endif

	// Turn off interrupt sources
	WORD wIntStatusEn = ReadWord(SDHC_NORMAL_INT_STATUS_ENABLE);
	wIntStatusEn &= ~c_wTransferIntSources;
	WriteWord(SDHC_NORMAL_INT_STATUS_ENABLE, wIntStatusEn);

	// Clear any remaining spurious interrupts.
#if (BSP_TYPE == BSP_SMDK2443)
	WriteWord(SDHC_NORMAL_INT_STATUS, c_wTransferIntSources);

#elif (BSP_TYPE == BSP_SMDK2450)
	do {
		WriteWord(SDHC_NORMAL_INT_STATUS, c_wTransferIntSources);
	} while(ReadWord(SDHC_NORMAL_INT_STATUS) & c_wTransferIntSources);
#endif

#ifdef DEBUG
	m_dwReadyInts = 0;
#endif

	m_fCommandCompleteOccurred = FALSE;

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
	ASSERT(m_pCurrentRequest!=NULL);
	m_pCurrentRequest = NULL;
	if (m_fCurrentRequestFastPath) {
		if (status == SD_API_STATUS_SUCCESS) {
			status = SD_API_STATUS_FAST_PATH_SUCCESS;
		}
		m_FastPathStatus = status;
	}
	else
#endif
		SDHCDIndicateBusRequestComplete(m_pHCDContext, pRequest, status);

}


#if (BSP_TYPE == BSP_SMDK2443)
VOID 
CSDHCSlotBase::SetClockRate(
		PDWORD pdwRate
		)
{
	RETAILMSG(0,(TEXT("DEBUGCHK(m_dwMaxClockRate)\n")));
	DEBUGCHK(m_dwMaxClockRate);
	RETAILMSG(0,(TEXT("DEBUGCHK(m_dwMaxClockRate) success\n")));

	DWORD dwClockRate = *pdwRate;
	DWORD       dwMaxClockRate = m_dwMaxClockRate;
	int         i = 0; // 2^i is the divisor value


	//   For supporting lower Clock
	//	added by JJG 06.10.16
	if(*pdwRate <= 400000)
	{
		dwMaxClockRate = 12000000;		// 12Mhz
		WriteDword(SDHC_CONTROL2, (0x1<<15)|(0x1<<14)|(0x1<<8)|(0x3/*EXT*/<<4) );
	}
	else if(*pdwRate <= 20000000)
	{
		dwMaxClockRate = 133000000;//S3C2450_HCLK;		// maybe 133Mhz		
		WriteDword(SDHC_CONTROL2, (0x1<<15)|(0x1<<14)|(0x1<<8)|(0x1/*EPLL*/<<4) );	
	}
	else
	{		
		WriteDword(SDHC_CONTROL2, (0x1<<15)|(0x1<<14)|(0x1<<8)|(0x2/*EPLL*/<<4) );	
	}
	/////////////////////////////////////////////

	// shift MaxClockRate until we find the closest frequency <= target
	while ( (dwClockRate < dwMaxClockRate) && ( i < 8 ) ) {
		dwMaxClockRate = dwMaxClockRate >> 1;
		i++;
	}

	// set the actual clock rate 
	*pdwRate = dwMaxClockRate;
	m_wRegClockControl = CLOCK_INTERNAL_ENABLE;

	if (i != 0) {
		DWORD dwDivisor = 1 << (i - 1);
		m_wRegClockControl |= (dwDivisor << 8);
	}

	RETAILMSG(1,(TEXT("SDHCSetRate - Clock Control Reg = %X\n"), 
				m_wRegClockControl));
	RETAILMSG(1,(TEXT("SDHCSetRate - Actual clock rate = %d\n"), *pdwRate));
	DEBUGMSG(SDHC_CLOCK_ZONE,(TEXT("SDHCSetRate - Clock Control Reg = %X\n"), 
				m_wRegClockControl));
	DEBUGMSG(SDHC_CLOCK_ZONE,(TEXT("SDHCSetRate - Actual clock rate = %d\n"), *pdwRate));
}

#elif (BSP_TYPE == BSP_SMDK2450)
VOID 
CSDHCSlotBase::SetClockRate(
		PDWORD pdwRate
		)
{
	DEBUGCHK(m_dwMaxClockRate);

	const DWORD dwClockRate = *pdwRate;
	DWORD       dwMaxClockRate = m_dwMaxClockRate;
	int         i = 0; // 2^i is the divisor value
	DWORD dwControlValue = 0;

	dwControlValue = (ReadDword(SDHC_CONTROL2)|(0x1<<30));	// Command Conflict Mask

	// 08.06.16 by KYS
	// Write Status Clear Async Mode Enable
	// and, if this bit is enabled, it must be needed that Status Busy bit checking for clearing the interrupt status register before "CMD ISSUE"!!
	dwControlValue |= (0x1<<31);

#ifndef _FB_ON_
	RETAILMSG(1,(TEXT("[HSMMC1] Turn OFF the F/B delay control.\r\n")));
	dwControlValue &= ~(0x3<<14); // Turn off the feedback clock delay
#else
	RETAILMSG(1,(TEXT("[HSMMC1] Turn ON the F/B delay control.\r\n")));
	dwControlValue &= ~(0x3<<14) | (0x1<<14); // Turn on the RX feedback clock delay
	//WriteWord(SDHC_CONTROL3, ((ReadWord(SDHC_CONTROL3) & ~(0x1<<15)) |(0x1<<7)));	
	WriteDword(SDHC_CONTROL3, ((ReadDword(SDHC_CONTROL3) |(0x1<<15)) |(0x1<<7)));
#endif

	dwControlValue |= ((0x3<<9) |  // Debounce Filter Count 0x3=64 iSDCLK
			(0x1<<8) |  // SDCLK Hold Enable
			(0x2<<4));  // Base Clock Source = EPLL out
	WriteDword(SDHC_CONTROL2,  dwControlValue);

	// shift MaxClockRate until we find the closest frequency <= target
	while ( (dwClockRate < dwMaxClockRate) && ( i < 8 ) ) {
		dwMaxClockRate = dwMaxClockRate >> 1;
		i++;
	}

	// set the actual clock rate 
	*pdwRate = dwMaxClockRate;
	m_wRegClockControl = CLOCK_INTERNAL_ENABLE;

	if (i != 0) {
		DWORD dwDivisor = 1 << (i - 1);
		m_wRegClockControl |= (dwDivisor << 8);
	}

	DEBUGMSG(SDHC_CLOCK_ZONE,(TEXT("SDHCSetRate - Clock Control Reg = %X\n"), 
				m_wRegClockControl));
	DEBUGMSG(SDHC_CLOCK_ZONE,(TEXT("SDHCSetRate - Actual clock rate = %d\n"), *pdwRate));
}
#endif  // !(BSP_TYPE == BSP_SMDK2443)


VOID 
CSDHCSlotBase::SDClockOn(
		)
{
	SETFNAME();

	// Must call SetClockRate() first to init m_wRegClockControl
	// We separate clock divisor calculation with ClockOn, so we can call 
	// ClockOn without recalcuating the divisor.
	WriteWord(SDHC_CLOCK_CONTROL, m_wRegClockControl);
	RETAILMSG(0,(TEXT("SDHC_CLOCK_CONTROL = 0x%X\n"),m_wRegClockControl));
	RETAILMSG(0,(TEXT("CONTROL2 = 0x%X\n"),ReadDword(SDHC_CONTROL2)));
	RETAILMSG(0,(TEXT("CONTROL3 = 0x%X\n"),ReadDword(SDHC_CONTROL3)));	

	// wait until clock stable
	BOOL fSuccess = WaitForReg<WORD>(ReadWord, SDHC_CLOCK_CONTROL, 
			CLOCK_STABLE, CLOCK_STABLE); 
	if (fSuccess) {
		RETAILMSG(0, (_T("%s Enable Clock\r\n"),	pszFname));	
		// enable it
		//WriteWord(SDHC_CLOCK_CONTROL, (0x20<<8) | CLOCK_ENABLE | CLOCK_INTERNAL_ENABLE);
		WriteWord(SDHC_CLOCK_CONTROL, m_wRegClockControl | CLOCK_ENABLE);
#if (BSP_TYPE == BSP_SMDK2443)
		fSuccess = WaitForReg<WORD>(ReadWord, SDHC_CLOCK_CONTROL, 
				CLOCK_EXTERNAL_ENABLE, CLOCK_EXTERNAL_ENABLE);
		RETAILMSG(0,(TEXT("fSuccess = 0x%X\n"),fSuccess));

#elif (BSP_TYPE == BSP_SMDK2450)
#endif
	}
	else {
		RETAILMSG(0, (_T("%s Timeout waiting for CLOCK_STABLE\r\n"),
					pszFname));		
		DEBUGMSG(SDCARD_ZONE_ERROR, (_T("%s Timeout waiting for CLOCK_STABLE\r\n"),
					pszFname));
	}
}


VOID 
CSDHCSlotBase::SDClockOff(
		)
{
	WriteWord(SDHC_CLOCK_CONTROL, m_wRegClockControl);
	RETAILMSG(0, (_T("CSDHCSlotBase::SDClockOff      Disable Clock\r\n")));		
}


DWORD
CSDHCSlotBase::DetermineVddWindows(
		)
{
	// Set voltage window mask  +- 10%
	SSDHC_CAPABILITIES caps = GetCapabilities();
	DWORD dwVddWindows = 0;

	if (caps.bits.Voltage18) {
		dwVddWindows |= SD_VDD_WINDOW_1_7_TO_1_8 | SD_VDD_WINDOW_1_8_TO_1_9;
	}
	if (caps.bits.Voltage30) {
		dwVddWindows |= SD_VDD_WINDOW_2_9_TO_3_0 | SD_VDD_WINDOW_3_0_TO_3_1;
	}
	if (caps.bits.Voltage33) {
		dwVddWindows |= SD_VDD_WINDOW_3_2_TO_3_3 | SD_VDD_WINDOW_3_3_TO_3_4;
	}

#if (BSP_TYPE == BSP_SMDK2443)

#elif (BSP_TYPE == BSP_SMDK2450)
	// 08.02.03 by KYS
	// For SDHC/MMC (not SD/MMCPlus) detection problem, below code line is must inserted.
#endif
	dwVddWindows = SD_VDD_WINDOW_3_2_TO_3_3 | SD_VDD_WINDOW_3_3_TO_3_4;
	return dwVddWindows;
}


DWORD
CSDHCSlotBase::GetDesiredVddWindow(
		)
{
	DEBUGCHK(m_dwVddWindows);

	DWORD dwDesiredVddWindow = 0;

	// Return lowest supportable voltage.
	if (m_dwVddWindows & SD_VDD_WINDOW_1_7_TO_1_8) {
		dwDesiredVddWindow = SD_VDD_WINDOW_1_7_TO_1_8;
	}
	else if (m_dwVddWindows & SD_VDD_WINDOW_2_9_TO_3_0) {
		dwDesiredVddWindow = SD_VDD_WINDOW_2_9_TO_3_0;
	}
	else if (m_dwVddWindows & SD_VDD_WINDOW_3_2_TO_3_3) {
		dwDesiredVddWindow = SD_VDD_WINDOW_3_2_TO_3_3;
	}

	return dwDesiredVddWindow;
}


DWORD
CSDHCSlotBase::GetMaxVddWindow(
		)
{
	DEBUGCHK(m_dwVddWindows);

	DWORD dwMaxVddWindow = 0;

	if (m_dwVddWindows & SD_VDD_WINDOW_3_2_TO_3_3) {
		dwMaxVddWindow = SD_VDD_WINDOW_3_2_TO_3_3;
	}
	else if (m_dwVddWindows & SD_VDD_WINDOW_2_9_TO_3_0) {
		dwMaxVddWindow = SD_VDD_WINDOW_2_9_TO_3_0;
	}
	else if (m_dwVddWindows & SD_VDD_WINDOW_1_7_TO_1_8) {
		dwMaxVddWindow = SD_VDD_WINDOW_1_7_TO_1_8;
	}

	return dwMaxVddWindow;
}


#ifdef DEBUG

VOID 
CSDHCSlotBase::Validate(
		)
{
	DEBUGCHK(m_pregDevice && m_pregDevice->IsOK());
	DEBUGCHK(m_dwSlot < SDHC_MAX_SLOTS);
	DEBUGCHK(m_pbRegisters);
	DEBUGCHK(m_pHCDContext);
	DEBUGCHK(m_pbDmaBuffer || m_paDmaBuffer == 0);
	DEBUGCHK(m_paDmaBuffer || m_pbDmaBuffer == 0);
	DEBUGCHK(VALID_DX(m_cpsCurrent));
	DEBUGCHK(VALID_DX(m_cpsAtPowerDown));    
}


VOID 
CSDHCSlotBase::DumpRegisters(
		)
{
	BYTE                rgbSdRegs[sizeof(SSDHC_REGISTERS)];
	SSDHC_REGISTERS    *pStdHCRegs = (SSDHC_REGISTERS *) rgbSdRegs;

	DEBUGCHK(sizeof(SSDHC_REGISTERS) % sizeof(DWORD) == 0);

	for (DWORD dwRegIndex = 0; dwRegIndex < dim(rgbSdRegs); ++dwRegIndex) {
		rgbSdRegs[dwRegIndex] = ReadByte(dwRegIndex);
	}

	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("+DumpStdHCRegs - Slot %d -------------------------\n"),
				m_dwSlot));

	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SystemAddressLo:    0x%04X \n"), pStdHCRegs->SystemAddressLo)); 
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SystemAddressHi:    0x%04X \n"), pStdHCRegs->SystemAddressHi));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("BlockSize:  0x%04X \n"), pStdHCRegs->BlockSize));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("BlockCount: 0x%04X \n"), pStdHCRegs->BlockCount));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("Argument0:  0x%04X \n"), pStdHCRegs->Argument0));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("Argument1:  0x%04X \n"), pStdHCRegs->Argument1));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("TransferMode:   0x%04X \n"), pStdHCRegs->TransferMode));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("Command:    0x%04X \n"), pStdHCRegs->Command));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("R0: 0x%04X \n"), pStdHCRegs->R0));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("R1: 0x%04X \n"), pStdHCRegs->R1));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("R2: 0x%04X \n"), pStdHCRegs->R2));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("R3: 0x%04X \n"), pStdHCRegs->R3));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("R4: 0x%04X \n"), pStdHCRegs->R4));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("R5: 0x%04X \n"), pStdHCRegs->R5));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("R6: 0x%04X \n"), pStdHCRegs->R6));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("R7: 0x%04X \n"), pStdHCRegs->R7));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("BufferDataPort0:    0x%04X \n"), pStdHCRegs->BufferDataPort0));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("BufferDataPort1:    0x%04X \n"), pStdHCRegs->BufferDataPort1));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("PresentState:   0x%04X \n"), pStdHCRegs->PresentState));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("HostControl:    0x%04X \n"), pStdHCRegs->HostControl));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("PowerControl:   0x%04X \n"), pStdHCRegs->PowerControl));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("BlockGapControl:    0x%04X \n"), pStdHCRegs->BlockGapControl));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("WakeUpControl:  0x%04X \n"), pStdHCRegs->WakeUpControl));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("ClockControl:   0x%04X \n"), pStdHCRegs->ClockControl));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("TimeOutControl: 0x%04X \n"), pStdHCRegs->TimeOutControl));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SoftReset:  0x%04X \n"), pStdHCRegs->SoftReset));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("NormalIntStatus:    0x%04X \n"), pStdHCRegs->NormalIntStatus));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("ErrorIntStatus: 0x%04X \n"), pStdHCRegs->ErrorIntStatus));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("NormalIntStatusEnable:  0x%04X \n"), pStdHCRegs->NormalIntStatusEnable));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("ErrorIntStatusEnable:   0x%04X \n"), pStdHCRegs->ErrorIntStatusEnable));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("NormalIntSignalEnable:  0x%04X \n"), pStdHCRegs->NormalIntSignalEnable));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("ErrorIntSignalEnable:   0x%04X \n"), pStdHCRegs->ErrorIntSignalEnable));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("AutoCMD12ErrorStatus:   0x%04X \n"), pStdHCRegs->AutoCMD12ErrorStatus));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("Capabilities:   0x%04X \n"), pStdHCRegs->Capabilities));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("MaxCurrentCapabilites:  0x%04X \n"), pStdHCRegs->MaxCurrentCapabilites));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SlotInterruptStatus:    0x%04X \n"), pStdHCRegs->SlotInterruptStatus));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("HostControllerVersion:  0x%04X \n"), pStdHCRegs->HostControllerVersion));
	DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("-DumpStdHCRegs-------------------------\n")));
}

#endif

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE


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

#ifndef _PWR_DRV_H_
#define _PWR_DRV_H_

#include <pm.h>
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <s3c2450.h>
#include <s3c2450_intr.h>
//#include <intr.h>

#define THRDPRI_BATT_FLT        100
#define THRDPRI_POWER           101

#define POWER_THREAD_PRIORITY    THRDPRI_POWER


typedef enum _PWR_STATE {

    UNINITIALIZED,
    INITIALIZE,
    RUN,
    SUSPEND,
    RESUME,
    ADC_ERROR,
    

} PWR_STATE, *PPWR_STATE;


#define PCF_SIG ' FCP'

#define VALID_CONTEXT( p ) \
   ( p && p->Sig && PCF_SIG == p->Sig )


//
// PWR_CONTEXT
//
#pragma pack(1)

typedef struct _PWR_CONTEXT {

    DWORD   Sig;    // Signature
    
    // Saved our OOCS & INTx registers during init/reset/resume 
    // because they are cleared on read, so we loose them.
    UCHAR ResetReg[4];
    BOOL  ResetRegReady;

	volatile S3C2450_IOPORT_REG *pIORegs;

    PWR_STATE   State;

    CRITICAL_SECTION RegCS;

    HANDLE          hI2C;   // I2C Bus Driver
    HANDLE          hUtil;   // UTIL Driver to map in VA
	
    HANDLE IST;
    HANDLE ISTEvent;

    HANDLE hBATT_FLT_IST;
    HANDLE hBATT_FLT_Event;

    DWORD OpenCount;

    //
    // System Global Events
    //
    HANDLE hADC;        // A-to-D Conversion Done
    HANDLE hTSCPRES;    // Touch Pressed

    CEDEVICE_POWER_STATE    Dx;

} PWR_CONTEXT, *PPWR_CONTEXT;

#pragma pack()




// Registry names 
#define REG_MODE_VAL_NAME TEXT("Mode") 
#define REG_MODE_VAL_LEN  sizeof( DWORD )

#define REG_SLAVEADDR_VAL_NAME TEXT("SlaveAddress") 
#define REG_SLAVEADDR_VAL_LEN  sizeof( DWORD )

#define REG_IRQ_VAL_NAME TEXT("IRQ") 
#define REG_IRQ_VAL_LEN  sizeof( DWORD )

#define REG_IOBASE_VAL_NAME TEXT("IoBase") 
#define REG_IOBASE_VAL_LEN  sizeof( DWORD )
#define REG_IOLEN_VAL_NAME TEXT("IoLen") 
#define REG_IOLEN_VAL_LEN  sizeof( DWORD )

#define REG_CONFIGBASE_VAL_NAME TEXT("ConfigBase") 
#define REG_CONFIGBASE_VAL_LEN  sizeof( DWORD )
#define REG_CONFIGLEN_VAL_NAME TEXT("ConfigLen") 
#define REG_CONFIGLEN_VAL_LEN  sizeof( DWORD )


//
// (logical) PDD functions
//
DWORD 
HW_Init(
    PPWR_CONTEXT pPWR
    );

DWORD 
HW_Deinit( 
    PPWR_CONTEXT pPWR
    );

DWORD 
HW_Open(
    PPWR_CONTEXT pPWR
    );

DWORD 
HW_Close(
    PPWR_CONTEXT pPWR
    );

DWORD 
HW_Read(
    PPWR_CONTEXT pPWR,  
    DWORD slvAddr,      // target slave address
    UCHAR addr,         // starting slave word address
    PUCHAR pData,       // buffer
    DWORD count         // nBytes to read
    );

DWORD 
HW_Write(
    PPWR_CONTEXT pPWR,  
    DWORD slvAddr,      // target slave address
    UCHAR addr,         // starting slave word address
    PUCHAR pData,       // buffer
    DWORD count         // nBytes to write
    );

BOOL
HW_IOControl(
    PPWR_CONTEXT pPWR,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
    );

BOOL
HW_PowerUp(
    PPWR_CONTEXT pPWR
    );

BOOL
HW_PowerDown(
    PPWR_CONTEXT pPWR
   );

BOOL
HW_PowerCapabilities(
    PPWR_CONTEXT pPWR,
    PPOWER_CAPABILITIES ppc
   );

BOOL
HW_PowerSet(
    PPWR_CONTEXT pPWR,
    PCEDEVICE_POWER_STATE pDx
   );

BOOL
HW_PowerGet(
    PPWR_CONTEXT pPWR,
    PCEDEVICE_POWER_STATE pDx
   );


#ifdef IMGEBOOT
#define DEBUGBREAK      DebugBreak
#else
#define DEBUGBREAK()
#endif

#ifndef SHIP_BUILD

DBGPARAM dpCurSettings;

#define ZONE_ERR            DEBUGZONE(0)
#define ZONE_WRN            DEBUGZONE(1)
#undef  ZONE_INIT
#define ZONE_INIT           DEBUGZONE(2)
#define ZONE_OPEN           DEBUGZONE(3)
#define ZONE_READ           DEBUGZONE(4)
#define ZONE_WRITE          DEBUGZONE(5)
#define ZONE_IOCTL          DEBUGZONE(6)
#define ZONE_IST            DEBUGZONE(7)
#define ZONE_REG            DEBUGZONE(8)
#define ZONE_REG_DUMP       DEBUGZONE(9)
#define ZONE_ADC            DEBUGZONE(10)
#define ZONE_BATT           DEBUGZONE(11)
#define ZONE_EVENTS         DEBUGZONE(12)
#define ZONE_POWER          DEBUGZONE(13)
//...
#define ZONE_TRACE          DEBUGZONE(15)

#define DUMP_INTx()                                           \
{                                                             \
    DEBUGMSG(ZONE_REG,(TEXT("INT1: 0x%.2x\r\n"), pPWR->Reg[INT1]));    \
    DEBUGMSG(ZONE_REG,(TEXT("INT2: 0x%.2x\r\n"), pPWR->Reg[INT2]));    \
    DEBUGMSG(ZONE_REG,(TEXT("INT3: 0x%.2x\r\n"), pPWR->Reg[INT3]));    \
}

#define DUMP_INTxM()                                            \
{                                                               \
    DEBUGMSG(ZONE_REG,(TEXT("INT1M: 0x%.2x\r\n"), pPWR->Reg[INT1M]));    \
    DEBUGMSG(ZONE_REG,(TEXT("INT2M: 0x%.2x\r\n"), pPWR->Reg[INT2M]));    \
    DEBUGMSG(ZONE_REG,(TEXT("INT3M: 0x%.2x\r\n"), pPWR->Reg[INT3M]));    \
}

#define DUMP_RTC()                                              \
{                                                               \
    DEBUGMSG(ZONE_REG,(TEXT("RTC.0[yr:mo:day:wkd] (hr:min:sec) : [%.2u:%.2u:%.2u:%.2u] (%.2u:%.2u:%.2u) \r\n"),   \
    FROM_BCD(pPWR->Reg[RTCYR])+2000, FROM_BCD(pPWR->Reg[RTCMT]), FROM_BCD(pPWR->Reg[RTCDT]), pPWR->Reg[RTCWD], \
    FROM_BCD(pPWR->Reg[RTCHR]), FROM_BCD(pPWR->Reg[RTCMN]), FROM_BCD(pPWR->Reg[RTCSC]) )); \
}

#define DUMP_REGS(StartReg, nRegs)                                                              \
{                                                                                               \
    DWORD i;                                                                                    \
    RETAILMSG(1,(TEXT("PCF50606 Reg [0x%X - 0x%X] { \r\n"), StartReg, StartReg+nRegs-1)); \
    for (i = StartReg; i < StartReg+nRegs; i++) {                                               \
        RETAILMSG(1,(TEXT("\t [0x%.2X] : 0x%.2x \r\n"), i, pPWR->Reg[i]));                        \
    }                                                                                           \
    RETAILMSG(1,(TEXT("} \r\n")));                                                              \
}

//
// these should be removed in the code if you can 'g' past these successfully
//
#define TEST_TRAP { \
    NKDbgPrintfW( TEXT("%s: Code Coverage Trap in: PWR, Line: %d\n"), TEXT(__FILE__), __LINE__); \
    DEBUGBREAK();  \
}

#else
#define TEST_TRAP
#define ZONE_REG_DUMP   0

#define DUMP_INTx()
#define DUMP_INTxM()
#define DUMP_RTC()
#define DUMP_REGS(StartReg, nRegs)
#endif


#endif _PWR_DRV_H_


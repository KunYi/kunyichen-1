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
    atapipci.h

Abstract:
    ATA/ATAPI for 2440 Ref Board.

Revision History:

--*/

#ifndef _ATAPIROMI_H_
#define _ATAPIROMI_H_

#include <atamain.h>




// Config port undo flags
#define CP_CLNUP_IRQEVENT    1
#define CP_CLNUP_HEVENT      2
#define CP_CLNUP_INTCHNHDNLR 4
#define CP_CLNUP_IST         8
#define ISTFLG_EXIT          1

#define MUX_REG             0x1800

#define ATA_CONTROL         0x1900
#define ATA_STATUS          0x1904
#define ATA_COMMAND         0x1908
#define ATA_SWRST           0x190c
#define ATA_IRQ             0x1910
#define ATA_IRQ_MASK        0x1914
#define ATA_CFG             0x1918
#define ATA_PIO_TIME        0x192c
#define ATA_UDMA_TIME       0x1930
#define ATA_XFR_NUM         0x1934
#define ATA_XFR_CNT         0x1938
#define ATA_TBUF_START      0x193c
#define ATA_TBUF_SIZE       0x1940
#define ATA_SBUF_START      0x1944
#define ATA_SBUF_SIZE       0x1948
#define ATA_CADDR_TBUR      0x194c
#define ATA_CADDR_SBUF      0x1950

#define ATA_PIO_DTR         0x1954
#define ATA_PIO_FED         0x1958
#define ATA_PIO_SCR         0x195c
#define ATA_PIO_LLR         0x1960
#define ATA_PIO_LMR         0x1964
#define ATA_PIO_LHR         0x1968    
#define ATA_PIO_DVR         0x196c
#define ATA_PIO_CSD         0x1970
#define ATA_PIO_DAD         0x1974

#define ATA_PIO_READY       0x1978
#define ATA_PIO_RDATA       0x197c
#define BUS_FIFO_STATUS     0x1990
#define ATA_FIFO_STATUS     0x1994



class CRomiDisk : public CDisk {
  public:

    // member variables
    static LONG  m_lDeviceCount;
    HANDLE m_hIsr; // handle to ISR



    // (DMA support)
    LPBYTE       m_pStartMemory;
    PDMATable    m_pPRD;
    PPhysTable   m_pPhysList;
    PSGCopyTable m_pSGCopy;
    PDWORD       m_pPFNs;
    DWORD        m_dwSGCount;
    DWORD        m_pPRDPhys;
    DWORD        m_dwPhysCount;
    DWORD        m_dwPRDSize;
    LPBYTE       m_pBMCommand;

    volatile S3C2450_MATRIX_REG *m_vpEBIRegs;
    volatile S3C2450_IOPORT_REG *m_vpIOPORTRegs;    

//    volatile S3C2450_CFCARD_REG *m_vpATAPIRegs;

    // constructors/destructors
    CRomiDisk(HKEY hKey);
    virtual ~CRomiDisk();

    // member functions
    virtual VOID ConfigureRegisterBlock(DWORD dwStride);
    virtual BOOL Init(HKEY hActiveKey);
    virtual DWORD MainIoctl(PIOREQ pIOReq);
    // virtual void TakeCS();
    // virtual void ReleaseCS();
    virtual BOOL WaitForInterrupt(DWORD dwTimeOut);
    virtual void EnableInterrupt();
    virtual BOOL ConfigPort();
    virtual BOOL SetupDMA(PSG_BUF pSgBuf, DWORD dwSgCount, BOOL fRead);
    virtual BOOL BeginDMA(BOOL fRead);
    virtual BOOL EndDMA();
    virtual BOOL AbortDMA();
    virtual BOOL CompleteDMA(PSG_BUF pSgBuf, DWORD dwSgCount, BOOL fRead);
    virtual BOOL WakeUp();
    
    void FreeDMABuffers();
    void CopyDiskInfoFromPort();
    BOOL TranslateAddress(PDWORD pdwAddr);
    virtual CDiskPower *GetDiskPowerInterface(void);


    inline virtual void CRomiDisk::TakeCS() {
        m_pPort->TakeCS();
    }
    inline virtual void CRomiDisk::ReleaseCS() {
        m_pPort->ReleaseCS();
    }
    inline void CRomiDisk::WriteBMCommand(BYTE bCommand) {
        ATA_WRITE_BYTE(m_pBMCommand, bCommand);
    }
    inline BYTE CRomiDisk::ReadBMStatus() {
        return ATA_READ_BYTE(m_pBMCommand + 2);
    }
    inline void CRomiDisk::WriteBMTable(DWORD dwPhys) {
        ATA_WRITE_DWORD((LPDWORD)(m_pBMCommand + 4), dwPhys);
    }
    inline void CRomiDisk::WriteBMStatus(BYTE bStatus) {
        ATA_WRITE_BYTE(m_pBMCommand + 2, bStatus);
    }
    inline BOOL CRomiDisk::DoesDeviceAlreadyExist() {
        return FALSE;
    }

    virtual void SetPioMode(UCHAR pmode);
    virtual void SetUdmaMode(void) ;    

};

#endif //_ATAPIPCI_H_


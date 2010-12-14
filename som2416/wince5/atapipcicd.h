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
    atapipcicd.h

Abstract:
    Base ATA/ATAPI PCI CD-ROM/DVD device support.

Revision History:

--*/

#pragma once

#include <atapipci.h>

class CPCIDiskAndCD : public CPCIDisk {
  public:
    CPCIDiskAndCD(HKEY hKey);
    virtual DWORD MainIoctl(PIOREQ pIOReq);
    DWORD AtapiIoctl(PIOREQ pIOReq);
    DWORD ReadCdRom(CDROM_READ *pReadInfo, PDWORD pBytesReturned);
    DWORD SetupCdRomRead(BOOL bRawMode, DWORD dwLBAAddr, DWORD dwTransferLength, PATAPI_COMMAND_PACKET pCmdPkt);
    virtual DWORD ReadCdRomDMA(DWORD dwLBAAddr, DWORD dwTransferLength, WORD wSectorSize, DWORD dwSgCount, SGX_BUF *pSgBuf);
    BOOL AtapiSendCommand(PATAPI_COMMAND_PACKET pCmdPkt, WORD wCount = 0, BOOL fDMA = FALSE);
    BOOL AtapiReceiveData(PSGX_BUF pSgBuf, DWORD dwSgCount,LPDWORD pdwBytesRead);
    BOOL AtapiSendData(PSGX_BUF pSgBuf, DWORD dwSgCount,LPDWORD pdwBytesWritten);
    BOOL AtapiIsUnitReady(PIOREQ pIOReq = NULL);
    BOOL AtapiIsUnitReadyEx();
    BOOL AtapiGetSenseInfo(CD_SENSE_DATA *pSenseData);
    BOOL AtapiIssueInquiry(INQUIRY_DATA *pInqData);
    BOOL AtapiGetToc(CDROM_TOC *pTOC);
    DWORD AtapiGetDiscInfo(PIOREQ pIOReq);
    DWORD AtapiReadQChannel(PIOREQ pIOReq);
    DWORD AtapiLoadMedia(BOOL bEject=FALSE);
    DWORD AtapiStartDisc();
    BOOL AtapiDetectDVD();
    void AtapiDumpSenseData();
    DWORD ControlAudio(PIOREQ pIOReq);
    DWORD DVDReadKey(PIOREQ pIOReq);
    DWORD DVDGetRegion(PIOREQ pIOReq);
    DWORD DVDSendKey(PIOREQ pIOReq);
    DWORD DVDSetRegion(PIOREQ pIOReq);
    BOOL DVDGetCopySystem(LPBYTE pbCopySystem, LPBYTE pbRegionManagement);

    virtual void SetPioMode(UCHAR pmode) ;
    virtual void SetUdmaMode(void) ;    
};


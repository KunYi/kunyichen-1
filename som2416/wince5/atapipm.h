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

#pragma once

class CDisk;                 // can delete forward reference once CDisk supports PM operations

class CDiskPower
{
protected:
    CEDEVICE_POWER_STATE m_curDx;
    CEDEVICE_POWER_STATE m_timeoutDx;
    BOOL m_fBoostRequested;
    BOOL m_fReductionRequested;
    DWORD m_dwPowerTimeout;
    HANDLE m_htPower;
    HANDLE m_hevPowerSignal;
    HANDLE m_hevDummy;          // this event is never signaled
    BOOL m_fShutdownPowerThread;
    CRITICAL_SECTION m_csPower;
    int m_PowerThreadPriority;
    CDisk *m_pDisk;
    LPCWSTR m_pszPMName;
    LONG m_UseCount;
    DWORD m_dwStartTickCount;   // GetTickCount() value when PM code initialized
    LARGE_INTEGER m_startQPC;   // QPC value at the time we entered the current state
    struct {
        DWORD dwCount;          // number of times entering this state
        LARGE_INTEGER totalQPC; // cumulative total of QPC counts
    } m_dxInfo[PwrDeviceMaximum];   // one entry for each device power state D0-D4
    DWORD (*m_pfnDevicePowerNotify)(PVOID, CEDEVICE_POWER_STATE, DWORD);

public:
    CDiskPower(void);
    virtual ~CDiskPower(void);
    virtual BOOL Init(CDisk *pDiskParent);
    virtual void SignalActivity(void);
    virtual BOOL RequestDevice(void);
    virtual void ReleaseDevice(void);
    virtual DWORD DiskPowerIoctl(PIOREQ pIOReq);

protected:
    virtual DWORD SetDiskPower(CEDEVICE_POWER_STATE newDx);
    virtual CEDEVICE_POWER_STATE GetDiskPower(void);
    virtual DWORD GetDiskCapabilities(PPOWER_CAPABILITIES pCap);
    virtual void TakeCS(void);
    virtual void ReleaseCS(void);

private:
    static DWORD WINAPI DiskPowerThreadStub(LPVOID lpvParam);
    virtual DWORD DiskPowerThread(void);
};


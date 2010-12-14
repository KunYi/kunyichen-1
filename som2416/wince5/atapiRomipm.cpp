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
#include "atapiRomipm.h"

// This routine marks the disk as in use.  If it is powered down,
// it spins it up and waits for it to become ready.  The caller
// must hold the disk critical section.
BOOL CRomiDiskPower::RequestDevice(void)
{
    BOOL fOk = TRUE;
    PREFAST_DEBUGCHK(m_pfnDevicePowerNotify != NULL);

    TakeCS();

    //jungpil 060404 TBR
    DEBUGMSG(ZONE_POWER, (_T("CRomiDiskPower::RequestDevice: TakeCS D%d\r\n"), m_curDx));

    // is the disk powered up?
    if(m_curDx != D0) {
        // don't bother requesting from the PM if we've already asked in a previous request
        DEBUGMSG(ZONE_POWER, (_T("CRomiDiskPower::RequestDevice: device at D%d, m_fBoostRequested is %d\r\n"), m_curDx, m_fBoostRequested));
//jungpil 060404 TBC
//        if(!m_fBoostRequested) 
        {
            // request that the PM make us available
            m_fBoostRequested = TRUE;
            DWORD dwStatus = m_pfnDevicePowerNotify((PVOID) m_pszPMName, D0, POWER_NAME);
            if(dwStatus != ERROR_SUCCESS) {
                DEBUGMSG(ZONE_WARNING, (_T("CRomiDiskPower::RequestDevice: DevicePowerNotify('%s') failed %d\r\n"), m_pszPMName, dwStatus));
                m_fBoostRequested = FALSE;
                fOk = FALSE;
            }
        }
    }

    //jungpil 060404 TBC
//    if(m_curDx == D0) {
    if(m_curDx == D0 || m_curDx == D1 || m_curDx == D2) {
        // wait for the disk to spin up so that we can do I/O
        DEBUGCHK(m_UseCount == 0);
        m_UseCount++;
    } else {
        fOk = FALSE;
    }

    //jungpil 060404 TBR
    DEBUGMSG(ZONE_POWER, (_T("CRomiDiskPower::RequestDevice: ReleaseCS D%d\r\n"), m_curDx));
    ReleaseCS();

    return fOk;
}


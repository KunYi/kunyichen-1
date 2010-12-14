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

#ifndef __SMDK2450BOARD_H
#define __SMDK2450BOARD_H

#include "SDIOControllerBase.h"

class CSDIOController : public CSDIOControllerBase
{
public:
    explicit CSDIOController( PSDCARD_HC_CONTEXT pHCContext );
    virtual ~CSDIOController();

protected:
    // override to customize for specific hardware
    virtual BOOL InitializeHardware( BOOL bOnPowerUp = FALSE );
    virtual void DeinitializeHardware( BOOL bOnPowerDown = FALSE );
    virtual BOOL CustomSetup( LPCTSTR pszRegistryPath );
    virtual BOOL IsCardWriteProtected();
    virtual BOOL IsCardPresent();

protected:

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
};

#endif // __SMDK2450BOARD_H

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

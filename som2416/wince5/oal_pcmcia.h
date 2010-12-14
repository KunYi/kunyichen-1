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
//------------------------------------------------------------------------------
//
//  Header: oal_pcmcia.h
//
//  This header define OAL PCMCIA module.
//
#ifndef __OAL_PCMCIA_H
#define __OAL_PCMCIA_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Function: OALPCMCIATransBusAddress 
//
//  This function translates PCMCIA bus relative address to processor address
//  space.
//
BOOL OALPCMCIATransBusAddress(
    UINT32 busId, UINT64 busAddress, UINT32 *pAddressSpace, 
    UINT64 *pSystemAddress
);

VOID OALPCMCIAPowerOn(UINT32 busId);
VOID OALPCMCIAPowerOff(UINT32 busId);

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif


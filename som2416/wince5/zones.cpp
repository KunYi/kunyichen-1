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
#include <windows.h>
#include <debug.h>

DBGPARAM dpCurSettings = {
    TEXT("ATAPI"), {
    TEXT("Init"),TEXT("Deinit"),TEXT("Main"),TEXT("I/O"),
    TEXT("PCMCIA"),TEXT("PCI"),TEXT("IOCTL"),TEXT("CDROM"),
    TEXT("DMA"),TEXT("Power"),TEXT("Undefined"),TEXT("Undefined"),
    TEXT("Warning"),TEXT("Error"),TEXT("Helper"), TEXT("CELOG") },
#if 0
    ZONEMASK_IOCTL | 
    ZONEMASK_DMA | 
    ZONEMASK_INIT | 
    ZONEMASK_HELPER | 
    ZONEMASK_DEINIT | 
    ZONEMASK_PCMCIA | 
    ZONEMASK_PCI | 
    ZONEMASK_MAIN | 
    ZONEMASK_IO | 
    ZONEMASK_CDROM | 
    ZONEMASK_ERROR |
    ZONEMASK_CELOG 
#else    
    ZONEMASK_ERROR |
    ZONEMASK_WARNING |
    ZONEMASK_INIT
#endif    
};


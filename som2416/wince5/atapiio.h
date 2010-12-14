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

#ifndef _ATAPIIO_H_
#define _ATAPIIO_H_

typedef struct _IOREQ {
    DWORD  dwChd;          // ATA command
    HANDLE hEvent;         // Win32 event to signal blocked requesting thread
    DWORD  dwStatus;       // request status
    HANDLE hProcess;       // calling process
    DWORD  dwReqPerm;      // rermissions required to access caller's buffers
    DWORD  dwCode;
    PBYTE  pInBuf;
    DWORD  dwInBufSize;
    PBYTE  pOutBuf;
    DWORD  dwOutBufSize;
    PDWORD pBytesReturned;
    DWORD  dwRef;          // reference returned to callback
} IOREQ, *PIOREQ;

#endif // _ATAPIIO_H_

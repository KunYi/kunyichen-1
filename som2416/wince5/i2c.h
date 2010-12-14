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

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

    I2C.H   Public I2C Driver Interface

Abstract:

    Supports:
    ---------
    o) Master Tx/Rx Modes only.
    o) Multi-byte Tx/Rx (slave must support auto increment)

    Notes
    -----
    o) Call CreateFile(L"I2C0:") to open driver
    o) Call CloseHandle to close driver

--*/

#ifndef __I2C_H__
#define __I2C_H__

#define I2C_THREAD_PRIORITY     99

//
// Operating Modes: configured via Registry during Init.
//
//  POLLING:
//  INTERRUPT:  Generates an interrupt on EVERY byte Tx/Rx, including address.
//              Launches an IST which runs at I2C_THREAD_PRIORITY.
//
typedef enum I2C_MODE {
    POLLING,
    INTERRUPT 
} I2C_MODE, *PI2C_MODE;


//
// I/O DESCRIPTOR for Read/Write
//
typedef struct _I2C_IO_DESC {
    DWORD   SlaveAddr;      // Target Slave Address
    UCHAR   WordAddr;       // Starting Slave Word Address
    PUCHAR  Data;           // pBuffer
    DWORD   Count;          // nBytes to read/write
} I2C_IO_DESC, *PI2C_IO_DESC;


//
// I2C_FASTCALL dispatch table for direct Driver-To-Driver calling in device.exe 
// process space. Callers outside of device.exe will fail and MUST use the IOCTLs provided.
// To use these routines an I2C client driver must call CreateFile on the I2C bus driver,
// and call IOCTL_I2C_GET_FASTCALL to get the entrypoints. CreateFile calls the 
// bus driver's Xxx_Open routine. The client driver will call CloseHandle when 
// it is completely done with the bus driver, which calls the bus driver's Xxx_Close.
//
typedef struct _I2C_FASTCALL {
    PVOID Context;   // opaque context
    DWORD (*I2CRead) (PVOID Context, DWORD SlaveAddr, UCHAR WordAddr, PUCHAR pData, DWORD Count);
    DWORD (*I2CWrite)(PVOID Context, DWORD SlaveAddr, UCHAR WordAddr, PUCHAR pData, DWORD Count);
} I2C_FASTCALL, *PI2C_FASTCALL;


//
// I2C Bus Driver IOCTLS
//
#define FILE_DEVICE_I2C     FILE_DEVICE_CONTROLLER

// IN:  PI2C_IO_DESC
#define IOCTL_I2C_READ \
    CTL_CODE(FILE_DEVICE_I2C, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

// IN:  PI2C_IO_DESC
#define IOCTL_I2C_WRITE \
    CTL_CODE(FILE_DEVICE_I2C, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

// OUT: PI2C_FASTCALL
#define IOCTL_I2C_GET_FASTCALL  \
    CTL_CODE(FILE_DEVICE_I2C, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)



#endif // __I2C_H__


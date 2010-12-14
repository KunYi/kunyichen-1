//
// Copyright (c) Cambridge Silicon Radio.  All rights reserved.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Header for serial driver wrapper

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SERIALCSR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SERIALCSR_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
// Function Pointers

#ifndef _SERIALCSR_H
#define _SERIALCSR_H


// Standard constants
#undef  FALSE
#undef  TRUE
#undef  NULL
#define FALSE   0
#define TRUE    1
#define NULL    0

// Header not found
typedef struct _SERIAL_QUEUE_SIZE {
  ULONG  InSize;
  ULONG  OutSize;
} SERIAL_QUEUE_SIZE, *PSERIAL_QUEUE_SIZE;


// Typedef wrapped functions to ensure accurate use
	// Exported to API
typedef HANDLE (WINAPI *PFNCOM_Init)(ULONG);
typedef HANDLE (WINAPI *PFNCOM_Open)(HANDLE, DWORD, DWORD);
typedef BOOL   (WINAPI *PFNCOM_Close)(DWORD);
typedef BOOL   (WINAPI *PFNCOM_Deinit)(void);
typedef ULONG  (WINAPI *PFNCOM_Read)(HANDLE, PUCHAR, ULONG, PULONG);
typedef ULONG  (WINAPI *PFNCOM_Write)(HANDLE, PUCHAR, ULONG);
typedef ULONG  (WINAPI *PFNCOM_Seek)(HANDLE, LONG, DWORD);
typedef BOOL   (WINAPI *PFNCOM_Power)(HANDLE);
typedef BOOL   (WINAPI *PFNCOM_IOControl)(DWORD, DWORD, PBYTE, DWORD, PBYTE, DWORD, PDWORD);

	// Usually internal only 
    //- *** TEMP *** Commented are those BT.dll does not export
    //
//typedef VOID   (WINAPI *PFNSerialEventHandler)(ULONG);
//typedef DWORD  (WINAPI *PFNSerialDispatchThread)(PVOID);
//typedef ULONG  (WINAPI *PFNSerialGetDroppedByteNumber)(HANDLE);
//typedef BOOL   (WINAPI *PFNWaitCommEvent)(ULONG, PULONG, LPOVERLAPPED);
//typedef VOID   (WINAPI *PFNEvaluateEventFlag)(PVOID, ULONG);
//typedef BOOL   (WINAPI *PFNProcessExiting)(ULONG);
//typedef BOOL   (WINAPI *PFNApplyDCB)(ULONG, DCB, BOOL);


 ULONG WINAPI  CSR_Read(HANDLE pContext, PUCHAR pTargetBuffer, 
							  ULONG BufferLength, PULONG pBytesRead);
 ULONG WINAPI  CSR_Write(HANDLE COM_Write, PUCHAR pSourceBytes,
				 			   ULONG NumberOfBytes);

// Internal use
BOOL UpdatePersistentStore(HANDLE hResult);
BOOL GetLibraryPointers(void);

#endif //_SERIALCSR_H


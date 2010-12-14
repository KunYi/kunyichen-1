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
//  Header:  oal_nkxp.h
//
//  This header file declares all the useful functions exported by NK.
//
#ifndef __OAL_NKXP_H
#define __OAL_NKXP_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Globals:  CurMSec, dwReschedTime, curridlehigh, curridlelow, idleconv
//
//  Following external variables are exported by kernel. They are updated
//  or used by timer module implementation.
//
#if _MIPS_
#define CurMSec *((volatile UINT32*)AddrCurMSec)
#else
extern volatile UINT32 CurMSec;
#endif
extern volatile UINT32 dwReschedTime;
extern volatile UINT32 curridlehigh;
extern volatile UINT32 curridlelow;
extern UINT32 idleconv;
extern BOOL (*pQueryPerformanceFrequency)(LARGE_INTEGER*);
extern BOOL (*pQueryPerformanceCounter)(LARGE_INTEGER*);
extern VOID (*pOEMUpdateRescheduleTime)(DWORD);

//------------------------------------------------------------------------------
//
//  Global:  pOEMIsProcessorFeaturePresent
//
extern BOOL (*pOEMIsProcessorFeaturePresent)(DWORD feature);

//------------------------------------------------------------------------------
//
//  Function:  SC_SetLastError
//
//  Kernel variant of SetLastError function.
//
void SC_SetLastError(DWORD code);

//------------------------------------------------------------------------------
//
//  Function:  NKwvsprintfW
//
//  Formated printing to a buffer
//
int NKwvsprintfW(
    LPWSTR pszBuffer, LPCWSTR pszFormat, va_list pParms, int maxChars
);

//------------------------------------------------------------------------------
//
//  Function: kstrncmpi
//
//  Function compares two unicode strings ignoring case, with count, in chars
//
int kstrncmpi(LPCWSTR string1, LPCWSTR string2, int count);

//------------------------------------------------------------------------------
//
//  Function: kstrcmpi
//
//  Function compares two unicode strings ignoring case
//
int kstrcmpi(LPCWSTR string1, LPCWSTR string2);

//------------------------------------------------------------------------------
//
//  Function: kstrcmpiAandW
//
//  Function compares an ASCII and an unicode string
//
int strcmpiAandW(LPSTR pszAscii, LPWSTR pszUnicode);

//------------------------------------------------------------------------------
//
//  Function:  kstrcpyW
//
//  Function copies an unicode string
//
void kstrcpyW(LPWSTR pszDst, LPCWSTR lpszSrc);

//------------------------------------------------------------------------------
//
//  Function:  strlenW
//
//  Function returns lenght of an unicode string
//
unsigned int strlenW(LPCWSTR lpszStr);

//------------------------------------------------------------------------------
//
// to make things consistant, we #define NK prefix for all the functions
// so we know what functions we used. In the future release, these function
// might be formally exported and we won't have to do the defines anymore
//
#ifndef FORMAL_NK_EXPORT

#define NKSetLastError          SC_SetLastError
#define NKwcscpy                kstrcpyW
#define NKwcsicmp               kstrcmpi
#define NKwcsnicmp              kstrncmpi
#define NKstrcmpiAandW          strcmpiAandW
#define NKwcslen                strlenW

#endif

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif // __OAL_NKXP

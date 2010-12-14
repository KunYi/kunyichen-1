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
#ifndef _HELPER_H_
#define _HELPER_H_


BOOL AtaGetRegistryValue(HKEY hKey, PTSTR szValueName, PDWORD pdwValue);
BOOL AtaGetRegistryString( HKEY hKey, PTSTR szValueName, PTSTR *pszValue, DWORD dwSize=0);
BOOL AtaGetRegistryString2( HKEY hKey, PTSTR szValueName, PTSTR *pszValue, DWORD dwSize=0);
BOOL AtaSetRegistryValue(HKEY hKey, PTSTR szValueName, DWORD dwValue);
BOOL AtaSetRegistryValue2(HKEY hKey, PTSTR szValueName, DWORD dwValue);
BOOL AtaSetRegistryString( HKEY hKey, PTSTR szValueName, PTSTR szValue);
BOOL ATAParseIdString(BYTE *str, int len, DWORD *pdwOffset,BYTE **ppDest, DWORD *pcBytesLeft);

#endif // _HELPER_H_

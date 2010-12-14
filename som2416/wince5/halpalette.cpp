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
// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------

#include "precomp.h"

DWORD
WINAPI
HalCreatePalette(
    LPDDHAL_CREATEPALETTEDATA pd
    )
{
    /*
    typedef struct _DDHAL_CREATEPALETTEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWPALETTE_GBL   lpDDPalette;    // ddraw palette struct
        LPPALETTEENTRY              lpColorTable;   // colors to go in palette
        HRESULT                     ddRVal;         // return value
        LPDDHAL_CREATEPALETTE       CreatePalette;  // PRIVATE: ptr to callback
        BOOL                        is_excl;        // process has exclusive mode
    } DDHAL_CREATEPALETTEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

//////////////////////////// DDHAL_DDPALETTECALLBACKS ////////////////////////////

DWORD
WINAPI
HalDestroyPalette(
    LPDDHAL_DESTROYPALETTEDATA pd
    )
{
    DEBUGENTER( HalDestroyPalette );
    /*
    typedef struct _DDHAL_DESTROYPALETTEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWPALETTE_GBL   lpDDPalette;    // palette struct
        HRESULT                     ddRVal;         // return value
        LPDDHALPALCB_DESTROYPALETTE DestroyPalette; // PRIVATE: ptr to callback
    } DDHAL_DESTROYPALETTEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}


DWORD
WINAPI
HalSetEntries(
    LPDDHAL_SETENTRIESDATA pd
    )
{
    DEBUGENTER( HalSetEntries );
    /*
    typedef struct _DDHAL_SETENTRIESDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWPALETTE_GBL   lpDDPalette;    // palette struct
        DWORD                       dwBase;         // base palette index
        DWORD                       dwNumEntries;   // number of palette entries
        LPPALETTEENTRY              lpEntries;      // color table
        HRESULT                     ddRVal;         // return value
        LPDDHALPALCB_SETENTRIES     SetEntries;     // PRIVATE: ptr to callback
    } DDHAL_SETENTRIESDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}


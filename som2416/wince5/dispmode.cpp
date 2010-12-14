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
Copyright (c) 2007-2008. Samsung Electronics, co. ltd  All rights reserved.

Module Name:	dispmode.cpp

Abstract:	Display Mode setting Implementation.

Functions:	SetMode, InitializeDisplayMode, GetModeInfo, GetModeInfoEx, NumModes

Notes:	In S3C2443 and S3C2450 support only 1 mode(initial mode , ex RGB565)

--*/

#include "precomp.h"
#include <syspal.h>    // for 8Bpp we use the natural palette

EGPEFormat eFormat[] =
{
    gpe8Bpp,
    gpe16Bpp,
    gpe24Bpp,
    gpe32Bpp,
};

EDDGPEPixelFormat ePixelFormat[4] = 
{
    ddgpePixelFormat_8bpp,
    ddgpePixelFormat_565,
    ddgpePixelFormat_8880,
    ddgpePixelFormat_8888,
};

ULONG BitMasks[][3] =
{
    { 0, 0, 0 },
    { 0xF800, 0x07E0, 0x001F },
    { 0xFF0000, 0x00FF00, 0x0000FF },
    { 0x00FF0000, 0x0000FF00, 0x000000FF }
};

void S3C2450DISP::InitializeDisplayMode()
{
	//APR check
	m_pModeEx = &m_ModeInfoEx;
	m_pMode = &m_ModeInfoEx.modeInfo;
	memset(m_pModeEx, 0, sizeof(GPEModeEx));
	m_pModeEx->dwSize = sizeof(GPEModeEx);
	m_pModeEx->dwVersion = GPEMODEEX_CURRENTVERSION;

	// Setup main ModeInfo
	m_pMode->modeId = 0; 
	m_pMode->Bpp = m_colorDepth;
	m_pMode->frequency = 60;
	m_pMode->width = m_nScreenWidth;
	m_pMode->height = m_nScreenHeight;
	
	int nBPP = m_pMode->Bpp/8 - 1;
	switch (m_pMode->Bpp)
	{
//        case    8:
    case    16:
    case    24:
    case    32:
        m_pMode->format = eFormat[nBPP];
        m_pModeEx->ePixelFormat = ePixelFormat[nBPP];
        m_pModeEx->lPitch = m_pMode->width * m_pMode->Bpp / 8;
        m_pModeEx->dwRBitMask = BitMasks[nBPP][0];
        m_pModeEx->dwGBitMask = BitMasks[nBPP][1];
        m_pModeEx->dwBBitMask = BitMasks[nBPP][2];
        break;

    default:
        RETAILMSG(1,(TEXT("Invalid BPP value passed to driver - Bpp = %d\r\n"), m_pMode->Bpp));
        m_pMode->format = gpeUndefined;
        break;
	}
}

SCODE    S3C2450DISP::SetMode (INT modeId, HPALETTE *palette)
{
    DEBUGMSG(GPE_ZONE_INIT,(TEXT("S3C2450DISP::SetMode\r\n")));

    if (modeId != 0)
    {
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("S3C2450DISP::SetMode Want mode %d, only have mode 0\r\n"),modeId));
        return    E_INVALIDARG;
    }

    if (palette)
    {
        switch (m_colorDepth)
        {
            case    8:
                *palette = EngCreatePalette (PAL_INDEXED,
                                             PALETTE_SIZE,
                                             (ULONG*)_rgbIdentity,
                                             0,
                                             0,
                                             0);
                break;

            case    16:
            case    24:
            case    32:
                *palette = EngCreatePalette (PAL_BITFIELDS,
                                             0,
                                             NULL,
                                             ((1 << m_RedMaskSize) - 1) << m_RedMaskPosition,
                                             ((1 << m_GreenMaskSize) - 1) << m_GreenMaskPosition,
                                             ((1 << m_BlueMaskSize) - 1) << m_BlueMaskPosition);
                break;
        }
        
    	m_nSurfaceBitsAlign = (m_pMode->Bpp == 24) ? (128 * 3) : 128;        
		//Allocate our primary surface here
		if(NULL == m_pPrimarySurface)
		{
			if(FAILED(AllocSurface((DDGPESurf **)&m_pPrimarySurface, m_nScreenWidth, 
				m_nScreenHeight, m_pMode->format, m_pModeEx->ePixelFormat, 
				GPE_REQUIRE_VIDEO_MEMORY)))
			{
				RETAILMSG (1, (L"Couldn't allocate primary surface\n"));
				return E_INVALIDARG;
			}
			m_pVisibleSurface = (S3C2450Surf*)m_pPrimarySurface;
		}

		m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, m_iRotate);    	
    }

	DynRotate(m_iRotate);
    return S_OK;
}

SCODE    S3C2450DISP::GetModeInfo(GPEMode *mode,    INT modeNumber)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::GetModeInfo\r\n")));

    if (modeNumber != 0)
    {
        return E_INVALIDARG;
    }


	*mode = *m_pMode;	

    return S_OK;
}

//APR added func override
SCODE S3C2450DISP::GetModeInfoEx(GPEModeEx *pModeEx, int modeNo)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::GetModeInfoEx\r\n")));
	if (modeNo != 0)
    	{
        	return    E_INVALIDARG;
    	}

    	*pModeEx = *m_pModeEx;

    	return S_OK;
}

int        S3C2450DISP::NumModes()
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::NumModes\r\n")));
    return    1;
}

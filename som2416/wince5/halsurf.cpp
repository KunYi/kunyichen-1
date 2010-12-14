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

#define HAL_ZONE_ERROR 0
#define HAL_ZONE_INFO 0
#define HAL_ZONE_WARNING 0

#define GDI_SCREEN_WIDTH	(g_pGPE->ScreenWidth())
#define GDI_SCREEN_HEIGHT	(g_pGPE->ScreenHeight())

#define DDRAW_SCREEN_WIDTH	( (g_pDDrawPrimarySurface != NULL) \
								? (g_pDDrawPrimarySurface->Width()) \
								: GDI_SCREEN_WIDTH )
#define DDRAW_SCREEN_HEIGHT	( (g_pDDrawPrimarySurface != NULL) \
								? (g_pDDrawPrimarySurface->Height()) \
								: GDI_SCREEN_HEIGHT )

bool UpdateHALInit(
	LPDDRAWI_DIRECTDRAW_GBL lpDD,           // driver struct
	DWORD modeidx )
{
    DDHALINFO   ddhalinfo;

	DEBUGENTER(UpdateHALInit);

	memset(&ddhalinfo, 0, sizeof(ddhalinfo));
	ddhalinfo.dwSize = sizeof(ddhalinfo);

	buildDDHALInfo(&ddhalinfo, modeidx);

	lpDD->vmiData = ddhalinfo.vmiData;
	lpDD->dwMonitorFrequency = ddhalinfo.dwMonitorFrequency;
	lpDD->ddCaps = ddhalinfo.ddCaps;
	
	DEBUGLEAVE(UpdateHALInit);
	return TRUE;
}


BOOL HalGDIHasSplitFromDDraw(void)
{
	BOOL	bDifferentPrimaries = FALSE;

	DEBUGENTER(HalGDIHasSplitFromDDraw);
	
	bDifferentPrimaries = g_pGPE->PrimarySurface() != g_pDDrawPrimarySurface;

	DEBUGLEAVE(HalGDIHasSplitFromDDraw);

	return bDifferentPrimaries;
}


DWORD WINAPI HalFlipToGDISurface( LPDDHAL_FLIPTOGDISURFACEDATA pd )
{
    DEBUGENTER( HalFlipToGDISurface );
    /*
    typedef struct _DDHAL_FLIPTOGDISURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL    lpDD;             // driver struct
        DWORD                      dwToGDI;          // TRUE if flipping to the GDI surface, FALSE if flipping away
        DWORD                      dwReserved;       // reserved for future use
        HRESULT                    ddRVal;           // return value
        LPDDHAL_FLIPTOGDISURFACE   FlipToGDISurface; // PRIVATE: ptr to callback
    } DDHAL_FLIPTOGDISURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

SCODE HalGetPixelFormatFromSurfaceDesc(
					LPDDSURFACEDESC			lpDDSurfaceDesc,
					EDDGPEPixelFormat*		pPixelFormat,
					EGPEFormat*				pFormat
					)
{
	SCODE ddRVal = S_OK;

	DEBUGENTER(HalGetPixelFormatFromSurfaceDesc);

	if ((lpDDSurfaceDesc == NULL) || (pPixelFormat == NULL) || (pFormat == NULL))
	{
		DEBUGMSG(HAL_ZONE_ERROR, (TEXT("DDGPEGetPixelFormatFromSurfaceDesc ERROR - Invalid Parameters\r\n") ));
		return DDERR_INVALIDPARAMS;
	}

	if ( (lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT) )
	{
		ddRVal = g_pGPE->DetectPixelFormat(
						lpDDSurfaceDesc->ddsCaps.dwCaps,
						&lpDDSurfaceDesc->ddpfPixelFormat,
						pFormat,
						pPixelFormat
						);

		DEBUGMSG(HAL_ZONE_INFO, (TEXT("got pixelFormat %d\r\n"), *pPixelFormat));

		if( FAILED(ddRVal) )
		{
			DEBUGMSG(HAL_ZONE_WARNING,(TEXT("DDGPEGetPixelFormatFromSurfaceDesc returned DDERR_UNSUPPORTEDFORMAT\r\n")));
			ddRVal = DDERR_UNSUPPORTEDFORMAT;
		}
	}
	else
	{
		// just use primary surface info
		
		if (g_pDDrawPrimarySurface != NULL)
		{
			*pPixelFormat	= g_pDDrawPrimarySurface->PixelFormat();
			*pFormat		= g_pDDrawPrimarySurface->Format();
		}
		else
		{
			// it should never come in here, but this code works...

			SCODE		sc;
			GPEMode		modeInfo;
			DDGPESurf*	pDDGPESurf = NULL;

			sc = g_pGPE->GetModeInfo(&modeInfo, g_pGPE->GetPhysicalModeId());
			if (FAILED(sc))
			{
				ddRVal = DDERR_UNSUPPORTEDFORMAT;
			}
			else
			{
				*pFormat = modeInfo.format;

				GPEModeEx	modeInfoEx; // restrict the scope of this, since we can't rely on it being valid for all drivers
				sc = g_pGPE->GetModeInfoEx(&modeInfoEx, g_pGPE->GetPhysicalModeId());
				if (FAILED(sc))
				{
					// function probably wasn't supported by the driver
					*pPixelFormat = EGPEFormatToEDDGPEPixelFormat[modeInfo.format];
					ddRVal = S_OK;
				}
				else
				{
					*pPixelFormat = modeInfoEx.ePixelFormat;
				}
			}
		}

	}

	DEBUGLEAVE(HalGetPixelFormatFromSurfaceDesc);
	return ddRVal;
}


DWORD WINAPI HalCreateSurface( LPDDHAL_CREATESURFACEDATA pd )
{
    /*
    typedef struct _DDHAL_CREATESURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDSURFACEDESC             lpDDSurfaceDesc;// description of surface being created
        LPDDRAWI_DDRAWSURFACE_LCL   FAR *lplpSList; // list of created surface objects
        DWORD                       dwSCnt;         // number of surfaces in SList
        HRESULT                     ddRVal;         // return value
        LPDDHAL_CREATESURFACE       CreateSurface;  // PRIVATE: ptr to callback
    } DDHAL_CREATESURFACEDATA;
    */

    // Implementation
	SCODE				sc = S_OK;
	unsigned int		iSurf;		// Surface index
	//unsigned int		nBPP;		// Bits-per-pixel on surface
	unsigned int		nWidth;		// Width of surface in pixels
	//unsigned int		nPitch;		// Width of surface in bytes
	unsigned int		nHeight;	// Height of surface in pixels
	LPDDRAWI_DDRAWSURFACE_LCL
						pSurf;		// Pointer to surface data
	EDDGPEPixelFormat	pixelFormat;
	EGPEFormat			format;		// Pixel format of surface(s) being created
	DWORD 				dwFlags = pd->lpDDSurfaceDesc->dwFlags;
	DWORD 				dwCaps = pd->lpDDSurfaceDesc->ddsCaps.dwCaps;

	DEBUGENTER( HalCreateSurface );

	//DebugBreak();

	/*
	typedef struct _DDHAL_CREATESURFACEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
	    LPDDSURFACEDESC             lpDDSurfaceDesc;// description of surface being created
	    LPDDRAWI_DDRAWSURFACE_LCL   FAR *lplpSList; // list of created surface objects
	    DWORD                       dwSCnt;         // number of surfaces in SList
	    HRESULT                     ddRVal;         // return value
	    LPDDHAL_CREATESURFACE       CreateSurface;  // PRIVATE: ptr to callback
	} DDHAL_CREATESURFACEDATA;
	*/


	sc = HalGetPixelFormatFromSurfaceDesc(
				pd->lpDDSurfaceDesc,
				&pixelFormat,
				&format
				);

	if (FAILED(sc))
	{
		RETAILMSG(1,(TEXT("DDGPECreateSurface ERROR - DDERR_UNSUPPORTEDFORMAT (0x%08x)\r\n"),
											sc ));
		pd->ddRVal = DDERR_UNSUPPORTEDFORMAT;
		DEBUGLEAVE(HalCreateSurface);
		return DDHAL_DRIVER_HANDLED;
	}

	// Use pd->lpDDSurfaceDesc->dwFlags to determine which fields are valid and use them
/*	nBPP = (pd->lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT) ?
		((USHORT)(pd->lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount)) :
		g_pDDrawPrimarySurface->Bpp();*/
/*#ifdef FB16BPP
		16;		// REVIEW!
#else
		8;	// REVIEW!
#endif*/


	nWidth = (pd->lpDDSurfaceDesc->dwFlags & DDSD_WIDTH) ?
		( pd->lpDDSurfaceDesc->dwWidth ) : DDRAW_SCREEN_WIDTH;		// resolves to a call to
																	//  g_pDDrawPrimarySurface or a GPE query

	nHeight = (pd->lpDDSurfaceDesc->dwFlags & DDSD_HEIGHT) ?
		( pd->lpDDSurfaceDesc->dwHeight ) : DDRAW_SCREEN_HEIGHT;	// resolves to a call to
																	//  g_pDDrawPrimarySurface or a GPE query
	RETAILMSG(1,(TEXT("nWidth=%d  nHeight=%d\n"),nWidth,nHeight));
	//RETAILMSG(GPE_ZONE_CREATE,(TEXT("nBPP: %d\r\n"), nBPP ));

	/*switch(nBPP)
	{
	case 8:
		format = gpe8Bpp;
		break;
	case 16:
		format = gpe16Bpp;
		break;
	case 24:
//		if( pd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS )
//			format = pfmt_24a;
//		else
//			format = pfmt_24;
//		nBPP = 32;
//		break;
		RETAILMSG(GPE_ZONE_WARNING,(TEXT("HalCreateSurface rejecting %d BPP surface\r\n"), nBPP));
		pd->ddRVal = DDERR_UNSUPPORTEDFORMAT;
		return DDHAL_DRIVER_HANDLED;
	case 32:
		format = gpe32Bpp;
		break;
	}*/

	//nPitch = ( nWidth * nBPP ) >>3;
	//nPitch = ( nPitch + 3 ) & ~3;


#if DEBUG
	RETAILMSG(1,(TEXT("Number of surfaces to create: %d\r\n"), pd->dwSCnt ));
	RETAILMSG(1,(TEXT("Create Surface flags: ")));
//	DumpDDSCAPS(pd->lpDDSurfaceDesc->ddsCaps);
#endif

	for( iSurf=0; iSurf<pd->dwSCnt; iSurf++ )
	{
		pSurf = pd->lplpSList[iSurf];

//#if DEBUG
#if 0
		RETAILMSG(1,(TEXT("Surface #%d: LCL:%08x FLAGS:"), iSurf, pSurf ));
		DumpDDSCAPS( pSurf->ddsCaps );
		if( dwCaps & DDSCAPS_MIPMAP )
		{
			RETAILMSG(1,(TEXT("MipMap count: %d\r\n"), pSurf->lpSurfMore->dwMipMapCount ));
		}
		RETAILMSG(1,(TEXT("\r\nAddr of GBL: 0x%08x, Addr of LCL: 0x%08x\r\n"),
			pSurf->lpGbl, pSurf ));
#endif

        if ( pSurf->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE )
        {
			RETAILMSG(1, (TEXT("dwCaps4 = 0x%x\r\n"), pSurf->lpSurfMore->ddsCapsEx.dwCaps4));
			if ( (pSurf->lpSurfMore->ddsCapsEx.dwCaps4 & DDSCAPS4_NONGDIPRIMARY) )
			{
				DWORD	dwModeID = -1L;

				RETAILMSG(1, (TEXT("SPLITTING THE DDRAW SURFACE\r\n") ));

				// g_pDrawPrimarySurface is originally set to the GDI primary surface, but this isn't what we want
				// for our model. We want a ddraw primary surface that is separate from the GDI primary surface.
				// (and that could have a different pixel depth, stride, format, etc.)

				RETAILMSG(1, (TEXT("still have pixelFormat %d\r\n"), pixelFormat));

				if ( (pd->lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT) )
				{
					RETAILMSG(1, (TEXT("Using pixel format to find split mode\r\n") ));
					sc = g_pGPE->DetectMode(&dwModeID, nWidth, nHeight, format, pixelFormat, &pd->lpDDSurfaceDesc->ddpfPixelFormat);
				}
				else
				{
					RETAILMSG(1, (TEXT("NOT using pixel format to find split mode\r\n") ));
					sc = g_pGPE->DetectMode(&dwModeID, nWidth, nHeight, format, pixelFormat);
				}

				GPEMode		modeInfo;
				DDGPESurf*	pDDGPESurf = NULL;

				if (FAILED(sc))
				{

					RETAILMSG(1, (TEXT("Could not find requested primary surface mode 0x%08x\r\n"), sc));

					// try to just split off a surface of the same type as the current primary surface (GDI surface)
					sc = S_OK;
					dwModeID = g_pGPE->GetModeId(); // can only split off GDI surface
				}
				else
				{
					RETAILMSG(1, (TEXT("Found requested primary surface mode: ID = %d\r\n"), dwModeID));
				}

				if (SUCCEEDED(sc))
				{
					sc = g_pGPE->GetModeInfo(&modeInfo, dwModeID);
					if (SUCCEEDED(sc))
					{
						GPEModeEx	modeInfoEx; // restrict the scope of this, since we can't rely on it being valid for all drivers
						sc = g_pGPE->GetModeInfoEx(&modeInfoEx, dwModeID);
						if (FAILED(sc))
						{
							// function probably wasn't supported by the driver
							pixelFormat = EGPEFormatToEDDGPEPixelFormat[modeInfo.format];
							sc = S_OK;
						}
						else
						{
							pixelFormat = modeInfoEx.ePixelFormat;
						}
					}
				}
				
				if (FAILED(sc))
				{
					RETAILMSG(1, (TEXT("DDGPECreateSurface- Could not find a good mode: DetectMode and GetModeInfo failed. 0x%x\r\n"), sc ));
				}
				else
				{
					BOOL	bSplitToSame = FALSE;

					if (HalGDIHasSplitFromDDraw())
					{
						// for now, don't worry about the case where the rect is invalid.
						// let it just fall through.

						if (	(g_pDDrawPrimarySurface->PixelFormat()	== pixelFormat)		&&
								(g_pDDrawPrimarySurface->Format()		== modeInfo.format)	&&
								(g_pDDrawPrimarySurface->Width()		== modeInfo.width)	&&
								(g_pDDrawPrimarySurface->Height()		== modeInfo.height) &&
								(1) )
						{
							bSplitToSame = TRUE;
						}

					}

					// don't need to split if the ddraw surface is already split, and
					// the mode requested is the same as the current ddraw primary surface's mode
					if (bSplitToSame)
					{
						pDDGPESurf = g_pDDrawPrimarySurface;

						RETAILMSG(1, (TEXT("Surface is already split in requested mode!\r\n") ));
					}
					else
					{
						unsigned long	dwOffsetInVideoMemory = 0L;

						// don't deallocate our existing surface yet, because the allocate
						// might fail

						sc = g_pGPE->AllocVideoSurface(
								&pDDGPESurf,
								modeInfo.width,
								modeInfo.height,
								modeInfo.format,
								pixelFormat,
								&dwOffsetInVideoMemory );
					
						RETAILMSG(
							1,
							(TEXT("Created primary surface at: 0x%x (%d)\r\n"), 
							dwOffsetInVideoMemory, 
							dwOffsetInVideoMemory
							));
					}

				}
				if (FAILED(sc))
				{
					RETAILMSG(1, (TEXT("DDGPECreateSurface-AllocSurface failed 0x%x\r\n"), sc ));
				}
				else
				{
					if (pDDGPESurf != NULL)
					{
						g_pDDrawPrimarySurface = pDDGPESurf; //(DDGPESurf*)g_pGPE->PrimarySurface();
						RETAILMSG(1, (TEXT("DDGPECreateSurface-AllocSurface successful 0x%x\r\n"), g_pDDrawPrimarySurface->OffsetInVideoMemory() ));
					}
					else
					{
						RETAILMSG(1, (TEXT("DDGPECreateSurface-AllocSurface returned NULL 0x%x\r\n"), sc ));
					}
				}

				if (dwModeID != (DWORD)-1L)
				{
					if (dwModeID != (DWORD)g_pGPE->GetPhysicalModeId())
					{
						RETAILMSG(1, (TEXT("Setting DDraw Primary mode to id %d\r\n"), dwModeID));

						sc = g_pGPE->SetMode(dwModeID, NULL, FALSE); // don't change GDI mode (just HW)
						if (FAILED(sc))
						{
							RETAILMSG(1, (TEXT("Failed to set the physical mode 0x%08x\r\n"), sc));
						}

						RETAILMSG(1, (TEXT("Calling UpdateHALInit\r\n") ));
						UpdateHALInit(pd->lpDD, dwModeID);
						RETAILMSG(1, (TEXT("Done calling UpdateHALInit\r\n") ));

						/*
						// This is handled by UpdateHALInit, so we shouldn't have to call this any more
						pd->lpDD->vmiData.ddpfDisplay.dwFlags = DDPF_RGB; // NOTENOTE this must be updated when splitting or changing display mode
						pd->lpDD->vmiData.ddpfDisplay.dwFlags |= (pDDGPESurf->HasAlpha()) ? DDPF_ALPHAPIXELS : 0;

						pd->lpDD->vmiData.ddpfDisplay.dwRBitMask = pd->lpDD->lpModeInfo[dwModeID].dwRBitMask;
 						pd->lpDD->vmiData.ddpfDisplay.dwGBitMask = pd->lpDD->lpModeInfo[dwModeID].dwGBitMask;
 						pd->lpDD->vmiData.ddpfDisplay.dwBBitMask = pd->lpDD->lpModeInfo[dwModeID].dwBBitMask;
 						pd->lpDD->vmiData.ddpfDisplay.dwRGBAlphaBitMask = pd->lpDD->lpModeInfo[dwModeID].dwAlphaBitMask;
						pd->lpDD->vmiData.ddpfDisplay.dwRGBBitCount = pd->lpDD->lpModeInfo[dwModeID].dwBPP;
						*/
					}
					else
					{
						RETAILMSG(1, (TEXT("Current mode is same as requested split mode (%d)\r\n"), dwModeID));
					}
				}

				PREFAST_ASSERT(g_pDDrawPrimarySurface);

				//RETAILMSG(HAL_ZONE_INFO, (TEXT("Setting up lpGbl\r\n") ));
				pSurf->lpGbl->fpVidMem = (unsigned long)(g_pVideoMemory) + g_pDDrawPrimarySurface->OffsetInVideoMemory();
				pSurf->lpGbl->lPitch = g_pDDrawPrimarySurface->Stride();

				// BUGBUG
				// This will cause a problem if more than one process creates a primary surface
				// but I don't think ddraw lets that happen.
				//RETAILMSG(HAL_ZONE_INFO, (TEXT("Setting DDGPESurf for new ddraw primary surface\r\n") ));
				g_pDDrawPrimarySurface->SetDDGPESurf(pSurf->lpGbl);

				// show the new primary surface						// don't wait for vblank
				//RETAILMSG(HAL_ZONE_INFO, (TEXT("Flipping to the new primary surface\r\n") ));
				g_pGPE->SetVisibleSurface(g_pDDrawPrimarySurface, NULL, FALSE);
				//RETAILMSG(HAL_ZONE_INFO, (TEXT("Done flipping to the new primary surface\r\n") ));

				// OLDOLD
				// last but not least, change the video mode
				// GPE should NOT know about this change
				// DDGPE must keep track of the OLD mode so it knows what to restore it as when
				// FlipToGDISurface is called.
				// TODO: Will there be a case where the caller wants to flip the GDI to front, then flip the
				// ddraw primary to front? Mode switching will need to be done there.
				// NOTE: This is done above, now.
			}
			else // not split primary
			{
				// We could get here in one of two cases:
				// 1. The program is creating ddraw and just wants to create a primary surface
				// 2. The ddraw primary is split and the user is getting the GDI ddraw surface

				// The GDI primary surface should already exist, so we don't need to
				// do any allocation for it

				DDGPESurf* pPrimary = (DDGPESurf*)g_pGPE->PrimarySurface();

				//PrimarySurface() should always return a valid surface
				//ASSERT(pPrimary != NULL);

				if (pPrimary != NULL)
				{
					//RETAILMSG(HAL_ZONE_INFO, (TEXT("Setting up lpGbl\r\n") ));
					pSurf->lpGbl->fpVidMem = (unsigned long)(g_pVideoMemory) + pPrimary->OffsetInVideoMemory();
					pSurf->lpGbl->lPitch = pPrimary->Stride();

					// BUGBUG
					// This will cause a problem if more than one process creates a primary surface
					// but I don't think ddraw lets that happen.
					//RETAILMSG(HAL_ZONE_INFO, (TEXT("Setting DDGPESurf for new ddraw primary surface\r\n") ));
					//
					// Note: pSurf is stored in lcl so we need to attach it to the surface
					pPrimary->SetDDGPESurf(pSurf->lpGbl);

					// No change is required to g_pDDrawPrimarySurface, because it is by default
					// the primary surface.
					// If it's not the primary surface then it's been split and we definitely
					// don't want to change the ddraw primary
					/*
					// This should already be the case in an un-split world
					if (!DDGPEGDIHasSplitFromDDraw())
					{
						g_pDDrawPrimarySurface = pPrimary;
					}
					*/

					// the ddraw primary should already be visible if necessary (it's the GDI surface)
					//g_pGPE->SetVisibleSurface(pPrimary, NULL, FALSE);
				}
			}

			RETAILMSG(
				1 ,
				(TEXT("Create %s Primary Surface! (&GPESurf = 0x%08x, fpvidmem=0x%08x)\r\n"),
				(((pSurf->lpSurfMore->ddsCapsEx.dwCaps4 & DDSCAPS4_NONGDIPRIMARY) == DDSCAPS4_NONGDIPRIMARY) 
					? L"SPLIT" : L""),
				(unsigned long)g_pDDrawPrimarySurface,
				pSurf->lpGbl->fpVidMem
				));
        }
		else
		{
			unsigned long offset;
			if( !(pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY ) )
			{
				sc = g_pGPE->AllocVideoSurface(
						pSurf->lpGbl,
						nWidth,
						nHeight,
						format,
						pixelFormat,//EGPEFormatToEDDGPEPixelFormat[format],
						&offset );

				if( SUCCEEDED(sc) )
				{
					DDGPESurf* pGPESurf = DDGPESurf::GetDDGPESurf(pSurf);

					pSurf->lpGbl->fpVidMem = (unsigned long)(g_pVideoMemory + offset);
					pSurf->lpGbl->lPitch = pGPESurf->Stride();
					pd->lpDDSurfaceDesc->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

					RETAILMSG(1, (TEXT("Created a surface at: 0x%x (%d)\r\n"), offset, offset));
				}
			}
			if( ( ( sc == E_OUTOFMEMORY )
				&& ( ! ( pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY ) )
				&& ( ! ( pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER ) )
				&& ( ! ( pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_BACKBUFFER ) )
				&& ( ! ( pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_COMPLEX ) )
				)
				|| (pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY ) )
			{
				// Try allocating in system memory
				//DWORD	nPitch;
				//nPitch = ( nWidth * nBPP ) >>3;
				//nPitch = ( nPitch + 3 ) & ~3;
				DWORD	nBPP = EGPEFormatToBpp[format];

				// don't LocalAlloc here any more
				//pSurf->lpGbl->lPitch = ( nWidth * nBPP / 8 + 3 ) & 0xFFFFFFFC;
				//pSurf->lpGbl->fpVidMem = (unsigned long)LocalAlloc( LMEM_FIXED, pSurf->lpGbl->lPitch * nHeight );
				//if( !pSurf->lpGbl->fpVidMem )
				//{
				//	pd->ddRVal = DDERR_OUTOFMEMORY;
				//	RETAILMSG(GPE_ZONE_ERROR,(TEXT("Failed to create surface, sc=0x%08x\r\n"),sc));
				//	return DDHAL_DRIVER_HANDLED;
				//}
                if (pd->lpDDSurfaceDesc->dwFlags & DDSD_LPSURFACE)
                {
    				sc = g_pGPE->WrapSurface(
    						pSurf->lpGbl,
    						nWidth,
    						nHeight,
    						format,
    						pixelFormat,//EGPEFormatToEDDGPEPixelFormat[format],
    						(unsigned char *)pd->lpDDSurfaceDesc->lpSurface,
    						pd->lpDDSurfaceDesc->lPitch
    						);
                }
                else
                {
    				sc = g_pGPE->WrapSurface(
    						pSurf->lpGbl,
    						nWidth,
    						nHeight,
    						format,
    						pixelFormat,//EGPEFormatToEDDGPEPixelFormat[format],
    						//(unsigned char *)pSurf->lpGbl->fpVidMem,
    						pSurf->lpGbl->lPitch,
    						0);
                }

				if( SUCCEEDED(sc) )
				{
					RETAILMSG(1, (TEXT("DDGPEHAL: DDGPE has allocated system memory surface for me at 0x%08x\r\n"), pSurf->lpGbl->fpVidMem));
					pSurf->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
				}
			}
			else if ( sc == E_OUTOFMEMORY )
			{
                sc = DDERR_OUTOFVIDEOMEMORY;
				RETAILMSG(1,(TEXT("Not attempting system memory allocation!")));
			}

			if( FAILED( sc ) )
			{
				pd->ddRVal = sc;
				RETAILMSG(1,(TEXT("Failed to create surface, sc=0x%08x\r\n"),sc));
				return DDHAL_DRIVER_HANDLED;
			}

			pd->lpDDSurfaceDesc->lPitch = pSurf->lpGbl->lPitch;
			RETAILMSG(1,(TEXT("Create Non-Primary Surface! (&GPESurf = 0x%08x, fpvidmem=0x%08x)\r\n"),
				DDGPESurf::GetDDGPESurf(pSurf),
				pSurf->lpGbl->fpVidMem));

        }

		// any operations that should be performed on all created surfaces
		if (pSurf != NULL)
		{
			DDGPESurf*	pDDGPESurf = NULL;
			
			pDDGPESurf = DDGPESurf::GetDDGPESurf(pSurf);
			if (pDDGPESurf != NULL)
			{
				pDDGPESurf->SetColorKeyLow(pd->lpDDSurfaceDesc->ddckCKDestBlt.dwColorSpaceLowValue);
				pDDGPESurf->SetColorKeyHigh(pd->lpDDSurfaceDesc->ddckCKDestBlt.dwColorSpaceHighValue);

			}

		}

	} // end of for loop

	pd->ddRVal = DD_OK;
	DEBUGLEAVE( HalCreateSurface );
	return DDHAL_DRIVER_HANDLED;
}

//////////////////////////// DDHAL_DDEXEBUFCALLBACKS ////////////////////////////

DWORD WINAPI HalCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pd )
{
    DEBUGENTER( HalCanCreateSurface );
    /*
    typedef struct _DDHAL_CANCREATESURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL    lpDD;                // driver struct
        LPDDSURFACEDESC            lpDDSurfaceDesc;    // description of surface being created
        DWORD                    bIsDifferentPixelFormat;
                                                    // pixel format differs from primary surface
        HRESULT                    ddRVal;                // return value
        LPDDHAL_CANCREATESURFACE    CanCreateSurface;
                                                    // PRIVATE: ptr to callback
    } DDHAL_CANCREATESURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalCreateExecuteBuffer( LPDDHAL_CREATESURFACEDATA pd )
{
    DEBUGENTER( HalCreateExecuteBuffer );
    /*
    typedef struct _DDHAL_CREATESURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDSURFACEDESC             lpDDSurfaceDesc;// description of surface being created
        LPDDRAWI_DDRAWSURFACE_LCL   FAR *lplpSList; // list of created surface objects
        DWORD                       dwSCnt;         // number of surfaces in SList
        HRESULT                     ddRVal;         // return value
        LPDDHAL_CREATESURFACE       CreateSurface;  // PRIVATE: ptr to callback
    } DDHAL_CREATESURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}


DWORD WINAPI HalDestroyExecuteBuffer( LPDDHAL_DESTROYSURFACEDATA pd )
{
    DEBUGENTER( HalDestroyExecutebuffer );
    /*
    typedef struct _DDHAL_DESTROYSURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_DESTROYSURFACE DestroySurface;// PRIVATE: ptr to callback
        BOOL                        fDestroyGlobal;
    } DDHAL_DESTROYSURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalLock( LPDDHAL_LOCKDATA pd )
{
    DEBUGENTER( HalLock );
    /*
    typedef struct _DDHAL_LOCKDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       bHasRect;       // rArea is valid
        RECTL                       rArea;          // area being locked
        LPVOID                      lpSurfData;     // pointer to screen memory (return value)
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_LOCK          Lock;           // PRIVATE: ptr to callback
    } DDHAL_LOCKDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalUnlock( LPDDHAL_UNLOCKDATA pd )
{
    DEBUGENTER( HalUnlock );
    /*
    typedef struct _DDHAL_UNLOCKDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        HRESULT                     ddRVal;            // return value
        LPDDHALSURFCB_UNLOCK        Unlock;         // PRIVATE: ptr to callback
    } DDHAL_UNLOCKDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

//////////////////////////// DDHAL_DDSURFACECALLBACKS ////////////////////////////

DWORD WINAPI HalDestroySurface( LPDDHAL_DESTROYSURFACEDATA pd )
{
	DEBUGENTER( HalDestroySurface );
	/*
	typedef struct _DDHAL_DESTROYSURFACEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
	    HRESULT                     ddRVal;         // return value
	    LPDDHALSURFCB_DESTROYSURFACE DestroySurface;// PRIVATE: ptr to callback
		BOOL						fDestroyGlobal;
	} DDHAL_DESTROYSURFACEDATA;
	*/

	//RETAILMSG(1,(TEXT("Destroy GPESurf *:0x%08x\r\n"), DDGPESurf::GetDDGPESurf(pd->lpDDSurface) ));

	// Make sure we're not destroying the GDI surface.
	if( (DDGPESurf*)(g_pGPE->PrimarySurface()) != DDGPESurf::GetDDGPESurf(pd->lpDDSurface) )
	{
		//RETAILMSG(1,(TEXT("DeleteSurface\n")));
		DDGPESurf::DeleteSurface(pd->lpDDSurface);
		pd->lpDDSurface = NULL;
	}

	pd->ddRVal = DD_OK;
	DEBUGLEAVE( HalDestroySurface );
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetSurfaceDesc(LPDDHAL_HALSETSURFACEDESCDATA pd)
{
    DEBUGENTER( HalSetSurfaceDesc );
    /*
        typedef struct _DDHAL_HALSETSURFACEDESCDATA
        {
            DWORD           dwSize;                 // Size of this structure
            LPDDRAWI_DDRAWSURFACE_LCL  lpDDSurface; // Surface
            LPDDSURFACEDESC    lpddsd;                    // Description of surface
            HRESULT         ddrval;
        } DDHAL_HALSETSURFACEDESCDATA;
    */

    // Implementation
    pd->ddrval = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalFlip( LPDDHAL_FLIPDATA pd )
{
	S3C2450Surf               * lpSurfInfo;	
    DEBUGENTER( HalFlip );
    /*
    typedef struct _DDHAL_FLIPDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfCurr;     // current surface
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfTarg;     // target surface (to flip to)
        DWORD                       dwFlags;        // flags
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_FLIP          Flip;           // PRIVATE: ptr to callback
    } DDHAL_FLIPDATA;
    */

    if( IS_BUSY )
    {
        if( pd->dwFlags & DDFLIP_WAIT )
        {
            WAIT_FOR_NOT_BUSY;
        }
        else
        {
            DEBUGMSG(GPE_ZONE_FLIP,(TEXT("Graphics engine busy\r\n")));
            pd->ddRVal = DDERR_WASSTILLDRAWING;
            return DDHAL_DRIVER_HANDLED;
        }
    }

    //////
    //WAIT_FOR_VBLANK;
    //////
    lpSurfInfo = (S3C2450Surf *)pd->lpSurfTarg->lpGbl->dwReserved1;
	if ( pd->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_OVERLAY )
	{
		((S3C2450DISP *)g_pGPE)->fpOverlayFlipFrom = pd->lpSurfTarg->lpGbl->fpVidMem;
		lpSurfInfo->m_bIsOverlay = TRUE;
	}
	else
	{
		lpSurfInfo->m_bIsOverlay = FALSE;
	}

    DDGPEFlip(pd);


    //RETAILMSG(1,(TEXT("Flip!\r\n")));
    DEBUGMSG(GPE_ZONE_FLIP,(TEXT("Flip done\r\n")));

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetClipList( LPDDHAL_SETCLIPLISTDATA pd )
{
    DEBUGENTER( HalSetClipList );
    /*
    typedef struct _DDHAL_SETCLIPLISTDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_SETCLIPLIST   SetClipList;    // PRIVATE: ptr to callback
    } DDHAL_SETCLIPLISTDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}


DWORD WINAPI HalBlt( LPDDHAL_BLTDATA pd )
{
	DEBUGENTER( HalBlt );
	/*
	typedef struct _DDHAL_BLTDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL   lpDDDestSurface;// dest surface
	    RECTL                       rDest;          // dest rect
	    LPDDRAWI_DDRAWSURFACE_LCL   lpDDSrcSurface; // src surface
	    RECTL                       rSrc;           // src rect
	    DWORD                       dwFlags;        // blt flags
	    DWORD                       dwROPFlags;     // ROP flags (valid for ROPS only)
	    DDBLTFX                     bltFX;          // blt FX
	    HRESULT                     ddRVal;         // return value
	    LPDDHALSURFCB_BLT           Blt;            // PRIVATE: ptr to callback
	} DDHAL_BLTDATA;
	*/
	RETAILMSG(1,(TEXT("BLT (dwflags=%08x) rDest= %d,%d - %d,%d\n"),
		pd->dwFlags,
		pd->rDest.left,
		pd->rDest.top,
		pd->rDest.right,
		pd->rDest.bottom
		)); 

	//DebugBreak();

	DDGPESurf * pDst       = DDGPESurf::GetDDGPESurf(pd->lpDDDestSurface);
	DDGPESurf * pSrc       = (DDGPESurf* )NULL;
	DDGPESurf * pPattern   = (DDGPESurf* )NULL;
	RECT      * prclDst    = (RECT *)&(pd->rDest);
	RECT      * prclSrc    = (RECT *)&(pd->rSrc);
	ULONG       solidColor = 0xffffffff;
	ULONG       bltFlags   = (pd->dwFlags & DDBLT_WAIT)?0x100:0;
	ULONG       rop4;
 	DWORD       dwROP      = pd->bltFX.dwROP >> 16;

	//DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("Blit 0x%x, 0x%x\r\n"), pDst, pDst->OffsetInVideoMemory()));

	if( pd->lpDDSrcSurface )
		pSrc = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);
	else
		pSrc = (DDGPESurf*)NULL;

	if ( ( pd->dwFlags & DDBLT_COLORFILL ) ||
		( ( pd->dwFlags & DDBLT_ROP ) &&
			( dwROP == ( BLACKNESS >> 16 ) || dwROP == ( WHITENESS >> 16 ) ) ) )
	{
		/*
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("DDRAW Dest Info:\n\r") ));
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    left    = %d\n\r"),  pd->rDest.left ));
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    top     = %d\n\r"),  pd->rDest.top ));
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    right   = %d\n\r"),  pd->rDest.right ));
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    bottom  = %d\n\r"),  pd->rDest.bottom ));

		if (pSrc != NULL)
		{
			DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("DDRAW Src Info:\n\r") ));
			DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    left    = %d\n\r"),  pd->rSrc.left ));
			DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    top     = %d\n\r"),  pd->rSrc.top ));
			DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    right   = %d\n\r"),  pd->rSrc.right ));
			DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    bottom  = %d\n\r"),  pd->rSrc.bottom ));
		}
		*/

		// FILL BLT
		RETAILMSG(1,(TEXT("FILL BLT\n")));

		if( ( pd->dwFlags & DDBLT_ROP ) && ( dwROP == ( BLACKNESS >> 16 ) ) )
			rop4 = 0x0000;
		else if( ( pd->dwFlags & DDBLT_ROP ) && ( dwROP == ( WHITENESS >> 16 ) ) )
			rop4 = 0xffff;
		else
		{
			rop4 = 0xf0f0;	// PATCOPY
			solidColor = pd->bltFX.dwFillColor;
                        switch (pDst->Bpp()) {
                        case 1:
                        case 2:
                        case 4:
                        case 8:
                          solidColor &= 0x000000FF;
                          break;
                        case 15:
                        case 16:
                          solidColor &= 0x0000FFFF;
                          break;
                        case 24:
                          solidColor &= 0x00FFFFFF;
                          break;
                        case 32:
                          solidColor &= 0xFFFFFFFF;
                          break;
                        }
		}
	}
	else if ( ( pd->dwFlags & DDBLT_ROP ) && ( dwROP == ( PATCOPY >> 16 ) ) )
	{
		// PAT BLT
		RETAILMSG(1,(TEXT("PAT BLT\n")));
		rop4 = 0xf0f0;	// PATCOPY;
		pPattern = DDGPESurf::GetDDGPESurf((LPDDRAWI_DDRAWSURFACE_LCL)(pd->bltFX.lpDDSPattern));
	}
	else if ( pd->dwFlags & ( DDBLT_KEYSRCOVERRIDE | DDBLT_KEYSRC ) )
	{
		// TRANSPARENT BLT
		RETAILMSG(1,(TEXT("TRANSPARENT BLT\n")));

		if ( pd->dwFlags & DDBLT_KEYSRCOVERRIDE )
		{
			RETAILMSG(1,(TEXT("DDBLT_KEYSRCOVERRIDE, xparent = 0x%x\n"),pd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue));
			solidColor = pd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
		}
		else
		{
			RETAILMSG(1,(TEXT("DDBLT_KEYSRC, xparent = 0x%x\n"),pd->lpDDSrcSurface->ddckCKSrcBlt.dwColorSpaceLowValue));
			solidColor = pd->lpDDSrcSurface->ddckCKSrcBlt.dwColorSpaceLowValue;
		}

		rop4 = 0xCCCC;	// SRCCOPY
		bltFlags |= BLT_TRANSPARENT;

		// BLT_STRETCH?
		if ((prclDst != NULL) && (prclSrc != NULL))
		{
			if (( (prclDst->bottom - prclDst->top) != (prclSrc->bottom - prclSrc->top))
				|| ((prclDst->right - prclDst->left)  != (prclSrc->right - prclSrc->left) ))
			{
				bltFlags |= BLT_STRETCH;
			}
		}
	}
	else
	{
		// SIMPLE BLT
		RETAILMSG(1,(TEXT("SIMPLE BLT\n")));

		// BLT_STRETCH?
		if ((prclDst != NULL) && (prclSrc != NULL))
		{
			if (( (prclDst->bottom - prclDst->top) != (prclSrc->bottom - prclSrc->top))
				|| ((prclDst->right - prclDst->left)  != (prclSrc->right - prclSrc->left) ))
			{
				bltFlags |= BLT_STRETCH;
			}
		}

		rop4 = 0xCCCC;	// SRCCOPY

	}

	/*
	DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("DDRAW Dest Info:\n\r") ));
	DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    left    = %d\n\r"),  prclDst->left ));
	DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    top     = %d\n\r"),  prclDst->top ));
	DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    right   = %d\n\r"),  prclDst->right ));
	DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    bottom  = %d\n\r"),  prclDst->bottom ));

	if (pSrc != NULL)
	{
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("DDRAW Src Info:\n\r") ));
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    left    = %d\n\r"),  prclSrc->left ));
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    top     = %d\n\r"),  prclSrc->top ));
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    right   = %d\n\r"),  prclSrc->right ));
		DEBUGMSG(GPE_ZONE_BLT_HI, (TEXT("    bottom  = %d\n\r"),  prclSrc->bottom ));
	}
	*/
		
	pd->ddRVal = g_pGPE->BltExpanded(
					pDst,
					pSrc,
					pPattern,
					NULL, // pMask
					NULL, // pco
					NULL, // pxlo
					prclDst,
					prclSrc,
					solidColor,
					bltFlags,
					rop4
					);

	DEBUGLEAVE( HalBlt );
	return DDHAL_DRIVER_HANDLED;
}

// not to be confused with
// DWORD WINAPI HalSetColorKey( LPDDHAL_DRVSETCOLORKEYDATA pd )
DWORD WINAPI HalSetColorKey( LPDDHAL_SETCOLORKEYDATA pd )
{
    DEBUGENTER( HalSetColorKey );
    /*
    typedef struct _DDHAL_DRVSETCOLORKEYDATA
    {
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       dwFlags;        // flags
        DDCOLORKEY                  ckNew;          // new color key
        HRESULT                     ddRVal;         // return value
        LPDDHAL_SETCOLORKEY         SetColorKey;    // PRIVATE: ptr to callback
    } DDHAL_DRVSETCOLORKEYDATA;
    */

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalAddAttachedSurface( LPDDHAL_ADDATTACHEDSURFACEDATA pd )
{
    DEBUGENTER( HalAddAttachedSurface );
    /*
    typedef struct _DDHAL_ADDATTACHEDSURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL        lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL    lpDDSurface;    // surface struct
        LPDDRAWI_DDRAWSURFACE_LCL    lpSurfAttached; // surface to attach
        HRESULT                        ddRVal;         // return value
        LPDDHALSURFCB_ADDATTACHEDSURFACE AddAttachedSurface;
                                                    // PRIVATE: ptr to callback
    } DDHAL_ADDATTACHEDSURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalGetBltStatus( LPDDHAL_GETBLTSTATUSDATA pd )
{
    DEBUGENTER( HalGetBltStatus );
    /*
    typedef struct _DDHAL_GETBLTSTATUSDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       dwFlags;        // flags
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_GETBLTSTATUS  GetBltStatus;   // PRIVATE: ptr to callback
    } DDHAL_GETBLTSTATUSDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pd )
{
    DEBUGENTER( HalGetFlipStatus );
    /*
    typedef struct _DDHAL_GETFLIPSTATUSDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       dwFlags;        // flags
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_GETFLIPSTATUS GetFlipStatus;  // PRIVATE: ptr to callback
    } DDHAL_GETFLIPSTATUSDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalUpdateOverlay( LPDDHAL_UPDATEOVERLAYDATA lpUpdateOverlay )
{
    LPDDRAWI_DDRAWSURFACE_LCL   lpLcl = lpUpdateOverlay->lpDDSrcSurface;
    LPDDRAWI_DDRAWSURFACE_GBL   lpGbl = lpLcl->lpGbl;
    LPDDRAWI_DDRAWSURFACE_GBL   lpSource;
    LPDDRAWI_DDRAWSURFACE_GBL   lpDestination;
    S3C2450Surf               * lpSurfInfo;
    DWORD                       dwStride;
//    LONG                        lSrcWidth;
//    LONG                        lSrcHeight;
//    LONG                        lDstWidth;
//    LONG                        lDstHeight;
//    LONG                        lHInc;
    DWORD /*SrcBufOffset,*/ Temp;
//    RECTL rOverlay;
//    DWORD dwVideoInSave;

    BOOL  bAllocatedOverlay = FALSE;
    ULONG ulController = 0;

    DEBUGENTER( HalUpdateOverlay );

    /* 'Source' is the overlay surface, 'destination' is the surface to
     * be overlayed:
     */

    lpSource = lpUpdateOverlay->lpDDSrcSurface->lpGbl;
	
//	RETAILMSG(1,(TEXT("HalUpdateOverlay  lpSource->fpVidMem=0x%08X\n"),lpSource->fpVidMem));

    dwStride = lpSource->lPitch;

	//RETAILMSG(1,(TEXT("%X %X %X %X %X\n"),DDOVER_HIDE,DDOVER_SHOW,DDOVER_KEYSRCOVERRIDE,DDOVER_DDFX,lpUpdateOverlay->dwFlags));
    if (lpUpdateOverlay->dwFlags & DDOVER_HIDE)
    {
        /* Turn off overlay */

        if (lpSource->fpVidMem)
        {
        	//RETAILMSG(1,(TEXT("((S3C2450DISP *)g_pGPE)->fpOverlayFlipFrom=0x%08X\n"),((S3C2450DISP *)g_pGPE)->fpOverlayFlipFrom));
            if ((lpSource->fpVidMem == (FLATPTR) ((S3C2450DISP *)g_pGPE)->fpVisibleOverlay) ||
            	(lpSource->fpVidMem == (FLATPTR) ((S3C2450DISP *)g_pGPE)->fpOverlayFlipFrom))
            {
            	((S3C2450DISP *)g_pGPE)->fpVisibleOverlay = 0;
            	((S3C2450DISP *)g_pGPE)->fpOverlayFlipFrom = 0;
                ((S3C2450DISP *)g_pGPE)->DisableOverlay();
            }
        }

        lpUpdateOverlay->ddRVal = DD_OK;
        return (DDHAL_DRIVER_HANDLED);
    }

    /*
     * Dereference 'lpDDDestSurface' only after checking for the DDOVER_HIDE
     * case:
     */

    lpDestination = lpUpdateOverlay->lpDDDestSurface->lpGbl;

    /* check, if the overlay was already allocated and in use */

    if (lpSource->fpVidMem)
    {
        if ( (lpSource->fpVidMem != (FLATPTR) ((S3C2450DISP *)g_pGPE)->fpVisibleOverlay) &&
        	(lpSource->fpVidMem != (FLATPTR) ((S3C2450DISP *)g_pGPE)->fpOverlayFlipFrom))
        {
            if (lpUpdateOverlay->dwFlags & DDOVER_SHOW)
            {
                if (((S3C2450DISP *)g_pGPE)->fpVisibleOverlay)
                {
                    // Some other overlay is already visible:

                    lpUpdateOverlay->ddRVal = DDERR_OUTOFCAPS;
                    return (DDHAL_DRIVER_HANDLED);
                }
                else
                {
                    // We're going to make the overlay visible, so mark it as
                    // such:
                    ((S3C2450DISP *)g_pGPE)->fpVisibleOverlay = lpSource->fpVidMem;

                    // initialize overlay hardware
                    //((S3C2450DISP *)g_pGPE)->EnableOverlay();
                }
            }
            else
            {
                // The overlay isn't visible, and we haven't been asked to make
                // it visible, so this call is trivially easy:

                lpUpdateOverlay->ddRVal = DD_OK;
                return(DDHAL_DRIVER_HANDLED);
            }
        }
    }
    
    lpSurfInfo = (S3C2450Surf *) lpGbl->dwReserved1;
    
    ((S3C2450DISP *)g_pGPE)->InitOverlay(lpSurfInfo, lpUpdateOverlay->rSrc);
    ((S3C2450DISP *)g_pGPE)->SetOverlayPosition(lpUpdateOverlay->rDest.left,lpUpdateOverlay->rDest.top, lpSurfInfo->Width(), lpSurfInfo->Height());

    /* Set up the colour keying, if any? */

    if (lpUpdateOverlay->dwFlags & DDOVER_KEYSRC          ||
         lpUpdateOverlay->dwFlags & DDOVER_KEYSRCOVERRIDE  ||
         lpUpdateOverlay->dwFlags & DDOVER_KEYDEST         ||
         lpUpdateOverlay->dwFlags & DDOVER_KEYDESTOVERRIDE)
    {

        if (lpUpdateOverlay->dwFlags & DDOVER_KEYSRC ||
             lpUpdateOverlay->dwFlags & DDOVER_KEYSRCOVERRIDE)
        {
            //Set source colour key
            if (lpUpdateOverlay->dwFlags & DDOVER_KEYSRC)
            {
                Temp = lpUpdateOverlay->lpDDDestSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
            }
            else
            {
                Temp = lpUpdateOverlay->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
            }
            ((S3C2450DISP *)g_pGPE)->SetOverlayColorKey(TRUE,Temp);
		}
    }
    else
    {
		((S3C2450DISP *)g_pGPE)->SetOverlayColorKey(FALSE,0);
    }
    
     /* Set up the alpha blending, if any? */
    if (lpUpdateOverlay->dwFlags & DDOVER_ALPHASRC          ||
         lpUpdateOverlay->dwFlags & DDOVER_ALPHASRCCONSTOVERRIDE)
    {
        //Set source colour key
        if (lpUpdateOverlay->dwFlags & DDOVER_ALPHASRC)
        {
            ((S3C2450DISP *)g_pGPE)->SetOverlayAlpha(TRUE, TRUE, 0);
        }
        else
        {
            ((S3C2450DISP *)g_pGPE)->SetOverlayAlpha(TRUE, FALSE, lpUpdateOverlay->overlayFX.dwAlphaSrcConst);
        }
    }
    else
    {
		((S3C2450DISP *)g_pGPE)->SetOverlayAlpha(FALSE, FALSE, 0);
    }    
    
    ((S3C2450DISP *)g_pGPE)->EnableOverlay();
    /*
     * return to DirectDraw.
     */
	//RETAILMSG(1,(TEXT("UpdateOverlay End!!\n")));
    lpUpdateOverlay->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetOverlayPosition( LPDDHAL_SETOVERLAYPOSITIONDATA pd )
{
	S3C2450Surf               * lpSurfInfo;	
    DEBUGENTER( HalSetOverlayPosition );
    /*
    typedef struct _DDHAL_SETOVERLAYPOSITIONDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSrcSurface; // src surface
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDDestSurface;// dest surface
        LONG                        lXPos;          // x position
        LONG                        lYPos;          // y position
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_SETOVERLAYPOSITION SetOverlayPosition;
                                                    // PRIVATE: ptr to callback
    } DDHAL_SETOVERLAYPOSITIONDATA;
    */

    // Implementation
	lpSurfInfo = (S3C2450Surf *)pd->lpDDSrcSurface->lpGbl->dwReserved1;
	
	if( (pd->lXPos+lpSurfInfo->Width() <= LCD_XSIZE_TFT) && (pd->lYPos+lpSurfInfo->Height() <= LCD_YSIZE_TFT))
	{
		((S3C2450DISP *)g_pGPE)->SetOverlayPosition(pd->lXPos,pd->lYPos, lpSurfInfo->Width(), lpSurfInfo->Height());
		pd->ddRVal = DD_OK;
	}
	else
	{
		pd->ddRVal = DDERR_INVALIDPOSITION;
	}
	

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetPalette( LPDDHAL_SETPALETTEDATA pd )
{
    DEBUGENTER( HalSetPalette );
    /*
    typedef struct _DDHAL_SETPALETTEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        LPDDRAWI_DDRAWPALETTE_GBL   lpDDPalette;    // palette to set to surface
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_SETPALETTE    SetPalette;     // PRIVATE: ptr to callback
        BOOL                        Attach;         // attach this palette?
    } DDHAL_SETPALETTEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}


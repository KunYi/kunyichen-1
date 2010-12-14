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

Module Name:	blt.cpp

Abstract:	Related to Blit function

Functions:	BltPrepare, BltComplete, ...

Notes:

--*/

#include "precomp.h"
#include <dispperf.h>

#if	G2D_ACCELERATE
DDGPESurf *gpScratchSurf;
GPESurf *oldSrcSurf;
#endif

#define TEMP_DEBUG	(FALSE)
#define FULL_TEST_OK	(0)

SCODE S3C2450DISP::BltPrepare(GPEBltParms *blitParameters)
{
	RECTL rectl;
	int   iSwapTmp;
	BOOL  bRotate = FALSE;

	DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::BltPrepare\r\n")));

#ifdef DO_DISPPERF
	DispPerfStart(blitParameters->rop4);
	DispPerfParam(pBltParms);
#endif 

    // default to base EmulatedBlt routine
    blitParameters->pBlt = EmulatedBlt;

    // see if we need to deal with cursor

    // check for destination overlap with cursor and turn off cursor if overlaps
    if (blitParameters->pDst == m_pPrimarySurface)    // only care if dest is main display surface
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {    
            if (blitParameters->prclDst != NULL)        // make sure there is a valid prclDst
            {
                rectl = *blitParameters->prclDst;        // if so, use it

                // There is no guarantee of a well ordered rect in blitParamters
                // due to flipping and mirroring.
                if(rectl.top > rectl.bottom)
                {
                    iSwapTmp     = rectl.top;
                    rectl.top    = rectl.bottom;
                    rectl.bottom = iSwapTmp;
                }

                if(rectl.left > rectl.right)
                {
                    iSwapTmp    = rectl.left;
                    rectl.left  = rectl.right;
                    rectl.right = iSwapTmp;
                }
            }
            else
            {
                rectl = m_CursorRect;                    // if not, use the Cursor rect - this forces the cursor to be turned off in this case
            }

            if (m_CursorRect.top <= rectl.bottom && m_CursorRect.bottom >= rectl.top &&
                m_CursorRect.left <= rectl.right && m_CursorRect.right >= rectl.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }        

		if (m_iRotate )	// if screen (destination primary surface) is rotated.
        {
            bRotate = TRUE;
        }
    }

    // check for source overlap with cursor and turn off cursor if overlaps
    if (blitParameters->pSrc == m_pPrimarySurface)    // only care if source is main display surface
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {
            if (blitParameters->prclSrc != NULL)        // make sure there is a valid prclSrc
            {
                rectl = *blitParameters->prclSrc;        // if so, use it
            }
            else
            {
                rectl = m_CursorRect;                    // if not, use the CUrsor rect - this forces the cursor to be turned off in this case
            }
            if (m_CursorRect.top < rectl.bottom && m_CursorRect.bottom > rectl.top &&
                m_CursorRect.left < rectl.right && m_CursorRect.right > rectl.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }

        
		if (m_iRotate) //if screen(source primary surface) is rotated
        {
            bRotate = TRUE;
        }        
    }

	if (bRotate)	// if source or desitnation surface is rotated.
    {
        blitParameters->pBlt = (SCODE (GPE::*)(GPEBltParms *))EmulatedBltRotate;
    }

//	EmulatedBltSelect02(blitParameters);
//	EmulatedBltSelect08(blitParameters);
#if (BSP_TYPE == BSP_SMDK2450)
#if G2D_ACCELERATE
	if(	//(m_VideoPowerState != VideoPowerOff) &&		// to avoid hanging while bring up display H/W
		((S3C2450Surf *)(blitParameters->pDst))->InVideoMemory() &&
		(((S3C2450Surf *)(blitParameters->pDst))->Format() == gpe16Bpp) 
//		&&	blitParameters->pBlt == &GPE::EmulatedBlt		// Disable Rotation Case
		)
	{
		AcceleratedBltSelect16(blitParameters);
	}
	else
	{
#endif	
#endif
//	EmulatedBltSelect16(blitParameters);
#if (BSP_TYPE == BSP_SMDK2450)
#if G2D_ACCELERATE
	}
	m_oG2D->WaitForIdle();	//< Wait for Fully Empty Command Fifo for all HW BitBlt request
#endif
#endif


    return S_OK;
}

// This function would be used to undo the setting of clip registers etc
SCODE    S3C2450DISP::BltComplete(GPEBltParms *blitParameters)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::BltComplete\r\n")));

    // see if cursor was forced off because of overlap with source or destination and turn back on
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }

#if	G2D_ACCELERATE
	if(gpScratchSurf)
	{
		blitParameters->pSrc = oldSrcSurf;
		delete gpScratchSurf;
		gpScratchSurf=NULL;
	}
#endif
    	
    
#ifdef DO_DISPPERF
    DispPerfEnd(0);
#endif
    return S_OK;
}


/**
*	@fn	SCODE S3C2450DISP::AcceleratedBltFIll(GPEBltParms *pBltParms)
*	@brief	Rectangle Solid Filling Function. Solid Fill has no Source Rectangle
*	@param	pBltParms	Blit Parameter Information Structure
*	@sa		GPEBltParms
*	@note	ROP : 0xF0F0
*	@note	Using Information : DstSurface, ROP, Solidcolor
*/
#if (BSP_TYPE == BSP_SMDK2450)
SCODE S3C2450DISP::AcceleratedBltFill(GPEBltParms *pBltParms)
{
	PRECTL    prclDst         = pBltParms->prclDst;
	DEBUGMSG(GPE_ZONE_BLT_LO, (TEXT("++S3C6410DISP::AcceleratedBltFill\r\n")));

	/**
	*	Prepare Source & DestinationSurface Information
	*
	*/
//	m_oG2D->Init();
	
	SURFACE_DESCRIPTOR descDstSurface;
	
	// When Screen is rotated, ScreenHeight and ScreenWidth always has initial surface property.
	descDstSurface.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + (( S3C2450Surf *)(pBltParms->pDst))->OffsetInVideoMemory());
	descDstSurface.dwColorMode = pBltParms->pDst->Format();
	descDstSurface.dwHoriRes = pBltParms->pDst->Stride()/(EGPEFormatToBpp[pBltParms->pDst->Format()]/8);
	descDstSurface.dwVertRes = pBltParms->pDst->ScreenHeight();	

	m_oG2D->SetSrcSurface(&descDstSurface);		// Fill dummy value
	m_oG2D->SetDstSurface(&descDstSurface);	
	if(pBltParms->prclClip)
	{
		m_oG2D->SetClipWindow(pBltParms->prclClip);
	}
	else
	{
		if(pBltParms->pDst->IsRotate())
		{
			RotateRectl(prclDst);
			RETAILMSG(FULL_TEST_OK, (TEXT("AfterRotate DstRect:(L%d,T%d,R%d,B%d)\r\n"),
				prclDst->left, prclDst->top, prclDst->right, prclDst->bottom			
			));
		}
		m_oG2D->SetClipWindow(prclDst);	
	}
	m_oG2D->SetRopEtype(ROP_SRC_ONLY);
	RETAILMSG(FULL_TEST_OK, (TEXT("FillRectA %x\r\n"), this));
	RETAILMSG(FULL_TEST_OK, (TEXT("BlitFill : Dst Stride:%d, W:%d, H:%d, Screen(W:%d,H:%d), DstRect:(L%d,T%d,R%d,B%d)\r\n"),
			pBltParms->pDst->Stride(), pBltParms->pDst->Width(), pBltParms->pDst->Height(),
			pBltParms->pDst->ScreenWidth(), pBltParms->pDst->ScreenHeight(),
			prclDst->left, prclDst->top, prclDst->right, prclDst->bottom
			));
	EnterCriticalSection(&m_cs2D);
	m_oG2D->FillRect(prclDst, pBltParms->solidColor);
	LeaveCriticalSection(&m_cs2D);	
	RETAILMSG(FULL_TEST_OK, (TEXT("FillRectB\r\n")));

	if(pBltParms->pDst->IsRotate())
	{
		RotateRectlBack(prclDst);
		RETAILMSG(FULL_TEST_OK, (TEXT("End Fill DstRect:(L%d,T%d,R%d,B%d)\r\n"),
			prclDst->left, prclDst->top, prclDst->right, prclDst->bottom		
		));		
	}
		
	DEBUGMSG(GPE_ZONE_BLT_LO, (TEXT("--S3C2450DISP::AcceleratedBltFill\r\n")));

	return	S_OK;
}
#else
SCODE S3C2450DISP::AcceleratedBltFill(GPEBltParms *pBltParms)
{
	return S_OK;
}
#endif

SCODE S3C2450DISP::AcceleratedDestInvert(GPEBltParms *pBltParms)
{
	int		dstX   = pBltParms->prclDst->left;
	int		dstY   = pBltParms->prclDst->top;
	int		width  = pBltParms->prclDst->right  - pBltParms->prclDst->left;
	int		height = pBltParms->prclDst->bottom - pBltParms->prclDst->top;
//	printf("DestInvert need to implement");
	return	S_OK;
}

/// From Frame Buffer to Frame Buffer Directly
///	Constraints
/// Source Surface's width is same with Destination Surface's width.
/// Source and Dest must be in Video FrameBuffer Region
/// In Surface Format
/// ScreenHeight and ScreenWidth means logical looking aspect for application
/// Height and Width means real data format.
/**
*	@fn	SCODE S3C2450DISP::AcceleratedSrcCopyBlt(GPEBltParms *pBltParms)
*	@brief	Do Blit with SRCCOPY, SRCAND, SRCPAINT, SRCINVERT
*	@param	pBltParms	Blit Parameter Information Structure
*	@sa		GPEBltParms
*	@note	ROP : 0xCCCC(SRCCOPY), 0x8888(SRCAND), 0x6666(SRCINVERT), 0XEEEE(SRCPAINT)
*	@note	Using Information : DstSurface, ROP, Solidcolor
*/
#if (BSP_TYPE == BSP_SMDK2450)
SCODE S3C2450DISP::AcceleratedSrcCopyBlt (GPEBltParms *pBltParms)
{
	PRECTL prclSrc, prclDst;
	RECT rectlSrcBackup;
	BOOL bHWSuccess = FALSE;
	prclSrc = pBltParms->prclSrc;
	prclDst = pBltParms->prclDst;

	// Set Destination Offset In Video Memory, this point is Dest lefttop point
	//
//	m_oG2D->Init();

	/**
	*	Prepare Source & Destination Surface Information
	*/
	SURFACE_DESCRIPTOR descSrcSurface, descDstSurface;
	DWORD	dwSrcBaseAddrOffset = 0;
	DWORD	dwTopStrideStartAddr = 0;

	descSrcSurface.dwColorMode = pBltParms->pSrc->Format();
	/// !!!!Surface Width can not match to Real Data format!!!!
	/// !!!!Set Width by Scan Stride Size!!!!
	descSrcSurface.dwHoriRes = pBltParms->pSrc->Stride()/ (EGPEFormatToBpp[pBltParms->pSrc->Format()]/8);
	/// If source surface is created by user temporary, that has no screen width and height.
	descSrcSurface.dwVertRes = (pBltParms->pSrc->ScreenHeight() != 0 ) ? pBltParms->pSrc->ScreenHeight() : pBltParms->pSrc->Height();	

	 if(pBltParms->pDst->IsRotate())
	 {
		RotateRectl(prclDst);		//< RotateRectl rotate rectangle with screen rotation information
		if(pBltParms->prclClip)
		{
			RotateRectl(pBltParms->prclClip);
		}
	 }
	 if(pBltParms->pSrc->IsRotate())
	 {
		RotateRectl(prclSrc);
	 }		 

	if (pBltParms->bltFlags & BLT_TRANSPARENT)
	{
		RETAILMSG(0,(TEXT("TransparentMode Color : %d\n"), pBltParms->solidColor));
		m_oG2D->SetTransparentMode(1, pBltParms->solidColor);		// turn on transparency & set comparison color
	}

	switch (pBltParms->rop4)
	{
		case	0x6666:	// SRCINVERT
			RETAILMSG(G2D_MSG, (TEXT("SRCINVERT\r\n")));
			m_oG2D->SetRopEtype(ROP_SRC_XOR_DST);
			break;

		case	0x8888:	// SRCAND
			RETAILMSG(G2D_MSG, (TEXT("SRCAND\r\n")));
			m_oG2D->SetRopEtype(ROP_SRC_AND_DST);
			break;

		case	0xCCCC:	// SRCCOPY
			RETAILMSG(G2D_MSG, (TEXT("SRCCOPY\r\n")));
			m_oG2D->SetRopEtype(ROP_SRC_ONLY);
			break;

		case	0xEEEE:	// SRCPAINT
			RETAILMSG(G2D_MSG, (TEXT("SRCPAINT\r\n")));
			m_oG2D->SetRopEtype(ROP_SRC_OR_DST);
			break;
	}
	

	/// Check Source Rectangle Address
	/// HW Coordinate limitation is 2048
	/// 1. Get the Top line Start Address
	/// 2. Set the base offset to Top line Start Address
	/// 3. Recalulate top,bottom rectangle
	/// 4. Do HW Bitblt

	CopyRect(&rectlSrcBackup, (LPRECT)pBltParms->prclSrc);
	/// Set Source Surface Information
	/// If using PLA control for surface in virtual address, use PLA
	if((pBltParms->pSrc)->InVideoMemory() )	{
		descSrcSurface.dwBaseaddr = IMAGE_FRAMEBUFFER_DMA_BASE + pBltParms->pSrc->OffsetInVideoMemory();
		descSrcSurface.dwVertRes = (pBltParms->pSrc->ScreenHeight() != 0 ) ? pBltParms->pSrc->ScreenHeight() : pBltParms->pSrc->Height();			
	} 
	else {
		dwTopStrideStartAddr = m_dwSourceSurfacePA + pBltParms->prclSrc->top * pBltParms->pSrc->Stride();
		descSrcSurface.dwBaseaddr = dwTopStrideStartAddr;
		descSrcSurface.dwVertRes = pBltParms->prclSrc->bottom - pBltParms->prclSrc->top;

		pBltParms->prclSrc->top = 0;
		pBltParms->prclSrc->bottom = descSrcSurface.dwVertRes;
	}

	/// Set Destination Surface Information
	descDstSurface.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + (( S3C2450Surf *)(pBltParms->pDst))->OffsetInVideoMemory());
	descDstSurface.dwColorMode = pBltParms->pDst->Format();
	descDstSurface.dwHoriRes = pBltParms->pDst->Stride()/ (EGPEFormatToBpp[pBltParms->pDst->Format()]/8);
	descDstSurface.dwVertRes = pBltParms->pDst->ScreenHeight();	

	/// Transparency does not relate with alpha blending
	m_oG2D->SetAlphaMode(G2D_NO_ALPHA_MODE);
	/// No transparecy with alphablend
	m_oG2D->SetAlphaValue(0xff);
	m_oG2D->Set3rdOperand(G2D_OPERAND3_PAT);

	/// Real Register Surface Description setting will be done in HWBitBlt
	bHWSuccess = HWBitBlt(pBltParms, &descSrcSurface, &descDstSurface);

	CopyRect((LPRECT)pBltParms->prclSrc, &rectlSrcBackup);

	 if(pBltParms->pDst->IsRotate())
	 {
		RotateRectlBack(prclDst);
		if(pBltParms->prclClip)
		{
			RotateRectlBack(pBltParms->prclClip);
		}		
	 }
	 if(pBltParms->pSrc->IsRotate())
	 {
		RotateRectlBack(prclSrc);
	 }		 
	if (pBltParms->bltFlags & BLT_TRANSPARENT)
	{
		m_oG2D->SetTransparentMode(0, pBltParms->solidColor);			// turn off Transparency
	}
	if(!bHWSuccess)
	{	
		return EmulatedBlt(pBltParms);
	}

	return	S_OK;
}
#endif
#if (BSP_TYPE == BSP_SMDK2450)
/**
*	@fn		void S3C2450Disp::HWBitBlt(GpeBltParms *pBltParms)
*	@brief	Check Further Optional processing. This is intemediate layer between display driver and 2D driver
*	@param	pBltParms	Blit Parameter Information Structure
*	@sa		GPEBltParms
*	@note	currently support only rotation
*	@note	DMDO_0	= 0, DMDO_90 = 1, DMDO_180 = 2, DMDO_270 = 4
*/
BOOL S3C2450DISP::HWBitBlt(GPEBltParms *pBltParms, PSURFACE_DESCRIPTOR pdescSrcSurface, PSURFACE_DESCRIPTOR pdescDstSurface)
{
	PRECTL prclSrc;		//< Src is already rotated.
	PRECTL prclDst;		//< Dst is already rotated.
	#if 0 //unused code, f.w.lin
	RECT     rectlPhySrc;		//< If screen is rotated, this value has RotateRectl(prclSrc), physically addressed coordinate.
	RECT	rectlPhyDst;		//< If screen is rotated, this value has RotateRectl(prclDst), physically addressed coordinate.
	#endif
	prclSrc = pBltParms->prclSrc;
	prclDst = pBltParms->prclDst;

    #if 0 //unused code, f.w.lin
	CopyRect(&rectlPhySrc, (LPRECT)prclSrc);
	CopyRect(&rectlPhyDst, (LPRECT)prclDst);
    #endif
    
	int	iRotate = DMDO_0;

#if 0 
	if(pBltParms->prclClip)
	{
	RETAILMSG(0,(TEXT("ClipWindow set, prclClip: 0x%x, Cl:%d, Ct:%d, Cr:%d, Cb:%d\r\n"), pBltParms->prclClip,
		(pBltParms->prclClip ? pBltParms->prclClip->left : 0),
		(pBltParms->prclClip ? pBltParms->prclClip->top : 0),
		(pBltParms->prclClip ? pBltParms->prclClip->right : 0),
		(pBltParms->prclClip ? pBltParms->prclClip->bottom : 0)
		));			
		m_oG2D->SetClipWindow(pBltParms->prclClip);
	}
	else
	{
	RETAILMSG(0,(TEXT("ClipWindow set, prclDst: 0x%x, Cl:%d, Ct:%d, Cr:%d, Cb:%d\r\n"), pBltParms->prclDst,
		(pBltParms->prclDst ? pBltParms->prclDst->left : 0),
		(pBltParms->prclDst ? pBltParms->prclDst->top : 0),
		(pBltParms->prclDst ? pBltParms->prclDst->right : 0),
		(pBltParms->prclDst ? pBltParms->prclDst->bottom : 0)
		));						
		m_oG2D->SetClipWindow(prclDst);
	}	
#endif
	

	/// OnScreen Case
	if(pBltParms->pSrc == pBltParms->pDst)
	{
		iRotate = DMDO_0;
	}
	/// OffScreen Case
	else		
	{
		/// this code assume that source surface is always DMDO_0
		iRotate =	pBltParms->pDst->Rotate();
	}


#define SWAP(x,y, _type)  { _type i; i = x; x = y; y = i; }		
	RETAILMSG(0,(TEXT("HWBitBlt Src(%d,%d)~(%d,%d), Dst(%d,%d)~(%d,%d)\n"),
		prclSrc->left, prclSrc->top, prclSrc->right, prclSrc->bottom,
		prclDst->left, prclDst->top, prclDst->right, prclDst->bottom));
	RotateRectlBack(prclSrc);
	RotateRectlBack(prclDst);	
	/// if Stretch option is set, run StretchBlt
	/// if StretchBlt is on, but source region and destination region are same. use BitBlt
	if(((pBltParms->bltFlags & BLT_STRETCH) == BLT_STRETCH) 
		//< If Does not need to stretch or shrink. just BitBlt is faster.
		&&	ABS(prclSrc->right - prclSrc->left) != ABS(prclDst->right - prclDst->left)		
		&&	ABS(prclSrc->bottom - prclSrc->top) != ABS(prclDst->bottom - prclDst->top)
		)
	{
		RotateRectl(prclSrc);
		RotateRectl(prclDst);		
		RECTL t_rect;
		DWORD	dwSrcWidth;
		DWORD	dwSrcHeight;
		SURFACE_DESCRIPTOR descScratch;

		dwSrcWidth = ABS(prclSrc->right - prclSrc->left);
		dwSrcHeight  = ABS(prclSrc->bottom - prclSrc->top);

		/// Set Scratch Destination Region
		t_rect.left = 0;
		t_rect.top = 0;
		t_rect.right = dwSrcWidth;
		t_rect.bottom = dwSrcHeight;
#if 1
	RETAILMSG(TEMP_DEBUG,(TEXT("t_rect,realstretch: (%d,%d)~(%d,%d), R:%d\r\n"), 
		t_rect.left,		t_rect.top,		t_rect.right,		t_rect.bottom, iRotate		));
#endif
#if TEMP_DEBUG
	if(pBltParms->pSrc)
	{
	RETAILMSG(TEMP_DEBUG,(TEXT("Src:0x%x SrcB 0x%x, Surf(W:%d,H:%d,BPP:%d,STRIDE:%d), Screen(W:%d,H:%d), rect: (%d,%d)~(%d,%d), R:%d\r\n"), 
		pBltParms->pSrc,
		pBltParms->pSrc->Buffer(),
		pBltParms->pSrc->Width(),
		pBltParms->pSrc->Height(),
		EGPEFormatToBpp[pBltParms->pSrc->Format()],
		pBltParms->pSrc->Stride(),
		pBltParms->pSrc->ScreenWidth(),
		pBltParms->pSrc->ScreenHeight(),
		pBltParms->prclSrc->left,
		pBltParms->prclSrc->top,
		pBltParms->prclSrc->right,
		pBltParms->prclSrc->bottom,
		pBltParms->pSrc->Rotate()
		));
	}
	if(pBltParms->pDst)
	{
	RETAILMSG(TEMP_DEBUG,(TEXT("Dst:0x%x DstB 0x%x,  Surf(W:%d,H:%d,BPP:%d,STRIDE:%d), Screen(W:%d,H:%d), rect: (%d,%d)~(%d,%d), R:%d\r\n"), 
		pBltParms->pDst,
		pBltParms->pDst->Buffer(),
		pBltParms->pDst->Width(),
		pBltParms->pDst->Height(),
		EGPEFormatToBpp[pBltParms->pDst->Format()],
		pBltParms->pDst->Stride(),		
		pBltParms->pDst->ScreenWidth(),
		pBltParms->pDst->ScreenHeight(),
		pBltParms->prclDst->left,
		pBltParms->prclDst->top,
		pBltParms->prclDst->right,
		pBltParms->prclDst->bottom,
		pBltParms->pDst->Rotate()
		));									
	}
	RETAILMSG(TEMP_DEBUG, (TEXT("ROP : 0x%0x\r\n"), pBltParms->rop4));				
	RETAILMSG(TEMP_DEBUG, (TEXT("xPositive : %d\r\n"),pBltParms->xPositive));
	RETAILMSG(TEMP_DEBUG, (TEXT("yPositive : %d\r\n"),pBltParms->yPositive));	
#endif			
		/// Set Source Surface Descriptor 
		m_oG2D->SetSrcSurface(pdescSrcSurface);	

		/// Check whether XY flip or not, 
		///if XY flip is requested, just Rotation 180 degree
		RotateRectlBack(prclDst);
		if( (prclDst->right < prclDst->left)  && (prclDst->bottom < prclDst->top) )
		{
			RotateRectl(prclDst);		
			switch(iRotate)
			{
				case DMDO_0:
					iRotate = DMDO_180;
					break;
				case DMDO_90:
					iRotate = DMDO_270;
					break;
				case DMDO_180:
					iRotate = DMDO_0;
					break;
				case DMDO_270:
					iRotate = DMDO_90;
					break;
			}

			/// Set Destination Surface to real Framebuffer Surface
			m_oG2D->SetDstSurface(pdescDstSurface);
			
			/// SWAP rect
			SWAP(prclDst->top, prclDst->bottom, LONG);
			SWAP(prclDst->left, prclDst->right, LONG);			
			/// Set Destination Clipping window Rect
			if(pBltParms->prclClip)
			{
				m_oG2D->SetClipWindow(pBltParms->prclClip);
			}
			else
			{
				m_oG2D->SetClipWindow(prclDst);
			}	
					
			EnterCriticalSection(&m_cs2D);	
			m_oG2D->StretchBlt( prclSrc, prclDst, m_oG2D->GetRotType(iRotate));
			LeaveCriticalSection(&m_cs2D);				
			/// Recover rect
			SWAP(prclDst->top, prclDst->bottom, LONG);
			SWAP(prclDst->left, prclDst->right, LONG);			

			RotateRectl(prclDst);
			
			return TRUE;
			
		}
		RotateRectl(prclDst);		

		
		/// Reconfigure HW to set destination framebuffer address as Scratch Framebuffer
		
		/// Check mirror case, and reset region rectangle
		/// Doing FlipBlt from Source to Sratch
		/// In mirror case, source region does not change.
		/// only destination's regions has reverse coordinate, this cannot be negative.
		if(iRotate == DMDO_90 || iRotate == DMDO_270)
		{	
			RotateRectlBack(prclDst);
		 	if(prclDst->right < prclDst->left)
			{
				RotateRectl(prclDst);
				/// Allocation Scratch Framebuffer for Flip Operation.
				DDGPESurf *ScratchSurf;
				
				AllocSurface(&ScratchSurf, dwSrcWidth, dwSrcHeight, pBltParms->pDst->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pDst->Format()], GPE_REQUIRE_VIDEO_MEMORY);
				if(ScratchSurf == NULL)
				{
					RETAILMSG(TRUE,(TEXT("Scratch Surface Allocation is failed. %d\n"), __LINE__));
#if 0//USE_PACSURF, To increase video memory is better than to use system memory as Video Surface
					RETAILMSG(TRUE,(TEXT("try to allocate surface usign PA Surface\r\n")));
					PACSurf *ScratchSurf;
#endif
					RETAILMSG(TRUE,(TEXT("Maybe There's no sufficient video memory. please increase video memory\r\n")));
					RETAILMSG(TRUE,(TEXT("try to redirect to SW Emulated Bitblt\r\n")));		
					return FALSE;
				}
				

				/// Set Scratch Surface Information
				descScratch.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + ScratchSurf->OffsetInVideoMemory());
				RETAILMSG(TEMP_DEBUG,(TEXT("ScratchBaseAddr : 0x%x\n"), descScratch.dwBaseaddr));
				descScratch.dwColorMode = pBltParms->pDst->Format();
				descScratch.dwHoriRes = dwSrcWidth;
				descScratch.dwVertRes = dwSrcHeight;
			
				/// Set Destination Surface to Scratch Surface
				m_oG2D->SetDstSurface(&descScratch);
				/// Set Destination Clipping window Rect
				m_oG2D->SetClipWindow(&t_rect);
				/// Set Y-axis flip flag
				EnterCriticalSection(&m_cs2D);	
				m_oG2D->FlipBlt( prclSrc, &t_rect,  FLIP_Y );
				LeaveCriticalSection(&m_cs2D);
				/// Y-axis mirror case. left-right inversion
				/// Set Source Address to Scratch Memory
				m_oG2D->SetSrcSurface(&descScratch);
				/// Set Destination Surface to real Framebuffer Surface
				m_oG2D->SetDstSurface(pdescDstSurface);\
	
				/// Swap top, left coordinate when 90 and 270
				RETAILMSG(TEMP_DEBUG,(TEXT("S TBSWAP:%d,%d,%d,%d,%d,%d\n"),prclDst->left, prclDst->top, prclDst->right, prclDst->bottom, iRotate, m_oG2D->GetRotType(iRotate)));
				SWAP(prclDst->top, prclDst->bottom, LONG);
				/// Set Destination Clipping window Rect
				if(pBltParms->prclClip)
				{
					m_oG2D->SetClipWindow(pBltParms->prclClip);
				}
				else
				{
					m_oG2D->SetClipWindow(prclDst);
				}	
				
				EnterCriticalSection(&m_cs2D);	
				RETAILMSG(TEMP_DEBUG,(TEXT("S TASWAP:%d,%d,%d,%d,%d,%d\n"),prclDst->left, prclDst->top, prclDst->right, prclDst->bottom, iRotate, m_oG2D->GetRotType(iRotate)));				
				m_oG2D->StretchBlt( &t_rect, prclDst, m_oG2D->GetRotType(iRotate));
				LeaveCriticalSection(&m_cs2D);		
				/// recover left, right coordinate
				SWAP(prclDst->top, prclDst->bottom, LONG);				

				/// Disallocate Scratch Surface
				delete ScratchSurf;

				RETAILMSG(TEMP_DEBUG, (TEXT("Stretch Y-axis flip: R:%d\r\n"), pBltParms->pDst->Rotate()));
				
				return TRUE;
			}
			if(prclDst->bottom < prclDst->top)
			{
				RotateRectl(prclDst);			
				/// Allocation Scratch Framebuffer for Flip Operation.
				DDGPESurf *ScratchSurf;
				AllocSurface(&ScratchSurf, dwSrcWidth, dwSrcHeight, pBltParms->pDst->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pDst->Format()], GPE_REQUIRE_VIDEO_MEMORY);
				if(ScratchSurf == NULL)
				{
					RETAILMSG(TRUE,(TEXT("Scratch Surface Allocation is failed. %d\n"), __LINE__));
#if 0//USE_PACSURF, To increase video memory is better than to use system memory as Video Surface
					RETAILMSG(TRUE,(TEXT("try to allocate surface usign PA Surface\r\n")));
					PACSurf *ScratchSurf;
#endif
					RETAILMSG(TRUE,(TEXT("Maybe There's no sufficient video memory. please increase video memory\r\n")));
					RETAILMSG(TRUE,(TEXT("try to redirect to SW Emulated Bitblt\r\n")));		
					return FALSE;
				}
				

				/// Set Scratch Surface Information
				descScratch.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + ScratchSurf->OffsetInVideoMemory());
				RETAILMSG(TEMP_DEBUG,(TEXT("ScratchBaseAddr : 0x%x, Offset:%d,SrcW:%d, SrcH:%d, Stride:%d, R:%d\n"), descScratch.dwBaseaddr, ScratchSurf->OffsetInVideoMemory(), dwSrcWidth, dwSrcHeight, ScratchSurf->Stride(), ScratchSurf->Rotate()));
				descScratch.dwColorMode = pBltParms->pDst->Format();
				descScratch.dwHoriRes = dwSrcWidth;
				descScratch.dwVertRes = dwSrcHeight;

				/// Set Destination Surface to Scratch Surface
				m_oG2D->SetDstSurface(&descScratch);
				/// Set Destination Clipping window Rect
				m_oG2D->SetClipWindow(&t_rect);
				/// Set X-axis flip flag
				EnterCriticalSection(&m_cs2D);
				m_oG2D->FlipBlt( prclSrc, &t_rect,  FLIP_X );
				LeaveCriticalSection(&m_cs2D);

				/// Set Source Address to Scratch Memory
				m_oG2D->SetSrcSurface(&descScratch);
				/// Set Destination Surface to real Framebuffer Surface
				m_oG2D->SetDstSurface(pdescDstSurface);

				/// X-axis mirror case. up-down inversion
				/// Swap left, right coordinate when 90 and 270 degree
				RETAILMSG(TEMP_DEBUG,(TEXT("S LRSWAP:%d,%d,%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate, m_oG2D->GetRotType(iRotate)));					
				SWAP(prclDst->left, prclDst->right, LONG);			
				/// Set Destination Clipping window Rect
				if(pBltParms->prclClip)
				{
					m_oG2D->SetClipWindow(pBltParms->prclClip);
				}
				else
				{
					m_oG2D->SetClipWindow(prclDst);
				}	
				
				EnterCriticalSection(&m_cs2D);
				m_oG2D->StretchBlt( &t_rect, prclDst, m_oG2D->GetRotType(iRotate));			
				LeaveCriticalSection(&m_cs2D);
				/// recover top, bottom coordinate
				SWAP(prclDst->left, prclDst->right, LONG);

				/// Disallocate Scratch Surface
				delete ScratchSurf;

				RETAILMSG(TEMP_DEBUG, (TEXT("Stretch X-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));			
				
				return TRUE;
			}						
			RotateRectl(prclDst);
		}
		else		//< DMDO_0 and DMDO_180 does not need to modify prclDst region
		{
			RotateRectlBack(prclDst);
		 	if(prclDst->right < prclDst->left)
			{
				RotateRectl(prclDst);
				/// Allocation Scratch Framebuffer for Flip Operation.
				DDGPESurf *ScratchSurf;
				
				AllocSurface(&ScratchSurf, dwSrcWidth, dwSrcHeight, pBltParms->pDst->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pDst->Format()], GPE_REQUIRE_VIDEO_MEMORY);
				if(ScratchSurf == NULL)
				{
					RETAILMSG(TRUE,(TEXT("Scratch Surface Allocation is failed. %d\n"), __LINE__));
#if 0//USE_PACSURF, To increase video memory is better than to use system memory as Video Surface
					RETAILMSG(TRUE,(TEXT("try to allocate surface usign PA Surface\r\n")));
					PACSurf *ScratchSurf;
#endif
					RETAILMSG(TRUE,(TEXT("Maybe There's no sufficient video memory. please increase video memory\r\n")));
					RETAILMSG(TRUE,(TEXT("try to redirect to SW Emulated Bitblt\r\n")));		
					return FALSE;
				}
				

				/// Set Scratch Surface Information
				descScratch.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + ScratchSurf->OffsetInVideoMemory());
				descScratch.dwColorMode = pBltParms->pDst->Format();
				descScratch.dwHoriRes = dwSrcWidth;
				descScratch.dwVertRes = dwSrcHeight;
			
				/// Set Destination Surface to Scratch Surface
				m_oG2D->SetDstSurface(&descScratch);
				/// Set Destination Clipping window Rect
				m_oG2D->SetClipWindow(&t_rect);
			
				/// Set Y-axis flip flag
				EnterCriticalSection(&m_cs2D);
				m_oG2D->FlipBlt( prclSrc, &t_rect,  FLIP_Y );
				LeaveCriticalSection(&m_cs2D);
				/// Y-axis mirror case. left-right inversion
				/// Set Source Address to Scratch Memory
				m_oG2D->SetSrcSurface(&descScratch);
				/// Set Destination Surface to real Framebuffer Surface
				m_oG2D->SetDstSurface(pdescDstSurface);

				/// Swap left, right coordinate
				RETAILMSG(TEMP_DEBUG,(TEXT("%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom));			
				SWAP(prclDst->right, prclDst->left, LONG);
				/// Set Destination Clipping window Rect
				if(pBltParms->prclClip)
				{
					m_oG2D->SetClipWindow(pBltParms->prclClip);
				}
				else
				{
					m_oG2D->SetClipWindow(prclDst);
				}	
				
				RETAILMSG(TEMP_DEBUG,(TEXT("%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom));			
				EnterCriticalSection(&m_cs2D);	
				m_oG2D->StretchBlt( &t_rect, prclDst, m_oG2D->GetRotType(iRotate));
				LeaveCriticalSection(&m_cs2D);		
				/// recover left, right coordinate
				SWAP(prclDst->right, prclDst->left, LONG);				

				/// Disallocate Scratch Surface
				delete ScratchSurf;

				RETAILMSG(TEMP_DEBUG, (TEXT("Stretch Y-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));
				
				return TRUE;
			}
			if(prclDst->bottom < prclDst->top)
			{
				RotateRectl(prclDst);
				/// Allocation Scratch Framebuffer for Flip Operation.
				DDGPESurf *ScratchSurf;
				AllocSurface(&ScratchSurf, dwSrcWidth, dwSrcHeight, pBltParms->pDst->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pDst->Format()], GPE_REQUIRE_VIDEO_MEMORY);
				if(ScratchSurf == NULL)
				{
					RETAILMSG(TRUE,(TEXT("Scratch Surface Allocation is failed. %d\n"), __LINE__));
#if 0//USE_PACSURF, To increase video memory is better than to use system memory as Video Surface
					RETAILMSG(TRUE,(TEXT("try to allocate surface usign PA Surface\r\n")));
					PACSurf *ScratchSurf;
#endif
					RETAILMSG(TRUE,(TEXT("Maybe There's no sufficient video memory. please increase video memory\r\n")));
					RETAILMSG(TRUE,(TEXT("try to redirect to SW Emulated Bitblt\r\n")));		
					return FALSE;
				}
				

				/// Set Scratch Surface Information
				descScratch.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + ScratchSurf->OffsetInVideoMemory());
				descScratch.dwColorMode = pBltParms->pDst->Format();
				descScratch.dwHoriRes = dwSrcWidth;
				descScratch.dwVertRes = dwSrcHeight;

				/// Set Destination Surface to Scratch Surface
				m_oG2D->SetDstSurface(&descScratch);
				/// Set Destination Clipping window Rect
				m_oG2D->SetClipWindow(&t_rect);
			
				/// Set X-axis flip flag
				EnterCriticalSection(&m_cs2D);
				m_oG2D->FlipBlt( prclSrc, &t_rect,  FLIP_X );
				LeaveCriticalSection(&m_cs2D);
				
				/// Set Source Address to Scratch Memory
				m_oG2D->SetSrcSurface(&descScratch);
				/// Set Destination Surface to real Framebuffer Surface
				m_oG2D->SetDstSurface(pdescDstSurface);

				/// X-axis mirror case. up-down inversion
				/// Swap top, bottom coordinate
				SWAP(prclDst->top, prclDst->bottom, LONG);			
				/// Set Destination Clipping window Rect
				if(pBltParms->prclClip)
				{
					m_oG2D->SetClipWindow(pBltParms->prclClip);
				}
				else
				{
					m_oG2D->SetClipWindow(prclDst);
				}	
				
				EnterCriticalSection(&m_cs2D);
				m_oG2D->StretchBlt( &t_rect, prclDst, m_oG2D->GetRotType(iRotate));			
				LeaveCriticalSection(&m_cs2D);
				/// recover top, bottom coordinate
				SWAP(prclDst->top, prclDst->bottom, LONG);

				/// Disallocate Scratch Surface
				delete ScratchSurf;

				RETAILMSG(TEMP_DEBUG, (TEXT("Stretch X-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));			
				
				return TRUE;
			}				
			RotateRectlBack(prclDst);
		}
		/// This case does not need to flip.
		
		/// Set Destination Surface to real Framebuffer Surface
		m_oG2D->SetDstSurface(pdescDstSurface);
		/// Set Destination Clipping window Rect
		if(pBltParms->prclClip)
		{
			m_oG2D->SetClipWindow(pBltParms->prclClip);
		}
		else
		{
			m_oG2D->SetClipWindow(prclDst);
		}	
		
		EnterCriticalSection(&m_cs2D);	
		m_oG2D->StretchBlt( prclSrc, prclDst, m_oG2D->GetRotType(iRotate));
		LeaveCriticalSection(&m_cs2D);		

		RETAILMSG(TEMP_DEBUG, (TEXT("\nStretch no flip: R:%d\r\n"),pBltParms->pDst->Rotate()));		
	}
	else		// Do not stretch.
	{
		RotateRectl(prclSrc);
		RotateRectl(prclDst);	
	
		/// Set Source Surface Descriptor
		m_oG2D->SetSrcSurface(pdescSrcSurface);	

		/// Check whether XY flip or not, 
		///if XY flip is requested, just Rotation 180 degree
		RotateRectlBack(prclDst);
		if( ((pBltParms->bltFlags & BLT_STRETCH) == BLT_STRETCH)
			&& (prclDst->right < prclDst->left)  && (prclDst->bottom < prclDst->top) )
		{
			RotateRectl(prclDst);		
			switch(iRotate)
			{
				case DMDO_0:
					iRotate = DMDO_180;
					break;
				case DMDO_90:
					iRotate = DMDO_270;
					break;
				case DMDO_180:
					iRotate = DMDO_0;
					break;
				case DMDO_270:
					iRotate = DMDO_90;
					break;
			}

			/// Set Destination Surface to real Framebuffer Surface
			m_oG2D->SetDstSurface(pdescDstSurface);
			/// Set Destination Clipping window Rect
			if(pBltParms->prclClip)
			{
				m_oG2D->SetClipWindow(pBltParms->prclClip);
			}
			else
			{
				m_oG2D->SetClipWindow(prclDst);
			}	
			
			/// SWAP rect
			SWAP(prclDst->top, prclDst->bottom, LONG);
			SWAP(prclDst->left, prclDst->right, LONG);			
		
			EnterCriticalSection(&m_cs2D);	
			m_oG2D->BitBlt( prclSrc, prclDst, m_oG2D->GetRotType(iRotate));
			LeaveCriticalSection(&m_cs2D);				
			/// Recover rect
			SWAP(prclDst->top, prclDst->bottom, LONG);
			SWAP(prclDst->left, prclDst->right, LONG);			

			RETAILMSG(TEMP_DEBUG,(TEXT("XY Flip R:%d\n"), iRotate));
			
			return TRUE;
			
		}
		//RotateRectl(prclDst);			
		//RotateRectlBack(prclDst);

		/// Mirroring is needed.
		if( ((pBltParms->bltFlags & BLT_STRETCH) == BLT_STRETCH) 
			&&  ((prclDst->left > prclDst->right) || (prclDst->top > prclDst->bottom) )
			)
		{
			RotateRectl(prclDst);	
			
			RECTL t_rect;
			DWORD	dwSrcWidth;
			DWORD	dwSrcHeight;
			SURFACE_DESCRIPTOR descScratch;
		
			dwSrcWidth = ABS(prclSrc->right - prclSrc->left);
			dwSrcHeight  = ABS(prclSrc->bottom - prclSrc->top);

			/// Set Scratch Destination Region		
			t_rect.left = 0;
			t_rect.top = 0;
			t_rect.right = dwSrcWidth;
			t_rect.bottom = dwSrcHeight;
#if TEMP_DEBUG
	RETAILMSG(TEMP_DEBUG,(TEXT("t_rect,justbitbltflip(%d,%d)~(%d,%d), R:%d\r\n"), 
		t_rect.left,		t_rect.top,		t_rect.right,		t_rect.bottom,iRotate		));
#endif
#if TEMP_DEBUG
	if(pBltParms->pSrc)
	{
	RETAILMSG(TEMP_DEBUG,(TEXT("Src:0x%x SrcB 0x%x, Surf(W:%d,H:%d,BPP:%d,STRIDE:%d), Screen(W:%d,H:%d), rect: (%d,%d)~(%d,%d), R:%d\r\n"), 
		pBltParms->pSrc,
		pBltParms->pSrc->Buffer(),
		pBltParms->pSrc->Width(),
		pBltParms->pSrc->Height(),
		EGPEFormatToBpp[pBltParms->pSrc->Format()],
		pBltParms->pSrc->Stride(),
		pBltParms->pSrc->ScreenWidth(),
		pBltParms->pSrc->ScreenHeight(),
		pBltParms->prclSrc->left,
		pBltParms->prclSrc->top,
		pBltParms->prclSrc->right,
		pBltParms->prclSrc->bottom,
		pBltParms->pSrc->Rotate()
		));
	}
	if(pBltParms->pDst)
	{
	RETAILMSG(TEMP_DEBUG,(TEXT("Dst:0x%x DstB 0x%x,  Surf(W:%d,H:%d,BPP:%d,STRIDE:%d), Screen(W:%d,H:%d), rect: (%d,%d)~(%d,%d), R:%d\r\n"), 
		pBltParms->pDst,
		pBltParms->pDst->Buffer(),
		pBltParms->pDst->Width(),
		pBltParms->pDst->Height(),
		EGPEFormatToBpp[pBltParms->pDst->Format()],
		pBltParms->pDst->Stride(),		
		pBltParms->pDst->ScreenWidth(),
		pBltParms->pDst->ScreenHeight(),
		pBltParms->prclDst->left,
		pBltParms->prclDst->top,
		pBltParms->prclDst->right,
		pBltParms->prclDst->bottom,
		pBltParms->pDst->Rotate()
		));									
	}
	RETAILMSG(TEMP_DEBUG, (TEXT("ROP : 0x%0x\r\n"), pBltParms->rop4));				
	RETAILMSG(TEMP_DEBUG, (TEXT("xPositive : %d\r\n"),pBltParms->xPositive));
	RETAILMSG(TEMP_DEBUG, (TEXT("yPositive : %d\r\n"),pBltParms->yPositive));	
#endif			
			
			/// In mirror case, source region does not change.
			/// only destination's regions has reverse coordinate, this cannot be negative.
			if(iRotate == DMDO_0)	//< This is Difference between source and destination.
			{
//				RotateRectlBack(prclDst);
				if(prclDst->right < prclDst->left)
				{
//					RotateRectl(prclDst);
					RETAILMSG(TEMP_DEBUG, (TEXT("BitBlt Y-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));											
					/// Set Destination Surface to real Framebuffer Surface		
					m_oG2D->SetDstSurface(pdescDstSurface);

					RETAILMSG(TEMP_DEBUG,(TEXT("BY TBSWAP:%d,%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate));
					/// Y-axis mirror case. left-right inversion
					/// Swap left, right coordinate
					SWAP(prclDst->right, prclDst->left, LONG);
					
					RETAILMSG(TEMP_DEBUG,(TEXT("BY TASWAP:%d,%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate));										
					/// Set Destination Clipping window Rect
					if(pBltParms->prclClip)
					{
						m_oG2D->SetClipWindow(pBltParms->prclClip);
					}
					else
					{
						m_oG2D->SetClipWindow(prclDst);
					}	
					
					/// Set Y-axis flip flag
					EnterCriticalSection(&m_cs2D);
					m_oG2D->FlipBlt( prclSrc, prclDst,  FLIP_Y );
					LeaveCriticalSection(&m_cs2D);				
					/// recover left, right coordinate
					SWAP(prclDst->right, prclDst->left, LONG);				

					return TRUE;
				}
				else if(prclDst->bottom < prclDst->top)
				{
//					RotateRectl(prclDst);
					RETAILMSG(TEMP_DEBUG, (TEXT("BitBlt X-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));									
					/// Set Destination Surface to real Framebuffer Surface		
					m_oG2D->SetDstSurface(pdescDstSurface);

					RETAILMSG(TEMP_DEBUG,(TEXT("BX TBSWAP:%d,%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate));				
					/// X-axis mirror case. up-down inversion
					/// Swap top, bottom coordinate
					SWAP(prclDst->top, prclDst->bottom, LONG);
					RETAILMSG(TEMP_DEBUG,(TEXT("BX TASWAP:%d,%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate));									
					/// Set Destination Clipping window Rect
					if(pBltParms->prclClip)
					{
						m_oG2D->SetClipWindow(pBltParms->prclClip);
					}
					else
					{
						m_oG2D->SetClipWindow(prclDst);
					}	
					
					/// Set X-axis flip flag
					EnterCriticalSection(&m_cs2D);
					m_oG2D->FlipBlt( prclSrc, prclDst,  FLIP_X );
					LeaveCriticalSection(&m_cs2D);				
					/// recover top, bottom coordinate
					SWAP(prclDst->top, prclDst->bottom, LONG);

					return TRUE;
				}
//				RotateRectl(prclDst);
			}
			else if(iRotate == DMDO_90 || iRotate == DMDO_270)
			{	
				RotateRectlBack(prclDst);
				/// Original Coordinate
				RETAILMSG(TEMP_DEBUG, (TEXT("R:%d, DR:%d, DST(%d,%d,%d,%d)\r\n"),pBltParms->pDst->Rotate(), iRotate, 
					pBltParms->prclDst->left,
					pBltParms->prclDst->top,
					pBltParms->prclDst->right,
					pBltParms->prclDst->bottom));
				/// if screen rotation is not DMDO_0. we need to bitblt once more. and use scratch memory
				 if(prclDst->right < prclDst->left)	
				{
					RotateRectl(prclDst);					
					/// Screen rotated 
					/// if 90, 270 degree, that is T,B Swaped and rotate 
					///  +-----+ (L,T)
					///  |     |
					///  |     |       
					///  |     |
					///  +-----+
					/// (R,B)

					/// Allocation Scratch Framebuffer for Flip Operation.
					DDGPESurf *ScratchSurf;
			
					AllocSurface(&ScratchSurf, dwSrcWidth, dwSrcHeight, pBltParms->pDst->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pDst->Format()], GPE_REQUIRE_VIDEO_MEMORY);
					if(ScratchSurf == NULL)
					{
						RETAILMSG(TRUE,(TEXT("Scratch Surface Allocation is failed. %d\n"), __LINE__));
#if 0//USE_PACSURF, To increase video memory is better than to use system memory as Video Surface
						RETAILMSG(TRUE,(TEXT("try to allocate surface usign PA Surface\r\n")));
						PACSurf *ScratchSurf;
#endif
						RETAILMSG(TRUE,(TEXT("Maybe There's no sufficient video memory. please increase video memory\r\n")));
						RETAILMSG(TRUE,(TEXT("try to redirect to SW Emulated Bitblt\r\n")));		
						return FALSE;
					}
					

					/// Set Scratch Surface Information
					descScratch.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + ScratchSurf->OffsetInVideoMemory());
					descScratch.dwColorMode = pBltParms->pDst->Format();
					descScratch.dwHoriRes = dwSrcWidth;
					descScratch.dwVertRes = dwSrcHeight;
		
					/// Set Destination Surface as Scratch Surface
					m_oG2D->SetDstSurface(&descScratch);
					/// Set Destination Clipping window Rect
					m_oG2D->SetClipWindow(&t_rect);

					/// Y-axis mirror case. left-right inversion

					EnterCriticalSection(&m_cs2D);
					m_oG2D->FlipBlt( prclSrc, &t_rect,  FLIP_Y );
					LeaveCriticalSection(&m_cs2D);				
			
					/// Set Source Address to Scratch Memory
					m_oG2D->SetSrcSurface(&descScratch);
					/// Set Destination Surface to real Framebuffer Surface
					m_oG2D->SetDstSurface(pdescDstSurface);

					/// Swap left, right coordinate
					/// Y-axis mirror case. left-right inversion
					RETAILMSG(TEMP_DEBUG,(TEXT("B TBSWAP:%d,%d,%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate, m_oG2D->GetRotType(iRotate)));	
					SWAP(prclDst->top, prclDst->bottom, LONG);
					/// Set Destination Clipping window Rect
					if(pBltParms->prclClip)
					{
						m_oG2D->SetClipWindow(pBltParms->prclClip);
					}
					else
					{
						m_oG2D->SetClipWindow(prclDst);
					}	
					
					/// Set Y-axis flip flag					
					EnterCriticalSection(&m_cs2D);	
					m_oG2D->BitBlt( &t_rect, prclDst,  m_oG2D->GetRotType(iRotate) );
					LeaveCriticalSection(&m_cs2D);		
					/// recover left, right coordinate
					SWAP(prclDst->top, prclDst->bottom, LONG);				

					/// Disallocate Scratch Surface
					delete ScratchSurf;

					RETAILMSG(TEMP_DEBUG, (TEXT("BitBlt Y-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));

				//	RotateRectl(prclDst);					
					
					return TRUE;

				}
				else if(prclDst->bottom < prclDst->top)
				{
					RotateRectl(prclDst);					
					/// Allocation Scratch Framebuffer for Flip Operation.
					DDGPESurf *ScratchSurf;
			
					AllocSurface(&ScratchSurf, dwSrcWidth, dwSrcHeight, pBltParms->pDst->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pDst->Format()], GPE_REQUIRE_VIDEO_MEMORY);
					if(ScratchSurf == NULL)
					{
						RETAILMSG(TRUE,(TEXT("Scratch Surface Allocation is failed. %d\n"), __LINE__));
#if 0//USE_PACSURF, To increase video memory is better than to use system memory as Video Surface
						RETAILMSG(TRUE,(TEXT("try to allocate surface usign PA Surface\r\n")));
						PACSurf *ScratchSurf;
#endif
						RETAILMSG(TRUE,(TEXT("Maybe There's no sufficient video memory. please increase video memory\r\n")));
						RETAILMSG(TRUE,(TEXT("try to redirect to SW Emulated Bitblt\r\n")));		
						return FALSE;
					}
					

					/// Set Scratch Surface Information
					descScratch.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + ScratchSurf->OffsetInVideoMemory());
					descScratch.dwColorMode = pBltParms->pDst->Format();
					descScratch.dwHoriRes = dwSrcWidth;
					descScratch.dwVertRes = dwSrcHeight;
		
					/// Set Destination Surface to Scratch Surface
					m_oG2D->SetDstSurface(&descScratch);
					/// Set Destination Clipping window Rect
					m_oG2D->SetClipWindow(&t_rect);
				
					/// X-axis mirror case. top-bottom inversion
					RETAILMSG(TEMP_DEBUG,(TEXT("B LRSWAP:%d,%d,%d,%d,%d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate, m_oG2D->GetRotType(iRotate)));			
					EnterCriticalSection(&m_cs2D);
					m_oG2D->FlipBlt( prclSrc, &t_rect,  FLIP_X );
					LeaveCriticalSection(&m_cs2D);				
			
					/// Set Source Address to Scratch Memory
					m_oG2D->SetSrcSurface(&descScratch);
					/// Set Destination Surface to real Framebuffer Surface
					m_oG2D->SetDstSurface(pdescDstSurface);

					/// Swap left, right coordinate	
					/// LT <-> RB
					SWAP(prclDst->left, prclDst->right, LONG);
					/// Set Destination Clipping window Rect
					if(pBltParms->prclClip)
					{
						m_oG2D->SetClipWindow(pBltParms->prclClip);
					}
					else
					{
						m_oG2D->SetClipWindow(prclDst);
					}	
					
					/// Set Y-axis flip flag					
					EnterCriticalSection(&m_cs2D);	
					m_oG2D->BitBlt( &t_rect, prclDst,  m_oG2D->GetRotType(iRotate) );
					LeaveCriticalSection(&m_cs2D);		
					/// recover left, right coordinate
					SWAP(prclDst->left, prclDst->right, LONG);				

					/// Disallocate Scratch Surface
					delete ScratchSurf;

					RETAILMSG(TEMP_DEBUG, (TEXT("BitBlt X-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));					

					return TRUE;
				}				
				RotateRectl(prclDst);				
			}
			else		//< DMDO_180
			{
				RotateRectlBack(prclDst);
				/// if screen rotation is not DMDO_0. we need to bitblt once more. and use scratch memory
				 if(prclDst->right < prclDst->left)	
				{
					RotateRectl(prclDst);
					RETAILMSG(TEMP_DEBUG, (TEXT("BitBlt Y-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));
				
					/// Allocation Scratch Framebuffer for Flip Operation.
					DDGPESurf *ScratchSurf;
			
					AllocSurface(&ScratchSurf, dwSrcWidth, dwSrcHeight, pBltParms->pDst->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pDst->Format()], GPE_REQUIRE_VIDEO_MEMORY);
					if(ScratchSurf == NULL)
					{
						RETAILMSG(TRUE,(TEXT("Scratch Surface Allocation is failed. %d\n"), __LINE__));
#if 0//USE_PACSURF, To increase video memory is better than to use system memory as Video Surface
						RETAILMSG(TRUE,(TEXT("try to allocate surface usign PA Surface\r\n")));
						PACSurf *ScratchSurf;
#endif
						RETAILMSG(TRUE,(TEXT("Maybe There's no sufficient video memory. please increase video memory\r\n")));
						RETAILMSG(TRUE,(TEXT("try to redirect to SW Emulated Bitblt\r\n")));		
						return FALSE;
					}

					/// Set Scratch Surface Information
					descScratch.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + ScratchSurf->OffsetInVideoMemory());
					descScratch.dwColorMode = pBltParms->pDst->Format();
					descScratch.dwHoriRes = dwSrcWidth;
					descScratch.dwVertRes = dwSrcHeight;
		
					/// Set Destination Surface to Scratch Surface
					m_oG2D->SetDstSurface(&descScratch);
					/// Set Destination Clipping window Rect
					m_oG2D->SetClipWindow(&t_rect);

					/// Y-axis mirror case. left-right inversion
					RETAILMSG(TEMP_DEBUG,(TEXT("(%d,%d)~(%d,%d) %d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate, m_oG2D->GetRotType(iRotate)));			
					EnterCriticalSection(&m_cs2D);
					m_oG2D->FlipBlt( prclSrc, &t_rect,  FLIP_Y );
					LeaveCriticalSection(&m_cs2D);				
			
					/// Set Source Address to Scratch Memory
					m_oG2D->SetSrcSurface(&descScratch);
					/// Set Destination Surface to real Framebuffer Surface
					m_oG2D->SetDstSurface(pdescDstSurface);

					/// Swap left, right coordinate
					SWAP(prclDst->right, prclDst->left, LONG);
					/// Set Destination Clipping window Rect
					if(pBltParms->prclClip)
					{
						m_oG2D->SetClipWindow(pBltParms->prclClip);
					}
					else
					{
						m_oG2D->SetClipWindow(prclDst);
					}	
					
					/// Set Y-axis flip flag					
					EnterCriticalSection(&m_cs2D);	
					m_oG2D->BitBlt( &t_rect, prclDst,  m_oG2D->GetRotType(iRotate) );
					LeaveCriticalSection(&m_cs2D);		
					/// recover left, right coordinate
					SWAP(prclDst->right, prclDst->left, LONG);				

					/// Disallocate Scratch Surface
					delete ScratchSurf;
				
					return TRUE;

				}
				else	 if(prclDst->bottom < prclDst->top)
				{
					RotateRectl(prclDst);
					RETAILMSG(TEMP_DEBUG, (TEXT("BitBlt X-axis flip: R:%d\r\n"),pBltParms->pDst->Rotate()));									
					/// Allocation Scratch Framebuffer for Flip Operation.
					DDGPESurf *ScratchSurf;
			
					AllocSurface(&ScratchSurf, dwSrcWidth, dwSrcHeight, pBltParms->pDst->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pDst->Format()], GPE_REQUIRE_VIDEO_MEMORY);
					if(ScratchSurf == NULL)
					{
						RETAILMSG(TRUE,(TEXT("Scratch Surface Allocation is failed. %d\n"), __LINE__));
#if 0//USE_PACSURF, To increase video memory is better than to use system memory as Video Surface
						RETAILMSG(TRUE,(TEXT("try to allocate surface usign PA Surface\r\n")));
						PACSurf *ScratchSurf;
#endif
						RETAILMSG(TRUE,(TEXT("Maybe There's no sufficient video memory. please increase video memory\r\n")));
						RETAILMSG(TRUE,(TEXT("try to redirect to SW Emulated Bitblt\r\n")));		
						return FALSE;
					}
					

					/// Set Scratch Surface Information
					descScratch.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + ScratchSurf->OffsetInVideoMemory());
					descScratch.dwColorMode = pBltParms->pDst->Format();
					descScratch.dwHoriRes = dwSrcWidth;
					descScratch.dwVertRes = dwSrcHeight;
		
					/// Set Destination Surface to Scratch Surface
					m_oG2D->SetDstSurface(&descScratch);
					/// Set Destination Clipping window Rect
					m_oG2D->SetClipWindow(&t_rect);
				
					/// X-axis mirror case. top-down inversion
					RETAILMSG(TEMP_DEBUG,(TEXT("(%d,%d)~(%d,%d) %d,%d\n"),pBltParms->prclDst->left,pBltParms->prclDst->top,pBltParms->prclDst->right,pBltParms->prclDst->bottom, iRotate, m_oG2D->GetRotType(iRotate)));			
					EnterCriticalSection(&m_cs2D);
					m_oG2D->FlipBlt( prclSrc, &t_rect,  FLIP_X );
					LeaveCriticalSection(&m_cs2D);				
			
					/// Set Source Address to Scratch Memory
					m_oG2D->SetSrcSurface(&descScratch);
					/// Set Destination Surface to real Framebuffer Surface
					m_oG2D->SetDstSurface(pdescDstSurface);

					/// Swap left, right coordinate
					SWAP(prclDst->top, prclDst->bottom, LONG);
					/// Set Destination Clipping window Rect
					if(pBltParms->prclClip)
					{
						m_oG2D->SetClipWindow(pBltParms->prclClip);
					}
					else
					{
						m_oG2D->SetClipWindow(prclDst);
					}	
					
					/// Set Y-axis flip flag					
					EnterCriticalSection(&m_cs2D);	
					m_oG2D->BitBlt( &t_rect, prclDst,  m_oG2D->GetRotType(iRotate) );
					LeaveCriticalSection(&m_cs2D);		
					/// recover left, right coordinate
					SWAP(prclDst->top, prclDst->bottom, LONG);				

					/// Disallocate Scratch Surface
					delete ScratchSurf;

					return TRUE;
				}				
				RotateRectl(prclDst);
			}			
		}
		RotateRectl(prclDst);
		/// Set Destination Surface to real Framebuffer Surface		
		m_oG2D->SetDstSurface(pdescDstSurface);
		/// Set Destination Clipping window Rect
		if(pBltParms->prclClip)
		{
			m_oG2D->SetClipWindow(pBltParms->prclClip);
		}
		else
		{
			m_oG2D->SetClipWindow(prclDst);
		}	
		
		
		EnterCriticalSection(&m_cs2D);
		m_oG2D->BitBlt( prclSrc, prclDst,  m_oG2D->GetRotType(iRotate) );
		LeaveCriticalSection(&m_cs2D);
		RETAILMSG(FULL_TEST_OK, (TEXT("BitBlt no flip: R:%d\r\n"),pBltParms->pDst->Rotate()));
	}
	RotateRectl(prclSrc);
	RotateRectl(prclDst);	

	return TRUE;
	
}
#endif

/**
*	@fn		void S3C2450Disp::AcceleratedBltSelect16(GpeBltParms *pBltParms)
*	@brief	Select appropriate hardware acceleration function or software emulation function. 
*			if there's no appropriate accelerated function,
*			Leave Blit funciton to intial setting, EmulatedBlt(generic Bit blit emulator)
*	@param	pBltParms	Blit Parameter Information Structure
*	@sa		GPEBltParms
*/
SCODE S3C2450DISP::AcceleratedBltSelect16(GPEBltParms *pBltParms)
{
	// In SMDK2450, Stretch function support only integer multipling calculation
	//  Do not support shrink, fractional stretching
	/// All these condition is same to EmulatedBltSelect16
	if ((pBltParms->pDst->Format() != gpe16Bpp) 
		|| 	(pBltParms->bltFlags & BLT_ALPHABLEND) 		//< if AlphaBlend is required, emulate
//		||	(pBltParms->bltFlags & BLT_TRANSPARENT) 		//< Our HW support Transparent blit for 16bpp, 24bpp
#if G2D_BYPASS_HW_STRETCHBLT
		||	(pBltParms->bltFlags & BLT_STRETCH)			//< can support Stretch Blit for 16bpp, 24bpp
#endif
		 ) //< Emulate if AlphaBlend is required
	{
		return S_OK;
	}
	if(pBltParms->pLookup)
	{
		RETAILMSG(0, (TEXT("Lookup is required\r\n")));
		return S_OK;
	}
	if(pBltParms->pConvert)		//< Emulate if color conversion required
	{
		RETAILMSG(0, (TEXT("Color Converting is required\r\n")));	
		return S_OK;
	}

	if(pBltParms->prclClip && (pBltParms->prclClip->left == pBltParms->prclClip->right) && (pBltParms->prclClip->top == pBltParms->prclClip->bottom))
	{
		// Just skip, there is no image flushing to screen
		// SW bitblt takes this case, and it can skip more efficiently.
		return S_OK;
	}

	
	/**
	*	Check if source and destination regions' coordinates has positive value.
	*
	*
	**/
	if ((pBltParms->bltFlags & BLT_STRETCH))			// Stretch Bllitting with X or Y axis mirroring
	{
		if(!pBltParms->prclDst)
		{
//			goto emulsel;
			return S_OK;
		}
		else
		{
			if ((pBltParms->prclDst->left < 0) || (pBltParms->prclDst->right <0 ) || (pBltParms->prclDst->top <0 ) || (pBltParms->prclDst->bottom <0))
			{
				return S_OK;			
		//		goto emulsel;
			}
			if ((pBltParms->prclSrc->left < 0) || (pBltParms->prclSrc->right <0 ) || (pBltParms->prclSrc->top <0 ) || (pBltParms->prclSrc->bottom <0))
			{
				return S_OK;			
		//		goto emulsel;
			}
			/// Odd case do nothing
			if ((pBltParms->prclSrc->right == pBltParms->prclSrc->left) || (pBltParms->prclSrc->bottom == pBltParms->prclSrc->top) ||
			    (pBltParms->prclDst->right == pBltParms->prclDst->left) || (pBltParms->prclDst->bottom == pBltParms->prclDst->top))
			{
				return S_OK;
			}

			/*
			/// XY Mirrored image case
			/// In this case rotation is needed so 180 rotation.
			if ((pBltParms->prclDst->left > pBltParms->prclDst->right) && (pBltParms->prclDst->top > pBltParms->prclDst->bottom) )
			{
				/// Pick TopLeft pixel as rotation origin.
				/// But rotation setting is automatically done in HWBitBlt
				/// So Just Set Rotation to 180 degree
				goto emulsel;
			}
			*/
			/*
			if ((pBltParms->prclDst->left > pBltParms->prclDst->right) || (pBltParms->prclDst->top > pBltParms->prclDst->bottom) )
			{
				/// In mirror case, source region does not change.
				/// only destination's regions has reverse coordinate, this cannot be negative.
			 	if(pBltParms->prclDst->right < pBltParms->prclDst->left)
				{
					/// Y-axis mirror case. left-right inversion
					/// !Currently go to emul path
					goto emulsel;
					/// Set Y-axis flip flag
				}
				if(pBltParms->prclDst->bottom < pBltParms->prclDst->top)
				{
					/// X-axis mirror case. up-down inversion
					/// ! Currently goto emul path
					goto emulsel;
					/// Set X-axis flip flag
				}
			}
			*/
		}
	}
	// select accelerated function based on rop value
	switch (pBltParms->rop4)
	{

		case 0x0000:	// BLACKNESS
			pBltParms->solidColor = 0;
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &Emulator::EmulatedBltFill16;
#if 0
//			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &S3C2450DISP::AcceleratedBltFill;
#endif
			return S_OK;
		case 0xFFFF:	// WHITENESS	// Done have prclSrc, and pMask
			pBltParms->solidColor = 0x00ffffff;
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &Emulator::EmulatedBltFill16;
#if 0
//			pBltParms->solidColor = 0x0000ffff;
//			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &S3C2450DISP::AcceleratedBltFill;
#endif
			return S_OK;
	case 0x5555:    // DSTINVERT
//		  pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &Emulator::EmulatedBltDstInvert16;
			return S_OK;		  
//			goto emulsel;
	case 0xAAF0:	// Special PATCOPY rop for text output -- fill where mask is set.	// Not a pattern brush?
//		goto emulsel;
		return S_OK;
	case 0x5A5A:	// PATINVERT
		return S_OK;
	case 0xF0F0:	// PATCOPY
		if( pBltParms->solidColor != -1)		// must be a solid colored brush
		{	
			if(	pBltParms->prclDst &&
				( (pBltParms->prclDst->left >= 0) && (pBltParms->prclDst->left < 2048)  &&
				  (pBltParms->prclDst->top >= 0) && (pBltParms->prclDst->top < 2048) &&
				  (pBltParms->prclDst->right >= 0 ) && (pBltParms->prclDst->top < 2048) &&
				  (pBltParms->prclDst->bottom >= 0 ) && (pBltParms->prclDst->top < 2048)  ) &&
				 (pBltParms->pDst->Stride() / (EGPEFormatToBpp[pBltParms->pDst->Format()]/8) < 2048) 
//				(ABS(pBltParms->prclDst->right - pBltParms->prclDst->left)*ABS(pBltParms->prclDst->bottom - pBltParms->prcDst->top)   > G2D_COMPROMISE_LIMIT)
			 )
			{
#ifdef DO_DISPPERF
		DispPerfType(DISPPERF_ACCEL_HARDWARE);		
#endif
					pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &S3C2450DISP::AcceleratedBltFill;
					return S_OK;
			}
		}
//		goto emulsel;		
		return S_OK;
	case 0x6666:	// SRCINVERT
		if( pBltParms->prclDst &&
			((pBltParms->prclDst->left >= 0) &&
			(pBltParms->prclDst->top >= 0) &&
			(pBltParms->prclDst->right >= 0 ) &&
			(pBltParms->prclDst->bottom >= 0 )) &&
			pBltParms->pSrc &&
			 (pBltParms->pDst->Stride() / (EGPEFormatToBpp[pBltParms->pDst->Format()]/8) < 2048) &&
			 (pBltParms->pSrc->Stride() / (EGPEFormatToBpp[pBltParms->pSrc->Format()]/8) < 2048) 
#if G2D_BLT_OPTIMIZE	
			 && (ABS(pBltParms->prclSrc->right - pBltParms->prclSrc->left)*ABS(pBltParms->prclSrc->bottom - pBltParms->prclSrc->top)   > G2D_COMPROMISE_LIMIT)
#endif
		)
		{
#ifdef DO_DISPPERF
		DispPerfType(DISPPERF_ACCEL_HARDWARE);		
#endif
			if( ( pBltParms->pSrc->InVideoMemory()
#if G2D_TRY_CBLT			
					|| SUCCEEDED(SourceRegionCacheClean(pBltParms)) 
#endif
				)
					&& (ABS(pBltParms->prclSrc->bottom - pBltParms->prclSrc->top) < 2048)
					&& (ABS(pBltParms->prclSrc->right - pBltParms->prclSrc->left) < 2048)
				) {
				pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &S3C2450DISP::AcceleratedSrcCopyBlt;						
				return S_OK;
			}
			else
			{
				if( (pBltParms->pSrc)->InVideoMemory() ){
					RETAILMSG(FALSE,(TEXT("INVideo:SRCINVERT\r\n")));
				}
				else
				{
					RETAILMSG(FALSE,(TEXT("NotInVideo:SRCINVERT\r\n")));				
				}
			}
		}
		return S_OK;
//		goto emulsel;
	case 0xCCCC:	// SRCCOPY
		if( pBltParms->prclDst &&
				((pBltParms->prclDst->left >= 0) &&
					(pBltParms->prclDst->top >= 0) &&
					(pBltParms->prclDst->right >= 0 ) &&
					(pBltParms->prclDst->bottom >= 0 )) &&
				pBltParms->pSrc &&
			 (pBltParms->pDst->Stride() / (EGPEFormatToBpp[pBltParms->pDst->Format()]/8) < 2048) &&
			 (pBltParms->pSrc->Stride() / (EGPEFormatToBpp[pBltParms->pSrc->Format()]/8) < 2048) 
#if G2D_BLT_OPTIMIZE	
			&&	(ABS(pBltParms->prclSrc->right - pBltParms->prclSrc->left)*ABS(pBltParms->prclSrc->bottom - pBltParms->prclSrc->top)   > G2D_COMPROMISE_LIMIT)			 			 
#endif
		)
		{
#ifdef DO_DISPPERF
		DispPerfType(DISPPERF_ACCEL_HARDWARE);		
#endif
			if( ( (pBltParms->pSrc)->InVideoMemory()
#if G2D_TRY_CBLT			
					|| SUCCEEDED(SourceRegionCacheClean(pBltParms))
#endif
					)
					&& (ABS(pBltParms->prclSrc->bottom - pBltParms->prclSrc->top) < 2048)
					&& (ABS(pBltParms->prclSrc->right - pBltParms->prclSrc->left) < 2048)					
				) {
				pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &S3C2450DISP::AcceleratedSrcCopyBlt;
				return S_OK;
			}
			else{
				if( (pBltParms->pSrc)->InVideoMemory() ){
					RETAILMSG(FALSE,(TEXT("INVideo:SRCCOPY\r\n")));				
				}
				else
				{
					RETAILMSG(FALSE,(TEXT("NotInVideo:SRCCOPY\r\n")));				
#define DUMP_BLTPARAM	(0)					
					#if DUMP_BLTPARAM
						if(pBltParms->pSrc)
	{
	RETAILMSG(DUMP_BLTPARAM,(TEXT("Src 0x%x, Surf(W:%d,H:%d,BPP:%d,STRIDE:%d), Screen(W:%d,H:%d), rect: (%d,%d)~(%d,%d), R:%d\r\n"), 
		pBltParms->pSrc->Buffer(),
		pBltParms->pSrc->Width(),
		pBltParms->pSrc->Height(),
		EGPEFormatToBpp[pBltParms->pSrc->Format()],
		pBltParms->pSrc->Stride(),
		pBltParms->pSrc->ScreenWidth(),
		pBltParms->pSrc->ScreenHeight(),
		pBltParms->prclSrc->left,
		pBltParms->prclSrc->top,
		pBltParms->prclSrc->right,
		pBltParms->prclSrc->bottom,
		pBltParms->pSrc->Rotate()
		));
	}
	if(pBltParms->pDst)
	{
	RETAILMSG(DUMP_BLTPARAM,(TEXT("Dst 0x%x,  Surf(W:%d,H:%d,BPP:%d,STRIDE:%d), Screen(W:%d,H:%d), rect: (%d,%d)~(%d,%d), R:%d\r\n"), 
		pBltParms->pDst->Buffer(),
		pBltParms->pDst->Width(),
		pBltParms->pDst->Height(),
		EGPEFormatToBpp[pBltParms->pDst->Format()],
		pBltParms->pDst->Stride(),		
		pBltParms->pDst->ScreenWidth(),
		pBltParms->pDst->ScreenHeight(),
		pBltParms->prclDst->left,
		pBltParms->prclDst->top,
		pBltParms->prclDst->right,
		pBltParms->prclDst->bottom,
		pBltParms->pDst->Rotate()
		));									
	}
	RETAILMSG(DUMP_BLTPARAM, (TEXT("ROP : 0x%0x\r\n"), pBltParms->rop4));				
	RETAILMSG(DUMP_BLTPARAM, (TEXT("xPositive : %d\r\n"),pBltParms->xPositive));
	RETAILMSG(DUMP_BLTPARAM, (TEXT("yPositive : %d\r\n"),pBltParms->yPositive));	
	#endif
					#if 0
					/// If Stretch is needed, HW is more efficient. 
					/// so in this case, copy va to pa's scratched buffer, then use HW
					if( (pBltParms->bltFlags & BLT_STRETCH) )
					{
						ADDRESS pSrcStart;
						BOOL	bBottomUp=FALSE;

						AllocSurface(&gpScratchSurf, ABS(pBltParms->pSrc->Stride() / (EGPEFormatToBpp[pBltParms->pSrc->Format()]/8)), pBltParms->pSrc->Height(), pBltParms->pSrc->Format(), EGPEFormatToEDDGPEPixelFormat[pBltParms->pSrc->Format()], GPE_REQUIRE_VIDEO_MEMORY);

						if(!gpScratchSurf)
						{
							RETAILMSG(TRUE,(TEXT("cannot create gpScratchSurf\r\n")));
							return S_OK;
						}
						else
						{
							if(!gpScratchSurf->Buffer())
							{
								RETAILMSG(TRUE,(TEXT("Video memory is not enough to create gpScratchSurf\r\n")));
								return S_OK;
							}
						}

						if( pBltParms->pSrc->Stride() < 0)	//< This is bottom-up image, need to flip. and recalculate Base Address
						{
							pSrcStart = (ADDRESS)pBltParms->pSrc->Buffer() + pBltParms->pSrc->Stride() * (pBltParms->pSrc->Height()-1);
							bBottomUp=TRUE;
						}
						else
						{
							pSrcStart = (ADDRESS)pBltParms->pSrc->Buffer();
						}
						RETAILMSG(TRUE,(TEXT("copy Original Source Surface to Scratch Surface\r\n")));
						memmove(gpScratchSurf->Buffer(), (LPVOID)pSrcStart, ABS(pBltParms->pSrc->Stride()*pBltParms->pSrc->Height()));
						RETAILMSG(TRUE,(TEXT("Swap Original Source Surface and Scratch Surface\r\n")));						
						oldSrcSurf = pBltParms->pSrc;
						pBltParms->pSrc = gpScratchSurf;
					}	
					#endif
				}
			}
		}
	//	goto emulsel;
		return S_OK;
	case 0x8888:	//	SRCAND
		if( pBltParms->prclDst &&
				((pBltParms->prclDst->left >= 0) &&
					(pBltParms->prclDst->top >= 0) &&
					(pBltParms->prclDst->right >= 0 ) &&
					(pBltParms->prclDst->bottom >= 0 )) &&
				pBltParms->pSrc &&
			 (pBltParms->pDst->Stride() / (EGPEFormatToBpp[pBltParms->pDst->Format()]/8) < 2048) &&
			 (pBltParms->pSrc->Stride() / (EGPEFormatToBpp[pBltParms->pSrc->Format()]/8) < 2048) 
#if G2D_BLT_OPTIMIZE	
			&& (ABS(pBltParms->prclSrc->right - pBltParms->prclSrc->left)*ABS(pBltParms->prclSrc->bottom - pBltParms->prclSrc->top)   > G2D_COMPROMISE_LIMIT)
#endif
		)	{
#ifdef DO_DISPPERF
		DispPerfType(DISPPERF_ACCEL_HARDWARE);		
#endif		
			if( ( (pBltParms->pSrc)->InVideoMemory()
#if G2D_TRY_CBLT			
					|| SUCCEEDED(SourceRegionCacheClean(pBltParms))
#endif
					)
					&& (ABS(pBltParms->prclSrc->bottom - pBltParms->prclSrc->top) < 2048)
					&& (ABS(pBltParms->prclSrc->right - pBltParms->prclSrc->left) < 2048)					
				) {
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &S3C2450DISP::AcceleratedSrcCopyBlt;
			return S_OK;
		}
			else{
				if( (pBltParms->pSrc)->InVideoMemory() ){
					RETAILMSG(FALSE,(TEXT("INVideo:SRCAND\r\n")));
				}
				else
				{
					RETAILMSG(FALSE,(TEXT("NotInVideo:SRCAND\r\n")));				
				}
			}
		}
		return S_OK;
//		goto emulsel;		
	case 0xEEEE:	// SRCPAINT
		if( pBltParms->prclDst &&
				((pBltParms->prclDst->left >= 0) &&
					(pBltParms->prclDst->top >= 0) &&
					(pBltParms->prclDst->right >= 0 ) &&
					(pBltParms->prclDst->bottom >= 0 )) &&
				pBltParms->pSrc &&
			 (pBltParms->pDst->Stride() / (EGPEFormatToBpp[pBltParms->pDst->Format()]/8) < 2048) &&
			 (pBltParms->pSrc->Stride() / (EGPEFormatToBpp[pBltParms->pSrc->Format()]/8) < 2048) 
#if G2D_BLT_OPTIMIZE	
			&&	(ABS(pBltParms->prclSrc->right - pBltParms->prclSrc->left)*ABS(pBltParms->prclSrc->bottom - pBltParms->prclSrc->top)   > G2D_COMPROMISE_LIMIT)									 
#endif
		)	{
#ifdef DO_DISPPERF
		DispPerfType(DISPPERF_ACCEL_HARDWARE);		
#endif		
			if( ( (pBltParms->pSrc)->InVideoMemory()
#if	G2D_TRY_CBLT
				|| SUCCEEDED(SourceRegionCacheClean(pBltParms))
#endif
					)
					&& (ABS(pBltParms->prclSrc->bottom - pBltParms->prclSrc->top) < 2048)
					&& (ABS(pBltParms->prclSrc->right - pBltParms->prclSrc->left) < 2048)				
				) {
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *)) &S3C2450DISP::AcceleratedSrcCopyBlt;
			return S_OK;
		}
			else{
				if( (pBltParms->pSrc)->InVideoMemory() ){
					RETAILMSG(FALSE,(TEXT("INVideo:SRCPAINT\r\n")));
				}
				else
				{
					RETAILMSG(FALSE,(TEXT("NotInVideo:SRCPAINT\r\n")));				
				}
			}
			
		}
//		goto emulsel;		
		return S_OK;
	default:		// some random ROP4
//		goto emulsel;	
		return S_OK;
	}
//emulsel:
//	return EmulatedBltSelect16(pBltParms);	//< This will be called in EmulatedBlt (EmulatedBltRotate == SelectBlt)
	return S_OK;
}
/**
*	@fn	SCODE S3C2450DISP::SourceRegionCacheClean(GPEBltParms *pBltParms)
*	@brief	If possible, clean dcahce for Source Rectangle
*	@param	pBltParms	Blit Parameter Information Structure
*	@sa		GPEBltParms
*	@note	for support cb to ncnb bitblt
*	@note	Bitmap image has top-down or bottom-up style memory region,
*			we can determine that by stride as positive(top-down) or as negative(bottom-up)
*			bottom-up bitmap mean buffer's start address is last addres of image buffer
*			image's start address is calculated as (Buffer address + Stride(negative) * height)
*			and 
*/
#define PFN_SHIEFT UserKInfo[KINX_PFN_SHIFT]
#define MAX_SUPPORTED_PFN (MAXDWORD>>PFN_SHIEFT)
#define PAGE_MASK (PAGE_SIZE-1)
#define DBGMSG_SRCC	(FALSE)
SCODE S3C2450DISP::SourceRegionCacheClean(	GPEBltParms *pBltParms	)
{
	BOOL		m_fLocked;
	LPVOID		m_lpvLockedAddress;
	DWORD		m_dwLockedSize;
	DWORD		dwLength = 0;
	DWORD		dwSrcSurfaceSize = 0;
	PDWORD		m_pdwPhysAddress = 0;
	PDWORD		m_pdwPhysLength;
	PBYTE *		m_ppVirtAddress;
	PVOID		pUseBufferPtr = 0;
//	static DWORD		uCount = 0;
//	static DWORD		uPossibleCount = 0;

	if(!pBltParms->pSrc || (pBltParms->pSrc->Format() != pBltParms->pDst->Format())	)
	{
		RETAILMSG(DBGMSG_SRCC,(TEXT("Color Format is not matched.\r\n")));
		return -1;
	}
#if USE_PACSURF
	if( ((PACSurf *)pBltParms->pSrc)->m_fPLAllocated == 0)
	{
		RETAILMSG(DBGMSG_SRCC,(TEXT("This is not PLA.\r\n")));
		return -1;
	}
#endif
	if(pBltParms->pSrc->Stride() < 0)
	{
		RETAILMSG(DBGMSG_SRCC, (TEXT("Currently we cannot handle bottom-up bitmap using HW\r\n")));
		return -1;
	}
	
	// Check the start address
	// Check if data is allocated across different pages
	// usable macros
	// 	VM_PAGE_OFST_MASK
	//	VM_PAGE_SHIFT
	//	VM_PAGE_SIZE

//	uCount++;

	ASSERT(m_pdwPhysAddress==0);
	ASSERT((PAGE_MASK & PAGE_SIZE) == 0 );

	dwLength = pBltParms->pSrc->Height() * abs(pBltParms->pSrc->Stride());
	RETAILMSG(DBGMSG_SRCC,(TEXT("Source Height : %d, Stride : %d, X:%d, Y:%d\r\n"),pBltParms->pSrc->Height(), pBltParms->pSrc->Stride(), pBltParms->xPositive, pBltParms->yPositive));
	// Todo: Check the Data Size
	// if size is not big to get efficiency. return false
	if(pBltParms->pSrc->Stride() < 0)
	{
		pUseBufferPtr = (PDWORD)pBltParms->pSrc->Buffer() - dwLength;	
	}
	else
	{
		pUseBufferPtr = (PDWORD)pBltParms->pSrc->Buffer();	
	}
	
	UINT nPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES( pUseBufferPtr, dwLength );

	m_pdwPhysAddress = new DWORD[3*nPages];		// It's really sturcture {m_pdwPhysAddress, m_pdwPhysLength, m_ppVirtAddress}
        
	if (m_pdwPhysAddress) {
		m_pdwPhysLength = m_pdwPhysAddress + nPages;
		m_ppVirtAddress = (PBYTE *)(m_pdwPhysAddress + 2*nPages);

		RETAILMSG(DBGMSG_SRCC,(TEXT("pUseBufferPtr:0x%x, dwLength:%d, m_pdwPhysAddress:0x%x.\r\n"), pUseBufferPtr, dwLength, m_pdwPhysAddress));
		
		m_fLocked = LockPages( pUseBufferPtr, dwLength, m_pdwPhysAddress, LOCKFLAG_QUERY_ONLY);		// Src to Dst
            
		if (!m_fLocked) { // Create table for Physical Address and length.
			RETAILMSG(DBGMSG_SRCC,(TEXT("LockPages is Failed : %d\r\n"), GetLastError() ));
			
			if(m_pdwPhysAddress)
			{
				delete [] m_pdwPhysAddress;
				m_pdwPhysAddress=NULL;
			}				
			return -1;
		}

		m_lpvLockedAddress = pUseBufferPtr;
		m_dwLockedSize = dwLength;

		RETAILMSG(DBGMSG_SRCC,(TEXT("pUseBufferPtr:0x%x.\r\n"), pUseBufferPtr));

#if 0	//USE_PACSURF
/// This is fast-path for PACSurf. skip contigious checking
	if( ((PACSurf *)pBltParms->pSrc)->m_fPLAllocated == 1)
	{
		// Just Contiguous Source Surface can use HW // Cahce flush for all surface region
		CacheRangeFlush( (PBYTE)pBltParms->pSrc->Buffer(), 
			pBltParms->pSrc->Height() * abs(pBltParms->pSrc->Stride()),
			CACHE_SYNC_WRITEBACK);		
		RETAILMSG(DBGMSG_SRCC,(TEXT("m_pdwPhysAddress : 0x%x\r\n"), m_pdwPhysAddress));
		m_dwSourceSurfacePA = m_pdwPhysAddress[0];

		if(m_pdwPhysAddress)
		{
			delete [] m_pdwPhysAddress;
			m_pdwPhysAddress=NULL;
		}	
		return S_OK;
	}
#endif		

		
		/// Get each Physical address pages from pagelocked information
		for (DWORD dwIndex = 0; dwIndex< nPages; dwIndex++) {
			if (m_pdwPhysAddress[dwIndex] > MAX_SUPPORTED_PFN) {
				ASSERT(FALSE);
				if(m_pdwPhysAddress)
				{
					delete [] m_pdwPhysAddress;
					m_pdwPhysAddress=NULL;
				}					
				return -1;
			}
                
			DWORD dwSize = min((PAGE_SIZE - ((DWORD)pUseBufferPtr & PAGE_MASK)),dwLength) ;
		
			m_pdwPhysAddress[dwIndex] = (m_pdwPhysAddress[dwIndex]<<PFN_SHIEFT) + ((DWORD)pUseBufferPtr & PAGE_MASK);
			m_pdwPhysLength[dwIndex] = dwSize;
			m_ppVirtAddress[dwIndex] = (PBYTE)pUseBufferPtr;
			dwLength -= dwSize;
			pUseBufferPtr = (PBYTE)pUseBufferPtr+dwSize;
			RETAILMSG(DBGMSG_SRCC,(TEXT("dwIndex : %d, m_pdwPhysAddress[%d]:0x%x, m_pdwPhysLength[%d]:%d, dwSize:%d, pUseBufferPtr:0x%x.\r\n"),
				dwIndex,
				dwIndex, m_pdwPhysAddress[dwIndex],
				dwIndex, m_pdwPhysLength[dwIndex],
				dwSize,
				pUseBufferPtr
			));			
		}
                
		/// Check if Source Pages is contiguous in Physical memory address.
		DWORD dwRead = 1;
		while (dwRead < nPages) {
			if (m_pdwPhysAddress[dwRead - 1] + m_pdwPhysLength[dwRead - 1] == m_pdwPhysAddress[dwRead]) {
				// m_dwBlocks and m_dwBlocks+1 is contiguous.
				dwRead++;
			}
			else { // No match, We cannot use HW
				RETAILMSG(DBGMSG_SRCC,(TEXT("Source Memory Blocks is not contiguous : Go Emul path\r\n")));
				if(m_pdwPhysAddress)
				{
					delete [] m_pdwPhysAddress;
					m_pdwPhysAddress=NULL;
				}					
				return -1;
			}
		}
		// Merge to one big contiguous memory block
		if(nPages > 1){
			for(dwRead = 1 ; dwRead < nPages; dwRead++) {
				m_pdwPhysLength[0] += m_pdwPhysLength[dwRead];
			}
		}
        }
        else
        {
        	RETAILMSG(DBGMSG_SRCC,(TEXT("Not Enough Memory for m_pdwPhysAddress\r\n")));
		if(m_pdwPhysAddress)
		{
			delete [] m_pdwPhysAddress;
			m_pdwPhysAddress=NULL;
		}	        	
        	return -1;
        }
       
//	 uPossibleCount ++;				
//	RETAILMSG(TRUE,(TEXT("DCache Clean , Available Flow Count :  %d avail. / %d Total \r\n"),uPossibleCount, uCount));				
	RETAILMSG(DBGMSG_SRCC,(TEXT("CacheFlush Start : 0x%x length : %d\r\n"),
		(PBYTE)pBltParms->pSrc->Buffer(), 
		pBltParms->pSrc->Height() * abs(pBltParms->pSrc->Stride())));
	// Just Contiguous Source Surface can use HW // Cahce flush for all surface region
	CacheRangeFlush( (PBYTE)pBltParms->pSrc->Buffer(), 
		pBltParms->pSrc->Height() * abs(pBltParms->pSrc->Stride()),
		CACHE_SYNC_WRITEBACK);		
	RETAILMSG(DBGMSG_SRCC,(TEXT("m_pdwPhysAddress : 0x%x\r\n"), m_pdwPhysAddress));
	m_dwSourceSurfacePA = m_pdwPhysAddress[0];

	if(m_pdwPhysAddress)
	{
		delete [] m_pdwPhysAddress;
		m_pdwPhysAddress=NULL;
	}
		
	return S_OK;	
}

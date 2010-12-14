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

Module Name:	line.cpp

Abstract:		Implementation for linedraw on display driver

Functions:	WrappedEmulatedLine, Line, AcceleratedSolidLine

Notes:	

--*/
#include <dispperf.h>
#include "precomp.h"

#define SIMPLE_LINEACCEL	//< This will be removed in the future

/**
*	@fn	SCODE S3C2450DISP::AcceleratedSolidLine(GPELineParms *pLineParms)
*	@brief Draw Solid line with 1 width.
*	@param	pLineParms	Line Drawing Information Structure
*	@	
*/
SCODE S3C2450DISP::AcceleratedSolidLine(GPELineParms *pLineParms)
{
	SCODE retval;

	retval = S_OK;

	int xStart = pLineParms->xStart;
	int yStart = pLineParms->yStart;
	

	/**
	*	Prepare DestinationSurface Information
	*
	*/
	SURFACE_DESCRIPTOR descDstSurface;

	descDstSurface.dwBaseaddr = (IMAGE_FRAMEBUFFER_DMA_BASE + (( S3C2450Surf *)(pLineParms->pDst))->OffsetInVideoMemory());
	descDstSurface.dwColorMode = pLineParms->pDst->Format();
	descDstSurface.dwHoriRes = pLineParms->pDst->Width();
	descDstSurface.dwVertRes = pLineParms->pDst->Height();	
	
	m_oG2D->SetDstSurface(&descDstSurface);
	m_oG2D->SetClipWindow(pLineParms->prclClip);
	m_oG2D->SetTransparentMode(0, 0x00000000L);	

	if(pLineParms->cPels == 1)
	{
		RETAILMSG(1,(_T("PI\r\n")));	
		EnterCriticalSection(&m_cs2D);	
		m_oG2D->PutPixel(xStart, yStart, pLineParms->solidColor);
		LeaveCriticalSection(&m_cs2D);		
		RETAILMSG(1,(_T("PO\r\n")));
	}
	else
	{
#ifdef SIMPLE_LINEACCEL
		int xEnd = 1;
		int yEnd = 1;	

		// Line is vertical or horizontal
		// Use fill-blt to draw a line starting at
		// pLineParms->xStart, pLineParms->yStart
		// in the direction specified by pLineParms->iDir
		// for a total of pLineParms->cPels pixels
		//           -y
		//           ^
		//     ¢Ø  5 | 6  ¢Ö 
		//       ¢Ø  |  ¢Ö
		//      4  ¢Ø|¢Ö  7
		// -x<-------+-------> +x
		//      3  ¢×|¢Ù  0
		//       ¢×  |  ¢Ù	
		//     ¢×  2 | 1  ¢Ù
		//			 +y

		switch (pLineParms->iDir & 0x07)
		{
			// major axis is X-axis, dM = X-axis, dN = Y-axis
			case	0:				// +x +1/2y
				yEnd = yStart ;
				xEnd = xStart + pLineParms->cPels;
				break;
			case	1:				// +1/2x + y
				yEnd = yStart + pLineParms->cPels;
				xEnd = xStart;
				break;
			case	2:				// -1/2x + y
				yEnd = yStart + pLineParms->cPels;
				xEnd = xStart;
				break;
			case	3:				// -x + 1/2y
				yEnd = yStart;
				xEnd = xStart - pLineParms->cPels;
				break;			
			case	4:				// -x - 1/2y
				yEnd = yStart;
				xEnd = xStart - pLineParms->cPels;
				break;
			case	5:				// -1/2x - y
				yEnd = yStart - pLineParms->cPels;
				xEnd = xStart;
				break;
			case	6:				// +1/2x - y
				yEnd = yStart - pLineParms->cPels;
				xEnd = xStart;
				break;
			case	7:				// +x -1/2y
				yEnd = yStart;
				xEnd = xStart + pLineParms->cPels;
				break;
		}	


		bool IsLastDraw = false;
		
		if(xEnd < 0) xEnd = 0;		// Cut negative coordinate
		if(yEnd < 0) yEnd = 0;		// negative coordinate is not supported on H/W IP
		EnterCriticalSection(&m_cs2D);
		m_oG2D->PutLine(xStart, yStart, xEnd, yEnd, pLineParms->solidColor, IsLastDraw);
		LeaveCriticalSection(&m_cs2D);		
#else
		int nMajorLength = pLineParms->dM / 16;
		int nMinorLength = pLineParms->dN / 16;
		int xEnd = 1;
		int yEnd = 1;	
		int minorlength = 0;						// default 0
		
		if (pLineParms->dN)            // The line has a diagonal component (we'll refresh the bounding rect)
		{
			minorlength = ((pLineParms->cPels * pLineParms->dN) / pLineParms->dM);
		}

		// Line is vertical or horizontal
		// Use fill-blt to draw a line starting at
		// pLineParms->xStart, pLineParms->yStart
		// in the direction specified by pLineParms->iDir
		// for a total of pLineParms->cPels pixels
		//           -y
		//           ^
		//     ¢Ø  5 | 6  ¢Ö 
		//       ¢Ø  |  ¢Ö
		//      4  ¢Ø|¢Ö  7
		// -x<-------+-------> +x
		//      3  ¢×|¢Ù  0
		//       ¢×  |  ¢Ù	
		//     ¢×  2 | 1  ¢Ù
		//			 +y

		switch (pLineParms->iDir & 0x07)
		{
			// major axis is X-axis, dM = X-axis, dN = Y-axis
			case	0:				// +x +1/2y
				yEnd = yStart + nMinorLength;
				xEnd = xStart + nMajorLength;
				break;
			case	1:				// +1/2x + y
				yEnd = yStart + nMajorLength;
				xEnd = xStart + nMinorLength;
				break;
			case	2:				// -1/2x + y
				yEnd = yStart + nMajorLength;
				xEnd = xStart - nMinorLength;
				break;
			case	3:				// -x + 1/2y
				yEnd = yStart + nMinorLength;
				xEnd = xStart - nMajorLength;
				break;			
			case	4:				// -x - 1/2y
				yEnd = yStart - nMinorLength;
				xEnd = xStart - nMajorLength;
				break;
			case	5:				// -1/2x - y
				yEnd = yStart - nMajorLength;
				xEnd = xStart - nMinorLength;
				break;
			case	6:				// +1/2x - y
				yEnd = yStart - nMajorLength;
				xEnd = xStart + nMinorLength;
				break;
			case	7:				// +x -1/2y
				yEnd = yStart - nMinorLength;
				xEnd = xStart + nMajorLength;
				break;
		}	


		bool IsLastDraw = false;
		int y_intercept = 0;
		int x_intercept = 0;
		int x1 = xStart * 16;		// interpolation
		int x2 = xEnd * 16;
		int y1 = yStart * 16;
		int y2 = yEnd * 16;

		ASSERT(xStart >=0);
		ASSERT(yStart >=0);
		// y=(y2-y1)/(x2-x1)x + b
		// b = (y1x2-x1y2)/(x2-x1)		
		// 1. line is out over y-axis
		// y value when x=0 is y=b=(y1x2-x1y2)/(x2-x1)
		if(x2 < 0)
		{
			if(x2 == x1)	// do not draw. it is clipped.
			{
				return retval; 
			}
			if(y2 == y1)	// Horizontal Line.
			{
				if(yEnd < 0) yEnd = 0;
			RETAILMSG(0,(_T("ACCDRAW1 : LT(%d,%d)~RB(%d,%d)\n "), xStart, yStart, xEnd, yEnd));				
				m_oG2D->PutLine(xStart, yStart, 0, yEnd, pLineParms->solidColor, IsLastDraw);
				return retval;
			}

			y_intercept = ((y1*x2)-(x1*y2))/(x2-x1);

			if(y_intercept < 0 )	// Recalc for x
			{
				x_intercept = ((x1*y2)-(y1*x2))/(y2-y1);
				if(x_intercept < 0)
				{
					RETAILMSG(TRUE,(_T("Line Draw error\n")));
				}
				else	// (xStart, yStart) ~ (x_intercept/16, 0)
				{
			RETAILMSG(0,(_T("ACCDRAW2 : LT(%d,%d)~RB(%d,%d)\n "), xStart, yStart, xEnd, yEnd));									
					m_oG2D->PutLine(xStart, yStart, x_intercept/16, 0, pLineParms->solidColor, IsLastDraw);
					return retval;
				}
			}
			else	// (xStart, yStart) ~ (0, y_intercept/16)
			{
		RETAILMSG(0,(_T("ACCDRAW3 : LT(%d,%d)~RB(%d,%d)\n "), xStart, yStart, xEnd, yEnd));								
					m_oG2D->PutLine(xStart, yStart, 0, y_intercept/16, pLineParms->solidColor, IsLastDraw);
					return retval;
			}
		}
		else if(y2 < 0)
		{
			if(y1 == y2)		// do not draw. it is clipped
			{
				return retval;		
			}
			if(x1 == x2)	// Vertical Line
			{
				if(xEnd < 0) xEnd = 0;
		RETAILMSG(0,(_T("ACCDRAW4 : LT(%d,%d)~RB(%d,%d)\n "), xStart, yStart, xEnd, yEnd));								
				m_oG2D->PutLine(xStart, yStart, xEnd, 0, pLineParms->solidColor, IsLastDraw);
				return retval;			
			}				
			x_intercept = ((x1*y2)-(y1*x2))/(y2-y1);			
			if(x_intercept < 0)	// Recalc for y
			{
				y_intercept = ((y1*x2)-(x1*y2))/(x2-x1);
				if(y_intercept < 0)
				{
					RETAILMSG(TRUE,(_T("Line Draw error\n")));
				}
				else	// (xStart, yStart) ~ (0, y_intercept)
				{
		RETAILMSG(0,(_T("ACCDRAW5 : LT(%d,%d)~RB(%d,%d)\n "), xStart, yStart, xEnd, yEnd));									
					m_oG2D->PutLine(xStart, yStart, 0, y_intercept/16, pLineParms->solidColor, IsLastDraw);
					return retval;
				}
			}
			else	// (xStart, yStart) ~ (x_intercept, 0)
			{
		RETAILMSG(0,(_T("ACCDRAW6 : LT(%d,%d)~RB(%d,%d)\n "), xStart, yStart, xEnd, yEnd));								
					m_oG2D->PutLine(xStart, yStart, x_intercept/16, 0, pLineParms->solidColor, IsLastDraw);				
					return retval;
			}
		}
		else
		{
		RETAILMSG(0,(_T("ACCDRAW7 : LT(%d,%d)~RB(%d,%d)\n "), xStart, yStart, xEnd, yEnd));											
			m_oG2D->PutLine(xStart, yStart, xEnd, yEnd, pLineParms->solidColor, IsLastDraw);
			return retval;
		}
#endif
	}


	return retval;
}

SCODE
S3C2450DISP::WrappedEmulatedLine(
    GPELineParms *lineParameters
    )
{
    SCODE retval;
    RECT  bounds;
    int   N_plus_1;                // Minor length of bounding rect + 1

    // calculate the bounding-rect to determine overlap with cursor
    if (lineParameters->dN)            // The line has a diagonal component (we'll refresh the bounding rect)
    {
        N_plus_1 = 2 + ((lineParameters->cPels * lineParameters->dN) / lineParameters->dM);
    }
    else
    {
        N_plus_1 = 1;
    }

    switch(lineParameters->iDir)
    {
        case 0:
            bounds.left = lineParameters->xStart;
            bounds.top = lineParameters->yStart;
            bounds.right = lineParameters->xStart + lineParameters->cPels + 1;
            bounds.bottom = bounds.top + N_plus_1;
            break;
        case 1:
            bounds.left = lineParameters->xStart;
            bounds.top = lineParameters->yStart;
            bounds.bottom = lineParameters->yStart + lineParameters->cPels + 1;
            bounds.right = bounds.left + N_plus_1;
            break;
        case 2:
            bounds.right = lineParameters->xStart + 1;
            bounds.top = lineParameters->yStart;
            bounds.bottom = lineParameters->yStart + lineParameters->cPels + 1;
            bounds.left = bounds.right - N_plus_1;
            break;
        case 3:
            bounds.right = lineParameters->xStart + 1;
            bounds.top = lineParameters->yStart;
            bounds.left = lineParameters->xStart - lineParameters->cPels;
            bounds.bottom = bounds.top + N_plus_1;
            break;
        case 4:
            bounds.right = lineParameters->xStart + 1;
            bounds.bottom = lineParameters->yStart + 1;
            bounds.left = lineParameters->xStart - lineParameters->cPels;
            bounds.top = bounds.bottom - N_plus_1;
            break;
        case 5:
            bounds.right = lineParameters->xStart + 1;
            bounds.bottom = lineParameters->yStart + 1;
            bounds.top = lineParameters->yStart - lineParameters->cPels;
            bounds.left = bounds.right - N_plus_1;
            break;
        case 6:
            bounds.left = lineParameters->xStart;
            bounds.bottom = lineParameters->yStart + 1;
            bounds.top = lineParameters->yStart - lineParameters->cPels;
            bounds.right = bounds.left + N_plus_1;
            break;
        case 7:
            bounds.left = lineParameters->xStart;
            bounds.bottom = lineParameters->yStart + 1;
            bounds.right = lineParameters->xStart + lineParameters->cPels + 1;
            bounds.top = bounds.bottom - N_plus_1;
            break;
        default:
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Invalid direction: %d\r\n"), lineParameters->iDir));
            return E_INVALIDARG;
    }

    // check for line overlap with cursor and turn off cursor if overlaps
	if (m_CursorVisible && !m_CursorDisabled) 
	{
		RotateRectl(&m_CursorRect);
		if (m_CursorRect.top < bounds.bottom && m_CursorRect.bottom > bounds.top &&
			m_CursorRect.left < bounds.right && m_CursorRect.right > bounds.left)
		{	
			RotateRectlBack(&m_CursorRect);
			CursorOff();
			m_CursorForcedOff = TRUE;
		}
		else 
			RotateRectlBack(&m_CursorRect);
	}

#if G2D_ACCELERATE
	if(//	(m_VideoPowerState != VideoPowerOff) &&	// to avoid hanging while bring up display H/W
		(!m_iRotate) &&
		((S3C2450Surf *)(lineParameters->pDst))->InVideoMemory() &&
		(lineParameters->mix == 0x0d0d) &&
		(lineParameters->style == 0) 
#ifdef SIMPLE_LINEACCEL
			&& (lineParameters->dN == 0) &&
			(lineParameters->dM == lineParameters->cPels*16) 
#endif
	 	)
	{
#ifdef DO_DISPPERF
		DispPerfType(DISPPERF_ACCEL_HARDWARE);
#endif
		m_oG2D->WaitForIdle();	//< Wait for Fully Empty Command Fifo for all HW Line Drawing Request	
		RETAILMSG(FALSE, (TEXT("ALine:(%d,%d) + m(%d), n(%d), cPels(%d), dir(%d)\n"), lineParameters->xStart, lineParameters->yStart, lineParameters->dM, lineParameters->dN, lineParameters->cPels, lineParameters->iDir));
		retval = AcceleratedSolidLine(lineParameters);
	}
	else
	{
		RETAILMSG(FALSE, (TEXT("ELine:(%d,%d) + m(%d), n(%d), cPels(%d), dir(%d)\n"), lineParameters->xStart, lineParameters->yStart, lineParameters->dM, lineParameters->dN, lineParameters->cPels, lineParameters->iDir));	
		retval = EmulatedLine (lineParameters);
	}
#else
	// do emulated line
	retval = EmulatedLine (lineParameters);
#endif

    // se if cursor was forced off because of overlap with line bouneds and turn back on
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }

    return    retval;

}

SCODE    S3C2450DISP::Line(GPELineParms *lineParameters, EGPEPhase phase)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::Line\r\n")));

    if (phase == gpeSingle || phase == gpePrepare)
    {
#ifdef DO_DISPPERF    	
        DispPerfStart(ROP_LINE);
#endif
        if ((lineParameters->pDst != m_pPrimarySurface))
        {
            lineParameters->pLine = EmulatedLine;
        }
        else
        {
            lineParameters->pLine = (SCODE (GPE::*)(struct GPELineParms *)) WrappedEmulatedLine;
        }
    }
    else if (phase == gpeComplete)
    {
#ifdef DO_DISPPERF    
        DispPerfEnd(0);
#endif        
    }
    return S_OK;
}



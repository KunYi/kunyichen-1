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

Module Name:	surface allocation/manipulation/free routines

Abstract:

Functions:


Notes:


--*/

#include "precomp.h"

#define DBGLCD	0
#define DBGLCD1	0

static UINT nCount = 0;

UINT32 saved_x;
UINT32 saved_y;
UINT32 saved_width;
UINT32 saved_height;

//  This method is used to allocate DirectDraw enabled surfaces
SCODE	S3C2450DISP::AllocSurface(DDGPESurf **ppSurf, int width, int height, 
					EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags)
{
	SCODE sc = S_OK;
	DWORD pa;
#if 1
	DWORD 	bpp  	= EGPEFormatToBpp[format];
	LONG 	align 	= m_nSurfaceBitsAlign / 8;
    	DWORD 	alignedWidth = ((width + align - 1) & (- align));
    	DWORD 	nSurfaceSize = (bpp * (alignedWidth * height)) / 8;
#endif
	ULONG stride = (width * EGPEFormatToBpp[format] + 7) >> 3;
	ULONG size = stride * height;//MulDiv(stride, height, 1);

	if(width == 0)
		RETAILMSG(1, (TEXT("WidthSave = %d, HeightSave = %d"), m_nScreenWidthSave, m_nScreenHeightSave));

	RETAILMSG(DBGLCD, (TEXT("AllocSurface DirectDraw, surfaceFlags = %d, f = %d, m_pMode->format = %d , pF = %d\r\n"), surfaceFlags, format, m_pMode->format , pixelFormat));
	RETAILMSG(DBGLCD, (TEXT("align = %d, alignedwidth = %d, surfacesize = %d, height = %d, width = %d stride = %d\r\n"), align, alignedWidth, nSurfaceSize, height, width, stride));


	// Is video memory required/prefered?
	//apart from the primary and directdraw surfaces, all others are 
	//allocated in system memory
	if (/*(surfaceFlags & GPE_BACK_BUFFER) ||*/
        	(surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY) || 
        	((surfaceFlags & GPE_PREFER_VIDEO_MEMORY) && (format == m_pMode->format)))
	{		
		if (size == -1) 
		{
	    		return E_OUTOFMEMORY;
		}
		/*if(surfaceFlags & GPE_BACK_BUFFER)
			RETAILMSG(DBGLCD, (TEXT("BB -------------- In BackBuffer alloc -----\r\n")));*/

		// Attempt to allocate from internal SRAM
		SurfaceHeap *pHeap = m_pVideoMemoryHeap->Alloc(size);

		if (pHeap != NULL) 
		{
			RETAILMSG(DBGLCD, (TEXT("pHeap->Address() = 0x%x, pHeap = 0x%x, remains = 0x%x, size = 0x%x\r\n"), pHeap->Address(), pHeap, m_pVideoMemoryHeap->Available(), m_pVideoMemoryHeap->Size()));		
			// We get memory in SRAM, build surface there			
			pa  = pHeap->Address() - (DWORD)m_VirtualFrameBuffer/* + IMAGE_FRAMEBUFFER_DMA_BASE*/;
			
			RETAILMSG(DBGLCD, (TEXT("---------------------> pa = 0x%x\r\n"), pa));
			if ((*ppSurf = new S3C2450Surf(width, height, pa, (VOID*)pHeap->Address(),
				stride, format, pixelFormat, pHeap)) == NULL)
			{
			    	// When allocation failed, we are out of memory...
			    	RETAILMSG(DBGLCD, (TEXT("--------------------> Failed allocating memory for S3C2450Surf --------------------------\r\n")));
			    	pHeap->Free();
			    	return E_OUTOFMEMORY;
			}

			// We are done
			RETAILMSG(DBGLCD, (TEXT("--------------------> Succeeded allocating memory for S3C2450Surf --------------------------\r\n")));
			return S_OK;
		}
		RETAILMSG(DBGLCD, (TEXT("Surface Heap size is not enough to allocation requested surface\r\n")));				
		if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
		{
		    	*ppSurf = (DDGPESurf *)NULL;
		    	return E_OUTOFMEMORY;
		}
	}

	if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
	{
	    	*ppSurf = (DDGPESurf *)NULL;
	    	return  E_OUTOFMEMORY;
	}

	RETAILMSG(DBGLCD, (TEXT("* Allocating surface in system memory DD*\r\n")));
    	*ppSurf = new DDGPESurf(width, height, stride, format, pixelFormat);
#if 0	//{		
	if (*ppSurf == NULL || (*ppSurf)->Buffer() == NULL) 
	{
	    	RETAILMSG(DBGLCD, (L"ERROR: S3C2450Surf::AllocSurface: Failed allocate surface (width: %d, height: %d, format %d)\r\n", width, height, format));
	    	delete *ppSurf;
	    	*ppSurf = NULL;
	    	sc = E_OUTOFMEMORY;
	    	goto cleanUp;
	}
#else
	if (*ppSurf != NULL)
    	{
		// check we allocated bits succesfully
		if ( (*ppSurf)->Buffer() == NULL)
		{
		    	delete *ppSurf;
		    	ppSurf = NULL;
		}
		else
		{
			// We are done
			RETAILMSG(DBGLCD, (L"S3C2450Surf::AllocSurface: Surface in memory (width: %d, height: %d, format %d, 0x%08x)\r\n",
	    			width, height, format, (*ppSurf)->Buffer()));
		    	return  S_OK;
		}
	}
#endif	//}

		return E_OUTOFMEMORY;

}

/*
SCODE S3C2450DISP::AllocSurface(DDGPESurf **ppSurf, int width, int height, EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags)
{
	RETAILMSG(DBGLCD, (TEXT("AllocSurface DirectDraw, surfaceFlags = %d, f = %d, m_pMode->format = %d , pF = %d\r\n"), surfaceFlags, format, m_pMode->format , pixelFormat));
	RETAILMSG(DBGLCD, (TEXT("height = %d, width = %d\r\n"), height, width));

    if ((surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY) || (format == m_pMode->format) && (surfaceFlags & GPE_PREFER_VIDEO_MEMORY))
    {
        // Attempt to allocate from video memory

        WORD bytepp = (EGPEFormatToBpp[format]/8);
        ulong stride = ((width+7)&~0x7)*bytepp;        
        ulong heapsize = MulDiv (stride, height, 1);
        if (heapsize == -1)
        {
            return E_OUTOFMEMORY;       // overflow
        }

        SurfaceHeap *pHeap = m_pVideoMemoryHeap->Alloc(heapsize);
        if (pHeap)
        {
            *ppSurf = new S3C2450Surf ( width, height,
                                    (DWORD)pHeap->Address() - (DWORD)m_VirtualFrameBuffer,
                                    (void*)pHeap->Address(),
                                    stride,
                                    format,
                                    pixelFormat,
                                    pHeap);

            if (!(*ppSurf))
            {
                pHeap->Free();
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }

        if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
        {
            *ppSurf = (DDGPESurf *)NULL;
            return E_OUTOFMEMORY;
        }
    }

    if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
    {
        *ppSurf = (DDGPESurf *)NULL;

        return  E_OUTOFMEMORY;
    }

    // Allocate from system memory
    RETAILMSG(1, (TEXT("Creating a GPESurf in system memory DDDD. EGPEFormat = %d\r\n"), (int) format));
    {
        DWORD bpp  = EGPEFormatToBpp[format];
        DWORD stride = ((bpp * width + 31) >> 5) << 2;
        DWORD nSurfaceBytes = stride * height;

        *ppSurf = new DDGPESurf(width, height, stride, format, pixelFormat);
    }
    if (*ppSurf != NULL)
    {
        // check we allocated bits succesfully
        if ( (*ppSurf)->Buffer() == NULL)
        {
            delete *ppSurf;
        }
        else
        {
            return  S_OK;
        }
    }

    return E_OUTOFMEMORY;
}
*/

//  This method is called for all normal surface allocations from ddgpe and gpe
SCODE S3C2450DISP::AllocSurface(GPESurf **ppSurf, INT width, INT height, EGPEFormat format, INT surfaceFlags)
{
	SCODE sc = S_OK;
	
	ULONG stride = (width * EGPEFormatToBpp[format] + 7) >> 3;
#if USE_PACSURF	
	/// Only Support 16bpp, 24bpp, 32bpp
	if(  ( (format == gpe16Bpp || format == gpeDeviceCompatible)
#if G2D_BLT_OPTIMIZE	
	         && (width*height*2 > PAC_ALLOCATION_BOUNDARY) 
#endif
	         )
	||
	      ( (format == gpe24Bpp || format == gpe32Bpp)
#if G2D_BLT_OPTIMIZE	      
	      	&& (width*height*4 > PAC_ALLOCATION_BOUNDARY )
#endif	      	
	      	)
	      	) 
	{
		/// try to allocate physically linear address
			*ppSurf = new PACSurf(width, height, format);
		
		if (*ppSurf == NULL)
		{
			RETAILMSG(1,(_T("\n[DISPDRV:ERR] AllocSurface() : PACSurface allocate Failed -> Try to allocate Normal GPE Surface")));
		}
		else if ((*ppSurf)->Buffer() == NULL)
		{
			delete *ppSurf;
			*ppSurf = NULL;

			RETAILMSG(1,(_T("\n[DISPDRV:ERR] AllocSurface() : PACSurface Buffer is NULL -> Try to allocate Normal GPE Surface")));
		}
		else		/// PAC Allocation succeeded.
		{
			RETAILMSG(DBGLCD,(_T("\n[DISPDRV] AllocSurface() : PACSurf() Allocated in System Memory")));	
			return S_OK;
		}
	}
#endif
	/// if allocation is failed or boundary condition is not met, just create GPESurf in normal system memory that can be non-linear physically.	
	
	RETAILMSG(DBGLCD, (TEXT("* Allocating surface in system memory normal*\r\n")));

    	*ppSurf = new GPESurf(width, height, format);

	if (*ppSurf == NULL || (*ppSurf)->Buffer() == NULL) 
	{
	    	RETAILMSG(DBGLCD, (L"ERROR: S3C2450Surf::AllocSurface: Failed allocate surface (width: %d, height: %d, format %d)\r\n",width, height, format));
	    	delete *ppSurf;
	    	*ppSurf = NULL;
	    	sc = E_OUTOFMEMORY;
	    	goto cleanUp;
	}

	// We are done
	RETAILMSG(DBGLCD, (L"S3C2450Surf::AllocSurface: Surface in memory (width: %d, height: %d, format %d, pixelformat %d 0x%08x)\r\n",
	    width, height, format, EGPEFormatToEDDGPEPixelFormat[format], (*ppSurf)->Buffer()));

cleanUp:
	return sc;
}

void S3C2450DISP::SetVisibleSurface( GPESurf *pTempSurf, BOOL bWaitForVBlank)
{
	static int timeoutcnt=0;
    DWORD we;
#ifdef	HIGH_PRIORITY_INTR
	int iPriority;
	HANDLE hThread;

	hThread = GetCurrentThread();
	iPriority = CeGetThreadPriority(hThread);
	CeSetThreadPriority(hThread, DISPDRV_IST_PRIORITY);
#endif	
	S3C2450Surf *pSurf = (S3C2450Surf *) pTempSurf;
	EnterCriticalSection(&m_CS);
	// assume Synchronous to VSYNC

	EnableInterrupt();
	we = WaitForSingleObject(m_hVSYNCInterruptEvent,1000/*INFINITE*/);
	DisableInterrupt();
	InterruptDone(m_dwVSYNCSysIntr);

	if(we != WAIT_OBJECT_0)
	{
		timeoutcnt++;
		RETAILMSG(1,(TEXT("Surface Flipping Time Out  %d !!!\n"), timeoutcnt));
		for(int i=0;i<78;i++)
			RETAILMSG(1,(TEXT("0x%08X = 0x%08X\n"),(DWORD*)((DWORD*)m_pLCDReg + i),*(DWORD*)((DWORD*)m_pLCDReg + i)));
			
		RETAILMSG(1,(TEXT("saved_x=%d"),saved_x));
		RETAILMSG(1,(TEXT("saved_y=%d"),saved_y));
		RETAILMSG(1,(TEXT("saved_width=%d"),saved_width));
		RETAILMSG(1,(TEXT("saved_height=%d"),saved_height));				
		while(1);
	}

	if(pSurf->m_bIsOverlay == FALSE)
	{
		//RETAILMSG(1,(TEXT("pSurf->OffsetInVideoMemory()=0x%08X\n"),pSurf->OffsetInVideoMemory()));
		m_pVisibleSurface = pSurf;
		m_pLCDReg->VIDW00ADD0B0 = (UINT32)(pSurf->OffsetInVideoMemory() + IMAGE_FRAMEBUFFER_DMA_BASE);		
				// buffer end address
		m_pLCDReg->VIDW00ADD1B0 = (UINT32)(pSurf->OffsetInVideoMemory() + IMAGE_FRAMEBUFFER_DMA_BASE) + (LCD_XSIZE_TFT*LCD_YSIZE_TFT*2);
				// buffer size 
		m_pLCDReg->VIDW00ADD2B0 = (0<<VIDWxADD2_OFFSET_SIZE_S)|(LCD_XSIZE_TFT*2);
	}
	else
	{

		m_pLCDReg->VIDW01ADD0 = (UINT32)(pSurf->OffsetInVideoMemory() + IMAGE_FRAMEBUFFER_DMA_BASE);		
				// buffer end address
		m_pLCDReg->VIDW01ADD1 = (UINT32)(pSurf->OffsetInVideoMemory() + IMAGE_FRAMEBUFFER_DMA_BASE) + 
														(pSurf->Width()*pSurf->Height()*2);
				// buffer size 
		m_pLCDReg->VIDW01ADD2 = (0<<VIDWxADD2_OFFSET_SIZE_S)|(pSurf->Width()*2);		
	}
	RETAILMSG(DBGLCD, (TEXT("S3C2450DISP::SetVisibleSurface\r\n")));
	LeaveCriticalSection(&m_CS);

#ifdef	HIGH_PRIORITY_INTR
	CeSetThreadPriority(hThread, iPriority);
#endif	
}

S3C2450Surf::S3C2450Surf(int width, int height, DWORD pa, VOID *pBits, int stride, 
			EGPEFormat format, EDDGPEPixelFormat pixelFormat, SurfaceHeap *pHeap) 
			: DDGPESurf(width, height, pBits, stride, format, pixelFormat)
{
	BOOL rc = FALSE;
	RETAILMSG(DBGLCD, (TEXT("nCount = %d\r\n"), ++nCount));
	m_pHeap = pHeap;
	m_nOffsetInVideoMemory = pa;
	m_fInVideoMemory = TRUE;//(pa != 0);
	RETAILMSG(DBGLCD,(TEXT("m_nOffsetInVideoMemory=0x%08X  m_fInVideoMemory=0x%08X\n "),m_nOffsetInVideoMemory,m_fInVideoMemory));
	m_hSurface = NULL;
	m_pSurface = NULL;
}

//-----------------------------------------------------------------------------

S3C2450Surf::~S3C2450Surf()
{	
	RETAILMSG(DBGLCD, (TEXT("+++++++++++++++++++S3C2450Surf::~S3C2450Surf  0x%08x\r\n"),m_nOffsetInVideoMemory));		
    // Delete surface mapping if exists
	if (m_pSurface != NULL) 
	{
		ADDRESS address = (ADDRESS)m_pSurface;
		address &= ~(PAGE_SIZE - 1);
		UnmapViewOfFile((VOID*)address);
	}

	if (m_hSurface != NULL) 
	CloseHandle(m_hSurface);

	// Free memory if it was allocated in SRAM...
	if (m_pHeap != NULL) 
	m_pHeap->Free();
	//RETAILMSG(1, (TEXT("-------------------S3C2450Surf::~S3C2450Surf  \r\n")));		
	RETAILMSG(DBGLCD, (TEXT("-------------------S3C2450Surf::~S3C2450Surf  nCount = %d\r\n"), --nCount));
}

#if	USE_PACSURF

/**
*	@class	PACSurf
*	@desc	This Surface will try to allocate physically linear address
*		
**/
/**
*	@fn		PACSurf::PACSurf
*	@brief	try to allocate memory region that is physically linear
*	@param	GPESurf **ppSurf, INT width, INT height, EGPEFormat format, int surfaceFlags
*	@sa		GPESurf
*	@note	This Surface format is compatible to GPESurf
**/
PACSurf::PACSurf(int width, int height, EGPEFormat format)
{
	RETAILMSG(DBGLCD, (_T("\n[DISPDRV] PACSurf Constructor(%d, %d, %d)"), width, height, format));	

    // Even though "width" and "height" are int's, they must be positive.
    ASSERT(width > 0);
    ASSERT(height > 0);

    memset( &m_Format, 0, sizeof ( m_Format ) );

    m_pVirtAddr            = NULL;
    m_nStrideBytes         = 0;
    m_eFormat              = gpeUndefined;
    m_fInVideoMemory       = 0;
    m_fOwnsBuffer          = 0;
    m_nWidth               = 0;
    m_nHeight              = 0;
    m_nOffsetInVideoMemory = 0;
    m_iRotate              = DMDO_0;
    m_ScreenWidth          = 0;
    m_ScreenHeight         = 0;
    m_BytesPixel           = 0;
    m_nHandle              = NULL;
    m_fPLAllocated		= 0;

    if (width > 0 && height > 0)
    {
		m_nWidth               = width;
		m_nHeight              = height;
		m_eFormat              = format;
		m_nStrideBytes         = ( (EGPEFormatToBpp[ format ] * width + 7 )/ 8 + 3 ) & ~3L;
		m_pVirtAddr  = (ADDRESS) AllocPhysMem(  m_nStrideBytes * height, PAGE_READWRITE, 0, 0,&m_pPhysAddr);
		if(m_pVirtAddr != NULL)
		{
	       	 m_fPLAllocated = 1;		
//	       	 m_fInVideoMemory = 1;
	       	 RETAILMSG(DBGLCD,(TEXT("\nPAC Surf PA Base : 0x%x VA Base : 0x%x STRIDE : %d"), m_pPhysAddr, m_pVirtAddr, m_nStrideBytes));
       	 }
		else
		{
			m_fPLAllocated = 0;
	       	 RETAILMSG(DBGLCD,(TEXT("\nPAC Surf PA Base : 0x%x VA Base : 0x%x STRIDE : %d  new unsigned char"), m_pPhysAddr, m_pVirtAddr, m_nStrideBytes));		
	       	 m_pVirtAddr = (ADDRESS) new unsigned char[ m_nStrideBytes * height ];
	        }
		m_fOwnsBuffer          = 1;
		m_BytesPixel           = EGPEFormatToBpp[m_eFormat] >> 3;
    }
}

PACSurf::~PACSurf()
{
    if( m_fOwnsBuffer )
    {
    	if(m_fPLAllocated)
    	{
    		if(m_pVirtAddr)
    		{
			if( !FreePhysMem((LPVOID)m_pVirtAddr) )
			{
				RETAILMSG(1,(TEXT("\nPACSurface deallocation is failed")));
			}
			else
			{
				RETAILMSG(DBGLCD,(TEXT("\nPACSurface deallocation is succeeded : 0x%x"), m_pVirtAddr));			
				m_pVirtAddr = NULL;
				m_fPLAllocated = 0;
				m_fOwnsBuffer = 0;
			}
    		}

    	}
    	else if( m_pVirtAddr )
        {
		RETAILMSG(TRUE,(TEXT("\nPACSurface dealloc is trying for non-contigious physical memory : 0x%x"),m_pVirtAddr));        
            delete [] (void *)m_pVirtAddr;
            m_pVirtAddr = NULL;
            m_fOwnsBuffer = 0;
        }
    }
}

#endif
	
//-----------------------------------------------------------------------------
//------------------------------------------------------------------------------
// 
//  Method:  WriteBack
//
//  Flush surface memory in cache.
//
VOID S3C2450Surf::WriteBack()
{
    	ASSERT(m_pSurface != NULL);

	RETAILMSG(DBGLCD, (TEXT("S3C2450Surf::WriteBack\n")));

    	if (m_pSurface != NULL) 
		CacheRangeFlush((VOID*)m_pSurface, m_nStrideBytes * m_nHeight, CACHE_SYNC_WRITEBACK);
}

//------------------------------------------------------------------------------
// 
//  Method:  Discard
//
//  Flush and invalidate surface memory in cache.
//
VOID S3C2450Surf::Discard()
{
    	ASSERT(m_pSurface != NULL);
		RETAILMSG(DBGLCD, (TEXT("S3C2450Surf::Discard\n")));
    	if (m_pSurface != NULL) 
		CacheRangeFlush((VOID*)m_pSurface, m_nStrideBytes * m_nHeight, CACHE_SYNC_DISCARD);
}


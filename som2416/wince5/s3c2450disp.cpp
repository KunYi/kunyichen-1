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

Module Name:	s3c2450disp.cpp

Abstract:	Main Display Driver Implementation. This driver can support directdraw(DDGPE)

Functions:	DrvEscape, ...

Notes:

--*/

#include "precomp.h"
#include <nkintr.h>
#include <gxinfo.h>
#define DISPPERF_DECLARE
#include "dispperf.h"
#include <dbgmsg_david.h>

INSTANTIATE_GPE_ZONES(0x3,"MGDI Driver","unused1","unused2")    // Start with errors and warnings

DDGPE * gGPE = (DDGPE *)NULL;

// This prototype avoids problems exporting from .lib
BOOL
APIENTRY
GPEEnableDriver(
    ULONG           engineVersion,
    ULONG           cj,
    DRVENABLEDATA * data,
    PENGCALLBACKS   engineCallbacks
    );

BOOL
APIENTRY
DrvEnableDriver(
    ULONG           engineVersion,
    ULONG           cj,
    DRVENABLEDATA * data,
    PENGCALLBACKS   engineCallbacks
    )
{
    return GPEEnableDriver(engineVersion, cj, data, engineCallbacks);
}

// this routine converts a string into a GUID and returns TRUE if the
// conversion was successful.
BOOL 
ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid)
{
	UINT Data4[8];
	int  Count;
	BOOL fOk = FALSE;
	TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

	DEBUGCHK(pGuid != NULL && pszGuid != NULL);
	__try
	{
		if (_stscanf(pszGuid, pszGuidFormat, &pGuid->Data1, 
		    &pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], 
		    &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
		{
			for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
			{
	        	        pGuid->Data4[Count] = (UCHAR) Data4[Count];
			}
		}
		fOk = TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return fOk;
}

// This routine notifies the OS that we support the Power Manager IOCTLs (through
// ExtEscape(), which calls DrvEscape()).
BOOL
AdvertisePowerInterface(HMODULE hInst)
{
	BOOL fOk = FALSE;
	HKEY hk;
	DWORD dwStatus;
	TCHAR szTemp[MAX_PATH];
	GUID gClass;

	// assume we are advertising the default class
	fOk = ConvertStringToGuid(PMCLASS_DISPLAY, &gClass);
	DEBUGCHK(fOk);

	// check for an override in the registry
	dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\GDI\\Drivers"), 0, 0, &hk);
	if(dwStatus == ERROR_SUCCESS)
	{
		DWORD dwType, dwSize;
		dwSize = sizeof(szTemp);
		dwStatus = RegQueryValueEx(hk, _T("DisplayPowerClass"), NULL, &dwType, (LPBYTE) szTemp, &dwSize);
		szTemp[MAX_PATH-1] = 0;
		if(dwStatus == ERROR_SUCCESS && dwType == REG_SZ)
		{
			// got a guid string, convert it to a guid
			GUID gTemp;
			fOk = ConvertStringToGuid(szTemp, &gTemp);
			DEBUGCHK(fOk);
			if(fOk)
			{
				gClass = gTemp;
			}
		}

		// release the registry key
		RegCloseKey(hk);
	}

	// figure out what device name to advertise
	if(fOk)
	{
		fOk = GetModuleFileName(hInst, szTemp, sizeof(szTemp) / sizeof(szTemp[0]));
		DEBUGCHK(fOk);
	}

	// now advertise the interface
	if(fOk)
	{
		fOk = AdvertiseInterface(&gClass, szTemp, TRUE);
		DEBUGCHK(fOk);
	}
    
	return fOk;
}

//
// Main entry point for a GPE-compliant driver
//

GPE *
GetGPE()
{
    if (!gGPE)
    {
        gGPE = new S3C2450DISP();
    }

    return gGPE;
}

ULONG gBitMasks[] = { 0xf800,0x07e0,0x001f};	//< This is for RGB565 Format Bitmask.

S3C2450DISP::S3C2450DISP()
{
    DWORD      oldMode;
    ULONG      fbSize;
    ULONG      fbOffset;
    ULONG      offsetX;
    ULONG      offsetY;

    m_InDDraw = FALSE;

    DEBUGMSG(GPE_ZONE_INIT,(TEXT("S3C2450DISP::S3C2450DISP\r\n")));
	 //[david.modify] 2008-09-04 14:22
	 DPNOK(m_InDDraw);


    m_pIntrReg = (S3C2450_INTR_REG *)VirtualAlloc(0, sizeof(S3C2450_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
    if (m_pIntrReg == NULL) 
    {
        return;
    }
    if (!VirtualCopy((PVOID)m_pIntrReg, (PVOID)(S3C2450_BASE_REG_PA_INTR >> 8), sizeof(S3C2450_INTR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
		return;
    }
    
    m_pLCDReg = (S3C2450_LCD_REG *)VirtualAlloc(0, sizeof(S3C2450_LCD_REG), MEM_RESERVE, PAGE_NOACCESS);
    if (m_pLCDReg == NULL) 
    {
        return;
    }
    if (!VirtualCopy((PVOID)m_pLCDReg, (PVOID)(S3C2450_BASE_REG_PA_LCD >> 8), sizeof(S3C2450_LCD_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) {
		return;
    }

	// regist LCD Interrupt
	m_dwVSYNCIrq = IRQ_LCD_VSYNC;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &m_dwVSYNCIrq, sizeof(DWORD), &m_dwVSYNCSysIntr, sizeof(DWORD), NULL))
    {
        m_dwVSYNCSysIntr = SYSINTR_UNDEFINED;
        return;
    } 
    m_hVSYNCInterruptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == m_hVSYNCInterruptEvent) 
    {
     	return;
    }
	if (!(InterruptInitialize(m_dwVSYNCSysIntr, m_hVSYNCInterruptEvent, 0, 0))) 
	{
		return;
	}
	m_pLCDReg->VIDINTCON &= ~((0x3)<<15 | (0x3)<<13 | (0x1) <<12 | (0x1) );	// FRAMESEL0, INTFRMEN Bit clear
	m_pLCDReg->VIDINTCON |= ((0x3)<<15 /*| (0x0)<<13 | (0x1)<<12*/ );	/// Video Frame Disalbe
    //m_pIntrReg->INTSUBMSK |= 1 << IRQ_SUB_LCD3;							// Disable Interrupt
    //m_pIntrReg->INTMSK |= 1 << IRQ_LCD;									// Disable Interrupt
    //DisableInterrupt();

	fpVisibleOverlay = 0;
	fpOverlayFlipFrom = 0;
	InitializeCriticalSection(&m_CS);
	InitializeCriticalSection(&m_cs2D);	

    oldMode = SetKMode(TRUE);



#if DISPLAY_24BIT_MODE
	m_RedMaskSize = 8;
	m_RedMaskPosition = 16;
	m_GreenMaskSize = 8;
	m_GreenMaskPosition = 8; 
	m_BlueMaskSize = 8;
	m_BlueMaskPosition = 0;
#else
	m_RedMaskSize = 5;
	m_RedMaskPosition = 11; 
	m_GreenMaskSize = 6; 
	m_GreenMaskPosition = 5;
	m_BlueMaskSize = 5; 
	m_BlueMaskPosition = 0;
#endif

	

	gBitMasks[0] =((1<<m_RedMaskSize) -1) << m_RedMaskPosition;  
	gBitMasks[1] =((1<<m_GreenMaskSize)-1) << m_GreenMaskPosition;
	gBitMasks[2] =((1<<m_BlueMaskSize) -1) << m_BlueMaskPosition;

	m_VesaMode = 0;
	m_nScreenWidth = LCD_XSIZE_TFT;
	m_nScreenHeight = LCD_YSIZE_TFT;
	
#if DISPLAY_24BIT_MODE 
	m_colorDepth = 32;	
#else
	m_colorDepth = 16;
#endif
	m_cxPhysicalScreen = LCD_XSIZE_TFT;
	m_cyPhysicalScreen = LCD_YSIZE_TFT;

	m_pvFlatFrameBuffer = IMAGE_FRAMEBUFFER_DMA_BASE;

#if DISPLAY_24BIT_MODE 
	m_cbScanLineLength = m_nScreenWidth * 4;
#else
	m_cbScanLineLength = m_nScreenWidth * 2;
#endif	

	
	m_FrameBufferSize = m_nScreenHeight * m_cbScanLineLength;
	m_VideoPowerState = VideoPowerOn;

	m_iRotate = GetRotateModeFromReg();
	SetRotateParams();

	SetKMode(oldMode);

	//< Initialize Display Mode	
	InitializeDisplayMode();

	// compute frame buffer displayable area offset
	offsetX = (m_cxPhysicalScreen - m_nScreenWidthSave) / 2;
	offsetY = (m_cyPhysicalScreen - m_nScreenHeightSave) / 2;
	fbOffset = (offsetY * m_cbScanLineLength) + offsetX;

	// compute physical frame buffer size
	fbSize = m_cyPhysicalScreen * m_cbScanLineLength;

	// for DDraw enabled, make sure we also have some off-screen video memory available for surface allocations
	fbSize = IMAGE_FRAMEBUFFER_SIZE;

	// Use CreateFileMapping/MapViewOfFile to guarantee the VirtualFrameBuffer
	// pointer is allocated in shared memory.

	m_hVFBMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, fbSize, NULL);

	if (m_hVFBMapping != NULL) 
	{
		m_VirtualFrameBuffer = (DWORD)MapViewOfFile(m_hVFBMapping, FILE_MAP_WRITE, 0, 0, 0);
	}
	else
	{
		m_VirtualFrameBuffer = NULL;
	}

//////    if (m_VesaMode != 0)
//////    {
        if (VirtualCopy((void *)m_VirtualFrameBuffer, (void *)(m_pvFlatFrameBuffer >> 8), fbSize, PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
        {
//            CeSetMemoryAttributes ((void *)m_VirtualFrameBuffer, (void *)(m_pvFlatFrameBuffer >> 8), fbSize, PAGE_WRITECOMBINE);
		if (VirtualSetAttributes((void *)m_VirtualFrameBuffer, fbSize, 0x4, 0xc, NULL) != TRUE)
		{
			RETAILMSG (1, (_T("Couldn't change framebuffer's page attributes as NCB.")));
			RETAILMSG (1, (_T("VirtualSetAttributes failed.")));
		}
        }
//////    }
//////    else
//////    {
//////        if (VirtualCopy((void *)m_VirtualFrameBuffer, (void *)m_pvFlatFrameBuffer, fbSize, PAGE_READWRITE | PAGE_NOCACHE)) {
//////            // since we don't know if the physical address under is contiguious or not, we have to settle with
//////            // PHYSICAL_ADDRESS_UNKNOWN and hope that the CPU supports it.
//////            CeSetMemoryAttributes ((void *)m_VirtualFrameBuffer, PHYSICAL_ADDRESS_UNKNOWN, fbSize, PAGE_WRITECOMBINE);
//////        }
//////    }

    PREFAST_ASSERT(m_VirtualFrameBuffer);

//////    {
//////        DWORD index;
//////        volatile PUSHORT ptr = (PUSHORT)m_VirtualFrameBuffer;
//////
//////		for (index = 0; index < 320*240; index++)
//////		{
//////			if(index < 240*80)
//////			{
//////				ptr[index] = 0xf800;
//////			}
//////			else if(index < 240*160)
//////			{
//////				ptr[index] = 0x07e0;
//////			}
//////			else if(index < 240*240)
//////			{
//////				ptr[index] = 0x001f;
//////			}
//////			else
//////			{
//////				ptr[index] = 0xffff;
//////			}
//////		}
//////
//////    }

	 //[david.modify] 2008-09-04 14:22
	 
	DPSTR("SCREEN CLEAR+");
#if 0
	DPSTR("5000");
	Sleep(5000);
#endif	

 //[david.modify] 2008-09-04 15:33
 // 拿掉此行，去解屏幕突然黑一下的问题
//	memset ((void*)m_VirtualFrameBuffer, 0x0, fbSize);	//< Screen Clear

	m_VirtualFrameBuffer += fbOffset;
	fbSize -= fbOffset;

#if 0
	DPSTR("5000");
	Sleep(5000);
#endif	

	DPSTR("SCREEN CLEAR-");
	m_pVideoMemoryHeap = new SurfaceHeap(fbSize, m_VirtualFrameBuffer, NULL, NULL);
	if(!m_pVideoMemoryHeap)
	{
		RETAILMSG (1, (L"Failed to create surface heap on internal SRAM memory\n"));
		return;
	}

	m_CursorVisible = FALSE;
	m_CursorDisabled = TRUE;
	m_CursorForcedOff = FALSE;
	memset (&m_CursorRect, 0x0, sizeof(m_CursorRect));
	
#if G2D_ACCELERATE
	// Enable Block Power and Clock Source
//	DevHWPowerGating(HWPWR_2D_ON);
//	DevHWClockGating(HWCLK_2D_ON);
	BOOL	bResult;
	m_oG2D = new FIMGSE2D;
	if(m_oG2D == NULL)
	{
		RETAILMSG(1, (TEXT("--S3C2450DISP() 2D Accelerator Initialization Fail\r\n")));
	}
	else
	{
		m_oG2D->Init();
		RETAILMSG(1, (TEXT("--S3C2450DISP() 2D Accelerator Initialization Succeed\r\n")));	
	}
	if(m_oG2D)
	{
		bResult = m_oG2D->InitializeInterrupt();
		if(bResult==FALSE)
		{
			RETAILMSG(1, (TEXT("--S3C2450DISP() 2D Acclerator Interrupt Initialization Failed.\r\n")));
		}
		else
		{
			RETAILMSG(1, (TEXT("--S3C2450DISP() 2D Acclerator Interrupt Initialization Succeed.\r\n")));		
		}
	}
	else
	{
		RETAILMSG(1, (TEXT("--S3C2450DISP() 2D Acclerator Object was not created.\r\n")));	
	}
	
#endif	
    
	AdvertisePowerInterface(g_hmodDisplayDll);    

	DPNOK(0);

}

S3C2450DISP::~S3C2450DISP()
{
	if (m_VirtualFrameBuffer != NULL)
	{
		UnmapViewOfFile((LPVOID)m_VirtualFrameBuffer);
	}
	if (m_hVFBMapping != NULL)
	{
		CloseHandle(m_hVFBMapping);
	}
	if (m_pLCDReg)
	{
		VirtualFree((PVOID)m_pLCDReg, 0, MEM_RELEASE);
		m_pLCDReg = NULL;
	}
	if (m_pIntrReg)
	{
		VirtualFree((PVOID)m_pIntrReg, 0, MEM_RELEASE);
		m_pIntrReg = NULL;
	}    
#if G2D_ACCELERATE
	if (m_oG2D)
	{
		m_oG2D->DeinitInterrupt();
		delete m_oG2D;
	}

	// Disable Block Power and Clock Source
	//	DevHWClockGating(HWCLK_2D_OFF);
	//	DevHWPowerGating(HWPWR_2D_OFF);
#endif		
}



void    S3C2450DISP::CursorOn (void)
{
    UCHAR    *ptrScreen = (UCHAR*)m_pPrimarySurface->Buffer();
    UCHAR    *ptrLine;
    UCHAR    *cbsLine;
    int        x, y;

    if (!m_CursorForcedOff && !m_CursorDisabled && !m_CursorVisible)
    {
        RECTL cursorRectSave = m_CursorRect;
        int   iRotate;
        RotateRectl(&m_CursorRect);
        for (y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
        {
            if (y < 0)
            {
                continue;
            }
            if (y >= m_nScreenHeightSave)
            {
                break;
            }

            ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride()];
            cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * (m_CursorSize.x * (m_colorDepth >> 3))];

            for (x = m_CursorRect.left; x < m_CursorRect.right; x++)
            {
                if (x < 0)
                {
                    continue;
                }
                if (x >= m_nScreenWidthSave)
                {
                    break;
                }

                // x' = x - m_CursorRect.left; y' = y - m_CursorRect.top;
                // Width = m_CursorSize.x;   Height = m_CursorSize.y;
                switch (m_iRotate)
                {
                    case DMDO_0:
                        iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
                        break;
                    case DMDO_90:
                        iRotate = (x - m_CursorRect.left)*m_CursorSize.x + m_CursorSize.y - 1 - (y - m_CursorRect.top);   
                        break;
                    case DMDO_180:
                        iRotate = (m_CursorSize.y - 1 - (y - m_CursorRect.top))*m_CursorSize.x + m_CursorSize.x - 1 - (x - m_CursorRect.left);
                        break;
                    case DMDO_270:
                        iRotate = (m_CursorSize.x -1 - (x - m_CursorRect.left))*m_CursorSize.x + y - m_CursorRect.top;
                        break;
                    default:
                        iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
                        break;
                }
                cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3)] = ptrLine[x * (m_colorDepth >> 3)];
                ptrLine[x * (m_colorDepth >> 3)] &= m_CursorAndShape[iRotate];
                ptrLine[x * (m_colorDepth >> 3)] ^= m_CursorXorShape[iRotate];
                if (m_colorDepth > 8)
                {
                    cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 1] = ptrLine[x * (m_colorDepth >> 3) + 1];
                    ptrLine[x * (m_colorDepth >> 3) + 1] &= m_CursorAndShape[iRotate];
                    ptrLine[x * (m_colorDepth >> 3) + 1] ^= m_CursorXorShape[iRotate];
                    if (m_colorDepth > 16)
                    {
                        cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 2] = ptrLine[x * (m_colorDepth >> 3) + 2];
                        ptrLine[x * (m_colorDepth >> 3) + 2] &= m_CursorAndShape[iRotate];
                        ptrLine[x * (m_colorDepth >> 3) + 2] ^= m_CursorXorShape[iRotate];
                    }
                }
            }
        }
        m_CursorRect = cursorRectSave;
        m_CursorVisible = TRUE;
    }
}

void    S3C2450DISP::CursorOff (void)
{
	UCHAR	*ptrScreen = (UCHAR*)m_pPrimarySurface->Buffer();
	UCHAR	*ptrLine;
	UCHAR	*cbsLine;
	int		x, y;

	if (!m_CursorForcedOff && !m_CursorDisabled && m_CursorVisible)
	{
		RECTL rSave = m_CursorRect;
		RotateRectl(&m_CursorRect);
		for (y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
		{
			// clip to displayable screen area (top/bottom)
			if (y < 0)
			{
				continue;
			}
			if (y >= m_nScreenHeightSave)
			{
				break;
			}

			ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride()];
			cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * (m_CursorSize.x * (m_colorDepth >> 3))];

			for (x = m_CursorRect.left; x < m_CursorRect.right; x++)
			{
				// clip to displayable screen area (left/right)
				if (x < 0)
				{
					continue;
				}
				if (x >= m_nScreenWidthSave)
				{
					break;
				}

				ptrLine[x * (m_colorDepth >> 3)] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3)];
				if (m_colorDepth > 8)
				{
					ptrLine[x * (m_colorDepth >> 3) + 1] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 1];
					if (m_colorDepth > 16)
					{
						ptrLine[x * (m_colorDepth >> 3) + 2] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 2];
					}
				}
			}
		}
		m_CursorRect = rSave;
		m_CursorVisible = FALSE;
	}
}

SCODE    S3C2450DISP::SetPointerShape(GPESurf *pMask, GPESurf *pColorSurf, INT xHot, INT yHot, INT cX, INT cY)
{
	UCHAR	*andPtr;		// input pointer
	UCHAR	*xorPtr;		// input pointer
	UCHAR	*andLine;		// output pointer
	UCHAR	*xorLine;		// output pointer
	char	bAnd;
	char	bXor;
	int		row;
	int		col;
	int		i;
	int		bitMask;

	DEBUGMSG(GPE_ZONE_CURSOR,(TEXT("S3C2450DISP::SetPointerShape(0x%X, 0x%X, %d, %d, %d, %d)\r\n"),pMask, pColorSurf, xHot, yHot, cX, cY));

	// turn current cursor off
	CursorOff();

	// release memory associated with old cursor
	if (!pMask)							// do we have a new cursor shape
	{
		m_CursorDisabled = TRUE;		// no, so tag as disabled
	}
	else
	{
		m_CursorDisabled = FALSE;		// yes, so tag as not disabled

		// store size and hotspot for new cursor
		m_CursorSize.x = cX;
		m_CursorSize.y = cY;
		m_CursorHotspot.x = xHot;
		m_CursorHotspot.y = yHot;

		andPtr = (UCHAR*)pMask->Buffer();
		xorPtr = (UCHAR*)pMask->Buffer() + (cY * pMask->Stride());

		// store OR and AND mask for new cursor
		for (row = 0; row < cY; row++)
		{
			andLine = &m_CursorAndShape[cX * row];
			xorLine = &m_CursorXorShape[cX * row];

			for (col = 0; col < cX / 8; col++)
			{
				bAnd = andPtr[row * pMask->Stride() + col];
				bXor = xorPtr[row * pMask->Stride() + col];

				for (bitMask = 0x0080, i = 0; i < 8; bitMask >>= 1, i++)
				{
					andLine[(col * 8) + i] = bAnd & bitMask ? 0xFF : 0x00;
					xorLine[(col * 8) + i] = bXor & bitMask ? 0xFF : 0x00;
				}
			}
		}
	}

    return    S_OK;
}

SCODE    S3C2450DISP::MovePointer(INT xPosition, INT yPosition)
{
    DEBUGMSG(GPE_ZONE_CURSOR, (TEXT("S3C2450DISP::MovePointer(%d, %d)\r\n"), xPosition, yPosition));

    CursorOff();

    if (xPosition != -1 || yPosition != -1)
    {
        // compute new cursor rect
        m_CursorRect.left = xPosition - m_CursorHotspot.x;
        m_CursorRect.right = m_CursorRect.left + m_CursorSize.x;
        m_CursorRect.top = yPosition - m_CursorHotspot.y;
        m_CursorRect.bottom = m_CursorRect.top + m_CursorSize.y;

        CursorOn();
    }

    return    S_OK;
}

void    S3C2450DISP::WaitForNotBusy(void)
{
	DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::WaitForNotBusy\r\n")));
#if G2D_ACCELERATE
	m_oG2D->WaitForIdle();	//< Wait for Fully Empty Command Fifo for all BitBlt request
#endif
    return;
}

int        S3C2450DISP::IsBusy(void)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::IsBusy\r\n")));
    return    0;
}

void    S3C2450DISP::GetPhysicalVideoMemory(unsigned long *physicalMemoryBase, unsigned long *videoMemorySize)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::GetPhysicalVideoMemory\r\n")));
    RETAILMSG (1, (TEXT("S3C2450DISP::GetPhysicalVideoMemory\r\n")));    

    *physicalMemoryBase = m_pvFlatFrameBuffer;
    *videoMemorySize = m_cbScanLineLength * m_cyPhysicalScreen;
}

void
S3C2450DISP::GetVirtualVideoMemory(
    unsigned long *virtualMemoryBase,
    unsigned long *videoMemorySize
    )
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::GetVirtualVideoMemory\r\n")));
    RETAILMSG (1, (TEXT("S3C2450DISP::GetVirtualVideoMemory\r\n")));    

    *virtualMemoryBase = m_VirtualFrameBuffer;
//    *videoMemorySize = m_cbScanLineLength * m_cyPhysicalScreen;
    *videoMemorySize = 0x100000;
}

INT        S3C2450DISP::InVBlank(void)
{
    static    BOOL    value = FALSE;
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::InVBlank\r\n")));
    value = !value;
    return value;
}

SCODE    S3C2450DISP::SetPalette(const PALETTEENTRY *source, USHORT firstEntry, USHORT numEntries)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("S3C2450DISP::SetPalette\r\n")));

    if (firstEntry < 0 || firstEntry + numEntries > 256 || source == NULL)
    {
        return    E_INVALIDARG;
    }


    return    S_OK;
}

VIDEO_POWER_STATE
PmToVideoPowerState(CEDEVICE_POWER_STATE pmDx)
{
	VIDEO_POWER_STATE vps;
	RETAILMSG(0,(_T("++PmToVideoPowerState\r\n")));

	switch(pmDx) {
	case D0:        // turn the display on
		vps = VideoPowerOn;
		break;

	case D1:        // if asked for a state we don't support, go to the next lower one
	case D2:
	case D3:
	case D4:
		vps = VideoPowerOff;
		break;

	default:
		RETAILMSG(0 , (L"PmToVideoPowerState: mapping unknown PM state %d to VideoPowerOn\r\n", pmDx));
		vps = VideoPowerOn;
		break;
	}

	return vps;
}

// this routine maps video power states to PM power states.
CEDEVICE_POWER_STATE
VideoToPmPowerState(VIDEO_POWER_STATE vps)
{
	CEDEVICE_POWER_STATE pmDx;
	RETAILMSG(0,(_T("++VideoToPmPowerState\r\n")));

	switch(vps)
	{
	case VideoPowerOn:
		pmDx = D0;
		break;

	case VideoPowerStandBy:
		pmDx = D1;
		break;

	case VideoPowerSuspend:
		pmDx = (CEDEVICE_POWER_STATE)D2;
		break;

	case VideoPowerOff:
		pmDx = (CEDEVICE_POWER_STATE)D4;
		break;

	default:
		pmDx = D0;
		RETAILMSG(0, (L"VideoToPmPowerState: mapping unknown video state %d to pm state %d\r\n",
		         vps, pmDx));
		break;
	}

	return pmDx;
}

#define ESC_SUCCESS             0x00000001
#define ESC_FAILED              0xFFFFFFFF
#define ESC_NOT_SUPPORTED       0x00000000
ULONG
S3C2450DISP::DrvEscape(
    SURFOBJ * pso,
    ULONG     iEsc,
    ULONG     cjIn,
    void    * pvIn,
    ULONG     cjOut,
    void    * pvOut
    )
{
	ULONG Result = 0;	
	if (iEsc == QUERYESCSUPPORT)
	{
		if (*(DWORD*)pvIn == GETGXINFO
			|| *(DWORD*)pvIn == DRVESC_GETSCREENROTATION
			|| *(DWORD*)pvIn == DRVESC_SETSCREENROTATION
			|| *(DWORD*)pvIn == SETPOWERMANAGEMENT
			|| *(DWORD*)pvIn == GETPOWERMANAGEMENT
			|| *(DWORD*)pvIn == IOCTL_POWER_CAPABILITIES
			|| *(DWORD*)pvIn == IOCTL_POWER_QUERY
			|| *(DWORD*)pvIn == IOCTL_POWER_SET
			|| *(DWORD*)pvIn == IOCTL_POWER_GET            
			)
		{
			// The escape is supported.
			return 1;
		}
		else
		{
			// The escape isn't supported.
#if DO_DISPPERF
			return DispPerfQueryEsc(*(DWORD*)pvIn);;
#else
			return 0;
#endif
		}

	}
    else if (iEsc == DRVESC_GETSCREENROTATION)
    {
        *(int *)pvOut = ((DMDO_0 | DMDO_90 | DMDO_180 | DMDO_270) << 8) | ((BYTE)m_iRotate);
        return DISP_CHANGE_SUCCESSFUL;
    }
    else if (iEsc == DRVESC_SETSCREENROTATION)
    {
        if ((cjIn == DMDO_0)   ||
            (cjIn == DMDO_90)  ||
            (cjIn == DMDO_180) ||
            (cjIn == DMDO_270) )
            {
                return DynRotate(cjIn);
            }

        return DISP_CHANGE_BADMODE;
    }
    else if (iEsc == GETGXINFO)
    {
        return GetGameXInfo(iEsc, cjIn, pvIn, cjOut, pvOut);
    }
    else if (iEsc == SETPOWERMANAGEMENT)
    {
        if ((cjIn >= sizeof (VIDEO_POWER_MANAGEMENT)) && (pvIn != NULL))
        {
            PVIDEO_POWER_MANAGEMENT pvpm = (PVIDEO_POWER_MANAGEMENT)pvIn;
			
            if (pvpm->Length >= sizeof (VIDEO_POWER_MANAGEMENT))
            {
                switch (pvpm->PowerState)
                {
				case VideoPowerStandBy:
				case VideoPowerOn:
					SetDisplayPower(VideoPowerOn);
					Result = ESC_SUCCESS;
                    break;
					
				case VideoPowerOff:
				case VideoPowerSuspend:
					SetDisplayPower(VideoPowerOff);
					Result = ESC_SUCCESS;
                    break;
                }
            }
        }
		
        if (Result != ESC_SUCCESS)
        {
            // Shouldn't get here if everything was ok.
            SetLastError(ERROR_INVALID_PARAMETER);
            Result = ESC_FAILED;
        }
		return Result;
	}
	else if (iEsc == GETPOWERMANAGEMENT)
	{
		RETAILMSG(1, (L"GETPOWERMANAGEMENT\n"));
        if ((cjOut >= sizeof (VIDEO_POWER_MANAGEMENT)) && (pvOut != NULL))
        {
            PVIDEO_POWER_MANAGEMENT pvpm = (PVIDEO_POWER_MANAGEMENT)pvOut;
			
            pvpm->Length = sizeof (VIDEO_POWER_MANAGEMENT);
            pvpm->DPMSVersion = 0;
			
            pvpm->PowerState = m_VideoPowerState;
			
            Result = ESC_SUCCESS;
        }
        else
        {
            // Shouldn't get here if everything was ok.
            SetLastError(ERROR_INVALID_PARAMETER);
            Result = ESC_FAILED;
        }
		return Result;
    }
    else if (iEsc == IOCTL_POWER_CAPABILITIES)
    {
        // tell the power manager about ourselves
 //       RETAILMSG(0, (L"%s: IOCTL_POWER_CAPABILITIES\r\n", pszFname));
        if (pvOut != NULL && cjOut == sizeof(POWER_CAPABILITIES))
        {
            __try
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pvOut;
                memset(ppc, 0, sizeof(*ppc));
                ppc->DeviceDx = 0x11;	// support D0 and D4
                Result = ESC_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                RETAILMSG(0, (L"%s: exception in ioctl1\r\n"));
            }
        }
        return Result;
    }
	else if(iEsc == IOCTL_POWER_QUERY)
	{
        if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
        {
            // return a good status on any valid query, since we are always ready to
            // change power states.
            __try
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE) pvOut;
                if(VALID_DX(NewDx))
                {
                    // this is a valid Dx state so return a good status
                    Result = ESC_SUCCESS;
                }
//                RETAILMSG(0, (L"%s: IOCTL_POWER_QUERY %u %s\r\n", pszFname, 
//					NewDx, Result == ESC_SUCCESS ? L"succeeded" : L"failed"));
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                RETAILMSG(0, (L"%s: exception in ioctl2\r\n"));
            }
        }
        return Result;
	}
	else if(iEsc == IOCTL_POWER_SET)	
	{		
        if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
        {
            __try
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE) pvOut;
                CEDEVICE_POWER_STATE CurrentDx;
                if(VALID_DX(NewDx))
                {
                    VIDEO_POWER_STATE ulPowerState = PmToVideoPowerState(NewDx);

                    SetDisplayPower(ulPowerState);

                    CurrentDx = VideoToPmPowerState((VIDEO_POWER_STATE)m_VideoPowerState);

                    Result = ESC_SUCCESS;
 //                   RETAILMSG(0, (L"%s: IOCTL_POWER_SET %u: passing back %u\r\n", pszFname,
//						NewDx, CurrentDx));
                }
                else
                {
//                    RETAILMSG(0, 
//						(L"%s: IOCTL_POWER_SET: invalid state request %u\r\n", pszFname, NewDx));
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                RETAILMSG(0, (L"%s: exception in ioctl3\r\n"));
            }
        }
        return Result;
	}
	else if(iEsc == IOCTL_POWER_GET)
	{
        if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
        {
            __try
            {
                CEDEVICE_POWER_STATE CurrentDx = D0;//VideoToPmPowerState((VIDEO_POWER_STATE)m_VideoPowerState);
                *(PCEDEVICE_POWER_STATE) pvOut = D0; //CurrentDx;
                Result = ESC_SUCCESS;
//                RETAILMSG(0, (L"%s: IOCTL_POWER_GET: passing back %u\r\n", pszFname, 
//					CurrentDx));
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                RETAILMSG(0, (L"%s: exception in ioctl4\r\n"));
            }
        }
        return Result;
	}
#if DO_DISPPERF	
	else {
		return DispPerfDrvEscape(iEsc, cjIn, pvIn, cjOut,pvOut);
	}
#else
	return 0;
#endif
}

#undef ESC_NOT_SUPPORTED
#undef ESC_FAILED
#undef ESC_SUCCESS

void S3C2450DISP::SetDisplayPower(ULONG PowerState)
{
	static BYTE * pVideoMemory = NULL;
//	WORD *ptr;
	RETAILMSG(0,(_T("++SetDisplayPower\r\n")));

    // If we're already in the appropriate state, just return 
    if (m_VideoPowerState == PowerState)
    {
        return;
    }

    if (PowerState == VideoPowerOff)
    {
    	RETAILMSG(0,(TEXT("VideoPowerOff\n")));

 //[david.modify] 2008-06-19 15:40
 //==========================
#if 0 
/* Save video memory*/
			if (NULL == pVideoMemory)
			{
				pVideoMemory = new BYTE [m_FrameBufferSize];
			}
			
			if (NULL != pVideoMemory)
			{
				memcpy(pVideoMemory, m_pPrimarySurface->Buffer(), m_FrameBufferSize);
			}
			
			/*Blank the screen*/
			memset ((void*)m_pPrimarySurface->Buffer(), 0xFF, m_FrameBufferSize);
#endif			
 //==========================		
        m_VideoPowerState = VideoPowerOff;
		m_pLCDReg->WIN0MAP |= 1<<24;
		m_pLCDReg->WIN1MAP |= 1<<24;
		Sleep(100);

    }
    else
    {
		RETAILMSG(0,(TEXT("VideoPowerOn\n")));

 //[david.modify] 2008-06-19 15:40
 //==========================		
#if 0 
// Restore the screen		
			if (NULL != pVideoMemory)
			{
				#if 0 /*[yeob-]*/
				// Swap the buffers.
				BYTE * pTempBuffer = (BYTE*)m_pPrimarySurface->Buffer();

				RETAILMSG(DBGJUNINJX,(_T("[DISPLAY] ++SetDisplayPower pTempBuffer(%x) m_pPrimarySurface->Buffer()(%x) \r\n"),pTempBuffer,m_pPrimarySurface->Buffer()));
				m_pPrimarySurface->Init(m_nScreenWidth, m_nScreenHeight, pVideoMemory, m_cbScanLineLength, m_pMode->format);
				RETAILMSG(DBGJUNINJX,(_T("[DISPLAY] ++SetDisplayPower m_nScreenWidth(%x) m_nScreenHeight(%x) \r\n"),m_nScreenWidth,m_nScreenHeight));
				RETAILMSG(DBGJUNINJX,(_T("[DISPLAY] ++SetDisplayPower pVideoMemory(%x) m_cbScanLineLength(%x) \r\n"),pVideoMemory,m_cbScanLineLength));

				pVideoMemory = pTempBuffer;

				RETAILMSG(DBGJUNINJX,(_T("[DISPLAY] ++SetDisplayPower pVideoMemory(%x) m_FrameBufferSize(%x) \r\n"),pVideoMemory,m_FrameBufferSize));
				#endif
				// Actually copy the bits.
				memcpy(m_pPrimarySurface->Buffer(), pVideoMemory, m_FrameBufferSize);

				delete [] pVideoMemory;
				pVideoMemory = NULL;

//				RETAILMSG(DBGJUNINJX,(_T("[DISPLAY] ++SetDisplayPower ..(3)\r\n")));
			}
			else
			{
				memset ((void*)m_pPrimarySurface->Buffer(), 0xFF, m_FrameBufferSize);
//				RETAILMSG(DBGJUNINJX,(_T("[DISPLAY] ++SetDisplayPower ..(4)\r\n")));
			}
#endif			
 //==========================		
			
        m_VideoPowerState = VideoPowerOn;
		m_pLCDReg->WIN0MAP &= ~(1<<24);
		m_pLCDReg->WIN1MAP &= ~(1<<24);
    }
}

int
S3C2450DISP::GetRotateModeFromReg()
{
    HKEY hKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\GDI\\ROTATION"), 0, 0, &hKey))
    {
        DWORD dwSize, dwAngle, dwType = REG_DWORD;
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                             TEXT("ANGLE"),
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&dwAngle,
                                             &dwSize))
        {
            switch (dwAngle)
            {
            case 0:
                return DMDO_0;

            case 90:
                return DMDO_90;

            case 180:
                return DMDO_180;

            case 270:
                return DMDO_270;

            default:
                return DMDO_0;
            }
        }

        RegCloseKey(hKey);
    }

    return DMDO_0;
}

void
S3C2450DISP::SetRotateParams()
{
    int iswap;

    switch(m_iRotate)
    {
    case DMDO_0:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave  = m_nScreenWidth;
        break;

    case DMDO_180:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave  = m_nScreenWidth;
        break;

    case DMDO_90:
    case DMDO_270:
        iswap               = m_nScreenHeight;
        m_nScreenHeight     = m_nScreenWidth;
        m_nScreenWidth      = iswap;
        m_nScreenHeightSave = m_nScreenWidth;
        m_nScreenWidthSave  = m_nScreenHeight;
        break;

    default:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave  = m_nScreenWidth;
        break;
    }

    return;
}


LONG
S3C2450DISP::DynRotate(
    int angle
    )
{
    GPESurfRotate * pSurf = (GPESurfRotate *)m_pPrimarySurface;
	
    // DirectDraw and rotation can't co-exist.
    if (m_InDDraw)
    {
        return DISP_CHANGE_BADMODE;
    }
/*
    if (angle == m_iRotate)
    {
        return DISP_CHANGE_SUCCESSFUL;
    }*/

    CursorOff();

    m_iRotate = angle;

    switch(m_iRotate)
    {
    case DMDO_0:
    case DMDO_180:
        m_nScreenHeight = m_nScreenHeightSave;
        m_nScreenWidth  = m_nScreenWidthSave;
        break;

    case DMDO_90:
    case DMDO_270:
        m_nScreenHeight = m_nScreenWidthSave;
        m_nScreenWidth  = m_nScreenHeightSave;
        break;
    }

    m_pMode->width  = m_nScreenWidth;
    m_pMode->height = m_nScreenHeight;

    pSurf->SetRotation(m_nScreenWidth, m_nScreenHeight, angle);

    CursorOn();

    return DISP_CHANGE_SUCCESSFUL;
}

void S3C2450DISP::EnableInterrupt()
{
	m_pLCDReg->VIDINTCON |= (0x1<<12)|(0x1<<0);	/// Video Frame Disalbe
	/*
    m_pIntrReg->INTSUBMSK &= ~(1 << IRQ_SUB_LCD3);							// Disable Interrupt
    m_pIntrReg->INTMSK &= ~(1 << IRQ_LCD);									// Disable Interrupt
	*/
}

void S3C2450DISP::DisableInterrupt()
{
	m_pLCDReg->VIDINTCON &= ~((0x1<<12)|(0x1<<0));	/// Video Frame Disalbe
	/*
    m_pIntrReg->INTSUBMSK |= 1 << IRQ_SUB_LCD3;							// Disable Interrupt
    m_pIntrReg->INTMSK |= 1 << IRQ_LCD;									// Disable Interrupt	
    */
}


ULONG *
APIENTRY
DrvGetMasks(
    DHPDEV dhpdev
    )
{
    return gBitMasks;
}




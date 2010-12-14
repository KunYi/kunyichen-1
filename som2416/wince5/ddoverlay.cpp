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

Module Name:	ddoverlay.cpp

Abstract:	DirectDraw Overlay Support Implementation.

Functions:	Init/Enable/Disable Overlay

Notes:	In S3C2443 and S3C2450 support only 1 overlay + background surface

--*/

#include "precomp.h"

void S3C2450DISP::EnableOverlay()
{
	RETAILMSG(0,(TEXT("Eanble Overlay !!!\n") ));	
	m_pLCDReg->WINCON1 |= (1<<0);	

}

void S3C2450DISP::DisableOverlay()
{
	RETAILMSG(0,(TEXT("DisableOverlay\n")));
	m_pLCDReg->WINCON1 &= ~(1<<0);
}

void S3C2450DISP::InitOverlay(S3C2450Surf* pOverlaySurface, RECTL rcSrc)
{
	m_pLCDReg->WINCON1 = (1<<WINCON_SWAP_S)|(WINCONx_16WORD_BURST<<WINCON_BURSTLEN_S)|(WINCONx_16BPP_565<<WINCON_BPP_S); // 4word burst, 16bpp,  
	
	m_pLCDReg->VIDW01ADD0 = (UINT32)(pOverlaySurface->OffsetInVideoMemory() + IMAGE_FRAMEBUFFER_DMA_BASE);		
			// buffer end address
	m_pLCDReg->VIDW01ADD1 = (UINT32)(pOverlaySurface->OffsetInVideoMemory() + IMAGE_FRAMEBUFFER_DMA_BASE) + 
													(pOverlaySurface->Width()*pOverlaySurface->Height()*2);
			// buffer size 
	m_pLCDReg->VIDW01ADD2 = (0<<VIDWxADD2_OFFSET_SIZE_S)|(pOverlaySurface->Width()*2);
	
	m_pLCDReg->VIDOSD1C = (0xF << 20) | (0xF << 16) | (0xF << 12) | (0x0 << 8) | (0x0 << 4) | (0x0 << 0);
	
	
	
	
}

extern "C"
{
void SetOverlayPosReg(PVOID pAddress, UINT32 lt, UINT32 rb);
}

void	S3C2450DISP::SetOverlayPosition(UINT32 x,UINT32 y,UINT32 width,UINT32 height)
{
	
#ifdef	HIGH_PRIORITY_INTR
	int iPriority;
	HANDLE hThread;

	hThread = GetCurrentThread();
	iPriority = CeGetThreadPriority(hThread);
	CeSetThreadPriority(hThread, DISPDRV_IST_PRIORITY);
#endif	

    DWORD we;	
	UINT32 lefttop;
	UINT32 rightbottom;

	
	EnterCriticalSection(&m_CS);	
	lefttop = ((x)<<11) | ((y)<<0);
	rightbottom = ((x+width-1)<<11) | ((y+height-1)<<0);	
	EnableInterrupt();	
	we = WaitForSingleObject(m_hVSYNCInterruptEvent,1000/*INFINITE*/);
	DisableInterrupt();
	InterruptDone(m_dwVSYNCSysIntr);

	if(we != WAIT_OBJECT_0)
	{
		RETAILMSG(1,(TEXT("SetOverlayPosition Time Out !!!\n") ));
	}	
	SetOverlayPosReg((PVOID)(&m_pLCDReg->VIDOSD1A), lefttop, rightbottom);
	LeaveCriticalSection(&m_CS);	

#ifdef	HIGH_PRIORITY_INTR
	CeSetThreadPriority(hThread, iPriority);
#endif		
}

void	S3C2450DISP::SetOverlayColorKey(BOOL bUseColorKey, DWORD colorKey)
{
	BYTE R,G,B;
	R = (BYTE)((colorKey & m_pModeEx->dwRBitMask) >> m_RedMaskPosition);
	G = (BYTE)((colorKey & m_pModeEx->dwGBitMask) >> m_GreenMaskPosition);
	B = (BYTE)((colorKey & m_pModeEx->dwBBitMask) >> m_BlueMaskPosition);
	//RETAILMSG(1,(TEXT("R=%d G=%d B=%d\n"),R,G,B));
	if(bUseColorKey)
	{
		m_pLCDReg->W1KEYCON0 |= (0x1<<25) | (0x0<<24) | (0x7<<16) | (0x3<<8) | (0x7<<0);
		m_pLCDReg->W1KEYCON1 = (R<<19) | (G<<10) | (B<<3);
		m_bIsOverlayColorKey = TRUE;
	}
	else
	{
		m_pLCDReg->W1KEYCON0 &= ~(0x1<<25);
		m_bIsOverlayColorKey = FALSE;
	}
}

void	S3C2450DISP::SetOverlayAlpha(BOOL bUseAlpha, BOOL bUsePixelBlend, DWORD color)
{
	BYTE R,G,B;
	if(bUseAlpha)
	{
		R = (BYTE)((color & 0xF00) >> 8);
		G = (BYTE)((color & 0x0F0) >> 4);
		B = (BYTE)((color & 0x00F) >> 0);
		
		if(bUsePixelBlend == TRUE)
		{
			m_pLCDReg->WINCON1 |= (0x1<<6)|(0x1<<1);
		}
		else
		{
			m_pLCDReg->VIDOSD1C &= ~((0xF << 20) | (0xF << 16) | (0xF << 12));
			m_pLCDReg->VIDOSD1C |= ((R << 20) | (G << 16) | (B << 12));
			m_pLCDReg->WINCON1 &= ~((0x1<<6)|(0x1<<1));
		}
		if(m_bIsOverlayColorKey)
		{
			if(bUsePixelBlend == FALSE)
			{
				m_pLCDReg->WINCON1 |= (0x1<<6);
			}
			
			m_pLCDReg->W1KEYCON0 |= (0x1<<26);
		}
		else
		{
			m_pLCDReg->W1KEYCON0 &= ~(0x1<<26);
		}		
	}
	else
	{
		m_pLCDReg->W1KEYCON0 &= ~(0x1<<26);
		m_pLCDReg->WINCON1 &= ~((0x1<<6)|(0x1<<1));
		m_pLCDReg->VIDOSD1C = (0xF << 20) | (0xF << 16) | (0xF << 12) | (0x0 << 8) | (0x0 << 4) | (0x0 << 0);
	}
}

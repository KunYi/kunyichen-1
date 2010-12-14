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

Module Name:

Abstract:

Functions:

Notes:

--*/

#ifndef __S3C2450DISP_H__
#define __S3C2450DISP_H__

#include "fimgse2d.h"

#define HIGH_PRIORITY_INTR
#define DISPDRV_IST_PRIORITY	(100)

class S3C2450Surf;
class FIMGSE2D;

class S3C2450DISP : public DDGPE
{
protected:
	GPEModeEx       	m_ModeInfoEx;           // local mode info
	FIMGSE2D		*m_oG2D;
	/// for Cache Region Clean
	DWORD		m_dwSourceSurfacePA;

    ULONG           	m_nSurfaceBitsAlign;    // Surface bits alignments	
    volatile S3C2450_LCD_REG    *m_pLCDReg;                // pointer to the LCD special registers	
    volatile S3C2450_INTR_REG 	*m_pIntrReg; 				// pointer to the Interrupt special registers
    DWORD                   m_dwVSYNCIrq;                     // LCD VSYNC IRQ
    DWORD                   m_dwVSYNCSysIntr;                 // LCD VSYNC SysIntr
    HANDLE                  m_hVSYNCInterruptEvent;     // card insert/remove interrupt event  

    BOOL				m_bIsOverlayColorKey;      
    S3C2450Surf			*m_pVisibleSurface;
    CRITICAL_SECTION		m_CS;
	CRITICAL_SECTION	m_cs2D;				// Ciritcal Section for 2D
    
private:
		void			InitializeDisplayMode();
		
    DWORD     m_pvFlatFrameBuffer;
    DWORD     m_cbScanLineLength;
    DWORD     m_cxPhysicalScreen;
    DWORD     m_cyPhysicalScreen;
    DWORD     m_colorDepth;
    DWORD     m_VirtualFrameBuffer;
    DWORD     m_RedMaskSize;
    DWORD     m_RedMaskPosition;
    DWORD     m_GreenMaskSize;
    DWORD     m_GreenMaskPosition;
    DWORD     m_BlueMaskSize;
    DWORD     m_BlueMaskPosition;
    DWORD     m_VesaMode;

    SurfaceHeap    	*m_pVideoMemoryHeap;     // Base entry representing all video memory

    BOOL      m_CursorDisabled;
    BOOL      m_CursorVisible;
    BOOL      m_CursorForcedOff;
    RECTL     m_CursorRect;
    POINTL    m_CursorSize;
    POINTL    m_CursorHotspot;

    // allocate enough backing store for a 64x64 cursor on a 32bpp (4 bytes per pixel) screen
    UCHAR     m_CursorBackingStore[64 * 64 * 4];
    UCHAR     m_CursorXorShape[64 * 64];
    UCHAR     m_CursorAndShape[64 * 64];

    HANDLE    m_hVFBMapping;
    ULONG     m_VideoPowerState;
	DWORD	  m_FrameBufferSize;    

public:
    BOOL      m_InDDraw;

	// for overlay
    FLATPTR         fpVisibleOverlay;
    FLATPTR			fpOverlayFlipFrom;

    S3C2450DISP();

    virtual
    ~S3C2450DISP();

    virtual
    int
    NumModes();

    virtual
    SCODE
    SetMode(
        int        modeId,
        HPALETTE * palette
        );

    virtual
    int
    InVBlank();

    virtual
    SCODE
    SetPalette(
        const PALETTEENTRY * source,
        USHORT               firstEntry,
        USHORT               numEntries
        );

    virtual
    SCODE
    GetModeInfo(
        GPEMode * pMode,
        int       modeNumber
        );

    virtual
    SCODE
    GetModeInfoEx(
        GPEModeEx *pModeEx,
        int       modeNumber
        );

    virtual
    SCODE
    SetPointerShape(
        GPESurf * mask,
        GPESurf * colorSurface,
        int       xHot,
        int       yHot,
        int       cX,
        int       cY
        );

    virtual
    SCODE
    MovePointer(
        int xPosition,
        int yPosition
        );

    virtual
    void
    WaitForNotBusy();

    virtual
    int
    IsBusy();

    virtual
    void
    GetPhysicalVideoMemory(
        unsigned long * physicalMemoryBase,
        unsigned long * videoMemorySize
        );

    void
    GetVirtualVideoMemory(
        unsigned long * virtualMemoryBase,
        unsigned long * videoMemorySize
        );

    virtual
    SCODE
    Line(
        GPELineParms * lineParameters,
        EGPEPhase      phase
        );

	// blt.cpp
    virtual
    SCODE
    BltPrepare(
        GPEBltParms * blitParameters
        );

    virtual
    SCODE
    BltComplete(
        GPEBltParms * blitParameters
        );

	virtual SCODE AcceleratedBltFill(GPEBltParms *pParms);
	BOOL HWBitBlt(GPEBltParms *pBltParms, PSURFACE_DESCRIPTOR Src, PSURFACE_DESCRIPTOR Dst);
	virtual SCODE AcceleratedSrcCopyBlt(GPEBltParms *pBltParms);
	virtual SCODE AcceleratedBltSelect16(GPEBltParms *pBltParms);
	virtual SCODE AcceleratedDestInvert(GPEBltParms *pBltParms);
	virtual SCODE AcceleratedSolidLine(GPELineParms *pLineParms);
	virtual SCODE SourceRegionCacheClean(GPEBltParms *pBltParms);

	int 
	GetBpp(void);
    virtual
    ULONG
    DrvEscape(
        SURFOBJ * pso,
        ULONG     iEsc,
        ULONG     cjIn,
        void    * pvIn,
        ULONG     cjOut,
        void    * pvOut
        );

    int
    GetGameXInfo(
        ULONG   iEsc,
        ULONG   cjIn,
        void  * pvIn,
        ULONG   cjOut,
        void  * pvOut
        );

    SCODE
    WrappedEmulatedLine(
        GPELineParms * lineParameters
        );

    void
    CursorOn();

    void
    CursorOff();

    // surf.cpp
    virtual
    SCODE
    AllocSurface(
        GPESurf    ** surface,
        int           width,
        int           height,
        EGPEFormat    format,
        int           surfaceFlags
        );

    virtual
    SCODE
    AllocSurface(
        DDGPESurf         ** ppSurf,
        int                  width,
        int                  height,
        EGPEFormat           format,
        EDDGPEPixelFormat    pixelFormat,
        int                  surfaceFlags
        );

    virtual
    void
    SetVisibleSurface(
        GPESurf * pSurf,
        BOOL      bWaitForVBlank = FALSE 
        );

    int
    GetRotateModeFromReg();

    void SetRotateParams();

    long
    DynRotate(
        int angle
        );

	void			SetDisplayPower(ULONG);

	void	EnableInterrupt();
	void	DisableInterrupt();

	// Functions for Overlay 
	void	EnableOverlay();
	void	DisableOverlay();
	void	InitOverlay(S3C2450Surf* pOverlaySurface, RECTL rcSrc);
	void	SetOverlayPosition(UINT32 x,UINT32 y,UINT32 width,UINT32 height);
	void	SetOverlayColorKey(BOOL bUseColorKey, DWORD colorKey);
	void	SetOverlayAlpha(BOOL bUseAlpha, BOOL bUsePixelBlend, DWORD color);

    friend
    void
    buildDDHALInfo(
        LPDDHALINFO lpddhi,
        DWORD       modeidx
        );
};

class S3C2450Surf : public DDGPESurf
{
private:
    SurfaceHeap *m_pHeap;
    HANDLE m_hSurface;
    UCHAR *m_pSurface;

public:
	BOOL  	m_bIsOverlay;			
    S3C2450Surf(int, int, DWORD, VOID*, int, EGPEFormat, EDDGPEPixelFormat pixelFormat, SurfaceHeap*);
    virtual ~S3C2450Surf();

    VOID* SurfaceAddress() { return m_pSurface; }
    VOID  WriteBack();
    VOID  Discard();
    BOOL  SurfaceOk() { return m_pHeap != NULL || m_pSurface != NULL; }
};

class PACSurf : public GPESurf
{
public:
	int m_fPLAllocated;
	DWORD	m_pPhysAddr;

	PACSurf(int, int, EGPEFormat);
	virtual ~PACSurf();
};

#endif __S3C2450DISP_H__


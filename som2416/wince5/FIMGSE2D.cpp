//
// Copyright (c) Samsung Electronics. Co. LTD.  All rights reserved.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
--*/
/**
*	@file		fimgse2d.cpp
*	@brief	hardware control implementation for FIMGSE-2D v2.0, This class is a kind of adapter
*	@author	Jiwon Kim
*	
*	@note This version is made for S3C2450.
*/

#include "precomp.h"
#include <nkintr.h>
#include <s3c2450.h>
#include <bsp.h>
#include "fimgse2d.h"
#include "regctrl_g2d.h"

#define	 HW_PROBE		0
#define	ZONE_NONE		(0)
#define	ZONE_BITBLT	(1<<0)
#define	ZONE_STRETCHBLT	(1<<1)
#define	ZONE_LINE		(1<<2)
#define	ZONE_FLIPBLT	(1<<3)
#define	ZONE_TEMPORARY	(1<<4)
#define	DBG_ZONE		(ZONE_NONE)//(ZONE_BITBLT)
#define	ZONE_CHECK(checkedzone)		(((DBG_ZONE&(checkedzone))==(checkedzone)) ? (1) :(0))
#if HW_PROBE	// For check HW consume time, insert GPIO LED triggering code.

static volatile S3C2450_IOPORT_REG *g_pGPIORegs = NULL;

#define	PROBE_BIT		(7)
#define	PROBE_TRIGGER_OFF(structure, port)	 	{structure##->##port##DAT &= ~(1<<PROBE_BIT);}
#define	PROBE_TRIGGER_ON(structure, port)		{structure##->##port##DAT |= (1<<PROBE_BIT);}
#endif

FIMGSE2D::FIMGSE2D() : RegCtrlG2D()
{
//	Reset();
		m_iROPMapper[ROP_SRC_ONLY] = G2D_ROP_SRC_ONLY;
		m_iROPMapper[ROP_PAT_ONLY] = G2D_ROP_PAT_ONLY;
		m_iROPMapper[ROP_DST_ONLY] = G2D_ROP_DST_ONLY;
		m_iROPMapper[ROP_SRC_OR_DST] = G2D_ROP_SRC_OR_DST;
		m_iROPMapper[ROP_SRC_OR_PAT] = G2D_ROP_SRC_OR_PAT;
		m_iROPMapper[ROP_DST_OR_PAT] = G2D_ROP_DST_OR_PAT;
		m_iROPMapper[ROP_SRC_AND_DST] = G2D_ROP_SRC_AND_DST;
		m_iROPMapper[ROP_SRC_AND_PAT] =	G2D_ROP_SRC_AND_PAT;
		m_iROPMapper[ROP_DST_AND_PAT] = G2D_ROP_DST_AND_PAT;
		m_iROPMapper[ROP_SRC_XOR_DST] = G2D_ROP_SRC_XOR_DST;
		m_iROPMapper[ROP_SRC_XOR_PAT] = G2D_ROP_SRC_XOR_PAT;
		m_iROPMapper[ROP_DST_XOR_PAT] = G2D_ROP_DST_XOR_PAT;
		m_iROPMapper[ROP_NOTSRCCOPY] = G2D_ROP_NOTSRCCOPY;
		m_iROPMapper[ROP_DSTINVERT] = G2D_ROP_DSTINVERT;
		m_iROPMapper[ROP_R2_NOTCOPYPEN] = G2D_ROP_R2_NOTCOPYPEN;

		m_iColorModeMapper[gpe1Bpp] = -1;
		m_iColorModeMapper[gpe2Bpp] = -1;
		m_iColorModeMapper[gpe4Bpp] = -1;
		m_iColorModeMapper[gpe8Bpp] = -1;
		m_iColorModeMapper[gpe16Bpp] = G2D_COLOR_RGB_565;
		m_iColorModeMapper[gpe24Bpp] = G2D_COLOR_XRGB_8888;
		m_iColorModeMapper[gpe32Bpp] = -1;
		m_iColorModeMapper[gpe16YCrCb] = -1;
		m_iColorModeMapper[gpeDeviceCompatible] = -1;		
		m_iColorModeMapper[gpeUndefined] = G2D_COLOR_UNUSED;				
			
}

FIMGSE2D::~FIMGSE2D()
{
}

// Set Ternary raster operation
// Support 256 raster operation
// Refer to ternary raster operation table if you know 256 ROP

// Set Alpha Value
void FIMGSE2D::SetAlphaValue(BYTE ucAlphaVal)
{
	ucAlphaVal &= 0xff;
	m_pG2DReg->ALPHA = (m_pG2DReg->ALPHA&(~0xff)) | ucAlphaVal;
}

// Set alpha blending mode
void FIMGSE2D::SetAlphaMode(G2D_ALPHA_BLENDING_MODE eMode)
{
	DWORD uAlphaBlend;

	uAlphaBlend =
		(eMode == G2D_NO_ALPHA_MODE) ? G2D_NO_ALPHA_BIT :
		(eMode == G2D_PP_ALPHA_SOURCE_MODE) ? G2D_PP_ALPHA_SOURCE_BIT :
		(eMode == G2D_ALPHA_MODE) ? G2D_ALPHA_BIT : 
		(eMode == G2D_FADING_MODE) ? G2D_FADING_BIT : G2D_NO_ALPHA_BIT;

	m_pG2DReg->ROP = (m_pG2DReg->ROP & ~(0x7<<10)) | uAlphaBlend;
}

// Set fade value
void FIMGSE2D::SetFadingValue(BYTE ucFadeVal)
{
	ucFadeVal &= 0xff;
	m_pG2DReg->ALPHA = (m_pG2DReg->ALPHA & ~(0xff<<8)) | (ucFadeVal<<8);
}

void FIMGSE2D::DisableEffect(void)
{
	m_pG2DReg->ROP &= ~(0x7<<10);
}

void FIMGSE2D::EnablePlaneAlphaBlending(BYTE ucAlphaVal)
{
	ucAlphaVal &= 0xff;

	// Set Alpha Blending Mode
	m_pG2DReg->ROP = ((m_pG2DReg->ROP) & ~(0x7<<10)) | G2D_ALPHA_BIT;


	// Set Alpha Value
	m_pG2DReg->ALPHA = ((m_pG2DReg->ALPHA) & ~(0xff)) | ucAlphaVal;

	m_ucAlphaVal = ucAlphaVal;
	m_bIsAlphaCall = true;
}

void FIMGSE2D::DisablePlaneAlphaBlending(void)
{
	DisableEffect();
}

void FIMGSE2D::EnablePixelAlphaBlending(void) // Only Support 24bpp and Only used in BitBlt
{

	Assert( (m_iColorModeMapper[m_descDstSurface.dwColorMode] != -1) && (m_iColorModeMapper[m_descDstSurface.dwColorMode] != gpe24Bpp) );

	m_pG2DReg->ROP = ((m_pG2DReg->ROP) & ~(0x7<<10)) | G2D_PP_ALPHA_SOURCE_BIT;
}

void FIMGSE2D::DisablePixelAlphaBlending(void) // Only Support 24bpp and only used in BitBlt
{
	Assert( (m_iColorModeMapper[m_descDstSurface.dwColorMode] != -1) && (m_iColorModeMapper[m_descDstSurface.dwColorMode] != gpe24Bpp) );
	DisableEffect();
}

void FIMGSE2D::EnableFadding(BYTE ucFadingVal)
{
	BYTE ucAlphaVal;

	ucAlphaVal = (m_bIsAlphaCall == true) ? m_ucAlphaVal : 255;

	ucFadingVal &= 0xff;

	// Set Fadding Mode	
	m_pG2DReg->ROP = ((m_pG2DReg->ROP) & ~(0x7<<10)) | G2D_FADING_BIT;

	// Set Fadding Value	
	m_pG2DReg->ALPHA = ((m_pG2DReg->ALPHA) & ~(0xff<<8)) | (ucFadingVal<<8) | (ucAlphaVal<<0);
}

void FIMGSE2D::DisableFadding(void)
{
	DisableEffect();
}



/**
*	@fn	FIMGSE2D::GetRotType(int m_iRotate)
*	@brief	This function convert rotation degree value to ROT_TYPE
*
*/
ROT_TYPE FIMGSE2D::GetRotType(int m_iRotate)
{
	switch(m_iRotate)
	{
		case DMDO_0:	
			return	ROT_0;
		case DMDO_90:
			return	ROT_270;
		case DMDO_180:
			return	ROT_180;
		case DMDO_270:
			return	ROT_90;
		default:	
			return	ROT_0;
	}
	return ROT_0;	
}

/**
*	@fn	DWORD FIMGSE2D::CalculateXYIncrFormat(DWORD uDividend, DWORD uDivisor)
*	@brief	This function returns x_incr or y_incr vaule in register format
*	@input	this function accept real pixel coordinate, ex) (0,0)~(9,9) means that 10pixel by pixel image
*	@return	Result value
*/
DWORD FIMGSE2D::CalculateXYIncrFormat(DWORD uDividend, DWORD uDivisor)
{
	int i;
	DWORD uQuotient;
	DWORD uUnderPoint=0;

//printf("\nuDivend:%x(%d), uDivisor:%x(%d), uUnderPoint:%x(%d)", uDividend, uDividend, uDivisor, uDivisor,uUnderPoint, uUnderPoint);	

	Assert(uDivisor != 0);
	if(uDivisor == 0)
	{
		uDivisor = 1;	//< this will prevent data abort. but result is incorrect.
	}

	uQuotient = (DWORD)(uDividend/uDivisor);

	Assert(uQuotient <= 2048); // Quotient should be less than 2048.

	uDividend-=(uQuotient*uDivisor);

	/// Now under point is calculated.
	for (i=0; i<12; i++)
	{
		uDividend <<= 1;
		uUnderPoint <<= 1;
		
		if (uDividend >= uDivisor)
		{
			uUnderPoint = uUnderPoint | 1;
			uDividend -= uDivisor;
		}
//		printf("\nuDivend:%x(%d), uDivisor:%x(%d), uUnderPoint:%x(%d)", uDividend, uDividend, uDivisor, uDivisor,uUnderPoint, uUnderPoint);
	}

	uUnderPoint = (uUnderPoint  + 1 )>> 1;
//	uUnderPoint = ~uUnderPoint + 1 ;

	return ( uUnderPoint|(uQuotient<<11) );
}

/**
*	@fn	FIMGSE2D::BitBlt(PRECTL prclSrc, PRECTL prclDst, ROT_TYPE m_iRotate)
*	@param	prclSrc	Source Rectangle
*	@param	prclDst	Destination Rectangle
*	@param	m_iRotate	Rotatation Degree. See also ROT_TYPE type
*	@note This funciton performs real Bit blit using 2D HW. this functio can handle rotation case.
*			There's predefine macro type for presenting rotation register's setting value
*			G2D_ROTATION
@	@sa	ROT_TYPE	this can be set mixed value.
*/
void FIMGSE2D::BitBlt(PRECTL prclSrc, PRECTL prclDst, ROT_TYPE m_iRotate )
{
	DWORD uCmdRegVal=0;
	RECT	rectDst;			//< If rotation case this value must be corrected.
	
	RETAILMSG(ZONE_CHECK(ZONE_BITBLT),(TEXT("[2DHW] BitBlt Entry\r\n")));	
	
	/// Set Destination's Rotation mode
	SetRotationMode(m_iRotate);		
	SetCoordinateSrcBlock(prclSrc->left, prclSrc->top, prclSrc->right - 1, prclSrc->bottom - 1);	

	if(m_iRotate == ROT_180)		//< origin set to (x2,y2)
	{
		rectDst.left = prclDst->right - 1;						//< x2
		rectDst.top = prclDst->bottom - 1;						//< y2
		rectDst.right = 2 * (prclDst->right - 1) - prclDst->left ;		//< x2 + (x2 - x1)
		rectDst.bottom = 2 * (prclDst->bottom -1) - prclDst->top;	//< y2 + (y2 - y1)
	}
	else	 if(m_iRotate == ROT_90)		//<In this time, Height and Width are swapped.	
	{
		rectDst.left = prclDst->right - 1;						//< x2
		rectDst.right = prclDst->right - 1 + prclDst->bottom - 1 - prclDst->top;	//< x2 + (y2 - y1)
		rectDst.top = prclDst->top;										//< y1
		rectDst.bottom = prclDst->top + prclDst->right - 1 - prclDst->left;		//< y1 + (x2 - x1)
	}
	else	 if(m_iRotate == ROT_270)		//<In this time, Height and Width are swapped.	
	{
		rectDst.left = prclDst->left;							//< x1
		rectDst.right = prclDst->left + prclDst->bottom - 1- prclDst->top;		//< x1 + (y2 - y1)
		rectDst.top = prclDst->bottom - 1;									//< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->right - 1- prclDst->left;	//< y2 + (x2 - x1)
	}	
	else		//< ROT_0
	{
		rectDst.left = prclDst->left;
		rectDst.top = prclDst->top;		
		rectDst.right = prclDst->right - 1;
		rectDst.bottom = prclDst->bottom - 1;
	}

	SetRotationOrg((WORD)rectDst.left, (WORD)rectDst.top);	
	SetCoordinateDstBlock(rectDst.left, rectDst.top, rectDst.right, rectDst.bottom);			

	RETAILMSG(ZONE_CHECK(ZONE_BITBLT),(TEXT("ROT:%d, Src:(%d,%d)~(%d,%d), Dst:(%d,%d)~(%d,%d)\r\n"), 
		m_iRotate, prclSrc->left, prclSrc->top, prclSrc->right, prclSrc->bottom, 
		rectDst.left, rectDst.top, rectDst.right, rectDst.bottom));

	uCmdRegVal = G2D_NORMAL_BITBLT_BIT;
	
//	WaitForIdleStatus();		//< When only Engine is idle, 2D Engine runs.	

#if	(G2D_CMDPROCESSING==G2D_FASTRETURN)
	WaitForEmptyFifo();		//< This is check fully empty command fifo.
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request
#elif (G2D_CMDPROCESSING==G2D_INTERRUPT)
	CheckFifo(1);
	IntEnable();	
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request
	WaitForSingleObject(m_hInterrupt2D, INFINITE);

	IntDisable();	
	IntPendingClear();	

	InterruptDone(m_dwSysIntr2D);
#elif (G2D_CMDPROCESSING==G2D_BUSYWAITING)
	CheckFifo(1);
	
	IntEnable();	
	while(!WaitForFinish());						// Polling Style	
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request

	IntDisable();	
#else
	RETAILMSG(TRUE,(TEXT("CMDPROCESSING TYPE is invalid, Please Check Header Definition\n")));
	return FALSE;
#endif

	RETAILMSG(ZONE_CHECK(ZONE_BITBLT),(TEXT("[2DHW] BitBlt Exit\r\n")));			
	/// TODO: Resource Register clearing can be needed.

}

/**
*	@fn	FIMGSE2D::StretchBlt(PRECTL prclSrc, PRECTL prclDst, ROT_TYPE m_iRotate)
*	@param	prclSrc	Source Rectangle
*	@param	prclDst	Destination Rectangle
*	@param	m_iRotate	Rotatation Degree. See also ROT_TYPE type
*	@note This funciton performs real Stretched Bit blit using 2D HW. this functio can handle rotation case.
*			There's predefine macro type for presenting rotation register's setting value
*			G2D_ROTATION
*	@note This function can not support Multiple Operation ex) mirrored + rotation because of HW
*	@sa	ROT_TYPE
**/
void FIMGSE2D::StretchBlt(PRECTL prclSrc, PRECTL prclDst, ROT_TYPE m_iRotate)
{
	WORD usSrcWidth = 0;
	WORD usSrcHeight = 0;
	WORD usDstWidth = 0;
	WORD usDstHeight = 0;
	DWORD uXYIncr = 0;
	DWORD uCmdRegVal=0;
	RECTL	rectDst;			//< If rotation case this value must be corrected.

	RETAILMSG(ZONE_CHECK(ZONE_STRETCHBLT),(TEXT("[2DHW] StretchBlt Entry\r\n")));	

	/// Set Stretch parameter
	/// Stretch ratio calculation, width and height is not include last line
	usSrcWidth=(WORD) ABS( prclSrc->right  - prclSrc->left);
	usDstWidth=(WORD) ABS( prclDst->right  - prclDst->left);
	usSrcHeight=(WORD) ABS( prclSrc->bottom  - prclSrc->top);
	usDstHeight=(WORD) ABS( prclDst->bottom  - prclDst->top);

	if((m_iRotate == ROT_90) ||(m_iRotate == ROT_270) )
	{
		if(usSrcWidth == usDstHeight && usSrcHeight == usDstWidth)
		{
			RETAILMSG(TRUE, (TEXT("This is not stretch or shrink BLT, redirect to BitBlt, R:%d\n"), m_iRotate));
			BitBlt(prclSrc, prclDst, m_iRotate);
			return;
		}
		uXYIncr = CalculateXYIncrFormat(usSrcWidth - 1, usDstHeight - 1);	
		SetYIncr(uXYIncr);
	}
	else
	{
		if(usSrcWidth == usDstWidth && usSrcHeight == usDstHeight)
		{
			RETAILMSG(TRUE, (TEXT("This is not stretch or shrink BLT, redirect to BitBlt, R:%d\n"), m_iRotate));
			BitBlt(prclSrc, prclDst, m_iRotate);
			return;
		}
	
		uXYIncr = CalculateXYIncrFormat(usSrcWidth - 1, usDstWidth - 1);	
		SetXIncr(uXYIncr);
	}
	
//	printf("\nXIncr : %d.%x", (uXYIncr&0x003ff800)>>11, (uXYIncr & 0x000007ff));

	if((m_iRotate == ROT_90) ||(m_iRotate == ROT_270) )
	{
		uXYIncr = CalculateXYIncrFormat(usSrcHeight - 1, usDstWidth - 1);
		SetXIncr(uXYIncr);
	}
	else
	{
		uXYIncr = CalculateXYIncrFormat(usSrcHeight - 1, usDstHeight - 1);
		SetYIncr(uXYIncr);
	}
//	printf("\nYIncr : %d.%x", (uXYIncr&0x003ff800)>>11, (uXYIncr & 0x000007ff));	
	
	SetCoordinateSrcBlock(prclSrc->left, prclSrc->top, prclSrc->right - 1, prclSrc->bottom - 1);	

	if(m_iRotate == ROT_180)		//< origin set to (x2,y2)
	{
		rectDst.left = prclDst->right - 1;						//< x2
		rectDst.top = prclDst->bottom - 1;						//< y2
		rectDst.right = 2 * (prclDst->right - 1) - prclDst->left ;		//< x2 + (x2 - x1)
		rectDst.bottom = 2 * (prclDst->bottom -1) - prclDst->top;	//< y2 + (y2 - y1)
	}
	else	 if(m_iRotate == ROT_90)		//<In this time, Height and Width are swapped.	
	{
		rectDst.left = prclDst->right - 1;						//< x2
		rectDst.right = prclDst->right - 1 + prclDst->bottom - 1 - prclDst->top;	//< x2 + (y2 - y1)
		rectDst.top = prclDst->top;										//< y1
		rectDst.bottom = prclDst->top + prclDst->right - 1 - prclDst->left;		//< y1 + (x2 - x1)
	}
	else	 if(m_iRotate == ROT_270)		//<In this time, Height and Width are swapped.	
	{
		rectDst.left = prclDst->left;							//< x1
		rectDst.right = prclDst->left + prclDst->bottom - 1- prclDst->top;		//< x1 + (y2 - y1)
		rectDst.top = prclDst->bottom - 1;									//< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->right - 1- prclDst->left;	//< y2 + (x2 - x1)
	}	
	else		//< ROT_0
	{
		rectDst.left = prclDst->left;
		rectDst.top = prclDst->top;		
		rectDst.right = prclDst->right - 1;
		rectDst.bottom = prclDst->bottom - 1;
	}
	
	/// Set Destination's Rotation mode
	SetRotationMode(m_iRotate);		
	SetRotationOrg((WORD)rectDst.left, (WORD)rectDst.top);	
	SetCoordinateDstBlock(rectDst.left, rectDst.top, rectDst.right, rectDst.bottom);			

//	if(m_iRotate!=ROT_0)
//	{
		RETAILMSG(ZONE_CHECK(ZONE_STRETCHBLT),(TEXT("ROT:%d, Src:(%d,%d)~(%d,%d), Dst:(%d,%d)~(%d,%d), OC:(%d,%d)\r\n"), 
			m_iRotate, prclSrc->left, prclSrc->top, prclSrc->right, prclSrc->bottom, 
			rectDst.left, rectDst.top, rectDst.right, rectDst.bottom, rectDst.left, rectDst.top));
//	}

	uCmdRegVal = G2D_STRETCH_BITBLT_BIT;

#if	(G2D_CMDPROCESSING==G2D_FASTRETURN)
	WaitForEmptyFifo();		//< This is check fully empty command fifo.
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request
#elif (G2D_CMDPROCESSING==G2D_INTERRUPT)
	CheckFifo(1);
	IntEnable();	
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request
	WaitForSingleObject(m_hInterrupt2D, INFINITE);

	IntDisable();	
	IntPendingClear();	

	InterruptDone(m_dwSysIntr2D);
#elif (G2D_CMDPROCESSING==G2D_BUSYWAITING)
	CheckFifo(1);
	IntEnable();	
	while(!WaitForFinish());						// Polling Style	
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request

	IntDisable();	
#else
	RETAILMSG(TRUE,(TEXT("CMDPROCESSING TYPE is invalid, Please Check Header Definition\n")));
	return FALSE;
#endif

	RETAILMSG(ZONE_CHECK(ZONE_STRETCHBLT),(TEXT("[2DHW] StretchBlt Exit\r\n")));			
	/// TODO: Resource Register clearing can be needed.

}

/**
*	@fn	FIMGSE2D::FlipBlt(PRECTL prclSrc, PRECTL prclDst, ROT_TYPE m_iRotate)
*	@param	prclSrc	Source Rectangle
*	@param	prclDst	Destination Rectangle
*	@param	m_iRotate	Flip Setting. See also ROT_TYPE type
*	@note This funciton performs ONLY FLIP Bit blit using 2D HW. this function cannot handle rotation case.
*			There's predefine macro type for presenting rotation register's setting value 
*			This function requires Scratch Memory for Destination.
*			This function don't support X&Y flipping
*	@sa	ROT_TYPE
**/
BOOL FIMGSE2D::FlipBlt(PRECTL prclSrc, PRECTL prclDst, ROT_TYPE m_iRotate)
{
	DWORD uCmdRegVal=0;
	RECTL	rectDst;			//< If rotation case this value must be corrected.	

	RETAILMSG(ZONE_CHECK(ZONE_FLIPBLT),(TEXT("[2DHW] FlipBlt Entry\r\n")));				

	/// Always LeftTop Coordinate is less than RightBottom for Source and Destination Region
	Assert( (prclSrc->left < prclSrc->right) && (prclSrc->top < prclSrc->bottom) );
	Assert( (prclDst->left < prclDst->right) && (prclDst->top < prclDst->bottom) );

	/// Check Flip Option, we only do care about only flip, don't care about rotation option although it set.
	if(HASBIT_COND(m_iRotate, FLIP_X))
	{
		SetRotationMode(FLIP_X);
		/// Set rotation origin on destination's bottom line.
		rectDst.left = prclDst->left;					//< x1
		rectDst.right = prclDst->right - 1;				//< x2
		rectDst.top = prclDst->bottom - 1;			//< y2
		rectDst.bottom = prclDst->bottom - 1 + prclDst->bottom - 1 - prclDst->top;	//< y2 + (y2-y1)
	}
	else if(HASBIT_COND(m_iRotate, FLIP_Y))
	{
		SetRotationMode(FLIP_Y);	
		/// Set rotation origin on destination's right line.		
		rectDst.left = prclDst->right - 1;				//< x2
		rectDst.right = prclDst->right - 1 + prclDst->right - 1 - prclDst->left;		//< x2 + (x2 - x1)
		rectDst.top = prclDst->top;					//< y1
		rectDst.bottom = prclDst->bottom - 1;			//< y2
	}
	else
	{
		/// Do not need to do Flip operation.
		return FALSE;
	}
	
	SetCoordinateSrcBlock(prclSrc->left, prclSrc->top, prclSrc->right - 1, prclSrc->bottom - 1);	
	SetRotationOrg((WORD)rectDst.left, (WORD)rectDst.top);
	SetCoordinateDstBlock(rectDst.left, rectDst.top, rectDst.right, rectDst.bottom);	//< exclude Destination Right, Bottom line

	uCmdRegVal = G2D_NORMAL_BITBLT_BIT;

#if	(G2D_CMDPROCESSING==G2D_FASTRETURN)
	WaitForEmptyFifo();		//< This is check fully empty command fifo.
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request
#elif (G2D_CMDPROCESSING==G2D_INTERRUPT)
	CheckFifo(1);
	IntEnable();	
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request
	WaitForSingleObject(m_hInterrupt2D, INFINITE);

	IntDisable();	
	IntPendingClear();	

	InterruptDone(m_dwSysIntr2D);
#elif (G2D_CMDPROCESSING==G2D_BUSYWAITING)
	CheckFifo(1);
	IntEnable();	
	while(!WaitForFinish());						// Polling Style	
	m_pG2DReg->CMDR1 = uCmdRegVal;		// Process Only One Instruction per bitblt request

	IntDisable();	
#else
	RETAILMSG(TRUE,(TEXT("CMDPROCESSING TYPE is invalid, Please Check Header Definition\n")));
	return FALSE;
#endif

	RETAILMSG(ZONE_CHECK(ZONE_FLIPBLT),(TEXT("[2DHW] FlipBlt Exit\r\n")));			
	/// TODO: Resource Register clearing can be needed.
	//Reset();

	return TRUE;

}

// if ucTransMode is '1', Transparent Mode
// else '0', Opaque Mode
void FIMGSE2D::SetTransparentMode(bool bIsTransparent, COLOR uBsColor)
{
	DWORD uRopRegVal;

	CheckFifo(2);

	uRopRegVal = m_pG2DReg->ROP;

	uRopRegVal =
		(bIsTransparent == 1) ? (uRopRegVal | G2D_TRANSPARENT_BIT) : (uRopRegVal & ~(G2D_TRANSPARENT_BIT));

	m_pG2DReg->ROP = uRopRegVal;

	// register Blue Screen Color
	m_pG2DReg->BS_COLOR = uBsColor;
}

// if ucTransMode is '1', Transparent Mode
// else '0', Opaque Mode
void FIMGSE2D::SetColorKeyOn(DWORD uBsColor)
{
	CheckFifo(2);

	m_pG2DReg->ROP = m_pG2DReg->ROP | G2D_TRANSPARENT_BIT;

	// register Blue Screen Color
	m_pG2DReg->BS_COLOR = uBsColor;
}

void FIMGSE2D::SetColorKeyOff(void)
{
	CheckFifo(2);

	// Blue screen off
	m_pG2DReg->ROP =  m_pG2DReg->ROP & ~(G2D_TRANSPARENT_BIT);

	// color key off	
	m_pG2DReg->COLORKEY_CNTL = (m_pG2DReg->COLORKEY_CNTL & ~(0x1U<<31));
}

void FIMGSE2D::SetFgColor(DWORD uFgColor)
{
	CheckFifo(1);
	uFgColor &= 0x00ffffff;		//< Remove Alpha value
	m_pG2DReg->FG_COLOR = uFgColor;
}

void FIMGSE2D::SetBgColor(DWORD uBgColor)
{
	CheckFifo(1);
	uBgColor &= 0x00ffffff;		//< Remove Alpha value
	m_pG2DReg->BG_COLOR = uBgColor;
}

void FIMGSE2D::SetBsColor(DWORD uBsColor)
{
	CheckFifo(1);
	uBsColor &= 0x00ffffff;		//< Remove Alpha value
	m_pG2DReg->BS_COLOR = uBsColor;
}


/**
*	@fn	void FIMGSE2D::FillRect(PRECT prtDst, DWORD uColor)
*	@param	prtDst	Destination Rectangle
*	@param	uColor	Filled Color
*	@attention	prtDst must have positive value
*	@brief	prclDst must be rotated when screen is rotated.
*/
void FIMGSE2D::FillRect(PRECTL prclDst, COLOR uColor)
{
	RETAILMSG(0,(TEXT("C:0x%x, prclDst:%d,%d,%d,%d\n"),
	uColor, prclDst->left, prclDst->top, prclDst->right, prclDst->bottom
	));

	SetFgColor(uColor);
	Set3rdOperand(G2D_OPERAND3_FG);	
	SetRopEtype(ROP_PAT_ONLY);
	BitBlt(prclDst, prclDst, ROT_0);		// Fill Rect doesn't care about screen rotation,
}

/*
 * 	@fn	void FIMGSE2D::SetSrcSurface(PSURFACE_DESCRIPTOR desc_surface)
 *	@brief	Set Source Surface Information to FIMG2D HW Register
 *	@param	desc_surface	Surface Information : Base Address, Horizontal&Vertical Resolution, Color mode
 */
void FIMGSE2D::SetSrcSurface(PSURFACE_DESCRIPTOR desc_surface)
{
	CheckFifo(4);
	
	RETAILMSG(0,(TEXT("SRCaddr: 0x%x, Source Surface Base: 0x%x, Color:%d, Hori:%d, Vert:%d\r\n"),
		&(m_pG2DReg->SRC_BASE_ADDR),
		desc_surface->dwBaseaddr, desc_surface->dwColorMode, desc_surface->dwHoriRes, desc_surface->dwVertRes));
	m_pG2DReg->SRC_BASE_ADDR = desc_surface->dwBaseaddr;
	
	m_pG2DReg->SRC_COLOR_MODE = m_iColorModeMapper[desc_surface->dwColorMode];
	
	m_pG2DReg->SRC_HORI_RES = desc_surface->dwHoriRes;
	m_pG2DReg->SRC_VERT_RES = desc_surface->dwVertRes;
}

/*
 * 	@fn	void FIMGSE2D::SetDstSurface(PSURFACE_DESCRIPTOR desc_surface)
 *	@brief	Set Destination Surface Information to FIMG2D HW Register
 *	@param	desc_surface	Surface Information : Base Address, Horizontal&Vertical Resolution, Color mode */
void FIMGSE2D::SetDstSurface(PSURFACE_DESCRIPTOR desc_surface)
{
	CheckFifo(4);
	
	RETAILMSG(0,(TEXT("DSTaddr: 0x%x, Destination Surface Base: 0x%x, Color:%d, Hori:%d, Vert:%d\r\n"),
		&(m_pG2DReg->DST_BASE_ADDR),
		desc_surface->dwBaseaddr, desc_surface->dwColorMode, desc_surface->dwHoriRes, desc_surface->dwVertRes));
	m_pG2DReg->DST_BASE_ADDR = desc_surface->dwBaseaddr;
	
	m_pG2DReg->DST_COLOR_MODE = m_iColorModeMapper[desc_surface->dwColorMode];
	
	m_pG2DReg->SC_HORI_RES = desc_surface->dwHoriRes;
	m_pG2DReg->SC_VERT_RES = desc_surface->dwVertRes;
}


/**
*	Initialize 2D HW
*/
void FIMGSE2D::Init() 
{
	CheckFifo(4);
	
#if HW_PROBE	// For check HW consume time, insert GPIO LED triggering code.
	// GPIO Virtual alloc
	if(g_pGPIORegs == NULL)
	{
		g_pGPIORegs = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	}
	if (g_pGPIORegs == NULL)
	{
		RETAILMSG(1,(TEXT("[GPIO] g_pGPIORegs: VirtualAlloc failed!\r\n")));
		g_pGPIORegs = NULL;
	}
	else
	if (!VirtualCopy((PVOID)g_pGPIORegs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT>>8), sizeof(S3C2450_IOPORT_REG), PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL ))
	{
		RETAILMSG(1, (TEXT("[GPIO] g_pGPIORegs: VirtualCopy failed!\r\n")));
		VirtualFree((PVOID)g_pGPIORegs, 0, MEM_RELEASE);
	}
//	g_pGPIORegs->GPNPUD &= ~(0xff<<24);	// Pull Up/Down Disable
//	g_pGPIORegs->GPNCON = (g_pGPIORegs->GPNCON & ~(0xff<<24)) | (0x55<<24);	// GPN[15:14] set to output

#endif

	/// Font Operation Related
	m_bIsBitBlt = true;	
	m_bIsScr2Scr = false;
	DisableEffect(); // Disable per-pixel/per-plane alpha blending and fading
	SetColorKeyOff();

	m_pG2DReg->ALPHA = (FADING_OFFSET_DISABLE | ALPHA_VALUE_DISABLE);
	m_pG2DReg->ROP = (G2D_OPERAND3_FG_BIT | G2D_NO_ALPHA_BIT | OPAQUE_ENABLE | G2D_ROP_SRC_ONLY);
	SetRotationOrg(0, 0);
	m_pG2DReg->ROT_MODE = ROT_0;
	m_pG2DReg->ALPHA = 0;
}

void FIMGSE2D::PutPixel(DWORD uPosX, DWORD uPosY, DWORD uColor) //modification
{
	CheckFifo(4);
	
	m_pG2DReg->COORD0_X = uPosX;
	m_pG2DReg->COORD0_Y = uPosY;
	m_pG2DReg->FG_COLOR = uColor;

#if	(G2D_CMDPROCESSING==G2D_FASTRETURN)
	WaitForEmptyFifo();		//< This is check fully empty command fifo.
	m_pG2DReg->CMDR0 = G2D_REND_POINT_BIT;
#elif (G2D_CMDPROCESSING==G2D_INTERRUPT)
	CheckFifo(1);
	IntEnable();	
	m_pG2DReg->CMDR0 = G2D_REND_POINT_BIT;
	WaitForSingleObject(m_hInterrupt2D, INFINITE);

	IntDisable();	
	IntPendingClear();	

	InterruptDone(m_dwSysIntr2D);
#elif (G2D_CMDPROCESSING==G2D_BUSYWAITING)
	CheckFifo(1);
	IntEnable();	
	while(!WaitForFinish());						// Polling Style	
	m_pG2DReg->CMDR0 = G2D_REND_POINT_BIT;

	IntDisable();	
#else
	RETAILMSG(TRUE,(TEXT("CMDPROCESSING TYPE is invalid, Please Check Header Definition\n")));
	return ;
#endif

}

/**
 * Draw Line
 * (usPosX1, usPosY1) ~ (usPosX2, usPosY2)
 * Do not draw last point
 *   0 < usPosX, usPosY1, usPosX2, usPosY2 < 2048
 */
void FIMGSE2D::PutLine(DWORD usPosX1, DWORD usPosY1, DWORD usPosX2, DWORD usPosY2, DWORD uColor, bool bIsDrawLastPoint) //modification
{
	int nMajorCoordX;
	DWORD uHSz, uVSz;
	int i;
	int nIncr=0;
	DWORD uCmdRegVal;

	CheckFifo(7);

	m_pG2DReg->COORD0_X = usPosX1;
	m_pG2DReg->COORD0_Y = usPosY1;
	m_pG2DReg->COORD2_X = usPosX2;
	m_pG2DReg->COORD2_Y = usPosY2;	

	RETAILMSG(0,(TEXT("COORD0_X: %d, COORD0_Y:%d, COORD2_X:%d, COORD2_Y:%d\n"), 
			m_pG2DReg->COORD0 & (0x7ff),
			(m_pG2DReg->COORD0 & (0x7ff<<16)) >> 16,			
			m_pG2DReg->COORD2 & (0x7ff),
			(m_pG2DReg->COORD2 & (0x7ff<<16)) >> 16));			
	RETAILMSG(0,(TEXT("CLIP_LT_X: %d, CLIP_LT_Y:%d, CLIP_RB_X:%d, CLIP_RB_Y:%d\n"), 
			m_pG2DReg->CW_LEFT_TOP & (0x7ff),
			(m_pG2DReg->CW_LEFT_TOP & (0x7ff<<16)) >> 16,			
			m_pG2DReg->CW_RIGHT_BOTTOM & (0x7ff),
			(m_pG2DReg->CW_RIGHT_BOTTOM & (0x7ff<<16)) >> 16));				

	uVSz = ABS((WORD)usPosY1 - (WORD)usPosY2);
	uHSz = ABS((WORD)usPosX1 - (WORD)usPosX2);

	nMajorCoordX = (uHSz>=uVSz);

	if(nMajorCoordX)
	{
		for (i=0; i<12; i++)
		{
	    	uVSz <<= 1;
	    	nIncr <<= 1;
	    	if (uVSz >= uHSz)
	    	{
				nIncr = nIncr | 1;
				uVSz -= uHSz;
	    	}
		}
		nIncr = (nIncr + 1) >> 1;
		if (usPosY1 > usPosY2)
		{
	    	nIncr = (~nIncr) + 1; // 2's complement
		}
//		printf("pre YINCR: %x  ", nIncr );						
  }
	else
	{
		for (i=0; i<12; i++)
		{
	    	uHSz <<= 1;
	    	nIncr <<= 1;
	    	if (uHSz >= uVSz)
	    	{
				nIncr = nIncr | 1;
				uHSz -= uVSz;
	    	}
		}
		nIncr = (nIncr + 1) >> 1;
		if (usPosX1 > usPosX2)
		{
	    	nIncr = (~nIncr) + 1; // 2's complement
		}
//		printf("pre XINCR: %x  ", nIncr );				
	}

	m_pG2DReg->FG_COLOR = uColor;

	uCmdRegVal = 0;

	if(nMajorCoordX)
	{
		SetYIncr(nIncr);

		uCmdRegVal =
			(bIsDrawLastPoint == true) ? (G2D_REND_LINE_BIT | G2D_MAJOR_COORD_X_BIT & G2D_DRAW_LAST_POINT_BIT) :
			(G2D_REND_LINE_BIT | G2D_MAJOR_COORD_X_BIT | G2D_NOT_DRAW_LAST_POINT_BIT);
		
		RETAILMSG(0,(TEXT("m_pG2DReg:0x%x, CMD: %x, XINCR: %x, YINCR: %x\n"), m_pG2DReg, uCmdRegVal, m_pG2DReg->X_INCR, m_pG2DReg->Y_INCR ));
	}
	else
	{
		SetXIncr(nIncr);

		uCmdRegVal =
			(bIsDrawLastPoint == true) ? (G2D_REND_LINE_BIT | G2D_MAJOR_COORD_Y_BIT & G2D_DRAW_LAST_POINT_BIT) :
			(G2D_REND_LINE_BIT | G2D_MAJOR_COORD_Y_BIT | G2D_NOT_DRAW_LAST_POINT_BIT);

		RETAILMSG(0,(TEXT("CMD: %x, XINCR: %x, YINCR: %x\n"), uCmdRegVal, m_pG2DReg->X_INCR, m_pG2DReg->Y_INCR ));
	}

#if	(G2D_CMDPROCESSING==G2D_FASTRETURN)
	WaitForEmptyFifo();		//< This is check fully empty command fifo.
	m_pG2DReg->CMDR0 = uCmdRegVal;
#elif (G2D_CMDPROCESSING==G2D_INTERRUPT)
	CheckFifo(1);
	IntEnable();	
	m_pG2DReg->CMDR0 = uCmdRegVal;
	WaitForSingleObject(m_hInterrupt2D, INFINITE);

	IntDisable();	
	IntPendingClear();	

	InterruptDone(m_dwSysIntr2D);
#elif (G2D_CMDPROCESSING==G2D_BUSYWAITING)
	CheckFifo(1);
	IntEnable();	
	while(!WaitForFinish());						// Polling Style	
	m_pG2DReg->CMDR0 = uCmdRegVal;

	IntDisable();	
#else
	RETAILMSG(TRUE,(TEXT("CMDPROCESSING TYPE is invalid, Please Check Header Definition\n")));
	return FALSE;
#endif

	
}


/**
*	@fn	FIMGSE2D::SetRopEtype(G2D_ROP_TYPE eRopType)
*	@note	Set Ternary Raster Operation
*	@note	Only support 7 raster operation (most used Rop)
*/
void FIMGSE2D::SetRopEtype(G2D_ROP_TYPE eRopType)
{
	DWORD uRopVal;

	uRopVal =
		(eRopType == ROP_SRC_ONLY) ? G2D_ROP_SRC_ONLY :
		(eRopType == ROP_DST_ONLY) ? G2D_ROP_DST_ONLY :
		(eRopType == ROP_PAT_ONLY) ? G2D_ROP_PAT_ONLY :
		(eRopType == ROP_SRC_AND_DST) ? G2D_ROP_SRC_AND_DST:
		(eRopType == ROP_SRC_AND_PAT) ? G2D_ROP_SRC_AND_PAT :		
		(eRopType == ROP_DST_AND_PAT) ? G2D_ROP_DST_AND_PAT :
		(eRopType == ROP_SRC_OR_DST) ? G2D_ROP_SRC_OR_DST :
		(eRopType == ROP_SRC_OR_PAT) ? G2D_ROP_SRC_OR_PAT :		
		(eRopType == ROP_DST_OR_PAT) ? G2D_ROP_DST_OR_PAT :		
		(eRopType == ROP_SRC_XOR_DST) ? G2D_ROP_SRC_XOR_DST :
		(eRopType == ROP_SRC_XOR_PAT) ? G2D_ROP_SRC_XOR_PAT :
		(eRopType == ROP_DST_XOR_PAT) ? G2D_ROP_DST_XOR_PAT :
	 	G2D_ROP_SRC_ONLY;

	SetRopValue(uRopVal); 

}



void FIMGSE2D::SetStencilKey(DWORD uIsColorKeyOn, DWORD uIsInverseOn, DWORD uIsSwapOn)
{	
	CheckFifo(1);
	m_pG2DReg->COLORKEY_CNTL = ((uIsColorKeyOn&1)<<31)|((uIsInverseOn&1)<<23)|(uIsSwapOn&1);
}

void FIMGSE2D::SetStencilMinMax(DWORD uRedMin, DWORD uRedMax, DWORD uGreenMin, DWORD uGreenMax, DWORD uBlueMin, DWORD uBlueMax)
{
	CheckFifo(2);
	m_pG2DReg->COLORKEY_DR_MIN = ((uRedMin&0xff)<<16)|((uGreenMin&0xff)<<8)|(uBlueMin&0xff);
	m_pG2DReg->COLORKEY_DR_MAX = ((0xffU<<24)|(uRedMax&0xff)<<16)|((uGreenMax&0xff)<<8)|(uBlueMax&0xff);	
}

void FIMGSE2D::SetColorExpansionMethod(bool bIsScr2Scr)
{
	m_bIsScr2Scr  = (bIsScr2Scr) ? 1 :	0;
}

void FIMGSE2D::BlendingOut(DWORD uSrcData, DWORD uDstData, BYTE ucAlphaVal, bool bFading, BYTE ucFadingOffset, DWORD *uBlendingOut)
{

	DWORD uSrcRed, uSrcGreen, uSrcBlue;
	DWORD uDstRed, uDstGreen, uDstBlue;
	DWORD uBldRed, uBldGreen, uBldBlue;	
	
	uSrcRed= (uSrcData & 0x00ff0000)>>16;  // Mask R
	uSrcGreen = (uSrcData & 0x0000ff00)>>8;	 // Mask G
	uSrcBlue = uSrcData & 0x000000ff;		 // Mask B

	uDstRed = (uDstData & 0x00ff0000)>>16; // Mask R
	uDstGreen = (uDstData & 0x0000ff00)>>8;  // Mask G
	uDstBlue = uDstData & 0x000000ff;		 // Mask B

	if(bFading) {
		uBldRed= ((uSrcRed*(ucAlphaVal+1))>>8) + ucFadingOffset; // R output
		uBldGreen= ((uSrcGreen*(ucAlphaVal+1))>>8) + ucFadingOffset; // G output
		uBldBlue= ((uSrcBlue*(ucAlphaVal+1)>>8)) + ucFadingOffset; // B output
		if(uBldRed>=256) uBldRed=255;
		if(uBldGreen>=256) uBldGreen=255;
		if(uBldBlue>=256) uBldBlue=255;
	}
	else {
		uBldRed= ((uSrcRed*(ucAlphaVal+1)) + (uDstRed*(256-ucAlphaVal)))>>8; // R output
		uBldGreen= ((uSrcGreen*(ucAlphaVal+1)) + (uDstGreen*(256-ucAlphaVal)))>>8; // G output
		uBldBlue= ((uSrcBlue*(ucAlphaVal+1)) + (uDstBlue*(256-ucAlphaVal)))>>8; // B output
	}

	*uBlendingOut = (uBldRed<<16) | (uBldGreen<<8) | uBldBlue;
}


void FIMGSE2D::Convert24bpp(DWORD uSrcData, EGPEFormat eBpp, bool bSwap, DWORD *uConvertedData)
{

	DWORD uRed, uGreen, uBlue;
	
	switch(eBpp) {
/*		case  ARGB8: // 15 bit color mode
			if(bSwap == 1) {  // pde_state == 2(BitBlt)
				uRed = uSrcData & 0x00007c00;  // R
				uGreen = uSrcData & 0x000003e0;  // G
				uBlue = uSrcData & 0x0000001f;  // B
			
				*uConvertedData = uRed<<9 | uGreen<<6 | uBlue<<3; // SUM
			}
			else { //hsel = 0
				uRed = uSrcData & 0x7c000000;
				uGreen = uSrcData & 0x03e00000;
				uBlue = uSrcData & 0x001f0000;

				*uConvertedData = uRed>>7 | uGreen>>10 | uBlue>>13;
			} 
			break;*/
		case gpe16Bpp : // 16 bit color mode
			if(bSwap == 1) {
				uRed = uSrcData & 0x0000f800;
				uGreen = uSrcData & 0x000007e0;
				uBlue = uSrcData & 0x0000001f;

				*uConvertedData = uRed<<8 | uGreen<<5 | uBlue<<3;
			}
			else {
				uRed = uSrcData & 0xf8000000;
				uGreen = uSrcData & 0x07e00000;
				uBlue = uSrcData & 0x001f0000;

				*uConvertedData = uRed>>8 | uGreen>>11 | uBlue>>13;
			}
			break;	

/*		case RGB18 : // 18 bit color mode
			uRed = uSrcData & 0x0003f000;
			uGreen = uSrcData & 0x00000fc0;
			uBlue = uSrcData & 0x0000003f;
	
			*uConvertedData = uRed<<6 | uGreen<<4 | uBlue<<2;
			break;

		case RGB24 : // 24 bit color mode
			*uConvertedData = uSrcData;
			break;*/
	} // End of switch
} // End of g2d_cvt24bpp function


void FIMGSE2D::GetRotateCoordinate(DWORD uDstX, DWORD uDstY, DWORD uOrgX, DWORD uOrgY, DWORD uRType, DWORD *uRsltX, DWORD *uRsltY) 
{

	switch(uRType) {
		case  1 : // No Rotate. bypass.
			*uRsltX = uDstX;
			*uRsltY = uDstY;
			break;
		case  2 : // 90 degree Rotation
			*uRsltX = uOrgX + uOrgY - uDstY;
			*uRsltY = uDstX - uOrgX + uOrgY;			
			break;
		case  4 : // 180 degree Rotation
			*uRsltX = 2*uOrgX - uDstX;
			*uRsltY = 2*uOrgY - uDstY;
			break;
		case  8 : // 270 degree Rotation
			*uRsltX = uDstY + uOrgX - uOrgY;
			*uRsltY = uOrgX + uOrgY - uDstX;
			break;
		case 16 : // X-flip
			*uRsltX = uDstX;
			*uRsltY = 2*uOrgY - uDstY;
			break;
		case 32 : // Y-flip
			*uRsltX = 2*uOrgX - uDstX;
			*uRsltY = uDstY;		
			break;
		default :
			Assert(0);
			break;
	}
}



BOOL FIMGSE2D::InitializeInterrupt(void)
{
	DWORD dwIRQ;

	dwIRQ = IRQ_2D;					// 2D Accelerator IRQ
	m_dwSysIntr2D = SYSINTR_UNDEFINED;
	m_hInterrupt2D = NULL;

	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIRQ, sizeof(DWORD), &m_dwSysIntr2D, sizeof(DWORD), NULL))
	{
		m_dwSysIntr2D = SYSINTR_UNDEFINED;
		return FALSE;
	}
	RETAILMSG(1, (TEXT("2D Sysintr : %d\r\n"),m_dwSysIntr2D));

	m_hInterrupt2D = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(NULL == m_hInterrupt2D)
	{
		return FALSE;
	}

	if (!(InterruptInitialize(m_dwSysIntr2D, m_hInterrupt2D, 0, 0)))
	{
		return FALSE;
	}	
	return TRUE;
}

void FIMGSE2D::DeinitInterrupt(void)
{
	if (m_dwSysIntr2D != SYSINTR_UNDEFINED)
	{
		InterruptDisable(m_dwSysIntr2D);
	}

	if (m_hInterrupt2D != NULL)
	{
		CloseHandle(m_hInterrupt2D);
	}

	if (m_dwSysIntr2D != SYSINTR_UNDEFINED)
	{
		KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwSysIntr2D, sizeof(DWORD), NULL, 0, NULL);
	}	
}

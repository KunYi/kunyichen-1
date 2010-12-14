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

Notes:
--*/
#include <windows.h>
#include <types.h>
#include <winddi.h>
#include <emul.h>
#include <ceddk.h>

#include <ddraw.h>
#include <ddrawi.h>
#include <ddpguids.h>
#include <dvp.h>
#include <ddgpe.h>
#include <ddhfuncs.h>
#include "ddgpeusr.h"
#include <image_cfg.h>
#include <S3C2450.h>
#include <display.h>
#include "S3C2450DISP.h"
#if (BSP_TYPE == BSP_SMDK2450)
#include "fimgse2d.h"
#endif
#define G2D_MSG		(FALSE)

#define G2D_ACCELERATE	(FALSE)			//< If you want to use 2D HW for GDI, set this to "TRUE", if not, set to "FALSE"
#define G2D_TRY_CBLT	(FALSE)		//< Try to bitblt from cached source surface to non cached destinatino surface, this do cache flush

/// For using Physically Linear Surface on System Memory to wide 2D HW usage.
/// 2D HW need physically contiguous memory, and its address. 
/// This will consume System Memory and allocate Physically and Virtually contiguous memory.
/// So if system has small memory, allocation may fail.
/// Then 2D HW will not work for that memory.
/// BUGBUG: in Media Player, Occasionly PACSurf object cannot be bitblted correctly.
#define USE_PACSURF		(TRUE)		
#define PAC_ALLOCATION_BOUNDARY	(160*120*2)	//(320*240*2)		//<  PACSurf creation request is processed only for the surface has over QVGA 16bpp size

#define G2D_BLT_OPTIMIZE	(FALSE)				//< This option will enable above two optimization method. This can increase 2D processing overhead.
#define G2D_COMPROMISE_LIMIT	(28800)		//< Transferring below this size(byte) using HW will be poor than using SW. so we will use software 2D flow under this size transfer request.
#define	G2D_BYPASS_HW_STRETCHBLT	(TRUE)		//< HW Stretchblt algorithm differs from MS'SW Stretching BLT algorithms, 
												//< So, CETK 218, 219 can fails.


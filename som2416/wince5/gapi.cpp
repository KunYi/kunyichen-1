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

Module Name:	gapi.cpp

Abstract:	GAPI Interface Implementation. this will be deprecated.

Functions:	GetGameXInfo

Notes:

--*/

#include "precomp.h"
#include <gxinfo.h>

int S3C2450DISP::GetGameXInfo(
    ULONG iEsc,
    ULONG cjIn,
    PVOID pvIn,
    ULONG cjOut,
    PVOID pvOut
    )
{
    int     RetVal = 0;     // Default not supported
    GXDeviceInfo * pgxoi;

    // GAPI only support P8, RGB444, RGB555, RGB565, and RGB888
    if ((cjOut >= sizeof(GXDeviceInfo)) && (pvOut != NULL)
        && (m_pMode->Bpp == 8 || m_pMode->Bpp == 16 || m_pMode->Bpp == 24 || m_pMode->Bpp == 32))
    {
        if (((GXDeviceInfo *) pvOut)->idVersion == kidVersion100)
        {
            pgxoi = (GXDeviceInfo *) pvOut;
            pgxoi->idVersion = kidVersion100;
            pgxoi->pvFrameBuffer = (void *)m_pPrimarySurface->Buffer();
            pgxoi->cbStride = m_pPrimarySurface->Stride();
            pgxoi->cxWidth = m_pPrimarySurface->Width();
            pgxoi->cyHeight = m_pPrimarySurface->Height();
    
            if (m_pMode->Bpp == 8)
            {
                pgxoi->cBPP = 8;
                pgxoi->ffFormat = kfPalette;
            }
            else if (m_pMode->Bpp == 16)
            {
                pgxoi->cBPP = 16;
                pgxoi->ffFormat= kfDirect | kfDirect565;
            }
            else if (m_pMode->Bpp == 24)
            {
                pgxoi->cBPP = 24;
                pgxoi->ffFormat = kfDirect | kfDirect888;
            }
            else
            {
                pgxoi->cBPP = 32;
                pgxoi->ffFormat = kfDirect | kfDirect888;
            }

            pgxoi->vkButtonUpPortrait = VK_UP;
            pgxoi->vkButtonUpLandscape = VK_LEFT;
            pgxoi->vkButtonDownPortrait = VK_DOWN;
            pgxoi->vkButtonDownLandscape = VK_RIGHT;
            pgxoi->vkButtonLeftPortrait = VK_LEFT;
            pgxoi->vkButtonLeftLandscape = VK_DOWN;
            pgxoi->vkButtonRightPortrait = VK_RIGHT;
            pgxoi->vkButtonRightLandscape = VK_UP;
            pgxoi->vkButtonAPortrait = 0xC3;            // far right button
            pgxoi->vkButtonALandscape = 0xC5;            // record button on side
            pgxoi->vkButtonBPortrait = 0xC4;            // second from right button
            pgxoi->vkButtonBLandscape = 0xC1;
            pgxoi->vkButtonCPortrait = 0xC5;            // far left button
            pgxoi->vkButtonCLandscape = 0xC2;            // far left button
            pgxoi->vkButtonStartPortrait = 134;            // action button
            pgxoi->vkButtonStartLandscape = 134;
            pgxoi->ptButtonUp.x = 120;
            pgxoi->ptButtonUp.y = 330;
            pgxoi->ptButtonDown.x = 120;
            pgxoi->ptButtonDown.y = 390;
            pgxoi->ptButtonLeft.x = 90;
            pgxoi->ptButtonLeft.y = 360;
            pgxoi->ptButtonRight.x = 150;
            pgxoi->ptButtonRight.y = 360;
            pgxoi->ptButtonA.x = 180;
            pgxoi->ptButtonA.y = 330;
            pgxoi->ptButtonB.x = 210;
            pgxoi->ptButtonB.y = 345;
            pgxoi->ptButtonC.x = -50;
            pgxoi->ptButtonC.y = 0;
            pgxoi->ptButtonStart.x = 120;
            pgxoi->ptButtonStart.y = 360;
            pgxoi->pvReserved1 = (void *) 0;
            pgxoi->pvReserved2 = (void *) 0;
            RetVal = 1;

        }
        else
        {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = -1;
        }
    }
    else 
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        RetVal = -1;
    }

    return(RetVal);
}

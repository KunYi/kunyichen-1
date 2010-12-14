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

#ifndef __GPEDDHALUSER_H__
#define __GPEDDHALUSER_H__

// some macros to help in debugging

#define DEBUGENTER(func) 										\
	{															\
		DEBUGMSG(GPE_ZONE_ENTER,(TEXT("Entering function %s\r\n"),TEXT(#func)));		\
	}

#define DEBUGLEAVE(func) 										\
	{															\
		DEBUGMSG(GPE_ZONE_ENTER,(TEXT("Leaving function %s\r\n"),TEXT( #func )));	\
	}


// HARDWARE-SPECIFIC Required #defines
#define IN_VBLANK           (g_pGPE->InVBlank())
#define IS_BUSY				(g_pGPE->IsBusy())



#endif //__GPEDDHALUSER_H__
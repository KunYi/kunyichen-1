;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;******************************************************************************
;*
;* System On Chip(SOC)
;*
;* Copyright (c) 2002 Software Center, Samsung Electronics, Inc.
;* All rights reserved.
;*
;* This software is the confidential and proprietary information of Samsung 
;* Electronics, Inc("Confidential Information"). You Shall not disclose such 
;* Confidential Information and shall use it only in accordance with the terms 
;* of the license agreement you entered into Samsung.
;*
;******************************************************************************

    INCLUDE kxarm.h

	TEXTAREA

	LEAF_ENTRY SetOverlayPosReg

	STMIA	r0!, {r1-r2}

	mov	pc, lr

	END


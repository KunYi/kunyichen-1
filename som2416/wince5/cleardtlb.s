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
;  Copyright (c) 1998, 1999 ARM Limited
;  All Rights Reserved
;
;-------------------------------------------------------------------------------
;
;  File:  cleardtlb.s
;
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s

        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALClearDTLB
;
;  Flush and invalidate the data TLB
;
        LEAF_ENTRY OALClearDTLB

        mov     r0, #0
        mcr     p15, 0, r0, c8, c6, 0   ; flush data TLB

        RETURN

        END

;-------------------------------------------------------------------------------
        

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
;-------------------------------------------------------------------------------
;
;  File:  cleandclines.s
;
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s
        INCLUDE oal_cache.inc

        IMPORT g_oalCacheInfo

        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALClearDCacheLines
;
        LEAF_ENTRY OALCleanDCacheLines

        ldr     r2, =g_oalCacheInfo
        ldr     r3, [r2, #L1DLineSize]

10      mcr     p15, 0, r0, c7, c10, 1          ; clean entry
        add     r0, r0, r3                      ; move to next entry
        subs    r1, r1, r3
        bgt     %b10                            ; loop while > 0 bytes left

        mov     r2, #0
        mcr     p15, 0, r2, c7, c10, 4          ; drain write buffer

        RETURN

        END

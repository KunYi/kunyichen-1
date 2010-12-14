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
; oemabort.s - header file for the data abort veneer
;
; This file selects options suitable for Windows CE's use of
; the data abort veneer.
;

        OPT     2       ; disable listing
        INCLUDE kxarm.h
        OPT     1       ; reenable listing
		
        TEXTAREA
        IMPORT DataAbortHandler
	
        LEAF_ENTRY OEMDataAbortHandler
        b       DataAbortHandler
        ENTRY_END

        END

; EOF oemabort.s

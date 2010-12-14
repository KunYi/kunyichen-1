; Module Name: usb.s                                           

	OPT	2	; disable listing
	INCLUDE kxarm.h
	INCLUDE s3c2450.inc
	OPT	1	; reenable listing
	OPT	128	; disable listing of macro expansions


    TEXTAREA
	IMPORT  IsrUsbd
	IMPORT  IsrPowerButton
; ---------------------------------------------------------------------
; ---------------------------------------------------------------------        
    LEAF_ENTRY IsrHandler

;	sub	sp,sp,#4	;decrement sp(to store jump address)
	sub lr, lr, #4
	stmfd   sp!, {r0-r12,lr}
	mov	r0, lr

	ldr		r9, =vINTPND1
	ldr		r8, [r9]
	cmp		r8, #0x1
	beq		%F2

	bl	IsrUsbd
	ldmfd   sp!, {r0-r12,lr}
	movs pc, lr

2	bl	IsrPowerButton
	ldmfd   sp!, {r0-r12,lr}
	movs pc, lr

    ENDP    ; |IsrHandler|

    END    

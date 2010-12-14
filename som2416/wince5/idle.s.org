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


	INCLUDE kxarm.h
		
	TEXTAREA

;------------------------------------------------------------------------------
;
;  Function:  void MMU_WaitForInterrupt(void)
;
;  Enter Idle mode.
;
;	LEAF_ENTRY OALTimerSetCompare
;    mov  pc, lr          ; return

;void MMU_WaitForInterrupt(void)
   EXPORT MMU_WaitForInterrupt 
MMU_WaitForInterrupt
   [{TRUE}
   mov  r0,#0x0    
   mcr  p15,0,r0,c7,c0,4
   mov pc,lr
   ;bx		lr
   ]
	[{FALSE}
	stmfd	sp!, {r4-r13}
	
	;;step1 : I-bit mask
   	mrs  	r0,cpsr
   	mov  	r1,r0
   	orr  	r1,r1,#(0xc0)
   	msr  	cpsr_cxsf,r1

	;;step2 : do
	mov 	r4,#0
	mov		r5,#1	

startDo
	;;2nd
	cmp 	r4,	#1
	bne 	forCache
	   		
	;;step2 : PWRDN register set

	mov 	r4,#0x0 
;;;;;;;;; WFI ;;;;;;;;;;;;;;

	ldr 	r0,	=0x90C00024
	ldr 	r1,	=0x80
	str 	r1,	[r0]
	
	;;step3 : MMU interrupt enable
  	mov  	r0,#0x0    
	mcr  	p15,0,r0,c7,c0,4
;	ldr		r0,=vTICINT0
;	ldr		r1,[r0]
;	bic		r1,r1,#(0x1<<7)
;	str		r1,[r0]

;;;;;;;;; COMMAND ;;;;;;;;;;
;	ldr 	r0,	=0xb0C00020
;	ldr 	r1,	=0x10000
;	str 	r1,	[r0]


	;;1st - to get 20cycles during the exit self-refresh
	;;      cmp-1cycle, bne-3cycle
	;;ALIGN   4	
forCache
	cmp r4,#0
	bne incFlag
	[{FALSE}
	ldr 	r0,	=0x90A00000  ;; clear srcpnd 
	ldr 	r1,	=0x0
	str 	r1,	[r0]

	ldr 	r0,	=0x90A00010  ;; clear intpnd 
	ldr 	r1,	=0x0
	str 	r1,	[r0]

	ldr 	r0,	=0x90A00018  ;; clear subsrcpnd 
	ldr 	r1,	=0x0
	str 	r1,	[r0]
	]

	[{TRUE}
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	nop
	nop
	]
	cmp 	r5,	#0x0
	bne		exitStop
	mov		r4, #2
exitStop
	mov		r5,	#0x0	
	
	
incFlag
	add		r4,	r4,#1
	cmp		r4,	#2	;;while(r4<2);
	
	blt	startDo


   	;;clear interrupt pending
   	ldr	r0,=vINTPND
  	ldr	r1,[r0]
  	str	r1,[r0]

   	ldr	r0,=vEINTPEND
   	ldr	r1,[r0]
   	str	r1,[r0]

	mrs  	r0,cpsr
   	mov  	r1,r0
   	bic  	r1,r1,#(0xc0)
   	msr  	cpsr_cxsf,r1
	
	ldmfd	sp!, {r4-r13}
	mov 	pc, lr	
	]   




;------------------------------------------------------------------------------
;
;  Function:  void Nop(void)
;
;  NOP function.
;
	LEAF_ENTRY Nop
	
	;stmfd	sp!,{r1 - r11}

	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	
	;ldmfd	sp!, {r1 - r11}
	mov  pc, lr		  ; return





	END

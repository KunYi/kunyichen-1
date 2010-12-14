;
;  Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
;  Use of this source code is subject to the terms of the Microsoft end-user
;  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
;  If you did not accept the terms of the EULA, you are not authorized to use
;  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
;  install media.
;
;------------------------------------------------------------------------------
;
;   File:  startup.s
;
;   Kernel startup routine for Samsung SMDK2450 board. Hardware is
;   initialized in boot loader - so there isn't much code at all.
;
;------------------------------------------------------------------------------

        INCLUDE kxarm.h
		INCLUDE s3c2450.inc

		IMPORT  OALClearUTLB
		IMPORT  OALFlushICache
		IMPORT  OALFlushDCache
;-------------------------------------------------------------------------------


; Pre-defined constants.

USERMODE	EQU    0x10
FIQMODE		EQU    0x11
IRQMODE	EQU    0x12
SVCMODE	EQU    0x13
ABORTMODE	EQU    0x17
UNDEFMODE	EQU    0x1b
MODEMASK	EQU    0x1f
NOINT		EQU    0xc0

; Amount of memory (in bytes) allocated for stacks

Len_FIQ_Stack	EQU    256
Len_IRQ_Stack	EQU    256
Len_ABT_Stack	EQU    256
Len_UND_Stack	EQU    256
Len_SVC_Stack	EQU    1024

; Offsets will be loaded as immediate values.
; Offsets must be 8 byte aligned.

Offset_FIQ_Stack	EQU    0
Offset_IRQ_Stack	EQU    Offset_FIQ_Stack + Len_FIQ_Stack
Offset_ABT_Stack	EQU    Offset_IRQ_Stack + Len_IRQ_Stack
Offset_UND_Stack	EQU    Offset_ABT_Stack + Len_ABT_Stack
Offset_SVC_Stack	EQU    Offset_UND_Stack + Len_UND_Stack

; Stack locations.

FIQStack		EQU    (top_of_stacks - 0x0)				; 0x33ffff00 ~
IRQStack		EQU    (FIQStack   - Offset_FIQ_Stack)	; 0x33fffe00 ~
AbortStack	EQU    (IRQStack   - Offset_IRQ_Stack)	; 0x33fffd00 ~
UndefStack	EQU    (AbortStack - Offset_ABT_Stack)	; 0x33fffc00 ~
SVCStack	EQU    (UndefStack - Offset_UND_Stack)	; 0x33fffb00 ~
UserStack	EQU    (SVCStack   - Offset_SVC_Stack)	; 0x33fff700 ~

;-------------------------------------------------------------------------------

;MemoryMap	  EQU	0x2a4
;BANK_SIZE	   EQU	 0x00100000	  ; 1MB per bank in MemoryMap array
BANK_SHIFT	  EQU	 20


;   Define RAM space for the Page Tables:
;
PHYBASE		EQU		0x30000000	  ; physical start
PTs			EQU		0x30010000	  ; 1st level page table address (PHYBASE + 0x10000)
					; save room for interrupt vectors.

;-------------------------------------------------------------------------------

; Data Cache Characteristics.
;
	[ BSP_TYPE = BSP_SMDK2443
DCACHE_LINES_PER_SET_BITS       EQU     6
DCACHE_LINES_PER_SET            EQU     64
DCACHE_NUM_SETS                 EQU     8
DCACHE_SET_INDEX_BIT            EQU     (32 - DCACHE_LINES_PER_SET_BITS)
DCACHE_LINE_SIZE                EQU     32


SLEEPDATA_BASE_VIRTUAL          EQU	0xAC028000		; keep in sync w/ config.bib
	]

	[ BSP_TYPE = BSP_SMDK2450
DCACHE_LINES_PER_SET_BITS       EQU     2
DCACHE_LINES_PER_SET            EQU     4
DCACHE_NUM_SETS                 EQU     64
DCACHE_SET_INDEX_BIT            EQU     (32 - DCACHE_LINES_PER_SET_BITS)
DCACHE_LINE_SIZE                EQU     32

SLEEPDATA_BASE_VIRTUAL          EQU	0xA0028000		; keep in sync w/ config.bib
	]


SLEEPDATA_BASE_PHYSICAL         EQU	0x30028000

SleepState_Data_Start		EQU     (0)

SleepState_WakeAddr    		EQU     (SleepState_Data_Start		    )
SleepState_MMUCTL               EQU     (SleepState_WakeAddr    + WORD_SIZE )
SleepState_MMUTTB       	EQU     (SleepState_MMUCTL  	+ WORD_SIZE )
SleepState_MMUDOMAIN    	EQU     (SleepState_MMUTTB  	+ WORD_SIZE )
SleepState_SVC_SP       	EQU     (SleepState_MMUDOMAIN   + WORD_SIZE )
SleepState_SVC_SPSR     	EQU     (SleepState_SVC_SP  	+ WORD_SIZE )
SleepState_FIQ_SPSR     	EQU     (SleepState_SVC_SPSR    + WORD_SIZE )
SleepState_FIQ_R8       	EQU     (SleepState_FIQ_SPSR    + WORD_SIZE )
SleepState_FIQ_R9       	EQU     (SleepState_FIQ_R8  	+ WORD_SIZE )
SleepState_FIQ_R10      	EQU     (SleepState_FIQ_R9  	+ WORD_SIZE )
SleepState_FIQ_R11      	EQU     (SleepState_FIQ_R10 	+ WORD_SIZE )
SleepState_FIQ_R12      	EQU     (SleepState_FIQ_R11 	+ WORD_SIZE )
SleepState_FIQ_SP       	EQU     (SleepState_FIQ_R12 	+ WORD_SIZE )
SleepState_FIQ_LR       	EQU     (SleepState_FIQ_SP  	+ WORD_SIZE )
SleepState_ABT_SPSR     	EQU     (SleepState_FIQ_LR  	+ WORD_SIZE )
SleepState_ABT_SP       	EQU     (SleepState_ABT_SPSR    + WORD_SIZE )
SleepState_ABT_LR       	EQU     (SleepState_ABT_SP  	+ WORD_SIZE )
SleepState_IRQ_SPSR     	EQU     (SleepState_ABT_LR  	+ WORD_SIZE )
SleepState_IRQ_SP       	EQU     (SleepState_IRQ_SPSR    + WORD_SIZE )
SleepState_IRQ_LR       	EQU     (SleepState_IRQ_SP  	+ WORD_SIZE )
SleepState_UND_SPSR     	EQU     (SleepState_IRQ_LR  	+ WORD_SIZE )
SleepState_UND_SP       	EQU     (SleepState_UND_SPSR    + WORD_SIZE )
SleepState_UND_LR       	EQU     (SleepState_UND_SP  	+ WORD_SIZE )
SleepState_SYS_SP       	EQU     (SleepState_UND_LR  	+ WORD_SIZE )
SleepState_SYS_LR       	EQU     (SleepState_SYS_SP  	+ WORD_SIZE )

SleepState_Data_End     	EQU     (SleepState_SYS_LR	+ WORD_SIZE )

SLEEPDATA_SIZE		    	EQU     ((SleepState_Data_End - SleepState_Data_Start) / 4)


;---------------------------------------------------------------------------
;
; Macro to feed the LED Reg (The GPIO) with the value desired for debugging.
; Uses physical address
;
; GPFDAT [7:4] is assigned to LEDs.

	MACRO
	LED_ON	$data

	LDR	r10, =GPFUDP
	LDR     r11, =0x5500	;Pull-Up-Down Disable
	STR	r11, [r10]
	
	LDR	r10, =GPFCON
	LDR	r11, =0x5500	; GPF[7:4] Output .
	STR	r11, [r10]
	LDR	r10, =GPFDAT
	LDR	r11, =$data
	MOV     r11, r11, lsl #4	; [7:4]
  	STR	r11, [r10]
  	MEND


; LED_ON using virtual address
;
	MACRO
	VLED_ON	$data
	
	LDR	r10, =vGPFUDP
	LDR     r11, =0x5500	;Pull-Up-Down Disable
	STR	r11, [r10]
	
	LDR	r10, =vGPFCON
	LDR	r11, =0x5500	; GPF[7:4] Output .
	STR	r11, [r10]
	LDR	r10, =vGPFDAT
	LDR	r11, =$data
	MOV     r11, r11, lsl #4	; [7:4]
  	STR	r11, [r10]
    	MEND
   
;---------------------------------------------------------------------------



        TEXTAREA

	IMPORT  Max1718_Init
	IMPORT  Max1718_Set

        IMPORT  main

       ; Include memory configuration file with g_oalAddressTable

        INCLUDE oemaddrtab_cfg.inc   

        LEAF_ENTRY StartUp

        ; Compute the OEMAddressTable's physical address and 
        ; load it into r0. KernelStart expects r0 to contain
        ; the physical address of this table. The MMU isn't 
        ; turned on until well into KernelStart.  

	; Jump over power-off code. 
1	b		ResetHandler
	b %B1		;HandlerUndef	;handler for Undefined mode
	b %B1		;HandlerSWI		;handler for SWI interrupt
	b %B1		;HandlerPabort	;handler for PAbort
	b %B1		;HandlerDabort	;handler for DAbort
	b %B1		;				;reserved
	b %B1		;HandlerIRQ		;handler for IRQ interrupt 
	b %B1		;HandlerFIQ		;handler for FIQ interrupt

ResetHandler
    ; Make sure that TLB & cache are consistent
    mov     r0, #0
    mcr     p15, 0, r0, c8, c7, 0           ; flush both TLB
    mcr     p15, 0, r0, c7, c5, 0           ; invalidate instruction cache
    mcr     p15, 0, r0, c7, c6, 0           ; invalidate data cache

    ldr	r0, =WTCON       ; disable the watchdog timer.
    mov	r1,#0         
    str	r1, [r0]


	;;;;;;;;;;;; set voltage test
;	bl		Max1718_Init
;	mov		r0, #0x1
;	mov		r1, #125
;	mov		r1, #130
;	mov		r1, #135
;	bl		Max1718_Set


;    ldr	r0, =EBICON		; EBI
;    ldr	r1, =EBICON_VAL			; Refer s3c2450.inc
;    str	r1,[r0]

    ldr r0, = GPACDH
    ldr r1, = 0x1AA8A      
    str r1, [r0]

    ldr r0, = GPFCON
    ldr r1, = 0x5500      
    str r1, [r0]


	[ BSP_TYPE = BSP_SMDK2443 
	      ldr	r0, =INTMSK      ; mask all first-level interrupts.
	    ldr	r1, =0xffffffff
	    str	r1, [r0]

	    ldr	r0, =INTSUBMSK   ; mask all second-level interrupts.
	    ldr	r1, =0x1fffffff
	    str	r1, [r0]

	    ldr r0, = INTMOD
	    mov r1, #0x0			; set all interrupt as IRQ
	    str r1, [r0]

	  ]

	  [ BSP_TYPE = BSP_SMDK2450 
	    ldr	r0, =INTMSK1      ; mask all first-level interrupts.
	    ldr	r1, =0xffffffff
	    str	r1, [r0]

	    ldr	r0, =INTMSK2      ; mask all Second-level interrupts.
	    ldr	r1, =0xffffffff
	    str	r1, [r0]
	   
	    ldr	r0, =INTSUBMSK   ; mask all second-level interrupts.
	    ldr	r1, =0x1fffffff
	    str	r1, [r0]

	    ldr r0, = INTMOD1
	    mov r1, #0x0			; set all interrupt as IRQ
	    str r1, [r0]

	    ldr r0, = INTMOD2
	    mov r1, #0x0			; set all interrupt as IRQ
	    str r1, [r0]
	]    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; When the Eboot is already, No need to set again.
;; IF set as "FALSE", The FCLK,HCLK,PCLK Clock will detect automatically in OS.
;; IF set as "TRUE", The FCLK,HCLK,PCLK Clock and Memory setting will be set again here and detect automatically in OS.

 	[ CHANGE_CLK_EBOOT=1	; Refer the S3c2450.inc.

	ldr		r0,=CLKDIV0			;	Set Clock Divider
	ldr		r1,[r0]
	bic		r1,r1,#0x37		; clear HCLKDIV, PREDIV, PCLKDIV
	bic		r1,r1,#(0xf<<9) ; clear ARMCLKDIV
	ldr		r2,=((Startup_ARMCLKdiv<<9)+(Startup_PREdiv<<4)+(Startup_PCLKdiv<<2)+(Startup_HCLKdiv)) 
	orr		r1,r1,r2
	str		r1,[r0]			

	ldr		r0,=LOCKCON0		;	Set lock time of MPLL. added by junon
	mov		r1,#0xe10			;	Fin = 12MHz - 0x800, 16.9844MHz - 0xA00
	str		r1,[r0]	

	ldr		r0,=LOCKCON1		;	Set lock time of EPLL. added by junon
	mov		r1,#0x800			;	Fin = 12MHz - 0x800, 16.9844MHz - 0xA00
	str		r1,[r0]	

	ldr		r0,=MPLLCON			;	Set MPLL
	[ BSP_TYPE = BSP_SMDK2443 	
	ldr		r1,=((0<<24)+(Startup_Mdiv<<16)+(Startup_Pdiv<<8)+(Startup_Sdiv))
	]
	[ BSP_TYPE = BSP_SMDK2450 
	ldr		r1,=((0<<24)+(Startup_Mdiv<<14)+(Startup_Pdiv<<5)+(Startup_Sdiv))
	]

	str		r1,[r0]			

  	ldr		r0,=EPLLCON			;	Set EPLL
	ldr		r1,=((0<<24)+(Startup_EMdiv<<16)+(Startup_EPdiv<<8)+(Startup_ESdiv))
	str		r1,[r0]			

	ldr		r0,=CLKSRC			;	Select MPLL clock out for SYSCLK
	ldr		r1,[r0]
	orr		r1,r1,#0x50
	str		r1,[r0]			
	
    bl		MMU_SetAsyncBusMode


	bl		InitSSMC				
	]	; End of PLL setting
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

			
;;;;;;;;;;;;;;;;;; set clkout1 - HCLK
	ldr		r0,=MISCCR
	ldr		r1,[r0]
	bic		r1,r1, #0x770
	orr		r1,r1,#0x320
	str		r1,[r0]
	
	ldr		r0,=GPHCON
	ldr		r1,[r0]
	bic		r1,r1, #0x3C000000
	orr		r1,r1, #0x28000000
	str		r1,[r0]	
;;;;;;;;;;;;;;;;;;

	;bl loop_led

		[ {FALSE}		
	; Clear RAM.
	;
	mov 	r1,#0
	mov 	r2,#0
	mov 	r3,#0
	mov 	r4,#0
	mov 	r5,#0
	mov 	r6,#0
	mov 	r7,#0
	mov 	r8,#0
	
	ldr		r0,=0x30100000   ; Start address (physical 0x3000.0000).
	;ldr		r9,=0x00E00000   ; 64MB of RAM.
	ldr		r9,=0x03F00000   ; 64MB of RAM.
100	
	stmia	r0!, {r1-r8}
	subs	r9, r9, #32 
	bne		%B20
	]

        ; Compute physical address of the OEMAddressTable.
20	add	 r11, pc, #g_oalAddressTable - (. + 8)
	ldr	 r10, =PTs		; (r10) = 1st level page table


	; Setup 1st level page table (using section descriptor)	 
	; Fill in first level page table entries to create "un-mapped" regions
	; from the contents of the MemoryMap array.
	;
	;   (r10) = 1st level page table
	;   (r11) = ptr to MemoryMap array

	add	 r10, r10, #0x2000	   ; (r10) = ptr to 1st PTE for "unmapped space"
	mov	 r0, #0x0E		   ; (r0) = PTE for 0: 1MB cachable bufferable
	orr	 r0, r0, #0x400	  ; set kernel r/w permission
25	mov	 r1, r11		 ; (r1) = ptr to MemoryMap array

	
30	ldr	 r2, [r1], #4		; (r2) = virtual address to map Bank at
	ldr	 r3, [r1], #4		; (r3) = physical address to map from
	ldr	 r4, [r1], #4		; (r4) = num MB to map

	cmp	 r4, #0		  ; End of table?
	beq	 %f40

	ldr	 r5, =0x1FF00000
	and	 r2, r2, r5		  ; VA needs 512MB, 1MB aligned.		

	ldr	 r5, =0xFFF00000
	and	 r3, r3, r5		  ; PA needs 4GB, 1MB aligned.

	add	 r2, r10, r2, LSR #18
	add	 r0, r0, r3		  ; (r0) = PTE for next physical page

35	str	 r0, [r2], #4
	add	 r0, r0, #0x00100000	 ; (r0) = PTE for next physical page
	sub	 r4, r4, #1		  ; Decrement number of MB left 
	cmp	 r4, #0
	bne	 %b35			; Map next MB

	bic	 r0, r0, #0xF0000000	 ; Clear Section Base Address Field
	bic	 r0, r0, #0x0FF00000	 ; Clear Section Base Address Field
	b	   %b30			; Get next element
	
40	tst	 r0, #8
	bic	 r0, r0, #0x0C	   ; clear cachable & bufferable bits in PTE
	add	 r10, r10, #0x0800	   ; (r10) = ptr to 1st PTE for "unmapped uncached space"
	bne	 %b25			; go setup PTEs for uncached space
	sub	 r10, r10, #0x3000	   ; (r10) = restore address of 1st level page table

	; 1. Setup mmu to map (VA == 0) to (PA == 0x30000000).
	; 1-1. cached area.
	ldr	 r0, =PTs		; PTE entry for VA = 0
	ldr	 r1, =0x3000040E	 ; cache/buffer/rw, PA base == 0x30000000
	;ldr	 r1, =0x30000402	 ; cache/buffer/rw, PA base == 0x30000000
	str	 r1, [r0]

	; 1-2. uncached area.
	add	 r0, r0, #0x0800	 ; PTE entry for VA = 0x0200.0000 , uncached	 
	ldr	 r1, =0x30000402	 ; uncache/unbuffer/rw, base == 0x30000000
	str	 r1, [r0]
	
	; Comment:
	; The following loop is to direct map RAM VA == PA. i.e. 
	;   VA == 0x30XXXXXX => PA == 0x30XXXXXX for S3C2400
	; Fill in 8 entries to have a direct mapping for DRAM
	;
	ldr	 r10, =PTs		   ; restore address of 1st level page table
	ldr	 r0,  =PHYBASE

	add	 r10, r10, #(0x3000 / 4) ; (r10) = ptr to 1st PTE for 0x30000000

	add	 r0, r0, #0x1E	   ; 1MB cachable bufferable
	orr	 r0, r0, #0x400	  ; set kernel r/w permission
	mov	 r1, #0 
	mov	 r3, #64
45	mov	 r2, r1		  ; (r2) = virtual address to map Bank at
	cmp	 r2, #0x20000000:SHR:BANK_SHIFT
	add	 r2, r10, r2, LSL #BANK_SHIFT-18
	strlo    r0, [r2]
	add	 r0, r0, #0x00100000	 ; (r0) = PTE for next physical page
	subs     r3, r3, #1
	add	 r1, r1, #1
	bgt	 %b45

	ldr	 r10, =PTs		   ; (r10) = restore address of 1st level page table

	; The page tables and exception vectors are setup.
	; Initialize the MMU and turn it on.
	mov	 r1, #1
	mcr	 p15, 0, r1, c3, c0, 0   ; setup access to domain 0
	mcr	 p15, 0, r10, c2, c0, 0

	mcr	 p15, 0, r0, c8, c7, 0   ; flush I+D TLBs
	
	mrc     p15,0,r1,c1,c0,0
	
	orr	 r1, r1, #0x0071		 ; Enable: MMU
	orr	 r1, r1, #0x0004	 ; Enable the cache


	ldr	 r0, =VirtualStart

	cmp	 r0, #0		  ; make sure no stall on "mov pc,r0" below
	mcr	 p15, 0, r1, c1, c0, 0

	mov	 pc, r0		  ;  & jump to new virtual address
	nop


	; MMU & caches now enabled.
	;   (r10) = physcial address of 1st level page table
	;


VirtualStart

	mov	 sp, #0x80000000	; have to be modefied. refer oemaddrtab_cfg.inc, DonGo
	add	 sp, sp, #0x30000	; arbitrary initial super-page stack pointer
	
;------------------------------------------------------------
	;---------------------
	; Initialize stacks.

30
	mrs		r0, cpsr
	bic		r0, r0, #MODEMASK

;	orr		r1, r0, #UNDEFMODE | NOINT
;	msr		cpsr_cxsf, r1			; UndefMode
;	ldr		sp, =UndefStack		; UndefStack=0x33FF_5C00

;	orr		r1, r0, #ABORTMODE | NOINT
;	msr		cpsr_cxsf, r1			; AbortMode
;	ldr		sp, =AbortStack		; AbortStack=0x33FF_6000

	orr		r1, r0, #IRQMODE | NOINT
	msr		cpsr_cxsf, r1			; IRQMode
	ldr		sp, =IRQStack		; IRQStack=0x33FF_7000

;	orr		r1, r0, #FIQMODE | NOINT
;	msr		cpsr_cxsf, r1			; FIQMode
;	ldr		sp, =FIQStack			; FIQStack=0x33FF_8000

	bic		r0, r0, #MODEMASK | NOINT
	orr		r1, r0, #SVCMODE
	msr		cpsr_cxsf, r1			; SVCMode
	ldr		sp, =SVCStack		; SVCStack=0x33FF_5800
;--------------------------------------------------------------


	b	   main


        ENTRY_END 




	LEAF_ENTRY OALCPUPowerOff

;       1. Push SVC state onto our stack
	stmdb   sp!, {r4-r12}                   
	stmdb   sp!, {lr}

;       2. Save MMU & CPU Register to RAM
    ldr     r3, =SLEEPDATA_BASE_VIRTUAL     ; base of Sleep mode storage

	ldr     r2, =Awake_address              ; store Virtual return address
	str     r2, [r3], #4

	mrc     p15, 0, r2, c1, c0, 0           ; load r2 with MMU Control
	ldr     r0, =MMU_CTL_MASK               ; mask off the undefined bits
	bic     r2, r2, r0
	str     r2, [r3], #4                    ; store MMU Control data

	mrc     p15, 0, r2, c2, c0, 0           ; load r2 with TTB address.
	ldr     r0, =MMU_TTB_MASK               ; mask off the undefined bits
	bic     r2, r2, r0
	str     r2, [r3], #4                    ; store TTB address

	mrc     p15, 0, r2, c3, c0, 0           ; load r2 with domain access control.
	str     r2, [r3], #4                    ; store domain access control

	str     sp, [r3], #4                    ; store SVC stack pointer

	mrs     r2, spsr
	str     r2, [r3], #4                    ; store SVC status register

	mov     r1, #Mode_FIQ:OR:I_Bit:OR:F_Bit ; Enter FIQ mode, no interrupts
	msr     cpsr, r1
	mrs     r2, spsr
	stmia   r3!, {r2, r8-r12, sp, lr}       ; store the FIQ mode registers

	mov     r1, #Mode_ABT:OR:I_Bit:OR:F_Bit ; Enter ABT mode, no interrupts
	msr     cpsr, r1
	mrs		r0, spsr
	stmia   r3!, {r0, sp, lr}               ; store the ABT mode Registers

	mov     r1, #Mode_IRQ:OR:I_Bit:OR:F_Bit ; Enter IRQ mode, no interrupts
	msr     cpsr, r1
	mrs     r0, spsr
	stmia   r3!, {r0, sp, lr}               ; store the IRQ Mode Registers

	mov     r1, #Mode_UND:OR:I_Bit:OR:F_Bit ; Enter UND mode, no interrupts
	msr     cpsr, r1
	mrs     r0, spsr
	stmia   r3!, {r0, sp, lr}               ; store the UND mode Registers

	mov     r1, #Mode_SYS:OR:I_Bit:OR:F_Bit ; Enter SYS mode, no interrupts
	msr     cpsr, r1
	stmia   r3!, {sp, lr}                   ; store the SYS mode Registers

	mov     r1, #Mode_SVC:OR:I_Bit:OR:F_Bit ; Back to SVC mode, no interrupts
	msr     cpsr, r1

;       3. do Checksum on the Sleepdata
	ldr     r3, =SLEEPDATA_BASE_VIRTUAL	; get pointer to SLEEPDATA
	ldr     r2, =0x0
	ldr     r0, =(SLEEPDATA_SIZE-1)		; get size of data structure (in words)
30
	ldr     r1, [r3], #4
	and     r1, r1, #0x1
	mov     r1, r1, ROR #31
	add     r2, r2, r1
	subs    r0, r0, #1
	bne     %b30

	ldr     r0, =vGPIOBASE
	;;;add		r2, r2, #1				; test checksum of the Sleep data error
	str     r2, [r0, #oGSTATUS3]		; Store in Power Manager Scratch pad register

	ldr     r0, =vGPIOBASE
	ldr     r1, =0x550a
	str     r1, [r0, #oGPFCON]
	
	ldr		r1, =0x30
	str		r1, [r0, #oGPFDAT]	

;       4. Interrupt Disable 
	[ BSP_TYPE = BSP_SMDK2443 
	ldr     r0, =vINTBASE
 	mvn     r2, #0
	str     r2, [r0, #oINTMSK]
	str     r2, [r0, #oSRCPND]
	str     r2, [r0, #oINTPND]

	]
	[ BSP_TYPE = BSP_SMDK2450 
       ldr     r0, =vINTBASE1
       mvn     r2, #0
	str     r2, [r0, #oINTMSK1]
	str     r2, [r0, #oSRCPND1]
	str     r2, [r0, #oINTPND1]

       ldr     r0, =vINTBASE2
        mvn     r2, #0
	str     r2, [r0, #oINTMSK2]
	str     r2, [r0, #oSRCPND2]
	str     r2, [r0, #oINTPND2]	
	]


;;       5. Cache Flush
	bl		OALClearUTLB
	bl		OALFlushICache
	ldr     r0, = (DCACHE_LINES_PER_SET - 1)    
	ldr     r1, = (DCACHE_NUM_SETS - 1)    
	ldr     r2, = DCACHE_SET_INDEX_BIT    
	ldr     r3, = DCACHE_LINE_SIZE     
	bl		OALFlushDCache

;       6. Setting Wakeup External Interrupt(EINT0,1,2) Mode
	ldr     r0, =vGPIOBASE

	ldr     r1, =0x550a
	str     r1, [r0, #oGPFCON]

;	ldr     r1, =0x55550100
;	str     r1, [r0, #oGPGCON]

;       7. Go to Power-Off Mode
;	ldr 	r0, =vMISCCR			; hit the TLB
;	ldr		r0, [r0]
;	ldr 	r0, =vCLKCON
;	ldr		r0, [r0]

;	ldr     r0, =vREFRESH		
;	ldr     r1, [r0]		; r1=rREFRESH	
;	orr     r1, r1, #(1 << 22)

;	ldr 	r2, =vMISCCR
;	ldr		r3, [r2]
;	orr		r3, r3, #(3<<17)        ; Make sure that SCLK0:SCLK->0, SCLK1:SCLK->0, SCKE=L during boot-up 
;	bic		r3, r3, #(7<<20)
;	orr		r3, r3, #(6<<20)

	ldr     r4, =vPWRMODE
	ldr     r5, =0x2BED            ; Power Off Mode
	str     r5, [r4]		; Power Off !!
        b .


;;;	LTORG

; This point is called from EBOOT's startup code(MMU is enabled)
;       in this routine, left information(REGs, INTMSK, INTSUBMSK ...)

Awake_address

;       1. Recover CPU Registers

	ldr     r3, =SLEEPDATA_BASE_VIRTUAL		; Sleep mode information data structure
	add     r2, r3, #SleepState_FIQ_SPSR
	mov     r1, #Mode_FIQ:OR:I_Bit:OR:F_Bit		; Enter FIQ mode, no interrupts - also FIQ
	msr     cpsr, r1
	ldr     r0,  [r2], #4
	msr     spsr, r0
	ldr     r8,  [r2], #4
	ldr     r9,  [r2], #4
	ldr     r10, [r2], #4
	ldr     r11, [r2], #4
	ldr     r12, [r2], #4
	ldr     sp,  [r2], #4
	ldr     lr,  [r2], #4

	mov     r1, #Mode_ABT:OR:I_Bit			; Enter ABT mode, no interrupts
	msr     cpsr, r1
	ldr     r0, [r2], #4
	msr     spsr, r0
	ldr     sp, [r2], #4
	ldr     lr, [r2], #4

	mov     r1, #Mode_IRQ:OR:I_Bit			; Enter IRQ mode, no interrupts
	msr     cpsr, r1
	ldr     r0, [r2], #4
	msr     spsr, r0
	ldr     sp, [r2], #4
	ldr     lr, [r2], #4

	mov     r1, #Mode_UND:OR:I_Bit			; Enter UND mode, no interrupts
	msr     cpsr, r1
	ldr     r0, [r2], #4
	msr     spsr, r0
	ldr     sp, [r2], #4
	ldr     lr, [r2], #4

	mov     r1, #Mode_SYS:OR:I_Bit			; Enter SYS mode, no interrupts
	msr     cpsr, r1
	ldr     sp, [r2], #4
	ldr     lr, [r2]

	mov     r1, #Mode_SVC:OR:I_Bit					; Enter SVC mode, no interrupts - FIQ is available
	msr     cpsr, r1
	ldr     r0, [r3, #SleepState_SVC_SPSR]
	msr     spsr, r0

;       2. Recover Last mode's REG's, & go back to caller of OALCPUPowerOff()

	ldr     sp, [r3, #SleepState_SVC_SP]
	ldr     lr, [sp], #4
	ldmia   sp!, {r4-r12}
	mov     pc, lr                          ; and now back to our sponsors

	

InitSSMC

	;Set SSMC Memory parameter control registers : AMD Flash
	ldr		r0,=SMBIDCYR0
	ldr		r1,=IDCY0
	str		r1,[r0]
	
	ldr		r0,=SMBWSTRDR0
	ldr		r1,=WSTRD0
	str		r1,[r0]
	
	ldr		r0,=SMBWSTWRR0
	ldr		r1,=WSTWR0
	str		r1,[r0]
	
	ldr		r0,=SMBWSTOENR0
	ldr		r1,=WSTOEN0
	str		r1,[r0]
	
	ldr		r0,=SMBWSTWENR0
	ldr		r1,=WSTWEN0
	str		r1,[r0]
	
	ldr		r0,=SMBCR0
	ldr		r1,=(SMBCR0_2+SMBCR0_1+SMBCR0_0)
	str		r1,[r0]
	
	ldr		r0,=SMBWSTBRDR0
	ldr		r1,=WSTBRD0
	str		r1,[r0]

	
	ldr		r0,=SMBWSTBRDR0
	ldr		r1,=WSTBRD0
	str		r1,[r0]

	ldr		r0,=SSMCCR
	ldr		r1,=((MemClkRatio<<1)+(SMClockEn<<0))
	str		r1,[r0]
	
	;ldr		r0,=SMBWSTRDR5
	;ldr		r1,=0xe
	;str		r1,[r0]
	
	mov pc, lr

        LTORG

MMU_SetAsyncBusMode

        mrc     p15,0,r0,c1,c0,0
        orr     r0,r0,#R1_nF:OR:R1_iA
        mcr     p15,0,r0,c1,c0,0
        mov 	pc, lr




loop_led

	LED_ON	0x3

	ldr r0,=0x800000
10	subs r0, r0, #1
	bne %B10	

	LED_ON	0xC

	ldr r0,=0x800000
12	subs r0, r0, #1
	bne %B12	

	b loop_led

vloop_led

	VLED_ON	0xe

	ldr r0,=0x80000
10	subs r0, r0, #1
	bne %B10	

	VLED_ON	0xf

	ldr r0,=0x80000
12	subs r0, r0, #1
	bne %B12	

	b vloop_led	; Infinite loop

        ENTRY_END 




;------------------------------------------------------------------------------
; Clock Division Change funtion for DVS on S3C2450.
;------------------------------------------------------------------------------

;	LEAF_ENTRY CLKDIV124
;	ldr     r0, = vCLKDIVN
;	ldr     r1, = 0x3		; 0x3 = 1:2:4
;	str     r1, [r0]
;	mov     pc, lr

;	LEAF_ENTRY CLKDIV144
;	ldr     r0, = vCLKDIVN
;	ldr     r1, = 0x4		; 0x4 = 1:4:4
;	str     r1, [r0]
;	mov     pc, lr

;	LEAF_ENTRY CLKDIV136
;	ldr     r0, = vCLKDIVN
;	ldr     r1, = 0x7			; 1:6:12
;	str     r1, [r0]
;	ldr     r0, = vCAMDIVN
;	ldr		r1, [r0]
;	bic		r1, r1, #(0x3<<8)
;	orr		r1, r1, #(0x0<<8)	; 1:3:6
;	str     r1, [r0]	
;	mov     pc, lr

;	LEAF_ENTRY CLKDIV166
;	ldr     r0, = vCAMDIVN
;	ldr		r1, [r0]
;	bic		r1, r1, #(0x3<<8)
;	orr		r1, r1, #(0x1<<8)	; 1:6:12
;	str     r1, [r0]
;	ldr     r0, = vCLKDIVN
;	ldr     r1, = 0x6			; 1:6:6
;	str     r1, [r0]	
;	mov     pc, lr

;	LEAF_ENTRY CLKDIV148
;	ldr     r0, = vCLKDIVN
;	ldr     r1, = 0x5			; 1:8:16
;	ldr     r2, = vCAMDIVN
;	ldr		r3, [r2]
;	bic		r3, r3, #(0x3<<8)
;	orr		r3, r3, #(0x0<<8)	; 1:4:8
;	str     r1, [r0]
;	str     r3, [r2]
;	mov     pc, lr

;	LEAF_ENTRY CLKDIV188
;	ldr     r0, = vCAMDIVN
;	ldr		r1, [r0]
;	bic		r1, r1, #(0x3<<8)
;	orr		r1, r1, #(0x2<<8)	; 1:8:16
;	ldr     r2, = vCLKDIVN
;	ldr     r3, = 0x4			; 1:8:8
;	str     r1, [r0]
;	str     r3, [r2]
;	mov     pc, lr

;	LEAF_ENTRY DVS_ON	
;	ldr		r0, = vCAMDIVN
;	ldr		r1, [r0]
;	orr		r1, r1, #(0x1<<12)	; DVS_EN bit = 1(FCLK = HCLK)
;	str		r1, [r0]
;	mov		pc, lr

;	LEAF_ENTRY DVS_OFF
;	ldr		r0, = vCAMDIVN
;	ldr		r1, [r0]
;	bic		r1, r1, #(0x1<<12)	; DVS_EN bit = 0(FCLK = MPLL clock)
;	str		r1, [r0]
;	mov		pc, lr
	
        END

;------------------------------------------------------------------------------



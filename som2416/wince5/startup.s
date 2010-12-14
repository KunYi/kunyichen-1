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

;926EJ, 2450

	[ BSP_TYPE = BSP_SMDK2450
DCACHE_LINES_PER_SET_BITS       EQU     2
DCACHE_LINES_PER_SET            EQU     4
DCACHE_NUM_SETS                 EQU     128
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

	[BSP_TYPE = BSP_SMDK2443 
	MACRO
	LED_ON	$data

	LDR	r10, =GPFUDP
	LDR     r11, =0x5500	;Pull-Up-Down Disable
	STR	r11, [r10]
	
	LDR	r10, = GPFCON
	LDR	r11, = (0x5500)	; GPF[7:4] Output .
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
	
	LDR	r10, = vGPFUDP
	LDR     r11, =0x5500	;Pull-Up-Down Disable
	STR	r11, [r10]
	
	LDR	r10, = vGPFCON
	LDR	r11, = (0x5500)	; GPF[7:4] Output .
	STR	r11, [r10]
	LDR	r10, =vGPFDAT
	LDR	r11, =$data
	MOV     r11, r11, lsl #4	; [7:4]
  	STR	r11, [r10]
    	MEND
    	]
   
;; //[david.modify] 2008-04-25 11:31
;;//#define GLED GPB0		//工作指示灯，0：灯亮；1：灯灭。
	MACRO
	LED_ON_PND	$data

  	MEND

;;//#define GLED GPB0		//工作指示灯，0：灯亮；1：灯灭。
;;下面函数在虚拟地址情况下用
	MACRO
	VLED_ON_PND	$data

  	MEND  	
;---------------------------------------------------------------------------   
;---------------------------------------------------------------------------   
;---------------------------------------------------------------------------

;	IMPORT  Max1718_Init
;	IMPORT  Max1718_Set
        IMPORT  KernelStart

        TEXTAREA
        
        ; Include memory configuration file with g_oalAddressTable
 
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
;; //[david.modify] 2008-05-12 14:16
;; 点亮GPB0
	LED_ON_PND		0x0
		
    ldr	r0, =WTCON       ; disable the watchdog timer.
    mov	r1,#0         
    str	r1, [r0]

	;;;;;;;;;;;; set voltage test
	;bl		Max1718_Init
	;mov		r0, #0x1
	;mov		r1, #120
	;mov		r1, #125
	;mov		r1, #130
	;mov		r1, #135
	;bl		Max1718_Set

;    ldr	r0, =EBICON		; EBI
;    ldr	r1, =EBICON_VAL			; Refer s3c2450.inc
;    str	r1,[r0]


;; //[david.modify] 2008-08-01 10:59
;;=========================
	[ {FALSE}
    	ldr r0, = GPFCON
    	ldr r1, = 0x5500      
    	str r1, [r0]
    	]
;;=========================

   [ BSP_TYPE = BSP_SMDK2443 	
	ldr		r0, =INTMSK			; mask all first-level interrupts.
	ldr		r1, =0xffffffff
	str		r1, [r0]

	ldr		r0, =INTSUBMSK		; mask all second-level interrupts.
	ldr		r1, =0x1fffffff
	str		r1, [r0]

	ldr		r0, = INTMOD			; set all interrupt as IRQ
	mov		r1, #0x0
	str		r1, [r0]

	]
	[ BSP_TYPE = BSP_SMDK2450 
    ldr	r0, =INTMSK1      ; mask all first-level interrupts.
    ldr	r1, =0xffffffff
    str	r1, [r0]

    ldr	r0, =INTMSK2      ; mask all first-level interrupts.
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

    [BSP_TYPE = BSP_SMDK2443 
    LED_ON	0x2
    ]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; When the Eboot is already, No need to set again.
;; IF set as "FALSE", The FCLK,HCLK,PCLK Clock will detect automatically in OS.
;; IF set as "TRUE", The FCLK,HCLK,PCLK Clock and Memory setting will be set again here and detect automatically in OS.

 	[ CHANGE_CLK_OAL=1	; Refer the S3c2450.inc.


	[BSP_TYPE = BSP_SMDK2443 
       LED_ON	0x4
       ]

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
		
	[BSP_TYPE = BSP_SMDK2443 	
       LED_ON	0x3
       ]

	ldr		r0,=CLKDIV0			;	Set Clock Divider
	ldr		r1,[r0]
	bic		r1,r1,#0x37		; clear HCLKDIV, PREDIV, PCLKDIV
	bic		r1,r1,#(0xf<<9) ; clear ARMCLKDIV
	ldr		r2,=((Startup_ARMCLKdiv<<9)+(Startup_PREdiv<<4)+(Startup_PCLKdiv<<2)+(Startup_HCLKdiv)) 
	orr		r1,r1,r2
	str		r1,[r0]	
	
	[BSP_TYPE = BSP_SMDK2443 
       LED_ON	0x5
       ]
	ldr		r0,=CLKSRC			;	Select MPLL clock out for SYSCLK
	ldr		r1,[r0]
	orr		r1,r1,#0x50
	str		r1,[r0]	

    ;bl		MMU_SetAsyncBusMode

	[BSP_TYPE = BSP_SMDK2443 
     LED_ON	0x7
     ]

	bl		InitSSMC				
	]	; End of PLL setting
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	[ DVSON = 1
	
;		bl	DVS_ON
;	ldr		r0,=CLKDIV0			;	Set DVS
;	ldr		r1,[r0]
;	orr		r1,r1,#0x2000
;	str		r1,[r0]	
	
	]
	
    bl		MMU_SetAsyncBusMode	

	[ {TRUE}				; Check WakeUp signature after sleep mode in Eboot
	ldr		r1, =INFORM1
	ldr		r0, [r1]
	cmp		r0, #0xEB
	ldr		r0, =0x0
	str		r0, [r1]
	beq		%f4
	]
    
		
; :::::::::::::::::::::::::::::::::::::::::::::
;           Add for Power Management 
; - - - - - - - - - - - - - - - - - - - - - - -
	[ {TRUE}
	ldr		r1, =RSTSTAT           ; Determine Booting Mode
	ldr		r0, [r1]
	tst		r0, #0x8				; Power-Off reset Check
	beq		%f4

	ldr 	r5, =SLEEPDATA_BASE_PHYSICAL	; pointer to physical address of reserved Sleep mode info data structure 
	mov		r3, r5					; pointer for checksum calculation
	mov		r2, #0
	ldr		r0, =SLEEPDATA_SIZE		; get size of data structure to do checksum on
40
	ldr		r1, [r3], #4			; pointer to SLEEPDATA
	and		r1, r1, #0x1
	mov		r1, r1, LSL #31
	orr		r1, r1, r1, LSR #1
	add		r2, r2, r1
	subs	r0, r0, #1				; dec the count
	bne		%b40			        ; loop till done	

	ldr		r0,=INFORM3
	ldr		r3, [r0]				; get the Sleep data checksum from the Power Manager Scratch pad register
	teq		r2, r3			        ; compare to what we saved before going to sleep
	bne		JumpToRAM				; bad news - do a cold boot - If emergency power off case, normal booting.
	b		MMUENABLE
	
JumpToRAM
	ldr		r2, =0x200000 ;=0x201000					; offset into the RAM 
	ldr		r3, =0x30000000					; add physical base
	add		r2, r2, r3
	mov     pc, r2							;  & jump to StartUp address
	
MMUENABLE
;  2. MMU Enable

	ldr     r10, [r5, #SleepState_MMUDOMAIN]	; load the MMU domain access info
	ldr     r9,  [r5, #SleepState_MMUTTB]		; load the MMU TTB info	
	ldr     r8,  [r5, #SleepState_MMUCTL]		; load the MMU control info	
	ldr     r7,  [r5, #SleepState_WakeAddr]		; load the LR address
	nop			
	nop
	nop
	nop
	nop	
	
; wakeup routine
1
	mcr		p15, 0, r10, c3, c0, 0		; setup access to domain 0
	mcr		p15, 0, r9,  c2, c0, 0		; PT address
	mcr		p15, 0, r0,  c8, c7, 0	   	; flush I+D TLBs
	mcr		p15, 0, r8,  c1, c0, 0		; restore MMU control

;  3. Jump to Kernel Image's fw.s(Awake_address)
	mov     pc, r7						;  & jump to new virtual address (back up Power management stack)
	nop
	
	]
			
4		
;;;;;;;;;;;;;;;;;; set clkout1 - HCLK
	ldr		r0,=MISCCR
	ldr		r1,[r0]

	bic		r1,r1, #0x770
	orr		r1,r1,#0x320
	str		r1,[r0]
	
	ldr		r0,=GPHUDP
	ldr		r1,[r0]	
	bic		r1,r1, #0x3C000000
	orr		r1,r1, #0x14000000
	str		r1,[r0]	
		
	ldr		r0,=GPHCON
	ldr		r1,[r0]

	bic		r1,r1, #0x3C000000
	orr		r1,r1, #0x28000000
	str		r1,[r0]	
;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;; PWR & CLK OFF FOR TEST
;	ldr 	r0,=HCLKCON
;	ldr		r1,=0xFFFFFFFF   ;0xFFFCE0FF   ;0xFFFCF6FF
;	str		r1,[r0]
;	ldr 	r0,=PCLKCON
;	ldr		r1,=0xFFFF3F82   ;0xFFFF3E02   ;0xFFFF3F82   
;	str		r1,[r0]
;	ldr 	r0,=SCLKCON
;	ldr		r1,=0xFFFF87FD   ;0xFFFE83FD   ;0xFFFF87FD  
;	str		r1,[r0]	
;;;;;;;;;;;;;;;;;;
	[BSP_TYPE = BSP_SMDK2443 
       LED_ON	0x8
       ]
        
	add		r0, pc, #g_oalAddressTable - (. + 8)

	bl		KernelStart	; Call the WinCE kernel.

        LTORG

	INCLUDE oemaddrtab_cfg.inc

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
	mov     r2, #0
	ldr     r0, =SLEEPDATA_SIZE		; get size of data structure (in words)
30
	ldr     r1, [r3], #4
	and     r1, r1, #0x1
	mov     r1, r1, LSL #31
	orr     r1, r1, r1, LSR #1
	add     r2, r2, r1
	subs    r0, r0, #1
	bne     %b30
	
	ldr     r0, =vINFORM3
	str     r2, [r0]		; Store in Power Manager Scratch pad register



	[ BSP_TYPE = BSP_SMDK2443 
;       4. Interrupt Disable 
	ldr     r0, =vINTBASE
	mvn     r2, #0
	str     r2, [r0, #oINTMSK]
	str     r2, [r0, #oSRCPND]	
	]
	[ BSP_TYPE = BSP_SMDK2450 
;       4. Interrupt Disable 
	ldr     r0, =vINTBASE1
	mvn     r2, #0
	str     r2, [r0, #oINTMSK1]
	str     r2, [r0, #oSRCPND1]
	str     r2, [r0, #oINTPND1]
;	 //[david.modify] 2008-08-01 10:33
;; enable EINT0-PWR BUTTON, EINT2-USB INSERT
;;==========================
;	ldr	  r2,=0xfffffefe
	ldr	  r2,=0xfffffefa
;;==========================	
	str     r2, [r0, #oINTMSK1]

	ldr     r0, =vINTBASE2
	mvn     r2, #0
	str     r2, [r0, #oINTMSK2]
	str     r2, [r0, #oSRCPND2]
	str     r2, [r0, #oINTPND2]	
	]


;;       5. Cache Flush
	[ {TRUE}
	bl      OALClearUTLB
	bl      OALFlushICache
	ldr     r0, = (DCACHE_LINES_PER_SET - 1)    
	ldr     r1, = (DCACHE_NUM_SETS - 1)    
	ldr     r2, = DCACHE_SET_INDEX_BIT    
	ldr     r3, = DCACHE_LINE_SIZE     
	bl      OALFlushDCache
	]

; //[david.modify] 2008-08-01 10:36
;;=============================
	[ {FALSE}
;       6. Setting Wakeup External Interrupt(EINT0) Mode
	ldr     r0, =vGPIOBASE

	ldr     r1, =0x5502
	str     r1, [r0, #oGPFCON]
	]
;;=============================
	
	[ BSP_TYPE = BSP_SMDK2443
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

	ldr     r4, =vRSTCON
	ldr     r5, =0x0ff80            
	str     r5, [r4]		

	ldr     r4, =vOSCSET
	ldr     r5, =0x8000            
	str     r5, [r4]

	ldr     r4, =vPWRCFG
	ldr     r5, =0x8201           
	str     r5, [r4]
	]

	[ BSP_TYPE = BSP_SMDK2450 
	ldr     r4, =vRSTCON
	ldr     r5, =0x1ff80            
	str     r5, [r4]		

	ldr     r4, =vOSCSET
	ldr     r5, =0xffff            
	str     r5, [r4]

	ldr     r4, =vPWRCFG
;	ldr     r5, =((1<<15)+(0<<9)+(1<<8)+(1<<7)+(0<<4)+(0<<3)+(0<<2)+(1<<0))
;	ldr     r5, =0x8181
	ldr     r5, =0x8101
	str     r5, [r4]
   ; ODT Disable..
;	pMemCtrl->BANKCON3 = (pMemCtrl->BANKCON3 & ~((0x3<<30)|(0x1<<22)|(0x1<<18))) | (0x1<<30);
;	pMemCtrl->BANKCON1 = (pMemCtrl->BANKCON1 & ~(0x3)) | (0x3); // EMRS command
;	pMemCtrl->BANKCON1 = (pMemCtrl->BANKCON1 & ~(0x3)); // Normal operation
	


	ldr		r2,=vBANKCON3			
	ldr		r1,[r2]
	bic		r1,r1,#(3<<30)
	bic		r1,r1,#(1<<22)
	bic		r1,r1,#(1<<18)
;	bic		r1,r1,#((3<<30)+(1<<22)+(1<<18))
	orr		r1,r1,#(0x1<<30)		
	str		r1,[r2]


	
	ldr		r2,=vBANKCON1			;	EMRS command
	bic		r1,r1,#(0x3<<0)
	orr		r1,r1,#(0x3<<0)			
	str		r1,[r2]

		
	ldr		r2,=vBANKCON1			;	Normal operation
	bic		r1,r1,#(0x3<<0)
	str		r1,[r2]
	]

	
	ldr     r4, =vPWRMODE
	ldr		r5, [r4]
	bic		r5, r5, #0xff00
	bic		r5, r5, #0x00ff
	ldr		r6, =0x2BED
	orr     r5, r5, r6            ; Power Off Mode

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
        mov pc, lr




	[BSP_TYPE = BSP_SMDK2443 
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
	]
        ENTRY_END 




;------------------------------------------------------------------------------
; Clock Division Change funtion for DVS on S3C2450.
;------------------------------------------------------------------------------
; ARMCLK = MPLL/ARMCLKdiv, 
;		ARMCLK:MPLL    ARMCLKdiv
;					1/1					0
;					1/2					8
;					1/3					2
;					1/4					9
;					1/6					10
;					1/8					11
;					1/12				13
;					1/16				15
;											1 and other values couldn't be seen.
;		PREDIVCLK = ARMCLK / (PREDIV+1) should be less than 266Mhz
;		HCLK = PREDIVCLK / (HCLKDIV+1)
;		PCLK = HCLK / (PCLKDIV+1)
; CLKDIV 0 : DVS[13], ARMDIV[12:9], EXTDIV[8:6], PREDIV[5:4], HALFHCLK[3], PCLKDIV[2], HCLKDIV[1:0]
ARMDIV_bit	EQU				9
PREDIV_bit	EQU				4
HCLKDIV_bit	EQU				0
PCLKDIV_bit	EQU				2

	LEAF_ENTRY	HCLK_DOWNTO_PCLK
	ldr		r0, =vCLKDIV0			;	Set Clock Divider
	ldr		r1, [r0]
	bic		r1,	r1,	#0x7			; clear PCLKDIV, HCLKDIV
	ldr		r2, =((0<<PCLKDIV_bit)+(3<<HCLKDIV_bit));
	orr		r1, r1, r2
	str		r1, [r0]
	mov		pc, lr

	LEAF_ENTRY	HCLK_RECOVERYUP
	ldr		r0, =vCLKDIV0			; Set Clock Divider
	ldr		r1, [r0]
	bic		r1, r1, #0x7			; clear PCLKDIV, HCLKDIV
	ldr		r2, =((1<<PCLKDIV_bit)+(1<<HCLKDIV_bit));
	orr		r1, r1, r2
	str		r1, [r0]
	mov		pc, lr	


	LEAF_ENTRY DVS_ON	
	ldr		r0, =vCLKDIV0
	ldr		r1, [r0]
	orr		r1, r1, #(0x1 << 13)	; DVS bit = 1 (FCLK = HCLK)
	str		r1, [r0]
	mov		pc, lr

	LEAF_ENTRY DVS_OFF
	ldr		r0, =vCLKDIV0
	ldr		r1, [r0]
	bic		r1, r1, #(0x1 << 13)	; DVS bit = 0 (FCLK = MPLL clock)
	str		r1, [r0]	
	mov		pc, lr
	
        END

;------------------------------------------------------------------------------



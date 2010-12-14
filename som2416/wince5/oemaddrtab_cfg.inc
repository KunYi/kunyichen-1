;------------------------------------------------------------------------------
;
;  File:  memory_cfg.inc
;
;  This file is used to define g_oalAddressTable. This table is passed to
;  KernelStart to estabilish physical to virtual memory mapping. This table
;  is used also in IOMEM OAL module to map between physical and virtual
;  memory addresses via OALPAtoVA/OALVAtoPA functions.
;
;------------------------------------------------------------------------------

; Export Definition

        EXPORT  g_oalAddressTable[DATA]

;------------------------------------------------------------------------------
;
; TABLE FORMAT
;       cached address, physical address, size
;------------------------------------------------------------------------------

g_oalAddressTable
    DCD     0x80000000, 0x30000000, 64      ; 64 MB DRAM BANK 6

       ; DCD     0x84000000, 0x10000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 2
       ; DCD     0x86000000, 0x18000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 3
       ; DCD     0x88000000, 0x20000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 4
       ; DCD     0x8A000000, 0x28000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 5
       ; DCD     0x8C000000, 0x08000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 1

	DCD     0x90800000, 0x48000000,  1      ; SDRAM control register
	DCD     0x90900000, 0x48800000,  1      ; EBI control register
	DCD     0x90A00000, 0x49000000,  1      ; USB Host control register
	DCD     0x90B00000, 0x49800000,  1      ; USB Device control register
	DCD     0x90C00000, 0x4A000000,  1      ; Interrupt control register
	DCD     0x90D00000, 0x4A800000,  1      ; HS-MMC control register

	DCD     0x90E00000, 0x4B000000,  1      ; DMA0~7 control register

	
	DCD     0x90F00000, 0x4B800000,  1      ; CF Card control register
	DCD     0x91000000, 0x4C000000,  1      ; SYSCON register
	DCD     0x91100000, 0x4C800000,  1      ; TFT-LCD control register
	DCD     0x91200000, 0x4D000000,  1      ; STN-LCD control register
	DCD     0x91300000, 0x4D408000,  1      ; 2D
	DCD     0x91400000, 0x4D800000,  1      ; Camera control register
	DCD     0x91500000, 0x4E000000,  1      ; NAND flash control regist
	DCD     0x91600000, 0x4E800000,  1      ; Matrix control register
	DCD     0x91700000, 0x4F000000,  1      ; SSMC control register
	DCD     0x91800000, 0x4F800000,  1      ; TIC control register
	DCD     0x91900000, 0x50000000,  1      ; UART control register
	DCD     0x91A00000, 0x51000000,  1      ; PWM control register
	DCD     0x91B00000, 0x52000000,  1      ; HS-SPI and SPI0 control register
	DCD     0x91C00000, 0x53000000,  1      ; WDT control register
	DCD     0x91D00000, 0x54000000,  1      ; IIC control register
	DCD     0x92000000, 0x55000000,  1      ; IIS control register
	DCD     0x92100000, 0x56000000,  1      ; I/O Port register
	DCD     0x92200000, 0x57000000,  1      ; RTC Port register
	DCD     0x92300000, 0x58000000,  1      ; TSADC Port register
	DCD     0x92400000, 0x59000000,  1      ; SPI Port register
	DCD     0x92500000, 0x5A000000,  1      ; SDI Port register
	DCD     0x92600000, 0x5B000000,  1      ; AC97 Port register
	DCD     0x92700000, 0x5C000000,  1      ; PCM 0 ~ 1 control register
	DCD     0x92800000, 0x5D000000,  1      ; GPS control register	
	DCD     0x92900000, 0x5E000000,  1      ; CHIP ID control register
;	 //[david.modify] 2008-07-30 20:58
;;在EBOOT中支持SD卡
	DCD 	0x92A00000, 0x4AC00000, 1 ; HSMMC0 control register
;	 //[david. end] 2008-07-30 20:58
	DCD     0x93000000, 0x00000000, 32      ; 32 MB SROM(SRAM/ROM) BANK 0
        DCD     0x00000000, 0x00000000,  0      ; end of table

;------------------------------------------------------------------------------

        END


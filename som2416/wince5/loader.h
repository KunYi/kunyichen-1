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

#ifndef _LOADER_H_
#define _LOADER_H_

#include <blcommon.h>
#include <bootpart.h>

#include <WMRConfig.h>
#include <WMRTypes.h>

// Bootloader version.
//
#define EBOOT_VERSION_MAJOR     2
#define EBOOT_VERSION_MINOR     4


//------------------------------------------------------------------------------
//
//  Section Name:   Memory Configuration
//  Description.:   The constants defining memory layout below must match
//                  those defined in the .bib files.
//
//------------------------------------------------------------------------------

#define CACHED_TO_UNCACHED_OFFSET   0x20000000

#define STEPLDR_RAM_IMAGE_BASE      0x00000000
#define STEPLDR_RAM_IMAGE_SIZE      0x0003F000	// 0x00004000 //only use 0x00001000

#define EBOOT_RAM_IMAGE_BASE        0x80038000
#define EBOOT_RAM_IMAGE_SIZE        0x00080000


#define EBOOT_STORE_OFFSET          0
#define EBOOT_STORE_ADDRESS         (EBOOT_RAM_IMAGE_BASE + EBOOT_STORE_OFFSET)
#define EBOOT_STORE_MAX_LENGTH      EBOOT_RAM_IMAGE_SIZE

// BinFS work area defined in boot.bib
#define BINFS_RAM_START             (0x83C40000 | CACHED_TO_UNCACHED_OFFSET)   // uncached	// by dodan2 061103
#define BINFS_RAM_LENGTH            0x400000	// hmseo-061029 for MLC

//#define BINFS_RAM_START             (0x80180000 | CACHED_TO_UNCACHED_OFFSET)   // uncached	// by dodan2 061103
//#define BINFS_RAM_LENGTH            0x80000	// hmseo-061029 for MLC

//
// Nk Memory reigions defined in config.bib...
//
#define ROM_RAMIMAGE_START          0x80000000

 //[david.modify] 2008-06-14 17:07
 // ���IMAGE �ĳ�36MB
 //=========================
//#define ROM_RAMIMAGE_SIZE           0x02000000
#define ROM_RAMIMAGE_SIZE           0x02400000
 //=========================

#define OS_RAM_IMAGE_BASE			0x80040000
#define OS_RAM_IMAGE_SIZE			(0x03C00000)

#define FILE_CACHE_START            (0x80200000 | CACHED_TO_UNCACHED_OFFSET)        // Start of file cache (temporary store

//[david.modify] 2008-11-10 10:03
//========================================= 
// �ڴ��ܴ�С64MBYTES
#define BSP_RAMSIZE 0x04000000
#define FILE_CACHE_TMPBUF1 FILE_CACHE_START
//from bootloader.bib and config.bib
//DISPLAY             80100000  00100000  RESERVED    
//#define DISPLAY_RESERVED_BUFFER (0x80100000+0x80000)

//#define DEF_DOWNLOAD_ADDR           0xA1000000
//[david.modify] 2008-08-16 12:02
// FROM BOOT.BIB
//  	RAM      83D00000  00200000  RAM 
#define BOOT_RAM_START 0x83D00000
#define BOOT_RAM_START_UNCACHED  (BOOT_RAM_START | CACHED_TO_UNCACHED_OFFSET) 
#define MAX_DOWNLOAD_SIZE (30*1024*1024)
#define DEF_DOWNLOAD_ADDR           ( BOOT_RAM_START_UNCACHED-MAX_DOWNLOAD_SIZE)


#define BOOT_LOGO_MAXSIZE (1*1024*1024)
#define BOOT_LOGO_SIZE (256*1024)
#define BOOT_LOGO_BUFFER (BOOT_RAM_START-BOOT_LOGO_MAXSIZE)
//#define BOOT_LOGO_BUFFER (0x80100000+0x80000)




//=========================================

// Driver globals pointer (parameter sharing memory used by bootloader and OS).
//
#define pBSPArgs                    ((BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START)


// Platform "BOOTME" root name.
//
#define PLATFORM_STRING         "SMDK2450"

#define SECTOR_TO_BLOCK(sector)		(sector/(SECTORS_PER_SUPAGE*PAGES_PER_SUBLK))
#define BLOCK_TO_SECTOR(block)		(block*(SECTORS_PER_SUPAGE*PAGES_PER_SUBLK))

// fs: sector number
// returns block aligned value
__inline DWORD SECTOR_TO_BLOCK_SIZE(DWORD sn) {
    return ( (sn / SECTORS_PER_SUBLK) + ( (sn % SECTORS_PER_SUBLK) ? 1 : 0) );
}

// fs: file size in bytes
// returns sector aligned value
__inline DWORD FILE_TO_SECTOR_SIZE(DWORD fs) {
    return ( (fs / SECTOR_SIZE) + ( (fs % SECTOR_SIZE) ? 1 : 0) );
}

// ns: number of sectors
// N.B: returns sector aligned value since we can't tell
__inline DWORD SECTOR_TO_FILE_SIZE(DWORD ns) {
    return ( ns * SECTOR_SIZE );
}

//
// Boot Config Flags...
//

// BOOT_MODE_ flags indicate where we are booting from
#define BOOT_MODE_NAND          0x00000001
#define BOOT_MODE_NOR_AMD       0x00000002
#define BOOT_MODE_NOR_STRATA    0x00000004
#define BOOT_MODE_TEST          0x00000008
#define BOOT_MODE_MASK(dw)     (0x0000000F & (dw))

// BOOT_TYPE_ flags indicate whether we are booting directly off media, or downloading an image
#define BOOT_TYPE_DIRECT        0x00000010  // boot off media?
#define BOOT_TYPE_MULTISTAGE    0x00000020  // multi-stage bootloaders?
#define BOOT_TYPE_MASK(dw)     (0x000000F0 & (dw))

// TARGET_TYPE_ flags indicate where the image is to be written to.
#define TARGET_TYPE_RAMIMAGE    0x00000100  // Directly to SDRAM
#define TARGET_TYPE_CACHE       0x00000200  // FLASH_CACHE
#define TARGET_TYPE_NOR         0x00000400  // NOR Flash
#define TARGET_TYPE_NAND        0x00000800  // NAND Flash
//...
#define TARGET_TYPE_MASK(dw)   (0x00000F00 & (dw))

#define CONFIG_FLAGS_DHCP       0x00001000
#define CONFIG_FLAGS_DEBUGGER   0x00002000
#define CONFIG_FLAGS_MASK(dw)  (0x0000F000 & (dw))


//[david.modify] 2008-05-30 14:07
// Ϊ�˼ӿ�����5-->1
#define CONFIG_BOOTDELAY_DEFAULT       1	//5


//
// Bootloader configuration parameters.
//
typedef struct _BOOTCFG {

    ULONG       ImageIndex;

    ULONG       ConfigFlags;
    ULONG       BootDelay;

    EDBG_ADDR   EdbgAddr;
    ULONG       SubnetMask;

} BOOT_CFG, *PBOOT_CFG;





 
//
// On disk structures for NAND bootloaders...
//

//
// OEM Reserved (Nand) Blocks for TOC and various bootloaders
//

// NAND Boot (loads into SteppingStone) @ Block 0
#define NBOOT_BLOCK				(0)
#define NBOOT_BLOCK_SIZE		(1)
#define NBOOT_SECTOR			BLOCK_TO_SECTOR(NBOOT_BLOCK)

// TOC @ Block 1
#define TOC_BLOCK				(1)
#define TOC_BLOCK_SIZE			(1)
#define TOC_BLOCK_RESERVED		(1)
#define TOC_SECTOR			BLOCK_TO_SECTOR(TOC_BLOCK)

// Eboot @ Block 2
#define EBOOT_BLOCK				(3)
//#define EBOOT_BLOCK_SIZE		(5)		//EBOOT 512KB���£�ռ2��BLOCK
//EBOOT 512KB���£�ռ2��BLOCK ,��ԭ��ʡ������(5-3)=2��BLOCK���ڴ�LOGO
#define EBOOT_BLOCK_SIZE		(3)		
#define EBOOT_BLOCK_RESERVED	(2)
#define EBOOT_SECTOR		BLOCK_TO_SECTOR(EBOOT_BLOCK)

//[david.modify] 2008-10-07 16:36
// 1. toc����1��BLOCK��������һ��BLOCK����.
// 2. config.bin ���� 1��BLOCK�� 1��BLOCK��������
// 3. LOGO��׼����2��BLOCK,2��BLOCK��������  (320x240, 480x272�ķֱ��ʿ�������)
// ���һ���ܹ�����֮ǰ��� 1+1+2+2=6��BLOCK�ռ������������������Լ��
// 6*256k=1.5MB
//=======================================

#define CONFIG_UINT32_FLAG 0x434F4E46 	// "CONF"

typedef struct
{
	UINT32 u32Flag;		// CONFIG_UINT32_FLAG
	UINT32 u32LCDType;	// LCD�ͺ�
	UINT32 u32BootDelay;	//�ȴ�����Զ�����OS
	UINT32 u32TrueDevId;	// �Ƿ�ʹ����ʵ��ID
	UINT8 u8DevId[16];		//���ʹ���ֶ�ID����������
	UINT32 u32DefLang;		// GPS UIȱʡ����
	UINT8 u8MapCode[32];	//ר����PPC��ͼ
	UINT8 u8DevCode[32];		
}stFlashCfg;



//[david.modify] 2008-11-10 09:38
// ��ΪOS��BLOCK��Ҫλ�ڵ�10BLOCK�ϣ�����BOOTLOADER����Խ�����
#if 0	
#define CONFIG_START_BLOCK (NBOOT_BLOCK_SIZE+TOC_BLOCK_SIZE+TOC_BLOCK_RESERVED+EBOOT_BLOCK_SIZE+EBOOT_BLOCK_RESERVED)
#define CONFIG_BLOCK_SIZE (1)
#define CONFIG_BLOCK_RESERVED (1)
#define CONFIG_START_SECTOR BLOCK_TO_SECTOR(CONFIG_START_BLOCK)

//[david.modify] 2008-09-27 10:42
// ��LOGO BLOCK������EBOOT_BLOCK_RESERVED��������
// LOGO_START_BLOCK = 1 + 1+ 1+ 5 = 8
#define LOGO_START_BLOCK  (CONFIG_START_BLOCK + CONFIG_BLOCK_SIZE +CONFIG_BLOCK_RESERVED)
#define LOGO_BLOCK_SIZE (2)
#define LOGO_BLOCK_RESERVED (2)
#define LOGO_START_SECTOR BLOCK_TO_SECTOR(LOGO_START_BLOCK)
//#define RESERVED_BOOT_BLOCKS	(LOGO_START_BLOCK + LOGO_BLOCK_SIZE +LOGO_BLOCK_RESERVED)
#endif

#if 1
//[david.modify] 2008-09-27 10:42
// ��LOGO BLOCK������EBOOT_BLOCK_RESERVED��������, ��2��BLOCK
#define LOGO_START_BLOCK  (EBOOT_BLOCK+EBOOT_BLOCK_SIZE+EBOOT_BLOCK_RESERVED)
#define LOGO_BLOCK_SIZE (1)
#define LOGO_BLOCK_RESERVED (1)
#define LOGO_START_SECTOR BLOCK_TO_SECTOR(LOGO_START_BLOCK)
//#define RESERVED_BOOT_BLOCKS	(LOGO_START_BLOCK + LOGO_BLOCK_SIZE +LOGO_BLOCK_RESERVED)


#endif


#define RESERVED_BOOT_BLOCKS	10
//#define RESERVED_BOOT_BLOCKS	(NBOOT_BLOCK_SIZE + TOC_BLOCK_SIZE +TOC_BLOCK_RESERVED + EBOOT_BLOCK_SIZE + EBOOT_BLOCK_RESERVED)

//=======================================





// Images start after OEM Reserved Blocks
#define IMAGE_START_BLOCK           RESERVED_BOOT_BLOCKS
#define IMAGE_START_SECTOR          BLOCK_TO_SECTOR(IMAGE_START_BLOCK)

//
// OEM Defined TOC...
//
#define MAX_SG_SECTORS              14
#define IMAGE_STRING_LEN            16  // chars
#define MAX_TOC_DESCRIPTORS         3   // per sector
#define TOC_SIGNATURE               0x434F544E  // (NAND TOC)

// Default image descriptor to load.
// We store Eboot as image 0, nk.nb0 as image 1
#define DEFAULT_IMAGE_DESCRIPTOR    1

// IMAGE_TYPE_ flags indicate whether the image is a Bootloader,
// RAM image, supports BinFS, Multiple XIP, ...
//

#define IMAGE_TYPE_LOG0 	0x101	//[david.modify] 2008-09-27 11:31 david added

#define IMAGE_TYPE_LOADER           0x00000001  // eboot.bin
#define IMAGE_TYPE_RAMIMAGE         0x00000002  // nk.bin
#define IMAGE_TYPE_BINFS            0x00000004
#define IMAGE_TYPE_MXIP             0x00000008
#define IMAGE_TYPE_RAWBIN			0x00000040	// raw bin .nb0
#define IMAGE_TYPE_STEPLDR			0x00000080	// stepldr.bin
#define IMAGE_TYPE_MASK(dw)        (0x0000000F & (dw))



// SG_SECTOR: supports chained (scatter gather) sectors.
// This structure equates to a sector-based MXIP RegionInfo in blcommon.
//
typedef struct _SG_SECTOR {

    DWORD dwSector;     // Starting sector of the image segment
    DWORD dwLength;     // Image length of this segment, in contigious sectors.

} SG_SECTOR, *PSG_SECTOR;


// IMAGE_DESCRIPTOR: describes the image to load.
// The image can be anything: bootloaders, RAMIMAGE, MXIP, ...
// Note: Our NAND uses H/W ECC, so no checksum needed.
//
typedef struct _IMAGE_DESCRIPTOR {

    // File version info
    DWORD dwVersion;                    // e.g: build number
    DWORD dwSignature;                  // e.g: "EBOT", "CFSH", etc
    UCHAR ucString[IMAGE_STRING_LEN];   // e.g: "PocketPC_2002"

    DWORD dwImageType;      // IMAGE_TYPE_ flags
    DWORD dwTtlSectors;     // TTL image size in sectors.
                            // We store size in sectors instead of bytes
                            // to simplify sector reads in Nboot.

    DWORD dwLoadAddress;    // Virtual address to load image (ImageStart)
    DWORD dwJumpAddress;    // Virtual address to jump (StartAddress/LaunchAddr)

    // This array equates to a sector-based MXIP MultiBINInfo in blcommon.
    // Unused entries are zeroed.
    // You could chain image descriptors if needed.
    SG_SECTOR sgList[MAX_SG_SECTORS];

    // BinFS support to load nk region only
	//struct
	//{
		ULONG dwStoreOffset;    // byte offset - not needed - remove!
		//ULONG RunAddress;     // nk dwRegionStart address
		//ULONG Length;         // nk dwRegionLength in bytes
		//ULONG LaunchAddress;  // nk dwLaunchAddr
	//} NKRegion;

} IMAGE_DESCRIPTOR, *PIMAGE_DESCRIPTOR;

//
// IMAGE_DESCRIPTOR signatures we know about
//
#define IMAGE_EBOOT_SIG             0x45424F54          // "EBOT", eboot.nb0
#define IMAGE_RAM_SIG               0x43465348          // "CFSH", nk.nb0
#define IMAGE_BINFS_SIG             (IMAGE_RAM_SIG + 1)

__inline BOOL VALID_IMAGE_DESCRIPTOR(PIMAGE_DESCRIPTOR pid) {
    return ( (pid) &&
      ((IMAGE_EBOOT_SIG == (pid)->dwSignature) ||
       (IMAGE_RAM_SIG == (pid)->dwSignature) ||
       (IMAGE_BINFS_SIG == (pid)->dwSignature)));
}

//  This is for MXIP chain.bin, which needs to be loaded into the SDRAM
//  during the start up. 
typedef struct _CHAININFO {
	
    DWORD   dwLoadAddress;          // Load address in SDRAM
    DWORD   dwFlashAddress;         // Start location on the NAND
    DWORD   dwLength;               // The length of the image
} CHAININFO, *PCHAININFO;

//
// TOC: Table Of Contents, OEM on disk structure.
// sizeof(TOC) = SECTOR_SIZE.
// Consider the TOC_BLOCK to contain an array of PAGES_PER_BLOCK
// TOC entries, since the entire block is reserved.
//
typedef struct _TOC {
    DWORD               dwSignature;
    // How to boot the images in this TOC.
    // This could be moved into the image descriptor if desired,
    // but I prefer to conserve space.
    BOOT_CFG            BootCfg;

    // Array of Image Descriptors.
    IMAGE_DESCRIPTOR    id[MAX_TOC_DESCRIPTORS];

    CHAININFO           chainInfo;
} TOC, *PTOC;           // 512 bytes


__inline BOOL VALID_TOC(PTOC ptoc) {
    return ((ptoc) &&  (TOC_SIGNATURE == (ptoc)->dwSignature));
}


// Loader function prototypes.
//
BOOL InitEthDevice(PBOOT_CFG pBootCfg);
int OEMReadDebugByte(void);
BOOL OEMVerifyMemory(DWORD dwStartAddr, DWORD dwLength);
void Launch(DWORD dwLaunchAddr);
PVOID GetKernelExtPointer(DWORD dwRegionStart, DWORD dwRegionLength);
void OEMWriteDebugLED(WORD wIndex, DWORD dwPattern);

BOOL WriteOSImageToBootMedia(DWORD dwImageStart, DWORD dwImageLength, DWORD dwLaunchAddr);
BOOL ReadOSImageFromBootMedia(void);
BOOL WriteRawImageToBootMedia(DWORD dwImageStart, DWORD dwImageLength, DWORD dwLaunchAddr);

BOOL    TOC_Init(DWORD dwEntry, DWORD dwImageType, DWORD dwImageStart, DWORD dwImageLength, DWORD dwLaunchAddr);
BOOL    TOC_Read(void);
BOOL    TOC_Write(void);
void    TOC_Print(void);

void MLC_LowLevelTest(void);

 //[david.modify] 2008-09-27 12:07
BOOL LOGO_Write(UINT8* pu8Logo, UINT32* pu32Bytes);
 BOOL LOGO_Read(UINT8* pu8Logo, UINT32* pu32Bytes);
 int Show565Bmp(UINT16 *pu16Framebuffer, UINT8 *p565bmpFile, UINT32 u32Bytes, UINT32 u32Cx, UINT32 u32Cy);


#endif  // _LOADER_H_.

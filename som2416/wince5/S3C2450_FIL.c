///////////////////////////////////////////////////////////////
//
//	MODULE		: FIL
//	NAME		: S3C2450X Flash Interface Layer
//	FILE			: S3C2450_FIL.c
//	PURPOSE		:
//
///////////////////////////////////////////////////////////////
//
//		COPYRIGHT 2003-2006 SAMSUNG ELECTRONICS CO., LTD.
//					ALL RIGHTS RESERVED
//
//	Permission is hereby granted to licensees of Samsung Electronics
//	Co., Ltd. products to use or abstract this computer program for the
//	sole purpose of implementing a product based on Samsung
//	Electronics Co., Ltd. products. No other rights to reproduce, use,
//	or disseminate this computer program, whether in part or in whole,
//	are granted.
//
//	Samsung Electronics Co., Ltd. makes no representation or warranties
//	with respect to the performance of this computer program, and
//	specifically disclaims any responsibility for any damages,
//	special or consequential, connected with the use of this program.
//
///////////////////////////////////////////////////////////////
//
//	REVISION HISTORY
//
//	2006.10.19	dodan2(gabjoo.lim@samsung.com)
//				Draft Version
//	2007.03.25	ksk
//				Support 4KByte/Page NAND Flash Device
//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////
// Header File
///////////////////////////////////////////////
#include <WMRConfig.h>
#include <WMRTypes.h>
#include <OSLessWMROAM.h>
#include <FIL.h>
#include <string.h>
#include <S3C2450_FIL.h>
#include <s3c2450_dma.h>
#include <s3c2450_nand.h>
#include <s3c2450_matrix.h>

//#define USE_SETKMODE
#define USE_2CE_NAND

///////////////////////////////////////////////
// Transfer Mode
///////////////////////////////////////////////
#define ASM					(0)		// Assembly Code
#define DMA					(1)		// DMA Transfer
#define NAND_TRANS_MODE	ASM

///////////////////////////////////////////////
// NAND DMA Buffer
///////////////////////////////////////////////
//#define NAND_DMA_BUFFER_UA	(0xA0100000+0x25800)		// LCD Frame Buffer
//#define NAND_DMA_BUFFER_PA	(0x30100000+0x25800)
#define NAND_DMA_BUFFER_UA	(0xB0700000)		// Stepping Stone (Check Oemaddrtab_cgf.inc !!!!)
#define NAND_DMA_BUFFER_PA	(0x40000000)

///////////////////////////////////////////////
// Debug Print Macro
///////////////////////////////////////////////
#define RETAILMSG(cond, printf_exp)	((cond)?(NKDbgPrintfW printf_exp),1:0)
#define NAND_ERR(x)	RETAILMSG(TRUE, x)
//#define NAND_ERR(x)
//#define NAND_MSG(x)	RETAILMSG(TRUE, x)
#define NAND_MSG(x)
#define NAND_LOG(x)	RETAILMSG(TRUE, x)
//#define NAND_LOG(x)

///////////////////////////////////////////////
// Device type context definitions
///////////////////////////////////////////////
typedef struct
{
	UINT8	nDevID;				// Device ID
	UINT8	nHidID;				// Hidden ID
	UINT32	nNumOfBlocks;		// Number of Blocks
	UINT32	nPagesPerBlock;		// Pages per block
	UINT32	nSectorsPerPage;	// Sectors per page
	BOOL32	b2XProgram;		// 2 plane program
	BOOL32	b2XRead;						/* 2 plane read	 	 */	
	BOOL32	b2XReadStatus;						/* 2 plane read status	 	 */	
	BOOL32	bDualDie;			// internal interleaving
	BOOL32	bMLC;				// MLC
} DEVInfo;

PRIVATE const DEVInfo stDEVInfo[] = {
		/*****************************************************************************/
		/* Device ID																 */
		/*	   Hidden ID															 */
		/*			 Blocks															 */
		/*				   Pages per block											 */
		/*						Sectors per page									 */
		/*						   2X program										 */
		/*						   			2X read									 */
		/*											 2x status 						 */		
		/*											 		  internal Interleaving	 */		
		/*												               MLC			 */
		/*****************************************************************************/
#if (WMR_SLC_SUPPORT)
		/* SLC NAND ID TABLE */
		{0xF1, 0x80, 1024,  64, 4, FALSE32, FALSE32, FALSE32, FALSE32, FALSE32},	/* 1Gb SLC(K9F1G08) Mono */
		{0xF1, 0x00, 1024,  64, 4, FALSE32, FALSE32, FALSE32, FALSE32, FALSE32},	/* 1Gb SLC(K9F1G08) Mono B-die */
		{0xDA, 0x80, 2048,  64, 4, FALSE32, FALSE32, FALSE32, FALSE32, FALSE32},	/* 2Gb SLC(K9F2G08) Mono */
		{0xDA, 0x10, 2048,  64, 4, TRUE32,  FALSE32, FALSE32, FALSE32, FALSE32},	/* 2Gb SLC(K9F2G08) Mono A-die */
		{0xDA, 0xC1, 2048,  64, 4, FALSE32, FALSE32, FALSE32, FALSE32, FALSE32},	/* 2Gb SLC(K9K2G08) DDP  */
		{0xDC, 0x10, 4096,  64, 4, TRUE32,  FALSE32, FALSE32, FALSE32, FALSE32},	/* 4Gb SLC(K9F4G08) Mono */
		{0xDC, 0xC1, 4096,  64, 4, FALSE32, FALSE32, FALSE32, TRUE32,  FALSE32},	/* 4Gb SLC(K9K4G08) DDP  */
		{0xD3, 0x51, 8192,  64, 4, TRUE32,  FALSE32, FALSE32, TRUE32,  FALSE32},	/* 8Gb SLC(K9K8G08) DDP  */
		{0xD3, 0x10, 4096,  64, 8, TRUE32,  TRUE32,  TRUE32,  FALSE32, FALSE32},	/* 8Gb SLC(K9F8G08) Mono  */			
#endif
#if (WMR_MLC_SUPPORT)
		/* MLC NAND ID TABLE */
		{0xDC, 0x14, 2048, 128, 4, TRUE32,  FALSE32, FALSE32, FALSE32, TRUE32},	/* 4Gb MLC(K9G4G08) Mono */
		{0xD3, 0x55, 4096, 128, 4, TRUE32,  FALSE32, FALSE32, TRUE32,  TRUE32},	/* 8Gb MLC(K9L8G08) DDP  */
		{0xD3, 0x14, 4096, 128, 4, TRUE32,  FALSE32, FALSE32, FALSE32, TRUE32},	/* 8Gb MLC(K9G8G08) Mono */
		{0xD5, 0x55, 8192, 128, 4, TRUE32,  FALSE32, FALSE32, TRUE32,  TRUE32},	/* 16Gb MLC(K9LAG08) DDP  */
		{0xD5, 0x14, 4096, 128, 8, TRUE32,  TRUE32,  TRUE32,  FALSE32, TRUE32},	/* 16Gb MLC(K9GAG08) Mono */			
		{0xD7, 0x55, 8192, 128, 8, TRUE32,  TRUE32,  TRUE32,  TRUE32,  TRUE32},	/* 32Gb MLC(K9LBG08) DDP  */						
#endif			
};

///////////////////////////////////////////////
// PRIVATE variables definitions
///////////////////////////////////////////////
PRIVATE BOOL32 bInternalInterleaving	= FALSE32;
PRIVATE 	BOOL32 aNeedSync[WMR_MAX_DEVICE * 2];

PRIVATE volatile S3C2450_NAND_REG *pNANDFConReg = NULL;
PRIVATE volatile S3C2450_DMA_REG *pDMAConReg = NULL;
PRIVATE volatile S3C2450_MATRIX_REG *pMatrixConReg = NULL;

PRIVATE UINT8 aTempSBuf[NAND_SPAREPAGE_SIZE] =
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

///////////////////////////////////////////////
// Private Function Prototype
///////////////////////////////////////////////
PRIVATE INT32	Read_DeviceID(UINT32 nBank, UINT8 *pDID, UINT8 *pHID);
PRIVATE UINT32	Read_Sector(UINT32 nBank, UINT32 nPpn, UINT32 nSctOffset, UINT8* pBuf, pSECCCxt pSpareCxt, BOOL32 bCheckAllFF);
PRIVATE UINT32	Read_Spare(UINT32 nBank, UINT32 nPpn, pSECCCxt pSpareCxt);
PRIVATE VOID	Write_Sector(UINT32 nPpn, UINT32 nSctOffset, UINT8* pBuf);
PRIVATE VOID	Write_Spare(UINT32 nBank, UINT32 nPpn, pSECCCxt pSpareCxt);
PRIVATE UINT32	Decoding_MainECC(UINT8* pBuf);
PRIVATE UINT32	Decoding_SpareECC(UINT8* pBuf);
PRIVATE UINT32	_IsAllFF(UINT8* pBuf, UINT32 nSize);
PRIVATE UINT32	_TRDelay(UINT32 nNum);
PRIVATE UINT32	_TRDelay2(UINT32 nNum);


///////////////////////////////////////////////
// ECC Decoding Function Return Value
///////////////////////////////////////////////
#define ECC_CORRECTABLE_ERROR					(0x1)
#define ECC_UNCORRECTABLE_ERROR				(0x2)
#define ECC_ALL_FF								(0x4)
//#define PAGE_CORRECTABLE_ERROR_MASK			(0x11111)  // for 2KByte/Page
//#define PAGE_UNCORRECTABLE_ERROR_MASK		(0x22222)  // for 2KByte/Page
#define PAGE_CORRECTABLE_ERROR_MASK			(0x15555)  // for 4KByte/Page
#define PAGE_UNCORRECTABLE_ERROR_MASK		(0x2AAAA)  // for 4KByte/Page
#define PAGE_ALL_FF_MASK						(0x44444)
#define SPARE_ALL_FF_MASK						(0x40000)

///////////////////////////////////////////////
// ECC Decoding Value for All FF
///////////////////////////////////////////////
#define ECCVAL_ALLFF  (0xFFFFFFFF)

#if (NAND_TRANS_MODE == ASM)
extern void _Read_512Byte(UINT8* pBuf);
extern void _Read_512Byte_Unaligned(UINT8* pBuf);
extern void _Write_512Byte(UINT8* pBuf);
extern void _Write_512Byte_Unaligned(UINT8* pBuf);
extern void _Write_Dummy_468Byte_AllFF(void);
extern void _Write_Dummy_436Byte_AllFF(void);
#elif (NAND_TRANS_MODE == DMA)
PRIVATE VOID Read_512Byte_DMA(UINT8* pBuf);
PRIVATE VOID Write_512Byte_DMA(UINT8* pBuf);
PRIVATE VOID Write_Dummy_468Byte_AllFF_DMA(void);
#endif

///////////////////////////////////////////////
// Code Implementation
///////////////////////////////////////////////


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      NAND_Init		                                                     */
/* DESCRIPTION                                                               */
/*      This function inits NAND device.							 		 */
/* PARAMETERS                                                                */
/*      None													             */
/* RETURN VALUES                                                             */
/*		FIL_SUCCESS															 */
/*					NAND_Init is success.									 */
/*		FIL_CRITICAL_ERROR													 */
/*					NAND_Init is failed.									 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
NAND_Init(VOID)
{
	UINT32 nDevIdx;
	UINT32 nScanIdx = 0, nCompIdx = 0;
	UINT8 nDevID, nHiddenID;
	UINT32 nDevCnt = 0;
	BOOL32 bComp = FALSE32;

	NAND_MSG((_T("[FIL]++NAND_MLC_Init()\r\n")));

	pNANDFConReg = (volatile S3C2450_NAND_REG *)0xB1500000;			// 0x4E000000
	pMatrixConReg = (volatile S3C2450_MATRIX_REG *)0xB1600000;			// 0x4E800000
	pDMAConReg = (volatile S3C2450_DMA_REG *)0xB0E00000;				// 0x4B000000

	// Initialize NAND Flash Controller for SLC Large Block NAND Flash
	pNANDFConReg->NFCONF = NF_1BIT_ECC | NF_TACLS(DEFAULT_TACLS) | NF_TWRPH0(DEFAULT_TWRPH0) | NF_TWRPH1(DEFAULT_TWRPH1);
	pNANDFConReg->NFCONT = NF_MAIN_ECC_LOCK | NF_SPARE_ECC_LOCK | NF_INIT_MECC | NF_INIT_SECC | NF_NFCE1 | NF_NFCE0 | NF_NFCON_EN;
	pNANDFConReg->NFSTAT = NF_RNB_READY;	// Clear RnB Transition Detect Bit

	// Initialize EBICON for 2nd nCE pin (nFCE1)
#ifdef	USE_2CE_NAND
	pMatrixConReg->EBICON |= (0x1<<8);	// Bank1_Cfg -> NAND
#endif

	/* Device's ID must be available and equal to each other */
	for(nDevIdx = 0; nDevIdx < WMR_MAX_DEVICE; nDevIdx++)
	{
		nScanIdx = Read_DeviceID(nDevIdx, &nDevID, &nHiddenID);

		if (nScanIdx == FIL_CRITICAL_ERROR)
		{
			nScanIdx = nCompIdx;
			break;
		}
		if ((nCompIdx != nScanIdx) && (bComp))
		{
			return FIL_CRITICAL_ERROR;
		}

		nCompIdx = nScanIdx;
		bComp = TRUE32;
		nDevCnt++;
	}
	
	if(stDEVInfo[nScanIdx].bMLC)
	{
		SET_DevType(WMR_MLC);
	}
	else
	{
		SET_DevType(WMR_SLC);
	}

	if(stDEVInfo[nScanIdx].bDualDie && nDevCnt <= 2)
	{
		/* multi chip dual die (DDP) */
		BLOCKS_PER_BANK = stDEVInfo[nScanIdx].nNumOfBlocks >> 1;
		BANKS_TOTAL = nDevCnt * 2;
		bInternalInterleaving = TRUE32;
	}
	else
	{
		BLOCKS_PER_BANK = stDEVInfo[nScanIdx].nNumOfBlocks;
		BANKS_TOTAL = nDevCnt;
		bInternalInterleaving = FALSE32;
	}

	SECTORS_PER_PAGE = stDEVInfo[nScanIdx].nSectorsPerPage;

	TWO_PLANE_PROGRAM = stDEVInfo[nScanIdx].b2XProgram;

	if (TWO_PLANE_PROGRAM == TRUE32)
	{
		BLOCKS_PER_BANK /= 2;
	}

	TWO_PLANE_READ = stDEVInfo[nScanIdx].b2XRead;

	TWO_PLANE_READ_STATUS = stDEVInfo[nScanIdx].b2XReadStatus;

	PAGES_PER_BLOCK = stDEVInfo[nScanIdx].nPagesPerBlock;

	/* DDP */
	if (bInternalInterleaving)
	{
		for (nDevIdx = 0; nDevIdx < nDevCnt; nDevIdx++)
		{
			aNeedSync[nDevIdx * 2] = FALSE32;
			aNeedSync[nDevIdx * 2 + 1] = FALSE32;
		}
	}

	#if (WMR_MLC_LSB_RECOVERY)
	MLC_LSB_CLASS = GetMlcClass( stDEVInfo[nScanIdx].nDevID, 
								 stDEVInfo[nScanIdx].nHidID);
	#endif

	CalcGlobal(bInternalInterleaving);
	
	NAND_LOG((_T("[FIL] ##############################\r\n")));
	NAND_LOG((_T("[FIL]  FIL Global Information\r\n")));
	NAND_LOG((_T("[FIL]  BANKS_TOTAL = %d\r\n"), BANKS_TOTAL));
	NAND_LOG((_T("[FIL]  BLOCKS_PER_BANK = %d\r\n"), BLOCKS_PER_BANK));
	NAND_LOG((_T("[FIL]  TWO_PLANE_PROGRAM = %d\r\n"), TWO_PLANE_PROGRAM));
	NAND_LOG((_T("[FIL]  SUPPORT_INTERLEAVING = %d\r\n"), IS_SUPPORT_INTERLEAVING));
	NAND_LOG((_T("[FIL]  SUBLKS_TOTAL = %d\r\n"), SUBLKS_TOTAL));
	NAND_LOG((_T("[FIL]  PAGES_PER_SUBLK = %d\r\n"), PAGES_PER_SUBLK));
	NAND_LOG((_T("[FIL]  PAGES_PER_BANK = %d\r\n"), PAGES_PER_BANK));
	NAND_LOG((_T("[FIL]  SECTORS_PER_PAGE = %d\r\n"), SECTORS_PER_PAGE));
	NAND_LOG((_T("[FIL]  SECTORS_PER_SUPAGE = %d\r\n"), SECTORS_PER_SUPAGE));
	NAND_LOG((_T("[FIL]  SECTORS_PER_SUBLK = %d\r\n"), SECTORS_PER_SUBLK));
	NAND_LOG((_T("[FIL]  USER_SECTORS_TOTAL = %d\r\n"), USER_SECTORS_TOTAL));
	NAND_LOG((_T("[FIL]  ADDRESS_CYCLE = %d\r\n"), DEV_ADDR_CYCLE));
	NAND_LOG((_T("[FIL] ##############################\r\n\r\n")));
	NAND_LOG((_T("[INFO] WMR_AREA_SIZE = %d\n"), WMR_AREA_SIZE));
	NAND_LOG((_T("[INFO] SPECIAL_AREA_START = %d\n"), SPECIAL_AREA_START));
	NAND_LOG((_T("[INFO] SPECIAL_AREA_SIZE = %d\n"), SPECIAL_AREA_SIZE));
	NAND_LOG((_T("[INFO] VFL_AREA_START = %d\n"), VFL_AREA_START));
	NAND_LOG((_T("[INFO] VFL_AREA_SIZE = %d\n"), VFL_AREA_SIZE));
	NAND_LOG((_T("[INFO] VFL_INFO_SECTION_START = %d\n"), VFL_INFO_SECTION_START));
	NAND_LOG((_T("[INFO] VFL_INFO_SECTION_SIZE = %d\n"), VFL_INFO_SECTION_SIZE));
	NAND_LOG((_T("[INFO] RESERVED_SECTION_START = %d\n"), RESERVED_SECTION_START));
	NAND_LOG((_T("[INFO] RESERVED_SECTION_SIZE = %d\n"), RESERVED_SECTION_SIZE));
	NAND_LOG((_T("[INFO] FTL_INFO_SECTION_START = %d\n"), FTL_INFO_SECTION_START));
	NAND_LOG((_T("[INFO] FTL_INFO_SECTION_SIZE = %d\n"), FTL_INFO_SECTION_SIZE));
	NAND_LOG((_T("[INFO] LOG_SECTION_SIZE = %d\n"), LOG_SECTION_SIZE));
	NAND_LOG((_T("[INFO] FREE_SECTION_START = %d\n"), FREE_SECTION_START));
	NAND_LOG((_T("[INFO] FREE_SECTION_SIZE = %d\n"), FREE_SECTION_SIZE));
	NAND_LOG((_T("[INFO] FREE_LIST_SIZE = %d\n"), FREE_LIST_SIZE));
	NAND_LOG((_T("[INFO] DATA_SECTION_START = %d\n"), DATA_SECTION_START));
	NAND_LOG((_T("[INFO] DATA_SECTION_SIZE = %d\n"), DATA_SECTION_SIZE));
	NAND_LOG((_T("[INFO] FTL_AREA_START = %d\n"), FTL_AREA_START));
	NAND_LOG((_T("[INFO] FTL_AREA_SIZE = %d\n"), FTL_AREA_SIZE));
	NAND_LOG((_T("[FIL] ##############################\r\n")));
	NAND_LOG((_T("[INFO] IS_CHECK_SPARE_ECC = %d\n"), IS_CHECK_SPARE_ECC));
	NAND_LOG((_T("[INFO] IS_SUPPORT_INTERNAL_INTERLEAVING = %d\n"), IS_SUPPORT_INTERNAL_INTERLEAVING));
	NAND_LOG((_T("[INFO] PAGES_PER_BLOCK = %d\n"), PAGES_PER_BLOCK));

	NAND_MSG((_T("[FIL]--NAND_Init()\r\n")));

	return FIL_SUCCESS;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      NAND_Read		                                                     */
/* DESCRIPTION                                                               */
/*      This function reads NAND page area							 		 */
/* PARAMETERS                                                                */
/*      nBank    	[IN] 	Physical device number			               	 */
/*      nPpn     	[IN] 	Physical page number				         	 */
/*      nSctBitmap  [IN] 	Physical sector bitmap in a page area        	 */
/*      nPlaneBitmap[IN]    The indicator of the plane  					 */
/*      pDBuf		[OUT]	Buffer pointer of main area to read          	 */
/*      pSBuf		[OUT]	Buffer pointer of spare area to read         	 */
/*      bECCIn		[IN] 	Whether read page with ECC value or not      	 */
/*      bCleanCheck [IN] 	When it's TRUE, checks the clean status        	 */
/*							of the page if the data of spare area is all	 */
/*							0xFF, returns PAGE_CLEAN						 */
/* RETURN VALUES                                                             */
/*		FIL_SUCCESS															 */
/*					NAND_Read is success.									 */
/*		FIL_SUCCESS_CLEAN													 */
/*					NAND_Read is success and all data is 0xFF.				 */
/*		FIL_U_ECC_ERROR														 */
/*					ECC value is not correct.								 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
NAND_Read(UINT32 nBank, UINT32 nPpn, UINT32 nSctBitmap, UINT32 nPlaneBitmap,
				UINT8* pDBuf, UINT8* pSBuf, BOOL32 bECCIn, BOOL32 bCleanCheck)
{
	UINT32 nPbn;
	UINT32 nPOffset;

	UINT32 nPageReadStatus = 0;
	UINT32 nPageReadStatus1st = 0;
	UINT32 nPageReadStatus2nd = 0;
	UINT32 nCnt;
	UINT32 nRet = 0;

	BOOL32 bECCErr = FALSE32;
	BOOL32 bPageClean = TRUE32;		// When the data is all 0xFF, regard the page as clean
	BOOL32 bIsSBufNull = FALSE32;	// When the pSBuf is NULL, set to check whether the page is clean or not

	BOOL32 bSecondRead = FALSE32;	// In case of twice read
	BOOL32 bLoopNeed = FALSE32;		// Only for nSctOffset == 8
	BOOL32 bRetry = TRUE32;

	UINT32  nLoopCount;
	UINT32  nVBank;
	UINT32  nPairBank;

	UINT32  nSyncRet;

	pSECCCxt pSpareCxt;

#ifdef	USE_SETKMODE
	BOOL bLastMode;
#endif

	NAND_MSG((_T("[FIL]++NAND_Read(%d, %d, 0x%02x, 0x%02x, %d)\r\n"), nBank, nPpn, nSctBitmap, nPlaneBitmap, bCleanCheck));

	// pDBuf & pSBuf can't be the NULL at the same time
	if (nBank >= BANKS_TOTAL || nPpn >= PAGES_PER_BANK || (pDBuf == NULL && pSBuf == NULL))
	{
		NAND_ERR((_T("[FIL:ERR]--NAND_Read() : Parameter overflow\r\n")));
		WMR_ASSERT(FALSE32);
		return FIL_CRITICAL_ERROR;
	}

	WMR_ASSERT((nPlaneBitmap == enuBOTH_PLANE_BITMAP) || (nPlaneBitmap == enuLEFT_PLANE_BITMAP) || (nPlaneBitmap == enuRIGHT_PLANE_BITMAP));

	nVBank = nBank;	// Do not change nBank before copy to nVBank

	// avoid r/b check error with internal interleaving
	if (bInternalInterleaving == TRUE32)
	{
		nPairBank = ((nBank & 0x1) == 1) ? (nBank - 1) : (nBank + 1);
	}

	/* 
	In case of Internal Interleaving, the first address of the second bank should be 
	the half of toal block number of NAND.
	For example, In 4Gbit DDP NAND, its total block number is 4096.
	So, the bank 0 has 2048 blocks (Physical number : 0 ~ 2047),
	the bank 1 has another 2048 blocks (Physical number : 2048 ~ 4095).
	therefore, first block address of Bank 1 could be the physically 2048th block.
	*/
	if (bInternalInterleaving == TRUE32)
	{
		if ((nBank & 0x1) == 1)
		{
			nPpn += PAGES_PER_BANK;
		}
		nBank /= 2;
	}

	nSctBitmap &= FULL_SECTOR_BITMAP_PAGE;

#if (WMR_STDLIB_SUPPORT)
	nPbn = nPpn / PAGES_PER_BLOCK;
	nPOffset = nPpn % PAGES_PER_BLOCK;
#else
	nPbn = DIV(nPpn, PAGES_PER_BLOCK_SHIFT);
	nPOffset = REM(nPpn, PAGES_PER_BLOCK_SHIFT);
#endif

	// In case of 2 Plane Program, re-calculate the page address
	if (TWO_PLANE_PROGRAM == TRUE32)
	{
		nPpn = nPbn * 2 * PAGES_PER_BLOCK + nPOffset;

		if (nPlaneBitmap == enuBOTH_PLANE_BITMAP)
		{
			if(pDBuf != NULL)
			{
				if ((nSctBitmap & LEFT_SECTOR_BITMAP_PAGE) > 0 && (nSctBitmap & RIGHT_SECTOR_BITMAP_PAGE) > 0)
				{
					// read from both plane
					bLoopNeed = TRUE32;
				}
				else if ((nSctBitmap & LEFT_SECTOR_BITMAP_PAGE) > 0 && (nSctBitmap & RIGHT_SECTOR_BITMAP_PAGE) == 0)
				{
					// read from left plane
					bLoopNeed = FALSE32;
				}
				else if ((nSctBitmap & LEFT_SECTOR_BITMAP_PAGE) == 0 && (nSctBitmap & RIGHT_SECTOR_BITMAP_PAGE) > 0)
				{
					// read from right plane
					bLoopNeed = FALSE32;
					bSecondRead = TRUE32;
				}
			}
			else
			{
				// When read only the spare area, must read twice
				bLoopNeed = TRUE32;
			}
		}
		else if (nPlaneBitmap == enuRIGHT_PLANE_BITMAP)
		{
			nPpn += PAGES_PER_BLOCK;
		}
	}

	nLoopCount = SECTORS_PER_PAGE;

	NAND_Sync(nVBank, &nSyncRet);

	if (bInternalInterleaving == TRUE32)
	{
		// avoid r/b check error with internal interleaving
		aNeedSync[nVBank] = FALSE32;
	}

_B_SecondRead:	// back here again for read right plane

	if(bSecondRead)
	{
		nPpn += PAGES_PER_BLOCK;
		nSctBitmap = (nSctBitmap >> SECTORS_PER_PAGE);

		if(pDBuf != NULL)
		{
			pDBuf += BYTES_PER_MAIN_PAGE;
		}
		else
		{
			// When read only the spare, read 64 + 64 bytes to check the Bad Block
			pSBuf += BYTES_PER_SPARE_PAGE;
		}
	}

#if (WMR_READ_RECLAIM)
	READ_ERR_FLAG = FALSE32;
#endif

	if(pSBuf == NULL)
	{
		pSBuf = aTempSBuf;
		bIsSBufNull = TRUE32;
	}

	pSpareCxt = (pSECCCxt)pSBuf;

	//NAND_Sync(nVBank, &nSyncRet);

#ifdef	USE_SETKMODE
	bLastMode = SetKMode(TRUE);
#endif

	// Chip Select
	NF_CE_L(nBank);

	NF_WAIT_RnB(nBank);

	// Read Command (Always read spare area before main area in a page
	NF_CMD(CMD_READ);
	NF_SET_ADDR(nPpn, BYTES_PER_MAIN_PAGE);
	NF_CMD(CMD_READ_CONFIRM);

	if (bInternalInterleaving == TRUE32)
	{
#if	1
		_TRDelay(nPpn);
#else
		if (nVBank%2)
		{
			NF_CMD(CMD_READ_STATUS_CHIP1);
		}
		else
		{
			NF_CMD(CMD_READ_STATUS_CHIP0);
		}
		while(!(NF_DATA_R()&0x40));

		// Dummy Command to Set Proper Pointer to Read Position after NF_WAIT_RnB()
		NF_CMD(CMD_READ);
#endif
	}
	else
	{
		NF_WAIT_RnB(nBank);

		// Dummy Command to Set Proper Pointer to Read Position after NF_WAIT_RnB()
		NF_CMD(CMD_READ);
	}

	// Read Spare Area
	nRet = Read_Spare(nBank, nPpn, pSpareCxt);
	if ((nRet & ECC_UNCORRECTABLE_ERROR) == 0)
	{
		nPageReadStatus = (nRet<<16);

		if (WMR_GetChkSum(&pSpareCxt->cCleanMark, 1) >= 4)
		{
			// Not Clean Page
			bPageClean = FALSE32;
		}

		// Read Main Area
		if(pDBuf != NULL)
		{
			for (nCnt=0; nCnt<nLoopCount; nCnt++)
			{
				if (nSctBitmap&(0x1<<nCnt))
				{
					nRet = Read_Sector(nBank, nPpn, nCnt, pDBuf, pSpareCxt, bPageClean);
					//nPageReadStatus |= (nRet<<(nCnt*4));  // if 2KByte/Page
					nPageReadStatus |= (nRet<<(nCnt*2));  // if both non-case for all-ff and 4KByte/Page
				}
			}
		}
	}

	// Chip Unselect
	NF_CE_H(nBank);

#ifdef	USE_SETKMODE
	SetKMode(bLastMode);
#endif

	if(bSecondRead)
	{
		nPageReadStatus2nd = nPageReadStatus;
	}
	else
	{
		nPageReadStatus1st = nPageReadStatus;
	}

	if(bLoopNeed && !bSecondRead)
	{
		bSecondRead = TRUE32;
		goto _B_SecondRead;
	}

	if(nPageReadStatus1st&PAGE_UNCORRECTABLE_ERROR_MASK || nPageReadStatus2nd&PAGE_UNCORRECTABLE_ERROR_MASK)
	{
		// Uncorrectable ECC Error
		NAND_ERR((_T("[FIL:ERR]--NAND_Read() : Uncorrectable Error in Bank %d, Page %d [0x%08x] [0x%08x]\r\n"), nBank, nPpn, nPageReadStatus1st, nPageReadStatus));
		return FIL_U_ECC_ERROR;
	}
	else
	{

#if (WMR_READ_RECLAIM)
		if (nPageReadStatus1st&PAGE_CORRECTABLE_ERROR_MASK || nPageReadStatus2nd&PAGE_CORRECTABLE_ERROR_MASK)
		{
			READ_ERR_FLAG = TRUE32;
			NAND_MSG((_T("[FIL:INF] NAND_Read() : Correctable Error in Bank %d, Page %d [0x%08x] [0x%08x]\r\n"), nBank, nPpn, nPageReadStatus1st, nPageReadStatus2nd));
		}
#endif
		if (bCleanCheck&&bPageClean)
		{
			if (bIsSBufNull == FALSE32)
			{
				BOOL32 bClean;

				// Check 32 bytes is all 0xFF & don't care about ECC Value
				if ((pDBuf == NULL) && (bECCIn))
				{
					// When the pMBuf is NULL, read 128 bytes(twice read) in the spare area
					if (bSecondRead)
					{
						pSBuf -= BYTES_PER_SPARE_PAGE;
					}

					bClean = _IsAllFF(pSBuf, BYTES_PER_SPARE_SUPAGE);
				}
				else
				{
					// TODO: to be changed all FF check Size
					bClean = _IsAllFF(pSBuf, ((SECTORS_PER_PAGE == 8) ? NAND_MECC_OFFSET_4K : NAND_MECC_OFFSET));
				}

				if (bClean)
				{
					NAND_MSG((_T("[FIL]--NAND_Read() : FIL_SUCCESS_CLEAN\r\n")));
					return FIL_SUCCESS_CLEAN;
				}
				else
				{
					NAND_MSG((_T("[FIL]--NAND_Read()[bClean==FASLE32]\r\n")));
					return FIL_SUCCESS;
				}
			}
			else
			{
				NAND_MSG((_T("[FIL]--NAND_Read()[bIsSBufNull != FALSE32] : FIL_SUCCESS_CLEAN\r\n")));
				return FIL_SUCCESS_CLEAN;
			}
		}
		else
		{
			NAND_MSG((_T("[FIL]--NAND_Read()[bCleanCheck&&bPageClean == FASLE32]\r\n")));
			return FIL_SUCCESS;
		}
	}
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      NAND_Write		                                                     */
/* DESCRIPTION                                                               */
/*      This function writes NAND page area							 		 */
/* PARAMETERS                                                                */
/*      nBank    	[IN] 	Physical device number			               	 */
/*      nPpn     	[IN] 	Physical page number				         	 */
/*      nSctBitmap 	[IN] 	The indicator for the sector to write         	 */
/*      nPlaneBitmap[IN]    The indicator of the plane  					 */
/*      pDBuf		[IN]	Buffer pointer of main area to write          	 */
/*      pSBuf		[IN]	Buffer pointer of spare area to write         	 */
/*																			 */
/* RETURN VALUES                                                             */
/*		None																 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
VOID
NAND_Write(UINT32 nBank, UINT32 nPpn, UINT32 nSctBitmap,
			UINT32  nPlaneBitmap, UINT8* pDBuf, UINT8* pSBuf)
{
	UINT32 nCnt;
	UINT32 nPbn;
	UINT32 nPOffset;

	//BOOL32	bFirstWrite = TRUE32;
	BOOL32	bSecondWrite = FALSE32;
	BOOL32  bLoopNeed = FALSE32;

	UINT32  nSyncRet;

	pSECCCxt pSpareCxt = NULL;

#ifdef	USE_SETKMODE
	BOOL bLastMode;
#endif

	NAND_MSG((_T("[FIL] ++NAND_Write(%d, %d, 0x%02x, 0x%02x)\r\n"), nBank, nPpn, nSctBitmap, nPlaneBitmap));

	if (nBank >= BANKS_TOTAL || nPpn >= PAGES_PER_BANK || (pDBuf == NULL && pSBuf == NULL))
	{
		NAND_ERR((_T("[FIL:ERR]--NAND_Write() : Parameter Overflow\r\n")));
		WMR_ASSERT(FALSE32);
		return;
	}

	// avoid r/b check error with internal interleaving
	if (bInternalInterleaving == TRUE32)
	{
		aNeedSync[nBank] = TRUE32;
	}

	/* 
	In case of Internal Interleaving, the first address of the second bank should be 
	the half of toal block number of NAND.
	For example, In 4Gbit DDP NAND, its total block number is 4096.
	So, the bank 0 has 2048 blocks (Physical number : 0 ~ 2047),
	the bank 1 has another 2048 blocks (Physical number : 2048 ~ 4095).
	therefore, first block address of Bank 1 could be the physically 2048th block.
	*/
	if (bInternalInterleaving == TRUE32)
	{
		if ((nBank & 0x1) == 1)
		{
			nPpn += PAGES_PER_BANK;
		}
		nBank /= 2;
	}

#if (WMR_STDLIB_SUPPORT)
	nPbn = nPpn / PAGES_PER_BLOCK;
	nPOffset = nPpn % PAGES_PER_BLOCK;
#else
	nPbn = DIV(nPpn, PAGES_PER_BLOCK_SHIFT);
	nPOffset = REM(nPpn, PAGES_PER_BLOCK_SHIFT);
#endif

	// In case of 2-Plane Program, re-calculate the page address
	if(TWO_PLANE_PROGRAM == TRUE32)
	{
		nPpn = nPbn * 2 * PAGES_PER_BLOCK + nPOffset;

		if (nPlaneBitmap == enuBOTH_PLANE_BITMAP)
		{
			bLoopNeed = TRUE32;
		}
		else if (nPlaneBitmap == enuRIGHT_PLANE_BITMAP)
		{
			nPpn += PAGES_PER_BLOCK;
			//nSctBitmap = nSctBitmap>>4;
		}
	}

	nSctBitmap &= FULL_SECTOR_BITMAP_PAGE;

#ifdef	USE_SETKMODE
	bLastMode = SetKMode(TRUE);
#endif

	// CE Select
	NF_CE_L(nBank);

_B_SecondWrite:

	// 2-Plane Program, page address is changed
	if (bSecondWrite)
	{
		nPpn += PAGES_PER_BLOCK;
		nSctBitmap = nSctBitmap >> SECTORS_PER_PAGE;
	}

	if(pSBuf == NULL)
	{
		pSBuf = aTempSBuf;
		memset(pSBuf, 0xFF, BYTES_PER_SPARE_PAGE);		// Initialize the spare buffer
	}
	else
	{
		// Set 0xFF to ECC Area
		if (IS_CHECK_SPARE_ECC == TRUE32)
		{
			pSBuf[2] = 0xff;	// Reserved
			pSBuf[3] = 0xff;
			//WMR_MEMSET(pSBuf+2, 0xFF, 2);						// Clear Reserved area in Spare Buffer
			memset(pSBuf+((SECTORS_PER_PAGE == 8) ? NAND_MECC_OFFSET_4K : NAND_MECC_OFFSET),
					0xFF,
					BYTES_PER_SPARE_PAGE-((SECTORS_PER_PAGE == 8) ? NAND_MECC_OFFSET_4K : NAND_MECC_OFFSET));		// Clear ECC area in Spare Buffer
		}
	}

	pSpareCxt = (pSECCCxt)pSBuf;
	pSpareCxt->cCleanMark = 0x0;	// Clean mark to 0. It means that page is written

	if (bSecondWrite)
	{
		NF_CMD(CMD_2PLANE_PROGRAM);
		NF_SET_ADDR(nPpn, 0);
	}
	else
	{
		NF_CMD(CMD_PROGRAM);
		NF_SET_ADDR(nPpn, 0);
	}

	// Write Main Sector
	if (pDBuf != NULL)
	{
		// In case of the second write, the position of buffer pointer is moved backward as much as 1 page size
		if (bSecondWrite)
		{
			pDBuf += BYTES_PER_MAIN_PAGE;
		}

		for (nCnt=0; nCnt<SECTORS_PER_PAGE; nCnt++)
		{
			if (nSctBitmap&(0x1<<nCnt))
			{
				Write_Sector(nPpn, nCnt, pDBuf);

				if (IS_CHECK_SPARE_ECC == TRUE32)
				{
					pSpareCxt->aMECC[nCnt] = NF_MECC0();
				}
			}
			else
			{
				pSpareCxt->aMECC[nCnt] = ECCVAL_ALLFF;	// All 0xFF ECC
			}
		}
	}
	else if (IS_CHECK_SPARE_ECC == TRUE32)
	{
		for (nCnt=0; nCnt<SECTORS_PER_PAGE; nCnt++)
		{
			pSpareCxt->aMECC[nCnt] = ECCVAL_ALLFF;	// All 0xFF ECC
		}
	}

	// Write Spare
	Write_Spare(nBank, nPpn, pSpareCxt);

	// Write Confirm
	if(TWO_PLANE_PROGRAM == TRUE32 && !bSecondWrite && bLoopNeed)
	{
		bSecondWrite = TRUE32;
		NF_CMD(CMD_2PLANE_PROGRAM_DUMMY);

		_TRDelay2(0);

		goto _B_SecondWrite;
	}

	NF_CMD(CMD_PROGRAM_CONFIRM);

	// Chip Unselect
	NF_CE_H(nBank);

#ifdef	USE_SETKMODE
	SetKMode(bLastMode);
#endif

	NAND_MSG((_T("[FIL]--NAND_Write()\r\n")));

	return;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      NAND_Erase		                                                     */
/* DESCRIPTION                                                               */
/*      This function erases NAND block area						 		 */
/* PARAMETERS                                                                */
/*      nBank    	[IN] 	Physical device number			               	 */
/*      nPpn     	[IN] 	Physical page number				         	 */
/*      nPlaneBitmap[IN]    The indicator of the plane  					 */
/*																			 */
/* RETURN VALUES                                                             */
/*		None																 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
VOID
NAND_Erase (UINT32 nBank, UINT32 nPbn, UINT32 nPlaneBitmap)
{
	UINT32	nVBank;
	UINT32	nPageAddr;
	UINT32 	nPairBank;
	UINT32  nSyncRet;

	BOOL32	bSecondErase = FALSE32;
	BOOL32	bLoopNeed = FALSE32;


#ifdef	USE_SETKMODE
	BOOL bLastMode;
#endif

	NAND_MSG((_T("[FIL]++NAND_Erase(%d, %d, 0x%02x)\r\n"), nBank, nPbn, nPlaneBitmap));

	if (nBank >= BANKS_TOTAL || nPbn >= BLOCKS_PER_BANK)
	{
		NAND_MSG((_T("[FIL:ERR]--NAND_Erase() : Parameter overflow\r\n")));
		WMR_ASSERT(FALSE32);
		return;
	}

	nVBank = nBank;

	// avoid r/b check error with internal interleaving
	if (bInternalInterleaving == TRUE32)
	{
		nPairBank = ((nBank & 0x1) == 1) ? (nBank - 1) : (nBank + 1);
	}

	/* 
	In case of Internal Interleaving, the first address of the second bank should be 
	the half of toal block number of NAND.
	For example, In 4Gbit DDP NAND, its total block number is 4096.
	So, the bank 0 has 2048 blocks (Physical number : 0 ~ 2047),
	the bank 1 has another 2048 blocks (Physical number : 2048 ~ 4095).
	therefore, first block address of Bank 1 could be the physically 2048th block.
	*/
	if (bInternalInterleaving == TRUE32)
	{
		if ((nBank & 0x1) == 1)
		{
			nPbn += BLOCKS_PER_BANK;
		}
		nBank /= 2;
	}

	if (TWO_PLANE_PROGRAM == TRUE32)
	{
		if (nPlaneBitmap == enuBOTH_PLANE_BITMAP)
		{
			bLoopNeed = TRUE32;
		}
		else if (nPlaneBitmap == enuRIGHT_PLANE_BITMAP)
		{
			bSecondErase = TRUE32;
		}
	}

	nPbn = nPbn*(1+(TWO_PLANE_PROGRAM == TRUE32));

	/* 
	   In the Internal Interleaving, it's forbidden NAND to do Write operation & the other operation
	   at the same time. When Write is going on in Bank 1, Bank 0 has to wait to finish 
	   the operation of Bank 1 if the next operation is not Write.

	   While Bank 1 is erased, Bank 0 do not start Write operation. (But, Erase or Read is allowed)
	   Internal Interleaving concept is only existed between Write and Write.	   
	*/
	if (bInternalInterleaving == TRUE32)
	{
		NAND_Sync(nVBank, &nSyncRet);

		// TODO: what does this means???
#if	1
		NF_CE_L(nBank);
		NF_CMD(CMD_READ);
		NF_CE_H(nBank);
#endif

		NAND_Sync(nPairBank, &nSyncRet);

		aNeedSync[nVBank] = TRUE32;
	}

#ifdef	USE_SETKMODE
	bLastMode = SetKMode(TRUE);
#endif

	// Chip Select
	NF_CE_L(nBank);

_B_SecondErase:

	if(bSecondErase)
	{
		nPbn++;
	}

	// Calculate Row Address of the Block (128 page/block)
	nPageAddr = (nPbn << (6 + IS_MLC));

	// Erase Command
	NF_CMD(CMD_ERASE);

	// Write Row Address
	NF_ADDR(nPageAddr&0xff);
	NF_ADDR((nPageAddr>>8)&0xff);
	NF_ADDR((nPageAddr>>16)&0xff);

	if (TWO_PLANE_PROGRAM == TRUE32 && !bSecondErase && bLoopNeed)
	{
		bSecondErase = TRUE32;
		goto _B_SecondErase;
	}

	// Erase confirm command
	NF_CMD( CMD_ERASE_CONFIRM);

	// Chip Unselect
	NF_CE_H(nBank);

#ifdef	USE_SETKMODE
	SetKMode(bLastMode);
#endif

	/* In the Internal Interleaving, it's forbidden NAND to do Write operation & the other operation
	   at the same time. When Write is going on in Bank 1, Bank 0 has to wait to finish 
	   the operation of Bank 1 if the next operation is not Write.

	   While Bank 1 is erased, Bank 0 do not start Write operation. (But, Erase or Read is allowed)
	   Internal Interleaving concept is only existed between Write and Write.	   
	*/
	if (bInternalInterleaving == TRUE32)
	{
		NAND_Sync(nVBank, &nSyncRet);
	}

	NAND_MSG((_T("[FIL]--NAND_Erase()\r\n")));

	return;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      NAND_Sync		                                                     */
/* DESCRIPTION                                                               */
/*      This function checks the R/B signal of NAND					 		 */
/*		When it's busy, it means NAND is under operation.					 */
/*		When it's ready, it means NAND is ready for the next operation.		 */
/* PARAMETERS                                                                */
/*      nBank    	[IN] 	Physical device number			               	 */
/*																			 */
/* RETURN VALUES                                                             */
/*		FIL_CRITICAL_ERROR													 */
/*				When the input value is more than its range					 */
/*				or it has erase fail										 */
/*		FIL_SUCCESS															 */
/*				When the erase operation has done clearly					 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
NAND_Sync(UINT32 nBank, UINT32 *nPlaneBitmap)
{
	UINT32	nData;
	UINT32	nPairBank;
	UINT32	nVBank;

	INT32	nRet = FIL_SUCCESS;

	BOOL32	bInternalBank = FALSE32;

#ifdef	USE_SETKMODE
	BOOL bLastMode;
#endif

	NAND_MSG((_T("[FIL]++NAND_Sync(%d, %d)\r\n"), nBank, nPlaneBitmap));

	if (nBank >= BANKS_TOTAL)
	{
		NAND_ERR((_T("[FIL:ERR]--NAND_Sync() : Parameter overflow\r\n")));
		WMR_ASSERT(FALSE32);
		return FIL_CRITICAL_ERROR;
	}

	WMR_ASSERT(nPlaneBitmap != NULL);

	nVBank = nBank;

	// avoid r/b check error with internal interleaving
	if (bInternalInterleaving == TRUE32)
	{
		nPairBank = ((nBank & 0x1) == 1) ? (nBank - 1) : (nBank + 1);
	}

	if (bInternalInterleaving == TRUE32)
	{
		if ((nBank & 0x1) == 1)
		{
			bInternalBank = TRUE32;
		}
		nBank = (nBank >> 1);
	}

	if ((bInternalInterleaving == TRUE32) && (aNeedSync[nPairBank] == FALSE32))
	{
		// TODO: what does this means???
#if	1
		NF_CE_L(nBank);
		NF_ADDR(0x0);
		NF_CE_H(nBank);
#else
		BLUES_NF_ADDR0(0x0);
		rFMCTRL1 = (1 << BLUES_FM_ADDR_SET);

		BLUES_NF_CPU_CTRL0(nBank);
#endif
	}

#ifdef	USE_SETKMODE
	bLastMode = SetKMode(TRUE);
#endif

	// Chip Select
	NF_CE_L(nBank);

	if (bInternalInterleaving || TWO_PLANE_READ_STATUS)
	{
		if (bInternalBank)
		{
			NF_CMD(CMD_READ_STATUS_CHIP1);
		}
		else
		{
			NF_CMD(CMD_READ_STATUS_CHIP0);
		}
	}
	else
	{
		// Read Status Command is acceptable during Busy
		NF_CMD(CMD_READ_STATUS);
	}

	do
	{
		nData = NF_DATA_R();
	}
	while(!(nData&NAND_STATUS_READY));

	*nPlaneBitmap = enuNONE_PLANE_BITMAP;

	// Read Status
	if (nData&NAND_STATUS_ERROR)
	{
		if (TWO_PLANE_READ_STATUS == TRUE32)
		{
			if (nData & NAND_STATUS_PLANE0_ERROR)
			{
				NAND_ERR((_T("[FIL:ERR] NAND_Sync() : Left-plane Sync Error\r\n")));
				*nPlaneBitmap = enuLEFT_PLANE_BITMAP;
			}
			if (nData & NAND_STATUS_PLANE1_ERROR)
			{
				NAND_ERR((_T("[FIL:ERR] NAND_Sync() : Right-plane Sync Error\r\n")));
				*nPlaneBitmap = enuRIGHT_PLANE_BITMAP;
			}
		}
		else
		{
			NAND_ERR((_T("[FIL:ERR] NAND_Sync() : Status Error\r\n")));
			*nPlaneBitmap = enuLEFT_PLANE_BITMAP;
		}

		nRet = FIL_CRITICAL_ERROR;
	}

	// Chip Unselect
	NF_CE_H(nBank);

#ifdef	USE_SETKMODE
	SetKMode(bLastMode);
#endif

	NAND_MSG((_T("[FIL]--NAND_Sync()\r\n")));

	return nRet;
}

VOID
NAND_Reset(UINT32 nBank)
{
#ifdef	USE_SETKMODE
	BOOL bLastMode;
#endif

	NAND_MSG((_T("[FIL]++NAND_Reset(%d)\r\n"), nBank));

#ifdef	USE_SETKMODE
	bLastMode = SetKMode(TRUE);
#endif

	// Chip Select
	NF_CE_L(nBank);

	// Reset Command is accepted during Busy
	NF_CMD(CMD_RESET);

	// Chip Unselect
	NF_CE_H(nBank);

#ifdef	USE_SETKMODE
	SetKMode(bLastMode);
#endif

	NAND_MSG((_T("[FIL]--NAND_Reset()\r\n")));

	return;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      Read_DeviceID		                                                 */
/* DESCRIPTION                                                               */
/*      This function reads manufacturer id, device id and hidden id. 		 */
/* PARAMETERS                                                                */
/*      nBank    [IN] 		Physical device number				             */
/*      pDID     [OUT] 		NAND flash density id							 */
/*		pHID	 [OUT]		NAND flash hidden id							 */
/* RETURN VALUES                                                             */
/*		nScanIdx			Device's stDEVInfo[nScanIdx]					 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
PRIVATE INT32
Read_DeviceID(UINT32 nBank, UINT8 *pDID, UINT8 *pHID)
{
	UINT8 nMID, nDID, nHID[3];
	UINT32 nScanIdx;
	UINT32 i;
#ifdef	USE_SETKMODE
	BOOL bLastMode;
#endif

	NAND_MSG((_T("[FIL]++Read_DeviceID(%d)\r\n"), nBank));

#ifdef	USE_SETKMODE
	bLastMode = SetKMode(TRUE);
#endif

	// Chip Select
	NF_CE_L(nBank);
	NF_WAIT_RnB(nBank);

	// Read ID Command
	NF_CMD(CMD_READ_ID);
	NF_ADDR(0x00);

	// Find Maker Code
	for (i=0; i<5; i++)
	{
		nMID = NF_DATA_R();		// Maker Code
		if (nMID == 0xEC) break;
	}

	// Read Device Code
	nDID = NF_DATA_R();		// Device Code
	nHID[0] = NF_DATA_R();	// Internal Chip Number
	nHID[1] = NF_DATA_R();	// Page, Block, Redundant Area Size
	nHID[2] = NF_DATA_R();	// Plane Number, Size

	// Chip Unselect
	NF_CE_H(nBank);

#ifdef	USE_SETKMODE
	SetKMode(bLastMode);
#endif

	for (nScanIdx = 0; nScanIdx < sizeof(stDEVInfo)/sizeof(DEVInfo); nScanIdx++)
	{
		if ((nMID == (UINT8)0xEC) && (nDID == stDEVInfo[nScanIdx].nDevID) && (nHID[0] == stDEVInfo[nScanIdx].nHidID))
		{
			*pDID = nDID;
			*pHID = nHID[0];
			
			NAND_LOG((_T("[FIL] ################\r\n")));
			NAND_LOG((_T("[FIL]  MID    = 0x%02x\r\n"), nMID));
			NAND_LOG((_T("[FIL]  DID    = 0x%02x\r\n"), nDID));
			NAND_LOG((_T("[FIL]  HID[0] = 0x%02x\r\n"), nHID[0]));
			NAND_LOG((_T("[FIL]  HID[1] = 0x%02x\r\n"), nHID[1]));
			NAND_LOG((_T("[FIL]  HID[2] = 0x%02x\r\n"), nHID[2]));
			NAND_LOG((_T("[FIL] ################\r\n")));

			NAND_MSG((_T("[FIL]  Bank %d Detect\r\n"), nBank));
			return nScanIdx;
		}
	}

	*pDID = 0x00;
	*pHID = 0x00;

	NAND_MSG((_T("[FIL]--Read_DeviceID()\r\n")));

	return FIL_CRITICAL_ERROR;

}


PRIVATE UINT32
Read_Sector(UINT32 nBank, UINT32 nPpn, UINT32 nSctOffset, UINT8* pBuf, pSECCCxt pSpareCxt, BOOL32 bCheckAllFF)
{
	UINT32 nOffSet;
	UINT32 nRet = 0;
	UINT32 nRetEcc = 0;
	int i = 0;
	unsigned char bECC[4];

	NAND_MSG((_T("[FIL]++Read_Sector(%d, %d)\r\n"), nPpn, nSctOffset));

	// Move pointer to Sector Offset
	nOffSet = nSctOffset * NAND_SECTOR_SIZE;

	// Random data output command
	NF_CMD(CMD_RANDOM_DATA_OUTPUT);
	NF_ADDR(nOffSet&0xFF);
	NF_ADDR((nOffSet>>8)&0xFF);
	NF_CMD(CMD_RANDOM_DATA_OUTPUT_CONFIRM);

	// Initialize 1-bit ECC Decoding
	NF_MECC_Reset();
	NF_MECC_UnLock();

	// Read 512 bytes Sector data
#if (NAND_TRANS_MODE == ASM)
	if ((UINT32)pBuf&0x3)
	{
		_Read_512Byte_Unaligned(pBuf+NAND_SECTOR_SIZE*nSctOffset);
	}
	else
	{
		_Read_512Byte(pBuf+NAND_SECTOR_SIZE*nSctOffset);
	}
#elif (NAND_TRANS_MODE == DMA)
	Read_512Byte_DMA(pBuf+NAND_SECTOR_SIZE*nSctOffset);
#endif

	NF_MECC_Lock();		

	if (bCheckAllFF)
	{
		NF_WRMECCD0(ECCVAL_ALLFF);	// All 0xFF ECC
		NF_WRMECCD1(ECCVAL_ALLFF);
	}
	else
	{
		bECC[0] =  (unsigned char)(((pSpareCxt->aMECC[nSctOffset])>>0)&0xffffffff);
		bECC[1] =  (unsigned char)(((pSpareCxt->aMECC[nSctOffset])>>8)&0xffffffff);
		bECC[2] =  (unsigned char)(((pSpareCxt->aMECC[nSctOffset])>>16)&0xffffffff);
		bECC[3] =  (unsigned char)(((pSpareCxt->aMECC[nSctOffset])>>24)&0xffffffff);

		NF_WRMECCD0(((bECC[1]&0xff)<<16)|(bECC[0]&0xff));
		NF_WRMECCD1(((bECC[3]&0xff)<<16)|(bECC[2]&0xff));
	}
	nRetEcc = NF_ECC_ERR0();

	switch(nRetEcc & 0x3)
	{
	case 0:	// No Error
		break;
	case 1:	// 1-bit Error(Correctable)
		(pBuf)[(nRetEcc>>7)&0x7ff] ^= (1<<((nRetEcc>>4)&0x7));
		nRet = ECC_CORRECTABLE_ERROR;		
		break;
	case 2:	// Multiple Error
	case 3:	// ECC area Error
		nRet = ECC_UNCORRECTABLE_ERROR;			
		break;
	}
	
	NAND_MSG((_T("[FIL]--Read_Sector()\r\n")));

	return nRet;
}

PRIVATE UINT32
Read_Spare(UINT32 nBank, UINT32 nPpn, pSECCCxt pSpareCxt)
{
	UINT32 nOffset;
	UINT32 nRet = 0;
	UINT32 nResult = 0;

	BOOL32 bCheckAllFF = FALSE32;

	NAND_MSG((_T("[FIL]++Read_Spare(%d)\r\n"), nPpn));

	// Read Spare Area
	pSpareCxt->cBadMark = NF_DATA_R();			// 1 byte Bad Mark
	pSpareCxt->cCleanMark = NF_DATA_R();		// 1 byte Clean Mark

	pSpareCxt->cReserved[0]  = NF_DATA_R();		// 2 byte Reserved
	pSpareCxt->cReserved[1]  = NF_DATA_R();

	if (IS_CHECK_SPARE_ECC == TRUE32)
	{
		// Initialize 1-bit ECC Decoding
		NF_SECC_Reset();
		NF_SECC_UnLock();

		if (WMR_GetChkSum(&pSpareCxt->cCleanMark, 1) < 4)
		{
			bCheckAllFF = TRUE32;
		}
	}

	if (SECTORS_PER_PAGE == 4)
	{
		pSpareCxt->aSpareData[0] = NF_DATA_R4();		// 12 byte Spare Context
		pSpareCxt->aSpareData[1] = NF_DATA_R4();
		pSpareCxt->aSpareData[2] = NF_DATA_R4();

		if (IS_CHECK_SPARE_ECC == TRUE32)
		{
			NF_SECC_Lock();
		}

		//CommentPilsun 
		//Only Use 4Byte for SLC ECC x 4 (Because, there is 4 sector in each page)
		pSpareCxt->aMECC[0] = NF_DATA_R4();			// 4 byte Sector0 ECC data
		pSpareCxt->aMECC[1] = NF_DATA_R4();			// 4 byte Sector1 ECC data
		pSpareCxt->aMECC[2] = NF_DATA_R4();			// 4 byte Sector2 ECC data
		pSpareCxt->aMECC[3] = NF_DATA_R4();			// 4 byte Sector3 ECC data
		pSpareCxt->aMECC[4] = NF_DATA_R4();			// 4 byte Sector4 ECC data if 4KByte/Page
		pSpareCxt->aMECC[5] = NF_DATA_R4();			// 4 byte Sector5 ECC data if 4KByte/Page
		pSpareCxt->aMECC[6] = NF_DATA_R4();			// 4 byte Sector6 ECC data if 4KByte/Page
		pSpareCxt->aMECC[7] = NF_DATA_R4();			// 4 byte Sector7 ECC data if 4KByte/Page
	}
	else if (SECTORS_PER_PAGE == 8)
	{
		pSpareCxt->aSpareData[0] = NF_DATA_R4();		// 20 byte Spare Context for 4KByte/Page
		pSpareCxt->aSpareData[1] = NF_DATA_R4();
		pSpareCxt->aSpareData[2] = NF_DATA_R4();
		pSpareCxt->aSpareData[3] = NF_DATA_R4();
		pSpareCxt->aSpareData[4] = NF_DATA_R4();

		if (IS_CHECK_SPARE_ECC == TRUE32)
		{
			NF_SECC_Lock();
		}

		//CommentPilsun 
		//Only Use 4Byte for SLC ECC x 4 (Because, there is 4 sector in each page)
		pSpareCxt->aMECC[0] = NF_DATA_R4();			// 4 byte Sector0 ECC data
		pSpareCxt->aMECC[1] = NF_DATA_R4();			// 4 byte Sector1 ECC data
		pSpareCxt->aMECC[2] = NF_DATA_R4();			// 4 byte Sector2 ECC data
		pSpareCxt->aMECC[3] = NF_DATA_R4();			// 4 byte Sector3 ECC data
		pSpareCxt->aMECC[4] = NF_DATA_R4();			// 4 byte Sector4 ECC data if 4KByte/Page
		pSpareCxt->aMECC[5] = NF_DATA_R4();			// 4 byte Sector5 ECC data if 4KByte/Page
		pSpareCxt->aMECC[6] = NF_DATA_R4();			// 4 byte Sector6 ECC data if 4KByte/Page
		pSpareCxt->aMECC[7] = NF_DATA_R4();			// 4 byte Sector7 ECC data if 4KByte/Page
		pSpareCxt->aMECC[8] = NF_DATA_R4();			// 4 byte dummy data
		pSpareCxt->aMECC[9] = NF_DATA_R4();			// 4 byte dummy data
		pSpareCxt->aMECC[10] = NF_DATA_R4();			// 4 byte dummy data
		pSpareCxt->aMECC[11] = NF_DATA_R4();			// 4 byte dummy data
		pSpareCxt->aMECC[12] = NF_DATA_R4();			// 4 byte dummy data
		pSpareCxt->aMECC[13] = NF_DATA_R4();			// 4 byte dummy data
		pSpareCxt->aMECC[14] = NF_DATA_R4();			// 4 byte dummy data
		pSpareCxt->aMECC[15] = NF_DATA_R4();			// 4 byte dummy data
	}
	else
	{
		WMR_ASSERT(FALSE32);
	}

	if (IS_CHECK_SPARE_ECC == TRUE32)
	{

		if (bCheckAllFF)
		{
			NF_WRSECCD(0x3FF03FF);
		}
		else
		{
			pSpareCxt->aSECC[0] = NF_DATA_R4();	// Read 4 byte Spare ECC data
			pSpareCxt->aSECC[1] = NF_DATA_R4();	// Read 4 byte dummy ECC data
			pSpareCxt->aSECC[2] = NF_DATA_R4();	// Read 4 byte dummy ECC data
			pSpareCxt->aSECC[3] = NF_DATA_R4();	// Read 4 byte dummy ECC data

			NF_WRSECCD(((pSpareCxt->aSECC[0]&0xff)<<16)|(pSpareCxt->aSECC[0]&0xff));

		}

		nResult = NF_ECC_ERR0();		

		if (bCheckAllFF)
		{
			pSpareCxt->aSECC[0] = NF_DATA_R4();	// Read 4 byte Spare ECC data
			pSpareCxt->aSECC[1] = NF_DATA_R4();	// Read 4 byte dummy ECC data
			pSpareCxt->aSECC[2] = NF_DATA_R4();	// Read 4 byte dummy ECC data
			pSpareCxt->aSECC[3] = NF_DATA_R4();	// Read 4 byte dummy ECC data
		}

	}
	else
	{
//CommentPilsun
		// just read Spare ECC from NAND for read pointer, NOT decoding ECC
		pSpareCxt->aSECC[0] = NF_DATA_R4();			// 4 byte Spare ECC data
		pSpareCxt->aSECC[1] = NF_DATA_R4();			// 4 byte dummy ECC data
		pSpareCxt->aSECC[2] = NF_DATA_R4();			// 4 byte dummy ECC data
		pSpareCxt->aSECC[3] = NF_DATA_R4();			// 4 byte dummy ECC data

		NF_WRSECCD(((pSpareCxt->aSECC[0]&0xff)<<16)|(pSpareCxt->aSECC[0]&0xff));	
		nResult = NF_ECC_ERR0();				
	}

	switch(nResult & 0xc)
	{
		case 0:	// No Error
		{
			break;
		}
		case 1:	// 1-bit Error(Correctable)
		{
			NAND_ERR((_T("[FIL:ERR] Read_Spare() : ECC Uncorrectable Error in Spare of Page %d\r\n"), nPpn));
			(pSpareCxt->aSpareData)[(nRet>>21)&0x3] ^= (1<<((nRet>>18)&0x7));
			break;
		}
		case 2:	// Multiple Error
		case 3:	// ECC area Error
		{
			NAND_MSG((_T("[FIL] Read_Spare() : ECC Correctable Error in Spare of Page %d\r\n"), nPpn));
			break;
		}
	}

	NAND_MSG((_T("[FIL]--Read_Spare()\r\n")));

	return nRet;
}

PRIVATE VOID
Write_Sector(UINT32 nPpn, UINT32 nSctOffset, UINT8* pBuf)
{
	UINT32 nOffset;
	int i = 0;

	NAND_MSG((_T("[FIL]++Write_Sector(%d, %d)\r\n"), nPpn, nSctOffset));

	nOffset = NAND_SECTOR_SIZE*nSctOffset;

	NF_CMD(CMD_RANDOM_DATA_INPUT);
	NF_ADDR(nOffset&0xFF);
	NF_ADDR((nOffset>>8)&0xFF);

	if (IS_CHECK_SPARE_ECC == TRUE32)
	{
		NF_MECC_Reset();
		NF_MECC_UnLock();		
	}

#if (NAND_TRANS_MODE == ASM)
	if ((UINT32)pBuf&0x3)
	{
		_Write_512Byte_Unaligned(pBuf+NAND_SECTOR_SIZE*nSctOffset);
	}
	else
	{
		_Write_512Byte(pBuf+NAND_SECTOR_SIZE*nSctOffset);
	}
#elif (NAND_TRANS_MODE == DMA)
	Write_512Byte_DMA(pBuf+NAND_SECTOR_SIZE*nSctOffset);
#endif

	if (IS_CHECK_SPARE_ECC == TRUE32)
	{
		NF_MECC_Lock();		
	}

	NAND_MSG((_T("[FIL]--Write_Sector()\r\n")));

	return;
}

PRIVATE VOID
Write_Spare(UINT32 nBank, UINT32 nPpn, pSECCCxt pSpareCxt)
{
	UINT32 nOffset;

	NAND_MSG((_T("[FIL]++Write_Spare(%d, %d)\r\n"), nBank, nPpn));

	nOffset = BYTES_PER_MAIN_PAGE;

	NF_CMD(CMD_RANDOM_DATA_INPUT);
	NF_ADDR(nOffset&0xFF);
	NF_ADDR((nOffset>>8)&0xFF);

	NF_DATA_W(pSpareCxt->cBadMark);			// 1 byte Bad Mark
	NF_DATA_W(pSpareCxt->cCleanMark);			// 1 byte Clean Mark

#if	1
	NF_DATA_W(0xff);			// 2 byte Reserved
	NF_DATA_W(0xff);
#else
	NF_DATA_W(pSpareCxt->cReserved[0]); 	// 2 byte Reserved
	NF_DATA_W(pSpareCxt->cReserved[1]);
#endif

	if (IS_CHECK_SPARE_ECC == TRUE32)
	{
		// Initialize 1-bit ECC Encoding
		NF_SECC_Reset();
		NF_SECC_UnLock();

	}

	if (SECTORS_PER_PAGE == 4)
	{
		NF_DATA_W4(pSpareCxt->aSpareData[0]);		// 12 byte Spare Context
		NF_DATA_W4(pSpareCxt->aSpareData[1]);
		NF_DATA_W4(pSpareCxt->aSpareData[2]);
		
			//CommentPilsun 
		if (IS_CHECK_SPARE_ECC == TRUE32)
		{
			NF_SECC_Lock();
		}

		//CommentPilsun 
		//Only Use 4Byte for SLC ECC
		NF_DATA_W4(pSpareCxt->aMECC[0]);		// 4 byte Sector0 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[1]);		// 4 byte Sector1 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[2]);		// 4 byte Sector2 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[3]);		// 4 byte Sector3 ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
	}
	else if (SECTORS_PER_PAGE == 8)
	{
		NF_DATA_W4(pSpareCxt->aSpareData[0]);		// 20 byte Spare Context for 4KByte/Page
		NF_DATA_W4(pSpareCxt->aSpareData[1]);
		NF_DATA_W4(pSpareCxt->aSpareData[2]);
		NF_DATA_W4(pSpareCxt->aSpareData[3]);
		NF_DATA_W4(pSpareCxt->aSpareData[4]);
		
			//CommentPilsun 
		if (IS_CHECK_SPARE_ECC == TRUE32)
		{
			NF_SECC_Lock();
		}

		//CommentPilsun 
		//Only Use 4Byte for SLC ECC
		NF_DATA_W4(pSpareCxt->aMECC[0]);		// 4 byte Sector0 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[1]);		// 4 byte Sector1 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[2]);		// 4 byte Sector2 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[3]);		// 4 byte Sector3 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[4]);		// 4 byte Sector4 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[5]);		// 4 byte Sector5 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[6]);		// 4 byte Sector6 ECC data
		NF_DATA_W4(pSpareCxt->aMECC[7]);		// 4 byte Sector7 ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
		NF_DATA_W4(0xffffffff);		// 4 byte dummy ECC data
	}
	else
	{
		WMR_ASSERT(FALSE32);
	}

	if (IS_CHECK_SPARE_ECC == TRUE32)
	{
		pSpareCxt->aSECC[0] = NF_SECC();	// Spare ECC x 2 copies
	}

	NF_DATA_W4(pSpareCxt->aSECC[0]);		// Spare ECC 4 bytes
	NF_DATA_W4(0xffffffff);		// dummy ECC 4 bytes
	NF_DATA_W4(0xffffffff);		// dummy ECC 4 bytes
	NF_DATA_W4(0xffffffff);		// dummy ECC 4 bytes

	NAND_MSG((_T("[FIL]--Write_Spare()\r\n")));

	return;
}

PRIVATE UINT32
Decoding_MainECC(UINT8* pBuf)
{
	UINT32 nError0, nError1;
	UINT32 nErrorCnt, nErrorByte, nErrorPattern;
	UINT32 nRet = 0;

	NAND_MSG((_T("[FIL]++Decoding_MainECC()\r\n")));

	nError0 = NF_ECC_ERR0();
	nError1 = NF_ECC_ERR1();

	//NAND_MSG((_T("[FIL] NF_ECC_ERR0 = 0x%08x()\r\n"), nError0));
	//NAND_MSG((_T("[FIL] NF_ECC_ERR1 = 0x%08x()\r\n"), nError1));

	nErrorCnt = (nError0>>26)&0x7;

	if (nErrorCnt == 0)			// No Error
	{
		NAND_MSG((_T("[FIL] Decoding_MainECC() : No ECC Error\r\n")));
	}
	else	 if (nErrorCnt > 4)			// Uncorrectable Error
	{
		NAND_ERR((_T("[FIL:ERR] Decoding_MainECC() : Uncorrectable Error\r\n")));
		nRet = ECC_UNCORRECTABLE_ERROR;
	}
	else							// Correctable Error
	{
		NAND_MSG((_T("[FIL] Decoding_MainECC() : Correctable Error %d bit\r\n"), nErrorCnt));

		nErrorPattern = NF_ECC_ERR_PATTERN();

		// 1st Bit Error Correction
		nErrorByte = nError0&0x3ff;
		if (nErrorByte < 512)
		{
			NAND_MSG((_T("[FIL] Decoding_MainECC() : 1st Error Buf[%d] [%02x]->"), nErrorByte, pBuf[nErrorByte]));
			pBuf[nErrorByte] = pBuf[nErrorByte]^(nErrorPattern&0xff);
			NAND_MSG((_T("[%02x]\r\n"), pBuf[nErrorByte]));
		}

		if (nErrorCnt > 1)
		{
			// 2nd Bit Error Correction
			nErrorByte = (nError0>>16)&0x3ff;
			if (nErrorByte < 512)
			{
				NAND_MSG((_T("[FIL] Decoding_MainECC() : 2nd Error Buf[%d] [%02x]->"), nErrorByte, pBuf[nErrorByte]));
				pBuf[nErrorByte] = pBuf[nErrorByte]^((nErrorPattern>>8)&0xff);
				NAND_MSG((_T("[%02x]\r\n"), pBuf[nErrorByte]));
			}

			if (nErrorCnt > 2)
			{
				// 3rd Bit Error Correction
				nErrorByte = nError1&0x3ff;
				if (nErrorByte < 512)
				{
					NAND_MSG((_T("[FIL] Decoding_MainECC() : 3rd Error Buf[%d] [%02x]->"), nErrorByte, pBuf[nErrorByte]));
					pBuf[nErrorByte] = pBuf[nErrorByte]^((nErrorPattern>>16)&0xff);
					NAND_MSG((_T("[%02x]\r\n"), pBuf[nErrorByte]));
				}

				if (nErrorCnt > 3)
				{
					// 4 th Bit Error Correction
					nErrorByte = (nError1>>16)&0x3ff;
					if (nErrorByte < 512)
					{
						NAND_MSG((_T("[FIL] Decoding_MainECC() : 4th Error Buf[%d] [%02x]->"), nErrorByte, pBuf[nErrorByte]));
						pBuf[nErrorByte] = pBuf[nErrorByte]^((nErrorPattern>>24)&0xff);
						NAND_MSG((_T("[%02x]\r\n"), pBuf[nErrorByte]));
					}
				}
			}
		}

		nRet = ECC_CORRECTABLE_ERROR;
	}

	NAND_MSG((_T("[FIL]--Decoding_MainECC()\r\n")));

	return nRet;
}

//////////////////////////////////////////////////////////////////////////////
//
//	Meaningful ECC error is first 24 bytes of 512 byte
//
//	SpareData + MECC_Data     + DummyData
//	12 byte   + MECC 8x4 byte + 468 byte Dummy = 512 bytes : 2KByte/Page
//	20 byte   + MECC 8x8 byte + 428 byte Dummy = 512 bytes : 4KByte/Page
//
//////////////////////////////////////////////////////////////////////////////
PRIVATE UINT32
Decoding_SpareECC(UINT8* pBuf)
{
	UINT32 nError0, nError1;
	UINT32 nErrorCnt;
	UINT32 nRet = 0;
	UINT32 nEffectiveByte;
	BOOL32 bDummyError = FALSE32;

	NAND_MSG((_T("[FIL]++Decoding_SpareECC()\r\n")));

	if (SECTORS_PER_PAGE == 8)
	{
		nEffectiveByte = NAND_SECC_OFFSET_4K - NAND_SCXT_OFFSET;  // 20B + 8*8B
	}
	else
	{
		nEffectiveByte = NAND_SECC_OFFSET - NAND_SCXT_OFFSET;  // 12B + 8*4B
	}

	nError0 = NF_ECC_ERR0();
	nError1 = NF_ECC_ERR1();

	//NAND_MSG((_T("[FIL] NF_ECC_ERR0 = 0x%08x()\r\n"), nError0));
	//NAND_MSG((_T("[FIL] NF_ECC_ERR1 = 0x%08x()\r\n"), nError1));

	nErrorCnt = (nError0>>26)&0x7;

	if (nErrorCnt == 0)			// No ECC Error
	{
		NAND_MSG((_T("[FIL] Decoding_SpareECC() : No ECC Error\r\n")));
	}
	else if (nErrorCnt > 4)			// Uncorrectable Error
	{
		NAND_ERR((_T("[FIL:ERR] Decoding_SpareECC() : Uncorrectable Error\r\n")));
		nRet = ECC_UNCORRECTABLE_ERROR;
	}
	else		// Check ECC error occurs in first 44 (12+32) bytes (468 byte is Dummy 0xFF) for 2KByte/Page
	{
		UINT32 nErrorByte, nErrorPattern;
		UINT8 cTempBuf;

		nErrorPattern = NF_ECC_ERR_PATTERN();

		// 1st Bit Error Correction
		nErrorByte = nError0&0x3ff;
		if (nErrorByte < nEffectiveByte)
		{
			cTempBuf = pBuf[nErrorByte];
			pBuf[nErrorByte] = cTempBuf^(nErrorPattern&0xff);
			NAND_MSG((_T("[FIL] Decoding_SpareECC() : 1st Error Buf[%d] [%02x]->[%02x]\r\n"), nErrorByte, cTempBuf, pBuf[nErrorByte]));
		}
		else if (nErrorByte < 512)
		{
			NAND_MSG((_T("[FIL] Decoding_SpareECC() : 1st Error in Dummy Data Buf[%d]\r\n"), nErrorByte));
			bDummyError = TRUE32;
		}

		if (nErrorCnt > 1)
		{
			// 2nd Bit Error Correction
			nErrorByte = (nError0>>16)&0x3ff;
			if (nErrorByte < nEffectiveByte)
			{
				cTempBuf = pBuf[nErrorByte];
				pBuf[nErrorByte] = cTempBuf^((nErrorPattern>>8)&0xff);
				NAND_MSG((_T("[FIL] Decoding_SpareECC() : 2nd Error Buf[%d] [%02x]->[%02x]\r\n"), nErrorByte, cTempBuf, pBuf[nErrorByte]));
			}
			else if (nErrorByte < 512)
			{
				NAND_MSG((_T("[FIL] Decoding_SpareECC() : 2nd Error in Dummy Data Buf[%d]\r\n"), nErrorByte));
				bDummyError = TRUE32;
			}

			if (nErrorCnt > 2)
			{
				// 3rd Bit Error Correction
				nErrorByte = nError1&0x3ff;
				if (nErrorByte < nEffectiveByte)
				{
					cTempBuf = pBuf[nErrorByte];
					pBuf[nErrorByte] = cTempBuf^((nErrorPattern>>16)&0xff);
					NAND_MSG((_T("[FIL] Decoding_SpareECC() : 3rd Error Buf[%d] [%02x]->[%02x]\r\n"), nErrorByte, cTempBuf, pBuf[nErrorByte]));
				}
				else if (nErrorByte < 512)
				{
					NAND_MSG((_T("[FIL] Decoding_SpareECC() : 3rd Error in Dummy Data Buf[%d]\r\n"), nErrorByte));
					bDummyError = TRUE32;
				}

				if (nErrorCnt > 3)
				{
					// 4 th Bit Error Correction
					nErrorByte = (nError1>>16)&0x3ff;
					if (nErrorByte < nEffectiveByte)
					{
						cTempBuf = pBuf[nErrorByte];
						pBuf[nErrorByte] = cTempBuf^((nErrorPattern>>24)&0xff);
						NAND_MSG((_T("[FIL] Decoding_SpareECC() : 4th Error Buf[%d] [%02x]->[%02x]\r\n"), nErrorByte, cTempBuf, pBuf[nErrorByte]));
					}
					else if (nErrorByte < 512)
					{
						NAND_MSG((_T("[FIL] Decoding_SpareECC() : 4th Error in Dummy Data Buf[%d]\r\n"), nErrorByte));
						bDummyError = TRUE32;
					}
				}
			}
		}

		if (bDummyError)			// ECC Error in Dummy Data
		{
			NAND_ERR((_T("[FIL] Decoding_SpareECC() : ECC Error in Dummy Data\r\n")));
			nRet = ECC_UNCORRECTABLE_ERROR;
		}
		else						// Correctable Error
		{
			NAND_MSG((_T("[FIL] Decoding_SpareECC() : Correctable Error %d bits\r\n"), nErrorCnt));
			nRet = ECC_CORRECTABLE_ERROR;
		}
	}

	NAND_MSG((_T("[FIL]--Decoding_SpareECC()\r\n")));

	return nRet;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _IsAllFF		                                                     */
/* DESCRIPTION                                                               */
/*      This function inspects the specific area whether its data is 		 */
/*		all 0xFF or not													     */
/* PARAMETERS                                                                */
/*      pBuf     [IN] 		Data buffer to inspect				             */
/*      pSize    [IN] 		Amount of data to inspect						 */
/* RETURN VALUES                                                             */
/*		FALSE	There is a data that is not 0xFF							 */
/*		TURE	All data is 0xFF											 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
PRIVATE UINT32
_IsAllFF(UINT8* pBuf, UINT32 nSize)
{
	register UINT32 nIdx;
	register UINT32 nLoop;
	UINT32 *pBuf32;

	pBuf32 = (UINT32 *)pBuf;
	nLoop = nSize / sizeof(UINT32);

	for (nIdx = nLoop; nIdx > 0; nIdx--)
	{
		if(*pBuf32++ != 0xFFFFFFFF)
		{
			return FALSE32;
		}
	}

	return TRUE32;
}

/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _TRDelay		                                                     */
/* DESCRIPTION                                                               */
/*      This function wait TR.										 		 */
/* PARAMETERS                                                                */
/*		None																 */
/* RETURN VALUES                                                             */
/*		None																 */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
PRIVATE UINT32
_TRDelay(UINT32 nNum)
{
	volatile int count;

	//count = 2200;		// 55us
	count = 2500;		// 62.5us

	while(count--)
	{
		nNum++;
	}

	return nNum;
}

PRIVATE UINT32
_TRDelay2(UINT32 nNum)
{
	volatile int count;

	count = 50;		// 1.25us

	while(count--)
	{
		nNum++;
	}

	return nNum;
}


#if (NAND_TRANS_MODE == DMA)	// for DMA Trasfer Function

PRIVATE VOID
Read_512Byte_DMA(UINT8* pBuf)
{
	NAND_MSG((_T("[FIL] ++Read_512Byte_DMA()\r\n")));

	pDMAConReg->DISRC0 = 0x4E000010;				// Nand flash data register
	pDMAConReg->DISRCC0 = (0<<1) | (1<<0);			//arc=AHB,src_addr=fix
	pDMAConReg->DIDST0 = NAND_DMA_BUFFER_PA;		// DMA Buffer
	pDMAConReg->DIDSTC0 = (0<<1) | (0<<0);			//dst=AHB, dst_addr=inc;
	pDMAConReg->DCON0 = (1<<31) | (1<<30) | (0<<29) | (1<<28) | (1<<27) | (0<<23) | (1<<22) | (2<<20) | (512/4/4);
									//Handshake, AHB, No interrupt, (4-burst), whole, S/W, no_autoreload, word, count=512/4/4;
	// DMA on and start.
	pDMAConReg->DMASKTRIG0 = (1<<1)|(1<<0);

	// Wait for DMA transfer finished
	while(pDMAConReg->DSTAT0&0xfffff);

	memcpy((void *)pBuf, (void *)NAND_DMA_BUFFER_UA, 512);

	NAND_MSG((_T("[FIL] --Read_512Byte_DMA()\r\n")));
}

PRIVATE VOID
Write_512Byte_DMA(UINT8* pBuf)
{
	NAND_MSG((_T("[FIL] ++Write_512Byte_DMA()\r\n")));

	memcpy( (void *)NAND_DMA_BUFFER_UA, (void *)pBuf, 512);

	// Memory to Nand dma setting
	pDMAConReg->DISRC0 = NAND_DMA_BUFFER_PA;		// DMA Buffer
	pDMAConReg->DISRCC0 = (0<<1) | (0<<0); 			//arc=AHB,src_addr=inc
	pDMAConReg->DIDST0 = 0x4E000010;				// Nand flash data register
	pDMAConReg->DIDSTC0 = (0<<1) | (1<<0); 			//dst=AHB,dst_addr=fix;
	pDMAConReg->DCON0 = (1<<31) | (1<<30) | (0<<29) | (0<<28) | (1<<27) | (0<<23) | (1<<22) | (2<<20) | (512/4);
											//  only unit transfer in writing!!!!
											// Handshake,AHB, No interrupt,(unit),whole,S/W,no_autoreload,word,count=512/4;

	// DMA on and start.
	pDMAConReg->DMASKTRIG0 = (1<<1) | (1<<0);

	// Wait for DMA transfer finished
	while(pDMAConReg->DSTAT0&0xfffff);

	NAND_MSG((_T("[FIL] --Write_512Byte_DMA()\r\n")));
}

PRIVATE VOID
Write_Dummy_468Byte_AllFF_DMA(void)
{
	NAND_MSG((_T("[FIL] ++Write_Dummy_468Byte_AllFF_DMA()\r\n")));

	*((UINT32 *)NAND_DMA_BUFFER_UA) = 0xffffffff;

	// Memory to Nand dma setting
	pDMAConReg->DISRC0 = NAND_DMA_BUFFER_PA;		// DMA Buffer
	pDMAConReg->DISRCC0 = (0<<1) | (1<<0); 			//arc=AHB,src_addr=fix
	pDMAConReg->DIDST0 = 0x4E000010;				// Nand flash data register
	pDMAConReg->DIDSTC0 = (0<<1) | (1<<0); 			//dst=AHB,dst_addr=fix;
	pDMAConReg->DCON0 = (1<<31) | (1<<30) | (0<<29) | (0<<28) | (1<<27) | (0<<23) | (1<<22) | (2<<20) | (468/4);
											//  only unit transfer in writing!!!!
											// Handshake,AHB, No interrupt,(unit),whole,S/W,no_autoreload,word,count=500/4;
	// DMA on and start.
	pDMAConReg->DMASKTRIG0 = (1<<1) | (1<<0);

	// Wait for DMA transfer finished
	while(pDMAConReg->DSTAT0&0xfffff);

	NAND_MSG((_T("[FIL] --Write_Dummy_468Byte_AllFF_DMA()\r\n")));
}

#endif	// #if (NAND_TRANS_MODE == DMA)

#if	1	// for test
void MLC_Set_SpareECC(void)
{
	IS_CHECK_SPARE_ECC = TRUE32;
}

void MLC_Clear_SpareECC(void)
{
	IS_CHECK_SPARE_ECC = FALSE32;
}

UINT32 MLC_Read_RAW(UINT32 nBank, UINT32 nPpn, UINT8 *pMBuf, UINT8 *pSBuf)
{
	UINT32 nCnt;
	UINT32 nRet = 0;

	NAND_MSG((_T("[TST]++MLC_Read_RAW(%d)\r\n"), nPpn));

	NF_CE_L(nBank);

	NF_CMD(CMD_READ);
	NF_SET_ADDR(nPpn, 0x00);
	NF_CMD(CMD_READ_CONFIRM);
	NF_WAIT_RnB(nBank);

	// Dummy Command to Set Proper Pointer to Read Position after NF_WAIT_RnB()
	NF_CMD(CMD_READ);

	// Read Main
	for (nCnt = 0; nCnt < SECTORS_PER_PAGE; nCnt++)
	{
		_Read_512Byte(pMBuf+NAND_SECTOR_SIZE*nCnt);
	}

	// Read Spare
	for (nCnt=0; nCnt<BYTES_PER_SPARE_PAGE; nCnt++)
		*pSBuf++ = NF_DATA_R();

	NF_CE_H(nBank);

	NAND_MSG((_T("[TST]--MLC_Read_RAW()\r\n")));

	return 0;
}

UINT32 MLC_Write_RAW(UINT32 nBank, UINT32 nPpn, UINT8 *pMBuf, UINT8 *pSBuf)
{
	UINT32 nCnt;
	UINT32 nRet = 0;

	NAND_MSG((_T("[TST]++MLC_Write_RAW(%d)\r\n"), nPpn));

	NF_CE_L(nBank);

	NF_CMD(CMD_PROGRAM);
	NF_SET_ADDR(nPpn, 0x00);

	// Write Main
	for (nCnt = 0; nCnt < SECTORS_PER_PAGE; nCnt++)
	{
		_Write_512Byte(pMBuf+NAND_SECTOR_SIZE*nCnt);
	}

	// Write Spare
	for (nCnt=0; nCnt<(BYTES_PER_SPARE_PAGE/4); nCnt++)
		NF_DATA_W(*pSBuf++);

	NF_CMD(CMD_PROGRAM_CONFIRM);

	NF_WAIT_RnB(nBank);

	// Write Status Check
	NF_CMD(CMD_READ_STATUS);

	if (NF_DATA_R()&NAND_STATUS_ERROR)
	{
		NAND_ERR((_T("[TST] MLC_Write_RAW() : Status Error\r\n")));
		nRet = 1;
	}

	NF_CE_H(nBank);

	NAND_MSG((_T("[TST]--MLC_Write_RAW()\r\n")));

	return nRet;
}

UINT32 MLC_Erase_RAW(UINT32 nBank, UINT32 nBlock)
{
	UINT32 RowAddr;
	UINT32 nRet = 0;

	NAND_MSG((_T("[TST]++MLC_Erase_RAW(%d)\r\n"), nBlock));

	// Chip Select
	NF_CE_L(nBank);

	// Calculate Row Address of the Block (128 page/block)
	RowAddr = (nBlock * PAGES_PER_BLOCK);//<< (6 + IS_MLC));

	// Erase Command
	NF_CMD( CMD_ERASE);

	// Write Row Address
	NF_ADDR(RowAddr&0xff);
	NF_ADDR((RowAddr>>8)&0xff);
	NF_ADDR((RowAddr>>16)&0xff);

	// Erase confirm command
	NF_CMD( CMD_ERASE_CONFIRM);

	// Wait for Ready
	NF_WAIT_RnB(nBank);

	// Write Status Check
	NF_CMD(CMD_READ_STATUS);

	if (NF_DATA_R()&NAND_STATUS_ERROR)
	{
		NAND_ERR((_T("MLC_Erase_RAW() : Status Error\r\n")));
		nRet = 1;
	}

	NF_CE_H(nBank);

	NAND_ERR((_T("[TST]--MLC_Erase_RAW()\r\n")));

	return nRet;
}

PRIVATE VOID MLC_Print_SFR(VOID)
{
	NAND_ERR((_T("NFCONF = 0x%08x\r\n"), pNANDFConReg->NFCONF));
	NAND_ERR((_T("NFCONT = 0x%08x\r\n"), pNANDFConReg->NFCONT));
	NAND_ERR((_T("NFMECCD0 = 0x%08x\r\n"), pNANDFConReg->NFMECCD0));
	NAND_ERR((_T("NFMECCD1 = 0x%08x\r\n"), pNANDFConReg->NFMECCD1));
	NAND_ERR((_T("NFSECCD = 0x%08x\r\n"), pNANDFConReg->NFSECCD));
	NAND_ERR((_T("NFSBLK = 0x%08x\r\n"), pNANDFConReg->NFSBLK));

	NAND_ERR((_T("NFEBLK = 0x%08x\r\n"), pNANDFConReg->NFEBLK));
	NAND_ERR((_T("NFSTAT = 0x%08x\r\n"), pNANDFConReg->NFSTAT));
	NAND_ERR((_T("NFECCERR0 = 0x%08x\r\n"), pNANDFConReg->NFECCERR0));
	NAND_ERR((_T("NFECCERR1 = 0x%08x\r\n"), pNANDFConReg->NFECCERR1));
	NAND_ERR((_T("NFMECC0 = 0x%08x\r\n"), pNANDFConReg->NFMECC0));
	NAND_ERR((_T("NFMECC1 = 0x%08x\r\n"), pNANDFConReg->NFMECC1));
	NAND_ERR((_T("NFSECC = 0x%08x\r\n"), pNANDFConReg->NFSECC));
	NAND_ERR((_T("NFMLCBITPT = 0x%08x\r\n"), pNANDFConReg->NFMLCBITPT));
}
#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2001  Microsoft Corporation

Module Name:	S3C2443.H

Abstract:		FLASH Media Driver Interface Samsung S3C2443 CPU with NAND Flash 
                controller.
  
Environment:	As noted, this media driver works on behalf of the FAL to directly
				access the underlying FLASH hardware.  Consquently, this module 
				needs to be linked with FLASHFAL.LIB to produce the device driver 
				named FLASHDRV.DLL.

-----------------------------------------------------------------------------*/
#ifndef _S3C2443_DEVBOARD_
#define _S3C2443_DEVBOARD_

//#include "FMD_LB.h"
//#include "FMD_SB.h"
#include "nand.h"

#define BW_X08                              0
#define BW_X16                              1
#define BW_X32                              2

/*****************************************************************************/
/* S2443 Internal Data Structure Definition                                    */
/*****************************************************************************/
typedef struct
{
	UINT16          nMID;           /* Manufacturer ID               */
	UINT16          nDID;           /* Device ID                     */

	UINT16          nNumOfBlks;     /* Number of Blocks              */
	UINT16          nPgsPerBlk;     /* Number of Pages per block     */
	UINT16          nSctsPerPg;     /* Number of Sectors per page    */
	UINT16          nNumOfPlanes;   /* Number of Planes              */
	UINT16          nBlksInRsv;     /* The Number of Blocks
									   in Reservior for Bad Blocks   */
	UINT8           nBadPos;        /* BadBlock Information Poisition*/
	UINT8           nLsnPos;        /* LSN Position                  */
	UINT8           nECCPos;        /* ECC Policy : HW_ECC, SW_ECC   */
	UINT16          nBWidth;        /* Nand Organization X8 or X16   */

	UINT16          nTrTime;        /* Typical Read Op Time          */
	UINT16          nTwTime;        /* Typical Write Op Time         */
	UINT16          nTeTime;        /* Typical Erase Op Time         */
	UINT16          nTfTime;        /* Typical Transfer Op Time      */
} S2443Spec;

static S2443Spec     astNandSpec[] = {
	/*************************************************************************/
	/* nMID, nDID,                                                           */
	/*            nNumOfBlks                                                 */
	/*                  nPgsPerBlk                                           */
	/*                      nSctsPerPg                                       */
	/*                         nNumOfPlanes                                  */
	/*                            nBlksInRsv                                 */ 
	/*                                nBadPos                                */
	/*                                   nLsnPos                             */
	/*                                      nECCPos                          */
	/*                                         nBWidth                       */
	/*                                                nTrTime                */
	/*                                                    nTwTime            */
	/*                                                         nTeTime       */
	/*                                                                nTfTime*/
	/*************************************************************************/
	/* 8Gbit DDP NAND Flash */
	{ 0xEC, 0xD3, 8192, 64, 4, 2,160, 0, 2, 8, BW_X08, 50, 350, 2000, 50}, 
	/* 4Gbit DDP NAND Flash */
	{ 0xEC, 0xAC, 4096, 64, 4, 2, 80, 0, 2, 8, BW_X08, 50, 350, 2000, 50},
	{ 0xEC, 0xDC, 4096, 64, 4, 2, 80, 0, 2, 8, BW_X08, 50, 350, 2000, 50},
	//{ 0xEC, 0xBC, 4096, 64, 4, 2, 80, 0, 2, 8, BW_X16, 50, 350, 2000, 50},
	//{ 0xEC, 0xCC, 4096, 64, 4, 2, 80, 0, 2, 8, BW_X16, 50, 350, 2000, 50},
	/* 2Gbit NAND Flash */
	{ 0xEC, 0xAA, 2048, 64, 4, 1, 40, 0, 2, 8, BW_X08, 50, 350, 2000, 50},
	{ 0xEC, 0xDA, 2048, 64, 4, 1, 40, 0, 2, 8, BW_X08, 50, 350, 2000, 50},
	//{ 0xEC, 0xBA, 2048, 64, 4, 1, 40, 0, 2, 8, BW_X16, 50, 350, 2000, 50},
	//{ 0xEC, 0xCA, 2048, 64, 4, 1, 40, 0, 2, 8, BW_X16, 50, 350, 2000, 50},
	/* 2Gbit DDP NAND Flash */
	{ 0xEC, 0xDA, 2048, 64, 4, 2, 40, 0, 2, 8, BW_X08, 50, 350, 2000, 50},
	{ 0xEC, 0xAA, 2048, 64, 4, 2, 40, 0, 2, 8, BW_X08, 50, 350, 2000, 50},
	//{ 0xEC, 0xBA, 2048, 64, 4, 2, 40, 0, 2, 8, BW_X16, 50, 350, 2000, 50},
	//{ 0xEC, 0xCA, 2048, 64, 4, 2, 40, 0, 2, 8, BW_X16, 50, 350, 2000, 50},
	/*1Gbit NAND Flash */
	{ 0xEC, 0xA1, 1024, 64, 4, 1, 20, 0, 2, 8, BW_X08, 50, 350, 2000, 50},
	{ 0xEC, 0xF1, 1024, 64, 4, 1, 20, 0, 2, 8, BW_X08, 50, 350, 2000, 50},
	//{ 0xEC, 0xB1, 1024, 64, 4, 1, 20, 0, 2, 8, BW_X16, 50, 350, 2000, 50},
	//{ 0xEC, 0xC1, 1024, 64, 4, 1, 20, 0, 2, 8, BW_X16, 50, 350, 2000, 50},
	/* 1Gbit NAND Flash */	
	{ 0xEC, 0x79, 8192, 32, 1, 4,120, 5, 0, 6, BW_X08, 50, 350, 2000, 50},
	{ 0xEC, 0x78, 8192, 32, 1, 4,120, 5, 0, 6, BW_X08, 50, 350, 2000, 50},
	//{ 0xEC, 0x74, 8192, 32, 1, 4,120,11, 0, 6, BW_X16, 50, 350, 2000, 50},
	//{ 0xEC, 0x72, 8192, 32, 1, 4,120,11, 0, 6, BW_X16, 50, 350, 2000, 50},
	/* 512Mbit NAND Flash */
	{ 0xEC, 0x76, 4096, 32, 1, 4, 70, 5, 0, 6, BW_X08, 50, 350, 2000, 50},
	{ 0xEC, 0x36, 4096, 32, 1, 4, 70, 5, 0, 6, BW_X08, 50, 350, 2000, 50},
	
	/* 512Mbit XP Card */
	{ 0x98, 0x76, 4096, 32, 1, 4, 70, 5, 0, 6, BW_X08, 50, 350, 2000, 50},	
	{ 0x98, 0xfe, 4096, 32, 1, 4, 70, 5, 0, 6, BW_X08, 50, 350, 2000, 50},		
	{ 0x98, 0x79, 4096, 32, 1, 4, 70, 5, 0, 6, BW_X08, 50, 350, 2000, 50},		

	//{ 0xEC, 0x56, 4096, 32, 1, 4, 70,11, 0, 6, BW_X16, 50, 350, 2000, 50},
	//{ 0xEC, 0x46, 4096, 32, 1, 4, 70,11, 0, 6, BW_X16, 50, 350, 2000, 50},
	/* 256Mbit NAND Flash */   
	{ 0xEC, 0x75, 2048, 32, 1, 1, 35, 5, 0, 6, BW_X08, 50, 350, 2000, 50},
	{ 0xEC, 0x35, 2048, 32, 1, 1, 35, 5, 0, 6, BW_X08, 50, 350, 2000, 50},
	//{ 0xEC, 0x55, 2048, 32, 1, 1, 35,11, 0, 6, BW_X16, 50, 350, 2000, 50},
	//{ 0xEC, 0x45, 2048, 32, 1, 1, 35,11, 0, 6, BW_X16, 50, 350, 2000, 50},
	/* 128Mbit NAND Flash */
	{ 0xEC, 0x73, 1024, 32, 1, 1, 20, 5, 0, 6, BW_X08, 50, 350, 2000, 50},
	{ 0xEC, 0x33, 1024, 32, 1, 1, 20, 5, 0, 6, BW_X08, 50, 350, 2000, 50},
	//{ 0xEC, 0x53, 1024, 32, 1, 1, 20,11, 0, 6, BW_X16, 50, 350, 2000, 50},
	//{ 0xEC, 0x43, 1024, 32, 1, 1, 20,11, 0, 6, BW_X16, 50, 350, 2000, 50},

    { 0x00, 0x00,    0,  0, 0, 0,  0, 0, 0, 0,      0,  0,   0,    0,  0}
};


#define POS_BADBLOCK				0x05
#define POS_OEMRESERVED				0x04					// 5th byte in spare area

/*
//  Use Macros here to avoid extra over head for c function calls
#define READ_REGISTER_BYTE(p)		(*(PBYTE)(p))
#define WRITE_REGISTER_BYTE(p, v)	(*(PBYTE)(p)) = (v)
#define READ_REGISTER_USHORT(p)		(*(PUSHORT)(p))
#define WRITE_REGISTER_USHORT(p, v)	(*(PUSHORT)(p)) = (v)
#define READ_REGISTER_ULONG(p)		(*(PULONG)(p))
#define WRITE_REGISTER_ULONG(p, v)	(*(PULONG)(p)) = (v)

#define NF_CE_L()			WRITE_REGISTER_USHORT(pNFCONT, (USHORT) (READ_REGISTER_USHORT(pNFCONT) & ~(1<<1)))
#define NF_CE_H()			WRITE_REGISTER_USHORT(pNFCONT, (USHORT) (READ_REGISTER_USHORT(pNFCONT) | (1<<1)))
#define NF_CMD(cmd)			WRITE_REGISTER_USHORT(pNFCMD, (USHORT) (cmd))
#define NF_ADDR(addr)		WRITE_REGISTER_USHORT(pNFADDR, (USHORT) (addr))
#define NF_DATA_R()			READ_REGISTER_BYTE(pNFDATA)
#define NF_DATA_W(val)		WRITE_REGISTER_BYTE(pNFDATA, (BYTE) (val))
#define NF_DATA_R4()		READ_REGISTER_ULONG(pNFDATA)
#define NF_DATA_W4(val)		WRITE_REGISTER_ULONG(pNFDATA, (ULONG) (val))
#define NF_STAT()			READ_REGISTER_USHORT(pNFSTAT)
#define NF_MECC_UnLock()	WRITE_REGISTER_USHORT(pNFCONT, (USHORT) (READ_REGISTER_USHORT(pNFCONT) & ~(1<<5)))
#define NF_MECC_Lock()		WRITE_REGISTER_USHORT(pNFCONT, (USHORT) (READ_REGISTER_USHORT(pNFCONT) | (1<<5)))
#define NF_SECC_UnLock()	WRITE_REGISTER_USHORT(pNFCONT, (USHORT) (READ_REGISTER_USHORT(pNFCONT) & ~(1<<6)))
#define NF_SECC_Lock()		WRITE_REGISTER_USHORT(pNFCONT, (USHORT) (READ_REGISTER_USHORT(pNFCONT) | (1<<6)))
#define NF_RSTECC()			WRITE_REGISTER_USHORT(pNFCONT, (USHORT) (READ_REGISTER_USHORT(pNFCONT) | (1<<4)))
#define NF_WAITRB()			{while(!(NF_STAT() & (1<<1))) ;}
#define NF_CLEAR_RB()		WRITE_REGISTER_USHORT(pNFSTAT, (USHORT) (READ_REGISTER_USHORT(pNFSTAT) | (1<<2)))
#define NF_DETECT_RB()		{while(!(NF_STAT() & (1<<2)));}
#define NF_ECC()			READ_REGISTER_ULONG(pNFECC)
*/

#ifdef __cplusplus
extern "C"  {
#endif
#ifdef __cplusplus
}
#endif


#endif _S3C2443_DEVBOARD_


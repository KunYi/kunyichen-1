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
#include <fmd.h>
#include <s3c2450_nand.h>
#include <args.h>
#include <ethdbg.h>
#include "Cfnand.h"
#if MAGNETO
#include "args.h"
#include <image_cfg.h>
#endif

#define CHECK_SPAREECC 1

//#define NAND_BASE 0xB1400000	// PA:0x4e00_0000

#define NAND_BASE 0xB1500000


#if MAGNETO
#define MAX_REGIONS 16
#define BAD_BLOCKS_MAS 20    // 2% of total block size

BSP_ARGS *pBSPArgs;
#endif
/*
//  Registers
volatile PUSHORT pNFReg;
volatile PUSHORT pNFCONF;
volatile PUSHORT pNFCONT;
volatile PUSHORT pNFCMD;
volatile PUSHORT pNFADDR;
volatile PULONG  pNFDATA;
volatile PUSHORT pNFSTAT;
volatile PULONG  pNFMECCD0;
volatile PULONG  pNFMECCD1;
volatile PULONG  pNFSECCD;
volatile PULONG  pNFESTAT0;
volatile PULONG  pNFESTAT1;
volatile PULONG  pNFMECC0;
volatile PULONG  pNFMECC1;
volatile PULONG  pNFECC;
volatile PULONG  pNFSECC;
volatile PULONG  pNFSBLK;
volatile PULONG  pNFEBLK;
*/
//BOOL  FMD_ReadUID(UINT8* pUID,UINT8 size);

static volatile S3C2450_NAND_REG *s2450NAND = (S3C2450_NAND_REG *)NAND_BASE;

BOOL  FMD_ReadUID(UINT8* pUID,UINT8* pSIG)
{
	int i;
	UINT8 rddata[2048];
	UINT8 Sparedata[16];	
	RETAILMSG(1,(TEXT("@@@FMD_ReadUID----from uuid libs\r\n")));
//	if(size!=16||pUID==NULL)
//	  return FALSE;

	NF_RSTECC();
	NF_MECC_UnLock();

	NF_nFCE_L();

	NF_CLEAR_RB();
	// 30h-65h is for Samsung chip
	NF_CMD(0x30);
	NF_CMD(0x65);
	NF_CMD(0x00);
	NF_ADDR(0);
	NF_ADDR(0);
	NF_ADDR(0);
	NF_ADDR(0);
	NF_ADDR(0);
	NF_CMD(0x30);

	NF_DETECT_RB();

	for(i=0;i<2048;i++)
	{
		rddata[i] = NF_RDDATA_BYTE();// read one page
	}

	NF_MECC_Lock();
	NF_SECC_UnLock();

	for(i=0;i<16;i++)
	{
		Sparedata[i]=NF_RDDATA_BYTE();
	}
	NF_SECC_Lock();
	NF_nFCE_H();
//#if FMD_DEBUG	
#if 0
	RETAILMSG(1,(TEXT("@@@signature:\r\n")));
	for(i=0;i<16;i++)
	{
		RETAILMSG(1,(TEXT("0x%x "),Sparedata[i]));
	}
	RETAILMSG(1,(TEXT("@@@UID:\r\n")));

	for(i=0;i<512;i++)
	{
//		pUID[i]=rddata[i];
		RETAILMSG(1,(TEXT("0x%x "),rddata[i]));
	}
//	RETAILMSG(1,(TEXT("*************\r\n")));
//#endif	
#endif

	memcpy(pUID,rddata,2048);
       memcpy(pSIG,Sparedata,16);
	   	      // RETAILMSG(1,(TEXT("4\r\n")));

//	SB_NF_Reset(USE_NFCE);
//	RETAILMSG(1,(TEXT("@@@@@@@@@@@@@\r\n")));
 
	//reset NAND FLASH
	NF_nFCE_L();
	NF_CLEAR_RB();
	NF_CMD(0xFF);
//	for(i=0;i<10;i++); 
//	RETAILMSG(1,(TEXT("*******@@@@@*****\r\n")));
	NF_nFCE_H();
//	RETAILMSG(1,(TEXT("@@@FMD_ReadUID end----from uuid libs\r\n")));

	return TRUE;
}


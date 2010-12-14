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
#ifndef __NAND_H__
#define __NAND_H__

//#define MAGNETO 0
/*
typedef struct {
	UINT16		nNumOfBlks;
	UINT16		nPagesPerBlk;
	UINT16		nSctsPerPage;
} NANDDeviceInfo;

NANDDeviceInfo stDeviceInfo;

#ifdef __cplusplus
extern "C"  {
#endif
NANDDeviceInfo GetNandInfo(void);
#ifdef __cplusplus
}
#endif

#define NUM_OF_BLOCKS		(stDeviceInfo.nNumOfBlks)
#define PAGES_PER_BLOCK		(stDeviceInfo.nPagesPerBlk)
#define SECTORS_PER_PAGE		(stDeviceInfo.nSctsPerPage)

#undef SECTOR_SIZE
#define SECTOR_SIZE	(512)
#define NAND_SECTOR_SIZE		(SECTOR_SIZE*SECTORS_PER_PAGE)

#define IS_LB		(SECTORS_PER_PAGE == 4)

#define USE_NFCE				0
#define USE_GPIO				0

#define BADBLOCKMARK                0x00

#define CMD_READID			0x90		//  ReadID
#define CMD_READ			0x00		//  Read
#define CMD_READ2			0x50		//  Read2
#define CMD_READ3               0x30        //  Read3
#define CMD_RESET			0xff		//  Reset
#define CMD_ERASE			0x60		//  Erase phase 1
#define CMD_ERASE2			0xd0		//  Erase phase 2
#define CMD_WRITE			0x80		//  Write phase 1
#define CMD_WRITE2			0x10		//  Write phase 2
#define CMD_STATUS			0x70		//  STATUS
#define CMD_RDI                 0x85        //  Random Data Input
#define CMD_RDO                 0x05        //  Random Data Output
#define CMD_RDO2                0xE0        //  Random Data Output

//  Status bit pattern
#define STATUS_READY		0x40		//  Ready
#define STATUS_ERROR		0x01		//  Error
*/
#ifdef USE_ATAHDD
/* !!! If you use ATA-HDD, NAND timing parameter needs more delay. */
/* !!! The delay value depends on the ATA-HDD. You should find out */
/* !!! the best timing parameter.								   */
						   
#define TACLS			7  //7  //1
#define TWRPH0			7  //7  //6
#define TWRPH1			7  //7  //2

#else

#define TACLS			7  //0    //1
#define TWRPH0			7  //4    //6
#define TWRPH1			7  //2    //2

#endif

#define	STATUS_ILLACC			(1<<5)		//	Illigar Access

#define NF_CMD(cmd)			{s2450NAND->NFCMD   =  (unsigned char)(cmd);}
#define NF_ADDR(addr)		{s2450NAND->NFADDR  =  (unsigned char)(addr);}	

#define NF_nFCE_L()				{s2450NAND->NFCONT &= ~(1<<1);}
#define NF_nFCE_H()				{s2450NAND->NFCONT |=  (1<<1);}
#define NF_ECC_ERR0				(s2450NAND->NFECCERR0)
#define NF_ECC_ERR1				(s2450NAND->NFECCERR1)

//-----------------------------------------

#define NF_RSTECC()				{s2450NAND->NFCONT |=  ((1<<5) | (1<<4));}

#define NF_MECC_UnLock()		{s2450NAND->NFCONT &= ~(1<<7);}
#define NF_MECC_Lock()			{s2450NAND->NFCONT |= (1<<7);}
#define NF_SECC_UnLock()		{s2450NAND->NFCONT &= ~(1<<6);}
#define NF_SECC_Lock()			{s2450NAND->NFCONT |= (1<<6);}

#define NF_CLEAR_RB()			{s2450NAND->NFSTAT |=  (1<<4);}	// Have write '1' to clear this bit.

//#define NF_DETECT_RB()			{ while((s2450NAND->NFSTAT&0x11)!=0x11);} // RnB_Transdetect & RnB
#define NF_DETECT_RB()    		{while(!(s2450NAND->NFSTAT&(1<<4)));}
#define NF_WAITRB()		 		{while (!(s2450NAND->NFSTAT & (1<<0))) ; } 


#define NF_RDDATA_BYTE() 				(s2450NAND->NFDATA)
//#define NF_RDDATA_BYTE() 				(*(volatile unsigned char *)0xb1400010)

//#define NF_RDDATA_WRD2() 				(*(volatile UINT16 *)0xb1400010)
//#define NF_RDDATA_WORD() 				(*(volatile UINT32 *)0xb1400010)
//#define NF_RDDATA_WORD() 				(s2450NAND->NFDATA)
//Q1
#define NF_RDDATA_WRD2() 				(*(volatile UINT16 *)0xB1500000)
#define NF_RDDATA_WORD() 				(*(volatile UINT32 *)0xB1500000)


#define NF_WRDATA_BYTE(data)			 {s2450NAND->NFDATA  =  (UINT8)(data);}
//#define NF_WRDATA_WRD2(data)			 {*(volatile UINT16 *)((UINT16)0xb1400010)  =  (UINT16)(data);}
//#define NF_WRDATA_WORD(data)			 {*(volatile UINT32 *)((UINT32)0xb1400010)  =  (UINT32)(data);}
//Q1
#define NF_WRDATA_WRD2(data)			 {*(volatile UINT16 *)((UINT16)0xB1500000)  =  (UINT16)(data);}
#define NF_WRDATA_WORD(data)			 {*(volatile UINT32 *)((UINT32)0xB1500000)  =  (UINT32)(data);}

#define NF_RDMECC0()			(s2450NAND->NFMECC0)
#define NF_RDMECC1()			(s2450NAND->NFMECC1)
#define NF_RDSECC()				(s2450NAND->NFSECC)

#define NF_RDMECCD0()			(s2450NAND->NFMECCD0)
#define NF_RDMECCD1()			(s2450NAND->NFMECCD1)
#define NF_RDSECCD()			(s2450NAND->NFSECCD)

#define NF_WRMECCD0(data)		{s2450NAND->NFMECCD0 = (data);}
#define NF_WRMECCD1(data)		{s2450NAND->NFMECCD1 = (data);}
#define NF_WRSECCD(data)		{s2450NAND->NFSECCD = (data);}

#define NF_RDSTAT				(s2450NAND->NFSTAT)

//#define NF_CE_L()  				{(s2450NAND->NFCONT &=~(1<<1));}
//#define NF_CE_H()					{(s2450NAND->NFCONT |=(1<<1));}
//#define NF_DATA_R()                       (*(PBYTE)(s2450NAND->NFDATA))
//#define NF_DATA_R4()			(*(PULONG)(s2450NAND->NFDATA))

#define NF_DATA_R()                       (s2450NAND->NFDATA)
#define NF_DATA_R4()			(s2450NAND->NFDATA)

#endif	// __NAND_H_.


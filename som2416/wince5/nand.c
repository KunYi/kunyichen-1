#include <windows.h>
#include "s2450addr.h"
#include "utils.h"
#include "nand.h"

#define DEV_ADDR_CYCLE (5)

///////////////////////////////////////////////
// NAND Controller Macro
///////////////////////////////////////////////
#define NF_CE_L(bank)		{	\
								if (bank == 0) rNFCONT &= ~NF_NFCE0;	\
								else if (bank == 1) rNFCONT &= ~NF_NFCE1;	\
							}
#define NF_CE_H(bank)		{	\
								if (bank == 0) rNFCONT |= NF_NFCE0;	\
								else if (bank == 1) rNFCONT |= NF_NFCE1;		\
							}
#define NF_CMD(cmd)			(rNFCMD = (unsigned char)cmd)
#define NF_ADDR(addr)		(rNFADDR = (unsigned char)addr)
#define NF_DATA_R()			(rNFDATA8)
#define NF_DATA_R4()		(rNFDATA32)
#define NF_DATA_W4(n)		(rNFDATA32 = (DWORD)n)
#define NF_SET_ECC_DEC()	(rNFCONT &= ~NF_4BIT_ECC_ENC)
#define NF_MECC_Reset()		(rNFCONT |= NF_INIT_MECC)
#define NF_MECC_Lock()		(rNFCONT |= NF_MAIN_ECC_LOCK)
#define NF_MECC_UnLock()	(rNFCONT &= ~NF_MAIN_ECC_LOCK)
#define NF_CLEAR_ECC_DEC_DONE()	(rNFSTAT |= NF_ECC_DEC_DONE)
#define NF_WAIT_ECC_DEC_DONE()	{	\
										while(!(rNFSTAT&NF_ECC_DEC_DONE));	\
									}

#define NF_WRMECCD0(data)			{rNFMECCD0 = (data);}
#define NF_WRMECCD1(data)			{rNFMECCD1 = (data);}
#define NF_ECC_DEC_ERROR()			((rNFECCERR0>>26)&0x7)
#define NF_ECC_ERR0()				(rNFECCERR0)
#define NF_ECC_ERR1()				(rNFECCERR1)
#define NF_ECC_ERR_PATTERN()		(rNFMLCBITPT)


#define NF_DETECT_RB()	{	\
								while(!(rNFSTAT&NF_RNB_READY));	\
							}

#define NF_WAIT_RnB(bank)	{	\
								NF_CMD(CMD_READ_STATUS);		\
								while(!(NF_DATA_R()&0x40));			\
							}


#define NF_SET_ADDR(nPpn, nOffset)	{	\
										NF_ADDR(nOffset&0xFF);			\
										NF_ADDR((nOffset>>8)&0xFF);		\
										NF_ADDR(nPpn&0xFF);			\
										NF_ADDR((nPpn>>8)&0xFF);		\
										if (DEV_ADDR_CYCLE > 4)			\
											NF_ADDR((nPpn>>16)&0xFF);	\
									}

#define NF_SET_CLK(tacls, twrph0, twrph1)		(rNFCONF = (rNFCONF&~0x7770)		\
											|NF_TACLS(tacls) | NF_TWRPH0(twrph0) | NF_TWRPH1(twrph1))


///////////////////////////////////////////////
// Assembly Read Function (in nand.s)
///////////////////////////////////////////////
void _Read_512Byte(unsigned char* pBuf);
void _Write_Dummy_Byte_AllFF(int nByteSize);  // nByteSize is the multifilication of 4
int CorrectECC8Data(unsigned char *pEncodingBaseAddr);


void NAND_Init(void)
{

	// Initialize NAND Flash Controller for MLC NAND Flash
	rNFCONF = NF_8BIT_ECC | NF_TACLS(DEFAULT_TACLS) | NF_TWRPH0(DEFAULT_TWRPH0) | NF_TWRPH1(DEFAULT_TWRPH1);
	rNFCONT = NF_MAIN_ECC_LOCK | NF_SPARE_ECC_LOCK | NF_INIT_MECC | NF_INIT_SECC | NF_NFCE1 | NF_NFCE0 | NF_NFCON_EN;
	rNFSTAT = NF_RNB_READY;	// Clear RnB Transition Detect Bit

	rNFCONF = rNFCONF & ~(1<<30);
	rNFCONT |= (1<<18)|(1<<13)|(1<<12)|(1<<11)|(1<<10)|(1<<9); //ECC for programming.// Enable RnB Interrupt 
	rNFSTAT |= ((1<<6)|(1<<5)|(1<<4));

	NAND_Reset(0);
}

void NAND_Reset(DWORD dwBank)
{
	// Chip Select
	NF_CE_L(dwBank);

	// Reset Command is accepted during Busy
	NF_CMD(CMD_RESET);

	// Chip Unselect
	NF_CE_H(dwBank);
}


BOOL NAND_Read(DWORD dwBank, DWORD dwPage, unsigned char *pBuf, BOOL b4KPage)
{
	DWORD SpareDataDummy;
	DWORD dwOffset;
	DWORD dwCnt;
	DWORD dwSpareAddress = 2048 + 1;	//include Bad block

	unsigned char uSctCnt;
	BOOL bRet = TRUE;

	if (b4KPage == TRUE) uSctCnt = 8;
	else uSctCnt = 4;

	rNFCONF = ( rNFCONF&~(0x3<<23) ) |(2<<30)|(1<<23)|(0x7<<12)|(0x7<<8)|(0x7<<4);
	rNFCONT = ( rNFCONT&~(0x1<<18) ) |(0<<18)|(0<<11)|(0<<10)|(0<<9)|(1<<6)|(1<<0); // Init NFCONT	
	rNFSTAT |= ((1<<6)|(1<<5)|(1<<4));

	NAND_Reset(dwBank);
	
	NF_MECC_Lock(); // Main ECC Lock
	NF_CE_L(dwBank);
	rNFSTAT |= (1<<4); // RnB Clear


	NF_CMD(CMD_READ);
	NF_SET_ADDR(dwPage, 0x00);
	NF_CMD(CMD_READ_CONFIRM);
	NF_DETECT_RB();
	rNFSTAT |= (1<<4); // RnB Clear

	//READ Spare ECC Data
	for(dwCnt = 0; dwCnt < uSctCnt; dwCnt++) {
		NF_MECC_Lock(); // Main ECC Lock
		rNFSTAT |= (1<<4); // RnB Clear

		if(!dwCnt) {
			NF_CMD(CMD_READ);
			NF_SET_ADDR(dwPage, 0);
			NF_CMD(CMD_READ_CONFIRM);

			NF_DETECT_RB();
			rNFSTAT |= (1<<4); // RnB Clear			
		}
		else
		{
			dwOffset = dwCnt * 512;
			NF_CMD(CMD_RANDOM_DATA_OUTPUT);
			NF_ADDR(dwOffset&0xFF);
			NF_ADDR((dwOffset>>8)&0xFF);
			NF_CMD(CMD_RANDOM_DATA_OUTPUT_CONFIRM);
		}

		NF_MECC_UnLock(); 	// Main ECC Unlock
		NF_MECC_Reset();	 // Initialize ECC
 
		//read 512byte
		_Read_512Byte(pBuf + 512*dwCnt);

                if (b4KPage == TRUE)
                        dwOffset = 4096 + (dwCnt * 13);
                else
		dwOffset = 2048 + (dwCnt * 13);

		NF_CMD(CMD_RANDOM_DATA_OUTPUT);
		NF_ADDR(dwOffset&0xFF);
		NF_ADDR((dwOffset>>8)&0xFF);
		NF_CMD(CMD_RANDOM_DATA_OUTPUT_CONFIRM);

		//read Spare ECC Data		
		SpareDataDummy = NF_DATA_R4();
		SpareDataDummy = NF_DATA_R4();
		SpareDataDummy = NF_DATA_R4();
		SpareDataDummy = NF_DATA_R();

		NF_MECC_Lock(); // Main ECC Lock

		while(!(rNFSTAT&(1<<6))); 	// Check decoding done 
		rNFSTAT = rNFSTAT | (1<<6);   // Decoding done Clear

		while( rNF8ECCERR0 & (unsigned int)(1<<31) ) ; // 8bit ECC Decoding Busy Check.

		if(CorrectECC8Data((pBuf + 512*dwCnt))) {
			return FALSE;

		}		
	}

	NF_CE_H(dwBank);
	
	return TRUE;
}




int CorrectECC8Data(unsigned char *pEncodingBaseAddr)
{	
	unsigned int i,uErrorByte[9];
	unsigned char uErrorBit[9];
	unsigned int uErrorType;
	
	
	uErrorType = (rNF8ECCERR0>>25)&0xf;// Searching Error Type //How many Error bits does exist?	
	uErrorByte[1] = rNF8ECCERR0&0x3ff;// Searching Error Byte //Where is the error byte?
	uErrorByte[2] = (rNF8ECCERR0>>15)&0x3ff;	
	uErrorByte[3] = (rNF8ECCERR1)&0x3ff;
	uErrorByte[4] = (rNF8ECCERR1>>11)&0x3ff;	
	uErrorByte[5] = (rNF8ECCERR1>>22)&0x3ff;	
	uErrorByte[6] = (rNF8ECCERR2)&0x3ff;
	uErrorByte[7] = (rNF8ECCERR2>>11)&0x3ff;
	uErrorByte[8] = (rNF8ECCERR2>>22)&0x3ff;
	
	uErrorBit[1] = rNFMLC8BITPT0&0xff;// Searching Error Bit //Where is the error bit?
	uErrorBit[2] = (rNFMLC8BITPT0>>8)&0xff;
	uErrorBit[3] = (rNFMLC8BITPT0>>16)&0xff;
	uErrorBit[4] = (rNFMLC8BITPT0>>24)&0xff;	
	uErrorBit[5] = rNFMLC8BITPT1&0xff;
	uErrorBit[6] = (rNFMLC8BITPT1>>8)&0xff;
	uErrorBit[7] = (rNFMLC8BITPT1>>16)&0xff;
	uErrorBit[8] = (rNFMLC8BITPT1>>24)&0xff;
	
	if(uErrorType == 0x0) {
		return 0;
	}

	if(uErrorType == 0x9) {
		return 1;
	}
	for(i=1;i<=uErrorType ;i++)	
	{
		if(uErrorByte[i] < 512)	
			pEncodingBaseAddr[uErrorByte[i]]^=uErrorBit[i];
		else
		{;	}			
	}
	return 0;
	
}



void Read_DeviceID(DWORD dwBank, unsigned char *pDID, unsigned char *pHID)
{
	unsigned char ucMID, ucDID, ucHID[3];
	int i;

	// Chip Select
	NF_CE_L(dwBank);
	NF_WAIT_RnB(dwBank);

	// Read ID Command
	NF_CMD(CMD_READ_ID);
	NF_ADDR(0x00);

	// Find Maker Code
	for (i=0; i<10; i++)
	{
		ucMID = NF_DATA_R();		// Maker Code
		if (ucMID == 0xEC) break;
	}

	// Read Device Code
	ucDID = NF_DATA_R();		// Device Code
	ucHID[0] = NF_DATA_R();		// Internal Chip Number
	ucHID[1] = NF_DATA_R();		// Page, Block, Redundant Area Size
	ucHID[2] = NF_DATA_R();		// Plane Number, Size

	// Chip Unselect
	NF_CE_H(dwBank);

	if (ucMID == 0xEC)
	{
		*pDID = ucDID;
		*pHID = ucHID[0];
	}
	else
	{
		*pDID = 0x00;
		*pHID = 0x00;
	}
}


//[david.modify] 2008-04-26 09:59
#define __POLLING (1)
#define		NUM_ID				8
#define		SIZE_OF_MAN_NAME	64
#define		SLC_TYPE			0				
#define		MLC_TYPE			1	
#define		N_ROW_ADDRESS		8	

typedef struct st_NandInfo
{
	unsigned int uNId;
	unsigned char uId[NUM_ID];
	char sName[SIZE_OF_MAN_NAME];
	unsigned int uNandSize;
	unsigned int uColCycle;     //PageAddr      
	unsigned int uRowCycle;		//Nth PageAddr  
	unsigned int uAddrCycle;	//uAddrCycle = uColCycle + uRowCycle
	unsigned int uIsMLC;
	unsigned int uAddrCycleNum;
	unsigned int uBlockShift;
	unsigned char uRowAddr[N_ROW_ADDRESS];
	unsigned int u512BytesPerPage;
	
	unsigned int uCurBlock;
	unsigned int uCurPage;
	unsigned int uPagesPerBlock;
	
	//3rd ID data
	unsigned int uIntChipNum;
	unsigned int uCellType;
	unsigned int uNSimProgPages; //Num of Simultaneously Prgrammed Pages
	unsigned int uInterleaveProg;//Interleave Program Between multiple chips
	unsigned int uCacheProg;    //Cache Program
	
	//4th ID Data
	unsigned int uPageSize;
	unsigned int uBlockSize;
	unsigned int uRedundantAreaSize;
	unsigned int uOrganization;
	unsigned int uSerialAccessMin;
		
	//5th ID Data
	unsigned int uPlaneNum;
	unsigned int uPlaneSize;	
		
}NANDINFO;

#define FAIL 0
#define OK   1

#define TACLS		7	// 1-clk(7.5ns) 
#define TWRPH0		7	// 3-clk(22.5ns)
#define TWRPH1		7	// 2-clk(15ns)  //TACLS+TWRPH0+TWRPH1>=30ns

#define NF_nFCE_L()			{rNFCONT&=~(1<<1);}
#define NF_CLEAR_RB()    		{rNFSTAT |= (1<<4);}	// Have write '1' to clear this bit.

#define NF_DETECT_RB()    		{while(!(rNFSTAT&(1<<4)));}
#define NF_RSTECC()			{rNFCONT|=(1<<5)|(1<<4);}
#define NF_RDDATA8() 		((*(volatile unsigned char*)0x4E000010) )
#define NF_nFCE_H()			{rNFCONT|=(1<<1);}



#define rNF8ECCERR0	  	(*(volatile unsigned *)0x4E000044)  		
#define rNF8ECCERR1  		(*(volatile unsigned *)0x4E000048)
#define rNF8ECCERR2    		(*(volatile unsigned *)0x4E00004C)
#define rNFM8ECC0			(*(volatile unsigned *)0x4E000050)
#define rNFM8ECC1			(*(volatile unsigned *)0x4E000054)
#define rNFM8ECC2			(*(volatile unsigned *)0x4E000058)
#define rNFM8ECC3			(*(volatile unsigned *)0x4E00005C)
#define rNFMLC8BITPT0		(*(volatile unsigned *)0x4E000060)
#define rNFMLC8BITPT1		(*(volatile unsigned *)0x4E000064)

/*
 * Standard NAND flash commands
 */
#define CMD_READ0				0x00
#define CMD_READID				0x90
#define CMD_ERASE2				0xd0
#define CMD_RESET				0xff


/* Extended commands for large page devices */
#define CMD_READ0_2CYCLE		0x30
#define CMD_CACHEDPROG				0x15
#define	CMD_RANDOM_DATA_OUT_1ST_CYCLE	0x05
#define	CMD_RANDOM_DATA_OUT_2ND_CYCLE	0xE0
#define	CMD_RANDOM_DATA_IN			0x85

#define		PAGE_LEN_ECC8		512
#define		VAL_LEN_ECC8		13

static unsigned char g_uSpareData[128];
NANDINFO	sNandInfo;

//[david.modify] 2007-07-11 20:27
//================================
#define NBL_PRINT 
#define __POLLING (1)

int CorrectECC8Data2(unsigned char *pEncodingBaseAddr)
{	
	unsigned int i,uErrorByte[9];
	unsigned char uErrorBit[9];
	unsigned int uErrorType;
	
	
	uErrorType = (rNF8ECCERR0>>25)&0xf;// Searching Error Type //How many Error bits does exist?	
	uErrorByte[1] = rNF8ECCERR0&0x3ff;// Searching Error Byte //Where is the error byte?
	uErrorByte[2] = (rNF8ECCERR0>>15)&0x3ff;	
	uErrorByte[3] = (rNF8ECCERR1)&0x3ff;
	uErrorByte[4] = (rNF8ECCERR1>>11)&0x3ff;	
	uErrorByte[5] = (rNF8ECCERR1>>22)&0x3ff;	
	uErrorByte[6] = (rNF8ECCERR2)&0x3ff;
	uErrorByte[7] = (rNF8ECCERR2>>11)&0x3ff;
	uErrorByte[8] = (rNF8ECCERR2>>22)&0x3ff;
	
	uErrorBit[1] = rNFMLC8BITPT0&0xff;// Searching Error Bit //Where is the error bit?
	uErrorBit[2] = (rNFMLC8BITPT0>>8)&0xff;
	uErrorBit[3] = (rNFMLC8BITPT0>>16)&0xff;
	uErrorBit[4] = (rNFMLC8BITPT0>>24)&0xff;	
	uErrorBit[5] = rNFMLC8BITPT1&0xff;
	uErrorBit[6] = (rNFMLC8BITPT1>>8)&0xff;
	uErrorBit[7] = (rNFMLC8BITPT1>>16)&0xff;
	uErrorBit[8] = (rNFMLC8BITPT1>>24)&0xff;
	
	if(!uErrorType) 
		return 0;
	if(uErrorType == 0x9) 
		return 1;
	
	for(i=1;i<=uErrorType ;i++)	
	{
		if(uErrorByte[i] < 512)	
			pEncodingBaseAddr[uErrorByte[i]]^=uErrorBit[i];
		else
		{;	}			
	}
	return 0;
	
}

//
#if 0
// S805G NAND FLASH基本信息
===========================================================
1th ID : 0xec	 Manufacterer
2nd ID : 0xd3	 Device ID
3rh ID : 0x14
4th ID : 0xa5
5th ID : 0x64
Nand Size :  1024 MB, 	1073741824 B			// 1G BYTES
Page Size : 2048 Bytes						// 每PAGE = 2048 BYTES
Block Size : 256 KBytpes					//每个BLOCK = 256KB * 4000 = 1GBYTES

512Bytes Per Page : 4
Nand Col Cycle : 2
Nand Row Cycle : 3
Nand Row Addr By Block Size : 7
Pages per Block : 128 pages					//1// 1个BLOCK有 128 pages * 2048 = 256KB

Nand Total Address : A30
Is MLC ? : 1
Inter Chip Number : 1
Cell Type : 4 Level Cell
Number of Simultaneously Programmed Pages : 2
Interleave Program Between multiple chips : 0
Cache Program : 0
Redundant Area Size : 16(byte/512bytes)
Organization : x8
Serial Access Minimum : 0
Plane Number : 2
Plane Size : 512MB
===========================================================
#endif

unsigned int PrintNandInfo2(void)
{
	if(!sNandInfo.uPageSize)
	{
		NBL_PRINT("\n Error : No Page Size");
	}
	else
	{		
	NBL_PRINT("\n===========================================================");
	NBL_PRINT("\n1th ID : 0x%x\t Manufacterer", sNandInfo.uId[0]);
	NBL_PRINT("\n2nd ID : 0x%x\t Device ID", sNandInfo.uId[1]);
	NBL_PRINT("\n3rh ID : 0x%x", sNandInfo.uId[2]);
	NBL_PRINT("\n4th ID : 0x%x", sNandInfo.uId[3]);
#if 0	
	Uart_SendString("NAND ID:");
	Uart_SendBYTE(sNandInfo.uId[0], 1);	
	Uart_SendBYTE(sNandInfo.uId[1], 1);		
	Uart_SendBYTE(sNandInfo.uId[2], 1);	
	Uart_SendBYTE(sNandInfo.uId[3], 1);	
#endif	
	NBL_PRINT("\n5th ID : 0x%x", sNandInfo.uId[4]);
	NBL_PRINT("\nNand Size :  %ld MB, \t%d B", sNandInfo.uNandSize>>20, sNandInfo.uNandSize);
	NBL_PRINT("\nPage Size : %d Bytes", sNandInfo.uPageSize);
	NBL_PRINT("\nBlock Size : %d KBytpes\n", sNandInfo.uBlockSize >> 10);
	NBL_PRINT("\n512Bytes Per Page : %d", sNandInfo.u512BytesPerPage);
	
	NBL_PRINT("\nNand Col Cycle : %d", sNandInfo.uColCycle);
	NBL_PRINT("\nNand Row Cycle : %d", sNandInfo.uRowCycle);	
	NBL_PRINT("\nNand Row Addr By Block Size : %d", sNandInfo.uBlockShift);
	NBL_PRINT("\nPages per Block : %d pages\n", sNandInfo.uPagesPerBlock);
	
	NBL_PRINT("\nNand Total Address : A%d", sNandInfo.uAddrCycleNum);
	NBL_PRINT("\nIs MLC ? : %d", sNandInfo.uIsMLC);
	NBL_PRINT("\nInter Chip Number : %d", sNandInfo.uIntChipNum);
	NBL_PRINT("\nCell Type : %d Level Cell", sNandInfo.uCellType);
	NBL_PRINT("\nNumber of Simultaneously Programmed Pages : %d", sNandInfo.uNSimProgPages);
	NBL_PRINT("\nInterleave Program Between multiple chips : %d", sNandInfo.uInterleaveProg);
	NBL_PRINT("\nCache Program : %d", sNandInfo.uCacheProg);	
	NBL_PRINT("\nRedundant Area Size : %d(byte/512bytes)", sNandInfo.uRedundantAreaSize);
	NBL_PRINT("\nOrganization : x%d", sNandInfo.uOrganization);
	NBL_PRINT("\nSerial Access Minimum : %d", sNandInfo.uSerialAccessMin);
	NBL_PRINT("\nPlane Number : %d", sNandInfo.uPlaneNum);
	NBL_PRINT("\nPlane Size : %dMB", sNandInfo.uPlaneSize>>20);
	NBL_PRINT("\n===========================================================");
	}
	return 0;	
}


//Normal Initalization of NFC
void InitNFC2(void)
{
	rNFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4)|(0<<0);		
	rNFCONT = (0<<17)|(0<<16)|(0<<10)|(0<<9)|(0<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(0x3<<1)|(1<<0);			  
}

void ReadID2(void)
{
	unsigned char uId[16], i, uScan;	
	unsigned int uTemp, uSize;
	
	memset(&sNandInfo,0,sizeof(NANDINFO));
	
	InitNFC2();
	NBL_PRINT("\nK9F5608 : 2 IDs");
	NBL_PRINT("\nK9F1208 : 4 IDs");
	NBL_PRINT("\nK9HBG08U1M / K9MZG08U3M / K9LAG08U0M / K9MCG08U5M : 5 IDs");
	NBL_PRINT("\nK9GAG08U0M / K9HCG08U5M / K9GAG08B0M / K9LBG08U1M : 5 IDs");	
	NBL_PRINT("\n\nHow Many IDs? (2/3/4/5/6/7) : ");
		
	sNandInfo.uNId = 5;

	NF_nFCE_L();
	NF_CMD(CMD_READID);
    NF_ADDR(0x00);

	for (i=0; i<10; i++);//delay

	for(i=0; i<sNandInfo.uNId; i++)	
		sNandInfo.uId[i] = NF_RDDATA8();
				
	NF_nFCE_H();		
	
	
	if((sNandInfo.uId[0]==0xEC || sNandInfo.uId[0]==0x98) && ((sNandInfo.uId[1]==0x76)||(sNandInfo.uId[1]==0x75)))	sNandInfo.uNId = 4;
	
	if(sNandInfo.uNId < 5)
	{
		sNandInfo.uPageSize = 512;
		sNandInfo.uNandSize = 64*1024*1024;
		sNandInfo.uColCycle = 1;
		if((sNandInfo.uId[1]==0x75))
		{			
			sNandInfo.uRowCycle = 2;
		}
		else
		{			
			sNandInfo.uRowCycle = 3;
		}
		sNandInfo.uBlockSize = 16*1024;
		sNandInfo.u512BytesPerPage = 1;
		sNandInfo.uRedundantAreaSize = 16;
		
		//Scan Cell Type	
		uTemp = (sNandInfo.uId[2]>>2)& 0x03;
		sNandInfo.uCellType = 1;
		for(i=0;i<=uTemp;i++)	sNandInfo.uCellType *= 2;
		sNandInfo.uIsMLC = uTemp;		
	}
	else
	{
		//------> 3rd ID
		uTemp = sNandInfo.uId[2]& 0x03;
		//Scan Internal Chip Number
		sNandInfo.uIntChipNum = 1;					
		for(i=0;i<uTemp;i++)	sNandInfo.uIntChipNum *= 2;				
			
		//Scan Cell Type	
		uTemp = (sNandInfo.uId[2]>>2)& 0x03;
		sNandInfo.uCellType = 1;
		for(i=0;i<=uTemp;i++)	sNandInfo.uCellType *= 2;
		sNandInfo.uIsMLC = uTemp;
			
			
		uTemp = (sNandInfo.uId[2]>>4)& 0x03;
		sNandInfo.uNSimProgPages = 1;
		for(i=0;i<uTemp;i++)	sNandInfo.uNSimProgPages *= 2;				
			
		sNandInfo.uInterleaveProg = (sNandInfo.uId[2]>>6)& 0x01;
		sNandInfo.uCacheProg = (sNandInfo.uId[2]>>7)& 0x01;
		
		//------> 4th ID
		uTemp = sNandInfo.uId[3]& 0x03;
		sNandInfo.uPageSize = 1024;					
		for(i=0;i<uTemp;i++)	sNandInfo.uPageSize *=2;
		sNandInfo.u512BytesPerPage = sNandInfo.uPageSize >> 9;		
					
		uTemp = (sNandInfo.uId[3]>>2)& 0x01;
		if(uTemp)		sNandInfo.uRedundantAreaSize = 16;
		else			sNandInfo.uRedundantAreaSize = 8;
		
		uTemp = (sNandInfo.uId[3]>>4)& 0x03;
		sNandInfo.uBlockSize = 64*1024;
		for(i=0;i<uTemp;i++)	sNandInfo.uBlockSize *=2;
					
		uTemp = (sNandInfo.uId[3]>>6)& 0x01;//TBD
		if(uTemp)		sNandInfo.uOrganization = 16;
		else			sNandInfo.uOrganization = 8;

		//------> 5th ID
		uTemp = (sNandInfo.uId[4]>>2)& 0x03;
		sNandInfo.uPlaneNum = 1;					
		for(i=0;i<uTemp;i++)	sNandInfo.uPlaneNum *= 2;				
			
		uTemp = (sNandInfo.uId[4]>>4)& 0x07;
		sNandInfo.uPlaneSize = (8388608); //64*1024*1024/8
		for(i=0;i<uTemp;i++)	sNandInfo.uPlaneSize *=2;	
		
		sNandInfo.uNandSize = sNandInfo.uPlaneNum*sNandInfo.uPlaneSize;
		
		uTemp = sNandInfo.uPageSize;		
		for(i=0; uTemp>1; i++)		uTemp>>=1;
		uTemp = i;
		sNandInfo.uAddrCycleNum+=uTemp;
		for(i=1; uTemp>8; i++)		uTemp-=8;		
		sNandInfo.uColCycle = i;		
		
		uTemp = sNandInfo.uPageSize;
		uSize = sNandInfo.uNandSize;
		for(i=0; uTemp>1; i++)
		{	
			uTemp>>=1;			
			uSize>>=1;
		}
		for(i=0; uSize>1; i++)			uSize>>=1;
		uSize = i;
		sNandInfo.uAddrCycleNum+=uSize;
		for(i=1; uSize>8; i++)			uSize-=8;
		sNandInfo.uRowCycle = i;			
		
	}	
	sNandInfo.uAddrCycle = sNandInfo.uColCycle + sNandInfo.uRowCycle;	
	uTemp = sNandInfo.uPageSize;
	uSize = sNandInfo.uBlockSize;
	
	for(i=0; uTemp>1; i++)
	{	
		uTemp>>=1;			
		uSize>>=1;
	}
	if(sNandInfo.u512BytesPerPage > 1)  //large block
	{		
		rNFCONF |= (1<<2);
		if(sNandInfo.uAddrCycle ==4) //addr cycle : 4
		{
			rNFCONF &= ~(1<<1);
			if(sNandInfo.u512BytesPerPage == 4)
			{
				rNFCONF &= ~(1<<3);
			}
			else
			{
				rNFCONF |= (1<<3);
			}
		}
		else							//addr cycle : 5
		{
			rNFCONF |= (1<<1);
			if(sNandInfo.u512BytesPerPage == 4)
			{
				rNFCONF &= ~(1<<3);
			}
			else
			{
				rNFCONF |= (1<<3);
			}
		}		
	}
	else  //small block
	{
		rNFCONF &= ~(1<<2);
		if(sNandInfo.uAddrCycle ==3)
		{
			rNFCONF &= ~(1<<1);			//addr cycle : 3
			if(sNandInfo.u512BytesPerPage == 1)
			{
				rNFCONF &= ~(1<<3);
			}
			else
			{
				rNFCONF |= (1<<3);
			}
		}
		else
		{
			rNFCONF |= (1<<1);			//addr cycle : 4
			if(sNandInfo.u512BytesPerPage == 1)
			{
				rNFCONF &= ~(1<<3);
			}
			else
			{
				rNFCONF |= (1<<3);
			}
		}		
	}
	sNandInfo.uPagesPerBlock = uSize;
	for(i=0; uSize>1; i++)			uSize>>=1;	
	sNandInfo.uBlockShift = i;
	PrintNandInfo2();		
}

int ReadPage2(unsigned char* uRBuf,unsigned int  uBlock, unsigned int uPage)
{
	int i,m;		
	unsigned char Addr_Cycle, option2;
	unsigned int uResult[8]={0,	};
	unsigned char *pSpare;
	unsigned int uCurPageAddr, uRet=0;
	pSpare = g_uSpareData;

 //[david.modify] 2008-04-26 14:32
//	Uart_SendString("ReadPage2");
	
	uCurPageAddr =  uBlock*sNandInfo.uPagesPerBlock + uPage;	 	
	
		//rNFCONF &= ~(0x3<<23)|(1<<23);
	//rNFCONT = ( rNFCONT&~(0x1<<18) ) |(0<<18)|(1<<11)|(1<<10)|(1<<9); // Init NFCONT
	rNFCONT &= ~(0x1<<18);
	rNFCONF &= ~(0x3<<23);
	rNFCONF|=(1<<23);
	rNFSTAT = rNFSTAT|(1<<6)|(1<<7)|(1<<5)|(1<<4);	
		
	NF_MECC_Lock(); // Main ECC Lock
	NF_nFCE_L(); // Chip Select Low
	NF_CLEAR_RB(); // RnB Clear
	
#if		__POLLING
#else	
	rNFCONT |= (1<<12);
	rNFCONT |= (1<<11);
	rNFCONT |= (1<<10);
	rNFCONT |= (1<<9);
	
	NFConDone=0;
	NFECCDecDone=0;
	NFECCEncDone=0;
	
    pISR_NFCON= (unsigned)IsrNFC;
    rSRCPND=BIT_NFCON;
    rINTMSK=~(BIT_NFCON);
#endif
	
	
	
	for(i=0; i<sNandInfo.u512BytesPerPage; i++)
	{
		NF_MECC_Lock(); // Main ECC Lock
		NF_CLEAR_RB(); // RnB Clear
		if(!i)
		{			
			NF_CMD(CMD_READ0);	// Read command		
			for(m=0; m<sNandInfo.uColCycle; m++) //Addr Cycle
				NF_ADDR(0);
			for(m=0; m<sNandInfo.uRowCycle; m++)
				NF_ADDR((uCurPageAddr>>(m<<3))&0xFF);			
			NF_CMD(CMD_READ0_2CYCLE);	// Read command	
			
#if __POLLING
				NF_DETECT_RB();
				rNFSTAT = rNFSTAT | (1<<4); // RnB Pending Clear
#else
			    while(!NFConDone);
			    NFConDone = 0;				
#endif
		}
		else
		{
			NF_CMD(CMD_RANDOM_DATA_OUT_1ST_CYCLE); // Random Address Access = 512K start
			NF_ADDR(((PAGE_LEN_ECC8*i)>>0)&0xFF);
			NF_ADDR(((PAGE_LEN_ECC8*i)>>8)&0xFF);
			NF_CMD(CMD_RANDOM_DATA_OUT_2ND_CYCLE);
		}
		
		NF_MECC_UnLock(); // Main ECC Unlock
		NF_RSTECC();    // Initialize ECC
		
		for(m=0;m<PAGE_LEN_ECC8;m++)  
			*uRBuf++=NF_RDDATA8();	// Read Main data
			
		// Spare Area Address Setting.
		if(sNandInfo.uPageSize > 512)
		{			
			NF_CMD(CMD_RANDOM_DATA_OUT_1ST_CYCLE);
			NF_ADDR(((sNandInfo.uPageSize + VAL_LEN_ECC8*i)>>0)&0xFF);
			NF_ADDR(((sNandInfo.uPageSize + VAL_LEN_ECC8*i)>>8)&0xFF);
			NF_CMD(CMD_RANDOM_DATA_OUT_2ND_CYCLE);
		}
		
		
		for(m=0; m<VAL_LEN_ECC8 ; m++)
			*pSpare++ = NF_RDDATA8(); // Spare Data Read

		NF_MECC_Lock(); // Main ECC Lock

#if		__POLLING
		while(!(rNFSTAT&(1<<6))); 	// Check decoding done 
		rNFSTAT = rNFSTAT | (1<<6);   // Decoding done Clear
#else
		while(!NFECCDecDone);
		NFECCDecDone=0;
#endif		
		while( rNF8ECCERR0 & (unsigned int)(1<<31) ) ; // 8bit ECC Decoding Busy Check.
		uResult[i] = CorrectECC8Data2((uRBuf-PAGE_LEN_ECC8));
		//NBL_PRINT("\nuResult[%d] : %d", i, uResult[i]);
		uRet += uResult[i];
	}
#if		__POLLING	
	if(rNFSTAT&0x20)
	{
		rNFSTAT|=0x20;		
		NBL_PRINT("\nError to erase a block");
		NF_nFCE_H();  // Chip Select is High
		return FAIL;
	}
#else		
	rNFCONT&=~(1<<9);
	rNFCONT&=~(1<<10); // Disable Illegal Access Interrupt
	rNFCONT&=~(1<<12);
#endif
	NF_nFCE_H();  // Chip Select is High
	
	return uRet;
}
	 //[david.modify] 2008-04-28 10:53
typedef void (*PFN_IMAGE_LAUNCH)();	 
int ReadPage_test()
{
	unsigned char *pBuf;
	unsigned char * downPt;
	unsigned char * PTestAddr, *PTestAddr2,*PTestAddr3;
	UINT32 u32TestBytes = 0;
	unsigned int i;
	int nRet = 0;
	unsigned int block=0, page=0;
	BOOL b4KPage = 0;


	ReadID2();
#ifndef LOAD_ADDRESS_PHYSICAL
#define LOAD_ADDRESS_PHYSICAL		(0x30000000)//(0x30038000)		
#define 	_NONCACHE_STARTADDRESS		0x31000000
#endif

	
	downPt=(unsigned char *)LOAD_ADDRESS_PHYSICAL;
	PTestAddr = downPt;
	for(i=0;i<128;i++)
		g_uSpareData[i]=0xFF;	
#if 1	
//	Uart_SendString("ReadPage2 nRet:\r\n");		
	page = 4;
	for(i=page;i<128;i++) {
		nRet = ReadPage2(downPt, block, page);
		page++;
#if 0		
		Uart_SendDWORD(i, 1);				
		Uart_SendString("-");
		Uart_SendDWORD(nRet, 1);			
		Uart_SendString("\r\n");
#endif		
		downPt +=sNandInfo.uPageSize;
	}

	Uart_Print(PTestAddr, 512);
	Uart_SendString("\r\n");
//	Uart_Print(PTestAddr+0x2000, 512);

	Uart_SendString("Jump to 2nd Bootloader...\r\n");
	Uart_SendDWORD(PTestAddr, 1);	
	Uart_SendString("\r\n");	
#if 0
	Uart_SendString("enter into while\r\n");	 		
	while(1){};
#endif	
// 打印EBOOT 内存
	PTestAddr2=(unsigned char *)_NONCACHE_STARTADDRESS;
	PTestAddr3 = PTestAddr2;
	page = 0;block=3;
	for(i=page;i<128;i++) {
		nRet = ReadPage2(PTestAddr2, block, page);
		page++;
#if 0		
		Uart_SendDWORD(i, 1);				
		Uart_SendString("-");
		Uart_SendDWORD(nRet, 1);			
		Uart_SendString("\r\n");
#endif		
		PTestAddr2 +=sNandInfo.uPageSize;
	}
	Uart_Print(PTestAddr3, 512);
	Uart_SendString("\r\n");	


// 打印EBOOT 内存
	Uart_SendString("NAND_Read==>\r\n");	
	PTestAddr2=(unsigned char *)_NONCACHE_STARTADDRESS;
	PTestAddr3 = PTestAddr2;
	page = 0;block=0;
	b4KPage = b4KPage;	// b4KPage?
	for(i=page;i<128;i++) {
		nRet = NAND_Read(block, page, PTestAddr2, b4KPage);
		page++;
		PTestAddr2 +=sNandInfo.uPageSize;
	}
	Uart_Print(PTestAddr3, 0x2000);
	Uart_SendString("\r\n");		
	


	((PFN_IMAGE_LAUNCH)(PTestAddr))();
	
	while(1);
	
//	nRet = ReadPage2(downPt, block, page);
//	Uart_SendString("ReadPage2 nRet:");
//	Uart_SendDWORD(nRet, 1);
#else
	Uart_SendString("NAND_Read nRet:\r\n");		
	for(i=0;i<10;i++) {
		nRet = NAND_Read(block, page, downPt);
		page++;
		Uart_SendDWORD(i, 1);				
		Uart_SendString("-");
		Uart_SendDWORD(nRet, 1);			
		Uart_SendString("\r\n");
		downPt +=sNandInfo.uPageSize;
	}
//	nRet = ReadPage2(downPt, block, page);
//	Uart_SendString("ReadPage2 nRet:");
//	Uart_SendDWORD(nRet, 1);



#endif
	
	return nRet;

}







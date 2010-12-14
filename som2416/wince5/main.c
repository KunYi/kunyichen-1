//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include <windows.h>
#include <bsp.h>
#include <pehdr.h>
#include <romldr.h>
#include "WMRConfig.h"
#include "WMRTypes.h"
#include "FIL.h"

ROMHDR * volatile const pTOC = (ROMHDR *)-1;

// Base address of the image flash part.
//
// NOTE: This is the base address of the NAND flash controller...
//

#if (BSP_TYPE == BSP_SMDK2443)
#define EBOOT_VIRTUAL_BASEADDR	0x8C038000
#define IPL_VIRTUAL_BASEADDR		0x8C138000
#elif (BSP_TYPE == BSP_SMDK2450)
#define EBOOT_VIRTUAL_BASEADDR	0x80038000
#define IPL_VIRTUAL_BASEADDR		0x80138000
#endif



#define VFL_EBOOT_START_BLOCK	(3)
#define VFL_EBOOT_AREA_SIZE	(5)	// (2 Block + Reserved 3 Block for MLC)(4 Block + Reserved 1 Block for SLC)
#define VFL_EBOOT_BYTE_SIZE	(0x80000)
#define VFL_EBOOT_START_PAGE	(VFL_EBOOT_START_BLOCK*PAGES_PER_SUBLK)
#define VFL_EBOOT_PAGE_SIZE	((VFL_EBOOT_BYTE_SIZE-1)/(BYTES_PER_SECTOR*SECTORS_PER_SUPAGE)+1)
#define VFL_EBOOT_BLOCK_SIZE	((VFL_EBOOT_BYTE_SIZE-1)/(BYTES_PER_MAIN_SUPAGE*PAGES_PER_SUBLK)+1)

void Launch(UINT32 ulLaunchAddr);
void OEMInitDebugSerial(void);
void OEMWriteDebugString(unsigned short *str);

#if	0
BOOL SetKMode(BOOL mode)	// to avoid link error with FIL.lib by dodan2 061106
{
	return TRUE;
}
#endif

void OEMLaunchImage(UINT32 ulLaunchAddr)
{
    UINT32 ulPhysicalJump = 0;

    // The IPL is running with the MMU on - before we jump to the loaded image, we need to convert
    // the launch address to a physical address and turn off the MMU.
    //

    // Convert jump address to a physical address.
    ulPhysicalJump = OALVAtoPA((void *)ulLaunchAddr);

	//RETAILMSG(1, (L"LaunchAddr=0x%x PhysicalJump=0x%x\r\n", ulLaunchAddr, ulPhysicalJump));
    // Jump...
    Launch(ulPhysicalJump);
}

static BOOLEAN SetupCopySection(ROMHDR *const pTOC)
{
	UINT32 ulLoop;
	COPYentry *pCopyEntry;

	if (pTOC == (ROMHDR *const) -1)
	{
		return(FALSE);
	}

	// This is where the data sections become valid... don't read globals until after this
	//
	for (ulLoop = 0; ulLoop < pTOC->ulCopyEntries; ulLoop++)
	{
		pCopyEntry = (COPYentry *)(pTOC->ulCopyOffset + ulLoop*sizeof(COPYentry));
		if (pCopyEntry->ulCopyLen)
		{
			memcpy((LPVOID)pCopyEntry->ulDest, (LPVOID)pCopyEntry->ulSource, pCopyEntry->ulCopyLen);
		}
		if (pCopyEntry->ulCopyLen != pCopyEntry->ulDestLen)
		{
			memset((LPVOID)(pCopyEntry->ulDest+pCopyEntry->ulCopyLen), 0, pCopyEntry->ulDestLen-pCopyEntry->ulCopyLen);
		}
	}

	return(TRUE);
}

void ShadowEboot(void)
{
	LowFuncTbl *pLowFuncTbl;

	UINT32 dwStartBlock, dwNumBlock, dwBlock;
	UINT32 dwPageOffset;
	INT32 nRet;

	UINT8 pSBuf[256];
	UINT8 *pBuffer;
	BOOL32 bIsBadBlock = FALSE32;
	
	memset(pSBuf, 0xFF, BYTES_PER_SPARE_SUPAGE);		// Initialize the spare buffer

	OEMWriteDebugString(L" [NBL2] ++ShadowEboot()\r\n");

	pLowFuncTbl = FIL_GetFuncTbl();

	pBuffer = (UINT8 *)EBOOT_VIRTUAL_BASEADDR;
	dwStartBlock = VFL_EBOOT_START_BLOCK;
	dwNumBlock = VFL_EBOOT_BLOCK_SIZE;

	dwBlock = dwStartBlock;

	while(dwNumBlock > 0)
	{
		if (dwBlock == (VFL_EBOOT_START_BLOCK+VFL_EBOOT_AREA_SIZE))
		{
			OEMWriteDebugString(L" [NBL2:ERR] ShadowEboot() Critical Error\r\n");
			OEMWriteDebugString(L" [NBL2:ERR] Too many Bad Block\r\n");
			while(1);
		}

		IS_CHECK_SPARE_ECC = FALSE32;
		pLowFuncTbl->Read(0, dwBlock*PAGES_PER_BLOCK+PAGES_PER_BLOCK-1, 0x0, enuBOTH_PLANE_BITMAP, NULL, pSBuf, TRUE32, FALSE32);
		IS_CHECK_SPARE_ECC = TRUE32;

		if (TWO_PLANE_PROGRAM == TRUE32)
		{
			if (pSBuf[0] == 0xff && pSBuf[BYTES_PER_SPARE_PAGE] == 0xff)
				bIsBadBlock = TRUE32;
		}
		else
		{
			if (pSBuf[0] == 0xff)
				bIsBadBlock = TRUE32;
		}

		if (bIsBadBlock)
		{
			for (dwPageOffset=0; dwPageOffset<PAGES_PER_BLOCK; dwPageOffset++)
			{
				nRet = pLowFuncTbl->Read(0, dwBlock*PAGES_PER_BLOCK+dwPageOffset, FULL_SECTOR_BITMAP_PAGE, enuBOTH_PLANE_BITMAP, pBuffer+BYTES_PER_MAIN_SUPAGE*dwPageOffset, NULL, FALSE32, FALSE32);
				if (nRet == FIL_U_ECC_ERROR)
				{
					OEMWriteDebugString(L" [NBL2:ERR] FIL_Read() : Uncorrectable ECC Error\r\n");
					while(1);
				}
				else if (nRet == FIL_CRITICAL_ERROR)
				{
					OEMWriteDebugString(L" [NBL2:ERR] VFL_Read() : Critical Error\r\n");
					while(1);
				}
			}

			dwBlock++;
			dwNumBlock--;
			pBuffer += BYTES_PER_MAIN_SUPAGE*PAGES_PER_BLOCK;
			continue;
		}
		else
		{
			OEMWriteDebugString(L" [NBL2:ERR] Bad Block Skipped\r\n");
			dwBlock++;
			continue;
		}
	}

	OEMWriteDebugString(L" [NBL2] --ShadowEboot()\r\n");
}

 //[david.modify] 2008-04-28 13:58
//====================================================================
static char *hex2char(unsigned int val)
{
    static char str[2];

    str[1]='\0';

    if(val<=9)
        str[0]='0'+val;
    else
        str[0]=('a'+val-10);

    return str;
}
static  void Uart_SendString(char *pt)
{
    while(*pt)
        OEMWriteDebugByte(*pt++);
}
 
static void Uart_SendBYTE(BYTE d, BOOL cr)
{
	//Uart_SendString("0x");
	Uart_SendString(hex2char((d & 0x000000f0) >> 4));
	Uart_SendString(hex2char((d & 0x0000000f) >> 0));
	Uart_SendString(" ");
	if (cr)
		Uart_SendString("\n");
}
static void Uart_SendDWORD(DWORD d, BOOL cr)
{
    Uart_SendString("0x");
    Uart_SendString(hex2char((d & 0xf0000000) >> 28));
    Uart_SendString(hex2char((d & 0x0f000000) >> 24));
    Uart_SendString(hex2char((d & 0x00f00000) >> 20));
    Uart_SendString(hex2char((d & 0x000f0000) >> 16));
    Uart_SendString(hex2char((d & 0x0000f000) >> 12));
    Uart_SendString(hex2char((d & 0x00000f00) >> 8));
    Uart_SendString(hex2char((d & 0x000000f0) >> 4));
    Uart_SendString(hex2char((d & 0x0000000f) >> 0));
    if (cr)
        Uart_SendString("\n");
}
 

void Uart_Print(unsigned char *pAddr, unsigned int nBytes)
 {
 	int i=0;
	OEMWriteDebugString(L"\r\n");	
	Uart_SendString("pAddr=");
	Uart_SendDWORD((DWORD)pAddr, 0);
	Uart_SendString(" nBytes=");
	Uart_SendDWORD((DWORD)nBytes, 0);	
	OEMWriteDebugString(L"\r\n");	
	for(i=0;i<nBytes;i++) {
		if(i%16==0) {
			OEMWriteDebugString(L"\r\n");
			Uart_SendDWORD(pAddr+i, 0);
			Uart_SendString(L": ");						
		}
		
		Uart_SendBYTE(pAddr[i], 0);
	}
 }

 //[david.modify] 2008-04-28 15:52
#define RETAILMSG(cond, printf_exp)	((cond)?(NKDbgPrintfW printf_exp),1:0)
#define NAND_ERR(x)	RETAILMSG(TRUE, x)

void main(void)
{
	INT32 nRet;

	// Set up the copy section data.
	if (!SetupCopySection(pTOC))
	{
		while(1);
	}

	// Clear LEDs.
	// 0 µãÁÁ, 1-¹Ø±Õ
	OEMWriteDebugLED(0, 0x0);

	OEMInitDebugSerial();
	OEMWriteDebugString(L" [NBL2] main() Starts !\r\n");
	OEMWriteDebugString(L" [NBL2] Serial Initialized...\r\n");



	nRet = FIL_Init();
	if (nRet != FIL_SUCCESS)
	{
		OEMWriteDebugString(L" [NBL2:ERR] FIL_Init() : Failed\r\n");
		while(1);
	}
	OEMWriteDebugString(L" [NBL2] FIL_Init() : Passed\r\n");

	ShadowEboot();

	OEMWriteDebugString(L" [NBL2] Launch Eboot...\r\n");

	Uart_Print(EBOOT_VIRTUAL_BASEADDR, 0x200);
#if 0
	OEMWriteDebugString(L"[NBL2]enter into while\r\n");	 		
	while(1){};
#endif	
	OEMLaunchImage(EBOOT_VIRTUAL_BASEADDR);
}


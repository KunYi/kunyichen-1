// testapp.cpp : Defines the entry point for the application.
//

#pragma optimize("", off)

#include <windows.h>
#include <nkintr.h>
#include <stdio.h>
#include <s3c2450.h>
#include "VFLBuffer.h"
#include "WMRTypes.h"
#include "VFL.h"
#include "HALWrapper.h"

#define DUMPOS
#define DUMPFS

#define K9G8G08
//#define K9LAG08
//#define K9GAG08
//#define K9HAG08

#ifdef K9G8G08
	#define PAGESIZE2K				1
	#define SPECIAL_AREA_START_L 	10
	#define SPECIAL_AREA_SIZE_L 	100
	#define FTL_AREA_START_L		216
	#define FTL_AREA_SIZE_L			1832
	#define PAGES_PER_SUBLK_L 		128
	
	#define SECTORS_PER_PAGE_L 		4
	#define BYTES_PER_SECTOR_L		512
	#define BYTES_PER_SPARE_L		16
	#define BYTES_PER_SPARE_PAGE_L	128
	
	#define USE2PLANE_L
#endif

#ifdef K9LAG08
	#define PAGESIZE2K				1
	#define SPECIAL_AREA_START_L 	10
	#define SPECIAL_AREA_SIZE_L 	50
	#define FTL_AREA_START_L		166
	#define FTL_AREA_SIZE_L			1882
	#define PAGES_PER_SUBLK_L 		256
	
	#define SECTORS_PER_PAGE_L 		4
	#define BYTES_PER_SECTOR_L		512
	#define BYTES_PER_SPARE_L		16
	#define BYTES_PER_SPARE_PAGE_L	128
	
	#define USE2PLANE_L
#endif

#ifdef K9HAG08
	#define PAGESIZE2K				1
	#define SPECIAL_AREA_START_L 	10
	#define SPECIAL_AREA_SIZE_L 	25
	#define FTL_AREA_START_L		90
	#define FTL_AREA_SIZE_L			934
	#define PAGES_PER_SUBLK_L 		512
	
	#define SECTORS_PER_PAGE_L 		4
	#define BYTES_PER_SECTOR_L		512
	#define BYTES_PER_SPARE_L		16
	#define BYTES_PER_SPARE_PAGE_L	128
	
	#define USE2PLANE_L
#endif

#ifdef K9GAG08
	#define PAGESIZE4K				1
	#define SPECIAL_AREA_START_L 	10
	#define SPECIAL_AREA_SIZE_L 	100
	#define FTL_AREA_START_L		216
	#define FTL_AREA_SIZE_L			1832
	#define PAGES_PER_SUBLK_L 		128
	
	#define SECTORS_PER_PAGE_L 		8
	#define BYTES_PER_SECTOR_L		512
	#define BYTES_PER_SPARE_L		16
	#define BYTES_PER_SPARE_PAGE_L	128
	
	#define USE2PLANE_L
#endif

void ReadPage(DWORD nVpn, unsigned char * pData, unsigned char * pSpare)
{
	Buffer 		pBuf;
	VFLPacket   stPacket;
	UINT32      nResult;

#ifdef PAGESIZE4K
	#ifdef USE2PLANE_L    
		pBuf.nBitmap = 0xFFFF;
	#else
		pBuf.nBitmap = 0xFF;
	#endif
#else
	#ifdef USE2PLANE_L    
		pBuf.nBitmap = 0xFF;
	#else
		pBuf.nBitmap = 0xF;
	#endif
#endif

	pBuf.eStatus = BUF_AUX;
	pBuf.nBank = 0;

	// Read Main Data Area
	pBuf.pData = pData;
	pBuf.pSpare = NULL;
	
	do {
	    /* VFL_Read */
	    stPacket.nCtrlCode  = PM_HAL_VFL_READ;
	    stPacket.nVbn       = 0;            // Not used
	    stPacket.nVpn       = nVpn;
	    stPacket.pBuf       = &pBuf;
	    stPacket.nSrcVpn    = 0;
	    stPacket.nDesVpn    = 0;
	    stPacket.bCleanCheck= FALSE32;
	
	    KernelIoControl(IOCTL_POCKETSTOREII_CMD,  /* IO Control Code */
	                    &stPacket,                /* Input buffer (Additional Control Code) */
	                    sizeof(VFLPacket),        /* Size of Input buffer */
	                    NULL,                     /* Output buffer */
	                    0,                        /* Size of Output buffer */
	                    &nResult);                /* Error Return */
	                    
	    if (nResult != VFL_SUCCESS)
	    {
	        RETAILMSG(1, ((TEXT("[VFLP:ERR]  VFL_Read() failure. ERR Code=%x\r\n"), nResult)));
	        break;
	    }

	} while(0);

	// Read Spare Area
	pBuf.pData = NULL;
	pBuf.pSpare = pSpare;

	do {
	    /* VFL_Read */
	    stPacket.nCtrlCode  = PM_HAL_VFL_READ;
	    stPacket.nVbn       = 0;            // Not used
	    stPacket.nVpn       = nVpn;
	    stPacket.pBuf       = &pBuf;
	    stPacket.nSrcVpn    = 0;
	    stPacket.nDesVpn    = 0;
	    stPacket.bCleanCheck= FALSE32;
	
	    KernelIoControl(IOCTL_POCKETSTOREII_CMD,  /* IO Control Code */
	                    &stPacket,                /* Input buffer (Additional Control Code) */
	                    sizeof(VFLPacket),        /* Size of Input buffer */
	                    NULL,                     /* Output buffer */
	                    0,                        /* Size of Output buffer */
	                    &nResult);                /* Error Return */
	                    
	    if (nResult != VFL_SUCCESS)
	    {
	        RETAILMSG(1, ((TEXT("[VFLP:ERR]  VFL_Read() failure. ERR Code=%x\r\n"), nResult)));
	        break;
	    }

	} while(0);
}

BOOL32 CheckAllFF( unsigned char * pMBuf, DWORD size )
{
	DWORD i;

	for ( i = 0; i < size; i++ )
	{
		if ( *(pMBuf + i) == 0xFF )
		{
			continue;
		}
		else
		{
			return FALSE32;
		}
	}
	return TRUE32;
}

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	register int nVpn;

	FILE *pFileOutPtr = NULL;

	int nFileLength;

	char *cBuffer = NULL;

	#ifdef USE2PLANE_L    
    UINT8	pData[(SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L)*2];
    UINT8	pSpare[(SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L)*2];
	#else
    UINT8	pData[(SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L)];
    UINT8	pSpare[(SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L)];
	#endif

	DWORD	dwOSStartPage;
	DWORD	dwOSEndPage;
	DWORD	dwFTLStartPage;
	DWORD	dwFTLEndPage;

#ifdef DUMPOS
	// Extract OS Image routine...

	RETAILMSG(1,(TEXT("Dump OS Image\r\n")));

	pFileOutPtr = fopen("Temp/OSIMG.IMG", "wb");
	if (!pFileOutPtr)
	{
		RETAILMSG(1,(TEXT("Storage Card/OSIMG.IMG file is not opened.!!!\r\n")));
		goto Fail;
	}

	fseek(pFileOutPtr, 0, SEEK_SET);

	dwOSStartPage = SPECIAL_AREA_START_L * PAGES_PER_SUBLK_L;
	dwOSEndPage = (SPECIAL_AREA_START_L+SPECIAL_AREA_SIZE_L) * PAGES_PER_SUBLK_L - 1;

	RETAILMSG(1,(TEXT("OS Area is from %d to %d\r\n"), dwOSStartPage, dwOSEndPage));
	RETAILMSG(1,(TEXT("Scanning...\r\n")));
	// Scan All FF area
	for ( nVpn = dwOSEndPage; nVpn > dwOSStartPage; nVpn-- )
	{
		memset(pData, 0xFF, sizeof(pData));
		memset(pSpare, 0xFF, sizeof(pSpare));
		ReadPage( nVpn, pData, pSpare );
		if ( CheckAllFF(pData, sizeof(pData)) == TRUE32 && CheckAllFF(pSpare, sizeof(pSpare)) ) // TRUE32 means All FF
		{
			continue;
		}
		else
		{
			break;
		}
	}
	dwOSEndPage = nVpn + 1;

	RETAILMSG(1,(TEXT("Read Sector from %d to %d\r\n"), dwOSStartPage, dwOSEndPage));

	for( nVpn=dwOSStartPage; nVpn<dwOSEndPage; nVpn++ )
	{
//		RETAILMSG(1,(TEXT("Read Page %d\r\n"), nVpn));
		RETAILMSG(1,(TEXT(" %02d Percent Completed"), ((nVpn - dwOSStartPage)*100)/(dwOSEndPage - dwOSStartPage)));
		
		memset(pData, 0xFF, sizeof(pData));
		memset(pSpare, 0xFF, sizeof(pSpare));

		ReadPage( nVpn, pData, pSpare );
		
#ifdef USE2PLANE_L    
		// Write First plane
//		RETAILMSG(1,(TEXT("Write First plane : page number (%d) mem (0x%x)\r\n"), nVpn*2, (nVpn-dwOSStartPage)*(2048+64)));
		fwrite(pData, sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L), pFileOutPtr);
		fwrite(pSpare, sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L), pFileOutPtr);

		// Write Second plane
//		RETAILMSG(1,(TEXT("Write Second plane : page number (%d) mem (0x%x)\r\n"), nVpn*2+128, (nVpn-dwOSStartPage+128)*(2048+64)));
		fwrite(pData+(SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L), sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L), pFileOutPtr);
		fwrite(pSpare+(SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L), sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L), pFileOutPtr);
#else
		// Write First plane
//		RETAILMSG(1,(TEXT("Write First plane : page number (%d) mem (0x%x)\r\n"), nVpn*2, (nVpn-dwOSStartPage)*(2048+64)));
		fwrite(pData, sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L), pFileOutPtr);
		fwrite(pSpare, sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L), pFileOutPtr);
#endif
		RETAILMSG(1,(TEXT("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b")));
	}

	if (pFileOutPtr) fclose(pFileOutPtr);
	
	RETAILMSG(1,(TEXT("Dump OS Image Finished\r\n")));

#endif

#ifdef DUMPFS
	// Extract FileSystem Image routine...

	RETAILMSG(1,(TEXT("Dump FTL Image\r\n")));

	pFileOutPtr = fopen("Temp/FTLIMG.IMG", "wb");
	if (!pFileOutPtr)
	{
		RETAILMSG(1,(TEXT("Storage Card/FTLIMG.IMG file is not opened.!!!\r\n")));
		goto Fail;
	}
	
	fseek(pFileOutPtr, 0, SEEK_SET);

	dwFTLStartPage = FTL_AREA_START_L * PAGES_PER_SUBLK_L;
	dwFTLEndPage = (FTL_AREA_START_L+FTL_AREA_SIZE_L) * PAGES_PER_SUBLK_L - 1;

	RETAILMSG(1,(TEXT("FTL Area is from %d to %d\r\n"), dwFTLStartPage, dwFTLEndPage));
	RETAILMSG(1,(TEXT("Scanning...\r\n")));
	// Scan All FF area
	for ( nVpn = dwFTLEndPage; nVpn > dwFTLStartPage; nVpn-- )
	{
		memset(pData, 0xFF, sizeof(pData));
		memset(pSpare, 0xFF, sizeof(pSpare));
		ReadPage( nVpn, pData, pSpare );
		if ( CheckAllFF(pData, sizeof(pData)) == TRUE32 && CheckAllFF(pSpare, sizeof(pSpare)) ) // TRUE32 means All FF
		{
			continue;
		}
		else
		{
			break;
		}
	}
	dwFTLEndPage = nVpn + 1;

	RETAILMSG(1,(TEXT("Read Sector from %d to %d\r\n"), dwFTLStartPage, dwFTLEndPage));
	
	for( nVpn=dwFTLStartPage; nVpn<dwFTLEndPage; nVpn++ )
	{
//		RETAILMSG(1,(TEXT("Read Page %d\r\n"), nVpn));
		RETAILMSG(1,(TEXT(" %02d Percent Completed"), ((nVpn - dwFTLStartPage)*100)/(dwFTLEndPage - dwFTLStartPage)));
		
		memset(pData, 0xFF, sizeof(pData));
		memset(pSpare, 0xFF, sizeof(pSpare));

		ReadPage( nVpn, pData, pSpare );

#ifdef USE2PLANE_L    
		// Write First plane
//		RETAILMSG(1,(TEXT("Write First plane : page number (%d) mem (0x%x)\r\n"), nVpn*2, (nVpn-dwOSStartPage)*(2048+64)));
		fwrite(pData, sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L), pFileOutPtr);
		fwrite(pSpare, sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L), pFileOutPtr);

		// Write Second plane
//		RETAILMSG(1,(TEXT("Write Second plane : page number (%d) mem (0x%x)\r\n"), nVpn*2+128, (nVpn-dwOSStartPage+128)*(2048+64)));
		fwrite(pData+(SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L), sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L), pFileOutPtr);
		fwrite(pSpare+(SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L), sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L), pFileOutPtr);
#else
		// Write First plane
//		RETAILMSG(1,(TEXT("Write First plane : page number (%d) mem (0x%x)\r\n"), nVpn*2, (nVpn-dwOSStartPage)*(2048+64)));
		fwrite(pData, sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SECTOR_L), pFileOutPtr);
		fwrite(pSpare, sizeof(char), (SECTORS_PER_PAGE_L*BYTES_PER_SPARE_L), pFileOutPtr);
#endif
		RETAILMSG(1,(TEXT("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b")));
	}

	if (pFileOutPtr) fclose(pFileOutPtr);
	
	RETAILMSG(1,(TEXT("Dump FTL Image Finished\r\n")));
#endif

	return 0;


Fail:
	RETAILMSG(1,(TEXT("WMR_RW_test is failed.!!\r\n")));
	if (pFileOutPtr) fclose(pFileOutPtr);
	return 0;

}



#pragma optimize("", on)

#include <bsp.h>
#include <ethdbg.h>
#include <fmd.h>
#include "loader.h"
#include <eboot_inc_david.h>
extern UINT8 g_u8Temp;


#define LST_BUFFER_SIZE 8096

#define MAX_FILE_NUM	32
#define MAX_FILE_NAME   32

#define DBG_UPGRADE_SDIMG   1

//#define DEF_DOWNLOAD_ADDR           0xA1000000
 //[david.modify] 2008-07-30 21:11
  //[david.modify] 2008-08-16 12:02
  // FROM BOOT.BIB
//  	RAM      83D00000  00200000  RAM 
//#define DEF_DOWNLOAD_ADDR           (0xA3D00000-30*1024*1024)

#define NB0IMAGE	0
#define BINIMAGE	1
#define LSTIMAGE	2
#define UBIIMAGE	3
#define DIOIMAGE	4

#define MAX_PATH        260

extern int HSMMCInit(BYTE ChSelect);
void ChooseImageFromSD();

volatile UINT32 		readPtIndex;
volatile UINT8			*g_pDownPt;

extern DWORD        g_ImageType;

extern BOOL DownloadImage (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
BOOL Parsing ( const char *sFileName, UINT32 dwImageType, BYTE *Buffer );
void BigToSmall(LPBYTE pAlphabet);
DWORD ReadFrommoviNAND(UINT32 dwStartSector, UINT32 dwSector, UINT32 dwAddr);

char strLSTFileBuffer[LST_BUFFER_SIZE];

void uSleep(UINT32 cnt)
{
	volatile UINT32 i, j;
	for ( i = 0; i < cnt; i++ )
	{
		for ( j=0; i < 10; j++ ){};
	}
}

BOOL mmc_read ( UINT32 nDev, UINT32 dwStartSector, UINT32 dwAddr, UINT32 dwSector )
{
	ReadFrommoviNAND(dwStartSector, dwSector, dwAddr);
	return TRUE;
}

#if 0
BOOL IsCardInserted(void)
{
    BOOL        result = FALSE;
    int         count = 5;
	volatile 	S3C2450_IOPORT_REG *s24500IOP = (volatile S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
    s24500IOP->GPFCON &= ~(0x3<<2);   // as input

 //[david.modify] 2008-07-31 17:33
 	
	

    return TRUE;
}
#else
extern BOOL IsCardInserted();
#endif

#pragma optimize ("",off)
BOOL Parsing ( const char *sFileName, UINT32 dwImageType, BYTE *Buffer )
{
	ULONG nFileNumber;
    ULONG i, j;
	BYTE * ptxBuf;
	unsigned int nCheckSum = 0;
    ULONG fileSize;
	char szBinFileName[MAX_FILE_NUM][MAX_FILE_NAME];

	if ( dwImageType == LSTIMAGE )
	{
#if 1
        for ( i = 0 ; i < LST_BUFFER_SIZE ; i++)
            strLSTFileBuffer[i] = (char)(NULL);


        if ( FATGetFileSize(sFileName,NULL) > LST_BUFFER_SIZE )
        {
            EdbgOutputDebugString("%s file contain more thatn %d bytes\n",sFileName, LST_BUFFER_SIZE);
            return FALSE;
        }

        if ( !FATReadFile(sFileName,strLSTFileBuffer))
        {
            EdbgOutputDebugString("%s file Read Error\n",sFileName);
            return FALSE;
        }

		// Read chain.lst file
		for ( nFileNumber = 0,j=0; nFileNumber < MAX_FILE_NUM; nFileNumber++ )
		{
            for ( i = 0 ; i < MAX_FILE_NAME; )
            {
                if ( strLSTFileBuffer[j] == '\0' ) 
                {
                    szBinFileName[nFileNumber][i] = '\0';
                    j = LST_BUFFER_SIZE;
                    break;
                }
                else if ( (strLSTFileBuffer[j] == '\r') || (strLSTFileBuffer[j] == '\n') || (strLSTFileBuffer[j] == '\t') || (strLSTFileBuffer[j] == '+'))
                {
                    j++;
                    if ( i == 0 )
                        continue;
                    else
                    {
                        szBinFileName[nFileNumber][i] = '\0';                        
                        break;
                    }
                }
    			szBinFileName[nFileNumber][i] = strLSTFileBuffer[j];
                j++;
                i++;
            }
            
            if ( j == LST_BUFFER_SIZE )
                break;
		}

		*((BYTE *)Buffer+0)=0x4E;
		*((BYTE *)Buffer+1)=0x30;
		*((BYTE *)Buffer+2)=0x30;
		*((BYTE *)Buffer+3)=0x30;
		*((BYTE *)Buffer+4)=0x46;
		*((BYTE *)Buffer+5)=0x46;
		*((BYTE *)Buffer+6)=0xa;

		ptxBuf = Buffer  + 7 /* X000FF\n */
						+ 4 /* check sum */
						+ 4 /* num Regions */
						+ nFileNumber*(8+260); /* start address + length */
        
        
		for ( i = 0; i < nFileNumber; i++ )
		{
            BigToSmall(szBinFileName[i]);
            EdbgOutputDebugString("%dth File Read %s AT 0x%x\n",i, szBinFileName[i],ptxBuf);   
            
            fileSize = FATGetFileSize (szBinFileName[i],NULL);
                
            if ( !fileSize )
            {
                EdbgOutputDebugString("%s file Get size Error\n",szBinFileName[i]);
                return FALSE;
            }
            
            if ( !FATReadFile(szBinFileName[i],ptxBuf))
            {
                EdbgOutputDebugString("%s file Read Error\n",szBinFileName[i]);
                return FALSE;
            }

			for ( j = 0; j < 8 + 260; j++ )
			{
				if ( j < 8 )
				{
					*((BYTE *)Buffer+15+(i*(8+260)+j))=(BYTE)(ptxBuf[7+j]);
					nCheckSum += (BYTE)(ptxBuf[7+j]);
				}
				else if ( j >= 8 && j < (8+strlen(szBinFileName[i])) )
				{
					*((BYTE *)Buffer+15+(i*(8+260)+j))=(BYTE)(szBinFileName[i][j-8]);
					nCheckSum += (BYTE)(szBinFileName[i][j-8]);
				}
				else
				{
					*((BYTE *)Buffer+15+(i*(8+260)+j))=0;
				}
			}

			ptxBuf += fileSize;
		}

        
		ptxBuf = Buffer + 7;

        memcpy(ptxBuf, (PVOID)(&nCheckSum), 4);
        memcpy((PVOID)(ptxBuf+4), (PVOID)(&nFileNumber), 4);
        
#endif
	}
	else if ( dwImageType == UBIIMAGE || dwImageType == BINIMAGE )
	{
		FATReadFile(sFileName, Buffer);
		g_pDownPt += FATFileSize ();
	}
	else if ( dwImageType == NB0IMAGE || dwImageType == DIOIMAGE )
	{
		nFileNumber = 1;

		memset((void *)Buffer, 0, 7+4+4+4+4+MAX_PATH);

		ptxBuf = Buffer;
		
		*(ptxBuf++)=0x4E;
		*(ptxBuf++)=0x30;
		*(ptxBuf++)=0x30;
		*(ptxBuf++)=0x30;
		*(ptxBuf++)=0x46;
		*(ptxBuf++)=0x46;
		*(ptxBuf++)=0xa;

		// Read nb0 file
		if (!FATReadFile(sFileName, (BYTE *)(Buffer+7+4+4+4+4+MAX_PATH)))
		{
            RETAILMSG(1,(TEXT("#### File READ ERROR\r\n")));
            while(1);
		}

		ptxBuf = Buffer + 7 + 4 + 4;

		*(ptxBuf+0) = 0;				//nb0 start address == 0
		*(ptxBuf+1) = 0;				//nb0 start address == 0
		*(ptxBuf+2) = 0;				//nb0 start address == 0
		*(ptxBuf+3) = 0;				//nb0 start address == 0
		fileSize = FATFileSize();		//nb0 filesize
		*(ptxBuf+4) = (BYTE)((fileSize >> 0) & 0xff);
		*(ptxBuf+5) = (BYTE)((fileSize >> 8) & 0xff);
		*(ptxBuf+6) = (BYTE)((fileSize >> 16) & 0xff);
		*(ptxBuf+7) = (BYTE)((fileSize >> 24) & 0xff);

		strcpy((char *)(ptxBuf+8), sFileName);

		nCheckSum = 0;
		for ( i = 0; i < 4+4+MAX_PATH; i++ )
		{
			nCheckSum += (unsigned char)(*(ptxBuf+i));
		}

		ptxBuf = Buffer+7;

		*(ptxBuf+0) = (BYTE)((nCheckSum >> 0) & 0xff);
		*(ptxBuf+1) = (BYTE)((nCheckSum >> 8) & 0xff);
		*(ptxBuf+2) = (BYTE)((nCheckSum >> 16) & 0xff);
		*(ptxBuf+3) = (BYTE)((nCheckSum >> 24) & 0xff);
		
		*(ptxBuf+4) = (BYTE)((nFileNumber >> 0) & 0xff);
		*(ptxBuf+5) = (BYTE)((nFileNumber >> 8) & 0xff);
		*(ptxBuf+6) = (BYTE)((nFileNumber >> 16) & 0xff);
		*(ptxBuf+7) = (BYTE)((nFileNumber >> 24) & 0xff);

		g_pDownPt += (7+4+4+4+4+MAX_PATH);
		g_pDownPt += FATFileSize();
	}
    return TRUE;
}
#pragma optimize ("",on)

void ChooseSDMMCChannel()
{
    BYTE ChSelect = 0;
    
    EdbgOutputDebugString ( "\r\nChoose a Number of SDMMC Slot Channel :\r\n\r\n" );
    EdbgOutputDebugString ( "0) Channel 0\r\n" );
    EdbgOutputDebugString ( "1) Channel 1\r\n" );
    EdbgOutputDebugString ( "\r\nEnter your selection: ");

    while (!(((ChSelect >= '0') && (ChSelect <= '1'))))
    {
        ChSelect = OEMReadDebugByte();
    }
    EdbgOutputDebugString ( "%c\r\n", ChSelect);

    HSMMCInit(ChSelect);
    ChooseImageFromSD();
}

extern BOOL			g_bDownloadImage ;
extern DWORD			g_ImageType;
extern PBOOT_CFG		g_pBootCfg;
void ChooseImageFromSD()
{
    BYTE KeySelect = 0;
    int cnt;
    DWORD dwImageStart = 0, dwImageLength = 0, dwLaunchAddr = 0;
	UINT32 u32Temp=0;
   
	EdbgOutputDebugString ( "\r\nChoose Download Image:\r\n\r\n" );
	EdbgOutputDebugString ( "0) block0.nb0\r\n" );
	EdbgOutputDebugString ( "1) eboot.bin\r\n" );
	EdbgOutputDebugString ( "2) nk.bin\r\n" );
	EdbgOutputDebugString ( "3) chain.lst\r\n" );    
	EdbgOutputDebugString ( "4) Everything (block0.nb0 + eboot.nb0 + nk.bin)\r\n" );
	EdbgOutputDebugString ( "5) xip.bin\r\n" );		
	//EdbgOutputDebugString ( "5) Everything (block0img.nb0 + eboot.bin + chain.lst)\r\n" );    
    EdbgOutputDebugString ( "\r\nEnter your selection: ");
	
    while (!(((KeySelect >= '0') && (KeySelect <= '9'))))
    {
        KeySelect = OEMReadDebugByte();
    }

    EdbgOutputDebugString ( "%c\r\n", KeySelect);

    g_pDownPt = (UINT8 *)DEF_DOWNLOAD_ADDR;
    readPtIndex = (UINT32)DEF_DOWNLOAD_ADDR;
	DPNOK(g_pDownPt);
	DPNOK(readPtIndex);	

    if (IsCardInserted())
    {
		FATInit();

	    switch(KeySelect)
	    {
	    case '0':
			Parsing("block0.nb0", NB0IMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
	              EPRINT ( "1-PRINT 2-no\r\n");		  
			g_u8Temp = ReadSerialByte();	

			if('1'==g_u8Temp){
//				u32Temp=OALPAtoVA(g_pDownPt);
//				DPNOK(u32Temp);
				DPNOK(g_pDownPt);				
				PrintMsg(DEF_DOWNLOAD_ADDR, 0X100, sizeof(UINT32));
			}		
	        break;
	    case '1':
			DPSTR("+Parsing Eboot");
			Parsing("Eboot.bin", BINIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
			DPSTR("-Parsing Eboot");
			
	              EPRINT ( "1-PRINT 2-no\r\n");		  
			g_u8Temp = ReadSerialByte();	

			if('1'==g_u8Temp){
//				u32Temp=OALPAtoVA(g_pDownPt);
//				DPNOK(u32Temp);
				DPNOK(g_pDownPt);				
				PrintMsg(DEF_DOWNLOAD_ADDR, 0X100, sizeof(UINT32));
			}
			
			
	        break;
	    case '2':
    		Parsing("nk.bin", BINIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
	        break;
	    case '5':
    		Parsing("xip.bin", BINIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
	              EPRINT ( "1-PRINT 2-no\r\n");		  
			g_u8Temp = ReadSerialByte();	

			if('1'==g_u8Temp){
//				u32Temp=OALPAtoVA(g_pDownPt);
//				DPNOK(u32Temp);
				DPNOK(g_pDownPt);				
				PrintMsg(DEF_DOWNLOAD_ADDR, 0X100, sizeof(UINT32));

				u32Temp = FATGetFileSize("block0.nb0",NULL);
				DPNOK(u32Temp);
				u32Temp = FATGetFileSize("eboot.bin",NULL);
				DPNOK(u32Temp);
				u32Temp = FATGetFileSize("nk.bin",NULL);
				DPNOK(u32Temp);
				u32Temp = FATGetFileSize("xip.bin",NULL);
				DPNOK(u32Temp);
				u32Temp = FATGetFileSize("test.dat",NULL);					
				DPNOK(u32Temp);				
				
			}

						dwImageStart = 0x80200000;
						dwImageLength = FATFileSize();
						dwLaunchAddr = 0x80200000;
			memcpy(dwImageStart, DEF_DOWNLOAD_ADDR, dwImageLength);						
						DPNOK(dwImageStart);
						DPNOK(dwImageLength);						
						DPNOK(dwLaunchAddr);												
	
	        break;			
	    case '3':
    		Parsing("chain.lst", LSTIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
	        break;
	    case '4':
			{
				for ( cnt = 0; cnt < 3; cnt++ )
				{
					g_pDownPt = (BYTE *)DEF_DOWNLOAD_ADDR;
                    readPtIndex = (UINT32)DEF_DOWNLOAD_ADDR;
					switch (cnt)
					{
					case 0 :
						Parsing("Block0.nb0", BINIMAGE, (BYTE *)(FILE_CACHE_TMPBUF1));
						dwImageStart = 0;
						dwImageLength = FATFileSize();
						dwLaunchAddr = 0;
						g_ImageType = IMAGE_TYPE_STEPLDR;
						break;
					case 1 :
						Parsing("Eboot.nb0", BINIMAGE, (BYTE *)(FILE_CACHE_TMPBUF1));
						dwImageStart = 0x80038000;
						dwImageLength = FATFileSize();
						dwLaunchAddr = 0x80038000;
						g_ImageType = IMAGE_TYPE_LOADER;
						break;
					case 2 :
						Parsing("nk.bin", BINIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
						OALMSG(1, (TEXT("-Download OS Image\r\n")));
						return;
				        break;
					}

					if (!WriteRawImageToBootMedia(dwImageStart, dwImageLength, dwLaunchAddr))
					{
						OALMSG(OAL_ERROR, (TEXT("ERROR: OEMLaunch: Failed to store image to Smart Media.\r\n")));
						while(1);
					}
				}
			}
    	EdbgOutputDebugString ( "Read Finished \r\n");
    	
    	EdbgOutputDebugString ( "g_pDownPt = 0x%x \r\n", g_pDownPt);


 //[david.modify] 2008-07-31 12:13
 	
 	

		
	    }
    }
}

 //[david.modify] 2008-07-31 17:01

extern void Format_VFL();
 extern void Format_FTL();
  extern void Format_ALL();
 
 // 自动更新XIP.BIN
#define OSIMG_NAME "XIP.BIN"
#define CHAINLST_NAME "CHAIN.LST"
#define BOOT1_NAME "BLOCK0.NB0"
#define BOOT2_NAME "EBOOT.NB0"
#define FORTMAT_A_NAME "FMT_ALL.NB0"		//格式化ALL,按A效果
#define FORTMAT_B_NAME "FMT_VFL.NB0"	//格式化VFL,按B效果
#define FORTMAT_C_NAME "FMT_FTL.NB0"	//格式化FTL,按C效果

#define LOGOFILE_NAME "LOGO.565"





int SDMMC_Update_BOOTLOADER(int nSn)
{
	DWORD dwImageStart, dwImageLength, dwLaunchAddr;
	BOOL bRet = 0;
	UINT8 u8Temp;

	DPNOK(nSn);
	if(0==nSn) {
		Parsing(BOOT1_NAME, BINIMAGE, (BYTE *)(FILE_CACHE_TMPBUF1));
		dwImageStart = 0;
		dwImageLength = FATFileSize();
		dwLaunchAddr = 0;
		g_ImageType = IMAGE_TYPE_STEPLDR;						
	}else 
	if (1==nSn){
		Parsing(BOOT2_NAME, BINIMAGE, (BYTE *)(FILE_CACHE_TMPBUF1));
		dwImageStart = 0x80038000;
		dwImageLength = FATFileSize();
		dwLaunchAddr = 0x80038000;
		g_ImageType = IMAGE_TYPE_LOADER;
	}

#if 0	
       EPRINT ( "1-WriteRawImageToBootMedia, 0-cancel\r\n ");				
	u8Temp = ReadSerialByte();			
#else
	u8Temp='1';
#endif
	if('1'==u8Temp) {
		bRet = WriteRawImageToBootMedia(dwImageStart, dwImageLength, dwLaunchAddr);
		DPNOK(bRet);	
	}	

	return TRUE;
}



int SDMMC_Update_LOGO(int nSn)
{
	DWORD dwImageStart, dwImageLength, dwLaunchAddr;
	BOOL bRet = 0;
	int nRet=0;
	UINT8 u8Temp;


 //[david.modify] 2008-09-27 12:08
 #if 0
 		dwImageStart = FILE_CACHE_TMPBUF1;
		dwImageLength =256*1024;
		
 	DPSTR("SET 0");
 	memset(0xA0200000, 0, dwImageLength);
	DPSTR("LOGO_Read");
 	LOGO_Read((UINT8*)dwImageStart, &dwImageLength);
	PrintMsg(0xA0200000, 0x100, sizeof(UINT32));	
 #endif
	

	DPNOK(nSn);
	if(0==nSn) {
		Parsing(LOGOFILE_NAME, BINIMAGE, (BYTE *)(FILE_CACHE_TMPBUF1));
		dwImageStart = FILE_CACHE_TMPBUF1;
		dwImageLength = FATFileSize();
		dwLaunchAddr = 0;
//		g_ImageType = IMAGE_TYPE_LOG0;						
	} 

// debug
	PrintMsg(FILE_CACHE_TMPBUF1, 0x100, sizeof(UINT32));

 #if 0
       DPSTR ( "0-Show565Bmp 1-Cancel\r\n");		  
	g_u8Temp = ReadSerialByte();	
	if('0'==g_u8Temp) {
		nRet = Show565Bmp(IMAGE_FRAMEBUFFER_UA_BASE,  dwImageStart, dwImageLength, 320, 240 );
		DPNOK(nRet);
	}else{
	}
#endif	



	u8Temp='1';
	DPNOK(u8Temp);
	if('1'==u8Temp) {
		DPNOK(u8Temp);
		bRet = LOGO_Write((UINT8*)dwImageStart, &dwImageLength);
		DPNOK(bRet);	
	}
	

 //[david.modify] 2008-09-27 12:08
 #if 0
 	DPSTR("SET 0");
 	memset(FILE_CACHE_TMPBUF1, 0, dwImageLength);
	DPSTR("LOGO_Read");
 	LOGO_Read((UINT8*)dwImageStart, &dwImageLength);
	PrintMsg(FILE_CACHE_TMPBUF1, 0x100, sizeof(UINT32));	
 #if 1
       DPSTR ( "0-Show565Bmp 1-Cancel\r\n");		  
	g_u8Temp = ReadSerialByte();	
	if('0'==g_u8Temp) {
		nRet = Show565Bmp(IMAGE_FRAMEBUFFER_UA_BASE,  dwImageStart, dwImageLength, 320, 240 );
		DPNOK(nRet);
	}else{
	}
#endif		
 #endif
	

	return TRUE;
}


extern BOOL g_bBootMediaExist;
extern DWORD			wNUM_BLOCKS;
int Re_InitNand()
{
    FlashInfo flashInfo;

	DPNOK(0);
#if 1
    // Try to initialize the boot media block driver and BinFS partition.
    //
    ///*
    OALMSG(TRUE, (TEXT("BP_Init\r\n")));
    if ( !BP_Init((LPBYTE)BINFS_RAM_START, BINFS_RAM_LENGTH, NULL, NULL, NULL) )
    {
        OALMSG(OAL_WARN, (TEXT("WARNING: OEMPlatformInit failed to initialize Boot Media.\r\n")));
        g_bBootMediaExist = FALSE;
    }
    else
        g_bBootMediaExist = TRUE;

    // Get flash info
    if (!FMD_GetInfo(&flashInfo)) {
        OALMSG(OAL_ERROR, (L"ERROR: BLFlashDownload: "
            L"FMD_GetInfo call failed\r\n"
        ));
    }
	wNUM_BLOCKS = flashInfo.dwNumBlocks;
	RETAILMSG(1, (TEXT("wNUM_BLOCKS : %d(0x%x) \r\n"), wNUM_BLOCKS, wNUM_BLOCKS));

    // Try to retrieve TOC (and Boot config) from boot media
    //
    if ( !TOC_Read( ) )
    {
        // use default settings
        TOC_Init(DEFAULT_IMAGE_DESCRIPTOR, (IMAGE_TYPE_RAMIMAGE), 0, 0, 0);
    }
#else
        // use default settings
        TOC_Init(DEFAULT_IMAGE_DESCRIPTOR, (IMAGE_TYPE_RAMIMAGE), 0, 0, 0);
	
#endif	

}



int SDMMC_UpdateImage(UINT32 u32channel, UINT32 u32UpdateBit)
{
	int nRet = 0;
	UINT32 u32Temp=0;
#define UPDATE_OS_CHAINLST 1	
#define UPDATE_OS_XIPBIN 2
	UINT32 u32UpdateType=UPDATE_OS_CHAINLST;	//
	UINT32 u32Block0BurnOk=0;
	UINT32 u32EbootBurnOk=0;	
	UINT32 u32OSBurnOk=0;	


 //[david.modify] 2008-09-06 11:45
 #if 0
       EPRINT ( "0-UPDATE_OS_CHAINLST 1-UPDATE_OS_XIPBIN\r\n");		  
	g_u8Temp = ReadSerialByte();	
	if('0'==g_u8Temp) {
		u32UpdateType=UPDATE_OS_CHAINLST;	
	}else{
		u32UpdateType=UPDATE_OS_XIPBIN;	
	}
#else
	g_u8Temp='1';
#endif
	

#if 0
       EPRINT ( "0-Re_InitNand 1-cancel\r\n");		  
	g_u8Temp = ReadSerialByte();	
#else
	g_u8Temp='1';
#endif
	
	if(g_u8Temp=='0') {
	Re_InitNand();
	}


#if 0	
       EPRINT ( "0-ch0 1-ch1\r\n");		  
	g_u8Temp = ReadSerialByte();	
#else
	g_u8Temp='0';
	if(u32channel==0) {
		g_u8Temp='0';
	}else if(u32channel==1){
		g_u8Temp='1';
	}

#endif


	
	if('0'==g_u8Temp) {
		nRet = HSMMCInit('0');
    	}else{
		nRet = HSMMCInit('1');
    	}
	if(nRet!=TRUE) {
		DPSTR("HSMMCInit FAIL");
		return -1;
	}


	g_pDownPt = (UINT8 *)DEF_DOWNLOAD_ADDR;
	readPtIndex = (UINT32)DEF_DOWNLOAD_ADDR;
#if 0	
	nRet=IsCardInserted();
	if(nRet!=TRUE){
		DPSTR("IsCardInserted FAIL");
		return -2;
	}
#endif	


#if 1
		// Toggle download/launch status.
		g_pBootCfg->ConfigFlags = (g_pBootCfg->ConfigFlags | BOOT_TYPE_DIRECT);
	       // Toggle image storage to Smart Media.
		g_pBootCfg->ConfigFlags = (g_pBootCfg->ConfigFlags | TARGET_TYPE_NAND);
		if (!TOC_Write())
		{
			DPSTR("TOC_Write OK!");
		}
		else
		{
			DPSTR("TOC_Write FAIL!");
		}		   
#endif		 


	FATInit();

#if 1

	if(u32UpdateBit&SD_UPDATE_FORTMAT_A) {
		// 如果有FORTMAT_B_NAME文件，执行格式化B
		u32Temp = FATGetFileSize(FORTMAT_A_NAME, NULL);
		DPNOK(u32Temp);
		if(u32Temp>0) {
			Format_ALL();
		}
	}


	if(u32UpdateBit&SD_UPDATE_FORTMAT_B) {
		// 如果有FORTMAT_B_NAME文件，执行格式化B
		u32Temp = FATGetFileSize(FORTMAT_B_NAME, NULL);
		DPNOK(u32Temp);
		if(u32Temp>0) {
			Format_VFL();
		}
	}


	if(u32UpdateBit&SD_UPDATE_FORTMAT_C){		
		// 如果有FORTMAT_C_NAME文件，执行格式化C
		u32Temp = FATGetFileSize(FORTMAT_C_NAME, NULL);
		DPNOK(u32Temp);
		if(u32Temp>0) {
			Format_FTL();
		}
	}


	if(u32UpdateBit&SD_UPDATE_BLOCK0){
		u32Temp = FATGetFileSize(BOOT1_NAME, NULL);
		DPNOK(u32Temp);
		if(u32Temp>0) {
			SDMMC_Update_BOOTLOADER(0);
			u32Block0BurnOk = 1;
		}
	}

	if(u32UpdateBit&SD_UPDATE_EBOOT){
		u32Temp = FATGetFileSize(BOOT2_NAME, NULL);
		DPNOK(u32Temp);
		if(u32Temp>0) {
			SDMMC_Update_BOOTLOADER(1);
			u32EbootBurnOk = 1;
		}
	}
	
#endif


 //[david.modify] 2008-09-27 13:34
#if 1
	if(u32UpdateBit&SD_UPDATE_LOGO){
		u32Temp = FATGetFileSize(LOGOFILE_NAME, NULL);
		DPNOK(u32Temp);
		if(u32Temp>0) {
			SDMMC_Update_LOGO(0);
		}
	}
#endif	



	if(u32UpdateBit&SD_UPDATE_OS){

		if(UPDATE_OS_CHAINLST ==u32UpdateType)
		{
			u32Temp = FATGetFileSize(CHAINLST_NAME, NULL);	
			DPNOK(u32Temp);
			if(u32Temp<=0) {
				DPSTR("NO IMG");
				if(u32Block0BurnOk&&u32EbootBurnOk)
					return UPDATE_BOOTLOADEROK_OSFAIL;
				return -3;		
			}
		}else
		if(UPDATE_OS_XIPBIN==u32UpdateType)
		{
			u32Temp = FATGetFileSize(OSIMG_NAME, NULL);	
			DPNOK(u32Temp);
			if(u32Temp<=0) {
				DPSTR("NO IMG");
				if(u32Block0BurnOk&&u32EbootBurnOk)
					return UPDATE_BOOTLOADEROK_OSFAIL;			
				return -3;		
			}		
		}


		ShowProgress_Start();	

		DPSTR("READ IMG TO RAM");
		g_pDownPt = (UINT8 *)DEF_DOWNLOAD_ADDR;
		readPtIndex = (UINT32)DEF_DOWNLOAD_ADDR;	
		DPNOK(g_pDownPt);
		DPNOK(readPtIndex);	

		if(UPDATE_OS_XIPBIN==u32UpdateType){
			nRet = Parsing(OSIMG_NAME, BINIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
		}else
		if(UPDATE_OS_CHAINLST==u32UpdateType){
			nRet=Parsing(CHAINLST_NAME, LSTIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
		}

		DPNOK(u32UpdateType);
		DPNOK(nRet);
		if(nRet!=TRUE) {
			DPSTR("Parsing fail!");
			if(u32Block0BurnOk&&u32EbootBurnOk)
				return UPDATE_BOOTLOADEROK_OSFAIL;		
			return -4;	
		}		
		
	    	EdbgOutputDebugString ( "Read Finished \r\n");	
	    	EdbgOutputDebugString ( "g_pDownPt = 0x%x \r\n", g_pDownPt);	
		BspEbootUpdateProgressString(0, 0);
		BspEbootUpdateProgressBar(0, 50);   	


	}
	
	return TRUE;
	
}



void BL1ReadFromSD()
{
    
    g_pDownPt = (UINT8 *)DEF_DOWNLOAD_ADDR;
    readPtIndex = (UINT32)DEF_DOWNLOAD_ADDR;

    if (IsCardInserted())
    {
		FATInit();
		
		Parsing("Block0.nb0", NB0IMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
    	
    	EdbgOutputDebugString ( "g_pDownPt = 0x%x \r\n", g_pDownPt);
	}
}

void MULTIXIPReadFromSD()
{

    g_pDownPt = (UINT8 *)DEF_DOWNLOAD_ADDR;
    readPtIndex = (UINT32)DEF_DOWNLOAD_ADDR;

    if (IsCardInserted())
    {
		//FATInit();
		
        Parsing("chain.lst", LSTIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
    	
    	EdbgOutputDebugString ( "g_pDownPt = 0x%x \r\n", g_pDownPt);
	}
}

void NKReadFromSD()
{
    
    g_pDownPt = (UINT8 *)DEF_DOWNLOAD_ADDR;
    readPtIndex = (UINT32)DEF_DOWNLOAD_ADDR;

    if (IsCardInserted())
    {
		//FATInit();
		
        Parsing("nk.bin", BINIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
    	
    	EdbgOutputDebugString ( "g_pDownPt = 0x%x \r\n", g_pDownPt);
	}
}


void EbootReadFromSD()
{

    memset((PVOID)DEF_DOWNLOAD_ADDR,0xff,0x100000);
    g_pDownPt = (UINT8 *)DEF_DOWNLOAD_ADDR;
    readPtIndex = (UINT32)DEF_DOWNLOAD_ADDR;

    if (IsCardInserted())
    {
		//FATInit();
		
		Parsing("Eboot.bin", BINIMAGE, (BYTE *)DEF_DOWNLOAD_ADDR);
    	
    	EdbgOutputDebugString ( "g_pDownPt = 0x%x \r\n", g_pDownPt);
	}
}



BOOL SDReadData(DWORD cbData, LPBYTE pbData)
{
   	UINT8* pbuf = NULL;

    //EdbgOutputDebugString ("#### check point SDintf #1 cbdata = 0x%x pbData = 0x%x readPt = 0x%x\r\n",cbData,pbData,readPtIndex);    

	while(1)
	{
      
		if (1)// ((UINT32)g_pDownPt >= readPtIndex + cbData ) it has already copied data from SD card.
		{
			pbuf = (PVOID)readPtIndex;
			memcpy((PVOID)pbData, pbuf, cbData);
			//pbuf = (PVOID)OALPAtoUA(readPtIndex);
			// clear partial download memory to 0xff because data is already copied to buffer(pbData)
			memset(pbuf, 0xff, cbData);
            readPtIndex += cbData;
			break;
		}
	}

	return TRUE;
}

void BigToSmall(LPBYTE pAlphabet)
{
    int i;

    for ( i = 0 ; pAlphabet[i] != '\0' ; i++)
    {
        if ( pAlphabet[i] >= 65 && pAlphabet[i] <= 90 )
            pAlphabet[i] += 32;
    }
}


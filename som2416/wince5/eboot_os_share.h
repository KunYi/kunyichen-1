#ifndef EBOOT_OS_SHARE_H
#define  EBOOT_OS_SHARE_H


#include <spi_david.h>
#include <pwm_david.h>


#pragma pack(1)

typedef struct {
	/* BITMAPFILEHEADER*/
	BYTE	bfType[2];
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
} BMPFILEHEAD;

/* windows style*/
typedef struct {
	/* BITMAPFILEHEADER*/
	BMPFILEHEAD bmpH;
	/* BITMAPINFOHEADER*/
	DWORD	BiSize;
	LONG	BiWidth;
	LONG	BiHeight;
	WORD	BiPlanes;			// 01
	WORD	BiBitCount;			
	DWORD	BiCompression;		// 00 00 00 00
	DWORD	BiSizeImage;		//
	LONG	BiXpelsPerMeter;
	LONG	BiYpelsPerMeter;
	DWORD	BiClrUsed;
	DWORD	BiClrImportant;
} BMPHEAD;

// david added.   2006-10-17 11:9
typedef struct
{
	char flag[4];		// "LZW!"
	UINT32 u32Size;	// FILE size = DATASIZE + SIZEOF(stLZW) 
}stLZW, *pstLZW;

// end
#pragma pack()


void RawData2LcdBuffer2(UINT16 *pu16Framebuffer, void *pVirAddr, UINT32 u32Bytes, UINT8 u8Dir, UINT32 u32Width, UINT32 u32Height)
{
	UINT32 u32Words, u32cnt;
//	UINT16 *pu16Framebuffer;
	UINT16 *pu16VirBuf, *pu16Line, *pu16SrcLine;
	int x, y;
// david 	 2006-09-23 11:41	
	UINT32 u32Cx, u32Cy, u32bits, u32MaxBytes;
//	volatile PDRIVER_GLOBALS v_pDriverGlobals = (volatile PDRIVER_GLOBALS)DRIVER_GLOBALS_U_VIRTUAL;		
	UINT32 u32Rotate = 0;
	UINT16 *pu16Dot=0;
	UINT8 u8Temp = 0;

 //[david.modify] 2008-07-03 14:32
DPNOK(u32Width); 
DPNOK(u32Height); 

	u32Cx = u32Width;
	u32Cy = u32Height;

	
	u32bits = 16;	
	u32MaxBytes = u32Cx*u32Cy*(u32bits/8);
// end	
	
//	pu16Framebuffer= (UINT16 *)FRAME_BUFFER_0_BASE_VIRTUAL;
	pu16VirBuf = (UINT16 *)pVirAddr;
	u32Words = u32Bytes/2;
	u32cnt = 0;



	// 从上向下填充
// 保证u32Width <= u32Cx, u32Height<= u32Cy;
	
	if(u8Dir ==1) {
		for(y=0;y<u32Height;y++){
			pu16Line = pu16Framebuffer + y*u32Cx;
			pu16SrcLine = pu16VirBuf + y*u32Width;
			for(x=0;x<u32Width;x++) {

	pu16Line[x] = pu16SrcLine[x];
				
			}
		}
	}else {
		//从下往上 向填充 BMP
		for(y=0;y<u32Height;y++){
			pu16Line = pu16Framebuffer + y*u32Cx;
			 //[david.modify] 2007-07-23 12:19 fix bug
			pu16SrcLine = pu16VirBuf + (u32Height-1-y)*u32Width;
			for(x=0;x<u32Width;x++) {

	pu16Line[x] = pu16SrcLine[x];

			}
		}	
	}


//	EPRINT("<<RawData2LcdBuffer\r\n");		
}




#define MAX_WIDTH 800
#define ERR_OPENFILE_FAIL -2
#define ERR_NOTBMPFILE -3
#define ERR_NOT24BITBMP -4
#define ERR_WIDTH_OVERFLOW -5
#define ERR_NOT565BMP -6

int Show565Bmp(UINT16 *pu16Framebuffer, UINT8 *p565bmpFile, UINT32 u32Bytes, UINT32 u32Cx, UINT32 u32Cy)
{
	UINT32 u32VirtAddr, u32Len, u32Temp;
	UINT8 u8Dir;
	int ret;
	BMPHEAD *stBmp565;
//	UINT32 u32Cx, u32Cy;
	UINT32 u32bits, u32MaxBytes;	
	UINT8 g_u8DebugTemp=0;
#if 0	
	volatile PDRIVER_GLOBALS v_pDriverGlobals = (volatile PDRIVER_GLOBALS)DRIVER_GLOBALS_U_VIRTUAL;		

	u32Cx = v_pDriverGlobals->lcd.CxScreen;
	u32Cy = v_pDriverGlobals->lcd.CyScreen;
	u32bits = v_pDriverGlobals->lcd.Bpp;	
	u32MaxBytes = u32Cx*u32Cy*(u32bits/8);	
#else
//	u32Cx = PANEL_WIDTH;
//	u32Cy = PANEL_HEIGHT;
	u32bits = 16;	
	u32MaxBytes = u32Cx*u32Cy*(u32bits/8);
#endif
		
//	u32VirtAddr = FLASH_CFG_LOGO_ADDR;	
	u32VirtAddr = (UINT32)p565bmpFile;
	stBmp565 = (BMPHEAD *)p565bmpFile; 
//	PrintMsg(u32VirtAddr, sizeof(BMPHEAD), 1);

	if( ( stBmp565->bmpH.bfType[0]!='B' )||( stBmp565->bmpH.bfType[1]!='M' ) )
	{
	      DPN(-1);
	      return ERR_NOTBMPFILE;		  
	}
	
	if(stBmp565->BiBitCount!=0x565) 		{
	      DPN(stBmp565->BiBitCount);
		return ERR_NOT565BMP;

	}	

	if(stBmp565->BiWidth>MAX_WIDTH) 		{
		DPN(stBmp565->BiWidth);
	      return ERR_WIDTH_OVERFLOW; 
	}	
	
	u32Len = stBmp565->BiSizeImage;
	u8Dir = 0;
	if(u32Len>u32MaxBytes) u32Len = u32MaxBytes;	// davi d 2006-09-23 12:31
	

	//DisplayInitialize();
	ret = 1;
	if(ret==1) {
//		ClearFrameBuffer();	
//		SetBackColor2Lcd(WHITECOLOR);
		DPNOK(u32Cx);
		DPNOK(u32Cy);
		DPNOK(stBmp565->BiWidth);
		DPNOK(stBmp565->BiHeight);
		RawData2LcdBuffer2(pu16Framebuffer, (void*)(u32VirtAddr+stBmp565->bmpH.bfOffBits), u32Len, u8Dir, stBmp565->BiWidth, stBmp565->BiHeight);	
	}else {
		DPN(ret);
		return FALSE;
	}

	return TRUE;


}


#endif


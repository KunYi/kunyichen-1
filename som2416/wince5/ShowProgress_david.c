/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:
ShowProgress.c

Abstract:
Show progress of updating image from SD card.

Functions:


Notes:

--*/
/******************************************************************************
* This source file is the proprietary property of Centrality Communications Inc
* Copyright (c) 2003 Centrality Communications Inc
* All Rights Reserved
*
*
*
******************************************************************************/

#include <windows.h>
#include <eboot_inc_david.h>
#include "Showprogress_david.h"
#include <image_cfg.h>	//IMAGE_FRAMEBUFFER_UA_BASE
#include "display.h"

#define DISPLAY_WIDTH LCD_SCR_XSIZE
#define DISPLAY_HEIGHT LCD_SCR_YSIZE


extern DWORD     dwBinTotalLen;
//extern FlashInfo g_fi;

static BYTE  g_bPhasePercentArray[] = {0, 0, 0, 0, 0};

static int g_dwPgrBorderLeft, g_dwPgrBorderTop;
static int g_dwPercStrPosX, g_dwPercStrPosY;
BOOL g_bScreenInited = FALSE;

#define MAX(a, b)   ((a)>(b)?(a):(b))
#define MIN(a, b)   ((a)<(b)?(a):(b))
#define ABS(a)      ((a)>=0?(a):(-(a)))

static ATLASDC g_CurrentDC;
ATLASHDC  g_hCurrentDC = &g_CurrentDC;
#if 0
static DC g_OSDDC;
HDC g_hOSDDC = &g_OSDDC;
static DC g_OSD2DC;
HDC g_hOSD2DC = &g_OSD2DC;
#endif


void InitScreenDC_ORG (BYTE device_num, DWORD dwFrameBuffAddr)
{
    if(DISPLAY_LCD_MAIN ==    device_num)
    {
	DPNOK(DISPLAY_WIDTH);
	DPNOK(DISPLAY_HEIGHT);
	
        g_hCurrentDC->bpp = 16;//sizeof(COLORVAL) * 8;
        g_hCurrentDC->width = DISPLAY_WIDTH;
        g_hCurrentDC->height = DISPLAY_HEIGHT;
        g_hCurrentDC->stride = DISPLAY_WIDTH * sizeof(COLORVAL);
        g_hCurrentDC->size = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(COLORVAL);
        g_hCurrentDC->addr = (unsigned char*)dwFrameBuffAddr;

        g_hCurrentDC->font = &FONT_8X16;
        g_hCurrentDC->bkcolor = LGRAY6;
        g_hCurrentDC->textcolor = XBLACK;
        g_hCurrentDC->bkmode = BKMODE_OPAQUE;
    }
}

void InitScreenDC (dwFrameBuffAddr)
{
	InitScreenDC_ORG(DISPLAY_LCD_MAIN, dwFrameBuffAddr);
}

// we only support 16bit draw char function now
// support other mode later 
int DrawChar(char c, int* x, int* y)
{
    int i, j;
    unsigned short* pFB;
    unsigned char* pCharEntryB;
    unsigned short* pCharEntryW;
    int char_width, char_height;
    unsigned int pixval;

    if (*x < 0 || *y < 0)
    {
        *x = *x + g_hCurrentDC->font->width + g_hCurrentDC->hspacing;
        *y = *y + g_hCurrentDC->font->height + g_hCurrentDC->vspacing;	
        return 0;
    }
    if (*x >= g_hCurrentDC->width || *y>=g_hCurrentDC->height)
    {
        return 0;
    }	

    pFB = (unsigned short*)g_hCurrentDC->addr;
    pFB += (*y * g_hCurrentDC->width) + *x;
    char_width = g_hCurrentDC->font->width;
    char_height = g_hCurrentDC->font->height;
    pCharEntryB = (unsigned char *)g_hCurrentDC->font->table;
    pCharEntryB += ((unsigned char)c * g_hCurrentDC->font->charlen);
    pCharEntryW = (unsigned short *)pCharEntryB;

    switch (g_hCurrentDC->font->width)
    {
    case 8:
        for (i=0; i<g_hCurrentDC->font->height; i++)
        {
            if ((*y + i) < 0) 
            {
                pCharEntryB++;
                continue;
            }
            if ((*y + i) >= g_hCurrentDC->height)
            {
                return 0;
            }

            for (j=0; j<g_hCurrentDC->font->width; j++)
            {
                if ((*x + j) < 0) 
                {
                    continue;
                }
                if ((*x + j) >= g_hCurrentDC->width) 
                {
                    break;
                }

                pixval = ((*pCharEntryB) >> (7-j)) & 0x01;

                if (pixval)
                {
                    *pFB = g_hCurrentDC->textcolor;
                }
                else if (g_hCurrentDC->bkmode == BKMODE_OPAQUE)
                {
                    *pFB = g_hCurrentDC->bkcolor;
                }
                pFB++;
            }
            pCharEntryB++;
            pFB+= (g_hCurrentDC->width - j);
        }

        break;
    case 16:
        for (i=0; i<g_hCurrentDC->font->height; i++)
        {
            if ((*y + i) < 0) 
            {
                pCharEntryW++;
                continue;
            }
            if ((*y + i) >= g_hCurrentDC->height)
            {
                return 0;
            }

            for (j=0; j<g_hCurrentDC->font->width; j++)
            {
                if ((*x + j) < 0) continue;
                if ((*x + j) >= g_hCurrentDC->width) break;

                pixval = ((*pCharEntryW) >> (g_hCurrentDC->font->width-1-j)) & 0x01;

                if (pixval)
                {
                    *pFB = g_hCurrentDC->textcolor;
                }
                else if (g_hCurrentDC->bkmode == BKMODE_OPAQUE)
                {
                    *pFB = g_hCurrentDC->bkcolor;
                }
                pFB++;
            }
            pCharEntryW++;
            pFB+= g_hCurrentDC->width - j;
        }
        break;
    default:
        break;
    }

    *x = *x + g_hCurrentDC->font->width + g_hCurrentDC->hspacing;
    *y = *y + g_hCurrentDC->font->height + g_hCurrentDC->vspacing;

    return 1;

}

int strlen_(char* c)
{
    int i;
    i = 0;
    while(0 != *c++)
    {
        i++;
    }
    return i;
}

void GetTextSize(char* s, DWORD *width, DWORD *height)
{
    int slen = strlen_(s);
    *width = slen * (g_hCurrentDC->font->width + g_hCurrentDC->hspacing);
    *height = g_hCurrentDC->font->height + g_hCurrentDC->vspacing;
}

extern UINT32 g_u32DisplayOK;

extern void InitDisplay();
void DrawString (
                 PUCHAR s,
                 int startx,
                 int starty,
                 unsigned int uFormat
                 )
{
    int startx_m = startx;
    int starty_m = starty;
    int width, height;
    int x, y;
    int line_no;

 //[david.modify] 2008-09-07 17:25
 // 进入更新后再初始化LCD
 	DPNOK(g_u32DisplayOK);
	if(!g_u32DisplayOK){
		InitDisplay();
		InitScreenDC(IMAGE_FRAMEBUFFER_UA_BASE);
	}



    GetTextSize(s, &width, &height);

    if (uFormat & DT_CENTER)
    {
        startx_m = startx - width/2;
    }
    if (uFormat & DT_RIGHT)
    {
        startx_m = startx - width;
    }

    if (uFormat & DT_VCENTER)
    {
        starty_m = starty - height/2;
    }
    if (uFormat & DT_BOTTOM)
    {
        starty_m = starty - height;
    }

    /*
    if (startx_m < 0) startx_m = 0;
    if (startx_m >= g_hCurrentDC->width) startx_m  == g_hCurrentDC->width - 1;
    if (starty_m < 0) startx_m = 0;
    if (starty_m >= g_hCurrentDC->height) starty_m  == g_hCurrentDC->height - 1;
    */  
    x = startx_m;
    y = starty_m;

    if (uFormat & DT_SINGLELINE)
    {
        while (*s!='\0')
        {
            DrawChar(*s, &x, &y);
            s++;
            y = starty_m;
        }
    }
    else
    {   // multiline
        line_no = 0;
        if (width > g_hCurrentDC->width)
        {
            x = startx_m = 0;
        }
        while (*s!='\0')
        {
            DrawChar( *s, &x, &y);
            s++;
            if (x >= g_hCurrentDC->width)
            {
                x = startx_m;
                line_no++;
            }
            y = starty_m + line_no*height;
        }
    }

}

int GetROPValue(COLORVAL *D, COLORVAL P, int rop2)
{
    switch( rop2 )
    {
    case R2_BLACK:	        *D = 0;			break;
    case R2_NOTMERGEPEN:    *D = ~( P | *D );	break;
    case R2_MASKNOTPEN:	    *D = ~P & *D;		break;
    case R2_NOTCOPYPEN:	    *D = ~P;			break;
    case R2_MASKPENNOT:	    *D = P & ~*D;		break;
    case R2_NOT:	        *D = ~*D;			break;
    case R2_XORPEN:	        *D = P ^ *D;		break;
    case R2_NOTMASKPEN:	    *D = ~( P & (*D) );	break;
    case R2_MASKPEN:	    *D = P & *D;		break;
    case R2_NOTXORPEN:      *D = ~( P ^ *D );	break;
    case R2_NOP:            *D = *D;			break;
    case R2_MERGENOTPEN:    *D = ~P | *D;		break;
    case R2_COPYPEN:        *D = P;				break;
    case R2_MERGEPENNOT:    *D = P | ~*D;		break;
    case R2_MERGEPEN:       *D = P | *D;		break;
    case R2_WHITE:          *D = 0xffff;		break;
    default:
        return 0;
    }

    return 1;
}

int DrawPixelRop(int x, int y, COLORVAL color, int rop2)
{
    unsigned char *pFB_b;
    unsigned short *pFB_w;
    COLORVAL colord;


    if (x < 0 || x >= g_hCurrentDC->width || y < 0 || y >= g_hCurrentDC->height)
        return 0;


    switch (g_hCurrentDC->bpp)
    {
    case 16:
        pFB_w = (unsigned short *)(g_hCurrentDC->addr + y*g_hCurrentDC->stride + x*g_hCurrentDC->bpp/8);
        colord = *pFB_w;
        GetROPValue(&colord, color, rop2);
        *pFB_w = (unsigned short)(colord & 0xffff);
        break;
    case 8:
        pFB_b = (unsigned char *)(g_hCurrentDC->addr + y*g_hCurrentDC->stride + x*g_hCurrentDC->bpp/8);
        colord = *pFB_b;
        GetROPValue(&colord, color, rop2);
        *pFB_b = (unsigned char)(colord & 0xff);
        break;
    default:
        break;
    }
    return 1;
}

int DrawPixel( int x, int y, COLORVAL color)
{
    return DrawPixelRop(x, y, color, R2_COPYPEN);
}


int DrawLineRop (
                 int x1,
                 int y1,
                 int x2,
                 int y2,
                 COLORVAL color,
                 int rop2
                 )
{
    int i;
    int deltax, deltay, delta;
    int stepx, stepy;
    int starty, endy;
    int swaptmp;
    int steep;


    starty = y1; endy = y2;

    if (x1 == x2)   	// vertical line
    {
        for (i=MIN(y1, y2); i<=MAX(y1, y2); i++)
        {
            DrawPixelRop(x1, i, color, rop2);
        }
    }
    else if (y1 == y2)  	// horizontal line
    {
        for (i=MIN(x1, x2); i<=MAX(x1, x2); i++)
        {
            DrawPixelRop( i, y1, color, rop2);
        }
    }
    else    // arbitrary line drawing using bresenham's algorithm
    {
        deltax = ABS(x2 - x1);
        stepx = (x2>x1)?1:(-1);
        deltay = ABS(y2 - y1);
        stepy = (y2>y1)?1:(-1);

        steep = (deltay > deltax);
        if (steep)
        {
            swaptmp = x1; x1 = y1; y1 = swaptmp;    //swap x1, y1
            swaptmp = deltax; deltax = deltay; deltay = swaptmp;// swap deltax/y
            swaptmp = stepx; stepx = stepy; stepy = swaptmp;    // swap stepx/y
        }
        delta = deltay*2 - deltax;
        for (i = 0; i < deltax; i++)
        {
            if (steep)
                DrawPixelRop(y1, x1, color, rop2);
            else
                DrawPixelRop(x1, y1, color, rop2);

            while (delta > 0)
            {
                y1 += stepy;
                delta -= deltax * 2;
            }
            x1 += stepx;
            delta += deltay * 2;
        }

        DrawPixelRop(x2, y2, color, rop2);   // final pixel
    }
    return 1;

}

int DrawLine (
              int x1,
              int y1,
              int x2,
              int y2,
              COLORVAL color
              )
{
    return DrawLineRop (x1, y1, x2, y2, color, R2_COPYPEN);
}


int DrawRectRop(
                int x1,
                int y1,
                int x2,
                int y2,
                COLORVAL color,
                int rop2
                )
{
    int retval = 1;
    retval &= DrawLineRop(x1, y1, x2, y1, color, rop2);
    retval &= DrawLineRop(x1, y2, x2, y2, color, rop2);
    retval &= DrawLineRop(x1, y1, x1, y2, color, rop2);
    retval &= DrawLineRop(x2, y1, x2, y2, color, rop2);
    return retval;
}

int DrawRect(
             int x1,
             int y1,
             int x2,
             int y2,
             COLORVAL color
             )
{
    return DrawRectRop(x1, y1, x2, y2, color, R2_COPYPEN);
}

int FillRectRop(
                int x1, 
                int y1,
                int x2,
                int y2,
                COLORVAL color,
                int rop2
                )
{
    int i;
    int retval = 1;
    for(i=MIN(y1, y2); i<=MAX(y1, y2); i++)
    {
        retval &= DrawLineRop(x1, i, x2, i, color, rop2);
    }

    return retval;
}

int FillRectangle(
                  int x1, 
                  int y1,
                  int x2,
                  int y2,
                  COLORVAL color
                  )
{
    return FillRectRop(x1, y1, x2, y2, color, R2_COPYPEN);
}


void DrawBackGround(COLORVAL Color)
{
    FillRectangle(0, 0, g_hCurrentDC->width, g_hCurrentDC->height, Color); 
}


 //[david.modify] 2008-07-19 09:55
BOOL ShowProgress_Start(UINT32 u32Percent)
{
//    InitScreenDC(DISPLAY_LCD_MAIN, IMAGE_FRAMEBUFFER_UA_BASE);
    DrawBackGround(LGRAY6);

    g_dwPgrBorderLeft = (DISPLAY_WIDTH - PGR_BAR_WIDTH - 2) / 2;
    g_dwPgrBorderTop = (DISPLAY_HEIGHT - PGR_BAR_HEIGHT - 2) / 2;

    g_dwPercStrPosX = (DISPLAY_WIDTH - DEFAULT_FONT_WIDTH * 3 - 2) / 2; // Assume the width of string xx% is 3 * DEFAULT_FONT_WIDTH
    g_dwPercStrPosY = g_dwPgrBorderTop + PGR_BAR_HEIGHT + 5;

    //    DrawString ("ROM Update", 125, g_dwPgrBorderTop - 70, DT_LEFT);
    //    DrawString ("Ver 01.01", 130, g_dwPgrBorderTop - 45, DT_LEFT);
    DrawString("Updating image...", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
    DrawRect( g_dwPgrBorderLeft, g_dwPgrBorderTop, 
        g_dwPgrBorderLeft+PGR_BAR_WIDTH+1, g_dwPgrBorderTop+PGR_BAR_HEIGHT+1,XBLACK);

    DrawString("0% ", g_dwPercStrPosX, g_dwPercStrPosY,DT_LEFT);

    g_bScreenInited = TRUE;
    return g_bScreenInited;

}

VOID BspEbootUpdateProgressString(BYTE phase, BOOL bSuccess)
{
    if (!g_bScreenInited)
        return;

    switch(phase) {
    case UPD_PHASE_READFILE:
        if (bSuccess)
            DrawString("Download Images Success   ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        else
            DrawString("Downloading Images...     ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
//        DrawString("Update process... 2/4", g_dwPgrBorderLeft, g_dwPgrBorderTop - 40,DT_LEFT);

    	break;
    case UPD_PHASE_ERASE:
        if (bSuccess)
            DrawString("Erase Nandflash Success   ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        else
           DrawString("Erasing Nandflash...       ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        DrawString("Update process... 1/4", g_dwPgrBorderLeft, g_dwPgrBorderTop - 40,DT_LEFT);

    	break;
    case UPD_PHASE_CHECKNF:
        if (bSuccess)
            DrawString("Check Nandflash Success   ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        else
            DrawString("Checking Nandflash...     ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        DrawString("Update process... 0/4", g_dwPgrBorderLeft, g_dwPgrBorderTop - 40,DT_LEFT);

        break;
    case UPD_PHASE_CREATEPARITITION:
        if (bSuccess)
            DrawString("Create Partitions Success ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        else
            DrawString("Creating Partitions...    ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        DrawString("Update process... 3/4", g_dwPgrBorderLeft, g_dwPgrBorderTop - 40,DT_LEFT);
        break;

    case UPD_PHASE_PROGRAM:
        if (bSuccess)
            DrawString("Program Images Success    ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        else
            DrawString("Programming Images...     ", g_dwPgrBorderLeft, g_dwPgrBorderTop - 20,DT_LEFT);
        DrawString("Update process... 4/4", g_dwPgrBorderLeft, g_dwPgrBorderTop - 40,DT_LEFT);
        break;
    }

}


VOID BspEbootUpdateProgressBar(BYTE phase, BYTE percent)
{
    static BYTE oldPercent = 0;
    DWORD i;
    BOOL bEndOfUpdate=FALSE;	
    BYTE  pStr[20] = {0};
//    RETAILMSG(1,(_T("Try to UpdateProgressBar phase =%d Percent =%d....\r\n"),(DWORD)phase,(DWORD)percent));
    
    if (!g_bScreenInited || percent > 100)
        return;

    BspEbootUpdateProgressString(phase, 0);

    if (percent < oldPercent)
        FillRectangle(g_dwPgrBorderLeft+1, g_dwPgrBorderTop+1, g_dwPgrBorderLeft+PGR_BAR_WIDTH,
            g_dwPgrBorderTop+PGR_BAR_HEIGHT, g_hCurrentDC->bkcolor);
    
    oldPercent = percent;

    FillRectangle(g_dwPgrBorderLeft+1, g_dwPgrBorderTop+1, g_dwPgrBorderLeft + PGR_BAR_WIDTH* percent / 100,
        g_dwPgrBorderTop+PGR_BAR_HEIGHT, XBLUE);

    i = 0;
    pStr[i++] = ' ';
    pStr[i++] = ' ';
    pStr[i++] = ' ';
    pStr[i++] = ' ';
    pStr[i++] = ' ';
    pStr[i++] = ' ';
    pStr[i++] = ' ';
    pStr[i++] = ' ';
    DrawString(pStr, g_dwPercStrPosX, g_dwPercStrPosY,DT_LEFT);
	
    i = 0;
    if (percent == 100)
    {
        pStr[i++] = '1';
        pStr[i++] = '0';
        bEndOfUpdate =TRUE;	
        percent -= 100;
    }
    if (percent >= 10)
        pStr[i++] = percent / 10 + '0';
    pStr[i++] = percent % 10 + '0';
    pStr[i++] = '%';
    pStr[i++] = 0;
    pStr[i++] = 0;

    DrawString(pStr, g_dwPercStrPosX, g_dwPercStrPosY,DT_LEFT);
    if(bEndOfUpdate)
    {
        BspEbootUpdateProgressString(phase, 1);

        FillRectangle(g_dwPgrBorderLeft+1, g_dwPgrBorderTop+1, g_dwPgrBorderLeft+PGR_BAR_WIDTH,
            g_dwPgrBorderTop+PGR_BAR_HEIGHT, XBLUE);

        //DeInitLCD();
    }	

//    RETAILMSG(1,(_T("Try to UpdateProgressBar phase =%d Percent =%d done.\r\n"),(DWORD)phase,(DWORD)percent));


}

 



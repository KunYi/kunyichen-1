#ifndef _SHOWPROGRESS_H_
#define _SHOWPROGRESS_H_


 //[david.modify] 2008-09-09 16:52



//#include "display.h"

/**
* Eboot Frame buffer physical address & virtual address
**/
#if 0
#define EBOOT_LCD_FRAME_BUF_PHYS_ADDR			0xc0100000
#define EBOOT_LCD_FRAME_BUF_VIRT_ADDR			0x8c100000
#define EBOOT_LCD_FRAME_UNBUF_VIRT_ADDR			0xac100000
#endif

// Customize area

#define	BACK_GROUND_COLOR	0x3219
#define	TEXT_COLOR          0xFFFF
#define	PGR_BORDER_COLOR    0xFFFF
#define	PGR_FILL_COLOR      0xFFFF

#define ROTATION_ANGLE  	0

// Progress bar length exclude border
#define	PGR_BAR_WIDTH	    200
#define	PGR_BAR_HEIGHT	    30

// Update phase time parameters, should be tuned for different hardware
// We use 10us as unit or the calculated value will exceed DWORD range
#define	TIME_ERASE_ONE_BLOCK    0  // Average time(unit: 10us) to erase a block of Samsung K9F1208U0A
#define TIME_CHECK_ONE_BLOCK    719 // Average time(unit: 10us) to check a block of Samsung K9F1208U0A

#define	TIME_PROGRAM_ONE_BLOCK  653 // Average time(unit: 10us) to erase and write a block of Samsung K9F1208U0A
#define TIME_READ_1KB           (1L * 1000 * 100 / 1800)   // Average time(unit: 10us) to read 1KB data from
// card, assume card read speed is 1800KB/s


// Default font width, we can use it as width of space char
#define	DEFAULT_FONT_WIDTH	8

#define CALCULATE_STRIDE( width, bpp ) (((((ULONG)bpp) * ((ULONG)width) + 31) >> 5) << 2)
#define	INTERRUPT_TIMEOUT	100000

// Update phase
#define UPD_PHASE_READFILE	0
#define	UPD_PHASE_ERASE		1
#define UPD_PHASE_CHECKNF   2
#define UPD_PHASE_CREATEPARITITION   3
#define UPD_PHASE_PROGRAM   4

#define LCD_REFRESH_RATE	60



// Make rgb value into 16 bits
//#define _T_Sdramaddr 		0xc0800000

typedef unsigned short	COLORVAL;	/* color value*/

#define DISPLAY_LCD_MAIN	0
#define DISPLAY_OSD_1		1
#define DISPLAY_OSD_2       2

#define LCD_OSD_WIDTH  120
#define LCD_OSD_HEIGHT 160




#define RGB565(r,g,b) ((COLORVAL)(((r)&0x1f)<<11)|(((g)&0x3f)<<5)|((b)&0x1f))

#define XBLACK       (RGB565(0,0,0))
#define XWHITE       (RGB565(31,63,31))
#define XGRAY        (RGB565(15,31,15))
#define XRED         (RGB565(31,0,0))
#define XGREEN       (RGB565(0,63,0))
#define XBLUE        (RGB565(0,0,31))
#define XYELLOW      (RGB565(31,63,0))
#define XCYAN        (RGB565(0,63,31))
#define XMAGENTA     (RGB565(31,0,31))
#define XGRAY0       (RGB565(3,7,3))
#define XGRAY1       (RGB565(7,15,7))
#define XGRAY2       (RGB565(11,23,11))
#define XGRAY3       (RGB565(15,31,15))
#define XGRAY4       (RGB565(19,39,19))
#define XGRAY5       (RGB565(23,47,23))
#define XGRAY6       (RGB565(27,55,27))
#define XGRAY7       (RGB565(31,63,31))

#define LBLACK      (RGB565(0,0,0))
#define LWHITE      (RGB565(30,62,30))
#define LGRAY       (RGB565(14,30,14))
#define LRED        (RGB565(30,0,0))
#define LGREEN      (RGB565(0,62,0))
#define LBLUE       (RGB565(0,0,30))
#define LYELLOW     (RGB565(30,62,0))
#define LCYAN       (RGB565(0,62,30))
#define LMAGENTA    (RGB565(30,0,30))
#define LGRAY0      (RGB565(2,6,2))
#define LGRAY1      (RGB565(6,14,6))
#define LGRAY2      (RGB565(10,22,10))
#define LGRAY3      (RGB565(14,30,14))
#define LGRAY4      (RGB565(18,38,18))
#define LGRAY5      (RGB565(22,46,22))
#define LGRAY6      (RGB565(26,54,26))
#define LGRAY7      (RGB565(30,62,30))

// bkmode
#define BKMODE_OPAQUE      0
#define BKMODE_TRANSPARENT 1

// DrawText() Format Flags
#define DT_TOP                      0x00000000
#define DT_LEFT                     0x00000000
#define DT_CENTER                   0x00000001
#define DT_RIGHT                    0x00000002
#define DT_VCENTER                  0x00000004
#define DT_BOTTOM                   0x00000008
#define DT_SINGLELINE               0x00000020

// private ROP code
#define R2_BLACK            1   /*  0       */
#define R2_NOTMERGEPEN      2   /* DPon     */
#define R2_MASKNOTPEN       3   /* DPna     */
#define R2_NOTCOPYPEN       4   /* PN       */
#define R2_MASKPENNOT       5   /* PDna     */
#define R2_NOT              6   /* Dn       */
#define R2_XORPEN           7   /* DPx      */
#define R2_NOTMASKPEN       8   /* DPan     */
#define R2_MASKPEN          9   /* DPa      */
#define R2_NOTXORPEN        10  /* DPxn     */
#define R2_NOP              11  /* D        */
#define R2_MERGENOTPEN      12  /* DPno     */
#define R2_COPYPEN          13  /* P        */
#define R2_MERGEPENNOT      14  /* PDno     */
#define R2_MERGEPEN         15  /* DPo      */
#define R2_WHITE            16  /*  1       */
#define R2_LAST             16

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FONTOBJ
{
    int height;		/* font height in pixel */
    int width;		/* font width in pixel */
    int charlen;	/* font byte count per char */
    const unsigned char* table;	/* font table entry */
} FONTOBJ, *LPFONTOBJ;

/* device context*/
typedef struct _atlashdc
{
    int	bpp;		/* # bpp*/
    int	height;		/* height */
    int	width;		/* width */
    int   stride;		/* bytes in a line */
    DWORD	size;		/* size of memory allocated */
    unsigned char *addr;		/* address of memory allocated (memdc or fb) */
    LPFONTOBJ	font;   /* font object used in the dc */
    int hspacing;			/* space in pixel between chars */
    int vspacing;			/* space in pixel between lines */
    COLORVAL	textcolor;	/* text color */
    COLORVAL	bkcolor;	/* background color */
    int bkmode;     /* background mode */
} ATLASDC, *ATLASHDC;

extern FONTOBJ FONT_8X8;
extern FONTOBJ FONT_8X16;
extern FONTOBJ FONT_16X32;




void InitScreenDC (DWORD dwFrameBuffAddr);
//HDC InitOSDDC(HDC dc);
//HDC InitOSD2DC(HDC dc);

int DrawChar(char c, int* x, int* y);
//COLORVAL SetTextColor(ATLASHDC  dc, COLORVAL color);
void DrawString(PUCHAR s, int startx, int starty, unsigned int uFormat);
void DrawBackGround(COLORVAL color);

//LPFONTOBJ SetFont(LPFONTOBJ pFont);
int DrawPixel( int x, int y, COLORVAL color);

int DrawLine(int x1, int y1, int x2, int y2, COLORVAL color);

int DrawRect(int x1, int y1, int x2, int y2, COLORVAL color);

int FillRectangle(int x1, int y1, int x2, int y2, COLORVAL color);

//void DrawCircle( int x_center, int y_center, int radius, COLORVAL color);

//void FillCircle(int x_center, int y_center, int radius, COLORVAL color);
//int DrawBitmap(ATLASHDC  dc, int x, int y, BITMAP *pBmp, int rop2);
#ifdef __cplusplus
}
#endif


#endif //_SHOWPROGRESS_H_


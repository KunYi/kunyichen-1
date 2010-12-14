#ifndef __POST_H_
#define __POST_H_

#ifdef __cplusplus
extern "C" {
#endif



#define U8	unsigned char
#define U16	unsigned short
#define U32	unsigned int

#define POST_STATE_INIT		1
#define POST_STATE_RUN		2
#define POST_STATE_PAUSE	3
#define POST_STATE_STOP	0

#define	POST_INPUT_BUFFER	0x33DE0000
#define	POST_OUTPUT_BUFFER	0x33EE0000

BOOL PST_Close(DWORD hOpenContext);
BOOL PST_Deinit(DWORD hDeviceContext);
DWORD PST_Init(DWORD dwContext);
DWORD PST_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);
void PST_PowerUp(DWORD hDeviceContext);
void PST_PowerDown(DWORD hDeviceContext);
DWORD PST_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count);
DWORD PST_Seek(DWORD hOpenContext, long Amount, DWORD Type);
DWORD PST_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes);

BOOL PST_IOControl(DWORD hOpenContext, 
				   DWORD dwCode, 
				   PBYTE pBufIn, 
				   DWORD dwLenIn, 
				   PBYTE pBufOut, 
				   DWORD dwLenOut, 
				   PDWORD pdwActualOut);



BOOL PostClockOn(BOOL bOnOff);
void PostProcessor_PowerDown(void);
void PostProcessor_PowerUp(void);


#ifdef __cplusplus
}
#endif


//POST Regigster Value
#define	POST_MPEG4		(1<<9)
#define	POST_H263			(0<<9)
#define	POST_SRC420		(1<<8)
#define	POST_SRC422		(0<<8)
#define	POST_INTEN			(1<<7)
#define	POST_POSTINT		(1<<6)
#define	POST_POSTENVID	(1<<5)
#define	POST_OUTRGB16		(0<<4)
#define	POST_OUTRGB24		(1<<4)
#define	POST_INRGB			(1<<3)
#define	POST_INYUV			(0<<3)
#define	POST_INTERLEAVE	(1<<2)
#define	POST_NOINTERLEAVE	(0<<2)
#define	POST_INRGBFMT		(1<<1)
#define	POST_INYCBYCR		(0)
#define	POST_INCBYCRY		(1)

typedef struct _POSTINFO
{
	U32	nOrgSrcWidth;
	U32	nOrgSrcHeight;
	U32	nSrcWidth;	
	U32	nSrcHeight;
	U32	nDestWidth;	
	U32	nDestHeight;
	U32 nSrcStartX;
	U32 nSrcStartY;
} POSTINFO;

// changed by yash for 24A0 wince .net 5.0 200105
#define POSTINPUT_BASE		0x13D10000   //MAX 640*480*3/2= 0x70800 added by junkim for post input DMA-> virtual copy page acess 0x71000
#define POSTOUTPUT_BASE 	0x13D81000 //MAX 640*480*2= 0x96000 added by junkim for post output DMA 


#define VIRTUAL_OFFSET		0x70000000
#define VIRTUAL_ADDR_OFFSET	VIRTUAL_OFFSET // for MPEG4

#define IOCTL_POST_INIT			CTL_CODE( FILE_DEVICE_VIDEO, 1, METHOD_NEITHER,FILE_ANY_ACCESS)
#define IOCTL_POST_RUN			CTL_CODE( FILE_DEVICE_VIDEO, 2, METHOD_NEITHER,FILE_ANY_ACCESS)
#define IOCTL_POST_STOP		CTL_CODE( FILE_DEVICE_VIDEO, 3, METHOD_NEITHER,FILE_ANY_ACCESS)
#define IOCTL_POST_GETINPUTBUFFER		CTL_CODE( FILE_DEVICE_VIDEO, 4, METHOD_NEITHER,FILE_ANY_ACCESS)
#define IOCTL_POST_GETOUTPUTBUFFER		CTL_CODE( FILE_DEVICE_VIDEO, 5, METHOD_NEITHER,FILE_ANY_ACCESS)

#endif /* __POST_H_ */

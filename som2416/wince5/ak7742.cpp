
#include <windows.h>
#include <Ak7742.h>
//#include "DMA.h"
#include <ceddk.h>
//#include <S3C2450REF_GPIO.h>
//#include <S3c2450_ioport.h>
#include <i2c.h>
#include <bsp.h>	

#include <xllp_gpio_david.h>
#include "S3C2450_ioport.h"
#include "S3C2450_base_regs.h"


//#define AK7742_CHIP	 	1

#define AK7742_IDW   	0x30
#define AK7742_IDR   	0x31

#define WODM_BT_SCO_AUDIO_CONTROL       500

//----------------------------------IIC------------------------------------------------
static  HANDLE  v_hI2C = NULL;
static  I2C_FASTCALL v_I2C_fc;     // I2C Fastcall driver-to-driver entrypoints


//-------------------------------- Global Variables --------------------------------------

volatile S3C2450_IOPORT_REG *v_pIOPregs		= NULL;		// GPIO registers (needed to enable AC97)
volatile S3C2450_AC97_REG	 *v_pAC97regs	= NULL;		// AC97 control registers
volatile S3C2450_DMA_REG    *v_pDMAregs		= NULL;		// DMA registers (needed for I/O on AC97)
volatile S3C2450_CLKPWR_REG *v_pCLKPWRreg	= NULL;		// Clock power registers (needed to enable AC97)
volatile S3C2450_INTR_REG *s2450INT 		= NULL;

stGPIOInfo g_AKM_GPIOInfo[]={
	{ AKM_RST, 0, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},
	};




static void I2cInit(void)
{
     DWORD dwErr,dwbytes;

     RETAILMSG(1, (TEXT("akm I2cInit I2C0\r\n")) );

     v_hI2C =  CreateFile( L"I2C0:",
                             GENERIC_READ|GENERIC_WRITE,
                             FILE_SHARE_READ|FILE_SHARE_WRITE,
                             NULL, OPEN_EXISTING, 0, 0);
     if ( INVALID_HANDLE_VALUE == v_hI2C ) {
        dwErr = GetLastError();
        //DEBUGMSG(ZONE_ERR, (TEXT("Error %d opening device '%s' \r\n"), dwErr, L"I2C0:" ));
        RETAILMSG(1, (TEXT("Error %d opening device '%s' \r\n"), dwErr, L"I2C0:" ));
        return ;
    }
	 
     //RETAILMSG(1, (TEXT("WAV::CreateFile(\"I2C0\") \r\n")));

    //
    // Gat Fastcall driver-to-driver entrypoints
    //
    if ( !DeviceIoControl(v_hI2C,
                          IOCTL_I2C_GET_FASTCALL, 
                          NULL, 0, 
                          &v_I2C_fc, sizeof(v_I2C_fc),
                          &dwbytes, NULL) ) 
    {
        dwErr = GetLastError();
        RETAILMSG(1,(TEXT("IOCTL_I2C_GET_FASTCALL ERROR: %u \r\n"), dwErr));
        return;
    }            

}


//Written by Evan Tan, 2008-02-18

static DWORD AK7742_I2CWrite( UINT8 reg, UINT8 value)
{    
     DWORD dwErr;
     UCHAR data =(UCHAR)value;
	 
   //RETAILMSG(AUD_DBG, (TEXT("AKM7742_I2CWriteEx Reg:0x%.2X, Val:0x%.2X\r\n"), reg, data));
   
    // use the driver-to-driver call
    dwErr = v_I2C_fc.I2CWrite(v_I2C_fc.Context,
                              AK7742_IDW,   	// SlaveAddress
                              (UCHAR)reg,         // WordAddress
                              &data,
                              1);

    if ( dwErr ) {
        RETAILMSG(1, (TEXT("AK7742_I2CWrite ERROR: %u \r\n"), dwErr));
    }            
   
    return dwErr;
}


static DWORD AK7742_I2CWriteNBytes( UINT8 reg, PBYTE pdata, int n)
{
	DWORD dwErr;
//     	UCHAR data;

	// use the driver-to-driver call
    	dwErr = v_I2C_fc.I2CWrite(v_I2C_fc.Context,
                              AK7742_IDW,   	// SlaveAddress
                              (UCHAR)reg,         // WordAddress
                              (unsigned char *)pdata,
                              n);

    if ( dwErr ) {
        RETAILMSG(1, (TEXT("AK7742_I2CWrite ERROR: %u \r\n"), dwErr));
    }            
   
    return dwErr;

	
}



static DWORD AK7742_I2CRead(UINT8 reg)
{
    DWORD dwErr;
    UCHAR rdata = 0;
    //RETAILMSG(1,(TEXT("AK7742_I2CRead begin ==++=========\r\n")) );

    // use the driver-to-driver call
    dwErr = v_I2C_fc.I2CRead(v_I2C_fc.Context,
                             AK7742_IDR, 	// SlaveAddress
                             (UCHAR)reg,      // WordAddress
                             (unsigned char *)&rdata ,
                             1);
    
    if ( dwErr )  {        
        RETAILMSG(1,(TEXT("AK7742_I2CRead ERROR: %u \r\n"), dwErr));
    }            

    RETAILMSG(1,(TEXT("AK7742_I2CRead data =  0x%x \r\n"), rdata));

    return dwErr;
}



BOOL
DllEntry(
    HINSTANCE   hinstDll,             /*@parm Instance pointer. */
    DWORD   dwReason,                 /*@parm Reason routine is called. */
    LPVOID  lpReserved                /*@parm system parameter. */
    )
{
	return TRUE;
}

DWORD AKM_Init()
{
	RETAILMSG(1, (TEXT("AKM_Init begin\r\n")));

#if 1
	v_pIOPregs = (volatile S3C2450_IOPORT_REG*)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!v_pIOPregs)
	{
		DEBUGMSG(1, (TEXT("IOPreg: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("IOPreg: VirtualCopy failed!\r\n")));
		return(FALSE);
	}


	I2cInit();


	//init process
	// 1, IRESET set low
	#if 0
	v_pIOPregs->GPECON = (v_pIOPregs->GPECON & ~(0x3) )|(0x1);
	v_pIOPregs->GPEDAT &=(0x0);
	Sleep(500);
	v_pIOPregs->GPEDAT |=(0x1);
	Sleep(500);
    #endif

    g_AKM_GPIOInfo[0].u32Stat = 0;
    SetGPIOInfo(&g_AKM_GPIOInfo[0], (void*)v_pIOPregs);
    Sleep(500);
    g_AKM_GPIOInfo[0].u32Stat = 1;
    SetGPIOInfo(&g_AKM_GPIOInfo[0], (void*)v_pIOPregs);
    Sleep(500);

	RETAILMSG(1, (TEXT("AKM_Init end\r\n")));
	//RETAILMSG(1, (TEXT("===============IRESET set hign===========\r\n")));

#if 0
	// 2, SRESET bit 0, CONT8 register
	//default value is 0
	
	// 3, BICK(32fs)
	//supply by BT 
	
	// 4, Register writing
	AK7742_I2CWrite(0xc0, 0x86);
	AK7742_I2CWrite(0xc1, 0x20);
	AK7742_I2CWrite(0xc2, 0xe6);
	AK7742_I2CWrite(0xc3, 0xca);
	AK7742_I2CWrite(0xc4, 0x88);
	AK7742_I2CWrite(0xc5, 0x18);
	AK7742_I2CWrite(0xc6, 0x10);
	AK7742_I2CWrite(0xc7, 0x00);
	AK7742_I2CWrite(0xc8, 0x00);


	AK7742_I2CRead(0x40);
	AK7742_I2CRead(0x41);
	AK7742_I2CRead(0x42);
	AK7742_I2CRead(0x43);
	AK7742_I2CRead(0x44);
	AK7742_I2CRead(0x45);
	AK7742_I2CRead(0x46);
	AK7742_I2CRead(0x47);
	AK7742_I2CRead(0x48);



	// 5, PRAM writing
//	AK7742_I2CWriteNBytes(0xb8, AK7742_PRAM, sizeof(AK7742_PRAM));
#if 0 //no macro AK7742_PRAMX, why?? f.w.lin
	AK7742_I2CWriteNBytes(0xb8, AK7742_PRAM1, sizeof(AK7742_PRAM1));
	AK7742_I2CWriteNBytes(0xb8, AK7742_PRAM2, sizeof(AK7742_PRAM2));
	AK7742_I2CWriteNBytes(0xb8, AK7742_PRAM3, sizeof(AK7742_PRAM3));
	AK7742_I2CWriteNBytes(0xb8, AK7742_PRAM4, sizeof(AK7742_PRAM4));


	// 6, CRAM writing
	AK7742_I2CWriteNBytes(0xb4, AK7742_CRAM, sizeof(AK7742_CRAM));
#endif

	//7, SRESET bit set to 1
	AK7742_I2CWrite(0xc8, 0x80);
#endif	
#endif
	return TRUE;
}

DWORD AKM_Deinit()
{
	return TRUE;
}


DWORD AKM_Open()
{
	RETAILMSG(1, (TEXT("==AKM_Open==\r\n")));
	return TRUE;
}

DWORD AKM_Close()
{
	RETAILMSG(1, (TEXT("==AKM_Close==\r\n")));
	return TRUE;
}

DWORD AKM_Read()
{
	return TRUE;
}

DWORD AKM_Write()
{
	return TRUE;
}

DWORD AKM_PowerUp()
{
	return TRUE;
}

DWORD AKM_PowerDown()
{
	return TRUE;
}

DWORD AKM_Seek()
{
	return TRUE;
}

BOOL AKM_IOControl(DWORD  dwOpenData,
                   DWORD  dwCode,
                   PBYTE  pBufIn,
                   DWORD  dwLenIn,
                   PBYTE  pBufOut,
                   DWORD  dwLenOut,
                   PDWORD pdwActualOut)
{
    RETAILMSG(1, (TEXT("AKM_IOControl:%d \r\n"), dwCode) );

	switch (dwCode) {

		case WODM_BT_SCO_AUDIO_CONTROL:

			Sleep(1000);
			RETAILMSG(1, (TEXT("WODM_BT_SCO_AUDIO_CONTROL:Bluetooth==>>>>AK7742 \r\n")));
#if 0
			//init process
			// 1, IRESET set low
			v_pIOPregs->GPECON = (v_pIOPregs->GPECON & ~(0x3) )|(0x1);
			v_pIOPregs->GPEDAT &=(0x0);
			Sleep(500);
			v_pIOPregs->GPEDAT |=(0x1);
			Sleep(500);
#endif

#if 1
			// 2, SRESET bit 0, CONT8 register
			AK7742_I2CWrite(0xc8, 0x00);
			
			// 3, BICK(32fs)
			//supply by BT 

			// 4, Register writing
			//AK7742_I2CWrite(0xc0, 0x86);
			AK7742_I2CWrite(0xc0, 0xa6); //long frame ,rise edge
			AK7742_I2CWrite(0xc1, 0x20);
			AK7742_I2CWrite(0xc2, 0xe6);
			AK7742_I2CWrite(0xc3, 0xca);
			AK7742_I2CWrite(0xc4, 0x88);
			AK7742_I2CWrite(0xc5, 0x18);
			AK7742_I2CWrite(0xc6, 0x10);
			AK7742_I2CWrite(0xc7, 0x00);
			AK7742_I2CWrite(0xc8, 0x00);


			AK7742_I2CWrite(0xd0, 0x30);
			AK7742_I2CWrite(0xd1, 0x30);
			AK7742_I2CWrite(0xd2, 0x18);
			AK7742_I2CWrite(0xd3, 0x18);
			AK7742_I2CWrite(0xd4, 0x18);
			AK7742_I2CWrite(0xd5, 0x18);

			// 5, PRAM writing
			AK7742_I2CWriteNBytes(0xb8, AK7742_PRAM, sizeof(AK7742_PRAM));


			// 6, CRAM writing
			AK7742_I2CWriteNBytes(0xb4, AK7742_CRAM, sizeof(AK7742_CRAM));


			//7, SRESET bit set to 1
			AK7742_I2CWrite(0xc8, 0x80);

			//Read the REG
			AK7742_I2CRead(0x40);
			AK7742_I2CRead(0x41);
			AK7742_I2CRead(0x42);
			AK7742_I2CRead(0x43);
			AK7742_I2CRead(0x44);
			AK7742_I2CRead(0x45);
			AK7742_I2CRead(0x46);
			AK7742_I2CRead(0x47);
			AK7742_I2CRead(0x48);

            break;
#endif

        default:  //added by f.w.lin for test
            RETAILMSG(1, (TEXT("AKM_IOControl,default:%d \r\n"),dwCode));
            break;
	}	
	
	return TRUE;
}


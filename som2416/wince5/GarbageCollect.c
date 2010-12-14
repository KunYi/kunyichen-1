// testapp.cpp : Defines the entry point for the application.
//

#pragma optimize("", off)

#include <windows.h>
#include "WMRTypes.h"

//////////////////////////////////////////////////////////
// in HALWrapper.h"

#define PM_HAL_FTL_GARBAGECOLLECT	28

#define IOCTL_POCKETSTOREII_CMD	CTL_CODE(FILE_DEVICE_HAL, 4080, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    UINT    nCtrlCode;
    UINT    nLsn;
    UINT    nNumOfScts;
    UINT8  *pBuf;
    UINT32 *pTotalScts;
} FTLPacket;

//////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	FTLPacket   stPacket;
	UINT32      nResult;

	do {
		stPacket.nCtrlCode  = PM_HAL_FTL_GARBAGECOLLECT;

		stPacket.nLsn       = 0;
		stPacket.nNumOfScts = 0;
		stPacket.pBuf       = NULL;
		stPacket.pTotalScts = NULL;

		KernelIoControl(IOCTL_POCKETSTOREII_CMD,  /* Io Control Code */
			&stPacket,                /* Input buffer (Additional Control Code) */
			sizeof(FTLPacket),        /* Size of Input buffer */
			NULL,                     /* Output buffer */
			0,                        /* Size of Output buffer */
			&nResult);                /* Error Return */

		if (nResult != TRUE)  // 1 : TRUE,  0 : FALSE
		{
			RETAILMSG(1, (TEXT("KernelIoControl(IOCTL_POCKETSTOREII_CMD) failure.\r\n")));
			break;
		}
		else
		{
			RETAILMSG(1, (TEXT("KernelIoControl(IOCTL_POCKETSTOREII_CMD) success.\r\n")));
		}
	} while(0);

	return 0;
}

#pragma optimize("", on)

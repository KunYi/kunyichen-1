#include <bsp.h>

//------------------------------------------------------------------------------
//
//  Function:  OALArgsQuery
//
//  This function is called from other OAL modules to return boot arguments.
//  Boot arguments are typically placed in fixed memory location and they are
//  filled by boot loader. In case that boot arguments can't be located
//  the function should return NULL. The OAL module then must use default
//  values.
//


VOID* OALArgsQuery(UINT32 type)
{
    VOID *pData = NULL;
    BSP_ARGS *pArgs;
    UINT8 tmpUUID[16]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};

    OALMSG(OAL_ARGS&&OAL_FUNC, (L"+OALArgsQuery(%d)\r\n", type));

    // Get pointer to expected boot args location
    pArgs = (BSP_ARGS*)IMAGE_SHARE_ARGS_UA_START;

	if(type == OAL_ARGS_QUERY_UUID)
	{
		// Get your own uuid from here
		 //[david.modify] 2008-07-11 11:02
		 //将UUID设成和DEVICE ID一样的
		 //=========================
//pData = &tmpUUID;
        pData = &pArgs->deviceId;
		 //=========================			 
	}



    // Check if there is expected signature
    if (
        pArgs->header.signature  != OAL_ARGS_SIGNATURE ||
        pArgs->header.oalVersion != OAL_ARGS_VERSION   ||
        pArgs->header.bspVersion != BSP_ARGS_VERSION
    ) 
    {
    	RETAILMSG(1, (L"Goto Clean up.\r\n"));
    	goto cleanUp;
    }

    // Depending on required args
    switch (type) {
    case OAL_ARGS_QUERY_DEVID:
        pData = &pArgs->deviceId;
        break;
    case OAL_ARGS_QUERY_KITL:
        pData = &pArgs->kitl;
        break;
    case BSP_ARGS_QUERY_DBGSERIAL:
        if ( (pArgs->dbgSerPhysAddr != S3C2450_BASE_REG_PA_UART0) && (pArgs->dbgSerPhysAddr != S3C2450_BASE_REG_PA_UART1)
			&& (pArgs->dbgSerPhysAddr != S3C2450_BASE_REG_PA_UART2)&& (pArgs->dbgSerPhysAddr != S3C2450_BASE_REG_PA_UART3))
        {
		pArgs->dbgSerPhysAddr=S3C2450_BASE_REG_PA_NOUART;
		pData = NULL;
        }else {
	       pData = &pArgs->dbgSerPhysAddr;
	 }


        break;		
    }

cleanUp:
    OALMSG(OAL_ARGS&&OAL_FUNC, (L"-OALArgsQuery(pData = 0x%08x)\r\n", pData));
    return pData;
}

//------------------------------------------------------------------------------


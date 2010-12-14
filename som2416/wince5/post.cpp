// Ported for 2450 Win CE 5.0
//
#include <windows.h>

#include <nkintr.h>
//#include <oalintr.h>
#include <pm.h>
#include "pmplatform.h"
#include <ceddk.h>

#include <S3c2450.h>
#include <bsp.h>
#include "post.h"


#define PM_MSG 		0
#define POST_MSG	0
#define POST_DEBUG	0
#define RETAIL_ON		0

#define DOTNET_DRIVER 		1 // 0:PPC, 1:CE.NET

#define POST_CLOCK_OFF_ON_IDLE 0

#if (POST_CLOCK_OFF_ON_IDLE == 1)
#define POST_THREAD_TIMEOUT	3000
#else
#define POST_THREAD_TIMEOUT	INFINITE
#endif

#define	USE_RESERVED_BUFFER	0

#if (USE_RESERVED_BUFFER == 1)


volatile unsigned char* pVirtSrcAddr;// = (volatile unsigned char*)((DWORD)POST_INPUT_BUFFER + VIRTUAL_ADDR_OFFSET);
volatile unsigned char* pVirtDstAddr;// = (volatile unsigned char*)((DWORD)POST_OUTPUT_BUFFER + VIRTUAL_ADDR_OFFSET);
#else
PHYSICAL_ADDRESS g_PhysSrcAddr;
PHYSICAL_ADDRESS g_PhysDstAddr;
PBYTE pVirtSrcAddr = NULL;
PBYTE pVirtDstAddr = NULL;
#endif

volatile S3C2450_IOPORT_REG 	*s2450IOP = (S3C2450_IOPORT_REG *)S3C2450_BASE_REG_PA_IOPORT;
volatile S3C2450_CAM_REG 	*s2450CAM = (S3C2450_CAM_REG *)S3C2450_BASE_REG_PA_CAM;
volatile S3C2450_INTR_REG 	*s2450INT = (S3C2450_INTR_REG *)S3C2450_BASE_REG_PA_INTR;
volatile S3C2450_CLKPWR_REG	*s2450PWR = (S3C2450_CLKPWR_REG *)S3C2450_BASE_REG_PA_CLOCK_POWER;

volatile U8 *PostInputBuffer = NULL;
volatile U8 *PostOutputBuffer = NULL;

unsigned char *Post_input_yuv;
unsigned char *Post_output_rgb;

unsigned int dwPostProcessTimeout = POST_THREAD_TIMEOUT;

BOOL bIdlePwrDown = FALSE;

BOOL bAppOpenFlag = FALSE;

HANDLE PostThread;
HANDLE PostEvent;
HANDLE PostDoneEvent;

// Added yash
//
DWORD gIntrPost 		= SYSINTR_NOP;

static U32 ADDRStartRGB, ADDREndRGB ,ADDRStartY, ADDREndY;
static U32 ADDRStartCb, ADDREndCb, ADDRStartCr, ADDREndCr;


void Virtual_Alloc();						// Virtual allocation
void Display_POST_Image(U32 pos_x, U32 pos_y, U32 size_x, U32 size_y);


BOOL POSTClockOn(BOOL bOnOff);
void PostInit(POSTINFO *pBufIn);
void PostProcessOn(U8 *pBufIn, U8 *pBufOut);
void PostProcessDone(U8 *pBufIn, U8 *pBufOut);
void Post_CalculatePrescaler(U32 SrcSize, U32 DstSize, U32 *ratio,U32 *shift);

DWORD PostProcessThread(void);
BOOL InitInterruptThread();

CEDEVICE_POWER_STATE m_Dx;

#ifdef DEBUG

    #define DBG_INIT    0x0001
    #define DBG_OPEN    0x0002
    #define DBG_READ    0x0004
    #define DBG_WRITE   0x0008
    #define DBG_CLOSE   0x0010
    #define DBG_IOCTL   0x0020
    #define DBG_THREAD  0x0040
    #define DBG_EVENTS  0x0080
    #define DBG_CRITSEC 0x0100
    #define DBG_FLOW    0x0200
    #define DBG_IR      0x0400
    #define DBG_NOTHING 0x0800
    #define DBG_ALLOC   0x1000
    #define DBG_FUNCTION 0x2000
    #define DBG_WARNING 0x4000
    #define DBG_ERROR   0x8000

DBGPARAM dpCurSettings = {
    TEXT("postdriver"), {
        TEXT("Init"),TEXT("Open"),TEXT("Read"),TEXT("Write"),
        TEXT("Close"),TEXT("Ioctl"),TEXT("Thread"),TEXT("Events"),
        TEXT("CritSec"),TEXT("FlowCtrl"),TEXT("Infrared"),TEXT("User Read"),
        TEXT("Alloc"),TEXT("Function"),TEXT("Warning"),TEXT("Error")},
    0
}; 

#define ZONE_INIT		DEBUGZONE(0)
#define ZONE_OPEN		DEBUGZONE(1)
#define ZONE_READ		DEBUGZONE(2)
#define ZONE_WRITE		DEBUGZONE(3)
#define ZONE_CLOSE		DEBUGZONE(4)
#define ZONE_IOCTL		DEBUGZONE(5)
#define ZONE_THREAD	DEBUGZONE(6)
#define ZONE_EVENTS		DEBUGZONE(7)
#define ZONE_CRITSEC	DEBUGZONE(8)
#define ZONE_FLOW		DEBUGZONE(9)
#define ZONE_IR			DEBUGZONE(10)
#define ZONE_USR_READ	DEBUGZONE(11)
#define ZONE_ALLOC		DEBUGZONE(12)
#define ZONE_FUNCTION	DEBUGZONE(13)
#define ZONE_WARN		DEBUGZONE(14)
#define ZONE_ERROR		DEBUGZONE(15)


#endif //DEBUG

POSTINFO sPOSTINFO;

CRITICAL_SECTION m_Lock;

void Lock()   {EnterCriticalSection(&m_Lock);}
void Unlock() {LeaveCriticalSection(&m_Lock);}


static void Delay(USHORT count)
{
	volatile int i, j = 0;
	volatile static int loop = S3C2450_FCLK/100000;
	
	for(;count > 0;count--)
		for(i=0;i < loop; i++) { j++; }
}

void PostProcessor_PowerDown(void)
{
	RETAILMSG(PM_MSG, (_T("PostProcessor_PowerDown()\r\n")));

	// Post Processor clock off
	PostClockOn(FALSE);
	RETAILMSG(POST_MSG, (_T("POST CLK_DOWN\r\n")));
}

void PostProcessor_PowerUp(void)
{
	RETAILMSG(PM_MSG, (_T("PostProcessor_PowerUp()\r\n")));

	// Post Processor clock on
	PostClockOn(TRUE);

	RETAILMSG(POST_MSG,(_T("POST CLK_UP\r\n")));	
}


DWORD PostProcessThread(void)
{

	DWORD	dwCause;					

	SetProcPermissions((DWORD)-1);


	
	while(TRUE)
	{
		RETAILMSG(PM_MSG, (TEXT("Before WaitForSingleObject(PostEvent,\r\n")));
		dwCause = WaitForSingleObject(PostEvent, dwPostProcessTimeout);
		RETAILMSG(PM_MSG, (TEXT("After WaitForSingleObject(PostEvent,\r\n")));		
		
		if (dwCause == WAIT_OBJECT_0) 
		{
			Lock();

			__try
			{
				RETAILMSG(PM_MSG, (TEXT("Before SetEvent(PostDoneEvent)\r\n")));
				SetEvent(PostDoneEvent);
				RETAILMSG(PM_MSG, (TEXT("After SetEvent(PostDoneEvent)\r\n")));				
				InterruptDone(gIntrPost);
				RETAILMSG(PM_MSG, (TEXT("After InterruptDone(gIntrPost)\r\n")));				
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				RETAILMSG(PM_MSG, (TEXT("[POST] InterruptThread() - EXCEPTION: %d"), GetExceptionCode()));
			}

			Unlock();
		}

		else if (dwCause == WAIT_TIMEOUT)
		{
			RETAILMSG(PM_MSG, (TEXT("WAIT_TIMEOUT\n")));
			//RETAILMSG(1, (TEXT("IOCTL_POST_RUN: %d"), IOCTL_POST_RUN));
			//RETAILMSG(1, (TEXT("IOCTL_POST_STOP: %d"), IOCTL_POST_STOP));
			Lock();
			
			//RETAILMSG(PM_MSG,(_T("[POST] InterruptThread Timeout : %d msec\r\n"), dwPostProcessTimeout));
			
			dwPostProcessTimeout = INFINITE;				// reset timeout until Post Interrupt occurs

			bIdlePwrDown = TRUE;					// Post is off

			PostProcessor_PowerDown();
			RETAILMSG(PM_MSG, (TEXT("[POST]Post Processor PowerDown Mode\r\n")));

			Unlock();
		}

		else
		{
			RETAILMSG(PM_MSG, (TEXT("[POST] InterruptThread : Exit %d, Cause %d\r\n"), GetLastError(), dwCause));
		}
	}

	return 0;
}



void Virtual_Alloc()
{
    DMA_ADAPTER_OBJECT Adapter1, Adapter2;

    memset(&Adapter1, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter1.InterfaceType = Internal;
    Adapter1.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    memset(&Adapter2, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter2.InterfaceType = Internal;
    Adapter2.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);


    // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
    //
#if USE_RESERVED_BUFFER==1
    // GPIO Virtual alloc
	pVirtSrcAddr = (volatile unsigned char*) VirtualAlloc(0,0x100000,MEM_RESERVE, PAGE_NOACCESS);
	if(pVirtSrcAddr == NULL) {
		RETAILMSG(1,(TEXT("For pVirtSrcAddr: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)pVirtSrcAddr,(PVOID)(POST_INPUT_BUFFER>>8),0x100000,PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)) {
			RETAILMSG(1,(TEXT("For pVirtSrcAddr: VirtualCopy failed!\r\n")));
		}
	}
	pVirtDstAddr = (volatile unsigned char*) VirtualAlloc(0,0x100000,MEM_RESERVE, PAGE_NOACCESS);
	if(pVirtDstAddr == NULL) {
		RETAILMSG(1,(TEXT("For pVirtSrcAddr: pVirtDstAddr failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)pVirtDstAddr,(PVOID)(POST_OUTPUT_BUFFER>>8),0x100000,PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)) {
			RETAILMSG(1,(TEXT("For pVirtDstAddr: VirtualCopy failed!\r\n")));
		}
	}	
#else
    pVirtSrcAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter1, 0x71000, &g_PhysSrcAddr, FALSE);
    if (pVirtSrcAddr == NULL)
    {
        RETAILMSG(TRUE, (TEXT("Camera:Virtual_Alloc() - Failed to allocate DMA buffer for Source.\r\n")));
    }

    pVirtDstAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter2, 0x96000, &g_PhysDstAddr, FALSE);
    if (pVirtDstAddr == NULL)
    {
        RETAILMSG(TRUE, (TEXT("Camera:Virtual_Alloc() - Failed to allocate DMA buffer for Destination.\r\n")));
    }
#endif
	
    // GPIO Virtual alloc
	s2450IOP = (volatile S3C2450_IOPORT_REG*) VirtualAlloc(0,sizeof(S3C2450_IOPORT_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(s2450IOP == NULL) {
		RETAILMSG(1,(TEXT("For s2450IOP: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)s2450IOP,(PVOID)(S3C2450_BASE_REG_PA_IOPORT>>8),sizeof(S3C2450_IOPORT_REG),PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)) {
			RETAILMSG(1,(TEXT("For s2450IOP: VirtualCopy failed!\r\n")));
		}
	}
	
	// Interrupt Virtual alloc
	s2450CAM = (volatile S3C2450_CAM_REG *) VirtualAlloc(0,sizeof(S3C2450_CAM_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(s2450INT == NULL) {
		RETAILMSG(1,(TEXT("For s2450CAM: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)s2450CAM,(PVOID)(S3C2450_BASE_REG_PA_CAM>>8),sizeof(S3C2450_CAM_REG),PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)) {
			RETAILMSG(1,(TEXT("For s2450CAM: VirtualCopy failed!\r\n")));
		}
	}
	// Interrupt Virtual alloc
	s2450INT = (volatile S3C2450_INTR_REG *) VirtualAlloc(0,sizeof(S3C2450_INTR_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(s2450INT == NULL) {
		RETAILMSG(1,(TEXT("For s2450INT: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)s2450INT,(PVOID)(S3C2450_BASE_REG_PA_INTR>>8),sizeof(S3C2450_INTR_REG),PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)) {
			RETAILMSG(1,(TEXT("For s2450INT: VirtualCopy failed!\r\n")));
		}
	}

	// PWM clock Virtual alloc
	s2450PWR = (volatile S3C2450_CLKPWR_REG*) VirtualAlloc(0,sizeof(S3C2450_CLKPWR_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(s2450PWR == NULL) {
		RETAILMSG(1,(TEXT("For s2450PWR: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)s2450PWR,(PVOID)(S3C2450_BASE_REG_PA_CLOCK_POWER>>8),sizeof(S3C2450_CLKPWR_REG),PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)) {
			RETAILMSG(1,(TEXT("For s2450PWR: VirtualCopy failed!\r\n")));
		}
	}

}

void Display_POST_Image(U32 pos_x, U32 pos_y, U32 size_x, U32 size_y)
{
	U8 *buffer_rgb;
	U32 y;
	
	RETAILMSG(POST_MSG,(_T("Display_POST_Image()\r\n")));

			buffer_rgb = (U8 *)PostOutputBuffer;
	
#if (DOTNET_DRIVER)
	SetKMode(TRUE);
#endif

	RETAILMSG(POST_MSG,(_T("buffer_rgb = 0x%x\r\n"), buffer_rgb));
	


	for (y=0; y<size_y; y++) // YCbCr 4:2:0 format
	{
		//memcpy((void *)(FRAMEBUF_BASE+0x5e00+y*240*2),(void *)buffer_rgb,(QCIF_XSIZE)*2);
		memcpy((void *)(IMAGE_FRAMEBUFFER_UA_BASE+ (240*pos_y + pos_x) + y*240*2),(void *)buffer_rgb,size_x*2);
		buffer_rgb += (size_x*2);
	}

#if (DOTNET_DRIVER)
	SetKMode(FALSE);
#endif

}



BOOL WINAPI  
DllEntry(HANDLE	hinstDLL, 
			DWORD dwReason, 
			LPVOID /* lpvReserved */)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DEBUGREGISTER((HINSTANCE)hinstDLL);
		DEBUGMSG(ZONE_INIT,(TEXT("POST: DLL_PROCESS_ATTACH\r\n")));
		return TRUE;
	case DLL_THREAD_ATTACH:
		DEBUGMSG(ZONE_THREAD,(TEXT("POST: DLL_THREAD_ATTACH\r\n")));
		break;
	case DLL_THREAD_DETACH:
		DEBUGMSG(ZONE_THREAD,(TEXT("POST: DLL_THREAD_DETACH\r\n")));
		break;
	case DLL_PROCESS_DETACH:
		DEBUGMSG(ZONE_INIT,(TEXT("POST: DLL_PROCESS_DETACH\r\n")));
		break;
#ifdef UNDER_CE
	case DLL_PROCESS_EXITING:
		DEBUGMSG(ZONE_INIT,(TEXT("POST: DLL_PROCESS_EXITING\r\n")));
		break;
	case DLL_SYSTEM_STARTED:
		DEBUGMSG(ZONE_INIT,(TEXT("POST: DLL_SYSTEM_STARTED\r\n")));
		break;
#endif
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL PST_Deinit(DWORD hDeviceContext)
{
    DMA_ADAPTER_OBJECT Adapter1, Adapter2;

    memset(&Adapter1, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter1.InterfaceType = Internal;
    Adapter1.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    memset(&Adapter2, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter2.InterfaceType = Internal;
    Adapter2.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    	
	BOOL bRet = TRUE;
	
	RETAILMSG(PM_MSG,(TEXT("POST: PST_Deinit\r\n")));
	
	CloseHandle(PostThread);
	
	VirtualFree((void*)s2450IOP, 0, MEM_RELEASE);
	VirtualFree((void*)s2450CAM, 0, MEM_RELEASE);	
	VirtualFree((void*)s2450INT, 0, MEM_RELEASE);
	VirtualFree((void*)s2450PWR, 0, MEM_RELEASE);

#if USE_RESERVED_BUFFER == 1
	VirtualFree((void*)pVirtSrcAddr, 0, MEM_RELEASE);
	VirtualFree((void*)pVirtDstAddr, 0, MEM_RELEASE);

#else
	VirtualFree((void*)PostInputBuffer, 0,MEM_RELEASE);
	VirtualFree((void*)PostOutputBuffer,0,MEM_RELEASE);
    HalFreeCommonBuffer(&Adapter1, 0x71000, g_PhysSrcAddr, (PVOID)pVirtSrcAddr, FALSE);

    HalFreeCommonBuffer(&Adapter2, 0x96000, g_PhysDstAddr, (PVOID)pVirtDstAddr, FALSE);
#endif 

	return TRUE;
} 



BOOL InitInterruptThread()
{
	DWORD         threadID;                         // thread ID
	BOOL bSuccess;
	UINT32 Irq;

    PostEvent = CreateEvent(NULL, FALSE, FALSE, NULL);    //
    PostDoneEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    
    if (!PostEvent||!PostDoneEvent)
    {
    	return FALSE;
    }

	// Obtain sysintr values from the OAL for the CAMIF interrupt.
	//
	Irq = IRQ_CAM_P;
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &Irq, sizeof(UINT32), &gIntrPost, sizeof(UINT32), NULL))
	{
		RETAILMSG(RETAIL_ON, (TEXT("ERROR: Failed to request the post sysintr.\r\n")));
		gIntrPost = SYSINTR_UNDEFINED;
		return(FALSE);
	}

	RETAILMSG(RETAIL_ON, (TEXT("POST Interrupt is ....... %d \r\n"), gIntrPost));

	bSuccess = InterruptInitialize(gIntrPost, PostEvent, NULL, 0);
	RETAILMSG(RETAIL_ON, (TEXT("After InterruptInitialize\r\n")));
    if (!bSuccess) 
    {
        RETAILMSG(1,(TEXT("Fail to initialize Post interrupt event\r\n")));
        return FALSE;
    }    
	RETAILMSG(RETAIL_ON, (TEXT("PostThread = CreateThread\r\n")));
    PostThread = CreateThread(NULL,
                                 0,
                                 (LPTHREAD_START_ROUTINE)PostProcessThread,
                                 0,
                                 0,
                                 &threadID);
    
    if (NULL == PostThread ) {
    	RETAILMSG(1,(TEXT("Create Post Thread Fail\r\n")));
    }
	
	RETAILMSG(PM_MSG,(_T("POST.DLL::InterruptThread Initialized.\r\n")));
	return TRUE;
}


BOOL PostClockOn(BOOL bOnOff)
{
	// Post Processor clock
	if (!bOnOff)
		s2450PWR->HCLKCON &= ~(1<<8); // Post clock disable
	else 
		s2450PWR->HCLKCON |= (1<<8); // Post clock enable

	RETAILMSG(POST_MSG,(_T("PostClockOn = %d\r\n"), bOnOff));
	
	Delay(500);

	return TRUE;
}


DWORD PST_Init(DWORD dwContext)
{

	// 1. Virtual Alloc
	Virtual_Alloc();
 	RETAILMSG(POST_DEBUG, (TEXT("PST::Virtual_Alloc \r\n")));	
      
	PostClockOn(TRUE);
	
	if (!InitInterruptThread())
	{
        RETAILMSG(POST_DEBUG,(TEXT("Fail to initialize Post interrupt event\r\n")));
        return FALSE;
    }    

	m_Dx = (_CEDEVICE_POWER_STATE)D0;
	DevicePowerNotify(_T("PST1:"),(_CEDEVICE_POWER_STATE)D0, POWER_NAME);
       RETAILMSG(POST_MSG, (TEXT("PST::POST_Init() \r\n")));
	  return TRUE;
	
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL PST_IOControl(DWORD hOpenContext, 
				   DWORD dwCode, 
				   PBYTE pBufIn, 
				   DWORD dwLenIn, 
				   PBYTE pBufOut, 
				   DWORD dwLenOut, 
				   PDWORD pdwActualOut)
{
    BOOL RetVal = TRUE;
    DWORD dwErr = ERROR_SUCCESS;
    PPVOID	temp;
	static unsigned int time=0,old_time=0;

	switch (dwCode)
	{

//-----------------------------------------------------------------------------------------
		case IOCTL_POWER_CAPABILITIES: 
        {
            PPOWER_CAPABILITIES ppc;
			RETAILMSG(PM_MSG, (TEXT("PST: IOCTL_POWER_CAPABILITIES\r\n")));   
            
			if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(POWER_CAPABILITIES)) ) {
                RetVal = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
			
            ppc = (PPOWER_CAPABILITIES)pBufOut;
            
            memset(ppc, 0, sizeof(POWER_CAPABILITIES));

            // support D0, D4 
            ppc->DeviceDx = 0x11;

            // Report our power consumption in uAmps rather than mWatts. 
            ppc->Flags = POWER_CAP_PREFIX_MICRO | POWER_CAP_UNIT_AMPS;
            
			// 25 m = 25000 uA
            // TODO: find out a more accurate value
			ppc->Power[D0] = 15000;
            
            *pdwActualOut = sizeof(POWER_CAPABILITIES);
        } break;

		case IOCTL_POWER_SET: 
        {
            CEDEVICE_POWER_STATE NewDx;
            
            RETAILMSG(PM_MSG,(_T("==================IOCTL_POWER_SET======================\r\n")));

            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
                RetVal = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            
            NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;

            if ( VALID_DX(NewDx) ) {
                switch ( NewDx ) {
                case D0:
                    if (m_Dx != D0) {
                        PST_PowerUp(hOpenContext);
                        m_Dx = D0;
                    }
                    break;

                default:
                    if (m_Dx != (_CEDEVICE_POWER_STATE)D4) {
                        PST_PowerDown(hOpenContext);
                        m_Dx = (_CEDEVICE_POWER_STATE)D4;
                    }
                    break;
                }

                // return our state
                *(PCEDEVICE_POWER_STATE)pBufOut = m_Dx;

                RETAILMSG(PM_MSG, (TEXT("POST: IOCTL_POWER_SET: D%u \r\n"), NewDx));

                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            } else {
                RetVal = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
            }
            
        } break;

        case IOCTL_POWER_GET: 

            if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
                RetVal = FALSE;
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

			*(PCEDEVICE_POWER_STATE)pBufOut = m_Dx;

            RETAILMSG(PM_MSG, (TEXT("POST: IOCTL_POWER_GET: D%u \r\n"), m_Dx));

            *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
	        break;

//-----------------------------------------------------------------------------------------
		case IOCTL_POST_INIT :
			if (bIdlePwrDown == TRUE)
			{
				RETAILMSG(POST_DEBUG, (_T("[POST] IOControl POST_INIT(%x)\r\n"),dwLenIn));

				PostProcessor_PowerUp();
	
				bIdlePwrDown = FALSE;
			}
			dwPostProcessTimeout = POST_THREAD_TIMEOUT;
			PostInit((POSTINFO*)pBufIn);
			
 			break;

		case IOCTL_POST_RUN:
			s2450INT->INTMSK1 &= ~(1<<IRQ_CAM);
			s2450INT->INTSUBMSK &= ~(1<<IRQ_SUB_CAM_P);		
			PostProcessOn(pBufIn, pBufOut);
			RETAILMSG(POST_DEBUG,(TEXT("POST:IOCTL_POST_RUN\r\n")));
			break;

		case IOCTL_POST_STOP : 
			RETAILMSG(POST_DEBUG,(TEXT("POST:IOCTL_POST_STOP\r\n")));

			// Disable POST interrupt
			s2450INT->INTMSK1 |= (1<<IRQ_CAM);
			s2450INT->INTSUBMSK |= (1<<IRQ_SUB_CAM_P);
			
			if(s2450INT->SUBSRCPND & (1<<IRQ_SUB_CAM_P)) s2450INT->SUBSRCPND  = (1<<IRQ_SUB_CAM_P);
			if(s2450INT->SRCPND1 & (1<<IRQ_CAM)) s2450INT->SRCPND1  = (1<<IRQ_CAM);
			if(s2450INT->INTPND1 & (1<<IRQ_CAM)) s2450INT->INTPND1 = (1<<IRQ_CAM);

			break;
		case IOCTL_POST_GETINPUTBUFFER:
			temp = (PPVOID)pBufOut;
			*temp = (PVOID)(pVirtSrcAddr);
			RETAILMSG(1,(TEXT("0x%08X\n"), (BYTE*)(pVirtSrcAddr)));
			break;
		case IOCTL_POST_GETOUTPUTBUFFER:
			temp = (PPVOID)pBufOut;
			*temp = (PVOID)(pVirtDstAddr);
			RETAILMSG(1,(TEXT("0x%08X\n"), (BYTE*)(pVirtDstAddr)));		
			break;					
		default : 
			RETAILMSG(POST_MSG,(TEXT("POST:Ioctl code = 0x%x\r\n"), dwCode));
			return FALSE;

	}
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD PST_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{

	RETAILMSG(PM_MSG,(TEXT("POST: PST_Open\r\n")));
	bAppOpenFlag = TRUE;

	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL PST_Close(DWORD hOpenContext)
{
	RETAILMSG(POST_MSG,(TEXT("POST: PST_Close\r\n")));
	bAppOpenFlag = FALSE;
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void PST_PowerDown(DWORD hDeviceContext)
{
	RETAILMSG(POST_MSG,(TEXT("POST: PST_PowerDown\r\n")));

	m_Dx = (_CEDEVICE_POWER_STATE)D4;

	bIdlePwrDown = TRUE;
	PostProcessor_PowerDown();

} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void PST_PowerUp(DWORD hDeviceContext)
{
	RETAILMSG(POST_MSG,(TEXT("POST: POST_PowerUp\r\n")));
	
	m_Dx = (_CEDEVICE_POWER_STATE)D0;

	if(bAppOpenFlag == FALSE)
 	PostClockOn(FALSE);
	else
	PostProcessor_PowerUp();

	RETAILMSG(POST_MSG,(TEXT("POST: PST_PowerUp, m_Dx = D%u\r\n"), m_Dx));
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD PST_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	RETAILMSG(POST_MSG,(TEXT("POST: PST_Read\r\n")));
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD PST_Seek(DWORD hOpenContext, long Amount, DWORD Type)
{
	RETAILMSG(POST_MSG,(TEXT("POST: PST_Seek\r\n")));
	return 0;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD PST_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes)
{
	RETAILMSG(POST_MSG,(TEXT("POST: PST_Write\r\n")));
	return 0;
}

void Post_CalculatePrescaler(U32 SrcSize, U32 DstSize, U32 *ratio,U32 *shift)
{

	RETAILMSG(POST_MSG, (TEXT("[POST] Caculate Prescaler..\r\n")));
	if (SrcSize>=64*DstSize)
	{
		RETAILMSG(POST_MSG,(TEXT("ERROR: out of the prescaler range: SrcSize/DstSize = %d(< 64)\n"),SrcSize/DstSize));
		while(1);
	}
	else if (SrcSize>=32*DstSize)
	{
		*ratio = 32;
		*shift = 5;
	}
	else if (SrcSize>=16*DstSize)
	{
		*ratio = 16;
		*shift = 4;
	}
	else if (SrcSize>=8*DstSize)
	{
		*ratio = 8;
		*shift = 3;
	}
	else if (SrcSize>=4*DstSize)
	{
		*ratio = 4;
		*shift = 2;
	}
	else if (SrcSize>=2*DstSize)
	{
		*ratio = 2;
		*shift = 1;
	}
	else
	{
		*ratio = 1;
		*shift = 0;
	}
}

void CalculateBurstSize(unsigned int hSize,unsigned int *mainBurstSize,unsigned int *remainedBurstSize)
{
	unsigned int tmp;	
	tmp=(hSize/4)%16;
	switch(tmp) {
		case 0:
			*mainBurstSize=16;
			*remainedBurstSize=16;
			break;
		case 4:
			*mainBurstSize=16;
			*remainedBurstSize=4;
			break;
		case 8:
			*mainBurstSize=16;
			*remainedBurstSize=8;
			break;
		default: 
			tmp=(hSize/4)%8;
			switch(tmp) {
				case 0:
					*mainBurstSize=8;
					*remainedBurstSize=8;
					break;
				case 4:
					*mainBurstSize=8;
					*remainedBurstSize=4;
				default:
					*mainBurstSize=4;
					tmp=(hSize/4)%4;
					*remainedBurstSize= (tmp) ? tmp: 4;
					break;
			}
			break;
	}		    	    		
}

void PostInit(POSTINFO *pBufIn)
{
	U32 ScaleUp_H_Pr, ScaleUp_V_Pr,MainHorRatio, MainVerRatio;	
	U32 H_Shift, V_Shift, PreHorRatio, PreVerRatio;
	U32 MainBurstSizeRGB, RemainedBurstSizeRGB;
//	U32 WinOfsEn=0;
//	U32 WinHorOffset1=0,WinVerOffset1=0,WinHorOffset2=0,WinVerOffset2=0;
	U32 inmultiplier, outmultiplier;
	U32 OffsetY, OffsetC;	
	U32 ADDRStartY, ADDREndY, ADDRStartCb, ADDREndCb, ADDRStartCr, ADDREndCr;

	memset(&sPOSTINFO, 0, sizeof(sPOSTINFO));
	memcpy(&sPOSTINFO, pBufIn, sizeof(sPOSTINFO));

	RETAILMSG(POST_DEBUG,(TEXT("[POST] Post Processor Init for Run!!\r\n")));
#if USE_RESERVED_BUFFER==1
	Post_input_yuv = (unsigned char *)POST_INPUT_BUFFER;
	Post_output_rgb = (unsigned char *)POST_OUTPUT_BUFFER;
#else
	Post_input_yuv = (unsigned char *)g_PhysSrcAddr.LowPart;
	Post_output_rgb = (unsigned char *)g_PhysDstAddr.LowPart;
#endif
/*
	if(sPOSTINFO.nSrcWidth != sPOSTINFO.nOrgSrcWidth || sPOSTINFO.nSrcHeight != sPOSTINFO.nOrgSrcHeight)
	{
		WinOfsEn=1;
		WinHorOffset1 = sPOSTINFO.nSrcStartX;
		WinVerOffset1 = sPOSTINFO.nSrcStartY;
		WinHorOffset2 =	sPOSTINFO.nOrgSrcWidth - WinHorOffset1 - sPOSTINFO.nSrcWidth;
		WinVerOffset2 =	sPOSTINFO.nOrgSrcHeight - WinVerOffset1 - sPOSTINFO.nSrcHeight;
	}



	s2450CAM->CIWDOFST = (1<<30)|(0xf<<12); // clear overflow 
	s2450CAM->CIWDOFST = 0;	
	s2450CAM->CIWDOFST=(WinOfsEn<<31)|(WinHorOffset1<<16)|(WinVerOffset1);
	s2450CAM->CIDOWSFT2=(WinHorOffset2<<16)|(WinVerOffset2);*/
	
	
	if(sPOSTINFO.nSrcWidth >= sPOSTINFO.nDestWidth) ScaleUp_H_Pr=0; //down
		else ScaleUp_H_Pr=1;		//up

	if(sPOSTINFO.nSrcHeight >= sPOSTINFO.nDestHeight ) ScaleUp_V_Pr=0;
		else ScaleUp_V_Pr=1;		
	
	Post_CalculatePrescaler(sPOSTINFO.nSrcWidth, sPOSTINFO.nDestWidth, &PreHorRatio, &H_Shift);
	Post_CalculatePrescaler(sPOSTINFO.nSrcHeight, sPOSTINFO.nDestHeight, &PreVerRatio, &V_Shift);
				
	MainHorRatio=(sPOSTINFO.nSrcWidth<<8)/(sPOSTINFO.nDestWidth<<H_Shift);
	MainVerRatio=(sPOSTINFO.nSrcHeight<<8)/(sPOSTINFO.nDestHeight<<V_Shift);		

	CalculateBurstSize(sPOSTINFO.nDestWidth*2, &MainBurstSizeRGB, &RemainedBurstSizeRGB);		
	

	s2450CAM->CIPRTRGFMT=(2<<30)|(sPOSTINFO.nDestWidth<<16)|(sPOSTINFO.nDestHeight);		
	s2450CAM->CIPRCTRL=(MainBurstSizeRGB<<19)|(RemainedBurstSizeRGB<<14);
	s2450CAM->CIPRSCPRERATIO=((10-H_Shift-V_Shift)<<28)|(PreHorRatio<<16)|(PreVerRatio);		 
	s2450CAM->CIPRSCPREDST=((sPOSTINFO.nSrcWidth/PreHorRatio)<<16)|(sPOSTINFO.nSrcHeight/PreVerRatio);
	s2450CAM->CIPRSCCTRL=(1<<31)|(0/*16bit RGB*/<<30)|(ScaleUp_H_Pr<<29)|(ScaleUp_V_Pr<<28)|(MainHorRatio<<16)|(1<<15)|(MainVerRatio);

	s2450CAM->CIPRCLRSA1=(U32)(Post_output_rgb/*g_PhysDstAddr.LowPart*/);
	s2450CAM->CIPRCLRSA2=(U32)(Post_output_rgb/*g_PhysDstAddr.LowPart*/);
	s2450CAM->CIPRCLRSA3=(U32)(Post_output_rgb/*g_PhysDstAddr.LowPart*/);
	s2450CAM->CIPRCLRSA4=(U32)(Post_output_rgb/*g_PhysDstAddr.LowPart*/);
	s2450CAM->CIPRTAREA = sPOSTINFO.nDestWidth*sPOSTINFO.nDestHeight;

	inmultiplier=1;	
	outmultiplier=2;
	OffsetY = (sPOSTINFO.nOrgSrcWidth-sPOSTINFO.nSrcWidth)*inmultiplier;
	OffsetC = (sPOSTINFO.nOrgSrcWidth-sPOSTINFO.nSrcWidth)*inmultiplier/2;

	ADDRStartY = (U32)(Post_input_yuv)+(sPOSTINFO.nOrgSrcWidth*sPOSTINFO.nSrcStartY+sPOSTINFO.nSrcStartX)*inmultiplier;
	ADDREndY = ADDRStartY+sPOSTINFO.nSrcWidth*sPOSTINFO.nSrcHeight*inmultiplier + OffsetY*(sPOSTINFO.nSrcHeight-1);
	ADDRStartCb = (U32)(Post_input_yuv)+(sPOSTINFO.nOrgSrcWidth*sPOSTINFO.nOrgSrcHeight)*inmultiplier+(sPOSTINFO.nOrgSrcWidth/2*sPOSTINFO.nSrcStartY/2+sPOSTINFO.nSrcStartX/2)*inmultiplier;
	ADDREndCb = ADDRStartCb+(sPOSTINFO.nSrcWidth/2*sPOSTINFO.nSrcHeight/2)*inmultiplier+OffsetC*(sPOSTINFO.nSrcHeight/2-1);
	ADDRStartCr = (U32)(Post_input_yuv)+(sPOSTINFO.nOrgSrcWidth*sPOSTINFO.nOrgSrcHeight+(sPOSTINFO.nOrgSrcWidth/2*sPOSTINFO.nOrgSrcHeight/2))*inmultiplier+(sPOSTINFO.nOrgSrcWidth/2*sPOSTINFO.nSrcStartY/2+sPOSTINFO.nSrcStartX/2)*inmultiplier;
	ADDREndCr = ADDRStartCr+(sPOSTINFO.nSrcWidth/2*sPOSTINFO.nSrcHeight/2)*inmultiplier+OffsetC*(sPOSTINFO.nSrcHeight/2-1);

	s2450CAM->CIMSYSA = ADDRStartY;
	s2450CAM->CIMSCBSA = ADDRStartCb; 
	s2450CAM->CIMSCRSA = ADDRStartCr; 
	s2450CAM->CIMSYEND = ADDREndY;
	s2450CAM->CIMSCBEND= ADDREndCb;
	s2450CAM->CIMSCREND= ADDREndCr;
	
	s2450CAM->CIMSYOFF = OffsetY;
	s2450CAM->CIMSCBOFF = OffsetC;
	s2450CAM->CIMSCROFF = OffsetC;
	s2450CAM->CIMSWIDTH = sPOSTINFO.nSrcWidth;

	
	
}

#if USE_RESERVED_BUFFER == 1
void PostProcessOn(U8 *pBufIn, U8 *pBufOut)
{

	s2450CAM->CIMSCTRL &= ~(0x3f);
	s2450CAM->CIMSCTRL |= (0<<5|1<<2|1<<1);			
	s2450CAM->CIPRSCCTRL |=(1<<15);
	s2450CAM->CIIMGCPT |=(1<<31)|(1<<29);		
	s2450CAM->CIMSCTRL |= (1<<0);
	

	WaitForSingleObject(PostDoneEvent, INFINITE);

}
#else
void PostProcessOn(U8 *pBufIn, U8 *pBufOut)
{

	memcpy((BYTE*)(g_PhysSrcAddr.LowPart + VIRTUAL_ADDR_OFFSET),  pBufIn, sPOSTINFO.nOrgSrcWidth*sPOSTINFO.nOrgSrcHeight*3/2);
	
	
	s2450CAM->CIMSCTRL &= ~(0x3f);
	s2450CAM->CIMSCTRL |= (0<<5|1<<2|1<<1);			
	s2450CAM->CIPRSCCTRL |=(1<<15);
	s2450CAM->CIIMGCPT |=(1<<31)|(1<<29);		
	s2450CAM->CIMSCTRL |= (1<<0);
	

	WaitForSingleObject(PostDoneEvent, INFINITE);

	memcpy(pBufOut, (BYTE*)(g_PhysDstAddr.LowPart + VIRTUAL_ADDR_OFFSET), sPOSTINFO.nDestWidth*sPOSTINFO.nDestHeight*2);


}
#endif

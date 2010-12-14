//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  
    system.c
    
Abstract:  
    Device dependant part of the USB Universal Host Controller Driver (OHCD).

Notes: 
--*/
#include <windows.h>
#include <ceddk.h>
#include <ddkreg.h>
#include <hcdddsi.h>
#include <S3C2450.h>	

#if (BSP_TYPE == BSP_SMDK2450)
volatile S3C2450_IOPORT_REG *pIOPregs	= NULL;	   
#elif (BSP_TYPE == BSP_SMDK2443)	
#endif
#define REG_PHYSICAL_PAGE_SIZE TEXT("PhysicalPageSize")
// Amount of memory to use for HCD buffer
static const DWORD gcTotalAvailablePhysicalMemory = 65536; // 64K
static const DWORD gcHighPriorityPhysicalMemory = 0x4000; // 16K

volatile S3C2450_CLKPWR_REG *pCLKPWR	= NULL;	


typedef struct _SOhcdPdd
{
    LPVOID lpvMemoryObject;
    LPVOID lpvOhcdMddObject;
    PVOID pvVirtualAddress;                        // DMA buffers as seen by the CPU
    DWORD dwPhysicalMemSize;
    PHYSICAL_ADDRESS LogicalAddress;        // DMA buffers as seen by the DMA controller and bus interfaces
    DMA_ADAPTER_OBJECT AdapterObject;
    TCHAR szDriverRegKey[MAX_PATH];
    PUCHAR ioPortBase;
    DWORD dwSysIntr;
    CRITICAL_SECTION csPdd;                     // serializes access to the PDD object
    HANDLE IsrHandle;
    HANDLE hParentBusHandle;
} SOhcdPdd;

#define UnusedParameter(x)  x = x

/* HcdPdd_DllMain
 * 
 *  DLL Entry point.
 *
 * Return Value:
 */
extern BOOL HcdPdd_DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    UnusedParameter(hinstDLL);
    UnusedParameter(dwReason);
    UnusedParameter(lpvReserved);

    return TRUE;
}

static BOOL
GetRegistryPhysicalMemSize(
    LPCWSTR RegKeyPath,         // IN - driver registry key path
    DWORD * lpdwPhyscialMemSize)       // OUT - base address
{
    HKEY hKey;
    DWORD dwData;
    DWORD dwSize;
    DWORD dwType;
    BOOL  fRet=FALSE;
    DWORD dwRet;
    // Open key
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegKeyPath,0,0,&hKey);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("!UHCD:GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
                             RegKeyPath, dwRet));
        return FALSE;
    }

    // Read base address, range from registry and determine IOSpace
    dwSize = sizeof(dwData);
    dwRet = RegQueryValueEx(hKey, REG_PHYSICAL_PAGE_SIZE, 0, &dwType, (PUCHAR)&dwData, &dwSize);
    if (dwRet == ERROR_SUCCESS) {
        if (lpdwPhyscialMemSize)
            *lpdwPhyscialMemSize = dwData;
        fRet=TRUE;
    }
    RegCloseKey(hKey);
    return fRet;
}


/* ConfigureOHCICard
 * 
 */
BOOL
ConfigureOHCICard(
    SOhcdPdd * pPddObject, // IN - contains PDD reference pointer.
    PUCHAR *pioPortBase,   // IN - contains physical address of register base
                           // OUT- contains virtual address of register base
    DWORD dwAddrLen,
    DWORD dwIOSpace,
    INTERFACE_TYPE IfcType,
    DWORD dwBusNumber
    )
{
    ULONG               inIoSpace = dwIOSpace;
    ULONG               portBase;
    PHYSICAL_ADDRESS    ioPhysicalBase = {0, 0};
    
    portBase = (ULONG)*pioPortBase;
    ioPhysicalBase.LowPart = portBase;

    if (!BusTransBusAddrToVirtual(pPddObject->hParentBusHandle, IfcType, dwBusNumber, ioPhysicalBase, dwAddrLen, &inIoSpace, (PPVOID)pioPortBase)) {
        DEBUGMSG(ZONE_ERROR, (L"OHCD: Failed TransBusAddrToVirtual\r\n"));
        return FALSE;
    }

    DEBUGMSG(ZONE_INIT, 
             (TEXT("OHCD: ioPhysicalBase 0x%X, IoSpace 0x%X\r\n"),
              ioPhysicalBase.LowPart, inIoSpace));
    DEBUGMSG(ZONE_INIT, 
             (TEXT("OHCD: ioPortBase 0x%X, portBase 0x%X\r\n"),
              *pioPortBase, portBase));

    return TRUE;
}


/* InitializeOHCI
 *
 *  Configure and initialize OHCI card
 *
 * Return Value:
 *  Return TRUE if card could be located and configured, otherwise FALSE
 */
static BOOL 
InitializeOHCI(
    SOhcdPdd * pPddObject,    // IN - Pointer to PDD structure
    LPCWSTR szDriverRegKey)   // IN - Pointer to active registry key string
{
    PUCHAR ioPortBase = NULL;
    DWORD dwAddrLen;
    DWORD dwIOSpace;
    BOOL InstallIsr = FALSE;
    BOOL fResult = FALSE;
    LPVOID pobMem = NULL;
    LPVOID pobOhcd = NULL;
    DWORD PhysAddr;
    DWORD dwHPPhysicalMemSize;
    HKEY hKey=NULL;

    DDKWINDOWINFO dwi;
    DDKISRINFO dii;


	RETAILMSG(0,(TEXT("InitializeOHCI\n")));
	pCLKPWR = (volatile S3C2450_CLKPWR_REG*)VirtualAlloc(0, sizeof(S3C2450_CLKPWR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (!pCLKPWR)
	{
		DEBUGMSG(1, (TEXT("pCLKPWR: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if (!VirtualCopy((PVOID)pCLKPWR, (PVOID)(S3C2450_BASE_REG_PA_CLOCK_POWER >> 8), sizeof(S3C2450_CLKPWR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG(1, (TEXT("pCLKPWR: VirtualCopy failed!\r\n")));
		return(FALSE);
	}    
#if (BSP_TYPE == BSP_SMDK2450)
	pIOPregs = (volatile S3C2450_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2450_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if(!pIOPregs) {
		DEBUGMSG(1, (TEXT("pIOPregs: VirtualAlloc failed!\r\n")));
		return(FALSE);
	}
	if(!VirtualCopy((PVOID)pIOPregs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
	{
		DEBUGMSG(1, (TEXT("pIOPregs: VirtualCopy failed!\r\n")));
		return(FALSE);
	}


 //[david.modify] 2008-08-02 11:05
 // usb host  5v电源使能
#if 0
    	pIOPregs->GPBUDP = (pIOPregs->GPBUDP & ~(0x3<<8)) |(0x2 << 8);
	pIOPregs->GPBCON = (pIOPregs->GPBCON & ~(0x3<<8)) | (0x1<<8);
       pIOPregs->GPBDAT = (pIOPregs->GPBDAT & ~(0x1<<4)) | (0x1<<4);
#endif	   
	   
#elif (BSP_TYPE == BSP_SMDK2443)
#endif
	pCLKPWR->HCLKCON |= (1<<11);
	pCLKPWR->CLKDIV1 = (pCLKPWR->CLKDIV1 & ~(0x3<<4))|(1<<4);
	pCLKPWR->SCLKCON |= (1<<1);

	pCLKPWR->USB_RSTCON = (0<<2)|(1<<1)|(0<<0);
	pCLKPWR->USB_RSTCON = (0<<2)|(0<<1)|(0<<0);
	pCLKPWR->USB_PHYCTRL &= ~(0x1<<1);  //48Mhz,Oscillator,External X-tal,device


#if 0    
	pCLKPWR->USB_RSTCON = (0<<2)|(1<<1)|(0<<0);
	pCLKPWR->USB_RSTCON = (0<<2)|(0<<1)|(0<<0);
	pCLKPWR->USB_PHYCTRL = 0x3;
	pCLKPWR->USB_PHYPWR = 0x0;
	pCLKPWR->USB_CLKCON = 0x12;  
#endif
	
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,szDriverRegKey,0,0,&hKey)!= ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("InitializeOHCI:GetRegistryConfig RegOpenKeyEx(%s) failed\r\n"),
                             szDriverRegKey));
        return FALSE;
    }
    dwi.cbSize=sizeof(dwi);
    dii.cbSize=sizeof(dii);
    if ( DDKReg_GetWindowInfo(hKey, &dwi ) !=ERROR_SUCCESS || DDKReg_GetIsrInfo (hKey, &dii ) != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("InitializeOHCI:DDKReg_GetWindowInfo or  DDKReg_GetWindowInfo failed\r\n")));
        goto InitializeOHCI_Error;
    }
    if (dwi.dwNumMemWindows!=0) {
        PhysAddr = dwi.memWindows[0].dwBase;
        dwAddrLen= dwi.memWindows[0].dwLen;
        dwIOSpace = 0;
    }
    else if (dwi.dwNumIoWindows!=0) {
        PhysAddr= dwi.ioWindows[0].dwBase;
        dwAddrLen = dwi.ioWindows[0].dwLen;
        dwIOSpace = 1;
    }
    else
        goto InitializeOHCI_Error;
        
    DEBUGMSG(ZONE_INIT,(TEXT("OHCD: Read config from registry: Base Address: 0x%X, Length: 0x%X, I/O Port: %s, SysIntr: 0x%X, Interface Type: %u, Bus Number: %u\r\n"),
                    PhysAddr, dwAddrLen, dwIOSpace ? L"YES" : L"NO", dii.dwSysintr, dwi.dwInterfaceType, dwi.dwBusNumber));
    
    ioPortBase = (PBYTE) PhysAddr;
    
    if (!(fResult = ConfigureOHCICard(pPddObject, &ioPortBase, dwAddrLen, dwIOSpace,dwi.dwInterfaceType, dwi.dwBusNumber))) {
        goto InitializeOHCI_Error;
    }    
    if (dii.szIsrDll[0] != 0 && dii.szIsrHandler[0]!=0 && dii.dwIrq<0xff && dii.dwIrq>0 ) {
        // Install ISR handler
        pPddObject->IsrHandle = LoadIntChainHandler(dii.szIsrDll, dii.szIsrHandler, (BYTE)dii.dwIrq);

        if (!pPddObject->IsrHandle) {
            DEBUGMSG(ZONE_ERROR, (L"OHCD: Couldn't install ISR handler\r\n"));
        } else {
            GIISR_INFO Info;
            PHYSICAL_ADDRESS PortAddress = {PhysAddr, 0};
            
            DEBUGMSG(ZONE_INIT, (L"OHCD: Installed ISR handler, Dll = '%s', Handler = '%s', Irq = %d\r\n",
                dii.szIsrDll, dii.szIsrHandler, dii.dwIrq));
            
            if (!BusTransBusAddrToStatic(pPddObject->hParentBusHandle,dwi.dwInterfaceType, dwi.dwBusNumber, PortAddress, dwAddrLen, &dwIOSpace, &(PVOID)PhysAddr)) {
                DEBUGMSG(ZONE_ERROR, (L"OHCD: Failed TransBusAddrToStatic\r\n"));
                goto InitializeOHCI_Error;
            }
        
            // Set up ISR handler
            Info.SysIntr = dii.dwSysintr;
            Info.CheckPort = TRUE;
            Info.PortIsIO = (dwIOSpace) ? TRUE : FALSE;
            Info.UseMaskReg = TRUE;
            Info.PortAddr = PhysAddr + 0x0C;
            Info.PortSize = sizeof(DWORD);
            Info.MaskAddr = PhysAddr + 0x10;
            
            if (!KernelLibIoControl(pPddObject->IsrHandle, IOCTL_GIISR_INFO, &Info, sizeof(Info), NULL, 0, NULL)) {
                DEBUGMSG(ZONE_ERROR, (L"OHCD: KernelLibIoControl call failed.\r\n"));
            }
        }
    }
    // The PDD can supply a buffer of contiguous physical memory here, or can let the 
    // MDD try to allocate the memory from system RAM.  We will use the HalAllocateCommonBuffer()
    // API to allocate the memory and bus controller physical addresses and pass this information
    // into the MDD.
    if (GetRegistryPhysicalMemSize(szDriverRegKey,&pPddObject->dwPhysicalMemSize)) {
        // A quarter for High priority Memory.
        dwHPPhysicalMemSize = pPddObject->dwPhysicalMemSize/4;
        // Align with page size.        
        pPddObject->dwPhysicalMemSize = (pPddObject->dwPhysicalMemSize + PAGE_SIZE -1) & ~(PAGE_SIZE -1);
        dwHPPhysicalMemSize = ((dwHPPhysicalMemSize +  PAGE_SIZE -1) & ~(PAGE_SIZE -1));
    }
    else {
        pPddObject->dwPhysicalMemSize=0;
        dwHPPhysicalMemSize = 0 ;
    }
    
    if (pPddObject->dwPhysicalMemSize<gcTotalAvailablePhysicalMemory) { // Setup Minimun requirement.
        pPddObject->dwPhysicalMemSize = gcTotalAvailablePhysicalMemory;
        dwHPPhysicalMemSize = gcHighPriorityPhysicalMemory;
    }

    pPddObject->AdapterObject.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    pPddObject->AdapterObject.InterfaceType = dwi.dwInterfaceType;
    pPddObject->AdapterObject.BusNumber = dwi.dwBusNumber;
    if ((pPddObject->pvVirtualAddress = HalAllocateCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, &pPddObject->LogicalAddress, FALSE)) == NULL) {
        goto InitializeOHCI_Error;
    }
    if (!(pobMem = HcdMdd_CreateMemoryObject(pPddObject->dwPhysicalMemSize, dwHPPhysicalMemSize, (PUCHAR) pPddObject->pvVirtualAddress, (PUCHAR) pPddObject->LogicalAddress.LowPart))) {
        goto InitializeOHCI_Error;
    }

    if (!(pobOhcd = HcdMdd_CreateHcdObject(pPddObject, pobMem, szDriverRegKey, ioPortBase, dii.dwSysintr))) {
        goto InitializeOHCI_Error;
    }

    pPddObject->lpvMemoryObject = pobMem;
    pPddObject->lpvOhcdMddObject = pobOhcd;
    _tcsncpy(pPddObject->szDriverRegKey, szDriverRegKey, MAX_PATH);
    pPddObject->ioPortBase = ioPortBase;
    pPddObject->dwSysIntr = dii.dwSysintr;


// PCI OHCI support suspend and resume
    if ( hKey!=NULL)  {
        DWORD dwCapability;
        DWORD dwType;
        DWORD dwLength = sizeof(DWORD);
        if (RegQueryValueEx(hKey, HCD_CAPABILITY_VALNAME, 0, &dwType, (PUCHAR)&dwCapability, &dwLength) == ERROR_SUCCESS)
            HcdMdd_SetCapability (pobOhcd,dwCapability);
		RETAILMSG(1,(TEXT("HOST CAP : %d\n"),dwCapability));
        RegCloseKey(hKey);
    }

    return TRUE;

InitializeOHCI_Error:

    if (pCLKPWR)
    {   
        VirtualFree((PVOID)pCLKPWR, 0, MEM_RELEASE);
        pCLKPWR = NULL;
    } 
#if (BSP_TYPE == BSP_SMDK2450)
    if (pIOPregs)
    {   
        VirtualFree((PVOID)pIOPregs, 0, MEM_RELEASE);
        pIOPregs = NULL;
    } 
#elif (BSP_TYPE == BSP_SMDK2443)	
#endif	
    if (pPddObject->IsrHandle) {
        FreeIntChainHandler(pPddObject->IsrHandle);
        pPddObject->IsrHandle = NULL;
    }
    
    if (pobOhcd)
        HcdMdd_DestroyHcdObject(pobOhcd);
    if (pobMem)
        HcdMdd_DestroyMemoryObject(pobMem);
    if(pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);

    pPddObject->lpvMemoryObject = NULL;
    pPddObject->lpvOhcdMddObject = NULL;
    pPddObject->pvVirtualAddress = NULL;
    if ( hKey!=NULL) 
        RegCloseKey(hKey);

    return FALSE;
}

/* HcdPdd_Init
 *
 *   PDD Entry point - called at system init to detect and configure OHCI card.
 *
 * Return Value:
 *   Return pointer to PDD specific data structure, or NULL if error.
 */
extern DWORD 
HcdPdd_Init(
    DWORD dwContext)  // IN - Pointer to context value. For device.exe, this is a string 
                      //      indicating our active registry key.
{
    SOhcdPdd *  pPddObject = malloc(sizeof(SOhcdPdd));
    BOOL        fRet = FALSE;

    if (pPddObject) {
        pPddObject->pvVirtualAddress = NULL;
        InitializeCriticalSection(&pPddObject->csPdd);
        pPddObject->IsrHandle = NULL;
        pPddObject->hParentBusHandle = CreateBusAccessHandle((LPCWSTR)g_dwContext);        
        
        if (pPddObject->hParentBusHandle) 
            fRet = InitializeOHCI(pPddObject, (LPCWSTR)dwContext);
        
        if(!fRet) {
            if (pPddObject->hParentBusHandle)
                CloseBusAccessHandle(pPddObject->hParentBusHandle);
            
            DeleteCriticalSection(&pPddObject->csPdd);
            free(pPddObject);
            pPddObject = NULL;
        }
    }

    return (DWORD)pPddObject;
}

/* HcdPdd_CheckConfigPower
 *
 *    Check power required by specific device configuration and return whether it
 *    can be supported on this platform.  For CEPC, this is trivial, just limit to
 *    the 500mA requirement of USB.  For battery powered devices, this could be 
 *    more sophisticated, taking into account current battery status or other info.
 *
 * Return Value:
 *    Return TRUE if configuration can be supported, FALSE if not.
 */
extern BOOL HcdPdd_CheckConfigPower(
    UCHAR bPort,         // IN - Port number
    DWORD dwCfgPower,    // IN - Power required by configuration
    DWORD dwTotalPower)  // IN - Total power currently in use on port
{
    return ((dwCfgPower + dwTotalPower) > 500) ? FALSE : TRUE;
}

extern void HcdPdd_PowerUp(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;    
    HcdMdd_PowerUp(pPddObject->lpvOhcdMddObject);

    return;
}

extern void HcdPdd_PowerDown(DWORD hDeviceContext)
{	
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;	
    HcdMdd_PowerDown(pPddObject->lpvOhcdMddObject);

    return;
}


extern BOOL HcdPdd_Deinit(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;

    if (pCLKPWR)
    {   
        VirtualFree((PVOID)pCLKPWR, 0, MEM_RELEASE);
        pCLKPWR = NULL;
    } 

    if(pPddObject->lpvOhcdMddObject)
        HcdMdd_DestroyHcdObject(pPddObject->lpvOhcdMddObject);
    if(pPddObject->lpvMemoryObject)
        HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);
    if(pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);

    if (pPddObject->IsrHandle) {
        FreeIntChainHandler(pPddObject->IsrHandle);
        pPddObject->IsrHandle = NULL;
    }
    if (pPddObject->hParentBusHandle)
        CloseBusAccessHandle(pPddObject->hParentBusHandle);
    
    free(pPddObject);
    
    return TRUE;
}


extern DWORD HcdPdd_Open(DWORD hDeviceContext, DWORD AccessCode,
        DWORD ShareMode)
{
    UnusedParameter(hDeviceContext);
    UnusedParameter(AccessCode);
    UnusedParameter(ShareMode);

    return 1; // we can be opened, but only once!
}


extern BOOL HcdPdd_Close(DWORD hOpenContext)
{
    UnusedParameter(hOpenContext);

    return TRUE;
}


extern DWORD HcdPdd_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pBuffer);
    UnusedParameter(Count);

    return (DWORD)-1; // an error occured
}


extern DWORD HcdPdd_Write(DWORD hOpenContext, LPCVOID pSourceBytes,
        DWORD NumberOfBytes)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pSourceBytes);
    UnusedParameter(NumberOfBytes);

    return (DWORD)-1;
}


extern DWORD HcdPdd_Seek(DWORD hOpenContext, LONG Amount, DWORD Type)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(Amount);
    UnusedParameter(Type);

    return (DWORD)-1;
}


extern BOOL HcdPdd_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
        DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(dwCode);
    UnusedParameter(pBufIn);
    UnusedParameter(dwLenIn);
    UnusedParameter(pBufOut);
    UnusedParameter(dwLenOut);
    UnusedParameter(pdwActualOut);

    return FALSE;
}


// Manage WinCE suspend/resume events

// This gets called by the MDD's IST when it detects a power resume.
// By default it has nothing to do.
extern void HcdPdd_InitiatePowerUp (DWORD hDeviceContext)
{ 

	RETAILMSG(0,(TEXT("HcdPdd_InitiatePowerUp\n")));
	pCLKPWR->HCLKCON |= (1<<11);
	pCLKPWR->CLKDIV1 = (pCLKPWR->CLKDIV1 & ~(0x3<<4))|(1<<4);
	pCLKPWR->SCLKCON |= (1<<1);
	
	pCLKPWR->USB_RSTCON = (0<<2)|(1<<1)|(0<<0);
	pCLKPWR->USB_RSTCON = (0<<2)|(0<<1)|(0<<0);
	pCLKPWR->USB_PHYCTRL &= ~(0x1<<1);  //48Mhz,Oscillator,External X-tal,device	
#if 0	
	/*
    pCLKPWR->USB_RSTCON = (0x0<<2)|(0x0<<1)|(0x1<<0);//Function 2.0 S/W reset, Host 1.1 S/W reset,PHY 2.0 S/W reset
    Sleep(1); // phy reset must be asserted for at 10us 
    pCLKPWR->USB_RSTCON = (0x0<<2)|(0x1<<1)|(0x0<<0);//Function 2.0 S/W reset, Host 1.1 S/W reset,PHY 2.0 S/W reset
    pCLKPWR->USB_RSTCON = (0x0<<2)|(0x0<<1)|(0x0<<0);//Function 2.0 S/W reset, Host 1.1 S/W reset,PHY 2.0 S/W reset
    	
	pCLKPWR->USB_PHYCTRL &= ~(0x1<<1);  //48Mhz,Oscillator,External X-tal,device*/
#endif    	
    return;
}

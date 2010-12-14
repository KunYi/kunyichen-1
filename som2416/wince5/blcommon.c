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
    blcommon.c
    
Abstract:  
    Bootloader common main module. This file contains the C BootloaderMain
    function for the boot loader.    NOTE: The firmware "entry" point (the real
    entry point) is _StartUp in init assembler file.

    The Windows CE boot loader is the code that is executed on a Windows CE
    development system at power-on reset and loads the Windows CE
    operating system. The boot loader also provides code that monitors
    the behavior of a Windows CE platform between the time the boot loader
    starts running and the time the full operating system debugger is 
    available. Windows CE OEMs are supplied with sample boot loader code 
    that runs on a particular development platform and CPU.
    
Functions:


Notes: 

--*/
#include <windows.h>
#include <halether.h>
#include <blcommon.h>

//#define FUSING_NK // if you want to fuse chain.lst, remove this definition

#ifndef FUSING_NK
#define FUSING_MULTIXIP
#endif

#define SPIN_FOREVER        while (1)

ROMHDR * volatile const pTOC = (ROMHDR *)-1;     // Gets replaced by RomLoader with real address
static DownloadManifest g_DownloadManifest;
static BOOLEAN g_bBINDownload = TRUE;

static BOOL KernelRelocate (ROMHDR *const pTOC);
static BOOL DownloadImage (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
static BOOL IsKernelRegion(DWORD dwRegionStart, DWORD dwRegionLength);

#define CURRENT_VERSION_MAJOR       1
#define CURRENT_VERSION_MINOR       1

const unsigned char NKSignon[] = {
    "\nMicrosoft Windows CE Ethernet Bootloader Common Library Version %d.%d Built "
    __DATE__ " " __TIME__ "\r\n"
};

DWORD g_dwROMOffset;

PFN_OEMVERIFYMEMORY     g_pOEMVerifyMemory;
PFN_OEMREPORTERROR      g_pOEMReportError;
PFN_OEMCHECKSIGNATURE   g_pOEMCheckSignature;
PFN_OEMMULTIBINNOTIFY   g_pOEMMultiBINNotify;


void BL1ReadFromSD();
void NKReadFromSD();
void EbootReadFromSD();
void MULTIXIPReadFromSD();

extern BOOLEAN g_bAutoDownload;
DWORD g_dwFusingIndex;

static void HALT (DWORD dwReason)
{
    if (g_pOEMReportError)
    {
        g_pOEMReportError (dwReason, 0);
    }
    SPIN_FOREVER;
}


void BootloaderMain (void)
{
    DWORD dwAction;   
    DWORD dwpToc = 0;
    DWORD dwImageStart = 0, dwImageLength = 0, dwLaunchAddr = 0;
    BOOL bDownloaded = FALSE;

    // relocate globals to RAM
    if (!KernelRelocate (pTOC))
    {
        // spin forever
        HALT (BLERR_KERNELRELOCATE);
    }

    // (1) Init debug support. We can use OEMWriteDebugString afterward.
    if (!OEMDebugInit ())
    {
        // spin forever
        HALT (BLERR_DBGINIT);
    }
    // output banner
    EdbgOutputDebugString (NKSignon, CURRENT_VERSION_MAJOR, CURRENT_VERSION_MINOR);

    // (3) initialize platform (clock, drivers, transports, etc)
    if (!OEMPlatformInit ())
    {
        // spin forever
        HALT (BLERR_PLATINIT);
    }

    // system ready, preparing for download
    EdbgOutputDebugString ("System ready!\r\nPreparing for download... aa %d a\r\n",g_bAutoDownload);

    // (4) call OEM specific pre-download function

    if ( g_bAutoDownload == TRUE )
    {
        BL1ReadFromSD();
        if (!DownloadImage (&dwImageStart, &dwImageLength, &dwLaunchAddr))
        {
            SPIN_FOREVER;
        }
        bDownloaded = TRUE;

        if (*(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
        {
            dwpToc = *(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG));
            // need to map the content again since the pointer is going to be in a fixup address
            dwpToc = (DWORD) OEMMapMemAddr (dwImageStart, dwpToc + g_dwROMOffset);

            EdbgOutputDebugString ("ROMHDR at Address %Xh\r\n", dwImageStart + ROM_SIGNATURE_OFFSET + sizeof (DWORD)); // right after signature
        }
        if (g_bBINDownload && g_pOEMCheckSignature)
        {
            if (!g_pOEMCheckSignature(dwImageStart, g_dwROMOffset, dwLaunchAddr, bDownloaded))
                HALT(BLERR_CAT_SIGNATURE);
        }
        OEMLaunch (dwImageStart, dwImageLength, dwLaunchAddr, (const ROMHDR *)dwpToc);        

        g_dwFusingIndex = 1;
        EbootReadFromSD();
        if (!DownloadImage (&dwImageStart, &dwImageLength, &dwLaunchAddr))
        {
            SPIN_FOREVER;
        }
        bDownloaded = TRUE;

        if (*(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
        {
            dwpToc = *(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG));
            // need to map the content again since the pointer is going to be in a fixup address
            dwpToc = (DWORD) OEMMapMemAddr (dwImageStart, dwpToc + g_dwROMOffset);

            EdbgOutputDebugString ("ROMHDR at Address %Xh\r\n", dwImageStart + ROM_SIGNATURE_OFFSET + sizeof (DWORD)); // right after signature
        }
        if (g_bBINDownload && g_pOEMCheckSignature)
        {
            if (!g_pOEMCheckSignature(dwImageStart, g_dwROMOffset, dwLaunchAddr, bDownloaded))
                HALT(BLERR_CAT_SIGNATURE);
        }
        OEMLaunch (dwImageStart, dwImageLength, dwLaunchAddr, (const ROMHDR *)dwpToc);        

#ifdef FUSING_NK

        NKReadFromSD();
        if (!DownloadImage (&dwImageStart, &dwImageLength, &dwLaunchAddr))
        {
            SPIN_FOREVER;
        }
        bDownloaded = TRUE;

        if (*(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
        {
            dwpToc = *(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG));
            // need to map the content again since the pointer is going to be in a fixup address
            dwpToc = (DWORD) OEMMapMemAddr (dwImageStart, dwpToc + g_dwROMOffset);

            EdbgOutputDebugString ("ROMHDR at Address %Xh\r\n", dwImageStart + ROM_SIGNATURE_OFFSET + sizeof (DWORD)); // right after signature
        }
        if (g_bBINDownload && g_pOEMCheckSignature)
        {
            if (!g_pOEMCheckSignature(dwImageStart, g_dwROMOffset, dwLaunchAddr, bDownloaded))
                HALT(BLERR_CAT_SIGNATURE);
        }
        OEMLaunch (dwImageStart, dwImageLength, dwLaunchAddr, (const ROMHDR *)dwpToc); 
        
#else        
        MULTIXIPReadFromSD();
        if (!DownloadImage (&dwImageStart, &dwImageLength, &dwLaunchAddr))
        {
            SPIN_FOREVER;
        }
        bDownloaded = TRUE;

        if (*(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
        {
            dwpToc = *(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG));
            // need to map the content again since the pointer is going to be in a fixup address
            dwpToc = (DWORD) OEMMapMemAddr (dwImageStart, dwpToc + g_dwROMOffset);

            EdbgOutputDebugString ("ROMHDR at Address %Xh\r\n", dwImageStart + ROM_SIGNATURE_OFFSET + sizeof (DWORD)); // right after signature
        }
        if (g_bBINDownload && g_pOEMCheckSignature)
        {
            if (!g_pOEMCheckSignature(dwImageStart, g_dwROMOffset, dwLaunchAddr, bDownloaded))
                HALT(BLERR_CAT_SIGNATURE);
        }
        OEMLaunch (dwImageStart, dwImageLength, dwLaunchAddr, (const ROMHDR *)dwpToc);        
#endif

    }
    else
    {
        switch (dwAction = OEMPreDownload ())
        {
        case BL_DOWNLOAD:
            // (5) download image
            if (!DownloadImage (&dwImageStart, &dwImageLength, &dwLaunchAddr))
            {
                // error already reported in DownloadImage
                SPIN_FOREVER;
            }
            bDownloaded = TRUE;

            // Check for pTOC signature ("CECE") here, after image in place
            if (*(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)
            {
                dwpToc = *(LPDWORD) OEMMapMemAddr (dwImageStart, dwImageStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG));
                // need to map the content again since the pointer is going to be in a fixup address
                dwpToc = (DWORD) OEMMapMemAddr (dwImageStart, dwpToc + g_dwROMOffset);

                EdbgOutputDebugString ("ROMHDR at Address %Xh\r\n", dwImageStart + ROM_SIGNATURE_OFFSET + sizeof (DWORD)); // right after signature
            }
    	//RETAILMSG(1,(TEXT("Download finished.! Spin Forever.!\r\n")));
    	//while(1);
            // fall through
        case BL_JUMP:
            // Before jumping to the image, optionally check the image signature.
            // NOTE: if we haven't downloaded the image by now, we assume that it'll be loaded from local storage in OEMLaunch (or it
            // already resides in RAM from an earlier download), and in this case, the image start address might be 0.  This means 
            // that the image signature routine will need to find the image in storage or in RAM to validate it.  Since the OEM"s 
            // OEMLaunch function will need to do this anyways, we trust that it's within their abilities to do it here.
            //
            if (g_bBINDownload && g_pOEMCheckSignature)
            {
                if (!g_pOEMCheckSignature(dwImageStart, g_dwROMOffset, dwLaunchAddr, bDownloaded))
                    HALT(BLERR_CAT_SIGNATURE);
            }
            // (5) final call to launch the image. never returned
            OEMLaunch (dwImageStart, dwImageLength, dwLaunchAddr, (const ROMHDR *)dwpToc);
            // should never return
            // fall through
        default:
            // ERROR! spin forever
            HALT (BLERR_INVALIDCMD);
        }
    }
}



//
// KernelRelocate: move global variables to RAM
//
static BOOL KernelRelocate (ROMHDR *const pTOC)
{
    ULONG loop;
    COPYentry *cptr;
    if (pTOC == (ROMHDR *const) -1)
    {
        return (FALSE); // spin forever!
    }
    // This is where the data sections become valid... don't read globals until after this
    for (loop = 0; loop < pTOC->ulCopyEntries; loop++)
    {
        cptr = (COPYentry *)(pTOC->ulCopyOffset + loop*sizeof(COPYentry));
        if (cptr->ulCopyLen)
            memcpy((LPVOID)cptr->ulDest,(LPVOID)cptr->ulSource,cptr->ulCopyLen);
        if (cptr->ulCopyLen != cptr->ulDestLen)
            memset((LPVOID)(cptr->ulDest+cptr->ulCopyLen),0,cptr->ulDestLen-cptr->ulCopyLen);
    }
    return (TRUE);
}

static BOOL VerifyChecksum (DWORD cbRecord, LPBYTE pbRecord, DWORD dwChksum)
{
    // Check the CRC
    DWORD dwCRC = 0;
    DWORD i;
    for (i = 0; i < cbRecord; i++)
        dwCRC += *pbRecord ++;

    if (dwCRC != dwChksum)
        EdbgOutputDebugString ("ERROR: Checksum failure (expected=0x%x  computed=0x%x)\r\n", dwChksum, dwCRC);

    return (dwCRC == dwChksum);
}

#define BL_HDRSIG_SIZE		7
static BOOL DownloadImage (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr)
{
    BYTE hdr[BL_HDRSIG_SIZE];
    DWORD dwRecLen, dwRecChk, dwRecAddr;
    BOOL fIsFlash = FALSE;
    LPBYTE lpDest = NULL;
    int nPkgNum = 0;
    BYTE nNumDownloadFiles = 1;
    DWORD dwImageStart = 0;
    DWORD dwImageLength = 0;
    RegionInfo *pCurDownloadFile;
    int i;
    *pdwImageStart = *pdwImageLength = *pdwLaunchAddr = 0;

    g_DownloadManifest.dwNumRegions = 0 ;
    for( i = 0 ; i < BL_MAX_BIN_REGIONS ; i++)
    {
        g_DownloadManifest.Region[i].dwRegionStart = 0;
        g_DownloadManifest.Region[i].dwRegionLength = 0;
        g_DownloadManifest.Region[i].szFileName[0] = '\0';        
    }

    do
    {
        // read the 7 byte "magic number"
        //
        if (!OEMReadData (BL_HDRSIG_SIZE, hdr))
        {
            EdbgOutputDebugString ("\r\nUnable to read image signature.\r\n");
            HALT (BLERR_MAGIC);
            return (FALSE);
        }
        // An N000FF packet is manufactured by Platform Builder when we're
        // downloading multiple files or when we're downloading a .nb0 file.
        //

        if (!memcmp (hdr, "N000FF\x0A", BL_HDRSIG_SIZE))
        {
            // read the packet checksum.
            //
            if (!OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecChk))
            {
                EdbgOutputDebugString("\r\nUnable to read download manifest checksum.\r\n");
                HALT (BLERR_MAGIC);
                return (FALSE);
            }

            // read BIN region descriptions (start address and length).
            //
            if (!OEMReadData (sizeof (DWORD), (LPBYTE) &g_DownloadManifest.dwNumRegions) ||
                !OEMReadData ((g_DownloadManifest.dwNumRegions * sizeof(RegionInfo)), (LPBYTE) &g_DownloadManifest.Region[0]))
            {
                EdbgOutputDebugString("\r\nUnable to read download manifest information.\r\n");
                HALT (BLERR_MAGIC);
                return (FALSE);
            }
            // verify the packet checksum.
            //
            if (!VerifyChecksum((g_DownloadManifest.dwNumRegions * sizeof(RegionInfo)), (LPBYTE) &g_DownloadManifest.Region[0], dwRecChk))
            {
                EdbgOutputDebugString ("\r\nDownload manifest packet failed checksum verification.\r\n");
                HALT (BLERR_CHECKSUM);
                return (FALSE);
            }
            // Provide the download manifest to the OEM.  This gives the OEM the
            // opportunity to provide start addresses for the .nb0 files (which 
            // don't contain placement information like .bin files do).
            //
            if (g_pOEMMultiBINNotify)
            {
                g_pOEMMultiBINNotify((PDownloadManifest)&g_DownloadManifest);
            }
            // look for next download...
            nNumDownloadFiles = (BYTE)(g_DownloadManifest.dwNumRegions + 1);      // +1 to account for this packet.
            continue;
        }
        // Is this an old X000FF multi-bin packet header?  It's no longer supported.
        //
        else if (!memcmp (hdr, "X000FF\x0A", BL_HDRSIG_SIZE))
        {
            EdbgOutputDebugString ("ERROR: The X000FF packet is an old-style multi-bin download manifest and it's no longer supported. \
                                   \r\nPlease update your Platform Builder installation in you want to download multiple files.\r\n");
            HALT (BLERR_MAGIC);
            return (FALSE);
        }
        // Is this a standard bin image?  Check for the usual bin file signature.
        //
        else if (!memcmp (hdr, "B000FF\x0A", BL_HDRSIG_SIZE))
        {
            g_bBINDownload = TRUE;

            if (!OEMReadData (sizeof (DWORD), (LPBYTE) &dwImageStart)
                || !OEMReadData (sizeof (DWORD), (LPBYTE) &dwImageLength))
            {
                EdbgOutputDebugString ("Unable to read image start/length\r\n");
                HALT (BLERR_MAGIC);
                return (FALSE);
            }
        }
        // If the header signature isn't recognized, we'll assume the
        // download file is a raw .nb0 file.
        //
        else
        {
            g_bBINDownload = FALSE;
        }

        // If Platform Builder didn't provide a download manifest (i.e., we're
        // only downloading a single .bin file), manufacture a manifest so we
        // can notify the OEM.
        //
        if (!g_DownloadManifest.dwNumRegions)
        {
            g_DownloadManifest.dwNumRegions             = 1;
            g_DownloadManifest.Region[0].dwRegionStart  = dwImageStart;
            g_DownloadManifest.Region[0].dwRegionLength = dwImageLength;

            // Provide the download manifest to the OEM.
            //
            if (g_pOEMMultiBINNotify)
            {
                g_pOEMMultiBINNotify((PDownloadManifest)&g_DownloadManifest);
            }
        }

        // Locate the current download manifest entry (current download file).
        //
        pCurDownloadFile = &g_DownloadManifest.Region[g_DownloadManifest.dwNumRegions - nNumDownloadFiles];

        // give the OEM a chance to verify memory
        if (g_pOEMVerifyMemory && !g_pOEMVerifyMemory (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
        {
            EdbgOutputDebugString ("!OEMVERIFYMEMORY: Invalid image\r\n");
            HALT (BLERR_OEMVERIFY);
            return (FALSE);
        }

        // check for flash image. Start erasing if it is.
        if ((fIsFlash = OEMIsFlashAddr (pCurDownloadFile->dwRegionStart)) 
            && !OEMStartEraseFlash (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
        {
            EdbgOutputDebugString ("Invalid Flash Address/Length\r\n");
            HALT (BLERR_FLASHADDR);
            return (FALSE);
        }
        // if we're downloading a binary file, we've already downloaded part of the image when searching
        // for a file header.  copy what we've read so far to the destination buffer, then finish downloading.
        if (!g_bBINDownload)
        {

            lpDest = OEMMapMemAddr (pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart);
            memcpy(lpDest, hdr, BL_HDRSIG_SIZE);

            // complete the file download...
            // read data block
            if (!OEMReadData ((pCurDownloadFile->dwRegionLength - BL_HDRSIG_SIZE), (lpDest + BL_HDRSIG_SIZE)))
            {
                EdbgOutputDebugString ("ERROR: failed when reading raw binary file.\r\n");
                HALT (BLERR_CORRUPTED_DATA);
                return (FALSE);
            }
        }
        // we're downloading a .bin file - download each .bin record in turn...
        else
        {
            while (OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecAddr) &&
                   OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecLen)  &&
                   OEMReadData (sizeof (DWORD), (LPBYTE) &dwRecChk))
            {
                // last record of .bin file uses sentinel values for address and checksum.
                if (!dwRecAddr && !dwRecChk)
                {
                    break;
                }

                // map the record address (FLASH data is cached, for example)
                lpDest = OEMMapMemAddr (pCurDownloadFile->dwRegionStart, dwRecAddr);

                // read data block
                if (!OEMReadData (dwRecLen, lpDest))
                {
                    EdbgOutputDebugString ("****** Data record %d corrupted, ABORT!!! ******\r\n", nPkgNum);
                    HALT (BLERR_CORRUPTED_DATA);
                    return (FALSE);
                }

                if (!VerifyChecksum (dwRecLen, lpDest, dwRecChk))
                {
                    EdbgOutputDebugString ("****** Checksum failure on record %d, ABORT!!! ******\r\n", nPkgNum);
                    HALT (BLERR_CHECKSUM);
                    return (FALSE);
                }

                // Look for ROMHDR to compute ROM offset.  NOTE: romimage guarantees that the record containing
                // the TOC signature and pointer will always come before the record that contains the ROMHDR contents.
                //
                if (dwRecLen == sizeof(ROMHDR) && (*(LPDWORD) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE))
                {
                    DWORD dwTempOffset = (dwRecAddr - *(LPDWORD)OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG)));
                    ROMHDR *pROMHdr = (ROMHDR *)lpDest;

                    // Check to make sure this record really contains the ROMHDR.
                    //
                    if ((pROMHdr->physfirst == (pCurDownloadFile->dwRegionStart - dwTempOffset)) &&
                        (pROMHdr->physlast  == (pCurDownloadFile->dwRegionStart - dwTempOffset + pCurDownloadFile->dwRegionLength)) &&
                        (DWORD)(HIWORD(pROMHdr->dllfirst << 16) <= pROMHdr->dlllast) &&
                        (DWORD)(LOWORD(pROMHdr->dllfirst << 16) <= pROMHdr->dlllast))
                    {
                        g_dwROMOffset = dwTempOffset;
                        EdbgOutputDebugString("rom_offset=0x%x.\r\n", g_dwROMOffset); 
                    }
                }

                // verify partial checksum
                OEMShowProgress (nPkgNum ++);

                if (fIsFlash)
                {
                    OEMContinueEraseFlash ();
                }
            }
        }

        // The image start address and length are passed back to the OEM code (OEMLaunch) 
        // in the following circumstances:
        // 1. The file is a raw .nb0 file.
        // 2. The file is a .bin file with a TOC that contains the kernel executable.
        // 3. The file is a .bin file without a TOC.
        //
        // If the image is a .bin file with a TOC that doesn't contain the kernel exectuable,
        // then it's a multi-xip/mulit-bin image for a non-kernel region and we don't pass
        // the start address and length back to the OEM code.  OEMLaunch can then save the
        // start address and length with the assurance that if the values are non-zero, they
        // represent the values for the NK region.
        //
        if (g_bBINDownload)
        {
            // Does this .bin file contain a TOC?
            //if (*(UINT32 *)(pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)    //- ksk 20051219
            if (*(LPDWORD) OEMMapMemAddr(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionStart + ROM_SIGNATURE_OFFSET) == ROM_SIGNATURE)    //+ ksk 20051219
            {
                // Contain the kernel?
                if (IsKernelRegion(pCurDownloadFile->dwRegionStart, pCurDownloadFile->dwRegionLength))
                {
                    *pdwImageStart  = pCurDownloadFile->dwRegionStart;
                    *pdwImageLength = pCurDownloadFile->dwRegionLength;
                    *pdwLaunchAddr  = dwRecLen;
                }
            }
            // No TOC - not made by romimage.  However, if we're downloading more than one
            // .bin file, it's probably chain.bin which doesn't have a TOC (and which isn't
            // going to be downloaded on its own) and we should ignore it.
            //
            else if (g_DownloadManifest.dwNumRegions == 1)
                                        
            {
                *pdwImageStart  = pCurDownloadFile->dwRegionStart;
                *pdwImageLength = pCurDownloadFile->dwRegionLength;
                *pdwLaunchAddr  = dwRecLen;
            }
        }
        else    // Raw binary file.
        {
            *pdwImageStart  = pCurDownloadFile->dwRegionStart;
            *pdwLaunchAddr  = pCurDownloadFile->dwRegionStart;
            *pdwImageLength = pCurDownloadFile->dwRegionLength;
        }

        // write to flash if it's flash image
        if (fIsFlash)
        {
            // finish the flash erase
            if (!OEMFinishEraseFlash ())
            {
                HALT (BLERR_FLASH_ERASE);
                return (FALSE);
            }
            // Before writing the image to flash, optionally check the image signature.
            if (g_bBINDownload && g_pOEMCheckSignature)
            {
                if (!g_pOEMCheckSignature(pCurDownloadFile->dwRegionStart, g_dwROMOffset, *pdwLaunchAddr, TRUE))
                    HALT(BLERR_CAT_SIGNATURE);
            }
        }
    }
    while (--nNumDownloadFiles);

    if (fIsFlash)
    {
        nNumDownloadFiles = (BYTE)g_DownloadManifest.dwNumRegions;
        while (nNumDownloadFiles--)
        {
            if (!OEMWriteFlash (g_DownloadManifest.Region[nNumDownloadFiles].dwRegionStart, g_DownloadManifest.Region[nNumDownloadFiles].dwRegionLength))
            {
                HALT (BLERR_FLASH_WRITE);
                return (FALSE);
            }
        }
    }

    return (TRUE);
}


/*
    @func   BOOLEAN | IsKernelRegion | Determines if the expanded BIN file provided contains the kernel image.
    @rdesc  TRUE if the region contains the kernel image, FALSE if it doesn't.
    @comm   <l Download Image> 
    @xref   
    @notes  dwCurrentBase is the base address where the BIN records are currently stored (this can be a RAM, a RAM
            file cache, or flash).  dwImageStoreBase is the images base storage address (this is the base address of potentially
            multiple BIN regions and can be in RAM or flash) and is used to translate addresses to the file cache area.
            dwROMOffset is the difference between the address where the BIN records are stored versus where they're fixed-up
            to run from (for example, an image may be stored in flash, but fixed-up to run in RAM).
*/
static BOOL IsKernelRegion(DWORD dwRegionStart, DWORD dwRegionLength)
{
	DWORD dwCacheAddress = 0;
	ROMHDR *pROMHeader;
	DWORD dwNumModules = 0;
	TOCentry *plTOC;

    if (dwRegionStart == 0 || dwRegionLength == 0)
        return(FALSE);

    if (*(LPDWORD) OEMMapMemAddr (dwRegionStart, dwRegionStart + ROM_SIGNATURE_OFFSET) != ROM_SIGNATURE)
        return (FALSE);

    // A pointer to the ROMHDR structure lives just past the ROM_SIGNATURE (which is a longword value).  Note that
    // this pointer is remapped since it might be a flash address (image destined for flash), but is actually cached
    // in RAM.
    //
    dwCacheAddress = *(LPDWORD) OEMMapMemAddr (dwRegionStart, dwRegionStart + ROM_SIGNATURE_OFFSET + sizeof(ULONG));
    pROMHeader     = (ROMHDR *) OEMMapMemAddr (dwRegionStart, dwCacheAddress + g_dwROMOffset);

    // Make sure sure are some modules in the table of contents.
    //
    if ((dwNumModules = pROMHeader->nummods) == 0)
        return (FALSE);

	// Locate the table of contents and search for the kernel executable and the TOC immediately follows the ROMHDR.
	//
    plTOC = (TOCentry *)(pROMHeader + 1);

	while(dwNumModules--) {
		LPBYTE pFileName = OEMMapMemAddr(dwRegionStart, (DWORD)plTOC->lpszFileName + g_dwROMOffset);
		if (!strcmp(pFileName, "nk.exe")) {
			return TRUE;
		}
		++plTOC;
	}
	return FALSE;
}

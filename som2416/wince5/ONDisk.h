/*****************************************************************************/
/*                                                                           */
/* PROJECT : PocketStoreII                                                   */
/* MODULE  : NAND Block Driver for supporting FAT File system                */
/* FILE    : ONDisk.h                                                        */
/* PURPOSE : This file declares data types and symbols for SYSTEM.h.         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*          COPYRIGHT 2003-2006 SAMSUNG ELECTRONICS CO., LTD.                */
/*                          ALL RIGHTS RESERVED                              */
/*                                                                           */
/*   Permission is hereby granted to licensees of Samsung Electronics        */
/*   Co., Ltd. products to use or abstract this computer program for the     */
/*   sole purpose of implementing a product based on Samsung                 */
/*   Electronics Co., Ltd. products. No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in whole,      */
/*   are granted.                                                            */
/*                                                                           */
/*   Samsung Electronics Co., Ltd. makes no representation or warranties     */
/*   with respect to the performance of this computer program, and           */
/*   specifically disclaims any responsibility for any damages,              */
/*   special or consequential, connected with the use of this program.       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* REVISION HISTORY                                                          */
/*                                                                           */
/*   18-OCT-2006 [Seungkyu Kim]: first writing                               */
/*                                                                           */
/*****************************************************************************/

#ifndef _ONDISK_H_
#define _ONDISK_H_


#include <windows.h>

#define ONDISK_MULTIPARTITION 1  // set 1 to use multiple partition on ONDisk

typedef struct _DISK
{
    struct _DISK	   *pd_next;
    CRITICAL_SECTION	d_DiskCardCrit;	// guard access to global state and card
	HANDLE				hDevice;		// activate Handle
    DISK_INFO			d_DiskInfo;		// for DISK_IOCTL_GET/SETINFO
    DWORD				d_OpenCount;	// open ref count
    LPWSTR				d_ActivePath;   // registry path to active key for this device
	BOOL				fBusy;			// if do reading, writing, deleting operation, 
	//	fBusy is TRUE. otherwise, it's FALSE
	BOOL				DoThreading;	// if SSR reclaim is running, it is TRUE
	DWORD				SecPerCluster;	// FATxx relative information, Sector per cluster
	DWORD				FirstDataSector;// FATxx relative information, First Data Sector
    UINT                nVol;           // Volnum Number
	BOOL				bIsFTLOpen;		// if FTL_Open operation successes,
	DWORD				nBmlPartitionId;	// BML Partition Id for STL binding
#if ONDISK_MULTIPARTITION
	UINT32				nWMRStartSector;	// Start Sector for Multiple Partition
	UINT32				nWMRNumOfSector;	// the Number of Sector for Multiple Partition
#endif
	// it's TRUE otherwize it's FALSE
} DISK, *PDISK;

#endif /* _ONDISK_H_ */


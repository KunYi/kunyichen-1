// testapp.cpp : Defines the entry point for the application.
//

#pragma optimize("", off)

#include <windows.h>
#include <nkintr.h>
#include <stdio.h>
#include <s3c2450.h>

#define READ_CHUNK_SIZE	(0x300000)
#define WRITE_CHUNK_SIZE	(0x300000)
#define LOOP_COUNT		(10)

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	volatile S3C2450_IOPORT_REG *v_pIOPregs = NULL;
	register int i;

	FILE *pFilePtr = NULL;
	FILE *pFileOutPtr = NULL;

	int nFileLength;
	long nStartTick, nEndTick;
	int nBytePerSec;

	char *cBuffer = NULL;

	if(v_pIOPregs == NULL)
	{
		v_pIOPregs = (volatile S3C2450_IOPORT_REG *) VirtualAlloc(0,sizeof(S3C2450_IOPORT_REG),MEM_RESERVE, PAGE_NOACCESS);
		if(v_pIOPregs == NULL)
		{
				RETAILMSG(1,(TEXT("For v_pIOPregs: VirtualAlloc failed!\r\n")));
			goto Fail;
		}
		else if(!VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2450_BASE_REG_PA_IOPORT>>8),sizeof(S3C2450_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE ))
		{
			RETAILMSG(1,(TEXT("For v_pIOPregs: VirtualCopy failed!\r\n")));
			goto Fail;
		}

	}

	pFilePtr = fopen("PocketMory/input.wmv", "rb");
	if (!pFilePtr)
	{
		RETAILMSG(1,(TEXT("PocketMory/input.wmv file is not opened.!!!\r\n")));
		goto Fail;
	}

	v_pIOPregs->GPFCON &= ~(0xf<<12);
	v_pIOPregs->GPFCON |= (0x5<<12);
	v_pIOPregs->GPFUDP &= ~(0xF<<12);
	v_pIOPregs->GPFUDP |= (0x5<<12);
	v_pIOPregs->GPFDAT &= ~(0x3<<6);

	fseek(pFilePtr, 0, SEEK_END);
	nFileLength = ftell(pFilePtr);

	RETAILMSG(1,(TEXT("Input File size is %d byte\r\n"), nFileLength));

	cBuffer = malloc(nFileLength);
	if (cBuffer == NULL)
	{
		RETAILMSG(1,(TEXT("malloc() is failed!!!\r\n")));
		goto Fail;
	}

	RETAILMSG(1,(TEXT("Read Start!!!\r\n")));

	nStartTick = GetTickCount();

	v_pIOPregs->GPFDAT |= (0x1<<7);	// Set GPIO High

	for(i=0;i<LOOP_COUNT;i++)
	{
		int nReadCount = 0;

		fseek(pFilePtr, 0, SEEK_SET);

		while(nReadCount < nFileLength)
		{
			nReadCount += fread(cBuffer+nReadCount, sizeof(char), READ_CHUNK_SIZE, pFilePtr);
		}
	}

	v_pIOPregs->GPFDAT &= ~(0x1<<7);	// Set GPIO Low

	nEndTick = GetTickCount();

	RETAILMSG(1,(TEXT("Read Done!!!\r\n")));

	RETAILMSG(1,(_T(" Time for reading %d.%d MB file %d times\r\n"), nFileLength/1048576, (nFileLength*10/1048576)%10, LOOP_COUNT));
	RETAILMSG(1,(_T(" Elapsed time = %d ms\r\n"), nEndTick - nStartTick));

	nBytePerSec = nFileLength*LOOP_COUNT/(nEndTick - nStartTick)*1000;

	RETAILMSG(1,(_T(" Read Performance = %d Byte/Sec\n\r"), nBytePerSec));
	RETAILMSG(1,(_T(" Read Performance = %d.%d MB/Sec\n\r"), nBytePerSec/1048576, (nBytePerSec*100/1048576)%100));

	pFileOutPtr = fopen("PocketMory/Output.wmv", "wb");
	if (!pFileOutPtr)
	{
		RETAILMSG(1,(TEXT("PocketMory/Output.wmv file is not opened.!!!\r\n")));
		goto Fail;
	}

	RETAILMSG(1,(TEXT("Write Start!!!\r\n")));

	nStartTick = GetTickCount();

	v_pIOPregs->GPFDAT |= (1<<6);	// Set GPIO High

	for(i=0;i<LOOP_COUNT;i++)
	{
		int nWriteCount = 0;

		fseek(pFileOutPtr, 0, SEEK_SET);

		while(nWriteCount < nFileLength)
		{
			if ((nFileLength-nWriteCount) >= WRITE_CHUNK_SIZE)
			{
				nWriteCount += fwrite(cBuffer+nWriteCount, sizeof(char), WRITE_CHUNK_SIZE, pFileOutPtr);
			}
			else
			{
				nWriteCount += fwrite(cBuffer+nWriteCount, sizeof(char), (nFileLength-nWriteCount), pFileOutPtr);
			}
		}
	}

	v_pIOPregs->GPFDAT &= ~(1<<6);	// Set GPIO Low

	nEndTick = GetTickCount();

	RETAILMSG(1,(TEXT("Write Done!!!\r\n")));

	RETAILMSG(1,(_T(" Time for writting %d.%d MB file %d times\r\n"), nFileLength/1048576, (nFileLength*10/1048576)%10, LOOP_COUNT));
	RETAILMSG(1,(_T(" Elapsed time = %d ms\r\n"), nEndTick - nStartTick));

	nBytePerSec = nFileLength*LOOP_COUNT/(nEndTick - nStartTick)*1000;

	RETAILMSG(1,(_T(" Write Performance = %d Byte/Sec\n\r"), nBytePerSec));
	RETAILMSG(1,(_T(" Write Performance = %d.%d MB/Sec\n\r"), nBytePerSec/1048576, (nBytePerSec*100/1048576)%100));

	if (pFileOutPtr) fclose(pFileOutPtr);
	if (pFilePtr) fclose(pFilePtr);
	VirtualFree((void*)v_pIOPregs,0,MEM_RELEASE);
	return 0;


Fail:
	RETAILMSG(1,(TEXT("WMR_RW_test is failed.!!\r\n")));
	if (pFileOutPtr) fclose(pFileOutPtr);
	if (pFilePtr) fclose(pFilePtr);
	VirtualFree((void*)v_pIOPregs,0,MEM_RELEASE);
	return 0;

}



#pragma optimize("", on)

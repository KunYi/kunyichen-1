// testapp.cpp : Defines the entry point for the application.
//

#pragma optimize("", off)
#include <windows.h>
#include <nkintr.h>
#include <stdio.h>

#define _MAX_SIZE_	(UINT32)0x400000
#define _STORAGE_SIZE_ (UINT32)0x30000000
#define _MAX_FILES_ (UINT32)((_STORAGE_SIZE_)/(_MAX_SIZE_))

int InitBuffer(char *buffer);
int CompareBuffer(char *read, char *write);
void Str2Int_Read(int source);
void Str2Int_Write(int source);

char g_cReadFileName[25];
char g_cWriteFileName[25];
wchar_t g_wcReadFileName[25];
wchar_t g_wcWriteFileName[25];

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	//int nStartMS, nEndMS;
	char *cBuffer = NULL;
	char *cCompareBuffer = NULL;    
	FILE *pFilePtr = NULL;
	FILE *pFileOutPtr = NULL;
    	int i,j;

   	strncpy(g_cReadFileName,"PocketMory1/File0000.wmv",25);
   	strncpy(g_cWriteFileName,"PocketMory1/File0000.wmv",25);    
   	wcsncpy(g_wcReadFileName,(TEXT("PocketMory1/File0000.wmv")),25);    
    	wcsncpy(g_wcWriteFileName,(TEXT("PocketMory1/File0000.wmv")),25);        
    
	cBuffer = malloc(_MAX_SIZE_);
	if (cBuffer == NULL)
	{
		RETAILMSG(1,(TEXT("malloc() is failed!!!\r\n")));
		goto Fail;
	}

	cCompareBuffer = malloc(_MAX_SIZE_);
	if (cCompareBuffer == NULL)
	{
		RETAILMSG(1,(TEXT("malloc() is failed!!!\r\n")));
		goto Fail;
	}

	for ( j = 0 ; j < 20 ; j++ )
	{
	        RETAILMSG(1,(TEXT("The %dTh WMR_aging start.!!\r\n"),j));  
	        if(!(InitBuffer(cBuffer)))
	        {
	        	RETAILMSG(1,(TEXT("IninBuffer(cBuffer) error\n")));
	        	return 0;
	        }
	        
	        if(!(InitBuffer(cCompareBuffer)))
	        {
	        	RETAILMSG(1,(TEXT("IninBuffer(cCompareBuffer) error\n")));
	        	return 0;
	        }
	            
		pFilePtr = fopen("PocketMory1/input.wmv", "rb");
		if (!pFilePtr)
		{
			RETAILMSG(1,(TEXT("PocketMory1/input.wmv file is not opened.!!!\r\n")));
			goto Fail;
		}
	
		fread(cBuffer, sizeof(char), _MAX_SIZE_, pFilePtr);
		
		pFileOutPtr = fopen("PocketMory1/File0000.wmv", "wb");
		if (!pFileOutPtr)
		{
			RETAILMSG(1,(TEXT("PocketMory1/File0000.wmv file is not opened.!!!\r\n")));
			goto Fail;
		}
	
		fwrite(cBuffer, sizeof(char), _MAX_SIZE_, pFileOutPtr);
	
		fclose(pFilePtr);
		fclose(pFileOutPtr);
	
		for (i=0;i<_MAX_FILES_;i++)
	        {
	         	Str2Int_Read(i);
	        	Str2Int_Write(i+1);    
	                
	        	if(!(InitBuffer(cCompareBuffer)))
	        	{
	        		RETAILMSG(1,(TEXT("IninBuffer(cCompareBuffer) error\n")));
	        		return 0;
	        	}
	        	memcpy(cCompareBuffer,cBuffer,_MAX_SIZE_);
	
	        	pFilePtr = fopen(g_cReadFileName,"rb");
	        	if (!pFilePtr)
	        	{
	        		RETAILMSG(1,(TEXT("%s file is not opened(rb).!!!\r\n"),g_wcReadFileName));
	        		goto Fail;
	        	}
			
		        if(!(InitBuffer(cBuffer)))
		        {
		        	RETAILMSG(1,(TEXT("IninBuffer(cBuffer) error\n")));
		        	return 0;
		        }
	        	fread(cBuffer, sizeof(char), _MAX_SIZE_, pFilePtr);
	
	            if ( !(CompareBuffer(cBuffer,cCompareBuffer)) )
	            {
	                RETAILMSG(1,(TEXT("Compare Error %s and %s \n"),g_wcReadFileName,g_wcWriteFileName));
	                goto Fail;
	            }
	        	
	        	pFileOutPtr = fopen(g_cWriteFileName, "wb");
	        	if (!pFileOutPtr)
	        	{
	        		RETAILMSG(1,(TEXT("%s file is not opened(wb).!!!\r\n"),g_wcWriteFileName));
	        		goto Fail;
	        	}
	
	        	fwrite(cBuffer, sizeof(char), _MAX_SIZE_, pFileOutPtr);
	
	
	            fclose(pFilePtr);
	            fclose(pFileOutPtr);
	     
	    		RETAILMSG(1,(TEXT("Read : %s Write : %s done.\r\n"),g_wcReadFileName,g_wcWriteFileName));
	        }
	
		for (i=0;i<_MAX_FILES_;i++)
		{
			Str2Int_Read(i);
			if (!(DeleteFile((LPCTSTR)g_wcReadFileName)))
			{
				RETAILMSG(1,(TEXT("error when remove %s file.!!\r\n"),g_wcReadFileName));
				return 0;
	        	}
	        	RETAILMSG(1,(TEXT("Remove %s file successfully.!!\r\n"),g_wcReadFileName));
		}
		
		RETAILMSG(1,(TEXT("The %dTh WMR_aging's done successfully.!!\r\n"),j));

	}
	RETAILMSG(1,(TEXT("All WMR_aging've done successfully.!!\r\n")));
	return 0;
	

Fail:
	RETAILMSG(1,(TEXT("WMR_aging is failed.!!\r\n")));
	return 0;

}


void Str2Int_Read(int source)
{
    int i,j,k,l;

    i = (source/1000);
    j = (source-1000*i)/100;
    k = ((source-1000*i)-100*j)/10;
    l = (((source-1000*i)-100*j)-10*k);

    g_cReadFileName[16] = i+0x30; // to translate int to ascii int
    g_cReadFileName[17] = j+0x30; // to translate int to ascii int
    g_cReadFileName[18] = k+0x30; // to translate int to ascii int
    g_cReadFileName[19] = l+0x30; // to translate int to ascii int    
//    g_cFileName[15] = '\0';
    
    g_wcReadFileName[16] = i+0x30; // to translate int to ascii int
    g_wcReadFileName[17] = j+0x30; // to translate int to ascii int
    g_wcReadFileName[18] = k+0x30; // to translate int to ascii int
    g_wcReadFileName[19] = l+0x30; // to translate int to ascii int    
//    g_wcFileName[15] = '\0';

}

void Str2Int_Write(int source)
{
    int i,j,k,l;

    i = (source/1000);
    j = (source-1000*i)/100;
    k = ((source-1000*i)-100*j)/10;
    l = (((source-1000*i)-100*j)-10*k);

    g_cWriteFileName[16] = i+0x30; // to translate int to ascii int
    g_cWriteFileName[17] = j+0x30; // to translate int to ascii int
    g_cWriteFileName[18] = k+0x30; // to translate int to ascii int
    g_cWriteFileName[19] = l+0x30; // to translate int to ascii int    
//    g_cFileName[15] = '\0';
    
    g_wcWriteFileName[16] = i+0x30; // to translate int to ascii int
    g_wcWriteFileName[17] = j+0x30; // to translate int to ascii int
    g_wcWriteFileName[18] = k+0x30; // to translate int to ascii int
    g_wcWriteFileName[19] = l+0x30; // to translate int to ascii int    
//    g_wcFileName[15] = '\0';

}

int InitBuffer(char *buffer)
{
	if (!(memset(buffer,'\0',_MAX_SIZE_)))
	{
		return 0;
	}
	return 1;
}


int CompareBuffer(char *read, char *write)
{
    int i;

    for(i=0;i<_MAX_SIZE_;i++)
    {
        if (read[i] != write[i])
            return FALSE;
    }
    return TRUE;
}

#pragma optimize("",on)

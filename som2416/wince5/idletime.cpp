// testapp.cpp : Defines the entry point for the application.
//

#pragma optimize("", off)

#include <windows.h>
#include <nkintr.h>
//#include <oalintr.h>
//#include <drv_glob.h>
#include <s3c2450.h>


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	unsigned long dwStartTick, dwIdleSt, dwStopTick, dwIdleEd, PercentIdle;
	int i;

	while ( true )
	{
		dwStartTick = GetTickCount();
		dwIdleSt = GetIdleTime();
		Sleep(1000);
		dwStopTick = GetTickCount();
		dwIdleEd = GetIdleTime();
		PercentIdle = ((100*(dwIdleEd - dwIdleSt)) / (dwStopTick - dwStartTick));

//		RETAILMSG(1,(_T(" Idle Time : %3d%% :"),PercentIdle));
		RETAILMSG(1,(_T("%3d\r\n"),PercentIdle));
//		for ( i = 0; i < PercentIdle / 10; i++ )
//			RETAILMSG(1,(_T("*")));
//		RETAILMSG(1,(_T("\r\n")));
	}

	return 0;
}


#pragma optimize("", on)
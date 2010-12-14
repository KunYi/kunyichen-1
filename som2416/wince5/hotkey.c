/*
** hotkey.c
*/
#include <windows.h>
#include "hotkey.h"

//#define dim(arr)    (sizeof (arr) / sizeof (arr[0]))

typedef struct
{
    PFN_HOTKEY_HDLR pfHdlr;
    HANDLE          hEvent;
    LPCTSTR         szName;
} HOTKEY;

static HOTKEY g_HotKeys [MAX_HOTKEY_NUM];
void HotKeyInit()
{
	memset(g_HotKeys, 0, sizeof(g_HotKeys));
}

HANDLE HotKeyRegister (DWORD dwHotKey, PFN_HOTKEY_HDLR pfHdlr, LPTSTR szName)
{
    if (dwHotKey < MAX_HOTKEY_NUM)
    {
      	g_HotKeys[dwHotKey].szName = szName;    

        if (NULL == g_HotKeys [dwHotKey].hEvent)
        {
            	g_HotKeys [dwHotKey].hEvent = CreateEvent (
                    NULL, FALSE, FALSE, g_HotKeys[dwHotKey].szName);

		if (NULL==g_HotKeys [dwHotKey].hEvent)
		{
			DEBUGMSG(1, (TEXT("HotKeyRegister: create event fail, error code %d\r\n"), GetLastError()));
		}
        }

        if (NULL != g_HotKeys [dwHotKey].hEvent)
        {
            g_HotKeys [dwHotKey].pfHdlr = pfHdlr;
        }
    }

    return g_HotKeys [dwHotKey].hEvent;
}

// HotKeyProcess() function to call the hotkey handler registered in the g_HotKeys[] in sequence,
// if One handler returns TURE, means the key event is completely handled and not necessary to deliver to other handlers
// otherwise return FALSE means the handler not treat the key or it still necessary to be treated in other handler.
// Retrun value:
// FALSE: the key have not finished handling, continue to be transfer.
// TURE: the key have been finished handling.

BOOL HotKeyProcess (DWORD dwScanCode, BOOL bKeyUp)
{
    BOOL    bRet = FALSE;
    DWORD   i;

    for (i = 0; i < MAX_HOTKEY_NUM; i++)
    {
        if (g_HotKeys [i].pfHdlr != NULL)
        {
            if ((*g_HotKeys [i].pfHdlr)(i, dwScanCode, bKeyUp))
            {
                bRet = TRUE;
                break;
            }
        }
    }

    return bRet;
}

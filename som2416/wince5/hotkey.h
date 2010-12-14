/*
** hotkey.h
*/
#if !defined (_HOTKEY_H)
#define _HOTKEY_H

#if defined (__cplusplus)
extern "C"
{
#endif

/*
** hotkey common event name
*/
#define ATLAS_AUDIO_VOL_EVENT		_T("_ATLAS_AUDIO_VOL_EVENT")
#define ATLAS_AUDIO_VOL_MSG			_T("_ATLAS_AUDIO_VOL_MSG")

#define ATLAS_APP_EVENT		_T("_ATLAS_APP_EVENT")
/*
** hotkey id constants
*/
#define MAX_HOTKEY_NUM		16

#define HOTKEY_VOL_CHANGE   0
#define HOTKEY_ANOTHER      1
#define HOTKEY_ETC          2
#define HOTKEY_ATLAS_APP   3

/******************************************************************************
**  typedef BOOL (*PFN_HOTKEY_HDLR)(DWORD dwHotKey, DWORD dwScanCode, BOOL bKeyUp);
**  Param:  dwHotKey    hotkey id
**          dwScanCode  Scan code
**          bKeyUp      The key up/down state 
**  Return:	TRUE:   The specified key is handled by the hotkey handler.
**          FALSE:  The key should be handled as usual.
**  Remark:	After KBD driver get the scan code & key state, there's a chance
**          to handle it before mapping to the VKEY.
******************************************************************************/
typedef BOOL (*PFN_HOTKEY_HDLR)(DWORD dwHotKey, DWORD dwScanCode, BOOL bKeyUp);


/******************************************************************************
**  void HotKeyInit();
**  Remark:	This function is called in the keyboard driver's initialization routine to
**          clear the hotkey record.
******************************************************************************/
void HotKeyInit();

/******************************************************************************
**  HANDLE HotKeyRegister (DWORD dwHotKey, PFN_HOTKEY_HDLR pfHdlr);
**  Param:	dwHotKey    hotkey id
**          pfHdlr      hotkey handler
**		szName	hotkey functional name
**  Return:	The event handle of the specified hotkey id
**  Remark:	This function is called in the driver's initialization routine to
**          register the hotkey handler.
******************************************************************************/
HANDLE HotKeyRegister (DWORD dwHotKey, PFN_HOTKEY_HDLR pfHdlr, LPTSTR szName);

/******************************************************************************
**  BOOL HotKeyProcess (DWORD dwScanCode, BOOL bKeyUp);
**  Param:	dwScanCode  key scan code
**          bKeyUp      The key up/down state 
**  Return:	TRUE:   The specified key is handled by one of the hotkey handlers.
**          FALSE:  The key should be handled as usual.
**  Remark:	This function is called to handle hotkey. It will then call the
**          registered hotkey handler in turn.
******************************************************************************/
BOOL HotKeyProcess (DWORD dwScanCode, BOOL bKeyUp);

#if defined (__cplusplus)
}
#endif

#endif  /* !defined (_HOTKEY_H) */
/* end of hotkey.h */

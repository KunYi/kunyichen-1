/*
** SleepEvent.h
*/
#if !defined (SLEEP_EVENT_H)
#define SLEEP_EVENT_H

#define SDMMC_SLEEP_EVENT   TEXT("SDMMC_SLEEP_EVENT")

extern HANDLE  g_hSleepEvent;

__inline VOID SleepEventDeinit (VOID)
{
    if (NULL != g_hSleepEvent)
    {
        CloseHandle (g_hSleepEvent);
        g_hSleepEvent = NULL;
    }
}

__inline BOOL SleepEventInit (VOID)
{
    if (NULL == g_hSleepEvent)
    {
        g_hSleepEvent = CreateEvent (NULL, TRUE, FALSE, SDMMC_SLEEP_EVENT);
        SetEventData(g_hSleepEvent, 0);
    }

    return g_hSleepEvent != NULL;
}

#endif  /* !defined (SLEEP_EVENT_H) */


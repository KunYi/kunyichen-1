// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-2000 Microsoft Corporation.  All rights reserved.
//
// -----------------------------------------------------------------------------
#pragma once

#include "windows.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define WAVE_FORMAT_MIDI 0x3000
#define MM_MOM_MIDIMESSAGE  (WM_USER+0x100)
#define MM_MOM_SETMIDITICKS (WM_USER+0x101)

#define MIDI_MESSAGE_UPDATETEMPO 0x10000000
#define MIDI_MESSAGE_FREQGENON   0x20000000
#define MIDI_MESSAGE_FREQGENOFF  0x30000000

typedef struct _WAVEFORMAT_MIDI
{
    WAVEFORMATEX wfx;
    UINT32 USecPerQuarterNote;
    UINT32 TicksPerQuarterNote;
} WAVEFORMAT_MIDI, *LPWAVEFORMAT_MIDI;
#define WAVEFORMAT_MIDI_EXTRASIZE (sizeof(WAVEFORMAT_MIDI)-sizeof(WAVEFORMATEX))

typedef struct _WAVEFORMAT_MIDI_MESSAGE
{
    UINT32 DeltaTicks;
    DWORD  MidiMsg;
} WAVEFORMAT_MIDI_MESSAGE;

#ifdef __cplusplus
}
#endif // __cplusplus


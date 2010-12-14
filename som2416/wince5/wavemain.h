#pragma once
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-2000 Microsoft Corporation.  All rights reserved.
//
// -----------------------------------------------------------------------------

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <string.h>
#include <nkintr.h>
#include <excpt.h>
#include <wavedbg.h>
#include <wavedev.h>
#include <waveddsi.h>
#include <mmddk.h>
#include <cardserv.h>
#include <devload.h>

#include <linklist.h>
#include <audiosys.h>
#include <wfmtmidi.h>

class StreamContext;
class WaveStreamContext;
class InputStreamContext;
class OutputStreamContext;
class DeviceContext;
class InputDeviceContext;
class OutputDeviceContext;
class HardwareContext;

#include "RtCodec.h"
#include "wavepdd.h"
#include "devctxt.h"
#include "hwctxt.h"
#include "strmctxt.h"
#include "midistrm.h"
#include "mixerdrv.h"

 //[david.modify] 2008-06-19 18:50
#define WODM_WRITE_AC97    		(WIDM_GETNUMDEVS - 2)		
#define WIDM_READ_AC97  		(WIDM_GETNUMDEVS - 1)
#define WODM_GET_AUDIOLEVEL		(WM_USER + 3)
#define WODM_SET_AUDIOLEVEL		(WM_USER + 4)
#define WODM_GET_MAX_AUDIOLEVEL	(WM_USER + 5)
#define WODM_SET_MAX_AUDIOLEVEL	(WM_USER + 6)
#define WODM_GET_HPAUDIOLEVEL		(WM_USER + 7)
#define WODM_SET_HPAUDIOLEVEL		(WM_USER + 8)
#define WODM_GET_SPAUDIOLEVEL		(WM_USER + 9)
#define WODM_SET_SPAUDIOLEVEL		(WM_USER + 10)
 //[david. end] 2008-06-19 18:51



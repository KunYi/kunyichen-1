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
#ifndef __OAL_LOG_H
#define __OAL_LOG_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Macro:  OALMSG/OALMSGS
//
//  This macro should be used for logging in OAL. It supports log zones. All
//  tracing messages based on this macro is removed when SHIP_BUILD is
//  defined. OALMSGS uses always serial output.
//

#ifdef SHIP_BUILD
#define OALMSG(cond, exp)   ((void)FALSE)
#define OALMSGS(cond, exp)  ((void)FALSE)
#else
#define OALMSG(cond, exp)   ((void)((cond)?(OALLog exp), TRUE : FALSE))
#define OALMSGS(cond, exp)  ((void)((cond)?(OALLogSerial exp), TRUE : FALSE))
#endif

//------------------------------------------------------------------------------
//
//  Define:  OAL_LOG_XXX
//
//  OAL trace zone bit numbers
//

#define OAL_LOG_ERROR       (0)
#define OAL_LOG_WARN        (1)
#define OAL_LOG_FUNC        (2)
#define OAL_LOG_INFO        (3)
#define OAL_LOG_4           (4)         // avail
#define OAL_LOG_5           (5)         // avail
#define OAL_LOG_6           (6)         // avail
#define OAL_LOG_7           (7)         // avail
#define OAL_LOG_8           (8)         // avail
#define OAL_LOG_9           (9)         // avail
#define OAL_LOG_10          (10)        // avail
#define OAL_LOG_11          (11)        // avail
#define OAL_LOG_12          (12)        // avail
#define OAL_LOG_13          (13)        // avail
#define OAL_LOG_14          (14)        // avail
#define OAL_LOG_VERBOSE     (15)
#define OAL_LOG_STUB        (16)
#define OAL_LOG_KEYVAL      (17)
#define OAL_LOG_18          (18)        // avail
#define OAL_LOG_IO          (19)
#define OAL_LOG_CACHE       (20)
#define OAL_LOG_RTC         (21)
#define OAL_LOG_POWER       (22)
#define OAL_LOG_PCI         (23)
#define OAL_LOG_ARGS        (24)
#define OAL_LOG_MEMORY      (25)
#define OAL_LOG_IOCTL       (26)
#define OAL_LOG_TIMER       (27)
#define OAL_LOG_ETHER       (28)
#define OAL_LOG_FLASH       (29)
#define OAL_LOG_INTR        (30)
#define OAL_LOG_KITL        (31)

//------------------------------------------------------------------------------
//
//  Define:  OALZONE
//
//  Tests if a zone is being traced. Used in conditional in OALMSG macro.
//
#define OALZONE(n)          (g_oalLogMask&(1<<n))

//------------------------------------------------------------------------------
//
//  Define:  OAL_XXX
//
//  OAL trace zones for OALMSG function. The following trace zones may
//  be combined, via boolean operators, to support OAL debug tracing.
//
#define OAL_ERROR           OALZONE(OAL_LOG_ERROR)
#define OAL_WARN            OALZONE(OAL_LOG_WARN)
#define OAL_FUNC            OALZONE(OAL_LOG_FUNC)
#define OAL_INFO            OALZONE(OAL_LOG_INFO)

#define OAL_VERBOSE         OALZONE(OAL_LOG_VERBOSE)
#define OAL_STUB            OALZONE(OAL_LOG_STUB)
#define OAL_KEYVAL          OALZONE(OAL_LOG_KEYVAL)

#define OAL_IO              OALZONE(OAL_LOG_IO)
#define OAL_CACHE           OALZONE(OAL_LOG_CACHE)
#define OAL_RTC             OALZONE(OAL_LOG_RTC)
#define OAL_POWER           OALZONE(OAL_LOG_POWER)
#define OAL_PCI             OALZONE(OAL_LOG_PCI)
#define OAL_ARGS            OALZONE(OAL_LOG_ARGS)
#define OAL_MEMORY          OALZONE(OAL_LOG_MEMORY)
#define OAL_IOCTL           OALZONE(OAL_LOG_IOCTL)
#define OAL_TIMER           OALZONE(OAL_LOG_TIMER)
#define OAL_ETHER           OALZONE(OAL_LOG_ETHER)
#define OAL_FLASH           OALZONE(OAL_LOG_FLASH)
#define OAL_INTR            OALZONE(OAL_LOG_INTR)
#define OAL_KITL            OALZONE(OAL_LOG_KITL)

//------------------------------------------------------------------------------
//
//  Define: OALMASK
//
//  Utility macro used to setup OAL trace zones.
//
#define OALMASK(n)          (1 << n)

//------------------------------------------------------------------------------
//
//  Extern:  g_oalLogMask
//
//  Exports the global trace mask used by the OAL trace macros.
//
extern UINT32 g_oalLogMask;

//------------------------------------------------------------------------------
//
//  Function:  OALLogSetZones
//
//  This function is used to to send log zones.
//
void OALLogSetZones(UINT32 zones);

//------------------------------------------------------------------------------
//
//  Function:  OALLog
//
//  This function formats string and write it to debug output. We are using
//  kernel implementation. For boot loader this function is implemented in
//  support library.
//
#define OALLog          NKDbgPrintfW
#define OALLogV         NKvDbgPrintfW

//------------------------------------------------------------------------------
//
//  Function:  OALLogSerial
//
//  This function format string and write it to serial debug output. Function
//  implementation uses OALLogPrintf function to format string in buffer on
//  stack. Buffer size is limited and under some circumstances it can cause
//  stack overflow.
//
VOID OALLogSerial(LPCWSTR format, ...);

//------------------------------------------------------------------------------
//
//  Function:  OALLogPrintf
//
//  This function formats string to buffer. It uses standard format string
//  same as wsprintf function (which is identical to printf format without
//  float point support). We are using kernel implementation. For boot loader
//  this function is implemented in support library.
//
VOID OALLogPrintf(
    LPWSTR szBuffer, UINT32 maxChars, LPCWSTR szFormat, ...
);

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

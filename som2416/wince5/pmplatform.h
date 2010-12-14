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

//
// This module defines the platform power management customizations.
//
#ifndef _PM_PLATFORM_
#define _PM_PLATFORM_

//#define POWER_CAP_PARENT          0x00000001      // defined in pm.h

//
// These Flags define the meaning of POWER_CAPABILITIES.Flags field.
// They define the unit of measure and prefix reported in the Power field.
//
#define POWER_CAP_UNIT_WATTS        0x00000010
#define POWER_CAP_UNIT_AMPS         0x00000020

#define POWER_CAP_PREFIX_MILLI      0x00000040
#define POWER_CAP_PREFIX_MICRO      0x00000080
#define POWER_CAP_PREFIX_NAN0       0x00000100

#endif _PM_PLATFORM_


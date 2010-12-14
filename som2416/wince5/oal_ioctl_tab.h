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
//------------------------------------------------------------------------------
//
//  File:  oal_ioctl_tab.h
//
//  This file contains part of global IOCTL handler table for codes which
//  must (or should) be implemented on all platforms. Table in platform
//  will usually include this file.
//
//  This file is included by the platform's IOCTL table, g_oalIoCtlTable[].
//  Therefore, this file may ONLY define OAL_IOCTL_HANDLER entries. 
//
// IOCTL CODE,                          Flags   Handler Function
//------------------------------------------------------------------------------

{ IOCTL_HAL_TRANSLATE_IRQ,                  0,  OALIoCtlHalRequestSysIntr   },
{ IOCTL_HAL_REQUEST_SYSINTR,                0,  OALIoCtlHalRequestSysIntr   },
{ IOCTL_HAL_RELEASE_SYSINTR,                0,  OALIoCtlHalReleaseSysIntr   },
{ IOCTL_HAL_REQUEST_IRQ,                    0,  OALIoCtlHalRequestIrq       },

{ IOCTL_HAL_INITREGISTRY,                   0,  OALIoCtlHalInitRegistry     },
{ IOCTL_HAL_INIT_RTC,                       0,  OALIoCtlHalInitRTC          },
{ IOCTL_HAL_REBOOT,                         0,  OALIoCtlHalReboot           },

{ IOCTL_HAL_DDK_CALL,                       0,  OALIoCtlHalDdkCall          },

{ IOCTL_HAL_DISABLE_WAKE,                   0,  OALIoCtlHalDisableWake      },
{ IOCTL_HAL_ENABLE_WAKE,                    0,  OALIoCtlHalEnableWake       },
{ IOCTL_HAL_GET_WAKE_SOURCE,                0,  OALIoCtlHalGetWakeSource    },

{ IOCTL_HAL_GET_CACHE_INFO,                 0,  OALIoCtlHalGetCacheInfo     },
{ IOCTL_HAL_GET_DEVICEID,                   0,  OALIoCtlHalGetDeviceId      },
{ IOCTL_HAL_GET_DEVICE_INFO,                0,  OALIoCtlHalGetDeviceInfo    },
{ IOCTL_HAL_GET_UUID,                       0,  OALIoCtlHalGetUUID          },
{ IOCTL_PROCESSOR_INFORMATION,              0,  OALIoCtlProcessorInfo       },

{ IOCTL_VBRIDGE_802_3_MULTICAST_LIST,       0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_ADD_MAC,                    0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_CURRENT_PACKET_FILTER,      0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_GET_ETHERNET_MAC,           0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_GET_RX_PACKET,              0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_GET_RX_PACKET_COMPLETE,     0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_GET_TX_PACKET,              0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_GET_TX_PACKET_COMPLETE,     0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_SHARED_ETHERNET,            0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_WILD_CARD,                  0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_WILD_CARD_RESET_BUFFER,     0,  OALIoCtlVBridge             },
{ IOCTL_VBRIDGE_WILD_CARD_VB_INITIALIZED,   0,  OALIoCtlVBridge             },

//------------------------------------------------------------------------------

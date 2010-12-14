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
#ifndef __KITL_CFG_H
#define __KITL_CFG_H

//------------------------------------------------------------------------------
#include <bsp.h>
#include <oal_kitl.h>
#include <oal_ethdrv.h>

//------------------------------------------------------------------------------
// Serial kitl or Ethernet kitl 
//------------------------------------------------------------------------------
//#define KITL_SERIAL_UART0
//#define KITL_SERIAL_UART1
//#define KITL_USBSERIAL
//#define KITL_ETHERNET
//#define KITL_USBRNDIS

//------------------------------------------------------------------------------
// Kitl Serial UART 
//------------------------------------------------------------------------------
#ifdef KITL_SERIAL_UART0
//UART 0 use for serial kitl
#define KITL_SERIAL 1
#define UART_Kitl S3C2450_BASE_REG_PA_UART0
#elif KITL_SERIAL_UART1
//UART 1 use for serial kitl
#define KITL_SERIAL 1
#define UART_Kitl S3C2450_BASE_REG_PA_UART1
#else
#define UART_Kitl 0
#endif

#define BSP_UART_ULCON         0x03                // 8 bits, 1 stop, no parity
#define BSP_UART_UCON          0x0005              // pool mode, PCLK for UART
#define BSP_UART_UFCON         0x07                // disable FIFO
#define BSP_UART_UMCON         0x00                // disable auto flow control
#define BSP_UART_UBRDIV        (S3C2450_PCLK/(115200*16) - 1)






//------------------------------------------------------------------------------
// Kitl USB Serial/RNDIS
//------------------------------------------------------------------------------

//#define USBSER_KITL_POLL

// Lengths of the FIFOs for each endpoint

#define EP0Len          64
#define EP1Len          512
#define EP2Len 		512
#define EP3Len 		512
#define EP4Len          512


/* Define the configuration descriptor length
 */
#define CFGLEN 32
#define iCONF 18
#define TLEN   (CFGLEN + 18)

// Request Codes
#define GET_STATUS      0x00
#define CLEAR_FEATURE   0x01
#define SET_FEATURE     0x03
#define SET_ADDRESS     0x05
#define GET_DESCRIPTOR  0x06
#define SET_DESCRIPTOR  0x07
#define GET_CONFIG      0x08
#define SET_CONFIG      0x09
#define GET_INTERFACE   0x0a
#define SET_INTERFACE   0x0b

// Device specific request
#define SET_CONTROL_LINE_STATE  0x22

// Descriptor Types
#define DEVICE          0x01
#define CONFIGURATION   0x02
#define STRING          0x03
#define INTERFACE       0x04
#define ENDPOINT        0x05

//
// States we can be in
//
#define     KITLUSBSER_STATE_NONE           0
#define     KITLUSBSER_STATE_INIT           1
#define     KITLUSBSER_STATE_CONFIGURED     2
#define     KITLUSBSER_STATE_CONNECTED      3




// Extended descriptor values
#define CS_INTERFACE                0x24

//
// SetupPKG used for SETUP requests on EP0
//

typedef struct
{
    unsigned char bmRequest, bRequest;
    unsigned short wValue, wIndex, wLength;
} SetupPKG, *PSetupPKG;


// book keeping struct

typedef struct tagS3CUSBInfo
{
    SetupPKG  dReq;       // @field USB endpoint 0 command
    BYTE      dConfIdx;   // @field USB Configuration Index
    BYTE      dInterface; // @field USB Interface Index
    BYTE      dSetting;   // @field USB Setting Index
    BYTE      dAddress;   // @field USB device Address

} S3CUSB_INFO, *PS3CUSB_INFO;

//
// record-keeping structure
//
typedef struct tagUSBSERKITLInfo
{
    DWORD           dwState;
    KITLTRANSPORT   KitlTransport;
    DWORD           dwModemStatus;

} USBSERKITL_INFO, *PUSBSERKITL_INFO;



/*
 * Portable mechanism for writing bitfields in IO register structures
 */
#define IOW_REG_SET(_type,_ptr,_value)                    \
        {                            \
        *(volatile unsigned char*)(_ptr) = _value;            \
        }
#define IOW_REG_OR(_type,_ptr,_value)                    \
        {                            \
        *(volatile unsigned char*)(_ptr) |= _value;            \
        }
#define IOW_REG_AND(_type,_ptr,_value)                    \
        {                            \
        *(volatile unsigned char*)(_ptr) &= _value;            \
        }
#define IOW_REG_FIELD(_type,_ptr,_field,_value)                       \
        {                            \
        register union {                    \
          _type s;                        \
          unsigned char d;                    \
        } foo;                            \
        foo.d = *(volatile unsigned char*)(_ptr);            \
        foo.s._field = _value;                    \
        *(volatile unsigned char*)(_ptr) = foo.d;            \
        }
#define IOW_REG_BITSET(_type,_ptr,_field,_value)                   \
        {                            \
        register union {                    \
          _type s;                        \
          unsigned char d;                    \
        } foo;                            \
        foo.d = 0;                        \
        foo.s._field = _value;                    \
        *(volatile unsigned char*)(_ptr) = foo.d;            \
        }
#define IOW_REG_GET(_type,_ptr,_field,_value)                \
        {                            \
        register union {                    \
          _type s;                        \
          unsigned char d;                    \
        } foo;                            \
        foo.d = *(volatile unsigned char*)(_ptr);            \
        _value = foo.s._field;                    \
        }

#define UDC_REG_WRITE(_struct,_ptr,_field,_val) IOW_REG_FIELD(_struct,_ptr,_field,_val)

#define UDC_REG_BITSET(_struct,_ptr,_field,_val) { _struct xx;														\
  					 	   *(unsigned char *)&xx = 0;										\
  						   xx._field = _val;												\
  						   IOW_REG_SET(_struct,_ptr,*(unsigned char*)&xx); }

#define UDC_REG_WRITEX(_setptr,_setval)		(_setptr) = _setval;

//------------------------------------------------------------------------------
// Kitl ethernet cs8900a 
//------------------------------------------------------------------------------

//#define CS8900A_KITL_POLLMODE
//#define CS8900A_KITL_DHCP

#define CS8900A_MAC				L"11:22:33:44:55:66"
#define CS8900A_IP_ADDRESS		L"192.168.1.12"
#define CS8900A_IP_MASK			L"255.255.255.0"
#define CS8900A_IP_ROUTER		L"192.168.1.1"

//------------------------------------------------------------------------------

#define USBRNDIS_MAC				L"11:22:33:44:55:66"
#define USBRNDIS_IP_ADDRESS		L"192.168.1.12"
#define USBRNDIS_IP_MASK			L"255.255.255.0"
#define USBRNDIS_IP_ROUTER		L"192.168.1.1"

#endif

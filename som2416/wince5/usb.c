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

#include <windows.h>
#include <halether.h>
#include <bsp.h>
#include "loader.h"
 //[david.modify] 2008-05-08 17:54
#include "eboot_inc_david.h"



//[david.modify] 2008-06-14 17:02
// 将缓冲地址调整到16M处，方便下载大于32M的NK.BIN 
//============================
//#define DMABUFFER	0x32000000
#define DMABUFFER	0x31000000
//============================
#define USBDEV_BASE 0xB0B00000		// VIRTUAL Address
#define pISR		(*(volatile unsigned *)(0x30000000+0x18))		// Virtual Address 0x0 is mapped to 0x30000000, ISR Address is VA 0x18


extern  void delayLoop(int count);

#define Outp32(addr, data) (*(volatile UINT32 *)(addr) = (data))
#define Outp16(addr, data) (*(volatile UINT16 *)(addr) = (data))
#define Outp8(addr, data)  (*(volatile UINT8 *)(addr) = (data))
#define Inp32(addr, data) (data = (*(volatile UINT32 *)(addr)))
#define Inp16(addr, data) (data = (*(volatile UINT16 *)(addr)))
#define Inp8(addr, data)  (data = (*(volatile UINT16 *)(addr)))
#define Input32(addr) (*(volatile UINT32 *)(addr))
typedef enum 
{
	INDEX_REG               = (USBDEV_BASE+0x00), // Index register
	EP_INT_REG              = (USBDEV_BASE+0x04), // EP Interrupt pending and clear
	EP_INT_EN_REG           = (USBDEV_BASE+0x08), // EP Interrupt enable
	FUNC_ADDR_REG           = (USBDEV_BASE+0x0c), // Function address
	FRAME_NUM_REG           = (USBDEV_BASE+0x10), // Frame number
	EP_DIR_REG              = (USBDEV_BASE+0x14), // Endpoint direction
	TEST_REG                = (USBDEV_BASE+0x18), // Test register
	SYS_STATUS_REG          = (USBDEV_BASE+0x1c), // System status
	SYS_CON_REG             = (USBDEV_BASE+0x20), // System control
	EP0_STATUS_REG          = (USBDEV_BASE+0x24), // Endpoint 0 status
	EP0_CON_REG             = (USBDEV_BASE+0x28), // Endpoint 0 control
	EP_STATUS_REG           = (USBDEV_BASE+0x2c), // Endpoints status
	EP_CON_REG              = (USBDEV_BASE+0x30), // Endpoints control
	BYTE_READ_CNT_REG       = (USBDEV_BASE+0x34), // read count
	BYTE_WRITE_CNT_REG      = (USBDEV_BASE+0x38), // write count
	MAX_PKT_REG             = (USBDEV_BASE+0x3c), // Max packet size
	DMA_CON_REG             = (USBDEV_BASE+0x40), // DMA control
	DMA_CNT_REG             = (USBDEV_BASE+0x44), // DMA count
	DMA_FIFO_CNT_REG        = (USBDEV_BASE+0x48), // DMA FIFO count
	DMA_TOTAL_CNT1_REG      = (USBDEV_BASE+0x4c), // DMA Total count1
	DMA_TOTAL_CNT2_REG      = (USBDEV_BASE+0x50), // DMA Total count2
	DMA_IF_CON_REG          = (USBDEV_BASE+0x84), // DMA interface Control
	DMA_MEM_BASE_ADDR       = (USBDEV_BASE+0x88), // Mem Base Addr
	DMA_MEM_CURRENT_ADDR    = (USBDEV_BASE+0x8c), // Mem current Addr
	EP0_FIFO                = (USBDEV_BASE+0x60), // Endpoint 0 FIFO
	EP1_FIFO                = (USBDEV_BASE+0x64), // Endpoint 1 FIFO
	EP2_FIFO                = (USBDEV_BASE+0x68), // Endpoint 2 FIFO
	EP3_FIFO                = (USBDEV_BASE+0x6c), // Endpoint 3 FIFO
	EP4_FIFO                = (USBDEV_BASE+0x70), // Endpoint 4 FIFO
	FCON                    = (USBDEV_BASE+0x100) // Burst Fifo Control
 } USBD20_REGS ;


// Descriptor Types
typedef enum 
{
	DEVICE_TYPE_ = 0x1,
	CONFIGURATION_TYPE,
	STRING_TYPE,
	INTERFACE_TYPE_,
	ENDPOINT_TYPE
} DESC_TYPE;

// configuration descriptor: bmAttributes
typedef enum 
{
	CONF_ATTR_DEFAULT       = 0x80, // Spec 1.0 it was BUSPOWERED bit.
	CONF_ATTR_REMOTE_WAKEUP = 0x20,
	CONF_ATTR_SELFPOWERED   = 0x40
} DESC_CONF;

// endpoint descriptor
typedef enum 
{
	EP_ADDR_IN              = 0x80,
	EP_ADDR_OUT             = 0x00,

	EP_ATTR_CONTROL         = 0x0,
	EP_ATTR_ISOCHRONOUS     = 0x1,
	EP_ATTR_BULK            = 0x2,
	EP_ATTR_INTERRUPT       = 0x3
} DESC_ENDPT;

typedef enum 
{
	EP0, EP1, EP2, EP3, EP4
} EP_INDEX;

typedef enum  
{
	USB_FULL, USB_HIGH
} USB_SPEED;

typedef enum 
{
	USB_CPU, USB_DMA
} USB_OP;

typedef struct USB_DEVICE_DESCRIPTOR
{
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT8 bcdUSBL;
	UINT8 bcdUSBH;
	UINT8 bDeviceClass;
	UINT8 bDeviceSubClass;
	UINT8 bDeviceProtocol;
	UINT8 bMaxPacketSize0;
	UINT8 idVendorL;
	UINT8 idVendorH;
	UINT8 idProductL;
	UINT8 idProductH;
	UINT8 bcdDeviceL;
	UINT8 bcdDeviceH;
	UINT8 iManufacturer;
	UINT8 iProduct;
	UINT8 iSerialNumber;
	UINT8 bNumConfigurations;
} USB_DEVICE_DESCRIPTOR;

typedef struct DEVICE_REQUEST
{
	UINT8 bmRequestType;  
	UINT8 bRequest;       
	UINT8 wValue_L;       
	UINT8 wValue_H;       
	UINT8 wIndex_L;       
	UINT8 wIndex_H;       
	UINT8 wLength_L;      
	UINT8 wLength_H;      
} DEVICE_REQUEST;

typedef struct USB_CONFIGURATION_DESCRIPTOR
{
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT8 wTotalLengthL;
	UINT8 wTotalLengthH;
	UINT8 bNumInterfaces;
	UINT8 bConfigurationValue;
	UINT8 iConfiguration;
	UINT8 bmAttributes;
	UINT8 maxPower;
} USB_CONFIGURATION_DESCRIPTOR;

typedef struct USB_INTERFACE_DESCRIPTOR
{
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT8 bInterfaceNumber;
	UINT8 bAlternateSetting;
	UINT8 bNumEndpoints;
	UINT8 bInterfaceClass;
	UINT8 bInterfaceSubClass;
	UINT8 bInterfaceProtocol;
	UINT8 iInterface;
} USB_INTERFACE_DESCRIPTOR;

typedef struct USB_ENDPOINT_DESCRIPTOR
{
	UINT8 bLength;
	UINT8 bDescriptorType;
	UINT8 bEndpointAddress;
	UINT8 bmAttributes;
	UINT8 wMaxPacketSizeL;
	UINT8 wMaxPacketSizeH;
	UINT8 bInterval;
} USB_ENDPOINT_DESCRIPTOR;

// EP0 CSR register Bits
#define EP0_SENT_STALL              (0x01<<4)
#define EP0_DATA_END                (0x01<<3)
#define EP0_SETUP_END               (0x03<<2)
#define EP0_TX_SUCCESS              (0x01<<1)
#define EP0_RX_SUCCESS              (0x01<<0)

//  Defines for Endpoint CSR Register Bits
#define DMA_TOTAL_COUNT_ZERO        (0x1<<9)
#define EP_FIFO_FLUSH               (0x1<<6)
#define EP_SENT_STALL               (0x1<<5)
#define EP_TX_SUCCESS               (0x1<<1)
#define EP_RX_SUCCESS               (0x1<<0)


#define EP0_STATE_INIT              (0)
#define EP0_STATE_GD_DEV_0          (11)
#define EP0_STATE_GD_DEV_1          (12)
#define EP0_STATE_GD_DEV_2          (13)
#define EP0_STATE_GD_CFG_0          (21)
#define EP0_STATE_GD_CFG_1          (22)
#define EP0_STATE_GD_CFG_2          (23)
#define EP0_STATE_GD_CFG_3          (24)
#define EP0_STATE_GD_CFG_4          (25)
#define EP0_STATE_GD_CFG_ONLY_0     (41)
#define EP0_STATE_GD_CFG_ONLY_1     (42)
#define EP0_STATE_GD_IF_ONLY_0      (44)
#define EP0_STATE_GD_IF_ONLY_1      (45)
#define EP0_STATE_GD_EP0_ONLY_0     (46)
#define EP0_STATE_GD_EP1_ONLY_0     (47)
#define EP0_STATE_GD_EP2_ONLY_0     (48)
#define EP0_STATE_GD_EP3_ONLY_0     (49)
#define EP0_STATE_GD_STR_I0         (30)
#define EP0_STATE_GD_STR_I1         (31)
#define EP0_STATE_GD_STR_I2         (32)
#define EP0_STATE_GD_DEV_QUALIFIER  (33)
#define EP0_INTERFACE_GET           (34)


#define EP0_GET_STATUS0             (35)
#define EP0_GET_STATUS1             (36)
#define EP0_GET_STATUS2             (37)
#define EP0_GET_STATUS3             (38)
#define EP0_GET_STATUS4             (39)

// SPEC1.1
// Standard bmRequestType (Type)
#define STANDARD_TYPE               0x00
#define CLASS_TYPE                  0x20
#define VENDOR_TYPE                 0x40
#define RESERVED_TYPE               0x60

// Standard bmRequestType (Recipient)
// #define DEVICE_bmREQUEST_RECIPIENT(oDeviceRequest)  ((m_poDeviceRequest->bmRequestType) & 0x07)
#define DEVICE_RECIPIENT            0
#define INTERFACE_RECIPIENT         1
#define ENDPOINT_RECIPIENT          2
#define OTHER_RECIPIENT             3

// Standard bRequest codes
#define STANDARD_GET_STATUS         0
#define STANDARD_CLEAR_FEATURE      1
#define STANDARD_RESERVED_1         2
#define STANDARD_SET_FEATURE        3
#define STANDARD_RESERVED_2         4
#define STANDARD_SET_ADDRESS        5
#define STANDARD_GET_DESCRIPTOR     6
#define STANDARD_SET_DESCRIPTOR     7
#define STANDARD_GET_CONFIGURATION  8
#define STANDARD_SET_CONFIGURATION  9
#define STANDARD_GET_INTERFACE      10
#define STANDARD_SET_INTERFACE      11
#define STANDARD_SYNCH_FRAME        12

// Descriptor types
#define DEVICE_DESCRIPTOR           1
#define CONFIGURATION_DESCRIPTOR    2
#define STRING_DESCRIPTOR           3
#define INTERFACE_DESCRIPTOR        4
#define ENDPOINT_DESCRIPTOR         5
#define DEVICE_QUALIFIER            6
#define OTHER_SPEED_CONFIGURATION   7

// string descriptor
#define LANGID_US_L                 (0x09)
#define LANGID_US_H                 (0x04)

// USB Endpoints states
#define EP0_STATE_IDLE              (0)
#define EP0_STATE_TRANSFER          (1)
#define EP0_STATE_RECEIVER          (2)

#define BULK_OUT_STATUS_NOSTALL     (0x0000)
#define BULK_OUT_STATUS_STALL       (0x0001)

#define DEVICE_STATUS_DEFAULT       (0x0000)
#define DEVICE_STATUS_SELFPOWERED   (0x0001)
#define DEVICE_STATUS_REMOTEWAKEUP  (0x0002)


#define DEVICE_DESC_SIZE            18
#define STRING_DESC0_SIZE           4
#define STRING_DESC1_SIZE           22
#define STRING_DESC2_SIZE           44
#define CONFIG_DESC_TOTAL_SIZE      32
#define CONFIG_DESC_SIZE            9
#define INTERFACE_DESC_SIZE         9
#define ENDPOINT_DESC_SIZE          7
#define DEVICE_QUALIFIER_SIZE       10
#define OTHER_SPEED_CONFIGURATION_SIZE 9

// INT_REG status value
#define INT_ERR                     (0xff80)
#define INT_REG_ERROR               (0xff1<<6)
#define INT_REG_VBUS                (0x1<<8)
#define INT_REG_VBUS_CLEAR          (0x1<<6)
#define INT_REG_HSP                 (0x1<<4)
#define INT_REG_SDE                 (0x1<<3)
#define INT_REG_RESET               (0x1)
#define INT_REG_RESUME              (0x1<<2)
#define INT_REG_SUSPEND             (0x1<<1)
#define INT_REG_EP4                 (0x1<<4)
#define INT_REG_EP3                 (0x1<<3)
#define INT_REG_EP2                 (0x1<<2)
#define INT_REG_EP1                 (0x1<<1)
#define INT_REG_EP0                 (0x1)
#define INT_DTB_MISMATCH            (0x1FF<<7)

// USB Dma Operation
#define DMA_AUTO_RX_DISABLE         (0x1<<5)
#define DMA_FLY_ENABLE              (0x1<<4)
#define DMA_FLY_DISABLE             (0x0<<4)
#define DMA_DEMEND_ENABLE           (0x1<<3)
#define DMA_DEMEND_DISABLE          (0x0<<3)
#define DMA_TX_START                (0x1<<2)
#define DMA_TX_STOP                 (0x0<<2)
#define DMA_RX_START                (0x1<<1)
#define DMA_RX_STOP                 (0x0<<1)
#define USB_DMA_MODE                (0x1<<0)
#define USB_INT_MODE                (0x0<<0)

#define MAX_BURST_INCR16            (0x3<<0)
#define MAX_BURST_INCR8             (0x2<<0)
#define MAX_BURST_INCR4             (0x1<<0)

#define DMA_ENABLE                  (0x1<<8)
#define DMA_DISABLE                 (0x0<<8)


const UINT8 aDeviceQualifierDescriptor[] =
{
	0x0a,                   //  0 desc size
	0x06,                   //  1 desc type (DEVICE)
	0x00,                   //  2 USB release
	0x02,                   //  3 => 2.00
	0x00,                   //  4 class
	0x00,                   //  5 subclass
	0x00,                   //  6 protocol
	64,          			//  7 max pack size
	0x01,                   //  8 number of other-speed configuration
	0x00,                   //  9 reserved
};


const UINT8 aDescStr0[]=
{
	4, STRING_DESCRIPTOR, LANGID_US_L, LANGID_US_H, // codes representing languages
};

const UINT8 aDescStr1[]= // Manufacturer
{
	(0x14+2), STRING_TYPE,
	'S', 0x0, 'y', 0x0, 's', 0x0, 't', 0x0, 'e', 0x0, 'm', 0x0, ' ', 0x0, 'M', 0x0,
	'C', 0x0, 'U', 0x0,
};

const UINT8 aDescStr2[]= // Product
{
	(0x2a+2), STRING_TYPE,
	'S', 0x0, 'E', 0x0, 'C', 0x0, ' ', 0x0, 'S', 0x0, '3', 0x0, 'C', 0x0, '2', 0x0,
	'4', 0x0, '4', 0x0, '3', 0x0, 'X', 0x0, ' ', 0x0, 'T', 0x0, 'e', 0x0, 's', 0x0,
	't', 0x0, ' ', 0x0, 'B', 0x0, '/', 0x0, 'D', 0x0
};



UINT32  				g_uEp0MaxPktSize;
UINT32 	 				g_uEp1MaxPktSize;
UINT32					g_uEp3MaxPktSize;
UINT32					g_uEp0State;
USB_SPEED 				g_eSpeed;
USB_DEVICE_DESCRIPTOR 	g_oDescDevice;
DEVICE_REQUEST 			g_oDeviceRequest;
UINT32					g_uBulkInAddr;
UINT32					g_uBulkInCount;
UINT32					g_uDownloadFileSize;
UINT32				  	g_uDownloadAddress = DMABUFFER;
volatile UINT32 					readPtIndex;
volatile UINT8					*g_pDownPt;
UINT32					g_uEp0SubState;

#define		g_eOpMode 	USB_CPU


typedef struct USB_GET_STATUS
{
	UINT8 Device;
	UINT8 Interface;
	UINT8 Endpoint0;
	UINT8 Endpoint1;
	UINT8 Endpoint3;
} USB_GET_STATUS;

static USB_GET_STATUS 			oStatusGet;


typedef struct USB_INTERFACE_GET
{
	UINT8 AlternateSetting;
} USB_INTERFACE_GET;

static USB_INTERFACE_GET 		oInterfaceGet;

typedef struct USB_DESCRIPTORS
{
	USB_CONFIGURATION_DESCRIPTOR oDescConfig;
	USB_INTERFACE_DESCRIPTOR oDescInterface;
	USB_ENDPOINT_DESCRIPTOR oDescEndpt1;
	USB_ENDPOINT_DESCRIPTOR oDescEndpt3;
} USB_DESCRIPTORS;

USB_DESCRIPTORS oDesc;

void Isr_Init(void);
void IsrUsbd(unsigned int val);

#ifdef _EBOOT_SLEEP_
void IsrPowerButton(void);
#endif


void IsrHandler(void);
void SetEndpoint(void);

void WrPktEp1(UINT8 *buf, int num)
{
	int i;
	UINT16 Wr_Data=0;

	if (num&0x1) num++;
	for(i=0;i<num;i+=2)
	{
		Wr_Data=((*(buf+1))<<8)|*buf;
	   Outp32(EP1_FIFO, Wr_Data);
	   buf +=2;
	}
}

void RdPktEp3(UINT8 *buf, int num)
{
	int i;
	UINT16 Rdata;

	for(i=0;i<num;i+=2)
	{
		Inp32(EP3_FIFO, Rdata);
		buf[i] = (UINT8)Rdata;
		buf[i+1] = (UINT8)(Rdata>>8);
	}

	g_pDownPt += num;
}



void WrPktEp0(UINT8 *buf, int num)
{
	int i;
	UINT16 Wr_Data=0;

	if (num&0x1) num++;
	for(i=0;i<num;i+=2)
	{
		Wr_Data=((*(buf+1))<<8)|*buf;
		Outp32(EP0_FIFO, Wr_Data);
		buf +=2;
	}
}


void PrintEp0Pkt(UINT8 *pt, UINT8 count)
{
	int i;
	//EdbgOutputDebugString("[DBG:");
	for(i=0;i<count;i++);
		//EdbgOutputDebugString("%x,", pt[i]);
	//EdbgOutputDebugString("]");
}


void SetDescriptorTable(void)
{	
	// Standard device descriptor
	g_oDescDevice.bLength=0x12;	// EP0_DEV_DESC_SIZE=0x12 bytes
	g_oDescDevice.bDescriptorType=DEVICE_TYPE_;
	g_oDescDevice.bDeviceClass=0xFF; // 0x0
	g_oDescDevice.bDeviceSubClass=0x0;
	g_oDescDevice.bDeviceProtocol=0x0;
	g_oDescDevice.bMaxPacketSize0=g_uEp0MaxPktSize;
	g_oDescDevice.idVendorL=0xe8;
	g_oDescDevice.idVendorH=0x04;	
//	g_oDescDevice.idVendorL=0x45;
//	g_oDescDevice.idVendorH=0x53;
	g_oDescDevice.idProductL=0x34;
	g_oDescDevice.idProductH=0x12;
	g_oDescDevice.bcdDeviceL=0x00;
	g_oDescDevice.bcdDeviceH=0x01;
	g_oDescDevice.iManufacturer=0x1; // index of string descriptor
	g_oDescDevice.iProduct=0x2;	// index of string descriptor
	g_oDescDevice.iSerialNumber=0x0;
	g_oDescDevice.bNumConfigurations=0x1;
	
	if (g_eSpeed == USB_FULL) {
		g_oDescDevice.bcdUSBL=0x10;
		g_oDescDevice.bcdUSBH=0x01; 	// Ver 1.10
	}
	else {
		g_oDescDevice.bcdUSBL=0x00;
		g_oDescDevice.bcdUSBH=0x02; 	// Ver 2.0
	}

	// Standard configuration descriptor
	oDesc.oDescConfig.bLength=0x9;
	oDesc.oDescConfig.bDescriptorType=CONFIGURATION_TYPE;
	oDesc.oDescConfig.wTotalLengthL=0x20; // <cfg desc>+<if desc>+<endp0 desc>+<endp1 desc>
	oDesc.oDescConfig.wTotalLengthH=0;
	oDesc.oDescConfig.bNumInterfaces=1;
// dbg    descConf.bConfigurationValue=2; // why 2? There's no reason.
	oDesc.oDescConfig.bConfigurationValue=1;
	oDesc.oDescConfig.iConfiguration=0;
	oDesc.oDescConfig.bmAttributes=CONF_ATTR_DEFAULT; // bus powered only.
	oDesc.oDescConfig.maxPower=25; // draws 50mA current from the USB bus.

	// Standard interface descriptor
	oDesc.oDescInterface.bLength=0x9;
	oDesc.oDescInterface.bDescriptorType=INTERFACE_TYPE_;
	oDesc.oDescInterface.bInterfaceNumber=0x0;
	oDesc.oDescInterface.bAlternateSetting=0x0; // ?
	oDesc.oDescInterface.bNumEndpoints=2;	// # of endpoints except EP0
	oDesc.oDescInterface.bInterfaceClass=0xff; // 0x0 ?
	oDesc.oDescInterface.bInterfaceSubClass=0x0;
	oDesc.oDescInterface.bInterfaceProtocol=0x0;
	oDesc.oDescInterface.iInterface=0x0;

	// Standard endpoint0 descriptor
	oDesc.oDescEndpt1.bLength=0x7;
	oDesc.oDescEndpt1.bDescriptorType=ENDPOINT_TYPE;
	oDesc.oDescEndpt1.bEndpointAddress=1|EP_ADDR_IN; // 2400Xendpoint 1 is IN endpoint.
	oDesc.oDescEndpt1.bmAttributes=EP_ATTR_BULK;
	oDesc.oDescEndpt1.wMaxPacketSizeL=(UINT8)g_uEp1MaxPktSize; // 64
	oDesc.oDescEndpt1.wMaxPacketSizeH=(UINT8)(g_uEp1MaxPktSize>>8);
	oDesc.oDescEndpt1.bInterval=0x0; // not used

	// Standard endpoint1 descriptor
	oDesc.oDescEndpt3.bLength=0x7;
	oDesc.oDescEndpt3.bDescriptorType=ENDPOINT_TYPE;
	oDesc.oDescEndpt3.bEndpointAddress=3|EP_ADDR_OUT; // 2400X endpoint 3 is OUT endpoint.
	oDesc.oDescEndpt3.bmAttributes=EP_ATTR_BULK;
	oDesc.oDescEndpt3.wMaxPacketSizeL=(UINT8)g_uEp3MaxPktSize; // 64
	oDesc.oDescEndpt3.wMaxPacketSizeH=(UINT8)(g_uEp3MaxPktSize>>8);
	oDesc.oDescEndpt3.bInterval=0x0; // not used
}

BOOL InitUSB()
{
	int i=0;
	stGPIOInfo stGPIOInfo[]={
	{ PWREN_USB,  1, ALT_FUNC_OUT, PULL_UPDOWN_DISABLE},		
	};
	volatile S3C2450_CLKPWR_REG *s2450PWR = (S3C2450_CLKPWR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);
	volatile S3C2450_IOPORT_REG *s2450IOP = (S3C2450_IOPORT_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	//pUSBCtrlAddr = (S3C2450_USBD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_USBD, FALSE);


//[david.modify] 2008-05-08 17:44
// 打开USB电源
	i = 0;
	DPNOK(0);
	SetGPIOInfo(&stGPIOInfo[i],  s2450IOP);
	Print_IO_USB(s2450IOP);

//GPH14 在开发板上为USB_POWER ,我们使用GPB4
/*
    s2450IOP->GPHCON = (s2450IOP->GPHCON & ~(0x3<<28)) | (0x1<<28);
    s2450IOP->GPHUDP = (s2450IOP->GPHUDP & ~(0x3<<28));
    s2450IOP->GPHDAT = (s2450IOP->GPHDAT & ~(0x1<<14)) | (0x1<<14);
*/    

	s2450IOP->MISCCR=s2450IOP->MISCCR&~(1<<12);  // USBD is 0 ,normal mode ,1 is suspend mode /


	s2450PWR->PWRCFG |= (0x1<<4); // phy power enable 
//USB device 2.0 must reset like bellow , 1st phy reset and after at least 10us, func_reset & host reset     
//phy reset can reset bellow registers.
	s2450PWR->USB_RSTCON = (0x0<<2)|(0x0<<1)|(0x1<<0);//Function 2.0 S/W reset, Host 1.1 S/W reset,PHY 2.0 S/W reset
	delayLoop(1000000); // phy reset must be asserted for at 10us 
	s2450PWR->USB_RSTCON = (0x1<<2)|(0x1<<1)|(0x0<<0);//Function 2.0 S/W reset, Host 1.1 S/W reset,PHY 2.0 S/W reset
   	s2450PWR->USB_RSTCON = (0x0<<2)|(0x0<<1)|(0x0<<0);//Function 2.0 S/W reset, Host 1.1 S/W reset,PHY 2.0 S/W reset

#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1	
	s2450PWR->USB_PHYCTRL = (0<<3)|(1<<2)|(0<<1)|(0<<0);
#else
	s2450PWR->USB_PHYCTRL = (0<<3)|(0<<2)|(0<<1)|(0<<0);
#endif
#elif (BSP_TYPE == BSP_SMDK2450)
 //[david.modify] 2008-05-08 17:28
// 我们使用12MHZ晶振替换SMDK2450的48MHZ晶振
//	s2450PWR->USB_PHYCTRL = (0<<3)|(1<<2)|(0<<1)|(0<<0);

// 有源晶振使用如下:Oscillator
//	s2450PWR->USB_PHYCTRL = (2<<3)|(1<<2)|(0<<1)|(0<<0);

// 无源晶振使用如下:Crystal (新板)
	s2450PWR->USB_PHYCTRL = (2<<3)|(0<<2)|(0<<1)|(0<<0);

#endif



    //s2450PWR->USB_PHYPWR = (0x1<<31)|(0x0<<4)|(0x0<<3)|(0x0<<2)|(0x0<<1)|(0x0<<0); 
    s2450PWR->USB_PHYPWR = (0x0<<31)|(0x3<<4)|(0x0<<3)|(0x0<<2)|(0x0<<1)|(0x0<<0);
    //48Mhz clock on ,PHY2.0 analog block power on,XO block power on,XO block power in suspend mode,PHY 2.0 Pll power on ,suspend signal for save mode disable 
    
	s2450PWR->USB_CLKCON = (0x1<<31)|(0x1<<2)|(0x0<<1)|(0x1<<0); // vbus detect enable...
	
    //D+ pull up , USB2.0 Function clock Enable, USB1.1 HOST disable,USB2.0 PHY test enable


  //rHCLKCON &= ~(0x1<<12); //
  //	s2450PWR->CLKDIV1|=0x1<<4; //  for test clk enable
  

	g_uEp0State = EP0_STATE_INIT;
	g_pDownPt = (UINT8 *)DMABUFFER;
	readPtIndex = DMABUFFER;
	SetDescriptorTable();

	// *** End point information ***

	SetEndpoint();

	return TRUE;
}


void SetEndpoint(void)
{
	// *** End point information ***
	// EP0: control
//	UINT16 SysStatus;
	UINT16 Temp;

	Outp32(INDEX_REG, EP0);
	// For L18
	Outp32(EP_DIR_REG, 0x02); 		// EP1=> TX, EP2=>RX , 0b=report mode[1], 15b=report mode[2], 3b~8b=ep2 delay_con
	Outp32(EP_INT_EN_REG, 0x4d0f); 	// EP0, 1, 2 Interrupt enable, 15b=report mode[0], 3b~14b=ep0/1 delay_con
	Inp32(EP_DIR_REG, Temp);
	//EdbgOutputDebugString("EP_DIR_REG : %x \n", Temp);
	Inp32(EP_INT_EN_REG, Temp);
	//EdbgOutputDebugString("EP_INT_EN_REG : %x \n", Temp);

	Outp32(TEST_REG, 0x0000);

	//Outp32(SYS_CON_REG, 0x0283);		// error interrupt enable, 16bit bus, Little format, suspend&reset enable
	//Outp32(SYS_CON_REG, 0x0023);		// error interrupt enable, 16bit bus, Little format, suspend&reset enable
	Outp32(SYS_CON_REG, 0x4123);		// error interrupt enable, 16bit bus, Little format, suspend&reset enable	
// 	Outp32(MAX_PKT_REG, MAXP_SIZE_64BYTE); // Initial矫 size?
	Outp32(EP0_CON_REG, 0x0000);


	// EP1 OUT Max Packet size settings
	Outp32(INDEX_REG, EP1);
// 	Outp32(MAX_PKT_REG, 512); 	// max packet size 512 bytes
//	Outp32(EP_CON_REG, 0x0000); // dual enable
	Outp32(EP_CON_REG, 0x0080); // dual enable

	// EP2 IN Max Packet size settings
	Outp32(INDEX_REG, EP3);
// 	Outp32(MAX_PKT_REG, 512);	// max packet size 512 bytes
	Outp32(EP_CON_REG, 0x0080);    		// dual enable

	Outp32(INDEX_REG, EP0);

}

void SetMaxPktSizes(USB_SPEED eSpeed)
{
	if (eSpeed == USB_HIGH)
	{
		g_eSpeed = USB_HIGH;
		g_uEp0MaxPktSize = 64;
		g_uEp1MaxPktSize = 256;
		g_uEp3MaxPktSize = 256;
	}
	else
	{
		g_eSpeed = USB_FULL;
		g_uEp0MaxPktSize = 8;
		g_uEp1MaxPktSize = 64;
		g_uEp3MaxPktSize = 64;
	}
	// EP0 Max Packet size settings
	Outp32(INDEX_REG, EP0);
	Outp32(MAX_PKT_REG, g_uEp0MaxPktSize); 	// max packet size

	// EP1 OUT Max Packet size settings
	Outp32(INDEX_REG, EP1);
	Outp32(MAX_PKT_REG, g_uEp1MaxPktSize); 	// max packet size

	// EP2 IN Max Packet size settings
	Outp32(INDEX_REG, EP3);
	Outp32(MAX_PKT_REG, g_uEp3MaxPktSize);	// max packet size
}

void TransferEp0(void)
{
//	UINT32 i;
//	UINT32 dataLength;
	UINT16 usSysStatus;

// 	CLR_EP0_CSR_OUT_PACKET_READY;
	switch (g_uEp0State)
	{
		case EP0_STATE_INIT:
			break;

		// === GET_DESCRIPTOR:DEVICE ===
		case EP0_STATE_GD_DEV_0:
			if (g_eSpeed == 1)
			{
				Outp32(BYTE_WRITE_CNT_REG, 18);
						WrPktEp0((UINT8 *)&g_oDescDevice+0, 18); // EP0_PKT_SIZE
						g_uEp0State = EP0_STATE_INIT;
				//EdbgOutputDebugString("EndpointZeroTransfer(EP0_STATE_GD_DEV)\n");
			}
			else
			{
				Outp32(BYTE_WRITE_CNT_REG, 8);
						WrPktEp0((UINT8 *)&g_oDescDevice+0, 8); // EP0_PKT_SIZE
						g_uEp0State = EP0_STATE_GD_DEV_1;
				//EdbgOutputDebugString("EndpointZeroTransfer(EP0_STATE_GD_DEV_0)\n");
			}
			break;

		case EP0_STATE_GD_DEV_1:
			Outp32(BYTE_WRITE_CNT_REG, 8);
					WrPktEp0((UINT8 *)&g_oDescDevice+8, 8); // EP0_PKT_SIZE
					g_uEp0State = EP0_STATE_GD_DEV_2;
			//EdbgOutputDebugString("EndpointZeroTransfer(EP0_STATE_GD_DEV_1)\n");
			break;

		case EP0_STATE_GD_DEV_2:
			Outp32(BYTE_WRITE_CNT_REG, 2);
					WrPktEp0((UINT8 *)&g_oDescDevice+16, 2); // EP0_PKT_SIZE
					g_uEp0State = EP0_STATE_INIT;
			//EdbgOutputDebugString("EndpointZeroTransfer(EP0_STATE_GD_DEV_2)\n");
			break;

		// === GET_DESCRIPTOR:CONFIGURATION+INTERFACE+ENDPOINT0+ENDPOINT1 ===
		// Windows98 gets these 4 descriptors all together by issuing only a request.
		// Windows2000 gets each descriptor seperately.
		// === GET_DESCRIPTOR:CONFIGURATION ONLY for WIN2K===

		case EP0_STATE_GD_CFG_0:
			if (g_eSpeed == 1)
			{
				Outp32(BYTE_WRITE_CNT_REG, 32);
				WrPktEp0((UINT8 *)&oDesc.oDescConfig+0, 32); // EP0_PKT_SIZE
				g_uEp0State = EP0_STATE_INIT;
				//EdbgOutputDebugString("EndpointZeroTransfer(EP0_STATE_GD_CFG)\n");
			}
			else
			{
				Outp32(BYTE_WRITE_CNT_REG, 8);
				WrPktEp0((UINT8 *)&oDesc.oDescConfig+0, 8); // EP0_PKT_SIZE
				g_uEp0State = EP0_STATE_GD_CFG_1;
				//EdbgOutputDebugString("EndpointZeroTransfer(EP0_STATE_GD_CFG_0)\n");
			}
			break;

		case EP0_STATE_GD_CFG_1:
			Outp32(BYTE_WRITE_CNT_REG, 8);
			WrPktEp0((UINT8 *)&oDesc.oDescConfig+8, 8); // EP0_PKT_SIZE	WrPktEp0((UINT8 *)&descConf+8, 1); WrPktEp0((UINT8 *)&descIf+0, 7);
			g_uEp0State = EP0_STATE_GD_CFG_2;
			break;

		case EP0_STATE_GD_CFG_2:
			Outp32(BYTE_WRITE_CNT_REG, 8);
			WrPktEp0((UINT8 *)&oDesc.oDescConfig+16, 8); // EP0_PKT_SIZE	WrPktEp0((UINT8 *)&descIf+7, 2); WrPktEp0((UINT8 *)&descEndpt0+0, 6);
			g_uEp0State = EP0_STATE_GD_CFG_3;
			break;

		case EP0_STATE_GD_CFG_3:
			Outp32(BYTE_WRITE_CNT_REG, 8);
			WrPktEp0((UINT8 *)&oDesc.oDescConfig+24, 8); // EP0_PKT_SIZE	WrPktEp0((UINT8 *)&descEndpt0+6, 1); WrPktEp0((UINT8 *)&descEndpt1+0, 7);
			g_uEp0State = EP0_STATE_GD_CFG_4;
			break;

		case EP0_STATE_GD_CFG_4:
			Outp32(BYTE_WRITE_CNT_REG, 0);
			g_uEp0State = EP0_STATE_INIT;
			break;

		// === GET_DESCRIPTOR:CONFIGURATION ONLY===
		case EP0_STATE_GD_CFG_ONLY_0:
			if (g_eSpeed == 1)
			{
				//EdbgOutputDebugString("[DBG : EP0_STATE_GD_CFG_ONLY]\n");
				Outp32(BYTE_WRITE_CNT_REG, 9);
				WrPktEp0((UINT8 *)&oDesc.oDescConfig+0, 9); // EP0_PKT_SIZE
				g_uEp0State = EP0_STATE_INIT;
			}
			else
			{
				//EdbgOutputDebugString("[DBG : EP0_STATE_GD_CFG_ONLY_0]\n");
				Outp32(BYTE_WRITE_CNT_REG, 8);
				WrPktEp0((UINT8 *)&oDesc.oDescConfig+0, 8); // EP0_PKT_SIZE
				g_uEp0State = EP0_STATE_GD_CFG_ONLY_1;
			}
			break;

		case EP0_STATE_GD_CFG_ONLY_1:
			//EdbgOutputDebugString("[DBG : EP0_STATE_GD_CFG_ONLY_1]\n");
			Outp32(BYTE_WRITE_CNT_REG, 1);
			WrPktEp0((UINT8 *)&oDesc.oDescConfig+8, 1); // EP0_PKT_SIZE
			g_uEp0State = EP0_STATE_INIT;
			break;

		// === GET_DESCRIPTOR:INTERFACE ONLY===

		case EP0_STATE_GD_IF_ONLY_0:
			if (g_eSpeed == 1)
			{
				Outp32(BYTE_WRITE_CNT_REG, 9);				// INTERFACE_DESC_SIZE
				WrPktEp0((UINT8 *)&oDesc.oDescInterface+0, 9);
				g_uEp0State = EP0_STATE_INIT;
			}
			else
			{
				Outp32(BYTE_WRITE_CNT_REG, 8);				// INTERFACE_DESC_SIZE
				WrPktEp0((UINT8 *)&oDesc.oDescInterface+0, 8);
				g_uEp0State = EP0_STATE_GD_IF_ONLY_1;
			}
			break;

		case EP0_STATE_GD_IF_ONLY_1:
			Outp32(BYTE_WRITE_CNT_REG, 1);
			WrPktEp0((UINT8 *)&oDesc.oDescInterface+8, 1);
			g_uEp0State = EP0_STATE_INIT;
			break;


		// === GET_DESCRIPTOR:ENDPOINT 1 ONLY===
		case EP0_STATE_GD_EP0_ONLY_0:
			Outp32(BYTE_WRITE_CNT_REG, ENDPOINT_DESC_SIZE);
			Inp32(SYS_STATUS_REG, usSysStatus);
			WrPktEp0((UINT8 *)&oDesc.oDescEndpt1+0, ENDPOINT_DESC_SIZE);
			g_uEp0State = EP0_STATE_INIT;
			break;

		// === GET_DESCRIPTOR:ENDPOINT 2 ONLY===
		case EP0_STATE_GD_EP1_ONLY_0:
			Outp32(BYTE_WRITE_CNT_REG, ENDPOINT_DESC_SIZE);
			Inp32(SYS_STATUS_REG, usSysStatus);
			WrPktEp0((UINT8 *)&oDesc.oDescEndpt3+0, ENDPOINT_DESC_SIZE);
			g_uEp0State = EP0_STATE_INIT;
			break;

				// === GET_DESCRIPTOR:STRING ===
		case EP0_STATE_GD_STR_I0:
			Outp32(BYTE_WRITE_CNT_REG, 4);
			//EdbgOutputDebugString("[GDS0_0]");
			WrPktEp0((UINT8 *)aDescStr0, 4);
			g_uEp0State = EP0_STATE_INIT;
			break;

		case EP0_STATE_GD_STR_I1:
			//EdbgOutputDebugString("[GDS1_%d]", g_uEp0SubState);
			if ((g_uEp0SubState*g_uEp0MaxPktSize+g_uEp0MaxPktSize)<sizeof(aDescStr1))
			{
				Outp32(BYTE_WRITE_CNT_REG, g_uEp0MaxPktSize);
				WrPktEp0((UINT8 *)aDescStr1+(g_uEp0SubState*g_uEp0MaxPktSize), g_uEp0MaxPktSize);
				g_uEp0State = EP0_STATE_GD_STR_I1;
				g_uEp0SubState++;
			}
			else
			{
				Outp32(BYTE_WRITE_CNT_REG, sizeof(aDescStr1)-(g_uEp0SubState*g_uEp0MaxPktSize));
				WrPktEp0((UINT8 *)aDescStr1+(g_uEp0SubState*g_uEp0MaxPktSize), sizeof(aDescStr1)-(g_uEp0SubState*g_uEp0MaxPktSize));
				g_uEp0State = EP0_STATE_INIT;
				g_uEp0SubState = 0;
			}
			break;

		case EP0_STATE_GD_STR_I2:
			//EdbgOutputDebugString("[GDS2_%d]", g_uEp0SubState);
			if ((g_uEp0SubState*g_uEp0MaxPktSize+g_uEp0MaxPktSize)<sizeof(aDescStr2))
			{
				Outp32(BYTE_WRITE_CNT_REG, g_uEp0MaxPktSize);
				WrPktEp0((UINT8 *)aDescStr2+(g_uEp0SubState*g_uEp0MaxPktSize), g_uEp0MaxPktSize);
				g_uEp0State = EP0_STATE_GD_STR_I2;
				g_uEp0SubState++;
			}
			else
			{
				//EdbgOutputDebugString("[E]");
				Outp32(BYTE_WRITE_CNT_REG, sizeof(aDescStr2)-(g_uEp0SubState*g_uEp0MaxPktSize));
				WrPktEp0((UINT8 *)aDescStr2+(g_uEp0SubState*g_uEp0MaxPktSize), sizeof(aDescStr2)-(g_uEp0SubState*g_uEp0MaxPktSize));
				g_uEp0State = EP0_STATE_INIT;
				g_uEp0SubState = 0;
			}
			break;

		case EP0_STATE_GD_DEV_QUALIFIER:
			Outp32(BYTE_WRITE_CNT_REG, 10);
			WrPktEp0((UINT8 *)aDeviceQualifierDescriptor+0, 10);
			g_uEp0State = EP0_STATE_INIT;
			break;

		case EP0_INTERFACE_GET:
			Outp32(BYTE_WRITE_CNT_REG, 1);
			WrPktEp0((UINT8 *)&oInterfaceGet+0, 1);
			g_uEp0State = EP0_STATE_INIT;
			break;


		case EP0_GET_STATUS0:
			Outp32(BYTE_WRITE_CNT_REG, 1);
			WrPktEp0((UINT8 *)&oStatusGet+0, 1);
			g_uEp0State = EP0_STATE_INIT;
			break;

		case EP0_GET_STATUS1:
			Outp32(BYTE_WRITE_CNT_REG, 1);
			WrPktEp0((UINT8 *)&oStatusGet+1, 1);
			g_uEp0State = EP0_STATE_INIT;
			break;

		case EP0_GET_STATUS2:
			Outp32(BYTE_WRITE_CNT_REG, 1);
			WrPktEp0((UINT8 *)&oStatusGet+2, 1);
			g_uEp0State = EP0_STATE_INIT;
			break;

		case EP0_GET_STATUS3:
			Outp32(BYTE_WRITE_CNT_REG, 1);
			WrPktEp0((UINT8 *)&oStatusGet+3, 1);
			g_uEp0State = EP0_STATE_INIT;
			break;

		case EP0_GET_STATUS4:
			Outp32(BYTE_WRITE_CNT_REG, 1);
			WrPktEp0((UINT8 *)&oStatusGet+4, 1);
			g_uEp0State = EP0_STATE_INIT;
			break;

		default:
			break;

	}
}

void HandleEvent_EP0(void)
{
	UINT32 DeviceRequestLength;
	UINT16 ep0csr;
	UINT16 ReadCnt, i;
	UINT16 ReadBuf[64]={0x0000, };
	UINT8  setaddress;
	UINT32  uRemoteWakeUp=0;
	UINT16  usConfig=0;

	Outp32(INDEX_REG, EP0);
	Inp32(EP0_STATUS_REG, ep0csr);

	//EdbgOutputDebugString(" Endpoint0 CSR Register = %x \n", ep0csr);
	// EP0 CSR register status check

	if (ep0csr & EP0_SENT_STALL) // SENT STALL : protocol stall.
	{
		//EdbgOutputDebugString(" Sent Stall \n");
		Outp32(EP0_STATUS_REG, EP0_SENT_STALL);
		if (ep0csr & EP0_RX_SUCCESS)
			Outp32(EP0_STATUS_REG, EP0_RX_SUCCESS);
		g_uEp0State = EP0_STATE_INIT;
		return;
	}

	if (ep0csr & EP0_TX_SUCCESS)
	{
		//EdbgOutputDebugString(" EP0_TX_SUCCESS \n");
		Outp32(EP0_STATUS_REG, EP0_TX_SUCCESS);
	}

	//EdbgOutputDebugString(" g_uEp0State = %x \n", g_uEp0State);

// 	if ((ep0csr & EP0_RX_SUCCESS) & (g_uEp0State == EP0_STATE_INIT))
	if (g_uEp0State == EP0_STATE_INIT)
	{
		Inp32(BYTE_READ_CNT_REG, ReadCnt);
		for(i=0;i<4;i++) Inp32(EP0_FIFO, ReadBuf[i]);

		Outp32(EP0_STATUS_REG, EP0_RX_SUCCESS);

		g_oDeviceRequest.bmRequestType=(UINT8)ReadBuf[0];
		g_oDeviceRequest.bRequest=ReadBuf[0]>>8;
		g_oDeviceRequest.wValue_L=(UINT8)ReadBuf[1];
		g_oDeviceRequest.wValue_H=ReadBuf[1]>>8;
		g_oDeviceRequest.wIndex_L=(UINT8)ReadBuf[2];
		g_oDeviceRequest.wIndex_H=ReadBuf[2]>>8;
		g_oDeviceRequest.wLength_L=(UINT8)ReadBuf[3];
		g_oDeviceRequest.wLength_H=ReadBuf[3]>>8;

		PrintEp0Pkt((UINT8 *)&g_oDeviceRequest, 8);

		switch (g_oDeviceRequest.bRequest)
		{
			case STANDARD_SET_ADDRESS:
				setaddress = (g_oDeviceRequest.wValue_L); // Set Address Update bit
				//EdbgOutputDebugString("Func_ADDR_Setaddr : %d \n", setaddress);
				g_uEp0State = EP0_STATE_INIT;
				break;

			case STANDARD_SET_DESCRIPTOR:
				//EdbgOutputDebugString("\n MCU >> Set Descriptor \n");
				break;

			case STANDARD_SET_CONFIGURATION:
	//			EdbgOutputDebugString("\n MCU >> Set Configuration \n");
				usConfig = g_oDeviceRequest.wValue_L; // Configuration value in configuration descriptor
	//			m_uEnumerationDone = 1;
				break;

			case STANDARD_GET_CONFIGURATION:
				// Uart_Printf("\n MCU >> Get Configruation \n");
				Outp32(BYTE_WRITE_CNT_REG, 1);
				Outp32(EP0_FIFO, usConfig);
				break;

			case STANDARD_GET_DESCRIPTOR:
				switch (g_oDeviceRequest.wValue_H)
				{
					case DEVICE_DESCRIPTOR:
						//EdbgOutputDebugString("\n MCU >> Get Device Descriptor \n");
						g_uEp0State = EP0_STATE_GD_DEV_0;
						break;

					case CONFIGURATION_DESCRIPTOR:
						//EdbgOutputDebugString("\n MCU >> Get Configuration Descriptor \n");

						DeviceRequestLength = (UINT32)((g_oDeviceRequest.wLength_H << 8) |
							g_oDeviceRequest.wLength_L);

						if (DeviceRequestLength > CONFIG_DESC_SIZE){
						// === GET_DESCRIPTOR:CONFIGURATION+INTERFACE+ENDPOINT0+ENDPOINT1 ===
						// Windows98 gets these 4 descriptors all together by issuing only a request.
						// Windows2000 gets each descriptor seperately.
						// m_uEpZeroTransferLength = CONFIG_DESC_TOTAL_SIZE;
// 							m_uEpZeroTransferdata = 0;
							g_uEp0State = EP0_STATE_GD_CFG_0;
						}
						else
							g_uEp0State = EP0_STATE_GD_CFG_ONLY_0; // for win2k
						break;

					case STRING_DESCRIPTOR :
						switch(g_oDeviceRequest.wValue_L)
						{
							case 0:
								g_uEp0State = EP0_STATE_GD_STR_I0;
								break;
							case 1:
								g_uEp0State = EP0_STATE_GD_STR_I1;
								break;
							case 2:
								g_uEp0State = EP0_STATE_GD_STR_I2;
								break;
							default:
									break;
						}
						break;
					case ENDPOINT_DESCRIPTOR:
						switch(g_oDeviceRequest.wValue_L&0xf)
						{
						case 0:
							g_uEp0State=EP0_STATE_GD_EP0_ONLY_0;
							break;
						case 1:
							g_uEp0State=EP0_STATE_GD_EP1_ONLY_0;
							break;
						default:
							break;
						}
						break;

					case DEVICE_QUALIFIER:
						//EdbgOutputDebugString("\n MCU >> Get Device Qualifier Descriptor \n");
						g_uEp0State = EP0_STATE_GD_DEV_QUALIFIER;
						break;
				}
				break;

			case STANDARD_CLEAR_FEATURE:
				//EdbgOutputDebugString("\n MCU >> Clear Feature \n");
				switch (g_oDeviceRequest.bmRequestType)
				{
					case DEVICE_RECIPIENT:
						if (g_oDeviceRequest.wValue_L == 1)
							uRemoteWakeUp = FALSE;
						break;

					case ENDPOINT_RECIPIENT:
						if (g_oDeviceRequest.wValue_L == 0)
						{
							if ((g_oDeviceRequest.wIndex_L & 0x7f) == 0x00)
								oStatusGet.Endpoint0= 0;

							if ((g_oDeviceRequest.wIndex_L & 0x8f) == 0x01) // IN  Endpoint 1
								oStatusGet.Endpoint1= 0;

							if ((g_oDeviceRequest.wIndex_L & 0x8f) == 0x03) // OUT Endpoint 3
								oStatusGet.Endpoint3= 0;
						}
						break;

					default:
						break;
				}
				g_uEp0State = EP0_STATE_INIT;
				break;

			case STANDARD_SET_FEATURE:
				//EdbgOutputDebugString("\n MCU >> Set Feature \n");
				switch (g_oDeviceRequest.bmRequestType)
				{
					case DEVICE_RECIPIENT:
						if (g_oDeviceRequest.wValue_L == 1)
							uRemoteWakeUp = TRUE;
							break;

					case ENDPOINT_RECIPIENT:
						if (g_oDeviceRequest.wValue_L == 0)
						{
							if ((g_oDeviceRequest.wIndex_L & 0x7f) == 0x00)
								oStatusGet.Endpoint0= 1;

							if ((g_oDeviceRequest.wIndex_L & 0x8f) == 0x01)
								oStatusGet.Endpoint1= 1;

							if ((g_oDeviceRequest.wIndex_L & 0x8f) == 0x03)
								oStatusGet.Endpoint3= 1;
						}
						break;

					default:
						break;
				}
				g_uEp0State = EP0_STATE_INIT;
				break;

			case STANDARD_GET_STATUS:
				switch(g_oDeviceRequest.bmRequestType)
				{
					case  (0x80):
						oStatusGet.Device=((UINT8)uRemoteWakeUp<<1)|0x1;		// SelfPowered
						g_uEp0State = EP0_GET_STATUS0;
						break;

					case  (0x81):
						oStatusGet.Interface=0;
						g_uEp0State = EP0_GET_STATUS1;
						break;

					case  (0x82):
						if ((g_oDeviceRequest.wIndex_L & 0x7f) == 0x00)
							g_uEp0State = EP0_GET_STATUS2;

						if ((g_oDeviceRequest.wIndex_L & 0x8f) == 0x01)
							g_uEp0State = EP0_GET_STATUS3;

						if ((g_oDeviceRequest.wIndex_L & 0x8f) == 0x03)
							g_uEp0State = EP0_GET_STATUS4;
						break;

					default:
						break;
				}
				break;

			case STANDARD_GET_INTERFACE:
				g_uEp0State = EP0_INTERFACE_GET;
				break;

			case STANDARD_SET_INTERFACE:
				oInterfaceGet.AlternateSetting= g_oDeviceRequest.wValue_L;
				g_uEp0State = EP0_STATE_INIT;
				break;

			case STANDARD_SYNCH_FRAME:
				g_uEp0State = EP0_STATE_INIT;
				break;

			default:
				break;
		}
	}

	TransferEp0();
}

void PrepareEp1Fifo(UINT32 BaseAddr)
{
//	int i;
//	UINT32 in_csr1;
	UINT8* BulkInBuf = (UINT8*)BaseAddr;

	if (g_uBulkInCount > g_uEp1MaxPktSize)
	{
		Outp32(INDEX_REG, EP1);
		Outp32(BYTE_WRITE_CNT_REG, g_uEp1MaxPktSize);
		WrPktEp1(BulkInBuf, g_uEp1MaxPktSize);

		g_uBulkInAddr = BaseAddr + g_uEp1MaxPktSize;
		g_uBulkInCount -= g_uEp1MaxPktSize;
	}
	else
	{
		Outp32(INDEX_REG, EP1);
		Outp32(BYTE_WRITE_CNT_REG, g_uBulkInCount);
		WrPktEp1(BulkInBuf, g_uBulkInCount);

		g_uBulkInAddr = BaseAddr + g_uBulkInCount;
		g_uBulkInCount = 0;
	}
}


void HandleEvent_BulkIn(void)
{
	UINT16 ep1csr;
	UINT32 temp;

	// EP1 CSR register status check
	Outp32(INDEX_REG, 0x01);
	Inp32(EP_STATUS_REG, ep1csr);
	Outp32(EP_STATUS_REG, ep1csr);
	Inp32(DMA_TOTAL_CNT1_REG, temp);
	//EdbgOutputDebugString("EP1: DMA_TOTAL_CNT1_REG : %x\n", temp);

	if (ep1csr & EP_SENT_STALL) { // SENT STALL : protocol stall.
		//EdbgOutputDebugString("Sent Stall \n");
		Outp32(EP_STATUS_REG, EP_SENT_STALL);
	}

	if (ep1csr & EP_TX_SUCCESS)
	{
		//EdbgOutputDebugString("EP1_TX_SUCCESS\n");
		Outp32(EP_STATUS_REG, EP_TX_SUCCESS); // Endpoint Status Register Clear
		if (g_eOpMode == USB_CPU)
		{
			if (g_uBulkInCount != 0)
				PrepareEp1Fifo(g_uBulkInAddr);
		}
	}

	if (DMA_TOTAL_COUNT_ZERO & ep1csr)
	{
		//EdbgOutputDebugString("USB_DMA_MODE, DMA TX Done(DMA_TOTAL_COUNT_ZERO) !!\n");
		Outp32(DMA_CON_REG, DMA_TX_STOP|USB_INT_MODE);
		Outp32(FCON, DMA_DISABLE);
	}
}
void HandleEvent_BulkOut(void)
{
//	UINT32 ReadData;
//	UINT32 RecDataCnt;
	UINT16 ep3csr;//, ep2con;//, sys_status;
	UINT32 CBWSignature=0, CBWTag=0;//, i;
	UINT16 fifoCnt;
	UINT16 fifoCntByte;
//	UINT32 DmaCurrAddr;
	UINT32 temp;

	Outp32(INDEX_REG, EP3);
	Inp32(EP_STATUS_REG, ep3csr);
	Outp32(EP_STATUS_REG, ep3csr);
// 	DbgUsb(("Bulk Out Int, ep3csr : %x\n", ep3csr));
	Inp32(DMA_TOTAL_CNT1_REG, temp);
	//EdbgOutputDebugString("EP3: DMA_TOTAL_CNT1_REG : %x\n", temp);


	if (ep3csr & EP_SENT_STALL) // SENT STALL : protocol stall.
	{
		// Uart_Printf(" Sent Stall \n");
		Outp32(EP_STATUS_REG, EP_SENT_STALL);
		return;
	}

	if (ep3csr & EP_FIFO_FLUSH)
	{
		Outp32(EP_CON_REG, EP_FIFO_FLUSH);
		return;
	}

	if (ep3csr & EP_RX_SUCCESS)
	{
		//EdbgOutputDebugString("EP3_RX_PKT_SUCCESS\n");
		/*
		if (g_uDownloadFileSize==0)
		{
			UINT8  TempBuf[16];
			UINT32 uUploadSize;
			UINT32  uUploadAddr;

			Inp32(BYTE_READ_CNT_REG, fifoCnt);
		
			if (fifoCnt == 5)
			{
				RdPktEp3((UINT8 *)TempBuf, 10);
				temp = *((UINT8 *)(TempBuf+8))+
					(*((UINT8 *)(TempBuf+9))<<8);
				EdbgOutputDebugString("temp: %x\n", temp);
				if (temp==0x1)
				{
					uUploadAddr =
						*((UINT8 *)(TempBuf+0))+
						(*((UINT8 *)(TempBuf+1))<<8)+
						(*((UINT8 *)(TempBuf+2))<<16)+
						(*((UINT8 *)(TempBuf+3))<<24);

					uUploadSize =
						*((UINT8 *)(TempBuf+4))+
						(*((UINT8 *)(TempBuf+5))<<8)+
						(*((UINT8 *)(TempBuf+6))<<16)+
						(*((UINT8 *)(TempBuf+7))<<24);
				
					//EdbgOutputDebugString("UploadAddress : %x, UploadSize: %x\n", uUploadAddr, uUploadSize);
					if (g_eOpMode == USB_CPU)
					{
						//EdbgOutputDebugString("CPU_MODE Bulk In Function\n");
						g_uBulkInCount = uUploadSize;
						PrepareEp1Fifo(uUploadAddr);
					}
					else
					{
						//EdbgOutputDebugString("DMA_MODE Bulk In Function\n");
						Outp32(FCON, DMA_ENABLE);				// USB Dma Enable in Core Outside
						Outp32(INDEX_REG, EP1);					// IN Direction  Device -> Host
						Outp32(DMA_IF_CON_REG, MAX_BURST_INCR16);
						Outp32(BYTE_WRITE_CNT_REG, g_uEp1MaxPktSize);
						Outp32(MAX_PKT_REG, g_uEp1MaxPktSize);
						Outp32(DMA_FIFO_CNT_REG, g_uEp1MaxPktSize);
						Outp32(DMA_CNT_REG, g_uEp1MaxPktSize);
						Outp32(DMA_MEM_BASE_ADDR, uUploadAddr);
						Outp32(DMA_TOTAL_CNT1_REG, (UINT16)uUploadSize);
						Outp32(DMA_TOTAL_CNT2_REG, (UINT16)(uUploadSize>>16));
						Outp32(DMA_CON_REG, DMA_FLY_ENABLE|DMA_TX_START|USB_DMA_MODE);
					}
				}
				g_uDownloadFileSize=0;
				return;
			}
			else
			{
			//	printf("^");
				RdPktEp3((UINT8 *)TempBuf, 8);
				if (ep3csr&(0x1<<4))
					fifoCntByte = fifoCnt * 2 -1;
				else
					fifoCntByte = fifoCnt * 2;
				//EdbgOutputDebugString("downloadFileSize==0, 1'st BYTE_READ_CNT_REG : %x\n", fifoCntByte);
				g_uDownloadAddress=
					*((UINT8 *)(TempBuf+0))+
					(*((UINT8 *)(TempBuf+1))<<8)+
					(*((UINT8 *)(TempBuf+2))<<16)+
					(*((UINT8 *)(TempBuf+3))<<24);

				g_uDownloadFileSize=
					*((UINT8 *)(TempBuf+4))+
					(*((UINT8 *)(TempBuf+5))<<8)+
					(*((UINT8 *)(TempBuf+6))<<16)+
					(*((UINT8 *)(TempBuf+7))<<24);

				g_pDownPt=(UINT8 *)g_uDownloadAddress;
				//EdbgOutputDebugString("downloadAddress : %x, downloadFileSize: %x\n", g_uDownloadAddress, g_uDownloadFileSize);
				//printf("downloadAddress : %x, downloadFileSize: %x\n", g_uDownloadAddress, g_uDownloadFileSize));
				downloadAddress =g_uDownloadAddress; // add by cha
				downloadFileSize =g_uDownloadFileSize; // add by cha
				
				RdPktEp3((UINT8 *)g_pDownPt, fifoCntByte-8); // The first 8-bytes are deleted.

				if (g_eOpMode == USB_CPU)
				{
					if (ep3csr & (0x2<<2))
					{
						Inp32(BYTE_READ_CNT_REG, fifoCnt);
						if (ep3csr&(0x1<<4))
							fifoCntByte = fifoCnt * 2 -1;
						else
							fifoCntByte = fifoCnt * 2;
						//EdbgOutputDebugString("2'd BYTE_READ_CNT_REG : %x\n", fifoCntByte);
						RdPktEp3((UINT8 *)g_pDownPt, fifoCntByte);
					}
				}
				else
				{
					Outp32(FCON, DMA_ENABLE);				// USB Dma Enable in Core Outside
					Outp32(INDEX_REG, EP3);					// OUT Direction  Host -> Device
					Outp32(DMA_IF_CON_REG, MAX_BURST_INCR16);
					Outp32(MAX_PKT_REG, g_uEp3MaxPktSize);
					Outp32(DMA_FIFO_CNT_REG, g_uEp3MaxPktSize);
					Outp32(DMA_CNT_REG, g_uEp3MaxPktSize);
					Outp32(DMA_MEM_BASE_ADDR, g_uDownloadAddress+fifoCntByte-8);
					Inp32(DMA_MEM_BASE_ADDR, temp);
					//EdbgOutputDebugString("DMA_MEM_BASE_ADDR : %x\n", temp);
					Outp32(DMA_TOTAL_CNT1_REG, (UINT16)(g_uDownloadFileSize-fifoCntByte));
					Outp32(DMA_TOTAL_CNT2_REG, (UINT16)(g_uDownloadFileSize>>16));
					//EdbgOutputDebugString("Out Direction DMA RX Start\n");
					Outp32(DMA_CON_REG, DMA_FLY_ENABLE|DMA_RX_START|USB_DMA_MODE);
				}
			}
		}

		else*/
		{
			if (g_eOpMode == USB_CPU)
			{
				Inp32(BYTE_READ_CNT_REG, fifoCnt);
				if (ep3csr&(0x1<<4))
					fifoCntByte = fifoCnt * 2 -1;
				else
					fifoCntByte = fifoCnt * 2;
				//EdbgOutputDebugString("downloadFileSize!=0, 0x%x 1'st BYTE_READ_CNT_REG : %x\n", g_pDownPt, fifoCntByte);
				RdPktEp3((UINT8 *)g_pDownPt, fifoCntByte);

				if (ep3csr & (0x2<<2))
				{
					Inp32(BYTE_READ_CNT_REG, fifoCnt);
					if (ep3csr&(0x1<<4))
						fifoCntByte = fifoCnt * 2 -1;
					else
						fifoCntByte = fifoCnt * 2;
					//EdbgOutputDebugString("2'd BYTE_READ_CNT_REG : %x\n", fifoCntByte);
					RdPktEp3((UINT8 *)g_pDownPt, fifoCntByte);
				}
			}
		}
	}

	if (DMA_TOTAL_COUNT_ZERO & ep3csr)
	{
		//EdbgOutputDebugString("USB_DMA_MODE, DMA RX Done(DMA_TOTAL_COUNT_ZERO) !!\n");
		Outp32(DMA_CON_REG, DMA_RX_STOP|USB_INT_MODE);
		Outp32(FCON, DMA_DISABLE);
	}
}

#define    BIT_ALLMSK       (0xffffffff)
#define    BIT_USBD         (0x1<<IRQ_USBD)

#ifdef _EBOOT_SLEEP_
#define    BIT_EINT0         (0x1<<IRQ_EINT0)
#endif


void Isr_Init(void)
{
   	volatile S3C2450_INTR_REG *s2450INT = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);

#if (BSP_TYPE == BSP_SMDK2443)
	s2450INT->INTMOD=0x0;	  // All=IRQ mode
	s2450INT->INTMSK=BIT_ALLMSK;	  // All interrupt is masked.

//	//EdbgOutputDebugString("INFO: (unsigned)IsrUsbd : 0x%x\r\n", (unsigned)IsrUsbd);
//	//EdbgOutputDebugString("INFO: (unsigned)IsrHandler : 0x%x\r\n", (unsigned)IsrHandler);

	// make value to assemble code "b IsrHandler"
//	EdbgOutputDebugString("INFO: (unsigned)pISR : 0x%x\r\n", (unsigned)pISR);
	pISR =(unsigned)(0xEA000000)+(((unsigned)IsrHandler - (0x8C000000 + 0x18 + 0x8) )>>2);

//	EdbgOutputDebugString("INFO: (unsigned)pISR : 0x%x\r\n", (unsigned)pISR);
//	EdbgOutputDebugString("INFO: (unsigned)IsrHandler : 0x%x\r\n", (unsigned)IsrHandler);

	if (s2450INT->SRCPND & BIT_USBD) s2450INT->SRCPND  = BIT_USBD;
	if (s2450INT->INTPND & BIT_USBD) s2450INT->INTPND = BIT_USBD;
	s2450INT->INTMSK &= ~BIT_USBD;		// USB Interrupt enable.

#elif (BSP_TYPE == BSP_SMDK2450)
	s2450INT->INTMOD1=0x0;	  // All=IRQ mode
	s2450INT->INTMSK1=BIT_ALLMSK;	  // All interrupt is masked.
	//	//EdbgOutputDebugString("INFO: (unsigned)IsrUsbd : 0x%x\r\n", (unsigned)IsrUsbd);
//	//EdbgOutputDebugString("INFO: (unsigned)IsrHandler : 0x%x\r\n", (unsigned)IsrHandler);

	// make value to assemble code "b IsrHandler"

	 //[david.modify] 2008-05-08 18:49
//	EdbgOutputDebugString("INFO: (unsigned)pISR : 0x%x\r\n", (unsigned)pISR);
	pISR =(unsigned)(0xEA000000)+(((unsigned)IsrHandler - (0x80000000 + 0x18 + 0x8) )>>2);

//	EdbgOutputDebugString("INFO: (unsigned)pISR : 0x%x\r\n", (unsigned)pISR);
//	EdbgOutputDebugString("INFO: (unsigned)IsrHandler : 0x%x\r\n", (unsigned)IsrHandler);

	if (s2450INT->SRCPND1 & BIT_USBD) s2450INT->SRCPND1  = BIT_USBD;
	if (s2450INT->INTPND1 & BIT_USBD) s2450INT->INTPND1 = BIT_USBD;
	s2450INT->INTMSK1 &= ~BIT_USBD;		// USB Interrupt enable.

#ifdef _EBOOT_SLEEP_
	if (s2450INT->SRCPND1 & BIT_EINT0) s2450INT->SRCPND1  = BIT_EINT0;
	if (s2450INT->INTPND1 & BIT_EINT0) s2450INT->INTPND1 = BIT_EINT0;
	s2450INT->INTMSK1 &= ~BIT_EINT0;	// EINT0 Interrupt enable.
#endif	
#endif


}

void IsrUsbd(unsigned int val)
{    
   	volatile S3C2450_INTR_REG *s2450INT = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);	
	UINT32 uStatus;
	UINT16 ep_int_status, ep_int;
	
	Inp32(SYS_STATUS_REG, uStatus); // System status read
	//EdbgOutputDebugString("SYS_STATUS_REG : %x \n", uStatus);
//	DPNOK(0);

	if (uStatus & INT_REG_VBUS)
	{
		Outp32(SYS_STATUS_REG, INT_REG_VBUS); // Interrupt Clear
		//EdbgOutputDebugString("\n [USB_Diag_Log]  :  INT_REG_VBUS\n");
	}

	if (uStatus & 0xff80) 	 // Error interrupt check
	{
		Outp32(SYS_STATUS_REG, INT_ERR); // Interrupt Clear
		//EdbgOutputDebugString("\n [USB_Diag_Log]  :  Error_INT\n");
	}

	// Which USB interrupts happen
	if (uStatus & INT_REG_SUSPEND)
	{
		Outp32(SYS_STATUS_REG, INT_REG_SUSPEND); // Interrupt Clear
		//EdbgOutputDebugString("\n [USB_Diag_Log]  : Suspend Mode");
	}

	if (uStatus & INT_REG_RESUME)
	{
		Outp32(SYS_STATUS_REG, INT_REG_RESUME); // Host software send ClearPortFeature. Interrupt Clear
		//EdbgOutputDebugString("\n [USB_Diag_Log]  : Resume Mode \n");
	}

	if (uStatus & INT_REG_RESET) // Reset interrupt
	{
		Outp32(SYS_STATUS_REG, INT_REG_RESET); // Interrupt Clear
		SetEndpoint();
		g_uEp0State = EP0_STATE_INIT;
		//EdbgOutputDebugString("\n [USB_Diag_Log]  : Reset Mode \n");
	}

	if (uStatus & INT_REG_SDE) // Device Speed Detection interrupt
	{
		Outp32(SYS_STATUS_REG, INT_REG_SDE); // Interrupt Clear
		//EdbgOutputDebugString("\n [USB_Diag_Log]  : Speed Detection interrupt \n");

		if (uStatus & INT_REG_HSP) // Set if Device is High speed or Full speed
		{
			Outp32(SYS_STATUS_REG, INT_REG_HSP); // High Speed Device Interrupt Clear?? may be not.
			//EdbgOutputDebugString("\n [USB_Diag_Log]  : High Speed Detection\n");
			SetMaxPktSizes(USB_HIGH);
			SetDescriptorTable();
		}
		else
		{
			SetMaxPktSizes(USB_FULL);
			SetDescriptorTable();
		}
	}

	Inp32(EP_STATUS_REG, ep_int_status); // EP interrrupt status read
	//EdbgOutputDebugString("EP_STATUS_REG : %x \n", ep_int_status);
	Inp32(EP_INT_REG, ep_int);
	//EdbgOutputDebugString("EP_INT_REG : %x \n", ep_int);

	if (ep_int & INT_REG_EP0)
	{
 		//DbgUsb(("\n [USB_Diag_Log]  :  Control Transfer Interrupt \n"));
		Outp32(EP_INT_REG, INT_REG_EP0); // Interrupt Clear
		HandleEvent_EP0();
	}

	// Endpoint1 bulkIn
	else if (ep_int & INT_REG_EP1)
	{
		Outp32(EP_INT_REG, INT_REG_EP1); // Interrupt Clear
	//	EdbgOutputDebugString("\n [USB_Diag_Log]  :  Ep1 Interrupt  \n");
		HandleEvent_BulkIn();
	}

	// Endpoint2 bulkOut
	else if (ep_int & INT_REG_EP3)
	{
	//	EdbgOutputDebugString("\n [USB_Diag_Log]  :  Bulk Out Transfer Interrupt  \n");
		Outp32(EP_INT_REG, INT_REG_EP3); // Interrupt Clear
	//	printf("*");
		HandleEvent_BulkOut();
	}

#if (BSP_TYPE == BSP_SMDK2443)
	if (s2450INT->INTPND & BIT_USBD)
	{
		s2450INT->SRCPND  = BIT_USBD;
		if (s2450INT->INTPND & BIT_USBD) s2450INT->INTPND = BIT_USBD;
	}	
#elif (BSP_TYPE == BSP_SMDK2450)
if (s2450INT->INTPND1 & BIT_USBD)
	{
		s2450INT->SRCPND1  = BIT_USBD;
		if (s2450INT->INTPND1 & BIT_USBD) s2450INT->INTPND1 = BIT_USBD;
	}	
#endif	
	
	
}

#ifdef _EBOOT_SLEEP_
void IsrPowerButton(void)
{  
   	volatile S3C2450_INTR_REG *s2450INT = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);	

	RETAILMSG(1,(TEXT("\nEboot Sleep Button(EINT0) Pressed!!\r\n")));
	if (s2450INT->INTPND1 & BIT_EINT0)
	{
		s2450INT->SRCPND1  = BIT_EINT0;
		if (s2450INT->INTPND1 & BIT_EINT0) s2450INT->INTPND1 = BIT_EINT0;
		OEMPowerOff();
	}
}

extern void OALCPUPowerOff(void);

void OEMPowerOff()
{
	volatile S3C2450_IOPORT_REG *pIOPort = (S3C2450_IOPORT_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_IOPORT, FALSE);
	volatile S3C2450_CLKPWR_REG *pCLKPWR = (S3C2450_CLKPWR_REG*)OALPAtoVA(S3C2450_BASE_REG_PA_CLOCK_POWER, FALSE);
	volatile S3C2450_LCD_REG    *s2450LCD = (S3C2450_LCD_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_LCD, FALSE);

	// Go into power off mode
	pIOPort->GPACON = 0xFFFFFFFF;
	pIOPort->GPADAT = 0x1FFFF;
	
	pCLKPWR->INFORM1 = 0xEB;

    //For USB
    pCLKPWR->USB_CLKCON = (0<<31)|(0<<2)|(0<<1)|(0<<0); //pullup disable
		// disable the PHY Power	
	pCLKPWR->PWRCFG &= ~(1<<4);
	
		// Set the Normal mode regulator disable
	pIOPort->GPHCON = (pIOPort->GPHCON & ~(0x3<<28)) | (0x1<<28);
	pIOPort->GPHUDP = (pIOPort->GPHUDP & ~(0x3<<28)) | (0x2<<28);      
	pIOPort->GPHDAT = (pIOPort->GPHDAT & ~(0x1<<14));  
	pIOPort->MISCCR |= (1<<12);
    
    //For Display
    s2450LCD->WIN0MAP &= ~(1<<24);
	s2450LCD->WIN1MAP &= ~(1<<24);
    s2450LCD->VIDCON0   = 0;
	s2450LCD->VIDCON1   = 0;
	s2450LCD->VIDTCON0   = 0;
	s2450LCD->VIDTCON1   = 0;
	s2450LCD->VIDTCON2   = 0;
	s2450LCD->WINCON0 = 0;
	s2450LCD->WINCON1 = 0;
	s2450LCD->VIDOSD0A = 0;
	s2450LCD->VIDOSD0B    = 0;
	s2450LCD->VIDOSD0C      = 0;
	s2450LCD->VIDW00ADD0B0      = 0;
	s2450LCD->VIDW00ADD1B0      = 0;
	s2450LCD->VIDW00ADD2B0      = 0;	
 
    //Here, Please set the state of GPIO as the lowest power along to your platform 
        
    // For IROM Boot
    pIOPort->GPCCON    &= ~(0x3f<<10);
    pIOPort->GPCUDP &= ~(0x3f<<10);
    
 	OALCPUPowerOff();

	pCLKPWR->RSTCON |= pCLKPWR->RSTCON;		// This is for control GPIO pads.	
}
#endif
#pragma optimize ("",off)
 //[david.modify] 2008-05-12 16:41
BOOL UbootReadData(DWORD cbData, LPBYTE pbData)
{ 	
   	volatile S3C2450_INTR_REG *s2450INT = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);	
   	UINT8* pbuf = NULL;
	static UINT32 u32cnt=0;	
	//RETAILMSG(1,(TEXT("UbootReadData\n")));
	while(1)
	{
		//RETAILMSG(1,(TEXT("0x%x 0x%x\n"), g_pDownPt, (readPtIndex + cbData)));

		if ((UINT32)g_pDownPt >= readPtIndex + cbData )
		{

#if 0		
		 DPNOK(u32cnt++);
		 DPNOK(g_pDownPt);
		 DPNOK(readPtIndex);
		 DPNOK(pbData);		
		 DPNOK(cbData);		
#endif		 
		
			pbuf = (PVOID)readPtIndex;
			memcpy((PVOID)pbData, pbuf, cbData);
			pbuf = (PVOID)OALPAtoUA(readPtIndex);
			// clear partial download memory to 0xff because data is already copied to buffer(pbData)
			memset(pbuf, 0xff, cbData);
			readPtIndex += cbData;
			break;
		}
		else if((UINT32)g_pDownPt == DMABUFFER)
		{
			/*
			if (s2450INT->SRCPND & BIT_USBD) s2450INT->SRCPND  = BIT_USBD;
			if (s2450INT->INTPND & BIT_USBD) s2450INT->INTPND = BIT_USBD;
			s2450INT->INTMSK &= ~BIT_USBD;		// USB Interrupt enable.
			*/
		}
	}
	
	return TRUE;
}

// 先用仿真器将数据放在USB DMA缓冲地址:DMABUFFER = 0x32000000
// 然后确认; 以后用UbootReadData去读,将指针移位
BOOL UbootReadData_JTAG(DWORD cbData, LPBYTE pbData)
{ 	
   	volatile S3C2450_INTR_REG *s2450INT = (S3C2450_INTR_REG *)OALPAtoVA(S3C2450_BASE_REG_PA_INTR, FALSE);	
   	UINT8* pbuf = NULL;
	static UINT32 u32cnt=0;
	//RETAILMSG(1,(TEXT("UbootReadData\n")));
	while(1)
	{
		//RETAILMSG(1,(TEXT("0x%x 0x%x\n"), g_pDownPt, (readPtIndex + cbData)));
//		if ((UINT32)g_pDownPt >= readPtIndex + cbData )
		 //[david.modify] 2008-05-12 16:43
#if 0		 
		 DPNOK(u32cnt++);
		 DPNOK(g_pDownPt);
		 DPNOK(readPtIndex);
		 DPNOK(pbData);		
		 DPNOK(cbData);		
#endif		 

#if 1
		if (1)// ((UINT32)g_pDownPt >= readPtIndex + cbData ) it has already copied data from SD card.
		{
			pbuf = (PVOID)readPtIndex;
			memcpy((PVOID)pbData, pbuf, cbData);
			//pbuf = (PVOID)OALPAtoUA(readPtIndex);
			// clear partial download memory to 0xff because data is already copied to buffer(pbData)
			memset(pbuf, 0xff, cbData);
	            readPtIndex += cbData;
			break;
		}    
#else

		 
		 if(1)
		{
			pbuf = (PVOID)readPtIndex;
			memcpy((PVOID)pbData, pbuf, cbData);
			pbuf = (PVOID)OALPAtoUA(readPtIndex);
			// clear partial download memory to 0xff because data is already copied to buffer(pbData)
			memset(pbuf, 0xff, cbData);
			readPtIndex += cbData;
			break;
		}
		else if((UINT32)g_pDownPt == DMABUFFER)
		{
			/*
			if (s2450INT->SRCPND & BIT_USBD) s2450INT->SRCPND  = BIT_USBD;
			if (s2450INT->INTPND & BIT_USBD) s2450INT->INTPND = BIT_USBD;
			s2450INT->INTMSK &= ~BIT_USBD;		// USB Interrupt enable.
			*/
		}
#endif		
	}
	
	return TRUE;
}

#pragma optimize ("",on)


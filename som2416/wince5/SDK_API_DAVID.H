#ifndef SDK_API_DAVID_H
#define  SDK_API_DAVID_H
//--------------------------------------------------------------------------------------------------
// Total OEM API class name
#define	OEM_BKLIGHT		0x0001			// Backight class name
#define	OEM_PWR			0x0002			// Power class name
#define	OEM_AUDIO			0x0004			// Audio class name
#define	OEM_BUTTON			0x0008			// Button class name

//--------------------------------------------------------------------------------------------------
// Backlight API system ID
#define	SET_ON				0x0001			// Turn on backlight without changing backlight settings
#define	SET_OFF				0x0002			// Turn off backlight without changing backlight settings
#define	SET_ALWAYSON		0x0004			// Turn backlight always on regardless in battery or AC power
#define	SET_BATTLVL			0x0008			// Change backlight level when using battery power, The lpInBuf
											//   points to the input buffer with the backlight level wanted
											//   ranging from 0 to 20, and the nInBufSize has a value of 2
#define	SET_ACLVL			0x0010			// Change backlight level when using AC power, The lpInBuf points
											//   to the input buffer with the backlight level wanted ranging
											//   from 0 to 20, and the nInBufSize has a value of 2
#define	SET_LVL				0x0011			// Change backlight level, The lpInBuf points to the input buffer
											//   with the backlight level wanted ranging from 0 to 20, and the
											//   nInBufSize has a value of 2


// Standard global API ID
// Backlight standard API ID
#define	OEM_BKL_SET_ON				(OEM_BKLIGHT << 16 | SET_ON)
#define	OEM_BKL_SET_OFF				(OEM_BKLIGHT << 16 | SET_OFF)
#define	OEM_BKL_SET_ALWAYSON			(OEM_BKLIGHT << 16 | SET_ALWAYSON)
#define	OEM_BKL_SET_BATTLVL			(OEM_BKLIGHT << 16 | SET_BATTLVL)
#define	OEM_BKL_SET_ACLVL			(OEM_BKLIGHT << 16 | SET_ACLVL)
#define	OEM_BKL_SET_LVL				(OEM_BKLIGHT << 16 | SET_LVL)

 //[david.modify] 2008-07-14 11:07
#define BKL_STATE_FULLON	4
#define BKL_STATE_ON	3
#define BKL_STATE_EFFICIENT	2
#define BKL_STATE_IDLE	1
#define BKL_STATE_OFF	0

#define BKL_MAX_LEVEL	10
#define BKL_MIN_LEVEL	1

#define BKL_ON			0xff
#define BKL_OFF			0x0
 #include <windev.h>
#define IOCTL_BACKLIGHT_SET		CTL_CODE(FILE_DEVICE_UNKNOWN, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BACKLIGHT_GET		CTL_CODE(FILE_DEVICE_UNKNOWN, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_TOTAL_LEVELS		CTL_CODE(FILE_DEVICE_UNKNOWN, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BACKLIGHT_SET_MAX		CTL_CODE(FILE_DEVICE_UNKNOWN, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BACKLIGHT_SET_CURRENT	CTL_CODE(FILE_DEVICE_UNKNOWN, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)


#endif

@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this source code is subject to the terms of the Microsoft end-user
@REM license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
@REM If you did not accept the terms of the EULA, you are not authorized to use
@REM this source code. For a copy of the EULA, please see the LICENSE.RTF on your
@REM install media.
@REM

set WINCEREL=1


 @rem //[david.modify] 2008-07-07 09:57
 @rem 添加是否编译加入GLONAV支持
set BSP_GPS_NOGLONAV=
 

@REM for bib file and reg file and dat file
set BSP_SMDK2443_CFG=
set BSP_SMDK2450_CFG=1

@REM //[david.modify] 2008-05-06 09:57
@REM 添加一个16BIT 32MSDRAM控制
@REM============================
@rem 只能有效一个BSP_32MSDRAM 或者BSP_64MSDRAM
rem set BSP_32MSDRAM_CFG=1
set BSP_64MSDRAM_CFG=1
set BSP_RAMSIZE=64M
@REM============================

@REM for all files except bib and reg file and dat file
set BSP_SMDK2443=1
set BSP_SMDK2450=2

@REM set BSP_TYPE=%BSP_SMDK2443%
@REM set BSP_TYPE=%BSP_SMDK2450%

set BSP_TYPE=%BSP_SMDK2450%


if /i "%BSP_SMDK2443_CFG%"=="1" set BSP_EVT1=1

set BSP_NOPCIBUS=1

set BSP_NODISPLAY=
set BSP_NOSERIAL=
@rem  //[david.modify] 2008-05-14 10:47
;;    //[david.modify] 2008-05-19 15:41 串口0用作DEBUG调试使用
set BSP_NOUART0=
set BSP_NOUART1=
@rem  //[david.modify] 2008-05-14 10:48
set BSP_NOIRDA=
@rem  //[david.modify] 2008-05-14 10:48
set BSP_NOUART3=
set BSP_NOKEYBD=
set BSP_NOTOUCH=
set BSP_NOTSPCALIBRATE=1
@rem  //[david.modify] 2008-05-14 10:48
@REM 注释下行,OS起不来
set BSP_NOPWRBTN=
@rem  //[david. end] 2008-05-14 10:48

set BSP_NOAUDIO=
@REM  //[david.modify] 2008-05-14 11:03
set BSP_AUDIO_AC97=
@REM  //[david. end] 2008-05-14 11:03

@rem  //[david.modify] 2008-05-14 10:48
set BSP_NOUSB=
set BSP_NOUSBFN=
@rem  //[david. end] 2008-05-14 10:48
set BSP_NOCS8900=1

@REM  //[david.modify] 2008-04-23 18:29
set BSP_NOBACKLIGHT=
set BSP_NOBATTERY=
@REM  //[david. end] 2008-04-23 18:29
set BSP_NONANDFS=1
set BSP_NOPCCARD=1


set BSP_NOSD=

if /i "%BSP_SMDK2443_CFG%"=="1" set BSP_NOHSMMC=
if /i "%BSP_SMDK2450_CFG%"=="1" set BSP_NOHSMMC_CH0=
@Rem  //[david.modify] 2008-05-28 15:01 no ch1
if /i "%BSP_SMDK2450_CFG%"=="1" set BSP_NOHSMMC_CH1=1
if /i "%BSP_SMDK2450_CFG%"=="1" set BSP_HSMMC_CH1_8BIT=1

set BSP_NOPOST=1
set BSP_NOCFATAPI=1

set BSP_USEDVS=
set BSP_NOCAMERA=1
@rem //[david.modify] 2008-05-14 10:45
set BSP_NOI2C=
@rem  //[david. end] 2008-05-14 10:49
set BSP_NOHSSPI=1

set BSP_NONLED=
if /i "%BSP_USEDVS%"=="1" set BSP_NONLED=1
set BSP_USBFNCLASS=SERIAL
@REM set BSP_USBFNCLASS=MASS_STORAGE
set BSP_KITL=NONE
rem set BSP_KITL=USBSERIAL


set BSP_MOVINAND=
@REM - To support SD card
set SYSGEN_SDBUS=1

@REM - To support Multiple XIP
rem set IMGUPDATEXIP=1

@REM 2443 default MUTIPLEXIP SUPPORT
@REM  2450 can select MULTIPLEXIP or Single.bin
@rem //[david.modify] 2008-04-09 11:27
set IMGMULTIXIP=
@rem  //[david. end] 2008-04-09 11:27


@rem  //[david.modify] 2008-04-15 18:21
@REM For Hive Based Registry
set IMGHIVEREG=1

if /i "%IMGHIVEREG%"=="1" set PRJ_ENABLE_FSREGHIVE=1
if /i "%IMGHIVEREG%"=="1" set PRJ_ENABLE_REGFLUSH_THREAD=1
@rem  //[david. end] 2008-04-15 18:21

@REM  //[david.modify] 2008-06-11 18:29
set BSP_NOAPPDLL=1
set BSP_NOGPSAPP1=
set BSP_NOGPSAPP2=

set IMGBLOCKROM=1
rem set IMGNOKITL=1
rem set IMGNODEBUGGER=1

@REM - To support PocketMory
call %_TARGETPLATROOT%\src\Whimory\wmrenv.bat

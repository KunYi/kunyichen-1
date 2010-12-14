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



@rem  //[david.modify] 2008-11-17 09:53
@rem gps模块切换
@rem===============================
set BSP_GPS_GLONAV4540=100
set BSP_GPS_GLONAV7560=101
set BSP_GPS_NX3=200
set BSP_GPS_TYPE=%BSP_GPS_NX3%
@rem===============================

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
@rem  //[david.modify] 2008-09-04 12:15
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
set BSP_NONANDFS=
set BSP_NOPCCARD=1


set BSP_NOSD=

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

@rem //[david.modify] 2008-11-11 11:04
@rem 增加一个驱动用于读写NAND FLASH
set BSP_NOMLC_DAVID=1



@REM //[david.modify] 2008-11-17 10:16
@REM USB SERIAL和MASS Storage模式切换
@REM==============================
set BSP_USBFNCLASS=SERIAL
@REM set BSP_USBFNCLASS=MASS_STORAGE

@REM set BSP_KITL=NONE
rem set BSP_KITL=USBSERIAL


set BSP_MOVINAND=
@REM - To support SD card
set SYSGEN_SDBUS=1

@REM - To support Multiple XIP
rem set IMGUPDATEXIP=1

@REM 2443 default MUTIPLEXIP SUPPORT
@REM  2450 can select MULTIPLEXIP or Single.bin
@rem //[david.modify] 2008-04-09 11:27
set IMGMULTIXIP=1
@rem  //[david. end] 2008-04-09 11:27

;; //[david.modify] 2008-10-31 15:21
;; 当XIP模式，BSP_NONANDFS＝空
if /i "%IMGMULTIXIP%"=="1" set BSP_NONANDFS=


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

@REM //[david.modify] 2008-09-02 15:41
set BSP_NODEBUGTOOLS=


set IMGBLOCKROM=1
rem set IMGNOKITL=1
rem set IMGNODEBUGGER=1


@rem  //[david.modify] 2008-09-07 11:01
@rem 通过宏定义，确定是什么样的屏
@rem enum {BYD_4281L_LCD=100, YASSY_YF35F03CIB=200, SHARP_LQ035Q1=300, LCD_43INCH_480X272=400};
@REM ======================================================================
@REM display resolution
@REM ======================================================================
set BSP_LCD_BYD_4281L=100
set BSP_LCD_YASSY_YF35F03CIB=200
@YT50F01MHZ-SMR-E 4.3INCH LCD
set BSP_LCD_YASSY_43INCH_480X272=201
set BSP_LCD_SHARP_LQ035Q1=300
set BSP_LCD_BYD_43INCH_480X272=400
set BSP_LCD_INNOLUX_35=500
set BSP_LCD_INNOLUX_43=501

@rem SET BSP_LCD_TYPE=%BSP_LCD_BYD_43INCH_480X272%


rem  //[david.modify] 2008-11-08 09:06
rem 增加一个宏，用来定义哪家客户
Rem //[david.modify] 2008-11-04 11:11
set BSP_CUSTOMER_DEMO35=10
set BSP_CUSTOMER_DEMO43=1010

set BSP_CUSTOMER_PBC35=20
rem set BSP_CUSTOMER_PBC43=1020

set BSP_CUSTOMER_MAXCOM35=30
rem set BSP_CUSTOMER_MAXCOM43=1030

set BSP_CUSTOMER_LCP35=40
set BSP_CUSTOMER_LCP43=1040

set BSP_CUSTOMER_JETHMAR35=50
set BSP_CUSTOMER_JETHMAR43=1050

rem set BSP_CUSTOMER_CHANGTIAN35=60
set BSP_CUSTOMER_CHANGTIAN43=1060

@rem set BSP_LCD_TYPE=%BSP_LCD_BYD_4281L%
set BSP_LCD_TYPE=%BSP_LCD_INNOLUX_43%
set BSP_CUSTOMER=%BSP_CUSTOMER_DEMO43%

@rem if /i "%BSP_CUSTOMER%"=="%BSP_CUSTOMER_DEMO43%" set BSP_LCD_TYPE=%BSP_LCD_BYD_43INCH_480X272%
@rem if /i "%BSP_CUSTOMER%"=="%BSP_CUSTOMER_JETHMAR43%" set BSP_LCD_TYPE=%BSP_LCD_BYD_43INCH_480X272%
@rem if /i "%BSP_CUSTOMER%"=="%BSP_CUSTOMER_CHANGTIAN43%" set BSP_LCD_TYPE=%BSP_LCD_BYD_43INCH_480X272%
@rem if /i "%BSP_CUSTOMER%"=="%BSP_CUSTOMER_LCP43%" set BSP_LCD_TYPE=%BSP_LCD_BYD_43INCH_480X272%
@rem if /i "%BSP_CUSTOMER%"=="%BSP_CUSTOMER_LCP35%" set BSP_LCD_TYPE=%BSP_LCD_BYD_4281L%





set BSP_DISPLAY_WIDTH=320
set BSP_DISPLAY_HEIGHT=240

set BSP_NOHSMMC_CH0=
set BSP_NOHSMMC_CH1=1


rem ;; //[david.modify] 2008-10-20 14:35
rem 增加一个缺省UI语言项
REM 1-英文 2-中文 3-泰文
set BSP_AP_DEF_LANG=1

Rem //[david.modify] 2008-11-04 11:11
set BSP_GPS_AP1=1
set BSP_GPS_AP2=2
set BSP_GPS_IPHONE=3
set BSP_GPS_APP=%BSP_GPS_AP2%


if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_BYD_43INCH_480X272%" set BSP_DISPLAY_WIDTH=480
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_BYD_43INCH_480X272%" set BSP_DISPLAY_HEIGHT=272
@rem 4.3inch的PND项目 使用SLOT1 SD卡座
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_BYD_43INCH_480X272%" set BSP_NOHSMMC_CH0=1
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_BYD_43INCH_480X272%" set BSP_NOHSMMC_CH1=


if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_YASSY_43INCH_480X272%" set BSP_DISPLAY_WIDTH=480
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_YASSY_43INCH_480X272%" set BSP_DISPLAY_HEIGHT=272
@rem 4.3inch的PND项目 使用SLOT1 SD卡座
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_YASSY_43INCH_480X272%" set BSP_NOHSMMC_CH0=1
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_YASSY_43INCH_480X272%" set BSP_NOHSMMC_CH1=

if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_INNOLUX_43%" set BSP_DISPLAY_WIDTH=480
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_INNOLUX_43%" set BSP_DISPLAY_HEIGHT=272
@rem 4.3inch的PND项目 使用SLOT1 SD卡座
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_INNOLUX_43%" set BSP_NOHSMMC_CH0=1
if /i "%BSP_LCD_TYPE%"=="%BSP_LCD_INNOLUX_43%" set BSP_NOHSMMC_CH1=


echo "%BSP_LCD_TYPE%"
echo "%BSP_DISPLAY_WIDTH%"
echo "%BSP_DISPLAY_HEIGHT%"

@REM 蓝牙功能模块,f.w.lin
set BSP_BLUETOOTH=

IF "%BSP_BLUETOOTH%"=="1" (
    SET BSP_SERIALCSR=1
@rem    SET BSP_FM1182=1
    SET BSP_FM1182=

@rem    SET SYSGEN_BTH=1
@rem    SET SYSGEN_BTH_AG=1
@rem    SET SYSGEN_BTH_CSR_ONLY=1
@rem    SET SYSGEN_BTH_AUDIO=1
)






@REM - To support PocketMory
call %_TARGETPLATROOT%\src\Whimory\wmrenv.bat

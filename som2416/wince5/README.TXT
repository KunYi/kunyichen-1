15:31 2008-8-21
在编译BOOTLOADER时，需要先做2个动作:
1. 进入 SMDK2416\Src\Bootloader\Eboot.Whimory\BLCOMMON编译生成
blcommon_david.lib
2. 将SMDK2416\Src\Bootloader\Eboot.Whimory\FAT_LIB中的FAT_LIB.lib拷贝到
SMDK2416\lib\ARMV4I\retail目录下
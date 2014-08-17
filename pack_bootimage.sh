#!/bin/bash
#make to bootimage.sh
PRJ=wiko

 out/host/linux-x86/bin/acp -uv kernel/out/kernel_${PRJ}.bin out/target/product/${PRJ}/kernel
 out/host/linux-x86/bin/mkbootfs out/target/product/${PRJ}/root | out/host/linux-x86/bin/minigzip > out/target/product/${PRJ}/ramdisk.img
 mediatek/build/tools/mkimage out/target/product/${PRJ}/ramdisk.img ROOTFS > out/target/product/${PRJ}/ramdisk_android.img
 mv out/target/product/${PRJ}/ramdisk.img mediatek/kernel/trace32/${PRJ}_ramdisk.img
 mv out/target/product/${PRJ}/ramdisk_android.img out/target/product/${PRJ}/ramdisk.img
 out/host/linux-x86/bin/mkbootimg  --kernel out/target/product/${PRJ}/kernel --ramdisk out/target/product/${PRJ}/ramdisk.img --board 1336460062 --output out/target/product/${PRJ}/boot.img
 echo "pack Bootimage done!"


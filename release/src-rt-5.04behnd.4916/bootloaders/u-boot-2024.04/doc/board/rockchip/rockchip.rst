.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2019 Jagan Teki <jagan@amarulasolutions.com>

ROCKCHIP
========

About this
----------

This document describes the information about Rockchip supported boards
and it's usage steps.

Rockchip boards
---------------

Rockchip is a SoC solutions provider for tablets & PCs, streaming media
TV boxes, AI audio & vision, IoT hardware.

A wide range of Rockchip SoCs with associated boards are supported in
mainline U-Boot.

List of mainline supported Rockchip boards:

* px30
     - Rockchip Evb-PX30 (evb-px30)
     - Engicam PX30.Core C.TOUCH 2.0 (px30-core-ctouch2-px30)
     - Engicam PX30.Core C.TOUCH 2.0 10.1 (px30-core-ctouch2-of10-px30)
     - Engicam PX30.Core EDIMM2.2 Starter Kit (px30-core-edimm2.2-px30)
     - Firefly Core-PX30-JD4 (firefly-px30)
     - Theobroma Systems PX30-µQ7 SoM - Ringneck (ringneck-px30)
* rk3036
     - Rockchip Evb-RK3036 (evb-rk3036)
     - Kylin (kylin_rk3036)
* rk3066
     - Rikomagic MK808 (mk808)
* rk3128
     - Rockchip Evb-RK3128 (evb-rk3128)
* rk3188
     - Radxa Rock (rock)
* rk3229
     - Rockchip Evb-RK3229 (evb-rk3229)
* rk3288
     - Rockchip Evb-RK3288 (evb-rk3288)
     - Firefly-RK3288 (firefly-rk3288)
     - MQmaker MiQi (miqi-rk3288)
     - Phytec RK3288 PCM-947 (phycore-rk3288)
     - PopMetal-RK3288 (popmetal-rk3288)
     - Radxa Rock 2 Square (rock2)
     - Tinker-RK3288 (tinker-rk3288)
     - Google Jerry (chromebook_jerry)
     - Google Mickey (chromebook_mickey)
     - Google Minnie (chromebook_minnie)
     - Google Speedy (chromebook_speedy)
     - Amarula Vyasa-RK3288 (vyasa-rk3288)
* rk3308
     - Radxa ROCK Pi S (rock-pi-s-rk3308)
     - Rockchip Evb-RK3308 (evb-rk3308)
     - Roc-cc-RK3308 (roc-cc-rk3308)
* rk3326
     - ODROID-GO Advance (odroid-go2)
* rk3328
     - Rockchip Evb-RK3328 (evb-rk3328)
     - Pine64 Rock64 (rock64-rk3328)
     - Firefly-RK3328 (roc-cc-rk3328)
     - Radxa Rockpi E (rock-pi-e-rk3328)
* rk3368
     - GeekBox (geekbox)
     - PX5 EVB (evb-px5)
     - Rockchip Sheep (sheep-rk3368)
     - Theobroma Systems RK3368-uQ7 SoM - Lion (lion-rk3368)
* rk3399
     - 96boards RK3399 Ficus (ficus-rk3399)
     - 96boards Rock960 (rock960-rk3399)
     - Firefly-RK3399 (firefly_rk3399)
     - Firefly ROC-RK3399-PC
     - FriendlyElec NanoPC-T4 (nanopc-t4-rk3399)
     - FriendlyElec NanoPi M4 (nanopi-m4-rk3399)
     - FriendlyElec NanoPi M4B (nanopi-m4b-rk3399)
     - FriendlyARM NanoPi NEO4 (nanopi-neo4-rk3399)
     - Google Bob (chromebook_bob)
     - Google Kevin (chromebook_kevin)
     - Khadas Edge (khadas-edge-rk3399)
     - Khadas Edge-Captain (khadas-edge-captain-rk3399)
     - Khadas Edge-V (hadas-edge-v-rk3399)
     - Orange Pi RK3399 (orangepi-rk3399)
     - Pine64 RockPro64 (rockpro64-rk3399)
     - Radxa ROCK 4C+ (rock-4c-plus-rk3399)
     - Radxa ROCK 4SE (rock-4se-rk3399)
     - Radxa ROCK Pi 4A/B/A+/B+ (rock-pi-4-rk3399)
     - Radxa ROCK Pi 4C (rock-pi-4c-rk3399)
     - Rockchip Evb-RK3399 (evb_rk3399)
     - Theobroma Systems RK3399-Q7 SoM - Puma (puma_rk3399)

* rk3566
     - Anbernic RGxx3 (anbernic-rgxx3-rk3566)
     - Pine64 Quartz64-A Board (quartz64-a-rk3566)
     - Pine64 Quartz64-B Board (quartz64-b-rk3566)
     - Pine64 SOQuartz on Blade (soquartz-blade-rk3566)
     - Pine64 SOQuartz on CM4-IO (soquartz-cm4-rk3566)
     - Pine64 SOQuartz on Model A (soquartz-model-a-rk3566)
     - Radxa CM3 IO Board (radxa-cm3-io-rk3566)

* rk3568
     - Rockchip Evb-RK3568 (evb-rk3568)
     - Banana Pi BPI-R2 Pro (bpi-r2-pro-rk3568)
     - EmbedFire LubanCat 2 (lubancat-2-rk3568)
     - FriendlyElec NanoPi R5C (nanopi-r5c-rk3568)
     - FriendlyElec NanoPi R5S (nanopi-r5s-rk3568)
     - Generic RK3566/RK3568 (generic-rk3568)
     - Hardkernel ODROID-M1 (odroid-m1-rk3568)
     - Radxa E25 Carrier Board (radxa-e25-rk3568)
     - Radxa ROCK 3 Model A (rock-3a-rk3568)

* rk3588
     - Rockchip EVB (evb-rk3588)
     - Edgeble Neural Compute Module 6A SoM - Neu6a (neu6a-io-rk3588)
     - Edgeble Neural Compute Module 6B SoM - Neu6b (neu6b-io-rk3588)
     - FriendlyElec NanoPC-T6 (nanopc-t6-rk3588)
     - Pine64 QuartzPro64 (quartzpro64-rk3588)
     - Turing Machines RK1 (turing-rk1-rk3588)
     - Radxa ROCK 5A (rock5a-rk3588s)
     - Radxa ROCK 5B (rock5b-rk3588)
     - Xunlong Orange Pi 5 (orangepi-5-rk3588s)
     - Xunlong Orange Pi 5 Plus (orangepi-5-plus-rk3588)

* rv1108
     - Rockchip Evb-rv1108 (evb-rv1108)
     - Elgin-R1 (elgin-rv1108)

* rv1126
     - Edgeble Neural Compute Module 2 SoM - Neu2/Neu2k (neu2-io-r1126)
     - Itead Sonoff iHost (sonoff-ihost-rv1126)

Building
--------

TF-A
^^^^

TF-A is required when building ARM64 Rockchip SoCs images.

To build TF-A:

.. code-block:: bash

        git clone --depth 1 https://github.com/ARM-software/arm-trusted-firmware.git
        cd arm-trusted-firmware
        make realclean
        make CROSS_COMPILE=aarch64-linux-gnu- PLAT=rk3399
        cd ..

Specify the PLAT= with desired Rockchip platform to build TF-A for.

For SoCs whose TF-A code is not available as open source, use BL31 binary provided by Rockchip:

.. code-block:: bash

        git clone --depth 1 https://github.com/rockchip-linux/rkbin

TPL
^^^

For some SoCs U-Boot sources lack of support to inizialize DRAM.
In these cases, to get a fully functional image following :ref:`PackageWithTPLandSPL`, use DDR binary provided by Rockchip rkbin repository as ROCKCHIP_TPL when building U-Boot.
Otherwise, follow :ref:`PackageWithRockchipMiniloader`. 

U-Boot
^^^^^^

.. code-block:: bash

        git clone --depth 1 https://source.denx.de/u-boot/u-boot.git
        cd u-boot

To build px30 boards:

.. code-block:: bash

        export BL31=../arm-trusted-firmware/build/px30/release/bl31/bl31.elf
        make evb-px30_defconfig
        make CROSS_COMPILE=aarch64-linux-gnu-

To build rk3066 boards:

.. code-block:: bash

        make mk808_defconfig
        make CROSS_COMPILE=arm-linux-gnueabihf-

To build rk3288 boards:

.. code-block:: bash

        make evb-rk3288_defconfig
        make CROSS_COMPILE=arm-linux-gnueabihf-

To build rk3308 boards:

.. code-block:: bash

        export BL31=../rkbin/bin/rk33/rk3308_bl31_v2.26.elf
        export ROCKCHIP_TPL=../rkbin/bin/rk33/rk3308_ddr_589MHz_uartX_mY_v2.07.bin
        make evb-rk3308_defconfig
        make CROSS_COMPILE=aarch64-linux-gnu-

To build rk3328 boards:

.. code-block:: bash

        export BL31=../arm-trusted-firmware/build/rk3328/release/bl31/bl31.elf
        make evb-rk3328_defconfig
        make CROSS_COMPILE=aarch64-linux-gnu-

To build rk3368 boards:

.. code-block:: bash

        export BL31=../arm-trusted-firmware/build/rk3368/release/bl31/bl31.elf
        make evb-px5_defconfig
        make CROSS_COMPILE=aarch64-linux-gnu-

To build rk3399 boards:

.. code-block:: bash

        export BL31=../arm-trusted-firmware/build/rk3399/release/bl31/bl31.elf
        make evb-rk3399_defconfig
        make CROSS_COMPILE=aarch64-linux-gnu-

To build rk3568 boards:

.. code-block:: bash

        export BL31=../arm-trusted-firmware/build/rk3568/release/bl31/bl31.elf
        [or]export BL31=../rkbin/bin/rk35/rk3568_bl31_v1.34.elf
        export ROCKCHIP_TPL=../rkbin/bin/rk35/rk3568_ddr_1560MHz_v1.13.bin
        make evb-rk3568_defconfig
        make CROSS_COMPILE=aarch64-linux-gnu-

To build rk3588 boards:

.. code-block:: bash

        export BL31=../rkbin/bin/rk35/rk3588_bl31_v1.33.elf
        export ROCKCHIP_TPL=../rkbin/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2736MHz_v1.09.bin
        make evb-rk3588_defconfig
        make CROSS_COMPILE=aarch64-linux-gnu-

Flashing
--------

.. _`PackageWithTPLandSPL`:

1. Package the image with U-Boot TPL/SPL
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

SD Card
"""""""

All Rockchip platforms (except rk3128 which doesn't use SPL) are now
supporting a single boot image using binman.

To write an image that boots from a SD card (assumed to be /dev/sda):

.. code-block:: bash

        sudo dd if=u-boot-rockchip.bin of=/dev/sda seek=64
        sync

eMMC
""""

eMMC flash would probe on mmc0 in most of the Rockchip platforms.

Create GPT partition layout as defined in $partitions:

.. code-block:: bash

        mmc dev 0
        gpt write mmc 0 $partitions

Connect the USB-OTG cable between the host and a target device.

Launch fastboot on the target with:

.. code-block:: bash

        fastboot 0

Upon a successful gadget connection the host shows the USB device with:

.. code-block:: bash

        lsusb
        # Bus 001 Device 020: ID 2207:330c Fuzhou Rockchip Electronics Company RK3399 in Mask ROM mode

Program the flash with:

.. code-block:: bash

        sudo fastboot -i 0x2207 flash loader1 idbloader.img
        sudo fastboot -i 0x2207 flash loader2 u-boot.itb

Note:

For Rockchip 32-bit platforms the U-Boot proper image
is u-boot-dtb.img

SPI
"""

Write u-boot-rockchip-spi.bin to offset 0 of SPI flash.

Copy u-boot-rockchip-spi.bin into SD card and boot from SD:

.. code-block:: bash

        sf probe
        load mmc 1:1 $kernel_addr_r u-boot-rockchip-spi.bin
        sf update $fileaddr 0 $filesize

.. _`PackageWithRockchipMiniloader`:

2. Package the image with Rockchip miniloader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Image package with Rockchip miniloader requires rkbin [1].

.. code-block:: bash

        cd ..
        git clone --depth 1 https://github.com/rockchip-linux/rkbin

Create idbloader.img:

.. code-block:: bash

        cd u-boot
        ./tools/mkimage -n px30 -T rksd -d ../rkbin/bin/rk33/px30_ddr_333MHz_v1.16.bin idbloader.img
        cat ../rkbin/bin/rk33/px30_miniloader_v1.31.bin >> idbloader.img
        sudo dd if=idbloader.img of=/dev/sda seek=64

Create trust.img:

.. code-block:: bash

        cd ../rkbin
        ./tools/trust_merger RKTRUST/PX30TRUST.ini
        sudo dd if=trust.img of=/dev/sda seek=24576

Create uboot.img [2]:

.. code-block:: bash

        cd ../u-boot
        ../rkbin/tools/loaderimage --pack --uboot u-boot-dtb.bin uboot.img 0x200000
        sudo dd if=uboot.img of=/dev/sda seek=16384

Note:

1. rkbin binaries are regularly updated, so it would be recommended to use the latest version.
2. 0x200000 is a load address and is an option for some platforms.

3. Package the RK3066 image with U-Boot TPL/SPL on NAND
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Unlike later SoC models the rk3066 BootROM doesn't have SDMMC support.
If all other boot options fail then it enters into a BootROM mode on the USB OTG port.
This method loads TPL/SPL on NAND with U-Boot and kernel on SD card.

SD Card
"""""""

U-Boot expects a GPT partition map and a boot directory structure with files on the SD card.

.. code-block:: none

        Partition Map for MMC device 0  --   Partition Type: EFI
        Part     Start LBA         End LBA           Name
        1        0x00000040        0x00001f7f        "loader1"
        2        0x00004000        0x00005fff        "loader2"
        3        0x00006000        0x00007fff        "trust"
        4        0x00008000        0x0003ffff        "boot"
        5        0x00040000        0x00ed7fde        "rootfs"

Make sure boot and esp flag are set for the boot partition.
Loader1 partition is not used by RK3066.

Boot partition:

.. code-block:: none

        extlinux
          extlinux.conf

        zImage
        rk3066a-mk808.dtb

To write a U-Boot image to the SD card (assumed to be /dev/sda):

.. code-block:: bash

        sudo dd if=u-boot-dtb.img of=/dev/sda seek=16384
        sync

NAND
""""

Bring device in BootROM mode:

If bricked and no BootROM mode shows up then connect pin 8 and 9 of the NAND flash
with a needle while reconnecting to the USB OTG port to a PC.

Show connected devices with:

.. code-block:: bash

        lsusb
        # Bus 001 Device 004: ID 2207:300a Fuzhou Rockchip Electronics Company RK3066 in Mask ROM mode


Create NAND image:

Size of SPL and TPL must be aligned to 2kb.

Program with commands in a bash script ./flash.sh:

.. code-block:: bash

        #!/bin/sh

        printf "RK30" | dd conv=notrunc bs=4 count=1 of=u-boot-tpl.bin
        truncate -s %2048 u-boot-tpl.bin
        truncate -s %2048 u-boot-spl.bin
        ../tools/boot_merger --verbose config-flash.ini
        ../tools/upgrade_tool ul ./RK30xxLoader_uboot.bin

config-flash.ini:

.. code-block:: none

        [CHIP_NAME]
        NAME=RK30
        [VERSION]
        MAJOR=2
        MINOR=21
        [CODE471_OPTION]
        NUM=1
        Path1=30_LPDDR2_300MHz_DD.bin
        [CODE472_OPTION]
        NUM=1
        Path1=rk30usbplug.bin
        [LOADER_OPTION]
        NUM=2
        LOADER1=FlashData
        LOADER2=FlashBoot
        FlashData=u-boot-tpl.bin
        FlashBoot=u-boot-spl.bin
        [OUTPUT]
        PATH=RK30xxLoader_uboot.bin

TODO
----

- Add Rockchip idbloader image building
- Add Rockchip TPL image building
- Document SPI flash boot
- Add missing SoC's with it boards list

.. Jagan Teki <jagan@amarulasolutions.com>
.. Wednesday 28 October 2020 06:47:26 PM IST

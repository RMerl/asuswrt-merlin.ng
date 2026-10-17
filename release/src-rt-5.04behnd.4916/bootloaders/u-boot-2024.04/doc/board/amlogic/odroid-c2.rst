.. SPDX-License-Identifier: GPL-2.0+

U-Boot for ODROID-C2 (S905)
===========================

ODROID-C2 is a single board computer manufactured by Hardkernel with the following
specifications:

 - Amlogic S905 ARM Cortex-A53 quad-core SoC @ 1.5GHz
 - ARM Mali 450 GPU
 - 2GB DDR3 SDRAM
 - Gigabit Ethernet
 - HDMI 2.0 4K/60Hz display
 - 40-pin GPIO header
 - 4 x USB 2.0 Host, 1 x USB OTG
 - eMMC, microSD
 - Infrared receiver

Schematics are available on the manufacturer website: https://wiki.odroid.com

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make odroid-c2_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh odroid-c2 /path/to/u-boot/u-boot.bin my-output-dir

U-Boot Manual Signing
---------------------

Amlogic does not provide sources for the firmware and tools needed to create a bootloader
image so it is necessary to obtain binaries from sources published by the board vendor:

.. code-block:: bash

    $ DIR=odroid-c2
    $ git clone --depth 1 https://github.com/hardkernel/u-boot.git -b odroidc2-v2015.01 $DIR

    $ $DIR/fip/fip_create --bl30  $DIR/fip/gxb/bl30.bin \
                          --bl301 $DIR/fip/gxb/bl301.bin \
                          --bl31  $DIR/fip/gxb/bl31.bin \
                          --bl33  u-boot.bin \
                          $DIR/fip.bin

    $ $DIR/fip/fip_create --dump $DIR/fip.bin
    $ cat $DIR/fip/gxb/bl2.package $DIR/fip.bin > $DIR/boot_new.bin
    $ $DIR/fip/gxb/aml_encrypt_gxb --bootsig \
                                   --input $DIR/boot_new.bin \
                                   --output $DIR/u-boot.img
    $ dd if=$DIR/u-boot.img of=$DIR/u-boot.gxbb bs=512 skip=96

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/your_boot_device
    $ BL1=$DIR/sd_fuse/bl1.bin.hardkernel
    $ dd if=$BL1 of=$DEV conv=fsync bs=1 count=442
    $ dd if=$BL1 of=$DEV conv=fsync bs=512 skip=1 seek=1
    $ dd if=$DIR/u-boot.gxbb of=$DEV conv=fsync bs=512 seek=97

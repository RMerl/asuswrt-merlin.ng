.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Videostrong KII Pro (S905)
=====================================

Videostrong KII Pro is an Android STB manufactured by Videostrong and 
based on the Amlogic p201 reference board, with the following specification:

 - Amlogic S905 ARM Cortex-A53 quad-core SoC @ 1.5GHz
 - ARM Mali 450 GPU
 - 2GB DDR3 SDRAM
 - 16GB eMMC
 - Gigabit Ethernet
 - Boardcom BCM4335 WiFi and BT 4.0
 - HDMI 2.0 4K/60Hz display
 - 3x USB 2.0 host
 - 1x USB 2.0 otg
 - microSD
 - Infrared receiver
 - Blue LED
 - Red LED
 - Power button (case, front)
 - Reset button (underside)
 - DVB Card: DVB-S and DVB-T/C

Schematics are not publicly available.

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make videostrong-kii-pro_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh wetek-play2 /path/to/u-boot/u-boot.bin my-output-dir

U-Boot Manual Signing
---------------------

Amlogic does not provide sources for the firmware and tools needed to create 
a bootloader image and Videostrong has not publicly shared the U-Boot sources 
needed to build FIP binaries for signing. However you can use the WeTek 
Play2 binaries from the amlogic-boot-fip repo as the WeTek Play2 and the 
Videostrong KII Pro share the same RAM chips.

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip/wetek-play2
    $ export FIPDIR=$PWD

Go back to the mainline U-Boot source tree then:

.. code-block:: bash

    $ mkdir fip
    $ cp $FIPDIR/bl2.bin fip/
    $ cp $FIPDIR/acs.bin fip/
    $ cp $FIPDIR/bl21.bin fip/
    $ cp $FIPDIR/bl30.bin fip/
    $ cp $FIPDIR/bl301.bin fip/
    $ cp $FIPDIR/bl31.img fip/
    $ cp u-boot.bin fip/bl33.bin
    $ $FIPDIR/blx_fix.sh \
              fip/bl30.bin \
              fip/zero_tmp \
              fip/bl30_zero.bin \
              fip/bl301.bin \
              fip/bl301_zero.bin \
              fip/bl30_new.bin \
              bl30
    $ $FIPDIR/fip_create --bl30 fip/bl30_new.bin \
                         --bl31 fip/bl31.img \
                         --bl33 fip/bl33.bin \
                         fip/fip.bin
    $ sed -i 's/\x73\x02\x08\x91/\x1F\x20\x03\xD5/' fip/bl2.bin
    $ python3 $FIPDIR/acs_tool.py fip/bl2.bin fip/bl2_acs.bin fip/acs.bin 0
    $ $FIPDIR/blx_fix.sh \
              fip/bl2_acs.bin \
              fip/zero_tmp \
              fip/bl2_zero.bin \
              fip/bl21.bin \
              fip/bl21_zero.bin \
              fip/bl2_new.bin \
              bl2
    $ cat fip/bl2_new.bin fip/fip.bin > fip/boot_new.bin
    $ $FIPDIR/aml_encrypt_gxb --bootsig \
                              --input fip/boot_new.bin
                              --output fip/u-boot.bin

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin of=fip/u-boot.bin.gxbb bs=512 conv=fsync
    $ dd if=fip/u-boot.bin of=fip/u-boot.bin.gxbb bs=512 seek=9 skip=8 count=87 conv=fsync,notrunc
    $ dd if=/dev/zero of=fip/u-boot.bin.gxbb bs=512 seek=8 count=1 conv=fsync,notrunc
    $ dd if=bl1.bin.hardkernel of=fip/u-boot.bin.gxbb bs=512 seek=2 skip=2 count=1 conv=fsync,notrunc
    $ ./aml_chksum fip/u-boot.bin.gxbb
    $ dd if=fip/u-boot.bin.gxbb of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.gxbb of=$DEV conv=fsync,notrunc bs=1 count=440

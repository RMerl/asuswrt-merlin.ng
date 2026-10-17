.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Khadas VIM3L (S905D3)
================================

Khadas VIM3L is a Single Board Computer manufactured by Shenzhen Wesion Technology Co. Ltd
with the following specifications:

 - Amlogic S905D3 Arm Cortex-A55 quad-core SoC
 - 2GB LPDDR4 SDRAM
 - Gigabit Ethernet
 - HDMI 2.1 display
 - 40-pin GPIO header
 - 1 x USB 3.0 Host, 1 x USB 2.0 Host
 - eMMC, microSD
 - M.2
 - Infrared receiver

Schematics are available on the manufacturer website.

PCIe Setup
----------

The on-board MCU can mux the PCIe/USB3.0 shared differential lines using a FUSB340TMX USB
3.1 SuperSpeed Data Switch between a USB3.0 Type-A connector and an M.2 Key-M slot. The
PHY driving these differential lines is shared between the USB3.0 controller and the PCIe
Controller, thus only a single controller can use it.

To setup for PCIe run the following commands from U-Boot then power-cycle the board:

.. code-block:: none

    i2c dev i2c@5000
    i2c mw 0x18 0x33 1

To revert to USB3.0 run the following commands from U-Boot then power-cycle the board:

.. code-block:: none

    i2c dev i2c@5000
    i2c mw 0x18 0x33 0

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make khadas-vim3l_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh khadas-vim3l /path/to/u-boot/u-boot.bin my-output-dir

U-Boot Manual Signing
---------------------

Amlogic does not provide sources for the firmware and tools needed to create a bootloader
image so it is necessary to obtain binaries from sources published by the board vendor:

.. code-block:: bash

    $ wget https://releases.linaro.org/archive/13.11/components/toolchain/binaries/gcc-linaro-aarch64-none-elf-4.8-2013.11_linux.tar.xz
    $ wget https://releases.linaro.org/archive/13.11/components/toolchain/binaries/gcc-linaro-arm-none-eabi-4.8-2013.11_linux.tar.xz
    $ tar xvfJ gcc-linaro-aarch64-none-elf-4.8-2013.11_linux.tar.xz
    $ tar xvfJ gcc-linaro-arm-none-eabi-4.8-2013.11_linux.tar.xz
    $ export PATH=$PWD/gcc-linaro-aarch64-none-elf-4.8-2013.11_linux/bin:$PWD/gcc-linaro-arm-none-eabi-4.8-2013.11_linux/bin:$PATH

    $ DIR=vim3l-u-boot
    $ git clone --depth 1 https://github.com/khadas/u-boot.git -b khadas-vims-v2015.01 $DIR

    $ cd vim3l-u-boot
    $ make kvim3l_defconfig
    $ make CROSS_COMPILE=aarch64-none-elf-
    $ export UBOOTDIR=$PWD

Go back to the mainline U-Boot source tree then:

.. code-block:: bash

    $ mkdir fip

    $ wget https://github.com/BayLibre/u-boot/releases/download/v2017.11-libretech-cc/blx_fix_g12a.sh -O fip/blx_fix.sh
    $ cp $UBOOTDIR/build/scp_task/bl301.bin fip/
    $ cp $UBOOTDIR/build/board/khadas/kvim3l/firmware/acs.bin fip/
    $ cp $UBOOTDIR/fip/g12a/bl2.bin fip/
    $ cp $UBOOTDIR/fip/g12a/bl30.bin fip/
    $ cp $UBOOTDIR/fip/g12a/bl31.img fip/
    $ cp $UBOOTDIR/fip/g12a/ddr3_1d.fw fip/
    $ cp $UBOOTDIR/fip/g12a/ddr4_1d.fw fip/
    $ cp $UBOOTDIR/fip/g12a/ddr4_2d.fw fip/
    $ cp $UBOOTDIR/fip/g12a/diag_lpddr4.fw fip/
    $ cp $UBOOTDIR/fip/g12a/lpddr3_1d.fw fip/
    $ cp $UBOOTDIR/fip/g12a/lpddr4_1d.fw fip/
    $ cp $UBOOTDIR/fip/g12a/lpddr4_2d.fw fip/
    $ cp $UBOOTDIR/fip/g12a/piei.fw fip/
    $ cp $UBOOTDIR/fip/g12a/aml_ddr.fw fip/
    $ cp u-boot.bin fip/bl33.bin

    $ bash fip/blx_fix.sh \
           fip/bl30.bin \
           fip/zero_tmp \
           fip/bl30_zero.bin \
           fip/bl301.bin \
           fip/bl301_zero.bin \
           fip/bl30_new.bin \
           bl30

    $ bash fip/blx_fix.sh \
           fip/bl2.bin \
           fip/zero_tmp \
           fip/bl2_zero.bin \
           fip/acs.bin \
           fip/bl21_zero.bin \
           fip/bl2_new.bin \
           bl2

    $ $UBOOTDIR/fip/g12a/aml_encrypt_g12a --bl30sig --input fip/bl30_new.bin \
                                          --output fip/bl30_new.bin.g12a.enc \
                                          --level v3
    $ $UBOOTDIR/fip/g12a/aml_encrypt_g12a --bl3sig --input fip/bl30_new.bin.g12a.enc \
                                          --output fip/bl30_new.bin.enc \
                                          --level v3 --type bl30
    $ $UBOOTDIR/fip/g12a/aml_encrypt_g12a --bl3sig --input fip/bl31.img \
                                          --output fip/bl31.img.enc \
                                          --level v3 --type bl31
    $ $UBOOTDIR/fip/g12a/aml_encrypt_g12a --bl3sig --input fip/bl33.bin --compress lz4 \
                                          --output fip/bl33.bin.enc \
                                          --level v3 --type bl33 --compress lz4
    $ $UBOOTDIR/fip/g12a/aml_encrypt_g12a --bl2sig --input fip/bl2_new.bin \
                                          --output fip/bl2.n.bin.sig
    $ $UBOOTDIR/fip/g12a/aml_encrypt_g12a --bootmk \
                                          --output fip/u-boot.bin \
                                          --bl2 fip/bl2.n.bin.sig \
                                          --bl30 fip/bl30_new.bin.enc \
                                          --bl31 fip/bl31.img.enc \
                                          --bl33 fip/bl33.bin.enc \
                                          --ddrfw1 fip/ddr4_1d.fw \
                                          --ddrfw2 fip/ddr4_2d.fw \
                                          --ddrfw3 fip/ddr3_1d.fw \
                                          --ddrfw4 fip/piei.fw \
                                          --ddrfw5 fip/lpddr4_1d.fw \
                                          --ddrfw6 fip/lpddr4_2d.fw \
                                          --ddrfw7 fip/diag_lpddr4.fw \
                                          --ddrfw8 fip/aml_ddr.fw \
                                          --ddrfw9 fip/lpddr3_1d.fw \
                                          --level v3

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440

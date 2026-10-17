.. SPDX-License-Identifier: GPL-2.0+

U-Boot for BananaPi CM4 with CM4IO (A311D)
==========================================

BPI-CM4 is a system-on-module board manufactured by Sinovoip. It follows the Raspberry Pi
CM4 interface specification but with a single HDMI port and a single DSI output:

 - Amlogic A311D Arm Cortex-A53 dual-core + Cortex-A73 quad-core SoC 
 - 4GB DDR4 SDRAM
 - 16GB eMMC
 - NPU
 - HDMI 2.1 display
 - Gigabit Ethernet
 - RTL8822CS WiFi (a/b/g/n/ac) + BT 5.0

BPI-CM4IO is a carrier board for the BPI-CM4 module with the following specification:

 - CM4 interface
 - HDMI interface
 - MIPI CSI interface
 - MIPI DSI interface
 - Ethernet interface
 - PCIe interface
 - SD (micro)
 - SIM (micro)
 - 26-pin GPIO
 - UART serial
 - 1x USB-C (power)
 - 2x USB 2.0

Schematics are available from the manufacturer: https://wiki.banana-pi.org/Banana_Pi_BPI-CM4

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make bananapi-cm4io_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh bananapi-cm4io /path/to/u-boot/u-boot.bin my-output-dir

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

    $ DIR=bananapi-cm4io
    $ git clone --depth 1 https://github.com/Dangku/amlogic-u-boot.git -b khadas-g12b-v2015.01-m2s $DIR

    $ cd $DIR
    $ make bananapi_cm4_defconfig
    $ make
    $ export UBDIR=$PWD

Go back to the mainline U-Boot source tree then:

.. code-block:: bash

    $ mkdir fip

    $ wget https://github.com/BayLibre/u-boot/releases/download/v2017.11-libretech-cc/blx_fix_g12a.sh -O fip/blx_fix.sh
    $ cp $UBDIR/build/scp_task/bl301.bin fip/
    $ cp $UBDIR/build/board/bananapi/bananpi_cm4/firmware/acs.bin fip/
    $ cp $UBDIR/fip/g12a/bl2.bin fip/
    $ cp $UBDIR/fip/g12a/bl30.bin fip/
    $ cp $UBDIR/fip/g12a/bl31.img fip/
    $ cp $UBDIR/fip/g12a/ddr3_1d.fw fip/
    $ cp $UBDIR/fip/g12a/ddr4_1d.fw fip/
    $ cp $UBDIR/fip/g12a/ddr4_2d.fw fip/
    $ cp $UBDIR/fip/g12a/diag_lpddr4.fw fip/
    $ cp $UBDIR/fip/g12a/lpddr3_1d.fw fip/
    $ cp $UBDIR/fip/g12a/lpddr4_1d.fw fip/
    $ cp $UBDIR/fip/g12a/lpddr4_2d.fw fip/
    $ cp $UBDIR/fip/g12a/piei.fw fip/
    $ cp $UBDIR/fip/g12a/aml_ddr.fw fip/
    $ cp u-boot.bin fip/bl33.bin

    $ sh fip/blx_fix.sh \
         fip/bl30.bin \
         fip/zero_tmp \
         fip/bl30_zero.bin \
         fip/bl301.bin \
         fip/bl301_zero.bin \
         fip/bl30_new.bin \
         bl30

    $ sh fip/blx_fix.sh \
         fip/bl2.bin \
         fip/zero_tmp \
         fip/bl2_zero.bin \
         fip/acs.bin \
         fip/bl21_zero.bin \
         fip/bl2_new.bin \
         bl2

    $ $UBDIR/fip/g12b/aml_encrypt_g12b --bl30sig --input fip/bl30_new.bin \
                                       --output fip/bl30_new.bin.g12a.enc \
                                       --level v3
    $ $UBDIR/fip/g12b/aml_encrypt_g12b --bl3sig --input fip/bl30_new.bin.g12a.enc \
                                       --output fip/bl30_new.bin.enc \
                                       --level v3 --type bl30
    $ $UBDIR/fip/g12b/aml_encrypt_g12b --bl3sig --input fip/bl31.img \
                                       --output fip/bl31.img.enc \
                                       --level v3 --type bl31
    $ $UBDIR/fip/g12b/aml_encrypt_g12b --bl3sig --input fip/bl33.bin --compress lz4 \
                                       --output fip/bl33.bin.enc \
                                       --level v3 --type bl33 --compress lz4
    $ $UBDIR/fip/g12b/aml_encrypt_g12b --bl2sig --input fip/bl2_new.bin \
                                       --output fip/bl2.n.bin.sig
    $ $UBDIR/fip/g12b/aml_encrypt_g12b --bootmk \
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

Then write the image to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440

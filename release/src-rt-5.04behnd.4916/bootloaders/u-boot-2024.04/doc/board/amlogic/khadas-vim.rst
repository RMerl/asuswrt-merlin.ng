.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Khadas VIM (S905X)
=============================

Khadas VIM is a Single Board Computer manufactured by Shenzhen Wesion Technology Co. Ltd
with the following specifications:

 - Amlogic S905X ARM Cortex-A53 quad-core SoC @ 1.5GHz
 - ARM Mali 450 GPU
 - 2GB DDR3 SDRAM
 - 8GB/16GB eMMC
 - 10/100 Ethernet
 - HDMI 2.0 4K/60Hz display
 - 40-pin GPIO header
 - 2 x USB 2.0 Host, 1 x USB 2.0 Type-C OTG
 - microSD
 - SDIO Wifi Module, Bluetooth
 - Two channel IR receiver

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make khadas-vim_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh khadas-vim /path/to/u-boot/u-boot.bin my-output-dir

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
    $ git clone https://github.com/khadas/u-boot -b Vim vim-u-boot
    $ cd vim-u-boot
    $ make kvim_defconfig
    $ make CROSS_COMPILE=aarch64-none-elf-
    $ export FIPDIR=$PWD/fip

Go back to mainline U-Boot source tree then :

.. code-block:: bash

    $ mkdir fip

    $ cp $FIPDIR/gxl/bl2.bin fip/
    $ cp $FIPDIR/gxl/acs.bin fip/
    $ cp $FIPDIR/gxl/bl21.bin fip/
    $ cp $FIPDIR/gxl/bl30.bin fip/
    $ cp $FIPDIR/gxl/bl301.bin fip/
    $ cp $FIPDIR/gxl/bl31.img fip/
    $ cp u-boot.bin fip/bl33.bin

    $ $FIPDIR/blx_fix.sh \
              fip/bl30.bin \
              fip/zero_tmp \
              fip/bl30_zero.bin \
              fip/bl301.bin \
              fip/bl301_zero.bin \
              fip/bl30_new.bin \
              bl30

    $ python $FIPDIR/acs_tool.pyc fip/bl2.bin fip/bl2_acs.bin fip/acs.bin 0

    $ $FIPDIR/blx_fix.sh \
              fip/bl2_acs.bin \
              fip/zero_tmp \
              fip/bl2_zero.bin \
              fip/bl21.bin \
              fip/bl21_zero.bin \
              fip/bl2_new.bin \
              bl2

    $ $FIPDIR/gxl/aml_encrypt_gxl --bl3enc --input fip/bl30_new.bin
    $ $FIPDIR/gxl/aml_encrypt_gxl --bl3enc --input fip/bl31.img
    $ $FIPDIR/gxl/aml_encrypt_gxl --bl3enc --input fip/bl33.bin
    $ $FIPDIR/gxl/aml_encrypt_gxl --bl2sig --input fip/bl2_new.bin --output fip/bl2.n.bin.sig
    $ $FIPDIR/gxl/aml_encrypt_gxl --bootmk \
                                  --output fip/u-boot.bin \
                                  --bl2 fip/bl2.n.bin.sig \
                                  --bl30 fip/bl30_new.bin.enc \
                                  --bl31 fip/bl31.img.enc \
                                  --bl33 fip/bl33.bin.enc

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440

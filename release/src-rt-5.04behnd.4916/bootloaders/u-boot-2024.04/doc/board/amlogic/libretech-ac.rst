.. SPDX-License-Identifier: GPL-2.0+

U-Boot for LibreTech-AC 'LaFrite' (S805X)
=========================================

LibreTech-AC aka 'LaFrite' is a Single Board Computer manufactured by Libre Computer
with the following specifications:

 - Amlogic S805X ARM Cortex-A53 quad-core SoC @ 1.2GHz
 - ARM Mali 450 GPU
 - 512MiB DDR4 SDRAM
 - 10/100 Ethernet
 - HDMI 2.0 4K/60Hz display
 - 40-pin GPIO header
 - 4 x USB 2.0 Host
 - SPI NOR Flash
 - Removable eMMC module
 - Infrared receiver

Schematics are available on the manufacturer website.

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make libretech-ac_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh lafrite /path/to/u-boot/u-boot.bin my-output-dir

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
    $ git clone https://github.com/BayLibre/u-boot.git -b libretech-ac amlogic-u-boot
    $ cd amlogic-u-boot
    $ wget https://raw.githubusercontent.com/BayLibre/u-boot/libretech-cc/fip/blx_fix.sh
    $ make libretech_ac_defconfig
    $ make
    $ export UBOOTDIR=$PWD

Download the latest Amlogic buildroot package and extract it:

.. code-block:: bash

    $ wget http://openlinux2.amlogic.com:8000/ARM/filesystem/Linux_BSP/buildroot_openlinux_kernel_4.9_fbdev_20180418.tar.gz
    $ tar xfz buildroot_openlinux_kernel_4.9_fbdev_20180418.tar.gz buildroot_openlinux_kernel_4.9_fbdev_20180418/bootloader
    $ export BRDIR=$PWD/buildroot_openlinux_kernel_4.9_fbdev_20180418

Go back to the mainline U-Boot source tree then:

.. code-block:: bash

    $ mkdir fip

    $ cp $UBOOTDIR/build/scp_task/bl301.bin fip/
    $ cp $UBOOTDIR/build/board/amlogic/libretech_ac/firmware/bl21.bin fip/
    $ cp $UBOOTDIR/build/board/amlogic/libretech_ac/firmware/acs.bin fip/
    $ cp $BRDIR/bootloader/uboot-repo/bl2/bin/gxl/bl2.bin fip/
    $ cp $BRDIR/bootloader/uboot-repo/bl30/bin/gxl/bl30.bin fip/
    $ cp $BRDIR/bootloader/uboot-repo/bl31/bin/gxl/bl31.img fip/
    $ cp u-boot.bin fip/bl33.bin

    $ sh $UBOOTDIR/blx_fix.sh \
                   fip/bl30.bin \
                   fip/zero_tmp \
                   fip/bl30_zero.bin \
                   fip/bl301.bin \
                   fip/bl301_zero.bin \
                   fip/bl30_new.bin \
                   bl30

    $ $BRDIR/bootloader/uboot-repo/fip/acs_tool.pyc fip/bl2.bin fip/bl2_acs.bin fip/acs.bin 0

    $ sh $UBOOTDIR/blx_fix.sh \
                   fip/bl2_acs.bin \
                   fip/zero_tmp \
                   fip/bl2_zero.bin \
                   fip/bl21.bin \
                   fip/bl21_zero.bin \
                   fip/bl2_new.bin \
                   bl2

    $ $BRDIR/bootloader/uboot-repo/fip/gxl/aml_encrypt_gxl --bl3enc --input fip/bl30_new.bin
    $ $BRDIR/bootloader/uboot-repo/fip/gxl/aml_encrypt_gxl --bl3enc --input fip/bl31.img
    $ $BRDIR/bootloader/uboot-repo/fip/gxl/aml_encrypt_gxl --bl3enc --input fip/bl33.bin
    $ $BRDIR/bootloader/uboot-repo/fip/gxl/aml_encrypt_gxl --bl2sig --input fip/bl2_new.bin --output fip/bl2.n.bin.sig
    $ $BRDIR/bootloader/uboot-repo/fip/gxl/aml_encrypt_gxl --bootmk \
                   --output fip/u-boot.bin \
                   --bl2 fip/bl2.n.bin.sig \
                   --bl30 fip/bl30_new.bin.enc \
                   --bl31 fip/bl31.img.enc \
                   --bl33 fip/bl33.bin.enc

Then write U-Boot to USB or SPI-NOR with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440

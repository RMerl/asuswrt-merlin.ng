.. SPDX-License-Identifier: GPL-2.0+

U-Boot for ODROID-N2L (S922X)
=============================

ODROID-N2L is a Single Board Computer manufactured by Hardkernel with the following
specifications:

 - Amlogic S922X ARM Cortex-A53 dual-core + Cortex-A73 quad-core SoC
 - 4GB DDR4 SDRAM
 - HDMI 2.1 4K/60Hz display
 - 40-pin GPIO header
 - 1x USB 3.0 Host
 - 1x USB 2.0 Host
 - eMMC, microSD
 - MIPI DSI Port

Schematics are available on the manufacturer website: https://wiki.odroid.com

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make odroid-n2l_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh odroid-n2l /path/to/u-boot/u-boot.bin my-output-dir

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440

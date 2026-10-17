.. SPDX-License-Identifier: GPL-2.0+

imx8mn_bsh_smm_s2
=================

U-Boot for the BSH SystemMaster (SMM) S2 board family.

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get firmware-imx package
- Build U-Boot
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/ARM-software/arm-trusted-firmware
tag: v2.5

.. code-block:: bash

   $ make PLAT=imx8mn IMX_BOOT_UART_BASE=0x30a60000 bl31
   $ cp build/imx8mn/release/bl31.bin $(srctree)

Get the ddr firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.9.bin
   $ chmod +x firmware-imx-8.9.bin
   $ ./firmware-imx-8.9
   $ cp firmware-imx-8.9/firmware/ddr/synopsys/ddr3*.bin $(srctree)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ make imx8mn_bsh_smm_s2_defconfig
   $ make

Burn the flash.bin to MicroSD card offset 32KB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32 conv=notrunc

Boot
----

Start the board in USB serial downloader mode, plug-in the USB-OTG port and
load flash.bin using Freescale/NXP UUU tool:

.. code-block:: bash

   $ uuu -v flash.bin

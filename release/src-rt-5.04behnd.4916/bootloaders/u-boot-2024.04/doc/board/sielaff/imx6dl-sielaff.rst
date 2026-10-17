.. SPDX-License-Identifier: GPL-2.0+

Sielaff i.MX6 Solo Board
========================

The Sielaff i.MX6 Solo board is a control and Human Machine Interface (HMI)
board for vending machines.

Quick Start
-----------

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   make imx6dl_sielaff_defconfig
   make CROSS_COMPILE=arm-linux-gnueabi-

Copy the flash.bin file to an SD card at an offset of 1 KiB:

.. code-block:: bash

   dd if=flash.bin of=/dev/sd[x] bs=1K seek=1

Boot
^^^^

Put the SD card in the slot on the board and apply power.

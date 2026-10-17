.. SPDX-License-Identifier: GPL-2.0+

Kontron Electronics SL i.MX6UL/ULL SoM
======================================

The Kontron SoM-Line i.MX6UL/ULL (N6x1x) by Kontron Electronics GmbH is a SoM module
with either an i.MX6UL or i.MX6ULL SoC, 256/512 MB DDR3 RAM, SPI NOR, SPI NAND and Ethernet PHY.

The matching evaluation boards (Board-Line) have two Ethernet ports, USB 2.0,
RGB, SD card, CAN, RS485, RS232 and much more.

Quick Start
-----------

- Build U-Boot
- Boot

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   $ make kontron-sl-mx6ul_defconfig
   $ make

Burn the flash.bin to SD card at an offset of 1 KiB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1K seek=1 conv=notrunc

Boot
^^^^

Put the SD card in the slot on the board and apply power.

Further Information
-------------------

The bootloader configuration is setup to be used with kernel FIT images. Legacy
images might not be working out of the box.

Please see https://docs.kontron-electronics.de for further vendor documentation.

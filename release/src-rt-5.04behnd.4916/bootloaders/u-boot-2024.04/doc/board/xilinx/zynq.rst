.. SPDX-License-Identifier: GPL-2.0+
..  (C) Copyright 2013 Xilinx, Inc.

ZYNQ
====

About this
----------

This document describes the information about Xilinx Zynq U-Boot -
like supported boards, ML status and TODO list.

Zynq boards
-----------

Xilinx Zynq-7000 All Programmable SoCs enable extensive system level
differentiation, integration, and flexibility through hardware, software,
and I/O programmability.

* zc702 (single qspi, gem0, mmc) [1]
* zc706 (dual parallel qspi, gem0, mmc) [2]
* zed (single qspi, gem0, mmc) [3]
* microzed (single qspi, gem0, mmc) [4]
* zc770
     - zc770-xm010 (single qspi, gem0, mmc)
     - zc770-xm011 (8 or 16 bit nand)
     - zc770-xm012 (nor)
     - zc770-xm013 (dual parallel qspi, gem1)

Building
--------

configure and build for zc702 board::

   $ export DEVICE_TREE=zynq-zc702
   $ make xilinx_zynq_virt_defconfig
   $ make

Bootmode
--------

Zynq has a facility to read the bootmode from the slcr bootmode register
once user is setting through jumpers on the board - see page no:1546 on [5]

All possible bootmode values are defined in Table 6-2:Boot_Mode MIO Pins
on [5].

board_late_init() will read the bootmode values using slcr bootmode register
at runtime and assign the modeboot variable to specific bootmode string which
is intern used in autoboot.

SLCR bootmode register Bit[3:0] values

.. code-block:: c

   #define ZYNQ_BM_NOR		0x02
   #define ZYNQ_BM_SD		0x05
   #define ZYNQ_BM_JTAG		0x0

"modeboot" variable can assign any of "norboot", "sdboot" or "jtagboot"
bootmode strings at runtime.

Flashing
--------

SD Card
^^^^^^^

To write an image that boots from a SD card first create a FAT32 partition
and a FAT32 filesystem on the SD card::

        sudo fdisk /dev/sdx
        sudo mkfs.vfat -F 32 /dev/sdx1

Mount the SD card and copy the SPL and U-Boot to the root directory of the
SD card::

        sudo mount -t vfat /dev/sdx1 /mnt
        sudo cp spl/boot.bin /mnt
        sudo cp u-boot.img /mnt

Mainline status
---------------

- Added basic board configurations support.
- Added zynq U-Boot bsp code - arch/arm/mach-zynq
- Added zynq boards named - zc70x, zed, microzed, zc770_xm010/xm011/xm012/xm013
- Added zynq drivers:

  :serial: drivers/serial/serial_zynq.c
  :net: drivers/net/zynq_gem.c
  :mmc: drivers/mmc/zynq_sdhci.c
  :spi: drivers/spi/zynq_spi.c
  :qspi: drivers/spi/zynq_qspi.c
  :i2c: drivers/i2c/zynq_i2c.c
  :nand: drivers/mtd/nand/raw/zynq_nand.c

- Done proper cleanups on board configurations
- Added basic FDT support for zynq boards
- d-cache support for zynq_gem.c

* [1] http://www.xilinx.com/products/boards-and-kits/EK-Z7-ZC702-G.htm
* [2] http://www.xilinx.com/products/boards-and-kits/EK-Z7-ZC706-G.htm
* [3] http://zedboard.org/product/zedboard
* [4] http://zedboard.org/product/microzed
* [5] http://www.xilinx.com/support/documentation/user_guides/ug585-Zynq-7000-TRM.pdf


.. Jagannadha Sutradharudu Teki <jaganna@xilinx.com>
.. Sun Dec 15 14:52:41 IST 2013

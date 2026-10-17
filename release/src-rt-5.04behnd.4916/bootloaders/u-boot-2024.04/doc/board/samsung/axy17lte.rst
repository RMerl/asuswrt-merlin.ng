.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Dzmitry Sankouski <dsankouski@gmail.com>

Samsung 2017 A series phones
============================

About this
----------
This document describes the information about Samsung A(7/5/3) 2017 midrange
phones and u-boot usage steps.

U-Boot can be used as a chain-loaded bootloader to replace Samsung's original SBOOT bootloader.
It is loaded as an Android boot image through SBOOT.

Phone specs
-----------
A3 (SM-A320) (a3y17lte)
^^^^^^^^^^^^^^^^^^^^^^^
- 4.7 AMOLED display
- Exynos 7870 SoC
- 16GB flash
- 2GB RAM

.. A3 2017 wiki page: https://en.wikipedia.org/wiki/Samsung_Galaxy_A3_(2017)

A5 (SM-A520) (a5y17lte)
^^^^^^^^^^^^^^^^^^^^^^^
- 5.2 AMOLED display
- Exynos 7880 SoC
- 32GB flash
- 3GB RAM

.. A5 2017 wiki page: https://en.wikipedia.org/wiki/Samsung_Galaxy_A5_(2017)

A7 (SM-A720) (a5y17lte)
^^^^^^^^^^^^^^^^^^^^^^^
- 5.7 AMOLED display
- Exynos 7880 SoC
- 32GB flash
- 3GB RAM

.. A7 2017 wiki page: https://en.wikipedia.org/wiki/Samsung_Galaxy_A7_(2017)

Installation
------------

Building u-boot
^^^^^^^^^^^^^^^

First, setup ``CROSS_COMPILE`` for aarch64.
Then, build U-Boot for your phone, for example ``a5y17lte``::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make a5y17lte_defconfig
  $ make

This will build ``u-boot.bin`` in the configured output directory.

Payload
^^^^^^^
What is a payload?
""""""""""""""""""
A payload file is a file to be used instead of linux kernel in android boot image.
This file will be loaded into memory, and executed by SBOOT,
and is therefore SBOOT's payload.
It may be pure u-boot (with loading u-boot's payload from flash in mind),
or u-boot + u-boot's payload.

Creating payload file
"""""""""""""""""""""
- Assemble FIT image for your kernel

Creating android boot image
"""""""""""""""""""""""""""
Once payload created, it's time for android image::

  uboot=<path to u-boot.bin file>
  ramdisk=<path to FIT payload file>
  mkbootimg --base 0x40000000 --kernel_offset 0x00000000 --ramdisk_offset 0x01000000 --tags_offset 0x00000100 --pagesize 2048 --second_offset 0x00f00000 --kernel "$uboot" --ramdisk "$ramdisk" -o uboot.img

Note, that stock Samsung bootloader ignores offsets, set in mkbootimg.

Flashing
""""""""
Flash like regular android boot image.

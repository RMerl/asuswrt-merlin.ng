.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2022 Matthias Brugger <mbrugger@suse.com>

Raspberry Pi
============

About this
----------

This document describes the information about Raspberry Pi boards
and it's usage steps.

Raspberry Pi boards
-------------------

List of the supported Rasbperry Pi boards and the corresponding defconfig files:

32 bit
^^^^^^

* rpi_defconfig
  - Raspberry Pi
* rpi_0_w_defconfig
  - Raspberry Pi 1
  - Raspberry Pi zero
* rpi_2_defconfig
  - Raspberry Pi 2
* rpi_3_32b_defconfig
  - Raspberry Pi 3b
* rpi_4_32b_defconfig
  - Raspberry Pi 4b

64 bit
^^^^^^

* rpi_3_defconfig
  - Raspberry Pi 3b
* rpi_3_b_plus_defconfig
  - Raspberry Pi 3b+
* rpi_4_defconfig
  - Raspberry Pi 4b
* rpi_arm64_defconfig
  - Raspberry Pi 3b
  - Raspberry Pi 3b+
  - Raspberry Pi 4b
  - Raspberry Pi 400
  - Raspberry Pi CM 3
  - Raspberry Pi CM 3+
  - Raspberry Pi CM 4
  - Raspberry Pi zero 2 w

rpi_arm64_defconfig uses the device-tree provided by the firmware instead of
the embedded one. It allows to use the same U-Boot binary to boot different
boards.

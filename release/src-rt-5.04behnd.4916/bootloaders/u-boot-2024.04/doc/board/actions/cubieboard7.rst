.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2020 Amit Singh Tomar <amittomer25@gmail.com>

CUBIEBOARD7
===========

About this
----------

This document describes build and flash steps for Actions S700 SoC based Cubieboard7
board.

Cubieboard7 initial configuration
---------------------------------

Default Cubieboard7 comes with pre-installed Android where U-Boot is configured with
a bootdelay of 0, entering a prompt by pressing keys does not seem to work.

Though, one can enter ADFU mode and flash debian image(from host machine) where
getting into u-boot prompt is easy.

Enter ADFU Mode
---------------

Before write the firmware, let the development board entering the ADFU mode: insert
one end of the USB cable to the PC, press and hold the ADFU button, and then connect
the other end of the USB cable to the Mini USB port of the development board, release
the ADFU button, after connecting it will enter the ADFU mode.

Check whether entered ADFU Mode
-------------------------------

The user needs to run the following command on the PC side to check if the ADFU
device is detected. ID realted to "Actions Semiconductor Co., Ltd"  means that
the PC side has been correctly detected ADFU device, the development board
also enter into the ADFU mode.

.. code-block:: none

   $ lsusb
   Bus 001 Device 005: ID 04f2:b2eb Chicony Electronics Co., Ltd
   Bus 001 Device 004: ID 0a5c:21e6 Broadcom Corp. BCM20702 Bluetooth 4.0 [ThinkPad]
   Bus 001 Device 003: ID 046d:c534 Logitech, Inc. Unifying Receiver
   Bus 001 Device 002: ID 8087:0024 Intel Corp. Integrated Rate Matching Hub
   Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
   Bus 004 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
   Bus 003 Device 013: ID 10d6:10d6 Actions Semiconductor Co., Ltd
   Bus 003 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub

Flashing debian image
---------------------

.. code-block:: none

   $ sudo ./ActionsFWU.py --fw=debian-stretch-desktop-cb7-emmc-v2.0.fw
   ActionsFWU.py	: 1.0.150828.0830
   libScript.so    : 2.3.150825.0951
   libFileSystem.so: 2.3.150825.0952
   libProduction.so: 2.3.150915.1527
   =====burn all partition====
   FW_VER: 3.10.37.180608
   3% DOWNLOAD ADFUDEC ...
   5% DOWNLOAD BOOT PARA ...
   7% SWITCH ADFUDEC ...
   12% DOWNLOAD BL31 ...
   13% DOWNLOAD BL32 ...
   15% DOWNLOAD VMLINUX ...
   20% DOWNLOAD INITRD ...
   24% DOWNLOAD FDT ...
   27% DOWNLOAD ADFUS ...
   30% SWITCH ADFUS ...
   32% DOWNLOAD MBR ...
   35% DOWNLOAD PARTITIONS ...
   WRITE_MBRC_PARTITION
   35% write p0 size = 2048 : ok
   WRITE_BOOT_PARTITION
   35% write p1 size = 2048 : ok
   WRITE_MISC_PARTITION
   36% write p2 size = 98304 : ok
   WRITE_SYSTEM_PARTITION
   94% write p3 size = 4608000 : ok
   FORMAT_SWAP_PARTITION
   94% write p4 size = 20480 : ok
   95% TRANSFER OVER ...
   Firmware upgrade successfully!

Debian image can be downloaded from here[1].

Once debian image is flashed, one can get into u-boot prompt by pressing any key and from
there run ums command(make sure, usb cable is connected between host and target):

.. code-block:: none

   owl> ums 0 mmc 1

Above command would mount debian image partition on host machine.

Building U-BOOT proper image
----------------------------

.. code-block:: none

   $ make clean
   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ make cubieboard7_defconfig
   $ make u-boot-dtb.img -j16

u-boot-dtb.img can now be flashed to debian image partition mounted on host machine.

.. code-block:: none

   $ sudo dd if=u-boot-dtb.img of=/dev/sdb bs=1024 seek=3072

[1]: https://pan.baidu.com/s/1uawPr0Jao2HgWFLZCLzHAg#list/path=%2FCubieBoard_Download%2FBoard%2FCubieBoard7%2F%E6%96%B9%E7%B3%96%E6%96%B9%E6%A1%88%E5%BC%80%E5%8F%91%E8%B5%84%E6%96%99%2FImage%2FDebian%2FV2.1-test&parentPath=%2F

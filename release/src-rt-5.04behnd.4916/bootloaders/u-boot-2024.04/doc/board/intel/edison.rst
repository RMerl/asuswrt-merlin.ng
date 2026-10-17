.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Andy Shevchenko <andriy.shevchenko@linux.intel.com>

Edison
======

Build Instructions for U-Boot as main bootloader
------------------------------------------------

Simple you can build U-Boot and obtain u-boot.bin::

   $ make edison_defconfig
   $ make all

Updating U-Boot on Edison
-------------------------

By default Intel Edison boards are shipped with preinstalled heavily
patched U-Boot v2014.04. Though it supports DFU which we may be able to
use.

1. Prepare u-boot.bin as described in chapter above. You still need one
   more step (if and only if you have original U-Boot), i.e. run the
   following command::

     $ truncate -s %4096 u-boot.bin

2. Run your board and interrupt booting to U-Boot console. In the console
   call::

     => run do_force_flash_os

3. Wait for few seconds, it will prepare environment variable and runs
   DFU. Run DFU command from the host system::

     $ dfu-util -v -d 8087:0a99 --alt u-boot0 -D u-boot.bin

4. Return to U-Boot console and following hint. i.e. push Ctrl+C, and
   reset the board::

     => reset

Updating U-Boot using xFSTK
---------------------------

You can also update U-Boot using the xfstk-dldr-solo tool if you can build it.
One way to do that is to follow the `xFSTK`_ instructions. In short, after you
install all necessary dependencies and clone repository, it will look like this:

.. code-block:: sh

  cd xFSTK
  export DISTRIBUTION_NAME=ubuntu20.04
  export BUILD_VERSION=1.8.5
  git checkout v$BUILD_VERSION
  ...

Once you have built it, you can copy xfstk-dldr-solo to /usr/local/bin and
libboost_program_options.so.1.54.0 to /usr/lib/i386-linux-gnu/ and with luck
it will work. You might find this `drive`_ helpful.

If it does, then you can download and unpack the Edison recovery image,
install dfu-util, reset your board and flash U-Boot like this:

.. code-block:: sh

  xfstk-dldr-solo --gpflags 0x80000007 \
      --osimage u-boot-edison.img \
      --fwdnx recover/edison_dnx_fwr.bin \
      --fwimage recover/edison_ifwi-dbg-00.bin \
      --osdnx recover/edison_dnx_osr.bin

This should show the following

.. code-block:: none

  XFSTK Downloader Solo 1.8.5
  Copyright (c) 2015 Intel Corporation
  Build date and time: Aug 15 2020 15:07:13

  .Intel SoC Device Detection Found
  Parsing Commandline....
  Registering Status Callback....
  .Initiating Download Process....
  .......(lots of dots)........XFSTK-STATUS--Reconnecting to device - Attempt #1
  .......(even more dots)......................

You have about 10 seconds after resetting the board to type the above command.
If you want to check if the board is ready, type:

.. code-block:: none

  lsusb | grep -E "8087|8086"
  Bus 001 Device 004: ID 8086:e005 Intel Corp.

If you see a device with the same ID as above, the board is waiting for your
command.

After about 5 seconds you should see some console output from the board:

.. code-block:: none

  ******************************
  PSH KERNEL VERSION: b0182b2b
  		WR: 20104000
  ******************************

  SCU IPC: 0x800000d0  0xfffce92c

  PSH miaHOB version: TNG.B0.VVBD.0000000c

  microkernel built 11:24:08 Feb  5 2015

  ******* PSH loader *******
  PCM page cache size = 192 KB
  Cache Constraint = 0 Pages
  Arming IPC driver ..
  Adding page store pool ..
  PagestoreAddr(IMR Start Address) = 0x04899000
  pageStoreSize(IMR Size)          = 0x00080000

  *** Ready to receive application ***

After another 10 seconds the xFSTK tool completes and the board resets. About
10 seconds after that should see the above message again and then within a few
seconds U-Boot should start on your board:

.. code-block:: none

  U-Boot 2020.10-rc3 (Sep 03 2020 - 18:44:28 -0600)

  CPU:   Genuine Intel(R) CPU   4000  @  500MHz
  DRAM:  980.6 MiB
  WDT:   Started with servicing (60s timeout)
  MMC:   mmc@ff3fc000: 0, mmc@ff3fa000: 1
  Loading Environment from MMC... OK
  In:    serial
  Out:   serial
  Err:   serial
  Saving Environment to MMC... Writing to redundant MMC(0)... OK
  Saving Environment to MMC... Writing to MMC(0)... OK
  Net:   No ethernet found.
  Hit any key to stop autoboot:  0
  Target:blank
  Partitioning using GPT
  Writing GPT: success!
  Saving Environment to MMC... Writing to redundant MMC(0)... OK
  Flashing already done...
  5442816 bytes read in 238 ms (21.8 MiB/s)
  Valid Boot Flag
  Setup Size = 0x00003c00
  Magic signature found
  Using boot protocol version 2.0c
  Linux kernel version 3.10.17-poky-edison+ (ferry@kalamata) #1 SMP PREEMPT Mon Jan 11 14:54:18 CET 2016
  Building boot_params at 0x00090000
  Loading bzImage at address 100000 (5427456 bytes)
  Magic signature found
  Kernel command line: "rootwait ..."
  Magic signature found

  Starting kernel ...

  ...

  Poky (Yocto Project Reference Distro) 1.7.2 edison ttyMFD2

  edison login:

.. _xFSTK: https://github.com/edison-fw/xFSTK
.. _drive: https://drive.google.com/drive/u/0/folders/1URPHrOk9-UBsh8hjv-7WwC0W6Fy61uAJ

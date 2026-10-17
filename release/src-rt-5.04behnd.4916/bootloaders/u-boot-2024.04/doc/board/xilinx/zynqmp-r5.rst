.. SPDX-License-Identifier: GPL-2.0
..  (C) Copyright 2020 Xilinx, Inc.

ZYNQMP-R5
=========

About this
----------

This document describes the information about Xilinx Zynq UltraScale+ MPSOC
U-Boot Cortex R5 support.

ZynqMP R5 boards
----------------

* zynqmp-r5 - U-Boot running on RPU Cortex-R5

Building
--------

configure and build armv7 toolchain::

   $ make xilinx_zynqmp_r5_defconfig
   $ make

Notes
^^^^^

Output fragment is U-Boot.

Loading
-------

ZynqMP R5 U-Boot was created for supporting loading OS on RPU. There are two
ways how to start U-Boot on R5.

Bootgen
^^^^^^^

The first way is to use Xilinx FSBL (First stage
bootloader) to load U-Boot and start it. The following bif can be used for boot
image generation via Xilinx bootgen utility::


  the_ROM_image:
  {
  	[bootloader,destination_cpu=r5-0] fsbl_rpu.elf
  	[destination_cpu=r5-0]u-boot.elf
  }

Bootgen command for building boot.bin::

  bootgen -image <bif>.bif -r -w -o i boot.bin


U-Boot cpu command
^^^^^^^^^^^^^^^^^^

The second way to load U-Boot to Cortex R5 is from U-Boot running on A53 as is
visible from the following log::

  U-Boot SPL 2020.10-rc4-00090-g801b3d5c5757 (Sep 15 2020 - 14:07:24 +0200)
  PMUFW:	v1.1
  Loading new PMUFW cfg obj (2024 bytes)
  EL Level:	EL3
  Multiboot:	0
  Trying to boot from MMC2
  spl: could not initialize mmc. error: -19
  Trying to boot from MMC1
  spl_load_image_fat_os: error reading image u-boot.bin, err - -2
  NOTICE:  ATF running on XCZU7EG/EV/silicon v4/RTL5.1 at 0xfffea000
  NOTICE:  BL31: v2.2(release):v2.2-614-ged9dc512fb9c
  NOTICE:  BL31: Built : 09:32:09, Mar 13 2020


  U-Boot 2020.10-rc4-00090-g801b3d5c5757 (Sep 15 2020 - 14:07:24 +0200)

  Model: ZynqMP ZCU104 RevC
  Board: Xilinx ZynqMP
  DRAM:  2 GiB
  PMUFW:	v1.1
  EL Level:	EL2
  Chip ID:	zu7e
  WDT:   Started with servicing (60s timeout)
  NAND:  0 MiB
  MMC:   mmc@ff170000: 0
  Loading Environment from FAT... *** Warning - bad CRC, using default environment

  In:    serial
  Out:   serial
  Err:   serial
  Bootmode: LVL_SHFT_SD_MODE1
  Reset reason:	SOFT
  Net:
  ZYNQ GEM: ff0e0000, mdio bus ff0e0000, phyaddr 12, interface rgmii-id
  eth0: ethernet@ff0e0000
  Hit any key to stop autoboot:  0
  ZynqMP> setenv autoload no
  ZynqMP> dhcp
  BOOTP broadcast 1
  DHCP client bound to address 192.168.0.167 (8 ms)
  ZynqMP> tftpboot 20000000 192.168.0.105:u-boot-r5-2.elf
  Using ethernet@ff0e0000 device
  TFTP from server 192.168.0.105; our IP address is 192.168.0.167
  Filename 'u-boot-r5-2.elf'.
  Load address: 0x20000000
  Loading: #################################################################
  	 #################################################################
  	 #################################################################
  	 #################################################################
  	 #################################################################
  	 #################################################################
  	 ################
  	 376 KiB/s
  done
  Bytes transferred = 2075464 (1fab48 hex)
  ZynqMP> setenv autostart no
  ZynqMP> bootelf -p 20000000
  ZynqMP> cpu 4 release 10000000 lockstep
  Using TCM jump trampoline for address 0x10000000
  R5 lockstep mode
  ZynqMP>

Then on second uart you can see U-Boot up and running on R5::

  U-Boot 2020.10-rc4-00071-g7045622cc9ba (Sep 16 2020 - 13:38:53 +0200)

  Model: Xilinx ZynqMP R5
  DRAM:  512 MiB
  MMC:
  In:    serial@ff010000
  Out:   serial@ff010000
  Err:   serial@ff010000
  Net:   No ethernet found.
  ZynqMP r5>

Please make sure MIO pins for uart are properly configured to see output.

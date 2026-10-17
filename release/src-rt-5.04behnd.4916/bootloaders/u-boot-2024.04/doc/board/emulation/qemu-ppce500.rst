.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2021, Bin Meng <bmeng.cn@gmail.com>

QEMU PPC E500
=============

QEMU for PPC supports a special 'ppce500' machine designed for emulation and
virtualization purposes. This document describes how to run U-Boot under it.

The QEMU ppce500 machine models a generic PowerPC e500 virtual machine with
support for the VirtIO standard networking device connected to the built-in
PCI host controller. Some common devices in the CCSBAR space are modeled,
including MPIC, 16550A UART devices, GPIO, I2C and PCI host controller with
MSI delivery to MPIC. It uses device-tree to pass configuration information
to guest software.

Building U-Boot
---------------
Set the CROSS_COMPILE environment variable as usual, and run::

    $ make qemu-ppce500_defconfig
    $ make

Running U-Boot
--------------
The minimal QEMU command line to get U-Boot up and running is::

    $ qemu-system-ppc -nographic -machine ppce500 -bios u-boot

You can also run U-Boot using 'qemu-system-ppc64'::

    $ qemu-system-ppc64 -nographic -machine ppce500 -bios u-boot

The commands above create a target with 128 MiB memory by default. A freely
configurable amount of RAM can be created via the '-m' parameter. For example,
'-m 2G' creates 2 GiB memory for the target, and the memory node in the
embedded DTB created by QEMU reflects the new setting.

Both qemu-system-ppc and qemu-system-ppc64 provide emulation for the following
32-bit PowerPC CPUs:

* e500v1
* e500v2
* e500mc

Additionally qemu-system-ppc64 provides support for the following 64-bit CPUs:

* e5500
* e6500

The CPU type can be specified via the '-cpu' command line. If not specified,
it creates a machine with e500v2 core. The following example shows an e6500
based machine creation::

    $ qemu-system-ppc64 -nographic -machine ppce500 -cpu e6500 -bios u-boot

When U-Boot boots, you will notice the following::

    CPU:   Unknown, Version: 0.0, (0x00000000)
    Core:  e6500, Version: 2.0, (0x80400020)

This is because we only specified a core name to QEMU and it does not have a
meaningful SVR value which represents an actual SoC that integrates such core.
You can specify a real world SoC device that QEMU has built-in support but all
these SoCs are e500v1/e500v2 based MPC85xx series, hence you cannot test anything
built for P10xx/P2010/P2020 (e500v2), P204x/P304x/P40xx (e500mc), P50xx/T10xx (e5500)
and T208x/T4080/T4160/T4240 (e6500).

By default a VirtIO standard PCI networking device is connected as an ethernet
interface at PCI address 0.1.0, but we can switch that to an e1000 NIC by::

    $ qemu-system-ppc -nographic -machine ppce500 -bios u-boot \
                      -nic tap,ifname=tap0,script=no,downscript=no,model=e1000

The QEMU ppce500 machine can also dynamically instantiate an eTSEC device if
"-device eTSEC" is given to QEMU::

    -netdev tap,ifname=tap0,script=no,downscript=no,id=net0 -device eTSEC,netdev=net0

VirtIO BLK driver is also enabled to support booting from a disk image where
a kernel image is stored. Append the following to QEMU::

    -drive file=disk.img,format=raw,id=disk0 -device virtio-blk-pci,drive=disk0

Pericom pt7c4338 RTC is supported so we can use the 'date' command::

    => date
    Date: 2021-02-18 (Thursday)    Time: 15:33:20

Additionally, 'poweroff' command is supported to shut down the QEMU session::

    => poweroff
    poweroff ...

These have been tested in QEMU 5.2.0.

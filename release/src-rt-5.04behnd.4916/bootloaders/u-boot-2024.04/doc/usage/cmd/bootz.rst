.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: bootz (command)

bootz command
=============

Synopsis
--------

::

    bootz [<addr> [<initrd>[:<size>]] [<fdt>]]

Description
-----------

The bootz command is used to boot a Linux kernel in 'zImage' format.

addr
    address of kernel image, defaults to the value of the environment
    variable $loadaddr.

initrd
    address of the initial RAM disk. Use '-' to boot a kernel with a device
    tree but without an initial RAM disk.

size
    size of the initial RAM disk. This parameter must be specified for raw
    initial RAM disks.

fdt
    address of the device tree.

Example
-------

This is the boot log of an OrangePi PC board:

::

    => load mmc 0:2 $fdt_addr_r dtb
    23093 bytes read in 7 ms (3.1 MiB/s)
    => load mmc 0:2 $kernel_addr_r vmlinuz
    5079552 bytes read in 215 ms (22.5 MiB/s)
    => load mmc 0:2 $ramdisk_addr_r initrd.img
    23854965 bytes read in 995 ms (22.9 MiB/s)
    => bootz $kernel_addr_r $ramdisk_addr_r:$filesize $fdt_addr_r
    Kernel image @ 0x42000000 [ 0x000000 - 0x4d8200 ]
    ## Flattened Device Tree blob at 43000000
       Booting using the fdt blob at 0x43000000
    EHCI failed to shut down host controller.
       Loading Ramdisk to 48940000, end 49ffff75 ... OK
       Loading Device Tree to 48937000, end 4893fa34 ... OK

    Starting kernel ...

Configuration
-------------

The bootz command is only available if CONFIG_CMD_BOOTZ=y.

Return value
------------

Normally this command does not return. If an error occurs, the return value $?
is set to 1 (false). If the operating system returns to U-Boot, the system is
reset.

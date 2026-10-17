.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: booti (command)

booti command
=============

Synopsis
--------

::

    booti [<addr> [<initrd>[:<size>]] [<fdt>]]

Description
-----------

The booti command is used to boot a Linux kernel in flat or compressed
'Image' format. Which compressed formats are supported is configurable.

addr
    address of kernel image, defaults to CONFIG_SYS_LOAD_ADDR.

initrd
    address of the initial RAM disk. Use '-' to boot a kernel with a device
    tree but without an initial RAM disk.

size
    size of the initial RAM disk. This parameter must be specified for raw
    initial RAM disks.

fdt
    address of the device tree.

To support compressed Image files the following environment variables must be
set:

kernel_comp_addr_r
    start of memory area used for decompression

kernel_comp_size
    size of the compressed file. The value has to be at least the size of
    loaded image for decompression to succeed. For the booti command the
    maximum decompressed size is 10 times this value.

Example
-------

This is the boot log of an Odroid C2 board:

::

    => load mmc 0:1 $fdt_addr_r dtb-5.10.0-3-arm64
    27530 bytes read in 7 ms (3.7 MiB/s)
    => load mmc 0:1 $kernel_addr_r vmlinuz-5.10.0-3-arm64
    26990448 bytes read in 1175 ms (21.9 MiB/s)
    => load mmc 0:1 $ramdisk_addr_r initrd.img-5.10.0-3-arm64
    27421776 bytes read in 1209 ms (21.6 MiB/s)
    => booti $kernel_addr_r $ramdisk_addr_r:$filesize $fdt_addr_r
    Moving Image from 0x8080000 to 0x8200000, end=9c60000
    ## Flattened Device Tree blob at 08008000
       Booting using the fdt blob at 0x8008000
       Loading Ramdisk to 7a52a000, end 7bf50c50 ... OK
       Loading Device Tree to 000000007a520000, end 000000007a529b89 ... OK

    Starting kernel ...

The kernel can be compressed with gzip:

.. code-block:: bash

    cd /boot
    gzip -k vmlinuz-5.10.0-3-arm64

Here is the boot log for the compressed kernel:

::

    => setenv kernel_comp_addr_r 0x50000000
    => setenv kernel_comp_size 0x04000000
    => load mmc 0:1 $fdt_addr_r dtb-5.10.0-3-arm64
    27530 bytes read in 6 ms (4.4 MiB/s)
    => load mmc 0:1 $kernel_addr_r vmlinuz-5.10.0-3-arm64.gz
    9267730 bytes read in 402 ms (22 MiB/s)
    => load mmc 0:1 $ramdisk_addr_r initrd.img-5.10.0-3-arm64
    27421776 bytes read in 1181 ms (22.1 MiB/s)
    => booti $kernel_addr_r $ramdisk_addr_r:$filesize $fdt_addr_r
       Uncompressing Kernel Image
    Moving Image from 0x8080000 to 0x8200000, end=9c60000
    ## Flattened Device Tree blob at 08008000
       Booting using the fdt blob at 0x8008000
       Loading Ramdisk to 7a52a000, end 7bf50c50 ... OK
       Loading Device Tree to 000000007a520000, end 000000007a529b89 ... OK

    Starting kernel ...

Configuration
-------------

The booti command is only available if CONFIG_CMD_BOOTI=y.

Which compression types are supported depends on:

* CONFIG_BZIP2
* CONFIG_GZIP
* CONFIG_LZ4
* CONFIG_LZMA
* CONFIG_LZO
* CONFIG_ZSTD

Return value
------------

Normally this command does not return. If an error occurs, the return value $?
is set to 1 (false). If the operating system returns to U-Boot, the system is
reset.

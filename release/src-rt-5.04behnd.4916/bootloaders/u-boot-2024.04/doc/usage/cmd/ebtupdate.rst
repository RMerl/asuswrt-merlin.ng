.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: ebtupdate (command)

ebtupdate command
=================

Synopsis
--------

::

    ebtupdate [<bct> [<ebt>] [<size>]]

Description
-----------

The "ebtupdate" command is used to self-update bootloader on Tegra 2 and Tegra 3
production devices which were processed using re-cryption.

The "ebtupdate" performs encryption of new bootloader and decryption, patching
and re-encryption of BCT "in situ". After BCT and bootloader can be written in
their respective places.

bct
    address of BCT block pre-loaded into RAM.

ebt
    address of the bootloader pre-loaded into RAM.

size
    size of the pre-loaded bootloader.

Example
-------

This is the boot log of a LG Optimus Vu:

::

    => mmc dev 0 1
    switch to partitions #1, OK
    mmc0(part 1) is current device
    => mmc read $kernel_addr_r 0 $boot_block_size
    MMC read: dev # 0, block # 0, count 4096 ... 4096 blocks read: OK
    => load mmc 0:1 $ramdisk_addr_r $bootloader_file
    684783 bytes read in 44 ms (14.8 MiB/s)
    => size mmc 0:1 $bootloader_file
    => ebtupdate $kernel_addr_r $ramdisk_addr_r $filesize
    => mmc dev 0 1
    switch to partitions #1, OK
    mmc0(part 1) is current device
    => mmc write $kernel_addr_r 0 $boot_block_size
    MMC write: dev # 0, block # 0, count 4096 ... 4096 blocks written: OK
    => mmc dev 0 2
    switch to partitions #2, OK
    mmc0(part 2) is current device
    => mmc write $ramdisk_addr_r 0 $boot_block_size
    MMC write: dev # 0, block # 0, count 4096 ... 4096 blocks written: OK

Configuration
-------------

The ebtupdate command is only available if CONFIG_CMD_EBTUPDATE=y and
only on Tegra 2 and Tegra 3 configurations.

Return value
------------

The return value $? is set to 0 (true) if everything went successfully. If an
error occurs, the return value $? is set to 1 (false).

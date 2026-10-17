.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: load (command)

load command
============

Synopsis
--------

::

    load <interface> [<dev[:part]> [<addr> [<filename> [bytes [pos]]]]]

Description
-----------

The load command is used to read a file from a filesystem into memory.

The number of transferred bytes is saved in the environment variable filesize.
The load address is saved in the environment variable fileaddr.

interface
    interface for accessing the block device (mmc, sata, scsi, usb, ....)

dev
    device number

part
    partition number, defaults to 0 (whole device)

addr
    load address, defaults to environment variable loadaddr or if loadaddr is
    not set to configuration variable CONFIG_SYS_LOAD_ADDR

filename
    path to file, defaults to environment variable bootfile

bytes
    maximum number of bytes to load

pos
    number of bytes to skip

part, addr, bytes, pos are hexadecimal numbers.

Example
-------

::

    => load mmc 0:1 ${kernel_addr_r} snp.efi
    149280 bytes read in 11 ms (12.9 MiB/s)
    =>
    => load mmc 0:1 ${kernel_addr_r} snp.efi 1000000
    149280 bytes read in 9 ms (15.8 MiB/s)
    =>
    => load mmc 0:1 ${kernel_addr_r} snp.efi 1000000 100
    149024 bytes read in 10 ms (14.2 MiB/s)
    =>
    => load mmc 0:1 ${kernel_addr_r} snp.efi 10
    16 bytes read in 1 ms (15.6 KiB/s)
    =>

Configuration
-------------

The load command is only available if CONFIG_CMD_FS_GENERIC=y.

Return value
------------

The return value $? is set to 0 (true) if the file was successfully loaded
even if the number of bytes is less then the specified length.

If an error occurs, the return value $? is set to 1 (false).

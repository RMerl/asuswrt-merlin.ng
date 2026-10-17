.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: fatinfo (command)

fatinfo command
===============

Synopsis
--------

::

    fatinfo <interface> <dev[:part]>

Description
-----------

The fatinfo command displays information about a FAT partition.

interface
    interface for accessing the block device (mmc, sata, scsi, usb, ....)

dev
    device number

part
    partition number, defaults to 1

Example
-------

Here is the output for a partition on a 32 GB SD-Card:

::

    => fatinfo mmc 0:1
    Interface:  MMC
      Device 0: Vendor: Man 00001b Snr 97560602 Rev: 13.8 Prod: EB1QT0
                Type: Removable Hard Disk
                Capacity: 30528.0 MB = 29.8 GB (62521344 x 512)
    Filesystem: FAT32 "MYDISK     "
    =>

Configuration
-------------

The fatinfo command is only available if CONFIG_CMD_FAT=y.

Return value
------------

The return value $? is set to 0 (true) if the partition is a FAT partition.
Otherwise it is set to 1 (false).

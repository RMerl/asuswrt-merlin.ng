.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: mbr (command)

mbr command
===========

Synopsis
--------

::

    mbr verify [interface] [device no] [partition list]
    mbr write [interface] [device no] [partition list]

Description
-----------

The mbr command lets users create or verify the MBR (Master Boot Record)
partition layout based on the provided text description. The partition
layout is alternatively read from the 'mbr_parts' environment variable.
This can be used in scripts to help system image flashing tools to ensure
proper partition layout.

The syntax of the text description of the partition list is similar to
the one used by the 'gpt' command.

Supported partition parameters are:

* name (currently ignored)
* start (partition start offset in bytes)
* size (in bytes or '-' to expand it to the whole free area)
* bootable (boolean flag)
* id (MBR partition type)

If one wants to create more than 4 partitions, an 'Extended' primary
partition (with 0x05 ID) has to be explicitly provided as a one of the
first 4 entries.

Here is an example how to create a 6 partitions (3 on the 'extended
volume'), some of the predefined sizes:

::

    => setenv mbr_parts 'name=boot,start=4M,size=128M,bootable,id=0x0e;
        name=rootfs,size=3072M,id=0x83;
        name=system-data,size=512M,id=0x83;
        name=[ext],size=-,id=0x05;
        name=user,size=-,id=0x83;
        name=modules,size=100M,id=0x83;
        name=ramdisk,size=8M,id=0x83'
    => mbr write mmc 0

To check if the layout on the MMC #0 storage device matches the provided
text description one has to issue following command (assuming that
mbr_parts environment variable is set):

::

    => mbr verify mmc 0

The verify sub-command is especially useful in the system update scripts:

::

    => if mbr verify mmc 0; then
         echo MBR layout needs to be updated
         ...
       fi

The 'mbr write' command returns 0 on success write or 1 on failure.

The 'mbr verify' returns 0 if the layout matches the one on the storage
device or 1 if not.

Configuration
-------------

To use the mbr command you must specify CONFIG_CMD_MBR=y.

Return value
------------

The variable *$?* takes the following values

+---+------------------------------+
| 0 | mbr write was succesful      |
+---+------------------------------+
| 1 | mbr write failed             |
+---+------------------------------+
| 0 | mbr verify was succesful     |
+---+------------------------------+
| 1 | mbr verify was not succesful |
+---+------------------------------+
|-1 | invalid arguments            |
+---+------------------------------+

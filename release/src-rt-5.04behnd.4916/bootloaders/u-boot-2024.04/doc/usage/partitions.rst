.. SPDX-License-Identifier: GPL-2.0+
.. _partitions:

Partitions
==========

Synopsis
--------

::

    <command> <interface> [devnum][.hwpartnum][:partnum|#partname]

Description
-----------

Many U-Boot commands allow specifying partitions (or whole disks) using a
generic syntax.

interface
        The interface used to access the partition's device, like ``mmc`` or
        ``scsi``. For a full list of supported interfaces, consult the
        ``uclass_idname_str`` array in ``drivers/block/blk-uclass.c``

devnum
        The device number. This defaults to 0.

hwpartnum
        The hardware partition number. All devices have at least one hardware
        partition. On most devices, hardware partition 0 specifies the whole
        device. On eMMC devices, hardware partition 0 is the user partition,
        hardware partitions 1 and 2 are the boot partitions, hardware partition
        3 is the RPMB partition, and further partitions are general-purpose
        user-created partitions. The default hardware partition number is 0.

partnum
        The partition number, starting from 1. The partition number 0 specifies
        that the whole device is to be used as one "partition."

partname
        The partition name. This is the partition label for GPT partitions. For
        MBR partitions, the following syntax is used::

                <devtype><devletter><partnum>

        devtype
                A device type like ``mmcsd`` or ``hd``. See the
                ``part_set_generic_name`` function in ``disk/part.c`` for a
                complete list.

        devletter
                The device number as an offset from ``a``. For example, device
                number 2 would have a device letter of ``c``.

        partnum
                The partition number. This is the same as above.

If neither ``partname`` nor ``partnum`` is specified and there is a partition
table, then partition 1 is used. If there is no partition table, then the whole
device is used as one "partition." If none of ``devnum``, ``hwpartnum``,
``partnum``, or ``partname`` is specified, or only ``-`` is specified, then
``devnum`` defaults to the value of the ``bootdevice`` environmental variable.

Examples
--------

List the root directory contents on MMC device 2, hardware partition 1,
and partition number 3::

        ls mmc 2.1:3 /

Load ``/kernel.itb`` to address ``0x80000000`` from SCSI device 0, hardware partition
0, and the partition labeled ``boot``::

        load scsi #boot 0x80000000 /kernel.itb

Print the partition UUID of the SATA device ``$bootdevice``, hardware partition
0, and partition number 0::

        part uuid sata -

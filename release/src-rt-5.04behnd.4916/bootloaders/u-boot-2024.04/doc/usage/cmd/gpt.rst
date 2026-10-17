.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: gpt (command)

gpt command
===========

Synopsis
--------

::

    gpt enumerate <interface> <dev>
    gpt guid <interface> <dev> [<varname>]
    gpt read <interface> <dev> [<varname>]
    gpt rename <interface> <dev> <part> <name>
    gpt repair <interface> <dev>
    gpt set-bootable <interface> <dev> <partition list>
    gpt setenv <interface> <dev> <partition name>
    gpt swap <interface> <dev> <name1> <name2>
    gpt transpose <interface> <dev> <part1> <part2>
    gpt verify <interface> <dev> [<partition string>]
    gpt write <interface> <dev> <partition string>

Description
-----------

The gpt command lets users read, create, modify, or verify the GPT (GUID
Partition Table) partition layout.

Common arguments:

interface
    interface for accessing the block device (mmc, sata, scsi, usb, ....)

dev
    device number

partition string
    Describes the GPT partition layout for a disk.  The syntax is similar to
    the one used by the :doc:`mbr command <mbr>` command. The string contains
    one or more partition descriptors, each separated by a ";". Each descriptor
    contains one or more fields, with each field separated by a ",". Fields are
    either of the form "key=value" to set a specific value, or simple "flag" to
    set a boolean flag

    The first descriptor can optionally be used to describe parameters for the
    whole disk with the following fields:

    * uuid_disk=UUID - Set the UUID for the disk

    Partition descriptors can have the following fields:

    * name=<NAME> - The partition name, required
    * start=<BYTES> - The partition start offset in bytes, required
    * size=<BYTES> - The partition size in bytes or "-" to expand it to the whole free area
    * bootable - Set the legacy bootable flag
    * uuid=<UUID> - The partition UUID, optional if CONFIG_RANDOM_UUID=y is enabled
    * type=<UUID> - The partition type GUID, requires CONFIG_PARTITION_TYPE_GUID=y


    If 'uuid' is not specified, but CONFIG_RANDOM_UUID is enabled, a random UUID
    will be generated for the partition

gpt enumerate
~~~~~~~~~~~~~

Sets the variable 'gpt_partition_list' to be a list of all the partition names
on the device.

gpt guid
~~~~~~~~

Report the GUID of a disk. If 'varname' is specified, the command will set the
variable to the GUID, otherwise it will be printed out.

gpt read
~~~~~~~~

Prints the current state of the GPT partition table. If 'varname' is specified,
the variable will be filled with a partition string in the same format as a
'<partition string>', suitable for passing to other 'gpt' commands.  If the
argument is omitted, a human readable description is printed out.
CONFIG_CMD_GPT_RENAME=y is required.

gpt rename
~~~~~~~~~~

Renames all partitions named 'part' to be 'name'. CONFIG_CMD_GPT_RENAME=y is
required.

gpt repair
~~~~~~~~~~

Repairs the GPT partition tables if it they become corrupted.

gpt set-bootable
~~~~~~~~~~~~~~~~

Sets the bootable flag for all partitions in the table. If the partition name
is in 'partition list' (separated by ','), the bootable flag is set, otherwise
it is cleared. CONFIG_CMD_GPT_RENAME=y is required.

gpt setenv
~~~~~~~~~~

The 'gpt setenv' command will set a series of environment variables with
information about the partition named '<partition name>'. The variables are:

gpt_partition_addr
    the starting offset of the partition in blocks as a hexadecimal number

gpt_partition_size
    the size of the partition in blocks as a hexadecimal number

gpt_partition_name
    the name of the partition

gpt_partition_entry
    the partition number in the table, e.g. 1, 2, 3, etc.

gpt_partition_bootable
    1 if the partition is marked as bootable, 0 if not

gpt swap
~~~~~~~~

Changes the names of all partitions that are named 'name1' to be 'name2', and
all partitions named 'name2' to be 'name1'. CONFIG_CMD_GPT_RENAME=y is
required.

gpt transpose
~~~~~~~~~~~~~

Swaps the order of two partition table entries with indexes 'part1' and 'part2'
in the partition table, but otherwise leaves the actual partition data
untouched.

gpt verify
~~~~~~~~~~

Sets return value $? to 0 (true) if the partition layout on the
specified disk matches the one in the provided partition string, and 1 (false)
if it does not match. If no partition string is specified, the command will
check if the disk is partitioned or not.

gpt write
~~~~~~~~~

(Re)writes the partition table on the disk to match the provided
partition string. It returns 0 on success or 1 on failure.

Configuration
-------------

To use the 'gpt' command you must specify CONFIG_CMD_GPT=y. To enable 'gpt
read', 'gpt swap' and 'gpt rename', you must specify CONFIG_CMD_GPT_RENAME=y.

Examples
~~~~~~~~

Create 6 partitions on a disk::

    => setenv gpt_parts 'uuid_disk=bec9fc2a-86c1-483d-8a0e-0109732277d7;
        name=boot,start=4M,size=128M,bootable,type=ebd0a0a2-b9e5-4433-87c0-68b6b72699c7,
        name=rootfs,size=3072M,type=0fc63daf-8483-4772-8e79-3d69d8477de4;
        name=system-data,size=512M,type=0fc63daf-8483-4772-8e79-3d69d8477de4;
        name=[ext],size=-,type=0fc63daf-8483-4772-8e79-3d69d8477de4;
        name=user,size=-,type=0fc63daf-8483-4772-8e79-3d69d8477de4;
        name=modules,size=100M,type=0fc63daf-8483-4772-8e79-3d69d8477de4;
        name=ramdisk,size=8M,type=0fc63daf-8483-4772-8e79-3d69d8477de4
    => gpt write mmc 0 $gpt_parts


Verify that the device matches the partition layout described in the variable
$gpt_parts::

    => gpt verify mmc 0 $gpt_parts


Get the information about the partition named 'rootfs'::

    => gpt setenv mmc 0 rootfs
    => echo ${gpt_partition_addr}
    2000
    => echo ${gpt_partition_size}
    14a000
    => echo ${gpt_partition_name}
    rootfs
    => echo ${gpt_partition_entry}
    2
    => echo ${gpt_partition_bootable}
    0

Get the list of partition names on the disk::

    => gpt enumerate
    => echo ${gpt_partition_list}
    boot rootfs system-data [ext] user modules ramdisk

Get the GUID for a disk::

    => gpt guid mmc 0
    bec9fc2a-86c1-483d-8a0e-0109732277d7
    => gpt guid mmc gpt_disk_uuid
    => echo ${gpt_disk_uuid}
    bec9fc2a-86c1-483d-8a0e-0109732277d7

Set the bootable flag for the 'boot' partition and clear it for all others::

    => gpt set-bootable mmc 0 boot

Swap the order of the 'boot' and 'rootfs' partition table entries::

    => gpt setenv mmc 0 rootfs
    => echo ${gpt_partition_entry}
    2
    => gpt setenv mmc 0 boot
    => echo ${gpt_partition_entry}
    1

    => gpt transpose mmc 0 1 2

    => gpt setenv mmc 0 rootfs
    => echo ${gpt_partition_entry}
    1
    => gpt setenv mmc 0 boot
    => echo ${gpt_partition_entry}
    2

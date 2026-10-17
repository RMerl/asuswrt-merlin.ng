.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: part (command)

part command
============

Synopsis
--------

::

    part uuid <interface> <dev>:<part> [varname]
    part list <interface> <dev> [flags] [varname]
    part start <interface> <dev> <part> <varname>
    part size <interface> <dev> <part> <varname>
    part number <interface> <dev> <part> <varname>
    part set <interface> <dev> <part> <type>
    part type <interface> <dev>:<part> [varname]
    part types

Description
-----------

The `part` command is used to manage disk partition related commands.

The 'part uuid' command prints or sets an environment variable to partition UUID

    interface
        interface for accessing the block device (mmc, sata, scsi, usb, ....)
    dev
        device number
    part
        partition number
    varname
        an optional environment variable to store the current partition UUID value into.

The 'part list' command prints or sets an environment variable to the list of partitions

    interface
        interface for accessing the block device (mmc, sata, scsi, usb, ....)
    dev
        device number
    part
        partition number
    flags
        -bootable
            lists only bootable partitions
    varname
        an optional environment variable to store the list of partitions value into.

The 'part start' commnad sets an environment variable to the start of the partition (in blocks),
part can be either partition number or partition name.

    interface
        interface for accessing the block device (mmc, sata, scsi, usb, ....)
    dev
        device number
    part
        partition number
    varname
        a variable to store the current start of the partition value into.

The 'part size' command sets an environment variable to the size of the partition (in blocks),
part can be either partition number or partition name.

    interface
        interface for accessing the block device (mmc, sata, scsi, usb, ....)
    dev
        device number
    part
        partition number
    varname
        a variable to store the current size of the partition value into.

The 'part number' command sets an environment variable to the partition number using the partition name,
part must be specified as partition name.

    interface
        interface for accessing the block device (mmc, sata, scsi, usb, ....)
    dev
        device number
    part
        partition number
    varname
        a variable to store the current partition number value into

The 'part set' command sets the type of a partition. This is useful when
autodetection fails or does not do the correct thing:

    interface
        interface for accessing the block device (mmc, sata, scsi, usb, ....)
    dev
        device number
    part
        partition number
    type
        partition type to use (see 'part types') to check available types

The 'part type' command prints or sets an environment variable to the partition type UUID.

    interface
        interface for accessing the block device (mmc, sata, scsi, usb, ....)
    dev
        device number
    part
        partition number
    varname
        a variable to store the current partition type UUID value into

The 'part types' command list supported partition table types.

Examples
--------

::

    => host bind 0 ./test_gpt_disk_image.bin
    => part uuid host 0:1
    24156b69-3378-497f-bb3e-b982223de528
    => part uuid host 0:1 varname
    => env print varname
    varname=24156b69-3378-497f-bb3e-b982223de528
    =>
    => part list host 0

    Partition Map for HOST device 0  --   Partition Type: EFI

    Part	Start LBA	End LBA		Name
    Attributes
    Type GUID
    Partition GUID
    1	        0x00000800	0x00000fff	"second"
    attrs:	0x0000000000000000
    type:	ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
                (data)
    guid:	24156b69-3378-497f-bb3e-b982223de528
    2	        0x00001000	0x00001bff	"first"
    attrs:	0x0000000000000000
    type:	ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
                (data)
    guid:	5272ee44-29ab-4d46-af6c-4b45ac67d3b7
    =>
    => part start host 0 2 varname
    => env print varname
    varname=1000
    =>
    => part size host 0 2 varname
    => env print varname
    varname=c00
    =>
    => part number host 0 2 varname
    => env print varname
    varname=0x2
    =>
    => part type host 0:1
    ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
    => part type host 0:1 varname
    => env print varname
    varname=ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
    =>
    => part types
    Supported partition tables: EFI, AMIGA, DOS, ISO, MAC

This shows looking at a device with multiple partition tables::

    => virtio scan
    => part list virtio 0

    Partition Map for VirtIO device 0  --   Partition Type: EFI

    Part	Start LBA	End LBA		Name
            Attributes
            Type GUID
            Partition GUID
    1	0x00000040	0x0092b093	"ISO9660"
            attrs:	0x1000000000000001
            type:	ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
            guid:	a0891d7e-b930-4513-94d8-f629dbd637b2
    2	0x0092b094	0x0092d7e7	"Appended2"
            attrs:	0x0000000000000000
            type:	c12a7328-f81f-11d2-ba4b-00a0c93ec93b
            guid:	a0891d7e-b930-4513-94db-f629dbd637b2
    3	0x0092d7e8	0x0092da3f	"Gap1"
            attrs:	0x1000000000000001
            type:	ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
            guid:	a0891d7e-b930-4513-94da-f629dbd637b2
    => ls virtio 0:3
    => part types
    Supported partition tables: EFI, DOS, ISO
    => part set virtio 0 dos

    Partition Map for VirtIO device 0  --   Partition Type: DOS

    Part	Start Sector	Num Sectors	UUID		Type
    1	1         	9624191   	00000000-01	ee
    => part set virtio 0 iso

    Partition Map for VirtIO device 0  --   Partition Type: ISO

    Part   Start     Sect x Size Type
    1     3020        4    512 U-Boot
    2  9613460    10068    512 U-Boot
    => part set virtio 0 efi

    Partition Map for VirtIO device 0  --   Partition Type: EFI

    Part	Start LBA	End LBA		Name
            Attributes
            Type GUID
            Partition GUID
    1	0x00000040	0x0092b093	"ISO9660"
            attrs:	0x1000000000000001
            type:	ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
            guid:	a0891d7e-b930-4513-94d8-f629dbd637b2
    2	0x0092b094	0x0092d7e7	"Appended2"
            attrs:	0x0000000000000000
            type:	c12a7328-f81f-11d2-ba4b-00a0c93ec93b
            guid:	a0891d7e-b930-4513-94db-f629dbd637b2
    3	0x0092d7e8	0x0092da3f	"Gap1"
            attrs:	0x1000000000000001
            type:	ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
            guid:	a0891d7e-b930-4513-94da-f629dbd637b2
    =>

Return value
------------

The return value $? is set to 0 (true) if the command succededd. If an
error occurs, the return value $? is set to 1 (false).

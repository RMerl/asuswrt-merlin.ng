.. SPDX-License-Identifier: GPL-2.0-or-later:

read and write commands
=======================

Synopsis
--------

::

    read <interface> <dev[:part|#partname]> <addr> <blk#> <cnt>
    write <interface> <dev[:part|#partname]> <addr> <blk#> <cnt>

The read and write commands can be used for raw access to data in
block devices (or partitions therein), i.e. without going through a
file system.

read
----

The block device is specified using the <interface> (e.g. "mmc") and
<dev> parameters. If the block device has a partition table, one can
optionally specify a partition number (using the :part syntax) or
partition name (using the #partname syntax). The command then reads
the <cnt> blocks of data starting at block number <blk#> of the given
device/partition to the memory address <addr>.

write
-----

The write command is completely equivalent to the read command, except
of course that the transfer direction is reversed.

Examples
--------

    # Read 2 MiB from partition 3 of mmc device 2 to $loadaddr
    read mmc 2.3 $loadaddr 0 0x1000

    # Read 16 MiB from the partition named 'kernel' of mmc device 1 to $loadaddr
    read mmc 1#kernel $loadaddr 0 0x8000

    # Write to the third sector of the partition named 'bootdata' of mmc device 0
    write mmc 0#bootdata $loadaddr 2 1

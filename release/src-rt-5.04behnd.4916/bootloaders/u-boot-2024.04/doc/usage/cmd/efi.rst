.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>

.. index::
   single: efi (command)

efi command
===========

Synopsis
--------

::

    efi mem [all]
    efi tables

Description
-----------

The *efi* command provides information about the EFI environment U-Boot is
running in, when it is started from EFI.

When running as an EFI app, this command queries EFI boot services for the
information. When running as an EFI payload, EFI boot services have been
stopped, so it uses the information collected by the boot stub before that
happened.

efi mem
~~~~~~~

This shows the EFI memory map, sorted in order of physical address.

This is normally a very large table. To help reduce the amount of detritus,
boot-time memory is normally merged with conventional memory. Use the 'all'
argument to show everything.

The fields are as follows:

#
    Entry number (sequentially from 0)

Type
    Memory type. EFI has a large number of memory types. The type is shown in
    the format <n>:<name> where in is the format number in hex and <name> is the
    name.

Physical
    Physical address

Virtual
    Virtual address

Size
    Size of memory area in bytes

Attributes
    Shows a code for memory attributes. The key for this is shown below the
    table.

efi tables
~~~~~~~~~~

This shows a list of the EFI tables provided in the system table. These use
GUIDs so it is not possible in general to show the name of a table. But some
effort is made to provide a useful table, where the GUID is known by U-Boot.


Example
-------

::

    => efi mem
    EFI table at 0, memory map 000000001ad38b60, size 1260, key a79, version 1, descr. size 0x30
     #  Type              Physical     Virtual        Size  Attributes
     0  7:conv          0000000000  0000000000  00000a0000  f
        <gap>           00000a0000              0000060000
     1  7:conv          0000100000  0000000000  0000700000  f
     2  a:acpi_nvs      0000800000  0000000000  0000008000  f
     3  7:conv          0000808000  0000000000  0000008000  f
     4  a:acpi_nvs      0000810000  0000000000  00000f0000  f
     5  7:conv          0000900000  0000000000  001efef000  f
     6  6:rt_data       001f8ef000  0000000000  0000100000  rf
     7  5:rt_code       001f9ef000  0000000000  0000100000  rf
     8  0:reserved      001faef000  0000000000  0000080000  f
     9  9:acpi_reclaim  001fb6f000  0000000000  0000010000  f
    10  a:acpi_nvs      001fb7f000  0000000000  0000080000  f
    11  7:conv          001fbff000  0000000000  0000359000  f
    12  6:rt_data       001ff58000  0000000000  0000020000  rf
    13  a:acpi_nvs      001ff78000  0000000000  0000088000  f
        <gap>           0020000000              0090000000
    14  0:reserved      00b0000000  0000000000  0010000000  1

    Attributes key:
     f: uncached, write-coalescing, write-through, write-back
    rf: uncached, write-coalescing, write-through, write-back, needs runtime mapping
     1: uncached
    *Some areas are merged (use 'all' to see)


    => efi mem  all
    EFI table at 0, memory map 000000001ad38bb0, size 1260, key a79, version 1, descr. size 0x30
     #  Type              Physical     Virtual        Size  Attributes
     0  3:bs_code       0000000000  0000000000  0000001000  f
     1  7:conv          0000001000  0000000000  000009f000  f
        <gap>           00000a0000              0000060000
     2  7:conv          0000100000  0000000000  0000700000  f
     3  a:acpi_nvs      0000800000  0000000000  0000008000  f
     4  7:conv          0000808000  0000000000  0000008000  f
     5  a:acpi_nvs      0000810000  0000000000  00000f0000  f
     6  4:bs_data       0000900000  0000000000  0000c00000  f
     7  7:conv          0001500000  0000000000  000aa36000  f
     8  2:loader_data   000bf36000  0000000000  0010000000  f
     9  4:bs_data       001bf36000  0000000000  0000020000  f
    10  7:conv          001bf56000  0000000000  00021e1000  f
    11  1:loader_code   001e137000  0000000000  00000c4000  f
    12  7:conv          001e1fb000  0000000000  000009b000  f
    13  1:loader_code   001e296000  0000000000  00000e2000  f
    14  7:conv          001e378000  0000000000  000005b000  f
    15  4:bs_data       001e3d3000  0000000000  000001e000  f
    16  7:conv          001e3f1000  0000000000  0000016000  f
    17  4:bs_data       001e407000  0000000000  0000016000  f
    18  2:loader_data   001e41d000  0000000000  0000002000  f
    19  4:bs_data       001e41f000  0000000000  0000828000  f
    20  3:bs_code       001ec47000  0000000000  0000045000  f
    21  4:bs_data       001ec8c000  0000000000  0000001000  f
    22  3:bs_code       001ec8d000  0000000000  000000e000  f
    23  4:bs_data       001ec9b000  0000000000  0000001000  f
    24  3:bs_code       001ec9c000  0000000000  000002c000  f
    25  4:bs_data       001ecc8000  0000000000  0000001000  f
    26  3:bs_code       001ecc9000  0000000000  000000c000  f
    27  4:bs_data       001ecd5000  0000000000  0000006000  f
    28  3:bs_code       001ecdb000  0000000000  0000014000  f
    29  4:bs_data       001ecef000  0000000000  0000001000  f
    30  3:bs_code       001ecf0000  0000000000  000005b000  f
    31  4:bs_data       001ed4b000  0000000000  000000b000  f
    32  3:bs_code       001ed56000  0000000000  0000024000  f
    33  4:bs_data       001ed7a000  0000000000  0000006000  f
    34  3:bs_code       001ed80000  0000000000  0000010000  f
    35  4:bs_data       001ed90000  0000000000  0000002000  f
    36  3:bs_code       001ed92000  0000000000  0000025000  f
    37  4:bs_data       001edb7000  0000000000  0000003000  f
    38  3:bs_code       001edba000  0000000000  0000011000  f
    39  4:bs_data       001edcb000  0000000000  0000008000  f
    40  3:bs_code       001edd3000  0000000000  000002d000  f
    41  4:bs_data       001ee00000  0000000000  0000201000  f
    42  3:bs_code       001f001000  0000000000  0000024000  f
    43  4:bs_data       001f025000  0000000000  0000002000  f
    44  3:bs_code       001f027000  0000000000  0000009000  f
    45  4:bs_data       001f030000  0000000000  0000005000  f
    46  3:bs_code       001f035000  0000000000  000002f000  f
    47  4:bs_data       001f064000  0000000000  0000001000  f
    48  3:bs_code       001f065000  0000000000  0000005000  f
    49  4:bs_data       001f06a000  0000000000  0000005000  f
    50  3:bs_code       001f06f000  0000000000  0000007000  f
    51  4:bs_data       001f076000  0000000000  0000007000  f
    52  3:bs_code       001f07d000  0000000000  000000d000  f
    53  4:bs_data       001f08a000  0000000000  0000001000  f
    54  3:bs_code       001f08b000  0000000000  0000006000  f
    55  4:bs_data       001f091000  0000000000  0000004000  f
    56  3:bs_code       001f095000  0000000000  000000d000  f
    57  4:bs_data       001f0a2000  0000000000  0000003000  f
    58  3:bs_code       001f0a5000  0000000000  0000026000  f
    59  4:bs_data       001f0cb000  0000000000  0000005000  f
    60  3:bs_code       001f0d0000  0000000000  0000019000  f
    61  4:bs_data       001f0e9000  0000000000  0000004000  f
    62  3:bs_code       001f0ed000  0000000000  0000024000  f
    63  4:bs_data       001f111000  0000000000  0000008000  f
    64  3:bs_code       001f119000  0000000000  000000b000  f
    65  4:bs_data       001f124000  0000000000  0000001000  f
    66  3:bs_code       001f125000  0000000000  0000002000  f
    67  4:bs_data       001f127000  0000000000  0000002000  f
    68  3:bs_code       001f129000  0000000000  0000009000  f
    69  4:bs_data       001f132000  0000000000  0000003000  f
    70  3:bs_code       001f135000  0000000000  0000005000  f
    71  4:bs_data       001f13a000  0000000000  0000003000  f
    72  3:bs_code       001f13d000  0000000000  0000005000  f
    73  4:bs_data       001f142000  0000000000  0000003000  f
    74  3:bs_code       001f145000  0000000000  0000011000  f
    75  4:bs_data       001f156000  0000000000  000000b000  f
    76  3:bs_code       001f161000  0000000000  0000009000  f
    77  4:bs_data       001f16a000  0000000000  0000400000  f
    78  3:bs_code       001f56a000  0000000000  0000006000  f
    79  4:bs_data       001f570000  0000000000  0000001000  f
    80  3:bs_code       001f571000  0000000000  0000001000  f
    81  4:bs_data       001f572000  0000000000  0000002000  f
    82  3:bs_code       001f574000  0000000000  0000017000  f
    83  4:bs_data       001f58b000  0000000000  0000364000  f
    84  6:rt_data       001f8ef000  0000000000  0000100000  rf
    85  5:rt_code       001f9ef000  0000000000  0000100000  rf
    86  0:reserved      001faef000  0000000000  0000080000  f
    87  9:acpi_reclaim  001fb6f000  0000000000  0000010000  f
    88  a:acpi_nvs      001fb7f000  0000000000  0000080000  f
    89  4:bs_data       001fbff000  0000000000  0000201000  f
    90  7:conv          001fe00000  0000000000  00000e8000  f
    91  4:bs_data       001fee8000  0000000000  0000020000  f
    92  3:bs_code       001ff08000  0000000000  0000026000  f
    93  4:bs_data       001ff2e000  0000000000  0000009000  f
    94  3:bs_code       001ff37000  0000000000  0000021000  f
    95  6:rt_data       001ff58000  0000000000  0000020000  rf
    96  a:acpi_nvs      001ff78000  0000000000  0000088000  f
        <gap>           0020000000              0090000000
    97  0:reserved      00b0000000  0000000000  0010000000  1

    Attributes key:
     f: uncached, write-coalescing, write-through, write-back
    rf: uncached, write-coalescing, write-through, write-back, needs runtime mapping
     1: uncached


    => efi tables
    000000001f8edf98  ee4e5898-3914-4259-9d6e-dc7bd79403cf  EFI_LZMA_COMPRESSED
    000000001ff2ace0  05ad34ba-6f02-4214-952e-4da0398e2bb9  EFI_DXE_SERVICES
    000000001f8ea018  7739f24c-93d7-11d4-9a3a-0090273fc14d  EFI_HOB_LIST
    000000001ff2bac0  4c19049f-4137-4dd3-9c10-8b97a83ffdfa  EFI_MEMORY_TYPE
    000000001ff2cb10  49152e77-1ada-4764-b7a2-7afefed95e8b  (unknown)
    000000001f9ac018  060cc026-4c0d-4dda-8f41-595fef00a502  EFI_MEM_STATUS_CODE_REC
    000000001f9ab000  eb9d2d31-2d88-11d3-9a16-0090273fc14d  SMBIOS table
    000000001fb7e000  eb9d2d30-2d88-11d3-9a16-0090273fc14d  EFI_GUID_EFI_ACPI1
    000000001fb7e014  8868e871-e4f1-11d3-bc22-0080c73c8881  ACPI table
    000000001e654018  dcfa911d-26eb-469f-a220-38b7dc461220  (unknown)

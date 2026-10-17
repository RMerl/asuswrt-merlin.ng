.. SPDX-License-Identifier: GPL-2.0-or-later:

smbios command
==============

Synopsis
--------

::

        smbios

Description
-----------

The smbios command displays information from the SMBIOS tables.

Examples
--------

The example below shows an example output of the smbios command.

::

    => smbios
    SMBIOS 2.8.0 present.
    8 structures occupying 81 bytes
    Table at 0x6d35018

    Handle 0x0100, DMI type 1, 27 bytes at 0x6d35018
    System Information
        Manufacturer: QEMU
        Product Name: Standard PC (i440FX + PIIX, 1996)
        Version: pc-i440fx-2.5
        Serial Number:
        UUID 00000000-0000-0000-0000-000000000000
        Wake Up Type:
        Serial Number:
        SKU Number:

    Handle 0x0300, DMI type 3, 22 bytes at 0x6d35069
    Header and Data:
        00000000: 03 16 00 03 01 01 02 00 00 03 03 03 02 00 00 00
        00000010: 00 00 00 00 00 00
    Strings:
        String 1: QEMU
        String 2: pc-i440fx-2.5

    Handle 0x0400, DMI type 4, 42 bytes at 0x6d35093
    Header and Data:
        00000000: 04 2a 00 04 01 03 01 02 63 06 00 00 fd ab 81 07
        00000010: 03 00 00 00 d0 07 d0 07 41 01 ff ff ff ff ff ff
        00000020: 00 00 00 01 01 01 02 00 01 00
    Strings:
        String 1: CPU 0
        String 2: QEMU
        String 3: pc-i440fx-2.5

    Handle 0x1000, DMI type 16, 23 bytes at 0x6d350d7
    Header and Data:
        00000000: 10 17 00 10 01 03 06 00 00 02 00 fe ff 01 00 00
        00000010: 00 00 00 00 00 00 00

    Handle 0x1100, DMI type 17, 40 bytes at 0x6d350f0
    Header and Data:
        00000000: 11 28 00 11 00 10 fe ff ff ff ff ff 80 00 09 00
        00000010: 01 00 07 02 00 00 00 02 00 00 00 00 00 00 00 00
        00000020: 00 00 00 00 00 00 00 00
    Strings:
        String 1: DIMM 0
        String 2: QEMU

    Handle 0x1300, DMI type 19, 31 bytes at 0x6d35125
    Header and Data:
        00000000: 13 1f 00 13 00 00 00 00 ff ff 01 00 00 10 01 00
        00000010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

    Handle 0x2000, DMI type 32, 11 bytes at 0x6d35146
    Header and Data:
        00000000: 20 0b 00 20 00 00 00 00 00 00 00

    Handle 0x7f00, DMI type 127, 4 bytes at 0x6d35153
    End Of Table

Configuration
-------------

The command is only available if CONFIG_CMD_SMBIOS=y.

Return value
------------

The return value $? is 0 (true) on success, 1 (false) otherwise.

.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: cmp (command)

cmp command
===========

Synopsis
--------

::

    cmp [.b, .w, .l, .q] addr1 addr2 count

Description
-----------

The cmp command is used to compare two memory areas. By default it works on
four byte (32-bit) values. By appending .b, .w, .l, .q the size of the
values is controlled:

cmp.b
    compare 1 byte (8-bit) values

cmp.w
    compare 2 byte (16-bit) values

cmp.l
    compare 4 byte (32-bit) values

cmp.q
    compare 8 byte (64-bit) values

The parameters are used as follows:

addr1
    Address of the first memory area.

addr2
    Address of the second memory area.

count
    Number of bytes to compare (as hexadecimal number).

Example
-------

In the example below the strings "Hello world\n" and "Hello World\n" are written
to memory and then compared.

::

    => mm.b 0x1000000
    01000000: 00 ? 48
    01000001: 00 ? 65
    01000002: 00 ? 6c
    01000003: 00 ? 6c
    01000004: 00 ? 6f
    01000005: 00 ? 20
    01000006: 00 ? 77
    01000007: 00 ? 6f
    01000008: 00 ? 72
    01000009: 00 ? 6c
    0100000a: 00 ? 64
    0100000b: 00 ? 0d
    0100000c: 00 ? => <INTERRUPT>
    => mm.b 0x101000
    00101000: 00 ? 48
    00101001: 00 ? 65
    00101002: 00 ? 6c
    00101003: 00 ? 6c
    00101004: 00 ? 6f
    00101005: 00 ? 20
    00101006: 00 ? 57
    00101007: 00 ? 6f
    00101008: 00 ? 72
    00101009: 00 ? 6c
    0010100a: 00 ? 64
    0010100b: 00 ? 0d
    0010100c: 00 ? => <INTERRUPT>
    => cmp 0x1000000 0x101000 0xc
    word at 0x01000004 (0x6f77206f) != word at 0x00101004 (0x6f57206f)
    Total of 1 word(s) were the same
    => cmp.b 0x1000000 0x101000 0xc
    byte at 0x01000006 (0x77) != byte at 0x00101006 (0x57)
    Total of 6 byte(s) were the same
    => cmp.w 0x1000000 0x101000 0xc
    halfword at 0x01000006 (0x6f77) != halfword at 0x00101006 (0x6f57)
    Total of 3 halfword(s) were the same
    => cmp.l 0x1000000 0x101000 0xc
    word at 0x01000004 (0x6f77206f) != word at 0x00101004 (0x6f57206f)
    Total of 1 word(s) were the same
    => cmp.q 0x1000000 0x101000 0xc
    double word at 0x01000000 (0x6f77206f6c6c6548) != double word at 0x00101000 (0x6f57206f6c6c6548)
    Total of 0 double word(s) were the same

Configuration
-------------

The cmp command is only available if CONFIG_CMD_MEMORY=y. The cmp.q command is
only available on 64-bit targets.

Return value
------------

The return value $? is true (0) if the compared memory areas are equal.
The reutrn value is false (1) if the compared memory areas differ.

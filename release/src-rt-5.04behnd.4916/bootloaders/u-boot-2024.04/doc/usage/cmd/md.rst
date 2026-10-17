.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: md (command)

md command
==========

Synopsis
--------

::

    md <address>[<data_size>] [<length>]

Description
-----------

The md command is used to dump the contents of memory. It uses a standard
format that includes the address, hex data and ASCII display. It supports
various data sizes and uses the endianness of the target.

The specified data_size and length become the defaults for future memory
commands commands.

address
    start address to display

data_size
    size of each value to display (defaults to .l):

    =========  ===================
    data_size  Output size
    =========  ===================
    .b         byte
    .w         word (16 bits)
    .l         long (32 bits)
    .q         quadword (64 bits)
    =========  ===================

length
    number of values to dump. Defaults to 40 (0d64). Note that this is not
    the same as the number of bytes, unless .b is used.

Note that the format of 'md.b' can be emulated from linux with::

    # This works but requires using sed to get the extra spaces
    # <addr> is the address, <f> is the filename
    xxd -o <addr> -g1 <f> |sed 's/  /    /' >bad

    # This uses a single tool but the offset always starts at 0
    # <f> is the filename
    hexdump -v -e '"%08.8_ax: " 16/1 "%02x " "    "' -e '16/1 "%_p" "\n" ' <f>


Example
-------

::

    => md 10000
    00010000: 00010000 00000000 f0f30f00 00005596    .............U..
    00010010: 10011010 00000000 10011010 00000000    ................
    00010020: 10011050 00000000 b96d4cd8 00007fff    P........Lm.....
    00010030: 00000000 00000000 f0f30f18 00005596    .............U..
    00010040: 10011040 00000000 10011040 00000000    @.......@.......
    00010050: b96d4cd8 00007fff 10011020 00000000    .Lm..... .......
    00010060: 00000003 000000c3 00000000 00000000    ................
    00010070: 00000000 00000000 f0e892f3 00005596    .............U..
    00010080: 00000000 000000a1 00000000 00000000    ................
    00010090: 00000000 00000000 f0e38aa6 00005596    .............U..
    000100a0: 00000000 000000a6 00000022 00000000    ........".......
    000100b0: 00000001 00000000 f0e38aa1 00005596    .............U..
    000100c0: 00000000 000000be 00000000 00000000    ................
    000100d0: 00000000 00000000 00000000 00000000    ................
    000100e0: 00000000 00000000 00000000 00000000    ................
    000100f0: 00000000 00000000 00000000 00000000    ................
    => md.b 10000
    00010000: 00 00 01 00 00 00 00 00 00 0f f3 f0 96 55 00 00    .............U..
    00010010: 10 10 01 10 00 00 00 00 10 10 01 10 00 00 00 00    ................
    00010020: 50 10 01 10 00 00 00 00 d8 4c 6d b9 ff 7f 00 00    P........Lm.....
    00010030: 00 00 00 00 00 00 00 00 18 0f f3 f0 96 55 00 00    .............U..
    => md.b 10000 10
    00010000: 00 00 01 00 00 00 00 00 00 0f f3 f0 96 55 00 00    .............U..
    =>
    00010010: 10 10 01 10 00 00 00 00 10 10 01 10 00 00 00 00    ................
    =>
    00010020: 50 10 01 10 00 00 00 00 d8 4c 6d b9 ff 7f 00 00    P........Lm.....
    =>
    => md.q 10000
    00010000: 0000000000010000 00005596f0f30f00    .............U..
    00010010: 0000000010011010 0000000010011010    ................
    00010020: 0000000010011050 00007fffb96d4cd8    P........Lm.....
    00010030: 0000000000000000 00005596f0f30f18    .............U..
    00010040: 0000000010011040 0000000010011040    @.......@.......
    00010050: 00007fffb96d4cd8 0000000010011020    .Lm..... .......
    00010060: 000000c300000003 0000000000000000    ................
    00010070: 0000000000000000 00005596f0e892f3    .............U..

The empty commands cause a 'repeat', so that md shows the next available data
in the same format as before.


Return value
------------

The return value $? is always 0 (true).

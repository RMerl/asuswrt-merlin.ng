.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: sf (command)

sf command
==========

Synopsis
--------

::

    sf probe [[[<bus>:]<cs>] [<hz> [<mode>]]]
    sf read <addr> <offset>|<partition> <len>
    sf write <addr> <offset>|<partition> <len>
    sf erase <offset>|<partition> <len>
    sf update <addr> <offset>|<partition> <len>
    sf protect lock|unlock <sector> <len>
    sf test <offset>|<partition> <len>

Description
-----------

The *sf* command is used to access SPI flash, supporting read/write/erase and
a few other functions.

Probe
-----

The flash must first be probed with *sf probe* before any of the other
subcommands can be used. All of the parameters are optional:

bus
	SPI bus number containing the SPI-flash chip, e.g. 0. If you don't know
	the number, you can use 'dm uclass' to see all the spi devices,
	and check the value for 'seq' for each one (here 0 and 2)::

	   uclass 89: spi
	   0     spi@0 @ 05484960, seq 0
	   1     spi@1 @ 05484b40, seq 2

cs
	SPI chip-select to use for the chip. This is often 0 and can be omitted,
	but in some cases multiple slaves are attached to a SPI controller,
	selected by a chip-select line for each one.

hz
	Speed of the SPI bus in hertz. This normally defaults to 100000, i.e.
	100KHz, which is very slow. Note that if the device exists in the
	device tree, there might be a speed provided there, in which case this
	setting is ignored.

mode
	SPI mode to use:

	=====  ================
	Mode   Meaning
	=====  ================
	0      CPOL=0, CPHA=0
	1      CPOL=0, CPHA=1
	2      CPOL=1, CPHA=0
	3      CPOL=1, CPHA=1
	=====  ================

	Clock phase (CPHA) 0 means that data is transferred (sampled) on the
	first clock edge; 1 means the second.

	Clock polarity (CPOL) controls the idle state of the clock, 0 for low,
	1 for high.
	The active state is the opposite of idle.

	You may find this `SPI documentation`_ useful.

Parameters for other subcommands (described below) are as follows:

addr
	Memory address to start transfer

offset
	Flash offset to start transfer

partition
	If the parameter is not numeric, it is assumed to be a partition
	description in the format <dev_type><dev_num>,<part_num> which is not
	covered here. This requires CONFIG_CMD_MTDPARTS.

len
	Number of bytes to transfer

Read
~~~~

Use *sf read* to read from SPI flash to memory. The read will fail if an
attempt is made to read past the end of the flash.


Write
~~~~~

Use *sf write* to write from memory to SPI flash. The SPI flash should be
erased first, since otherwise the result is undefined.

The write will fail if an attempt is made to read past the end of the flash.


Erase
~~~~~

Use *sf erase* to erase a region of SPI flash. The erase will fail if any part
of the region to be erased is protected or lies past the end of the flash. It
may also fail if the start offset or length are not aligned to an erase region
(e.g. 256 bytes).


Update
~~~~~~

Use *sf update* to automatically erase and update a region of SPI flash from
memory. This works a sector at a time (typical 4KB or 64KB). For each
sector it first checks if the sector already has the right data. If so it is
skipped. If not, the sector is erased and the new data written. Note that if
the length is not a multiple of the erase size, the space after the data in
the last sector will be erased. If the offset does not start at the beginning
of an erase block, the operation will fail.

Speed statistics are shown including the number of bytes that were already
correct.


Protect
~~~~~~~

SPI-flash chips often have a protection feature where the chip is split up into
regions which can be locked or unlocked. With *sf protect* it is possible to
change these settings, if supported by the driver.

lock|unlock
	Selects whether to lock or unlock the sectors

<sector>
	Start sector number to lock/unlock. This may be the byte offset or some
	other value, depending on the chip.

<len>
	Number of bytes to lock/unlock


Test
~~~~

A convenient and fast *sf test* subcommand provides a way to check that SPI
flash is working as expected. This works in four stages:

   * erase - erases the entire region
   * check - checks that the region is erased
   * write - writes a test pattern to the region, consisting of the U-Boot code
   * read - reads back the test pattern to check that it was written correctly

Memory is allocated for two buffers, each <len> bytes in size. At typical
size is 64KB to 1MB. The offset and size must be aligned to an erase boundary.

Note that this test will fail if any part of the SPI flash is write-protected.


Examples
--------

This first example uses sandbox::

   => sf probe
   SF: Detected m25p16 with page size 256 Bytes, erase size 64 KiB, total 2 MiB
   => sf read 1000 1100 80000
   device 0 offset 0x1100, size 0x80000
   SF: 524288 bytes @ 0x1100 Read: OK
   => md 1000
   00001000: edfe0dd0 f33a0000 78000000 84250000    ......:....x..%.
   00001010: 28000000 11000000 10000000 00000000    ...(............
   00001020: 6f050000 0c250000 00000000 00000000    ...o..%.........
   00001030: 00000000 00000000 00000000 00000000    ................
   00001040: 00000000 00000000 00000000 00000000    ................
   00001050: 00000000 00000000 00000000 00000000    ................
   00001060: 00000000 00000000 00000000 00000000    ................
   00001070: 00000000 00000000 01000000 00000000    ................
   00001080: 03000000 04000000 00000000 01000000    ................
   00001090: 03000000 04000000 0f000000 01000000    ................
   000010a0: 03000000 08000000 1b000000 646e6173    ............sand
   000010b0: 00786f62 03000000 08000000 21000000    box............!
   000010c0: 646e6173 00786f62 01000000 61696c61    sandbox.....alia
   000010d0: 00736573 03000000 07000000 2c000000    ses............,
   000010e0: 6332692f 00003040 03000000 07000000    /i2c@0..........
   000010f0: 31000000 6963702f 00003040 03000000    ...1/pci@0......
   => sf erase 0 80000
   SF: 524288 bytes @ 0x0 Erased: OK
   => sf read 1000 1100 80000
   device 0 offset 0x1100, size 0x80000
   SF: 524288 bytes @ 0x1100 Read: OK
   => md 1000
   00001000: ffffffff ffffffff ffffffff ffffffff    ................
   00001010: ffffffff ffffffff ffffffff ffffffff    ................
   00001020: ffffffff ffffffff ffffffff ffffffff    ................
   00001030: ffffffff ffffffff ffffffff ffffffff    ................
   00001040: ffffffff ffffffff ffffffff ffffffff    ................
   00001050: ffffffff ffffffff ffffffff ffffffff    ................
   00001060: ffffffff ffffffff ffffffff ffffffff    ................
   00001070: ffffffff ffffffff ffffffff ffffffff    ................
   00001080: ffffffff ffffffff ffffffff ffffffff    ................
   00001090: ffffffff ffffffff ffffffff ffffffff    ................
   000010a0: ffffffff ffffffff ffffffff ffffffff    ................
   000010b0: ffffffff ffffffff ffffffff ffffffff    ................
   000010c0: ffffffff ffffffff ffffffff ffffffff    ................
   000010d0: ffffffff ffffffff ffffffff ffffffff    ................
   000010e0: ffffffff ffffffff ffffffff ffffffff    ................
   000010f0: ffffffff ffffffff ffffffff ffffffff    ................

This second example is running on coral, an x86 Chromebook::

   => sf probe
   SF: Detected w25q128fw with page size 256 Bytes, erase size 4 KiB, total 16 MiB
   => sf erase 300000 80000
   SF: 524288 bytes @ 0x300000 Erased: OK
   => sf update 1110000 300000 80000
   device 0 offset 0x300000, size 0x80000
   524288 bytes written, 0 bytes skipped in 0.457s, speed 1164578 B/s

   # This does nothing as the flash is already updated
   => sf update 1110000 300000 80000
   device 0 offset 0x300000, size 0x80000
   0 bytes written, 524288 bytes skipped in 0.196s, speed 2684354 B/s
   => sf test 00000 80000   # try a protected region
   SPI flash test:
   Erase failed (err = -5)
   Test failed
   => sf test 800000 80000
   SPI flash test:
   0 erase: 18 ticks, 28444 KiB/s 227.552 Mbps
   1 check: 192 ticks, 2666 KiB/s 21.328 Mbps
   2 write: 227 ticks, 2255 KiB/s 18.040 Mbps
   3 read: 189 ticks, 2708 KiB/s 21.664 Mbps
   Test passed
   0 erase: 18 ticks, 28444 KiB/s 227.552 Mbps
   1 check: 192 ticks, 2666 KiB/s 21.328 Mbps
   2 write: 227 ticks, 2255 KiB/s 18.040 Mbps
   3 read: 189 ticks, 2708 KiB/s 21.664 Mbps


.. _SPI documentation:
   https://en.wikipedia.org/wiki/Serial_Peripheral_Interface

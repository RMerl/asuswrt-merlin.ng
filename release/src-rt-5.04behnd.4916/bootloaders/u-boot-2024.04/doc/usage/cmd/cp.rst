.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: cp (command)

cp command
==========

Synopsis
--------

::

    cp source target count
    cp.b source target count
    cp.w source target count
    cp.l source target count
    cp.q source target count

Description
-----------

The cp command is used to copy *count* chunks of memory from the *source*
address to the *target* address. If the *target* address points to NOR flash,
the flash is programmed. When the *target* address points at ordinary memory,
memmove() is used, so the two regions may overlap.

The number bytes in one chunk is defined by the suffix defaulting to 4 bytes:

====== ==========
suffix chunk size
====== ==========
.b     1 byte
.w     2 bytes
.l     4 bytes
.q     8 bytes
<none> 4 bytes
====== ==========

source
        source address, hexadecimal

target
        target address, hexadecimal

count
        number of words to be copied, hexadecimal

Examples
--------

The example device has a NOR flash where the lower part of the flash is
protected. We first copy to RAM, then to unprotected flash. Last we try to
write to protectd flash.

::

    => mtd list
    List of MTD devices:
    * nor0
      - device: flash@0
      - parent: root_driver
      - driver: cfi_flash
      - path: /flash@0
      - type: NOR flash
      - block size: 0x20000 bytes
      - min I/O: 0x1 bytes
      - 0x000000000000-0x000002000000 : "nor0"
    => cp.b 4020000 5000000 200000
    => cp.b 4020000 1e00000 20000
    Copy to Flash... done
    => cp.b 4020000 0 20000
    Copy to Flash... Can't write to protected Flash sectors
    =>

Configuration
-------------

The cp command is available if CONFIG_CMD_MEMORY=y. Support for 64 bit words
(cp.q) is only available on 64-bit targets. Copying to flash depends on
CONFIG_MTD_NOR_FLASH=y.

Return value
------------

The return value $? is set to 0 (true) if the command was successfully,
1 (false) otherwise.

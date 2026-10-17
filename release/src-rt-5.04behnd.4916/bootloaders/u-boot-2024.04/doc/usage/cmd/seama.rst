.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: seama (command)

seama command
=============

Synopsis
--------

::

    seama <dst_addr> <index>

Description
-----------

The seama command is used to load and decode SEAttle iMAges from NAND
flash to memory.

This type of flash image is found in some D-Link routers such as
DIR-645, DIR-842, DIR-859, DIR-860L, DIR-885L, DIR890L and DCH-M225,
as well as in WD and NEC routers on the ath79 (MIPS), Broadcom
BCM53xx, and RAMIPS platforms.

This U-Boot command will read and decode a SEAMA image from raw NAND
flash on any platform. As it is always using big endian format for
the data decoding is always necessary on platforms  such as ARM.

dst_addr
    destination address of the byte stream to be loaded

index
    the image index (0, 1, 2..) can be omitted

Example
-------

::

    => seama 0x01000000
    Loading SEAMA image 0 from nand0
    SEMA IMAGE:
      metadata size 36
      image size 8781764
      checksum 054859cfb1487b59befda98824e09dd6
    Decoding SEAMA image 0x01000040..0x01860004 to 0x01000000


Configuration
-------------

The command is available if CONFIG_CMD_SEAMA=y.

Return value
------------

The return value $? is set 0 (true) if the loading is succefull, and
is set to 1 (false) in case of error.

The environment variable $seama_image_size is set to the size of the
loaded SEAMA image.

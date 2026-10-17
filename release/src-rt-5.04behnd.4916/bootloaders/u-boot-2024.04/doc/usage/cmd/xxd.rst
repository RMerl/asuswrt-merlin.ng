.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: xxd (command)

xxd command
===========

Synopsis
--------

::

    xxd <interface> <dev[:part]> <file>

Description
-----------

The xxd command prints the file content as hexdump to standard out.

interface
    interface for accessing the block device (mmc, sata, scsi, usb, ....)

dev
    device number

part
    partition number, defaults to 1

file
    path to file

Example
-------

Here is the output for a example text file:

::

    => xxd mmc 0:1 hello
    00000000: 68 65 6c 6c 6f 20 77 6f 72 6c 64 0a 00 01 02 03  hello world.....
    00000010: 04 05                                            ..
    =>

Configuration
-------------

The xxd command is only available if CONFIG_CMD_XXD=y.

Return value
------------

The return value $? is set to 0 (true) if the file is readable, otherwise it returns a non-zero error code.

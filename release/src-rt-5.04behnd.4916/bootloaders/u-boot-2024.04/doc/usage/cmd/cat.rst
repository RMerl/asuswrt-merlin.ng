.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: cat (command)

cat command
===========

Synopsis
--------

::

    cat <interface> <dev[:part]> <file>

Description
-----------

The cat command prints the file content to standard out.

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

    => cat mmc 0:1 hello
    hello world
    =>

Configuration
-------------

The cat command is only available if CONFIG_CMD_CAT=y.

Return value
------------

The return value $? is set to 0 (true) if the file is readable, otherwise it returns a non-zero error code.
